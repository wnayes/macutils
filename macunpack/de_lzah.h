#ifndef DE_LZAH_H
#define DE_LZAH_H

#include <stdint.h>

extern void de_lzah(uint32_t obytes);
extern unsigned char (*lzah_getbyte)(void);
extern void de_lzh(int32_t ibytes, int32_t obytes, char **data, int bits);

#endif
