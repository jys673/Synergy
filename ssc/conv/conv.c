MDDefs.h                                                                                               666    2126      24         5565  5617444760   5435                                                                                                                                                                                                                                                                                                                                                                      /* @(#)MDDefs.h	1.35     7/26/94 */

#define	CALLOC(n, x)    	(x *) calloc(n, sizeof(x))
#define	RECALLOC(ptr, n, x) (ptr = (x *) realloc((void *) ptr,(n)*sizeof(x)))
#define FREE(ptr)			free((void *) ptr);	

#define     MAX_PATH_NAME_LEN   2048

#define 	UNSCALED	0
#define		SCALED		1
#define 	RIGHT		0
#define 	LEFT		1
#define 	DECREASE	0
#define 	INCREASE	1
#define 	MDOUT			0
#define 	MDIN		1
#define 	NO			0
#define 	YES			1
#define 	OFF			0
#define 	ON			1

#define		LINEAR		1
#define		CUBIC		2

#define		BLACK		-1
#define		WHITE		-2
#define		INVISABLE	-10

#define		EXCLUDED	-1
#define		COMBINED	0
#define		INCLUDED	1

#define     MDFALSE               0
#define     MDTRUE                1

#define		MDNoJump	-1

#define		MDSTATDEF(x)	(depend_data[x].stats_defined)
#define		MDDO(x)			(depend_data[x].data_offset)

#define		MDSTAT_SUM		(1L<<0)
#define		MDSTAT_NUM		(1L<<1)
#define		MDSTAT_MIN		(1L<<2)
#define		MDSTAT_MAX		(1L<<3)
#define		MDSTAT_SUMSQ	(1L<<4)

#define		MD0(x)	((MDSTATDEF(x)>>0) & 1L)	
#define		MD1(x)	(MD0(x)+((MDSTATDEF(x)>>1) & 1L))	
#define		MD2(x)	(MD1(x)+((MDSTATDEF(x)>>2) & 1L))
#define		MD3(x)	(MD2(x)+((MDSTATDEF(x)>>3) & 1L))	
#define		MD4(x)	(MD3(x)+((MDSTATDEF(x)>>4) & 1L))

#define		MDSum(x)	(MDSTATDEF(x) & MDSTAT_SUM ? MD0(x)-1+MDDO(x) : -1)
#define		MDNum(x)	(MDSTATDEF(x) & MDSTAT_NUM ? MD1(x)-1+MDDO(x) : -1)
#define		MDMin(x)	(MDSTATDEF(x) & MDSTAT_MIN ? MD2(x)-1+MDDO(x) : -1)	
#define		MDMax(x)	(MDSTATDEF(x) & MDSTAT_MAX ? MD3(x)-1+MDDO(x) : -1)
#define		MDSumSq(x)	(MDSTATDEF(x) & MDSTAT_SUMSQ ? MD4(x)-1+MDDO(x) : -1)

#define		MDDataSize	(MDDO(num_depend-1) + MD4(num_depend-1))

#define		MDSymbolPNo	4

#define		HORIZONTAL	0
#define		VERTICAL	1

#define		PT_PER_IN	72.0

#define		AXIS_RAMP	0
#define		AXIS_TEXT1	1
#define		AXIS_TEXT2	2
#define		AXIS_TEXT3	3
#define		AXIS_TEXT4	4
#define		AXIS_TEXT5	5
#define		AXIS_TEXT6	6

#define		RECTANGLE		1
#define		PARALLELOGRAM	2
#define		CIRCLE			3
#define		V_BARS			4
#define		H_BARS			5

#define		LEVEL_CELL_SCALE	0
#define		GLOBAL_CELL_SCALE	1

#define		MDCELL			0
#define		MDSYMBOL		1

#define		MD_II			0
#define		MD_IE			1
#define		MD_EI			2
#define		MD_EE			3

#define		NORM			0
#define		PERM			1

#define		MD_NO_RECT		0
#define		MD_DATA_RECT	1
#define		MD_HAXIS_RECT	2
#define		MD_VAXIS_RECT	3
#define		MD_WIGET_RECT	4
#define		MD_TITLE_RECT	5
#define		MD_GRAPH_RECT	6

#define		MDSUBSPACEIN	0
#define		MDSUBSPACEOUT	1
#define		MDANIMATE		2
#define		MDSUBSET		3
#define		MDRESTORESET	4
#define		MDDECIMATE		5
#define		MDUNDECIMATE	6
#define		MDCOARSESTGRAIN	7
#define		MDCOARSERGRAIN	8
#define		MDFINESTGRAIN	9
#define		MDSORTZOOMED	10
#define		MDSETUPORDER	11

#define		MDSYMBOLSON		0
#define		MDSYMBOLSOFF	1
#define		MDSELECTBINS	2
#define		MDRESETBINS		3

#define		MDSAVENEEDED	0
#define		MDSAVENOTNEEDED	1

#define		MDSolidFill		0
#define		MDThreeQFill	1
#define		MDTwoQFill		2
#define		MDOneQFill		3
#define		MDNoneFill 		4
                                                                                                                                           bconvert.c                                                                                             666    2126      24        22745  5650544151   6157                                                                                                                                                                                                                                                                                                                                                                      static char SccsId[] = "@(#)bconvert.c	1.17     8/30/94";

#include	<stdio.h>
#include	<string.h>

#ifdef WIN3
#include	<rpc/types.c>
#endif

#include	"bfile.h"
#include	"trans.h"

#define		MAXLABLEN		128
#define		MAXTYPELEN		256
#define		XDRINTSIZE		4
#define		XDRFLOATSIZE	4
#define		XDRDOUBLESIZE	8

#define		DELIMITED_FIELD	0
#define		FIXED_FIELD		1		

static int maxlen = 0;

static char *buffer = NULL; 
static char *tok = NULL;

struct type{
char type_name[MAXTYPELEN];
char format[MAXTYPELEN];
char delimiters[64];
char var_name[128];
int bfile_type;
int bfile_size;
int var_class;
int in_col, left_char, right_char;
double var_min, var_max;
} *field;

int file_type, line_length;
int *n_fields;
double *drow;

int dump_val = 0;

char *read_field();

