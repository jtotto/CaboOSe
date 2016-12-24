#include "errcode.h"
#include "platform.h"
#include "state.h"
#include "syscall.h"
#include "tasks.h"
#include "util.h"

struct asyncmsg {
    struct node node;
    tid_t sender;
    int msglen;
    uint8_t data[];
};

#define ASYNC_SIZE_MAX \
    (CONFIG_ASYNC_MSG_BUFSIZE - offsetof(struct asyncmsg, data))

void init(void);

uint8_t *tasks_init(uint8_t *pool)
{
    struct tasks *tasks = &caboose.tasks;
    
    /* Align the pool pointer to an 8-byte boundary to satisfy the ARM stack ABI
     * requirement and allocate a stack for all possible tasks. */
    uintptr_t ptr = (uintptr_t)pool;
    ptr = ALIGN(ptr, 8);
    pool = (uint8_t *)ptr;

    pool += CONFIG_TASK_STACK_SIZE;
    for (int i = 0;
         i < CONFIG_TASK_COUNT;
         i++, pool += CONFIG_TASK_STACK_SIZE) {
        tasks->table[i] = (struct task) {
            .tid = i,
            .state = TASK_STATE_ZOMBIE,
            .sp = pool,
            .running_time = 0
        };

        list_init(&tasks->table[i].mail);
        list_init(&tasks->table[i].async);
    }

    /* Initialize the ready-queues. */
    for (int i = 0; i < CONFIG_PRIORITY_COUNT; i++) {
        list_init(&tasks->readyq[i]);
    }

    /* Initialize the async message free list. */
    list_init(&tasks->async_free);
    for (int i = 0; i < CONFIG_ASYNC_MSG_COUNT; i++) {
        struct asyncmsg *next = (struct asyncmsg *)pool;
        list_insert_tail(&tasks->async_free, &next->node);
        pool += CONFIG_ASYNC_MSG_BUFSIZE;
    }

    /* Initialize task 0, the init task. */
    struct task *tinit = &tasks->table[0];
    tinit->parent = 0;   /* The init process is its own parent. */
    tinit->priority = CONFIG_INIT_PRIORITY; 
    tinit->state = TASK_STATE_ACTIVE;

    platform_task_init(tinit, init);

    /* The init task is the active task at startup. */
    tasks->active = tinit;
    /* All of the queues are initially empty. */
    tasks->nonempty_queues = 0;
    /* No tasks have spent any time running yet. */
    tasks->scheduler_uptime = 0;

    /* After the init task, TIDs start from 1. */
    tasks->next_tid = 1;

    return pool;
}

uint8_t *schedule(void)
{
    struct tasks *tasks = &caboose.tasks;
    struct task *active = caboose.tasks.active;

    unsigned int highest_priority = __builtin_ctz(tasks->nonempty_queues);

    /* If there are no queued ready tasks, the currently active task had better
     * still be ready to run. */
    ASSERT(active->state == TASK_STATE_ACTIVE || tasks->nonempty_queues);

    uint16_t running_time = platform_scheduling_timer_read();
    active->running_time += running_time;
    tasks->scheduler_uptime += running_time;

    if (active->state == TASK_STATE_ACTIVE) {
        if (!tasks->nonempty_queues || highest_priority > active->priority) {
            /* No further scheduling needed - the active task is can continue
             * running and no tasks of greater or equal priority exist. */
            return active->sp;
        }

        /* We know now that highest is of either greater or equal priority than
         * active, and that active is still ready to run, so we need to adjust
         * its state and add it to the ready queue of its priority for future
         * re-scheduling. */
        active->state = TASK_STATE_READY;
        readyq_enqueue(tasks, active);
    }

    tasks->active = active = readyq_dequeue(tasks, highest_priority);
    active->state = TASK_STATE_ACTIVE;

    platform_scheduling_timer_reset();

    return active->sp;
}

