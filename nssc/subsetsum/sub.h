/* This is the header file used for subsetsum programs */
#include <stdio.h>
#include <time.h>

#define NUM 100 		/* max size of the set */ 
#define PARTITION 1 		/* how sizes packed in tuple */  
#define len 15 			/* # of sizes put in a tuple */ 
char tpname[128] ;		/* name for tuple */ 
int ituple[NUM+2] ;		/* tuple structure for ts1 */	 
int ftuple[1] ;			/* global found tuple structure */	 
int buf[NUM+2] ;		/* returned tuple structure */ 
