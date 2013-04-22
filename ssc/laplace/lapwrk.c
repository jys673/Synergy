/****************************************************************************/
/*                                                                          */
/* PROGRAM: lapwrk.c                                                        */
/*                                                                          */
/* PURPOSE: This is the worker program of the Laplace equation problem.     */
/*                                                                          */
/*    OVERVIEW: Program lapclnt.c with the worker program lapwrk.c solves   */
/*              the two-dimensional Laplace equation (an elliptical PDE)    */
/*              with prescribed boundary conditions on Synergy. Finite-     */
/*              difference  method is used to discretize the problem        */
/*                                                                          */
/*            (0,0)              TOPBOUND                     (0,DIM+1)     */
/*              ...|........................................|...            */
/*              .__|________________________________________|__.            */
/*           L  .  |                                        |  . R          */
/*           E  .  |                                        |  . I          */
/*           F  .  |                                        |  . G          */
/*           T  .  |                                        |  . H          */
/*           B  .  |                                        |  . T          */
/*           O  .  |      PROBLEM   DOMAIN OF A WORKER      |  . B          */
/*           U  .  |              (rdim X DIM)              |  . O          */
/*           N  .  |                                        |  . U          */
/*           D  .  |                                        |  . N          */
/*              .  |                                        |  . D          */
/*              .  |                                        |  .            */
/*              .  |                                        |  .            */
/*              .--|----------------------------------------|--.            */
/*              ...|........................................|...            */
/*            (rdim+1,0)             BOTBOUND             (rdim+1,DIM+1)    */ 
/*                                                                          */
/*              domain. The resulting sparse linear system is solved        */
/*              in the line of Gauss-Seidal method, but without             */
/*              synchronization in the beginning of an iteration.           */
/*                     The client first sends inTpl to workers for calibra  */
/*              tion, i.e. it asks workers to do a small amount of the      */
/*              communication and computaion work the worker is expected to */
/*              do. The time spent is returned to the client in powerTpl.   */
/*              The client then sends inTpl again to workers for their      */
/*              respective work domain. Each worker updates its own domain  */
/*              and reads information at the boundary asynchronously at the */
/*              beginning of a new iteratio. Indirect                       */
/*              synchronization among workers is  provided by    an         */
/*              efficient load balancing during the initial allocation of   */
/*              the problem chunk G=rdim using row decomposition technique. */
/*              Explicit synchronization (10 < thresold <= 100 lower the    */
/*              value higher is the explicit synchronization), might be used*/
/*              if an attempt to balance the load is not successful.        */
/*                      The workers terminate iterations if either the      */
/*              global residual is less than desired or MAXITER is reached. */
/*              The workers then send the results to the client and         */
/*              terminate themselves.                                       */
/*                      resTpl keeps track of the residual of the workers   */
/*              and synTpl helps provide explicit synchronization.          */
/*                                                                          */
/*                                                                          */
/* INPUT FILE: None                                                         */
/*                                                                          */
/* OUTPUT FILE: none                                                        */
/*                                                                          */
/* LANGUAGE: C                                                              */
/*                                                                          */
/* UPDATE HISTORY:                                                          */
/*      Initial code: Vasant Kumar, CIS Dept., Temple Univ, Phila.          */
/*      (12/12/94)                                                          */
/*                                                                          */
/* REMARK: The idea of rowwise decomposition technique is based on          */
/*   (1) Quinn, M.J.: Parallel Computing Theory and Practice, 2nd ed.,      */
/*          McGRAW-HILL, INC., New York                                     */
/*   (2) Intel iPSC/860 solution manuals.                                   */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/***                    includes, prototypes, globals                     ***/
/****************************************************************************/


# include <stdio.h>
# include <time.h>
# include <math.h>
# include "lap.h"

void calibrate();          /* to determine self processing power */
void initialize();         /* to initialize the work domain */
void inita();              /* to guess initial values in the work domain */
void preprocess();         /* to exchange the top and bottom boundaries */
void sendresults();        /* to send the result to the client */