tid_t sys_Create(int priority, task_f code)
{
    struct tasks *tasks = &caboose.tasks;
    tid_t next = tasks->next_tid;
    if (next >= CONFIG_TASK_COUNT) {
        return ENOTIDS;
    }

    if (priority > LOW_PRIORITY || priority < HIGH_PRIORITY) {
        return EBADPRIORITY;
    }

    tasks->next_tid += 1;

    struct task *next_task = &tasks->table[next];
    next_task->tid = next;
    next_task->priority = priority;
    next_task->parent = tasks->active->tid;
    
    platform_task_init(next_task, code);

    next_task->state = TASK_STATE_READY;
    readyq_enqueue(tasks, next_task);
    return next;
}

tid_t sys_MyTid(void)
{
    return caboose.tasks.active->tid;
}

tid_t sys_MyParentTid(void)
{
    return caboose.tasks.active->parent;
}

void sys_Pass(void)
{
    /* just let the scheduler run */
}

void sys_Exit(void)
{
    struct tasks *tasks = &caboose.tasks;
    struct task *active = tasks->active;

    /* The currently active task has just exited, ie. is now a zombie according
     * to the definition in the kernel manual. */
    active->state = TASK_STATE_ZOMBIE;
}

int sys_Send(tid_t tid, void *msg, int msglen, void *reply, int replylen)
{
    struct tasks *tasks = &caboose.tasks;
    struct task *active = tasks->active;

    if (tid < 0 || tid >= CONFIG_TASK_COUNT) {
        return EIMPOSSIBLETID;
    }

    struct task *recipient = &tasks->table[tid];
    if (recipient->state == TASK_STATE_ZOMBIE) {
        return ETASKNOTEXIST;
    }

    active->state = TASK_STATE_RECV_BLOCKED;

    active->sp -= sizeof(struct mail);
    struct mail *letter = (struct mail *)active->sp;
    *letter = (struct mail) {
        .sendertid = active->tid,
        .msg = msg,
        .msglen = msglen,
        .reply = reply,
        .replylen = replylen
    };

    if (recipient->state == TASK_STATE_SEND_BLOCKED) {
        /* Copy the message and then schedule receiver. */
        struct mailbox *recipient_mailbox = (struct mailbox *)recipient->sp;
        recipient->sp += sizeof(struct mailbox);

        *(recipient_mailbox->sender_tid) = active->tid;

        int len = MIN(msglen, recipient_mailbox->msglen);
        memcpy(recipient_mailbox->msg, msg, len);

        /* Update reciever trapframe with return. */
        platform_task_return_value(recipient, msglen);

        /* Block self on reply. */
        active->state = TASK_STATE_REPLY_BLOCKED;

        /* Wake up and requeue partner. */
        recipient->state = TASK_STATE_READY;
        readyq_enqueue(tasks, recipient);
    } else {
        /* Partner not ready to recieve => go to sleep and put self on queue. */
        list_insert_tail(&recipient->mail, &letter->next);
    }

    /* Return value always set by reply. */
    return EINCOMPLETESEND;
}

int sys_Receive(tid_t *tid, void *msg, int msglen)
{
    struct tasks *tasks = &caboose.tasks;
    struct task *active = tasks->active;
    active->state = TASK_STATE_SEND_BLOCKED;

    /* We have a queued sender */
    while (!list_empty(&active->mail)) {
        struct node *sendnode = list_remove_head(&active->mail);
        struct mail *letter = containerof(sendnode, struct mail, next);
        
        /* Get the known waiting sender. */
        *tid = letter->sendertid;

        struct task *sender = &tasks->table[*tid];
        /* Handle contrived scenario where replied to while recv blocked. */
        if (sender->state != TASK_STATE_RECV_BLOCKED) {
            ASSERT(sender->state == TASK_STATE_READY);
            readyq_enqueue(tasks, sender);
            continue;    
        }
        
        /* Do the copy. */
        int len = MIN(msglen, letter->msglen);
        memcpy(msg, letter->msg, len);

        /* Before returning, reset to active to be rescheduled to resume. */
        sender->state = TASK_STATE_REPLY_BLOCKED;
        active->state = TASK_STATE_ACTIVE;
        return letter->msglen;
    }
    
    /* There is no partner, save my stuff to my stack for partner to find. */
    active->sp -= sizeof(struct mailbox);
    struct mailbox *box = (struct mailbox *)active->sp;
    *box = (struct mailbox) {
        .sender_tid = tid,
        .msg = msg,
        .msglen = msglen
    };

    /* Since send blocked state, doesn't get rescheduled. */
    return 0; /* This is replaced with value by send and awoken. */
}

