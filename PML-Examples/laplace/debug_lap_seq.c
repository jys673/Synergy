/****************************************************************************/
/*                                                                          */
/* PROGRAM: lap_seq.c                                                       */
/*                                                                          */
/*   This is the sequential version of the Laplace equation algorithm. It   */
/*   solves the two-dimensional Laplace equation with prescribed boundary   */
/*   conditions on a single processor. Finite-difference method is used to  */
/*   discretize the problem domain. The resulting sparse linear system is   */
/*   solved by the Gauss-Seidal method.                                     */
/*                                                                          */
/*            (0,0)              TOPBOUND                     (0,DIM+1)     */
/*              ...|........................................|...            */
/*              .__|________________________________________|__.            */
/*           L  .  |                                        |  . R          */
/*           E  .  |                                        |  . I          */
/*           F  .  |                                        |  . G          */
/*           T  .  |                                        |  . H          */
/*           B  .  |                                        |  . T          */
/*           O  .  |           PROBLEM   DOMAIN             |  . B          */
/*           U  .  |              (DIM X DIM)               |  . O          */
/*           N  .  |                                        |  . U          */
/*           D  .  |                                        |  . N          */
/*              .  |                                        |  . D          */
/*              .  |                                        |  .            */
/*              .  |                                        |  .            */
/*              .--|----------------------------------------|--.            */
/*              ...|........................................|...            */
/*        (DIM+1,0)                BOTBOUND             (DIM+1,DIM+1)       */ 
/*                                                                          */
/*   The problem domain is (rdim X DIM), where rdim is the row dimension    */
/*   and DIM is the column dimension in general. Our domain has rdim=DIM.   */
/*   Successive Gauss-Seidal iterations refine the values of A. The         */
/*   program terminates iterations if either the global residual is less    */
/*   than the desired or MAXITER is reached.                                */
/*                                                                          */
/*                                                                          */
/* OUTPUT FILE: seq_time.dat                                                */
/*                                                                          */
/* LANGUAGE: C                                                              */
/*                                                                          */
/****************************************************************************/


/* <reference> */
# include <stdio.h>
# include <time.h>
# include <math.h>

# include "laplace.h"
/* </reference> */

void inita();                   /* initilizes A with initial guess */


/****************************************************************************/
/*** int main()                                                           ***/
/***                                                                      ***/
/*** This is the main function of lap_seq.c. It contains the iterative    ***/
/*** loop of the Gauss-Seidal method.                                     ***/ 
/****************************************************************************/

/* <parallel appname="lap"> */

