#include "mm.h"

#include "errno.h"
#include "memlayout.h"
#include "mmdefs.h"
#include "panic.h"
#include "printf.h"
#include "riscv.h"
#include "type.h"
#include "trap.h"

__asm__(
  ".section ._early.kernel_page_table\n\t"
  ".align 12\n"
  "kernel_page_table:\n\t"
  ".zero 0x1000"
);

struct page_table {
  sv39_pte entries[1 << 9];
};

extern struct page_table kernel_page_table[512];

static void populate_identical_mapping (page_table_t table) {
  sv39_pte pte = { .value = 0 };
  pte.flags = PTE_VALID | PTE_READ | PTE_WRITE | PTE_EXEC;
  for (int i = 0; i < 1 << 9; ++i) {
    pte.ppn2 = i;
    table->entries[i] = pte;
  }
}

void establish_identical_mapping () {
  printk("populating page table at %p\n", kernel_page_table);
  populate_identical_mapping(kernel_page_table);
  u64 satp = satp_from_table(kernel_page_table).value;
  __asm__(
    "sfence.vma x0, x0\n\t"
    "csrw satp, %0\n\t"
    "sfence.vma x0, x0"
    : : "r" (satp)
  );
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

static void map_trampoline_pages (page_table_t table) {
  set_page(table, TRAMPOLINE, (u64) &trampoline,
           PTE_VALID | PTE_READ | PTE_EXEC | PTE_GLOBAL);
  set_page(table, TRAPFRAME, (u64) &trapframe,
           PTE_VALID | PTE_READ | PTE_WRITE | PTE_GLOBAL);
}

void mm_init () {
  printk("Initializing memory management.\n");
  printk("trampoline page va %p pa %p\n", TRAMPOLINE, &trampoline);
  printk("trap frame      va %p pa %p\n", TRAPFRAME, &trapframe);
  page_allocator_init();

  // protect from kernel NULL dereferencing
  set_page(kernel_page_table, NULL, NULL, 0);
  map_trampoline_pages(kernel_page_table);
  __asm__("sfence.vma x0, x0");

  u64 satp = csrr("satp");
  trapframe.kernel_satp = satp;
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


static inline sv39_va check_va_1g (u64 va) {
  sv39_va sv_va = { .value = va };
  if (sv_va.off != 0 || sv_va.vpn0 != 0 || sv_va.vpn1 != 0) {
    panic("misaligned 1G superpage virtual address");
  }
  return sv_va;
}

static inline sv39_va check_va_2m (u64 va) {
  sv39_va sv_va = { .value = va };
  if (sv_va.off != 0 || sv_va.vpn0 != 0) {
    panic("misaligned 2M superpage virtual address");
  }
  return sv_va;
}

static inline sv39_va check_va (u64 va) {
  sv39_va sv_va = { .value = va };
  if (sv_va.off != 0) {
    panic("misaligned page virtual address");
  }
  return sv_va;
}

int set_page_1g (page_table_t table, u64 va, u64 pa, u32 flags) {
  sv39_va sv_va = check_va_1g(va);
  sv39_pa sv_pa = { .value = pa };
  if (sv_pa.off != 0 || ((sv_pa.ppn0 != 0 || sv_pa.ppn1 != 0) && pte_is_leaf(flags))) {
    panic("set_page_1g: misaligned superpage pa");
  }

  sv39_pte *pte = &table->entries[sv_va.vpn2];
  pte->flags = flags;
  pte->ppn = sv_pa.ppn;

  return 0;
}

static page_table_t expand_superpage (page_table_t table, sv39_pte pte, int level) {
  page_table_t subtable = alloc_pages(0).address;
  if (subtable == NULL) return NULL;

  for (int i = 0; i < 1 << 9; ++i) {
    subtable->entries[i] = pte;
    if (level == 1) {
      subtable->entries[i].ppn1 = i;
    } else if (level == 0) {
      subtable->entries[i].ppn0 = i;
    }
  }

  return subtable;
}

static sv39_pte *ensure_page_2m (page_table_t table, u64 va) {
  sv39_va sv_va = check_va_2m(va);
  page_table_t subtable;
  sv39_pte *pte2 = &table->entries[sv_va.vpn2];
  if (pte_invalid_or_leaf(*pte2)) {
    subtable = expand_superpage(table, *pte2, 1);
    if (subtable == NULL) return NULL;
    sv39_va subtable_va = sv_va;
    subtable_va.vpn1 = 0;
    set_page_1g(table, subtable_va.value, (u64) subtable, PTE_VALID);
  } else {
    subtable = subtable_from_pte(*pte2);
  }

  return &subtable->entries[sv_va.vpn1];
}

int set_page_2m (page_table_t table, u64 va, u64 pa, u32 flags) {
  sv39_pa sv_pa = { .value = pa };
  if (sv_pa.off != 0 || (sv_pa.ppn0 != 0 && pte_is_leaf(flags))) {
    panic("set_page_2m: misaligned superpage pa");
  }

  sv39_pte *pte1 = ensure_page_2m(table, va);
  if (pte1 == NULL) return -ENOMEM;
  pte1->flags = flags;
  pte1->ppn = sv_pa.ppn;

  return 0;
}

int set_page (page_table_t table, u64 va, u64 pa, u32 flags) {
  sv39_va sv_va = { .value = va };
  sv39_pa sv_pa = { .value = pa };
  if (sv_va.off != 0 || sv_pa.off != 0) {
    panic("set_page: misaligned page");
  }

  page_table_t subtable;
  sv39_va subtable_va = sv_va;
  subtable_va.vpn0 = 0;
  sv39_pte *pte1 = ensure_page_2m(table, subtable_va.value);
  if (pte1 == NULL) return -ENOMEM;
  if (pte_invalid_or_leaf(*pte1)) {
    subtable = expand_superpage(table, *pte1, 0);
    if (subtable == NULL) return -ENOMEM;
    set_page_2m(table, subtable_va.value, (u64) subtable, PTE_VALID);
  } else {
    subtable = subtable_from_pte(*pte1);
  }

  sv39_pte *pte0 = &subtable->entries[sv_va.vpn0];
  pte0->flags = flags;
  pte0->ppn = sv_pa.ppn;

  return 0;
}

int unset_page (page_table_t table, u64 va) {
  sv39_va sv_va = check_va(va);

  sv39_pte pte2 = table->entries[sv_va.vpn2];
  if (pte_invalid_or_leaf(pte2)) return -ENOENT;
  page_table_t subtable1 = subtable_from_pte(pte2);

  sv39_pte pte1 = subtable1->entries[sv_va.vpn1];
  if (pte_invalid_or_leaf(pte1)) return -ENOENT;
  page_table_t subtable0 = subtable_from_pte(pte1);

  sv39_pte *pte0 = &subtable0->entries[sv_va.vpn0];
  pte0->flags &= ~PTE_VALID;

  return 0;
}

int unset_page_2m (page_table_t table, u64 va) {
  sv39_va sv_va = check_va_2m(va);

  sv39_pte pte2 = table->entries[sv_va.vpn2];
  if (pte_invalid_or_leaf(pte2)) return -ENOENT;
  page_table_t subtable1 = subtable_from_pte(pte2);

  sv39_pte *pte1 = &subtable1->entries[sv_va.vpn1];
  if (!pte_invalid_or_leaf(*pte1)) return -EINVAL;
  pte1->flags &= ~PTE_VALID;

  return 0;
}

int unset_page_1g (page_table_t table, u64 va) {
  sv39_va sv_va = check_va_1g(va);

  sv39_pte *pte = &table->entries[sv_va.vpn1];
  if (!pte_invalid_or_leaf(*pte)) return -EINVAL;
  pte->flags &= ~PTE_VALID;

  return 0;
}


page_table_t create_page_table () {
  page_table_t table = alloc_pages(0).address;
  if (table == NULL) return NULL;
  for (int i = 0; i < 1 << 9; ++i) {
    table->entries[i].value = 0;
  }
  map_trampoline_pages(table);
  return table;
}

static void do_destroy_page_table (page_table_t table, int depth) {
  for (int i = 0; i < 1 << 9; ++i) {
    sv39_pte pte = table->entries[i];
    if (!pte_invalid_or_leaf(pte)) {
      if (depth == 1) {
        panic("destroy_page_table: invalid page table - too deep");
      }
      do_destroy_page_table(subtable_from_pte(pte), depth - 1);
    }
  }
  free_pages((struct page) {
    .address = table,
    .order = 0,
  });
}

void destroy_page_table (page_table_t table) {
  do_destroy_page_table(table, 2);
}
