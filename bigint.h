#ifndef BIGINT_H
#define BIGINT_H

#include <stdint.h>

#define BIGINT_ACTUAL_SIZE 64

typedef struct {
	uint8_t mess[BIGINT_ACTUAL_SIZE]; // little endian
} bigint;

extern void bigint_init(bigint *);

extern void bigint_from_bytes(bigint *value, const uint8_t *data, size_t len);

extern void bigint_to_bytes(bigint *, uint8_t *);

extern void bigint_copy(bigint *, bigint *);

extern void bigint_from_value(bigint *, uint64_t);

extern uint8_t bigint_add(bigint *, bigint *);

extern uint8_t bigint_subtract(bigint *, bigint *);

extern uint8_t bigint_subtract_u8(bigint *, uint8_t);

extern void bigint_add_mod(bigint *, bigint *, bigint *);

extern void bigint_mod(bigint *, bigint *);

extern void bigint_power_mod(bigint *, bigint *, bigint *);

extern void bigint_negate(bigint *);

extern void bigint_multiply(bigint *, bigint *);

extern void bigint_multiply_mod(bigint *, bigint *, bigint *);

extern void bigint_divide(bigint *, bigint *);

extern void bigint_divide_mod_prime(bigint *a, bigint *b, bigint *m);

extern uint8_t bigint_inc(bigint *);

extern uint8_t bigint_dec(bigint *);

extern int8_t bigint_compare(bigint *, bigint *);

extern int8_t bigint_unsigned_compare(bigint *, bigint *);

extern uint8_t bigint_is_zero(bigint *);

extern uint8_t bigint_is_one(bigint *);

extern uint8_t bigint_is_negative(bigint *);

extern uint8_t bigint_excerpt_is_zero(bigint *, size_t);

extern uint8_t bigint_test_bit(bigint *, size_t);

extern uint8_t bigint_most_significant_1(bigint *);

extern void bigint_square(bigint *);

#endif
