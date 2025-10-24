#pragma once
#include <stdint.h>

#if defined(__ELF__) && (defined(__riscv) || defined(__riscv_xlen))
  #define _fast __attribute__((section(".fast"), noinline))
#else
  #define _fast
#endif

/* ========================= MMIO ========================= */
#define IO_BASE       0x00400000u
#define IO_LEDS       0x0004u
#define IO_SEG_ONE    0x0008u
#define IO_SEG_TWO    0x0010u
#define IO_PMOD       0x0020u
#define IO_SW         0x0040u

#define MMIO32(addr)  (*(volatile uint32_t*)(uintptr_t)(addr))
#define IO_IN(port)        MMIO32(IO_BASE + (port))
#define IO_OUT(port,val)   (MMIO32(IO_BASE + (port)) = (uint32_t)(val))

#define PIN_SW4    (1u << 0)
#define PIN_SW3    (1u << 1)
#define PIN_SW2    (1u << 2)
#define PIN_SW1    (1u << 3)

/* ========================= Timing ========================= */
#ifndef CPU_HZ
#  define CPU_HZ 25000000u
#endif

#ifndef DELAY_CYCLES_PER_ITER
#  define DELAY_CYCLES_PER_ITER 12u
#endif

#define DELAY_LOOPS_PER_MS ((CPU_HZ) / (DELAY_CYCLES_PER_ITER * 1000u))

_fast static void delay_ms(uint32_t ms) {
  volatile uint32_t t = ms * (uint32_t)DELAY_LOOPS_PER_MS;
  while (t--) __asm__ volatile ("" ::: "memory");
}

/* ========================= 7-segment displays ========================= */
static const uint8_t digit_map[16] = {
  0x01, 0x4F, 0x12, 0x06,
  0x4C, 0x24, 0x20, 0x0F,
  0x00, 0x04, 0x08, 0x60,
  0x31, 0x42, 0x30, 0x38
};
static inline uint32_t to_seg(uint32_t n) {
  return (n > 0xF) ? 0x7F : digit_map[n & 0xF];
}
