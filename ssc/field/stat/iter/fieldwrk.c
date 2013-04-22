/*----------------------------------------------------------------------*/
/* This program solves Poisson's (&Laplaces's) equations for a magnetic */
/* field, using iterative methods:                                      */
/*    3. BRA on Gauss-Seidel                                            */
/*       The worker module exchanges row information when the residual  */
/*       falls under a certain threshold                                */
/* Worker module                                                        */
/*                                                                      */
/* Static Version                                                       */
/*                                                                      */
/* This version of the algorithm uses varying number of internal        */
/* iterations                                                           */
/*----------------------------------------------------------------------*/
#include "fieldwrk.h"
FILE *logfd;

main()
{
    int dead, calculated, done, iter, extiter, i, j, ind, new;
    double err;
    char logfname[80], junk[80];
    FILE *pfd;

/*----------------------------------------------------------------------*/
/* Open Tuple Spaces                                                    */
/*----------------------------------------------------------------------*/
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
    printf("Field Worker: Tuple spaces opened succesfully \n");


/*----------------------------------------------------------------------------*/
/* Learn your name from the system, to be used for the statistics tuple       */
/* name                                                                       */
/*----------------------------------------------------------------------------*/
    pfd = popen("hostname", "r");
    fscanf(pfd, "%s", junk);
    i=0;                       /* strip the domain */
    while ((junk[i]!='.')&&(i<strlen(junk))){
        hname[i] = junk[i];
        i++;
    }
    hname[i] = 0;

    printf("Host name is [%s] \n", hname);

    sprintf(logfname,"%s.log", hname);
    pclose(pfd);

    if  ((logfd = fopen(logfname,"w"))==NULL){
        perror("Cannot open log file");
    }

/*----------------------------------------------------------------------------*/
/* Receive misc info                                                          */
/*----------------------------------------------------------------------------*/
    strcpy(tpname,"misc");
    status = cnf_tsread(tsglb, tpname, (char *)&misc, 0);

    printf("Field Worker: Misc info. received succesfully \n\n\n");
    XRES   = misc.XRES;
    YRES   = misc.YRES;
    NTPL   = misc.NTPL;
    NROWS  = misc.NROWS;
    niter  = misc.niter;
    magn   = misc.magn;
    dx     = misc.dx;
    eps    = misc.eps;  
    ilevel = 0;
/*----------------------------------------------------------------------------*/
/* read in geometry info for the whole field because worker might get         */
/* different regions to calculate and we don't want to read subblocks of      */
/* geometry with every tuple to be calculated                                 */
/*----------------------------------------------------------------------------*/
    strcpy(tpname,"geomet");
    status = cnf_tsread(tsglb, tpname, (char *)geombl, 0);
    printf("Field Worker: Geometry received succesfully \n");
    ind = 0;
    for  (i=0; i<YRES; i++) {
        for (j=0; j<XRES; j++) {
            geom[i][j] = geombl[ind];
            ind++;
        }
    }

    dead = 0;
    new = 1;
/*
    printf("Node Block External UpperExt LowerExt Internal GlobalErr Starting Ending \n");
*/
/*
    fprintf(logfd, 
    "Node Block External UpperExt LowerExt Internal GlobalErr Starting Ending \n");
*/
    while (!dead) {
        if  ( new ) {
            GetAssign();
            printf("Recieved Assignment %d     Command %d \n", 
                   asgn.blindex, asgn.command);
            new = 0;
        }
        if  ( asgn.command == -666 ) { /* Done with this run */  
            done = 1;
            new = 1;
            SubmitStats();
            sleep(20);
            tplog.TotalIntern = 0;   /* Initialize log tuple */
            tplog.ExtIter = 0;
        } else if ( asgn.command == -999 ) {  /* All runs are done exit now */
             dead = 1;
        } else {
            err = 0;
            iter = 0;
            GetBorder();
            GaSeI( &calculated, &err, &iter );
            tplog.ExtIter++;
            tplog.TotalIntern = tplog.TotalIntern + iter;
            if ( tplog.ExtIter<MAXEXT ) tplog.InternIter[tplog.ExtIter] = iter;
/*
            printf("%s: %2d  %4d %4d %4d ",
                    hname, blindex, tplog.ExtIter, border.up, border.down);
*/
/*
            fprintf(logfd, "%s: %2d %4d %4d %4d  ",
                    hname, blindex, tplog.ExtIter, border.up, border.down);
*/
            SubmitBorder(); 
            if  ( SystemConverged(err) ) { 
                OutputBlock();
                new = 1;
            }
/* put the print statement here because maxerr was read in SystemConverged() */
/****
            printf("%4d %13.9f %13.9f %13.9f \n",
                    iter, maxerr[0], maxdiff, err);
            fprintf(logfd, "%4d %13.9f %13.9f %13.9f\n",
                    iter, maxerr[0], maxdiff, err);
****/
        }
    }  /* dead */
    
/*----------------------------------------------------------------------------*/
/* Close and terminate Tuple spaces                                           */
/*----------------------------------------------------------------------------*/
    printf("Field WORKER: Terminating \n");
    cnf_term();

}

