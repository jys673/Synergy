/*-----------------------------------------------------------------------------
Modified by Feijian Sun for XDR adaptation, 08/22/94
Modified by Wenhai Jiang for automatic load balancing, 09/22/94
Modified by Yuan Shi for correcting the load calculation formula, 10/1/94
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include "matrix.h"
#include "synergy.h"
double C[N][N];		/* result Matrix. */
double A[N][N];
double index_m[N];
unsigned index_p;
double total;
double least;
int G,  R, P, res, tsd, x;
int i, j, k, tester_cnt;

void alrmHandler()
{
	P = tester_cnt;
	return;
}

main()
{
	char host[128];
	int ix, iy, tplength, status;
	int G2, tplength2, received; 
	long t0, t1, t2; 
	double longest = 0.0, sum = 0.0;
	FILE *fd;

	gethostname(host, sizeof(host));
	t0 = time((long *)0);
	ix = 0;

	tsd = cnf_open("problem",0);
	res = cnf_open("result",0);

	G = cnf_getf();
	P = cnf_getP();
        printf(" mtclnt.  Test Chunk size (%d) \n",G);
	if (G > 500) {
		printf("Chunk size too large (Max: 500)\n");
		exit(-9);
	}
	R = N;
	signal(SIGALRM, alrmHandler);

        tplength = (1+N*N)*sizeof(double);
        for (i = 0; i < N; i++)
                for (j = 0; j < N; j++)
		{
                        ituple_B[i][j] = (double) i * j;
			A[i][j] = (double) i * j;
		}
        sprintf(tpname,"B%d\0",0);
	t2 = time((long *)0);
        status = cnf_tsput(tsd, tpname, (double *)ituple_B, tplength);
	t2 = time((long *)0) - t2;
	printf("Tsput B elapse %d\n",t2);
	tplength2 = ( 3 + G*N) * sizeof(double);
	if ( (ituple_A = (double *)malloc(tplength2)) == NULL )
			exit(-9);
	for ( i = 0; i < P; i++ ) {
		index_m[j] = 0.0;
		ituple_A[0] = -2;
		ituple_A[1] = 0;
		ituple_A[2] = G;
		memcpy(&ituple_A[2], A, G*N*sizeof(double));
		sprintf(tpname,"T%d\0",i);
		status = cnf_tsput(tsd, tpname, ituple_A, tplength2);
		if (status < 0) P--; /* unable to talk to a worker */
	}
	free(ituple_A);
	tester_cnt = i = 0;
	tplength2 = (2+G*N)*sizeof(double);
	if ((otuple = (double *)malloc(tplength2)) == NULL)
		exit(-9);
	fd = fopen("albm.time", "a");
	while (tester_cnt < P) {
		strcpy(tpname,"S*");
		tplength = cnf_tsget(res, tpname, otuple, 0);
		ix = atoi(&tpname[1]);
printf(" Master received P(%d) clock(%.3f) \n",ix,otuple[1]);
		index_m[ix] = otuple[1];
		if (index_m[ix] > longest) longest = index_m[ix];
		tester_cnt++;
	}
	free(otuple);
	t1 = time((long *)0) - t0;
	sum = 0;
	for ( i = 0; i < P; i++ ) {
		index_m[i] = longest/index_m[i];
		sum = sum + index_m[i];
		printf("index %.3f P(%d)\n",index_m[i], i );
	}
	G = (int)(R/sum);
	if (G == R) G = G / 10; /* P = 1. Take advantage of multi-tasking */
	printf(" G(%d)\n",G);
	for ( i = 0; i < P; i++ )
	   fprintf(fd,"P(%d) Index(%.3f) N(%d) G(%d) \n",i, index_m[i],N,G);
	ix = 0;
	tplength = (1+ G*N) * sizeof(double);
	if ((ituple_A = (double *)malloc(tplength)) == NULL)
			exit(-9);
	while (R > 0) {
		if (R < G) G = R;
		R = R - G ;
		printf(" mtclnt. G(%d) R(%d) \n", G,R);
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
		exit(-9);
	while (received < N) {
		strcpy(tpname,"*");
		tplength = cnf_tsget(res, tpname, otuple, 0);
printf("Master received (%s)\n",tpname);
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
		exit(-9);
	ituple_A[0]  = -1;
	sprintf(tpname, "A%d\0",N*N);
	status = cnf_tsput(tsd, tpname, ituple_A, tplength);
	free(ituple_A);
	t1 = time((long *)0) - t0;
	printf("End of Seesion \n");
	fprintf(fd, "nXDR: (%s) (%d)sec. P(%d) f(%d) n(%d) ",
			host, t1, P, G, N);
	if (t1>0) fprintf(fd, " (%f) MFLOPS.\n", (float) N*N*N/t1/1000000);
	else fprintf(fd, " MFLOPS: Not measured.\n");
	if (N == 10) 
        for (i=0; i < N; i++) {
                for (j=0; j < N; j++)
                        fprintf(fd, "%8.1f ", C[i][j]);
                fprintf(fd, "\n");
        }
	fclose(fd);
	cnf_term();
}

