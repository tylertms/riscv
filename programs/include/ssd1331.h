#pragma once
#include "go-board.h"

/* ---------------- Screen Definitions ---------------- */
#define SSD1331_WIDTH 96u
#define SSD1331_HEIGHT 64u
#define SSD1331_PIXEL_COUNT (SSD1331_WIDTH * SSD1331_HEIGHT)

/* ---------------- PMOD/OLED bits ---------------- */
#define SSD1331_PMOD_EN    (1u << 0)   /* PMOD power enable                      */
#define SSD1331_VCC_EN     (1u << 1)   /* Panel VCC enable                       */
#define SSD1331_RES_N      (1u << 2)   /* RESET#, active low                     */
#define SSD1331_DC         (1u << 3)   /* D/C (0=command, 1=data)                */
#define SSD1331_SCLK       (1u << 4)   /* SPI SCLK                               */
#define SSD1331_NC         (1u << 5)   /* not connected                          */
#define SSD1331_MOSI       (1u << 6)   /* SPI MOSI (DIN)                         */
#define SSD1331_CS_N       (1u << 7)   /* CHIP SELECT#, active low               */

/* ---------------- MMIO helpers ---------------- */
#define _PMOD_PORT   (*(volatile uint32_t*)(uintptr_t)(IO_BASE + IO_PMOD))

static volatile uint32_t _pmod_state = 0;
static inline void _pmod_write(uint32_t v) { _PMOD_PORT = v; }
static inline void _pmod_write_state(uint32_t v) { _pmod_state = v; _PMOD_PORT = v; }

/* ---------------- Safe idle / rail defaults ---------------- */
static inline void pmod_init(void) {
  uint32_t v = 0x00;
  v |=  (SSD1331_CS_N | SSD1331_RES_N | SSD1331_PMOD_EN);
  v &= ~(SSD1331_SCLK | SSD1331_MOSI | SSD1331_DC | SSD1331_VCC_EN);
  _pmod_write_state(v);
}

static inline void pmod_set(uint32_t m) { _pmod_write_state(_pmod_state |  m); }
static inline void pmod_clr(uint32_t m) { _pmod_write_state(_pmod_state & ~m); }

static inline void ssd1331_cs_assert(void)      { pmod_clr(SSD1331_CS_N);  }
static inline void ssd1331_cs_deassert(void)    { pmod_set(SSD1331_CS_N);  }
static inline void ssd1331_reset_assert(void)   { pmod_clr(SSD1331_RES_N); }
static inline void ssd1331_reset_deassert(void) { pmod_set(SSD1331_RES_N); }
static inline void ssd1331_dc_cmd(void)         { pmod_clr(SSD1331_DC);    }
static inline void ssd1331_dc_data(void)        { pmod_set(SSD1331_DC);    }

/* ---------------- SSD1331 command opcodes ---------------- */
typedef enum {
    // --- Addressing ---
    SSD1331_SET_COLUMN_ADDR              = 0x15,
    SSD1331_SET_ROW_ADDR                 = 0x75,

    // --- Brightness / current ---
    SSD1331_SET_CONTRAST_A               = 0x81,
    SSD1331_SET_CONTRAST_B               = 0x82,
    SSD1331_SET_CONTRAST_C               = 0x83,
    SSD1331_MASTER_CURRENT_CONTROL       = 0x87,
    SSD1331_SET_SECOND_PRECHARGE_A       = 0x8A,
    SSD1331_SET_SECOND_PRECHARGE_B       = 0x8B,
    SSD1331_SET_SECOND_PRECHARGE_C       = 0x8C,

    // --- Addressing / mapping / display position ---
    SSD1331_SET_REMAP_COLOR_DEPTH        = 0xA0,
    SSD1331_SET_DISPLAY_START_LINE       = 0xA1,
    SSD1331_SET_DISPLAY_OFFSET           = 0xA2,
    SSD1331_SET_DISPLAY_MODE_NORMAL      = 0xA4,
    SSD1331_SET_DISPLAY_MODE_ALL_ON      = 0xA5,
    SSD1331_SET_DISPLAY_MODE_ALL_OFF     = 0xA6,
    SSD1331_SET_DISPLAY_MODE_INVERT      = 0xA7,
    SSD1331_SET_MULTIPLEX_RATIO          = 0xA8,
    SSD1331_DIM_MODE_SETTING             = 0xAB,
    SSD1331_SET_MASTER_CONFIG            = 0xAD,

    // --- Display on/off sequence ---
    SSD1331_DISPLAY_OFF                  = 0xAE,
    SSD1331_DISPLAY_ON                   = 0xAF,
    SSD1331_ENTIRE_DISPLAY_ON            = 0xAC,

    // --- Power / timing / gray scale ---
    SSD1331_POWER_SAVE_MODE              = 0xB0,
    SSD1331_PHASE_PERIOD_ADJUST          = 0xB1,
    SSD1331_DISPLAY_CLOCK_DIV_OSC        = 0xB3,
    SSD1331_SET_GRAY_SCALE_TABLE         = 0xB8,
    SSD1331_ENABLE_LINEAR_GRAY_SCALE     = 0xB9,
    SSD1331_SET_PRECHARGE_VOLTAGE        = 0xBB,
    SSD1331_NOP_BC                       = 0xBC,
    SSD1331_NOP_BD                       = 0xBD,
    SSD1331_SET_VCOMH                    = 0xBE,
    SSD1331_NOP_E3                       = 0xE3,
    SSD1331_SET_COMMAND_LOCK             = 0xFD,

    // --- Graphics Acceleration (GAC) ---
    SSD1331_DRAW_LINE                    = 0x21,
    SSD1331_DRAW_RECT                    = 0x22,
    SSD1331_COPY                         = 0x23,
    SSD1331_DIM_WINDOW                   = 0x24,
    SSD1331_CLEAR_WINDOW                 = 0x25,
    SSD1331_FILL_ENABLE                  = 0x26,
    SSD1331_SCROLL_SETUP                 = 0x27,
    SSD1331_SCROLL_DEACTIVATE            = 0x2E,
    SSD1331_SCROLL_ACTIVATE              = 0x2F
} ssd1331_cmd_t;

