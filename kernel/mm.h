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
