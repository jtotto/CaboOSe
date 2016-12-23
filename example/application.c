#include <caboose-platform/qemu-pl011.h>

void application(void)
{
    uart0_tx("test text", sizeof "test text" - 1);
    while (1);
}
