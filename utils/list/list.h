#define TRUE 1
#define FALSE 0

#ifndef LIST_H
#define LIST_H

typedef struct Container {
  struct container *next;
  void *value;
} container;

container *list_new(void *value);

int list_size(container *list);

container *list_get_last(container *list);

void list_append(container *list, void *value);

void list_clear(container *list, int clear_value);

void list_print(container *list);

#endif
