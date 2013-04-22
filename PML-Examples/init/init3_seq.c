/*
 *- Partitioning On Both Dimmensions
 */

/* <reference> */
/* Constant, N, and the grain size, G, are defined in the header file */
#include “init.h”
/* </reference> */

/* <parallel appname="init3"> */
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
    /* <send var="A" type="double[N(i)][N(j)]"/> */

    /* <worker> */
    /* <read var="v" type="double" opt="ONCE"/> */
    /* <read var="A" type="double[N(i)][N(j)]"/> */

    /* <target index="i" limits="(0,N,1)" chunk="G" order="1"> */
    for (i = 0; i < N; i++)
    /* </target> */
    {
        /* <target index="j" limits="(0,N,1)" chunk="G" order="2"> */
        for (j = 0; j < N; j++)
        /* </target> */
        {
            A[i, j] = v;
        }
    }

    /* <send var="A" type="double[N(i)][N(j)]"/> */
    /* </worker> */

    /* <read var="A" type="double[N(i)][N(j)]"/> */
    /* </master> */
}
/* </parallel> */

