#include "mime_checker.h"
#include <magic.h>
#include <stdio.h>
#include <string.h>

int get_mime_type(const char *filename, char *typebuf) {
  magic_t magic;
  const char *buf;

  magic = magic_open(MAGIC_MIME_TYPE);
  if (magic == NULL) {
    printf("can NOT open magic\n");
    return -1;
  }

  if (magic_load(magic, NULL) != 0) {
    printf("can NOT load magic database - %s\n", magic_error(magic));
    magic_close(magic);
    return -2;
  }
  /* magic_compile(magic, NULL); */
  buf = magic_file(magic, filename);

  strcpy(typebuf, buf);

  magic_close(magic);

  return 0;
}
