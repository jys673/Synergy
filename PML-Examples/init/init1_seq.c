/*
 *- Partitioning Horizontally
 */

/* <reference> */
/* Constant, N, and the grain size, G, are defined in the header file */
#include “init.h”
/* </reference> */

/* <parallel appname="init1"> */
int main()
{
    /* <reference> */
    double A[N][N];
    double v;
    int i, j;
    /* </reference> */

    v = 99.9;

    /* <master id="123"> */
    /* <send var="v" type="double" opt="ONCE"/> */
    /* <send var="A" type="double[N(i)][N]"/> */

    /* <worker> */
    /* <read var="v" type="double" opt="ONCE"/> */
    /* <read var="A" type="double[N(i)][N]"/> */
			
    /* <target index="i" limits="(0,N,1)" chunk="G" order="1"> */
    for (i = 0; i < N; i++)
    /* </target> */
    {
        for (j = 0; j < N; j++)
        {
            A[i, j] = W;
        }
    }

    /* <send var="A" type="double[N(i)][N]"/> */
    /* </worker> */

    /* <read var="A" type="double[N(i)][N]"/> */
    /* </master> */
}
/* </parallel> */

