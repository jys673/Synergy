#include "parallel.h"

/****************************************************************************/
/*                                                                          */
/* PROGRAM: lap_seq.c                                                       */
/*                                                                          */
/*   This is the sequential version of the Laplace equation algorithm. It   */
/*   solves the two-dimensional Laplace equation with prescribed boundary   */
/*   conditions on a single processor. Finite-difference method is used to  */
/*   discretize the problem domain. The resulting sparse linear system is   */
/*   solved by the Gauss-Seidal method.                                     */
/*                                                                          */
/*            (0,0)              TOPBOUND                     (0,DIM+1)     */
/*              ...|........................................|...            */
/*              .__|________________________________________|__.            */
/*           L  .  |                                        |  . R          */
/*           E  .  |                                        |  . I          */
/*           F  .  |                                        |  . G          */
/*           T  .  |                                        |  . H          */
/*           B  .  |                                        |  . T          */
/*           O  .  |           PROBLEM   DOMAIN             |  . B          */
/*           U  .  |              (DIM X DIM)               |  . O          */
/*           N  .  |                                        |  . U          */
/*           D  .  |                                        |  . N          */
/*              .  |                                        |  . D          */
/*              .  |                                        |  .            */
/*              .  |                                        |  .            */
/*              .--|----------------------------------------|--.            */
/*              ...|........................................|...            */
/*        (DIM+1,0)                BOTBOUND             (DIM+1,DIM+1)       */ 
/*                                                                          */
/*   The problem domain is (rdim X DIM), where rdim is the row dimension    */
/*   and DIM is the column dimension in general. Our domain has rdim=DIM.   */
/*   Successive Gauss-Seidal iterations refine the values of A. The         */
/*   program terminates iterations if either the global residual is less    */
/*   than the desired or MAXITER is reached.                                */
/*                                                                          */
/*                                                                          */
/* OUTPUT FILE: seq_time.dat                                                */
/*                                                                          */
/* LANGUAGE: C                                                              */
/*                                                                          */
/****************************************************************************/


 
/* <reference> */
 
# include <stdio.h>
# include <time.h>
# include <math.h>

# include "laplace.h"
 
/* </reference> */
 

void inita();                   /* initilizes A with initial guess */


/****************************************************************************/
/*** int main()                                                           ***/
/***                                                                      ***/
/*** This is the main function of lap_seq.c. It contains the iterative    ***/
/*** loop of the Gauss-Seidal method.                                     ***/ 
/****************************************************************************/

 
/* <parallel appname="lap"> */
double * _tp_A_234567;
double * _tp_resid_234567;
int _x0_234567;
int _x1_234567;
int _y0_234567;
int _y1_234567;
 

