#include "list.h"
#include <stdio.h>
#include <stdlib.h>

int list_size(container *list) {
  container *it = list;
  int size = 1;
  while (it->value != NULL) {
    it = (container *)it->next;
    size++;
  }

  return size;
}

container *list_get_last(container *list) {
  container *it = list;
  while (it->next != NULL) {
    it = (container *)it->next;
  }

  return it;
}

container *list_new(void *value) {
  container *new = malloc(sizeof(container));
  new->next = NULL;
  new->value = value;

  return new;
}

void list_append(container *list, void *value) {
  container *last = list_get_last(list);
  container *new = list_new(value);
  last->next = (struct container *)new;
}

void list_clear(container *list, int clear_value) {
  container *it = list;
  while (it->next != NULL) {
    container *next = (container *)it->next;
    if (clear_value) {
      free(it->value);
    }
    free(it);
    it = (container *)next;
  }
}

void list_print(container *list) {
  container *it = list;
  if (it == NULL) {
    printf("list: NULL\n");
    return;
  }

  do {
    printf("list.value:%ld\n", (long)it->value);
    it = (container *)it->next;
  } while (it->next != NULL);
}
