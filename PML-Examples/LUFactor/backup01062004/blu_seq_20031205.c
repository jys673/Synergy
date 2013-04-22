/* <reference> */
#include "blu.h"
/* </reference> */

/* <parallel appname="blu"> */

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
            /* <send var="i"  type="int"/> */
            /* <send var="LU"     type="float[M   ][M   ]"/> */
            /* <send var="outMat" type="float[N   ][N   ]"/> */

        /* <worker> */
            /* <read var="i"  type="int"/> */
            /* <read var="LU"     type="float[M   ][M   ]"/> */
            /* <read var="outMat" type="float[N   ][N   ]"/> */

        /* Solve triangles, LZ and WU */

        /* <target index="k1" limits="(j+1,N,1)" chunk="M" order="1"> */
        for (k1 = j+1; k1 < N; k1 = k1+M)
        /* </target> */
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

            /* <send var="outMat" type="float[N(i~i+M)][N(k1)   ]"/> */
            /* <send var="outMat" type="float[N(k1)   ][N(i~i+M)]"/> */
        /* </worker> */

            /* <read var="outMat" type="float[N(i~i+M)][N(j+1~N)]"/> */
            /* <read var="outMat" type="float[N(j+1~N)][N(i~i+M)]"/> */
        /* </master> */

        /* <master id="234567"> */
            /* <send var="j"  type="int"/> */
            /* <send var="outMat" type="float[N   ][N   ]"/> */

        /* <worker> */
            /* <read var="j"  type="int"/> */
            /* <read var="outMat" type="float[N   ][N   ]"/> */

        /* A = A - WZ */

        /* <target index="k1" limits="(j+1,N,1)" chunk="M" order="1"> */
        for (k1 = j+1; k1 < N; k1 = k1+M)
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
        /* </worker> */

            /* <read var="outMat" type="float[N(j+1~N)][N(j~j+M)]"/> */
        /* </master> */

        i = i + M;
    }
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
