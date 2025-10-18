#pragma once
#include <stdint.h>

__asm__(
  ".option norvc\n"
  ".section .text\n"
  ".globl start\n"
  "start:\n"
  "  lui   gp, 0x400\n"
  "  lui   sp, 0x2\n"
  "  addi  sp, sp, -2048\n"
  "  jal   ra, main\n"
  "  ecall\n"
  "1: j 1b\n"
);


/* --- IO --- */
#define IO_BASE     0x400000u
#define IO_LEDS     0x0004u
#define IO_SEG_ONE  0x0008u
#define IO_SEG_TWO  0x0010u

#define LEDS    (volatile uint32_t *)(IO_BASE + IO_LEDS)
#define SEG_ONE (volatile uint32_t *)(IO_BASE + IO_SEG_ONE)
#define SEG_TWO (volatile uint32_t *)(IO_BASE + IO_SEG_TWO)

/* --- timing --- */
#define DELAY_LOOPS_PER_MS  (1786u)   /* ~25MHz / 14cy / 1000ms */

static const uint8_t digit_map[16] = {
  0x01, 0x4F, 0x12, 0x06,
  0x4C, 0x24, 0x20, 0x0F,
  0x00, 0x04, 0x08, 0x60,
  0x31, 0x42, 0x30, 0x38
};

static inline void delay(uint32_t ms) {
  volatile uint32_t t = ms * DELAY_LOOPS_PER_MS;
  while (t--) __asm__ volatile ("" ::: "memory");
}

static inline void write_digit(volatile uint32_t *reg, uint32_t n) {
  uint32_t v = (n > 0xF) ? 0x7F : digit_map[n & 0xF];
  *reg = v;
}
