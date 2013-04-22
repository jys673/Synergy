/*-----------------------------------------------------------------------------
                       Matrix Multiplication Master

This program has the following steps:

a) Generation of two input matrices A and B.  
b) Assemble working tuples according to a given chunk size.
c) Send the working tuples to tuple space "problem."
d) Wait for results from tuple space object "results."
e) Assemble the results as C.

date:  08/06/94

Modified by Feijian Sun, 08/22/94 for cosmetic changes + xdr.
Modified by Yuan Shi, 9/10/94 for factoring.
Modified by Justin Shi, 2/23/2013 for packaging.
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include "matrix.h"
double C[N][N];		/* result Matrix. */
double A[N][N];
double wall_clock();

main()
{ char host[128];
	int i, j, k, received;
	int ix, iy, tplength, status;
	int G, R, P, res, tsd, x;
	double t0, t1;
	float F;
	FILE *fd;

	gethostname(host, sizeof(host));
	t0 = wall_clock();
	ix = 0;

	printf("Before cnf_open... \n");
	tsd = cnf_open("problem",0);
	res = cnf_open("result",0);


	G = cnf_getf(); // Get chunk size
	P = cnf_getP(); // Get number of processors
    printf(" mtclnt.  Chunk size (%d) \n",G);
	R = N;

        tplength = (1+N*N)*sizeof(double);
	// Building Matrix A and B
        for (i = 0; i < N; i++)
                for (j = 0; j < N; j++)
		{
                        ituple_B[i][j] = (double) i * j;
			A[i][j] = (double) i * j;
		}

        sprintf(tpname,"B%d\0",0);

        status = cnf_tsput(tsd, tpname, (double *)ituple_B, tplength);
        tplength = (1+ G*N) * sizeof(double);


	//printf("tplength = (%d) \n", tplength);
	if ((ituple_A = (double *) malloc(tplength)) == NULL) exit(1);

	while (R > 0) {
		if (R < G) G = R;
		R = R - G ;
		//printf(" mtclnt. G(%d) R(%d) \n", G,R);
		ituple_A[0] = G;
		for (x = 0; x < G; x++)
			for (j = 0; j < N; j++) 
				ituple_A[x*N+j+1] = A[ix+x][j];
		sprintf(tpname,"A%d\0",ix);
		status = cnf_tsput(tsd, tpname, ituple_A, tplength);
		ix += G;
	}
	free(ituple_A);

	/* now receive the result  */
	received = i = 0;
	tplength = (1+N*N)*sizeof(double);
	if ((otuple = (double *)malloc(tplength)) == NULL)
		exit(1);
	while (received < N) {
		strcpy(tpname,"*");
		printf(" mtclnt.  waiting for a tuple) \n");
		tplength = cnf_tsget(res, tpname, otuple, 0);

		G = (int) otuple[0];
		ix = atoi(tpname);
		iy = 1;
		printf(" mtclnt.  tuple %d received %d) \n", ix, received);
					/* reassemble the result matrix */
		for (i= 0; i < G; i++) {
				received ++;
				for (j=0; j < N; j++) {
						C[ix][j] = otuple[iy];
						iy++;
				}
				ix ++;
		}
	}
	free(otuple);

	printf(" mtclnt.  received everything\n");
	/* insert zero size tuple as termination signal */
	tplength = sizeof(double);
	if ((ituple_A = (double *)malloc(tplength)) == NULL)
		exit(1);

	ituple_A[0]  = -1;
	sprintf(tpname, "A%d\0",N*N);
	status = cnf_tsput(tsd, tpname, ituple_A, tplength);
	free(ituple_A);
	t1 = wall_clock() - t0;

	//fd = fopen("matrix.par.time", "a");
	printf("Performance: (%s) (%f)sec. P(%d) f(%d) n(%d)\n",
			host, t1/1000000, P, G,  N*1);
	if (t1>0) printf(" (%f) MFLOPS.\n", (float) (N*N/t1)*N);
	else printf(" MFLOPS: Not measured.\n");
	//fclose(fd);
	cnf_term();

#ifdef bd
#endif 

}


