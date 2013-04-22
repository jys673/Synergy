/*----------------------------------------------------------------------*/
/* This program solves Poisson's (&Laplaces's) equations for a magnetic */
/* field, using iterative methods:                                      */
/*    1. Jacobi                                                         */
/*    2. Gauss-Seidel                                                   */
/*    3. BRA on Gauss-Seidel (Varying iterations)                       */
/*  Main module                                                         */
/*                                                                      */
/*  Parallel Version for Synergy 3.0                                    */
/*  Static Version                                                      */
/*                                                                      */
/*  This algorithm's version uses varying number of internal iteration  */
/*  to mark the data exchange between participating processors          */
/*                                                                      */
/*----------------------------------------------------------------------*/
#include "fieldcl.h"

FILE *fp, *outfd, *sfd;
void main(int argc, char **argv)
{
    int done, slvtype, i, j, itmp;
    char filenm[32];

/* Open Tuple Spaces */

    if  ( (tsinp=cnf_open("input",0)) < 0) {
        printf("Field WORKER: Error opening input Tuple space \n");
        exit(0);
    }
    if  ( (tsout=cnf_open("output",0)) < 0) {
        printf("Field WORKER: Error opening output Tuple space \n");
        exit(0);
    }
    if  ( (tsglb=cnf_open("global",0)) < 0) {
        printf("Field WORKER: Error opening output Tuple space \n");
        exit(0);
    }
    if  ( (tsrows=cnf_open("rows",0)) < 0) {
        printf("Field WORKER: Error opening output Tuple space \n");
        exit(0);
    }

    printf("Field Client: Tuple spaces opened succesfully \n");

    if  
((fp=fopen("field.cfg","r"))==NULL) {
        perror("Cannot open field.cfg:");
        exit(-1);
    }
    fscanf(fp, "%d %d ", &XDIS, &YDIS);
    fscanf(fp, "%d", &NRES);    
    fscanf(fp, "%d", &gmtype);
    fscanf(fp, "%d", &slvtype);
    fscanf(fp, "%d", &niter);
    fscanf(fp, "%d %d", &NPROC, &NTPL);
    fscanf(fp, "%lf", &size);
    fscanf(fp, "%lf", &magn );     
    fscanf(fp, "%lf %lf", &epscl, &thres );     
    fscanf(fp, "%lf %lf", &width, &length);    

    eps = epscl/10.0;
    sprintf(filenm,"ether.field%d.out", NRES); 
    if  
((outfd=fopen(filenm, "w"))==NULL) {
        perror("Cannot open field.out");
        exit(-1);
    }
    sprintf(filenm,"ether.field%d.stat", NRES); 
    if  
((sfd=fopen(filenm, "w"))==NULL) {
        perror("Cannot open field.stat");
        exit(-1);
    }
/*    fprintf(sfd, "Threshold \tTotalTime \tTuples \tRows 
\tTotalIterations \n"); */
    printf("\n\n\t\t\tMagnetic field \n");
    printf("----------------------------------------------------------------\n");
    printf("\tDisplay Resolution:\t %dx%d pixels\n", XDIS, YDIS);
    printf("\tField Resolution:\t %dx%d points \n", NRES, NRES);    
    printf("\tField Dimensions:\t %8.2fx%8.2f meters \n", size, size);
    printf("\tField Magnetic charge:\t %8.2f  \n", magn );     
    printf("\tField Solver type:\t %d  \tRelaxing iterations %d\n",
           slvtype, niter); 
    printf("\tNumber of Processors:\t %d \n", NPROC );     
    printf("\tNumber of Partitions (Tuples):\t %d \n", NTPL );     
    printf("\tMax Error (Epsilon):\t %10.8f \tThreshold %f\n", epscl, thres );     
    printf("\tMagnet's size:\t\t %8.2fx%8.2f \n", width, length);
    printf("----------------------------------------------------------------\n\n\n");
/*------------------------------------------------------------------------*/
/* Write it in the Stat File too                                          */
/*------------------------------------------------------------------------*/
    fprintf(sfd, "\n\n\t\t\tMagnetic field \n");
    fprintf(sfd, "----------------------------------------------------------------\n");
    fprintf(sfd, "\t\tIterative Model \n");
    fprintf(sfd, "\tDisplay Resolution:\t %dx%d pixels\n", XDIS, YDIS);
    fprintf(sfd, "\tField Resolution:\t %dx%d points \n", NRES, NRES);    
    fprintf(sfd, "\tField Dimensions:\t %8.2fx%8.2f meters \n", size, size);
    fprintf(sfd, "\tField Magnetic charge:\t %8.2f  \n", magn );     
    fprintf(sfd, "\tField Solver type:\t %d  \tRelaxing iterations %d\n",
           slvtype, niter); 
    fprintf(sfd, "\tNumber of Processors:\t %d \n", NPROC );     
    fprintf(sfd, "\tNumber of Partitions (Tuples):\t %d \n", NTPL );     
    fprintf(sfd, "\tMax Error (Epsilon):\t %10.8f \tThreshold %f\n", epscl, thres );     
    fprintf(sfd, "\tMagnet's size:\t\t %8.2fx%8.2f \n", width, length);
    fprintf(sfd, "----------------------------------------------------------------\n\n\n");


/*------------------------------------------------------------------------*/
/*  Initialise X11 and call the solver                                    */      
/*------------------------------------------------------------------------*/

/*    Init_X( XDIS, YDIS, 10, "Magnetic Field",argc, argv ); */

/*------------------------------------------------------------------------*/
/* NTPL (the number of Tuples) should be equal with the Number of         */
/* processors                                                             */
/*------------------------------------------------------------------------*/
    XRES = YRES = NRES;
    dx = size / (float)(XRES-1);
    dy = size / (float)(YRES-1);
    NROWS = XRES / NTPL;  /* chunk size */

    Solver(slvtype);

/***Display Block***
    for (i=0; i<YRES; i++) {
        for  (j=0; j<XRES; j++) {
            phinew[i][j] = log(1.0+ phiold[i][j]);
        }
    }

    ShowDbl(XRES, YRES, XDIS, YDIS, phinew); 

    done = 0;
    do {
       EventH(&done, XDIS, YDIS);
    } while (!done);
***end of Display Block***/

    for (i=0; i<NPROC; i++) {  
        sprintf(tpname,"asgn%d", i);
        tplength = sizeof(asgn); 
        asgn.command= -999;
        status = cnf_tsput(tsinp, tpname, (char *)&asgn, tplength);
    
        printf("Field Client: Succesful insertion of termination tuple  \n");
    }
    printf("Field CLIENT: Terminating \n");
    close(outfd);
    close(sfd);
}


