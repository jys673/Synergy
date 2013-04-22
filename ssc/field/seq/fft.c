#include "field.h"


void FftSolver()
{
    double dta[1024];
    int i,j, isign, width;
    struct _complex temp;

    isign = 1;
/* */
/* Fill in the complex array  */
/* */
    printf("Filling array Nres = %d \n", XRES);
    for  (i=0; i<YRES; i++) {
        for (j=0; j<XRES; j++) {
            cmp[i][j].r = 0.0;
            cmp[i][j].i = 0.0;
        }
    }
    for  (i=YRES/2-2; i<YRES/2+2; i++) {
        for (j=XRES/2-2; j<XRES/2+2; j++) {
            cmp[i][j].r = 10000.0;
            cmp[i][j].i = 0.0;
        }
    }
/* */
/* Fill in phiold and display it */
/* */
    for  (i=0; i<YRES; i++) {
        for  (j=0; j<XRES; j++) {
            phiold[i][j] = sqrt(pow(cmp[i][j].r,2.0) + pow(cmp[i][j].i,2.0));
        }
    }
    /***ShowDbl( XRES, YRES, XDIS, XDIS, phiold );***/

/* */
/* Transform the rows   */
/* */
    printf("Transforming rows \n");
    for  (i=0; i<YRES; i++) {
        four1( cmp[i], XRES, isign);
    }
/* */
/* Transpose and FTT again the new rows */
/* */
    printf("Transposing \n");
     for  ( i=0; i<XRES; i++ ) {
         for  ( j=i+1; j<YRES; j++ ) {
              temp = cmp[i][j];
              cmp[i][j] = cmp[j][i];
              cmp[j][i] = temp;
         }
     }

/*    Transpose(); */
    printf("Transforming columns \n");
    for  (i=0; i<YRES; i++) {
        four1( cmp[i], XRES, isign);
    }
    width = 512;
/*--------------------------------------------------------------------*/
/*    Now put the values in their proper places                       */
/*--------------------------------------------------------------------*/
    for  ( i=0; i<width; i++ ) {
        for  (j=0; j<width/2; j++ ) {
            temp.r = cmp[i][j].r;
            temp.i = cmp[i][j].i;
            cmp[i][j].r = cmp[i][j+width/2].r;
            cmp[i][j].i = cmp[i][j+width/2].i;
            cmp[i][j+width/2].r = temp.r;
            cmp[i][j+width/2].i = temp.i;
        }
    }
    for  ( i=0; i<width; i++ ) {
        for  (j=0; j<width/2; j++ ) {
            temp.r = cmp[j][i].r;
            temp.i = cmp[j][i].i;
            cmp[j][i].r = cmp[j+width/2][i].r;
            cmp[j][i].i = cmp[j+width/2][i].i;
            cmp[j+width/2][i].r = temp.r;
            cmp[j+width/2][i].i = temp.i;
        }
    }
/* */
/* Fill in phiold and sisplay it */
/* */
    printf("Displaying Results \n");
    for  (i=0; i<YRES; i++) {
        for  (j=0; j<XRES; j++) {
            phiold[i][j] = sqrt(pow(cmp[i][j].r,2.0) + pow(cmp[i][j].i,2.0));
            phiold[i][j] = log(1.0 + phiold[i][j]);
        }
    }
}

void Transpose()
{
    int i, j;
    struct _complex temp;
 
    for  (i=0; i<YRES-1; i++) {
        for (j=0; j<XRES; j++){
            temp.r = cmp[i][j].r;
            temp.i = cmp[i][j].i;
            cmp[i][j].r = cmp[j][i].r;
            cmp[i][j].i = cmp[j][i].i;
            cmp[j][i].r = temp.r;
            cmp[j][i].i = temp.i;
        }
    }
}

/*-------------------------------------------------------------------------*/
/*  FOUR1()                                                                */
/*    Replaces DTA array by its discreet Fourier Transform ( isign=1 )     */
/*    or with nn times its inverse Fourier transform ( isign=-1 ). Dta is  */
/*    a complex array of length n, or a real array of length 2*nn          */
/*-------------------------------------------------------------------------*/
void four1( double *dta, int nn, int isign)
{
    int  n, i, j, m, nmax, istep;
    double  wr, wi, wpr, wpi, wtemp, theta, tempr, tempi;
    double flops;

    flops = 0;
    n = 2 * nn;
    j = 1;
    for  ( i=1; i<=n; i+=2 ) {
        if ( j > i ) {
            tempr = dta[j-1];
            tempi = dta[j+1-1];
            dta[j-1] = dta[i-1];
            dta[j+1-1] = dta[i+1-1];
            dta[i-1] = tempr;
            dta[i+1-1] = tempi;
            flops = flops +3;
        }
        m = n/2;
        while ( (m>=2) && (j>m) ) {
            j = j - m;
            m = m/2;
            flops = flops +2;
        }  
        j = j + m;
        flops = flops + 1;
    } 
    nmax = 2;
    while ( n > nmax ) {
        istep = 2 * nmax;
/*        printf("Step %d   n %d    nmax %d \n", istep, n, nmax );   */
        theta = 6.28318530717959/((double)(isign * nmax));
        wpr = -2.0*sin( 0.5 * theta )*sin( 0.5 * theta );
        wpi = sin( theta );
        wr = 1.0;
        wi = 0.0;
        flops = flops + 20;
        for  ( m=1; m<=nmax; m +=2 ) {
            for  ( i=m; i<=n; i +=istep ) {
                j = i + nmax;
                tempr = wr * dta[j-1]-wi*dta[j+1-1];
                tempi = wr * dta[j+1-1]+wi*dta[j-1];
                dta[j-1] = dta[i-1] - tempr;
                dta[j+1-1] = dta[i+1-1] - tempi;
                dta[i-1] = dta[i-1]+tempr;
                dta[i+1-1] = dta[i+1-1]+tempi;
                flops = flops+11;
            }
            wtemp = wr;
            wr = wr * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
            flops = flops + 8;
        }
        nmax = istep;
    } 
    printf("FFT: N=%d  Flops = %f \n", nn, flops);
}





