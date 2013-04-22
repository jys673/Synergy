/*-----------------------------------------------------------------------
 *                        Traveling Salesperson Program
 *                       Branch and Bound, Sequential Implementation
 *-------------------------------------------------------------------*/
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define MAX_NODES 20
#define EXIT_FAILURE 0

#define TRUE 1
#define FALSE 0
#define ERROR -1
#define OK 1

/* Structure definitions for the list */

struct tree_node
   {
   int num_nodes;
   long cur_tour_cost;
   char graph_node[MAX_NODES+1];
   struct tree_node *next;
   };

/* Global Variables */
struct tree_node *search_tree_list;
long cost [MAX_NODES+1] [MAX_NODES+1];
long global_minimum,node_count=0;
double eff_calcs=0;
FILE *outfile=NULL;


/* Function Prototypes */
int empty();
void init_q();
void retrieve_process_info(/* int *n, long *seed */);
void load_cost_matrix(/* int n,long seed */);
void enqueue(/* int num_nodes, long cost, char tour[] */);
int dequeue(/*int *num_nodes, long *cost, char tour[]*/);
int make_child(/* char child_node, int par_num_nodes, long par_cost,
    char par_tour[], int *child_num_nodes, long *child_cost,
    char child_tour[]*/);

/* TSP program using Branch and Bound */

void main()
   {
   long temp_cost,cur_cost,seed;
   int j,branch_possible,n,temp_num_nodes;
   int cur_num_nodes,q_stat;
   char temp_tour[MAX_NODES+1];
   char cur_tour[MAX_NODES+1];
   char best_tour[MAX_NODES+1];
   char child;
   time_t start_time,end_time;

   start_time = time(NULL);

   outfile = fopen("tsp_seq.txt","a+");

   /* global minimum starts out as infinity (very high number) */
   global_minimum = LONG_MAX;

   /* load input information and create cost matrix */
   retrieve_process_info(&n,&seed);
   load_cost_matrix(n,seed);
   fprintf(outfile,"Sequential TSP starting, n: %d, seed %ld\n",n,seed);
   fflush(outfile);

   /* initialize queue, first entry */
   init_q();
   temp_tour[1] = 1;
   enqueue((int) 1, (long) 0, temp_tour);

   while (!empty())
     {
     q_stat = dequeue(&cur_num_nodes,&cur_cost,cur_tour);
     if (q_stat == ERROR)
        exit(EXIT_FAILURE);

     if (cur_cost < global_minimum)
        {
        for (child=1; (int)child<=n; child++)
          {
          branch_possible = make_child(child,cur_num_nodes,
                            cur_cost,cur_tour, &temp_num_nodes,
                            &temp_cost,temp_tour);
          if (branch_possible)
             {
             /* complete tour is found */
             if (temp_num_nodes == n)
                {
                if (temp_cost < global_minimum)
                   {
                   global_minimum = temp_cost;
                   for (j=1;j<=n;j++)
                       best_tour[j] = temp_tour[j];
                   }
                }
             else
                {
                enqueue(temp_num_nodes,temp_cost,temp_tour);
                eff_calcs++;
                }
             }
          } /* end-for */
        } /* end-if */
     } /* end-while */

   end_time = time(NULL);
   fprintf(outfile,"Best tour cost: %d. \npath: ",global_minimum);
   fflush(outfile);
   for (j=1;j<=n;j++)
      fprintf(outfile,"%d ",(int) best_tour[j]);
   fflush(outfile);

   fprintf(outfile,"\nNumber of operations: %0.f\n",eff_calcs);
   fflush(outfile);
   fprintf(outfile,"Run time: %d seconds\n",(end_time-start_time));
   fflush(outfile);
   fclose(outfile);
   } /* end-main */

/* initialize the search tree list */
void init_q()
   {
   search_tree_list = NULL;
   }

/* check if list is empty */
int empty()
    {
    if (search_tree_list == NULL)
       return(TRUE);
    else
    return(FALSE);
    }

