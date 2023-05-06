#pragma once

#include "list.h"
#include "memlayout.h"
#include "type.h"

struct vm_area {
  u64 va;
  u64 n_pages;
  u32 refcount;
  u32 flags;
  struct list pages;
};

#define VM_READ  (1 << 0)
#define VM_WRITE (1 << 1)
#define VM_EXEC  (1 << 2)
#define VM_STACK (1 << 10)

struct vm_area *vm_area_create (u64 va, u64 n_pages, u32 flags);
void vm_area_incref (struct vm_area *area);
void vm_area_decref (struct vm_area *area);
int vm_area_add_page (struct vm_area *area, void *page);

static inline u64 vm_area_end_va (struct vm_area *area) {
  return area->va + (area->n_pages << PAGE_INDEX_BITS);
}

int vm_add_area (struct list *list, struct vm_area *area);
struct vm_area *vm_find (struct list *list, u64 va);
struct vm_area *vm_find_stack (struct list *list, u64 va);
void vm_decref_all (struct list *list);

void vm_dump (struct list *list);

u32 pte_flags_from_vm_flags (u32 vm_flags);
