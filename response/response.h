#include "../request/header.h"
#include "../request/request_parser.h"
#include "../utils/list/list.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

typedef struct {
  int status;
  char *status_text;
} response_status;

// enum response_status_enum {
//   OK = 200,
//   BAD_REQUEST = 400,
//   NOT_FOUND = 404,
//   INTERNAL_SERVER_ERROR = 500,
// };

#define HTTP_STATUS_OK 200
#define HTTP_STATUS_NOT_FOUND 404
#define HTTP_STATUS_BAD_REQUEST 400
#define HTTP_STATUS_INTERNAL_SERVER_ERROR 500

// enum response_headers {
//   LOCATION = 0,
//   SERVER,
//   WWW_AUTHENTICATE,
//   ALLOW,
//   CONTENT_ENCODING,
//   CONTENT_LENGTH,
//   CONTENT_TYPE,
//   EXPIRES,
//   LAST_MODIFIED,
// };

typedef struct {
  int id;
  char *data;
} response_header;

typedef struct {
  content_length_h *content_length;
  content_type_h *content_type;
} response_headers;

typedef struct {
  int status;
  int version_upper;
  int version_lower;
  uint8_t *entity_body;
  container *headers;
  response_headers *known_headers;
} response;

// int respond_for_GET(int sock, char *uri);

// int respond_for_HEAD(int sock, char *uri);

// int respond_for_POST(int sock, char *uri);

int respond(int sock, request *rq);
