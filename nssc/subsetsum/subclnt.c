/* This is the subsetsum parallel master program.                 */
/* It firstly reads the size of the set, the target and all its   */
/* elements, then it packed them up as a tuple and put it into    */ 
/* tuple space. It then ckecks if PARTITION = 1 or 2, if it is 1, */
/* then based on "len", it packs len consecutive subset sizes as  */
/* a tuple and puts into ts1, if it is 2, then it packs subsizes  */
/* based on size of the set modulo len. After doing these, if     */
/* there are still subsizes left, it packs and puts them up.      */
/* It also puts a sentinel and a global found tuple into ts1.     */
/* Then it starts receiving results from ts2, whether solution is */
/* found or not, it prints the results and terminates             */ 

#include "sub.h"

main()                         
{
  long  t0, t1, ttime ;         /* for timing uses */
  int   status,                 /* return status from cnf function */
	ts1,                    /* tuple spaces */
	ts2, 
	n,                      /* set size */
	target,                 /* target to look for */
	set[NUM],               /* set to look for */
	tplength,               /* length of tuple */
	R,                      /* remainder left to put into tuple space */
	group,                  /* number of groups being partitioned */     
	current,                /* current index pointer */
	P ;                     /* number of processors invloved */
  int   i, j, k, res, done ;      
  int  fd ;                   /* file pointer */
  char buff[128]; 
  FILE *fdd;
				/* master program starts here */
  t0 = time((long *)0) ;                /* get starting time */   
  printf ("master started...\n") ;
  ts1 = cnf_open("sumlens",0) ;
  ts2 = cnf_open("results",0) ;
  fd = cnf_open("input","r") ;
  fdd = fopen("subset.time", "a");
  cnf_fgets(fd, buff, 128); 
  n = atoi(buff) ;         /* # of elements in the set */ 
  ituple[0] = n ;
  cnf_fgets(fd, buff, 128);
  target = atoi(buff);     /* target to be looked for */ 
  ituple[1] = target ;
  for (k=0; k<n; k++)  {
    cnf_fgets(fd, buff, 128);
    set[k] = atoi(buff);   /* read set elements */ 
    ituple[k+2] = set[k] ;
  }
  printf ("Size is %d and target is %d\n",n,target) ;
  cnf_close(fd) ;
  P = cnf_getP() ;                      /* get # of processors */
  strcpy(tpname,"readset") ;            /* put the data tuple into ts1 */ 
  tplength = (n + 2) * sizeof(int) ;
  status = cnf_tsput(ts1,tpname,ituple,tplength) ;
  tplength = (len + 1) * sizeof(int) ;
  if (PARTITION == 1)  {        /* consecutive "len" subset sizes */
    R = n ;
    current = 1 ;
    while ((R - len) >= 0)  {     
      sprintf (tpname,"i%d\0",current) ;
      ituple[0] = len ;
      for (i=0; i<len; i++)
	ituple[i+1] = current + i ;
      status = cnf_tsput(ts1,tpname,ituple,tplength) ;
    printf(" Master put out (%s)\n",tpname);
      R = R - len ;
      current = current + len ;
    }
  }
  else  {                       /* PARTITION=2, group by modulo "len" */
    group = n / len ;
    for (j=1; j<=group; j++)  {
      sprintf (tpname,"i%d\0",j) ;
      ituple[0] = len ;
      for (i=1; i<=len; i++)
	ituple[i] = ((i - 1) * group) + j ;
      status = cnf_tsput(ts1,tpname,ituple,tplength) ;
    printf(" Master put out (%s)\n",tpname);
    }
    current = (len * group) + 1 ;
    R = n - (len * group) ;
  }
  if (R > 0)  {                 /* pack the rest as a tuple and put */ 
    sprintf (tpname,"i%d\0",current) ;
    ituple[0] = R ;
    for (k=0; k<R; k++)
      ituple[k+1] = current + k ;
    tplength = (R + 1) * sizeof(int) ;
    status = cnf_tsput(ts1,tpname,ituple,tplength) ;
    printf(" Master put out (%s)\n",tpname);
  }
  sprintf (tpname,"i%d\0",n+1) ;      /* put sentinel tuple */
  ituple[0] = 0 ;
  ituple[1] = n + 1 ;
  tplength = 2 * sizeof(int) ; 
  status = cnf_tsput(ts1,tpname,ituple,tplength) ;
  strcpy(tpname,"found") ;      /* put global found tuple */
  ituple[0] = -1 ;
  tplength = sizeof(int) ;
  status = cnf_tsput(ts1,tpname,ituple,tplength) ;
  res = 1 ;
  done = 0 ;
  printf ("master start receiving...\n") ;
  while ((res <= P) && (done == 0))  {    
    strcpy(tpname,"*") ;
    status = cnf_tsget(ts2,tpname,buf,0) ;
    printf(" Master received tpname (%s)\n", tpname);
    if (buf[0] > 0)                     /* solution found */
      done = 1 ;
    else                                /* no, loop for another receive */
      res++ ;
  }                                     /* finish receiving */
  cnf_close(ts1) ;                      /* close the tuple spaces */ 
  cnf_close(ts2) ;
  t1 = time((long *)0) ;                        /* get finish time */
  ttime = (t1 - t0);                    /* total time in seconds */
					/* print out the original set */
  printf ("\nThe original set elements are : \n") ;
  fprintf (fdd, "\nThe original set elements are : \n") ;
  j = 1 ;
  for (i=0; i<n; i++)  
    if (j < 10)  {
      printf (" %5d,",set[i]) ;
      fprintf (fdd, " %5d,",set[i]) ;
      j++ ;
    }
    else  {
      printf (" %5d,\n",set[i]) ;
      fprintf (" %5d,",set[i]) ;
      j = 1 ;
    }
  printf ("\n") ;
  fprintf (fdd, "\n") ;
  if (done == 1)  {                     /* solution is found */
    printf ("\nTarget %d has been found from sum of a subset.\n",target) ;
    printf ("The subset has the following elements.\n") ;
    fprintf (fdd,"\nTarget %d has been found from sum of a subset.\n",target) ;
    fprintf (fdd,"The subset has the following elements.\n") ;
    i = 2 ;
    j = 0 ;
    k = 1 ;
    while (j < buf[1])  {
      if (buf[i] == 1)  {
	if (k < 10)  {
	  printf (" %5d,",set[i-2]) ;
	  fprintf (fdd," %5d,",set[i-2]) ;
	  k++ ;
	}
	else  {
	  printf (" %5d,\n",set[i-2]) ;
	  fprintf (fdd," %5d,\n",set[i-2]) ;
	  k = 1 ;
	}
	j++ ;
      }
      i++ ;
    }
    printf ("\n") ;
    printf ("Effective hops leading to this solution are %d.\n",buf[0]) ;
    fprintf (fdd,"\n") ;
    fprintf (fdd,"Effective hops leading to this solution are %d.\n",buf[0]) ;
  }
  else {                                 /* solution is not found */
    printf ("\nTarget %d has not been found.\n",target);
    fprintf (fdd,"\nTarget %d has not been found.\n",target);
  }
  printf ("Time spent for this program is %d seconds.\n",ttime) ;
  fprintf (fdd,"Time spent for this program is %d seconds.\n",ttime) ;
  fclose(fdd);
  cnf_term() ;                          /* terminate */
}                                       /* end of master program */
