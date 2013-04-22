#include "parallel.h"

/* <reference> */
 
# include <stdio.h>
# include <time.h>
# include <math.h>

# include "laplace.h"
 
/* </reference> */

/* <parallel appname="lap"> */
double * _tp_A_234567;
double * _tp_resid_234567;
int _x0_234567;
int _x1_234567;
int _y0_234567;
int _y1_234567;
int _i_start = 0;
int _i_stop = 0;
int _i_step = 0;
int _j_start = 0;
int _j_stop = 0;
int _j_step = 0;

int _batch_flag = 0;

main(int argc, char **argv[])
{

/* <worker id="234567"> */
_distributor = _open_space("distributor", 0, "234567");
_constructor = _open_space("constructor", 0, "234567");
 

         
while (1)
{

    /* <token action="GET" idxset="(i)(j)"/> */
    sprintf(_tp_name, "token#%s", "234567");
    _tp_size = 0;
    _tp_size = _get_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tp_size < 0) exit(-1);
    if (_tp_token[0] == '!') break;
    sscanf(_tp_token, "%d@(i:%d~%d,%d)(j:%d~%d,%d)", &_tokens, &_i_start, &_i_stop, &_i_step, &_j_start, &_j_stop, &_j_step);

    if (_tokens >= _batch_flag)
    {
    /* <read var="A" type="double[DIM+2     ][DIM+2     ]"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s", (DIM+2), (DIM+2), "234567");
    _tp_size = ((DIM+2) * (DIM+2)) * sizeof(double);
    _tp_A_234567 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_A_234567, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_234567 = 0; _x0_234567 < (DIM+2); _x0_234567 +=1) {
        for (_x1_234567 = 0; _x1_234567 < (DIM+2); _x1_234567 +=1) {

            A[_x0_234567][_x1_234567] = _tp_A_234567[_x0_234567 * (DIM+2) + _x1_234567];
        }
    }

    free(_tp_A_234567);
    }
    _batch_flag = _tokens;

         
    /* <read var="A" type="double[DIM+2(i:$L-1)][DIM+2(j)     ]" opt="XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (DIM+2), (DIM+2), "234567", (_i_start-1), _i_start, _i_step, _j_start, _j_stop, _j_step, sizeof(double));
    _tp_size = (((_i_start - (_i_start-1) - 1) / _i_step + 1) * ((_j_stop - _j_start - 1) / _j_step + 1)) * sizeof(double);
    _tp_A_234567 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_A_234567, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_234567 = (_i_start-1), _y0_234567 =0; _x0_234567 < _i_start; _x0_234567 +=_i_step, _y0_234567 ++) {
        for (_x1_234567 = _j_start, _y1_234567 =0; _x1_234567 < _j_stop; _x1_234567 +=_j_step, _y1_234567 ++) {

            A[_x0_234567][_x1_234567] = _tp_A_234567[_y0_234567 * ((_j_stop - _j_start - 1) / _j_step + 1) + _y1_234567];
        }
    }

    free(_tp_A_234567);

 
         
    /* <read var="A" type="double[DIM+2(i)     ][DIM+2(j:$L-1)]" opt="XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (DIM+2), (DIM+2), "234567", _i_start, _i_stop, _i_step, (_j_start-1), _j_start, _j_step, sizeof(double));
    _tp_size = (((_i_stop - _i_start - 1) / _i_step + 1) * ((_j_start - (_j_start-1) - 1) / _j_step + 1)) * sizeof(double);
    _tp_A_234567 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_A_234567, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_234567 = _i_start, _y0_234567 =0; _x0_234567 < _i_stop; _x0_234567 +=_i_step, _y0_234567 ++) {
        for (_x1_234567 = (_j_start-1), _y1_234567 =0; _x1_234567 < _j_start; _x1_234567 +=_j_step, _y1_234567 ++) {

            A[_x0_234567][_x1_234567] = _tp_A_234567[_y0_234567 * ((_j_start - (_j_start-1) - 1) / _j_step + 1) + _y1_234567];
        }
    }

    free(_tp_A_234567);

 

        /* 
         *  Refine the values in A, these two loops consume most of
         *  the computational time.  
         */

        resid = 0.0;

         
/* <target index ="i" order ="1" limits="(1,rdim+1,1)" chunk ="GRAIN"> */
for (i = _i_start; i < _i_stop; i +=_i_step) 
/* </target> */
 
         {
             
/* <target index ="j" order ="1" limits="(1,DIM+1,1)" chunk ="GRAIN"> */
for (j = _j_start; j < _j_stop; j +=_j_step) 
/* </target> */
 
             {
                Atmp     = A[i][j];
                A[i][j]  = 0.25 * (A[i+1][j] + A[i-1][j] + 
                                   A[i][j+1] + A[i][j-1]);

                residtmp = (Atmp!=0.0? fabs(fabs(A[i][j]-Atmp)/Atmp) : (1.0+EPS));

                if (residtmp > resid)
                    resid = residtmp;
             }
         }

         
    /* <send var="resid" type="double" opt="_MAX"/> */
    sprintf(_tp_name, "double:resid#%s?MAX@%d", "234567", _tokens);
    _tp_size = sizeof(double);
    _tp_resid_234567 = &resid;
    _status  = _send_data(_constructor, _tp_name, (char *)_tp_resid_234567, _tp_size);
    if (_status < 0) exit(-1);

 
         
    /* <send var="A" type="double[DIM+2(i)  ][DIM+2(j)  ]"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (DIM+2), (DIM+2), "234567", _i_start, _i_stop, _i_step, _j_start, _j_stop, _j_step, sizeof(double));
    _tp_size = (((_i_stop - _i_start - 1) / _i_step + 1) * ((_j_stop - _j_start - 1) / _j_step + 1)) * sizeof(double);
    _tp_A_234567 = (double *)malloc(_tp_size);
    for (_x0_234567 = _i_start, _y0_234567 =0; _x0_234567 < _i_stop; _x0_234567 +=_i_step, _y0_234567 ++) {
        for (_x1_234567 = _j_start, _y1_234567 =0; _x1_234567 < _j_stop; _x1_234567 +=_j_step, _y1_234567 ++) {

            _tp_A_234567[_y0_234567 * ((_j_stop - _j_start - 1) / _j_step + 1) + _y1_234567] = A[_x0_234567][_x1_234567];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_A_234567, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_234567);

 

         
}

_close_space(_constructor, "234567", 0);
_close_space(_distributor, "234567", 0);
/* </worker> */

exit(0);
}

/* </parallel> */
