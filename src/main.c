#include <string.h>
//#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "kernel.h"
#include "kernel_task.h"
#include "defs.h"
#include "task.h"
#include "display.h"


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

extern void adc_task(void*);
extern void memory_thread_test(void);
extern uint32_t heap_size_get(void);
extern char _Heap_Begin, _Heap_Limit,_estack;

int main(void)
{
  // switch modes and make main a normal user task
  os_start();

  int clk = HAL_RCC_GetHCLKFreq();
  char *clk_str = "Hz";
  if (clk > 1000000) {
    clk /= 1000000;
    clk_str = "MHz";
  }
  else if (clk > 10000) {
    clk /= 1000;
    clk_str = "KHz";
  }
  
  display_line_at(12, "Clock is %d%s\n", clk, clk_str);
  int heap_size = (int)(&_Heap_Limit - &_Heap_Begin);
  //  display_line_at(11, "Total Heap Space %d\n", heap_size);
    
  task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[0]);
  task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[1]);
  task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[2]);
  task_create_schedule(task_func, DEFAULT_STACK_SIZE, (void*)&fdata[3]);
  
  // Unocmment to test memory allocation syncronization
  // memory_thread_test();
  task_create_schedule(adc_task, 1024, NULL);
  
  uint32_t z = 0;
  int tid = current_task_id();
  while (1) {
    ++z;
    int heap_current = heap_size_get();
    int per = heap_current * 10000 / heap_size;
    int per_entier = per / 100;
    int per_mant = per % 100;
    display_line_at(11, "Heap Use %d of %d, %d.%d%%\n", heap_size_get(), heap_size, per_entier, per_mant);
    task_display_line("Main Task\tid : %d, counter : %d\n", tid, z);

    task_t * tonce =
      task_create_schedule(task_once, DEFAULT_STACK_SIZE, NULL);

    task_join(tonce);

    task_sleep(500);
  }
  
  return 0;
}

void task_func(void *context)
{
  int k = 0;
  int tid = current_task_id();
  struct func_data * fdata = context;
  while (1) {
    ++k;
    task_display_line("Simple Task\tid : %d, counter : %d\n", tid, k);
    task_sleep(fdata->sleep);

  };
}

void task_once(void *context)
{
  uint32_t tick = HAL_GetTick();
  int tid = current_task_id();
  
  display_line_at(9, "ONCE Task\tid : %d at %d ms\n", tid, tick);
}

