#include "sched.h"

#include <stdbool.h>

#include "errno.h"
#include "irq.h"
#include "list.h"
#include "memlayout.h"
#include "mm.h"
#include "mmdefs.h"
#include "panic.h"
#include "printf.h"
#include "riscv.h"
#include "syscall.h"
#include "trap.h"
#include "vm.h"

static pid_t next_task_id = 1;

static struct list tasks;
static struct list_node *current_task_node;
struct task *current_task;

static void *schedule_stack;

__noreturn static void do_schedule ();

void sched_init () {
  printk("Initializing scheduler.\n");
  list_init(&tasks);
  __asm__ volatile ("mv %0, sp" : "=r" (schedule_stack));

  current_task_node = NULL;
  current_task = NULL;
}

__noreturn void schedule () {
  __asm__ volatile ("mv sp, %0" : : "r" (schedule_stack));
  do_schedule();
}

static void yield_to_user (struct task *task) {
  trapframe.task = task->user_frame;
  if (ksetjmp(&task->kernel_frame) == 0) {
    return_to_user();
  }
}

static void task_entry (struct task *task) {
  while (true) {
    yield_to_user(task);
    struct cause scause = read_scause();
    if (scause.code == 8) {
      task->user_frame.pc += 4;
      task->user_frame.registers[REG_A0] = syscall(task);
    } else {
      if (scause.code == 13 || scause.code == 15) {
        u64 stval = csrr("stval");
        // stack autoexpand
        if (stval >= USERSTACKMIN && stval < USERSTACK) {
          struct vm_area *area = vm_find_stack(&task->vm_areas, (void __user *) stval);
          if (area != NULL) {
            let va_start = (void __user *) (stval & ~(PAGE_SIZE - 1));
            while (area->va > va_start) {
              void *page = alloc_pages(0).address;
              if (!page) {
                // TODO
                panic("stack autoexpand: out of memory");
              }
              clear_page(page);
              if (vm_area_add_page(area, page)) {
                panic("stack autoexpand: vm_area_add_page");
              }
              area->va = (u8 __user *) area->va - PAGE_SIZE;
              ++area->n_pages;
              u32 flags = pte_flags_from_vm_flags(area->flags);
              if (set_page_user(task->page_table, area->va, PA(page), flags)) {
                panic("stack autoexpand: set_page");
              }
            }
            continue;
          }
        }
      }
      printk("pid %d fault!\n", task->pid);
      dump_csr_s();
      break;
    }
  }
  task_destroy(task);
}

static int task_init_sched (struct task *task) {
  list_init(&task->vm_areas);
  task->mode = MODE_SUPERVISOR;
  task->kernel_frame.registers[REG_SP] = (u64) task->kernel_stack + PAGE_SIZE;
  task->kernel_frame.registers[REG_A0] = (u64) task;
  task->kernel_frame.pc = (u64) task_entry;

  struct list_node *node = list_node_create(task);
  if (!node) return -ENOMEM;
  task->sched = node;
  list_push(&tasks, node);

  return 0;
}

struct task *task_create (struct task *parent) {
  struct task *task = kmalloc(sizeof(struct task));
  struct task *retval = task;
  if (task == NULL) {
    return ERR_PTR(ENOMEM);
  }
#ifdef DEBUG_KERN_STACK
  printk("stack for %d is at %p\n", next_task_id, stack);
#endif

  task->pid = next_task_id++;
  task->parent = parent;

  void *stack = alloc_pages(0).address;
  if (stack == NULL) goto cleanup_task;
  task->kernel_stack = stack;

  task->page_table = create_page_table();
  if (task->page_table == NULL) goto cleanup_stack;

  int ret = task_init_sched(task);
  if (ret) {
    retval = ERR_PTR(-ret);
    goto cleanup_page_table;
  }

  return task;

  cleanup_page_table: destroy_page_table(task->page_table);
  cleanup_stack: free_pages((struct page) { .address = stack, .order = 0 });
  cleanup_task: kfree(task);
  return retval;
}

struct task *task_clone (struct task *parent) {
  struct task *task = task_create(parent);
  if (IS_ERR(task)) return task;

  int err = vm_clone(&parent->vm_areas, task);
  if (err) {
    task_destroy(task);
    return ERR_PTR(-err);
  }

  task->user_frame = parent->user_frame;
  return task;
}

static void task_destroy_sched (struct task *task) {
  vm_decref_all(&task->vm_areas);
  list_remove(task->sched);
  kfree(task->sched);
}

int task_reinit (struct task *task) {
  task_destroy_sched(task);
  unset_user_pages(task->page_table);
  return task_init_sched(task);
}

void task_destroy (struct task *task) {
  if (task->pid == 1) {
    panic("attempting to kill init");
  }

  task_destroy_sched(task);
  destroy_page_table(task->page_table);
  free_pages((struct page) { .address = task->kernel_stack, .order = 0 });
  kfree(task);

  if (task == current_task) {
    current_task = NULL;
    current_task_node = NULL;
    schedule();
  }
}

__noreturn static void do_schedule () {
  kassert(current_task_node == NULL, "schedule: current_task_node not null");
  kassert(current_task == NULL, "schedule: current_task not null");

  current_task_node = list_shift(&tasks);
  if (current_task_node == NULL) {
    idle();
  }
  current_task = current_task_node->value;

#ifdef DEBUG_SCHED
  printk("[debug sched] next task: %d\n", task->pid);
#endif

  let satp = satp_from_table(current_task->page_table);
  if (satp.value != csrr("satp")) {
    __asm__ volatile("sfence.vma x0, x0");
    csrw("satp", satp);
    __asm__ volatile("sfence.vma x0, x0");
  }

  if (current_task->mode == MODE_SUPERVISOR) {
    klongjmp(&current_task->kernel_frame);
  } else if (current_task->mode == MODE_USER) {
    trapframe.task = current_task->user_frame;
    return_to_user();
  } else {
    panic("invalid task mode %p", current_task->mode);
  }
}

__noreturn void schedule_next () {
  list_push(&tasks, current_task_node);
  current_task = NULL;
  current_task_node = NULL;
  schedule();
}

__noreturn void idle () {
  enable_interrupts();
  while (true) {
    __asm__("wfi");
  }
}

void wait (struct list *group) {
  kassert(group != NULL, "waiting on NULL group");
  kassert(current_task_node != NULL, "NULL task waiting");
  list_push(group, current_task_node);
  current_task->mode = MODE_SUPERVISOR;
  if (ksetjmp(&current_task->kernel_frame)) {
    return;
  }
  current_task = NULL;
  current_task_node = NULL;
  schedule();
}

void wakeup (struct list *group) {
  struct list_node *node;
  while ((node = list_shift(group))) {
    list_push(&tasks, node);
  }
}
