void rp(const char *text) {
    kprintf("\x1b[31m%s\x1b[0m", text);
}
void gp(const char *text) {
    kprintf("\x1b[32m%s\x1b[0m", text);
}
#include <types.h>
#include <machine/tlb.h>
