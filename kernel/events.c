#include "errcode.h"
#include "events.h"
#include "platform.h"
#include "ringbuffer.h"
#include "state.h"
#include "syscall.h"

#define CONFIG_EVENT_RING_SIZE (CONFIG_EVENT_RING_COUNT * sizeof(int))

uint8_t *events_init(uint8_t *pool)
{
    struct event *events = caboose.events;
    memset(events, 0, sizeof caboose.events);

    for (int i = 0; i < CONFIG_EVENT_COUNT; i++){
        ringbuffer_init(&events[i].ring,
                        pool,
                        sizeof(int),
                        CONFIG_EVENT_RING_SIZE);
        pool += CONFIG_EVENT_RING_SIZE;
    }

    return pool;
}

int sys_AwaitEvent(int event_id)
{
    if (event_id < 0 || event_id >= CONFIG_EVENT_COUNT){
        return EINVALIDEVENT;
    }

    struct event *event = &caboose.events[event_id];
    struct task *active =caboose.tasks.active;

    uint32_t ret;
    /* Has the event we're interested in already occurred?  If so, its value is
     * read into ret immediately.  Otherwise, we'll block until it does, and our
     * return value will be filled in then. */
    bool consumed = ringbuffer_consume_unit(&event->ring, (uint8_t *)&ret);
    if (!consumed) {
        /* Setting the state assures we aren't rescheduled. */
        active->state = TASK_STATE_EVENT_BLOCKED;

        ASSERT(!event->waiter);
        event->waiter = active;

        ret = ECOLLECTVOLATILE;
    }

    /* Return the collect flag as it will be replaced */
    return ret;
}
