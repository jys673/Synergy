/*-----------------------------------------------------------------------
 * 
 *                        Traveling Salesperson Program
 *                 Branch and Bound, Parallel Implementation
 *
 *                    --------->   Master  <-------------
 *
 * ---------------------------------------------------------------------
 * Coded by: Stephen Maldony, Temple University
 *           August/September 1994
 * email: smaldony@astro.cis.temple.edu
 * --------------------------------------------------------------------
 *
 * This parallel implementation of the TSP is code using Syngergy v2,
 * using tuple space objects for data communications
 *
 * The following tuples are used in the implementation:
 *     
 * tuple name            description                   datatype
 * ----------            -----------                   --------
 * g_min                 global minimum of tours       long integer
 *                       found, so far
 * 
 * eff_calcs             effective calculations of     double precision float
 *                       best tour found, so far
 *
 * best_tour             character array containing    char array[MAX_NODES+1]
 *                       the "best tour" found, so far
 *
 * ffff*                 tuple indicating node * has   char
 *                       been processed
 *
 * node*                 contains info about the       struct node_tuple
 *                       partial tour number *
 * 
 * start                 contains start up info        struct start_tuple
 *                       ie, n, seed value
 *
 * note: for descriptions of struct data types see the TSP header file:
 *       tsp.h
 *-------------------------------------------------------------------*/
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include "tsp.h"

#define EXIT_FAILURE 0


/* Parallel TSP program using Branch and Bound */
void main()
     {
     long temp_cost=0,cur_cost,seed=0;
     double temp_calcs=0;
     int i=0,j=0,branch_possible=0,n=0,temp_num_nodes=0,tour_found=0,c=0;
     int cur_num_nodes=0,q_stat=0,status=0,tuples_processed=0;
     int tuples_sent=0,tplength=0;
     char tpname[128];
     char temp_tour[MAX_NODES+1];
     char cur_tour[MAX_NODES+1];
     char best_tour[MAX_NODES+1];

     char child=0, finished_tuple=0;
     double t0=0, t1=0;
     long start_time=0, end_time=0; /* for timing purposes */
     int ts1=0, ts2=0, P=0;
     struct node_tuple o_node_tuple;
     struct start_tuple o_start_tuple;
     FILE *outfile=NULL;
     
     /* open output statistics file */
     outfile = fopen("tsp_out.txt","a+");
     
     /* set up timing vars */
     start_time = time(NULL);

     /* load input information and create cost matrix */
     retrieve_process_info(&n,&seed,&c);
     load_cost_matrix(n,seed);
     
     /* set up initial synergy stuff and tuples */
     ts1 = cnf_open("general",0);
     ts2 = cnf_open("nodes",0);
     P = cnf_getP();

     /* set up start tuple */
     o_start_tuple.n = n;
     o_start_tuple.seed = seed;
     tplength = sizeof(struct start_tuple);
     sprintf(tpname,"start\0");
     status = cnf_tsput(ts1,tpname,&o_start_tuple,tplength);

     /* setup global minimum tuple */
     t_global_minimum = LONG_MAX;
     tplength = sizeof(long);
     sprintf(tpname,"g_min\0");
     status = cnf_tsput(ts1,tpname,&t_global_minimum,tplength);

     /* set up best path tuple */
     for (i=1; i<= MAX_NODES; i++)
          best_tour[i] = 0;
     tplength = sizeof(char) * (MAX_NODES+1);
     sprintf(tpname,"best_tour\0");
     status = cnf_tsput(ts1,tpname,best_tour,tplength);

     /* put zero effective calculations tuple to the tuplespace */
     eff_calcs = 0;
     tplength = sizeof(double);
     sprintf(tpname,"eff_calcs\0");
     status = cnf_tsput(ts1,tpname,&eff_calcs,tplength);

     /* initialize queue, first entry */
     init_q();
     temp_tour[1] = 1;
     enqueue((int) 1, (long) 0, temp_tour);

     tour_found = FALSE;
     node_count = 0;

     /* create P*c tours, P is the number of processors, c is a load balancing
        constant */
     while ((node_count <= (P*c)) && (!empty()))
       {
       q_stat = dequeue(&cur_num_nodes,&cur_cost,cur_tour);
       if (q_stat == ERROR)
          cnf_term();
       
          if (cur_cost < t_global_minimum)
             {
             for (child=1; (int)child<=n; child++)
               {
               branch_possible = make_child(child,cur_num_nodes,cur_cost,
                                 cur_tour, &temp_num_nodes,&temp_cost,temp_tour);
          
               if (branch_possible)
                  {
                  /* complete tour is found */
                  if (temp_num_nodes == n)
                     {
                     /* this will handle the case when the problem is handled
                 so quickly that parallel is not needed */   
                     if (temp_cost < t_global_minimum)
                        {
                        t_global_minimum = temp_cost;
                        for (j=1;j<=n;j++)
                            best_tour[j] = temp_tour[j];
                        tour_found = TRUE;
                        }
                     }
                  else
              {
              eff_calcs++;
                     enqueue(temp_num_nodes,temp_cost,temp_tour);
              }
                  } /* end-if branch possible */
               } /* end-for */
             } /* end-if */
        } /* end-while q not empty */

     i=0;


     if (!tour_found)
        {
        while (!empty())
          {
          q_stat = dequeue(&(o_node_tuple.tour_length),
                  &(o_node_tuple.tour_cost),o_node_tuple.tour);
          if (q_stat == ERROR)
             exit(EXIT_FAILURE);

          /* assemple tuple and put to tuplespace */ 
          i++;
          sprintf(tpname,"node%d\0",i);
          tplength = sizeof(struct node_tuple);
          status = cnf_tsput(ts2,tpname,&o_node_tuple,tplength);
          }

        tuples_sent = i;

        /* wait for termination signals from all workers */
        tuples_processed=0;
        while (tuples_processed < tuples_sent)
          {
          sprintf(tpname,"f*\0");
          status = cnf_tsget(ts2,tpname,&finished_tuple,0);
          tuples_processed++;    
          } 

        /* all workers are finished, so output minimum cost, tour, etc */
        sprintf(tpname,"g_min\0");
        status = cnf_tsget(ts1,tpname,&t_global_minimum,0);

        sprintf(tpname,"best_tour\0");
        status = cnf_tsget(ts1,tpname,best_tour,0);

        sprintf(tpname,"eff_calcs\0");
        status = cnf_tsget(ts1,tpname,&temp_calcs,0);

        }
     else
     {
        /* master solved TSP, notify workers to terminate */
        fprintf(outfile,"TSP was solved by client, no workers processed.\n");
        o_node_tuple.tour_cost = -1;
        o_node_tuple.tour_length = 0;
        sprintf(tpname,"node0\0");
        tplength = sizeof(struct node_tuple);
        status = cnf_tsput(ts2,tpname,&o_node_tuple,tplength);
     }

     /* output statistics */
     end_time = time(NULL);

     fprintf(outfile,"Best tour cost: %d. \npath:",t_global_minimum);
     for (j=1;j<=n;j++)
         fprintf(outfile,"%d ",(int) best_tour[j]);

     fprintf(outfile,"\nNumber of effective calculation steps: master: %.0f, worker: %0.f, total: %0.f\n",
          eff_calcs,temp_calcs,(eff_calcs+temp_calcs));
     fprintf(outfile,"Elapsed time: Elapsed Time (%ld) p(%d) n(%d) c(%d). \n\n",
	  (end_time-start_time),P,n,c);
     fclose(outfile);
     cnf_term();
     } /* end-main */

