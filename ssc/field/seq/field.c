/*----------------------------------------------------------------------*/
/* This program solves Poisson's (&Laplaces's) equations for a magnetic */
/* field, using iterative methods:                                      */
/*    1. Jacobi                                                         */
/*    2. Gauss-Seidel                                                   */
/*    3. Successive Overrelaxation                                      */
/*----------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>

/*#define MAXNRES  1000 i*/
#define MAXNRES  255    /* Max Problem resolution */
#define MAXITER  100    /* Unused */
#define MAXP     10     /* Max. Number of processors */
#define MAXNP    100    /* Max. Grain size    */

/* Function prototypes */
void Setup(int type);
void Jacobi(int *done, int *iter);
void meshJ(int i, int j);
void GaSe(int *done, int *iter);
void meshGS(int i, int j);
void RectangGeom(void);
void Solver(int type);
void GeomPrint(void);
void GSOnce(int *done);
void relax( int niter );

/* Arrays to be used. Phi holds data, geom holds geometry info */
/* See comments before geometry routines for details           */
double phiold[MAXNRES][MAXNRES], phinew[MAXNRES][MAXNRES];
double phioldsm[MAXP][MAXNP][MAXNRES], phinewsm[MAXP][MAXNP][MAXNRES];
unsigned char geom[MAXNRES][MAXNRES];

double size,      /* length of the mesh side     */
       width,     /* width of the magnet         */
       length,    /* length of the magnet        */
       eps,       /* Convergence difference      */
       magn,      /* Magnetization               */
       dx, dy;    /* Mesh unit length            */

int   NRES, NP, P;   /* Resolution, chuck size, #of processors */
int   XDIS, YDIS;
void main(int argc, char **argv)
{
    int done;
    FILE *fp;

    if  ((fp=fopen("field.cfg","r"))==NULL) {
        perror("Cannot open field.cfg:");
        exit(-1);
    }
    fscanf(fp, "%d %d ", &XDIS, &YDIS);
    fscanf(fp, "%d", &NRES);    
    fscanf(fp, "%lf", &size);
    fscanf(fp, "%lf", &magn );     
    fscanf(fp, "%lf", &eps );     
    fscanf(fp, "%lf %lf", &width, &length);    

    printf("%d %d \n", XDIS, YDIS);
    printf("%d \n", NRES);    
    printf("%f \n", size);
    printf("%f \n", magn );     
    printf("%f \n", eps );     
    printf("%f %f \n", width, length);


    Init_X( XDIS, YDIS, 10, "Magnetic Field",argc, argv );
    dx = size / (float)(NRES-1);
    dy = size / (float)(NRES-1);
/*----------------------------------------------------------------------*/
/*  The argument of Setup indicates the geometry of the magnet          */
/*      1 - Rectangular magnet                                          */
/*----------------------------------------------------------------------*/

    Setup(1);
    ShowDbl(NRES, NRES, XDIS, YDIS, phiold);
    printf("Setup is done \n");
    Solver(2);
    ShowDbl(NRES, NRES, XDIS, YDIS, phiold);
    done = 0;
    do {
       EventH(&done, XDIS, YDIS);
    } while (!done);
}


void Setup( int type )
{
    if (type==1) { /* Rectangular magnet */
        RectangGeom();
    }
/*  GeomPrint(); */
}

