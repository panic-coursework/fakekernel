#include "sched.h"

#include <stdbool.h>

#include "errno.h"
#include "irq.h"
#include "list.h"
#include "memlayout.h"
#include "mm.h"
#include "panic.h"
#include "printf.h"
#include "riscv.h"
#include "syscall.h"
#include "trap.h"

static pid_t next_task_id = 1;

static struct list tasks;
static struct list_node *current_task_node;
struct task *current_task;

static void *schedule_stack;

__attribute__((noreturn)) static void do_schedule ();

void sched_init () {
  printk("Initializing scheduler.\n");
  list_init(&tasks);
  __asm__ volatile ("mv %0, sp" : "=r" (schedule_stack));

  current_task_node = NULL;
  current_task = NULL;
}

__attribute__((noreturn)) void schedule () {
  __asm__ volatile ("mv sp, %0" : : "r" (schedule_stack));
  do_schedule();
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
  struct list_node *node = list_node_create(task);
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

  list_push(&tasks, node);

  return task;
}

void destroy_task (struct task *task) {
  if (task->pid == 1) {
    panic("attempting to kill init");
  }

  struct list_node *entry = task->sched;
  list_remove(&tasks, entry);
  kfree(entry);
  kfree(task);
}

__attribute__((noreturn)) static void do_schedule () {
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

  if (current_task->mode == MODE_SUPERVISOR) {
    klongjmp(&current_task->kernel_frame);
  } else if (current_task->mode == MODE_USER) {
    trapframe.task = current_task->user_frame;
    return_to_user(current_task->page_table);
  } else {
    panic("schedule: invalid task mode");
  }
}

__attribute__((noreturn)) void schedule_next () {
  list_push(&tasks, current_task_node);
  current_task = NULL;
  current_task_node = NULL;
  schedule();
}

__attribute__((noreturn)) void idle () {
  enable_interrupts();
  while (true) {
    __asm__("wfi");
  }
}

void wait (struct list *group) {
  list_push(group, current_task_node);
  if (ksetjmp(&current_task->kernel_frame)) return;
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
