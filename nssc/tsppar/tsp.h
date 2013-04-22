/************************************************************
 * File: tsp.h                                              *
 *                                                          *
 * This is the header file for the parallel                 *
 * implementation of the TSP (Traveling Salesperson Problem)*
 *                                                          *
 *************************************************************/

#define MAX_NODES 20
#define TRUE 1
#define FALSE 0
#define ERROR -1
#define OK 1

/* Structure definition of search-tree list structure, the search
tree is organized as an ordered list */

struct tree_node
    {
    int num_nodes;
    long cur_tour_cost;
    char graph_node[MAX_NODES+1];
    struct tree_node *next;
    };


/* Tuple structures */

/* this tuple will hold the "partial tour" that will be expanded
by each worker */
struct node_tuple
     {
     long tour_cost;
     int tour_length;
     char tour[MAX_NODES+1];
     };

/* this tuple will store the initial information to load cost
matrix, etc */
struct start_tuple
     {
     int n;
     long seed;
     };

/* this tuple will hold the global mininum */
long t_global_minimum;

/* this tuple will hold the minimum tour */
char t_min_tour[MAX_NODES+1];

/* Global Variables */
struct tree_node *search_tree_list;
long cost [MAX_NODES+1] [MAX_NODES+1];
long local_minimum;
double node_count=0,eff_calcs=0;

/* Ordered List Function Prototypes */
int empty();
void init_q();
void enqueue(/* int num_nodes, long cost, char tour[] */);
int dequeue(/* int *num_nodes, long *cost, char tour[] */);

/* TSP Function Prototypes */
int make_child(/* char child_node, int par_num_nodes, long par_cost,
               char par_tour[], int *child_num_nodes, long *child_cost,
               char child_tour[] */);
void retrieve_process_info(/* int *n, long *seed, int *c */);
void load_cost_matrix(/* int n, long seed */);
