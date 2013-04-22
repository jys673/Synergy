/***************************************************************
 *	CIS750: distributed parallel programming project
 *
 *	USE SYNERGY TO SPEED UP THE N-QUEEN PROBLEM 
 *
 *	Auther:		yan shi
 *	SSN:		171-74-6065
 *	date:		Oct. 5 ,1993
 *	adviser:	Dr.Shi,Yuan
 *	Modified by Feijian Sun, 08/26/94. For XDR.
 *  ____________________________________________________________
 *	This program is a client program for this problem.
 *	In this version, we partition the program to the secord 
 *	row. 
 ***************************************************************/

# include <stdio.h>
# include <sys/types.h>
# include "queen.h"

int 		tsd,res,tplength;	
int    		n;

main()
{ 
	int 	P,tp_cnt,total,j;
	long    t0, t1;
	char    host[128];
	FILE    *fd;

	gethostname(host, sizeof(host));
	t0 = time((long *)0);
	tsd = cnf_open("work_ts",0);
	res = cnf_open("out_ts",0);

	n = 10; 
	P = cnf_getP();

	if (P > n)
		deeppartition(P,n);
	else 
		simplepartition(P,n);

	/* now receive the results */
	tp_cnt=100;
	while (tp_cnt>0)
	{
                sprintf(tpname,"count\0");
                tplength=2*8;
		cnf_xdr_tsread(tsd, tpname, (char *)iituple, 0, 2);
		total=iituple[1];
		tp_cnt=iituple[0];
	}
	t1 = time((long *)0) - t0;
	fd = fopen("nq_xdr.time", "a");
	/* stat file format: host total_sol n P elapsed */
        printf("\n--- WE FOUND %d WAYS TO PLACE %d QUEENS (%d,%d,%d sec.)\n",
				total,n, P, t1);
	fprintf(fd, "%s %d %d %d %d\n",host, total, n, P, t1);
	fflush(fd);
	fclose(fd);
	cnf_term();
}   

/* When # of workers is less and equal to the # of queen */
simplepartition() 
{
	int i,j,row,col;

	iituple[0]=n;	/*we total put n jobs into the space*/
	iituple[1]=0;	/*it will store the # of the results*/
       	sprintf(tpname,"count\0");
       	tplength=2*8;
	cnf_xdr_tsput(tsd, tpname, (char *)iituple, tplength, 2);
	tplength =(6*n+4)*sizeof(int);
	/* initial the values and partition the job*/
	for (j=1;j<=n;j++) 
	{
 		ituple[0] = n; 		/* the number of the Q */
		ituple[1] = j; 		/* the row */
		ituple[2] = 1; 		/*the col */
		for (i=0;i<= n-1;i++)	/* keep the previous Q pos.*/
			ituple[3+i]=0;
		ituple[3]=j;
		for (i=0;i<= n-1;i++)	/* initial the r[ ]*/
			ituple[n+3+i]=TRUE;
		for (i=0;i<=2*n-2;i++)	/* initial the d1[ ],d2[ ] */ 
		{
			ituple[2*n+3+i] = TRUE;
			ituple[4*n+2+i] = TRUE;
		}
		row=j;
		col=1;
		ituple[n+3+row-1]=FALSE;
		ituple[2*n+3+row+col-2]=FALSE;
		ituple[4*n+2+row-col+n-1]=FALSE;

		sprintf(tpname,"state%d\0",j);
		cnf_xdr_tsput(tsd, tpname, (char *)ituple, tplength, 2);
	}
	printf("Master put total number of packages: (%d)\n",j);

	/* the terminal package */
	ituple[0]=0;
	sprintf(tpname,"state%d\0",j+1);
	cnf_xdr_tsput(tsd, tpname, (char *)ituple, tplength, 2);
}  	/* end of the simplepartition program*/

/* When # of workers is bigger than the # of queen */
deeppartition()
{
	int h,H,num,i,row,col;

	/* initialize the variables */
	H=2;
	row=0;
	col=0;
	for (i=0;i<=n-1;i++)
	{
		r[i]=TRUE; 
		ituple[n+3+i]=TRUE;
	}
	for (i=0;i<=2*n-2;i++)
	{
		d1[i]=TRUE;
		d2[i]=TRUE;
		ituple[2*n+3+i]=TRUE;
		ituple[4*n+2+i]=TRUE;
	}

	h=1;
	num=0;
	num=slaver(col,h,H,num);
	printf("Master put total number of packages: (%d)\n",num);

	iituple[0]=num;
	iituple[1]=0;
	tplength=2*8;
	sprintf(tpname,"count\0");
	cnf_xdr_tsput(tsd, tpname, (char *)iituple, tplength, 2);
	/* the terminal package */
	ituple[0]=0;
	sprintf(tpname,"state%d\0",num+1);
	tplength=(6*n+4)*sizeof(int);
	cnf_xdr_tsput(tsd, tpname, (char *)ituple, tplength, 2);
}

int slaver(col,h,H,num)
int col,h,H,num;
{
	int flag,finish;
	int i,row; 

	col=col+1;
	for (row=1;row<=n;row++)
	{
		flag=TRUE;
		finish=FALSE;
		while ((h<=H) && flag && !finish)
		{
			if (r[row-1] && d1[row+col-2] && d2[row-col+n-1])
			{
				ituple[0]=n;
				ituple[1]=row;
				ituple[2]=col;
				ituple[2+col]=row;
				r[row-1]=FALSE;
				d1[row+col-2]=FALSE;
				d2[row-col+n-1]=FALSE;
				ituple[n+3+row-1]=FALSE;
				ituple[2*n+3+row+col-2]=FALSE;
				ituple[4*n+2+row-col+n-1]=FALSE;
				h++;
				if (h>H)
				{
					sprintf(tpname,"state%d\0",num);
					tplength=(6*n+4)*sizeof(int);
					cnf_xdr_tsput(tsd, tpname, (char *)ituple, tplength, 2);
					num++;

					h--;
					finish=TRUE;

					r[row-1]=TRUE;
					d1[row+col-2]=TRUE;
					d2[row-col+n-1]=TRUE;
					ituple[n+3+row-1]=TRUE;
					ituple[2*n+3+row+col-2]=TRUE;
					ituple[4*n+2+row-col+n-1]=TRUE;
				}
				else
				{
					num=slaver(col,h,H,num);	
					h--;
					finish=TRUE;

					r[row-1]=TRUE;
					d1[row+col-2]=TRUE;
					d2[row-col+n-1]=TRUE;
					ituple[n+3+row-1]=TRUE;
					ituple[2*n+3+row+col-2]=TRUE;
					ituple[4*n+2+row-col+n-1]=TRUE;
				}
			}
			else 
				flag=FALSE;
		} /*end of the while() loop*/
	}	/* end of the for() loop */
	return(num);
} /*end of the slaver()*/

