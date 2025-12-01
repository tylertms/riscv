#include <go-board.h>
#include <ssd1331.h>
#include "rotunda.h"

_fast int main(void) {
    ssd1331_init();

    ssd1331_cmd0(SSD1331_CMD_WRITE_RAM);
    ssd1331_stream_begin();

    for (uint16_t i = 0; i < SSD1331_PIXEL_COUNT; i++) {
        uint16_t color = image[i];

        ssd1331_spi_send(color >> 8);
        ssd1331_spi_send(color & 0xFF);
    }

    ssd1331_stream_end();

    return 0;
}
