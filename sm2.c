#include "sm2.h"

static bigint lambda, bufX, bufY, constant;

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
void ec_point_add(ec_system *sys, ec_point *p, ec_point *q) {
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
		bigint_multiply_mod(&bufX, &constant, sys->p);
		bigint_add_mod(&bufX, sys->a, sys->p);
		bigint_double(&bufY);
		bigint_mod(&bufY, sys->p);
		bigint_divide_mod_prime(&bufX, &bufY, sys->p);
		bigint_copy(&lambda, &bufX);
	} else {
		bigint_copy(&bufY, q->y);
		bigint_copy(&bufX, q->x);
		bigint_subtract(&bufY, p->y);
		bigint_subtract(&bufX, p->x);
		bigint_divide_mod_prime(&bufY, &bufX, sys->p);
		bigint_copy(&lambda, &bufY);
	}
	bigint_copy(&bufX, &lambda);
	bigint_copy(&bufY, &lambda);
	bigint_square(&bufX);
	bigint_subtract(&bufX, p->x);
	bigint_subtract(&bufX, q->x);
	bigint_mod(&bufX, sys->p);
	bigint_copy(&constant, p->x); // since we need to use x_1 later, the name become misleading now
	bigint_copy(p->x, &bufX); // x_3 calculated
	bigint_negate(&bufX);
	bigint_add_mod(&bufX, &constant, sys->p);
	bigint_multiply_mod(&bufX, &lambda, sys->p);
	bigint_subtract(&bufX, p->y);
	bigint_copy(p->y, &bufX); // y_3 calculated
	bigint_mod(p->x, sys->p);
	bigint_mod(p->y, sys->p); // bring things back
}

void ec_point_inverse(ec_point *p) {
	bigint_negate(p->y);
}

void ec_point_double(ec_system *sys, ec_point *p) {
	ec_point_add(sys, p, p);
}

void ec_point_scalar_multiply(ec_system *sys, ec_point *p, bigint *k) {
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
