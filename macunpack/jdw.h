#include <stdint.h>

#define	J_MAGIC		0
#define J_TYPE		6
#define J_AUTH		10
#define	J_FINFO		14
#define	J_DATALENGTH	22
#define J_RSRCLENGTH	26
#define	J_CTIME		30
#define	J_MTIME		34
#define	J_FLENGTH	38

typedef struct jdw_fileHdr {
	char		magic[6];
	uint32_t	type;
	uint32_t	auth;
	char		finfo[8];
	uint32_t	dataLength;
	uint32_t	rsrcLength;
	uint32_t	ctime;
	uint32_t	mtime;
	char		flength;
	char		fname[32];	/* actually flength */
} jdw_fileHdr;

