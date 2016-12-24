#ifndef CABOOSE_EVENTS_H
#define CABOOSE_EVENTS_H

#include "platform.h"
#include "ringbuffer.h"
#include "state.h"
#include "syscall.h"
#include "tasks.h"

uint8_t *events_init(uint8_t *pool);

static inline void event_deliver(int event_id, int data)
{
    struct event *event = &caboose.events[event_id];
    struct tasks *tasks = &caboose.tasks;
    struct task *waiter = event->waiter;

    if (waiter) {
        /* Deliver the return value to the waiting task and wake them up. */
        waiter->state = TASK_STATE_READY;
        readyq_enqueue(tasks, waiter);
        platform_task_return_value(waiter, data);

        /* Clear the waiter so that subsequent event delivery is queued. */
        event->waiter = NULL;
    } else {
        ringbuffer_append_unit(&event->ring, (uint8_t *)&data);
    }
}

#endif