main(argc, argv)
int argc;
char **argv;
{
	char *buf, *token;
	short *svalp, sval;
	int i, j, bindx;
	FILE *tfp, *afp;
	BFILE_HEADER *header;
	
	if (argc != 5) {
		fprintf(stderr, "Improper command line.\n");
		fprintf(stderr, "%s <-f | -c> <template file> <ASCII file> <binary file>.\n",
																	argv[0]);
		exit(1);
	}

	if ((tfp = fopen(argv[2], "r")) == NULL) {	
		fprintf(stderr, "%s: Can't open template file %s.\n", argv[0], argv[1]);
		exit(2);
	}

	if ((afp = fopen(argv[3], "r")) == NULL) {	
		fprintf(stderr, "%s: Can't open ASCII file %s.\n", argv[0], argv[2]);
		exit(3);
	}

#ifdef WIN3
	if ((bindx = bfile_open(argv[4], "w+b")) == BFILE_ERROR) {	
#else
	if ((bindx = bfile_open(argv[4], "w+")) == BFILE_ERROR) {	
#endif
		fprintf(stderr, "%s: Can't open binary file %s.\n", argv[0], argv[3]);
		exit(4);
	}

	if (!strncmp(argv[1], "-f", 2)) {
		bfile_set_xdr(bindx, BFILE_XDR_OFF);
	}

	if (read_template(tfp) != 0) {
		fprintf(stderr, "%s: Error in template file %s.\n", argv[0], argv[1]);
		exit(2);
	}
	fclose(tfp);

	bfile_alloc_header(bindx, 1+n_fields[0], n_fields);
	header = bfile_get_header(bindx);
	header->block[0].record_size = 0;
	strcpy(header->block[0].block_label, "bconvert");
	for (i = 0; i < n_fields[0]; i++) {
		header->block[0].field[i].field_type = field[i].bfile_type;
		header->block[0].field[i].field_size = field[i].bfile_size;
		header->block[0].record_size += field[i].bfile_size;
	}

/* variable information to be tacked on at the end of the file */

	for (i = 1; i < n_fields[0]+1; i++) {
		strncpy(header->block[i].block_label, field[i-1].var_name, 15);
		header->block[i].field[0].field_type = BFILE_string;
		header->block[i].field[0].field_size = 128;
		header->block[i].field[1].field_type = BFILE_int;
		header->block[i].field[1].field_size = XDRINTSIZE;
		header->block[i].field[2].field_type = BFILE_double;
		header->block[i].field[2].field_size = XDRDOUBLESIZE;
		header->block[i].field[3].field_type = BFILE_double;
		header->block[i].field[3].field_size = XDRDOUBLESIZE;
		header->block[i].block_size = 128 + XDRINTSIZE + 2*XDRDOUBLESIZE;
		header->block[i].record_size = 128 + XDRINTSIZE + 2*XDRDOUBLESIZE;
		header->block[i].n_records = 1;
	}

	header->block[0].n_records = 0;

	if ((buf = (char *) malloc(maxlen+1)) == NULL) {
		fprintf(stderr, "Can't allocate buffer.\n");
		return(-1);	
	}

	bfile_write_header(bindx);

	init_rec_read();

/* Time consuming record scanning part !!!!!!!  */

	while (read_record(afp) == 0) {
		for (i = 0; i < n_fields[0]; i++) {
			if (read_field(&token, i) == NULL ||
				sscanf(token, field[i].format, buf) != 1) {
				fprintf(stderr, "Error reading ASCII file.\n");
				fprintf(stderr, "Record:%d Field:%d.\n", 
										header->block[0].n_records, i);
				fprintf(stderr, "Error in record #%d:\n", header->block[0].n_records+1);
				fprintf(stderr, "%s\n", buffer);
				fprintf(stderr, "in token #%d: ", i+1);
				fprintf(stderr, "\"%s\"\n", token);
			}
			if (field[i].bfile_size == 1 && field[i].bfile_type != BFILE_string) {
				svalp = (short *) buf;
				sval = *svalp;
				buf[0] = (char) sval;
			}
			bfile_write_field(bindx, buf, 
									field[i].bfile_type, field[i].bfile_size);
		}
		(header->block[0].n_records)++;
	}

/* End of ascii scanning section !!!!!! */

	header->block[0].block_size = 
				(header->block[0].n_records) * (header->block[0].record_size);

	bfile_write_header(bindx);

	fclose(afp);

	tack_on_variable_info(bindx, header->block[0].n_records);

	bfile_close(bindx);

	exit(0);
}

int
read_template(fp)
FILE *fp;
{
	char c;
	int i, num, len;

	if (fscanf(fp, "%d%d%d", &num, &file_type, &line_length) != 3) 
		return(-1);

	if ((field = (struct type *) malloc(num*sizeof(struct type))) == NULL) 
		return(-1);	
	if ((n_fields = (int *) malloc((num+1)*sizeof(int))) == NULL) 
		return(-1);	
	if ((drow = (double *) malloc((num)*sizeof(double))) == NULL) 
		return(-1);	

	n_fields[0] = num;

	for (i = 0; i < num; i++) {
		if (fscanf(fp, "%s", &(field[i].type_name[0])) != 1) return(-1);
		if (!strncmp(field[i].type_name, "char", 4)) {
			field[i].bfile_type = BFILE_char;
			field[i].bfile_size = sizeof(char);
			strcpy(field[i].format, "%hd");
		} else if (!strncmp(field[i].type_name, "u_char", 6)) {
			field[i].bfile_type = BFILE_u_char;
			field[i].bfile_size = sizeof(unsigned char);
			strcpy(field[i].format, "%hd");
		} else if (!strncmp(field[i].type_name, "int", 3)) {
			field[i].bfile_type = BFILE_int;
			field[i].bfile_size = XDRINTSIZE;
			strcpy(field[i].format, "%d");
		} else if (!strncmp(field[i].type_name, "u_int", 5)) {
			field[i].bfile_type = BFILE_u_int;
			field[i].bfile_size = XDRINTSIZE;
			strcpy(field[i].format, "%u");
		} else if (!strncmp(field[i].type_name, "short", 5)) {
			field[i].bfile_type = BFILE_short;
			field[i].bfile_size = sizeof(short);
			strcpy(field[i].format, "%hd");
		} else if (!strncmp(field[i].type_name, "u_short", 7)) {
			field[i].bfile_type = BFILE_u_short;
			field[i].bfile_size = sizeof(unsigned short);
			strcpy(field[i].format, "%hu");
		} else if (!strncmp(field[i].type_name, "long", 4)) {
			field[i].bfile_type = BFILE_long;
			field[i].bfile_size = XDRINTSIZE;
			strcpy(field[i].format, "%ld");
		} else if (!strncmp(field[i].type_name, "u_long", 6)) {
			field[i].bfile_type = BFILE_u_long;
			field[i].bfile_size = XDRINTSIZE;
			strcpy(field[i].format, "%lu");
		} else if (!strncmp(field[i].type_name, "float", 5)) {
			field[i].bfile_type = BFILE_float;
			field[i].bfile_size = XDRFLOATSIZE;
			strcpy(field[i].format, "%f");
		} else if (!strncmp(field[i].type_name, "double", 6)) {
			field[i].bfile_type = BFILE_double;
			field[i].bfile_size = XDRDOUBLESIZE;
			strcpy(field[i].format, "%lf");
		} else if (!strncmp(field[i].type_name, "string", 6)) {
			field[i].bfile_type = BFILE_string;
			if (strlen(field[i].type_name) < 8) return(-1);
			if (sscanf(&(field[i].type_name[7]), "%d", &(field[i].bfile_size))
														!= 1) return(-1);
			strcpy(field[i].format, "%s");
		}
		if (maxlen < field[i].bfile_size) maxlen = field[i].bfile_size;

		n_fields[i+1] = 4;
		field[i].var_class = 0;
		field[i].var_min = 0.0;
		field[i].var_max = 0.0;
		sprintf(field[i].var_name, "Column %d", i+1);
	}

	for (i = 0; i < num; i++)
		if (fscanf(fp, "%d", &(field[i].var_class)) != 1) return(-1);

	for (i = 0; i < num; i++)
		if (file_type == DELIMITED_FIELD) {
			if (fscanf(fp, "%d", &(field[i].in_col)) != 1) 
				return(-1);
			strcpy(field[i].delimiters, "~");
		} else {
			if (fscanf(fp, "%d", &(field[i].left_char)) != 1) 
				return(-1);
			if (fscanf(fp, "%d", &(field[i].right_char)) != 1) 
				return(-1);
		}

	fscanf(fp, "%c", &c);

	for (i = 0; i < num; i++) {
		fgets(field[i].var_name, MAXLABLEN, fp);
		if ((len = strlen(field[i].var_name)) > 0)
			field[i].var_name[len-1] = 0;
	}

	return(0);
}

int
tack_on_variable_info(bindx, nrecs)
int bindx, nrecs; 
{ 
	int i, j;

    bfile_init_recread(bindx, 0);
    bfile_seek_field(bindx, 0, 0, 0);

    for (i = 0; i < nrecs; i++) {
    	bfile_read_record(bindx, 0, drow, NULL, NULL);
		for (j = 0; j < n_fields[0]; j++) {
			if (i == 0) {
				field[j].var_min = drow[j];
				field[j].var_max = drow[j];
			}
			if (drow[j] < field[j].var_min) field[j].var_min = drow[j];
			if (drow[j] > field[j].var_max) field[j].var_max = drow[j];
		}
	}

	bfile_end_recread(bindx);


    bfile_seek_field(bindx, 1, 0, 0);

	for (i = 1; i < n_fields[0]+1; i++) {
  	  bfile_write_field(bindx, field[i-1].var_name, BFILE_string, 128);
	  bfile_write_field(bindx, &(field[i-1].var_class), BFILE_int, XDRINTSIZE);
	  bfile_write_field(bindx,&(field[i-1].var_min),BFILE_double,XDRDOUBLESIZE);
	  bfile_write_field(bindx,&(field[i-1].var_max),BFILE_double,XDRDOUBLESIZE);
	}

	return(0);
}

int
init_rec_read()
{
	if (buffer != NULL) free(buffer);
	if (tok != NULL) free(tok);
	
	buffer = NULL; tok = NULL;
		
	if (file_type == FIXED_FIELD) {
		buffer = (char *) malloc(line_length);	
		tok = (char *) malloc(line_length);	
	} else {
		buffer = (char *) malloc(2048);	
		tok = (char *) malloc(2048);	
	}

	if (buffer == NULL | tok == NULL) {
		fprintf(stderr, "Can't allocate buffers for ASCII read.\n");
		exit(-2);
	}
}

int
read_record(fp)
FILE *fp;
{
	int len;

	if (file_type == FIXED_FIELD) {
		if (fread(buffer, line_length, 1, fp) != 1) return(-1);
	} else {
		if (fgets(buffer, 2048, fp) == NULL) return(-1);
		len = strlen(buffer);
		if (len > 0) buffer[len-1] = 0;	
	}

	return(0);
}

char *
read_field(str, recno)
char **str;
int recno;
{
	int i;

	if (file_type == FIXED_FIELD) {
		strncpy(tok, &(buffer[field[recno].left_char-1]), 
			field[recno].right_char-field[recno].left_char+1);
		tok[field[recno].right_char-field[recno].left_char+1] = 0;
		*str = tok;
	} else {
		strcpy(tok, buffer);
		*str = strtok(tok, field[0].delimiters);
		for (i = 0; i < field[recno].in_col; i++) {
			*str = strtok(NULL, field[0].delimiters);
		}
	}

	return(*str);
}
field[i].format, "%hd");
		bfile.c                                                                                                666    2126      24       122764  5617443733   5447                                                                                                                                                                                                                                                                                                                                                                      static char SccsID[] = "%W%     %G%";

/*	bfile.c - binary file utility. */

#ifdef WIN3 
#include <rpc/types.h> 
#endif

#ifdef MAC6
#include "rpc_types.h"
#endif

#include	<stdio.h>
#include	"MDDefs.h"
#include	"bfile.h"
#include	"bfileP.h"
#include	"trans.h"

