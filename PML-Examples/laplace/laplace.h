/*
 * laplace.h
 *
 */

# include <stdio.h>
# include <time.h>
# include <math.h>

# define DIM         3         /* problem domain size in each direction */
# define MAXITER     2         /* maximum permitted iterations */
# define EPS         0.1       /* maximum allowed relative precision */
# define TOPBOUND    200.0       /* Top B.C. */
# define BOTBOUND    0.0         /* Bot B.C. */
# define LFTBOUND    0.0         /* Lft B.C. */
# define RITBOUND    0.0         /* Rit B.C. */
# define DEBUG       1           /* For initial debugging 1, 0 o.w. */
# define LOG         0           /* 1 to output data to profile, 0 o.w. */

# define GRAIN       2

int     rdim;             /* row dimension of the domain */
double  A[DIM+2][DIM+2];  /* maxm. size of the domain with boundaryd */

int     rowsize,          /* number of bytes in each row */
        iterno,           /* current iteration number */
        i,                /* loop indices */
        j;                /* loop indices */

double  resid,            /* residual */
        residtmp,         /* temp */
        Atmp;             /* temp */

