#ifndef CABOOSE_STATE_H
#define CABOOSE_STATE_H

#include "config.h"
#include "list.h"
#include "ringbuffer.h"
#include "types.h"

struct mail {
    tid_t sendertid;
    void *msg;
    int msglen;
    void *reply;
    int replylen;
    struct node next;
};

struct mailbox {
    tid_t *sender_tid;
    void *msg;
    int msglen;
};

struct task {
    tid_t tid;
    tid_t parent;
    int32_t priority;
    enum task_state state;

    struct list mail;
    struct list async;

    uint8_t *sp;
    struct node rqn;

    uint32_t running_time;
};

struct tasks {
    tid_t next_tid;
    struct task *active;
    uint32_t nonempty_queues;
    uint32_t scheduler_uptime;

    struct list readyq[CONFIG_PRIORITY_COUNT];
    struct list async_free;

    struct task table[CONFIG_TASK_COUNT];
};

struct event {
    struct task *waiter;
    struct ringbuffer ring;
};

struct caboose {
    struct tasks tasks;
    struct event events[CONFIG_EVENT_COUNT];
};

extern struct caboose caboose;

#endif
