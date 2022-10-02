#include "macunpack.h"
#define DD_INTERNAL
#include "dd.h"
#ifdef DD
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "crc.h"
#include "cpt.h"
#include "de_compress.h"
#include "../fileio/machdr.h"
#include "../fileio/wrfile.h"
#include "../fileio/fileglob.h"
#include "../util/masks.h"
#include "../util/util.h"
#include "../util/transname.h"

static void dd_name(unsigned char *bin_hdr);
static int dd_filehdr(struct dd_fileHdr *f, struct dd_fileCHdr *cf, int skip);
static void dd_cfilehdr(struct dd_fileCHdr *f);
static int dd_valid(int dmethod, int rmethod);
static int dd_valid1(int method);
static char *dd_methname(int n);
static uint32_t dd_checksum(uint32_t init, char *buffer, uint32_t length);
static void dd_chksum(struct dd_fileHdr hdr, unsigned char *data);
static uint32_t dd_checkor(uint32_t init, char *buffer, uint32_t length);
static void dd_do_delta(char *out_ptr, uint32_t nbytes, int kind);
static void dd_delta(char *out_ptr, uint32_t nbytes);
static void dd_delta3(char *out_ptr, uint32_t nbytes);
static void dd_copy(struct dd_fileHdr hdr, unsigned char *data);
static void dd_copyfile(uint32_t obytes, unsigned char *data);
static void dd_expand(struct dd_fileCHdr hdr, unsigned char *data);
static void dd_expandfile(uint32_t obytes, uint32_t ibytes, int method, int kind, unsigned char *data, uint32_t chksum);
static void dd_nocomp(uint32_t obytes, unsigned char *data);
static void dd_lzc(uint32_t ibytes, uint32_t obytes, unsigned char *data, int mb, uint32_t chksum, uint32_t ckinit);
#ifdef UNTESTED
static void dd_rle(uint32_t ibytes, unsigned char *data);
#ifdef NOTIMPLEMENTED
static void dd_huffman(uint32_t ibytes, unsigned char *data);
#endif /* NOTIMPLEMENTED */
static void dd_lzss(unsigned char *data, uint32_t chksum);
static int dd_getbits(int n);
#endif /* UNTESTED */
static void dd_cpt_compat(uint32_t ibytes, uint32_t obytes, unsigned char *data, int sub_method, uint32_t chksum);

typedef struct methodinfo {
	char *name;
	int number;
} methodinfo;

static struct methodinfo methods[] = {
    {"NoComp",  nocomp},
    {"LZC",     lzc},
    {"???",	method2},
    {"RLE",     rle},
    {"Huffman", huffman},
    {"???",	method5},
    {"???",	method6},
    {"LZSS",	lzss},
    {"RLE/LZH",	cpt_compat},
    {"???",	method9},
};
static unsigned char *dd_archive;
static unsigned char *dd_data_ptr;
static int dd_archive_size;
static int dd_max_archive_size;
static unsigned char *dd_dirst;
static int dd_dirstptr;
static int dd_dirstmax;
static int dd_xor;
static int32_t dd_bitbuf;
static int dd_bitcount;
static unsigned char *dd_bitptr;
static char dd_LZbuff[2048];

