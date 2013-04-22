#include <stdio.h>
#include <stdlib.h>
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
//	struct node *parent;
} NODE;


void printNode(NODE *);

ELEM *items = NULL;
NODE *qLive = NULL, *endQLive = NULL;
double numOps = 0;
long numLiveNodes = 0; /*number of live nodes at a level, changes with level */
long maxLiveNodes, atLevel;
FILE *fp;

long startTime, endTime;

int numProcessors;

void printResults(int, int, int);



int compareElem(ELEM *e1, ELEM *e2)
{
	return (int) (-10000 * ((e1->pbyw) - (e2->pbyw))) ;
}



/* UCR */
/*
double getItems(int num)
{
	int i;
	double wt = 0;

	items = (ELEM *) calloc(num , sizeof(ELEM));	
	for(i = 0; i < num; i++) {
		items[i].wt = (rand() % 100) + 1;
		wt += items[i].wt;
	}	
	srand(SEED);	
	for(i = 0; i < num; i++) {
		items[i].profit = rand() % 100;
		items[i].pbyw = items[i].profit/items[i].wt;
		}
	puts("Going to qsort");
	qsort(items, num, sizeof(ELEM), compareElem);
	puts("After qsort");
	printf("Wt is %g\n", wt);
	fflush(stdout);
	return wt;
}	
*/



double getItems(int num)
{
	int i;
	double wt = 0;

	items = (ELEM *) calloc(num , sizeof(ELEM));	
	for(i = 0; i < num; i++) {
		items[i].wt = (rand() % 100) + 1;
		items[i].profit = items[i].wt;
		items[i].pbyw = 1.0;
		wt += items[i].wt;
	}	
	puts("Going to qsort");
	qsort(items, num, sizeof(ELEM), compareElem);
	puts("After qsort");
	printf("Wt is %g\n", wt);
	fflush(stdout);
	return wt;
}	




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

void bfsKnap(ELEM *PbyW, int M, int N) 
{
	int i, continueLoop = 1, knapCapacity;
	int tsd, res, tsItems, optVal, tpnamsz;
	double L, ubb, lbb, profit, cap;
	NODE *node;
	int CONS;

	CONS = atoi(getenv("NUMT"));	
//	CONS = 1;
	knapCapacity = M;
	puts("Going to start bfsKnap");
	fflush(stdout);
	maxLiveNodes = 1;
	atLevel = 0;
	
	tsd = cnf_open("Nodes", 0);
	tsItems = cnf_open("Items", 0);
	res = cnf_open("Results", 0);
	optVal = cnf_open("OptVal", 0);

	cnf_tsput(tsItems, "Items", PbyW, (N * sizeof(ELEM)));

	numProcessors = cnf_getP();
	puts("TIME STARTED");
	fflush(stdout);
	startTime = time((long *)0);
	i = 0;	
	luBound(PbyW, M, 0, N, 0, &L, &ubb);		
	//printf("L is %g and ubb is %g\n", L, ubb);
	newNode(NULL, 0, M, 0, ubb);	
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
			puts("Num Live Nodes becoms Zero. ERROR");
			printf("Value of optimal filling %g\n", L);
			break;
		}
		if(numLiveNodes >= (CONS * numProcessors)) {
			/* Going to parallelize */
			/* Distribute the live Nodes */
			/* exit from the while loop by setting i = N */

			int status, nodeNumber = 0;
			char tpname[128];
			int tplength = sizeof(struct ituple);
			NODE *liveNode;	
#ifdef DEBUG
			printf("Num Live nodes to distribute %ld\n", numLiveNodes);
#endif
			for(liveNode = qLive; liveNode->tag != -1; liveNode = liveNode->next, ++nodeNumber) { 
				ituple.level = i;
				ituple.cu = liveNode->cu;
				ituple.profit = liveNode->profit;
				ituple.tag = liveNode->tag;
				ituple.ub = liveNode->ub;
				ituple.capacity = M;
				ituple.ubb = ubb;
				ituple.L = L;
				ituple.numItems = N;
				ituple.nodeNumber = nodeNumber;
				sprintf(tpname, "node%d\0", nodeNumber);
#ifdef DEBUG
				printf("Master putting %s\n", tpname);
#endif
				status = cnf_tsput(tsd, tpname, &ituple, tplength);
			}
			/* now put the terminating tuple */
			ituple.numItems = 0;
			ituple.nodeNumber = nodeNumber;
			sprintf(tpname, "node%d\0", nodeNumber);
#ifdef DEBUG
			printf("Master putting the termination tuple %s\n", tpname);
#endif
			status = cnf_tsput(tsd, tpname, &ituple, tplength);
			
			strcpy(tpname, "OPTVAL\0");
#ifdef DEBUG
			printf("Master putting the optimal value tuple called %s with %g\n",tpname, L);	
#endif
			tplength = sizeof(double);
			status = cnf_tsput(optVal, tpname, &L, tplength);


			i = N; /* to exit from while loop */
		}			
	i++;
	}
	printResults(numLiveNodes, res, knapCapacity);
	cnf_close(tsd);
	cnf_close(tsItems);
	cnf_close(res);
	cnf_close(optVal);
	cnf_term();
}	


