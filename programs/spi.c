#include "go-board.h"
#define SPI_FLASH_BASE ((char*)(1 << 23))

int main() {
    for (int i = 0; i < 16; i++) {
        IO_OUT(IO_LEDS, i);

        uint32_t low = to_seg(SPI_FLASH_BASE[2 * i]);
        uint32_t high = to_seg(SPI_FLASH_BASE[2 * i + 1]);

        IO_OUT(IO_SEG_ONE, high);
        IO_OUT(IO_SEG_TWO, low);

        delay(500);
    }
}