int main()
{
    FILE *fd;                 /* file descriptor */
    char host[128];           /* host name */
    long t0, t1;              /* start and end elapsed time */

    /* 
     * initialize 
     *
     */

    gethostname(host, sizeof(host));
    t0 = time ((long*)0);

    rdim = DIM;
    inita();

    /* 
     * This is the main iteration loop for the Gauss-Seidal method. Refinement
     * is done in successive iterations till either the global residual is
     * less than desired or MAXITER is reached.
     *
     */

     
/* <master id="234567"> */
_distributor = _open_space("distributor", 0, "234567");
_constructor = _open_space("constructor", 0, "234567");
 

    for (iterno= 1; iterno <= MAXITER; iterno++)
     {
         
_cleanup_space(_distributor, "234567");
_cleanup_space(_constructor, "234567");
    /* <send var="A" type="double[DIM+2     ][DIM+2     ]" opt="ONCE"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s", (DIM+2), (DIM+2), "234567");
    _tp_size = ((DIM+2) * (DIM+2)) * sizeof(double);
    _tp_A_234567 = (double *)malloc(_tp_size);
    for (_x0_234567 = 0; _x0_234567 < (DIM+2); _x0_234567 +=1) {
        for (_x1_234567 = 0; _x1_234567 < (DIM+2); _x1_234567 +=1) {

            _tp_A_234567[_x0_234567 * (DIM+2) + _x1_234567] = A[_x0_234567][_x1_234567];
        }
    }

printf("\n---------------------------------------\n");
    for (_x0_234567 = 0; _x0_234567 < (DIM+2); _x0_234567 +=1) {
        for (_x1_234567 = 0; _x1_234567 < (DIM+2); _x1_234567 +=1) {
printf("%8.1f, ", _tp_A_234567[_x0_234567 * (DIM+2) + _x1_234567]);
        }
printf("\n");
    }
printf("\n---------------------------------------\n");
    _status  = _send_data(_distributor, _tp_name, (char *)_tp_A_234567, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_234567);

 

         
    /* <token action="SET" idxset="(i)(j)"/> */
    sprintf(_tp_name, "token#%s", "234567");
    sprintf(_tp_token, "=(i:%d~%d,%d:#%d)(j:%d~%d,%d:#%d)", 1, rdim+1, 1, 2, 1, DIM+1, 1, 2);
    _tp_size = sizeof(_tp_token);
    _tokens  = _set_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tokens < 0) exit(-1);


    /* <send var="A" type="double[DIM+2(0    ~1    )][DIM+2(0    ~DIM+1)]" opt="XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (DIM+2), (DIM+2), "234567", (0    ), (1    ), 1, (0    ), (DIM+1), 1, sizeof(double));
    _tp_size = (((1    ) - (0    )) * ((DIM+1) - (0    ))) * sizeof(double);
    _tp_A_234567 = (double *)malloc(_tp_size);
    for (_x0_234567 = (0    ), _y0_234567 =0; _x0_234567 < (1    ); _x0_234567 +=1, _y0_234567 ++) {
        for (_x1_234567 = (0    ), _y1_234567 =0; _x1_234567 < (DIM+1); _x1_234567 +=1, _y1_234567 ++) {

            _tp_A_234567[_y0_234567 * ((DIM+1) - (0    )) + _y1_234567] = A[_x0_234567][_x1_234567];
        }
    }

printf("\n------1--------------------------------\n");
    for (_x0_234567 = (0    ), _y0_234567 =0; _x0_234567 < (1    ); _x0_234567 +=1, _y0_234567 ++) {
        for (_x1_234567 = (0    ), _y1_234567 =0; _x1_234567 < (DIM+1); _x1_234567 +=1, _y1_234567 ++) {
printf("%8.1f, ", _tp_A_234567[_y0_234567 * ((DIM+1) - (0    )) + _y1_234567]);
        }
printf("\n");
    }
printf("\nsend _tp_name (%s), _tp_size (%d)\n", _tp_name, _tp_size);
printf("\n---------------------------------------\n");
    _status  = _send_data(_constructor, _tp_name, (char *)_tp_A_234567, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_234567);

 
         
    /* <send var="A" type="double[DIM+2(0    ~DIM+1)][DIM+2(DIM+1~DIM+2)]" opt="XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (DIM+2), (DIM+2), "234567", (0    ), (DIM+1), 1, (DIM+1), (DIM+2), 1, sizeof(double));
    _tp_size = (((DIM+1) - (0    )) * ((DIM+2) - (DIM+1))) * sizeof(double);
    _tp_A_234567 = (double *)malloc(_tp_size);
    for (_x0_234567 = (0    ), _y0_234567 =0; _x0_234567 < (DIM+1); _x0_234567 +=1, _y0_234567 ++) {
        for (_x1_234567 = (DIM+1), _y1_234567 =0; _x1_234567 < (DIM+2); _x1_234567 +=1, _y1_234567 ++) {

            _tp_A_234567[_y0_234567 * ((DIM+2) - (DIM+1)) + _y1_234567] = A[_x0_234567][_x1_234567];
        }
    }

printf("\n------2--------------------------------\n");
    for (_x0_234567 = (0    ), _y0_234567 =0; _x0_234567 < (DIM+1); _x0_234567 +=1, _y0_234567 ++) {
        for (_x1_234567 = (DIM+1), _y1_234567 =0; _x1_234567 < (DIM+2); _x1_234567 +=1, _y1_234567 ++) {
printf("%8.1f, ", _tp_A_234567[_y0_234567 * ((DIM+2) - (DIM+1)) + _y1_234567]);
        }
printf("\n");
    }
printf("\nsend _tp_name (%s), _tp_size (%d)\n", _tp_name, _tp_size);
printf("\n---------------------------------------\n");
    _status  = _send_data(_constructor, _tp_name, (char *)_tp_A_234567, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_234567);

 
         
    /* <send var="A" type="double[DIM+2(DIM+1~DIM+2)][DIM+2(1    ~DIM+2)]" opt="XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (DIM+2), (DIM+2), "234567", (DIM+1), (DIM+2), 1, (1    ), (DIM+2), 1, sizeof(double));
    _tp_size = (((DIM+2) - (DIM+1)) * ((DIM+2) - (1    ))) * sizeof(double);
    _tp_A_234567 = (double *)malloc(_tp_size);
    for (_x0_234567 = (DIM+1), _y0_234567 =0; _x0_234567 < (DIM+2); _x0_234567 +=1, _y0_234567 ++) {
        for (_x1_234567 = (1    ), _y1_234567 =0; _x1_234567 < (DIM+2); _x1_234567 +=1, _y1_234567 ++) {

            _tp_A_234567[_y0_234567 * ((DIM+2) - (1    )) + _y1_234567] = A[_x0_234567][_x1_234567];
        }
    }

printf("\n------3--------------------------------\n");
    for (_x0_234567 = (DIM+1), _y0_234567 =0; _x0_234567 < (DIM+2); _x0_234567 +=1, _y0_234567 ++) {
        for (_x1_234567 = (1    ), _y1_234567 =0; _x1_234567 < (DIM+2); _x1_234567 +=1, _y1_234567 ++) {
printf("%8.1f, ", _tp_A_234567[_y0_234567 * ((DIM+2) - (1    )) + _y1_234567]);
        }
printf("\n");
    }
printf("\nsend _tp_name (%s), _tp_size (%d)\n", _tp_name, _tp_size);
printf("\n---------------------------------------\n");
    _status  = _send_data(_constructor, _tp_name, (char *)_tp_A_234567, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_234567);

 
         
    /* <send var="A" type="double[DIM+2(1    ~DIM+2)][DIM+2(0    ~1    )]" opt="XCHG"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s[%d~%d,%d][%d~%d,%d]@%d", (DIM+2), (DIM+2), "234567", (1    ), (DIM+2), 1, (0    ), (1    ), 1, sizeof(double));
    _tp_size = (((DIM+2) - (1    )) * ((1    ) - (0    ))) * sizeof(double);
    _tp_A_234567 = (double *)malloc(_tp_size);
    for (_x0_234567 = (1    ), _y0_234567 =0; _x0_234567 < (DIM+2); _x0_234567 +=1, _y0_234567 ++) {
        for (_x1_234567 = (0    ), _y1_234567 =0; _x1_234567 < (1    ); _x1_234567 +=1, _y1_234567 ++) {

            _tp_A_234567[_y0_234567 * ((1    ) - (0    )) + _y1_234567] = A[_x0_234567][_x1_234567];
        }
    }

printf("\n------4--------------------------------\n");
    for (_x0_234567 = (1    ), _y0_234567 =0; _x0_234567 < (DIM+2); _x0_234567 +=1, _y0_234567 ++) {
        for (_x1_234567 = (0    ), _y1_234567 =0; _x1_234567 < (1    ); _x1_234567 +=1, _y1_234567 ++) {
printf("%8.1f, ", _tp_A_234567[_y0_234567 * ((1    ) - (0    )) + _y1_234567]);
        }
printf("\n");
    }
printf("\nsend _tp_name (%s), _tp_size (%d)\n", _tp_name, _tp_size);
printf("\n---------------------------------------\n");
    _status  = _send_data(_constructor, _tp_name, (char *)_tp_A_234567, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_A_234567);

 

          

         
    /* <read var="A" type="double[DIM+2     ][DIM+2     ]"/> */
    sprintf(_tp_name, "double(%d)(%d):A#%s", (DIM+2), (DIM+2), "234567");
    _tp_size = ((DIM+2) * (DIM+2)) * sizeof(double);
    _tp_A_234567 = (double *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_A_234567, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_234567 = 0; _x0_234567 < (DIM+2); _x0_234567 +=1) {
        for (_x1_234567 = 0; _x1_234567 < (DIM+2); _x1_234567 +=1) {

            A[_x0_234567][_x1_234567] = _tp_A_234567[_x0_234567 * (DIM+2) + _x1_234567];
        }
    }

printf("\n---------------------------------------\n");
    for (_x0_234567 = 0; _x0_234567 < (DIM+2); _x0_234567 +=1) {
        for (_x1_234567 = 0; _x1_234567 < (DIM+2); _x1_234567 +=1) {
printf("%8.1f, ", _tp_A_234567[_x0_234567 * (DIM+2) + _x1_234567]);
        }
printf("\n");
    }
printf("\nread _tp_name (%s), _tp_size (%d)\n", _tp_name, _tp_size);
printf("\n---------------------------------------\n");
    free(_tp_A_234567);

 
         
    /* <read var="resid" type="double" opt="_MAX"/> */
    sprintf(_tp_name, "double:resid#%s?MAX@%d", "234567", _tokens);
    _tp_size = sizeof(double);
    _tp_resid_234567 = &resid;
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_resid_234567, _tp_size);
    if (_tp_size < 0) exit(-1);

 
 
