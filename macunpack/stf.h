#include "macunpack.h"
#ifdef STF

#include <stdint.h>

#define MAGIC	"RTH"

#define	S_MAGIC		0
#define	S_FLENGTH	3
#define	S_RSRCLNGTH	3	/* + NAMELENGTH */
#define	S_DATALNGTH	7	/* + NAMELENGTH */

typedef struct stf_fileHdr {
	char		magic[3];
	char		flength;
	char		fname[32];	/* actually flength */
	uint32_t	rsrcLength;
	uint32_t	dataLength;
} stf_fileHdr;

void stf (uint32_t ibytes);
#endif