/*----------------------------------------------------------------------------*/
/* send both statistics and internal iterations tuple                         */
/*----------------------------------------------------------------------------*/
void SubmitStats()
{

    sprintf(tpname, "stat %s", hname);
    tplength = sizeof(struct _log);
    status = cnf_tsput(tsout, tpname, (char *)&tplog, tplength);
    if (status>=0) {
        printf("Statistics log tuple had been inserted as [%s] %d\n",
               tpname, status);
    }

}

/* Just get maxerr to update the array and the maximum norm in the */
/* calculations */
void GetMaxerror()
{
    strcpy( tpname, "maxerr");
    status = cnf_tsget(tsglb, tpname, (char *)maxerr, 0);

    maxnorm=maxerr[0];
    tplength = (NTPL+1)*sizeof(double);
    status = cnf_tsput(tsglb, tpname, (char *)maxerr, tplength); 
}


int SystemConverged( double err )
{
    double maxeps;
    int i;


/*--------------------------------------------------------------------------*/
/* update the maximum local error entry. Then update maximum global entry   */
/* maxerr[0] is the global entry, maxerr[blindex+1] is the local entry      */
/* Because this tuple might had produced the maxerror and now it has a new  */
/* value                                                                    */
/*--------------------------------------------------------------------------*/
    strcpy( tpname, "maxerr");
    status = cnf_tsget(tsglb, tpname, (char *)maxerr, 0);
    maxerr[blindex+1] = err;
/* find the maximum among the locals and insert it */
    maxeps = maxerr[1];
    for  (i=2; i<NTPL+1; i++) {
        if (maxerr[i] > maxeps) maxeps = maxerr[i];
    }
    maxerr[0] = maxeps;

    tplength = (NTPL+1)*sizeof(double);
    status = cnf_tsput(tsglb, tpname, (char *)maxerr, tplength);
/*
    printf("Field Worker: Max Error insertion Successfull \n"); 
*/
    if ( maxeps < eps ) {
        return(1);         /* Yes it did! Insert block to output TS */
    } else {
        return(0);         /* Sorry.. leave this tuple and get a new one */
    }
}

void ResubmitBlock()
{
    int i;
    
    asgn.blindex   = blindex;
    asgn.bllow     = bllow;
    asgn.blhigh    = blhigh;
    asgn.blstart   = blstart;
    asgn.blstop    = blstop;
    sprintf(tpname, "asgn%d", blindex);
    tplength = sizeof(asgn);
    PackBlock();
    status = cnf_tsput(tsinp, tpname, (char *)&asgn, tplength);
    
    row[0] = tplog.ExtIter;
    for  (i=0; i<XRES; i++) {
         row[i+1] = phioldsm[1][i];
    }
    sprintf(tpname, "row%d", blstart);
    tplength = (XRES+1) * sizeof(double);
    status = cnf_tsput(tsrows, tpname, (char *)row, tplength);
    
    row[0] = tplog.ExtIter;
    for  (i=0; i<XRES; i++) {
         row[i+1] = phioldsm[NROWS][i];
    }
    sprintf(tpname, "row%d", blstop);
    tplength = (XRES+1) * sizeof(double);
    status = cnf_tsput(tsrows, tpname, (char *)row, tplength);
    
}

/*---------------------------------------------------------------------------*/
/* Get an assigment and its corresponding block                              */
/*---------------------------------------------------------------------------*/
void GetAssign()
{

    strcpy(tpname,"asg*");
    status = cnf_tsget(tsinp, tpname, (char *)&asgn, 0);

    blindex  = asgn.blindex;
    bllow    = asgn.bllow;
    blhigh   = asgn.blhigh;
    blstart  = asgn.blstart;
    blstop   = asgn.blstop;
    thres    = asgn.thres;
    niter    = asgn.niter;

    if  ( asgn.command == 1 ) {
        UnpackBlock();
        NROWS = blstop - blstart + 1;
    }
}
    
/*---------------------------------------------------------------------------*/
/* READ ROWS                                                                 */
/*  high and low                                                             */
/*---------------------------------------------------------------------------*/
void GetBorder()
{
    int i;

    sprintf(tpname, "row%d", bllow);
    status = cnf_tsread(tsrows, tpname, (char *)row, 0);
    for  (i=0; i<XRES; i++) {
        phioldsm[0][i] = row[i+1];
    }
    border.down = (int)(row[0]+0.0005);
 
    sprintf(tpname, "row%d", blhigh);
    status = cnf_tsread(tsrows, tpname, (char *)row, 0);
    for  (i=0; i<XRES; i++) {
        phioldsm[NROWS+1][i] = row[i+1];
    }
    border.up = (int)(row[0]+0.0005);
}


