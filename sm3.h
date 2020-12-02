#ifndef SM_3_H
#define SM_3_H

#include <stdint.h>

extern uint32_t *sm3_digest(const uint8_t *, uint64_t);

extern void sm3_init();

extern void sm3_update(const uint8_t *, uint64_t, uint64_t);

extern uint32_t *sm3_finalize();

extern uint64_t sm3_get_padded_byte_length(uint64_t);

#endif
