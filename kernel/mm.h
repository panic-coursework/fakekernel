#pragma once

#include "type.h"

void establish_identical_mapping ();

struct page {
  void *address;
  u32 order;
};

void mm_init ();
struct page alloc_pages (u32 order);
void free_pages (struct page page);

void *kmalloc (size_t size);
void kfree (void *buf);

typedef struct page_table *page_table_t;

page_table_t create_page_table ();
void destroy_page_table (page_table_t table);
void switch_to_page_table (page_table_t table, u32 asid);

int set_page (page_table_t table, u64 va, u64 pa, u32 flags);
int set_page_2m (page_table_t table, u64 va, u64 pa, u32 flags);
int set_page_1g (page_table_t table, u64 va, u64 pa, u32 flags);
int unset_page (page_table_t table, u64 va);
int unset_page_2m (page_table_t table, u64 va);
int unset_page_1g (page_table_t table, u64 va);
