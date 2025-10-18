#include "go-board.h"
#include <stdint.h>

#define TOTAL 1000000000u // 1B

uint32_t random(uint32_t* state) {
    *state = *state * 747796405u + 2891336453u;
    uint32_t word = ((*state >> ((*state >> 28u) + 4u)) ^ *state) * 277803737u;
    return (word >> 22u) ^ word;
}

int main(void) {
    uint32_t state = 0;
    uint32_t progress = 0;

    const uint32_t step = TOTAL / 100u;
    uint32_t next_event = 0;

    for (uint32_t i = 0; i < TOTAL; i++) {
        if (i == next_event) {
            write_digit(SEG_ONE, (progress / 10) % 10);
            write_digit(SEG_TWO, progress % 10);
            progress++;
            next_event += step;
        }

        state = random(&state);
    }

    write_digit(SEG_ONE, (state / 10) % 10);
    write_digit(SEG_TWO, (state) % 10);

    return 0;
}