void printResults(int numLiveNodes, int res, int capacity)
{
	int received, status, num, CONS;
	double maxL = 0, workerOps=0;
  	char tpname[128]; 
	double elapsedTime;

#ifdef DEBUG
	printf("Master going to wait for %d otuples", numLiveNodes);
#endif
	for(received = 0; received < numLiveNodes; received++){
		/* pick the otuple with max L */
#ifdef DEBUG
		printf("In results collection");
		fflush(stdout); 
#endif
		strcpy(tpname,"*");
		status = cnf_tsget(res, tpname , &otuple, 0);
#ifdef DEBUG
		printf("result tuple name (%s) \n",tpname);
		printf("MaxL from nodeNumber %d is %g\n",otuple.nodeNumber, otuple.L);
		fflush(stdout); 
#endif
		if(maxL < otuple.L) 
			maxL = otuple.L;
		workerOps += otuple.numOps;
	}	

	workerOps += numOps;

	endTime = time((long *)0) - startTime;
	puts("TIME ENDED");
	fflush(stdout);
	printf("MaxL is %g\n", maxL);
	//fprintf(fp,"MAX L is %g\n", maxL);
	elapsedTime = endTime;
	printf("Time taken %g\n", elapsedTime);
	
	num = atoi(getenv("NUMEHCR"));
	CONS = atoi(getenv("NUMT"));	
        fprintf(fp, "#Elements = %d, Capacity %d, Elapsed Time(in seconds) %d, Processors %d, #Operations %g,\
	       MFLOPS %g CONS %d\n", num, capacity, elapsedTime, numProcessors, (numOps + workerOps), ((numOps + workerOps)/elapsedTime)/1000000.0, CONS);
	fflush(fp);
}
			
main()
{
	int num, i;
	double capacity;
	double elapsedTime;

/*
	printf("Number of elements ");
	scanf("%d", &num);
*/

	num = atoi(getenv("NUMEHCR"));
	//num = 2000;
	fp = fopen("bfs-hcr.parallel","a");
	if (fp == NULL) {
		printf("File can't be opened");
		exit(1);
	}
	capacity = floor(getItems(num) / 2.0);
	printf("Capacity is %g\n", capacity);
/*
	for(i = 0; i < num; i++)
		printf("Profit %g, Wt %g\n", items[i].profit, items[i].wt);
*/
	bfsKnap(items, capacity, num);
	elapsedTime = (endTime - startTime)/1000;
	printf("Time taken %g\n", elapsedTime);
	printf("Number of operations %g\n", numOps);
	printf("Max Lives Nodes were %ld at level(root at 0) %ld\n", maxLiveNodes, atLevel);
	printf("Total number of elements %d\n", num);
/*
	fprintf(fp, "#Elements = %d, Capacity %g, Elapsed Time(in seconds) %f, Processors %d, #Operations %g,\
	       MFLOPS %g\n", num, capacity, elapsedTime, numProcessors, numOps, (numOps/elapsedTime)/1000000.0);
	fflush(fp);
*/
	fclose(fp);
}	