/* ---------------- SPI ---------------- */
_fast static void ssd1331_spi_send(uint8_t byte) {
    uint32_t v = _pmod_state & ~(SSD1331_SCLK | SSD1331_MOSI);
    volatile uint32_t* const P = &_PMOD_PORT;
    *P = v;

    for (uint8_t i = 0; i < 8; i++) {
        uint32_t vd = (byte & 0x80) ? (v | SSD1331_MOSI) : v;
        *P = vd;
        *P = vd | SSD1331_SCLK;
        *P = vd;
        byte <<= 1;
    }

    _pmod_state = v;
}

/* ---------------- Command Helpers ---------------- */
_fast static void ssd1331_cmd0(uint8_t cmd) {
    ssd1331_dc_cmd(); ssd1331_cs_assert();
    ssd1331_spi_send(cmd);
    ssd1331_cs_deassert();
}

_fast static void ssd1331_cmd1(uint8_t cmd, uint8_t a) {
    ssd1331_dc_cmd(); ssd1331_cs_assert();
    ssd1331_spi_send(cmd); ssd1331_spi_send(a);
    ssd1331_cs_deassert();
}

_fast static void ssd1331_cmd2(uint8_t cmd, uint8_t a, uint8_t b) {
    ssd1331_dc_cmd(); ssd1331_cs_assert();
    ssd1331_spi_send(cmd); ssd1331_spi_send(a); ssd1331_spi_send(b);
    ssd1331_cs_deassert();
}

_fast static void ssd1331_cmd3(uint8_t cmd, uint8_t a, uint8_t b, uint8_t c) {
    ssd1331_dc_cmd(); ssd1331_cs_assert();
    ssd1331_spi_send(cmd); ssd1331_spi_send(a); ssd1331_spi_send(b); ssd1331_spi_send(c);
    ssd1331_cs_deassert();
}

_fast static void ssd1331_cmd4(uint8_t cmd, uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    ssd1331_dc_cmd(); ssd1331_cs_assert();
    ssd1331_spi_send(cmd); ssd1331_spi_send(a); ssd1331_spi_send(b); ssd1331_spi_send(c); ssd1331_spi_send(d);
    ssd1331_cs_deassert();
}

static inline uint8_t q5(uint8_t v) { return (uint8_t)((v >> 3) << 1); }
static inline uint8_t q6(uint8_t v) { return (uint8_t)(v >> 2); }

static inline void ssd1331_send_color(uint8_t r8, uint8_t g8, uint8_t b8) {
    ssd1331_spi_send(q5(r8));
    ssd1331_spi_send(q6(g8));
    ssd1331_spi_send(q5(b8));
}

/* ---------------- Pixel streaming helpers (DC=1 only for pixel bytes) ---------------- */
static inline void ssd1331_stream_begin(void)       {
    uint32_t v = _pmod_state;
    v |= SSD1331_DC;
    v &= ~(SSD1331_CS_N);
    v &= ~(SSD1331_SCLK | SSD1331_MOSI);
    _pmod_write_state(v);
}
static inline void ssd1331_stream_byte(uint8_t b)   { ssd1331_spi_send(b); }
static inline void ssd1331_stream_end(void)         { ssd1331_cs_deassert(); }

/* ---------------- Address window (params with DC=0) ---------------- */
_fast static void ssd1331_set_addr_window(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    uint8_t x1 = x;
    uint8_t y1 = y;
    uint8_t x2 = (uint8_t)((x + w - 1 > 95) ? 95 : (x + w - 1));
    uint8_t y2 = (uint8_t)((y + h - 1 > 63) ? 63 : (y + h - 1));

    ssd1331_cmd2(SSD1331_SET_COLUMN_ADDR, x1, x2);
    ssd1331_cmd2(SSD1331_SET_ROW_ADDR,    y1, y2);
}

