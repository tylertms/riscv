#pragma once
#include <stdint.h>

#if defined(__ELF__) && (defined(__riscv) || defined(__riscv_xlen))
  #define _fast __attribute__((section(".fast"), noinline))
#else
  #define _fast
#endif


/* --- IO --- */
#define IO_BASE     0x400000u
#define IO_LEDS     0x0004u
#define IO_SEG_ONE  0x0008u
#define IO_SEG_TWO  0x0010u

#define IO_IN(port)       *(volatile uint32_t*)(IO_BASE + port)
#define IO_OUT(port,val)  *(volatile uint32_t*)(IO_BASE + port)=(val)

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

static inline uint32_t to_seg(uint32_t n) {
  return (n > 0xF) ? 0x7F : digit_map[n & 0xF];
}
