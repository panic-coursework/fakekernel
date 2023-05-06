#pragma once

#include "mm.h"
#include "panic.h"
#include "type.h"

struct list_node {
  void *value;
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

static inline struct list_node *list_node_create (void *value) {
  struct list_node *node = kmalloc(sizeof(struct list_node));
  if (node == NULL) return node;
  node->value = value;
  node->prev = node->next = NULL;
  return node;
}

static inline void list_push (struct list *list, struct list_node *node) {
  node->prev = list->last;
  if (list->last != NULL) list->last->next = node;
  list->last = node;
  if (list->first == NULL) list->first = node;
}

static inline struct list_node *list_shift (struct list *list) {
  struct list_node *node = list->first;
  if (node == NULL) return NULL;

  list->first = node->next;
  if (node == list->last) list->last = node->next;
  node->next = NULL;
  return node;
}

static inline void list_remove (struct list *list, struct list_node *node) {
  if (node->next != NULL) node->next->prev = node->prev;
  if (node->prev != NULL) node->prev->next = node->next;
  if (node == list->first) list->first = node->next;
  if (node == list->last) list->last = node->prev;
}
