#define	HUFF_BE		0
#define	HUFF_LE		1

typedef struct node {
    int flag, byte;
    struct node *one, *zero;
} node;

extern int (*get_bit)(void);
extern void clrhuff(void);
int gethuffbyte(node *l_nodelist);
int getihuffbyte(void);

extern struct node nodelist[];
extern int bytesread;
