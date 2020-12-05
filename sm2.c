#include "sm2.h"
#include "sm3.h"
#include "common.h"
#include <string.h>

static bigint lambda, bufX, bufY, constant;
static uint8_t buf[32];
static bigint m256;

void ec_init(const ec_system *sys) {
	bigint_init(&m256);
	m256.mess[32] = 1;
	bigint_mod(&m256, sys->p);
}

void ec_set_m256(const uint8_t *mess, size_t length) {
	bigint_from_bytes(&m256, mess, length);
}

/**
 * $ x_3 = \lambda{}^{2} - x_1 - x_2 $
 * $ y_3 = \lambda{} \times (x_1 - x_3) - y_1 $
 * $ \lambda = \frac{y_2 - y_1}{x_2 - x_1} $
 * $ \lambda = \frac{3x_{1}^{2} + a}{2y_1} $
 *
 * @param sys Elliptic Curve System
 * @param p P point, will be overwritten by P + Q
 * @param q Q point, will preserve
 */
void ec_point_add(const ec_system *sys, ec_point *p, const ec_point *q) {
	if (p->is_infinity) {
		if (q->is_infinity) {
			return; // 0 + 0 = 0
		} else {
			p->is_infinity = 0;
			bigint_copy(p->x, q->x);
			bigint_copy(p->y, q->y);
			return; // 0 + P = P + 0 = P
		}
	}
	bigint_init(&lambda);
	if (bigint_compare(p->x, q->x) == 0) {
		if (bigint_is_opposite(p->y, q->y)) {
			p->is_infinity = 1;
			return;
		}
		bigint_from_value(&constant, 3);
		bigint_copy(&bufX, q->x);
		bigint_copy(&bufY, q->y);
		bigint_square(&bufX);
#ifdef USE_ACCELERATED_ALGORITHM
		bigint_multiply_mod_accelerated(&bufX, &constant, sys->p, &m256);
#else
		bigint_multiply_mod(&bufX, &constant, sys->p);
#endif
		bigint_add(&bufX, sys->a);
#ifdef USE_ACCELERATED_ALGORITHM
		bigint_mod_accelerated(&bufX, sys->p, &m256);
#else
		bigint_mod(&bufX, sys->p);
#endif
		bigint_double(&bufY);
#ifdef USE_ACCELERATED_ALGORITHM
		bigint_mod_accelerated(&bufY, sys->p, &m256);
		bigint_divide_mod_prime_accelerated(&bufX, &bufY, sys->p, &m256);
#else
		bigint_mod(&bufY, sys->p);
		bigint_divide_mod_prime(&bufX, &bufY, sys->p);
#endif
		bigint_copy(&lambda, &bufX);
	} else {
		bigint_copy(&bufY, q->y);
		bigint_copy(&bufX, q->x);
		bigint_subtract(&bufY, p->y);
		bigint_subtract(&bufX, p->x);
#ifdef USE_ACCELERATED_ALGORITHM
		bigint_divide_mod_prime_accelerated(&bufY, &bufX, sys->p, &m256);
#else
		bigint_divide_mod_prime(&bufY, &bufX, sys->p);
#endif
		bigint_copy(&lambda, &bufY);
	}
	bigint_copy(&bufX, &lambda);
	bigint_copy(&bufY, &lambda);
	bigint_square(&bufX);
	bigint_subtract(&bufX, p->x);
	bigint_subtract(&bufX, q->x);
#ifdef USE_ACCELERATED_ALGORITHM
	bigint_mod_accelerated(&bufX, sys->p, &m256);
#else
	bigint_mod(&bufX, sys->p);
#endif
	bigint_copy(&constant, p->x); // since we need to use x_1 later, the name become misleading now
	bigint_copy(p->x, &bufX); // x_3 calculated
	bigint_negate(&bufX);
	bigint_add(&bufX, &constant);
#ifdef USE_ACCELERATED_ALGORITHM
	bigint_mod_accelerated(&bufX, sys->p, &m256);
	bigint_multiply_mod_accelerated(&bufX, &lambda, sys->p, &m256);
#else
	bigint_mod(&bufX, sys->p);
	bigint_multiply_mod(&bufX, &lambda, sys->p);
#endif
	bigint_subtract(&bufX, p->y);
	bigint_copy(p->y, &bufX); // y_3 calculated
#ifdef USE_ACCELERATED_ALGORITHM
	bigint_mod_accelerated(p->x, sys->p, &m256);
	bigint_mod_accelerated(p->y, sys->p, &m256); // bring things back
#else
	bigint_mod(p->x, sys->p);
	bigint_mod(p->y, sys->p); // bring things back
#endif
}