/* enqueue a new tree entry */
void enqueue( num_nodes, cost, tour)
int num_nodes; 
long cost; 
char tour[];
   {
   struct tree_node *temp, *prev, *cur;
   int i,done;

   temp = (struct tree_node *) malloc(sizeof(struct tree_node));
   if (temp == NULL)
      {
      fprintf(outfile,"Problem ran out of memory");
      fflush(outfile);
      exit(EXIT_FAILURE);
      }

   temp->num_nodes = num_nodes;
 
   temp->cur_tour_cost = cost;
   temp->next = NULL;
   for (i=1; i<=num_nodes; i++)
       temp->graph_node[i] = tour[i];

   if (empty())
      search_tree_list = temp;
   else
      {
      if (search_tree_list->cur_tour_cost > cost)
      /* insert in front of list */
         {
         temp->next = search_tree_list;
         search_tree_list = temp;
         }
      else
         {
         prev = search_tree_list;
         cur = search_tree_list;
         done = FALSE;
         while (!done)
            {
            prev = cur;
            cur = cur->next;
            if ((cur == NULL) || (cur->cur_tour_cost > cost))
               done = TRUE;
            }
         prev->next = temp;
         temp->next = cur;
         }
      }
   }

int dequeue(num_nodes, cost, tour)
int *num_nodes; 
long *cost; 
char tour[];
    {
    int i,rc=ERROR;
    struct tree_node *temp;

    if (empty())
       rc = ERROR;
    else
       {
       temp = search_tree_list;
       *num_nodes = temp->num_nodes;
       *cost = temp->cur_tour_cost;
       rc = OK;
       for (i=1; i<= temp->num_nodes; i++)
           tour[i] = temp->graph_node[i];

       temp = search_tree_list;
       search_tree_list = search_tree_list->next;
       free(temp);

       }
    return (rc);
    }

/* make child routine, this is the "branching" function of the branch and
   bound algorithm */
int make_child(child, par_num_nodes, par_cur_cost, par_tour,
		child_num_nodes, child_cur_cost, child_tour)
char child; 
int par_num_nodes; 
long par_cur_cost;
char par_tour[];
int *child_num_nodes; 
long *child_cur_cost;
char child_tour[];
    {
    int j=1,found=FALSE,rc=FALSE;
    long tour_cost;

    while ((j<=par_num_nodes) && (!found))
      {
      if (child == par_tour[j])
         found = TRUE;
      j++;
      }

    tour_cost = par_cur_cost +
                cost[(int)par_tour[par_num_nodes]] [(int) child];

    if ((!found) && (tour_cost < global_minimum))
       {
       *child_num_nodes = par_num_nodes+1;
       *child_cur_cost = tour_cost;
       for (j=1; j <= par_num_nodes; j++)
           {
           child_tour[j] = par_tour[j];
           }
       child_tour[*child_num_nodes] = child;
       rc = TRUE;
       }
    return(rc);
    }

/* function to retieve processing info */
void retrieve_process_info(n, seed)
int *n; 
long *seed;
     {
     FILE *tsp_param=NULL;
     int f_stat,i,j,temp_n;
     long temp_seed;

     tsp_param = fopen("tsp_param.txt","r");
     if (tsp_param == NULL)
       exit(EXIT_FAILURE);

     f_stat = fscanf(tsp_param,"%d",&temp_n);
     f_stat = fscanf(tsp_param,"%ld",&temp_seed);
    
     *seed = temp_seed;
     *n = temp_n;
     fclose(tsp_param);
     }

/* using seed value retrieved set up the matrix of costs */
void load_cost_matrix(n, seed)
int n; 
long seed;
   {
   int i,j;

   srand(seed);
     for (i=1; i<=n; i++)
         {
         for (j=1; j<=n; j++)
             {
             cost[i][j] = rand() / 1000000;
	     printf("input [%d,%d] cost(%ld)\n",i,j,cost[i][j]);
             }
         }
   }
