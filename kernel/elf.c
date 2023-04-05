#include "elf.h"

#include "memlayout.h"
#include "mm.h"
#include "mmdefs.h"
#include "printf.h"

page_table_t load_elf (elf program) {
  page_table_t table = create_page_table();
  if (table == NULL) return NULL;

  // TODO: sanity check
  for (int i = 0; i < program->e_shnum; ++i) {
    Elf64_Shdr *shdr = (void *) ((u64) program + program->e_shoff +
                                 i * program->e_shentsize);
    if (shdr->sh_type == SHT_NULL || shdr->sh_size == 0) continue;
    if (!(shdr->sh_flags & SHF_ALLOC)) continue;
    u32 flags = PTE_VALID | PTE_READ | PTE_USER;
    if (shdr->sh_flags & SHF_WRITE) flags |= PTE_WRITE;
    if (shdr->sh_flags & SHF_EXECINSTR) flags |= PTE_EXEC;

    u8 *from = (u8 *) ((u64)program + shdr->sh_offset);
    u8 *to = from + shdr->sh_size;
    if (shdr->sh_type == SHT_NOBITS) from = NULL;

    sv39_va addr = { .value = shdr->sh_addr };
    sv39_va end_addr = { .value = shdr->sh_addr + shdr->sh_size };
    u32 off = addr.off;
    addr.off = 0;
    while (addr.value < end_addr.value) {
      u8 *page = alloc_pages(0).address;
      if (page == NULL) {
        // TODO
        panic("out of memory");
      }
#ifndef DEBUG_ELF
      printk("set_page: %p %p %b\n", addr, page, flags);
#endif
      int err = set_page(table, addr.value, (u64) page, flags);
      if (err) {
        printk("error in set_page: %d\n", err);
        panic("error in load_elf");
      }

      for (int k = 0; k < PAGE_SIZE; ++k) {
        if (off > 0) {
          page[k] = NULL;
          --off;
          continue;
        }
        page[k] = from ? *from++ : 0;
        if (from >= to) from = NULL;
      }

      addr.value += PAGE_SIZE;
    }
  }

  return table;
}
