#include <stdio.h>
#include "bigint.h"

const uint8_t aValue[4] = {0x12, 0x34, 0x56, 0x78};
const uint8_t bValue[4] = {0x78, 0x56, 0x34, 0x12};

void print_bigint(bigint *v) {
	printf("0x");
	for (uint8_t i = 0; i < BIGINT_ACTUAL_SIZE; i++) {
		if (i % 4 == 0) {
			putchar(' ');
		}
		printf("%02x", v->mess[BIGINT_ACTUAL_SIZE - 1 - i]);
	}
	putchar('\n');
}

int main() {
	bigint a, b, m;
	bigint_from_value(&a, 0xff);
	print_bigint(&a);
	bigint_from_value(&b, 0xff);
	print_bigint(&b);
	bigint_from_value(&m, 19);
	bigint_add(&a, &b);
	print_bigint(&a); // = 0x01fe
	bigint_negate(&b);
	print_bigint(&b); // = 0xffff...fff01
	bigint_add(&a, &b);
	print_bigint(&a); // = 0xff
	bigint_from_bytes(&a, aValue, 4);
	bigint_from_bytes(&b, bValue, 4);
	print_bigint(&a);
	print_bigint(&b);
	bigint_multiply(&a, &b);
	print_bigint(&a); // = 0x088ea9d1_358e7470
	bigint_from_value(&a, 9);
	bigint_from_value(&b, 3);
	bigint_power_mod(&a, &b, &m);
	print_bigint(&a); // 9^3 % 19 = 7
	bigint_from_value(&a, -4);
	bigint_mod(&a, &m);
	print_bigint(&a); // -4 % 19 = 15 = 0x0f, by definition
	bigint_from_value(&a, 4);
	bigint_from_value(&b, -2);
	bigint_divide_mod_prime(&a, &b, &m);
	print_bigint(&a); // (4 / (-2)) % 19 = -2 % 19 = 17 = 0x11
	bigint_from_value(&a, 0x55555555);
	bigint_shift_left(&a);
	print_bigint(&a); // 0xaaaaaaaa
	bigint_shift_right(&a);
	print_bigint(&a); // 0x55555555
	return 0;
}
