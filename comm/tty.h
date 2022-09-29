void cleanup(int sig);
void reset_tty (void);
void setup_tty (void);
int tgetrec(char *buf, int count, int timeout);
int tgetc (int timeout);
void tputc(int c);
void tputrec (char *buf, int count);