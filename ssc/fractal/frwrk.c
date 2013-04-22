/************************************************************************
 *
 *    Fractal worker
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

    /* start timing test */
    strcpy(tpname,"*");
    status = cnf_tsget(tsd,tpname, ituple, 0);
    if (tpname[0] != 'T') goto working_cycle;
    printf(" frwrk received test tuple (%d)\n",atoi(tpname+1));
    G = (int) ituple[0];
    ix = (int) ituple[1];
    xmin = ituple[2];
    xdis = ituple[3] - xmin;
    ymin = ituple[4];
    ydis = ituple[5] - ymin;
    xres = (int) ituple[6];
    yres = (int) ituple[7];
    iterat = (int) ituple[8];
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
    otuple[0] = (double) atoi(tpname+1);
    tplength = 1 * sizeof(double);
    printf("Worker Procesed %s tuple \n", tpname); 
    status = cnf_tsput(res, tpname, otuple, tplength);
    /* start regular work */ 
    while ( 1) {  /* loop forever  */
	strcpy(tpname, "W*");
	status = cnf_tsget(tsd, tpname, ituple, 0);
working_cycle:
	printf("worker received (%s) \n",tpname);
	if ( status > 0 ) {    /* normal receive */
		G = (int) ituple[0];
		if (G == 0) {	/* put the sentinel back */
			status = cnf_tsput(tsd, tpname, ituple, status);
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
		status = cnf_tsput(res, tpname, otuple, tplength);
         } else {
		printf(" Worker terminated\n");
		cnf_term();
         }
     }
}
