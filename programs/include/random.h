// https://www.stix.id.au/wiki/Fast_8-bit_pseudorandom_number_generator
#include <stdint.h>
static uint8_t a, b, c, x;

static uint8_t rand8() {
  x++;
  a = (a ^ c) ^ x;
  b = b + a;
  c = (c + ((b >> 1) | (b << 7))) ^ a;
  return c;
}
