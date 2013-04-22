/*-------------------------------------------------------------------------*/
/*  field.h                                                                */
/*  Main include file.. all global variables and function prototypes       */
/*  are here                                                               */
/*-------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#define MAXNRES  256   /* Max Problem resolution */
#define MAXITER  100    /* Unused */
#define MAXP     10     /* Max. Number of processors */
#define MAXNP    200    /* Max. Grain size    */
#define MAXNN    2048   /* For the FFT    */

struct _pnt {
    int istart, istop;
} pnt[100];  /* Padding indexes */

/* Function prototypes */
void FftSolver(void);
void GaSe(int *iter);
void GaSeI(int ip, int niter, int *done);
void GeomPrint(int il);
void GaSeOnce(int *done, double *maxdiff);
void Init_X(int xd, int yd, int w, char *title, int argc, char **argv);
void Jacobi(int *iter);
void fill();
void four1(double *data, int nn, int isign);  
void InterMesh(int i, int j);
void Interpolate(int xr, int yr);
void meshGS(int i, int j);
void meshGSR(int ip, int i, int j );
void meshJ(int i, int j);
void multigr();
void Project(int xr, int yr);
void RectangGeom(int il);
void relax( int niter, int *iter );
void Setup(int type, int il);
void ShowDbl(int xr, int yr, int xd, int yd, double arr[MAXNRES][MAXNRES]);
void Solver(int type);
void Transpose(void);

/* Arrays to be used. Phi holds data, geom holds geometry info */
/* See comments before geometry routines for details           */
double phiold[MAXNRES][MAXNRES], phinew[MAXNRES][MAXNRES];
double phioldsm[MAXP][MAXNP][MAXNRES], phinewsm[MAXP][MAXNP][MAXNRES];
unsigned char geom[10][MAXNRES][MAXNRES];

double size,      /* length of the mesh side     */
       width,     /* width of the magnet         */
       length,    /* length of the magnet        */
       eps,       /* Convergence difference      */
       magn,      /* Magnetization               */
       dx, dy;    /* Mesh unit length            */


int   NRES, NP, P;   /* Resolution, chuck size, #of processors */
int   XDIS, YDIS, XRES, YRES, gmtype, ilevel;

struct _complex {
    double r, i;
} cmp[MAXNRES][MAXNRES];

