#include <stdint.h>
#include "os.h"

static volatile bool __os_started = false;

static void os_services_start(void);

void os_start(void) {
	HAL_Init();
	display_init();
	usec_timer_init();

	HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

#ifdef ENABLE_FPU
	kernel_FPU_enable();
#endif /* ENABLE_FP */


	kernel_task_init();
	//NOTE: after here we are in user mode

	__os_started = true;

	os_services_start();
}

inline
bool os_started(void)
{
	return __os_started;
}

void assert_os_started(void)
{
	assert(__os_started && "OS has not started quite yet.");
}

static void os_services_start(void)
{

#ifdef WATCHDOG_ENABLE
	watchdog_init();
#endif /* WATCHDOG_ENABLE */

#ifdef SHELL_ENABLE
	shell_init();
#endif /* SHELL_ENABLE */

}

void _exit( __attribute__((unused)) int code)
{
#if defined(DEBUG)
	__asm volatile ("bkpt 0");
#endif
	while(1) ;
}
