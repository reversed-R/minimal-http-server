#include <string.h>

/* returns 0 when parsing succeeded , */
/* returns -1 when parsing failed  */
int parse_header_line(const char *header_line, char *header_type,
                      char *params_str) {
  char header_line_copy[strlen(header_line) + 1];
  strcpy(header_line_copy, header_line);

  char *p_colon = strstr(header_line_copy, ":");

  if (p_colon == NULL) {
    header_type = NULL;
    params_str = NULL;

    return -1;
  } else {
    *p_colon = 0;

    strcpy(header_type, header_line_copy);
    strcpy(params_str, p_colon + 1);

    return 0;
  }
}