/* ---------------- Init / Deinit ---------------- */
static void ssd1331_init(void) {
    pmod_init();
    delay_ms(10);

    ssd1331_reset_assert();   delay_ms(20);
    ssd1331_reset_deassert(); delay_ms(20);

    pmod_clr(SSD1331_VCC_EN);

    ssd1331_cmd0(SSD1331_DISPLAY_OFF);
    ssd1331_cmd1(SSD1331_SET_COMMAND_LOCK, 0x12);
    ssd1331_cmd1(SSD1331_SET_COMMAND_LOCK, 0xB1);

    ssd1331_cmd1(SSD1331_SET_REMAP_COLOR_DEPTH,  0x72);
    ssd1331_cmd1(SSD1331_SET_DISPLAY_START_LINE, 0x00);
    ssd1331_cmd1(SSD1331_SET_DISPLAY_OFFSET,     0x00);
    ssd1331_cmd0(SSD1331_SET_DISPLAY_MODE_NORMAL);
    ssd1331_cmd1(SSD1331_SET_MULTIPLEX_RATIO,    0x3F);
    ssd1331_cmd1(SSD1331_SET_MASTER_CONFIG,      0x8E);
    ssd1331_cmd1(SSD1331_POWER_SAVE_MODE,        0x0B);
    ssd1331_cmd1(SSD1331_PHASE_PERIOD_ADJUST,    0x31);
    ssd1331_cmd1(SSD1331_DISPLAY_CLOCK_DIV_OSC,  0xF0);

    ssd1331_cmd1(SSD1331_SET_SECOND_PRECHARGE_A, 0x64);
    ssd1331_cmd1(SSD1331_SET_SECOND_PRECHARGE_B, 0x78);
    ssd1331_cmd1(SSD1331_SET_SECOND_PRECHARGE_C, 0x64);
    ssd1331_cmd1(SSD1331_SET_PRECHARGE_VOLTAGE,  0x3A);
    ssd1331_cmd1(SSD1331_SET_VCOMH,              0x3E);
    ssd1331_cmd1(SSD1331_MASTER_CURRENT_CONTROL, 0x0A);
    ssd1331_cmd1(SSD1331_SET_CONTRAST_A,         0x91);
    ssd1331_cmd1(SSD1331_SET_CONTRAST_B,         0x50);
    ssd1331_cmd1(SSD1331_SET_CONTRAST_C,         0x7D);

    ssd1331_cmd4(SSD1331_CLEAR_WINDOW, 0x00, 0x00, 0x5F, 0x3F);
    delay_ms(10);

    pmod_set(SSD1331_VCC_EN);
    delay_ms(100);
    ssd1331_cmd0(SSD1331_DISPLAY_ON);
    delay_ms(10);
}

static void ssd1331_shutdown(void) {
    ssd1331_cmd0(SSD1331_DISPLAY_OFF);
    pmod_clr(SSD1331_VCC_EN);
    delay_ms(100);
    pmod_clr(SSD1331_PMOD_EN);
}

/* ---------------- GAC Drawing Helpers ---------------- */
_fast static void fill_solid(uint8_t r, uint8_t g, uint8_t b) {
    ssd1331_cmd2(SSD1331_FILL_ENABLE, 0x01, 0x00);

    ssd1331_dc_cmd();
    ssd1331_cs_assert();
    ssd1331_spi_send(SSD1331_DRAW_RECT);
    ssd1331_spi_send(0);
    ssd1331_spi_send(0);
    ssd1331_spi_send(SSD1331_WIDTH - 1);
    ssd1331_spi_send(SSD1331_HEIGHT - 1);

    ssd1331_send_color(r, g, b);
    ssd1331_send_color(r, g, b);

    ssd1331_cs_deassert();
}

_fast static void draw_filled_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t g, uint8_t b) {
    ssd1331_cmd2(SSD1331_FILL_ENABLE, 0x01, 0x00);

    uint8_t x2 = (uint8_t)((x + w - 1 > (SSD1331_WIDTH - 1))
        ? (SSD1331_WIDTH - 1) : (x + w - 1));
    uint8_t y2 = (uint8_t)((y + h - 1 > (SSD1331_HEIGHT - 1))
        ? (SSD1331_HEIGHT - 1) : (y + h - 1));

    ssd1331_dc_cmd();
    ssd1331_cs_assert();
    ssd1331_spi_send(SSD1331_DRAW_RECT);
    ssd1331_spi_send(x);
    ssd1331_spi_send(y);
    ssd1331_spi_send(x2);
    ssd1331_spi_send(y2);

    ssd1331_send_color(r, g, b);
    ssd1331_send_color(r, g, b);

    ssd1331_cs_deassert();
}

static inline void draw_line_color(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, uint8_t g, uint8_t b) {
    ssd1331_dc_cmd();
    ssd1331_cs_assert();
    ssd1331_spi_send(SSD1331_DRAW_LINE);
    ssd1331_spi_send(x0);
    ssd1331_spi_send(y0);
    ssd1331_spi_send(x1);
    ssd1331_spi_send(y1);
    ssd1331_send_color(r, g, b);
    ssd1331_cs_deassert();
}
