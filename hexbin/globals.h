#include <stdio.h>
#include <stdint.h>
#include <string.h>
#ifdef BSD
#include <strings.h>
#define search_last rindex
#else /* BSD */
#define search_last strrchr
#endif /* BSD */

extern char info[];
extern char trname[];

typedef struct macheader {
	char m_name[128];
	char m_type[4];
	char m_author[4];
	short m_flags;
	int32_t m_datalen;
	int32_t m_rsrclen;
	int32_t m_createtime;
	int32_t m_modifytime;
} macheader;

extern struct macheader mh;

extern int listmode;
extern int verbose;
extern int info_only;
extern int uneven_lines;
extern int to_read;
extern int was_macbin;

extern FILE *ifp;

extern void do_error(char *string);