void GeomPrint(void)
{
    int i,j;

    for  (i=0; i<NRES; i++) {
        for (j=0; j<NRES; j++) {
            printf(" %d", geom[i][j]);
        }
        printf("\n");
    }
}
/*-----------------------------------------------------------------------*/
/*  RectangGeom function generates the geometry for a rectangular magnet */
/*  The mesh is divided in the following 5 regions :                     */
/*     0 - Inside the magnet                                             */
/*     2 - North and south poles (magnetic sides)                        */
/*     1 - Non-magnetic sides                                            */
/*     4 - Mesh points outside the magnet                                */
/*     5 - Border of the mesh (must be 0 at all times)                   */
/*     3 - Corners of the magnet                                         */
/*-----------------------------------------------------------------------*/
void RectangGeom(void)
{
    int i, j, horiz, vert;
    float x, y, xleft, xright, ytop, ybot;

    xleft = size/2.0 - width/2.0;
    xright = size/2.0 + width/2.0;
    ytop = size/2.0 - length/2.0;
    ybot = size/2.0 + length/2.0;
/*---------------------------------------------------------------------*/
/*  Set border to 4. Then scan the mesh and put 4 outside the box and  */
/*  0 inside.                                                          */
/*---------------------------------------------------------------------*/
    for  (i=0; i<NRES; i++) {
        geom[i][0] = 5;
        geom[i][NRES-1] = 5;
        geom[0][i] = 5;
        geom[NRES-1][i] = 5;
    }

    y = dy;
    for (i=1; i<NRES-1; i++ ){
        x=dx;                /* The border has already been assigbned */
        for  (j=1; j<NRES-1; j++ ) {
            geom[i][j] = 4;
            if  (  ( (x>xleft) && (x<xright) ) &&
                   ( (y>ytop) &&  (y<ybot) ) ) geom[i][j] = 0;
            x += dx;
        }
        y += dy;
    }
/*---------------------------------------------------------------------*/
/*  Scan the mesh to locate the border of the magnet                   */
/*---------------------------------------------------------------------*/
    for  (i=1; i<NRES-1; i++) {
        horiz = 0;
        vert = 0;
        for  (j=1; j<NRES-1; j++) {
            if (!horiz) {
                if  ((geom[i][j]==4)&&(geom[i][j+1]==0)) {
                    geom[i][j] = 1;
                    horiz = 1;
                }
            } else {
                if  ((geom[i][j-1]==0)&&(geom[i][j]==4)) {
                    geom[i][j] = 1;
                    horiz = 0;
                }
            }
/* The vertical scan */
            if (!vert) {
                if  ((geom[j][i]==4)&&(geom[j+1][i]==0)) {
                    geom[j][i] = 2;
                    vert = 1;
                }
            } else {
                if  ((geom[j-1][i]==0)&&(geom[j][i]==4)) {
                    geom[j][i] = 2;
                    vert = 0;
                }
            }
        }
    }
/* Scan for the corners */
    for  (i=1; i<NRES-1; i++) {
        for  (j=1; j<NRES-1; j++) {
            if  ((geom[i][j+1]==2) && (geom[i+1][j]==1) && 
                 (geom[i][j]==4)) geom[i][j] =3;
            if  ((geom[i][j-1]==2) && (geom[i+1][j]==1) &&
                 (geom[i][j]==4)) geom[i][j] =3;
            if  ((geom[i][j-1]==2) && (geom[i-1][j]==1) &&
                 (geom[i][j]==4)) geom[i][j] =3;
            if  ((geom[i][j+1]==2) && (geom[i-1][j]==1) &&
                 (geom[i][j]==4)) geom[i][j] =3;
        }
    }
/*------------------------------------------------------------------------*/
/*  Setup the magnetic values                                             */
/*------------------------------------------------------------------------*/
    for  (i=0; i<NRES; i++) {
        for (j=0; j<NRES; j++) {
            switch (geom[i][j]) {
            case 0:
                phiold[i][j] = 0.0;
                break; 
            case 1:
                phiold[i][j] = 10.0;
                break; 
            case 2:
                phiold[i][j] = 20.0;
                break; 
            case 3:
                phiold[i][j] = 30.0;
                break; 
            case 4:
                phiold[i][j] = 250.0;
                break; 
            case 5:        /*outer border of mesh */
                phiold[i][j] = 0.0;
                break; 
            }
/*            phiold[i][j] = 100; */
        }
    }
}

