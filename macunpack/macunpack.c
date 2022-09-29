#include "macunpack.h"
#include <unistd.h>
#include "globals.h"
#include "../util/patchlevel.h"
#include "../fileio/wrfile.h"
#include "../fileio/wrfileopt.h"
#include "../fileio/kind.h"
#include "../util/util.h"

#define LOCALOPT	"ilvqVH"

extern char *strcat();
#ifdef STF
extern void stf();
#endif /* STF */
#ifdef PIT
extern void pit();
#endif /* PIT */
#ifdef SIT
extern void sit();
#endif /* SIT */
#ifdef CPT
extern void cpt();
#endif /* CPT */
void macbinary();

static void usage();

static char options[128];

int main(int argc, char **argv)
{
    int c;
    extern int optind;
    extern char *optarg;
    int errflg;

    set_wrfileopt(0);
    (void)strcat(options, get_wrfileopt());
    (void)strcat(options, LOCALOPT);
    errflg = 0;

    while((c = getopt(argc, argv, options)) != EOF) {
#ifdef SCAN
	if(c == 'S') {
	    no_dd++;
	}
#endif /* SCAN */
	if(!wrfileopt((char)c)) {
	    switch(c) {
	    case 'l':
		list++;
		break;
	    case 'q':
		query++;
		break;
	    case 'v':
		verbose++;
		break;
	    case 'i':
		info_only++;
		break;
	    case '?':
		errflg++;
		break;
	    case 'H':
		give_wrfileopt();
		fprintf(stderr, "Macunpack specific options:\n");
		fprintf(stderr,
			"-i:\tgive information only, do not unpack\n");
		fprintf(stderr, "-l:\tgive listing\n");
		fprintf(stderr, "-v:\tgive verbose listing\n");
		fprintf(stderr,
			"-q:\tquery for every file/folder before unpacking\n");
		fprintf(stderr,
			"-V:\tgive information about this version\n");
		fprintf(stderr, "-H:\tthis message\n");
		fprintf(stderr, "Default is silent unpacking\n");
		exit(0);
	    case 'V':
		fprintf(stderr, "Version %s, ", VERSION);
		fprintf(stderr, "patchlevel %d", PATCHLEVEL);
		fprintf(stderr, "%s.\n", get_mina());
		fprintf(stderr, "Archive/file types recognized:\n");
#ifdef BIN
		fprintf(stderr,
			"\tBinHex 5.0, MacBinary 1.0 and UMCP (with caveat)\n");
#endif /* BIN */
#ifdef JDW
		fprintf(stderr, "\tCompress It\n");
#endif /* JDW */
#ifdef STF
		fprintf(stderr, "\tShrinkToFit\n");
#endif /* STF */
#ifdef LZC
		fprintf(stderr, "\tMacCompress\n");
#endif /* LZC */
#ifdef ASQ
		fprintf(stderr, "\tAutoSqueeze\n");
#endif /* ASQ */
#ifdef ARC
		fprintf(stderr, "\tArcMac\n");
#endif /* ARC */
#ifdef PIT
		fprintf(stderr, "\tPackIt\n");
#endif /* PIT */
#ifdef SIT
		fprintf(stderr, "\tStuffIt and StuffIt Deluxe\n");
#endif /* SIT */
#ifdef DIA
		fprintf(stderr, "\tDiamond\n");
#endif /* DIA */
#ifdef CPT
		fprintf(stderr, "\tCompactor\n");
#endif /* CPT */
#ifdef ZMA
		fprintf(stderr, "\tZoom\n");
#endif /* ZMA */
#ifdef LZH
		fprintf(stderr, "\tMacLHa\n");
#endif /* LZH */
#ifdef DD
		fprintf(stderr, "\tDiskDoubler and AutoDoubler\n");
#endif /* DD */
		exit(0);
	    }
	}
    }
    if(errflg) {
	usage();
	exit(1);
    }

    if(optind == argc) {
	infp = stdin;
    } else {
	if((infp = fopen(argv[optind], "r")) == NULL) {
	    fprintf(stderr,"Can't open input file \"%s\"\n",argv[optind]);
	    exit(1);
	}
#ifdef SCAN
	do_idf(argv[optind], UNIX_NAME);
#endif /* SCAN */
    }

    if(info_only || verbose || query) {
	list++;
    }
    c = getc(infp);
    (void)ungetc(c, infp);
    switch(c) {
    case 0:
	macbinary();
	break;
#ifdef STF
    case 'R':
	if(verbose) {
	    fprintf(stderr, "This is a \"ShrinkToFit\" packed file.\n");
	}
	stf(~(unsigned long)1);
	break;
#endif /* STF */
#ifdef PIT
    case 'P':
	if(verbose) {
	    fprintf(stderr, "This is a \"PackIt\" archive.\n");
	}
	pit();
	break;
#endif /* PIT */
#ifdef SIT
    case 'S':
	if(verbose) {
	    fprintf(stderr, "This is a \"StuffIt\" archive.\n");
	}
	sit();
	break;
#endif /* SIT */
#ifdef CPT
    case 1:
	if(verbose) {
	    fprintf(stderr, "This is a \"Compactor\" archive.\n");
	}
	cpt();
	break;
#endif /* CPT */
    default:
	fprintf(stderr, "Unrecognized archive type\n");
	exit(1);
    }
    exit(0);
    /* NOTREACHED */
}

static void 
usage (void)
{
    fprintf(stderr, "Usage: macunpack [-%s] [filename]\n", options);
    fprintf(stderr, "Use \"macunpack -H\" for help.\n");
}
