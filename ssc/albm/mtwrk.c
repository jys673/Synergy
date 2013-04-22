/*----------------------------------------------------
Matrix multiplication worker program.

Authors:

Feijian Sun for adaptation of XDR, 08/22/94
Wenhai Jiang for automatic load balancing, 9/22/94
Yuan Shi for correction in load calculation formula, 10/1/94
----------------------------------------------------*/
#include <stdio.h>
#include <time.h>
#include "matrix.h"

double ita[N/2][N];
double ott[N/2][N];
double wall_clock();

main()
{
	int G, tsd, res, i, j, k, status;
	int ix, ia, ib, tplength;
	int offset;
	unsigned int hostid;
	int deltime;
	double dtime;
	
	hostid = gethostid();
	tsd = cnf_open("problem",0);
	res = cnf_open("result",0);

	strcpy(tpname,"B*");
	deltime = wall_clock();
	status = cnf_tsread(tsd, tpname, (double *)ituple_B, 0); 
printf(" mtwrk. received B (%s) elapse %d \n", tpname, wall_clock()-deltime);
	tplength = (1+N*N)*sizeof(double);
	if ((ituple_A = (double *)malloc(tplength)) == NULL) 
		exit(-7);
	/* receiving power calibration tuple */
	strcpy(tpname,"T*");
	tplength = cnf_tsget(tsd, tpname, ituple_A, 0);
	/* obtain the processor index */
	ix = atoi(&tpname[1]);
	G = (int) ituple_A[2];
	memcpy(ita, ituple_A+3, G*N*sizeof(double));
	memset(ott, 0x0, G*N*sizeof(double));
	if ((otuple = (double *)malloc(tplength)) == NULL)
				exit(-7);
	for (i =0; i < G; i++)
	    for (k =0; k < N; k++)
		for (j =0; j < N; j++)
		    ott[i][j] = ott[i][j] + ita[i][k] * ituple_B[k][j];
	dtime = (wall_clock() - deltime);
	otuple[0] = (double)(hostid);
	otuple[1] = (double)(dtime);
	otuple[2] = G;
	memcpy(otuple+3, ott, sizeof(double)*G*N);
	sprintf(tpname,"S%d",ix);
printf(" mtwrk. put in (%s)   elapse %.3f\n",tpname, otuple[1]);
	tplength = 3 * sizeof(double);
	status = cnf_tsput(res, tpname, otuple, tplength);
	free(otuple);

	while (1)  	
	{
		strcpy(tpname,"A*");
		tplength = cnf_tsget(tsd, tpname, ituple_A, 0);
printf(" mtwrk got (%s) \n",tpname);
		ix = atoi(&tpname[1]);
		if (tplength > 0) {		/* normal receive */
			G = (int) ituple_A[0];
printf(" mtwrk ix (%d) G(%d) \n",ix, G);
		/* check for the application termination signal */
			if (G == -1) {
				status = cnf_tsput(tsd, tpname, ituple_A, tplength);
			deltime = time((long *)0);
			printf("Worker Terminated %s\n",ctime(&deltime)); 
				cnf_term();
				return;
			}
			else {
			    memcpy(ita, ituple_A+1, G*N*sizeof(double));
			    memset(ott, 0x0, G*N*sizeof(double));
			}
/* Use these lines if your machine does not support memcpy, memset ...
			else {
				for (i = 0; i < G; i++)
				for (j = 0; j < N; j++)
				{
					ita[i][j] = ituple_A[i*N+j+1];
					ott[i][j] = 0;
				}
			}
*/
			if ((otuple = (double *)malloc(tplength)) == NULL)
				exit(-7);
			for (i =0; i < G; i++)
			    for (k =0; k < N; k++)
				for (j =0; j < N; j++)
				    ott[i][j] = ott[i][j] + ita[i][k] * ituple_B[k][j];
			otuple[0] = G;
			memcpy(otuple+1, ott, sizeof(double)*G*N);
/* Use these lines if your machine does not support memcpy, memset ...
                        for (i = 0; i < G; i++)
                            for (j = 0; j < N; j++)
                                otuple[i*N+j+1] = ott[i][j];
*/
			sprintf(tpname,"%d\0",ix);
			status = cnf_tsput(res, tpname, otuple, tplength);
			free(otuple);
		} else {
			printf("Worker Terminated %s\n",ctime(time((long *)0))); 
			cnf_term();
			return;
		}
	}
}
