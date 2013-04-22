#include "parallel.h"

 
/* <reference> */
 
#include <stdio.h>
 
/* </reference> */
 

 
/* <reference id="123456"> */
 
#define N 100
 
/* </reference> */
 

 
/* <parallel appname="matrix"> */
int * _tp_scalar_123456;
double * _tp_A_123456;
double * _tp_B_123456;
double * _tp_C_123456;
int _x0_123456;
int _x1_123456;
int _y0_123456;
int _y1_123456;
 

main(int argc, char **argv[])
{
 
/* <reference id="123456"> */
 
    int scalar = 0;
    double A[N][N], B[N][N], C[N][N];
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
    /* <send var="scalar" type="int" opt="ONCE|XCHG"/> */
    sprintf(_tp_name, "int:scalar#%s", "123456");
    _tp_size = sizeof(int);
    _tp_scalar_123456 = &scalar;
    _status  = _send_data(_constructor, _tp_name, (char *)_tp_scalar_123456, _tp_size);
    if (_status < 0) exit(-1);

 
         
    /* <send var="scalar" type="int" opt="ONCE"/> */
    sprintf(_tp_name, "int:scalar#%s", "123456");
    _tp_size = sizeof(int);
    _tp_scalar_123456 = &scalar;
    _status  = _send_data(_distributor, _tp_name, (char *)_tp_scalar_123456, _tp_size);
    if (_status < 0) exit(-1);

 

         
    /* <send var="A" type="double[N        ][N        ]" opt="ONCE|XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s", (N), (N), "123456");
    _tp_size = ((N) * (N)) * sizeof(double);
    _tp_A_123456 = (double *)malloc(_tp_size);
    for (_x0_123456 = 0; _x0_123456 < (N); _x0_123456 +=1) {
        for (_x1_123456 = 0; _x1_123456 < (N); _x1_123456 +=1) {

            _tp_A_123456[_x0_123456 * (N) + _x1_123456] = A[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_A_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_123456);

 
         
    /* <send var="A" type="double[N(i:i-1) ][N        ]" opt="ONCE|XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", (_i_start-1), (_i_stop-1), _i_step, 0, (N), 1, sizeof(double));
    _tp_size = ((((_i_stop-1) - (_i_start-1) - 1) / _i_step + 1) * (N)) * sizeof(double);
    _tp_A_123456 = (double *)malloc(_tp_size);
    for (_x0_123456 = (_i_start-1), _y0_123456 =0; _x0_123456 < (_i_stop-1); _x0_123456 +=_i_step, _y0_123456 ++) {
        for (_x1_123456 = 0; _x1_123456 < (N); _x1_123456 +=1) {

            _tp_A_123456[_y0_123456 * (N) + _x1_123456] = A[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_A_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_123456);

 
         
    /* <send var="A" type="double[N        ][N(k)     ]" opt="ONCE"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", 0, (N), 1, _k_start, _k_stop, _k_step, sizeof(double));
    _tp_size = ((N) * ((_k_stop - _k_start - 1) / _k_step + 1)) * sizeof(double);
    _tp_A_123456 = (double *)malloc(_tp_size);
    for (_x0_123456 = 0; _x0_123456 < (N); _x0_123456 +=1) {
        for (_x1_123456 = _k_start, _y1_123456 =0; _x1_123456 < _k_stop; _x1_123456 +=_k_step, _y1_123456 ++) {

            _tp_A_123456[_x0_123456 * ((_k_stop - _k_start - 1) / _k_step + 1) + _y1_123456] = A[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_A_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_123456);

 
         
    /* <send var="A" type="double[N(i)     ][N(k:$H-1)]" opt="ONCE"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", _i_start, _i_stop, _i_step, _k_stop, (_k_stop-1), _k_step, sizeof(double));
    _tp_size = (((_i_stop - _i_start - 1) / _i_step + 1) * (((_k_stop-1) - _k_stop - 1) / _k_step + 1)) * sizeof(double);
    _tp_A_123456 = (double *)malloc(_tp_size);
    for (_x0_123456 = _i_start, _y0_123456 =0; _x0_123456 < _i_stop; _x0_123456 +=_i_step, _y0_123456 ++) {
        for (_x1_123456 = _k_stop, _y1_123456 =0; _x1_123456 < (_k_stop-1); _x1_123456 +=_k_step, _y1_123456 ++) {

            _tp_A_123456[_y0_123456 * (((_k_stop-1) - _k_stop - 1) / _k_step + 1) + _y1_123456] = A[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_A_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_123456);

 

         
    /* <token action="SET" idxset="(i)(k)"/> */
    sprintf(_tp_name, "token#%s", "123456");
    sprintf(_tp_token, "=(i:%d~%d,%d:#%d)(k:%d~%d,%d:^%d)", 0, N, 1, 100, 0, N, 1, 20);
    _tp_size = sizeof(_tp_token);
    _tokens  = _set_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tokens < 0) exit(-1);


    /* <send var="scalar" type="int" opt="XCHG"/> */
    sprintf(_tp_name, "int:scalar#%s", "123456");
    _tp_size = sizeof(int);
    _tp_scalar_123456 = &scalar;
    _status  = _send_data(_constructor, _tp_name, (char *)_tp_scalar_123456, _tp_size);
    if (_status < 0) exit(-1);

 
         
    /* <send var="scalar" type="int"/> */
    sprintf(_tp_name, "int:scalar#%s", "123456");
    _tp_size = sizeof(int);
    _tp_scalar_123456 = &scalar;
    _status  = _send_data(_distributor, _tp_name, (char *)_tp_scalar_123456, _tp_size);
    if (_status < 0) exit(-1);

 

         
    /* <send var="B" type="double[N        ][N        ]"/> */
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

 
         
    /* <send var="B" type="double[N(i)     ][N        ]"/> */
    sprintf(_tp_name, "double(%d)(%d):B#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", _i_start, _i_stop, _i_step, 0, (N), 1, sizeof(double));
    _tp_size = (((_i_stop - _i_start - 1) / _i_step + 1) * (N)) * sizeof(double);
    _tp_B_123456 = (double *)malloc(_tp_size);
    for (_x0_123456 = _i_start, _y0_123456 =0; _x0_123456 < _i_stop; _x0_123456 +=_i_step, _y0_123456 ++) {
        for (_x1_123456 = 0; _x1_123456 < (N); _x1_123456 +=1) {

            _tp_B_123456[_y0_123456 * (N) + _x1_123456] = B[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_B_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_B_123456);

 
         
    /* <send var="B" type="double[N        ][N(k:k-1) ]" opt="XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):B#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", 0, (N), 1, (_k_start-1), (_k_stop-1), _k_step, sizeof(double));
    _tp_size = ((N) * (((_k_stop-1) - (_k_start-1) - 1) / _k_step + 1)) * sizeof(double);
    _tp_B_123456 = (double *)malloc(_tp_size);
    for (_x0_123456 = 0; _x0_123456 < (N); _x0_123456 +=1) {
        for (_x1_123456 = (_k_start-1), _y1_123456 =0; _x1_123456 < (_k_stop-1); _x1_123456 +=_k_step, _y1_123456 ++) {

            _tp_B_123456[_x0_123456 * (((_k_stop-1) - (_k_start-1) - 1) / _k_step + 1) + _y1_123456] = B[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_B_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_B_123456);

 
         
    /* <send var="B" type="double[N(i:$L-1)][N(k)     ]" opt="XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):B#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", (_i_start-1), _i_start, _i_step, _k_start, _k_stop, _k_step, sizeof(double));
    _tp_size = (((_i_start - (_i_start-1) - 1) / _i_step + 1) * ((_k_stop - _k_start - 1) / _k_step + 1)) * sizeof(double);
    _tp_B_123456 = (double *)malloc(_tp_size);
    for (_x0_123456 = (_i_start-1), _y0_123456 =0; _x0_123456 < _i_start; _x0_123456 +=_i_step, _y0_123456 ++) {
        for (_x1_123456 = _k_start, _y1_123456 =0; _x1_123456 < _k_stop; _x1_123456 +=_k_step, _y1_123456 ++) {

            _tp_B_123456[_y0_123456 * ((_k_stop - _k_start - 1) / _k_step + 1) + _y1_123456] = B[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_B_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_B_123456);

 

      

         
    /* <read var="C" type="double[N        ][N        ]"/> */
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

 
         
    /* <read var="C" type="double[N(i:i-1) ][N        ]" opt="XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):C#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", (_i_start-1), (_i_stop-1), _i_step, 0, (N), 1, sizeof(double));
    _tp_size = ((((_i_stop-1) - (_i_start-1) - 1) / _i_step + 1) * (N)) * sizeof(double);
    _tp_C_123456 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_C_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = (_i_start-1), _y0_123456 =0; _x0_123456 < (_i_stop-1); _x0_123456 +=_i_step, _y0_123456 ++) {
        for (_x1_123456 = 0; _x1_123456 < (N); _x1_123456 +=1) {

            C[_x0_123456][_x1_123456] = _tp_C_123456[_y0_123456 * (N) + _x1_123456];
        }
    }

    free(_tp_C_123456);

 
         
    /* <read var="C" type="double[N        ][N(k:k-1) ]" opt="XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):C#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", 0, (N), 1, (_k_start-1), (_k_stop-1), _k_step, sizeof(double));
    _tp_size = ((N) * (((_k_stop-1) - (_k_start-1) - 1) / _k_step + 1)) * sizeof(double);
    _tp_C_123456 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_C_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = 0; _x0_123456 < (N); _x0_123456 +=1) {
        for (_x1_123456 = (_k_start-1), _y1_123456 =0; _x1_123456 < (_k_stop-1); _x1_123456 +=_k_step, _y1_123456 ++) {

            C[_x0_123456][_x1_123456] = _tp_C_123456[_x0_123456 * (((_k_stop-1) - (_k_start-1) - 1) / _k_step + 1) + _y1_123456];
        }
    }

    free(_tp_C_123456);

 
         
    /* <read var="C" type="double[N(i:$H-1)][N(k:$L-1)]"/> */
    sprintf(_tp_name, "double(%d)(%d):C#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", _i_stop, (_i_stop-1), _i_step, (_k_start-1), _k_start, _k_step, sizeof(double));
    _tp_size = ((((_i_stop-1) - _i_stop - 1) / _i_step + 1) * ((_k_start - (_k_start-1) - 1) / _k_step + 1)) * sizeof(double);
    _tp_C_123456 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_C_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = _i_stop, _y0_123456 =0; _x0_123456 < (_i_stop-1); _x0_123456 +=_i_step, _y0_123456 ++) {
        for (_x1_123456 = (_k_start-1), _y1_123456 =0; _x1_123456 < _k_start; _x1_123456 +=_k_step, _y1_123456 ++) {

            C[_x0_123456][_x1_123456] = _tp_C_123456[_y0_123456 * ((_k_start - (_k_start-1) - 1) / _k_step + 1) + _y1_123456];
        }
    }

    free(_tp_C_123456);

 

         
    /* <read var="scalar" type="int" opt="_MAX|XCHG"/> */
    sprintf(_tp_name, "int:scalar#%s?MAX@%d", "123456", _tokens);
    _tp_size = sizeof(int);
    _tp_scalar_123456 = &scalar;
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_scalar_123456, _tp_size);
    if (_tp_size < 0) exit(-1);

 
         
    /* <read var="scalar" type="int" opt="_MIN"/> */
    sprintf(_tp_name, "int:scalar#%s?MIN@%d", "123456", _tokens);
    _tp_size = sizeof(int);
    _tp_scalar_123456 = &scalar;
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_scalar_123456, _tp_size);
    if (_tp_size < 0) exit(-1);

 

     
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
 
