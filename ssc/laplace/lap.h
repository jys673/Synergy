/*
 * lap.h
 *
 * This is the header file used by lap.c, lapclnt.c, and lapwrk.c
 *
 * Update History:
 *        12/12/94 -- Vasant Kumar, initial version
 */

# define DIM         500           /* problem domain size in each direction */
# define MAXITER     2000          /* maximum permitted iterations          */
# define EPS         0.00001       /* maximum allowed relative precision */
# define TOPBOUND    200.0         /* Top B.C. */
# define BOTBOUND      0.0         /* Bottom B.C. */
# define LEFTBOUND     0.0         /* Left B.C. */
# define RIGHTBOUND    0.0         /* Right B.C. */
# define DEBUG         1           /* For initial debugging 1, 0 o.w.*/
# define LOG           0           /* 1 to write output profile, 0 o.w. */

char      tpname[10];              /* name of a tuple */
double    inTpl[4];                /* input tuple */
double    powerTpl[2];             /* power tuple */
double    *outTpl;                 /* output tuple */
double    *residTpl;               /* residual tuple */
double    *synTpl;                 /* synchronization tuple */
