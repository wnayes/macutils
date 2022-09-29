#include "crc.h"

uint32_t crcinit;

uint32_t (*updcrc)(uint32_t icrc, unsigned char *icp, int32_t icnt);

