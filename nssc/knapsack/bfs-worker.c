#include <stdio.h>
#include <math.h>
#include "bfs.h"

#define SEED 111


typedef struct elem {
	double wt;
	double profit;
	double pbyw;
} ELEM;

typedef struct node {
	double cu;
	double profit;
	double ub;
	int tag;
	struct node *next;
	struct node *previous;
	//struct node *parent;
} NODE;


void printNode(NODE *);

ELEM *items = NULL;
NODE *qLive = NULL, *endQLive = NULL;
double numOps = 0;
long numLiveNodes = 0; /*number of live nodes at a level, changes with level */
long maxLiveNodes, atLevel;
FILE *fp;

long startTime, endTime;

void packResults(double ,NODE *, int, double, int , int, int );

void luBound(ELEM *PbyW, double rw, double cp, int N, int k, double *lbb, double *ubb)
{
	double c;
	int i, j;

	*lbb = cp;
	c = rw;	
	for(i = k; i < N; i++) {
		if (c < PbyW[i].wt) {
			*ubb = *lbb + c * PbyW[i].profit/PbyW[i].wt;
			numOps += 3;
			for(j = i + 1; j < N; j++) 
				if(c >= PbyW[j].wt) {
					c = c - PbyW[j].wt;
					*lbb = *lbb + PbyW[j].profit;
					numOps += 2;
				}
			return;
		}
		c = c - PbyW[i].wt;
		*lbb = *lbb + PbyW[i].profit;
		numOps += 2;
	}
	*ubb = *lbb;
}	

void newNode(NODE *par, int tag, double cap, double prof, double ub)
{
	NODE *n;

	n = (NODE *) calloc(1, sizeof(NODE));
	//n->parent = par;
	n->tag = tag;
	n->cu = cap;
	n->profit = prof;
	n->ub = ub;
	if(endQLive == NULL) {
		qLive = n;
		endQLive = n;
	} else {
		endQLive->next = n;
		n->previous = endQLive;
		endQLive = endQLive->next;
		
	}
	++numLiveNodes;
	//printNode(endQLive);
}

void addQLevelMarker()
{

	NODE *n;

	n = (NODE *) calloc(1, sizeof(NODE));
	n->tag = -1;
	if(endQLive != NULL) {
		endQLive->next = n;
		n->previous = endQLive;
		endQLive = endQLive->next;
	} else {
		endQLive = n;
		qLive = n;
	     }
}

NODE *deleteQ(void)
{
	NODE * n;

	if (qLive == NULL) {
		puts("Q empty!. Can't delete");
		exit(1);
	}
	n = qLive;
	qLive = qLive->next;
	if(qLive != NULL)
		qLive->previous = NULL;
	else endQLive = NULL;
	n->next = NULL;
	if(n->tag != -1)
		--numLiveNodes;
		
	return n;
}
void finish(double L, NODE *ans, int N, double M)
{
	int j;
	double wt = 0;
/*
	printf("Value of optimal filling is %g\n", L);
	puts("Objects in Knapsack are");
	for(j = N-1; j >= 0; --j, ans = ans->parent)
		if(ans->tag == 1) {
			printf("%d\n", j);
			wt = wt + items[j].wt;
		}
	printf("Capacity used %g against %g\n", wt, M);
*/
}

void printNode(NODE *n)
{
	printf("Node has ub %g\n", n->ub);
	printf("Node has cu %g\n", n->cu);
	printf("Node has profit %g\n", n->profit);
	printf("Node has tag %d\n", n->tag);

}

void packResults(double L,NODE *ans, int N, double M, int nodeNumber, int res, int optVal)
{
	//int tsOVector = cnf_open("OVector", 0);
	char tpname[128];
	int tplength, status;
	double globalOptimal;

/* read the global optimal */
	tplength = sizeof (double);
	strcpy(tpname, "OPTVAL");
	status = cnf_tsread(optVal, tpname, &globalOptimal, 0); 
	if(status > 0) {
		if (L > globalOptimal) 
			/* update the global optimal */
			cnf_tsput(optVal, tpname, &globalOptimal, tplength);
	
		/* put the results in result tuple space*/
		tplength = sizeof(struct otuple);

		otuple.nodeNumber = nodeNumber;
		otuple.L = L;
		otuple.numOps = numOps;
		sprintf(tpname, "node%d\0",nodeNumber);
#ifdef DEBUG
		printf("Worker going to put %s in results with L = %g\n", tpname, L);
		fflush(stdout);
#endif
		cnf_tsput(res, tpname, &otuple, tplength);
		fflush(stdout); 
	}
	else {
		puts("Error in reading optVal in packresults in worker ");	
		fflush(stdout);
		}

}


printItuple(struct ituple it)
{

   if(it.numItems == 0)
      printf("termination tuple with nodeNumber %d\n", it.nodeNumber);	
  else {
   printf("Level is %d\n", it.level);
   printf("cu is %g\n", it.cu);
   printf("profit is %g\n", it.profit);
   printf("tag is %d\n", it.tag);
   printf("capacity is %g\n", it.capacity);
   printf("ubb is %g\n", it.ubb);
   printf("L is %g\n", it.L);
   printf("numItems is %d\n", it.numItems);
   printf("nodeNumber is %d\n", it.nodeNumber);
  }
}
  