void Solver(int type)
{
    int niter, done, iter;
    
    done = 0;
    iter = 0;
    switch (type) {
    case 1:
        Jacobi(&done, &iter );
        printf("Jacobi: Size %dx%d  Iterations %d \n", NRES, NRES, iter);
        break;     
    case 2:
        GaSe(&done, &iter );
        printf("Gauss-Seidel: Size %dx%d  Iterations %d \n", 
               NRES, NRES, iter);
        break;     
    case 3:
        niter = 5; /* K: number of iterations before dataflow relaxing */
        P = 5;    /* Number of processors */
        NP = NRES / P;  /* chunk size */
        relax(niter, &iter);
        printf("Gauss-Seidel with a dataflow relaxation:");
        printf("Size %dx%d  Iterations %d   k %d \n", 
                NRES, NRES, iter, niter);
        break;
    }
}

void GaSe(int *done, int *iter)
{
    int i,j;
    double  maxdiff, diff;

    *done = 0;
    while (!*done) {
        *done = 1;            /* suppose we are done */
        maxdiff = -1.0;
        for (i=0; i<NRES; i++) {
            for  (j=0; j<NRES; j++) {
                switch (geom[i][j]) {
                case 1:  /* Sides  */
                    phinew[i][j] = (phinew[i][j-1] + phiold[i][j+1]) / 2.0;
                    break;
                case 2: /* poles */
                    phinew[i][j] = ( phinew[i+1][j] + phiold[i-1][j] + 
                                     magn*dx ) / 2.0;
                    break;
                case 3: /* Corners */
                    phinew[i][j] = ( phiold[i+1][j] + phinew[i-1][j] + 
                                     phinew[i][j-1] + phiold[i][j+1] + 
                                     magn*dx ) / 2.0;
                    break;
                case 4:
                    meshGS(i,j);
                    break;
                }
                diff = phinew[i][j] - phiold[i][j];
                if (fabs(diff) > maxdiff) maxdiff=fabs(diff);
                if (fabs(diff) > eps ) *done = 0;
             }
        }  
        if (!(*done)) {    /*store data to do one more iteration... */
            for (i=0; i<NRES; i++) {
                for  (j=0; j<NRES; j++) {
                    phiold[i][j] = phinew[i][j];
                }
            }
        }  /* end if */
        *iter = *iter+1;
        printf("Iteration %d   Diff %f \r", *iter, maxdiff);
        /* fflush(stdout); */
        if  (!(*iter%100)) ShowDbl(NRES, NRES, XDIS, YDIS, phiold); 
    }   /* end while */
}

void Jacobi(int *done, int *iter)
{
    int i,j;
    double  maxdiff, diff;

    *done = 0;
    while (!*done) {
        *done = 1;            /* suppose we are done */
        maxdiff = -1.0;
        for (i=0; i<NRES; i++) {
            for  (j=0; j<NRES; j++) {
                switch (geom[i][j]) {
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
                                     magn*dx ) / 2.0;
                    break;
                case 4:
                    meshJ(i,j);
                    break;
                }
                diff = phinew[i][j] - phiold[i][j];
                if (fabs(diff) > maxdiff) maxdiff=fabs(diff);
                if (fabs(diff) > eps ) *done = 0;
             }
        }  
        if (!(*done)) {    /*store data to do one more iteration... */
            for (i=0; i<NRES; i++) {
                for  (j=0; j<NRES; j++) {
                    phiold[i][j] = phinew[i][j];
                }
            }
        }  /* end if */
        *iter = *iter+1;
        printf("Iteration %d   Diff %f \r", *iter, maxdiff);
        fflush(stdout);
        ShowDbl(NRES, NRES, XDIS, YDIS, phiold); 
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
    if  ( (j+1) < NRES ){
        sum = sum + phiold[i][j+1];
        count = count +1;
    }
    if  ( (i+1) < NRES ){
        sum = sum + phiold[i+1][j];
        count = count +1;
    }
    phinew[i][j] = sum / count;
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
    if  ( (j+1) < NRES ){
        sum = sum + phiold[i][j+1];
        count = count +1;
    }
    if  ( (i+1) < NRES ){
        sum = sum + phiold[i+1][j];
        count = count +1;
    }
    phinew[i][j] = sum / count;
}


