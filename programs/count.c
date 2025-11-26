#include <go-board.h>


// _fast is our custom directive thattells the toolchain to
// place main() into RAM. After init.s copies this function
// into RAM, main() is called where the linker placed it.
_fast int main(void) {
    uint32_t num = 0;

    for (;;) {
        // Write `num` to our 7-seg displays
        // Using the compiler's / and % routines
        IO_OUT(IO_SEG_ONE, to_seg((num / 10) % 10));
        IO_OUT(IO_SEG_TWO, to_seg(num % 10));

        // Wait for SW3 to be pressed
        while (~IO_IN(IO_SW) & PIN_SW3);

        // Increment num (mod 100);
        num = (num == 99) ? 0 : (num + 1);
    }

    // jump back to init.s, stay in infinite loop
    return 0;
}
