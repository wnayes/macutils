#include <stdint.h>

extern char *out_buffer, *out_ptr;

extern void define_name(char *text);
void start_info(char *info, uint32_t rsize, uint32_t dsize);
extern void start_rsrc(void);
extern void start_data(void);
extern void end_file(void);
#ifdef SCAN
extern void do_idf(char *name, int kind);
#endif /* SCAN */
extern void do_mkdir(char *name, char *header);
extern void enddir(void);
extern char *get_mina(void);

