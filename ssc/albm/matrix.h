#define	N	600

void alrmHandler();

char tpname[20];

double ituple_B[N][N];
double *ituple_A;
/* [0] = grain size
   [1] = values per rows
   ...
*/
double *otuple;
/* [0] = grain size 
   [1] = values per rows
   ...
*/
