#include "elf.h"

#include "errno.h"
#include "memlayout.h"
#include "mm.h"
#include "mmdefs.h"
#include "printf.h"
#include "riscv.h"
#include "vm.h"

int load_elf (struct task *task, elf program) {
  if (program->e_machine != 0xf3) {
    return -EINVAL;
  }

  page_table_t table = task->page_table;
  bool execstack = false;

  // TODO: sanity check
  for (int i = 0; i < program->e_phnum; ++i) {
    Elf64_Phdr *phdr = (void *) ((u64) program + program->e_phoff +
                                 i * program->e_phentsize);
    switch (phdr->p_type) {
      case PT_LOAD:
        break;

      case PT_DYNAMIC:
      case PT_INTERP:
        return -EINVAL;

      case PT_GNU_STACK:
        execstack = !!(phdr->p_flags & PF_X);
        continue;

      case PT_NULL:
      case PT_NOTE:
      default:
        continue;
    }
    u32 flags = PTE_VALID | PTE_USER;
    u32 area_flags = 0;
    if (phdr->p_flags & PF_R) flags |= PTE_READ, area_flags |= VM_READ;
    if (phdr->p_flags & PF_W) flags |= PTE_WRITE, area_flags |= VM_WRITE;
    if (phdr->p_flags & PF_X) flags |= PTE_EXEC, area_flags |= VM_EXEC;

    u8 *from = (u8 *) ((u64)program + phdr->p_offset);
    u8 *to = from + phdr->p_filesz;

    sv39_va addr = { .value = phdr->p_vaddr };
    sv39_va end_addr = { .value = phdr->p_vaddr + phdr->p_memsz };
    u32 off = addr.off;
    addr.off = 0;
    if (end_addr.off != 0) {
      end_addr.off = 0;
      ++end_addr.vpn;
    }

    struct vm_area *area =
      vm_area_create(addr.value, end_addr.vpn - addr.vpn, area_flags);
    if (vm_add_area(&task->vm_areas, area)) {
      // TODO
      panic("load_elf: out of memory");
    }

    while (addr.value < end_addr.value) {
      u8 *page = alloc_pages(0).address;
      if (page == NULL) {
        panic("load_elf: out of memory");
      }
      if (vm_area_add_page(area, page)) {
        panic("load_elf: out of memory");
      }
#ifdef DEBUG_ELF
      printk("set_page: %p %p %b\n", addr, PA((u64) page), flags);
#endif
      int err = set_page(table, addr.value, PA((u64) page), flags);
      if (err) {
        panic("load_elf: set_page: %d", err);
      }

      for (int k = 0; k < PAGE_SIZE; ++k) {
        if (off > 0) {
          page[k] = 0;
          --off;
          continue;
        }
        page[k] = from ? *from++ : 0;
        if (from >= to) from = NULL;
      }

      addr.value += PAGE_SIZE;
    }
  }

  u32 stack_vm_flags = VM_READ | VM_WRITE | VM_STACK;
  u32 stack_pt_flags = PTE_VALID | PTE_USER | PTE_READ | PTE_WRITE;
  if (execstack) {
    stack_vm_flags |= VM_EXEC;
    stack_pt_flags |= PTE_EXEC;
  }
  void *stack_pa = alloc_pages(0).address;
  if (stack_pa == NULL) {
    panic("load_elf: out of memory");
  }

  u64 stack_begin = USERSTACK - PAGE_SIZE;
  struct vm_area *area = vm_area_create(stack_begin, 1, stack_vm_flags);
  if (vm_add_area(&task->vm_areas, area) || vm_area_add_page(area, stack_pa)) {
    panic("load_elf: out of memory");
  }
  int err = set_page(table, stack_begin, PA((u64) stack_pa), stack_pt_flags);
  if (err) {
    panic("load_elf: set_page: %d", err);
  }
  for (int i = 0; i < 32; ++i) {
    task->user_frame.registers[i] = 0;
  }
  task->user_frame.registers[REG_SP] = USERSTACK;

  return 0;
}
