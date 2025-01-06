#include "header.h"
#include "../utils/map/map.h"
#include <string.h>

/* int match_str_in_map(const char *str, map_int_str map[]); */

map_int_str header_types[] = {
    "Connection",
    HTTP_HEADER_CONNECTION,
};

/* int match_str_in_map(const char *str, map_int_str map[]) { */
/*   for (int i = 0; i < (sizeof(map) / sizeof(map[0])); i++) { */
/*     if (strcmp(str, map[i].name) == 0) { */
/*       return map[i].id; */
/*     } */
/*   } */
/*   return -1; */
/* } */

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

int match_header(const char *header_type) {
  return match_str_in_map(header_type, header_types,
                          sizeof header_types / sizeof header_types[0]);
}
