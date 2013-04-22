/*

  This is the function for testing the best chunk size.
  Written by Faisal G. Hassan and modify by many other people.
  see README

*/ 


#include <stdio.h>
#include <time.h>
#include "ch_header.h"
#define OUT fflush(stdout)

double ita[Number_chunk_test/2][Number_chunk_test];
double ott[Number_chunk_test/2][Number_chunk_test];

double second()
{
#include <sys/time.h>
#include <sys/resource.h>
struct rusage ru;
struct timeval tp;
struct timezone tzp;
double t;
gettimeofday(&tp, &tzp);
t = (tzp.tz_minuteswest*60 + tp.tv_sec+tp.tv_sec)*1.0e6 + (tp.tv_usec+tp.tv_usec)*1.0;
return t;
}

test_chunk_size(int tsd, int res)
{
	int G,  i, j, k, status;
	int ix, ia, ib, tplength;
	int offset;
	unsigned int hostid;
	int deltime;
	double dtime;
	
	hostid = gethostid();
	strcpy(tpname,"B*");
	second();
	deltime = time((long *)0);
	status = cnf_tsread(tsd, tpname, (double *)ituple_B, 0); 
	deltime = time((long *)0) - deltime;
printf(" mtwrk. received B (%s) elapse %d \n", tpname, deltime);
	tplength = (1+Number_chunk_test*Number_chunk_test)*sizeof(double);
	if ((ituple_A = (double *)malloc(tplength)) == NULL) 
		exit(1);
	/* receiving power calibration tuple */
	strcpy(tpname,"T*");
	tplength = cnf_tsget(tsd, tpname, ituple_A, 0);
	/* obtain the processor index */
	ix = atoi(&tpname[1]);
	dtime = second();
	G = (int) ituple_A[2];
	clock();
	memcpy(ita, ituple_A+3, G*Number_chunk_test*sizeof(double));
	memset(ott, 0x0, G*Number_chunk_test*sizeof(double));
	if ((otuple = (double *)malloc(tplength)) == NULL)
				exit(1);
	for (i =0; i < G; i++)
	    for (k =0; k < Number_chunk_test; k++)
		for (j =0; j < Number_chunk_test; j++)
		    ott[i][j] = ott[i][j] + ita[i][k] * ituple_B[k][j];
	dtime = (second() - dtime);
	otuple[0] = (double)(hostid);
	otuple[1] = (double)(dtime);
	otuple[2] = G;
	memcpy(otuple+3, ott, sizeof(double)*G*Number_chunk_test);
	sprintf(tpname,"S%d",ix);
printf(" mtwrk. put in (%s)   elapse %.3f\n",tpname, otuple[1]);
	tplength = 3 * sizeof(double);
	status = cnf_tsput(res, tpname, otuple, tplength);

        OUT;
	free(otuple);
	free(ituple_A);
}
