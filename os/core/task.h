#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "defs.h"
#include "memory.h"
#include "svc.h"
#include "event.h"
#include "list.h"
#include "usec_timer.h"

#define TASK_SIGNATURE 0xdeadbeef

typedef struct
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr; // r14
	uint32_t pc; // r15
	uint32_t xpsr;
} hw_stack_frame_t;

typedef struct
{
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
} sw_stack_frame_t;

typedef struct
{
	uint32_t s[16]; // s0-s15
	uint32_t fpscr;
	uint32_t reserved;
} hw_fp_stack_frame_t;

typedef struct
{
	uint32_t s[16]; // s16 - s31
} sw_fp_stack_frame_t;

struct task
{
	list_t node;
	const char * name;
	uint8_t * stack_top;
	uint8_t * sp;
	int32_t id;
	uint32_t state;
	uint32_t flags;
	uint32_t sleep_until;
	uint64_t runtime;
	uint64_t lasttime;
	sw_stack_frame_t sw_context;
#ifdef ENABLE_FPU
	sw_fp_stack_frame_t sw_fp_context;
#endif
	event_t join;
	uint32_t exc_return;
	const uint32_t signature;
}__attribute__((aligned(8)));

static inline
task_t * task_alloc(int stack_size)
{
	// ensure that eventual sp is 8 byte aligned
	size_t size = sizeof(task_t) + stack_size + (stack_size % 8);
	task_t * t = malloc_aligned(size, 8);
	memset(t, 0, size);
	t->sp = (uint8_t*) t + size;
	t->stack_top = t->sp;
	return t;
}

static inline
task_t * task_init(task_t *t, const char * name, int id)
{
	*(uint32_t*) &t->signature = TASK_SIGNATURE;
	t->id = id;
	t->name = name;
	t->exc_return = 0xfffffffd;
	list_init(&t->node);
	event_init(&t->join);
	return t;
}

static inline task_t * task_create(int stack_size, const char * name)
{
	task_t * t = task_alloc(stack_size);
	task_init(t, name, kernel_task_next_id());
	return t;
}

task_t * task_frame_init(task_t *t, void (*func)(void*), void *context);
void task_schedule(task_t *task);
void task_sleep(uint32_t ms);
void task_yield(void);

__attribute__((always_inline)) static inline
void task_free(task_t * t)
{
	assert(t->id > 0 && "Cannot free idle or main task.");
	service_call((svcall_t) kernel_task_destroy_task, t, true);
	if (!(t->flags & TASK_FLAG_STATIC))
		free_aligned(t);
}

typedef struct
{
	void (*func)(void*);
	int stack_size;
	void *context;
	const char * name;
	task_t *task;
} task_init_t;

__attribute__((always_inline)) static inline
void __task_create_schedule(void *ctx)
{
	task_init_t *ti = ctx;
	ti->task = task_create(ti->stack_size, ti->name);
	task_frame_init(ti->task, ti->func, ti->context);
	kernel_task_start_task(ti->task);
}

__attribute__((always_inline))   static inline
task_t * task_create_schedule(void (*func)(void*), int stack_size, void *context, const char * name)
{
	task_init_t ti =
	{ .func = func, .stack_size = stack_size, .context = context, .name = name,
			.task = 0 };
	service_call(__task_create_schedule, &ti, true);
	return ti.task;
}

__attribute__((always_inline)) static inline
void task_join(task_t * t)
{
	event_wait(&t->join);
	task_free(t);
}

static inline task_t * list_to_task(list_t * list)
{
	return (task_t *) list;
}

static inline list_t * task_to_list(task_t * task)
{
	return (list_t *) task;
}

typedef struct
{
	sw_stack_frame_t sw_frame;
	hw_stack_frame_t hw_frame;
} stack_frame_t;

#define TASK_STATIC_ALLOCATE(name, size)                \
  struct {                                              \
    task_t task;                                        \
    uint8_t stack[size] __attribute((aligned(8)));      \
  } name                                          

#define TASK_STATIC_INIT(_name, _name_str, _id) {            \
    { .node = LIST_STATIC_INIT(_name.task.node),             \
		.signature = TASK_SIGNATURE, \
        .name = _name_str,                                   \
        .sp = &_name.stack[0] + sizeof(_name.stack),         \
        .stack_top = &_name.stack[0] + sizeof(_name.stack),  \
        .id = _id,                                           \
        .state = TASK_ACTIVE,                                \
	.flags = (TASK_FLAG_STATIC),                         \
	.sleep_until = 0,                                    \
	.lasttime = 0, \
	.runtime = 0, \
        .join = EVENT_STATIC_INIT(_name.task.join),          \
        .exc_return = 0xfffffffd }, {0}                      \
  }

#define TASK_STATIC_CREATE(name, name_str, size, id) \
  TASK_STATIC_ALLOCATE(name, size) = TASK_STATIC_INIT(name, name_str, id)

#endif /* __TASK_H__ */