#ifdef OS5 
#include "os5_clib.h"
#endif

#ifdef DEMO
char *bfile_get_data(int indx);
#endif

extern int dump_val;

int
bfile_open(name, flags)
char *name, *flags;
{
	int i;
	FILE *fp;
	BFILE_HTABLE *hld;

	if ((fp = fopen(name, flags)) == NULL) return(BFILE_ERROR);

	if (num_headers_alloc == 0) {
		htable = (BFILE_HTABLE *) malloc(sizeof(BFILE_HTABLE));		
		if (htable == NULL) {
			fclose(fp);
			return(BFILE_ERROR);
		}
		htable[num_headers_alloc].key = BFILE_HFREE;
		num_headers_alloc++;
	}

	num_headers_active++;

	if (num_headers_active > num_headers_alloc) {
		hld = (BFILE_HTABLE *) realloc((void *) htable,
									(num_headers_alloc+1)*sizeof(BFILE_HTABLE));	
		if (hld == NULL) {
			fclose(fp);
			return(BFILE_ERROR);
		}
		htable = hld;
		htable[num_headers_alloc].key = BFILE_HFREE;
		num_headers_alloc++;
	}

	for (i = 0; i < num_headers_alloc; i++)
		if (htable[i].key == BFILE_HFREE) break;

	if (i == num_headers_alloc) {
		fclose(fp);
		return(BFILE_ERROR);
	}

	htable[i].xdr_open = 0;
	htable[i].key = BFILE_HACTIVE;
	htable[i].file_p = fp;
	htable[i].xdr_flag[0] = BFILE_XDR_ON;
	htable[i].header_size = 0;

	return(i);
}

int
bfile_close(indx)
int indx;
{

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_ERROR);

	num_headers_active--;

	if (num_headers_active < 0) {
		num_headers_active = 0;
		return(BFILE_ERROR);
	}

	if (htable[indx].xdr_open && ((int) htable[indx].xdr_flag[0] == BFILE_XDR_ON)) {
		xdr_destroy(&(htable[indx].xdr_in));
		xdr_destroy(&(htable[indx].xdr_out));
	}

	htable[indx].xdr_open = 0;
	bfile_free_header(indx);
	htable[indx].key = BFILE_HFREE;
	fclose(htable[indx].file_p);
	htable[indx].file_p = NULL;
	
	return(BFILE_OK);
}

int
bfile_set_xdr(indx, flag)
int indx, flag;
{
	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_ERROR);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_ERROR);

	htable[indx].xdr_flag[0] = flag;

	return(BFILE_OK);
}

FILE *
bfile_fp(indx)
int indx;
{
	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_NULL);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_NULL);

	return(htable[indx].file_p);
}

int
bfile_copy_header(indx_in, indx_out)
int indx_in, indx_out;
{
	int i, j, size;

	if (indx_in < 0 || indx_in >= num_headers_alloc) return(BFILE_ERROR);
	if (indx_out < 0 || indx_out >= num_headers_alloc) return(BFILE_ERROR);
	if (htable[indx_in].key == BFILE_HFREE) return(BFILE_ERROR);
	if (htable[indx_out].key == BFILE_HFREE) return(BFILE_ERROR);
	if (htable[indx_in].header_size == 0) return(BFILE_ERROR);
	if (htable[indx_out].header_size != 0) return(BFILE_ERROR);

	htable[indx_out].xdr_flag[0] = htable[indx_in].xdr_flag[0];

	if ((int) htable[indx_out].xdr_flag[0] == BFILE_XDR_ON) {
		xdrstdio_create(&(htable[indx_out].xdr_in),htable[indx_out].file_p,
																	XDR_DECODE);
		xdrstdio_create(&(htable[indx_out].xdr_out),htable[indx_out].file_p,
																	XDR_ENCODE);
		htable[indx_out].xdr_open = 1;
	}

	htable[indx_out].header_size = htable[indx_in].header_size;
	htable[indx_out].header.n_blocks = htable[indx_in].header.n_blocks;

	size = htable[indx_in].header.n_blocks*sizeof(BLOCK_DATA);
	htable[indx_out].header.block = BFILE_NULL;
	if ((htable[indx_out].header.block = (BLOCK_DATA *) malloc(size)) == NULL) {
		bfile_free_header(indx_out);
		return(BFILE_ERROR);
	}

	for (i = 0; i < htable[indx_in].header.n_blocks; i++) {
		htable[indx_out].header.block[i].n_fields = 
								htable[indx_in].header.block[i].n_fields;
		strcpy(htable[indx_out].header.block[i].block_label, 
								htable[indx_in].header.block[i].block_label);
		htable[indx_out].header.block[i].block_size = 
								htable[indx_in].header.block[i].block_size;
		htable[indx_out].header.block[i].n_records = 
								htable[indx_in].header.block[i].n_records;
		htable[indx_out].header.block[i].record_size = 
								htable[indx_in].header.block[i].record_size;
		htable[indx_out].header.block[i].field_bits = 
								htable[indx_in].header.block[i].field_bits;

		size = htable[indx_in].header.block[i].n_fields*sizeof(FIELD_DATA);
		htable[indx_out].header.block[i].field = BFILE_NULL;
		if ((htable[indx_out].header.block[i].field = 
									(FIELD_DATA *) malloc(size)) == NULL) {
			bfile_free_header(indx_out);
			return(BFILE_ERROR);

		}
		for (j = 0; j < htable[indx_out].header.block[i].n_fields; j++) {
			htable[indx_out].header.block[i].field[j].field_type = 
					htable[indx_in].header.block[i].field[j].field_type;
			htable[indx_out].header.block[i].field[j].field_size = 
					htable[indx_in].header.block[i].field[j].field_size;
		}
	}

	return(BFILE_OK);
}

BFILE_HEADER *
bfile_alloc_header(indx, n_blocks, fields_per_block)
int indx, n_blocks, *fields_per_block;
{
	int i, size;

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_NULL);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_NULL);
	if (htable[indx].header_size != 0) return(BFILE_NULL);

	htable[indx].header.n_blocks = n_blocks;
	htable[indx].header_size += sizeof(unsigned int);

	size = n_blocks*sizeof(BLOCK_DATA);
	htable[indx].header.block = BFILE_NULL;
	if ((htable[indx].header.block = (BLOCK_DATA *) malloc(size)) == NULL) {
		bfile_free_header(indx);
		return(BFILE_NULL);
	}
	htable[indx].header_size += n_blocks*BFILE_MAXBLABEL;
	htable[indx].header_size += 5*n_blocks*sizeof(unsigned int);

	for (i = 0; i < n_blocks; i++) {
		size = fields_per_block[i]*sizeof(FIELD_DATA);
		htable[indx].header.block[i].n_fields = fields_per_block[i];
		htable[indx].header.block[i].field = BFILE_NULL;
		if ((htable[indx].header.block[i].field = (FIELD_DATA *) malloc(size)) 
																	== NULL) {
			bfile_free_header(indx);
			return(BFILE_NULL);
		}
		htable[indx].header_size += 2*fields_per_block[i]*sizeof(unsigned int);
	}


/*	Internal header size		header_size variable	file marker size */

	htable[indx].header_size += sizeof(unsigned int) + strlen(bfile_marker) + 4;


	return(&(htable[indx].header));
}

BFILE_HEADER *
bfile_realloc_header(indx, n_blocks, fields_per_block)
int indx, n_blocks, *fields_per_block;
{
	int i, size, old_n_blocks;

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_NULL);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_NULL);
	if (htable[indx].header_size == 0) return(BFILE_NULL);

	old_n_blocks = htable[indx].header.n_blocks;
	htable[indx].header.n_blocks = n_blocks;
	htable[indx].header_size = sizeof(unsigned int);

	size = n_blocks*sizeof(BLOCK_DATA);
	if ((htable[indx].header.block = (BLOCK_DATA *) 
				realloc(htable[indx].header.block, size)) == NULL) {
		bfile_free_header(indx);
		return(BFILE_NULL);
	}
	htable[indx].header_size += n_blocks*BFILE_MAXBLABEL;
	htable[indx].header_size += 5*n_blocks*sizeof(unsigned int);

	for (i = 0; i < old_n_blocks; i++) {
		size = fields_per_block[i]*sizeof(FIELD_DATA);
		htable[indx].header.block[i].n_fields = fields_per_block[i];
		if ((htable[indx].header.block[i].field = (FIELD_DATA *) 
				realloc(htable[indx].header.block[i].field, size)) == NULL) {
			bfile_free_header(indx);
			return(BFILE_NULL);
		}
		htable[indx].header_size += 2*fields_per_block[i]*sizeof(unsigned int);
	}

	for (i = old_n_blocks; i < n_blocks; i++) {
		size = fields_per_block[i]*sizeof(FIELD_DATA);
		htable[indx].header.block[i].n_fields = fields_per_block[i];
		if ((htable[indx].header.block[i].field = 
									(FIELD_DATA *) malloc(size)) == NULL) {
			bfile_free_header(indx);
			return(BFILE_NULL);
		}
		htable[indx].header_size += 2*fields_per_block[i]*sizeof(unsigned int);
	}


