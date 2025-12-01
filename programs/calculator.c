#include <go-board.h>

_fast int main(void)
{
    uint32_t value = 0;
    uint32_t calc_op = 0;
    while (1)
    {
        // Busy-Wait
        while (!IO_IN(IO_SW)){}

        if (IO_IN(IO_SW) & PIN_SW1)
        {
            value++;
        }
        if (IO_IN(IO_SW) & PIN_SW2)
        {
            value--;
        }
        if (IO_IN(IO_SW) & PIN_SW3)
        {
            value = 1;
        }
        if (IO_IN(IO_SW) & PIN_SW4)
        {
            value = value << 1;
        }

        IO_OUT(IO_SEG_ONE, to_seg((value >> 4) & 0xF));
        IO_OUT(IO_SEG_TWO, to_seg(value & 0xF));

        delay_ms(250);
    }
    return 0;
}
