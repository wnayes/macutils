#define INITCRC binhex_crcinit

#include "../crc/crc.h"

extern uint32_t crc;

extern void comp_q_crc (register unsigned int c);
extern void comp_q_crc_n (register unsigned char *s, register unsigned char *e);
extern void verify_crc (uint32_t calc_crc, uint32_t file_crc);

