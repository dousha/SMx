#include <stdio.h>
#include "sm2.h"

ec_system system;
ec_point P, Q;
bigint p, a, b, px, py, qx, qy;

void print_point(ec_point *point) {
	if (point->is_infinity) {
		printf("x = infinity\ny = infinity\n");
		return;
	}
	printf("x = 0x");
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		if (i % 4 == 0) {
			putchar(' ');
		}
		printf("%02x", point->x->mess[BIGINT_ACTUAL_SIZE - 1 - i]);
	}
	putchar('\n');
	printf("y = 0x");
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		if (i % 4 == 0) {
			putchar(' ');
		}
		printf("%02x", point->y->mess[BIGINT_ACTUAL_SIZE - 1 - i]);
	}
	putchar('\n');
}

int main() {
	bigint_from_value(&p, 19);
	bigint_from_value(&a, 1);
	bigint_from_value(&b, 1);
	bigint_from_value(&px, 10);
	bigint_from_value(&py, 2);
	bigint_from_value(&qx, 9);
	bigint_from_value(&qy, 6);
	system.p = &p;
	system.a = &a;
	system.b = &b;
	system.g = NULL;
	P.is_infinity = 0;
	P.x = &px;
	P.y = &py;
	Q.is_infinity = 0;
	Q.x = &qx;
	Q.y = &qy;
	ec_point_add(&system, &P, &Q);
	print_point(&P);
	bigint_from_value(&px, 10);
	bigint_from_value(&py, 2);
	ec_point_double(&system, &P);
	print_point(&P);
	return 0;
}
