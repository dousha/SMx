#ifndef SM_COMMON_H
#define SM_COMMON_H

#include <stdint.h>
#include <stdbool.h>

extern uint32_t rol(uint32_t, uint32_t);

extern void copy_and_reverse_endianness(uint8_t *, const uint8_t *, uint64_t);

#endif
