#include "mm.h"

#include "errno.h"
#include "irq.h"
#include "memlayout.h"
#include "mmdefs.h"
#include "panic.h"
#include "printf.h"
#include "riscv.h"
#include "string.h"
#include "type.h"
#include "trap.h"

__asm__(
  ".section ._early.page_table\n\t"
  ".align 12\n"
  "early_page_table:\n\t"
  ".zero 0x1000\n\t"
  ".section .text"
);

struct page_table {
  sv39_pte entries[1 << 9];
};

extern struct page_table early_page_table[512];

static void map_kernel_pages (page_table_t table) {
  // set_page_1g could never fail
  set_page_1g(table, (void *) MMIOBASE, (void __phy *) MMIOPHY,
              PTE_VALID | PTE_READ | PTE_WRITE | PTE_GLOBAL);
  set_page_1g(table, (void *) KERNBASE, (void __phy *) KERNPHY,
              PTE_VALID | PTE_READ | PTE_WRITE | PTE_EXEC | PTE_GLOBAL);
}

void establish_identical_mapping () {
  printk("populating page table at %p\n", early_page_table);
  map_kernel_pages(early_page_table);
  csrw("satp", satp_from_table(early_page_table));
  __asm__ volatile("sfence.vma x0, x0");
}

static void *buddy_pages[MAX_ORDER];