printf ("\n>>>>>>> Current Iters = (%d), Currect Resid = (%f) <<<<<<<<\n", iterno, resid);

        if (DEBUG && iterno % 10 == 0)
            printf ("\nsequential(%d) iterations done\n",iterno);

        /* termination of iterations */
        if (resid < EPS) break;
     }
     
     
_close_space(_constructor, "234567", 1);
_close_space(_distributor, "234567", 1);
/* </master> */
 

    t1 = time((long *)0) - t0;

    printf ("\nTime: (%d), DIM: (%d), Iters/MAX: (%d/%d), Resid/EPS: (%f/%f)\n", 
             t1, DIM, iterno, MAXITER, resid, EPS);
    if (iterno >= MAXITER)
        printf("\nWARNING: Maximum iterations reached!\n");

    /* 
     * Output the solution 
     *
     */

    fd = fopen("seq_time.dat", "a");

    fprintf(fd, "\n(%s) Time = %d seconds. DIM: (%d) \n", host, t1, DIM);
    if (t1 > 0)
        fprintf(fd, "MFLOPS: (%f), Iters/MAX: (%d/%d), Resid/EPS: (%f/%f)\n",
                    (float)(DIM)*(DIM)*iterno*5.0/(t1)/1000000.0,
                     iterno, MAXITER, resid, EPS);
    else
        fprintf(fd, "MFLOPS: (--), Iters/MAX: (%d/%d), Resid/EPS: (%f/%f)\n",
                     iterno, MAXITER, resid, EPS);

    if (LOG)                  /* to output data to profile */
     {
        fprintf(fd,"\n");
        for (i=0; i<DIM+2; i++)
         {
            for (j=0; j<DIM+2; j++) 
             {
                fprintf(fd, "%8.1f, ",A[i][j]);
             }
            fprintf(fd,"\n");
         }
        fprintf (fd, "\n%s\n", "===============================");
     }
    fclose(fd);

    return 0;
}

 
/* </parallel> */
 


/****************************************************************************/
/*** void inita()                                                         ***/
/***                                                                      ***/
/*** This function initializes the work domain (rdim X DIM) by the user's ***/
/*** first guess (need not be zero). Boundary values are also set.        ***/
/****************************************************************************/

void inita()
{
    for (i = 0; i < rdim+2; i++)     /* initial guess */
        for (j = 0; j < DIM+2; j++)
            A[i][j]  = 0.0;

    for (i = 0; i < rdim+2; i++)     /* set boundaries */
     {
        A[i][0]      = LFTBOUND;
        A[i][DIM+1]  = RITBOUND;
     }

    for (j = 0; j < DIM+2; j++)
        A[0][j]      = TOPBOUND;

    for (j = 0; j < DIM+2; j++)
        A[rdim+1][j] = BOTBOUND;
}
