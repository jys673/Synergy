/* <reference> */
#include "matrix.h"
/* </reference> */

/* <parallel appname="matrix"> */

main(int argc, char **argv[])
{
/* <reference id="123456"> */
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
        /* <send var="B" type="double[N   ][N   ]" opt="ONCE"/> */
        /* <send var="A" type="double[N   ][N   ]"/>*/

        /* <worker> */
            /* <read var="B" type="double[N   ][N   ]" opt="ONCE"/> */
            /* <read var="A" type="double[N(i)][N   ]"/> */

            /* <target index="i" limits="(0,N,1)" chunk="G" order="1"> */
            for (i = 0; i < N; i++)
            /* </target> */
            {
                for (k = 0; k < N; k++)
                {
                    for (j = 0; j < N; j++)
                    {
                        C[i][j] += A[i][k]*B[k][j];
                    }
                }
            }

            /* <send var="C" type="double[N(i)][N   ]"/> */
        /* </worker> */

        /* <read var="C" type="double[N   ][N   ]"/> */
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
