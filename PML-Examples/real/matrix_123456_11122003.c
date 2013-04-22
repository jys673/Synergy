#include "parallel.h"

/* <reference> */
 
#include "matrix.h"
 
/* </reference> */

/* <reference id="123456"> */
 
    int i, j, k;
 
/* </reference> */

/* <parallel appname="matrix"> */
double * _tp_B_123456;
double * _tp_A_123456;
double * _tp_C_123456;
int _x0_123456;
int _x1_123456;
int _y0_123456;
int _y1_123456;
int _i_start = 0;
int _i_stop = 0;
int _i_step = 0;

main(int argc, char **argv[])
{

/* <worker id="123456"> */
_distributor = _open_space("distributor", 0, "123456");
_constructor = _open_space("constructor", 0, "123456");
 
             
    /* <read var="B" type="double[N   ][N   ]" opt="ONCE"/> */
    sprintf(_tp_name, "double(%d)(%d):B#%s", (N), (N), "123456");
    _tp_size = ((N) * (N)) * sizeof(double);
    _tp_B_123456 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_B_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = 0; _x0_123456 < (N); _x0_123456 +=1) {
        for (_x1_123456 = 0; _x1_123456 < (N); _x1_123456 +=1) {

            B[_x0_123456][_x1_123456] = _tp_B_123456[_x0_123456 * (N) + _x1_123456];
        }
    }

    free(_tp_B_123456);

 
             
while (1)
{

    /* <token action="GET" idxset="(i)"/> */
    sprintf(_tp_name, "token#%s", "123456");
    _tp_size = 0;
    _tp_size = _get_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tp_size < 0) exit(-1);
    if (_tp_token[0] == '!') break;
    sscanf(_tp_token, "%d@(i:%d~%d,%d)", &_tokens, &_i_start, &_i_stop, &_i_step);


    /* <read var="A" type="double[N(i)][N   ]"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", _i_start, _i_stop, _i_step, 0, (N), 1, sizeof(double));
    _tp_size = (((_i_stop - _i_start - 1) / _i_step + 1) * (N)) * sizeof(double);
    _tp_A_123456 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_A_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = _i_start, _y0_123456 =0; _x0_123456 < _i_stop; _x0_123456 +=_i_step, _y0_123456 ++) {
        for (_x1_123456 = 0; _x1_123456 < (N); _x1_123456 +=1) {

            A[_x0_123456][_x1_123456] = _tp_A_123456[_y0_123456 * (N) + _x1_123456];
        }
    }

    free(_tp_A_123456);

 

             
/* <target index ="i" order ="1" limits="(0,N,1)" chunk ="G"> */
for (i = _i_start; i < _i_stop; i +=_i_step) 
/* </target> */
 
            {
                for (k = 0; k < N; k++)
                {
                    for (j = 0; j < N; j++)
                    {
                        C[i][j] += A[i][k]*B[k][j];
                    }
                }
            }

             
    /* <send var="C" type="double[N(i)][N   ]"/> */
    sprintf(_tp_name, "double(%d)(%d):C#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", _i_start, _i_stop, _i_step, 0, (N), 1, sizeof(double));
    _tp_size = (((_i_stop - _i_start - 1) / _i_step + 1) * (N)) * sizeof(double);
    _tp_C_123456 = (double *)malloc(_tp_size);
    for (_x0_123456 = _i_start, _y0_123456 =0; _x0_123456 < _i_stop; _x0_123456 +=_i_step, _y0_123456 ++) {
        for (_x1_123456 = 0; _x1_123456 < (N); _x1_123456 +=1) {

            _tp_C_123456[_y0_123456 * (N) + _x1_123456] = C[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_C_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_C_123456);

 
         
}

_close_space(_constructor, "123456", 0);
_close_space(_distributor, "123456", 0);
/* </worker> */

exit(0);
}

/* </parallel> */
