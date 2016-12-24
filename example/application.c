#include <stdbool.h>

#include <caboose/caboose.h>

#include <caboose-platform/platform-events.h>
#include <caboose-platform/qemu-pl011.h>
#include <caboose-platform/timer.h>

#define TX_STR(s) uart0_tx(s, sizeof s - 1)

void ticker(void)
{
    tid_t parent = MyParentTid();
    while (true) {
        /* Wait for 100 ticks. */
        for (int i = 0; i < 100; i++) {
            AwaitEvent(TIMER_EVENTID);
        }

        /* Wake our parent. */
        Send(parent, NULL, 0, NULL, 0);
    }
}

void application(void)
{
    TX_STR("Application starting!\n");

    tid_t ticker_tid = Create(2, ticker);
    while (true) {
        tid_t sender;
        Receive(&sender, NULL, 0);
        ASSERT(sender == ticker_tid);

        Reply(sender, NULL, 0);

        TX_STR("tick\n");
    }
}
