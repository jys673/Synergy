static char SccsId[] = "%W%     %G%";

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
