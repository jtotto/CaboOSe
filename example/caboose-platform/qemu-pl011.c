#include <stdint.h>

#include "qemu-pl011.h"

/* PL011 register layout from Xen - thanks Tim! */

/* PL012 register addresses */
#define DR     (0x00)
#define RSR    (0x04)
#define FR     (0x18)
#define ILPR   (0x20)
#define IBRD   (0x24)
#define FBRD   (0x28)
#define LCR_H  (0x2c)
#define CR     (0x30)
#define IFLS   (0x34)
#define IMSC   (0x38)
#define RIS    (0x3c)
#define MIS    (0x40)
#define ICR    (0x44)
#define DMACR  (0x48)

/* CR bits */
#define CTSEN  (1<<15) /* automatic CTS hardware flow control */
#define RTSEN  (1<<14) /* automatic RTS hardware flow control */
#define RTS    (1<<11) /* RTS signal */
#define DTR    (1<<10) /* DTR signal */
#define RXE    (1<<9) /* Receive enable */
#define TXE    (1<<8) /* Transmit enable */
#define UARTEN (1<<0) /* UART enable */

/* FR bits */
#define TXFE   (1<<7) /* TX FIFO empty */
#define RXFE   (1<<4) /* RX FIFO empty */
#define BUSY   (1<<3) /* Transmit is not complete */

/* Defined in vexpress.c in motherboard_aseries_map */
#define UART0_BASE 0x1c090000

/* Do the bare minimum here - the QEMU PL011 doesn't need any initialization. */

static void uart0_putc(char c)
{
    volatile uint32_t *dr, *fr;
    dr = (uint32_t *)(UART0_BASE + DR);
    fr = (uint32_t *)(UART0_BASE + FR);

    while ((*fr & TXFE) == 0) {
        /* spin */
    }

    *dr = c;
}

void uart0_tx(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        uart0_putc(buf[i]);
    }
}
