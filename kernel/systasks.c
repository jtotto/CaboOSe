#include "caboose.h"
#include "config.h"
#include "errcode.h"
#include "platform.h"
#include "util.h"

void nameserver(void);
void nulltask(void);
void application(void);

void init(void)
{
    /* The name server must get TID 1. */
    Create(CONFIG_NAMESERVER_PRIORITY, nameserver);
    Create(LOW_PRIORITY, nulltask);
    Create(CONFIG_APPLICATION_INIT_PRIORITY, application);

    Exit();
}

void nulltask(void)
{
    while (true) {
        /* spin */
    }
}

/* It's all nameserver from here on down. */

struct record {
   char name[MAXNAMESIZE];
   tid_t tid;
};

struct request {
    enum {
        REGISTER,
        WHOIS
    } type;
    char name[MAXNAMESIZE];
};

struct register_response {
    int rc;
};

struct whois_response {
    tid_t tid;
};

static bool streq(const char *s1, const char *s2)
{
    return strcmp(s1, s2) == 0;
}

static struct record *lookup(struct record *records, size_t n, const char *name)
{
    for (size_t i = 0; i < n; i++) {
        if (streq(records[i].name, name)) {
            return &records[i];
        }
    }

    return NULL;
}

void nameserver(void)
{
    ASSERT(MyTid() == NAMESERVER_TID);

    int next_free = 0;
    struct record records[CONFIG_TASK_COUNT];

    struct whois_blocked {
        tid_t tid;
        char name[MAXNAMESIZE];
        bool blocked;
    } whois_blocked[CONFIG_TASK_COUNT];
    memset(whois_blocked, 0, sizeof whois_blocked);

    while (true) {
        struct request request;
        tid_t sender;
        int received = Receive(&sender, &request, sizeof request);
        ASSERT(received == sizeof request);

        switch (request.type) {
        case REGISTER: {
            struct record *record = lookup(records, next_free, request.name);
            if (!record) {
                ASSERT(next_free < CONFIG_TASK_COUNT);
                record = &records[next_free];
                next_free++;

                /* If this name isn't yet registered, it's possible we'll
                 * unblock some number of tasks that are blocked waiting for
                 * this registration. */
                for (int i = 0; i < CONFIG_TASK_COUNT; i++) {
                    struct whois_blocked *blocked_task = &whois_blocked[i];
                    if (blocked_task->blocked
                        && streq(blocked_task->name, request.name)) {
                        blocked_task->blocked = false;

                        struct whois_response response;
                        response = (struct whois_response) {
                            .tid = sender
                        };

                        int replied = !Reply(blocked_task->tid,
                                             &response,
                                             sizeof response);
                        ASSERT(replied);
                    }
                }
            }

            strcpy(record->name, request.name);
            record->tid = sender;

            struct register_response response = (struct register_response) {
                .rc = 0
            };

            int replied = !Reply(sender, &response, sizeof response);
            ASSERT(replied);
        } break;
        case WHOIS: {
            struct record *record = lookup(records, next_free, request.name);
            if (record) {
                struct whois_response response = (struct whois_response) {
                    .tid = record->tid
                };

                int replied = !Reply(sender, &response, sizeof response);
                ASSERT(replied);
            } else {
                struct whois_blocked *blocked_data = &whois_blocked[sender];
                blocked_data->tid = sender;
                strcpy(blocked_data->name, request.name);
                blocked_data->blocked = true;
            }
        } break;
        default:
            ASSERT(false);
            break;
        }
    }

    ASSERT(false); /* The nameserver never terminates. */
}

int RegisterAs(const char *name)
{
    struct request request = (struct request) {
        .type = REGISTER
    };
    strcpy(request.name, name);

    struct register_response response;

    int replylen = Send(NAMESERVER_TID,
                        &request,
                        sizeof request,
                        &response,
                        sizeof response);

    if (replylen != sizeof response) {
        return ENAMESERVERINVALID;
    }

    return response.rc;
}

int WhoIs(const char *name)
{
    struct request request = (struct request) {
        .type = WHOIS
    };
    strcpy(request.name, name);

    struct whois_response response;

    int replylen = Send(NAMESERVER_TID,
                        &request,
                        sizeof request,
                        &response,
                        sizeof response);

    if (replylen != sizeof response) {
        return ENAMESERVERINVALID;
    }

    return response.tid;
}
