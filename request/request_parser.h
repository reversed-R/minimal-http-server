#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#include "../utils/list/list.h"
#include "header.h"
#include <stdint.h>

// enum methods { GET_METHOD = 1, HEAD_METHOD, POST_METHOD };
#define HTTP_METHOD_GET 1
#define HTTP_METHOD_HEAD 2
#define HTTP_METHOD_POST 3

enum request_headers {
  AUTHORIZATION = 0,
  FROM,
  IF_MODIFIED_SINCE,
  REFERER,
  USER_AGENT,
  CONNECTION
};

typedef struct {
  int parse_success;
  int method;
  char *uri;
  int version_upper;
  int version_lower;
  container *headers;
  Headers *known_headers;
  uint8_t *entity_body;
} request;

request parse_request(int sock);

void print_request(request rq);

int read_request(int sock, char *buf, int bufsize, int *head_index,
                 request *rq);

#endif
