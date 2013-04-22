#include <stdio.h>
#include <math.h>

#define MAXNDIM  1000  /* Maximum Problem size*/
#define MAXP      10  /* Max Num of processors */
#define MAXNP     100  /* Max grain size */

double xold[MAXNDIM][MAXNDIM], xnew[MAXNDIM][MAXNDIM];
double xoldsm[MAXP][MAXNP][MAXNDIM], xnewsm[MAXP][MAXNP][MAXNDIM];
double eps;


void arrprint(), fill(), iterate( int ip, int iter, int *done );
void iteronce( int *done );
void relax(int niter), GS(int ip, int iter, int *done), GSonce( int *done );

int  P, NP, NDIM;

main ()
{
    int niter, p, np;

    eps = 0.0000001;
    for  (NDIM=4 ; NDIM<100; NDIM=NDIM+4) {
        printf("Problem size : %dx%d \n", NDIM, NDIM);
        P=4;
        NP = NDIM/4;
        for  (niter=0; niter<20; niter++) {
            fill();
            relax(niter);
        }
        printf("&\n");
    }
}

void fill()
{
    int i,j;
    for  (i=0; i<NDIM; i++){
        for (j=0; j<NDIM; j++){
            xold[i][j] = j;
            xnew[i][j] = 0.0;
        }
    }
}

void arrprint()
{
    int i,j;
    for  (i=0; i<NDIM; i++){
        for (j=0; j<NDIM; j++){
            printf(" %4.2f",xold[i][j]);
        }
        printf("\n");
    }
}

void relax(int niter)
{
    int  iter, ip, i,j, done;

    done = 0;
    iter = 0;
    while (!done) {
        for  (ip=0; ip<P; ip++){
            for  (i=0; i<NP; i++) {
                for (j=0; j<NDIM; j++) {
                    xoldsm[ip][i][j] = xold[ip*P+i][j];
                }
            }
/*            iterate(ip, niter, &done);*/
            GS(ip, niter, &done);
        }
        for  (ip=0; ip<P; ip++){
            for  (i=0; i<NP; i++) {
                for (j=0; j<NDIM; j++) {
                    xold[ip*P+i][j] = xoldsm[ip][i][j];
                }
            }
        }
/*        iteronce(&done); */
        GSonce(&done);
/**
        printf("Relaxation %d \n", iter);
        arrprint();
**/
        iter = iter + niter + 1;
    }
    printf(" %d   %d \n", niter, iter);
}

void iteronce( int *done)
{
    int i,j;
    double count, sum, diff, maxdiff;

    count = 0;
    *done = 1;            /* suppose we are done */
    maxdiff = -1.0;
    for (i=0; i<NDIM; i++) {
        for  (j=0; j<NDIM; j++) {
            count = 0;
            sum = 0;
            if  ( (j-1) >= 0 ){
                sum = sum + xold[i][j-1];
                count = count +1;
            }
            if  ( (i-1) >= 0 ){
                sum = sum + xold[i-1][j];
                count = count +1;
            }
            if  ( (j+1) < NDIM ){
                sum = sum + xold[i][j+1];
                count = count +1;
            }
            if  ( (i+1) < NDIM ){
                sum = sum + xold[i+1][j];
                count = count +1;
            }
            xnew[i][j] = sum / count;
            diff = xnew[i][j] - xold[i][j];
/**
            if ((i==0)&&(j==0)) 
               printf("Sum %f  Count %f xnew %f xold %f  Diff %f  Eps %f\n", 
                      sum, count, xnew[i][j], xold[i][j], diff, eps);
**/
            if (fabs(diff) > maxdiff) maxdiff=fabs(diff);
            if (fabs(diff) > eps ) *done = 0;
        }
    }
    if (!(*done)) {    /*store data to do one more iteration... */
        for (i=0; i<NDIM; i++) {
            for  (j=0; j<NDIM; j++) {
                xold[i][j] = xnew[i][j];
            }
        }
    }
}

