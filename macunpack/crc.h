#define INIT_CRC crcinit

#include "../crc/crc.h"

extern uint32_t crcinit;
extern uint32_t (*updcrc)(uint32_t icrc, unsigned char *icp, int32_t icnt);
