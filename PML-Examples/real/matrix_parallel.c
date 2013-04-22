#include "parallel.h"

 
/* <reference> */
 
#include "matrix.h"
 
/* </reference> */
 

 
/* <parallel appname="matrix"> */
double * _tp_B_123456;
double * _tp_A_123456;
double * _tp_C_123456;
int _x0_123456;
int _x1_123456;
 

main(int argc, char **argv[])
{
 
/* <reference id="123456"> */
 
    int i, j, k;
 
/* </reference> */
 

    for (i = 0; i < N ; i++)
    {
        for (j = 0; j < N; j++)
        {
            A[i][j] = (double) i * j ;
            B[i][j] = (double) i * j ;
            C[i][j] = 0;
        }
    }

     
/* <master id="123456"> */
_distributor = _open_space("distributor", 0, "123456");
_constructor = _open_space("constructor", 0, "123456");
 
         
_cleanup_space(_distributor, "123456");
_cleanup_space(_constructor, "123456");
    /* <send var="B" type="double[N   ][N   ]" opt="ONCE"/> */
    sprintf(_tp_name, "double(%d)(%d):B#%s", (N), (N), "123456");
    _tp_size = ((N) * (N)) * sizeof(double);
    _tp_B_123456 = (double *)malloc(_tp_size);
    for (_x0_123456 = 0; _x0_123456 < (N); _x0_123456 +=1) {
        for (_x1_123456 = 0; _x1_123456 < (N); _x1_123456 +=1) {

            _tp_B_123456[_x0_123456 * (N) + _x1_123456] = B[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_B_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_B_123456);

 
         
    /* <token action="SET" idxset="(i)"/> */
    sprintf(_tp_name, "token#%s", "123456");
    sprintf(_tp_token, "=(i:%d~%d,%d:#%d)", 0, N, 1, G);
    _tp_size = sizeof(_tp_token);
    _tokens  = _set_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tokens < 0) exit(-1);


    /* <send var="A" type="double[N(i)][N   ]"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s", (N), (N), "123456");
    _tp_size = ((N) * (N)) * sizeof(double);
    _tp_A_123456 = (double *)malloc(_tp_size);
    for (_x0_123456 = 0; _x0_123456 < (N); _x0_123456 +=1) {
        for (_x1_123456 = 0; _x1_123456 < (N); _x1_123456 +=1) {

            _tp_A_123456[_x0_123456 * (N) + _x1_123456] = A[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_A_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_123456);



          

         
    /* <read var="C" type="double[N(i)][N   ]"/> */
    sprintf(_tp_name, "double(%d)(%d):C#%s", (N), (N), "123456");
    _tp_size = ((N) * (N)) * sizeof(double);
    _tp_C_123456 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_C_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = 0; _x0_123456 < (N); _x0_123456 +=1) {
        for (_x1_123456 = 0; _x1_123456 < (N); _x1_123456 +=1) {

            C[_x0_123456][_x1_123456] = _tp_C_123456[_x0_123456 * (N) + _x1_123456];
        }
    }

    free(_tp_C_123456);

 
     
_close_space(_constructor, "123456", 1);
_close_space(_distributor, "123456", 1);
/* </master> */
 

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            printf("%8.1f ", C[i][j]);
        }
        printf("\n");
    }

    exit(0);
}

 
/* </parallel> */
 
