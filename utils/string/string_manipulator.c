#include "string_manipulator.h"
#include <string.h>

int cmp_part_str(char *sstr, char *lstr, int begin) {
  /* sstr is shorter than lstr which is longer */
  /* copy partial string of lstr to part */
  char part[strlen(sstr) + 1];
  for (int i = 0; i < strlen(sstr); i++) {
    part[i] = lstr[begin + i];
  }
  part[strlen(sstr)] = '\0';

  /* then compare sstr and part */
  return strcmp(sstr, part);
}

int split_str(char *str, char *regex, char **buf, size_t buflen) {
  int strlength = strlen(str);
  int length = 1;
  buf[0] = str;

  for (int i = 0; i + strlen(regex) <= strlength; i++) {
    if (cmp_part_str(regex, str, i) == 0) {
      str[i] = '\0';
      buf[length] = &str[i + strlen(regex)];
      length++;

      if (buflen <= length)
        break;
    }
  }

  return length;
}

int read_line_from_buf(const char *buf, int bufsize, char *dest) {
  char buf_copy[bufsize + 1];
  strcpy(buf_copy, buf);

  int i = 0;
  while (i < bufsize) {
    if (buf[i] == '\r') {
      buf_copy[i] = 0;
      strcpy(dest, buf_copy);
      if (i + 1 < bufsize && buf[i + 1] == '\n') {
        return i + 2;
      }
      return i + 1;
    }

    if (buf[i] == '\n') {
      buf_copy[i] = 0;
      strcpy(dest, buf_copy);
      return i + 1;
    }

    i++;
  }

  return bufsize + 1;
}

#define TRUE 1
#define FALSE 0
int is_empty_str(const char *str) {
  if (strlen(str) == 0) {
    return TRUE;
  }

  return FALSE;
}

int count_head_spaces(const char *str) {
  int count = 0;
  int i = 0;
  while (i < strlen(str)) {
    if (str[i] == ' ') {
      count++;
    }
    i++;
  }
  return count;
}

void copy_str_by_length(char *dest, const char *src, int length) {
  for (int i = 0; i < length; i++) {
    dest[i] = src[i];
  }
}
