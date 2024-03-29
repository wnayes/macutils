#include "machdr.h"

#define	ISATEND		0
#define	ISFILE		1
#define	ISDIR		2
#define	ENDDIR		3

extern char file_info[INFOBYTES];
extern char *data_fork, *rsrc_fork;
extern int data_size, rsrc_size;

extern void setup(int argc, char **argv);
extern int nextfile(void);
extern char *get_minb(void);
