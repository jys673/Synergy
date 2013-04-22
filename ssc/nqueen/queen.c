/*************************************************************	
 *	The Suquential Solution of the N-Queens Problem
 *      CIS750
 *	Yan Shi, CIS, Temple University
 *	Oct., 1993
 *************************************************************/

#include <stdio.h>
#include  <time.h>
#include <sys/types.h>

#define  TRUE 1
#define  FALSE 0
#define  SLIMIT  10000

typedef   struct
{
	int  stackarray[SLIMIT-1];
	int  top;
} stack;

typedef   int  rowcheck[SLIMIT];
typedef   int  diagonalcheck1[2*SLIMIT-1];
typedef   int  diagonalcheck2[2*SLIMIT-1];

rowcheck        r;
diagonalcheck1  d1;
diagonalcheck2  d2;

int count, probe_count;
long t1,t0;
double ti;
	
main(argc,argv)
int argc;
char *argv[];
{ 
	stack     	s;
	int       	n,i,row,col,found;
	FILE *fd;
	char host[128];

	gethostname(host,sizeof(host));
	probe_count = 0;
	col=1;
	row=1;
	count=0;
	if (argc < 2) 
	{
		printf(" Usage: queen total_positions\n");
		exit (1);
	}
	n=atoi(argv[1]);	
	for (i=0;i<=n-1;i++)
		r[i] = TRUE;
	for (i=0;i<=2*n-2;i++)
		d1[i] =TRUE;
	for (i=0;i<=2*n-2;i++)
		d2[i] = TRUE;
	found  = FALSE;
	setstack(&s);

	t0 = time((long *)0);
	while (((col <n+1) &&(row <n+1)) || !empty(&s))
		if ( (col<n+1) && (row<n+1) )
		{
			/*if (feasible(row,col,r,d1,d2,n))*/
			probe_count ++;
			if (r[row-1] && d1[row+col-2] && d2[row-col+n-1])
			   {
				process(row,col,&found,&s,n);
				push(row, &s);

				r[row-1]  = FALSE;
				d1[row+col-2] = FALSE;
				d2[row-col+n-1] = FALSE;
				col = col+1;
				row =1;
				}
			else
				row = row+1;
		}
		else
		   {
			pop(&s,&row);
			col = col-1;
			r[row-1] = TRUE;
			d1[row+col-2] = TRUE; 
			d2[row-col+n-1] = TRUE;
			row = row+1;
			}
	t1 = time((long *)0) - t0;
	fd = fopen("nq_seq.time", "a");
	if (!found )
	{
		printf ("\n NO SOLUTIONS");
		fprintf (fd, "\n NO SOLUTIONS");
	}
	else { 
		printf("\n--- WE FOUND TOTAL %d WAYS TO PLACE %d QUEENS\n",
				count,n);
		fprintf(fd, "%s %d %d Probes(%d) %d %d",
			host,	count,n, probe_count,t1);
	}
	printf("--- THE TOTAL TIME USED IN THIS PROGRAM IS %d secs \n",t1);
	if (t1 > 0) fprintf(fd," %f Million probes/sec\n",
		(float) probe_count/t1/1000000);
	else fprintf(fd,"* Million probes/sec \n");
	fflush(fd);
	fclose(fd);	
}


process(row,col,pfound,ps,n)
int row  ,col,*pfound,n;
stack *ps;
{ 
	int i;
	int  item();

	if (col == n)
	{
		count +=1;
		*pfound = TRUE;
	}
}

setstack(ps)
stack  *ps;
{ 
	(*ps).top = -1;
}

empty(ps)
stack  *ps;
{
	return((*ps).top == -1);
}

push(value ,ps)
int value;
stack  *ps;
{
	if ((*ps).top == (SLIMIT -1))
		overflow(ps);
	else 
	  {
		(*ps).top = (*ps).top +1;
		(*ps).stackarray[(*ps).top] =value;
		}
}

pop(ps,pvalue)
stack  *ps;
int    *pvalue;
{
	if(empty(ps))
		underflow(ps);
	else 
 	 {
		*pvalue = (*ps).stackarray[(*ps).top];
		(*ps).top = (*ps).top -1;
		}
}

int item(i,ps)
stack  *ps;
{
	return((*ps).stackarray[(*ps).top-i]);
}

overflow(ps)
stack  *ps;
{
	printf("\n stack overflow");
}

underflow(ps)
stack  *ps;
{
	printf("\n  stack  underflow");
}
