/* This is the subsetsum parallel worker program.                 */
/* It firstly reads the set with its size and a target from       */
/* data tuple, then it goes into a loop. Each time it gets        */
/* a tuple, if it is data tuple, puts it back, if it is global    */
/* found tuple, also puts it back and check if it is true then    */
/* terminate, if it is sentinel tuple, puts it back again and     */
/* terminate, if it is working tuple, read found tuple to see     */
/* if anyone found solution yet, if not, go ahead and process     */
/* it. Whenever solution is found, send back result and terminate */

#include <stdio.h>
#include "sub.h"

int     ts1,                    /* tuple spaces */ 
	ts2, 
	status,                 /* returned status from cnf function */ 
	n,                      /* set size */
	target,                 /* target to look for */
	set[NUM],               /* set to look for */
	hops,                   /* effective hops */
	buf_length ;            /* # of bytes in tuple */ 
char    foundname[128] ;        /* name for global found tuple */ 

main()                         
{
  int   position,               /* element position in the set */
	sum,                    /* sum of elements */
	i ;                     
  char  readname[128],          /* name for reading the data tuple */
	previous[128] ;         /* for keeping previous tuple name */
				/* worker starts here */
  printf ("worker started \n") ;
  ts1 = cnf_open("sumlens",0) ;         /* open the tuple spaces */
  ts2 = cnf_open("results",0) ;
  strcpy(readname,"readset") ;
  strcpy(foundname,"found") ;
  strcpy(previous,"initialize") ; 
  status = cnf_tsread(ts1,readname,ituple,0) ;  /* read data tuple */ 
  n = ituple[0] ;
  target = ituple[1] ;
  for (i=0; i<n; i++)  {
    set[i] = ituple[i+2] ;              /* put elements in the set */
    buf[i+2] = -1 ;                     /* initialize elements position */
  }
  printf ("Size is %d and target is %d\n",n,target) ;
  while (1)  {
    strcpy(tpname,"*") ;
    status = cnf_tsget(ts1,tpname,ituple,0) ;
    if ((strcmp(tpname,foundname) == 0) || (strcmp(tpname,readname) == 0)) { 
      /* found tuple or data tuple, puts it back */
      status = cnf_tsput(ts1,tpname,ituple,status) ;
      if ((strcmp(tpname,foundname) == 0) && (ituple[0] == 1))  { 
	buf[0] = -1 ;
	buf[1] = 0 ;
	buf_length = 2 * sizeof(int);
	status = cnf_tsput(ts2,previous,buf,buf_length) ;
	printf ("someone found the solution, so worker terminated\n") ;
	cnf_term() ;
      }
    }
    else  {                             
      if (ituple[1] == n+1)  {          /* sentinel tuple */
	status = cnf_tsput(ts1,tpname,ituple,status) ;
	buf[0] = -1 ;
	buf[1] = 0 ;
	buf_length = 2 * sizeof(int) ;
	status = cnf_tsput(ts2,previous,buf,buf_length) ;
	printf ("worker found the sentinel tuple and terminated\n") ;
	cnf_term() ;
      }
      else  {                           /* valid working tuple */
	i = 1 ;
	while (i <= ituple[0])  {
					/* Is anyone found solution yet */
	  status = cnf_tsread(ts1,foundname,ftuple,0) ;
	  if (ftuple[0] == 1)  {        /* yes */
	    buf[0] = -1 ;
	    buf[1] = 0 ;
	    buf_length = 2 * sizeof(int) ;
	    status = cnf_tsput(ts2,tpname,buf,buf_length) ;
	    printf ("someone found the solution, so worker terminated.\n") ;
	    cnf_term() ;
	  }
	  else  {                       /* no, work on it */
	    hops = 0 ;                  /* reset effective hops */
	    position = 0 ;              /* restart position */
	    sum = 0 ;                   /* reset sum */
	    if (rec(ituple[i],position,sum) == 1)     /* solution found */  
	      found_term(ituple[i],hops) ;
	    else                                        
	      i++ ;
	  }
	}               /* loop back if still have sizes in the tuple */ 
      }    /* end of else that it is not the sentinel tuple */
    }      /* end of else that it is not the global found tuple */
    strcpy(previous,tpname) ;           /* keep previous tuple name */
  }        /* end of main loop */
}          /* end of worker program */

/* solution is found, set the global found tuple to    */
/* true, send back the result into ts2, and terminate. */
found_term(ssize,h)
int ssize ;
int h ;
{
  status = cnf_tsget(ts1,foundname,ftuple,0) ;
  ftuple[0] = 1 ;               
  status = cnf_tsput(ts1,foundname,ftuple,status) ;
  buf[0] = h ;                  /* effective hops */
printf(" worker hops (%d) \n",h);
  buf[1] = ssize ;              /* # of elements in solution subset */
  buf_length = (n+2) * sizeof(int) ;
  status = cnf_tsput(ts2,tpname,buf,buf_length) ;
  printf ("worker found the solution (%s) and terminated.\n",tpname) ;
  cnf_term() ;
}                               /* end of found_term function */

/* This function, takes size, position and surrent sum as arguments. */
/* It calls itself recursively to make up subsets of size-1, then it */
/* loop to check if an additional element is added and equal to      */
/* target, whether found or not, return done as result of the search */
int rec(ssize,p,s)
int ssize ;
int p ;
int s ;
{
  int   pos,
	done=0 ;

  pos = p ;                             /* copy current position */
  if (ssize == 1)  {                    /* require 1 element to = size */
    while ((pos < n) && (done == 0))  {
      hops++ ;                          /* add one node from search tree */
      if ((s + set[pos]) == target)  {
	buf[pos+2] = 1 ;                /* solution subset has this pos */
	done = 1 ;
      }
      else
	pos++ ;
    }
  }
  else  {                               /* require more elements to = size */
    while ((pos <= n-ssize) && (done == 0))  {
      buf[pos+2] = 1 ;
      hops++ ;
      if (rec(ssize-1,pos+1,s+set[pos]) == 1)      /* do recursive calls */
	done = 1 ;
      else  {
	buf[pos+2] = -1 ;               /* ignore this position */
	pos++ ;
      }
    }
  }
  return(done) ;
}                                       /* end of rec function */