static void split_area (u32 order) {
  u8 *area1 = buddy_pages[order];
  if (area1 == NULL) {
    panic("invalid split_area - no pages of order %d", order);
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
  u8 *page = (u8 *) KERN_PHYBASE + area_size;
  buddy_pages[order] = page;
  while (page < (u8 *) KERN_PHYSTOP) {
    u8 *next_page = page + area_size;
    *(void **)page = next_page;
    page = next_page;
  }
  *(void **) (KERN_PHYSTOP - area_size) = NULL;

  _Static_assert(SKIP_ORDER < MAX_ORDER, "");
  while (order >= SKIP_ORDER) {
    u8 *page = (u8 *) KERN_PHYBASE + (PAGE_SIZE << order);
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

void dump_buddy () {
  for (u32 order = 0; order < MAX_ORDER; ++order) {
    int count = 0;
    void **page = buddy_pages[order];
    while (page != NULL) {
      ++count;
      page = *page;
    }
    printk("Order %2d has %8d free areas\n", order, count);
  }
}

void mm_init () {
  printk("Initializing memory management.\n");
  page_allocator_init();

  // protect from kernel NULL dereferencing
  set_page(early_page_table, NULL, NULL, 0);
  __asm__ volatile("sfence.vma x0, x0");
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

  u64 area_size = (PAGE_SIZE << page.order);
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
    free_pages((struct page) {
      .address = (void *) ((u64) page.address & ~area_size),
      .order = page.order + 1,
    });
    return;
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


static inline sv39_va check_va_1g (void __mm *va) {
  sv39_va sv_va = { .ptr = va };
  if (sv_va.off != 0 || sv_va.vpn0 != 0 || sv_va.vpn1 != 0) {
    panic("misaligned 1G superpage virtual address %p", va);
  }
  return sv_va;
}

static inline sv39_va check_va_2m (void __mm *va) {
  sv39_va sv_va = { .ptr = va };
  if (sv_va.off != 0 || sv_va.vpn0 != 0) {
    panic("misaligned 2M superpage virtual address %p", va);
  }
  return sv_va;
}

static inline sv39_va check_va (void __mm *va) {
  sv39_va sv_va = { .ptr = va };
  if (sv_va.off != 0) {
    panic("misaligned page virtual address %p", va);
  }
  return sv_va;
}

int set_page_1g_user (page_table_t table, void __user *va, void __phy *pa, u32 flags) {
  return set_page_1g(table, (void *) va, pa, flags | PTE_USER);
}
int set_page_1g (page_table_t table, void __mm *va, void __phy *pa, u32 flags) {
  sv39_va sv_va = check_va_1g(va);
  sv39_pa sv_pa = { .ptr = pa };
  if (sv_pa.off != 0 || ((sv_pa.ppn0 != 0 || sv_pa.ppn1 != 0) && pte_is_leaf(flags))) {
    panic("misaligned superpage pa %p", va);
  }

  sv39_pte *pte = &table->entries[sv_va.vpn2];
  pte->flags = flags;
  pte->ppn = sv_pa.ppn;

  return 0;
}

static page_table_t expand_superpage (sv39_pte pte, int level) {
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

static sv39_pte *ensure_page_2m (page_table_t table, void __mm *va) {
  sv39_va sv_va = check_va_2m(va);
  page_table_t subtable;
  sv39_pte *pte2 = &table->entries[sv_va.vpn2];
  if (pte_invalid_or_leaf(*pte2)) {
    subtable = expand_superpage(*pte2, 1);
    if (subtable == NULL) return NULL;
    sv39_va subtable_va = sv_va;
    subtable_va.vpn1 = 0;
    set_page_1g(table, subtable_va.ptr, PA(subtable), PTE_VALID);
  } else {
    subtable = subtable_from_pte(*pte2);
  }

  return &subtable->entries[sv_va.vpn1];
}

int set_page_2m_user (page_table_t table, void __user *va, void __phy *pa, u32 flags) {
  return set_page_2m(table, (void *) va, pa, flags | PTE_USER);
}
int set_page_2m (page_table_t table, void __mm *va, void __phy *pa, u32 flags) {
  sv39_pa sv_pa = { .ptr = pa };
  if (sv_pa.off != 0 || (sv_pa.ppn0 != 0 && pte_is_leaf(flags))) {
    panic("misaligned superpage pa");
  }

  sv39_pte *pte1 = ensure_page_2m(table, va);
  if (pte1 == NULL) return -ENOMEM;
  pte1->flags = flags;
  pte1->ppn = sv_pa.ppn;

  return 0;
}

int set_page_user (page_table_t table, void __user *va, void __phy *pa, u32 flags) {
  return set_page(table, (void *) va, pa, flags | PTE_USER);
}
int set_page (page_table_t table, void __mm *va, void __phy *pa, u32 flags) {
  sv39_va sv_va = { .ptr = va };
  sv39_pa sv_pa = { .ptr = pa };
  if (sv_va.off != 0 || sv_pa.off != 0) {
    panic("misaligned page");
  }

  page_table_t subtable;
  sv39_va subtable_va = sv_va;
  subtable_va.vpn0 = 0;
  sv39_pte *pte1 = ensure_page_2m(table, subtable_va.ptr);
  if (pte1 == NULL) return -ENOMEM;
  if (pte_invalid_or_leaf(*pte1)) {
    subtable = expand_superpage(*pte1, 0);
    if (subtable == NULL) return -ENOMEM;
    set_page_2m(table, subtable_va.ptr, PA(subtable), PTE_VALID);
  } else {
    subtable = subtable_from_pte(*pte1);
  }

  sv39_pte *pte0 = &subtable->entries[sv_va.vpn0];
  pte0->flags = flags;
  pte0->ppn = sv_pa.ppn;

  return 0;
}

int unset_page (page_table_t table, void __mm *va) {
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

int unset_page_2m (page_table_t table, void __mm *va) {
  sv39_va sv_va = check_va_2m(va);

  sv39_pte pte2 = table->entries[sv_va.vpn2];
  if (pte_invalid_or_leaf(pte2)) return -ENOENT;
  page_table_t subtable1 = subtable_from_pte(pte2);

  sv39_pte *pte1 = &subtable1->entries[sv_va.vpn1];
  if (!pte_invalid_or_leaf(*pte1)) return -EINVAL;
  pte1->flags &= ~PTE_VALID;

  return 0;
}

int unset_page_1g (page_table_t table, void __mm *va) {
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
  map_kernel_pages(table);
  return table;
}

static void do_destroy_page_table (page_table_t table, int depth) {
  for (int i = 0; i < 1 << 9; ++i) {
    sv39_pte pte = table->entries[i];
    if (!pte_invalid_or_leaf(pte)) {
      if (depth == 0) {
        panic("invalid page table - too deep");
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

static void enter_sum () {
  may_page_fault = true;
  csrns("sstatus", status, sum);
}
static void exit_sum () {
  may_page_fault = false;
  csrnc("sstatus", status, sum);
}

static int user_memcpy (u8 *dest, u8 *src, size_t n) {
  int retval = 0;
  if (ksetjmp(&page_fault_handler)) {
    retval = -EFAULT;
    goto cleanup;
  }
  enter_sum();

  for (size_t i = 0; i < n; ++i) {
    dest[i] = src[i];
  }

  cleanup:
  exit_sum();
  return retval;
}

int copy_from_user (void *dest, void __user *src, size_t n) {
  if (!access_ok(src, n)) return -EFAULT;
  return user_memcpy(dest, (u8 *) src, n);
}

int copy_to_user (void __user *dest, void *src, size_t n) {
  if (!access_ok(dest, n)) return -EFAULT;
  return user_memcpy((u8 *) dest, src, n);
}

size_t user_strlcpy (u8 *dest, u8 __user *src, size_t n) {
  kassert(n > 0, "user_strlcpy: bad n");
  if (!access_ok(src, n)) return -EFAULT;
  size_t retval = 0;
  if (ksetjmp(&page_fault_handler)) {
    retval = -EFAULT;
    goto cleanup;
  }
  enter_sum();
  retval = strlcpy(dest, (u8 *) src, n);

  cleanup:
  exit_sum();
  return retval;
}

u8 **user_copy_args (u8 * __user *args) {
  if (!access_ok(args, sizeof(void *))) {
    return ERR_PTR(EFAULT);
  }
  void *buf = kmalloc(ARG_MAX);
  if (!buf) {
    return ERR_PTR(ENOMEM);
  }
  u8 **retval = buf;

  if (ksetjmp(&page_fault_handler)) {
    retval = ERR_PTR(EFAULT);
    goto cleanup;
  }
  enter_sum();

  i64 argc;
  let args_ok = (u8 **) args;
  for (argc = 0; args_ok[argc]; ++argc);
  i64 arg_size = (argc + 1) * sizeof(void *);
  i64 off = arg_size;
  if (arg_size > ARG_MAX) {
    return ERR_PTR(E2BIG);
  }

  for (i64 i = 0; i < argc; ++i) {
    let addr = ((u8 *) buf) + off;
    retval[i] = addr;
    size_t length = strlcpy(addr, args_ok[i], ARG_MAX - off);
    if (length >= ARG_MAX - off) {
      retval = ERR_PTR(E2BIG);
      goto cleanup;
    }
    off += length + 1;
  }
  retval[argc] = NULL;

  goto ok;

  cleanup: kfree(buf);
  ok: exit_sum();
  return retval;
}
