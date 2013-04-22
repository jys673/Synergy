/****************************************************************************/
/*                                                                          */
/* PROGRAM: lap.c                                                           */
/*                                                                          */
/* PURPOSE: This is the sequential version of the Laplace equation problem. */
/*                                                                          */
/*    OVERVIEW: Program lap.c solves the two-dimensional Laplace equation   */
/*              (an elliptical PDE) with prescribed boundary conditions on  */
/*              a single processor. Finite-difference  method is used to    */
/*              discretize the problem                                      */
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
/*              domain. The resulting sparse linear system is solved        */
/*              by the Gauss-Seidal method                                  */
/*                      The problem domain is rdim X DIM, where rdim is     */
/*              the row dimension and DIM is the column dimension in        */
/*              general. Our domain has rdim=DIM. Successive Gauss-Seidal   */
/*              iterations refine the values of A. The program terminates   */
/*              iterations if either the global residual is less than the   */
/*              desired or MAXITER is reached.                              */
/*                                                                          */
/* INPUT FILE: None                                                         */
/*                                                                          */
/* OUTPUT FILE: seq_time.dat                                                */
/*                                                                          */
/* LANGUAGE: C                                                              */
/*                                                                          */
/* UPDATE HISTORY:                                                          */
/*      Initial code: Vasant Kumar, CIS Dept., Temple Univ, Phila.          */
/*      (12/12/94)                                                          */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/***                    includes, prototypes, globals                     ***/
/****************************************************************************/

# include <stdio.h>
# include <time.h>
# include <math.h>
# include "lap.h"

void inita();              /* initilizes A with initial guess */

int     rdim;              /* row dimension of the domain */
double  A[DIM+2][DIM+2];   /* maxm. size of the domain with boundaryd  */


/****************************************************************************/
/***                       main()                                         ***/
/***                                                                      ***/
/***                                                                      ***/
/*** CALLS: inita()                                                       ***/
/***                                                                      ***/
/*** CALLED BY: None                                                      ***/
/***                                                                      ***/
/*** DESCRIPTION:                                                         ***/
/***    main is the main function of lap.c. It contains the iterative     ***/
/*** loop of the Gauss-Seidal method.                                     ***/ 
/****************************************************************************/

main()

{
      int     rowsize,          /* number of bytes in each row */
              iterno,           /* current iteration number */
              i, 
              j;                /* loop indices */
      double  resid,            /* residual */
              residtmp,         /* temp */
              residglob,        /* global residual */
              Atmp,             /* temp */
              Amax;             /* value corresponding residglob */

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
      rowsize  = sizeof(double) * (DIM + 2);
      inita();                /* guess initial values for A and set B.C. */



   /* 
    * This is the main iteration loop for the Gauss-Seidal method. Refinement
    * is done in successive iterations till either the global residual is
    * less than desired or MAXITER is reached.
    *
    */

      Amax = 0.0;

      for (iterno= 1; iterno <= MAXITER; iterno++)
      {



            resid = 0.0;
            
            /* 
	     *  Refine the values in A, these two loops consume most of
             *  the computational time.  
             */
            for (i = 1; i <= rdim; i++)         
                for (j = 1; j <= DIM; j++)
                {
                     Atmp     = A[i][j];
                     A[i][j]  = 0.25 * (A[i+1][j] + A[i-1][j] + 
                                        A[i][j+1] + A[i][j-1]);
                     residtmp = fabs(A[i][j] - Atmp);
                     if (residtmp > resid)
                     {
                         resid = residtmp;
                         Amax  = Atmp;
                     }
                }

            if (Amax == 0.0)                  /* set global residual */
                residglob = 1.0 + EPS;
            else
                residglob = fabs( resid/Amax );
            if (DEBUG && iterno % 100 == 0)
	      printf ("\nsequential(%d) iterations done\n",iterno);

            if ( residglob < EPS)            /* termination of iterations */
            {
                break;
            }
      }
     
      t1 = time((long *)0) - t0;


   /* 
    * Output  the solution 
    *
    */

      fd = fopen("seq_time.dat", "a");

	if (t1 > 0) 
	 {
	   fprintf(fd, "\n (%s) Elapsed time = %d seconds. n: (%d)\n", 
		host, t1, DIM);
	   fprintf(fd, " MFLOPS: (%f) Iterations: (%d) Precision (%f)\n",
		   (float)(DIM)*(DIM) * iterno * 5.0/(t1)/1000000.0,
		   iterno, residglob);
	 } 
        else fprintf(fd, " (%s) Zero second recorded. n: (%d).\n",
			host, DIM);

        if (iterno >= MAXITER)
	{
	  printf ("iterno: (%d), Precision: (%f), time: (%d sec), n:(%d)\n", 
		      iterno, residglob, t1, DIM);
	  printf("\n \n WARNING: Maximum iterations reached   \n");
          printf("%s\n%s\n%s\n\n", 
		 "      Try increasing MAXITER if the current Precision,", 
                 "      i.e. maximum relative change at any gridblock, is",
                 "      to be further improved ");

	}

	if (LOG)                /* to get output profile */
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
	    fprintf (fd, "\n%s\n\n", 
                   "===============================");
	  }
	  printf ("iterno: (%d), residglob: (%f), t1: (%d), n:(%d)\n", 
		      iterno, residglob, t1, DIM);
	  fclose(fd);
  	  return 0;
}



/****************************************************************************/
/***                       void inita()                                   ***/
/***                                                                      ***/
/***                                                                      ***/
/*** CALLS: None                                                          ***/
/***                                                                      ***/
/*** CALLED BY: main()                                                    ***/
/***                                                                      ***/
/*** DESCRIPTION:                                                         ***/
/***    Function inita initializes the work domain rdim X DIM by the      ***/
/*** user's first guess (need not be zero). Boundary values are also set. ***/
/****************************************************************************/


void inita ()
{
      int      i, j;             /* loop indices */


      for (i = 0; i < rdim+2; i++)     /* initial guess */
         for (j = 0; j < DIM+2; j++)
            A[i][j] = 0.0;


      for (i = 0; i < rdim+2; i++)     /* set boundaries */
      {
         A[i][0]         = LEFTBOUND;
         A[i][DIM+1]     = RIGHTBOUND;
      }

       for (j = 0; j < DIM+2; j++)
         A[0][j]      = TOPBOUND;

       for (j = 0; j < DIM+2; j++)
         A[rdim+1][j] = BOTBOUND;
}















