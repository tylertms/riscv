#include <go-board.h>

_fast int main(void) {
    uint32_t num = 0;

    for (;;) {
        IO_OUT(IO_SEG_ONE, to_seg((num / 10) % 10));
        IO_OUT(IO_SEG_TWO, to_seg(num % 10));

        delay_ms(200);

        num = (num == 99) ? 0 : (num + 1);
    }

    return 0;
}