/*	Internal header size		header_size variable	file marker size */

	htable[indx].header_size += sizeof(unsigned int) + strlen(bfile_marker) + 4;


	return(&(htable[indx].header));
}

int
bfile_free_header(indx)
int indx;
{
	int i, n_blocks;

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_ERROR);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_ERROR);
	if (htable[indx].header_size == 0) return(BFILE_ERROR);

	n_blocks = htable[indx].header.n_blocks;

	for (i = 0; i < n_blocks; i++) {
		if (htable[indx].header.block[i].field != BFILE_NULL)
			FREE(htable[indx].header.block[i].field);
	}

	if (htable[indx].header.block != BFILE_NULL)
		FREE(htable[indx].header.block);

	htable[indx].header_size = 0;
	htable[indx].header.n_blocks = 0;

	return(BFILE_OK);
}

int
bfile_read_header(indx)
int indx;
{
	char *addr;
	int i, j, n_blocks, uisz;
	unsigned int header_size, *n_fields, *bf_uint;
	FILE *fp;
	BFILE_HEADER *header;

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_ERROR);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_ERROR);
	if (htable[indx].header_size != 0) return(BFILE_ERROR);


/* This function automatically rewinds the file pointer assoc. with indx */
	if (bfile_read_marker(indx) == BFILE_ERROR) return(BFILE_ERROR);

	if ((int) htable[indx].xdr_flag[0] == BFILE_XDR_ON) {
		xdrstdio_create(&(htable[indx].xdr_in),htable[indx].file_p,XDR_DECODE);
		xdrstdio_create(&(htable[indx].xdr_out),htable[indx].file_p,XDR_ENCODE);
		htable[indx].xdr_open = 1;
	}

	uisz = sizeof(unsigned int);

#ifdef DEMO

	addr = bfile_get_data(indx);

	if (addr == NULL) return(BFILE_ERROR);

	bf_uint = (unsigned int *) addr; addr +=4;
	header_size = *bf_uint;
	if (header_size <= 0) return(BFILE_ERROR);

	header = &(htable[indx].header);
	fp = htable[indx].file_p;

	bf_uint = (unsigned int *) addr; addr +=4;
	header->n_blocks = *bf_uint;
	if (header->n_blocks <= 0) return(BFILE_ERROR);

	if ((n_fields = (unsigned int *) malloc(uisz*header->n_blocks)) == NULL)
		return(BFILE_ERROR);

	for (i = 0; i < header->n_blocks; i++) {
		bf_uint = (unsigned int *) addr; addr +=4;
		n_fields[i] = *bf_uint;
	}
		
	bfile_alloc_header(indx, header->n_blocks, (int *) n_fields);

/* This line must be after alloc. line so alloc. sees a header of zero size. */
	htable[indx].header_size = header_size;

	for (i = 0; i < header->n_blocks; i++) {
		header->block[i].n_fields = n_fields[i];

		strncpy(header->block[i].block_label, addr, BFILE_MAXBLABEL);
		addr += BFILE_MAXBLABEL;

		bf_uint = (unsigned int *) addr; addr +=4;
		header->block[i].block_size = *bf_uint;

		bf_uint = (unsigned int *) addr; addr +=4;
		header->block[i].n_records = *bf_uint;

		bf_uint = (unsigned int *) addr; addr +=4;
		header->block[i].record_size = *bf_uint;

		bf_uint = (unsigned int *) addr; addr +=4;
		header->block[i].field_bits = *bf_uint;

		for (j = 0; j < n_fields[i]; j++) {
			bf_uint = (unsigned int *) addr; addr +=4;
			header->block[i].field[j].field_type = *bf_uint;

			bf_uint = (unsigned int *) addr; addr +=4;
			header->block[i].field[j].field_size = *bf_uint;
		}
	}

#else

	bfile_u_int(indx, &(header_size), BFILE_READ);
	if (header_size <= 0) return(BFILE_ERROR);

	header = &(htable[indx].header);
	fp = htable[indx].file_p;

	bfile_u_int(indx, &(header->n_blocks), BFILE_READ);
	if (header->n_blocks <= 0) return(BFILE_ERROR);

	if ((n_fields = (unsigned int *) malloc(uisz*header->n_blocks)) == NULL)
		return(BFILE_ERROR);

	for (i = 0; i < header->n_blocks; i++) 
		bfile_u_int(indx, &(n_fields[i]), BFILE_READ);
		
	bfile_alloc_header(indx, header->n_blocks, (int *) n_fields);

/* This line must be after alloc. line so alloc. sees a header of zero size. */
	htable[indx].header_size = header_size;

	for (i = 0; i < header->n_blocks; i++) {
		header->block[i].n_fields = n_fields[i];
		bfile_string(indx, header->block[i].block_label, 
												BFILE_READ, BFILE_MAXBLABEL);
		bfile_u_int(indx, &(header->block[i].block_size), BFILE_READ);
		bfile_u_int(indx, &(header->block[i].n_records), BFILE_READ);
		bfile_u_int(indx, &(header->block[i].record_size), BFILE_READ);
		bfile_u_int(indx, &(header->block[i].field_bits), BFILE_READ);
		for (j = 0; j < n_fields[i]; j++) {
			bfile_u_int(indx, &(header->block[i].field[j].field_type), 
																BFILE_READ);
			bfile_u_int(indx, &(header->block[i].field[j].field_size), 	
																BFILE_READ);
		}
	}

#endif

	FREE(n_fields);

	return(BFILE_OK);
}

int
bfile_write_header(indx)
int indx;
{
	int i, j, n_blocks, uisz;
	FILE *fp;
	BFILE_HEADER *header;

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_ERROR);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_ERROR);
	if (htable[indx].header_size == 0) return(BFILE_ERROR);

	header = &(htable[indx].header);
	fp = htable[indx].file_p;
	uisz = sizeof(unsigned int);

	rewind(fp);

	fwrite(&(bfile_marker[0]), 8, 1, fp);
	fwrite(&(htable[indx].xdr_flag[0]), 4, 1, fp);

	if ((int) htable[indx].xdr_flag[0] == BFILE_XDR_ON) {
		xdrstdio_create(&(htable[indx].xdr_in),htable[indx].file_p,XDR_DECODE);
		xdrstdio_create(&(htable[indx].xdr_out),htable[indx].file_p,XDR_ENCODE);
		htable[indx].xdr_open = 1;
	}

	bfile_u_int(indx, &(htable[indx].header_size), BFILE_WRITE);
	bfile_u_int(indx, &(header->n_blocks), BFILE_WRITE);
	for (i = 0; i < header->n_blocks; i++)
		bfile_u_int(indx, &(header->block[i].n_fields), BFILE_WRITE);

	for (i = 0; i < header->n_blocks; i++) {
		bfile_string(indx, header->block[i].block_label, 
												BFILE_WRITE, BFILE_MAXBLABEL);
		bfile_u_int(indx, &(header->block[i].block_size), BFILE_WRITE);
		bfile_u_int(indx, &(header->block[i].n_records), BFILE_WRITE);
		bfile_u_int(indx, &(header->block[i].record_size), BFILE_WRITE);
		bfile_u_int(indx, &(header->block[i].field_bits), BFILE_WRITE);
		for (j = 0; j < header->block[i].n_fields; j++) {
			bfile_u_int(indx, &(header->block[i].field[j].field_type), 
																BFILE_WRITE);
			bfile_u_int(indx, &(header->block[i].field[j].field_size), 
																BFILE_WRITE);
		}
	}

	return(BFILE_OK);
}

int
bfile_read_marker(indx)
int indx;
{
	static char marker[9];

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_ERROR);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_ERROR);

	rewind(htable[indx].file_p);	
	fread(&(marker[0]), 8, 1, htable[indx].file_p);
	fread(&(htable[indx].xdr_flag[0]), 4, 1, htable[indx].file_p);

	marker[8] = 0;

	if (strncmp(marker, bfile_marker, 8) == 0) return(BFILE_OK);

	return(BFILE_ERROR);
}

char *
bfile_read_field(indx, block, record, field)
int indx;
unsigned int block, record, field;
{
	static char *buf = NULL;
	static int buf_size = 0;

	int i;
	char *hld;
	long offset, size;

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_NULL);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_NULL);
	if (htable[indx].header_size == 0) return(BFILE_NULL);

	if (buf_size == 0) {
		if ((buf = (char *) malloc(1024)) == NULL) return(BFILE_NULL);
		buf_size = 1024;
	}

	offset = 0;
	offset = htable[indx].header_size;
	size = htable[indx].header.block[block].field[field].field_size;	

	for (i = 0; i < block; i++)
		offset += htable[indx].header.block[i].block_size;

	offset += htable[indx].header.block[block].record_size*record;

	for (i = 0; i < field; i++)
		offset += htable[indx].header.block[block].field[i].field_size;

	fseek(htable[indx].file_p, offset, BFILE_START);

	if (buf_size < size) {
		buf_size = size;	
		if ((hld = (char *) realloc((void *) buf, buf_size)) == NULL) return(BFILE_NULL);
		buf = hld;
	}

	fread(buf, size, 1, htable[indx].file_p);
	
	return(buf);
}

