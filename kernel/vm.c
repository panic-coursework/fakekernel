#include "vm.h"

#include "errno.h"
#include "list.h"
#include "memlayout.h"
#include "mm.h"
#include "mmdefs.h"
#include "panic.h"
#include "printf.h"

static void vm_area_check_alignment (struct vm_area *area) {
  if ((u64) area->va % PAGE_SIZE != 0) {
    panic("misaligned va %p", area->va);
  }
}

struct vm_area *vm_area_create (void __user *va, u64 n_pages, u32 flags) {
  struct vm_area *area = kmalloc(sizeof(struct vm_area));
  area->va = va;
  area->n_pages = n_pages;
  area->flags = flags;
  area->refcount = 0;
  list_init(&area->pages);
  vm_area_check_alignment(area);
  return area;
}

int vm_area_add_page (struct vm_area *area, void *page) {
  struct list_node *node = list_node_create(page);
  if (node == NULL) {
    return -ENOMEM;
  }
  list_push(&area->pages, node);
  return 0;
}

static void vm_area_destroy (struct vm_area *area) {
  void *page;
  list_foreach(page, &area->pages) {
    free_pages((struct page) { .address = page, .order = 0 });
  }
  list_destroy(&area->pages);
  kfree(area);
}

void vm_area_incref (struct vm_area *area) {
  vm_area_check_alignment(area);
  ++area->refcount;
  kassert(area->refcount > 0, "vm_area_incref: refcount overflow");
}

void vm_area_decref (struct vm_area *area) {
  vm_area_check_alignment(area);
  --area->refcount;
  if (area->refcount == 0) {
    vm_area_destroy(area);
  }
}


int vm_add_area (struct list *list, struct vm_area *area) {
  struct list_node *node = list_node_create(area);
  if (node == NULL) {
    return -ENOMEM;
  }
  vm_area_incref(area);

  for (struct list_node *n = list->first; n; n = n->next) {
    struct vm_area *a = n->value;
    if (a->va > area->va) {
      list_insert_before(list, node, n);
    }
  }
  list_push(list, node);
  return 0;
}

static inline bool vm_area_in (struct vm_area *area, void __user *va) {
  u64 size = area->n_pages << PAGE_INDEX_BITS;
  return va >= area->va && va <= (void __user *) ((u64) area->va + size);
}

struct vm_area *vm_find (struct list *list, void __user *va) {
  struct vm_area *area;
  list_foreach (area, list) {
    if (vm_area_in(area, va)) {
      return area;
    }
  }
  return NULL;
}

struct vm_area *vm_find_stack (struct list *list, void __user *va) {
  struct vm_area *area;
  list_foreach(area, list) {
    if (vm_area_in(area, va)) {
      return NULL;
    }
    if (area->va > va) {
      if (area->flags & VM_STACK) {
        return area;
      }
      return NULL;
    }
  }
  return NULL;
}

void vm_decref_all (struct list *list) {
  struct vm_area *area;
  list_foreach (area, list) {
    vm_area_decref(area);
  }
  list_destroy(list);
}

int vm_clone (struct list *list, struct task *task) {
  struct vm_area *area;
  list_foreach (area, list) {
    struct vm_area *area_copy = kmalloc(sizeof(*area_copy));
    if (!area_copy) {
      // TODO
      panic("out of memory");
    }
    let node = list_node_create(area_copy);
    if (!node) {
      panic("out of memory");
    }
    list_push(&task->vm_areas, node);

    *area_copy = *area;
    area_copy->refcount = 1;
    list_init(&area_copy->pages);

    u32 pte_flags = pte_flags_from_vm_flags(area->flags);

    let va = (u8 __user *) area->va;
    for (u64 i = 0; i < area->n_pages; ++i) {
      void *page = alloc_pages(0).address;
      if (!page) {
        panic("out of memory");
      }

      let node = list_node_create(page);
      if (!node) {
        panic("out of memory");
      }
      list_push(&area_copy->pages, node);

      int err = set_page_user(task->page_table, va, PA(page), pte_flags);
      if (err) {
        panic("set_page: %d", err);
      }

      copy_from_user(page, va, PAGE_SIZE);
      va += PAGE_SIZE;
    }
  }

  return 0;
}

u32 pte_flags_from_vm_flags (u32 vm_flags) {
  u32 pte_flags = PTE_VALID | PTE_USER;
  if (vm_flags | VM_READ)  pte_flags |= PTE_READ;
  if (vm_flags | VM_WRITE) pte_flags |= PTE_WRITE;
  if (vm_flags | VM_EXEC)  pte_flags |= PTE_EXEC;
  return pte_flags;
}

void vm_dump (struct list *list) {
  struct vm_area *area;
  list_foreach (area, list) {
    printk("%p-%p %08x ", area->va, vm_area_end_va(area), area->n_pages << PAGE_INDEX_BITS);
    putchar(area->flags & VM_READ  ? 'r' : '-');
    putchar(area->flags & VM_WRITE ? 'w' : '-');
    putchar(area->flags & VM_EXEC  ? 'x' : '-');
    if (area->flags & VM_STACK) printk(" [stack]");
    putchar('\n');
  }
}