/*---------------------------------------------------------------------------*/
/* Insert a results block in the output TS                                   */
/*---------------------------------------------------------------------------*/
void OutputBlock()
{
    int i;

    sprintf(tpname,"res %d", blindex);
    tplength = sizeof(asgn);
    PackBlock();
    status = cnf_tsput(tsout, tpname, (char *)&asgn, tplength);
/***
    printf("Field Worker:Result block %d was inserted succesfully\n",  blindex);
***/
}

void SubmitBorder()
{
    int i;

    row[0] = tplog.ExtIter;
    for  (i=0; i<XRES; i++) {
         row[i+1] = phioldsm[1][i];
    }
    sprintf(tpname, "row%d", blstart);
    tplength = (XRES+1) * sizeof(double);
    status = cnf_tsput(tsrows, tpname, (char *)row, tplength);
    
    row[0] = tplog.ExtIter;
    for  (i=0; i<XRES; i++) {
         row[i+1] = phioldsm[NROWS][i];
    }
    sprintf(tpname, "row%d", blstop);
    tplength = (XRES+1) * sizeof(double);
    status = cnf_tsput(tsrows, tpname, (char *)row, tplength);
}



void GaSeI( int *done, double *err, int *iter)
{
    int ia, ib, i,j, it, ip, icnt;
    double  diff, tsh, avgdiff, localmax, sum;
    int first;

    ip = blindex;

    *done = 0;   
    it = 0;
    ia = 1; 
    ib = NROWS;
    if  (it==0) { 
        ia = 1;
        ib = NROWS;
    }
    if  (it==NTPL) {
        ia = 1;
        ib = NROWS;
    }
    for (i=0; i<NROWS+2; i++) { 
        for  (j=0; j<XRES; j++) {
            phinewsm[i][j] = phioldsm[i][j];
        }
    }
    first = 1;
    tsh = eps;   /* to get us in the loop */
    localmax = tsh+1; /*sentinel NOT USED in this version anyway */

/***    while ((!*done)&&(localmax>tsh)) {  ***/
    while ((!*done)&&(*iter < niter)) {
        *done = 1;            /* suppose we are done */
        localmax = -1.0;
        sum = 0.0;
        icnt = 0;
        for (i=ia; i<=ib; i++) {
            for  (j=0; j<XRES; j++) {
                icnt++;
                switch (geom[blstart+(i-1)][j]) {
                case 1:  /* Sides  */
                    phinewsm[i][j] = (phinewsm[i][j-1] + 
                                      phioldsm[i][j+1]) / 2.0;
                    break;
                case 2: /* poles */
                    phinewsm[i][j] = ( phioldsm[i+1][j] + 
                             phinewsm[i-1][j] + magn*dx ) / 2.0;
                    break;
                case 3: /* Corners */
                    phinewsm[i][j] = ( phioldsm[i+1][j] + 
                               phinewsm[i-1][j] + phinewsm[i][j-1] + 
                               phioldsm[i][j+1] + magn*dx ) / 4.0;
                    break;
                case 4:
                    meshGSR(i,j); 
                    break;
                }
                diff = phinewsm[i][j] - phioldsm[i][j];
                sum = sum + fabs(diff);
                if (fabs(diff) > localmax) localmax=fabs(diff);
                if (fabs(diff) > eps ) *done = 0;
             }
        }
        *err = localmax;
        avgdiff = sum / (double)icnt;
        if (!(*done)) {    /*store data to do one more iteration... */
            if  (first) {
                first = 0;
                tsh = thres*localmax;  /* Not used in this version */
                maxdiff = localmax;
            }
            for (i=0; i<NROWS+2; i++) { 
                for  (j=0; j<XRES; j++) {
                    phioldsm[i][j] = phinewsm[i][j];
                }
            }
        }  /* end if */
        it = it + 1;
        *iter = *iter + 1; /* Accounting */
    }   /* end while */
}


void meshGSR(int i, int j )
{
    int count;
    double sum;
    
    count = 0;
    sum = 0;
    if  ( (j-1) >= 0 ){
        sum = sum + phinewsm[i][j-1];
        count = count +1;
    }
    if  ( (i-1) >= 0 ){
        sum = sum + phinewsm[i-1][j];
        count = count +1;
    }
    if  ( (j+1) < XRES ){
        sum = sum + phioldsm[i][j+1];
        count = count +1;
    }
    if  ( (i+1) <= NROWS+1 ){
        sum = sum + phioldsm[i+1][j];
        count = count +1;
    }
    phinewsm[i][j] = sum / count;
}

void PackBlock()
{   
    int i, j, ind;
    
    ind = 0;
    for  (i=0; i<NROWS; i++) {
        for (j=0; j<XRES; j++) {
            asgn.block[ind] = phioldsm[i+1][j];
            ind ++;
        }
    }
}
                                 
void UnpackBlock()
{
    int i, j, ind;
                                 
    ind = 0;
    for  (i=0; i<NROWS; i++) {
        for (j=0; j<XRES; j++) {
            phioldsm[i+1][j] = asgn.block[ind];
            ind ++;   
        }
    }
}

