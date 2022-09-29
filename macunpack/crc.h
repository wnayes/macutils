#define INIT_CRC crcinit

extern unsigned long arc_crcinit;
extern unsigned long binhex_crcinit;
extern unsigned long zip_crcinit;

extern unsigned long arc_updcrc(unsigned long icrc, unsigned char *icp, int icnt);
extern unsigned long binhex_updcrc(unsigned long icrc, unsigned char *icp, int icnt);
extern unsigned long zip_updcrc(unsigned long icrc, unsigned char *icp, int icnt);

extern unsigned long crcinit;
extern unsigned long (*updcrc)();

