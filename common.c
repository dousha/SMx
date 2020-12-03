#include "common.h"

/*
uint32_t rol(uint32_t x, uint32_t r) {
    bool carry = false;
    uint32_t out = x;
    for (uint32_t i = 0; i < r; i++) {
        if (out & 0x80000000) {
            carry = true;
        }
        out <<= 1;
        if (carry) {
            carry = false;
            out |= 1;
        }
    }
    return out;
}*/

uint32_t rol(uint32_t x, uint32_t r) {
	return (x << r) | (x >> (32u - r));
}


void copy_and_reverse_endianness(uint8_t *to, const uint8_t *from, uint64_t length) {
	for (uint64_t i = 0; i < length; i++) {
		to[i] = from[length - i - 1];
	}
}