main () 
{
	int i, M, N, nodeNumber, continueLoop = 1;
	int tsd, res, optVal,tsItems,tpnamsz, tplength, status;
	char tpname[128];
	double L, ubb, lbb, profit, cap, globalOptimal;
	NODE *node;	
	ELEM *PbyW;
	static int itemsCopied = 0;

	puts("Going to start Worker bfsKnap");
	fflush(stdout);
	tsItems = cnf_open("Items", 0);
	tsd = cnf_open("Nodes", 0);
	res = cnf_open("Results", 0);
	optVal = cnf_open("OptVal", 0);

	
    while(1) {	
	puts("WORKER TIME STARTED");
	fflush(stdout);
	startTime = time((long *)0);
	maxLiveNodes = 1;
	atLevel = 0;
	qLive = NULL;
	endQLive = NULL;
	numOps = 0;
	/* get tuple put by master */
	strcpy(tpname, "*\0");
	tpnamsz = strlen(tpname);
	tplength = sizeof(struct ituple);
	status = cnf_tsget(tsd, tpname, &ituple, 0);
	if (status > 0) {
	    //printItuple(ituple);
	    if(ituple.numItems == 0) {
#ifdef DEBUG
		puts("Worker found last tuple");
		fflush(stdout);
#endif
		status = cnf_tsput(tsd, tpname, &ituple,status);
		cnf_term();
	     }
	/* get the global optimal if not the first iteration of while loop*/	
	if(itemsCopied) {
		strcpy(tpname, "*");
		tpnamsz = strlen(tpname);
		tplength = sizeof(double);
		status = cnf_tsread(optVal, tpname, &globalOptimal, 0);
		if (status > 0)	{
			if (ituple.L < globalOptimal){
				tplength = sizeof(struct otuple);

				otuple.nodeNumber = ituple.nodeNumber;
				otuple.L = 0;
				otuple.numOps = numOps;
				sprintf(tpname, "node%d\0",ituple.nodeNumber);
#ifdef DEBUG
		printf("Worker going to put otuple without processing\n");
		fflush(stdout);
#endif
				cnf_tsput(res, tpname, &otuple, tplength);
		printf("worker fake tp(%s) returned\n",tpname);
		fflush(stdout); 
				continue; /* to the start of WHILE LOOP */
			}
		} else{
			printf("Error in reading global optimal\n");
			fflush(stdout);
		}
			
	}
	/* fill in L etc */
		i = ituple.level;
		L = ituple.L;
		ubb = ituple.ubb;
		M = ituple.capacity;
		N = ituple.numItems;
		nodeNumber = ituple.nodeNumber;	
	}
	else {
		printf("Error in getting node in worker ");
		fflush(stdout);
		exit(1);
	}	

	if(!itemsCopied) {
		PbyW = (ELEM *) malloc( N * sizeof(ELEM) );
		tplength = N * sizeof(ELEM);
#ifdef DEBUG
		puts("Going to get the items");
		fflush(stdout);
#endif
		strcpy(tpname,"Items"); 
		status = cnf_tsread(tsItems, tpname, PbyW, 0); /* Just copy */
		if (status <= 0) {
			printf("Error in getting items back in worker ");
			fflush(stdout);
			exit(2);
		}
		//else printf("Items copied\n");
		itemsCopied = 1;
	}
	
	//luBound(PbyW, M, 0, N, 0, &L, &ubb);		
	//printf("L is %g and ubb is %g\n", L, ubb);
	//newNode(NULL, 0, M, 0, ubb);	
	newNode(NULL, ituple.tag, ituple.cu, ituple.profit, ituple.ub);
	addQLevelMarker();
	while(i < N) {
		for(;continueLoop;) {
			node = deleteQ();
			//printNode(node);
			switch(node->tag) {
		   	   case -1:
				continueLoop = 0;
				break;
		   	   default:
	//printf("Node->ub is %g and L is %g\n", node->ub, L);
				if(node->ub >= L){
					cap = node->cu;
					profit = node->profit;
					if(cap >= PbyW[i].wt) {
						newNode(node, 1, (cap - PbyW[i].wt), (profit + PbyW[i].profit), node->ub);
						numOps += 2;
					}
					luBound(PbyW, cap, profit, N, (i+1), &lbb, &ubb);
					++numOps;
			//printf("Cap is %g, PbyW[%d].wt is %g, ubb is %g\n", cap,i ,PbyW[i].wt, ubb);
					if(ubb >= L) {
						newNode(node, 0, cap, profit, ubb);
						L = (L > lbb)? L: lbb;
					}
				}
				free(node);
				break;	      
			}
		}
		addQLevelMarker();
		continueLoop = 1;
		//printf("Live nodes at level(root at 0) %d are %ld\n", i, numLiveNodes);
		if(numLiveNodes == 0){
			printf("Going to discard this node %d\n", nodeNumber);
			qLive = NULL;
			endQLive = NULL;
			i = N; /* break from while loop */
		}
		if(numLiveNodes > maxLiveNodes){
			maxLiveNodes = numLiveNodes;
			atLevel = 1;
		}
			
	i++;
	}
        if(qLive == NULL)
		packResults(0, qLive, N, M, nodeNumber, res, optVal);
	else for(; qLive; qLive = qLive->next) {
		if(qLive->profit == L) {
			packResults(L, qLive, N, M, nodeNumber, res, optVal);
			break;
		}
	     }
	endTime = time((long *)0) - startTime;
	puts("WORKER TIME ENDED");
	fflush(stdout);
  
   /*
	for(; qLive; qLive = qLive->next) {
		if(qLive->profit == L) 
			packResults(L, qLive, N, M, nodeNumber, res, optVal);
	}
   */
   }
}	


