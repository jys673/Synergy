/*-------------------------------------------------------------------------*/
/*  Client include file for the parallel version of BRA                    */
/*  solving Poissons eq. for a magnetic field                              */
/*  All global variables and function prototypes are here                  */
/*                                                                         */
/*  Varying iteration version                                              */
/*-------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <sng.h>

#define MAXNRES  256   /* Max Problem resolution */
#define MAXNP    60    /* Max. Grain size in rows   */
#define MAXTPL   10   /* maximum # of tuples */
#define MAXEXT   20000   /* maximum # of external iterations */

/* Function prototypes */
void GeomPrint(int il);
void Init_X(int xd, int yd, int w, char *title, int argc, char **argv);
void fill();
void RectangGeom(int il);
void relax();
void Setup(int type, int il);
void ShowDbl(int xr, int yr, int xd, int yd, double arr[MAXNRES][MAXNRES]);
void ReceiveStats();
void Solver(int type);
void PackBlock( int blstart, int TpRows );
void UnpackBlock();
void meshGS( int i, int j);

/* Arrays to be used. Phi holds data, geom holds geometry info */
/* See comments before geometry routines for details           */
double phiold[MAXNRES][MAXNRES], phinew[MAXNRES][MAXNRES];
double row[MAXNRES+1]; /* extra entry for visit number */
unsigned char geom[MAXNRES][MAXNRES], geombl[MAXNRES*MAXNRES];

double size,      /* length of the mesh side     */
       width,     /* width of the magnet         */
       length,    /* length of the magnet        */
       eps,       /* Convergence difference      */
       epscl,     /* Local client eps(ilon)      */
       magn,      /* Magnetization               */
       dx, dy;    /* Mesh unit length            */
double thres;

int   NRES, NROWS, NTPL, NPROC;   /* Resolution, chuck size, #of Tuples & proc */
int   XDIS, YDIS, XRES, YRES, gmtype, ilevel;

/* Tuple Space stuff */

int tplength, tsinp, tsout, tsglb, tsrows; 
int blindex, bllow, blhigh, blstart, blstop, status;
int reinserted, niter;

char tpname[80];
double  maxerr[MAXTPL];
int     locstarts[MAXTPL];
char   hname[80];

struct _assignment {
    int command;         /* Code for what operation is asked       */
    int blindex;         /* Block index                            */
    int bllow, blhigh;   /* Indexes of neighboring rows            */
    int blstart, blstop; /* Indexes of own border rows             */
    int niter;           /* Max number of internal iterations      */
    double thres;        /* Threshold                              */
    double block[MAXNP*MAXNRES]; /* Block data                     */
} asgn, result;

struct _miscellanea {
    int XRES, YRES;
    int NTPL, NROWS;
    int niter;
    double magn, dx, eps;
} misc;

struct _log {
    char hname[80];
    int  ExtIter;
    int  TotalIntern;
    int  InternIter[MAXEXT];
} tplog[MAXTPL];