int  rdim,                 /* row dimension for a worker */
     iam,                  /* worker's identity (0 to P-1) */
     P,                    /* no. of workers */
     G,                    /* work load, equals rdim */
     rowsize,              /* number of bytes in each row */
     calno,                /* no of iterations for calibrn (=factor in .csl).
                            * larger calno means better load balance. Choose
                            * a value (50 to 10000) so that approx. 15 sec.
                            * is spent in calibration */
     synfactor,            /* synchronization factor (=thresold in .csl).
                            * range (1 to 110), less than 10 not desirable;
                            * lower the value higher the synchronization; a 
			    * value greater than 100 means no synchronization*/
     tplength,             /* a tpl length */  
     tplengthP,            /* a tpl length of size P doubles */
     status,               /* temp */
     res,                  /* TS descrptor for result  */
     tsd,                  /* TS descriptor for problem */
     stopIterationCheck,   /* synchronization point intervals */
     iterno,               /* current iteration no */
     i,                    /* temp */
     j,                    /* temp */
     k;                    /* temp */
 
long   t0,                  /* start clock time for calibration */
       t1;                  /* end clock time in calibration */

double  A[DIM+2][DIM+2],   /* maxm. size of the domain with boundary  */
        resid,             /* residual */
        residtmp,          /* temp */
        residloc,          /* maxm. relative residual on this worker */
        residglob,         /* maxm. relative residual on all worker */
        Atmp,              /* temp */
        Amax;              /* value corresponding residloc */

 char   tpname_fromtop[10],      /* send row # rdim to this worker */
        tpname_frombot[10],      /* send  row # 1 to this worker */
        tpname_fromtop_get[10],  /* get into row # 0 from this worker */
        tpname_frombot_get[10],  /* get into row # rdim+1 from this worker */
        tpname_resid[10],        /* residual tuple name */
        tpname_syn[10];          /* synchronization tuple name */
            

/****************************************************************************/
/***                             main()                                   ***/
/***                                                                      ***/
/***                                                                      ***/
/*** CALLS: calibrate(), initialize(), preprocess(), sendresults()        ***/
/***                                                                      ***/
/*** CALLED BY: None                                                      ***/
/***                                                                      ***/
/*** DESCRIPTION:                                                         ***/
/***    Function main is the main function in this file. main first calls ***/
/*** calibrate() to determine self-processing power. It then calls        ***/
/*** initialize() for initialization of the relevant parameters. Then the ***/
/*** main iterative loop starts for the solution of the system of simul.  ***/
/*** equations by Gauss-Seidal iterative method. In the beginning of each ***/
/*** preprocess() is called to exchange the top and bottom boundary rows. ***/
/*** postprocessing is then done to check the global (on all workers)     ***/
/*** residual's maximum value residglob. If residglob is less than        ***/
/*** desirable, the worker stops further iterations after some time; if   ***/
/*** not, it continues till the maximum iteration MAXITER is reached      ***/
/*** synchronizing at certain iteration numbers guided by stopIteration-  ***/
/*** check. Finally, main calls sendresult to send results to the client. ***/
/****************************************************************************/


main()
{
            
      tsd = cnf_open("problem",0);     /* open TS */
      res = cnf_open("result",0);

      calibrate();                     /* determine self-power */
            	
      initialize();                    /* initialize the work-domain */
      
    
    /* 
     * Perform the Calculation 
     * This step is the main body of the iterative computations for solving
     * the system of simultaneous equation in the line of iterative 
     * Gauss-Seidel method. Unlike the Gauss-Seidal method, we don't do
     * synchronization in the beginning of each loop. Our load balancing
     * techniques provides, more or less, passive synchronization  among 
     * processors. An explicit partial synchronization (i.e, at some specific 
     * iterations only) can be forced by setting the value of synfactor less
     * than 100 in the .csl file.
     */

      Amax = 0.0;
      residglob = 1.0 + EPS;      
      
      for (iterno= 1; iterno <= MAXITER; iterno++)
      {
	
            preprocess();    /* exchange the top and bottom boundary */

           /* 
            * Update the values in the domain 
            */

            resid = 0.0;
 
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

            if (Amax == 0.0)       /* relative residual */
                residloc = 1.0 + EPS;
            else
                residloc = fabs( resid/Amax );
            

           /*
            * This is postprocessing for termination. 
            * Terminate iterations if global residual is as desired.
            * Global residual is kept in powerTpl
            */

           if (residloc <= EPS && iterno % 20 == 0)
	    {
	      tplengthP = cnf_tsget(tsd, tpname_resid, residTpl, -1);
	      residTpl[iam] = residglob = residloc;
	      for (k = 0; k < P; k++)
		{
		  if (residTpl[k] > residglob) 
		    residglob = residTpl[k];
                }
	      status = cnf_tsput(tsd, tpname_resid, residTpl, tplengthP);
	      if (residglob <= EPS)
          	break;
           }
          
           /* This is 2nd part of post processing. Synchronization is
            * done at selected intervals determined by stopIterationCheck.
            * A worker checks at a synchronization point if other workers
            * have also completed the desired synchronization iteration;if
            * not, the current workers performs additional iterations at
            * that synchronization point until every worker is there.
            */

           if ( iterno % stopIterationCheck == 0)
	    {
	      tplengthP = cnf_tsget(res, tpname_syn, synTpl, 0);
	      synTpl[iam] = (double) iterno/stopIterationCheck;
	      for (k = 0; k < P; k++)
		{
		  if (synTpl[k] < synTpl[iam]) 
		     {
		        iterno = iterno--;
			break;
		      }
                }
	      status = cnf_tsput(res, tpname_syn, synTpl, tplengthP);
           }

	    if (DEBUG && iterno % 100  == 0)
	      printf("lapwrk(%d): iterno(%d) \n", iam, iterno);

      }    
      if (DEBUG) 
	printf ("lapwrk(%d): finished iterations, iterno(%d), residloc(%f)\n",
		iam, iterno, residloc);
           
     sendresults();             /* send results to client */

    free (outTpl);
    free (residTpl);
    free (synTpl);
      if (DEBUG)
	printf("lapwrk(%d): successful completion, returning. \n", iam);

    return 0;
}

