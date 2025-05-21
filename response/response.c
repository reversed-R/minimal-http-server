#define _OPEN_SYS_ITOA_EXT
#include "response.h"
#include "../utils/list/list.h"
#include "../utils/mime/mime_checker.h"
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int respond(int sock, request *rq);

void respond_status_line(int sock, int status);

void respond_header(int sock, response_header *header);

char *get_status_text(int status);

int build_response(request *rq, response *rs);

int write_response(int sock, response *rs);

int write_headers(int sock, response_headers *headers);

response_status rsp_stats[] = {
    {200, "200 OK"},
    {400, "400 Bad Request"},
    {404, "404 Not Found"},
    {500, "500 Internal Server Error"},
};

/* static char **response_header_types = { */
/*     "Location",     "Server",           "WWW-Authenticate", */
/*     "Allow",        "Content-Encoding", "Content-Length", */
/*     "Content-Type", "Expires",          "Last-Modified", */
/* }; */

static struct response_header_type {
  char *name;
  int id;
} response_header_types[] = {
    /* "Location", LOCATION, "Server", SERVER, "WWW-Authenticate",
       WWW_AUTHENTICATE, "Allow", ALLOW, "Content-Encoding", CONTENT_ENCODING,
     "Expires", EXPIRES, "Last-Modified", LAST_MODIFIED */
    "Content-Length",
    HTTP_HEADER_CONTENT_LENGTH,
    "Content-Type",
    HTTP_HEADER_CONTENT_TYPE,
};

int respond(int sock, request *rq) {
  /* initialization----> */
  response rs;
  rs.entity_body = NULL;
  rs.version_upper = 1;
  rs.version_lower = 0;
  rs.headers = NULL;

  response_headers known_headers;

  content_length_h content_length;
  content_type_h content_type;

  known_headers.content_length = &content_length;
  known_headers.content_type = &content_type;

  rs.known_headers = &known_headers;
  /* <----initialization */

  build_response(rq, &rs);

  write_response(sock, &rs);

  list_clear(rs.headers, 1);
  free(rs.known_headers->content_type->content_type);
  free(rs.entity_body);

  return 0;
}

