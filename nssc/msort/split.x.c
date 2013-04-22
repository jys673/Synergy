/*---------------------------------------------------------------------
    split.c - for the mergesort Configurator application.  this program
    splits the input file into four separate inputs for the sort programs
    to sort.

    Author: Doug Bagley
    Modified by Feijian Sun, 08/29/94. For XDR.
 ---------------------------------------------------------------------*/

#define  N  4		/* number of slaves */
#define  LINELEN  20	/* max line length of text data file */
#define  NULL     0

main()
{
    int   fin, sorter[N], nread, i, j, k;
    char  linbuf[LINELEN];
    
    fin = cnf_open("infile","r" );
    sorter[0] = cnf_open("sort1",0);
    sorter[1] = cnf_open("sort2",0);
    sorter[2] = cnf_open("sort3",0);
    sorter[3] = cnf_open("sort4",0);

    /* distribute file to the sorters */
    i = 0;
    nread = cnf_fgets(fin, linbuf, LINELEN);
    while (nread != 0) {
	j = atoi(linbuf);
	k = cnf_xdr_write(sorter[i++], (char *)&j, sizeof(int), 2);
	nread = cnf_fgets(fin, linbuf, LINELEN);
	if (i == N)  i = 0;
    }
    cnf_term(); 
}
