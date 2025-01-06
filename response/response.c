#define _OPEN_SYS_ITOA_EXT
#include "response.h"
#include "../utils/list/list.h"
#include "../utils/mime/mime_checker.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

response_status rsp_stats[] = {
    {200, "200 OK"},
    {400, "400 Bad Request"},
    {404, "404 Not Found"},
    {500, "500 Internal Server Error"},
};

/* static char ** response_header_types */
/*   = { */
/*   "Location",         */
/*   "Server",           */
/*   "WWW-Authenticate", */
/*   "Allow",            */
/*   "Content-Encoding", */
/*   "Content-Length",   */
/*   "Content-Type",     */
/*   "Expires",          */
/*   "Last-Modified",    */
/* }; */

static struct response_header_type {
  char *name;
  int id;
} response_header_types[] = {
    "Location",         LOCATION,         "Server",         SERVER,
    "WWW-Authenticate", WWW_AUTHENTICATE, "Allow",          ALLOW,
    "Content-Encoding", CONTENT_ENCODING, "Content-Length", CONTENT_LENGTH,
    "Content-Type",     CONTENT_TYPE,     "Expires",        EXPIRES,
    "Last-Modified",    LAST_MODIFIED};

int respond(int sock, response rs, ssize_t body_size) {
  printf("response---->\n");
  /****status line****/
  char status_line[1024];
  printf("HTTP/%d.%d %s\r\n", rs.version_upper, rs.version_lower,
         get_status_text(rs.status));
  sprintf(status_line, "HTTP/%d.%d %s\r\n", rs.version_upper, rs.version_lower,
          get_status_text(rs.status));
  write(sock, status_line, strlen(status_line));

  /****headers****/
  container *it = rs.headers;
  if (it == NULL) {
    printf("<----response\n");
    return 1;
  }
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

  char crlf[] = "\r\n";
  write(sock, crlf, strlen(crlf));
  printf("\r\n");

  /****entity body****/
  /* printf("entity_body---->\n%s\n<----entity_body\n", rs.entity_body); */
  if (rs.entity_body != NULL) {
    write(sock, rs.entity_body, body_size);
    write(sock, crlf, strlen(crlf));
    if (rs.entity_body[body_size - 1] != (uint8_t)'\n') {
      write(sock, crlf, strlen(crlf));
    }
#ifdef HTTP_LOG_ROW_ENTITY_BODY
    /* print */
    ssize_t size = body_size;
    uint8_t *body_pointer = rs.entity_body;
    while (size-- > 0) {
      printf("%c", (char)*(body_pointer++));
    }
#else
    printf("[entity body]\n");
#endif
    printf("\r\n");
    if (rs.entity_body[body_size - 1] != (uint8_t)'\n') {
      printf("\r\n");
    }
    /* print */
  } else {
#ifndef HTTP_LOG_ROW_ENTITY_BODY
    printf("[no entity body]\n");
#endif
    write(sock, crlf, strlen(crlf));
    printf("\r\n");
  }

  printf("<----response\n");
  fflush(stdout);

  list_clear(rs.headers, 1);

  return 0;
}

int respond_for_GET(int sock, char *uri) {
  /* printf("\nGET requested, uri = %s\n", uri); */

  response rs;
  /* initialization */
  rs.entity_body = NULL;
  rs.version_upper = 1;
  rs.version_lower = 0;
  rs.headers = NULL;
  /* response_header *pnewheader = malloc(sizeof(response_header)); */
  /* pnewheader = NULL; */
  /* rs.headers = list_new(pnewheader); */

  /**** read requested resource----> ****/
  int fd;
  FILE *f;
  int file_size;
  struct stat st_file;

  char *homedir = "./resources";
  char filename[strlen(uri) + strlen(homedir) + strlen("index.html") + 1];
  strcpy(filename, homedir);
  strcat(filename, uri);

  int st_result;
  if ((st_result = stat(filename, &st_file)) != 0) {
    if (st_result == ENOENT) {
      rs.status = NOT_FOUND;
    } else {
      rs.status = INTERNAL_SERVER_ERROR;
    }

    respond(sock, rs, 0);
    return 1;
  }

  if (S_ISDIR(st_file.st_mode)) {
    char *index = "index.html";
    strcat(filename, index);
    if ((st_result = stat(filename, &st_file)) != 0) {
      if (st_result == ENOENT) {
        rs.status = NOT_FOUND;
      } else {
        rs.status = INTERNAL_SERVER_ERROR;
      }

      respond(sock, rs, 0);
      return 1;
    }
  }

  {
    file_size = st_file.st_size;
    /* printf("file_size:%d\n", file_size); */
    char *size_str = malloc(1024);
    /* printf("file_size pointer:%ld", (long)size_str); */
    /* itoa(file_size, size_str, DECIMAL); */
    sprintf(size_str, "%d", file_size);
    response_header *content_length_header = malloc(sizeof(response_header));
    content_length_header->id = CONTENT_LENGTH;
    content_length_header->data = size_str;
    rs.headers = list_new(content_length_header);
  }

  {
    char *content_type = malloc(1024);
    /* printf("content_type pointer:%ld\n", (long)content_type); */
    int r = get_mime_type(filename, content_type);
    /* printf("content_type:%s, r:%d\n", content_type, r); */

    response_header *content_type_header = malloc(sizeof(response_header));
    content_type_header->id = CONTENT_TYPE;
    content_type_header->data = content_type;
    list_append(rs.headers, content_type_header);
  }

  fd = open(filename, O_RDONLY);
  if (fd == -1) {
    /* respond_status_line(sock, NOT_FOUND); */
    rs.status = NOT_FOUND;
    respond(sock, rs, 0);
    return 1;
  }

  /* list_print(rs.headers); */

  /* entity body ----> */
  /* char body[file_size + 1]; */
  /* void *body = malloc(sizeof(void) * file_size + 1); */
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
  rs.entity_body = body;
  if (rs.entity_body == NULL) {
    printf("entity_body == NULL\n");
  }
  /* <---- entity body */

  close(fd);
  /* fclose(f); */
  /**** <---- read requested resource ****/

  /* if no error occured, response status will 200 OK */
  rs.status = OK;

  /*respond, with using response structure*/
  respond(sock, rs, s);

  return 0;
}

int respond_for_HEAD(int sock, char *uri) {
  printf("\nPOST requested, uri = %s\n", uri);
  response rs;
  /* initialization */
  rs.entity_body = NULL;
  rs.version_upper = 1;
  rs.version_lower = 0;

  respond_status_line(sock, OK);

  return 0;
}

int respond_for_POST(int sock, char *uri) {
  printf("\nPOST requested, uri = %s\n", uri);
  response rs;
  /* initialization */
  rs.entity_body = NULL;
  rs.version_upper = 1;
  rs.version_lower = 0;

  respond_status_line(sock, OK);

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
  if (id >= sizeof response_header_types / sizeof response_header_types[0]) {
    return result;
  }

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

  /* printf("%d: %s\r\n", header->id, header->data); */
  /* printf("%s: %s\r\n", */
  /*   response_header_types[header->id], */
  /*   header->data */
  /* ); */
  /* sprintf(header_str, */
  /*   "%s: %s\r\n", */
  /*   response_header_types[header->id], */
  /*   header->data */
  /* ); */
  write(sock, header_str, strlen(header_str));
  printf("%s: %s\r\n", header_type, header->data);
}
