#include "parallel.h"

/*
 *- Partitioning Horizontally
 */

 
/* <reference> */
 
/* Constant, N, and the grain size, G, are defined in the header file */
#include “init.h”
 
/* </reference> */
 

 
/* <parallel appname="init1"> */
double * _tp_v_123;
double * _tp_A_123;
int _x0_123;
int _x1_123;
 
int main()
{
     
/* <reference> */
 
    double A[N][N];
    double v;
    int i, j;
     
/* </reference> */
 

    v = 99.9;

     
/* <master id="123"> */
_distributor = _open_space("distributor", 0, "123");
_constructor = _open_space("constructor", 0, "123");
 
     
_cleanup_space(_distributor, "123");
_cleanup_space(_constructor, "123");
    /* <send var="v" type="double" opt="ONCE"/> */
    sprintf(_tp_name, "double:v#%s", "123");
    _tp_size = sizeof(double);
    _tp_v_123 = &v;
    _status  = _send_data(_distributor, _tp_name, (char *)_tp_v_123, _tp_size);
    if (_status < 0) exit(-1);

 
     
    /* <token action="SET" idxset="(i)"/> */
    sprintf(_tp_name, "token#%s", "123");
    sprintf(_tp_token, "=(i:%d~%d,%d:#%d)", 0, N, 1, G);
    _tp_size = sizeof(_tp_token);
    _tokens  = _set_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tokens < 0) exit(-1);


    /* <send var="A" type="double[N(i)][N]"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s", (N), (N), "123");
    _tp_size = ((N) * (N)) * sizeof(double);
    _tp_A_123 = (double *)malloc(_tp_size);
    for (_x0_123 = 0; _x0_123 < (N); _x0_123 +=1) {
        for (_x1_123 = 0; _x1_123 < (N); _x1_123 +=1) {

            _tp_A_123[_x0_123 * (N) + _x1_123] = A[_x0_123][_x1_123];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_A_123, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_123);

 

      

     
    /* <read var="A" type="double[N(i)][N]"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s", (N), (N), "123");
    _tp_size = ((N) * (N)) * sizeof(double);
    _tp_A_123 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_A_123, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123 = 0; _x0_123 < (N); _x0_123 +=1) {
        for (_x1_123 = 0; _x1_123 < (N); _x1_123 +=1) {

            A[_x0_123][_x1_123] = _tp_A_123[_x0_123 * (N) + _x1_123];
        }
    }

    free(_tp_A_123);

 
     
_close_space(_constructor, "123", 1);
_close_space(_distributor, "123", 1);
/* </master> */
 
}
 
/* </parallel> */
 
