#define	BITBUFSIZ	16

extern unsigned int bit_be_bitbuf;
extern char *bit_be_filestart;
extern int bit_be_inbytes;

extern void bit_be_fillbuf(int n);
extern unsigned int bit_be_getbits(int n);
extern void bit_be_init_getbits(void);

