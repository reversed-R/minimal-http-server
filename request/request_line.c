#include "request_line.h"
#include "../utils/map/map.h"
#include "../utils/string/string_manipulator.h"
#include <stdlib.h>
#include <string.h>

map_int_str methods[] = {"GET",  HTTP_METHOD_GET, "HEAD", HTTP_METHOD_HEAD,
                         "POST", HTTP_METHOD_POST};

int match_method(char *str) {
  return match_str_in_map(str, methods, sizeof methods / sizeof methods[0]);
}

char *get_method_str_by_id(int method) {
  return match_int_in_map(method, methods, sizeof methods / sizeof methods[0]);
}

int parse_http_version(char *version_str, int *version_upper,
                       int *version_lower);

#define FULL_REQUEST 1
#define SIMPLE_REQUEST 0
#define PARSE_ERROR -1
int parse_request_line(char *line, int *method, char *uri, int *version_upper,
                       int *version_lower) {
  char *tokens[32];
  int token_count = split_str(line, " ", tokens, 32);
  if (token_count == 3) {
    *method = match_method(tokens[0]);
    strcpy(uri, tokens[1]);
    parse_http_version(tokens[2], version_upper, version_lower);

    return FULL_REQUEST;
  } else if (token_count == 2) {
    *method = match_method(tokens[0]);
    strcpy(uri, tokens[1]);
    *version_upper = 1;
    *version_lower = 0;

    return SIMPLE_REQUEST;
  } else {
    return PARSE_ERROR;
  }
}

int parse_http_version(char *version_str, int *version_upper,
                       int *version_lower) {
  char *tokens[32];
  int token_count = split_str(version_str, "/", tokens, 32);

  if (token_count == 2 && strcmp(tokens[0], "HTTP") == 0) {
    char *version_tokens[32];
    int version_token_count = split_str(tokens[1], ".", version_tokens, 32);

    if (version_token_count == 2) {
      *version_upper = atoi(version_tokens[0]);
      *version_lower = atoi(version_tokens[1]);

      return 1;
    }
  }

  return -1;
}
