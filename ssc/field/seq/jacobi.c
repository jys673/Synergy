/*----------------------------------------------------------------------*/
/* This field holds the Jacobi relaxation functions                     */
/*----------------------------------------------------------------------*/
#include "field.h"

void Jacobi(int *iter)
{
    int i,j, done;
    double  maxdiff, diff;

    done = 0;
    while (!done) {
        done = 1;            /* suppose we are done */
        maxdiff = -1.0;
        for (i=0; i<YRES; i++) {
            for  (j=0; j<XRES; j++) {
                switch (geom[ilevel][i][j]) {
                case 1:  /* Sides  */
                    phinew[i][j] = (phiold[i][j-1] + phiold[i][j+1]) / 2.0;
                    break;
                case 2: /* poles */
                    phinew[i][j] = ( phiold[i+1][j] + phiold[i-1][j] + 
                                     magn*dx ) / 2.0;
                    break;
                case 3: /* Corners */
                    phinew[i][j] = ( phiold[i+1][j] + phiold[i-1][j] + 
                                     phiold[i][j-1] + phiold[i][j+1] + 
                                     magn*dx ) / 4.0;
                    break;
                case 4:
                    meshJ(i,j);
                    break;
                }
                diff = phinew[i][j] - phiold[i][j];
                if (fabs(diff) > maxdiff) maxdiff=fabs(diff);
                if (fabs(diff) > eps ) done = 0;
             }
        }  
        if (!(done)) {    /*store data to do one more iteration... */
            for (i=0; i<YRES; i++) {
                for  (j=0; j<XRES; j++) {
                    phiold[i][j] = phinew[i][j];
                }
            }
        }  /* end if */
        *iter = *iter+1;
        printf("Iteration %d   Diff %f \r", *iter, maxdiff);
        fflush(stdout);
        /***ShowDbl(XRES, YRES, XDIS, YDIS, phiold); ***/
    }   /* end while */
}


void meshJ(int i, int j )
{
    int count;
    double sum;
    
    count = 0;
    sum = 0;
    if  ( (j-1) >= 0 ){
        sum = sum + phiold[i][j-1];
        count = count +1;
    }
    if  ( (i-1) >= 0 ){
        sum = sum + phiold[i-1][j];
        count = count +1;
    }
    if  ( (j+1) < XRES ){
        sum = sum + phiold[i][j+1];
        count = count +1;
    }
    if  ( (i+1) < YRES ){
        sum = sum + phiold[i+1][j];
        count = count +1;
    }
    phinew[i][j] = sum / count;
}

