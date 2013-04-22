/*---------------------------------------------------------------------
    sort.c - for the mergesort Configurator application.  this program
    takes an array of numbers and does a bubblesort on them.

    Author: Doug Bagley
    Modified by Feijian Sun, 08/29/94. For XDR.
 ---------------------------------------------------------------------*/

#define  SIZE  10000

main()
{
    int buf[SIZE], i, n, infile, outfile, unsorted, tmp;

    infile = cnf_open("in",0);
    outfile = cnf_open("out",0);
    n = 0;
    while (cnf_xdr_read(infile, (char *)&buf[n], sizeof(int), 2))
    {
	printf("sort read:(%d)\n",buf[n]);
	n++;
    } 
    printf(" after reading...\n"); 
    /* bubble sort */
    unsorted = 1;
    while (unsorted) {
	unsorted = 0;
	for (i=1; i<n; i++)
	    if (buf[i-1] > buf[i]) {
		tmp = buf[i];
		buf[i] = buf[i-1];
		buf[i-1] = tmp;
		unsorted = 1;
	    }
    }
    /* write sorted file back to the master */
    for (i=0; i<n; i++)
    {   printf(" **** SORT output(%d)\n",buf[i]);
	tmp = buf[i];
	cnf_xdr_write(outfile, (char *)&tmp, sizeof(int), 2);
    }
    cnf_term();
}
