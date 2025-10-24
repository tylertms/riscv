#include "go-board.h"
#define SPI_FLASH_BASE ((char*)((1 << 23) + (1 << 16)))

int main(void) {
    for(;;) for (uint8_t i=0; i < 16; i++) {
       uint32_t num = (uint32_t)SPI_FLASH_BASE[i];
       uint32_t high = to_seg((num >> 4) & 0xF);
       uint32_t low = to_seg(num & 0xF);

       IO_OUT(IO_LEDS, i);
       IO_OUT(IO_SEG_ONE, high);
       IO_OUT(IO_SEG_TWO, low);

       delay_ms(1000);
    }

    return 0;
}
