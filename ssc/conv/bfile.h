/* %W%     %G% */

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
