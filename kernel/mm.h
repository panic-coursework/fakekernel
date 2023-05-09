#pragma once

#include "main.h"
#include "memlayout.h"
#include "type.h"

void establish_identical_mapping ();

struct page {
  void *address;
  u32 order;
};

void mm_init ();
struct page alloc_pages (u32 order);
void free_pages (struct page page);
void dump_buddy ();

void *kmalloc (size_t size);
void kfree (void *buf);

typedef struct page_table *page_table_t;

page_table_t create_page_table ();
void destroy_page_table (page_table_t table);

int set_page (page_table_t table, void __mm *va, void __phy *pa, u32 flags);
int set_page_2m (page_table_t table, void __mm *va, void __phy *pa, u32 flags);
int set_page_1g (page_table_t table, void __mm *va, void __phy *pa, u32 flags);
int unset_page (page_table_t table, void __mm *va);
int unset_page_2m (page_table_t table, void __mm *va);
int unset_page_1g (page_table_t table, void __mm *va);

int set_page_user (page_table_t table, void __user *va, void __phy *pa, u32 flags);
int set_page_2m_user (page_table_t table, void __user *va, void __phy *pa, u32 flags);
int set_page_1g_user (page_table_t table, void __user *va, void __phy *pa, u32 flags);
static inline int unset_page_user (page_table_t table, void __user *va) {
  return unset_page(table, (void *) va);
}
static inline int unset_page_2m_user (page_table_t table, void __user *va) {
  return unset_page_2m(table, (void *) va);
}
static inline int unset_page_1g_user (page_table_t table, void __user *va) {
  return unset_page_1g(table, (void *) va);
}

#define TASK_SIZE_MAX (128*1024*1024)

static inline bool access_ok (void __user *addr, size_t size) {
  return size <= TASK_SIZE_MAX && (u64) addr <= (u64) SPLIT - size;
}

static inline void clear_page (void *page) {
  u64 *buf = page;
  for (u64 i = 0; i < PAGE_SIZE / sizeof(u64); ++i) {
    buf[i] = 0;
  }
}

int copy_from_user (void *dest, void __user *src, size_t n);
int copy_to_user (void __user *dest, void *src, size_t n);
size_t user_strlcpy (u8 *dest, u8 __user *src, size_t n);

static inline void __phy *PA (void __mm *ptr) {
  if (early) return (void __phy *) ptr;
  return (void __phy *) _PA((u64) ptr);
}

static inline void *VA (void __phy *ptr) {
  return (void *) _VA((u64) ptr);
}

#define ARG_MAX (PAGE_SIZE - 16)
u8 **user_copy_args (u8 * __user *args);
