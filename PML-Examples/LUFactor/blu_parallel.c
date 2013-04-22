#include "parallel.h"

 
/* <reference> */
 
#include "blu.h"
 
/* </reference> */
 

 
/* <parallel appname="blu"> */
int * _tp_i_123456;
float * _tp_outMat_123456;
int _x0_123456;
int _y0_123456;
int _x1_123456;
int _y1_123456;
int _k1_start = 0;
int _k1_stop = 0;
int _k1_step = 0;
 

int main()
{
 
/* <reference> */
 
    int i, j, dist, k1, k2, p1, p2, q1, q2, q3;
    int subdist, rowdist, coldist;
  
    float inSubMat[M][M], outSubMat[M][M], LU[M][M], L[M][M], U[M][M];
 
/* </reference> */
 

    double t0, t1;

    t0 = wall_clock();

    for (i=0; i<N; i++)
    {
        for (j=0; j<N; j++)
        {
            if (i==j)
            {
                outMat[i][j] = N;
            }
            else
            {
                outMat[i][j] = 1;
            }
        }
    }
  
     
/* <master id="123456"> */
_distributor = _open_space("distributor", 0, "123456");
_constructor = _open_space("constructor", 0, "123456");
 

    for (i = 0; i < N; i = i + M)
    {
        j = i + M - 1;
        dist = M;
        if (j > N-1) 
        {
            j = N-1;
            dist = N - i;
        }
    
        // LU factors for submatrix
    
        for (k1 = 0; k1 < dist; k1++)
        {
            for (k2 = 0; k2 < dist; k2++)
            {
                inSubMat[k1][k2] = outMat[i+k1][i+k2];
            }
        }
    
        LUFactor(inSubMat, outSubMat, dist);
    
        //update
        for (k1 = 0; k1 < dist; k1++)
        {
            for (k2 = 0; k2 < dist; k2++)
            {
                outMat[i+k1][i+k2] = outSubMat[k1][k2];
                LU[k1][k2] = outSubMat[k1][k2];
            }
        }
   
        for (k1 = j + 1; k1 < N; k1 = k1 + M)
        {
            k2 = k1 + M - 1;
            subdist = M;
            if (k2 > N-1)
            {
                k2 = N - 1;
                subdist = N - k1;
            }
      
            //Solve LZ
            for (p1 = i; p1 < i+M; p1++)
            {
                for (p2 = k1; p2 <= k2; p2++)
                {
                    inSubMat[p1-i][p2-k1] = outMat[p1][p2];
                }
            }
            TriangleSolver(LU, inSubMat, outSubMat, subdist, 1);
      
            //update
            for (p1 = i; p1 < i+M; p1++)
            {
                for (p2 = k1; p2 <= k2; p2++)
                {
                    outMat[p1][p2] = outSubMat[p1-i][p2-k1]; 
                }
            }    
      
            //Solve WU
            for (p1 = i; p1 < i+M; p1++)
            {
                for (p2 = k1; p2 <= k2; p2++)
                {
                    inSubMat[p2-k1][p1-i] = outMat[p2][p1];
                }
            }
            TriangleSolver(LU, inSubMat, outSubMat, subdist, 2);
      
            //update
            for (p1 = i; p1 < i+M; p1++)
            {
                for (p2 = k1; p2 <= k2; p2++)
                {
                    outMat[p2][p1] = outSubMat[p2-k1][p1-i]; 
                }
            }          
        }
    
             
_cleanup_space(_distributor, "123456");
_cleanup_space(_constructor, "123456");
    /* <token action="SET" idxset="(k1)"/> */
    sprintf(_tp_name, "token#%s", "123456");
    sprintf(_tp_token, "=(k1:%d~%d,%d:#%d)", i+M, N, M, M);
    _tp_size = sizeof(_tp_token);
    _tokens  = _set_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tokens < 0) exit(-1);


    /* <send var="i" type="int"/> */
    sprintf(_tp_name, "int:i#%s", "123456");
    _tp_size = sizeof(int);
    _tp_i_123456 = &i;
    _status  = _send_data(_distributor, _tp_name, (char *)_tp_i_123456, _tp_size);
    if (_status < 0) exit(-1);

 
             
    /* <send var="outMat" type="float[N(i~N)  ][N(i~N)  ]"/> */
    sprintf(_tp_name, "float(%d)(%d):outMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", (i), (N), 1, (i), (N), 1, sizeof(float));
    _tp_size = ((((N) - (i) - 1) / 1 + 1) * (((N) - (i) - 1) / 1 + 1)) * sizeof(float);
    _tp_outMat_123456 = (float *)malloc(_tp_size);
    for (_x0_123456 = (i), _y0_123456 =0; _x0_123456 < (N); _x0_123456 +=1, _y0_123456 ++) {
        for (_x1_123456 = (i), _y1_123456 =0; _x1_123456 < (N); _x1_123456 +=1, _y1_123456 ++) {

            _tp_outMat_123456[_y0_123456 * (((N) - (i) - 1) / 1 + 1) + _y1_123456] = outMat[_x0_123456][_x1_123456];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_outMat_123456, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_outMat_123456);

 

          

             
    /* <read var="outMat" type="float[N(i+M~N)  ][N(i+M~N)]"/> */
    sprintf(_tp_name, "float(%d)(%d):outMat#%s[%d~%d,%d][%d~%d,%d]@%d", (N), (N), "123456", (i+M), (N), 1, (i+M), (N), 1, sizeof(float));
    _tp_size = ((((N) - (i+M) - 1) / 1 + 1) * (((N) - (i+M) - 1) / 1 + 1)) * sizeof(float);
    _tp_outMat_123456 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_outMat_123456, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123456 = (i+M), _y0_123456 =0; _x0_123456 < (N); _x0_123456 +=1, _y0_123456 ++) {
        for (_x1_123456 = (i+M), _y1_123456 =0; _x1_123456 < (N); _x1_123456 +=1, _y1_123456 ++) {

            outMat[_x0_123456][_x1_123456] = _tp_outMat_123456[_y0_123456 * (((N) - (i+M) - 1) / 1 + 1) + _y1_123456];
        }
    }

    free(_tp_outMat_123456);

 
    }

     
_close_space(_constructor, "123456", 1);
_close_space(_distributor, "123456", 1);
/* </master> */
 
/*  
    for (i=0; i<N; i++)
    {
      for (j=0; j<N; j++)
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
 