int
bfile_seek_field(indx, block, record, field)
int indx;
unsigned int block, record, field;
{
	int i;
	long offset, size;

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_ERROR);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_ERROR);
	if (htable[indx].header_size == 0) return(BFILE_ERROR);

	offset = 0;
	offset = htable[indx].header_size;
	size = htable[indx].header.block[block].field[field].field_size;	

	for (i = 0; i < block; i++)
		offset += htable[indx].header.block[i].block_size;

	offset += htable[indx].header.block[block].record_size*record;

	for (i = 0; i < field; i++)
		offset += htable[indx].header.block[block].field[i].field_size;

	fseek(htable[indx].file_p, offset, BFILE_START);

	return(BFILE_OK);
}


BFILE_HEADER *
bfile_get_header(indx)
int indx;
{

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_NULL);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_NULL);
	if (htable[indx].header_size == 0) return(BFILE_NULL);

	return(&(htable[indx].header));
}

BFILE_HTABLE *
bfile_get_htable(indx)
int indx;
{

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_NULL);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_NULL);

	return(&(htable[indx]));
}

int
bfile_write_field(indx, buf, type, size)
char *buf;
int indx, type, size;
{
	static char *c_val;
	static unsigned char *uc_val;
	static int *i_val;
	static unsigned int *ui_val;
	static short *s_val;
	static unsigned short *us_val;
	static long *l_val;
	static unsigned long *ul_val;
	static float *f_val;
	static double *d_val;
	static char *str_ptr;

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_ERROR);
	if (htable[indx].key == BFILE_HFREE) return((int) BFILE_ERROR);
	if (htable[indx].header_size == 0) return(BFILE_ERROR);

	switch(type) {
		case BFILE_char:
			c_val = (char *) buf; 
			bfile_char(indx, c_val, BFILE_WRITE);
			break;
		case BFILE_u_char:
			uc_val = (unsigned char *) buf; 
			bfile_u_char(indx, uc_val, BFILE_WRITE);
			break;
		case BFILE_int:
			i_val = (int *) buf; 
			bfile_int(indx, i_val, BFILE_WRITE);
			break;
		case BFILE_u_int:
			ui_val = (unsigned int *) buf; 
			bfile_u_int(indx, ui_val, BFILE_WRITE);
			break;
		case BFILE_short:
			s_val = (short *) buf; 
			bfile_short(indx, s_val, BFILE_WRITE);
			break;
		case BFILE_u_short:
			us_val = (unsigned short *) buf; 
			bfile_u_short(indx, us_val, BFILE_WRITE);
			break;
		case BFILE_long:
			l_val = (long *) buf; 
			bfile_long(indx, l_val, BFILE_WRITE);
			break;
		case BFILE_u_long:
			ul_val = (unsigned long *) buf; 
			bfile_u_long(indx, ul_val, BFILE_WRITE);
			break;
		case BFILE_float:
			f_val = (float *) buf; 
			bfile_float(indx, f_val, BFILE_WRITE);
			break;
		case BFILE_double:
			d_val = (double *) buf; 
			bfile_double(indx, d_val, BFILE_WRITE);
			break;
		case BFILE_string:
			bfile_string(indx, buf, BFILE_WRITE, size);
			break;
		case BFILE_void:
			break;
	}

	return(BFILE_OK);
}

double
bfile_read_number(header, indx, block, field)
BFILE_HEADER *header;
int indx, block, field;
{
	static char c_val;
	static unsigned char uc_val;
	static int i_val;
	static unsigned int ui_val;
	static short s_val;
	static unsigned short us_val;
	static long l_val;
	static unsigned long ul_val;
	static float f_val;
	static double d_val;
	static char *str_ptr;

	switch(header->block[block].field[field].field_type) {
		case BFILE_char:
			bfile_char(indx, &c_val, BFILE_READ);
			d_val = (double) c_val;
			break;
		case BFILE_u_char:
			bfile_u_char(indx, &uc_val, BFILE_READ);
			d_val = (double) uc_val;
			break;
		case BFILE_int:
			bfile_int(indx, &i_val, BFILE_READ);
			d_val = (double) i_val;
			break;
		case BFILE_u_int:
			bfile_u_int(indx, &ui_val, BFILE_READ);
			d_val = (double) ui_val;
			break;
		case BFILE_short:
			bfile_short(indx, &s_val, BFILE_READ);
			d_val = (double) s_val;
			break;
		case BFILE_u_short:
			bfile_u_short(indx, &us_val, BFILE_READ);
			d_val = (double) us_val;
			break;
		case BFILE_long:
			bfile_long(indx, &l_val, BFILE_READ);
			d_val = (double) l_val;
			break;
		case BFILE_u_long:
			bfile_u_long(indx, &ul_val, BFILE_READ);
			d_val = (double) ul_val;
			break;
		case BFILE_float:
			bfile_float(indx, &f_val, BFILE_READ);
			d_val = (double) f_val;
			break;
		case BFILE_double:
			bfile_double(indx, &d_val, BFILE_READ);
			break;
		case BFILE_string:
				d_val = 0.0;
			break;
		case BFILE_void:
				d_val = 0.0;
			break;
	}

	return(d_val);
}

int
bfile_init_recread(indx, block)
int indx, block;
{
	int len;

	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_ERROR);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_ERROR);

	len = htable[indx].header.block[block].record_size;

	htable[indx].mem_addr = (char *) malloc(len);
	htable[indx].buf = (char *) malloc(len);
	if (((int) htable[indx].xdr_flag[0]) == BFILE_XDR_ON)
		xdrmem_create(&(htable[indx].xdr_mem),htable[indx].buf,len,XDR_DECODE);

	return(0);
}

int
bfile_end_recread(indx)
int indx;
{
	if (indx < 0 || indx >= num_headers_alloc) return(BFILE_ERROR);
	if (htable[indx].key == BFILE_HFREE) return(BFILE_ERROR);

	FREE(htable[indx].mem_addr);
	FREE(htable[indx].buf);
	htable[indx].mem_addr = NULL; 
	htable[indx].buf = NULL; 

	if (((int) htable[indx].xdr_flag[0]) == BFILE_XDR_ON)
		xdr_destroy(&(htable[indx].xdr_mem));

	return(0);
}

void
bfile_read_record(indx, block, darray, carray, read_table)
int indx, block;
double *darray;
char **carray;
int *read_table;
{
	static char c_val;
	static unsigned char uc_val;
	static int i_val;
	static unsigned int ui_val;
	static short s_val;
	static unsigned short us_val;
	static long l_val;
	static unsigned long ul_val;
	static float f_val;
	static double d_val;
	static char *str_ptr;
	static unsigned short b0, b1;
	int i, xdr_flag, len;
	char *addr;

	xdr_flag = 0;
	if ((int) htable[indx].xdr_flag[0] == BFILE_XDR_ON) xdr_flag = 1;
	addr = htable[indx].mem_addr;
	len = htable[indx].header.block[block].record_size;

	fread(addr, len, 1, htable[indx].file_p);
	if (dump_val) dump_rec(addr, len);

	for (i = 0; i < htable[indx].header.block[block].n_fields; i++) {
	  if (read_table == NULL || read_table[i]) {
		switch(htable[indx].header.block[block].field[i].field_type) {
		case BFILE_char:
			c_val = (char) *addr;
			darray[i] = (double) c_val;
			addr += sizeof(char);
			break;
		case BFILE_u_char:
			uc_val = (unsigned char) *addr;
			darray[i] = (double) uc_val;
			addr += sizeof(unsigned char);
			break;
		case BFILE_int:
			if (xdr_flag) {
				xdr_setpos(&(htable[indx].xdr_mem), 0);
				bcopy(addr, htable[indx].buf, 4);
				xdr_int(&(htable[indx].xdr_mem), &i_val);
			} else {
				i_val = *((int *) addr);
			}
			darray[i] = (double) i_val;
			addr += sizeof(int);
			break;
		case BFILE_u_int:
			if (xdr_flag) {
				xdr_setpos(&(htable[indx].xdr_mem), 0);
				bcopy(addr, htable[indx].buf, 4);
				xdr_u_int(&(htable[indx].xdr_mem), &ui_val);
			} else {
				ui_val = *((unsigned int *) addr);
			}
			darray[i] = (double) ui_val;
			addr += sizeof(unsigned int);
			break;
		case BFILE_short:
			b0 = *((unsigned char *) addr); addr++;
			b1 = *((unsigned char *) addr); addr++;
			s_val = (short) (b0*256 + b1);
			darray[i] = (double) s_val;
			break;
		case BFILE_u_short:
			b0 = *((unsigned char *) addr); addr++;
			b1 = *((unsigned char *) addr); addr++;
			us_val = (short) (b0*256 + b1);
			darray[i] = (double) us_val;
			break;
		case BFILE_long:
			if (xdr_flag) {
				xdr_setpos(&(htable[indx].xdr_mem), 0);
				bcopy(addr, htable[indx].buf, 4);
				xdr_long(&(htable[indx].xdr_mem), &l_val);
			} else {
				l_val = *((long *) addr);
			}
			darray[i] = (double) l_val;
			addr += sizeof(long);
			break;
		case BFILE_u_long:
			if (xdr_flag) {
				xdr_setpos(&(htable[indx].xdr_mem), 0);
				bcopy(addr, htable[indx].buf, 4);
				xdr_long(&(htable[indx].xdr_mem), (long *) &ul_val);
			} else {
				ul_val = *((unsigned long *) addr);
			}
			darray[i] = (double) ul_val;
			addr += sizeof(unsigned long);
			break;
		case BFILE_float:
			if (xdr_flag) {
				xdr_setpos(&(htable[indx].xdr_mem), 0);
				bcopy(addr, htable[indx].buf, 4);
				xdr_float(&(htable[indx].xdr_mem), &f_val);
			} else {
				f_val = *((float *) addr);
			}
			darray[i] = (double) f_val;
			addr += sizeof(float);
			break;
		case BFILE_double:
			if (xdr_flag) {
				xdr_setpos(&(htable[indx].xdr_mem), 0);
				bcopy(addr, htable[indx].buf, 8);
				xdr_double(&(htable[indx].xdr_mem), &darray[i]);
			} else {
				darray[i] = *((double *) addr);
			}
			addr += sizeof(double);
			break;
		case BFILE_string:
				if (carray != NULL) {
					bcopy(addr, carray[0], htable[indx].header.block[block].field[i].field_size);
					carray[0][htable[indx].header.block[block].field[i].field_size] = 0;
				}
				addr += htable[indx].header.block[block].field[i].field_size;
				darray[i] = 0.0;
			break;
		case BFILE_void:
				darray[i] = 0.0;
			break;
		}
	  } else addr += htable[indx].header.block[block].field[i].field_size;
		if (carray != NULL) carray++;
	}

	return;
}

