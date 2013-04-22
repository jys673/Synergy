#include "field.h"

void multigr()
{
    int iter, done, MINRES;
    char s[80];
    double maxdiff, diff;

    done = 0;
    diff = 1000.0;
    XRES = YRES = MINRES = 32;
    while ( !done ) {
        ilevel = 0;
        iter = 0;
        while ( XRES <= NRES ) {
            dx = size / (float)(XRES-1);
            dy = size / (float)(YRES-1);

            printf("Relaxing \n");
            GaSeOnce(&iter, &diff);
            printf("Interpolation: Finished on %dx%d %dx%d Difference = %f\n", 
                    XRES, YRES, NRES, NRES);
            /***ShowDbl(XRES, YRES, XDIS, YDIS, phiold);***/
            if  ((diff<eps)&&(XRES==NRES)) {
                done = 1;
            }
            if (XRES < NRES) {
                ilevel ++;
                XRES = 2*XRES;
                YRES = 2*YRES;
                printf("Interpolating from %dx%d to %dx%d\n", 
                       XRES/2, YRES/2, XRES,YRES);
                Interpolate(XRES/2, YRES/2);
            }
        }
        while (( XRES > MINRES )&&(!done)) {
            ilevel--;
            XRES = XRES/2;
            YRES = YRES/2;
            dx = size / (float)(XRES-1);
            dy = size / (float)(YRES-1);
            
            printf("Projecting from %dx%d to %dx%d\n",
                   XRES, YRES, XRES/2, YRES/2);
            Project(XRES, YRES);
            printf("Relaxing \n");
            GaSeOnce(&iter, &diff);
            printf("Projection: Finished on %dx%d %dx%d Difference = %f\n", 
                    XRES, YRES, NRES, NRES);
            /***ShowDbl(XRES, YRES, XDIS, YDIS, phiold);***/
        }
    }
}

void Interpolate(int xr, int yr)
{
    int i,j, xr2, yr2, i2, j2;
    char t[512];

    xr2 = 2*xr; yr2 = 2*yr;
/*-----------------------------------------------------------------------*/
/* Initialise phiold[0][0] to 0;                                         */
/* First transfer the existing solution points from phinew[][] to        */ 
/* phiold[][]                                                            */
/*-----------------------------------------------------------------------*/
    for (i=0; i<yr2; i++) {
        for  (j=0; j<xr2; j++) {
            phiold[i][j] = 0.0;
        }
    }

    for (i=0; i<yr2; i=i+2) {
        i2 = i/2;
        for (j=0; j<xr2; j=j+2) {
            j2 = j/2;
            phiold[i][j] = phinew[i2][j2];
        }
    }
/*-----------------------------------------------------------------------*/
/* Now set the values of the empty new points to the average of their    */
/* neighbors                                                             */
/*   1. Even rows                                                        */
/*   2. Odd rows                                                         */
/*       a. Even points                                                  */
/*       b. Odd points                                                   */
/*-----------------------------------------------------------------------*/
    for  (i=0; i<yr2; i=i+2) {
        for (j=1; j<xr2-1; j=j+2) {
            if  (geom[i][j]!=0)
                phiold[i][j] = (phiold[i][j-1] + phiold[i][j+1]) / 2.0;
	}
        if  (geom[i][xr2-1]!=0)
            phiold[i][xr2-1] = phiold[i][xr2-2];
    }

    for  (i=1; i<yr2-2; i=i+2) {
        for  (j=0; j<xr2; j=j+2) {              
            if  (geom[i][j]!=0)
                phiold[i][j] = (phiold[i-1][j]+phiold[i+1][j]) / 2.0;
        }
        for  (j=1; j<xr2-2; j=j+2) {           
            if  (geom[i][j]!=0)
                phiold[i][j] = (phiold[i-1][j-1] + phiold[i-1][j+1] + 
                                phiold[i+1][j-1] + phiold[i+1][j+1])/4.0 ;
        }
                                                
        if  (geom[i][xr2-1]!=0)
            phiold[i][xr2-1] = (phiold[i-1][xr2-2] + phiold[i+1][xr2-2])/2.0;
    }
/* and now the same for the last row */
    for  (j=0; j<xr2; j=j+2) {
        if  (geom[yr2-1][j]!=0)
            phiold[yr2-1][j] = phiold[yr2-2][j];
    }
    for  (j=1; j<xr2-2; j=j+2) {
        if  (geom[yr2-1][j]!=0)
            phiold[yr2-1][j] = (phiold[yr2-2][j-1] + phiold[yr2-2][j+1])/2.0;
    }
    if  (geom[yr2-1][xr2-1]!=0)
        phiold[yr2-1][xr2-1] = phiold[yr2-2][xr2-2];

    
}


void Project(int xr, int yr)
{
    int xr2, yr2, i, j;
    yr2 = xr2 = 2*xr;
    
    for  (i=0; i<yr2; i++) {
        for (j=0; j<xr2; j++ ) {
            phiold[i][j] = phinew[i*2][j*2];
        }
    }
}
