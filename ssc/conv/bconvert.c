static char SccsId[] = "@(#)bconvert.c	1.17     8/30/94";

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
	
#ifdef TIME
long st,et;
	time(&st);
#endif
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

printf("final records is %d\n",header->block[0].n_records);
	bfile_close(bindx);
#ifdef TIME
time(&et);
printf("starting from %d  end at %d total period is %d\n",st,et,(et-st));
#endif
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
			strcpy(field[i].delimiters, "\t\n ");
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
