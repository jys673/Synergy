/************************************************************************
 *
 *	 		Xep  Worker
 *
 *	Coded by 	Dr. Yuan Shi.	
 *      Revised by 	Sudhakar Nelamangala		December 1993
 *
 *		Synergy Distributed Systems 
 *	    	Temple University
 *
 ************************************************************************
 *
 *		List of modifications 
 *
 *	1. ituple will have G = -1 as the termination signal
 *	2. kernel of the program has been chaned to make the pg run faster. 
 *	3. otuple is a structure { char host[128]; double index[XRES*YRES+2] }
 *
 *
 ************************************************************************
 */
#include  <stdio.h>
#include  <math.h>
#include  <signal.h>
#include  <string.h>
#include  <ctype.h>
#include  "xep.h" 


main()
{
	int     	ix, iy, g;
	double   	x, y, xstep, ystep;
	int  	tsd, res;		/* Space object handels */
    	int	tpnamsz, tplength, status, iterat,G;

	register int i;			/* number of iterations until divergence */
	register double ar, ai;		/* accumulator */	
	register double a1, a2;
	int	COLOR = 1;
	int 	wd;
	char	host[128];
    	tsd = cnf_open("coord",0);
    	res = cnf_open("color",0);

	printf("INSIDE WORKER cnf open done\n");	
    while ( 1) {  		/* loop forever  */
	strcpy(tpname, "*\0");
	tpnamsz = strlen(tpname);
	status = cnf_tsget( tsd, tpname, ituple, tplength );
	if ( status > 0 ) {    /* normal receive */
		G = (int) ituple[0];
		if ( (G == 0) || (G == -1) ) {	/* put the sentinel back */
			status = cnf_tsput(tsd,tpname,ituple,status);
			if ( G == 0 ) 
			printf("Worker found last tuple. Waiting !!\n");
			else
			cnf_term();
		} else { 
		iy = (int) ituple[1];
		y = ituple [2];
		xstep = ituple[4];
		ystep = ituple[5];
		iterat = (int) ituple[6];
		wd = XRES;
		otuple.index[0] =  ituple[0];
		otuple.index[1] =  ituple[1];

	/* packing the results */
		for (g=0; g<G; g++) {
/*
			printf("\nX(%d)",ix+g);
*/
			x = ituple [3];
			y += ystep;
			for  ( ix=0; ix<wd; ix++ ) {
				x += xstep;
				ar = x;
				ai = y;
				for(i=0;i<iterat;i++) {
					a1 = (ar * ar);
					a2 = (ai * ai);
					if (a1 + a2 > 4.0)
						break;
					ai = 2 * ai * ar + y;
					ar = a1 - a2 + x;
				}	
			otuple.index[2+g*wd+ix] = (double) i;
			
/*
				printf("x(%f)ix(%d) y(%f)iy(%d) c(%d)\n",
					x,ix+g,y,iy,i);
*/
			}
		}
		tplength = (2+G*wd) * 8;
		tpnamsz = strlen(tpname);

	//	printf("Procesed %s tuple \n", tpname); 
		status = gethostname(host,sizeof(host));
		sprintf(otuple.host,"%s\0",host);

		status = cnf_tsput( res, tpname,&otuple, tplength );
	 }  	
         } else {
		printf(" Worker terminated\n");
		cnf_term();
         }
     }
}