void 
dd_file (unsigned char *bin_hdr)
{
    uint32_t data_size;
    int i;
    struct dd_fileCHdr cf;
    char ftype[5], fauth[5];

    updcrc = binhex_updcrc;
    crcinit = binhex_crcinit;
    dd_name(bin_hdr);
    for(i = 0; i < INFOBYTES; i++) {
	info[i] = bin_hdr[i];
    }
    transname(info + I_NAMEOFF + 1, text, (int)info[I_NAMEOFF] & BYTEMASK);
    data_size = get4(info + I_DLENOFF);
    if(data_size > dd_max_archive_size) {
	if(dd_max_archive_size == 0) {
	    dd_archive = (unsigned char *)malloc((unsigned)data_size);
	} else {
	    dd_archive = (unsigned char *)realloc((char *)dd_archive,
						(unsigned)data_size);
	}
	dd_max_archive_size = data_size;
	if(dd_archive == NULL) {
	    (void)fprintf(stderr, "Insufficient memory.\n");
	    exit(1);
	}
    }
    dd_archive_size = data_size;
    if(fread((char *)dd_archive, 1, (int)data_size, infp) != data_size) {
	(void)fprintf(stderr, "Premature EOF\n");
#ifdef SCAN
	do_error("Premature EOF");
#endif /* SCAN */
	exit(1);
    }
    dd_data_ptr = dd_archive;
    dd_cfilehdr(&cf);
    write_it = 1;
    if(list) {
	do_indent(indent);
	transname(info + I_TYPEOFF, ftype, 4);
	transname(info + I_AUTHOFF, fauth, 4);
	(void)fprintf(stderr,
		"name=\"%s\", type=%4.4s, author=%4.4s, data=%d, rsrc=%d",
		text, ftype, fauth,
		(int32_t)get4(info + I_DLENOFF), (int32_t)get4(info + I_RLENOFF));
	if(info_only) {
	    write_it = 0;
	}
	if(query) {
	    write_it = do_query();
	} else {
	    (void)fputc('\n', stderr);
	}
    }
    if(!dd_valid((int)cf.datamethod, (int)cf.rsrcmethod)) {
	(void)fprintf(stderr, "\tUnimplemented method found: %d %d\n",
		cf.datamethod, cf.rsrcmethod);
#ifdef SCAN
	do_error("macunpack: Unimplemented method found");
#endif /* SCAN */
	return;
    }

    if(write_it) {
	define_name(text);
    }
    if(write_it || list) {
	dd_expand(cf, dd_data_ptr);
    }
}

void 
dd_arch (unsigned char *bin_hdr)
{
    uint32_t data_size;
    uint32_t crc, filecrc;
    struct dd_fileHdr f;
    struct dd_fileCHdr cf;
    char locname[64];
    int i, nlength;

    updcrc = binhex_updcrc;
    crcinit = binhex_crcinit;
    data_size = get4((char *)bin_hdr + I_DLENOFF);
    if(data_size > dd_max_archive_size) {
	if(dd_max_archive_size == 0) {
	    dd_archive = (unsigned char *)malloc((unsigned)data_size);
	} else {
	    dd_archive = (unsigned char *)realloc((char *)dd_archive,
						(unsigned)data_size);
	}
	dd_max_archive_size = data_size;
    }
    dd_archive_size = data_size;
    if(fread((char *)dd_archive, 1, (int)data_size, infp) != data_size) {
	(void)fprintf(stderr, "Insufficient memory.\n");
	exit(1);
    }
    dd_name(bin_hdr);
    nlength = bin_hdr[I_NAMEOFF];
    for(i = 0; i < INFOBYTES; i++) {
	info[i] = 0;
    }
    info[I_NAMEOFF] = nlength;
    for(i = 1; i <= nlength; i++) {
	info[I_NAMEOFF + i] = bin_hdr[I_NAMEOFF + i];
    }
    transname(info + I_NAMEOFF + 1, text, nlength);
    (void)strcpy(locname, text);
    if(list) {
	do_indent(indent);
	(void)fprintf(stderr, "folder=\"%s\"", text);
	if(query) {
	    if(!do_query()) {
		return;
	    }
	} else {
	    (void)fputc('\n', stderr);
	}
	indent++;
    }
    if(!info_only) {
	do_mkdir(text, info);
    }

    if(strncmp((char *)dd_archive, "DDAR", 4)) {
	(void)fprintf(stderr, "Magic archive header error\n");
#ifdef SCAN
	do_error("macunpack: Magic archive header error");
#endif /* SCAN */
	exit(1);
    }
    crc = (*updcrc)(crcinit, dd_archive, ARCHHDRSIZE - 2);
    filecrc = get2((char *)dd_archive + ARCHHDRCRC);
    if(crc != filecrc) {
	(void)fprintf(stderr, "Header CRC mismatch: got 0x%02x, need 0x%02x\n",
		(int)crc, (int)filecrc);
#ifdef SCAN
	do_error("macunpack: Header CRC mismatch");
#endif /* SCAN */
	exit(1);
    }
    dd_data_ptr = dd_archive + ARCHHDRSIZE;
    while(dd_data_ptr < dd_archive + data_size) {
	switch(dd_filehdr(&f, &cf, dir_skip)) {
	case DD_FILE:
	    dd_chksum(f, dd_data_ptr);
	    dd_expand(cf, dd_data_ptr);
	case DD_IVAL:
	    dd_data_ptr += f.dataLength - CDD_FILEHDRSIZE;
	    break;
	case DD_COPY:
	    dd_copy(f, dd_data_ptr);
	    dd_data_ptr += f.dataLength + f.rsrcLength;
	    break;
	case DD_SDIR:
	    if(write_it || info_only) {
		if(write_it) {
		    do_mkdir(text, info);
		}
		if(dd_dirstptr == dd_dirstmax) {
		    if(dd_dirstmax == 0) {
			dd_dirst = (unsigned char *)malloc(64);
		    } else {
			dd_dirst = (unsigned char *)realloc((char *)dd_dirst,
						(unsigned)dd_dirstmax + 64);
		    }
		    dd_dirstmax += 64;
		}
		for(i = 0; i < 64; i++) {
		    dd_dirst[dd_dirstptr + i] = text[i];
		}
		dd_dirst += 64;
		indent++;
	    } else {
		dir_skip++;
	    }
	    break;
	case DD_EDIR:
	    if(dir_skip) {
		dir_skip--;
	    } else {
		dd_dirst -= 64;
		indent--;
		if(list) {
		    do_indent(indent);
		    (void)fprintf(stderr, "leaving folder \"%s\"\n",
			    dd_dirst + dd_dirstptr);
		}
		if(!info_only) {
		    enddir();
		}
	    }
	}
    }
    if(!info_only) {
	enddir();
    }
    if(list) {
	indent--;
	do_indent(indent);
	(void)fprintf(stderr, "leaving folder \"%s\"\n", locname);
    }
}