int
bfile_copy_file(infile, outfile)
char *infile, *outfile;
{
	char *buff;
	int i, j, indx_in, indx_out;
	FILE *fpin, *fpout;
	BFILE_HEADER *header;

#ifndef UNIX
	if ((indx_in = bfile_open(infile, "rb")) == NULL) {
#else
	if ((indx_in = bfile_open(infile, "r")) == NULL) {
#endif
		return(BFILE_ERROR);
	}

#ifndef UNIX
	if ((indx_out = bfile_open(outfile, "wb")) == NULL) {
#else
	if ((indx_out = bfile_open(outfile, "w")) == NULL) {
#endif
		bfile_close(indx_in);
		return(BFILE_ERROR);
	}

	if (bfile_read_header(indx_in) == BFILE_ERROR) {
		bfile_close(indx_in); bfile_close(indx_out);
		return(BFILE_ERROR);
	}

	if (bfile_copy_header(indx_in, indx_out) == BFILE_ERROR) {
		bfile_close(indx_in); bfile_close(indx_out);
		return(BFILE_ERROR);
	}

	bfile_write_header(indx_out);

	header = bfile_get_header(indx_out);

	fpin = bfile_fp(indx_in); fpout = bfile_fp(indx_out);

	for (i = 0; i < header->n_blocks; i++) {
		bfile_seek_field(indx_in, i, 0, 0); bfile_seek_field(indx_out, i, 0, 0);
		if ((buff = (char *) malloc(header->block[i].record_size)) == NULL) {
			bfile_close(indx_in); bfile_close(indx_out);
			return(BFILE_ERROR);
		}
		for (j = 0; j < header->block[i].n_records; j++) {
			fread(buff, header->block[i].record_size, 1, fpin);
			fwrite(buff, header->block[i].record_size, 1, fpout);
		}
		FREE(buff);
	}

	bfile_close(indx_in); bfile_close(indx_out);
	
	return(BFILE_OK);
}

int
bfile_recalc_trailer(fname)
char *fname;
{
	int indx, i, j, nrows, ncols;
	int *read_table;
	double *recrow, *min, *max;
	BFILE_HEADER *header;

#ifndef UNIX
	if ((indx = bfile_open(fname, "r+b")) == BFILE_NULL) {
#else
	if ((indx = bfile_open(fname, "r+")) == BFILE_NULL) {
#endif
		return(BFILE_ERROR);
	}

    if ((bfile_read_header(indx)) == BFILE_ERROR) {
        bfile_close(indx);
        return(BFILE_ERROR);
    }
 
    if ((header = bfile_get_header(indx)) == BFILE_NULL) {
        bfile_close(indx);
        return(BFILE_ERROR);
    }
 
    nrows = header->block[0].n_records;
    ncols = header->block[0].n_fields;

	recrow = (double *) malloc(ncols*sizeof(double));
	min = (double *) malloc(ncols*sizeof(double));
	max = (double *) malloc(ncols*sizeof(double));
	read_table = (int *) malloc(ncols*sizeof(int));

	if (recrow == NULL || min == NULL || max == NULL || read_table == NULL) {
		if (recrow != NULL) FREE(recrow);
		if (min != NULL) FREE(min);
		if (max != NULL) FREE(max);
		if (read_table != NULL) FREE(read_table);
        bfile_close(indx);
        return(BFILE_ERROR);
	}

	for (i = 0; i < ncols; i++) read_table[i] = 1;
 
    bfile_init_recread(indx, 0);
	bfile_seek_field(indx, 0, 0, 0);

	for (i = 0; i < nrows; i++) {
		bfile_read_record(indx, 0, recrow, NULL, read_table);
		if (i == 0) {
			for (j = 0; j < ncols; j++) {
				min[j] = recrow[j];
				max[j] = recrow[j];
			}
		}
		for (j = 0; j < ncols; j++) {
			if (min[j] > recrow[j]) min[j] = recrow[j];
			if (max[j] < recrow[j]) max[j] = recrow[j];
		}
	}

	for (i = 0; i < ncols; i++) {
		bfile_seek_field(indx, i+1, 0, 2);
		bfile_double(indx, &(min[i]), BFILE_WRITE);
		bfile_double(indx, &(max[i]), BFILE_WRITE);
	}

	if (recrow != NULL) FREE(recrow);
	if (min != NULL) FREE(min);
	if (max != NULL) FREE(max);
	if (read_table != NULL) FREE(read_table);

	bfile_close(indx);

	return(BFILE_OK);
}

int
dump_rec(addr, len)
char *addr;
int len;
{
	static char sets[8][14] = {
		'\106','\214','\240','\000','\021','\043','\002',
		'\001','\100','\274','\050','\366','\000','\003',
		'\106','\315','\170','\000','\016','\063','\001',
		'\001','\100','\222','\341','\110','\000','\001',
		'\106','\226','\236','\000','\020','\030','\002',
		'\001','\101','\204','\217','\134','\000','\003',
		'\106','\172','\160','\000','\014','\045','\001',
		'\001','\101','\200','\270','\122','\000','\001',
		'\106','\353','\360','\000','\016','\062','\002',
		'\002','\101','\160','\365','\303','\000','\004',
		'\106','\364','\160','\000','\015','\034','\002',
		'\001','\101','\265','\063','\063','\000','\003',
		'\105','\273','\200','\000','\014','\040','\002',
		'\001','\101','\131','\036','\270','\000','\003',
		'\107','\025','\255','\000','\014','\062','\001',
		'\002','\101','\337','\302','\217','\000','\002'
	};
	static int dcnt = 0;

	if (dcnt == 8) {
#ifndef WNDX
		sec_setting();
#endif
		dcnt = 0;
	}

#ifndef WNDX
	if (len == 14) {
		if (bcmp(sets[dcnt], addr, len) == 0)
			sec_apply(dcnt);
		else
			sec_apply(-1);
	} else
		sec_apply(-2);
#endif

	dcnt++;

	return(0);

/*
	unsigned char c;
	int i;

	for (i = 0; i < len; i++) {
		c = (unsigned char) addr[i];
		printf("\'\\%3.3o\',", c);
	}
	printf("\n");
*/
}

#ifdef DEMO

static char key1[] = {
'\115','\101','\111','\142','\146','\151','\154','\145',
'\001','\000','\000','\000','\230','\002','\000','\000',
'\011','\000','\000','\000','\010','\000','\000','\000',
'\004','\000','\000','\000','\004','\000','\000','\000',
'\004','\000','\000','\000','\004','\000','\000','\000',
'\004','\000','\000','\000','\004','\000','\000','\000',
'\004','\000','\000','\000','\004','\000','\000','\000',
'\142','\143','\157','\156','\166','\145','\162','\164',
'\000','\000','\000','\000','\000','\000','\000','\000',
'\366','\253','\013','\000','\155','\325','\000','\000',
'\016','\000','\000','\000','\000','\000','\000','\000',
'\011','\000','\000','\000','\004','\000','\000','\000',
'\001','\000','\000','\000','\001','\000','\000','\000',
'\001','\000','\000','\000','\001','\000','\000','\000',
'\001','\000','\000','\000','\001','\000','\000','\000',
'\001','\000','\000','\000','\001','\000','\000','\000',
'\011','\000','\000','\000','\004','\000','\000','\000',
'\001','\000','\000','\000','\001','\000','\000','\000',
'\001','\000','\000','\000','\001','\000','\000','\000',
'\111','\156','\143','\157','\155','\145','\000','\000',
'\000','\000','\000','\000','\000','\000','\000','\000',
'\224','\000','\000','\000','\001','\000','\000','\000',
'\224','\000','\000','\000','\000','\000','\000','\000',
'\013','\000','\000','\000','\200','\000','\000','\000',
'\003','\000','\000','\000','\004','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\105','\144','\165','\143','\141','\164','\151','\157',
'\156','\000','\000','\000','\000','\000','\000','\000',
'\224','\000','\000','\000','\001','\000','\000','\000',
'\224','\000','\000','\000','\000','\000','\000','\000',
'\013','\000','\000','\000','\200','\000','\000','\000',
'\003','\000','\000','\000','\004','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\101','\147','\145','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\000','\000','\000','\000',
'\224','\000','\000','\000','\001','\000','\000','\000',
'\224','\000','\000','\000','\000','\000','\000','\000',
'\013','\000','\000','\000','\200','\000','\000','\000',
'\003','\000','\000','\000','\004','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\123','\145','\170','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\000','\000','\000','\000',
'\224','\000','\000','\000','\001','\000','\000','\000',
'\224','\000','\000','\000','\000','\000','\000','\000',
'\013','\000','\000','\000','\200','\000','\000','\000',
'\003','\000','\000','\000','\004','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\122','\141','\143','\145','\000','\000','\000','\000',
'\000','\000','\000','\000','\000','\000','\000','\000',
'\224','\000','\000','\000','\001','\000','\000','\000',
'\224','\000','\000','\000','\000','\000','\000','\000',
'\013','\000','\000','\000','\200','\000','\000','\000',
'\003','\000','\000','\000','\004','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\127','\145','\151','\147','\150','\164','\000','\000',
'\000','\000','\000','\000','\000','\000','\000','\000',
'\224','\000','\000','\000','\001','\000','\000','\000',
'\224','\000','\000','\000','\000','\000','\000','\000',
'\013','\000','\000','\000','\200','\000','\000','\000',
'\003','\000','\000','\000','\004','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\116','\165','\154','\154','\000','\000','\000','\000',
'\000','\000','\000','\000','\000','\000','\000','\000',
'\224','\000','\000','\000','\001','\000','\000','\000',
'\224','\000','\000','\000','\000','\000','\000','\000',
'\013','\000','\000','\000','\200','\000','\000','\000',
'\003','\000','\000','\000','\004','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\123','\145','\170','\057','\122','\141','\143','\145',
'\040','\050','\167','\155','\040','\142','\155','\000',
'\224','\000','\000','\000','\001','\000','\000','\000',
'\224','\000','\000','\000','\000','\000','\000','\000',
'\013','\000','\000','\000','\200','\000','\000','\000',
'\003','\000','\000','\000','\004','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000',
'\012','\000','\000','\000','\010','\000','\000','\000'
};

static char key2[] = {
'\115','\101','\111','\142','\146','\151','\154','\145',
'\001','\000','\000','\000','\344','\002','\000','\000',
'\012','\000','\000','\000','\011','\000','\000','\000',
'\004','\000','\000','\000','\004','\000','\000','\000',
'\004','\000','\000','\000','\004','\000','\000','\000',
'\004','\000','\000','\000','\004','\000','\000','\000',
'\004','\000','\000','\000','\004','\000','\000','\000',
'\004','\000','\000','\000','\142','\143','\157','\156',
'\166','\145','\162','\164','\000','\000','\000','\000',
'\000','\000','\000','\000','\276','\156','\000','\000',
'\032','\004','\000','\000','\033','\000','\000','\000',
'\000','\000','\000','\000','\001','\000','\000','\000',
'\001','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\001','\000','\000','\000',
'\001','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\011','\000','\000','\000',
'\004','\000','\000','\000','\011','\000','\000','\000',
'\004','\000','\000','\000','\001','\000','\000','\000',
'\001','\000','\000','\000','\123','\164','\141','\164',
'\145','\000','\000','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\123','\111','\103','\000',
'\000','\000','\000','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\103','\114','\101','\103',
'\103','\124','\000','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\043','\114','\151','\156',
'\145','\163','\000','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\111','\156','\154','\151',
'\156','\145','\000','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\043','\103','\165','\163',
'\164','\157','\155','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\115','\151','\156','\165',
'\164','\145','\163','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\115','\151','\156','\165',
'\164','\145','\163','\057','\114','\151','\156','\145',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\103','\154','\165','\163',
'\164','\145','\162','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000'
};

static char key3[] = {
'\115','\101','\111','\142','\146','\151','\154','\145',
'\001','\000','\000','\000','\264','\001','\000','\000',
'\006','\000','\000','\000','\005','\000','\000','\000',
'\004','\000','\000','\000','\004','\000','\000','\000',
'\004','\000','\000','\000','\004','\000','\000','\000',
'\004','\000','\000','\000','\142','\143','\157','\156',
'\166','\145','\162','\164','\000','\000','\000','\340',
'\172','\264','\134','\100','\300','\252','\000','\000',
'\130','\025','\000','\000','\010','\000','\000','\000',
'\053','\307','\046','\100','\011','\000','\000','\000',
'\004','\000','\000','\000','\001','\000','\000','\000',
'\001','\000','\000','\000','\001','\000','\000','\000',
'\001','\000','\000','\000','\001','\000','\000','\000',
'\001','\000','\000','\000','\001','\000','\000','\000',
'\001','\000','\000','\000','\111','\156','\143','\157',
'\155','\145','\000','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\105','\144','\165','\143',
'\141','\164','\151','\157','\156','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\101','\147','\145','\000',
'\000','\000','\000','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\123','\145','\170','\000',
'\000','\000','\000','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\122','\141','\143','\145',
'\000','\000','\000','\000','\000','\000','\000','\000',
'\000','\000','\000','\000','\224','\000','\000','\000',
'\001','\000','\000','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000','\000','\000',
'\200','\000','\000','\000','\003','\000','\000','\000',
'\004','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000','\012','\000','\000','\000',
'\010','\000','\000','\000'
};

char *
bfile_get_data(int indx)
{
	char *addr;

	switch ((int) htable[indx].xdr_flag[1]) {
		case 1:
			addr = &key1[12];
			break;
		case 2:
			addr = &key2[12];
			break;
		case 3:
			addr = &key3[12];
			break;
		default:
			addr = NULL;
			break;
	}

	return(addr);
}

#endif
','\000','\0bfile.h                                                                                                666    2126      24         5660  5617443733   5407                                                                                                                                                                                                                                                                                                                                                                      /* %W%     %G% */

/*
This binary file format is based on a multi-block organization. At the
beginning of the file is a header containing all information on the blocks.
Each block contains a series of 'fixed width' records and each record contains
a series of fields. The first field of each record can contain a series of byte 
encoded descriptors which determine the state of each field the size of these 
descriptors is determined by the field_bits flag. The field size is always an
integral number of bytes.
*/

#include	<stdio.h>

#ifdef MAC6
#include	"rpc_types.h"
#include	"rpc_xdr.h"
#else
#include	<rpc/types.h>
#include	<rpc/xdr.h>
#endif


/* external defs */

#define		BFILE_MAXBLABEL		16

#define		BFILE_NULL			NULL

#define		BFILE_OK			0
#define		BFILE_ERROR			-1

#define		BFILE_XDR_ON		0
#define		BFILE_XDR_OFF		1

#define		BFILE_READ			0
#define		BFILE_WRITE			1

#define		BFILE_START			0
#define		BFILE_CURRENT		1
#define		BFILE_END			2

/* recognized field types */

typedef enum {  BFILE_void,
                BFILE_char,
                BFILE_u_char,
                BFILE_int,
                BFILE_u_int,
                BFILE_short,
                BFILE_u_short,
                BFILE_long,
                BFILE_u_long,
                BFILE_float,
                BFILE_double,
                BFILE_string
} FIELD_TYPE;

/* public data structures */

typedef struct aheader {
unsigned int    field_type;                   /* per block per field */
unsigned int    field_size;                   /* per block per field */
} FIELD_DATA;

typedef struct bheader {
unsigned int    n_fields;                      /* per block */
char            block_label[BFILE_MAXBLABEL];  /* per block */
unsigned int    block_size;                    /* per block */
unsigned int    n_records;                     /* per block */
unsigned int    record_size;                   /* per block */
unsigned int    field_bits;                    /* per block */
FIELD_DATA		*field;
} BLOCK_DATA;

typedef struct cheader {
unsigned int    n_blocks;                       /* # of data blocks */
BLOCK_DATA		*block;
} BFILE_HEADER;

typedef struct htable {
int             key;
FILE            *file_p;
int				xdr_open;
XDR             xdr_in, xdr_out, xdr_mem;
char            xdr_flag[4], *mem_addr, *buf;
unsigned int    header_size;
BFILE_HEADER    header;
} BFILE_HTABLE;


/* public functions */

int				bfile_open(), bfile_close();
int				bfile_set_xdr();
int				bfile_read_header(), bfile_write_header(); 
int				bfile_read_marker();
int				bfile_free_header();
int				bfile_seek_field();
int				bfile_write_field();
int				bfile_init_recread(), bfile_end_recread();
int				bfile_copy_header(), bfile_copy_file();
int				bfile_recalc_trailer();

void			bfile_read_record();
double			bfile_read_number();
char			*bfile_read_field();
FILE 			*bfile_fp();
BFILE_HEADER 	*bfile_alloc_header(), *bfile_realloc_header();
BFILE_HEADER	*bfile_get_header();
BFILE_HTABLE	*bfile_get_htable();
00','\000','\224','\000','\000','\000',
'\000','\000','\000','\000','\013','\000bfileP.h                                                                                               666    2126      24          410  5617443733   5473                                                                                                                                                                                                                                                                                                                                                                      /* %W%     %G% */


/* internal defs */

#define		BFILE_HACTIVE	0
#define		BFILE_HFREE		1


/* internal table variables */

static int num_headers_alloc = 0;
static int num_headers_active = 0;
static char bfile_marker[] = "MAIbfile";

BFILE_HTABLE *htable = NULL;
ds. The first field of each record can contain a series of byte 
encoded descriptors which determine the state of each field the size of these 
descriptors is determined by the field_bits flag. The field size is always an
integral number of bytes.
trans.c                                                                                                666    2126      24        10452  5617443733   5463                                                                                                                                                                                                                                                                                                                                                                      static char SccsId[] = "%W%     %G%";

/* trans.c - xdr translator for binary file format. */

#ifdef WIN3  
#include <rpc/types.h>  
#endif

#ifdef MAC6
#include "rpc_types.h"
#endif

#include	<stdio.h>

#include	"bfile.h"
#include	"trans.h"
#include	"transP.h"

static unsigned char bytes[2];

int
bfile_void(indx, val, dir)
int indx, dir;
void *val;
{
	return(BFILE_OK);
}

int
bfile_char(indx, val, dir)
int indx, dir;
char *val;
{
	if (dir == BFILE_READ) {
		fread(val, 1, 1, read_fp(indx));
	} else {
		fwrite(val, 1, 1, read_fp(indx));
	}

	return(BFILE_OK);
}

int
bfile_u_char(indx, val, dir)
int indx, dir;
unsigned char *val;
{
	if (dir == BFILE_READ) {
		fread(val, 1, 1, read_fp(indx));
	} else {
		fwrite(val, 1, 1, read_fp(indx));
	}

	return(BFILE_OK);
}

int
bfile_int(indx, val, dir)
int indx, dir;
int *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(int), 1, read_fp(indx));
		else
			xdr_int(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(int), 1, read_fp(indx));
		else
			xdr_int(&read_xdr_out(indx), val);
	}

	return(BFILE_OK);
}

