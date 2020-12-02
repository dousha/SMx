#include "sm3.h"
#include "common.h"

#include <string.h>
#ifdef DEBUG
#include <stdio.h>
extern void print_digest(uint32_t *digest);
#endif

static uint32_t reg[8] = {0}, next_reg[8] = {0};

static uint32_t w[68] = {0}, w_prime[64] = {0};

static uint32_t buf[16];

#ifdef DEBUG
static void print_w() {
    for (int i = 0; i < 68; i++) {
        printf("%08x ", w[i]);
    }
    putchar('\n');
}

static void print_w_prime() {
    for (int i = 0; i < 64; i++) {
        printf("%08x ", w_prime[i]);
    }
    putchar('\n');
}
#endif

void sm3_init() {
    reg[0] = 0x7380166f;
    reg[1] = 0x4914b2b9;
    reg[2] = 0x172442d7;
    reg[3] = 0xda8a0600;
    reg[4] = 0xa96f30bc;
    reg[5] = 0x163138aa;
    reg[6] = 0xe38dee4d;
    reg[7] = 0xb0fb0e4e;
    memset(next_reg, 0, 32);
    memset(w, 0, 4 * 68);
    memset(w_prime, 0, 4 * 64);
}

uint32_t sm3_t(uint8_t j) {
    return j < 16 ? 0x79cc4519 : 0x7a879d8a;
}

uint32_t sm3_ff(uint32_t x, uint32_t y, uint32_t z, uint8_t j) {
    return j < 16 ? x ^ y ^ z : (x & y) | (x & z) | (y & z);
}

uint32_t sm3_gg(uint32_t x, uint32_t y, uint32_t z, uint8_t j) {
    return j < 16 ? x ^ y ^ z : (x & y) | ((~x) & z);
}

uint32_t sm3_p0(uint32_t x) {
    return x ^ rol(x, 9) ^ rol(x, 17);
}

uint32_t sm3_p1(uint32_t x) {
    return x ^ rol(x, 15) ^ rol(x, 23);
}

void sm3_pad(const uint32_t *ws) {
    for (uint8_t i = 0; i < 16; i++) {
        w[i] = ws[i];
    }
		for (uint8_t i = 16; i < 68; i++) {
			w[i] = sm3_p1(w[i - 16] ^ w[i - 9] ^ rol(w[i - 3], 15)) ^ rol(w[i - 13], 7) ^ w[i - 6];
    }
    for (uint8_t i = 0; i < 64; i++) {
        w_prime[i] = w[i] ^ w[i + 4];
    }
}

/**
 * @param [in, out] v registers
 * @param [in] b 512bit (64bytes)
 */
void sm3_cf(uint32_t *v, uint32_t *b) {
    uint32_t ss1, ss2, tt1, tt2;
    for (uint8_t i = 0; i < 8; i++) {
        next_reg[i] = v[i];
    }
    for (uint8_t j = 0; j < 64; j++) {
        ss1 = rol(rol(next_reg[0], 12) + next_reg[4] + rol(sm3_t(j), j), 7);
        ss2 = ss1 ^ rol(next_reg[0], 12);
        tt1 = sm3_ff(next_reg[0], next_reg[1], next_reg[2], j) + next_reg[3] + ss2 + w_prime[j];
        tt2 = sm3_gg(next_reg[4], next_reg[5], next_reg[6], j) + next_reg[7] + ss1 + w[j];
        next_reg[3] = next_reg[2];
        next_reg[2] = rol(next_reg[1], 9);
        next_reg[1] = next_reg[0];
        next_reg[0] = tt1;
        next_reg[7] = next_reg[6];
        next_reg[6] = rol(next_reg[5], 19);
        next_reg[5] = next_reg[4];
        next_reg[4] = sm3_p0(tt2);
        #ifdef DEBUG
        printf("%2d ", j);
        print_digest(next_reg);
        #endif
    }
    for (uint8_t i = 0; i < 8; i++) {
        v[i] ^= next_reg[i];
    }
}

void sm3_process_buffer() {
    sm3_pad(buf);
    #ifdef DEBUG
    printf("w:\n");
    print_w();
    printf("w':\n");
    print_w_prime();
    #endif
    sm3_cf(reg, buf);
}

inline uint64_t sm3_get_padded_byte_length(uint64_t bitLength) {
	return ((bitLength + 512) / 512) * 64;
}

uint8_t sm3_get_relative_padded_byte(const uint8_t *bytes, uint64_t accumulatedByteLength, uint64_t bitLength, uint8_t relativeOffset) {
	const uint64_t paddedByteLength = sm3_get_padded_byte_length(bitLength);
	if (accumulatedByteLength + relativeOffset < bitLength / 8) {
		return bytes[relativeOffset];
	} else {
		if (accumulatedByteLength + relativeOffset == bitLength / 8) {
			return 0x80;
		} else {
			if (paddedByteLength - (accumulatedByteLength + relativeOffset) <= 4) {
				switch (paddedByteLength - (accumulatedByteLength + relativeOffset)) {
					case 4:
						return (bitLength & 0xff000000u) >> 24u;
					case 3:
						return (bitLength & 0x00ff0000u) >> 16u;
					case 2:
						return (bitLength & 0x0000ff00u) >> 8u;
					case 1:
						return (bitLength & 0x000000ffu);
					default:
						return 0;
				}
			} else {
				return 0x00;
			}
		}
	}
}

void sm3_update(const uint8_t *bytes, uint64_t accumulatedByteLength, uint64_t bitLength) {
	uint8_t *needle = (uint8_t *) buf;
	uint8_t offset = 0;
	for (uint8_t i = 0; i < 16; i++) {
		needle[4 * i + 3] = sm3_get_relative_padded_byte(bytes, accumulatedByteLength, bitLength, offset);
		needle[4 * i + 2] = sm3_get_relative_padded_byte(bytes, accumulatedByteLength, bitLength, offset + 1);
		needle[4 * i + 1] = sm3_get_relative_padded_byte(bytes, accumulatedByteLength, bitLength, offset + 2);
		needle[4 * i + 0] = sm3_get_relative_padded_byte(bytes, accumulatedByteLength, bitLength, offset + 3);
		offset += 4;
	}
	sm3_process_buffer();
}

uint32_t* sm3_finalize() {
	return reg;
}

uint32_t* sm3_digest(const uint8_t *bytes, uint64_t bitLength) {
    // padding of the message
    const uint64_t targetByteLength = sm3_get_padded_byte_length(bitLength);
    uint64_t accumulatedByteLength = 0;
    uint8_t *probe = (uint8_t *) bytes;
    sm3_init();
    while (accumulatedByteLength < targetByteLength) {
		sm3_update(probe + accumulatedByteLength, accumulatedByteLength, bitLength);
		accumulatedByteLength += 64;
    }
    return sm3_finalize();
}