static void 
dd_name (unsigned char *bin_hdr)
{
    int nlength;
    unsigned char *extptr;

    nlength = bin_hdr[I_NAMEOFF] & BYTEMASK;
    extptr = bin_hdr + I_NAMEOFF + nlength - 3;
    if(!strncmp((char *)extptr, ".sea", 4) ||
       !strncmp((char *)extptr, ".Sea", 4) ||
       !strncmp((char *)extptr, ".SEA", 4)) {
	nlength -= 4;
	extptr[0] = 0;
	extptr[1] = 0;
	extptr[2] = 0;
	extptr[3] = 0;
	bin_hdr[I_NAMEOFF] = nlength;
	return;
    }
    extptr++;
    if(!strncmp((char *)extptr, ".dd", 3)) {
	nlength -=3;
	extptr[0] = 0;
	extptr[1] = 0;
	extptr[2] = 0;
	bin_hdr[I_NAMEOFF] = nlength;
	return;
    }
    if(nlength < 31) {
	nlength++;
    }
    bin_hdr[I_NAMEOFF + nlength] = 0xA5;
    bin_hdr[I_NAMEOFF] = nlength;
}

static int 
dd_filehdr (struct dd_fileHdr *f, struct dd_fileCHdr *cf, int skip)
{
    register int i;
    uint32_t crc;
    int n, to_uncompress;
    unsigned char *hdr;
    char ftype[5], fauth[5];
    uint32_t datalength, rsrclength;

    to_uncompress = DD_COPY;
    hdr = dd_data_ptr;
    dd_data_ptr += DD_FILEHDRSIZE;
    for(i = 0; i < INFOBYTES; i++) {
	info[i] = '\0';
    }
    crc = INIT_CRC;
    crc = (*updcrc)(crc, hdr, DD_FILEHDRSIZE - 2);

    f->hdrcrc = get2((char *)hdr + D_HDRCRC);
    if(f->hdrcrc != crc) {
	(void)fprintf(stderr, "Header CRC mismatch: got 0x%04x, need 0x%04x\n",
		f->hdrcrc & WORDMASK, (int)crc);
#ifdef SCAN
	do_error("macunpack: Header CRC mismatch");
#endif /* SCAN */
	exit(1);
    }

    n = hdr[D_FNAME] & BYTEMASK;
    if(n > F_NAMELEN) {
	n = F_NAMELEN;
    }
    info[I_NAMEOFF] = n;
    copy(info + I_NAMEOFF + 1, (char *)hdr + D_FNAME + 1, n);
    transname((char *)hdr + D_FNAME + 1, text, n);

    if(!hdr[D_ISDIR]) {
	f->datacrc = get2((char *)hdr + D_DATACRC);
	f->rsrccrc = get2((char *)hdr + D_RSRCCRC);
	f->dataLength = get4((char *)hdr + D_DATALENGTH);
	f->rsrcLength = get4((char *)hdr + D_RSRCLENGTH);
	copy(info + I_DLENOFF, (char *)hdr + D_DATALENGTH, 4);
	copy(info + I_RLENOFF, (char *)hdr + D_RSRCLENGTH, 4);
	copy(info + I_CTIMOFF, (char *)hdr + D_CTIME, 4);
	copy(info + I_MTIMOFF, (char *)hdr + D_MTIME, 4);
	copy(info + I_TYPEOFF, (char *)hdr + D_FTYPE, 4);
	copy(info + I_AUTHOFF, (char *)hdr + D_CREATOR, 4);
	copy(info + I_FLAGOFF, (char *)hdr + D_FNDRFLAGS, 2);
    }

    if(hdr[D_ISDIR]) {
	to_uncompress = DD_SDIR;
    } else if(hdr[D_ENDDIR]) {
	to_uncompress = DD_EDIR;
    } else if(!no_dd && ((hdr[D_FNDRFLAGS] & 0x80) == 0)) {
	dd_cfilehdr(cf);
	to_uncompress = DD_FILE;
	datalength = cf->dataLength;
	rsrclength = cf->rsrcLength;
    } else {
	datalength = f->dataLength;
	rsrclength = f->rsrcLength;
    }
    hdr[D_FNDRFLAGS] &= 0x7f;
    write_it = !skip;
    if(list && !skip) {
	if(to_uncompress != DD_EDIR) {
	    do_indent(indent);
	}
	if(to_uncompress == DD_SDIR) {
	    (void)fprintf(stderr, "folder=\"%s\"", text);
	} else if(to_uncompress != DD_EDIR) {
	    transname(info + I_TYPEOFF, ftype, 4);
	    transname(info + I_AUTHOFF, fauth, 4);
	    (void)fprintf(stderr,
		    "name=\"%s\", type=%4.4s, author=%4.4s, data=%d, rsrc=%d",
		    text, ftype, fauth, (int32_t)datalength, (int32_t)rsrclength);
	}
	if(info_only) {
	    write_it = 0;
	}
	if(to_uncompress != DD_EDIR) {
	    if(query) {
		write_it = do_query();
	    } else {
		(void)fputc('\n', stderr);
	    }
	}
	if(to_uncompress == DD_FILE) {
	    if(!dd_valid((int)cf->datamethod, (int)cf->rsrcmethod)) {
		(void)fprintf(stderr, "\tUnimplemented method found: %d %d\n",
		    cf->datamethod, cf->rsrcmethod);
#ifdef SCAN
		do_error("macunpack: Unimplemented method found");
#endif /* SCAN */
		return DD_IVAL;
	    }
	}
    }

    if(write_it) {
	define_name(text);
    }
    return to_uncompress;
}

