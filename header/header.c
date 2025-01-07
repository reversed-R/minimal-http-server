#include "header.h"
#include "../utils/map/map.h"

/* int match_str_in_map(const char *str, map_int_str map[]); */

map_int_str header_types[] = {

#ifdef HTTP_HEADER_CONNECTION
    {
        "Connection",
        HTTP_HEADER_CONNECTION,
    },
#endif

};

/* int match_str_in_map(const char *str, map_int_str map[]) { */
/*   for (int i = 0; i < (sizeof(map) / sizeof(map[0])); i++) { */
/*     if (strcmp(str, map[i].name) == 0) { */
/*       return map[i].id; */
/*     } */
/*   } */
/*   return -1; */
/* } */

int match_header(const char *header_type) {
  return match_str_in_map(header_type, header_types,
                          sizeof header_types / sizeof header_types[0]);
}