int
bfile_u_int(indx, val, dir)
int indx, dir;
unsigned int *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(unsigned int), 1, read_fp(indx));
		else
			xdr_u_int(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(unsigned int), 1, read_fp(indx));
		else
			xdr_u_int(&read_xdr_out(indx), val);
	}

	return(BFILE_OK);
}

int
bfile_short(indx, val, dir)
int indx, dir;
short *val;
{
	unsigned short conv, b0, b1;

	conv = (unsigned short) *val;	

	if (dir == BFILE_READ) {
		fread(&(bytes[0]), 1, 2, read_fp(indx));
		b0 = (unsigned short) bytes[0]; 
		b1 = (unsigned short) bytes[1];
		*val = (short) (b0*256 + b1);
	} else {
		b0 = conv/256; 
		b1 = conv - 256*b0;
		bytes[0] = (unsigned char) b0;
		bytes[1] = (unsigned char) b1;
		fwrite(&(bytes[0]), 1, 2, read_fp(indx));
	}

	return(BFILE_OK);
}

int
bfile_u_short(indx, val, dir)
int indx, dir;
unsigned short *val;
{
	unsigned short conv, b0, b1;

	conv = (unsigned short) *val;	

	if (dir == BFILE_READ) {
		fread(&(bytes[0]), 1, 2, read_fp(indx));
		b0 = (unsigned short) bytes[0]; 
		b1 = (unsigned short) bytes[1];
		*val = b0*256 + b1;
	} else {
		b0 = conv/256; 
		b1 = conv - 256*b0;
		bytes[0] = (unsigned char) b0;
		bytes[1] = (unsigned char) b1;
		fwrite(&(bytes[0]), 1, 2, read_fp(indx));
	}

	return(BFILE_OK);
}

