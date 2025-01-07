#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../header/header.h"
#include "../utils/list/list.h"
#include "../utils/string/string_manipulator.h"
#include "request_line.h"
#include "request_parser.h"

#define TRUE 1
#define FALSE 0

void print_request(request rq);

void print_request(request rq) {
  printf("request.method:%d\n", rq.method);

  printf("request.uri:%s\n", rq.uri);

  container *it = rq.headers;
  int i = 0;
  while (it->next != NULL) {
    printf("request.headers[%d].value->id, data:%d,%s\n", i,
           ((header *)it->value)->id, ((header *)it->value)->data);
    it = (container *)it->next;
    i++;
  }
}

#define BUFSIZE 1024
request parse_request(int sock) {
  request rq;

  int r;
  char buf[BUFSIZE];

  int in_entity_body = FALSE;
  int i = 0;

  while ((r = read(sock, &buf[i], BUFSIZE - i)) > 0) {
    char buf_copy[BUFSIZE + 1];
    strcpy(buf_copy, buf);
    int line_cnt = 0;

    while (i <= BUFSIZE) {
      if (!in_entity_body) {
        char line[BUFSIZE - i + 1];
        int length = read_line_from_buf(&buf_copy[i], BUFSIZE - i, line);
        if (i + length <= BUFSIZE) {
          i += length;
          line_cnt++;
        } else {
          /* copy and concat final remain string */
          strcpy(buf, &buf_copy[i]);
          i = BUFSIZE - i;
          break;
        }

        if (line_cnt == 1) {
          /* parse request line  */
          rq.uri = malloc(sizeof(char) * BUFSIZE);
          parse_request_line(line, &rq.method, rq.uri, &rq.version_upper,
                             &rq.version_lower);

          printf("%s %s HTTP/%d.%d\n", get_method_str_by_id(rq.method), rq.uri,
                 rq.version_upper, rq.version_lower);
        } else {
          if (is_empty_str(line)) {
            in_entity_body = TRUE;
            printf("\n");

            /* entity body procedure */
            return rq;
          } else {
            header *pheader = malloc(sizeof(header));
            char header_type[BUFSIZE];
            char params_str[BUFSIZE];

            if (parse_header_line(line, header_type, params_str) == -1) {
              pheader->id = -1; // parse failed
            } else {
              int header_type_id = match_header(header_type);
              switch (header_type_id) {
              case HTTP_HEADER_CONTENT_LENGTH:
                rq.known_headers->content_length->length =
                    atoi(&params_str[count_head_spaces(params_str)]);
                break;
              }
              pheader->id = match_header(header_type); // if failed, return -1
              pheader->data = params_str;

              if (line_cnt == 2) {
                rq.headers = list_new(pheader);
              } else {
                list_append(rq.headers, pheader);
              }
              printf("%s:%s\n", header_type, params_str);
            }
          }
        }
      } else {
        ;
      }
    }
  }

  return rq;
}

/* rq entity_body should be set NULL before call and after call if it is NULL,
 * must free */
/* rq headers should be set NULL before call and after call if it is NULL,
 * must free */
/* returns 0 when correctly finish reading, */
/* returns 1 or larger when correctly read a request however readable remains,
 */
/* returns -1 or smaller when incorrectly finished */
int read_request(int sock, char *buf, int bufsize, int *head_index,
                 request *rq) {
  /* int in_entity_body = FALSE; */
  int read_length;
  int i = *head_index;

  /* initialzation of request */
  rq->known_headers->content_length->length = 0;

  printf("request---->\n");
  while ((read_length = read(sock, &buf[i], bufsize - i)) > 0) {
    char buf_copy[bufsize];
    copy_str_by_length(buf_copy, buf, read_length);
    int line_cnt = 0;

    while (i <= bufsize) {
      char line[bufsize - i + 1];
      int length = read_line_from_buf(&buf_copy[i], bufsize - i, line);
      if (i + length <= bufsize) {
        i += length;
        line_cnt++;
      } else {
        /* copy and concat final remain string */
        copy_str_by_length(buf, &buf_copy[i], read_length - i);
        i = bufsize - i;
        break;
      }

      if (line_cnt == 1) {
        /* parse request line  */
        rq->uri = malloc(sizeof(char) * bufsize);
        int r = parse_request_line(line, &rq->method, rq->uri,
                                   &rq->version_upper, &rq->version_lower);
        printf("%s %s HTTP/%d.%d\n", get_method_str_by_id(rq->method), rq->uri,
               rq->version_upper, rq->version_lower);
        if (r < 0) {
          printf("<----request (parse failed)\n");
          return -1;
        }

#ifdef MINIMAL_HTTP_SERVER_LOG
        printf("%s %s HTTP/%d.%d\n", get_method_str_by_id(rq->method), rq->uri,
               rq->version_upper, rq->version_lower);
#endif
      } else {
        if (is_empty_str(line)) {
          printf("[empty line]\n");
          /* in_entity_body = TRUE; */
#ifdef MINIMAL_HTTP_SERVER_LOG
          printf("\n");
          if (rq->known_headers->content_length->length == 0) {
            printf("[no entity body]\n")
          } else {
            printf("[entity body]\n")
          }
#endif

          /* entity body procedure */
          if (rq->known_headers->content_length->length != 0) {
            rq->entity_body =
                malloc(sizeof(void *) *
                       (rq->known_headers->content_length->length + 1));
            copy_str_by_length((char *)rq->entity_body, &buf_copy[i],
                               read_length - i);
            read(sock, &rq->entity_body[read_length - i],
                 rq->known_headers->content_length->length - read_length + i);
            rq->entity_body[rq->known_headers->content_length->length] = 0;
            printf("[entity body]\n");
          } else {
            printf("[no entity body]\n");
          }

          printf(
              "<----request (parse succeeded, next request still remains)\n");
          return 1;
        } else {
          char header_type[bufsize];
          char params_str[bufsize];

          if (parse_header_line(line, header_type, params_str) != -1) {
            int header_type_id = match_header(header_type);
            if (header_type_id >= 0) {
              switch (header_type_id) {
              case HTTP_HEADER_CONTENT_LENGTH:
                rq->known_headers->content_length->length =
                    atoi(&params_str[count_head_spaces(params_str)]);
                printf("Content-Length:%s\n", params_str);
                break;
              case HTTP_HEADER_CONNECTION:
                if (strcmp(&params_str[count_head_spaces(params_str)],
                           "keep-alive") == 0) {
                  rq->known_headers->connection->value =
                      HTTP_CONNECTION_KEEP_ALIVE;
                }
                if (strcmp(&params_str[count_head_spaces(params_str)],
                           "close") == 0) {
                  rq->known_headers->connection->value = HTTP_CONNECTION_CLOSE;
                }
                printf("Connection:%s\n", params_str);
                break;
              }
            } else {
              printf("%s:%s\n", header_type, params_str);
              header *pheader = malloc(sizeof(header));
              pheader->id = header_type_id; // if failed, return -1
              pheader->data = params_str;

              if (line_cnt == 2) { // should change to rq->headers == NULL ?
                rq->headers = list_new(pheader);
              } else {
                list_append(rq->headers, pheader);
              }
            }
#ifdef MINIMAL_HTTP_SERVER_LOG
            printf("%s:%s\n", header_type, params_str);
#endif
          } else {
            printf("<----request (parse failed)\n");
            return -1;
          }
        }
      }
    }
  }

  printf("<----request (no request remains)\n");
  return 0;
}
