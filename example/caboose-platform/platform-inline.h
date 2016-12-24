#ifndef CABOOSE_PLATFORM_INLINE_H
#define CABOOSE_PLATFORM_INLINE_H

#include <stdint.h>

struct task;

/* use the default ASSERT() for now */

static inline void platform_task_return_value(struct task *task, uint32_t rv)
{
    /* XXX */
}

#endif
