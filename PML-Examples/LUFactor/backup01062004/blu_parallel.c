#include "parallel.h"

 
/* <reference> */
 
#include "blu.h"
 
/* </reference> */
 

 
/* <parallel appname="blu"> */
int * _tp_i_234567;
int * _tp_j_234567;
float * _tp_outMat_234567;
int _x0_234567;
int _y0_234567;
int _x1_234567;
int _y1_234567;
int _k1_start = 0;
int _k1_stop = 0;
int _k1_step = 0;
 

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

     
/* <master id="234567"> */
_distributor = _open_space("distributor", 0, "234567");
_constructor = _open_space("constructor", 0, "234567");
 

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

        /* Solve triangles, LZ and WU */

        for (k1 = j+1; k1 < N; k1 = k1+M)
        {
            k2 = k1 + M - 1;
            subdist = M;
            if (k2 > N-1)
            {
                k2 = N - 1;
                subdist = N - k1;
            }
            /* Solve LZ */
            for (p1 = i; p1 < i+M; p1++)
            {
                for (p2 = k1; p2 <= k2; p2++)
                {
                    inSubMat[p1-i][p2-k1] = outMat[p1][p2];
                }
            }
            TriangleSolver(LU, inSubMat, outSubMat, subdist, 1);
            /* update */
            for (p1 = i; p1 < i+M; p1++)
            {
                for (p2 = k1; p2 <= k2; p2++)
                {
                    outMat[p1][p2] = outSubMat[p1-i][p2-k1]; 
                }
            }
            /* Solve WU */
            for (p1 = i; p1 < i+M; p1++)
            {
                for (p2 = k1; p2 <= k2; p2++)
                {
                    inSubMat[p2-k1][p1-i] = outMat[p2][p1];
                }
            }
            TriangleSolver(LU, inSubMat, outSubMat, subdist, 2);
            /* update */
            for (p1 = i; p1 < i+M; p1++)
            {
                for (p2 = k1; p2 <= k2; p2++)
                {
                    outMat[p2][p1] = outSubMat[p2-k1][p1-i]; 
                }
            }
        }

             
_cleanup_space(_distributor, "234567");
_cleanup_space(_constructor, "234567");
    /* <token action="SET" idxset="(k1)"/> */
    sprintf(_tp_name, "token#%s", "234567");
    sprintf(_tp_token, "=(k1:%d~%d,%d:#%d)", j+1, N, 1, M);
    _tp_size = sizeof(_tp_token);
    _tokens  = _set_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tokens < 0) exit(-1);


    /* <send var="i" type="int"/> */
    sprintf(_tp_name, "int:i#%s", "234567");
    _tp_size = sizeof(int);
    _tp_i_234567 = &i;
    _status  = _send_data(_distributor, _tp_name, (char *)_tp_i_234567, _tp_size);
    if (_status < 0) exit(-1);

 
             
    /* <send var="j" type="int"/> */
    sprintf(_tp_name, "int:j#%s", "234567");
    _tp_size = sizeof(int);
    _tp_j_234567 = &j;
    _status  = _send_data(_distributor, _tp_name, (char *)_tp_j_234567, _tp_size);
    if (_status < 0) exit(-1);

 
             
    /* <send var="outMat" type="float[N(i~N)][N(i~N)]"/> */
    sprintf(_tp_name, "float(%d)(%d):outMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "234567", (i), (N), 1, (i), (N), 1, sizeof(float));
    _tp_size = (((N) - (i)) * ((N) - (i))) * sizeof(float);
    _tp_outMat_234567 = (float *)malloc(_tp_size);
    for (_x0_234567 = (i), _y0_234567 =0; _x0_234567 < (N); _x0_234567 +=1, _y0_234567 ++) {
        for (_x1_234567 = (i), _y1_234567 =0; _x1_234567 < (N); _x1_234567 +=1, _y1_234567 ++) {

            _tp_outMat_234567[_y0_234567 * ((N) - (i)) + _y1_234567] = outMat[_x0_234567][_x1_234567];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_outMat_234567, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_outMat_234567);

 

          

             
    /* <read var="outMat" type="float[N(j+1~N)][N(j+1~N)]"/> */
    sprintf(_tp_name, "float(%d)(%d):outMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "234567", (j+1), (N), 1, (j+1), (N), 1, sizeof(float));
    _tp_size = (((N) - (j+1)) * ((N) - (j+1))) * sizeof(float);
    _tp_outMat_234567 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_outMat_234567, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_234567 = (j+1), _y0_234567 =0; _x0_234567 < (N); _x0_234567 +=1, _y0_234567 ++) {
        for (_x1_234567 = (j+1), _y1_234567 =0; _x1_234567 < (N); _x1_234567 +=1, _y1_234567 ++) {

            outMat[_x0_234567][_x1_234567] = _tp_outMat_234567[_y0_234567 * ((N) - (j+1)) + _y1_234567];
        }
    }

    free(_tp_outMat_234567);

 

        i = i + M;
    }

     
_close_space(_constructor, "234567", 1);
_close_space(_distributor, "234567", 1);
/* </master> */
 

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
 
