#include "elf.h"

#include "errno.h"
#include "memlayout.h"
#include "mm.h"
#include "mmdefs.h"
#include "panic.h"
#include "printf.h"
#include "riscv.h"
#include "string.h"
#include "vm.h"

int load_elf (struct task *task, elf program) {
  if (program->e_machine != 0xf3) {
    return -EINVAL;
  }

  page_table_t table = task->page_table;
  bool execstack = false;

  // TODO: sanity check
  for (int i = 0; i < program->e_phnum; ++i) {
    let phdr = (Elf64_Phdr *) ((u64) program + program->e_phoff +
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

    let user_addr = (void __user *) addr.ptr;
    let area = vm_area_create(user_addr, end_addr.vpn - addr.vpn, area_flags);
    if (vm_add_area(&task->vm_areas, area)) {
      // TODO
      panic("out of memory");
    }

    while (addr.value < end_addr.value) {
      u8 *page = alloc_pages(0).address;
      if (page == NULL) {
        panic("out of memory");
      }
      if (vm_area_add_page(area, page)) {
        panic("out of memory");
      }
#ifdef DEBUG_ELF
      printk("set_page: %p %p %b\n", addr, PA(page), flags);
#endif
      int err = set_page(table, addr.ptr, PA(page), flags);
      if (err) {
        panic("set_page: %d", err);
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
  void *stack_page = alloc_pages(0).address;
  if (stack_page == NULL) {
    panic("out of memory");
  }
  clear_page(stack_page);

  let stack_begin = (void __user *) (USERSTACK - PAGE_SIZE);
  struct vm_area *area = vm_area_create(stack_begin, 1, stack_vm_flags);
  if (vm_add_area(&task->vm_areas, area) || vm_area_add_page(area, stack_page)) {
    panic("out of memory");
  }
  int err = set_page_user(table, stack_begin, PA(stack_page), stack_pt_flags);
  if (err) {
    panic("set_page: %d", err);
  }
  for (int i = 0; i < 32; ++i) {
    task->user_frame.registers[i] = 0;
  }
  task->user_frame.registers[REG_SP] = USERSTACK;
  task->user_frame.pc = program->e_entry;

  return 0;
}

static void __user *user_stack_addr (u64 off) {
  return (void __user *) (USERSTACK - off);
}

int task_init_elf (struct task *task, elf program, u8 **argv, u8 **envp) {
  kassert(argv && envp, "task_init_elf with NULL argv or envp");
  kassert(program, "task_init_elf with NULL program");

  int retval = load_elf(task, program);
  if (retval) return retval;

  u64 arg_size = 0;
  u64 argc, envc;
  for (argc = 0; argv[argc]; ++argc) {
    arg_size += strlen(argv[argc]) + 1;
  }
  for (envc = 0; envp[envc]; ++envc) {
    arg_size += strlen(envp[envc]) + 1;
  }
  arg_size += (argc + envc + 2) * sizeof(void *);
  if (arg_size > PAGE_SIZE) {
    panic("argv too big");
    return -E2BIG;
  }

  // FIXME: how to get the address of the stack page?
  struct vm_area *area = task->vm_areas.last->value;
  kassert(area->flags & VM_STACK, "fixme");
  kassert(area->n_pages == 1, "fixme");
  kassert(area->pages.first == area->pages.last && area->pages.first, "fixme");
  u8 *stack = (u8 *) area->pages.first->value + PAGE_SIZE;

  u64 off = arg_size;
  u64 argv_off = (argc + envc + 2) * sizeof(void *);
  u64 envp_off = (envc + 2) * sizeof(void *);
  for (u64 i = 0; i < argc; ++i) {
    *(u8 * __user **) (stack - argv_off + i * sizeof(void *)) =
      user_stack_addr(off);
    off -= strlcpy(stack - off, argv[i], off) + 1;
  }
  for (u64 i = 0; i < envc; ++i) {
    *(u8 * __user **) (stack - envp_off + i * sizeof(void *)) =
      user_stack_addr(off);
    off -= strlcpy(stack - off, envp[i], off) + 1;
  }
  kassert(off == argv_off, "stack size mismatch");

  task->user_frame.registers[REG_A0] = argc;
  task->user_frame.registers[REG_A1] = (u64) user_stack_addr(argv_off);
  task->user_frame.registers[REG_A2] = (u64) user_stack_addr(envp_off);
  u64 aligned_off = (arg_size + 15) / 16 * 16;
  task->user_frame.registers[REG_SP] = USERSTACK - aligned_off;

  return 0;
}
