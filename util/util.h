#include <stdint.h>

typedef struct real_time {
	int year;
	int month;
	int day;
	int hours;
	int minutes;
	int seconds;
} real_time;

extern uint32_t get4(char *bp);
extern uint32_t get4i(char *bp);
extern uint32_t get2(char *bp);
extern uint32_t get2i(char *bp);
extern unsigned char getb(FILE *fp);
extern void copy(char *d, char *s, int n);
extern int do_query(void);
extern void put4(char *dest, uint32_t value);
extern void put2(char *dest, uint32_t value);
extern void do_indent(int indent);
extern real_time set_time(int year, int month, int day, int hours, int minutes, int seconds);
extern uint32_t tomactime(real_time time);
extern real_time frommactime(uint32_t accum);

