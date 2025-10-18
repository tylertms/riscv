#include "go-board.h"
#include <stdint.h>

int main(void) {
    uint32_t n = 0x0;

    for (;;) {
        write_digit(SEG_ONE, n / 10);
        write_digit(SEG_TWO, n % 10);
        delay(1000);
        n = (n + 1) % 100;
    }

    return 0;
}
