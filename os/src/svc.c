#include <stdint.h>
#include "svc.h"
#include "kernel.h"
#include "kernel_task.h"
#include "defs.h"

static uint32_t pendsv_stack[64];
static uint32_t *pendsv_stack_ptr = &pendsv_stack[0];

void service_call(svcall_t call, void *cxt)
{
  __asm volatile("svc 0\n");
}

void pend_service_call(svcall_t call, void *cxt)
{
  /* kernel_critical_begin(); */
  /* *pendsv_stack_ptr++ = (uint32_t)call; */
  /* *pendsv_stack_ptr++ = (uint32_t)cxt; */
  /* kernel_critical_end(); */

  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}


void SVC_Handler(void)
{
  register uint32_t * frame;
  __asm volatile("mrs %0, psp \n" : "=r" (frame) ::);

  svcall_t call = (svcall_t)frame[0];
  void *args = (void*)frame[1];
  
  call(args);
}

__attribute__ ((naked))
void PendSV_Handler(void)
{
  uint32_t exc_return;
  __asm volatile("mov %0, lr \n" : "=r"(exc_return));
  
  /* while (pendsv_stack_ptr != &pendsv_stack[0]) { */
  /*   kernel_critical_begin(); */
  /*   void * cxt = (void*)*(--pendsv_stack_ptr); */
  /*   svcall_t call = (svcall_t)*(--pendsv_stack_ptr); */
  /*   kernel_critical_end(); */
  /*   //if (cxt == NULL) cxt = (void*)exc_return; // pass exc_return sometimes */
  /*   call(cxt); */
  /* } */

  kernel_critical_begin();
  kernel_task_save_context(exc_return);

  kernel_task_update_local_SP();
  kernel_task_schedule();
  kernel_task_wakeup();
  kernel_task_active_next();
  kernel_task_update_global_SP();

  exc_return = kernel_task_load_context();
  kernel_critical_end();
  __asm volatile("bx %0 \n" :: "r"(exc_return)); 
}
