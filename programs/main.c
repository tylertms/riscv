#include <stdint.h>

#define IO_BASE     0x400000u
#define IO_LEDS_OFF 0x0004u

#define LEDS        (*(volatile uint32_t *)(IO_BASE + IO_LEDS_OFF))

static void wait(void) {
    volatile uint32_t t = 1u << 18;
    while (t--) __asm__ volatile ("");
}

int main(void) {
    uint32_t n = 0x432903;

    for (;;) {
        LEDS = n & 0xF;
        wait();
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * n + 1;
        }
    }

    return 0;
}
