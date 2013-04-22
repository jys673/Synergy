#include "parallel.h"

/* <reference> */
 
#include "blu.h"
 
/* </reference> */

/* <reference> */
 
    float inSubMat[M][M], outSubMat[M][M], LU[M][M], L[M][M], U[M][M];
    int subdist, rowdist, coldist;
    int i, j, dist, k1, k2, p1, p2, q1, q2, q3;
 
/* </reference> */

/* <parallel appname="blu"> */
int * _tp_j_234567;
float * _tp_outMat_234567;
int _x0_234567;
int _x1_234567;
int _y0_234567;
int _y1_234567;
int _k1_start = 0;
int _k1_stop = 0;
int _k1_step = 0;

main(int argc, char **argv[])
{

/* <worker id="234567"> */
_distributor = _open_space("distributor", 0, "234567");
_constructor = _open_space("constructor", 0, "234567");
 
             
while (1)
{

    /* <token action="GET" idxset="(k1)"/> */
    sprintf(_tp_name, "token#%s", "234567");
    _tp_size = 0;
    _tp_size = _get_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tp_size < 0) exit(-1);
    if (_tp_token[0] == '!') break;
    sscanf(_tp_token, "%d@(k1:%d~%d,%d)", &_tokens, &_k1_start, &_k1_stop, &_k1_step);


    /* <read var="j" type="int"/> */
    sprintf(_tp_name, "int:j#%s", "234567");
    _tp_size = sizeof(int);
    _tp_j_234567 = &j;
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_j_234567, _tp_size);
    if (_tp_size < 0) exit(-1);

 
             
    /* <read var="outMat" type="float[N   ][N   ]"/> */
    sprintf(_tp_name, "float(%d)(%d):outMat#%s", (N), (N), "234567");
    _tp_size = ((N) * (N)) * sizeof(float);
    _tp_outMat_234567 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_outMat_234567, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_234567 = 0; _x0_234567 < (N); _x0_234567 +=1) {
        for (_x1_234567 = 0; _x1_234567 < (N); _x1_234567 +=1) {

            outMat[_x0_234567][_x1_234567] = _tp_outMat_234567[_x0_234567 * (N) + _x1_234567];
        }
    }

    free(_tp_outMat_234567);

 

        /* A = A - WZ */

         
/* <target index ="k1" order ="1" limits="(j+1,N,1)" chunk ="M"> */
for (k1 = _k1_start; k1 < _k1_stop; k1 +=_k1_step) 
/* </target> */
 
        {
            k2 = k1 + M - 1;
            rowdist = M;
            if (k2 > N-1)
            {
                k2 = N - 1;
                rowdist = N - k1;
            }
            p1 = j + 1;
            while (p1 < N)
            {
                p2 = p1 + M - 1;
                coldist = M;
                if (p2 > N-1)
                {
                    p2 = N - 1;
                    coldist = N - p1;
                }
                /* matrix multiplication
                   outMat[k1:k2][p1:p2] = 
                   outMat[k1:k2][p1:p2] - outMat[k1:k2][i:i+M-1] * outMat[i:i+M-1][k1:k2];
                 */
                for (q1 = k1; q1 <= k2; q1++)
                {
                    for (q2 = p1; q2 <= p2; q2++)
                    {
                        for (q3 = 0; q3 < M; q3++)
                        {
                            outMat[q1][q2] = outMat[q1][q2] - outMat[q1][i+q3]*outMat[i+q3][q2];
                        }
                    }
                }
                p1 = p1 + M;  
            }
        }

             
    /* <send var="outMat" type="float[N(k1)   ][N(j~j+M)]"/> */
    sprintf(_tp_name, "float(%d)(%d):outMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "234567", _k1_start, _k1_stop, _k1_step, (j), (j+M), 1, sizeof(float));
    _tp_size = (((_k1_stop - _k1_start - 1) / _k1_step + 1) * ((j+M) - (j))) * sizeof(float);
    _tp_outMat_234567 = (float *)malloc(_tp_size);
    for (_x0_234567 = _k1_start, _y0_234567 =0; _x0_234567 < _k1_stop; _x0_234567 +=_k1_step, _y0_234567 ++) {
        for (_x1_234567 = (j), _y1_234567 =0; _x1_234567 < (j+M); _x1_234567 +=1, _y1_234567 ++) {

            _tp_outMat_234567[_y0_234567 * ((j+M) - (j)) + _y1_234567] = outMat[_x0_234567][_x1_234567];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_outMat_234567, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_outMat_234567);

 
         
}

_close_space(_constructor, "234567", 0);
_close_space(_distributor, "234567", 0);
/* </worker> */

exit(0);
}

/* </parallel> */
