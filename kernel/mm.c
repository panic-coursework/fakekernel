#include "mm.h"

#include "memlayout.h"
#include "mmdefs.h"
#include "panic.h"
#include "printf.h"
#include "type.h"

static inline void set_satp (satp value) {
  __asm__(
    "csrw satp, %0\n\t"
    "sfence.vma"
    : : "r" (value.value)
  );
}

__asm__(
  ".section ._early.kernel_page_table\n\t"
  ".align 12\n\t"
  "kernel_page_table:\n\t"
  ".zero 0x1000"
);
extern u64 kernel_page_table[512];

void establish_identical_mapping () {
  printk("populating page table at %p\n", kernel_page_table);

  sv39_pte pte;
  pte.flags = PTE_VALID | PTE_READ | PTE_WRITE | PTE_EXEC;
  pte.rsw = 0;
  pte.ppn0 = 0;
  pte.ppn1 = 0;
  pte.ppn2 = 0;
  pte.reserved = 0;
  for (int i = 0; i < 1 << 9; ++i) {
    pte.ppn2 = i;
    kernel_page_table[i] = pte.value;
  }

  set_satp((satp) {
    .ppn = (long) kernel_page_table >> 12,
    .asid = 0,
    .mode = MM_SV39,
  });
}

static void *buddy_pages[MAX_ORDER];

static void split_area (u32 order) {
  u8 *area1 = buddy_pages[order];
  if (area1 == NULL) {
    panic("invalid split_area - no pages of order");
  }
  buddy_pages[order] = *(void **)area1;
  u8 *area2 = area1 + (PAGE_SIZE << (order - 1));
  *(void **)area1 = area2;
  *(void **)area2 = buddy_pages[order - 1];
  buddy_pages[order - 1] = area1;
}

static void page_allocator_init () {
  for (u32 i = 0; i < MAX_ORDER; ++i) {
    buddy_pages[i] = NULL;
  }

  u32 order = MAX_ORDER - 1;
  u64 area_size = PAGE_SIZE << order;
  u8 *page = (u8 *) KERNBASE + area_size;
  buddy_pages[order] = page;
  while (page < (u8 *) PHYSTOP) {
    u8 *next_page = page + area_size;
    *(void **)page = next_page;
    page = next_page;
  }
  *(void **) (PHYSTOP - area_size) = NULL;

  _Static_assert(SKIP_ORDER < MAX_ORDER, "");
  while (order >= SKIP_ORDER) {
    u8 *page = (u8 *) KERNBASE + (PAGE_SIZE << order);
    buddy_pages[order] = page;
    *(void **)page = NULL;
    --order;
  }
}

struct page alloc_pages (u32 order) {
  if (order >= MAX_ORDER) {
    printk("unable to satisfy alloc_pages request of order %d\n", order);
    panic("bad alloc_pages");
  }
  u32 promoted_order = order;
  while (promoted_order < MAX_ORDER && buddy_pages[promoted_order] == NULL) {
    ++promoted_order;
  }
  if (promoted_order == MAX_ORDER) {
    panic("out of memory");
  }
  while (promoted_order > order) {
    split_area(promoted_order);
    --promoted_order;
  }
  void **page = buddy_pages[order];
  buddy_pages[order] = *page;
  return (struct page) {
    .address = page,
    .order = order,
  };
}

void mm_init () {
  printk("Initializing memory management.\n");
  page_allocator_init();
}

void free_pages (struct page page) {
  if (page.order >= MAX_ORDER) {
    printk("bad free_pages of order %d\n", page.order);
    panic("bad free_pages");
  }

  if (page.order == MAX_ORDER - 1) {
    *(void **)page.address = buddy_pages[page.order];
    buddy_pages[page.order] = page.address;
    return;
  }

  u64 area_size = (PAGE_SIZE >> page.order);
  void *buddy = (void *) ((u64) page.address ^ area_size);
  void **prev = &buddy_pages[page.order];
  void **free = *prev;
  while (free != NULL) {
    if (free == buddy) break;
    prev = free;
    free = *free;
  }
  if (free == buddy) {
    *prev = *free;
    return free_pages((struct page) {
      .address = page.address,
      .order = page.order + 1,
    });
  }
  *(void **)page.address = buddy_pages[page.order];
  buddy_pages[page.order] = page.address;
}


void *kmalloc (size_t size) {
  // TODO: finer-grained malloc
  u32 order = 0;
  u64 pages = (size + PAGE_SIZE - 1 + sizeof(order)) / PAGE_SIZE;
  while (pages > 1) {
    ++order;
    pages /= 2;
  }
  u8 *page = alloc_pages(order).address;
  *(u32 *)page = order;
  return page + sizeof(order);
}

void kfree (void *buf) {
  u32 *page = (u32 *) ((u8 *) buf - sizeof(u32));
  free_pages((struct page) {
    .address = page,
    .order = *page,
  });
}
