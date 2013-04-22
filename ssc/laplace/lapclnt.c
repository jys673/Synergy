/****************************************************************************/
/*                                                                          */
/* PROGRAM: lapclnt.c                                                       */
/*                                                                          */
/* PURPOSE: This is the client program of the Laplace equation problem.     */
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
/*           O  .  |              PROBLEM   DOMAIN          |  . B          */
/*           U  .  |              (DIM X DIM)               |  . O          */
/*           N  .  |                                        |  . U          */
/*           D  .  |                                        |  . N          */
/*              .  |                                        |  . D          */
/*              .  |                                        |  .            */
/*              .  |                                        |  .            */
/*              .--|----------------------------------------|--.            */
/*              ...|........................................|...            */
/*            (DIM+1,0)             BOTBOUND             (DIM+1,DIM+1)      */ 
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
/* OUTPUT FILE: nxdr_time.dat                                               */
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

double  A[DIM+2][DIM+2];   /* maxm. size of the domain with boundary  */
int  rdim;                 /* row dimension for a worker */

            

/****************************************************************************/
/***                             main()                                   ***/
/***                                                                      ***/
/***                                                                      ***/
/*** CALLS: None                                                          ***/
/***                                                                      ***/
/*** CALLED BY: None                                                      ***/
/***                                                                      ***/
/*** DESCRIPTION:                                                         ***/
/***    Function main is the single function in the file lapclnt.c        ***/
/*** The client or the master starts with sending inTpl to the workers    ***/
/*** with some calibration information including the number of time       ***/
/*** the worker should iterate  on a part of the similar problem it is    ***/
/*** going to execute later. The calibration result (how long a worker    ***/
/*** took to do a fixed amount of work) is sent by the workers in powerTpl***/
/*** Based on the information the master partition the DIM X DIM problem  ***/
/*** into G X DIM problems for different workers. The master then sends   ***/
/*** the chunksize (G) information to the workers in inTpl. It also       ***/
/*** initializes residTpl and synTpl and put them into the TS. residTpl   ***/
/*** is used by workers to find out the value of local residuals for      ***/
/*** other workers and then to compute residglob to determine if the requ ***/
/*** ired precision is achieved everywhere to finally end iterations.     ***/
/*** synTpl is used by the workers for synchronization.                   ***/
/*** The master then waits for results to arrive. The master then combines***/
/*** the results and prints the output. 
/****************************************************************************/


