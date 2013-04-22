/**********************************************************
 *	CIS750:distributed parallel programming project
 *
 *	USE SYNERGY TO SPEEED UP THE N-QUEEN PROBLEM
 *
 *	Auther: 	yan shi
 *	date: 		Oct. 5,1993
 *	adviser:	Dr.Shi, Yuan
 *________________________________________________________
 *	This is a server program for the problem
 **********************************************************/

# include <sys/types.h>
# include "queen.h"

int       	n,total;
int		tsd,res,tplength,tpnamesz;

main()
{ 
	int     i,row,col;

	tsd=cnf_open("work_ts",0);
	res=cnf_open("out_ts",0);

	while (1)
	{	
		sprintf(tpname,"state*\0");
		cnf_tsget(tsd, tpname, ituple, 0);
	
		if (ituple[0]==0)
		{
			tplength=(6*n+4)*sizeof(int);
			sprintf(tpname,"state*\0");
			cnf_tsput(tsd, tpname, ituple, tplength);
			printf("worker will terminal!!\n");
			cnf_term();
		}	

		/* get the informations from the tuple */
		n=ituple[0];
		row =ituple[1];	 
		col =ituple[2];
		for (i=0;i<=n-1;i++)
			r[i] = ituple[i+3+n];
		for (i=0;i<=2*n-2;i++)
	  	{
			d1[i] =ituple[2*n+3+i];
			d2[i] =ituple[4*n+2+i];
		}
		total=0;
		slaver(col);
		sprintf(tpname,"count\0");
		cnf_tsget(tsd, tpname, iituple, 0);
		iituple[0] =iituple[0]-1;
		iituple[1] +=total;
		sprintf(tpname,"count\0");
		tplength=2*8;
		cnf_tsput(tsd, tpname, iituple, tplength);
	  }	/* end of the while(1) loop */ 
}		/* end of the main() */
int slaver(col)
int col;
{
	int row,flag,found,finish,i;

	col=col+1;
        for (row=1;row<=n;row++)
	{
		flag=TRUE;
		found=FALSE;
		finish=FALSE;
	  	while ((col<=n)&&flag && !found && !finish)
		{
	    	    if (r[row-1] && d1[row+col-2] && d2[row-col+n-1])
		  	if (col ==n) 	/* Is there a solution? */
		 	{
				found=TRUE;
				total++;
			}					
		  	else
		 	{
				/*increase the number of the previous Q*/
				ituple[2+col] =row;
				r[row-1]  = FALSE;
				d1[row+col-2] = FALSE;
				d2[row-col+n-1] = FALSE;
				
				slaver(col);
				finish=TRUE;
				r[row-1] = TRUE;
				d1[row+col-2] =TRUE;
				d2[row-col+n-1] =TRUE;
			}
	   	   else
			 flag = FALSE;
		} /* end of the while( , , ) loop*/
	} 	  /* end of the for( )*/
}		  /* end of the slaver() */

