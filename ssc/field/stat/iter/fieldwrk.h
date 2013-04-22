/*-------------------------------------------------------------------------*/
/*  field.h                                                                */
/*  Main include file.. all global variables and function prototypes       */
/*  are here                                                               */
/*-------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <sng.h>

#define MAXNRES  256   /* Max Problem resolution */
#define MAXNP    60    /* Max. Grain size  256/5  */
#define MAXTPL   10   /* Maximum # of tuples */
#define MAXEXT   20000   /* maximum # of external iterations */


/* Function prototypes */
void GaSeI(int *done, double *err, int *iter);
void meshGSR(int i, int j );
void relax( int *iter );
void Solver(int type);
void GetAssign();
void GetBorder();
void GetMaxerror();
void InitLog();
void OutputBlock();
void ResubmitBlock();
void SubmitBorder();
void SubmitStats();
void BorderExch();
void PackBlock();
void UnpackBlock();
int  SystemConverged(double err);

/* Arrays to be used. Phi holds data, geom holds geometry info */
/* See comments before geometry routines for details           */
double phioldsm[MAXNP][MAXNRES], phinewsm[MAXNP][MAXNRES];
double row[MAXNRES+1]; /*extra entry for visit num*/
unsigned char geom[MAXNRES][MAXNRES], geombl[MAXNRES*MAXNRES];

double size,      /* length of the mesh side     */
       width,     /* width of the magnet         */
       length,    /* length of the magnet        */
       eps,       /* Convergence difference      */
       magn,      /* Magnetization               */
       dx, dy;    /* Mesh unit length            */
double thres;

int   NRES, NROWS, NTPL;   /* Resolution, chuck size, #of processors */
int   XDIS, YDIS, XRES, YRES, gmtype, ilevel, niter;


/* Tuple space stuff goes here */
int tsinp, tsout, tsglb, tsrows;
int tplength, blindex, bllow, blhigh, blstart, blstop, status, order;
int tpindex, locstart;

double maxnorm, maxdiff;

char   tpname[80];
double maxerr[MAXTPL];
char   hname[80];

struct _assignment {
    int command;
    int blindex;
    int bllow, blhigh;
    int blstart, blstop;
    int niter;
    double thres;
    double block[MAXNP*MAXNRES];
} asgn;
       
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
} tplog;

struct _brd {
    int up, down;
} border;