main()
{
   int P,                    /* no. of workers */
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
     status,               /* temp */
     res,                  /* TS descrptor for result  */
     tsd,                  /* TS descriptor for problem */
     iterno,               /* maxm iteration no returned by all workers */
     iternoloc,            /* iterno returned by a worker */
     ix,                   /* starting point in x dn. */
     iy,                   /* starting point in Y dn */
     i,                    /* temp */
     j,                    /* temp */
     k;                    /* temp */
      
  long   t0,                  /* start clock time */
         t1,                  /* end clock time */
         t2;                  /* intermediate clock time*/
 
  double  residloc,          /* maxm. relative residual on a worker */
          residglob,         /* maxm. relative residual on all worker */
          *index;             /* to determine index of each wworker */
  double indexsum = 0.0;     /* sum of index */
  double maxtime  = 0.0;     /* maxm time so far */
  double calTimesum = 0.0;   /* sum of the calibration time */
                
  FILE *fd;                  /* file descriptor */
  char host[128];            /* temp */


      /*
       * send inTpl to wworkers for calibration.
       */

      gethostname(host, sizeof(host));
      t0 = time((long *)0);
      iy = 1;               /* start of domain of interst */
      
      tsd = cnf_open("problem",0);
      res = cnf_open("result",0);

      rdim = DIM;               /* dimn. along y dn is rdim */
      P = cnf_getP();           
      calno = cnf_getf();       /* for iteration in calibration */
      synfactor = cnf_gett();   /* for synchronization interval */
      
      if ((index = (double *) malloc (P * sizeof(double))) == NULL)
	{ 
	  printf ("lapclnt : Error in dynamic alloc for index. Aborting ...");
          cnf_term();
	  exit(1);
	}

      tplength = 4 * sizeof(double);
      for (k = 0; k < P; k++)
	{
	  sprintf(tpname, "i%d\0", k);
	  inTpl[0] = (double)k;
          inTpl[1] = (double)calno;
          inTpl[2] = (double)synfactor;
          inTpl[3] = (double) P;
          status = cnf_tsput(tsd, tpname, inTpl, tplength);
	}
      if (DEBUG)
	printf("lapclnt: inTpl put for identification\n");
    
      
     /*
      * Get response from the workers in powerTpl and then determine
      * index for each worker and distribute the no. of rows in proportion
      * to the index.
      */
      
     for (k = 0; k < P; k++)
	
       {
	  strcpy(tpname, "p*");
          tplength = cnf_tsget(tsd, tpname, powerTpl, 0);
	  if (powerTpl[1] > maxtime) maxtime = powerTpl[1];
          calTimesum += powerTpl[1];
          j = (int) powerTpl[0];
	  index[j] = powerTpl[1];
	}
      if (DEBUG)
	printf("lapclnt: powerTpls received\n");

      for (k = 0; k < P; k++)
	{
          index[k] = calTimesum/index[k];
          indexsum = indexsum + index[k];
	}

      if (DEBUG)
       { 
	    printf("lapclnt: caltime->P: (%f)  P: (%d)\n",powerTpl[1], j);
	    t2 = time((long *) 0) - t0;
            printf("lapclnt. TotCalTime: (%d)\n", t2);
	}  
	  


     /*
      * put residTpl in the TS problem. residTpl is used by workers
      * to find the residloc of each worker in finding residglob.
      */
     
      t0 = time((long *)0);           /* the timing info starts now */
      
      sprintf (tpname, "r\0");
      tplength = P * sizeof(double);
      if ((residTpl= (double *)malloc(tplength)) == NULL)
	{ 
	  printf ("lapclnt : Error in dyn alloc for residTpl Aborting ...\n");
          free(index);
	  cnf_term();
	  exit(1);
	}
       for (k = 0; k < P; k++)
	 residTpl[k] = 1.0 + EPS;
       status = cnf_tsput(tsd, tpname, residTpl, tplength);

       if (DEBUG)
	 printf("lapclnt: residTpl put in TS problem\n");



     /*   
      * put synTpl in the TS result. synTpl is used by the workers
      * for synchronization if required
      */
     
      sprintf (tpname, "s\0");
      tplength = P * sizeof(double);
      if ((synTpl= (double *)malloc(tplength)) == NULL)
	{ 
	  printf ("lapclnt : Error in dyn alloc for synTpl Aborting ...\n");
          free(index);
          free(residTpl);
	  cnf_term();
	  exit(1);
	}
       for (k = 0; k < P; k++)
	 synTpl[k] = (double) 0;
       status = cnf_tsput(res, tpname, synTpl, tplength);

       if (DEBUG)
	 printf("lapclnt: synTpl put in TS problem\n");



      /* 
       * send inTpl to workers second time. The inTpl contains information
       * on the part of total row dimension each worker is allocated 
       * according to its overall computation and communication power
       * in the current configuration determined earlier
       */
      
      tplength = 4 * sizeof(double);
      for (k = 0; k < P; k++)
	{
          if (k == P-1)
	      G = 1 + rdim - iy;
	  else
	      G = (int) rdim * index[k]/indexsum;

          inTpl[0] = (double) k;       /* identity of a worker */
          inTpl[1] = (double) G;       /* chunk size (a part of total rows */
          inTpl[2] = (double) iy;      /* starting row number */
          inTpl[3] = (double) P;       /* total no. of workers */
 
	  sprintf(tpname, "i%d\0", k);
          status = cnf_tsput(tsd, tpname, inTpl, tplength);
          if (DEBUG) 
	    printf("lapclnt: Chunk size: (%d) for Worker: (%d) \n",G,k);
	  iy += G;
	}
        
	free(index);    



      /* 
       * Wait for the results. 
       * The client processes each result as it is sent by a worker. 
       * A worker also sends the corresponding chunk size (G) and the
       * the relative row no (iy), so that the results can be assembled
       * in the order they arrive.
       * the client also receives the information about the local 
       * relative maximum residual from each worker and comutes the 
       * maximum of those or the residglob.
       */

       if (DEBUG)
	 printf("lapclnt: inTpl put in problem. Waiting for results\n");

      tplength = (4 + DIM * DIM) * sizeof(double);
      if ((outTpl = (double *)malloc(tplength)) == NULL)
	{ 
	  printf ("lapclnt : Errorn dynamic alloc for outTple Aborting ...\n");
          cnf_term();
	  exit(1);
	}

     /* receive results */
      
      residglob = -1.0;
      iterno = -1;
      for (k = 0; k < P; k++)
       {
	 strcpy(tpname,"o*");
	 tplength = cnf_tsget(res, tpname, outTpl, 0);
         if (DEBUG)
	   printf("lapclnt: received outTpl(%d of %d)\n", k+1, P);

	 residloc = outTpl[0];
	 if (residloc > residglob) residglob = residloc;
	 iternoloc = (int) outTpl[3];
         if (iternoloc > iterno) iterno = iternoloc;
	 G = (int) outTpl[1];
	 iy = (int) outTpl[2];
	 ix = 4;
					/* reassemble the result matrix */
         for (i= 0; i < G; i++) 
	   {
           for (j=0; j <DIM; j++) 
	     {
                A[iy][j+1] = outTpl[ix];
                ix++;
              }
           iy ++;
          }
	}
	
        free(outTpl);
	t1 = time((long *)0) - t0;



        /* 
         * Print the Solution.  
         * The solution consists of timing information excluding the time
         * required for calibration. In addition, effective MFLOPS (if it
         * were computed on a single processor), and the maximum global
         * residual. An output Temperature profile can also be generated for
         * the entire domain by setting the LOG flag 1 in  lap.h.
         */

	fd = fopen("nxdr_time.dat", "a");
        if (DEBUG)
	  printf("lapclnt: All result received. Printing output files..\n");

	if (t1 > 0) 
	 {
	   fprintf(fd, "\n nXDR (%s) Elapsed time = %d sec n: (%d) P:(%d)\n", 
		host, t1, DIM, P);
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
	  printf("\n\n\n WARNING: Maximum iterations reached   \n");
          printf("%s\n%s\n%s\n%s\n%s\n\n%s\n%s\n%s\n%s\n\n", 
		 "  Try balancing load (increase factor in lap.csl) ",
		 "       load balancing should be the first choice. ",
                 "               NEXT you can try",  
		 "  Better synchronization (decrease thresold in lap.csl, ",
		 "       keep 10+ to avoid excessive commun. overhead; AND/OR",
		 "  inreasing MAXITER (in lap.h)",
                 "  if the current Precision,", 
                 "  i.e. maximum relative change at any gridblock, is",
                 "  to be further improved ");

	}

        if (LOG)                  /* also, print temperature profile */
	  {
            fprintf(fd,"\n");
            
            for (i = 0; i < DIM+2; i++)
	      {
		A[i][0] = LEFTBOUND;
		A[i][DIM + 1] = RIGHTBOUND;
		A[0][i] = TOPBOUND;
		A[DIM + 1][i] = BOTBOUND;
              }
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
	  printf ("iterno: (%d), residglob: (%f), t1: (%d), n:(%d), P:(%d)\n", 
		      iterno, residglob, t1, DIM, P);

	fclose(fd);
        free (residTpl);
        free (synTpl);
	cnf_term();
	printf("lapclnt: DONE, returning.\n");

  	 return 0;
}

/****************************   THE END    ******************************/  

