int sys_Reply(tid_t tid, void *reply, int replylen)
{
    struct tasks *tasks = &caboose.tasks;
    struct task *sender = &tasks->table[tid];

    if (tid < 0 || tid >= CONFIG_TASK_COUNT) {
        return EIMPOSSIBLETID;
    }
    if (sender->state == TASK_STATE_ZOMBIE) {
        return ETASKNOTEXIST;
    }
    if (sender->state != TASK_STATE_REPLY_BLOCKED) {
        if (sender->state == TASK_STATE_RECV_BLOCKED) {
            sender->state = TASK_STATE_READY;
        }
        return ETASKNOTREPLYBLOCKED;
    }

    struct mail *letter = (struct mail *)sender->sp;
    sender->sp += sizeof(struct mail);

    int len = MIN(replylen, letter->replylen);
    memcpy(letter->reply, reply, len);

    /* Wake up and requeue partner. */
    platform_task_return_value(sender, replylen);
    sender->state = TASK_STATE_READY;
    readyq_enqueue(tasks, sender);

    return replylen > letter->replylen ? EFULLREPLY : 0;
}

int sys_AsyncSend(tid_t tid, void *msg, int msglen)
{
    struct tasks *tasks = &caboose.tasks;
    struct task *active = tasks->active;

    if (tid < 0 || tid >= CONFIG_TASK_COUNT) {
        return EIMPOSSIBLETID;
    }

    struct task *recipient = &tasks->table[tid];
    if (recipient->state == TASK_STATE_ZOMBIE) {
        return ETASKNOTEXIST;
    }

    ASSERT(msglen <= ASYNC_SIZE_MAX);

    if (recipient->state == TASK_STATE_ASYNC_SEND_BLOCKED) {
        struct mailbox *recipient_mailbox = (struct mailbox *)recipient->sp;
        recipient->sp += sizeof(struct mailbox);

        *(recipient_mailbox->sender_tid) = active->tid;

        int len = MIN(msglen, recipient_mailbox->msglen);
        memcpy(recipient_mailbox->msg, msg, len);

        /* Update reciever trapframe with return. */
        platform_task_return_value(recipient, msglen);

        recipient->state = TASK_STATE_READY;
        readyq_enqueue(tasks, recipient);
        return 0;
    }

    ASSERT(!list_empty(&tasks->async_free));

    struct node *asyncbuf = list_remove_head(&tasks->async_free);
    struct asyncmsg *msgbuf = containerof(asyncbuf, struct asyncmsg, node);

    msgbuf->sender = active->tid;
    msgbuf->msglen = msglen;
    memcpy(msgbuf->data, msg, msglen);
    list_insert_tail(&recipient->async, &msgbuf->node);

    return 0;
}

int sys_AsyncReceive(tid_t *tid, void *msg, int msglen)
{
    struct tasks *tasks = &caboose.tasks;
    struct task *active = tasks->active;
    active->state = TASK_STATE_ASYNC_SEND_BLOCKED;

    if (!list_empty(&active->async)) {
        struct node *asyncbuf = list_remove_head(&active->async);
        struct asyncmsg *msgbuf = containerof(asyncbuf, struct asyncmsg, node);

        *tid = msgbuf->sender;

        /* Do the copy. */
        int len = MIN(msglen, msgbuf->msglen);
        memcpy(msg, msgbuf->data, len);

        /* Put the async buffer back on the free list. */
        list_insert_tail(&tasks->async_free, asyncbuf);

        active->state = TASK_STATE_ACTIVE;
        return msgbuf->msglen;
    }

    active->sp -= sizeof(struct mailbox);
    struct mailbox *box = (struct mailbox *)active->sp;
    *box = (struct mailbox) {
        .sender_tid = tid,
        .msg = msg,
        .msglen = msglen
    };

    /* This return value is subsequently replaced. */
    return 0;
}
