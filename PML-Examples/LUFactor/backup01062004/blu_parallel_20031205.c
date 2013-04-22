#include "parallel.h"

 
/* <reference> */
 
#include "blu.h"
 
/* </reference> */
 

 
/* <parallel appname="blu"> */
int * _tp_i_123456;
float * _tp_LU_123456;
float * _tp_outMat_123456;
int _x0_123456;
int _x1_123456;
int _y0_123456;
int _y1_123456;
int _k1_start = 0;
int _k1_stop = 0;
int _k1_step = 0;
int * _tp_j_234567;
float * _tp_outMat_234567;
int _x0_234567;
int _x1_234567;
int _y0_234567;
int _y1_234567;
 

int main(int argc, char **argv[])
{
 
/* <reference> */
 
    float inSubMat[M][M], outSubMat[M][M], LU[M][M], L[M][M], U[M][M];
    int subdist, rowdist, coldist;
    int i, j, dist, k1, k2, p1, p2, q1, q2, q3;
 
/* </reference> */
 

    double t0, t1;

    t0 = wall_clock();

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            if (i == j)
            {
                outMat[i][j] = N;
            }
            else
            {
                outMat[i][j] = 1;
            }
        }
    }


_distributor = _open_space("distributor", 0, "123456");
_constructor = _open_space("constructor", 0, "123456");
 
    i = 0;
    while (i < N)
    {
        j = i + M - 1;
        dist = M;
        if (j > N-1) 
        {
            j = N-1;
            dist = N - i;
        }
        /* LU factors for submatrix */
        for (k1 = 0; k1 < dist; k1++)
        {
            for (k2 = 0; k2 < dist; k2++)
            {
                inSubMat[k1][k2] = outMat[i+k1][i+k2];
            }
        }
        LUFactor(inSubMat, outSubMat, dist);
        /* update */
        for (k1 = 0; k1 < dist; k1++)
        {
            for (k2 = 0; k2 < dist; k2++)
            {
                outMat[i+k1][i+k2] = outSubMat[k1][k2];
                LU[k1][k2] = outSubMat[k1][k2];
            }
        }

         
/* <master id="123456"> */
             
_cleanup_space(_distributor, "123456");
_cleanup_space(_constructor, "123456");
    /* <token action="SET" idxset="(k1)"/> */
    sprintf(_tp_name, "token#%s", "123456");
    sprintf(_tp_token, "=(k1:%d~%d,%d:#%d)", j+1, N, 1, M);
    _tp_size = sizeof(_tp_token);
    _tokens  = _set_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tokens < 0) exit(-1);


    /* <send var="i" type="int"/> */
    sprintf(_tp_name, "int:i#%s", "123456");
    _tp_size = sizeof(int);
    _tp_i_123456 = &i;
    _status  = _send_data(_distributor, _tp_name, (char *)_tp_i_123456, _tp_size);
    if (_status < 0) exit(-1);

 
             
    /* <send var="LU" type="float[M   ][M   ]"/> */
    sprintf(_tp_name, "float(%d)(%d):LU#%s", (M), (M), "123456");
    _tp_size = ((M) * (M)) * sizeof(float);
    _tp_LU_123456 = (float *)malloc(_tp_size);
    for (_x0_123456 = 0; _x0_123456 < (M); _x0_123456 +=1) {
        for (_x1_123456 = 0; _x1_123456 < (M); _x1_123456 +=1) {

            _tp_LU_123456[_x0_123456 * (M) + _x1_123456] = LU[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_LU_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_LU_123456);

 
             
    /* <send var="outMat" type="float[N   ][N   ]"/> */
    sprintf(_tp_name, "float(%d)(%d):outMat#%s", (N), (N), "123456");
    _tp_size = ((N) * (N)) * sizeof(float);
    _tp_outMat_123456 = (float *)malloc(_tp_size);
    for (_x0_123456 = 0; _x0_123456 < (N); _x0_123456 +=1) {
        for (_x1_123456 = 0; _x1_123456 < (N); _x1_123456 +=1) {

            _tp_outMat_123456[_x0_123456 * (N) + _x1_123456] = outMat[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_outMat_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_outMat_123456);

 

          

             
    /* <read var="outMat" type="float[N(i~i+M)][N(j+1~N)]"/> */
    sprintf(_tp_name, "float(%d)(%d):outMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", (i), (i+M), 1, (j+1), (N), 1, sizeof(float));
    _tp_size = (((i+M) - (i)) * ((N) - (j+1))) * sizeof(float);
    _tp_outMat_123456 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_outMat_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = (i), _y0_123456 =0; _x0_123456 < (i+M); _x0_123456 +=1, _y0_123456 ++) {
        for (_x1_123456 = (j+1), _y1_123456 =0; _x1_123456 < (N); _x1_123456 +=1, _y1_123456 ++) {

            outMat[_x0_123456][_x1_123456] = _tp_outMat_123456[_y0_123456 * ((N) - (j+1)) + _y1_123456];
        }
    }

    free(_tp_outMat_123456);

 
             
    /* <read var="outMat" type="float[N(j+1~N)][N(i~i+M)]"/> */
    sprintf(_tp_name, "float(%d)(%d):outMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", (j+1), (N), 1, (i), (i+M), 1, sizeof(float));
    _tp_size = (((N) - (j+1)) * ((i+M) - (i))) * sizeof(float);
    _tp_outMat_123456 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_outMat_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = (j+1), _y0_123456 =0; _x0_123456 < (N); _x0_123456 +=1, _y0_123456 ++) {
        for (_x1_123456 = (i), _y1_123456 =0; _x1_123456 < (i+M); _x1_123456 +=1, _y1_123456 ++) {

            outMat[_x0_123456][_x1_123456] = _tp_outMat_123456[_y0_123456 * ((i+M) - (i)) + _y1_123456];
        }
    }

    free(_tp_outMat_123456);

 
         
/* </master> */
 

         
/* <master id="234567"> */
 
             
_cleanup_space(_distributor, "234567");
_cleanup_space(_constructor, "234567");
    /* <token action="SET" idxset="(k1)"/> */
    sprintf(_tp_name, "token#%s", "234567");
    sprintf(_tp_token, "=(k1:%d~%d,%d:#%d)", j+1, N, 1, M);
    _tp_size = sizeof(_tp_token);
    _tokens  = _set_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tokens < 0) exit(-1);


    /* <send var="j" type="int"/> */
    sprintf(_tp_name, "int:j#%s", "234567");
    _tp_size = sizeof(int);
    _tp_j_234567 = &j;
    _status  = _send_data(_distributor, _tp_name, (char *)_tp_j_234567, _tp_size);
    if (_status < 0) exit(-1);

 
             
    /* <send var="outMat" type="float[N   ][N   ]"/> */
    sprintf(_tp_name, "float(%d)(%d):outMat#%s", (N), (N), "234567");
    _tp_size = ((N) * (N)) * sizeof(float);
    _tp_outMat_234567 = (float *)malloc(_tp_size);
    for (_x0_234567 = 0; _x0_234567 < (N); _x0_234567 +=1) {
        for (_x1_234567 = 0; _x1_234567 < (N); _x1_234567 +=1) {

            _tp_outMat_234567[_x0_234567 * (N) + _x1_234567] = outMat[_x0_234567][_x1_234567];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_outMat_234567, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_outMat_234567);

 

          

             
    /* <read var="outMat" type="float[N(j+1~N)][N(j~j+M)]"/> */
    sprintf(_tp_name, "float(%d)(%d):outMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "234567", (j+1), (N), 1, (j), (j+M), 1, sizeof(float));
    _tp_size = (((N) - (j+1)) * ((j+M) - (j))) * sizeof(float);
    _tp_outMat_234567 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_outMat_234567, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_234567 = (j+1), _y0_234567 =0; _x0_234567 < (N); _x0_234567 +=1, _y0_234567 ++) {
        for (_x1_234567 = (j), _y1_234567 =0; _x1_234567 < (j+M); _x1_234567 +=1, _y1_234567 ++) {

            outMat[_x0_234567][_x1_234567] = _tp_outMat_234567[_y0_234567 * ((j+M) - (j)) + _y1_234567];
        }
    }

    free(_tp_outMat_234567);

 
/* </master> */
 

        i = i + M;
    }

         
         
_close_space(_constructor, "123456", 1);
_close_space(_distributor, "123456", 1);

_close_space(_constructor, "234567", 1);
_close_space(_distributor, "234567", 1);


/*
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            printf("%6.3f ", outMat[i][j]);
        }
        printf("\n");
    }
*/
    t1 = wall_clock() - t0;
    if (t1>0) printf(" (%f) MFLOPS.\n", (float) 2*N*N*N/3/t1);
    else printf(" MFLOPS: Not measured.\n");
    printf("elapse time = %10.6f\n", t1/1000000);

    return 0;
}

 
/* </parallel> */
 
