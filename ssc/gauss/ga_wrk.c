/************************************************************************
 *
 *    Guassian  elimination   worker
 *
 *    File name : guwrk.c
 *
 ************************************************************************
 */
#include  <stdio.h> 
#include  "gauss.h" 

void      mem_alloc(void);
main()
{

double  *a, *b;
int      *ind;

int       N, M, first=0;

    FILE        *fpb;
    char        filename[50];

    double      tmp;  

    int         g,
                num_elm;
    
    int         finish_wave=0,
                max_wave=-1,
                prev_ix=0;

    int         G,
                ix,
                col,
                icol;

    int         tsd, res,               /* Space object handels */
                tplength,
                len,
                c[1],
                status;

    int         i;  
    char host[128];
/*
    gethostname(host,sizeof(host));
*/ 

    tsd = cnf_open("ts1", 0);
    res = cnf_open("ts2", 0);

    printf("  Worker enter infinite loop  \n");


    while ( 1 ) {                       /* loop forever  */

        sprintf(tpname,"w%d*",finish_wave);
printf(" ??? worker wants (%s)\n",tpname); 
        tplength = cnf_tsget( tsd, tpname, ituple, 0 );

        printf("   Work  get (%s) tuple G(%d) ix(%d) col(%d) N(%d)\n",tpname,ituple[0], ituple[1], ituple[2], ituple[3]);

        if ( tplength < 0 ) 
            {           
               strcpy(tpname, "*");
               printf("tpname (%s) length (%d)--error \n",tpname, tplength);
	       exit(-2); /* error */ 
            }
                                    /* normal receive */


                G  = ituple [0];
                ix          = ituple [1];
                col         = ituple [2];
                N           = ituple [3];


                if  ( G == 0 ) 
		{  
                     if ( col == 0 )
                     {
                     		status = cnf_tsput(tsd, tpname, ituple, tplength);
                      		printf(" Worker found the last tuple. Term.\n");
                      		cnf_term(); 
                     } 
                     printf("** wave terminal. worker put term back (%s) len(%d) \n",tpname,tplength);
		     ix++;
			ituple[1] = ix;
                        status = cnf_tsput(tsd, tpname, ituple, tplength);
                        printf(" Worker found wave - %d term tuple.     \n",finish_wave);

                        sprintf(tpname,"E(w%d)%d", finish_wave,ix/*prev_ix*/);
                        tplength = 1 * sizeof(int);
                      	c[0] = 0;
                      	status = cnf_tsput(res, tpname, c, tplength);

                        printf("Just put tpname (%s) into Ts2\n",tpname);
                                  
                        prev_ix = 0;
                        finish_wave++;

                        if ( finish_wave > max_wave )
                        {
                               sprintf(tpname,"WT%d", prev_ix);
                               tplength = 1 * sizeof(int);
                               c[0] = 0;
                               status = cnf_tsput(res, tpname, c, tplength);
                                        
                               first = 0;              
                               finish_wave = 0;
                                        
                       	       free(a);
                       	       free(b);
                       	       free(ind);
                      	       continue;
                         }
		} else {
/*  
                printf("***** worker receive tpname %s G(%d) ix(%d) col(%d) N(%d)\n",tpname, G, ix, col,N);
*/
/*  read Array Index    */
                
                if (first == 0)
                 {
                   finish_wave = 0;
                   max_wave    = N - 2;

                   printf("  **** worker define work area for N = %d, max_wave = %d\n", N,max_wave);

                   M = N + 1;

    if (( a = (double *) malloc( M * sizeof(double)) ) == NULL)
        {
          printf("Unable to alloc for array  A\n");
          exit(1);
        }

    if (( b = (double *) malloc( M * sizeof(double)) ) == NULL)
        {
          printf("Unable to alloc for array  B\n");
          exit(1);
        }

    if (( ind = (int *) malloc( N * sizeof(int)) ) == NULL)
        {
          printf("Unable to alloc for index\n");
          exit(1);
        }

          first = 1;
           }
                strcpy(tpname,"IDX");
                tplength = N * sizeof(int);

                if ( ( len = cnf_tsread(res, tpname, ind, 0)) != tplength )  {
                         printf("  Error : act len %d accur length = %d\n",
                                    len, tplength);
                         printf("  Error : During worker READ array Index -- (%s) into TS2\n",
                             tpname);
                         exit(-1);
                         }

/*   read  pivot  row data  */
                sprintf(tpname, "%d", ind[col]);

                printf("** Worker want to read pivot (%s) from ts2\n",tpname);

                tplength = M * sizeof(double);
                len = cnf_tsread(res, tpname, a, 0);
                if ( len != tplength )  {
                     printf(" Error : act len %d accur length = %d\n",
                               len, tplength);
                     printf(" Read error : During worker read %s tuple from TS2\n",
                        tpname);
                     exit(2);
                     }   /*  if cond   */

                /* compute and packing the results */


                for (g=0; g<G; g++) {
/*  get row ix from res */
                        sprintf(tpname, "%d",ind[ix]);
/*
                        printf("** worker want to read (%s) from ts2\n",
                          tpname);
*/
                        tplength = cnf_tsget(res, tpname, b, 0);
/*  blocking get  */
                        tmp = b[col] / a[col];
                        for ( icol = col; icol <= N; icol++)
                              b[icol] = b[icol] - a[icol] * tmp;

/*  put row ix into res */
  

                        printf("Procesed %s tuple \n", tpname);

                        status = cnf_tsput( res, tpname, b, tplength );
                        if ( status < 0 )  {
                           printf(" Put error : during worker %s tuple into TS2\n",
                                    tpname);
                           exit(2);
                           }   /*  if cond   */
                        ix++;
/* 
                        printf("The output row is %s\n",tpname);
                        for (i = 0; i< M;i++)
                          printf("%lf ",b[i]);
                        printf("\n");
*/ 
                        }    /*  end of for loop  */
		} /* of normal working cycle */
     }      /*  end of while loop  */
}