void Setup( int gmtype, int il )
{
    if (gmtype==1) { /* Rectangular magnet */
        RectangGeom(il);
    }
/*  GeomPrint(); */
}

/*----------------------------------------------------------------------------*/
/* Solver is the main module so to speak.                                     */
/* It opens and closes the tuple spaces, and sends the geometry and misc      */
/* information to the workers. Then it calls function relax()  which sends    */
/* the individual work assignements                                           */
/*----------------------------------------------------------------------------*/
void Solver(int slvtype)
{
    int i, j, ind, iter;
    double t0, t1, dt, ttmp, itmp;

/*----------------------------------------------------------------------------*/
/* Initialise the maxerror Tuple. This is an array holding the maximum error  */
/* values for each block. A worker will write the error calculated in its     */
/* own assignement. Then if the max. of all the individual errors is less     */
/* than eps, then the wroker inserts the block (consindered to be calculated) */
/* in the output Tuple space, otherwise it continues iterating on it          */
/*----------------------------------------------------------------------------*/
    for  (i=0; i<=NTPL; i++) maxerr[i] = -666.0;
    strcpy(tpname, "maxerr");
    tplength = (NTPL+1)*sizeof(double);;
    status = cnf_tsput(tsglb, tpname, (char *)maxerr, tplength);

    printf("Field Client: Max Error insertion Successfull \n");
/*----------------------------------------------------------------------------*/
/* Setup geometry, Fill the original array and transmit geometry to the       */
/* tuple space.                                                               */
/*----------------------------------------------------------------------------*/
    ilevel = 0;   /* all except multigrid */
    Setup(gmtype, ilevel);

/***    ShowDbl(XRES, YRES, XDIS, YDIS, phiold); ***/
    printf("Setup is done \n");
    
    strcpy(tpname,"geomet");
    tplength = XRES*YRES*sizeof(char);
    ind = 0;
    for  (i=0; i<YRES; i++) {
        for (j=0; j<XRES; j++) {
            geombl[ind] = geom[i][j];
            ind++;
        }
    }

    status = cnf_tsput(tsglb, tpname, (char *)geombl, tplength);

    printf("Field Client: Geometry insertion Successfull \n");
    misc.XRES  = XRES;
    misc.YRES  = YRES;
    misc.NTPL  = NTPL;
    misc.NROWS = NROWS;
    misc.niter = niter;
    misc.magn  = magn;
    misc.dx    = dx;
    misc.eps   = eps;
    strcpy(tpname,"misc");
    tplength = sizeof(misc);
    status = cnf_tsput(tsglb, tpname, (char *)&misc, tplength);


    printf("Field Client: Misc. info insertion Successfull \n\n\n");
/*----------------------------------------------------------------------*/
/* Only GaSe Relaxation is being served in parallel                     */
/*                                                                      */
/* This program version uses varying number of internal iterations      */
/* This number is to be assigned on the asgn.niter field of the tuple   */
/*                                                                      */
/* Time data Collection for different number of iteration blocks        */
/*----------------------------------------------------------------------*/
/***    itmp = niter;
    for (niter = itmp; niter <= 3000; niter=niter+50) {
***/
    
/****
    ttmp = 0.05;
    for (thres=ttmp; thres>0.001; thres = thres-0.001) {
        asgn.thres = thres;
****/
    itmp = 25;
    for (niter = itmp; niter <= 30; niter=niter+5) {
        fill(); /* Initialise mesh with initial magnetic values */
        asgn.niter = niter;
        t0 = wall_clock();
        switch (slvtype) {
            case 3:
            printf("----------------------------------------------------------------\n");
            printf("Gauss-Seidel with a dataflow relaxation:\n");
            printf("Static Block asunchronous version\n");
            printf("Problem Size %dx%d \n", XRES, YRES);
            printf("Tuples %d,  dataflow, every %d iterations\n", NTPL, niter);
            printf("----------------------------------------------------------------\n");
            relax();
            break;
        }
        t1 = wall_clock();
        dt = (t1-t0)/1000000.0;
        fprintf(sfd, "InternIter %4d    Total-Time %8.2f    Reinserted %3d   ", 
                niter , dt, reinserted);
        ReceiveStats();
        fflush(outfd);
        fflush(sfd);
    }
}


