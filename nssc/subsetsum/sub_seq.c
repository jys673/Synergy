/* This is the subsetsum sequential program.                        */
/* It first reads s set with size(N), a target to look for,         */
/* and all its elements. Then it goes into a loop from size = 1     */
/* to N, each time in the loop, it explores all subsets with that   */
/* size to see if one of the subset's sum is equal to the target.   */
/* If it is, exit loop, otherwise loop back. After that, it prints  */
/* out the original set elements. If solution is found, prints out  */
/* the solution subset and the effective hops leading to the search */
/* of the solutio.                                                  */ 
/* Author: Hong Lee, September 1994                                 */
/* Revised: Yuan Shi, October 1994, cosmetic changes + stat file    */

#include "sub.h"

int     n,                              /* set size */
	target,                         /* target to look for */
	set[NUM],                       /* set to look for */
	solution[NUM] ;                 /* element positions set */
double  hops ;                          /* effective hops leading to solution*/
main()                                  /* main program starts here */
{
  int   found=0,                        /* indicate solution found or not */
	size,                           /* size of subset */
	position,                       /* element position */
	sum,                            /* sum of elements */
	i, j ;                           
  FILE  *fd, *fdd ;                           /* file pointer */
  long t0, t1, ttime ;                  /* for timing uses */
  char hostname[128];

  gethostname(hostname,sizeof(hostname));
  t0 = time((long *)0);                 /* get initial time */
  fd = fopen("subsum.dat","r") ;        /* open file and read data */
  fdd = fopen("subset_seq.time","a"); 
  fscanf (fd,"%d",&n) ;
  fscanf (fd,"%d",&target) ;
  printf ("Size is %d and target is %d.\n",n,target) ;
  for (i=0; i<n; i++)  {
    fscanf (fd,"%d",&set[i]) ;
    solution[i] = -1 ;                  /* set elements position */
  }
  fclose(fd) ;                          /* close the file */
  hops = 0 ;
  size = 1 ;                            /* initialize size=1 to start */
  while ((size <= n) && (found == 0))  {        /* loop for solution */
    position = 0 ;                      /* resatrt position */
    sum = 0 ;                           /* reset sum */
    if (rec(size,position,sum) == 1)    /* subset found from current size */ 
      found = 1 ;
    else                                /* no solution subset yet */
      size++ ;
  }
  t1 = time((long *)0) ;                        /* get finish time */
  ttime = (t1 - t0) ;           /* actual time used in seconds */
  printf ("\nThe original set elements are :\n") ;
  i = 1 ;
  for (j=0; j<n; j++)  
    if (i < 10)  {
      printf (" %5d,",set[j]) ;
      i++ ;
    }
    else  {
      printf (" %5d,\n",set[j]) ;
      i = 1 ;
    }
  printf ("\n") ;
  if (found == 1)  {                    /* solution subset is found */
    printf ("\nTarget %d has been found from sum of a subset.\n",target) ;
    printf ("The subset has the following elements.\n") ;
    fprintf (fdd, "\nTarget %d has been found from sum of a subset.\n",target) ;
    fprintf (fdd, "The subset has the following elements.\n") ;
    i = 1 ;
    for (j=0; j<n; j++)
      if (solution[j] == 1)  {
	if (i < 10)  {
	  printf (" %5d,",set[j]) ;
	  fprintf (fdd, " %5d,",set[j]) ;
	  i++ ;
	}
	else  {
	  printf (" %5d,\n",set[j]) ;
	  fprintf (fdd, " %5d,",set[j]) ;
	  i = 1 ;
	}
      }
    printf ("\n") ;
    printf ("The effective hops leading to this solution is %f.\n",hops) ;
    fprintf (fdd, "\n") ;
    fprintf (fdd, "The effective hops leading to this solution is %f.\n",hops) ;
  }
  else  {                                 /* no solution subset */
    printf ("\nTarget %d has not been found.\n",target) ;
    fprintf (fdd, "\nTarget %d has not been found.\n",target) ;
  }
  printf ("Time spent for this program is %d seconds.\n",ttime) ;
  fprintf (fdd, "(%s)Time spent for this program is %d seconds.\n",hostname, 
		ttime) ;
  fclose(fdd);
}                                       /* end of program */

/* This function, takes size, position and current sum as arguments. */
/* It calls itself recursively to make up subsets of size-1, then it */ 
/* loop to check if an additional element is added and equal to      */
/* target, either found or not, return done as result of search      */
int rec(l,p,s)
int l ;
int p ;
int s ;
{
  int   pos,                    
	done=0 ;

  pos = p ;                             /* copy current position */
  if (l == 1)  {                        /* require 1 element to be size */
    while ((pos < n) && (done == 0))  {
      hops++ ;                          /* add one node in search tree */
      if ((s + set[pos]) == target)  {
	solution[pos] = 1 ;             /* solution has this position */
	done = 1 ;                      
      }
      else
	pos++ ;
    }
  }
  else  {                               /* require more elements */
    while ((pos <= n-l) && (done == 0))  {
      solution[pos] = 1 ;
      hops++ ;
      if (rec(l-1,pos+1,s+set[pos]) == 1)       /* do recursive calls */
	done = 1 ;
      else  {
	solution[pos] = -1 ;            /* ignore this position */
	pos++ ;
      }
    }
  }
  return(done) ;                                
}                                        /* end of function */
