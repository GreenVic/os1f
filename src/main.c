#include <string.h>
#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "kernel.h"
#include "kernel_task.h"
#include "defs.h"
#include "task.h"
#include "semihosting.h"
#include "vsnprintf.h"

void printmsg(char *m);
static void task_func(void *);
static void task_once(void *);

struct func_data {
  char * name;
  uint32_t sleep;
};

static struct func_data fdata[4] = {
  {.name = "Task1", .sleep = 4000},
  {.name = "Task2", .sleep = 2000},
  {.name = "Task3", .sleep = 1000},
  {.name = "Task4", .sleep = 500},
};

extern void memory_thread_test(void);

int main(void)
{
  // switch modes and make main a normal user task
  os_start();

  printf("Clock is %d\n", HAL_RCC_GetHCLKFreq());
  extern char _Heap_Begin, _Heap_Limit,_estack;
  printf("Heap begin: 0x%x, Heap limit: 0x%x\n", &_Heap_Begin, &_Heap_Limit);
  
  task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[0]);
  struct task * tonce =
    task_create_schedule(task_once, DEFAULT_STACK_SIZE, NULL);

  task_join(tonce);
  
  // Unocmment to test memory allocation syncronization
  // memory_thread_test();
  
  uint32_t z = 0;
  while (1) {
    ++z;
    printf("Main Task [%d]\n", z);
    task_sleep(500);
  }
  
  return 0;
}

void task_func(void *context)
{
  int divisor = 1000000; // 10 million
  unsigned int k = 0;
  struct func_data * fdata = context;
  while (1) {
    ++k;
    printf("%s [%d]\n", fdata->name, k);
    task_sleep(fdata->sleep);

  };
}

void task_once(void *context)
{
  uint32_t tick = HAL_GetTick();
  int tid = current_task_id();
  
  printf("ONCE Task id [%d] at %d ms\n", tid, tick);
}

