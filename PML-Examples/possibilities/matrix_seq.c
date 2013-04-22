/* <reference> */
#include <stdio.h>
/* </reference> */

/* <reference id="123456"> */
#define N 100
/* </reference> */

/* <parallel appname="matrix"> */

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
        /* <send var="scalar" type="int" opt="ONCE|XCHG"/> */
        /* <send var="scalar" type="int" opt="ONCE"/> */

        /* <send var="A" type="double[N        ][N        ]" opt="ONCE|XCHG"/> */
        /* <send var="A" type="double[N(i:i-1) ][N        ]" opt="ONCE|XCHG"/> */
        /* <send var="A" type="double[N        ][N(k)     ]" opt="ONCE"/> */
        /* <send var="A" type="double[N(i)     ][N(k:$H-1)]" opt="ONCE"/> */

        /* <send var="scalar" type="int" opt="XCHG"/> */
        /* <send var="scalar" type="int"/> */

        /* <send var="B" type="double[N        ][N        ]"/> */
        /* <send var="B" type="double[N(i)     ][N        ]"/> */
        /* <send var="B" type="double[N        ][N(k:k-1) ]" opt="XCHG"/> */
        /* <send var="B" type="double[N(i:$L-1)][N(k)     ]" opt="XCHG"/> */

    /* <worker> */

        /* <read var="scalar" type="int" opt="ONCE|XCHG"/> */
        /* <read var="scalar" type="int" opt="ONCE"/> */

        /* <read var="A" type="double[N        ][N        ]" opt="ONCE|XCHG"/> */
        /* <read var="A" type="double[N(i:i-1) ][N        ]" opt="ONCE|XCHG"/> */
        /* <read var="A" type="double[N        ][N(k)     ]" opt="ONCE"/> */
        /* <read var="A" type="double[N(i)     ][N(k:$H-1)]" opt="ONCE"/> */

        /* <read var="scalar" type="int" opt="XCHG"/> */
        /* <read var="scalar" type="int"/> */

        /* <read var="B" type="double[N        ][N        ]"/> */
        /* <read var="B" type="double[N(i)     ][N        ]"/> */
        /* <read var="B" type="double[N        ][N(k:k-1) ]" opt="XCHG"/> */
        /* <read var="B" type="double[N(i:$L-1)][N(k)     ]" opt="XCHG"/> */

    scalar = 9;

    /* <target index="i" limits="(0,N,1)" chunk="100" order="1"> */
    for (i = 0; i < N; i++)
    /* </target> */
    {
        /* <target index="k" limits="(0,N,1)" jump="20" order="1"> */
        for (k = 0; k < N; k++)
        /* </target> */
        {
            for (j = 0; j < N; j++)
            {
                C[i][j] = C[i][j] + B[i][k]*A[k][j];
            }
        }
    }

        /* <send var="C" type="double[N        ][N        ]"/> */
        /* <send var="C" type="double[N(i:i-1) ][N        ]" opt="XCHG"/> */
        /* <send var="C" type="double[N        ][N(k:k-1) ]" opt="XCHG"/> */
        /* <send var="C" type="double[N(i:$H-1)][N(k:$L-1)]"/> */

        /* <send var="scalar" type="int" opt="_MAX|XCHG"/> */
        /* <send var="scalar" type="int" opt="_MIN"/> */

    /* </worker> */

        /* <read var="C" type="double[N        ][N        ]"/> */
        /* <read var="C" type="double[N(i:i-1) ][N        ]" opt="XCHG"/> */
        /* <read var="C" type="double[N        ][N(k:k-1) ]" opt="XCHG"/> */
        /* <read var="C" type="double[N(i:$H-1)][N(k:$L-1)]"/> */

        /* <read var="scalar" type="int" opt="_MAX|XCHG"/> */
        /* <read var="scalar" type="int" opt="_MIN"/> */

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
