/*----------------------------------------------------
Matrix multiplication worker program.

Author: Faisal G. Hassan
Date:   08/09/1994

Modified by Feijian Sun, 08/22/94
----------------------------------------------------*/
#include <stdio.h>
#include "matrix.h"

double ita[N/2][N];
double ott[N/2][N];
double wall_clock();

main()
{
	int G, tsd, res, i, j, k, status;
	int ix, ia, ib, tplength;
	double t0, t1;
	//sleep(20);

	tsd = cnf_open("problem",0);

	res = cnf_open("result",0);

	strcpy(tpname,"B*");
	status = cnf_tsread(tsd, tpname, (double *)ituple_B, 0); 
    //printf(" mtwrk. received B (%s) \n", tpname);
	tplength = (1+N*N)*sizeof(double);
	if ((ituple_A = (double *)malloc(tplength)) == NULL) 
		exit(-1);
	while (1)  		/* loop forever */
	{
		printf("Worker waiting for a tuple \n");
		strcpy(tpname,"A*");
		tplength = cnf_tsget(tsd, tpname, ituple_A, 0);
		t0 = wall_clock();

		//printf(" mtwrk got (%s) \n",tpname);

		ix = atoi(&tpname[1]);
		if (tplength > 0) {		/* normal receive */
			G = (int) ituple_A[0];

			printf(" mtwrk got ix (%d) G(%d) \n",ix, G);

		/* check for the application termination signal */
			if (G == -1) {
				status = cnf_tsput(tsd, tpname, ituple_A, tplength);
				cnf_term();
				return;
			}
			for (i = 0; i < G; i++)
				for (j = 0; j < N; j++)
				{
					ita[i][j] = ituple_A[i*N+j+1];
					ott[i][j] = 0;
				}
			if ((otuple = (double *)malloc(tplength)) == NULL)
				exit(-1);
			otuple[0] = ituple_A[0];

			for (i =0; i < G; i++)
			    for (k =0; k < N; k++)
			    	for (j =0; j < N; j++)
			    		ott[i][j] = ott[i][j] + ita[i][k] * ituple_B[k][j];


			for (i = 0; i < G; i++)
				for (j = 0; j < N; j++)
					otuple[i*N+j+1] = ott[i][j];


			 sprintf(tpname,"%d\0",ix);
			 //printf(" mtwrk. put in (%s) \n",tpname);
			 status = cnf_tsput(res, tpname, otuple, tplength);
			 t1 = wall_clock() - t0;
			 //printf(" Worker MFLOPS = (%f) \n", N*N*G/t1);
			 free(otuple);
		} else {
			printf("Worker Terminated \n"); 
			cnf_term();
			return;
		}
	}

//#endif
}
