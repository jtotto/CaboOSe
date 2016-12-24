#ifndef CABOOSE_TYPES_H
#define CABOOSE_TYPES_H

#include <stdint.h>

typedef int32_t tid_t;
typedef void (*task_f)(void);

enum task_state {
    TASK_STATE_READY,
    TASK_STATE_ACTIVE,
    TASK_STATE_ZOMBIE,
    TASK_STATE_SEND_BLOCKED,
    TASK_STATE_RECV_BLOCKED,
    TASK_STATE_REPLY_BLOCKED,
    TASK_STATE_ASYNC_SEND_BLOCKED,
    TASK_STATE_EVENT_BLOCKED
};

#endif
