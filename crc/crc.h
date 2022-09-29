#ifndef CRC_CRC_H
#define CRC_CRC_H

#include <stdint.h>

extern uint32_t arc_crcinit;
extern uint32_t binhex_crcinit;
extern uint32_t zip_crcinit;

extern uint32_t arc_updcrc(uint32_t icrc, unsigned char *icp, int32_t icnt);
extern uint32_t binhex_updcrc(uint32_t icrc, unsigned char *icp, int32_t icnt);
extern uint32_t zip_updcrc(uint32_t icrc, unsigned char *icp, int32_t icnt);

#endif