/*-----------------------------------------------------------------------------
 *                           TSP Helper Functions
 *-----------------------------------------------------------------------------*/

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

/* enqueue a new search tree entry */
void enqueue(num_nodes, cost, tour)
int num_nodes; 
long cost; 
char tour[];
     {
     struct tree_node *temp, *prev, *cur;
     int i,done;

     temp = (struct tree_node *) malloc(sizeof(struct tree_node));
     if (temp == NULL)
        {
        printf("\n\nRan out of memory!!!\n");
        exit(EXIT_FAILURE);
        }

     node_count++;
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



/* this will remove a search tree node from the front of the list */
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
       node_count--;
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
int make_child(child, par_num_nodes, par_cur_cost, par_tour, child_num_nodes,
	child_cur_cost, child_tour)
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

     if ((!found) && (tour_cost < t_global_minimum))
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


/* function to load cost matrix */
void retrieve_process_info(n, seed, c)
int *n; 
long *seed; 
int *c;
     {
     FILE *tsp_param=NULL;
     int f_stat,i,j,temp_n,temp_c;
     long temp_seed;

     tsp_param = fopen("tsp_param.txt","r");
     if (tsp_param == NULL)
       {
       printf("cannot open file.");
          exit(EXIT_FAILURE);
       }

     f_stat = fscanf(tsp_param,"%d",&temp_n);
     f_stat = fscanf(tsp_param,"%ld",&temp_seed);
     f_stat = fscanf(tsp_param,"%d",&temp_c);

     *seed = temp_seed;
     *c = temp_c;
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
             cost[i][j] = rand()/1000000 ;
             }
         }
   }
