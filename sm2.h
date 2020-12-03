#ifndef SM2_H
#define SM2_H

#include "bigint.h"

typedef struct {
	bigint* x;
	bigint* y;
	uint8_t is_infinity;
} ec_point;

typedef struct {
	bigint* p;
	bigint* a;
	bigint* b;
	ec_point* g;
} ec_system;

extern void ec_point_add(ec_system *, ec_point *, ec_point *);

extern void ec_point_inverse(ec_point *);

extern void ec_point_double(ec_system *, ec_point *);

extern void ec_point_scalar_multiply(ec_system *, ec_point *, bigint *);

#endif
