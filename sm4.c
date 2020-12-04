#include "sm4.h"
#include "common.h"

#ifdef DEBUG
#include <stdio.h>
#endif

const static uint8_t sm4_lut[16][16] = {
	0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7, 0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05,
	0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3, 0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
	0x9c, 0x42, 0x50, 0xf4, 0x91, 0xef, 0x98, 0x7a, 0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
	0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95, 0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6,
	0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba, 0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8,
	0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b, 0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
	0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2, 0x25, 0x22, 0x7c, 0x3b, 0x01, 0x21, 0x78, 0x87,
	0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52, 0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4, 0xc8, 0x9e,
	0xea, 0xbf, 0x8a, 0xd2, 0x40, 0xc7, 0x38, 0xb5, 0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
	0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55, 0xad, 0x93, 0x32, 0x30, 0xf5, 0x8c, 0xb1, 0xe3,
	0x1d, 0xf6, 0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60, 0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f, 
	0xd5, 0xdb, 0x37, 0x45, 0xde, 0xfd, 0x8e, 0x2f, 0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51, 
	0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd, 0xbc, 0x7f, 0x11, 0xd9, 0x5c, 0x41, 0x1f, 0x10, 0x5a, 0xd8, 
	0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd, 0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0, 
	0x89, 0x69, 0x97, 0x4a, 0x0c, 0x96, 0x77, 0x7e, 0x65, 0xb9, 0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84, 
	0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20, 0x79, 0xee, 0x5f, 0x3e, 0xd7, 0xcb, 0x39, 0x48
};

const static uint32_t sm4_fk[4] = {
		0xa3b1bac6, 0x56aa3350, 0x677d9197, 0xb27022dc
};

const static uint32_t sm4_ck[32] = {
		0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
		0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
		0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
		0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
		0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
		0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
		0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
		0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279
};

static uint32_t sm4_rk[32] = {0};

static uint8_t rk_generated = 0;

uint8_t sm4_sbox(uint8_t x) {
	return sm4_lut[(x & 0xf0u) >> 4u][x & 0x0fu];
}

uint8_t sm4_get_byte(const uint8_t *data, uint64_t length, uint64_t offset) {
	if (offset >= length) {
		return 0;
	} else {
		return data[offset];
	}
}

uint32_t sm4_assemble_word(const uint8_t *data, uint64_t length, uint64_t offset) {
	uint32_t out = 0;
	uint8_t *needle = (uint8_t *) &out;
	for (uint8_t i = 0; i < 4; i++) {
		needle[3 - i] = sm4_get_byte(data, length, offset + i);
	}
	return out;
}

void sm4_write_word(const uint32_t x, uint8_t *out) {
	out[0] = (x & 0xff000000u) >> 24u;
	out[1] = (x & 0x00ff0000u) >> 16u;
	out[2] = (x & 0x0000ff00u) >> 8u;
	out[3] = (x & 0x000000ffu);
}

uint32_t sm4_tau(uint32_t x) {
	uint8_t buf[4] = { 0 };
	sm4_write_word(x, buf);
	for (uint8_t i = 0; i < 4; i++) {
		buf[i] = sm4_sbox(buf[i]);
	}
	return sm4_assemble_word(buf, 4, 0);
}

uint32_t sm4_l(uint32_t x) {
	return x ^ rol(x, 2) ^ rol(x, 10) ^ rol(x, 18) ^ rol(x, 24);
}

uint32_t sm4_l_prime(uint32_t x) {
	return x ^ rol(x, 13) ^ rol(x, 23);
}

uint32_t sm4_t(uint32_t x) {
	return sm4_l(sm4_tau(x));
}

uint32_t sm4_t_prime(uint32_t x) {
	return sm4_l_prime(sm4_tau(x));
}

uint32_t sm4_f(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t rk) {
	return a ^ sm4_t(b ^ c ^ d ^ rk);
}

uint32_t sm4_get_round_key(uint32_t *k, uint8_t round) {
	uint32_t out = 0;
	out = k[0] ^ sm4_t_prime(k[1] ^ k[2] ^ k[3] ^ sm4_ck[round]);
	for (uint32_t i = 0; i < 3; i++) {
		k[i] = k[i + 1];
	}
	k[3] = out;
	return out;
}

void sm4_populate_rk_table(const uint32_t key[4]) {
	if (rk_generated) {
		return;
	}
	uint32_t k[4] = {key[0] ^ sm4_fk[0], key[1] ^ sm4_fk[1], key[2] ^ sm4_fk[2], key[3] ^ sm4_fk[3]};
	for (uint8_t i = 0; i < 32; i++) {
		sm4_rk[i] = sm4_get_round_key(k, i);
#ifdef DEBUG
		printf("rk[%02d] = %08x\n", i, sm4_rk[i]);
#endif
	}
	rk_generated = 1;
}

