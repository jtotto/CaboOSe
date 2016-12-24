#ifndef CABOOSE_PLATFORM_H
#define CABOOSE_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct task;
typedef void (*task_f)(void);

void platform_task_init(struct task *task, task_f code);

uint16_t platform_scheduling_timer_read(void);
void platform_scheduling_timer_reset(void);

void *memcpy(void *__restrict dst,
             const void *__restrict src,
             size_t len);
void *memset(void *s, int c, size_t len);

char *strcpy(char *destination, const char *source);
int strcmp(const char *s1, const char *s2);

/* Should provide as macros/inlines:
 * - ASSERT()
 * - platform_task_return_value() */
#include <caboose-platform/platform-inline.h>

#ifndef ASSERT
/* This is the best we can do without additional platform support. */
#define ASSERT(pred) do { if (!(pred)) while (true); } while (0)
#endif

#endif