/****************************************************************************/
/***                         void calibrate()                             ***/
/***                                                                      ***/
/***                                                                      ***/
/*** CALLS: None                                                          ***/
/***                                                                      ***/
/*** CALLED BY: main()                                                    ***/
/***                                                                      ***/
/*** DESCRIPTION:                                                         ***/
/***    Function calibrate receives inTpl from the client to determine    ***/
/*** own identity (iam), and then determines self processing capability   ***/
/*** with respect to the present task and configurationn. The capability  ***/
/*** includes both computing power and communication power with the top   ***/
/*** and bottom neighbor. The work it does during calibration is the      ***/
/*** similar work it is expected to do in future. The handle calno        ***/
/*** supplied by the user (see file lap.csl's factor) determines how many ***/
/*** iteration we want to use for calibration. One should choose a factor ***/
/*** so large that a significant amount of time is spent in calibration   ***/
/*** A value of 15-20 sec. was quite satisfactory in most cases. Based    ***/
/*** on the clock time powerTpl is constructed to be sent to the client,  ***/
/*** which then allocates the work load to different workers. Please see  ***/
/*** lapclnt.c for details.                                               ***/
/****************************************************************************/



void calibrate()
{
 
      strcpy(tpname, "i*");        /* determine identity */
      tplength = cnf_tsget(tsd, tpname, inTpl, 0);
      iam = (int) inTpl[0];
      calno = (int) inTpl[1];
      synfactor = (int) inTpl[2];
      P = (int) inTpl[3];
      if (DEBUG)
	printf("lapwrk(%d): got inTpl for calbn. calno(%d), synfactor(%d) \n",
	       iam, calno, synfactor);

      rdim = (int) DIM/P;              /* same work load to all worker */
      rowsize  = sizeof(double) * (DIM + 2);
      tplength = rowsize; 

      sprintf (tpname_fromtop, "ft%d\0", iam + 1); /* whom to communicate */
      sprintf (tpname_frombot, "fb%d\0", iam - 1);
      sprintf (tpname_fromtop_get, "ft%d\0", iam);
      sprintf (tpname_frombot_get, "fb%d\0", iam);
      Amax = 0.0;
      residglob = 1.0 + EPS;      

      /* 
       * This loop shows the nature of work the worker is expected to
       * do, though at small scale. For details refer to main().
       */

      t0 = time((long *)0);

      for (iterno= 1; iterno <= calno; iterno++)
      {
        /*
         * top and bottom row exchange 
         */

	if (iam == 0)
	  {
	    status = cnf_tsput (tsd, tpname_fromtop,
				(double *) &A[rdim][0], rowsize);
            tplength = cnf_tsread (tsd, tpname_frombot_get,
				  (double *) &A[rdim+1][0], -1);
	  }
	else if (iam == P - 1)
	  {
	    status = cnf_tsput (tsd, tpname_frombot,
				(double *) &A[1][0], rowsize);
	    tplength = cnf_tsread (tsd, tpname_fromtop_get,
				  (double *) &A[0][0], -1);
	  }
	else
	  {
	    status = cnf_tsput (tsd, tpname_frombot,
				(double *) &A[1][0], rowsize);
	    status = cnf_tsput (tsd, tpname_fromtop,
				(double *) &A[rdim][0], rowsize);
	   tplength = cnf_tsread (tsd, tpname_fromtop_get,
				  (double *) &A[0][0], -1);
	   tplength = cnf_tsread (tsd, tpname_frombot_get,
				  (double *) &A[rdim+1][0], -1);
	  }

            resid = 0.0;

         /*
          * computational load
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

            if (Amax == 0.0)
                residloc = 1.0 + EPS;
            else
                residloc = fabs( resid/Amax );
      }
            
      t1 = time((long *)0) - t0;

      if (t1 < 1)
	{ printf ("lapwrk(%d): calno too small. Aborting ...\n", iam);
          cnf_term();
	  exit(1);
	}
         
      /*
       * send the timing info to the client.
       */

      tplength = 2 * sizeof(double);
      sprintf(tpname, "p%d\0", iam);
      powerTpl[0] = inTpl[0];
      powerTpl[1] = (double) t1;
      status = cnf_tsput(tsd, tpname, powerTpl, tplength);
      if (DEBUG)
	printf("lapwrk(%d): put powerTpl in TS problem \n", iam);

     return;
}


/****************************************************************************/
/***                        void initialize()                             ***/
/***                                                                      ***/
/***                                                                      ***/
/*** CALLS: init                                                          ***/
/***                                                                      ***/
/*** CALLED BY: main()                                                    ***/
/***                                                                      ***/
/*** DESCRIPTION:                                                         ***/
/***      This functions receives inTpl from the client (second time)     ***/
/*** and then determines its work domain (G or rdim) X DIM. It calls      ***/
/*** inita to guess the initial values. residTpl to keep track of the     ***/
/*** residuals[P] and synTpl[p] to keep track of synchronization param.   ***/
/*** are also initiated.
/****************************************************************************/
 
void initialize()
{
      /* 
       * get inTpl from client with parameters.
       */

      sprintf(tpname, "i%d\0", iam);
      tplength = cnf_tsget(tsd, tpname, inTpl, 0);
      if (DEBUG)
	printf("lapwrk(%d): got inTpl with parameters \n", iam);

      if (inTpl[0] != iam)
	{ 
	  printf ("lapwrk(%d): got wrong inTpl. Aborting ...\n", iam);
          cnf_term();
	  exit(1);
	}

      G = (int) inTpl[1]; 
      P = (int) inTpl[3];

      rdim = G;
      rowsize  = sizeof(double) * (DIM + 2);
      stopIterationCheck = (int) MAXITER * synfactor / 100.0;

      inita();             /* first guess */

      /*
       * Initialize residTpl and synTpl.
       */
      
      sprintf (tpname_resid, "r\0");
      sprintf (tpname_syn, "s\0");
      tplengthP = P * sizeof(double);
      if ((residTpl = (double *)malloc(tplengthP)) == NULL)
	{ 
	  printf ("lapwrk(%d): Error in dyn alloc for residTpl Aborting ..\n");
	  cnf_term();
	  exit(1);
    	}
      if ((synTpl = (double *)malloc(tplengthP)) == NULL)
	{ 
	  printf ("lapwrk(%d): Error in dyn alloc for synTpl Aborting ..\n");
	  free(residTpl);
	  cnf_term();
	  exit(1);
    	}
      for (k=0; k<P; k++)
	{
	   residTpl[k] = 1.0 + EPS;
	   synTpl[k] =  (double) 0;
         }

      tplength = rowsize; 
      sprintf (tpname_fromtop, "ft%d\0", iam + 1);
      sprintf (tpname_frombot, "fb%d\0", iam - 1);
      sprintf (tpname_fromtop_get, "ft%d\0", iam);
      sprintf (tpname_frombot_get, "fb%d\0", iam);
      
      if (DEBUG)
	printf("lapwrk(%d): initialization complete \n", iam);
     
      return;      
}      
     


/****************************************************************************/
/***                       void inita()                                   ***/
/***                                                                      ***/
/***                                                                      ***/
/*** CALLS: None                                                          ***/
/***                                                                      ***/
/*** CALLED BY: initialize()                                              ***/
/***                                                                      ***/
/*** DESCRIPTION:                                                         ***/
/***    Function inita initializes the work domain rdim X DIM by the      ***/
/*** user's first guess (need not be zero). Boundary values are also set. ***/
/****************************************************************************/


void inita ()
{

      for (i = 0; i < rdim+2; i++)     /* initial guess */
         for (j = 0; j < DIM+2; j++)
            A[i][j] = 0.0;
      for (i = 0; i < rdim+2; i++)    /* boundaries */
      {
         A[i][0]         = LEFTBOUND;
         A[i][DIM+1]     = RIGHTBOUND;
      }

      if (iam == 0)                  /* worker at the top */
      {
         for (j = 0; j < DIM+2; j++)
            A[0][j]      = TOPBOUND;
      }
      
      if (iam == P-1)               /* bottommost worker */
      {
         for (j = 0; j < DIM+2; j++)
            A[rdim+1][j] = BOTBOUND;
      }
      return;
}



/****************************************************************************/
/***                        void preprocess()                             ***/
/***                                                                      ***/
/***                                                                      ***/
/*** CALLS: None                                                          ***/
/***                                                                      ***/
/*** CALLED BY: main()                                                    ***/
/***                                                                      ***/
/*** DESCRIPTION:                                                         ***/
/***    Function preprocess exchanges the top and bottom rows with its    ***/
/*** neighbor. Please note that the boundary values are not exchanged.    ***/
/*** For this purpose every worker has two extra rows one at the top      ***/
/*** and one at the bottom to keep the values of its neighbors. This      ***/
/*** function is called in the beginning of each iterative step.          ***/
/****************************************************************************/

void preprocess()
{

	if (iam == 0)     /* topmost worker */
	  {
	    status = cnf_tsput (tsd, tpname_fromtop,
				(double *) &A[rdim][0], rowsize);
            tplength = cnf_tsread (tsd, tpname_frombot_get,
				  (double *) &A[rdim+1][0], -1);
	  }

	else if (iam == P - 1)   /* bottommost workers */

	  {
	    status = cnf_tsput (tsd, tpname_frombot,
				(double *) &A[1][0], rowsize);
	    tplength = cnf_tsread (tsd, tpname_fromtop_get,
				  (double *) &A[0][0], -1);
	  }

	else                    /* other workers */
	  {
	    status = cnf_tsput (tsd, tpname_frombot,
				(double *) &A[1][0], rowsize);
	    status = cnf_tsput (tsd, tpname_fromtop,
				(double *) &A[rdim][0], rowsize);
	   tplength = cnf_tsread (tsd, tpname_fromtop_get,
				  (double *) &A[0][0], -1);
	   tplength = cnf_tsread (tsd, tpname_frombot_get,
				  (double *) &A[rdim+1][0], -1);
	  }
          return;
}



/****************************************************************************/
/***                       void sendresults()                             ***/
/***                                                                      ***/
/***                                                                      ***/
/*** CALLS: none                                                          ***/
/***                                                                      ***/
/*** CALLED BY: main()                                                    ***/
/***                                                                      ***/
/*** DESCRIPTION:                                                         ***/
/***    Function sendresult sends the results of the computation and the  ***/
/*** local domain identification (G and iy) as it relates to the whole    ***/
/*** problem, to the client for reassembling. 
/****************************************************************************/

void sendresults()
{

      sprintf (tpname, "o%d\0", iam);
      tplength = (4 + DIM * rdim) * sizeof(double);
      if ((outTpl = (double *)malloc(tplength)) == NULL)
	{ 
	  printf ("lapwrk(%d): Error in dyn alloc for outTpl Aborting ..\n");
	  free (residTpl);
	  cnf_term();
	  exit(1);
	}
		  
      outTpl[0] = residloc;                /* local residual */      
      outTpl[1] = inTpl[1];                /* G or rdim */
      outTpl[2] = inTpl[2];                /* starting global row no. for G */
      outTpl[3] = (double) iterno;         /* iterno at termination */

      for (i = 0; i < rdim; i++)
	  for (j = 0; j < DIM; j++)
	      outTpl[i * DIM + j + 4] = A[i+1][j+1];
     status = cnf_tsput (res, tpname, outTpl, tplength);
     if (DEBUG)
	printf("lapwrk(%d): outTpl put in TS result \n", iam);

     return;
}

/***************************     THE END    ********************************/
