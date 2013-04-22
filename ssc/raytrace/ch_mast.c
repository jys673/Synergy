/*

  This is the function for calculating the best chunk size.
  written by Faisal G. Hassan and modify by many other people,
  see README.
*/ 


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ch_header.h"
#define RECORDS "test_chunk"
#define OUT fflush(stdout)
double C[Number_chunk_test][Number_chunk_test];		/* result Matrix. */
double A[Number_chunk_test][Number_chunk_test];
double index_m[Number_chunk_test];
unsigned index_p;
double total;
double least;

double test_G(int,int);

 double test_G(int tsd,int res)
{

	char host[128];
	int i, j, k, received;
	int ix, iy, tplength, status;
	int G,  R, P,  x;
	int G2, tplength2; 
	long t0, t1, t2; 
	double longest = 0.0, sum = 0.0;
	float F;
	FILE *fd;

	gethostname(host, sizeof(host));
	t0 = time((long *)0);
	ix = 0;
	G = cnf_getf();
	P = cnf_getP();
        printf(" mtclnt.  Test Chunk size (%d) \n",G);
	if (G > 500) {
		printf("Chunk size too large (Max: 500)\n");
		exit (1);
	}
	R = Number_chunk_test;

        tplength = (1+Number_chunk_test*Number_chunk_test)*sizeof(double);
        for (i = 0; i < Number_chunk_test; i++)
                for (j = 0; j < Number_chunk_test; j++)
		{
                        ituple_B[i][j] = (double) i * j;
			A[i][j] = (double) i * j;
		}
        sprintf(tpname,"B%d\0",0);
	t2 = time((long *)0);
        status = cnf_tsput(tsd, tpname, (double *)ituple_B, tplength);
	t2 = time((long *)0) - t2;
	printf("Tsput B elapse %d\n",t2);
	free(ituple_B);
	tplength2 = ( 3 + G*Number_chunk_test) * sizeof(double);
	if ( (ituple_A = (double *)malloc(tplength2)) == NULL )
			exit(1);
	for ( i = 0; i < P; i++ ) {
		index_m[j] = 0.0;
		ituple_A[0] = -2;
		ituple_A[1] = 0;
		ituple_A[2] = G;
		memcpy(&ituple_A[2], A, G*Number_chunk_test*sizeof(double));
		sprintf(tpname,"T%d\0",i);
		status = cnf_tsput(tsd, tpname, ituple_A, tplength2);
	}
	free(ituple_A);
	received = i = 0;
	tplength2 = (2+G*Number_chunk_test)*sizeof(double);
	if ((otuple = (double *)malloc(tplength2)) == NULL)
		exit(1);
	fd = fopen(RECORDS, "a");
	while (received < P) {
		strcpy(tpname,"S*");
		tplength = cnf_tsget(res, tpname, otuple, 0);
		ix = atoi(&tpname[1]);
printf(" Master received P(%d) clock(%.3f) \n",ix,otuple[1]);
		index_m[ix] = otuple[1];
		if (index_m[ix] > longest) longest = index_m[ix];
		received++;
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
	printf(" G(%d)\n",G);
	for ( i = 0; i < P; i++ )
	fprintf(fd,"P(%3d) Index(%3.3f) Number_chunk_test(%4d) G(%3d) \n",i, index_m[i],Number_chunk_test,G);
        fflush(stdout); 
	fclose(fd);
        return (R/sum); 
}