void
sm4_encrypt_block_ecb(const uint8_t data[16], uint64_t actualLength, const uint32_t key[4], uint8_t *output) {
	uint32_t next, cipher[4];
	sm4_populate_rk_table(key);
	for (uint8_t i = 0; i < 4; i++) {
		cipher[i] = sm4_assemble_word(data, actualLength, 4 * i);
	}
	for (uint8_t i = 0; i < 32; i++) {
		next = sm4_f(cipher[0], cipher[1], cipher[2], cipher[3], sm4_rk[i]);
		for (uint8_t j = 0; j < 3; j++) {
			cipher[j] = cipher[j + 1];
		}
		cipher[3] = next;
	}
	for (uint8_t i = 0; i < 4; i++) {
		sm4_write_word(cipher[3 - i], output + 4 * i);
	}
}

void
sm4_decrypt_block_ecb(const uint8_t data[16], uint64_t actualLength, const uint32_t key[4], uint8_t *output) {
	uint32_t next, cipher[4];
	sm4_populate_rk_table(key);
	for (uint8_t i = 0; i < 4; i++) {
		cipher[i] = sm4_assemble_word(data, actualLength, 4 * i);
	}
	for (uint8_t i = 0; i < 32; i++) {
		next = sm4_f(cipher[0], cipher[1], cipher[2], cipher[3], sm4_rk[31 - i]);
		for (uint8_t j = 0; j < 3; j++) {
			cipher[j] = cipher[j + 1];
		}
		cipher[3] = next;
	}
	for (uint8_t i = 0; i < 4; i++) {
		sm4_write_word(cipher[3 - i], output + 4 * i);
	}
}

void sm4_encrypt_ecb(const uint8_t *data, uint64_t length, const uint32_t *key, uint8_t *output) {
	uint64_t offset = 0;
	sm4_populate_rk_table(key);
	while (offset < length) {
		sm4_encrypt_block_ecb(data + offset, length, key, output + offset);
		offset += 16;
	}
}

void sm4_decrypt_ecb(const uint8_t *data, uint64_t length, const uint32_t *key, uint8_t *output) {
	uint64_t offset = 0;
	sm4_populate_rk_table(key);
	while (offset < length) {
		sm4_decrypt_block_ecb(data + offset, length, key, output + offset);
		offset += 16;
	}
}

void sm4_encrypt_cbc(const uint8_t *data, uint64_t length, const uint32_t *key, const uint8_t *iv, uint8_t *output) {
	uint8_t buf[16], vec[16];
	uint64_t offset = 0;
	for (uint8_t i = 0; i < 16; i++) {
		vec[i] = iv[i];
	}
	while (offset < length) {
		for (uint8_t i = 0; i < 16; i++) {
			buf[i] = sm4_get_byte(data, length, offset + i);
			buf[i] ^= vec[i];
		}
		sm4_encrypt_block_ecb(buf, length - offset > 16 ? 16 : length - offset, key, vec);
		for (uint8_t i = 0; i < 16; i++) {
			output[offset + i] = vec[i];
		}
		offset += 16;
	}
}

void sm4_decrypt_cbc(const uint8_t *data, uint64_t length, const uint32_t *key, const uint8_t *iv, uint8_t *output) {
	uint8_t buf[16], vec[16];
	uint64_t offset = 0;
	for (uint8_t i = 0; i < 16; i++) {
		vec[i] = iv[i];
	}
	while (offset < length) {
		sm4_decrypt_block_ecb(data + offset, length - offset > 16 ? 16 : length - offset, key, buf);
		for (uint8_t i = 0; i < 16; i++) {
			output[offset + i] = buf[i] ^ vec[i];
			vec[i] = sm4_get_byte(data, length, offset + i);
		}
		offset += 16;
	}
}

void sm4_encrypt_cfb(const uint8_t *data, uint64_t length, const uint32_t *key, const uint8_t *iv, uint8_t *output) {
	uint8_t buf[16], vec[16], byte;
	uint64_t offset = 0;
	for (uint8_t i = 0; i < 16; i++) {
		vec[i] = iv[i];
	}
	while (offset < length) {
		sm4_encrypt_block_ecb(vec, 16, key, buf);
		for (uint8_t i = 0; i < 16; i++) {
			byte = buf[i] ^ sm4_get_byte(data, length, offset + i);
			output[offset + i] = byte;
			vec[i] = byte;
		}
		offset += 16;
	}
}

void sm4_decrypt_cfb(const uint8_t *data, uint64_t length, const uint32_t *key, const uint8_t *iv, uint8_t *output) {
	uint8_t buf[16], vec[16], byte;
	uint64_t offset = 0;
	for (uint8_t i = 0; i < 16; i++) {
		vec[i] = iv[i];
	}
	while (offset < length) {
		sm4_encrypt_block_ecb(vec, 16, key, buf); // yes, encrypt
		for (uint8_t i = 0; i < 16; i++) {
			output[offset + i] = buf[i] ^ sm4_get_byte(data, length, offset + i);
			vec[i] = sm4_get_byte(data, length, offset + i);
		}
		offset += 16;
	}
}