void ec_point_inverse(ec_point *p) {
	bigint_negate(p->y);
}

void ec_point_double(const ec_system *sys, ec_point *p) {
	ec_point_add(sys, p, p);
}

void ec_point_scalar_multiply(const ec_system *sys, ec_point *p, const bigint *k) {
	bigint x, y;
	ec_point o;
	o.is_infinity = 1;
	o.x = &x;
	o.y = &y;
	size_t offset = bigint_most_significant_1(k);
	while (offset > 0) {
		ec_point_double(sys, &o);
		if (bigint_test_bit(k, offset)) {
			ec_point_add(sys, &o, p);
		}
		--offset;
	}
	ec_point_double(sys, &o);
	if (bigint_test_bit(k, offset)) {
		ec_point_add(sys, &o, p);
	}
	bigint_copy(p->x, &x);
	bigint_copy(p->y, &y);
}

void sm2_keygen(const ec_system *sys, const bigint *d, ec_keypair *out) {
	bigint_copy(out->secret, d);
	bigint_copy(out->pubkey->x, sys->g->x);
	bigint_copy(out->pubkey->y, sys->g->y);
	ec_point_scalar_multiply(sys, out->pubkey, d);
}

uint32_t *sm2_identity_hash(const ec_system *sys, const ec_keypair *key, const uint8_t *id, uint16_t idBitLength) {
	sm3_init();
	// FIXME: not applicable if p is not 256bits long
	// FIXME: sm3_update requires input of fixed 64 bytes long
	uint64_t totalBitLength = idBitLength, accumulatedLength = 0;
	totalBitLength *= 8; // convert to bit length
	totalBitLength += 2 * 8; // 2byte id length length
	totalBitLength += 6 * 32 * 8; // 6 parameters
	copy_and_reverse_endianness(buf, (uint8_t *) &idBitLength, 2);
	sm3_update(buf, accumulatedLength, totalBitLength);
	accumulatedLength += 2;
	sm3_update(id, accumulatedLength, totalBitLength);
	accumulatedLength += idBitLength;
	copy_and_reverse_endianness(buf, sys->a->mess, 32);
	sm3_update(buf, accumulatedLength, totalBitLength);
	accumulatedLength += 32;
	copy_and_reverse_endianness(buf, sys->b->mess, 32);
	sm3_update(buf, accumulatedLength, totalBitLength);
	accumulatedLength += 32;
	copy_and_reverse_endianness(buf, sys->g->x->mess, 32);
	sm3_update(buf, accumulatedLength, totalBitLength);
	accumulatedLength += 32;
	copy_and_reverse_endianness(buf, sys->g->y->mess, 32);
	sm3_update(buf, accumulatedLength, totalBitLength);
	accumulatedLength += 32;
	copy_and_reverse_endianness(buf, key->pubkey->x->mess, 32);
	sm3_update(buf, accumulatedLength, totalBitLength);
	accumulatedLength += 32;
	copy_and_reverse_endianness(buf, key->pubkey->y->mess, 32);
	sm3_update(buf, accumulatedLength, totalBitLength);
	return sm3_finalize();
}

void sm2_sign(const ec_system *sys, const ec_keypair *key, const uint32_t *hash, const uint8_t *data, uint64_t length, uint8_t *out) {
	sm3_init();
	uint64_t totalBitLength = length * 8 + 32 * 8;
	// TODO
}
