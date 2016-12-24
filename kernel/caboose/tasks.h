#ifndef CABOOSE_TASKS_H
#define CABOOSE_TASKS_H

#include "config.h"

#define HIGH_PRIORITY 0
#define LOW_PRIORITY (CONFIG_PRIORITY_COUNT - 1)

#ifndef ASM
#include <stdint.h>

#include "config.h"
#include "list.h"
#include "platform.h"
#include "state.h"
#include "types.h"
#include "util.h"

uint8_t *tasks_init(uint8_t *pool);

uint8_t *schedule(int32_t highest_priority);

tid_t sys_Create(int priority, task_f code);
tid_t sys_MyTid(void);
tid_t sys_MyParentTid(void);
void sys_Pass(void);
void sys_Exit(void);

int sys_Send(tid_t tid, void *msg, int msglen, void *reply, int replylen);
int sys_Receive(tid_t *tid, void *msg, int msglen);
int sys_Reply(tid_t tid, void *reply, int replylen);

int sys_AsyncSend(tid_t tid, void *msg, int msglen);
int sys_AsyncReceive(tid_t *tid, void *msg, int msglen);

static inline void readyq_enqueue(struct tasks *tasks, struct task *task)
{
    tasks->nonempty_queues |= (1 << task->priority);
    list_insert_tail(&tasks->readyq[task->priority], &task->rqn);
}

static inline struct task *readyq_dequeue(struct tasks *tasks,
                                          int32_t priority)
{
    ASSERT(!list_empty(&tasks->readyq[priority]));
    struct node *next = list_remove_head(&tasks->readyq[priority]);
    struct task *task = containerof(next, struct task, rqn);
    if (list_empty(&tasks->readyq[priority])) {
        tasks->nonempty_queues &= ~((uint32_t)(1 << task->priority));
    }
    return task;
}

#endif

#endif
