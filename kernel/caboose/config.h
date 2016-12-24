#ifndef CABOOSE_CONFIG_H
#define CABOOSE_CONFIG_H

#include <caboose-platform/config.h>

#if    !defined(CONFIG_EVENT_RING_COUNT)                                       \
    || !defined(CONFIG_EVENT_COUNT)                                            \
    || !defined(CONFIG_TASK_COUNT)                                             \
    || !defined(CONFIG_TASK_STACK_SIZE)                                        \
    || !defined(CONFIG_ASYNC_MSG_BUFSIZE)                                      \
    || !defined(CONFIG_ASYNC_MSG_COUNT)                                        \
    || !defined(CONFIG_NAMESERVER_PRIORITY)                                    \
    || !defined(CONFIG_APPLICATION_INIT_PRIORITY)                              \
    || !defined(CONFIG_INIT_PRIORITY)                                          \
    || !defined(CONFIG_PRIORITY_COUNT)
#error "Incomplete platform configuration"
#endif

#endif
