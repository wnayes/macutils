#include "printhdr.h"
#include "globals.h"
#include "../util/transname.h"

/* print out header information in human-readable format */
void print_header0(int skip)
{
	if (listmode) {
		(void)fprintf(stderr, "name=\"%s\", ", trname);
		if (skip) {
			(void)fprintf(stderr, "\n");
		}
    }
}

/* print out header information in human-readable format */
void print_header1(int skip1, int skip2)
{
	char ftype[5], fauth[5];

	transname(mh.m_type, ftype, 4);
	transname(mh.m_author, fauth, 4);
	if (listmode) {
		if (skip1) {
			(void)fprintf(stderr, "\t");
		}
		(void)fprintf(stderr, "type=%4.4s, author=%4.4s, ", ftype, fauth);
		if (skip2) {
			(void)fprintf(stderr, "\n");
		}
    }
}

/* print out header information in human-readable format */
void print_header2(int skip)
{
	if (listmode) {
		if (skip) {
			(void)fprintf(stderr, "\t");
		}
		(void)fprintf(stderr, "data=%d, rsrc=%d\n",
			mh.m_datalen, mh.m_rsrclen);
    }
}
