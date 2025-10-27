#include <go-board.h>
#include <ssd1331.h>

_fast static void gradient_h(void) {
    for (int x = 0; x < SSD1331_WIDTH; x++) {
        uint8_t v = (uint8_t)((x * 255) / (SSD1331_WIDTH - 1));
        draw_line_color((uint8_t)x, 0, (uint8_t)x, (uint8_t)(SSD1331_HEIGHT - 1), v, v, v);
    }
}

_fast static void gradient_v(void) {
    for (int y = 0; y < SSD1331_HEIGHT; y++) {
        uint8_t v = (uint8_t)((y * 255) / (SSD1331_HEIGHT - 1));
        draw_line_color(0, (uint8_t)y, (uint8_t)(SSD1331_WIDTH - 1), (uint8_t)y, v, v, v);
    }
}

_fast static void checkerboard(void) {
    const int tile = 8;

    fill_solid(255, 255, 255);
    delay_ms(5);

    for (int ty = 0; ty < SSD1331_HEIGHT / tile; ty++) {
        for (int tx = 0; tx < SSD1331_WIDTH / tile; tx++) {
            if (((tx + ty) & 1) == 0) {
                draw_filled_rect((uint8_t)(tx * tile), (uint8_t)(ty * tile), (uint8_t)tile, (uint8_t)tile, 0, 0, 0);
            }
        }
    }
}

_fast static void color_bars(void) {
    draw_filled_rect(0,   0, 12, SSD1331_HEIGHT, 255, 0,   0  ); // Red
    draw_filled_rect(12,  0, 12, SSD1331_HEIGHT, 255, 255, 0  ); // Yellow
    draw_filled_rect(24,  0, 12, SSD1331_HEIGHT, 0,   255, 0  ); // Green
    draw_filled_rect(36,  0, 12, SSD1331_HEIGHT, 0,   255, 255); // Cyan
    draw_filled_rect(48,  0, 12, SSD1331_HEIGHT, 0,   0,   255); // Blue
    draw_filled_rect(60,  0, 12, SSD1331_HEIGHT, 255, 0,   255); // Magenta
    draw_filled_rect(72,  0, 12, SSD1331_HEIGHT, 255, 255, 255); // White
    draw_filled_rect(84,  0, 12, SSD1331_HEIGHT, 0,   0,   0  ); // Black
}

_fast int main(void) {
    ssd1331_init();

    int pat = 0;
    while (1) {
        IO_OUT(IO_LEDS, (uint32_t)(pat & 0x7));

        switch (pat) {
            case 0: fill_solid(255, 255, 255); break;
            case 1: fill_solid(255, 0,   0  ); break;
            case 2: fill_solid(0,   255, 0  ); break;
            case 3: fill_solid(0,   0,   255); break;
            case 4: gradient_h();              break;
            case 5: gradient_v();              break;
            case 6: checkerboard();            break;
            default: color_bars();             break;
        }

        delay_ms(250);
        while (~IO_IN(IO_SW) & PIN_SW3);
        pat = (pat + 1) % 8;
    }
}
