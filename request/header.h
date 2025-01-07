#ifndef HEADER_H
#define HEADER_H
// enum request_headers {
//   AUTHORIZATION = 0,
//   FROM,
//   IF_MODIFIED_SINCE,
//   REFERER,
//   USER_AGENT,
//   CONNECTION
// };

// HTTP Header
#define HTTP_HEADER_CONNECTION 0
#define HTTP_HEADER_KEEP_ALIVE 1
#define HTTP_HEADER_CONTENT_LENGTH 2
#define HTTP_HEADER_CONTENT_TYPE 3
// #define HTTP_HEADER_CONTENT_TYPE 3
// #define HTTP_HEADER_HOST 4
// #define HTTP_HEADER_USER_AGENT 5
// #define HTTP_HEADER_ACCEPT 6
// #define HTTP_HEADER_ACCEPT_ENCODING 7
// #define HTTP_HEADER_ACCEPT_LANGUAGE 8

// HTTP Header, Connection:
#define HTTP_CONNECTION_CLOSE 0
#define HTTP_CONNECTION_KEEP_ALIVE 1

typedef struct {
  int id;
  char *header_type;
  char *params_str;
} Header;

typedef struct {
  int id;
  char *data;
} header;

typedef struct {
  int value;
} connection_h;

typedef struct {
  int length;
} content_length_h;

typedef struct {
  char *content_type;
} content_type_h;

typedef struct {
  connection_h *connection;
  content_length_h *content_length;
} Headers;

int parse_header_line(const char *header_line, char *header_type,
                      char *params_str);

int match_header(const char *header_type);

// static map_int_str connection_h_params_map[] = {
//     {"close", HTTP_CONNECTION_CLOSE},
//     {"keep-alive", HTTP_CONNECTION_KEEP_ALIVE}};
//
#endif
