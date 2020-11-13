#ifndef SM_4_H
#define SM_4_H

#include <stdint.h>

extern void sm4_encrypt_ecb(const uint8_t *, uint64_t, const uint32_t *, uint8_t *);

extern void sm4_decrypt_ecb(const uint8_t *, uint64_t, const uint32_t *, uint8_t *);

extern void sm4_encrypt_cbc(const uint8_t *, uint64_t, const uint32_t *, const uint8_t *, uint8_t *);

extern void sm4_decrypt_cbc(const uint8_t *, uint64_t, const uint32_t *, const uint8_t *, uint8_t *);

extern void sm4_encrypt_cfb(const uint8_t *, uint64_t, const uint32_t *, const uint8_t *, uint8_t *);

extern void sm4_decrypt_cfb(const uint8_t *, uint64_t, const uint32_t *, const uint8_t *, uint8_t *);

#endif
