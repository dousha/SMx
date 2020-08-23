#ifndef SM_3_H
#define SM_3_H

#include <stdint.h>

extern void sm3_init();

extern uint32_t* sm3_digest(const uint8_t *, uint64_t);

#endif
