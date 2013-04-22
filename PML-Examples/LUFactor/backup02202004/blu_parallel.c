#include "parallel.h"

 
/* <reference> */
 
#include "blu.h"
 
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
 

int main()
{
 
/* <reference> */
 
    int i, j, k, row, col;
 
/* </reference> */
 

    double t0, t1;	

    for (i=0; i<N; i++)
    {
        for (j=0; j<N; j++)
        {
            if (i==j)
            {
                MainMat[i][j] = N;
            }
            else
            {
                MainMat[i][j] = 1;
            }
        }
    }

    t0 = wall_clock();

     
/* <master id="123456"> */
_distributor = _open_space("distributor", 0, "123456");
_constructor = _open_space("constructor", 0, "123456");
 

    for (k=0; k<N-1; k++)
    {
             
_cleanup_space(_distributor, "123456");
_cleanup_space(_constructor, "123456");
    /* <token action="SET" idxset="(row)"/> */
    sprintf(_tp_name, "token#%s", "123456");
    sprintf(_tp_token, "=(row:%d~%d,%d:#%d)", k+1, N, 1, G);
    _tp_size = sizeof(_tp_token);
    _tokens  = _set_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tokens < 0) exit(-1);


    /* <send var="k" type="int"/> */
    sprintf(_tp_name, "int:k#%s", "123456");
    _tp_size = sizeof(int);
    _tp_k_123456 = &k;
    _status  = _send_data(_distributor, _tp_name, (char *)_tp_k_123456, _tp_size);
    if (_status < 0) exit(-1);

 
             
    /* <send var="MainMat" type="float[N(k~N)  ][N(k~N)  ]"/> */
    sprintf(_tp_name, "float(%d)(%d):MainMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", (k), (N), 1, (k), (N), 1, sizeof(float));
    _tp_size = (((N) - (k)) * ((N) - (k))) * sizeof(float);
    _tp_MainMat_123456 = (float *)malloc(_tp_size);
    for (_x0_123456 = (k), _y0_123456 =0; _x0_123456 < (N); _x0_123456 +=1, _y0_123456 ++) {
        for (_x1_123456 = (k), _y1_123456 =0; _x1_123456 < (N); _x1_123456 +=1, _y1_123456 ++) {

            _tp_MainMat_123456[_y0_123456 * ((N) - (k)) + _y1_123456] = MainMat[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_MainMat_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_MainMat_123456);

 

          

             
    /* <read var="MainMat" type="float[N(k+1~N)][N(k~N)  ]"/> */
    sprintf(_tp_name, "float(%d)(%d):MainMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", (k+1), (N), 1, (k), (N), 1, sizeof(float));
    _tp_size = (((N) - (k+1)) * ((N) - (k))) * sizeof(float);
    _tp_MainMat_123456 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_MainMat_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = (k+1), _y0_123456 =0; _x0_123456 < (N); _x0_123456 +=1, _y0_123456 ++) {
        for (_x1_123456 = (k), _y1_123456 =0; _x1_123456 < (N); _x1_123456 +=1, _y1_123456 ++) {

            MainMat[_x0_123456][_x1_123456] = _tp_MainMat_123456[_y0_123456 * ((N) - (k)) + _y1_123456];
        }
    }

    free(_tp_MainMat_123456);

 
    }

     
_close_space(_constructor, "123456", 1);
_close_space(_distributor, "123456", 1);
/* </master> */
 

    t1 = wall_clock() - t0;

/*
    for (i=0; i<N; i++)
    {
        for (j=0; j<N; j++)
        {
            printf("%6.3f ", MainMat[i][j]);
        }
        printf("\n");
    }
*/

    if (t1>0) printf(" (%f) MFLOPS.\n", (float) 2*N*N*N/3/t1);
    else printf(" MFLOPS: Not measured.\n");
    printf("elapse time = %10.6f\n", t1/1000000);

    return 0;
}

 
/* </parallel> */
 