static void 
dd_cfilehdr (struct dd_fileCHdr *f)
{
    uint32_t crc;
    unsigned char *hdr;

    hdr = dd_data_ptr;
    dd_data_ptr += CDD_FILEHDRSIZE;
    crc = INIT_CRC;
    crc = (*updcrc)(crc, hdr, CDD_FILEHDRSIZE - 2);

    f->hdrcrc = get2((char *)hdr + DD_C_HDRCRC);
    if(f->hdrcrc != crc) {
	(void)fprintf(stderr, "Header CRC mismatch: got 0x%04x, need 0x%04x\n",
		f->hdrcrc & WORDMASK, (int)crc);
#ifdef SCAN
	do_error("macunpack: Header CRC mismatch");
#endif /* SCAN */
	exit(1);
    }

    f->dataLength = get4((char *)hdr + C_DATALENGTH);
    f->dataCLength = get4((char *)hdr + C_DATACLENGTH);
    f->rsrcLength = get4((char *)hdr + C_RSRCLENGTH);
    f->rsrcCLength = get4((char *)hdr + C_RSRCCLENGTH);
    f->datamethod = hdr[C_DATAMETHOD];
    f->rsrcmethod = hdr[C_RSRCMETHOD];
    f->datacrc = get2((char *)hdr + C_DATACRC);
    f->rsrccrc = get2((char *)hdr + C_RSRCCRC);
    f->datainfo = get2((char *)hdr + C_DATAINFO);
    f->rsrcinfo = get2((char *)hdr + C_RSRCINFO);
    f->datacrc2 = get2((char *)hdr + C_DATACRC2);
    f->rsrccrc2 = get2((char *)hdr + C_RSRCCRC2);
    f->info1 = hdr[C_INFO1];
    f->info2 = hdr[C_INFO2];
    copy(info + I_DLENOFF, (char *)hdr + C_DATALENGTH, 4);
    copy(info + I_RLENOFF, (char *)hdr + C_RSRCLENGTH, 4);
    copy(info + I_CTIMOFF, (char *)hdr + C_CTIME, 4);
    copy(info + I_MTIMOFF, (char *)hdr + C_MTIME, 4);
    copy(info + I_TYPEOFF, (char *)hdr + C_FTYPE, 4);
    copy(info + I_AUTHOFF, (char *)hdr + C_CREATOR, 4);
    copy(info + I_FLAGOFF, (char *)hdr + C_FNDRFLAGS, 2);
    if(f->info1 >= 0x2a && (f->info2 & 0x80) == 0) {
	dd_xor = 0x5a;
    } else {
	dd_xor = 0;
    }
}

