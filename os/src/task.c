#include <string.h>
#include <stdlib.h>
//#include "memory.h"
#include "task.h"
#include "defs.h"

static void task_end(void)
{
  while (1);
}

struct task * task_init(struct task *t, void (*func)(void*), void *context)
{
  struct regs *r = t->stackp - sizeof(struct regs);
  r->stacked.r0 = (uint32_t)context;
  r->stacked.pc = (uint32_t)func & 0xfffffffe;
  r->stacked.lr = (uint32_t)&task_end;
  r->stacked.xpsr = 0x01000000;   // thumb mode enabled (required);

  t->stackp = r;
  return t;
}

struct task * task_create(int stack_size)
{
  struct task *t = malloc(sizeof(struct task));
  memset(t, 0, sizeof(struct task));
  void * s = malloc(stack_size);
  memset(s, 0, stack_size);
  t->stackp = s + stack_size;
  
  return t;
}



