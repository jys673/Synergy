/* This program uses random function to generate a set of n */
/* numbers. Based on the subset size asked, the target will */
/* be the sum of the elements in the subset. n, target and  */
/* and set elements will all put in a file called subsum.dat */

#include <stdio.h>
#define NUM 100 
#define MAX 100000

main()			
{
  void srand48() ;
  double drand48() ;
  int sum, n, i, seed, x, subsize, A[NUM], B[NUM] ;
  FILE *fd ;
  double number ;
				/* program starts here */
  fd = fopen("subsum.dat","w") ;
  printf ("Enter size of the set : ") ;
  scanf ("%d",&n) ;
  printf ("\nEnter size of the solution subset : ") ;
  scanf ("%d",&subsize) ;
  fprintf (fd,"%d\n",n) ;
  printf ("\nEnter a seed number : ") ;
  scanf ("%d",&seed) ;
  srand48(seed) ;
  for (i=0 ; i<n; i++)
  {
    number = drand48() ;
    A[i] = 1 + (int)(MAX * number) ;
    B[i] = 1 ;
  }
  sum = 0 ;
  i = 0 ;
				/* to generate a subset of subsize numbers, */
        			/* so that all its members add up to target */
  printf ("The subset elements have these positions from the original set\n");
  while (i < subsize)  {  	 
    number = drand48() ;
    x = (int)(n * number) ;
    if (B[x] == 1)  {
      printf ("element position in the set is : %d\n",x) ;
      sum = sum + A[x] ;
      B[x] = -1 ;
      i++ ;
    }
  }
  fprintf (fd,"%d\n",sum) ;	/* write the target to the file */ 
  for (i=0; i<n; i++)  { 	/* write the set elements to the file */ 
    fprintf (fd,"%d\n",A[i]) ;
  }
  fclose(fd) ; 
}				/* end of the program */ 