static int 
dd_valid (int dmethod, int rmethod)
{
    return dd_valid1(dmethod) | dd_valid1(rmethod);
}

static int 
dd_valid1 (int method)
{
    switch(method) {
    case nocomp:
    case lzc:
#ifdef UNTESTED
    case rle:
#ifdef NOTIMPLEMENTED
    case huffman:
#endif /* NOTIMPLEMENTED */
    case lzss:
#endif /* UNTESTED */
    case cpt_compat:
	return 1;
    }
    return 0;
}

static char *
dd_methname (int n)
{
int i, nmeths;
    nmeths = sizeof(methods) / sizeof(struct methodinfo);
    for(i = 0; i < nmeths; i++) {
	if(methods[i].number == n) {
	    return methods[i].name;
	}
    }
    return NULL;
}

static uint32_t 
dd_checksum (uint32_t init, char *buffer, uint32_t length)
{
    int i;
    uint32_t cks;

    cks = init;
    for(i = 0; i < length; i++) {
	cks += *buffer++ & BYTEMASK;
    }
    return cks & WORDMASK;
}

static void 
dd_chksum (struct dd_fileHdr hdr, unsigned char *data)
{
    uint32_t cks;

    if(write_it) {
	cks = dd_checksum(INIT_CRC, (char *)data - CDD_FILEHDRSIZE,
			  hdr.dataLength);
	if(hdr.datacrc != cks) {
	    (void)fprintf(stderr,
		"Checksum error on compressed file: need 0x%04x, got 0x%04x\n",
		hdr.datacrc, (int)cks);
#ifdef SCAN
	    do_error("macunpack: Checksum error on compressed file");
#endif /* SCAN */
	    exit(1);
	}
    }
}

static uint32_t 
dd_checkor (uint32_t init, char *buffer, uint32_t length)
{
    int i;
    uint32_t cks;

    cks = init;
    for(i = 0; i < length; i++) {
	cks ^= *buffer++ & BYTEMASK;
    }
    return cks & WORDMASK;
}

static void 
dd_do_delta (char *out_ptr, uint32_t nbytes, int kind)
{
    switch(kind) {
    case 0:
	break;
    case 1:
	dd_delta(out_ptr, nbytes);
	break;
    case 2:
	dd_delta3(out_ptr, nbytes);
	break;
    default:
	(void)fprintf(stderr, "Illegal kind value found: %d\n", kind);
#ifdef SCAN
	do_error("Illegal kind value found");
#endif /* SCAN */
	exit(1);
    }
}

static void 
dd_delta (char *out_ptr, uint32_t nbytes)
{
    int i, sum = 0;

    for(i = 0; i < nbytes; i++) {
	sum = (sum + *out_ptr) & BYTEMASK;
	*out_ptr++ = sum;
    }
}

static void 
dd_delta3 (char *out_ptr, uint32_t nbytes)
{
    int i, sum1 = 0, sum2 = 0, sum3 = 0;

    for(i = 0; i < nbytes; i += 3) {
	sum1 = (sum1 + *out_ptr) & BYTEMASK;
	*out_ptr++ = sum1;
	if(i < nbytes - 1) {
	    sum2 = (sum2 + *out_ptr) & BYTEMASK;
	    *out_ptr++ = sum2;
	    if(i < nbytes) {
		sum3 = (sum3 + *out_ptr) & BYTEMASK;
		*out_ptr++ = sum3;
	    }
	}
    }
}