/*----------------------------------------------------------------------------*/
/*  Now receive stats from all workers                                        */
/*  Print them in the log file                                                */
/*----------------------------------------------------------------------------*/
void ReceiveStats()
{
    int isum, i, j, iproc, maxsteps;
    char hname[80], junk[80];


/*----------------------------------------------------------------------------*/
/* Gather log data from all the prcessors and arrange them in the             */
/* global statistics table                                                    */
/*----------------------------------------------------------------------------*/
    maxsteps = 0;   
    for (iproc=0; iproc<NPROC; iproc++) { 
        strcpy(tpname,"stat*");
        status = cnf_tsget(tsout, tpname, (char *)&tplog[iproc], 0);
        if  ( tplog[iproc].ExtIter > maxsteps ) 
            maxsteps = tplog[iproc].ExtIter;    
    }
/*----------------------------------------------------------------------------*/
/* Now print the results                                                      */
/*----------------------------------------------------------------------------*/
    fprintf(sfd,"ExtIter(max) %5d \n", maxsteps);
    fprintf(sfd,"\t");
    for (i=0; i<NPROC; i++) {
        fprintf(sfd,"Tuple%d\t",i);
    }
    fprintf(sfd,"\n");  

    fprintf(sfd,"\t");
    for  (i=0; i<NPROC; i++) {
        isum = 0;
        for (j=0; j<tplog[i].ExtIter; j++) isum = isum + tplog[i].InternIter[j];
        fprintf(sfd, "%d\t", isum );
    }
    fprintf(sfd, "\n-------------------------------------------------------------------------\n");

    for  (i=0; i<maxsteps; i++) {
        fprintf(sfd, "%d\t", i);
        for  (j=0; j<NPROC; j++) {
            fprintf(sfd,"%d\t", tplog[j].InternIter[i]);     
        }
        fprintf(sfd,"\n");
    }
    fprintf(sfd,"\n\n\n");
    fflush(sfd);

}

