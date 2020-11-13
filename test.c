#include <stdio.h>
#include <string.h>

#include "sm3.h"
#include "sm4.h"

void print_digest(uint32_t *digest) {
    for (uint8_t i = 0; i < 8; i++) {
        printf("%08x ", digest[i]);
    }
    putchar('\n');
}

void print_bytes(uint8_t *b, uint64_t len) {
    for (uint64_t i = 0; i < len; i++) {
        printf("%02x ", b[i]);
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
	uint8_t sm4_msg[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32,
						 0x10};
	uint8_t sm4_out_buffer[16], sm4_decrypt_buffer[16];
	uint32_t sm4_key[] = {0x01234567, 0x89abcdef, 0xfedcba98, 0x76543210};
	sm4_encrypt_ecb(sm4_msg, 16, sm4_key, sm4_out_buffer);
	print_bytes(sm4_msg, 16);
	print_bytes((uint8_t *) sm4_key, 16);
	print_bytes(sm4_out_buffer, 16);
	sm4_decrypt_ecb(sm4_out_buffer, 16, sm4_key, sm4_decrypt_buffer);
	print_bytes(sm4_decrypt_buffer, 16);
	char *sm4_text_msg = "Hello, world! This message is encrypted with SM4!";
	uint32_t len = strlen(sm4_text_msg);
	printf("%d\n", len);
	uint8_t sm4_text_out_buffer[64] = {0}, sm4_text_decrypt_buffer[64] = {0};
	sm4_encrypt_ecb(sm4_text_msg, len, sm4_key, sm4_text_out_buffer);
	sm4_decrypt_ecb(sm4_text_out_buffer, 64, sm4_key, sm4_text_decrypt_buffer);
	print_bytes(sm4_text_msg, len);
	print_bytes(sm4_text_out_buffer, len + 1);
	print_bytes(sm4_text_decrypt_buffer, len + 1);
	printf("%s\n", sm4_text_decrypt_buffer);
	uint8_t sm4_text_cbc_out_buffer[64] = {0}, sm4_text_cbc_decrypt_buffer[64] = {0};
	sm4_encrypt_cbc(sm4_text_msg, len, sm4_key, sm4_msg, sm4_text_cbc_out_buffer);
	sm4_decrypt_cbc(sm4_text_cbc_out_buffer, 64, sm4_key, sm4_msg, sm4_text_cbc_decrypt_buffer);
	print_bytes(sm4_text_cbc_out_buffer, len + 1);
	print_bytes(sm4_text_cbc_decrypt_buffer, len + 1);
	// FIXME: CBC needs proper padding of message
	sm4_text_cbc_decrypt_buffer[len] = 0;
	printf("%s\n", sm4_text_cbc_decrypt_buffer);
	uint8_t sm4_text_cfb_out_buffer[64] = {0}, sm4_text_cfb_decrypt_buffer[64] = {0};
	sm4_encrypt_cfb(sm4_text_msg, len, sm4_key, sm4_msg, sm4_text_cfb_out_buffer);
	sm4_decrypt_cfb(sm4_text_cfb_out_buffer, 64, sm4_key, sm4_msg, sm4_text_cfb_decrypt_buffer);
	printf("%s\n", sm4_text_cfb_decrypt_buffer);
	return 0;
}

