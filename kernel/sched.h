#pragma once

#include <stdbool.h>

#include "list.h"
#include "mm.h"
#include "trap.h"
#include "type.h"

enum task_mode {
  MODE_USER,
  MODE_SUPERVISOR,
};

struct task {
  pid_t pid;
  struct task *parent;
  struct list_node *sched;
  struct list vm_areas;
  void *kernel_stack;
  page_table_t page_table;
  enum task_mode mode;
  struct cpu kernel_frame;
  struct cpu user_frame;
};

void sched_init ();

extern struct task *current_task;

struct task *task_create (struct task *parent);
struct task *task_clone (struct task *parent);
void task_destroy (struct task *task);
int task_reinit (struct task *task);

__noreturn void schedule ();
__noreturn void schedule_next ();
__noreturn void idle ();

void wait (struct list *group);
void wakeup (struct list *group);