void relax()
{
    int  ip, i,j,  ipr, ind;
    double maxdiff;
    char junk[80];
    int Rowsleft, TpRows, RowsIdx;
 
/*----------------------------------------------------------------------------*/
/* Fill the sub arrays and introduce padding                                  */
/* compose assignement and border row tuples and insert them                  */
/*----------------------------------------------------------------------------*/
    reinserted = 0;
    do {              /* Perform sanity check at the end of each cycle */
        reinserted ++;
/*----------------------------------------------------------------------------*/
/* Initialise the maxerror Tuple. This is an array holding the maximum error  */
/* values for each block. A worker will write the error calculated in its     */
/* own assignement. Then if the max. of all the individual errors is less     */
/* than eps, then the wroker inserts the block (consindered to be calculated) */
/* in the output Tuple space, otherwise it continues iterating on it          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
        for  (i=0; i<=NTPL; i++) maxerr[i] = 666.0;
        strcpy(tpname, "maxerr");
        tplength = (NTPL+1)*sizeof(double);;
        status = cnf_tsput(tsglb, tpname, (char *)maxerr, tplength);
/*----------------------------------------------------------------------------*/
/* Divide the rows so that last tuple will get all remaining rows, in case    */
/* division (Resolution/#of processors) is not exact                          */
/*----------------------------------------------------------------------------*/
/*        RowsLeft = XRES; */
        RowsIdx = 0;
        for  (ip=0; ip<NTPL; ip++){
            bllow = blhigh = -666;
            blstart = RowsIdx;
            if  ( ip!=(NTPL-1) ) {
                blstop = RowsIdx+NROWS - 1;
            } else {
                blstop = XRES - 1;
            }
            TpRows = blstop - blstart + 1;
            RowsIdx = RowsIdx + TpRows;

            PackBlock( blstart, TpRows );
            if (ip==0) {
                bllow = 0;
            } else {
                bllow = blstart - 1;
            }
            if (ip==(NTPL-1)) {
                blhigh = XRES-1;
            } else {
                blhigh = blstop + 1;
            }
            blindex = ip;  /* store block index safely here ...*/
            asgn.command = 1;  /* go */
            InsertAsgn();
        }

        for  (ip=0; ip<NTPL; ip++){
            strcpy(tpname, "res*");
            status = cnf_tsget(tsout, tpname, (char *)&result, 0);

            sscanf(tpname,"%s %d",junk, &ipr);
            printf("Field Client: Result  %s (%d) retieval Successfull \n",
                   tpname, ipr);
            ind = 0; 
/*---------------------------------------------------------------------------*/
/*  Insert the result into phinew and phiold blocks according to its index,  */
/*  which is derived by its name.                                            */
/*---------------------------------------------------------------------------*/
            for  (i=0; i<NROWS; i++) {
                for (j=0; j<XRES; j++) {
                    phinew[ipr*NROWS+i][j] = phiold[ipr*NROWS+i][j] = result.block[ind];
                    ind++;
                }
            }
        }
    } while (!GaSeOnce());

/*----------------------------------------------------------------------------*/
/* Send the Sleep tuple. 0 means work, -666 means Die                         */
/* Start counting from blindex, upwards for tuple names                       */
/*----------------------------------------------------------------------------*/
    for  (i=blindex+1; i<=blindex+NPROC; i++) { 
        asgn.command = -666;
        sprintf( tpname, "asgn%d",i );
        tplength = sizeof(asgn);
        status = cnf_tsput(tsinp, tpname, (char *)&asgn, tplength);
    }
    printf("Field Client: DIE command has been transmitted \n");

}