/*---------------------------------------------------------------------------*/
/*	Archive only, no compression					     */
/*---------------------------------------------------------------------------*/
static void 
dd_copy (struct dd_fileHdr hdr, unsigned char *data)
{
    uint32_t cks;

    if(write_it) {
	start_info(info, hdr.rsrcLength, hdr.dataLength);
    }
    if(verbose) {
	(void)fprintf(stderr, "\tNo compression");
    }
    if(write_it) {
	start_data();
    }
    dd_copyfile(hdr.dataLength, data);
    data += hdr.dataLength;
    if(write_it) {
	cks = dd_checksum(INIT_CRC, out_buffer, hdr.dataLength);
	if(hdr.datacrc != cks) {
	    (void)fprintf(stderr,
		"Checksum error on data fork: need 0x%04x, got 0x%04x\n",
		hdr.datacrc, (int)cks);
#ifdef SCAN
	    do_error("macunpack: Checksum error on data fork");
#endif /* SCAN */
	    exit(1);
	}
    }
    if(write_it) {
	start_rsrc();
    }
    dd_copyfile(hdr.rsrcLength, data);
    data += hdr.rsrcLength;
    if(write_it) {
	cks = dd_checksum(INIT_CRC, out_buffer, hdr.rsrcLength);
	if(hdr.rsrccrc != cks) {
	    (void)fprintf(stderr,
		"Checksum error on resource fork: need 0x%04x, got 0x%04x\n",
		hdr.rsrccrc, (int)cks);
#ifdef SCAN
	    do_error("macunpack: Checksum error on resource fork");
#endif /* SCAN */
	    exit(1);
	}
	end_file();
    }
    if(verbose) {
	(void)fprintf(stderr, ".\n");
    }
}

static void 
dd_copyfile (uint32_t obytes, unsigned char *data)
{
    if(obytes == 0) {
	return;
    }
    if(write_it) {
	copy(out_ptr, (char *)data, (int)obytes);
    }
}

/*---------------------------------------------------------------------------*/
/*	Possible compression, and perhaps in an archive			     */
/*---------------------------------------------------------------------------*/
static void 
dd_expand (struct dd_fileCHdr hdr, unsigned char *data)
{
    uint32_t cks;
    char *out_buf;

    if(write_it) {
	start_info(info, hdr.rsrcLength, hdr.dataLength);
    }
    if(verbose) {
	(void)fprintf(stderr, "\tData: ");
    }
    if(write_it) {
	start_data();
    }
    out_buf = out_buffer;
    dd_expandfile(hdr.dataLength, hdr.dataCLength, (int)hdr.datamethod, 
	(int)hdr.datainfo, data, (uint32_t)hdr.datacrc);
    data += hdr.dataCLength;
    if(write_it) {
	if((hdr.info2 & 0x40) && (hdr.dataLength != 0)) {
	    cks = arc_updcrc(INIT_CRC, (unsigned char *)out_buf,
			     (int)hdr.dataLength);
	    if(cks != hdr.datacrc2) {
		(void)fprintf(stderr,
		    "Checksum error on data fork: need 0x%04x, got 0x%04x\n",
		    (int)hdr.datacrc2, (int)cks);
#ifdef SCAN
		do_error("macunpack: Checksum error on data fork");
#endif /* SCAN */
		exit(1);
	    }
	}
    }
    if(verbose) {
	(void)fprintf(stderr, ", Rsrc: ");
    }
    if(write_it) {
	start_rsrc();
    }
    out_buf = out_buffer;
    dd_expandfile(hdr.rsrcLength, hdr.rsrcCLength, (int)hdr.rsrcmethod,
	(int)hdr.rsrcinfo, data, (uint32_t)hdr.rsrccrc);
    data += hdr.rsrcCLength;
    if(write_it) {
	if((hdr.info2 & 0x40) && (hdr.rsrcLength != 0)) {
	    cks = arc_updcrc(INIT_CRC, (unsigned char *)out_buf,
			     (int)hdr.rsrcLength);
	    if(cks != hdr.rsrccrc2) {
		(void)fprintf(stderr,
		   "Checksum error on resource fork: need 0x%04x, got 0x%04x\n",
		    (int)hdr.rsrccrc2, (int)cks);
#ifdef SCAN
		do_error("macunpack: Checksum error on resource fork");
#endif /* SCAN */
		exit(1);
	    }
	}
	end_file();
    }
    if(verbose) {
	(void)fprintf(stderr, ".\n");
    }
}

