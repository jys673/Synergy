/*----------------------------------------------------------------------*/
/* This program solves Poisson's (&Laplaces's) equations for a magnetic */
/* field, using iterative methods:                                      */
/*    1. Jacobi                                                         */
/*    2. Gauss-Seidel                                                   */
/*    3. BRA on Gauss-Seidel                                            */
/*  Main module                                                         */
/*----------------------------------------------------------------------*/
#include "field.h"
#include <sng.h>
double wall_clock();
FILE *ofd;

void main(int argc, char **argv)
{
    int done, slvtype, i, j;
    FILE *fp;
    char fname[80];
    double dt, t0, t1;

    if  ((fp=fopen("field.cfg","r"))==NULL) {
        perror("Cannot open field.cfg:");
        exit(-1);
    }
    fscanf(fp, "%d %d ", &XDIS, &YDIS);
    fscanf(fp, "%d", &NRES);    
    fscanf(fp, "%d", &gmtype);
    fscanf(fp, "%d", &slvtype);
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


    XRES = YRES = NRES;
    /****Init_X( XDIS, YDIS, 10, "Magnetic Field",argc, argv );****/
    dx = size / (float)(XRES-1);
    dy = size / (float)(YRES-1);
/*----------------------------------------------------------------------*/
/*  The argument of Setup indicates the geometry of the magnet          */
/*      1 - Rectangular magnet                                          */
/*----------------------------------------------------------------------*/
    sprintf(fname,"fldsq%d.time", NRES);
    if  ((ofd=fopen(fname, "a"))==NULL){
        perror("Can't open field.time");
        exit(-1);
    }
    t0 = wall_clock();
    Solver(slvtype);
    t1 = wall_clock();
    dt = (t1-t0)/1000000.0;
    fprintf(ofd, "Elapsed time %f \n", dt);
    fclose(ofd);

    for (i=0; i<YRES; i++) {
        for  (j=0; j<XRES; j++) {
            phinew[i][j] = log(1.0+ phiold[i][j]);
        }
    }
/****
    ShowDbl(XRES, YRES, XDIS, YDIS, phinew);

    done = 0;
    do {
       EventH(&done, XDIS, YDIS);
    } while (!done);
****/
}


void Setup( int gmtype, int il )
{
    if (gmtype==1) { /* Rectangular magnet */
        RectangGeom(il);
    }
/*  GeomPrint(); */
}


void Solver(int slvtype)
{
    int niter, iter;
    char hostname[128];
    
    iter = 0;
    ilevel = 0;   /* all except multigrid */
    switch (slvtype) {
    case 1:
        Setup(gmtype, ilevel);
        fill();
        /***ShowDbl(XRES, YRES, XDIS, YDIS, phiold);***/
        printf("Setup is done \n");

        printf("Jacobi Solver.\n");
        printf("Problem size %dx%d  \n", XRES, YRES);
        Jacobi(&iter );
        printf("Total Iterations %d \n", iter);
	gethostname(hostname,sizeof(hostname));
	printf("Host (%s)\n", hostname);
        break;     
    case 2:
        Setup(gmtype, ilevel);
        fill();
/****        ShowDbl(XRES, YRES, XDIS, YDIS, phiold); ****/
        printf("Setup is done \n");

        printf("Gauss-Seidel Solver \n");
        printf("Problem Size %dx%d  \n", XRES, YRES);

        fprintf(ofd, "Gauss-Seidel Solver \n");
        fprintf(ofd, "Problem Size %dx%d  \n", XRES, YRES);
        GaSe(&iter );
        printf("Total Iterations %d \n", iter);
        fprintf(ofd, "Total Iterations %d \n", iter);
	gethostname(hostname,sizeof(hostname));
	printf("Host (%s)\n", hostname);
	fprintf(ofd, "Host=(%s)\n", hostname);
        break;     
    case 3:
        Setup(gmtype, ilevel);
        fill();
        /***ShowDbl(XRES, YRES, XDIS, YDIS, phiold);***/
        printf("Setup is done \n");

        niter =9;   /* K: number of iterations before dataflow relaxing */
        P = 4;          /* Number of processors */
        NP = XRES / P;  /* chunk size */
        printf("Gauss-Seidel with a dataflow relaxation:");
        printf("Problem Size %dx%d \n", XRES, YRES);
        printf("Processors %d,  dataflow, every %d iterations \n", P, niter);
        relax(niter, &iter);
        printf("Total Iterations %d \n", iter);
        break;
    case 4:
        Setup(gmtype, ilevel);
        fill();
        /***ShowDbl(XRES, YRES, XDIS, YDIS, phiold);***/
        printf("Setup is done \n");

        FftSolver();
        break;
    case 5: /* multigrid */
        printf("MultiGrid Solver \n");
        XRES = YRES = 32;
        for  (ilevel=0; ilevel<5; ilevel++) {
            dx = size / (float)(XRES-1);
            dy = size / (float)(YRES-1);
            Setup(gmtype, ilevel);
            XRES = 2*XRES;
            YRES = XRES;
        }
        ilevel = 0;
        YRES = XRES = 32;
        printf("Problem Size %dx%d  \n", XRES, YRES);
        fill();
        /***ShowDbl(XRES, YRES, XDIS, YDIS, phiold);***/
        printf("Setup is done \n");

        multigr();
        break;
    }
}



void relax(int niter, int *iter)
{
    int  ip, i,j, done;
    double maxdiff;

    done = 0;
    *iter = 0;
/**/
/* Fill the sub arrays and introduce padding */
/**/
    while (!done) {
        for  (ip=0; ip<P; ip++){
            for  (i=0; i<NP; i++) {
                for (j=0; j<XRES; j++) {
                    phioldsm[ip][i+1][j] = phiold[ip*NP+i][j];
                }
            }
            if (ip==0) {
                for  (j=0; j<XRES; j++) {
                    phioldsm[0][0][j] = phiold[0][j];
                }
            } else {
                for  (j=0; j<XRES; j++) {
                    phioldsm[ip][0][j] = phiold[ip*NP-1][j];
                }
            }
            if (ip==(P-1)) {
                for  (j=0; j<XRES; j++) {
                    phioldsm[P-1][NP+1][j] = phiold[XRES-1][j];
                }
            } else {
                for  (j=0; j<XRES; j++) {
                    phioldsm[ip][NP+1][j] = phiold[(ip+1)*NP][j];
                }
            }

        }
        for  (ip = 0; ip<P; ip++) {
            GaSeI(ip, niter, &done );  
        }
        for  (ip=0; ip<P; ip++){
            for  (i=0; i<NP; i++) {
                for (j=0; j<XRES; j++) {
                    phiold[ip*NP+i][j] = phioldsm[ip][i+1][j];
                }
            }
        }
        maxdiff = 0;
        GaSeOnce(&done, &maxdiff); 
        *iter = *iter + niter + 1;
        if  (!(*iter%100)) {
            printf("Iteration %d Difference %f\r", *iter, maxdiff);
            fflush(stdout);
            for (i=0; i<YRES; i++) {
                for  (j=0; j<XRES; j++) {
                    phinew[i][j] = log(1.0+ phiold[i][j]);
                }
            }
            /***ShowDbl(XRES, YRES, XDIS, YDIS, phinew);***/
        }
    }
}

