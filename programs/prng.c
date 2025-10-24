#include "include/go-board.h"
#include <stdint.h>

#define TOTAL 10000000u // 10M

_fast uint32_t random(uint32_t* state) {
    *state = *state * 747796405u + 2891336453u;
    uint32_t word = ((*state >> ((*state >> 28u) + 4u)) ^ *state) * 277803737u;
    return (word >> 22u) ^ word;
}

_fast int main(void) {
    uint32_t state = 0;
    uint32_t progress = 0;

    const uint32_t step = TOTAL / 100u;
    uint32_t next_event = 0;

    for (uint32_t i = 0; i < TOTAL; i++) {
        if (i == next_event) {
            uint32_t high = to_seg((progress / 10) % 10);
            uint32_t low = to_seg(progress % 10);

            IO_OUT(IO_SEG_ONE, high);
            IO_OUT(IO_SEG_TWO, low);

            progress++;
            next_event += step;
        }

        state = random(&state);
    }

    IO_OUT(IO_SEG_ONE, to_seg((state / 10) % 10));
    IO_OUT(IO_SEG_TWO, to_seg((state) % 10));

    return 0;
}
