#include <stdio.h>

void do_error (char *string);

extern char info[];
extern char text[];

extern int list, verbose, info_only, query, write_it, indent, dir_skip, no_dd;
extern FILE *infp;

extern int in_data_size, in_rsrc_size, in_ds, in_rs, ds_skip, rs_skip;

