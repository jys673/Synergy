/* %W%     %G% */

#define		read_xdr_flag(x)	((int) htable[x].xdr_flag[0])
#define		read_xdr_in(x)		(htable[x].xdr_in)
#define		read_xdr_out(x)		(htable[x].xdr_out)
#define		read_fp(x)			(htable[x].file_p)

extern BFILE_HTABLE *htable;
