/* This program uses random function to generate a set of n   */
/* numbers and makes up a target > all elements sum such that */
/* there is not subset sum equal to target. n, target and all */
/* set elements will put in a file called subsum.dat          */

#include <stdio.h>
#define NUM 100
#define MAX 100000

main()  				 
{
  void srand48() ;
  double drand48() ;
  int n, i, seed, s, A[NUM] ;
  FILE *fd ;
  double number ;
					/* program stzrts here */
  fd = fopen("subsum.dat","w") ;
  printf ("Please enter size of set : ") ;
  scanf ("%d",&n) ;
  fprintf (fd,"%d\n",n) ;		/* write size of the set to file */ 
  printf ("\nPlease enter a seed : ") ;
  scanf ("%d",&seed) ;
  srand48(seed) ;			/* initialize random function */ 
  s = 0 ;
  for (i=0; i<n; i++)
  {					 
    number = drand48() ;
    A[i] = 1 + (int)(MAX * number) ;
    s = s + A[i] ;
  }
  s = s + 1 ;				/* make sure > all element sum */
  fprintf (fd,"%d\n",s) ;		/* write target to the file */ 
  for (i=0; i<n; i++)
    fprintf (fd,"%d\n",A[i]) ;		/* write elements to the file */ 
}					/* end of program */ 
