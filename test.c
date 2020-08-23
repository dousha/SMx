#include <stdio.h>

#include "sm3.h"

void print_digest(uint32_t *digest) {
    for (uint8_t i = 0; i < 8; i++) {
        printf("%08x ", digest[i]);
    }
    putchar('\n');
}

int main() {
    char *msg = "abc";
    char *long_msg = "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd";
    uint32_t *digest = sm3_digest(msg, 3 * 8);
    print_digest(digest);
    digest = sm3_digest(long_msg, 64 * 8);
    print_digest(digest);
    return 0;
}