void iterate(int ip, int niter, int *done)
{
    int i,j, iter;
    double count, sum, diff, maxdiff;

    *done = 0;
    iter = 0;
    while ((!(*done))&&(iter<niter)) {
        count = 0;
        *done = 1;            /* suppose we are done */
        maxdiff = -1.0;
        for (i=0; i<NP; i++) {
            for  (j=0; j<NDIM; j++) {
                count = 0;
                sum = 0;
                if  ( (j-1) >= 0 ){
                    sum = sum + xoldsm[ip][i][j-1];
                    count = count +1;
                }
                if  ( (i-1) >= 0 ){
                    sum = sum + xoldsm[ip][i-1][j];
                    count = count +1;
                }
                if  ( (j+1) < NDIM ){
                    sum = sum + xoldsm[ip][i][j+1];
                    count = count +1;
                }
                if  ( (i+1) < NP ){
                    sum = sum + xoldsm[ip][i+1][j];
                    count = count +1;
                }
                xnewsm[ip][i][j] = sum / count;
                diff = xnewsm[ip][i][j] - xoldsm[ip][i][j];
                if (fabs(diff) > maxdiff) maxdiff=fabs(diff);
                if (fabs(diff) > eps ) *done = 0;
            }
        }
        if (!(*done)) {    /*store data to do one more iteration... */
            for (i=0; i<NP; i++) {
                for  (j=0; j<NDIM; j++) {
                    xoldsm[ip][i][j] = xnewsm[ip][i][j];
                }
            }
        }
        iter++;
/*        printf("Iteration %d   Max diff %f \n", iter, maxdiff);  */
    }
}

void GSonce( int *done)
{
    int i,j;
    double count, sum, diff, maxdiff;

    count = 0;
    *done = 1;            /* suppose we are done */
    maxdiff = -1.0;
    for (i=0; i<NDIM; i++) {
        for  (j=0; j<NDIM; j++) {
            count = 0;
            sum = 0;
            if  ( (j-1) >= 0 ){
                sum = sum + xnew[i][j-1];
                count = count +1;
            }
            if  ( (i-1) >= 0 ){
                sum = sum + xnew[i-1][j];
                count = count +1;
            }
            if  ( (j+1) < NDIM ){
                sum = sum + xold[i][j+1];
                count = count +1;
            }
            if  ( (i+1) < NDIM ){
                sum = sum + xold[i+1][j];
                count = count +1;
            }
            xnew[i][j] = sum / count;
            diff = xnew[i][j] - xold[i][j];
/**
            if ((i==0)&&(j==0)) 
               printf("Sum %f  Count %f xnew %f xold %f  Diff %f  Eps %f\n", 
                      sum, count, xnew[i][j], xold[i][j], diff, eps);
**/
            if (fabs(diff) > maxdiff) maxdiff=fabs(diff);
            if (fabs(diff) > eps ) *done = 0;
        }
    }
    if (!(*done)) {    /*store data to do one more iteration... */
        for (i=0; i<NDIM; i++) {
            for  (j=0; j<NDIM; j++) {
                xold[i][j] = xnew[i][j];
            }
        }
    }
}

void GS(int ip, int niter, int *done)
{
    int i,j, iter;
    double count, sum, diff, maxdiff;

    *done = 0;
    iter = 0;
    while ((!(*done))&&(iter<niter)) {
        count = 0;
        *done = 1;            /* suppose we are done */
        maxdiff = -1.0;
        for (i=0; i<NP; i++) {
            for  (j=0; j<NDIM; j++) {
                count = 0;
                sum = 0;
                if  ( (j-1) >= 0 ){
                    sum = sum + xnewsm[ip][i][j-1];
                    count = count +1;
                }
                if  ( (i-1) >= 0 ){
                    sum = sum + xnewsm[ip][i-1][j];
                    count = count +1;
                }
                if  ( (j+1) < NDIM ){
                    sum = sum + xoldsm[ip][i][j+1];
                    count = count +1;
                }
                if  ( (i+1) < NP ){
                    sum = sum + xoldsm[ip][i+1][j];
                    count = count +1;
                }
                xnewsm[ip][i][j] = sum / count;
                diff = xnewsm[ip][i][j] - xoldsm[ip][i][j];
                if (fabs(diff) > maxdiff) maxdiff=fabs(diff);
                if (fabs(diff) > eps ) *done = 0;
            }
        }
        if (!(*done)) {    /*store data to do one more iteration... */
            for (i=0; i<NP; i++) {
                for  (j=0; j<NDIM; j++) {
                    xoldsm[ip][i][j] = xnewsm[ip][i][j];
                }
            }
        }
        iter++;
/*        printf("Iteration %d   Max diff %f \n", iter, maxdiff);  */
    }
}

