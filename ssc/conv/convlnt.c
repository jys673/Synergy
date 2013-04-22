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

/**** We add chunk size ****/
static int  chunk_size = 2607;
/**** 2300 for snowhite, sleepy and dopey ****/

static int maxlen = 0;

static char *buffer = NULL; 
static char *tok = NULL;

/**** We add ituple, counter and processors ****/
static char *ituple = NULL;
static int  counter = 1;
static int  processors = 0;
/**** We add record_length and packet_size ****/
int record_length, packet_size;
static int  total_size = 0;

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

int file_type, line_length = 2048;
int *n_fields;
double *drow;

int dump_val = 0;

char *read_field();

/**** We add TSI, TSO ****/
int TSI, TSO;	/* Master send stuff to TSI and receive from TSO */
/**** We add configuration file name here ****/
/**** In config.sys, we put template, ascii data and binary data file name ****/
char *config_name = "/home/argo/shi/syn94/apps/ssc/conv/config.sys";
/**** We add fd_ln_length[4] ****/
int  fd_ln_length[4];	/* n_field ,line_length, file_type and chunk size */

main()
{
	char *buf, *token;
	short *svalp, sval;
	int i, j, bindx;
	FILE *tfp, *afp;
	BFILE_HEADER *header;

/****   We add cfg_fp, fname1, fname2, fname3  ****/
	FILE *cfg_fp;	/* configuration file pointer */
	char fname1[128], fname2[128], fname3[128];
#ifdef TIME
long st,mt,et;
	time(&st);
#endif

	cfg_fp = fopen(config_name, "r");
	fgets(fname1, 128, cfg_fp);
	fgets(fname2, 128, cfg_fp);
	fgets(fname3, 128, cfg_fp);
	fclose(cfg_fp);
	fname1[strlen(fname1)-1] = 0;
	fname2[strlen(fname2)-1] = 0;
	fname3[strlen(fname3)-1] = 0;


	if ((tfp = fopen(fname1, "r")) == NULL) {	
		fprintf(stderr, " Can't open template file %s.\n", fname1);
		exit(2);
	}

	if ((afp = fopen(fname2, "r")) == NULL) {	
		fprintf(stderr, " Can't open ASCII file %s.\n", fname2);
		exit(3);
	}

#ifdef WIN3
	if ((bindx = bfile_open(fname3, "w+b")) == BFILE_ERROR) {	
#else
	if ((bindx = bfile_open(fname3, "w+")) == BFILE_ERROR) {	
#endif
		fprintf(stderr, " Can't open binary file %s.\n", fname3);
		exit(4);
	}
	if (read_template(tfp) != 0) {
		fprintf(stderr, " Error in template file %s.\n", fname1);
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
/****   We start processing   ****/
	TSI = cnf_open("ASCII",0);
	TSO = cnf_open("XDR",0);
	processors = cnf_getP();

	ituple = (char *)malloc(chunk_size*line_length+1);
	put_into_TS(0);
	collect_TS();

/* We start of ascii scanning section and put data into tuple space*/
	i = 0;
	packet_size = 0;
	while ( read_record(afp) == 0 ) {
		/* reassign data from buffer to ituple */
		memcpy( ituple + packet_size + sizeof(int), buffer, record_length );
		/* keep record of buffer length in order to reduce the communication overhead */
		packet_size += record_length;
		total_size += record_length;
		i++;
		if ( i%chunk_size == 0 ) {
			put_into_TS(1);	
			packet_size = 0;
			i = 0;
		}
	}
	if ( i > 0 ) put_into_TS(i+2);
	put_into_TS(-1);
/* End of ascii scanning section !!!!!! */

/* Start of collection from tuple space and conversion */
#ifdef TIME
	time(&mt);
#endif
	collect_convt(bindx, header);

	free(ituple);

	header->block[0].block_size = 
				(header->block[0].n_records) * (header->block[0].record_size);

	bfile_write_header(bindx);

	fclose(afp);

	tack_on_variable_info(bindx, header->block[0].n_records);

	bfile_close(bindx);
#ifdef TIME
	time(&et);
	afp = fopen("conv.time","a");
	fprintf(afp,"Resources : Processors %d  Original File size %d\n",processors, total_size);
	fprintf(afp,"Total elapsed time: %d. Total write time: %d\n",(et-st),(et-mt));
	fclose(afp);
#endif
	cnf_close(TSI);
	cnf_close(TSO);
	cnf_term();
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
		
	if (file_type == FIXED_FIELD) 
		line_length = 1024;	
	buffer = (char *) malloc(line_length);	
	tok = (char *) malloc(line_length);	

	if (buffer == NULL | tok == NULL) {
		fprintf(stderr, "Can't allocate buffers for ASCII read.\n");
		exit(-2);
	}
}

/**** We add function call put_into_TS,which means put data into tuple space **/
int put_into_TS(sw)
int	sw;
{
 char tpname[40];
 int  tplength;
	if ( sw == 0 ) {
		tplength = 4*sizeof(int);
		sprintf(tpname,"n_field");
		fd_ln_length[0] = n_fields[0];
		fd_ln_length[1] = line_length;
		fd_ln_length[2] = file_type;
		fd_ln_length[3] = chunk_size;
		cnf_tsput(TSI, tpname, fd_ln_length, tplength);

		sprintf(tpname,"field");
		tplength = sizeof(field[0])*n_fields[0];
		memcpy(ituple,field,tplength);
		cnf_tsput(TSI, tpname, ituple, tplength);
	}
	else if ( sw > 0 ) {
		if ( sw > 2 ) tplength = sw - 2;
		else   tplength = chunk_size;
		memcpy(ituple, &tplength, sizeof(int));
		tplength = packet_size + sizeof(int);
/**** We place chunk_size at first, then append data ****/
/**** keep record of the counter ****/
		sprintf(tpname,"data%d",counter);
		cnf_tsput(TSI, tpname, ituple, tplength);
		counter++;
	}
	else if ( sw == -1 ) {
/**** Send final signal to every worker ****/
		for ( sw = 1; sw <= processors; sw++ ) {
			sprintf(tpname,"data-%d",sw);
			tplength = 1;
			ituple[0] = 0;
			cnf_tsput(TSI, tpname, ituple, tplength);
		}
	}
}

/**** We add function call collect_TS, which do the job of countting the processors ****/
int collect_TS()
{
 int  i=0;
 char tpname[40];
 char *otuple;
 int status;

 	while ( i < 1 ) {
		strcpy( tpname,"A*");
		otuple = (char *)malloc(10000);
		status = cnf_tsget(TSO, tpname, otuple, 0);
		i++;
		free(otuple);
	}
}

/**** We add function call here collect_convt, which collect binary data and write to the binary file ****/
int collect_convt(bindx, header)
int	bindx;
BFILE_HEADER *header;
{
 int  cnt = 1,i =0, j = 0;
 char tpname[40];
 char *otuple, *ptr;
 int  tplength;
 short *svalp, sval;

	printf("counter is %d\n",counter);
	otuple = (char *)malloc(chunk_size*n_fields[0]*(maxlen+1)+sizeof(int));
	while ( cnt < counter ) {
		sprintf(tpname,"XDR_M%d",cnt);
		tplength = cnf_tsget(TSO, tpname, otuple, 0);
		ptr = otuple + sizeof(int);
		memcpy(&chunk_size,otuple, sizeof(int));
		for ( j = 0; j < chunk_size; j++ ) {
		  for ( i = 0; i < n_fields[0]; i++ ) {
		   bfile_write_field(bindx, ptr, field[i].bfile_type, field[i].bfile_size);
		   ptr += maxlen;
		  }
		  (header->block[0].n_records)++ ;
		}
		cnt++;
	}
	free(otuple);
}

int
read_record(fp)
FILE *fp;
{
	int len;

	if (file_type == FIXED_FIELD) {
		if (fread(buffer, line_length, 1, fp) != 1) return(-1);
		record_length = line_length;
	} else {
		if (fgets(buffer, line_length, fp) == NULL) return(-1);
		len = strlen(buffer);
		if (len > 0) buffer[len-1] = 0;	
		record_length = len;
	}
	return(0);
}

