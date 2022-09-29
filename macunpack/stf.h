#include "macunpack.h"
#ifdef STF
#define MAGIC	"RTH"

#define	S_MAGIC		0
#define	S_FLENGTH	3
#define	S_RSRCLNGTH	3	/* + NAMELENGTH */
#define	S_DATALNGTH	7	/* + NAMELENGTH */

typedef struct stf_fileHdr {
	char		magic[3];
	char		flength;
	char		fname[32];	/* actually flength */
	unsigned long	rsrcLength;
	unsigned long	dataLength;
} stf_fileHdr;

void stf (unsigned long ibytes);
#endif