static void 
dd_expandfile (uint32_t obytes, uint32_t ibytes, int method, int kind, unsigned char *data, uint32_t chksum)
{
    int sub_method, m1, m2;
    char *optr = out_ptr;
    uint32_t cksinit;

    if(obytes == 0) {
	if(verbose) {
	    (void)fprintf(stderr, "empty");
	}
	return;
    }
    switch(method & 0x7f) {
    case nocomp:
	if(verbose) {
	    (void)fprintf(stderr, "No compression");
	}
	if(write_it) {
	    dd_nocomp(obytes, data);
	}
	break;
    case lzc:
	m1 = (*data++ & BYTEMASK) ^ dd_xor;
	m2 = (*data++ & BYTEMASK) ^ dd_xor;
	sub_method = (*data++ & BYTEMASK) ^ dd_xor;
	cksinit = m1 + m2 + sub_method;
	sub_method = sub_method & 0x1f;
	if(verbose) {
	    (void)fprintf(stderr, "LZC(%d) compressed (%4.1f%%)",
		    sub_method, 100.0 * ibytes / obytes);
	}
	if(write_it) {
	    dd_lzc(ibytes - 3, obytes, data, sub_method, chksum, cksinit);
	}
	break;
#ifdef UNTESTED
    case rle:
	if(verbose) {
	    (void)fprintf(stderr, "RLE compressed (%4.1f%%)",
		    100.0 * ibytes / obytes);
	}
	if(write_it) {
	    dd_rle(ibytes, data);
	}
	break;
#ifdef NOTIMPLEMENTED
    case huffman:
	if(verbose) {
	    (void)fprintf(stderr, "Huffman compressed (%4.1f%%)",
		    100.0 * ibytes / obytes);
	}
	if(write_it) {
	    dd_huffman(ibytes, data);
	}
	break;
#endif /* NOTIMPLEMENTED */
    case lzss:
	if(verbose) {
	    (void)fprintf(stderr, "LZSS compressed (%4.1f%%)",
		    100.0 * ibytes / obytes);
	}
	if(write_it) {
	    dd_lzss(data, chksum);
	}
	break;
#endif /* UNTESTED */
    case cpt_compat:
	sub_method = get2((char *)data);
	data += 16;
	if(sub_method != 0) {
	    sub_method = 0;
	} else {
	    sub_method = 1;
	}
	if(verbose) {
	    if(!sub_method) {
		(void)fprintf(stderr, "RLE compressed (%4.1f%%)",
			100.0 * ibytes / obytes);
	    } else {
		(void)fprintf(stderr, "RLE/LZH compressed (%4.1f%%)",
			100.0 * ibytes / obytes);
	    }
	}
	if(write_it) {
	    dd_cpt_compat(ibytes, obytes, data, sub_method, chksum);
	}
	break;
    default:
	break;
    }
    if(write_it) {
	dd_do_delta(optr, obytes, kind);
    }
}

/*---------------------------------------------------------------------------*/
/*	Method 0: no compression					     */
/*---------------------------------------------------------------------------*/
static void 
dd_nocomp (uint32_t obytes, unsigned char *data)
{
    copy(out_ptr, (char *)data, (int)obytes);
}

/*---------------------------------------------------------------------------*/
/*	Method 1: LZC compressed					     */
/*---------------------------------------------------------------------------*/
static void 
dd_lzc (uint32_t ibytes, uint32_t obytes, unsigned char *data, int mb, uint32_t chksum, uint32_t ckinit)
{
    int i;
    char *out_buf;
    uint32_t cks;

    out_buf = out_buffer;
    core_compress((char *)data);
    de_compress(ibytes, mb);
    out_buffer = out_buf;
    if(dd_xor != 0) {
	for(i = 0; i < obytes; i++) {
	    *out_buf++ ^= dd_xor;
	}
    }
    cks = dd_checksum(ckinit, out_buffer, obytes);
    if(chksum != cks) {
	(void)fprintf(stderr,
	    "Checksum error on fork: need 0x%04x, got 0x%04x\n",
	    (int)chksum, (int)cks);
#ifdef SCAN
	do_error("macunpack: Checksum error on fork");
#endif /* SCAN */
	exit(1);
    }
}

