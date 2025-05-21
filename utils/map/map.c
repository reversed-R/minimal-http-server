#include "map.h"
#include <string.h>

int match_str_in_map(const char *str, map_int_str map[], int mapsize) {
  for (int i = 0; i < mapsize; i++) {
    if (strcmp(str, map[i].name) == 0) {
      return map[i].id;
    }
  }
  return -1;
}

char *match_int_in_map(int id, map_int_str map[], int mapsize) {
  for (int i = 0; i < mapsize; i++) {
    if (id == map[i].id) {
      return map[i].name;
    }
  }
  return "";
}
