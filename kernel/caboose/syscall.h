#ifndef CABOOSE_SYSCALL_H
#define CABOOSE_SYSCALL_H

#include <caboose/types.h>

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

int sys_AwaitEvent(int event_id);

#endif
