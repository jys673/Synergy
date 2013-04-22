#include "parallel.h"

/* <reference> */
 
/* Constant, N, and the grain size, G, are defined in the header file */
#include “init.h”
 
/* </reference> */

/* <reference> */
 
    double A[N][N];
    double v;
    int i, j;
     
/* </reference> */

/* <parallel appname="init3"> */
double * _tp_v_123;
double * _tp_A_123;
int _x0_123;
int _y0_123;
int _x1_123;
int _y1_123;
int _i_start = 0;
int _i_stop = 0;
int _i_step = 0;
int _j_start = 0;
int _j_stop = 0;
int _j_step = 0;

main(int argc, char **argv[])
{

/* <worker id="123"> */
_distributor = _open_space("distributor", 0, "123");
_constructor = _open_space("constructor", 0, "123");
 
     
    /* <read var="v" type="double" opt="ONCE"/> */
    sprintf(_tp_name, "double:v#%s", "123");
    _tp_size = sizeof(double);
    _tp_v_123 = &v;
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_v_123, _tp_size);
    if (_tp_size < 0) exit(-1);

 
     
while (1)
{

    /* <token action="GET" idxset="(i)(j)"/> */
    sprintf(_tp_name, "token#%s", "123");
    _tp_size = 0;
    _tp_size = _get_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tp_size < 0) exit(-1);
    if (_tp_token[0] == '!') break;
    sscanf(_tp_token, "%d@(i:%d~%d,%d)(j:%d~%d,%d)", &_tokens, &_i_start, &_i_stop, &_i_step, &_j_start, &_j_stop, &_j_step);


    /* <read var="A" type="double[N(i)][N(j)]"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123", _i_start, _i_stop, 1, _j_start, _j_stop, 1, sizeof(double));
    _tp_size = (((_i_stop - _i_start - 1) / 1 + 1) * ((_j_stop - _j_start - 1) / 1 + 1)) * sizeof(double);
    _tp_A_123 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_A_123, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123 = _i_start, _y0_123 =0; _x0_123 < _i_stop; _x0_123 +=1, _y0_123 ++) {
        for (_x1_123 = _j_start, _y1_123 =0; _x1_123 < _j_stop; _x1_123 +=1, _y1_123 ++) {

            A[_x0_123][_x1_123] = _tp_A_123[_y0_123 * ((_j_stop - _j_start - 1) / 1 + 1) + _y1_123];
        }
    }

    free(_tp_A_123);

 

     
/* <target index ="i" order ="1" limits="(0,N,1)" chunk ="G"> */
for (i = _i_start; i < _i_stop; i +=_i_step) 
/* </target> */
 
    {
         
/* <target index ="j" order ="2" limits="(0,N,1)" chunk ="G"> */
for (j = _j_start; j < _j_stop; j +=_j_step) 
/* </target> */
 
        {
            A[i, j] = v;
        }
    }

     
    /* <send var="A" type="double[N(i)][N(j)]"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123", _i_start, _i_stop, 1, _j_start, _j_stop, 1, sizeof(double));
    _tp_size = (((_i_stop - _i_start - 1) / 1 + 1) * ((_j_stop - _j_start - 1) / 1 + 1)) * sizeof(double);
    _tp_A_123 = (double *)malloc(_tp_size);
    for (_x0_123 = _i_start, _y0_123 =0; _x0_123 < _i_stop; _x0_123 +=1, _y0_123 ++) {
        for (_x1_123 = _j_start, _y1_123 =0; _x1_123 < _j_stop; _x1_123 +=1, _y1_123 ++) {

            _tp_A_123[_y0_123 * ((_j_stop - _j_start - 1) / 1 + 1) + _y1_123] = A[_x0_123][_x1_123];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_A_123, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_123);

 
     
}

_close_space(_constructor, "123", 0);
_close_space(_distributor, "123", 0);
/* </worker> */

exit(0);
}

/* </parallel> */
