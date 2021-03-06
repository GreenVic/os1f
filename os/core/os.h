/*
 * os.h
 *
 *  Created on: Apr 4, 2018
 *      Author: sonny
 */

#ifndef OS_CORE_OS_H_
#define OS_CORE_OS_H_


#include "os_config.h"
#include "defs.h"

/* core type header files */
#include "event_type.h"
#include "task_type.h"
#include "mutex_type.h"

/* core header files */
#include "event.h"
#include "kernel_task.h"
#include "kernel.h"
#include "memory.h"
#include "mutex.h"
#include "spinlock.h"
#include "svc.h"
#include "task.h"

/* device header files */
#include "lcd.h"
#include "usec_timer.h"
#include "vcp.h"

/* diag header files */
#include "assertions.h"
#include "Trace.h"

/* util header files */
#include "os_printf.h"
#include "display.h"
#include "heap.h"
#include "list.h"
#include "ring_buffer.h"

/* services header files */
#ifdef SHELL_ENABLE
#include "shell.h"
#endif /* SHELL_ENABLE */

#ifdef WATCHDOG_ENABLE
#include "watchdog.h"
#endif /* WATCHDOG_ENABLE */

/* HAL header files */
#include "stm32f7xx_hal.h"

#endif /* OS_CORE_OS_H_ */
