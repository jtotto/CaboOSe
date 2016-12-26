#ifndef CABOOSE_H
#define CABOOSE_H

#include <stdbool.h>
#include <stdint.h>

#include "errcode.h"
#include "events.h"
#include "tasks.h"

/* Task management. */
tid_t Create(int priority, void (*code)());
tid_t MyTid();
tid_t MyParentTid();

void Pass();
void Exit();

/* Synchronous message passing. */
int Send(tid_t tid, const void *msg, int msglen, void *reply, int replylen);
int Receive(tid_t *tid, void *msg, int msglen);
int Reply(tid_t tid, void *reply, int replylen);

/* Asynchronous message passing. */
int AsyncSend(tid_t tid, void *msg, int msglen);
int AsyncReceive(tid_t *tid, void *msg, int msglen);

/* System event notification. */
int AwaitEvent(int eventid);

/* The name server. */
#define NAMESERVER_TID 1
#define MAXNAMESIZE 16
int RegisterAs(const char *name);
int WhoIs(const char *name);

#endif
