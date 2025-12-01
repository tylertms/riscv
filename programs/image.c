#include <go-board.h>
#include <ssd1331.h>
#include "rotunda.h"

_fast int main(void) {
    const uint16_t* images[NUM_IMAGES] = {rotunda_a, rotunda_c};
    uint8_t image_index = 0;

    ssd1331_init();

    ssd1331_cmd0(SSD1331_CMD_WRITE_RAM);
    ssd1331_stream_begin();

    while (1) {
        const uint16_t* image = images[image_index];

        for (uint16_t i = 0; i < SSD1331_PIXEL_COUNT; i++) {
            uint16_t color = image[i];

            ssd1331_spi_send(color >> 8);
            ssd1331_spi_send(color & 0xFF);
        }

        image_index = (image_index + 1 == NUM_IMAGES) ? 0 : (image_index + 1);
        delay_ms(100);
        while (~IO_IN(IO_SW) & PIN_SW3);
    }

    ssd1331_stream_end();

    return 0;
}
