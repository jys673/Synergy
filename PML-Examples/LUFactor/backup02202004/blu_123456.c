#include "parallel.h"

/* <reference> */
 
#include "blu.h"
 
/* </reference> */

/* <reference> */
 
    int i, j, k, row, col;
 
/* </reference> */

/* <parallel appname="blu"> */
int * _tp_k_123456;
float * _tp_MainMat_123456;
int _x0_123456;
int _y0_123456;
int _x1_123456;
int _y1_123456;
int _row_start = 0;
int _row_stop = 0;
int _row_step = 0;

main(int argc, char **argv[])
{

/* <worker id="123456"> */
_distributor = _open_space("distributor", 0, "123456");
_constructor = _open_space("constructor", 0, "123456");
 
             
while (1)
{

    /* <token action="GET" idxset="(row)"/> */
    sprintf(_tp_name, "token#%s", "123456");
    _tp_size = 0;
    _tp_size = _get_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tp_size < 0) exit(-1);
    if (_tp_token[0] == '!') break;
    sscanf(_tp_token, "%d@(row:%d~%d,%d)", &_tokens, &_row_start, &_row_stop, &_row_step);


    /* <read var="k" type="int"/> */
    sprintf(_tp_name, "int:k#%s", "123456");
    _tp_size = sizeof(int);
    _tp_k_123456 = &k;
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_k_123456, _tp_size);
    if (_tp_size < 0) exit(-1);

 

             
    /* <read var="MainMat" type="float[N(k~k+1)][N(k~N)  ]"/> */
    sprintf(_tp_name, "float(%d)(%d):MainMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", (k), (k+1), 1, (k), (N), 1, sizeof(float));
    _tp_size = (((k+1) - (k)) * ((N) - (k))) * sizeof(float);
    _tp_MainMat_123456 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_MainMat_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = (k), _y0_123456 =0; _x0_123456 < (k+1); _x0_123456 +=1, _y0_123456 ++) {
        for (_x1_123456 = (k), _y1_123456 =0; _x1_123456 < (N); _x1_123456 +=1, _y1_123456 ++) {

            MainMat[_x0_123456][_x1_123456] = _tp_MainMat_123456[_y0_123456 * ((N) - (k)) + _y1_123456];
        }
    }

    free(_tp_MainMat_123456);

 
             
    /* <read var="MainMat" type="float[N(row)  ][N(k~N)  ]"/> */
    sprintf(_tp_name, "float(%d)(%d):MainMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", _row_start, _row_stop, _row_step, (k), (N), 1, sizeof(float));
    _tp_size = (((_row_stop - _row_start - 1) / _row_step + 1) * ((N) - (k))) * sizeof(float);
    _tp_MainMat_123456 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_MainMat_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = _row_start, _y0_123456 =0; _x0_123456 < _row_stop; _x0_123456 +=_row_step, _y0_123456 ++) {
        for (_x1_123456 = (k), _y1_123456 =0; _x1_123456 < (N); _x1_123456 +=1, _y1_123456 ++) {

            MainMat[_x0_123456][_x1_123456] = _tp_MainMat_123456[_y0_123456 * ((N) - (k)) + _y1_123456];
        }
    }

    free(_tp_MainMat_123456);

 

         
/* <target index ="row" order ="1" limits="(k+1,N,1)" chunk ="G"> */
for (row = _row_start; row < _row_stop; row +=_row_step) 
/* </target> */
 
        {
            MainMat[row][k] = MainMat[row][k] / MainMat[k][k];
            for (col=k+1; col<N; col++)
            {
                MainMat[row][col] = MainMat[row][col] - MainMat[row][k] * MainMat[k][col];
            }
        }

             
    /* <send var="MainMat" type="float[N(row)  ][N(k~N)  ]"/> */
    sprintf(_tp_name, "float(%d)(%d):MainMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", _row_start, _row_stop, _row_step, (k), (N), 1, sizeof(float));
    _tp_size = (((_row_stop - _row_start - 1) / _row_step + 1) * ((N) - (k))) * sizeof(float);
    _tp_MainMat_123456 = (float *)malloc(_tp_size);
    for (_x0_123456 = _row_start, _y0_123456 =0; _x0_123456 < _row_stop; _x0_123456 +=_row_step, _y0_123456 ++) {
        for (_x1_123456 = (k), _y1_123456 =0; _x1_123456 < (N); _x1_123456 +=1, _y1_123456 ++) {

            _tp_MainMat_123456[_y0_123456 * ((N) - (k)) + _y1_123456] = MainMat[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_MainMat_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_MainMat_123456);

 
         
}

_close_space(_constructor, "123456", 0);
_close_space(_distributor, "123456", 0);
/* </worker> */

exit(0);
}

/* </parallel> */
