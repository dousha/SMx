#include "bigint.h"
#include <string.h>

#ifdef USE_SHARED_BUFFER
static bigint buf, buf2;
#endif

void bigint_init(bigint *value) {
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		value->mess[i] = 0;
	}
}

void bigint_from_bytes(bigint *value, const uint8_t *data, size_t len) {
	memset(value->mess, 0, BIGINT_ACTUAL_SIZE);
	memcpy(value->mess, data, len);
}

void bigint_to_bytes(bigint *value, uint8_t *out) {
	memcpy(out, value->mess, BIGINT_ACTUAL_SIZE);
}

void bigint_copy(bigint *to, const bigint *from) {
	memcpy(to->mess, from->mess, BIGINT_ACTUAL_SIZE);
}

void bigint_from_value(bigint *value, uint64_t number) {
	bigint_init(value);
	for (uint8_t i = 0; i < 8; i++) {
		// copy the value in little endian manner
		value->mess[i] = ((number >> (i * 8u)) & 0xffu);
	}
	if (number & 0x8000000000000000u) {
		// expand negative part
		for (uint8_t i = 8; i < BIGINT_ACTUAL_SIZE; i++) {
			value->mess[i] = 0xffu;
		}
	}
}

uint8_t bigint_add(bigint *a, bigint *b) {
	uint16_t t;
	uint8_t carry = 0;
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		t = a->mess[i];
		t += b->mess[i];
		t += carry;
		carry = t > 0xffu ? 1 : 0;
		a->mess[i] = (uint8_t) (t & 0xffu);
	}
	return carry;
}

uint8_t bigint_subtract(bigint *a, bigint *b) {
	uint16_t t;
	uint8_t borrow = 0;
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		t = a->mess[i];
		t -= b->mess[i];
		t -= borrow;
		borrow = t > a->mess[i] ? 1 : 0;
		a->mess[i] = (uint8_t) (t & 0xffu);
	}
	return borrow;
}

uint8_t bigint_subtract_u8(bigint *v, uint8_t n) {
	uint16_t t;
	uint8_t borrow = n;
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		t = v->mess[i];
		t -= borrow;
		borrow = t > v->mess[i] ? 1 : 0;
		v->mess[i] = (uint8_t) (t & 0xffu);
		if (!borrow) {
			break;
		}
	}
	return borrow;
}

/**
 * LSB      MSB
 * [a a a a a]
 *     [b b b] < byteOffset = 2
 * result truncated
 * @param a
 * @param b
 * @param byteOffset
 */
void bigint_shifted_add(bigint *a, bigint *b, uint8_t byteOffset) {
	uint16_t t;
	uint8_t carry = 0;
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE - byteOffset; i++) {
		t = a->mess[i + byteOffset];
		t += b->mess[i];
		t += carry;
		carry = (t >> 8u);
		a->mess[i + byteOffset] = (uint8_t) (t & 0xffu);
	}
}

void bigint_multiply(bigint *a, bigint *b) {
#ifndef USE_SHARED_BUFFER
	bigint buf, buf2;
#endif
	bigint_init(&buf);
	bigint_init(&buf2);
	uint16_t t;
	uint8_t carry = 0;
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		uint8_t currentByte = b->mess[i];
		for (uint8_t j = 0; j < BIGINT_ACTUAL_SIZE; j++) {
			t = a->mess[j];
			t *= currentByte;
			t += carry;
			carry = (t >> 8u);
			buf.mess[j] = (uint8_t) (t & 0xffu);
		}
		bigint_shifted_add(&buf2, &buf, i);
	}
	memcpy(a->mess, buf2.mess, BIGINT_ACTUAL_SIZE);
}

void bigint_add_mod(bigint *a, bigint *b, bigint *m) {
#ifndef USE_SHARED_BUFFER
	bigint buf;
#endif
	memcpy(buf.mess, b->mess, 64);
	bigint_mod(&buf, m);
	bigint_mod(a, m);
	bigint_add(a, &buf);
	bigint_mod(a, m);
}

void bigint_mod(bigint *v, bigint *m) {
	if (bigint_is_zero(m)) {
		return;
	}
	if (bigint_is_one(m)) {
		bigint_init(v);
		return;
	}
	int8_t compare = bigint_unsigned_compare(v, m);
	if (compare < 0) {
		return;
	} else if (compare == 0) {
		bigint_init(v);
	} else {
		uint8_t sign = bigint_is_negative(v);
		if (sign) {
			bigint_negate(v);
		}
		// FIXME: need better performance here
		while(bigint_compare(v, m) > 0) {
			bigint_subtract(v, m);
		}
		if (sign) {
			bigint_negate(v);
			bigint_add(v, m);
		}
	}
}

void bigint_multiply_mod(bigint *a, bigint *b, bigint *m) {
#ifndef USE_SHARED_BUFFER
	bigint buf;
#endif
	bigint_copy(&buf, b);
	bigint_mod(&buf, m);
	bigint_mod(a, m);
	bigint_multiply(a, b);
	bigint_mod(a, m);
}

void bigint_power_mod(bigint *g, bigint *b, bigint *m) {
#ifndef USE_SHARED_BUFFER
	bigint buf;
#endif
	if (bigint_is_zero(m)) {
		return;
	}
	if (bigint_is_one(m)) {
		bigint_init(g);
		return;
	}
	bigint_mod(g, m);
	bigint_from_value(&buf, 1);
	uint8_t offset = 0;
	while (!bigint_excerpt_is_zero(b, offset)) {
		if (bigint_test_bit(b, offset)) {
			bigint_multiply(&buf, g);
			bigint_mod(&buf, m);
		}
		bigint_square(g);
		bigint_mod(g, m);
		++offset;
	}
	bigint_copy(g, &buf);
}

