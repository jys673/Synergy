/* %W%     %G% */


/* internal defs */

#define		BFILE_HACTIVE	0
#define		BFILE_HFREE		1


/* internal table variables */

static int num_headers_alloc = 0;
static int num_headers_active = 0;
static char bfile_marker[] = "MAIbfile";

BFILE_HTABLE *htable = NULL;
