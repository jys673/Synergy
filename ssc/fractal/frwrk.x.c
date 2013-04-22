/************************************************************************
 *
 *    Fractal worker
 *
 *    revised by Feijian Sun for XDR, August 1994
 *
 ************************************************************************
 */
#include  <stdio.h>
#include  "fractal.h" 

main()
{
    int     	ix, iy, g;
    double   	x, y, xmin, ymin, xdis, ydis;

    register double a1, a2;
    register double ti, tr; 
    register int i, iterat;  

    int G;
    int  	tsd, res,		/* Space object handels */
    		tplength, status, xres, yres;

    tsd = cnf_open("coords",0);
    res = cnf_open("colors",0);

    while ( 1) {  /* loop forever  */
	strcpy(tpname, "*");
	status = cnf_xdr_tsget(tsd, tpname, (char *)ituple, 0, 5);
	printf("worker received (%s) \n",tpname);
	if ( status > 0 ) {    /* normal receive */
		G = (int) ituple[0];
		if (G == 0) {	/* put the sentinel back */
			status = cnf_xdr_tsput(tsd, tpname, (char *)ituple, status, 5);
			printf("Worker found last tuple. Term.\n");
			cnf_term();
		}
		ix = (int) ituple[1];
    		xmin = ituple[2];
    		xdis = ituple[3] - xmin;
		ymin = ituple[4];
		ydis = ituple[5] - ymin;
		xres = (int) ituple[6];
		yres = (int) ituple[7];
		iterat = (int) ituple[8];
		otuple[0] = ituple[0];
		otuple[1] = ituple[1];
		/* packing the results */
		for (g=0; g<G; g++) {
			x = (ix+g) * xdis / xres + xmin;
			for  ( iy=yres; iy-- > 0; ) {
				y = (iy * ydis)/yres + ymin; 
                                tr = x;
                        	ti = y;
				for  ( i=0; i<iterat; i++) {
					a1 = (tr  * tr);
					a2 = (ti * ti); 	
					if ( (a1 + a2) > 4.0  ) break; 
					ti= 2 * ti*tr + y;
					tr = a1 - a2 + x;
				}
				otuple[2+g*YRES+iy] = (double) i;
			}
		}
		tplength = (2+G*YRES) * sizeof(double);
		printf("Worker Procesed %s tuple \n", tpname); 
		status = cnf_xdr_tsput(res, tpname, (char *)otuple, tplength, 5);
         } else {
		printf(" Worker terminated\n");
		cnf_term();
         }
     }
}
