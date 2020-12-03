#ifndef SM2_H
#define SM2_H

#include "bigint.h"

typedef struct {
	bigint *x;
	bigint *y;
	uint8_t is_infinity;
} ec_point;

typedef struct {
	bigint *p;
	bigint *a;
	bigint *b;
	bigint *n;
	ec_point *g;
} ec_system;

typedef struct {
	bigint *secret;
	ec_point *pubkey;
} ec_keypair;

extern void ec_point_add(const ec_system *, ec_point *, const ec_point *);

extern void ec_point_inverse(ec_point *);

extern void ec_point_double(const ec_system *, ec_point *);

extern void ec_point_scalar_multiply(const ec_system *, ec_point *, const bigint *);

extern void sm2_keygen(const ec_system *sys, const bigint *d, ec_keypair *out);

extern uint32_t *sm2_identity_hash(const ec_system *sys, const ec_keypair *key, const uint8_t *id, uint16_t idBitLength);

extern void sm2_sign(const ec_system *sys, const ec_keypair *key, const uint32_t *hash, const uint8_t *data, uint64_t length, uint8_t *out);

extern uint8_t ec_verify(const ec_system *, const ec_point *, const uint8_t *, uint64_t);

#endif
