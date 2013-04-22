/***************************************************************************/
/****									****/
/****		Worker is doing the job of separating the binary data	****/
/****									****/
/***************************************************************************/

#include	<stdio.h>
#include	<string.h>

#ifdef WIN3
#include	<rpc/types.c>
#endif

#include	"bfile.h"

#define		MAXLABLEN		128
#define		MAXTYPELEN		256
#define		XDRINTSIZE		4
#define		XDRFLOATSIZE	4
#define		XDRDOUBLESIZE	8

#define		DELIMITED_FIELD	0
#define		FIXED_FIELD		1		


static char *buffer = NULL; 
static char *bptr;
static char *tok = NULL;
static char *ituple = NULL;
static int  record_no = 0;
static chunk_size = 0;

int  fd_ln_length[4];	/** Field , Line_length, File_type and Chunk_size **/

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
static int maxlen = 0;

char *read_field();

int TSI, TSO;

int put_into_TS(sw)
int sw;
{
 char  tpname[40], hostname[128];
 int	tplength, hostid,num = 1;

 
	if ( sw == 0 ) {
		hostid = gethostid();
		gethostname(hostname, 128);
		tplength = strlen(hostname) + 1;
		memcpy(ituple,hostname, tplength );
		sprintf(tpname,"A%d",hostid);
		cnf_tsput(TSO, tpname, ituple, tplength);
	}
	else if ( sw == 1 ) {
		tplength = (maxlen + 1)*n_fields[0]*chunk_size + sizeof(int);
		sprintf(tpname,"XDR_M%d",record_no);
		cnf_tsput(TSO, tpname, ituple, tplength);
	}
		
}

int collect_TS(sw)
int sw;
{
char *otuple;
char tpname[40];
int  tplength, status,num;

	if ( sw == 0 ) {
	   while( 1 ) {
		sprintf(tpname,"n_field");
		status = cnf_tsread(TSI, tpname, fd_ln_length, 0);
/*
		printf("worker received tuple %d\n",status);
*/
		num = fd_ln_length[0];
		line_length = fd_ln_length[1];
		file_type = fd_ln_length[2];
		chunk_size = fd_ln_length[3];
		tplength = sizeof(struct type)*num;
		sprintf(tpname,"field");
		otuple = (char *)malloc(tplength);
		status = cnf_tsread(TSI, tpname, otuple, 0);
/*
		printf("worker received %s tuplength %d\n",tpname, status);
*/
/**** We create a same template as in Master ****/
		create_template(num, tplength, otuple);
		init_rec_read();
		free(otuple);
		break;
	   }
	}
	else if ( sw == 1 ) {
		memset(buffer, 0x0, line_length*chunk_size);
		sprintf(tpname,"data*");
		buffer = bptr;
		status = cnf_tsget(TSI, tpname, buffer, 0);
		sscanf(tpname,"%*4c%d",&record_no);
		printf("client record %d size %d\n",record_no,status);
/*
		printf("client tpname %s record %d  size %d\n",tpname,record_no,status);
		printf("client data %s\n",buffer);
*/
/****	If record_no less than 0 , we'll no longer to waiting for the data and end ****/
		if ( record_no < 0 ) return -1;
		change_field();
	}
	else
		;
	return 0;
}

/**** Change_field is separating the fields and changing related data into binary mode ****/
change_field()
{
 int	i, j;
 char	*token, *ptr;
 short 	*svalp, sval;
 int	tplength;
	ptr = ituple + sizeof(int);
	tplength = n_fields[0]*(maxlen+1)*chunk_size;
	memset( ptr, 0x0, tplength);

/*   we get repetition here */
	memcpy(&chunk_size, buffer, sizeof(int));
	memcpy(ituple, buffer, sizeof(int));
	buffer += sizeof(int);
	for ( j = 0; j < chunk_size; j++ ) {
	   for ( i = 0; i < n_fields[0]; i++ ) {
		if ( read_field(&token, i) == NULL ||
			sscanf(token, field[i].format, ptr) != 1 ) {
			fprintf(stderr, "Error reading ASCII file.\n");
			fprintf(stderr, "%s\n",buffer);
			fprintf(stderr, "in token #%d: ",i+1);
			fprintf(stderr, "\"%s\"\n",token);
		}
/*
		printf("client token %s\n",token);
*/
		if ( field[i].bfile_size == 1 && field[i].bfile_type != BFILE_string) {
			svalp = (short *) ptr;
			sval = *svalp;
			ptr[0] = (char) sval;
		}
		ptr += maxlen;
	    }
	    if ( file_type != FIXED_FIELD ) buffer += strlen(buffer) + 1; 
		else buffer += line_length;
	}

}

int create_template(num, tplength, tuple)
int  num, tplength;
char *tuple;
{
 int i;
	if ((field = (struct type *) malloc(num*sizeof(struct type))) == NULL) 
		return(-1);	
	if ((n_fields = (int *) malloc((num+1)*sizeof(int))) == NULL) 
		return(-1);	
	if ((drow = (double *) malloc((num)*sizeof(double))) == NULL) 
		return(-1);	

	n_fields[0] = num;
	memcpy(field, tuple, tplength);
	for ( i = 0; i < num; i++ ) {
		if ( maxlen <  field[i].bfile_size) maxlen = field[i].bfile_size;
	}

}

int
init_rec_read()
{
	if (buffer != NULL) free(buffer);
	if (tok != NULL) free(tok);
	
	buffer = NULL; tok = NULL;
/****	make sure the buffer size is big enough to hold data	****/
	buffer = (char *) malloc(line_length*chunk_size);	
	tok = (char *) malloc(line_length);	
	ituple = (char *)malloc(n_fields[0]*(line_length+1)*chunk_size);
	if (buffer == NULL || tok == NULL || ituple == NULL) {
		fprintf(stderr, "Can't allocate buffers for ASCII read or field change.\n");
		exit(-2);
	}
	bptr = buffer;
}

int
free_rec_read()
{
	if (bptr != NULL) free(bptr);
	if (tok != NULL) free(tok);
	if (ituple != NULL) free(ituple);
	if (field != NULL) free(field);
	if (n_fields != NULL) free(n_fields);
	if (drow != NULL) free(drow);
	
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

main()
{
 double st, et;
 double wall_clock();
	TSI = cnf_open("ASCII",0);
	TSO = cnf_open("XDR",0);
	st = wall_clock();
	collect_TS(0);
	put_into_TS(0);
	while ( !collect_TS(1) ) 
	   put_into_TS(1);
	et = wall_clock();
	printf("start from %f end at %f \n total is %f us\t",st,et,(et-st));
	printf("end of work from client\n");
	free_rec_read();
	cnf_close(TSI);
	cnf_close(TSO);
	cnf_term();

}