int main()
{
    FILE *fd;                 /* file descriptor */
    char host[128];           /* host name */
    long t0, t1;              /* start and end elapsed time */

    /* 
     * initialize 
     *
     */

    gethostname(host, sizeof(host));
    t0 = time ((long*)0);

    rdim = DIM;
    inita();

    /* 
     * This is the main iteration loop for the Gauss-Seidal method. Refinement
     * is done in successive iterations till either the global residual is
     * less than desired or MAXITER is reached.
     *
     */

    /* <master id="234567"> */

    for (iterno= 1; iterno <= MAXITER; iterno++)
     {
        /* <send var="A" type="double[DIM+2     ][DIM+2     ]" opt="ONCE"/> */

        /* <send var="A" type="double[DIM+2(0    ~1    )][DIM+2(0    ~DIM+1)]" opt="XCHG"/> */
        /* <send var="A" type="double[DIM+2(0    ~DIM+1)][DIM+2(DIM+1~DIM+2)]" opt="XCHG"/> */
        /* <send var="A" type="double[DIM+2(DIM+1~DIM+2)][DIM+2(1    ~DIM+2)]" opt="XCHG"/> */
        /* <send var="A" type="double[DIM+2(1    ~DIM+2)][DIM+2(0    ~1    )]" opt="XCHG"/> */

        /* <worker> */

        /* <read var="A" type="double[DIM+2     ][DIM+2     ]"/> */

        /* <read var="A" type="double[DIM+2(i:$L-1)][DIM+2(j)     ]" opt="XCHG"/> */
        /* <read var="A" type="double[DIM+2(i)     ][DIM+2(j:$L-1)]" opt="XCHG"/> */

        /* 
         *  Refine the values in A, these two loops consume most of
         *  the computational time.  
         */

        resid = 0.0;

        /* <target index="i" limits="(1,rdim+1,1)" chunk="2" order="1"> */
        for (i = 1; i < rdim+1; i++)         
        /* </target> */
         {
            /* <target index="j" limits="(1,DIM+1,1)" chunk="2" order="1"> */
            for (j = 1; j < DIM+1; j++)
            /* </target> */
             {
                Atmp     = A[i][j];
                A[i][j]  = 0.25 * (A[i+1][j] + A[i-1][j] + 
                                   A[i][j+1] + A[i][j-1]);

                residtmp = (Atmp!=0.0? fabs(fabs(A[i][j]-Atmp)/Atmp) : (1.0+EPS));

                if (residtmp > resid)
                    resid = residtmp;
             }
         }

        /* <send var="resid" type="double" opt="_MAX"/> */
        /* <send var="A" type="double[DIM+2(i)  ][DIM+2(j)  ]"/> */

        /* </worker> */

        /* <read var="A" type="double[DIM+2     ][DIM+2     ]"/> */
        /* <read var="resid" type="double" opt="_MAX"/> */
 
printf ("\n>>>>>>> Current Iters = (%d), Currect Resid = (%f) <<<<<<<<\n", iterno, resid);

        if (DEBUG && iterno % 10 == 0)
            printf ("\nsequential(%d) iterations done\n",iterno);

        /* termination of iterations */
        if (resid < EPS) break;
     }
     
    /* </master> */

    t1 = time((long *)0) - t0;

    printf ("\nTime: (%d), DIM: (%d), Iters/MAX: (%d/%d), Resid/EPS: (%f/%f)\n", 
             t1, DIM, iterno, MAXITER, resid, EPS);
    if (iterno >= MAXITER)
        printf("\nWARNING: Maximum iterations reached!\n");

    /* 
     * Output the solution 
     *
     */

    fd = fopen("seq_time.dat", "a");

    fprintf(fd, "\n(%s) Time = %d seconds. DIM: (%d) \n", host, t1, DIM);
    if (t1 > 0)
        fprintf(fd, "MFLOPS: (%f), Iters/MAX: (%d/%d), Resid/EPS: (%f/%f)\n",
                    (float)(DIM)*(DIM)*iterno*5.0/(t1)/1000000.0,
                     iterno, MAXITER, resid, EPS);
    else
        fprintf(fd, "MFLOPS: (--), Iters/MAX: (%d/%d), Resid/EPS: (%f/%f)\n",
                     iterno, MAXITER, resid, EPS);

    if (LOG)                  /* to output data to profile */
     {
        fprintf(fd,"\n");
        for (i=0; i<DIM+2; i++)
         {
            for (j=0; j<DIM+2; j++) 
             {
                fprintf(fd, "%8.1f, ",A[i][j]);
             }
            fprintf(fd,"\n");
         }
        fprintf (fd, "\n%s\n", "===============================");
     }
    fclose(fd);

    return 0;
}

/* </parallel> */


/****************************************************************************/
/*** void inita()                                                         ***/
/***                                                                      ***/
/*** This function initializes the work domain (rdim X DIM) by the user's ***/
/*** first guess (need not be zero). Boundary values are also set.        ***/
/****************************************************************************/

void inita()
{
    for (i = 0; i < rdim+2; i++)     /* initial guess */
        for (j = 0; j < DIM+2; j++)
            A[i][j]  = 0.0;

    for (i = 0; i < rdim+2; i++)     /* set boundaries */
     {
        A[i][0]      = LFTBOUND;
        A[i][DIM+1]  = RITBOUND;
     }

    for (j = 0; j < DIM+2; j++)
        A[0][j]      = TOPBOUND;

    for (j = 0; j < DIM+2; j++)
        A[rdim+1][j] = BOTBOUND;
}















