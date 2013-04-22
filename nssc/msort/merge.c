/*---------------------------------------------------------------------
    merge.c - for the mergesort Configurator application.  this program
    receives the sorted outputs from the separate sort files and merges
    them into one sorted output.

    Author: Doug Bagley
 ---------------------------------------------------------------------*/

#define  N  4		/* number of sorters to read from */
#define  MAX  999999999  
			/* sentinel - larger than any number sorted */
#define  LINELEN  20	/* max line length of text data file */

main()
{
    int   nread, fout, merger[N], i, j, k, buf[N], l;
    char  linbuf[LINELEN];

    fout = cnf_open("out","w");
    merger[0] = cnf_open("in1",0);
    merger[1] = cnf_open("in2",0);
    merger[2] = cnf_open("in3",0);
    merger[3] = cnf_open("in4",0);

    /* now merge the numbers read from the slaves */

    cnf_read(merger[0], &buf[0], sizeof(int));
    cnf_read(merger[1], &buf[1], sizeof(int));
    cnf_read(merger[2], &buf[2], sizeof(int));
    cnf_read(merger[3], &buf[3], sizeof(int));

    while ((buf[0] != MAX) || (buf[1] != MAX) || 
		(buf[2] != MAX) || (buf[3] != MAX) ) 
    {
	i = (buf[0] <= buf[1]) ? 0 : 1;
	i = (buf[i] <= buf[2]) ? i : 2;
	i = (buf[i] <= buf[3]) ? i : 3;
	k = i;
	sprintf(linbuf, "%7d\n", buf[k]);
	l = cnf_fputs(fout, linbuf, LINELEN);
	l = cnf_read(merger[k], &j , sizeof(int));
	buf[k] = j;
	if (l==0)
	{
		buf[k] = MAX;
		printf(" END_OF_FILE(%d)\n",k);
	}
    }
    cnf_term();
}
