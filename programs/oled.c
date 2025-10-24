#include "go-board.h"
#include "ssd1331.h"

#define RGB565(r,g,b) ( ((uint16_t)((r) & 0xF8) << 8) | ((uint16_t)((g) & 0xFC) << 3) | ((uint16_t)((b) >> 3)) )

static inline void frame_begin(void) {
    ssd1331_set_addr_window(0, 0, SSD1331_WIDTH, SSD1331_HEIGHT);
    ssd1331_cmd0(0x5C);
    ssd1331_stream_begin();
}

static inline void frame_end(void) {
    ssd1331_stream_end();
}

_fast static void fill_solid(uint16_t c) {
    frame_begin();

    uint8_t hi = (uint8_t)(c >> 8);
    uint8_t lo = (uint8_t)(c & 0xFF);

    for (int i = 0; i < SSD1331_PIXEL_COUNT; i++) {
        ssd1331_stream_byte(hi);
        ssd1331_stream_byte(lo);
    }

    frame_end();
}

_fast static void gradient_h(void) {
    frame_begin();

    for (int y = 0; y < SSD1331_HEIGHT; y++) {
        for (int x = 0; x < SSD1331_WIDTH; x++) {
            uint8_t v = (uint8_t)((x * 255) / (SSD1331_WIDTH - 1));
            uint16_t c = RGB565(v, v, v);
            ssd1331_stream_byte(c >> 8);
            ssd1331_stream_byte(c & 0xFF);
        }
    }

    frame_end();
}

_fast static void gradient_v(void) {
    frame_begin();
    for (int y = 0; y < SSD1331_HEIGHT; y++) {
        uint8_t v = (uint8_t)((y * 255) / (SSD1331_HEIGHT - 1));
        uint16_t c = RGB565(v, v, v);

        uint8_t hi = c >> 8;
        uint8_t lo = c & 0xFF;

        for (int x = 0; x < SSD1331_WIDTH; x++) {
            ssd1331_stream_byte(hi);
            ssd1331_stream_byte(lo);
        }
    }
    frame_end();
}

_fast static void checkerboard(void) {
    const int tile = 4;
    frame_begin();
    for (int y = 0; y < SSD1331_HEIGHT; y++) {
        for (int x = 0; x < SSD1331_WIDTH; x++) {
            int t = ((x / tile) ^ (y / tile)) & 1;
            uint16_t c = t ? RGB565(255,255,255) : RGB565(0,0,0);

            ssd1331_stream_byte(c >> 8);
            ssd1331_stream_byte(c & 0xFF);
        }
    }
    frame_end();
}

_fast static void color_bars(void) {
    const uint16_t bars[] = {
        RGB565(255,0,0), RGB565(255,255,0), RGB565(0,255,0),
        RGB565(0,255,255), RGB565(0,0,255), RGB565(255,0,255),
        RGB565(255,255,255), RGB565(0,0,0)
    };

    const int nb = (int)(sizeof(bars) / sizeof(bars[0]));
    int bw = SSD1331_WIDTH / nb;

    frame_begin();

    for (int y = 0; y < SSD1331_HEIGHT; y++) {
        for (int x = 0; x < SSD1331_WIDTH; x++) {
            int i = x / bw; if (i >= nb) i = nb - 1;
            uint16_t c = bars[i];

            ssd1331_stream_byte(c >> 8);
            ssd1331_stream_byte(c & 0xFF);
        }
    }

    frame_end();
}

_fast int main(void) {
    ssd1331_init();

    int pat = 0;
    while (1) {
        IO_OUT(IO_LEDS, (uint32_t)(pat & 0x7));

        switch (pat) {
            case 0: fill_solid(RGB565(255,255,255)); break;
            case 1: fill_solid(RGB565(255,0,0));     break;
            case 2: fill_solid(RGB565(0,255,0));     break;
            case 3: fill_solid(RGB565(0,0,255));     break;
            case 4: gradient_h();                    break;
            case 5: gradient_v();                    break;
            case 6: checkerboard();                  break;
            default: color_bars();                   break;
        }

        while (~IO_IN(IO_SW) & PIN_SW3);
        pat = (pat + 1) % 8;
    }
}
