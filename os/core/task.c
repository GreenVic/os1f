#include <string.h>
#include <stdlib.h>
#include "task.h"
#include "defs.h"
#include "kernel_task.h"
#include "memory.h"

// implemented in kernel_task
extern void kernel_task_end(void);

task_t * task_frame_init(task_t *t, void (*func)(void*), void *context)
{
  hw_stack_frame_t *frame = (hw_stack_frame_t*)(t->sp - sizeof(hw_stack_frame_t));
  frame->r0 = (uint32_t)context;
  frame->pc = (uint32_t)func & 0xfffffffe;
  frame->lr = (uint32_t)&kernel_task_end;
  frame->xpsr = 0x01000000;   // thumb mode enabled (required);

  t->sp = (uint8_t*)frame;
  return t;
}

static void protected_task_start(void * cxt)
{
  task_t * new = cxt;
  __disable_irq();
  kernel_task_start_task(new);
  __enable_irq();
}

static void protected_task_sleep(void *cxt)
{
  uint32_t ms = (uint32_t)cxt;
  __disable_irq();
  kernel_task_sleep_current(ms);
  __enable_irq();
  protected_kernel_context_switch(NULL);
}

inline
void task_schedule(task_t *task)
{
  service_call(protected_task_start, task, false);
}

inline
void task_sleep(uint32_t ms) 
{
  service_call(protected_task_sleep, (void*)ms, false);
}

inline
void task_yield(void) 
{
  kernel_context_switch();
}
