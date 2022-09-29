#include <stdint.h>

#define HEADERBYTES 48
#define MAGIC1	"\253\315\000\060"
#define MAGIC2	"\037\235"

#define C_DLENOFF	4
#define C_DLENOFFC	8
#define C_RLENOFF	12
#define C_RLENOFFC	16
#define C_MTIMOFF	24
#define C_CTIMOFF	28
#define C_TYPEOFF	32
#define C_AUTHOFF	36
#define C_FLAGOFF	40

typedef struct lzc_fileHdr {
	uint32_t	magic1;
	uint32_t	dataLength;
	uint32_t	dataCLength;
	uint32_t	rsrcLength;
	uint32_t	rsrcCLength;
	uint32_t	unknown1;
	uint32_t	mtime;
	uint32_t	ctime;
	uint32_t	filetype;
	uint32_t	fileauth;
	uint32_t	flag1;
	uint32_t	flag2;
} lzc_fileHdr;
