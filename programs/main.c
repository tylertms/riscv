#include <stdint.h>

#define IO_BASE     0x400000u
#define IO_LEDS_OFF 0x0004u
#define LEDS        (*(volatile uint32_t *)(IO_BASE + IO_LEDS_OFF))

static void wait(void) {
    volatile uint32_t t = 1u << 18;
    while (t--) __asm__ volatile ("");
}

int main(void) {
    uint32_t v = 0xA;

    for (;;) {
        LEDS = v & 0xFu;
        wait();
        v = (~v) & 0xFu;
    }

    return 0;
}
