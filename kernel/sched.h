#pragma once

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
  struct task_list *sched;
  page_table_t page_table;
  enum task_mode mode;
  struct cpu kernel_frame;
  struct cpu user_frame;
};

void sched_init ();

extern struct task *current_task;

struct task *create_task (struct task *parent, page_table_t page_table);
void destroy_task (struct task *task);

__attribute__((noreturn)) void schedule ();
__attribute__((noreturn)) void schedule_next ();
__attribute__((noreturn)) void idle ();