void relax(int niter, int *iter)
{
    int  iter, ip, i,j, done;

    done = 0;
    *iter = 0;
    while (!done) {
        for  (ip=0; ip<P; ip++){
            for  (i=0; i<NP; i++) {
                for (j=0; j<NDIM; j++) {
                    phioldsm[ip][i][j] = phiold[ip*P+i][j];
                }
            }
            GaSeI(ip, niter, &done);
        }
        for  (ip=0; ip<P; ip++){
            for  (i=0; i<NP; i++) {
                for (j=0; j<NDIM; j++) {
                    phiold[ip*P+i][j] = phioldsm[ip][i][j];
                }
            }
        }
        GSonce(&done);
        *iter = *iter + niter + 1;
    }
}


void GaSeI(int ip, int niter, int *done)
{
    int i,j, iter;
    double  maxdiff, diff;

    *done = 0;   
    iter = 0;
    while ((!*done)&&(iter<niter)) {
        *done = 1;            /* suppose we are done */
        maxdiff = -1.0;
        for (i=0; i<NP; i++) {
            for  (j=0; j<NRES; j++) {
                switch (geom[i][j]) {
                case 1:  /* Sides  */
                    phinewsm[ip][i][j] = (phinewsm[ip][i][j-1] + 
                                          phioldsm[ip][i][j+1]) / 2.0;
                    break;
                case 2: /* poles */
                    phinewsm[ip][i][j] = ( phinewsm[ip][i+1][j] + 
                             phioldsm[ip][i-1][j] + magn*dx ) / 2.0;
                    break;
                case 3: /* Corners */
                    phinewsm[ip][i][j] = ( phioldsm[ip][i+1][j] + 
                               phinewsm[ip][i-1][j] + phinewsm[ip][i][j-1] + 
                               phioldsm[ip][i][j+1] + magn*dx ) / 2.0;
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
            for (i=0; i<NRES; i++) { 
                for  (j=0; j<NRES; j++) {
                    phioldsm[ip][i][j] = phinewsm[ip][i][j];
                }
            }
        }  /* end if */
        iter = iter+1;  
        printf("Iteration %d   Diff %f \r", *iter, maxdiff);
        /* fflush(stdout); */
        if  (!(iter%100)) ShowDbl(NRES, NRES, XDIS, YDIS, phiold);   
    }   /* end while */
}


void GaSeOnce(int *done)
{
    int i,j;
    double  maxdiff, diff;
     
    *done = 1;            /* suppose we are done */
    maxdiff = -1.0;
    for (i=0; i<NRES; i++) {
        for  (j=0; j<NRES; j++) {
            switch (geom[i][j]) {
            case 1:  /* Sides  */
                phinew[i][j] = (phinew[i][j-1] + phiold[i][j+1]) / 2.0;
                break;
            case 2: /* poles */
                phinew[i][j] = ( phinew[i+1][j] + phiold[i-1][j] +
                                 magn*dx ) / 2.0;
                break;
            case 3: /* Corners */
                phinew[i][j] = ( phiold[i+1][j] + phinew[i-1][j] +
                                 phinew[i][j-1] + phiold[i][j+1] +
                                 magn*dx ) / 2.0;
                break;
            case 4:
                meshGS(i,j);
                break;
            }
            diff = phinew[i][j] - phiold[i][j];
            if (fabs(diff) > maxdiff) maxdiff=fabs(diff);
            if (fabs(diff) > eps ) *done = 0;
        }   
    }        
    if (!(*done)) {    /*store data to do one more iteration... */
        for (i=0; i<NRES; i++) {
            for  (j=0; j<NRES; j++) {
                phiold[i][j] = phinew[i][j];
            }
        }
    }  /* end if */
    printf("Iteration %d   Diff %f \r", *iter, maxdiff);
    /* fflush(stdout); */
}

void meshGSR(itnt ip, int i, int j )
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
        sum = sum + phinew[ip][i-1][j];
        count = count +1;
    }
    if  ( (j+1) < NRES ){
        sum = sum + phiold[ip][i][j+1];
        count = count +1;
    }
    if  ( (i+1) < NRES ){
        sum = sum + phiold[ip][i+1][j];
        count = count +1;
    }
    phinew[ip][i][j] = sum / count;
}

