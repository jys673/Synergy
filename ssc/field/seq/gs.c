/*----------------------------------------------------------------------*/
/* This module holds the Gauss-Seidel funtions (relaxed or otherwise    */
/*----------------------------------------------------------------------*/
#include "field.h"

void GaSe(int *iter)
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
                    phinew[i][j] = (phinew[i][j-1] + phiold[i][j+1]) / 2.0;
                    break;
                case 2: /* poles */
                    phinew[i][j] = ( phiold[i+1][j] + phinew[i-1][j] + 
                                     magn*dx ) / 2.0;
                    break;
                case 3: /* Corners */
                    phinew[i][j] = ( phiold[i+1][j] + phinew[i-1][j] + 
                                     phinew[i][j-1] + phiold[i][j+1] + 
                                     magn*dx ) / 4.0;
                    break;
                case 4:
                    meshGS(i,j);
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
        if (!(*iter%10)) {
            printf("Iteration %d   Diff %f \r", *iter, maxdiff);
            fflush(stdout); 
            for (i=0; i<YRES; i++) {
                for  (j=0; j<XRES; j++) {
                    phinew[i][j] = log(1.0+ phiold[i][j]);
                } 
            }
            /***ShowDbl(XRES, YRES, XDIS, YDIS, phinew); ***/
        }
    }   /* end while */
}


void meshGS(int i, int j )
{
    int count;
    double sum;
    
    count = 0;
    sum = 0;
    if  ( (j-1) >= 0 ){
        sum = sum + phinew[i][j-1];
        count = count +1;
    }
    if  ( (i-1) >= 0 ){
        sum = sum + phinew[i-1][j];
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


void GaSeI(int ip, int niter, int *done)
{
    int ia, ib, i,j, it;
    double  maxdiff, diff;

    *done = 0;   
    it = 0;
    ia = 1; 
    ib = NP;
    if  (it==0) { 
        ia = 1;
        ib = NP;
    }
    if  (it==P) {
        ia = 1;
        ib = NP;
    }
    for (i=0; i<NP+2; i++) { 
        for  (j=0; j<XRES; j++) {
            phinewsm[ip][i][j] = phioldsm[ip][i][j];
        }
    }
    while ((!*done)&&(it<niter)) {
        *done = 1;            /* suppose we are done */
        maxdiff = -1.0;
        for (i=ia; i<=ib; i++) {
            for  (j=0; j<XRES; j++) {
                switch (geom[ilevel][ip*NP+(i-1)][j]) {
                case 1:  /* Sides  */
                    phinewsm[ip][i][j] = (phinewsm[ip][i][j-1] + 
                                          phioldsm[ip][i][j+1]) / 2.0;
                    break;
                case 2: /* poles */
                    phinewsm[ip][i][j] = ( phioldsm[ip][i+1][j] + 
                             phinewsm[ip][i-1][j] + magn*dx ) / 2.0;
                    break;
                case 3: /* Corners */
                    phinewsm[ip][i][j] = ( phioldsm[ip][i+1][j] + 
                               phinewsm[ip][i-1][j] + phinewsm[ip][i][j-1] + 
                               phioldsm[ip][i][j+1] + magn*dx ) / 4.0;
                    break;
                case 4:
                    meshGSR(ip, i,j); 
                    break;
                }
                diff = phinewsm[ip][i][j] - phioldsm[ip][i][j];
                if (fabs(diff) > maxdiff) maxdiff=fabs(diff);
                if (fabs(diff) > eps ) *done = 0;
             }
        }
        if (!(*done)) {    /*store data to do one more iteration... */
            for (i=0; i<NP+2; i++) { 
                for  (j=0; j<XRES; j++) {
                    phioldsm[ip][i][j] = phinewsm[ip][i][j];
                }
            }
        }  /* end if */
        it = it + 1;
    }   /* end while */
}


void GaSeOnce(int *done, double *maxdiff)
{
    int i,j;
    double  diff;
     
    *done = 1;            /* suppose we are done */
    *maxdiff = -1.0;
    for (i=0; i<YRES; i++) {
        for  (j=0; j<XRES; j++) {
            switch (geom[ilevel][i][j]) {
            case 1:  /* Sides  */
                phinew[i][j] = (phinew[i][j-1] + phiold[i][j+1]) / 2.0;
                break;
            case 2: /* poles */
                phinew[i][j] = ( phiold[i+1][j] + phinew[i-1][j] +
                                 magn*dx ) / 2.0;
                break;
            case 3: /* Corners */
                phinew[i][j] = ( phiold[i+1][j] + phinew[i-1][j] +
                                 phinew[i][j-1] + phiold[i][j+1] +
                                 magn*dx ) / 4.0;
                break;
            case 4:
                meshGS(i,j);
                break;
            }
            diff = phinew[i][j] - phiold[i][j];
            if (fabs(diff) > *maxdiff) *maxdiff=fabs(diff);
            if (fabs(diff) > eps ) *done = 0;
        }   
    }        
    if (!(*done)) {    /*store data to do one more iteration... */
        for (i=0; i<YRES; i++) {
            for  (j=0; j<XRES; j++) {
                phiold[i][j] = phinew[i][j];
            }
        }
    }  /* end if */
/*    printf("Iteration %d   Diff %f \r", iter, maxdiff); */
    /* fflush(stdout); */
}

void meshGSR(int ip, int i, int j )
{
    int count;
    double sum;
    
    count = 0;
    sum = 0;
    if  ( (j-1) >= 0 ){
        sum = sum + phinewsm[ip][i][j-1];
        count = count +1;
    }
    if  ( (i-1) >= 0 ){
        sum = sum + phinewsm[ip][i-1][j];
        count = count +1;
    }
    if  ( (j+1) < XRES ){
        sum = sum + phioldsm[ip][i][j+1];
        count = count +1;
    }
    if  ( (i+1) <= NP+1 ){
        sum = sum + phioldsm[ip][i+1][j];
        count = count +1;
    }
    phinewsm[ip][i][j] = sum / count;
}

