/*

  This is the header file for test the optimal chunk size,
  written by Faisal G. Hassan
*/


#define	Number_chunk_test	30

char tpname[20];

double ituple_B[Number_chunk_test][Number_chunk_test];
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
