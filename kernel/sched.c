#include "sched.h"

#include <stdbool.h>

#include "errno.h"
#include "irq.h"
#include "memlayout.h"
#include "mm.h"
#include "panic.h"
#include "printf.h"
#include "riscv.h"
#include "syscall.h"
#include "trap.h"

static pid_t next_task_id = 1;

struct task_list {
  struct task *task;
  struct task_list *prev;
  struct task_list *next;
};
static struct task_list *next_task;
static struct task_list *last_task;
struct task *current_task;

void sched_init () {
  printk("Initializing scheduler.\n");
  next_task = last_task = NULL;
  current_task = NULL;
}

static void yield_to_user (struct task *task) {
  trapframe.task = task->user_frame;
  if (ksetjmp(&task->kernel_frame) == 0) {
    return_to_user(task->page_table);
  }
}

static void task_entry (struct task *task) {
  for (int i = 0; i < 32; ++i) {
    task->user_frame.registers[i] = 0;
  }
  while (true) {
    yield_to_user(task);
    struct cause scause = read_scause();
    if (scause.code == 8) {
      task->user_frame.pc += 4;
      task->user_frame.registers[REG_A0] = syscall(task);
    } else {
      printk("pid %d fault!\n", task->pid);
      dump_csr_s();
      break;
    }
  }
  destroy_task(task);
}

struct task *create_task (struct task *parent, page_table_t page_table) {
  struct task *task = kmalloc(sizeof(struct task));
  if (task == NULL) return NULL;
  struct task_list *node = kmalloc(sizeof(struct task_list));
  if (node == NULL) {
    kfree(task);
    return NULL;
  }
  void *stack = alloc_pages(0).address;
  if (stack == NULL) {
    kfree(task);
    kfree(node);
    return NULL;
  }

  task->sched = node;
  task->pid = next_task_id++;
  task->parent = parent;
  task->page_table = page_table;
  task->mode = MODE_SUPERVISOR;
  task->kernel_frame.registers[REG_SP] = (u64) stack + PAGE_SIZE;
  task->kernel_frame.registers[REG_A0] = (u64) task;
  task->kernel_frame.pc = (u64) task_entry;

  node->next = NULL;
  node->prev = last_task;
  node->task = task;
  if (last_task != NULL) last_task->next = node;
  last_task = node;
  if (next_task == NULL) next_task = node;

  return task;
}

void destroy_task (struct task *task) {
  if (task->pid == 1) {
    panic("attempting to kill init");
  }

  struct task_list *entry = task->sched;
  if (entry->next != NULL) entry->next->prev = entry->prev;
  if (entry->prev != NULL) entry->prev->next = entry->next;
  if (entry == next_task) next_task = entry->next;
  if (entry == last_task) last_task = entry->prev;
  kfree(entry);
  kfree(task);
}

__attribute__((noreturn)) void schedule () {
  if (next_task == NULL) {
    current_task = NULL;
    idle();
  }
  struct task *task = next_task->task;

#ifdef DEBUG_SCHED
  printk("[debug sched] next task: %d\n", task->pid);
#endif
  current_task = task;

  if (task->mode == MODE_SUPERVISOR) {
    klongjmp(&task->kernel_frame);
  } else if (task->mode == MODE_USER) {
    trapframe.task = task->user_frame;
    return_to_user(task->page_table);
  } else {
    panic("schedule: invalid task mode");
  }
}

__attribute__((noreturn)) void schedule_next () {
  if (next_task == NULL) {
    panic("schedule_next: next_task is NULL");
  }
  next_task->prev = last_task;
  if (last_task != NULL) last_task->next = next_task;
  last_task = next_task;
  next_task = next_task->next;
  last_task->next = NULL;
  if (next_task) next_task->prev = NULL;

  schedule();
}

__attribute__((noreturn)) void idle () {
  enable_interrupts();
  while (true) {
    __asm__("wfi");
  }
}
