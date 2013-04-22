/**********************************************************
 *   Guassian Elimination  sequential program         
 *
 *   File name : ga_seq.c    
 **********************************************************/

#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>
#include  <time.h>
#include  "gauss.h"

double    a[SIZE+1][SIZE+1];
double    sol[SIZE];
int       ind[SIZE];

double    check_num(double, int);

main()
{
  FILE     *fpa;
  time_t   tnow;
  struct   tm  *tmnow;
  int      hour, min, sec,seed;
  int      UB = 20, LB = -20, rang, avg;

  double  tmp1, tmp2, sum, rest;

  char    c;
  int     col, icol, irow, jrow;
  int     s_row,  e_row;

  double  max_val,num, tmp;
  int     N, M, i, j;
  int     Grain_Size,         /* allocate grain size       */
          N_Wave,             /* number of wave            */
          Cur_Wave=0;         /* current wave              */

  int     next_ind,         /* next starting row ind   */
          pivot_ind,        /* pivot row ind           */
          pivot_row,          /* pivot row                 */
          ix;                 /* starting row ind        */


  double  e_time;
  long    t0, t1;

  rang = UB - LB;
  avg = (UB - LB) / 2;

  N = SIZE;
     if ( !(fpa = fopen("seq_data","a+")))  {
        perror("Can't open data file - seq_data\n");
        exit(1);
        }

     Cur_Wave = 0;
     M = N + 1;
   
     time(&tnow);
     tmnow = localtime(&tnow);
     hour = tmnow->tm_hour;
     min  = tmnow->tm_min;
     sec  = tmnow->tm_sec;

     seed = 2 * ( hour + 1) * (min+1) / (sec+1)*10;
/*
     printf("Seed = %d\n", seed);
*/
     srand(seed);

     t0= time((long *)0);

                                       /*  random  generate solution  */
      for ( i = 0; i < N; i++)
        {
           tmp2 = (double) rand() / RAND_MAX;
           tmp1 = ( tmp2 ) * rang - avg;
           rest = check_num(tmp1, 1);
           sol[i] = rest;

        }

      max_val = 0.0;

      for ( i = 0; i < N; i++) {
         sum = 0.0;
         for ( j = 0; j < N; j++) {
            tmp2 = (double) rand() / RAND_MAX;
            tmp1 = ( tmp2 ) * rang - avg;
            rest = check_num(tmp1, 1);
            a[i][j] = rest;
            sum = sum + rest * sol[j];
            }

         a[i][N] = check_num(sum, 2);

  
         if ( fabs(a[i][0]) > max_val )
           {
             max_val = fabs(a[i][0]);
             pivot_row = i;
           }

         ind[i] = i;

         }   /*   for loop */


 /*
       printf("Master begin to work !!!! \n");
 */


       next_ind = 0;

       while ( Cur_Wave < ( N - 1 ))
          {

/*
           printf("The pivoting row is %d\n", pivot_row);
*/
           pivot_ind = ind[pivot_row];
           s_row = pivot_row - 1;
           for ( e_row = pivot_row; e_row > Cur_Wave; e_row--,s_row--)
                ind[e_row] = ind[s_row];

           ind[Cur_Wave] = pivot_ind;
/*
           for ( j = 0; j < N; j++)
             {
               printf("ind[%d] = %d ",j, ind[j]);
             }
           printf("\n");
*/
           next_ind = next_ind + 1;
           ix = next_ind;

           for ( ix = next_ind; ix < N ; ix++ )
            {
                j = ind[ix];
                tmp = a[j][Cur_Wave] / a[pivot_ind][Cur_Wave];
                for ( icol = Cur_Wave; icol <= N; icol++)
                   a[j][icol] = a[j][icol] - a[pivot_ind][icol] * tmp;
            }
/*
           for ( ix = 0; ix < N; ix++)
             {
               for (j=0; j<=N; j++)
                 printf("%lf\t",a[ix][j]);
               printf("\n");
             }
*/
           max_val = 0.0;
           i = next_ind;
           pivot_row = Cur_Wave + 1;

           while ( i <= N - 1 ) {
             j = ind[i];                     
               
             if ( fabs(a[j][Cur_Wave+1]) > max_val )
                 {
                     max_val = fabs(a[j][Cur_Wave+1]);
                     pivot_row = i;
                 }
             i++;
             }  
           Cur_Wave++;
      }  



/*                                             */
/*  back   substitution                        */
/*                                             */

      j = ind[N-1];
      sol[N-1] = a[j][N] / a[j][N-1];

      for ( irow = N-2; irow >= 0;irow--)  {

         j = ind[irow];
         tmp = a[j][N];
         for  ( jrow = N - 1; jrow > irow; jrow--)
           {
             tmp = tmp - a[j][jrow] * sol[jrow];
           }

         sol[irow] = tmp / a[j][irow];
    
         }   /*  end of for loop */
     if (N == 10)
     {
     printf("The sol => \n");
     for ( irow = 0; irow < N; irow++)
       printf("\t\t %8.2lf \n",sol[irow]);
     printf("\n");
     }
     t1 = time((long *)0) - t0;
     fprintf(fpa, " Size (%d) \tE_time (%d)\tCur_time - %s", N, t1, 
		ctime(&tnow));  
     fclose(fpa);
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

