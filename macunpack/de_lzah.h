#ifndef DE_LZAH_H
#define DE_LZAH_H

extern void de_lzah(unsigned long obytes);
extern unsigned char (*lzah_getbyte)();
extern void de_lzh(long ibytes, long obytes, char **data, int bits);

#endif