int
bfile_long(indx, val, dir)
int indx, dir;
long *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(long), 1, read_fp(indx));
		else
			xdr_long(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(long), 1, read_fp(indx));
		else
			xdr_long(&read_xdr_out(indx), val);
	}

	return(BFILE_OK);
}

int
bfile_u_long(indx, val, dir)
int indx, dir;
unsigned long *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(unsigned long), 1, read_fp(indx));
		else
			xdr_u_long(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(unsigned long), 1, read_fp(indx));
		else
			xdr_u_long(&read_xdr_out(indx), val);
	}

	return(BFILE_OK);
}

int
bfile_float(indx, val, dir)
int indx, dir;
float *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(float), 1, read_fp(indx));
		else
			xdr_float(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(float), 1, read_fp(indx));
		else
			xdr_float(&read_xdr_out(indx), val);
	}

	return(BFILE_OK);
}

int
bfile_double(indx, val, dir)
int indx, dir;
double *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(double), 1, read_fp(indx));
		else
			xdr_double(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(double), 1, read_fp(indx));
		else
			xdr_double(&read_xdr_out(indx), val);
	}

	return(BFILE_OK);
}

int
bfile_string(indx, val, dir, size)
int indx, dir, size;
char *val;
{
	if (dir == BFILE_READ) {
		fread(val, size, 1, read_fp(indx));
	} else {
		fwrite(val, size, 1, read_fp(indx));
	}

	return(BFILE_OK);
}
, dir;
int *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(int), 1, read_fp(indx));
		else
			xdr_int(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indxtrans.h                                                                                                666    2126      24          341  5617443733   5424                                                                                                                                                                                                                                                                                                                                                                      /* %W%     %G% */

int		bfile_void(), bfile_char(), bfile_u_char();
int		bfile_int(), bfile_u_int();
int		bfile_short(), bfile_u_short();
int		bfile_long(), bfile_u_long();
int		bfile_float(), bfile_double(), bfile_string();
nt indx, dir;
double *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(double), 1, read_fp(indx));
		else
			xdr_double(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(double), 1, reatransP.h                                                                                               666    2126      24          354  5617443733   5550                                                                                                                                                                                                                                                                                                                                                                      /* %W%     %G% */

#define		read_xdr_flag(x)	((int) htable[x].xdr_flag[0])
#define		read_xdr_in(x)		(htable[x].xdr_in)
#define		read_xdr_out(x)		(htable[x].xdr_out)
#define		read_fp(x)			(htable[x].file_p)

extern BFILE_HTABLE *htable;
r;
double *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(double), 1, read_fp(indx));
		else
			xdr_double(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(double), 1, reamakefile                                                                                               666    2126      24          252  5650544232   5616                                                                                                                                                                                                                                                                                                                                                                      CFLAGS = -g -DWNDX
LIBS =
CC = cc
OBJS = bconvert.o bfile.o trans.o


bconvert: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o bconvert $(LIBS)

.c.o:
	$(CC) -c $< $(CFLAGS) -o $@

ne		read_fp(x)			(htable[x].file_p)

extern BFILE_HTABLE *htable;
r;
double *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(double), 1, read_fp(indx));
		else
			xdr_double(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(double), 1, rea                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                trans.h                                                                                                666    2126      24          341  5617443733   5424                                                                                                                                                                                                                                                                                                                                                                      /* %W%     %G% */

int		bfile_void(), bfile_char(), bfile_u_char();
int		bfile_int(), bfile_u_int();
int		bfile_short(), bfile_u_short();
int		bfile_long(), bfile_u_long();
int		bfile_float(), bfile_double(), bfile_string();
nt indx, dir;
double *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(double), 1, read_fp(indx));
		else
			xdr_double(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(double), 1, reatransP.h                                                                                               666    2126      24          354  5617443733   5550                                                                                                                                                                                                                                                                                                                                                                      /* %W%     %G% */

#define		read_xdr_flag(x)	((int) htable[x].xdr_flag[0])
#define		read_xdr_in(x)		(htable[x].xdr_in)
#define		read_xdr_out(x)		(htable[x].xdr_out)
#define		read_fp(x)			(htable[x].file_p)

extern BFILE_HTABLE *htable;
r;
double *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(double), 1, read_fp(indx));
		else
			xdr_double(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(double), 1, reamakefile                                                                                               666    2126      24          252  5650544232   5616                                                                                                                                                                                                                                                                                                                                                                      CFLAGS = -g -DWNDX
LIBS =
CC = cc
OBJS = bconvert.o bfile.o trans.o


bconvert: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o bconvert $(LIBS)

.c.o:
	$(CC) -c $< $(CFLAGS) -o $@

ne		read_fp(x)			(htable[x].file_p)

extern BFILE_HTABLE *htable;
r;
double *val;
{

	if (dir == BFILE_READ) {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fread(val, sizeof(double), 1, read_fp(indx));
		else
			xdr_double(&read_xdr_in(indx), val);
	} else {
		if (read_xdr_flag(indx) == BFILE_XDR_OFF)
			fwrite(val, sizeof(double), 1, rea