void bigint_negate(bigint *v) {
	if (bigint_is_zero(v)) {
		return;
	}
	bigint_dec(v);
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		v->mess[i] = ~(v->mess[i]);
	}
}

uint8_t bigint_inc(bigint *v) {
	uint16_t t = 0;
	uint8_t carry = 0;
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		t = v->mess[i];
		++t;
		carry = t > 0xffu ? 1 : 0;
		v->mess[i] = (uint8_t) (t & 0xffu);
	}
	return carry;
}

uint8_t bigint_dec(bigint *v) {
	uint8_t borrow = 1;
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		if (v->mess[i] == 0) {
			v->mess[i] = 0xffu;
			borrow = 1;
		} else {
			v->mess[i]--;
			borrow = 0;
			break;
		}
	}
	return borrow;
}

uint8_t bigint_is_negative(bigint *v) {
	return (v->mess[BIGINT_ACTUAL_SIZE - 1] & 0x80u) > 0;
}

int8_t bigint_compare(bigint *a, bigint *b) {
	if (a == b) {
		return 0;
	}
	uint8_t signOfA = bigint_is_negative(a);
	uint8_t signOfB = bigint_is_negative(b);
	if (signOfA != signOfB) {
		if (signOfA > 0 && signOfB == 0) {
			return -1;
		} else {
			return 1;
		}
	} else {
		return bigint_unsigned_compare(a, b);
	}
}

int8_t bigint_unsigned_compare(bigint *a, bigint *b) {
	for (uint8_t i = BIGINT_ACTUAL_SIZE; i > 0; i--) {
		if (a->mess[i - 1] > b->mess[i - 1]) {
			return 1;
		} else if (a->mess[i - 1] < b->mess[i - 1]) {
			return -1;
		}
	}
	return 0;
}

uint8_t bigint_is_zero(bigint *v) {
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		if (v->mess[i] != 0) {
			return 0;
		}
	}
	return 1;
}

uint8_t bigint_is_one(bigint *v) {
	if (v->mess[0] != 1) {
		return 0;
	}
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		if (v->mess[i] != 0) {
			return 0;
		}
	}
	return 1;
}

uint8_t bigint_excerpt_is_zero(bigint *v, size_t offset) {
	uint8_t byteOffset = offset / 8;
	uint8_t bitRemaining = offset % 8;
	if (bitRemaining != 0) {
		if ((v->mess[byteOffset] >> bitRemaining) > 0) {
			return 0;
		}
		++byteOffset;
	}
	for (; byteOffset < BIGINT_ACTUAL_SIZE; byteOffset++) {
		if (v->mess[byteOffset] > 0) {
			return 0;
		}
	}
	return 1;
}

uint8_t bigint_test_bit(const bigint *v, size_t offset) {
	uint8_t byteOffset = offset / 8;
	uint8_t bitRemaining = offset % 8;
	return (v->mess[byteOffset] & (1u << bitRemaining)) > 0;
}

size_t bigint_most_significant_1(const bigint *v) {
	size_t out = 0;
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		if (v->mess[i] > 0) {
			for (uint8_t j = 0; j < 8; j++) {
				if (((v->mess[i]) & (1u << j)) > 0) {
					out = ((size_t) i) * 8 + j;
				}
			}
		}
	}
	return out;
}

void bigint_square(bigint *v) {
#ifndef USE_SHARED_BUFFER
	bigint buf;
#endif
	bigint_copy(&buf, v);
	bigint_multiply(v, &buf);
}

void bigint_divide(bigint *a, bigint *b) {
#ifndef USE_SHARED_BUFFER
	bigint buf;
#endif
	bigint_init(&buf);
	uint8_t signOfA = bigint_is_negative(a);
	uint8_t signOfB = bigint_is_negative(b);
	if (signOfA) {
		bigint_negate(a);
	}
	if (signOfB) {
		bigint_negate(b);
	}
	// FIXME: need better performance here
	while(bigint_compare(a, b) > 0) {
		bigint_subtract(a, b);
		bigint_inc(&buf);
	}
	if (signOfA != signOfB) {
		bigint_negate(&buf);
	}
	bigint_copy(a, &buf);
}

void bigint_divide_mod_prime(bigint *a, bigint *b, bigint *m) {
#ifndef USE_SHARED_BUFFER
	bigint buf, buf2;
#endif
	if (bigint_is_zero(b) || bigint_is_zero(m)) {
		return;
	}
	if (bigint_is_one(m)) {
		bigint_init(a);
		return;
	}
	bigint_copy(&buf, b);
	bigint_copy(&buf2, m);
	bigint_mod(&buf, m);
	bigint_subtract_u8(&buf2, 2);
	bigint_power_mod(&buf, &buf2, m); // buf is the inverse of b
	bigint_multiply_mod(a, &buf, m);
}

uint8_t bigint_is_opposite(bigint *a, bigint *b) {
	if (a == b) {
		return 0;
	}
	uint8_t out;
	bigint_negate(a);
	out = bigint_compare(a, b);
	bigint_negate(a);
	if (out == 0) {
		out = 1;
	}
	return out;
}

uint8_t bigint_double(bigint *v) {
	uint8_t carry = 0;
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		carry = (v->mess[i] & 0x80u) > 0 ? 1 : 0;
		v->mess[i] <<= 1u;
		v->mess[i] |= carry;
	}
	return carry;
}
