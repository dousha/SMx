#include <stdio.h>
#include "sm2.h"

ec_system system;
ec_point P, Q;
bigint p, a, b, px, py, qx, qy, t, n;
const uint8_t sysP[32] = {
		0xff, 0xff, 0xff, 0xfe,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
};
const uint8_t sysA[32] = {
		0xff, 0xff, 0xff, 0xfe,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xfc
};
const uint8_t sysB[32] = {
		0x28, 0xe9, 0xfa, 0x9e,
		0x9d, 0x9f, 0x5e, 0x34,
		0x4d, 0x5a, 0x9e, 0x4b,
		0xcf, 0x65, 0x09, 0xa7,
		0xf3, 0x97, 0x89, 0xf5,
		0x15, 0xab, 0x8f, 0x92,
		0xdd, 0xbc, 0xbd, 0x41,
		0x4d, 0x94, 0x0e, 0x93,
};
const uint8_t sysN[32] = {
		0xff, 0xff, 0xff, 0xfe,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0x72, 0x03, 0xdf, 0x6b,
		0x21, 0xc6, 0x05, 0x2b,
		0x53, 0xbb, 0xf4, 0x09,
		0x39, 0xd5, 0x41, 0x23,
};
const uint8_t sysGx[32] = {
		0x32, 0xc4, 0xae, 0x2c,
		0x1f, 0x19, 0x81, 0x19,
		0x5f, 0x99, 0x04, 0x46,
		0x6a, 0x39, 0xc9, 0x94,
		0x8f, 0xe3, 0x0b, 0xbf,
		0xf2, 0x66, 0x0b, 0xe1,
		0x71, 0x5a, 0x45, 0x89,
		0x33, 0x4c, 0x74, 0xc7,
};
const uint8_t sysGy[32] = {
		0xbc, 0x37, 0x36, 0xa2,
		0xf4, 0xf6, 0x77, 0x9c,
		0x59, 0xbd, 0xce, 0xe3,
		0x6b, 0x69, 0x21, 0x53,
		0xd0, 0xa9, 0x87, 0x7c,
		0xc6, 0x2a, 0x47, 0x40,
		0x02, 0xdf, 0x32, 0xe5,
		0x21, 0x39, 0xf0, 0xa0,
};
const uint8_t m256[32] = {
		0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00,
};
const uint8_t m256_19[1] = {16};

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
	system.n = &n;
	system.g = NULL;
	P.is_infinity = 0;
	P.x = &px;
	P.y = &py;
	Q.is_infinity = 0;
	Q.x = &qx;
	Q.y = &qy;
	ec_set_m256(m256_19, 1);
	ec_point_add(&system, &P, &Q);
	print_point(&P);
	bigint_from_value(&px, 10);
	bigint_from_value(&py, 2);
	ec_point_double(&system, &P);
	print_point(&P);
	ec_point_double(&system, &P);
	bigint_from_value(&qx, 10);
	bigint_from_value(&qy, 2);
	bigint_from_value(&t, 4);
	ec_point_scalar_multiply(&system, &Q, &t);
	print_point(&P);
	print_point(&Q);
	printf("---\n");
	// performance test
	bigint_from_big_endian_bytes(&p, sysP, 32);
	bigint_from_big_endian_bytes(&a, sysA, 32);
	bigint_from_big_endian_bytes(&b, sysB, 32);
	bigint_from_big_endian_bytes(&n, sysN, 32);
	bigint_from_big_endian_bytes(&px, sysGx, 32);
	bigint_from_big_endian_bytes(&py, sysGy, 32);
	bigint_from_big_endian_bytes(&qx, sysGx, 32);
	bigint_from_big_endian_bytes(&qy, sysGy, 32);
	print_point(&P);
	ec_set_m256(m256, 32);
	ec_point_scalar_multiply(&system, &P, system.n);
	print_point(&P);
	return 0;
}
