extern char *out_buffer, *out_ptr;

extern void define_name(char *text);
void start_info(char *info, unsigned long rsize, unsigned long dsize);
extern void start_rsrc(void);
extern void start_data(void);
extern void end_file(void);
#ifdef SCAN
extern void do_idf();
#endif /* SCAN */
extern void do_mkdir(char *name, char *header);
extern void enddir(void);
extern char *get_mina(void);