#ifdef UNTESTED
/*---------------------------------------------------------------------------*/
/*	Method 3: Run length encoding					     */
/*---------------------------------------------------------------------------*/
static void 
dd_rle (uint32_t ibytes, unsigned char *data)
{
    int ch, lastch, n, i;

    while(ibytes != 0) {
	ch = *data++;
	ibytes--;
	if(ch == ESC) {
	    n = *data++ - 1;
	    ibytes--;
	    if(n < 0) {
		*out_ptr++ = ESC;
		lastch = ESC;
	    } else {
		for(i = 0; i < n; i++) {
		    *out_ptr++ = lastch;
		}
	    }
	} else {
	    *out_ptr++ = ch;
	    lastch = ch;
	}
    }
}

#ifdef NOTIMPLEMENTED
/*---------------------------------------------------------------------------*/
/*	Method 4: Huffman encoding					     */
/*---------------------------------------------------------------------------*/
static void 
dd_huffman (uint32_t ibytes, unsigned char *data)
{
}
#endif /* NOTIMPLEMENTED */

/*---------------------------------------------------------------------------*/
/*	Method 7: Slightly improved LZSS				     */
/*---------------------------------------------------------------------------*/
static void 
dd_lzss (unsigned char *data, uint32_t chksum)
{
    int i, LZptr, LZbptr, LZlength;
    char *optr = out_ptr;
    unsigned char cks;

    data += get4((char *)data + 6);
    LZptr = 0;
    while(1) {
	if(dd_getbits(1) == 0) {
	    *out_ptr++ = dd_LZbuff[LZptr++] = dd_getbits(8);
	    LZptr &= 0x7ff;
	} else {
	    if(dd_getbits(1) == 0) {
		LZbptr = dd_getbits(11);
	    } else {
		LZbptr = dd_getbits(7);
	    }
	    if(LZbptr == 0) {
		break;
	    }
	    LZbptr = (LZptr - LZbptr) & 0x7ff;
	    LZlength = dd_getbits(2);
	    if(LZlength == 3) {
		LZlength += dd_getbits(2);
		if(LZlength == 6) {
		    do {
			i = dd_getbits(4);
			LZlength += i;
		    } while(i == 15);
		}
	    }
	    LZlength += 2;
	    for(i = 0; i < LZlength; i++) {
		*out_ptr++ = dd_LZbuff[LZptr++] = dd_LZbuff[LZbptr++];
		LZptr &= 0x7ff;
		LZbptr &= 0x7ff;
	    }
	}
    }
    cks = dd_checkor(INIT_CRC, optr, (uint32_t)(out_ptr - optr));
    if(chksum != cks) {
	(void)fprintf(stderr,
	    "Checksum error on fork: need 0x%04x, got 0x%04x\n",
	    (int)chksum, (int)cks);
#ifdef SCAN
	do_error("macunpack: Checksum error on fork");
#endif /* SCAN */
	exit(1);
    }
}

static int 
dd_getbits (int n)
{
    int r;

    while(dd_bitcount < n) {
	dd_bitbuf = (dd_bitbuf << 8) | (~(*dd_bitptr++) & BYTEMASK);
	dd_bitcount += 8;
    }
    dd_bitcount -= n;
    r = (dd_bitbuf >> dd_bitcount);
    dd_bitbuf ^= (r << dd_bitcount);
    return r;
}

#endif /* UNTESTED */

/*---------------------------------------------------------------------------*/
/*	Method 8: Compactor compatible compression			     */
/*---------------------------------------------------------------------------*/
static void 
dd_cpt_compat (uint32_t ibytes, uint32_t obytes, unsigned char *data, int sub_method, uint32_t chksum)
{
    uint32_t cks;
    char *optr = out_buffer;

    cpt_wrfile1(data, ibytes, obytes, sub_method, (uint32_t)0x0fff0);
    cks = arc_updcrc(INIT_CRC, (unsigned char *)optr, (int)obytes);
    if(chksum != cks) {
	(void)fprintf(stderr,
	    "Checksum error on fork: need 0x%04x, got 0x%04x\n",
	    (int)chksum, (int)cks);
#ifdef SCAN
	do_error("macunpack: Checksum error on fork");
#endif /* SCAN */
	exit(1);
    }
}
#else /* DD */
int dd; /* keep lint and some compilers happy */
#endif /* DD */

