/*-----------------------------------------------------------------------------
                       Matrix Multiplication Master

This program has the following steps:

a) Generation of two input matrices A and B.  
b) Assemble working tuples according to the factoring load balancing algorithm.
c) Send the working tuples to tuple space "problem."
d) Wait for results from tuple space object "results."
e) Assemble the results as C.

date:  08/06/94

Modified by Feijian Sun, 08/22/94 for cosmetic changes + xdr.
Modified by Yuan Shi, 9/10/94 for factoring.
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include "matrix.h"
double C[N][N];		/* result Matrix. */
double A[N][N];

main()
{
	char host[128];
	int i, j, k, received;
	int ix, iy, tplength, status;
	int G, T, R, P, res, tsd, x;
	long t0, t1;
	float F;
	FILE *fd;

	gethostname(host, sizeof(host));
	t0 = time((long *)0);
	T = 1;
	ix = 0;

	tsd = cnf_open("problem",0);
	res = cnf_open("result",0);

	F = (float)cnf_getf()/100.0;
	P = cnf_getP();
        printf(" mtclnt. F (%f) P (%d) \n",F,P);
	T = cnf_gett();
	R = N;
	G = 1;

        tplength = (1+N*N)*sizeof(double);
        for (i = 0; i < N; i++)
                for (j = 0; j < N; j++)
		{
                        ituple_B[i][j] = (double) i * j;
			A[i][j] = (double) i * j;
		}
        sprintf(tpname,"B%d\0",0);
        status = cnf_tsput(tsd, tpname, (double *)ituple_B, tplength);
	free(ituple_B);
	while ((R > T) && (G > 0)) {
		G = (int) R*F/P;
		if (G > N/2) {
			printf("Grain size too large. Reset to (%d)\n",N/2);
			G = N/2;
		}
		R = R - G * P;
		printf(" mtclnt. G(%d) R(%d) \n", G,R);
		tplength = (1+ G*N) * sizeof(double);
		if ((ituple_A = (double *)malloc(tplength)) == NULL)
			exit(1);
		if ( G > 0) {
			for (i = 0; i < P ; i++) {
				ituple_A[0] = G;
				for (x = 0; x < G; x++)
					for (j = 0; j < N; j++) 
						ituple_A[x*N+j+1] = A[ix+x][j];
				sprintf(tpname,"A%d\0",ix);
				status = cnf_tsput(tsd, tpname, ituple_A, 							tplength);
				ix += G;
			}
		}
		free(ituple_A);
	}
	if (R > 0) {
		G = R;
		while (R > 0) {
			if (G > N/2) G = N/2;
			tplength = (1+ G*N) * sizeof(double);
			if ((ituple_A = (double *)malloc(tplength)) == NULL)
                       	 	exit(1);
			ituple_A[0] = G;
			for (x = 0; x < G; x++)
				for (j = 0; j < N; j++)
					ituple_A[x*N+j+1] = A[ix+x][j];
			sprintf(tpname,"A%d\0",ix);
			status = cnf_tsput(tsd, tpname, ituple_A, tplength);
			free(ituple_A);
			ix += G;
			R = R - G;
		}
	}
					/* now receive the result  */
	received = i = 0;
	tplength = (1+N*N)*sizeof(double);
	if ((otuple = (double *)malloc(tplength)) == NULL)
		exit(1);
	while (received < N) {
		strcpy(tpname,"*");
		tplength = cnf_tsget(res, tpname, otuple, 0);
		G = (int) otuple[0];
		ix = atoi(tpname);
		iy = 1;
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
	/* insert zero size tuple as termination signal */
	tplength = sizeof(double);
	if ((ituple_A = (double *)malloc(tplength)) == NULL)
		exit(1);
	ituple_A[0]  = -1;
	sprintf(tpname, "A%d\0",N*N);
	status = cnf_tsput(tsd, tpname, ituple_A, tplength);
	free(ituple_A);
	t1 = time((long *)0) - t0;

	fd = fopen("matrix.nxdr.factor.time", "a");
	fprintf(fd, "nXDR factored Elapsed Time: (%d) P:(%d) f(%f) t(%d) n(%d) ",
			t1, P, F, T, N);
	if (t1>0) fprintf(fd, " MFLOPS: (%f)\n", (float) N*N*N/t1/1000000);
	else fprintf(fd, " MFLOPS: Not measured.\n");
/*
        for (i=0; i < N; i++) {
                for (j=0; j < N; j++)
                        fprintf(fd, "%8.1f ", C[i][j]);
                fprintf(fd, "\n");
        }
*/
	fclose(fd);
	cnf_term();
}
