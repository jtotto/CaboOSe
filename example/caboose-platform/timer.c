/* This drives the VE_TIMER01 sp804 timer. */

#include <caboose/events.h>

#include "irq.h"
#include "platform-events.h"
#include "timer.h"

#define TIMER01_BASE 0x1c110000

#define LOAD    0x0
#define VALUE   0x4
#define CONTROL 0x8
#define INTCLR  0xc

#define CONTROL_ENABLE      0b10000000
#define CONTROL_MODE        0b01000000
#define CONTROL_INTENABLE   0b00100000
#define CONTROL_SIZE        0b00000010

static void timer_irq_handler(void)
{
    /* Deliver a dummy event. */
    event_deliver(TIMER_EVENTID, 0xcab005e);

    volatile uint32_t *clear = (uint32_t *)(TIMER01_BASE + INTCLR);
    *clear = 0xcab005e;
}

/* In periodic interrupt mode, we use the load register to specify the number of
 * ticks we want per interrupt.  I have no idea what the clock frequency is (and
 * honestly don't really care enough to find out for the purposes of this
 * demonstration), so I'm not sure exactly how much time I'm asking for here,
 * but experimentally it's about the amount I'm looking for. */
#define LOAD_INITIAL (10000 - 1)

uint8_t *timer_init(uint8_t *pool)
{
    volatile uint32_t *load = (uint32_t *)(TIMER01_BASE + LOAD);
    volatile uint32_t *ctrl = (uint32_t *)(TIMER01_BASE + CONTROL);
    *load = LOAD_INITIAL;
    *ctrl |= CONTROL_ENABLE |
             CONTROL_MODE |
             CONTROL_INTENABLE |
             CONTROL_SIZE;

    irq_register(2, timer_irq_handler);

    return pool;
}

uint32_t timer_read(void)
{
    volatile uint32_t *value = (uint32_t *)(TIMER01_BASE + VALUE);
    return *value;
}
