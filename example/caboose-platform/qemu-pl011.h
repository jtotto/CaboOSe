#ifndef CABOOSE_PLATFORM_QEMU_PL011_H
#define CABOOSE_PLATFORM_QEMU_PL011_H

#include <stddef.h>

void uart0_tx(const char *buf, size_t len);

#endif
