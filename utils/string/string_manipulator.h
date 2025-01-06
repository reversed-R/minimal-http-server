#include <stddef.h>

int cmp_part_str(char *sstr, char *lstr, int begin);

int split_str(char *str, char *regex, char **buf, size_t buflen);

int read_line_from_buf(const char *buf, int bufsize, char *dest);

int is_empty_str(const char *str);

int count_head_spaces(const char *str);

void copy_str_by_length(char *dest, const char *src, int length);
