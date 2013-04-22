/**********************************************************
 *   Guassian Elimination  parallel program -- master
 *
 *   File name : ga_master.c
 *
 *   tuple space used :
 *       
 *      ts1 :
 *            wiix  : working index (the range which each worker need to work on)
 *                    i  -- wave index
 *                    ix -- row index
 *      ts2 :
 *            IDX : store index of each original row
 *            i   : each row data
 *            Ei  : end of each wave that each worker respond 
 *            ETi : end of each matrix that each worker respond
 *
 **********************************************************/

#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>
#include  <time.h>
#include  "gauss.h"

double    check_num();

main()
{
  FILE     *fpa;
  time_t   tnow;
  struct   tm  *tmnow;
  int      hour, min, sec,seed;
  int      UB = 20, LB = -20, rang, avg;
  int      counter;

  double  *a, *sol;  
  double  tmp1, tmp2, sum, rest;

  int     *ind;  
  int     c[1];
  int     col, icol, irow, jrow;
  int     s_row,  e_row;

  double  max_val,num, tmp;
  int     N, M, i, j;
  int     Grain_Size,         /* allocate grain size       */
          N_Wave,             /* number of wave            */
          Cur_Wave=0;         /* current wave              */

  int     next_ind,           /* next starting row index   */
          pivot_ind,          /* pivot row index           */
          pivot_row,          /* pivot row                 */
          ix;                 /* starting row index        */


  int     status, tsd, tplength, res, len;
  float   f;
  int     G,         /* Tuple grain size     */
          R,         /* Remainder  size      */
          t=1,       /* cutoff value         */
          P;         /* number of processor  */

  double  e_time;
  long    t0, t1;
  
     tsd = cnf_open("ts1", 0);
     res = cnf_open("ts2", 0);
     if ( !(fpa = fopen("gauss_par.time","a+")))  {
         perror("Can't open stat file? \n");
         exit(1);
     }
     f = (float) cnf_getf();
     f = f / 100.0;
     P = cnf_getP();
     t = cnf_gett();

     printf(" get f(%f) P(%d) t(%d) \n", f, P, t);

     rang = UB - LB;
     avg = (UB - LB) / 2;

     time(&tnow);
     tmnow = localtime(&tnow);
     hour = tmnow->tm_hour;
     min  = tmnow->tm_min;
     sec  = tmnow->tm_sec;
  
     seed = ( hour + 1) * (min+1) / (sec+1)*10;
     printf("Seed = %d\n", seed);

     N = SIZE;
     Cur_Wave = 0;
     M = N + 1;
   
     time(&tnow);
     tmnow = localtime(&tnow);


     if (( a = (double *) malloc( M * sizeof(double)) ) == NULL)
     {
          printf("Unable to alloc for Array A\n");
          exit(1);
     }

     if (( ind = (int *) malloc( N * sizeof(int)) ) == NULL)
     {
          printf("Unable to alloc for Array Index\n");
          exit(1);
     }

     if (( sol   = (double *) malloc( N * sizeof(double)) ) == NULL)
     {
          printf("Unable to alloc for Array sol \n");
          exit(1);
     }


     t0 = time((long *)0);
/*    < step 0 > data generated        */

     srand(seed);

     printf("before   generate data \n");
                                       /*  random  generate solution  */
     for ( i = 0; i < N; i++)
     {
/*
           printf(" i = %d ", i);
*/
           tmp2 = (double) rand() / RAND_MAX;
           tmp1 = ( tmp2 ) * rang - avg;
           rest = check_num(tmp1, 1);
           sol[i] = rest;
/* 
                   printf("%lf\n ",sol[i]);
*/
     }

     printf("generate sol for size (%d) o.k.\n", N);

/*    < step 1 > put array A into TS2 */


      max_val = 0.0;
      tplength = M * sizeof(double);

      for ( i = 0; i < N; i++) 
      {
         sum = 0.0;
         for ( j = 0; j < N; j++) 
         {
            tmp2 = (double) rand() / RAND_MAX;
            tmp1 = ( tmp2 ) * rang - avg;
            rest = check_num(tmp1, 1);
            a[j] = rest;
            sum = sum + rest * sol[j];
         }

         a[N] = check_num(sum, 2);

         if ( fabs(a[0]) > max_val )
         {
             max_val = fabs(a[0]);
             pivot_row = i;
         }

         ind[i] = i;

         sprintf(tpname, "%d",i);
  /*
                 printf("Put Array  A  in tuple ts2 (%s)\n", tpname);
  */
         status = cnf_tsput(res, tpname, a, tplength);
         if ( status < 0 ) 
         {
             printf("  Error : During master put row (%d) of array A into TS2 -- tpname (%s)\n",
                        i, tpname);
             exit(-1);
         }
      }   /*   end   of    for   loop    */
/*
printf("The pivoting row is %d\n", pivot_row);
*/
   
printf("Master begin to work !!!! \n");
   
/*  put array  IDX  into TS2    */
       strcpy(tpname,"IDX");
       tplength = N * sizeof(int);

       if ( ( status = cnf_tsput(res, tpname, ind, tplength)) < 0)  
       {
          printf("  Error : During master put array Index -- (%s) into TS2\n",
                       tpname);
          exit(-1);
       }


       next_ind = 0;
       Grain_Size = N;

       while ( Cur_Wave < ( N - 1 ))
       {
/*    < step 2 > Chang index from IDX tuple in TS2  */

           strcpy(tpname,"IDX");
           tplength = N * sizeof(int);

           if ( ( len = cnf_tsget(res, tpname, ind, 0)) != tplength )  
           {
               printf("  Error : During master GET array Index -- (%s) into TS2\n",
                       tpname);
               exit(-1);
           }


           pivot_ind = ind[pivot_row];
           s_row = pivot_row - 1;
           for ( e_row = pivot_row; e_row > Cur_Wave; e_row--,s_row--)
                ind[e_row] = ind[s_row];

           ind[Cur_Wave] = pivot_ind;

           strcpy(tpname,"IDX");
           tplength = N * sizeof(int);

           if ( ( status = cnf_tsput(res, tpname, ind, tplength)) < 0)  
	   {
                 printf(" Error : During master put array Index back -- (%s) into TS2\n",
                       tpname);
                 exit(-1);
           }

/*    < step 3 > put worker into tuple space ts1  */
/*               factoring  based on number of processors */

           Grain_Size = Grain_Size - 1;
           R = Grain_Size;
           next_ind = next_ind + 1;
           ix = next_ind;
           G = 1;

           while ( (R > t) && (G > 0) )  
	   {
             G = (int) R*f/P;
             R = R - G * P;

             if ( G > 0 )  
             {
                for (i = 0; i < P; i++) 
                {
                  ituple[0] = G;
                  ituple[1] = ix;
                  ituple[2] = Cur_Wave;
                  ituple[3] = N;

                  tplength = 4 * sizeof(int);
                  sprintf(tpname, "w%d%d",Cur_Wave ,ix);

                  printf("Put worker index into ts1 (%s) G(%d) ix(%d) wave(%d) \n",
                        tpname, G, ix,Cur_Wave);

                  status = cnf_tsput(tsd, tpname, ituple, tplength);

                  ix += G;
                }   /*   end of for loop */
             }      /*   end of if       */
           }          /*   end of while loop */
printf("Master finished job packaging, Remaining (%d) \n",R);
           ituple[0] = R;        /*  assign the rest to one tuple */
           if (R == 0) ituple[1] = -1;
	   else ituple[1] = ix;
           ituple[2] = Cur_Wave;
           ituple[3] = N;

           sprintf(tpname, "w%d%d", Cur_Wave, ix);

           printf("Put remaining (%s) R(%d) ix(%d) wave(%d)\n",
                  tpname, R, ix,Cur_Wave);

           tplength = 4 * sizeof(int);

           status = cnf_tsput(tsd, tpname, ituple, tplength);


           if (R != 0) {
		ituple[0] = 0;
           	ituple[1] = -1;
           	ituple[2] = -1;
           	ituple[3] = -1;
           	ituple[4] = SIZE;

           	sprintf(tpname, "w%d0",Cur_Wave);

             	printf("############ Master put wave term (%s) in TS1\n", tpname);
 
           	tplength = 4 * sizeof(int);
           	status = cnf_tsput(tsd, tpname, ituple, tplength);
	   } 

   
/*    master wait for the singal to move on */
     
           for ( i = 0; i < P; i++)
           {
               sprintf(tpname,"E(w%d*",Cur_Wave);    
printf("++++ Master wants (%s)\n",tpname);
               len = cnf_tsget(res, tpname, c, 0);

               printf("Master recv tpname (%s) length (%d)\n",
                  tpname, len);

           }
   

/*                                                                            */
/*   < step 4 > read from tuple space ts2 and continue to work on next wave   */
/*                                                                            */


           tplength = M * sizeof(double);

           max_val = 0.0;
           i = next_ind;

           pivot_row = Cur_Wave + 1;

           printf("&&&&&&&&&&  Master read each row's data             &&&& \n");
           while ( i <= N - 1 ) 
	   {
             sprintf(tpname, "%d",ind[i]);
             len = cnf_tsread(res, tpname,  a, 0); /* blocking read */
printf("------ Master in read loop want to read (%s)\n",tpname); 
             if ( len != tplength )
             {
                 printf("Error : During read check from TS2 -- (%s)",
                          tpname);
                 exit(1);
             }
             if ( fabs(a[Cur_Wave+1]) > max_val )
             {
                     max_val = fabs(a[Cur_Wave+1]);
                     pivot_row = i;
             }
             i++;
           }   /* end of while loop   */

/*   master take the wave tuple out   */ 

           sprintf(tpname,"w%d0",Cur_Wave);

           printf("The master take out wave termination tuple--wave = %d\n", Cur_Wave);

           tplength = 4 * sizeof(int);
           len = cnf_tsget(tsd, tpname, ituple, 0);
           Cur_Wave++;
      }    /*    end  of while loop   Cur_Wave < N - 1  */


/*   at this moment all the worker finished their jobs   */
/* 
      
      for ( i = 0; i < P; i++)
      {
            strcpy(tpname,"WT*");    
            len = cnf_tsget(res, tpname, c, 0);
            printf("Master recv * term * tpname (%s) length (%d)\n",
               tpname, len);

      }
*/

/*                                             */
/*  back   substitution                        */
/*                                             */

      printf(" Master starting back-substitution... \n");
      tplength = M * sizeof(double);

      sprintf(tpname, "%d",ind[N-1]);
      len = cnf_tsget(res, tpname,  a, 0); /* blocking read */

      sol[N-1] = a[N] / a[N-1];
      for ( irow = N-2; irow >= 0;irow--)  {
         sprintf(tpname, "%d",ind[irow]);
         len = cnf_tsget(res, tpname,  a, 0); /* blocking read */
         tmp = a[N];
         for  ( jrow = N - 1; jrow > irow; jrow--)
         {
             tmp = tmp - a[jrow] * sol[jrow];
         }

         sol[irow] = tmp / a[irow];
    
     }   /*  end of for loop */

     strcpy(tpname,"IDX");
     tplength = N * sizeof(int);

     len = cnf_tsget(res, tpname, ind, 0);

     t1 = time((long *)0) - t0;

     free(a);
     free(sol);
     free(ind);

     printf(" cur time = %s\n", ctime(&tnow));
   
     fprintf(fpa, " Size (%d)\te_time (%lf)\tPro.(%d)\tCur_time- %s", 
		N,t1,P,ctime(&tnow));  
     fclose(fpa);
     ituple[0] = 0;
     ituple[1] = 0;
     ituple[2] = 0;
     ituple[3] = 0;


     tplength = 4 * sizeof(int);
     sprintf(tpname,"w0");

     status = cnf_tsput(tsd, tpname, ituple, tplength);

     printf("Put worker Termination tuple into TS1 (%s) \n",
               tpname );
     cnf_term();

}          /*  end of main   */


double  check_num(arg_num, iter)
double arg_num;
int    iter;
{
   int   int_part, fract_part;
   int   base = 1;
   int   i;
   double  result, temp;

   for ( i = 1; i <= iter; i++)
     base = base * 10;

   int_part = (int) arg_num;
   temp = arg_num - int_part;
   fract_part = (int) (temp * base);
   result = int_part + fract_part / (double) base;
   return result;
}

