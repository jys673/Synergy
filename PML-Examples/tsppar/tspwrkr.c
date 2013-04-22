/*-----------------------------------------------------------------------
 *                        Traveling Salesperson Program
 *                       Branch and Bound Implementation
 *
 *                    --------->   Worker  <-------------
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
#include "tsp.h"

/* Parallel TSP program using Branch and Bound */

void main()
     {
     long temp_cost=0,cur_cost=0,seed=0,temp_calc=0;
     int j=0,branch_possible=0,n=0,temp_num_nodes=0,tour_found=0,c=0,status=0;
     int cur_num_nodes=0,q_stat=0;
     char tpname[128];
     char hold_tpname[128];
     char temp_tour[MAX_NODES+1];
     char cur_tour[MAX_NODES+1];
     char best_tour[MAX_NODES+1];

     char child=0,processed_tuple=0;
     int ts1=0, ts2=0;
     struct node_tuple i_node_tuple;
     struct start_tuple i_start_tuple;     
   
     /* set up initial synergy stuff and tuples */
     ts1 = cnf_open("general",0);
     ts2 = cnf_open("nodes",0);

     /* perform retrieval of initial information */
     sprintf(tpname,"start\0");
     status = cnf_tsread(ts1,tpname,&i_start_tuple,0);
     n = i_start_tuple.n;
     seed = i_start_tuple.seed;

     load_cost_matrix(i_start_tuple.n,i_start_tuple.seed);

     sprintf(tpname,"g_min\0");
     status = cnf_tsread(ts1,tpname,&local_minimum,0);
    
     /* initialize queue, retrieve partial tour from tuple space
	(non blocking retrieval) */
     init_q();
     sprintf(tpname,"node*\0");
     status = cnf_tsget(ts2,tpname,&i_node_tuple,0);
     if (status == 0)
	cnf_term();

     strcpy(hold_tpname,tpname);

     /* check if the master solved the TSP w/o the need of the workers */
     if (i_node_tuple.tour_cost == -1)
        cnf_term();

     /* put the partial tour onto the tuple space */
     enqueue(i_node_tuple.tour_length, i_node_tuple.tour_cost, i_node_tuple.tour);

     /* keep processing partial tours until there are no more */
     while (TRUE)
       {   
       tour_found = FALSE;
       eff_calcs = 0;
       while (!empty())
         {
         q_stat = dequeue(&cur_num_nodes,&cur_cost,cur_tour);

         if (q_stat == ERROR)
            cnf_term();

         if (cur_cost < local_minimum)
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
                       if (temp_cost < local_minimum)
                          {
                          local_minimum = temp_cost;
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

       /* put processed tuple into tuple space, reset node name to ffff# */
       hold_tpname[0] = 'f';
       hold_tpname[1] = 'f';
       hold_tpname[2] = 'f';
       hold_tpname[3] = 'f';
     
       status = cnf_tsput(ts2,hold_tpname,&processed_tuple,sizeof(char));    

       /* update global minimum */   
       if (tour_found)
          {
          sprintf(tpname,"g_min\0");
          status = cnf_tsget(ts1,tpname,&t_global_minimum,0);
          /* check if new tour is the best found yet, if so update stats */
          if (local_minimum < t_global_minimum)
             {
             sprintf(tpname,"best_tour\0");
             status = cnf_tsget(ts1,tpname,temp_tour,0);  

             sprintf(tpname,"eff_calcs\0");
             status = cnf_tsget(ts1,tpname,&temp_calc,0);  

             sprintf(tpname,"best_tour\0");
             status = cnf_tsput(ts1,tpname,best_tour,(MAX_NODES+1)*sizeof(char));  

             sprintf(tpname,"eff_calcs\0");
             status = cnf_tsput(ts1,tpname,&eff_calcs,sizeof(double));  


             sprintf(tpname,"g_min\0");
             status = cnf_tsput(ts1,tpname,&local_minimum,sizeof(long));  
             }
          else
             {
             sprintf(tpname,"g_min\0");
             status = cnf_tsput(ts1,tpname,&t_global_minimum,sizeof(long));  
             }
          }              

       /* attempt to retrieve another partial tour from the tuple space,
          performs a non-blocking read, if tour exists continue, otherwise
          terminate */
       sprintf(tpname,"node*\0");
       status = cnf_tsget(ts2,tpname,&i_node_tuple,-1);
       sprintf(hold_tpname,tpname);

       if (status == 0) /* no more nodes exist */
          cnf_term();
       else
          {
          /* another node to process, put it into queue and continue */
          enqueue(i_node_tuple.tour_length, i_node_tuple.tour_cost,
            i_node_tuple.tour);

          /* update local minimum with the current global minimum, already
          retrieved */
          local_minimum = t_global_minimum;
          }

       } /* end-while TRUE */
    } /* end-main */

/*-------------------------------------------------------------------------------
 *                           TSP Helper Functions
 *  These functions are essentially the same as the functions in the Master
 *  program
 *--------------------------------------------------------------------------------*/
 
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
        cnf_term();

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

     if ((!found) && (tour_cost < local_minimum))
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
               cost[i][j] = rand();
               }
          }
     }

