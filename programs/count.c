#include "go-board.h"
#include <stdint.h>

int main(void) {
    uint32_t num = 0;

    for (;;) {
        write_digit(SEG_ONE, (num / 10) % 10);
        write_digit(SEG_TWO, num % 10);

        delay(200);

        num = (num == 99) ? 0 : (num + 1);
    }

    return 0;
}