InsertAsgn() 
{
    int i;

    asgn.blindex  = blindex;
    asgn.bllow    = bllow; 
    asgn.blhigh   = blhigh;
    asgn.blstart  = blstart;
    asgn.blstop   = blstop;

    sprintf(tpname, "asgn%d", blindex);
    tplength = sizeof(asgn);
    status = cnf_tsput(tsinp, tpname, (char *)&asgn, tplength);

    for  (i=0; i<XRES; i++) {
         row[i+1] = phiold[blstart][i];
    }
    sprintf(tpname, "row%d", blstart);
    tplength = (XRES+1) * sizeof(double);
    status = cnf_tsput(tsrows, tpname, (char *)row, tplength);
    
    for  (i=0; i<XRES; i++) {
         row[i+1] = phiold[blstop][i];
    }
    sprintf(tpname, "row%d", blstop);
    tplength = (XRES+1) * sizeof(double);
    status = cnf_tsput(tsrows, tpname, (char *)row, tplength);

    printf("Client: Inserted block %d  StartRow %d   EndRow %d \n", blindex, 
           blstart, blstop);
}    

int GaSeOnce()
{
    int i,j, done;
    double  diff, maxdiff;
     
    done = 1;            /* suppose we are done */
    maxdiff = -1.0;
    for (i=0; i<YRES; i++) {
        for  (j=0; j<XRES; j++) {
            switch (geom[i][j]) {
            case 1:  /* Sides  */
                phinew[i][j] = (phinew[i][j-1] + phiold[i][j+1]) / 2.0;
                break;
            case 2: /* poles */
                phinew[i][j] = ( phiold[i+1][j] + phinew[i-1][j] +
                                 magn*dx ) / 2.0;
                break;
            case 3: /* Corners */
                phinew[i][j] = ( phiold[i+1][j] + phinew[i-1][j] +
                                 phinew[i][j-1] + phiold[i][j+1] +
                                 magn*dx ) / 4.0;
                break;
            case 4:
                meshGS(i,j);
                break;
            }
            diff = phinew[i][j] - phiold[i][j];
            if (fabs(diff) > maxdiff) {
                maxdiff=fabs(diff);
            }
            if (fabs(diff) > epscl ) done = 0;
        }   
    }        
    if (!(done)) {    /*store data to do one more iteration... */
        for (i=0; i<YRES; i++) {
            for  (j=0; j<XRES; j++) {
                phiold[i][j] = phinew[i][j];
            }
        }
    }  /* end if */
    fprintf(outfd,"Maximum error %f\t\t ",maxdiff);
    printf("CLIENT:Maximum error %f\t\t ",maxdiff);
    if (!done) {
        fprintf(outfd,"Resubmitting work \n");
        printf("Resubmitting work \n\n");
    } else {
        fprintf(outfd,"End of Cycle \n");
        printf("End of Cycle \n\n");
    }
    return(done);
}

void PackBlock( int blstart, int TpRows )
{
    int i, j, ind;

    ind = 0;
    for  (i=0; i<TpRows; i++) {
        for (j=0; j<XRES; j++) {
            asgn.block[ind] = phiold[blstart+i][j];
            ind ++;
        }
    }
}


void meshGS(int i, int j )
{
    int count;
    double sum;
    
    count = 0;
    sum = 0;
    if  ( (j-1) >= 0 ){
        sum = sum + phinew[i][j-1];
        count = count +1;
    }
    if  ( (i-1) >= 0 ){
        sum = sum + phinew[i-1][j];
        count = count +1;
    }
    if  ( (j+1) < XRES ){
        sum = sum + phiold[i][j+1];
        count = count +1;
    }
    if  ( (i+1) < YRES ){
        sum = sum + phiold[i+1][j];
        count = count +1;
    }
    phinew[i][j] = sum / count;
}

