#include <stdio.h>
#include <math.h>

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
	struct node *parent;
} NODE;


void printNode(NODE *);

ELEM *items = NULL;
NODE *qLive = NULL, *endQLive = NULL;
double numOps = 0;
long numLiveNodes = 0; /*number of live nodes at a level, changes with level */
long maxLiveNodes, atLevel;
FILE *fp;

long startTime, endTime;

int compareElem(ELEM *e1, ELEM *e2)
{
	return (int) (-10000 * ((e1->pbyw) - (e2->pbyw))) ;
}


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
	n->parent = par;
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

	printf("Value of optimal filling is %g\n", L);
	puts("Objects in Knapsack are");
	for(j = N-1; j >= 0; --j, ans = ans->parent)
		if(ans->tag == 1) {
			printf("%d\n", j);
			wt = wt + items[j].wt;
		}
	printf("Capacity used %g against %g\n", wt, M);
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
	int i, continueLoop = 1;
	double L, ubb, lbb, profit, cap;
	NODE *node;	
	
	puts("Going to start bfsKnap");
	maxLiveNodes = 1;
	atLevel = 0;
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
		if(numLiveNodes > maxLiveNodes){
			maxLiveNodes = numLiveNodes;
			atLevel = i;
		}

	i++;
	}
	endTime = time((long *)0) - startTime;
	for(; qLive; qLive = qLive->next)  {
		if(qLive->profit == L) 
			finish(L, qLive, N, M);
	}
}	
			
main()
{
	int num, i;
	double capacity;
	double elapsedTime;

	printf("Number of elements ");
	scanf("%d", &num);
	fp = fopen("bfs.seq","a");
	capacity = floor(getItems(num) / 2.0);
	printf("Capacity is %g\n", capacity);
/*
	for(i = 0; i < num; i++)
		printf("Profit %g, Wt %g\n", items[i].profit, items[i].wt);
*/
	bfsKnap(items, capacity, num);
	elapsedTime = endTime;
	printf("Time taken %g\n", elapsedTime);
	printf("Number of operations %g\n", numOps);
	printf("Max Lives Nodes were %ld at level(root at 0) %ld\n", maxLiveNodes, atLevel);
	printf("Total number of elements %d\n", num);
	fprintf(fp, "#Elements = %d, Capacity %g, Elapsed Time(in seconds) %d, #Operations %g,\
		MFLOPS %g\n", num, capacity, elapsedTime, numOps, (numOps/elapsedTime)/1000000.0);
	fclose(fp);
}	
