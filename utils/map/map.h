
typedef struct {
  char *name;
  int id;
} map_int_str;

int match_str_in_map(const char *str, map_int_str map[], int mapsize);

char *match_int_in_map(int id, map_int_str map[], int mapsize);
