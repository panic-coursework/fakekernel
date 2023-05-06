#pragma once

#include "mm.h"
#include "panic.h"
#include "type.h"

struct list_node {
  void *value;
  struct list *list;
  struct list_node *prev;
  struct list_node *next;
};

struct list {
  struct list_node *first;
  struct list_node *last;
};

static inline void list_init (struct list *list) {
  list->first = list->last = NULL;
}

static inline bool list_empty (struct list *list) {
  return list->first == NULL;
}

static inline struct list_node *list_node_create (void *value) {
  struct list_node *node = kmalloc(sizeof(struct list_node));
  if (node == NULL) return node;
  node->value = value;
  node->prev = node->next = NULL;
  return node;
}

static inline void list_push (struct list *list, struct list_node *node) {
  node->list = list;
  node->prev = list->last;
  if (list->last != NULL) list->last->next = node;
  list->last = node;
  if (list->first == NULL) list->first = node;
}

static inline void list_insert_before (
  struct list *list,
  struct list_node *node,
  struct list_node *pos
) {
  node->list = list;
  node->prev = pos->prev;
  node->next = pos;
  pos->prev = node;
  if (node->prev) node->prev->next = node;
  if (pos == list->first) list->first = node;
}

static inline struct list_node *list_shift (struct list *list) {
  struct list_node *node = list->first;
  if (node == NULL) return NULL;

  node->list = NULL;
  list->first = node->next;
  if (node == list->last) list->last = node->next;
  node->next = NULL;
  return node;
}

static inline void list_remove (struct list_node *node) {
  struct list *list = node->list;
  if (list == NULL) return;

  if (node->next != NULL) node->next->prev = node->prev;
  if (node->prev != NULL) node->prev->next = node->next;
  if (node == list->first) list->first = node->next;
  if (node == list->last) list->last = node->prev;
}

static inline void __list_foreach_step (struct list_node **node, void *value) {
  *node = (*node)->next;
  if (*node) *(void **) value = (*node)->value;
}

static inline struct list_node *__list_foreach_init (struct list *list, void *value) {
  struct list_node *node = list->first;
  if (node) *(void **) value = node->value;
  return node;
}

#define list_foreach(var, list) \
  for (struct list_node *__node = __list_foreach_init((list), &(var)); __node; __list_foreach_step(&__node, &(var)))
