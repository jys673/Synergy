/* <reference> */
#include "blu.h"
/* </reference> */

/* <parallel appname="blu"> */

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

    for (k=0; k<N-1; k++)
    {
            /* <send var="k"  type="int"/> */
            /* <send var="MainMat" type="float[N(k~N)  ][N(k~N)  ]"/> */

        /* <worker> */
            /* <read var="k"  type="int"/> */

            /* <read var="MainMat" type="float[N(k~k+1)][N(k~N)  ]"/> */
            /* <read var="MainMat" type="float[N(row)  ][N(k~N)  ]"/> */

        /* <target index="row" limits="(k+1,N,1)" chunk="G" order="1"> */
        for (row=k+1; row<N; row++)
        /* </target> */
        {
            MainMat[row][k] = MainMat[row][k] / MainMat[k][k];
            for (col=k+1; col<N; col++)
            {
                MainMat[row][col] = MainMat[row][col] - MainMat[row][k] * MainMat[k][col];
            }
        }

            /* <send var="MainMat" type="float[N(row)  ][N(k~N)  ]"/> */
        /* </worker> */

            /* <read var="MainMat" type="float[N(k+1~N)][N(k~N)  ]"/> */
    }

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