int build_response(request *rq, response *rs) {
  /**** read requested resource----> ****/
  int fd;
  FILE *f;
  int file_size;
  struct stat st_file;

  char *homedir = "./resources";
  char filename[strlen(rq->uri) + strlen(homedir) + strlen("index.html") + 1];
  strcpy(filename, homedir);
  strcat(filename, rq->uri);

  int st_result;
  if ((st_result = stat(filename, &st_file)) != 0) {
    if (st_result == ENOENT) {
      rs->status = HTTP_STATUS_NOT_FOUND;
    } else {
      rs->status = HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

    rs->known_headers->content_length->length = 0;

    return 1;
  }

  if (S_ISDIR(st_file.st_mode)) {
    char *index = "index.html";
    strcat(filename, index);
    if ((st_result = stat(filename, &st_file)) != 0) {
      if (st_result == ENOENT) {
        rs->status = HTTP_STATUS_NOT_FOUND;
      } else {
        rs->status = HTTP_STATUS_INTERNAL_SERVER_ERROR;
      }

      rs->known_headers->content_length->length = 0;

      return 1;
    }
  }

  /* Content-Length header */
  rs->known_headers->content_length->length = st_file.st_size;

  {
    char *content_type = malloc(1024);
    int r = get_mime_type(filename, content_type);

    rs->known_headers->content_type->content_type = content_type;
  }

  fd = open(filename, O_RDONLY);
  if (fd == -1) {
    rs->status = HTTP_STATUS_NOT_FOUND;

    rs->known_headers->content_length->length = 0;

    return 1;
  }

  /* entity body ----> */
  uint8_t *body = calloc(1, sizeof(uint8_t) * file_size + 1);
  ssize_t s = read(fd, body, file_size);
  /* print */
  /* printf("size:%ld\n", s); */
  /* ssize_t size = s; */
  /* uint8_t *body_pointer = body; */
  /* while (size-- > 0) { */
  /*   printf("%c", (char)*(body_pointer++)); */
  /* } */
  /* print */
  rs->entity_body = body;
  /* <---- entity body */

  close(fd);
  /**** <---- read requested resource ****/

  /* if no error occured, response status will 200 OK */
  rs->status = HTTP_STATUS_OK;

  return 0;
}

int write_response(int sock, response *rs) {
  int body_size = rs->known_headers->content_length->length;

  printf("response---->\n");
  /****status line****/
  char status_line[1024];
  printf("HTTP/%d.%d %s\r\n", rs->version_upper, rs->version_lower,
         get_status_text(rs->status));
  sprintf(status_line, "HTTP/%d.%d %s\r\n", rs->version_upper,
          rs->version_lower, get_status_text(rs->status));
  write(sock, status_line, strlen(status_line));

  /****headers****/
  container *it = rs->headers;
  if (it != NULL) {
    while (1) {
      if (it->value == NULL) {
        continue;
      }
      respond_header(sock, (response_header *)it->value);

      if (it->next == NULL) {
        break;
      }
      it = (container *)it->next;
    }
  }

  write_headers(sock, rs->known_headers);

  char crlf[] = "\r\n";
  write(sock, crlf, strlen(crlf));
  printf("\r\n");

  /****entity body****/
  if (rs->entity_body != NULL) {
    write(sock, rs->entity_body, body_size);
    write(sock, crlf, strlen(crlf));

#ifdef HTTP_LOG_ROW_ENTITY_BODY
    ssize_t size = body_size;
    uint8_t *body_pointer = rs->entity_body;
    while (size-- > 0) {
      printf("%c", (char)*(body_pointer++));
    }
#else
    printf("[entity body]\r\n");
#endif
  } else {
    write(sock, crlf, strlen(crlf));
#ifdef HTTP_LOG_ROW_ENTITY_BODY
    printf("\r\n");
#else
    printf("[no entity body]\r\n");
#endif
  }

  printf("<----response\n");
  fflush(stdout);

  return 0;
}

char *get_status_text(int status) {
  int i = 0;
  while (i < sizeof rsp_stats / sizeof rsp_stats[0]) {
    if (status == rsp_stats[i].status) {
      return rsp_stats[i].status_text;
    }
    i++;
  }

  return NULL;
}

void respond_status_line(int sock, int status) {
  char version[] = "HTTP/1.0 ";
  write(sock, version, strlen(version));

  char *status_text = get_status_text(status);
  write(sock, status_text, strlen(status_text));

  char crlf[] = "\r\n";
  write(sock, crlf, strlen(crlf));
}

char *get_response_header_type(int id) {
  char *result = "";
  /* if (id >= sizeof response_header_types / sizeof response_header_types[0]) {
   */
  /*   return result; */
  /* } */

  int i = 0;
  while (i < sizeof response_header_types / sizeof response_header_types[0]) {
    if (response_header_types[i].id == id) {
      result = response_header_types[i].name;
      break;
    }
    i++;
  }

  return result;
}

void respond_header(int sock, response_header *header) {
  char *header_type = get_response_header_type(header->id);
  // + 5 means ": ", "\r\n\0"
  char header_str[strlen(header_type) + strlen(header->data) + 5];
  sprintf(header_str, "%s: %s\r\n", header_type, header->data);

  write(sock, header_str, strlen(header_str));
  printf("%s: %s\r\n", header_type, header->data);
}

int write_headers(int sock, response_headers *headers) {
  char content_length_header_line[strlen("Content-Length: \r\n") +
                                  (int)ceil(
                                      log10(headers->content_length->length)) +
                                  1];
  sprintf(content_length_header_line, "Content-Length: %d\r\n",
          headers->content_length->length);
  write(sock, content_length_header_line, strlen(content_length_header_line));
  printf("%s", content_length_header_line);

  char content_type_header_line[strlen("Content-Type: \r\n") +
                                strlen(headers->content_type->content_type) +
                                1];
  sprintf(content_type_header_line, "Content-Type: %s\r\n",
          headers->content_type->content_type);
  write(sock, content_type_header_line, strlen(content_type_header_line));
  printf("%s", content_type_header_line);

  return 0;
}
