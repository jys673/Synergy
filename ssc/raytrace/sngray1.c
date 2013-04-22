/*          
   Program: sngray1.c

   This is the parallel version of POV ray tracing program
   with fixed chunk. The chunk size will be defined in the
   CSL file.  

   C FU
*/
   



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "syn-ray.h"
#include "syn-ray.c"

#define YRES_MAX 1024
#define SNG_REC "SNG_REC"


int main()
{
 long t0,t1;
 int i,j,k;
 int XRES,YRES;
 int CHUNK;
 JOB_UNIT X;
 unsigned char *data[YRES_MAX];

 FILE *fp;
 int JOB_NO,received; 
 int status, job_tuple, tpnamez, tplength;
 int output_tuple;
 int SIZE;
 int MARK,R;
 char tpname[128];


 /* get the job range from 'povray.def' */
 getjob(&XRES,&YRES); 
 printf("\n %d,   %d,  ",XRES,YRES);

 for(i=1;i<=YRES;i++)
 { 
 data[i] = malloc((2+3*XRES)*sizeof(char));
 if (!data[i]) {
             printf("\n Memory alloc error at YRES %d.",i);
             OUT;
             exit(0);
            }
 } 

 job_tuple=cnf_open("job",0);
 output_tuple=cnf_open("output",0);
 CHUNK=cnf_getf();

 printf("\n Master Begin: with CHUNK size %d\n",CHUNK);
 OUT;

 time(&t0);

 /* assign jobs */

 JOB_NO=0;
 MARK=0;
 for (i=1;i<=YRES;i+=CHUNK)
 {
  JOB_NO++;
  X.y1=i;   X.y2=R=i+CHUNK-1;
  if (R>YRES) {
                 MARK=1;
                 break;
               }
 
  sprintf(tpname,"i%d\0",i);
  status = cnf_tsput(job_tuple,tpname,X,sizeof(JOB_UNIT));
  fflush(stdout);
  }

 if (MARK!=0) 
 {
      X.y1=i; X.y2=YRES;
      sprintf(tpname,"i%d\0",i);
      status = cnf_tsput(job_tuple,tpname,X,sizeof(JOB_UNIT));
      fflush(stdout);
  }
 /* ---- put the END tuple to job_tuple space ---- */
 sprintf(tpname,"i%d\0",0);

 X.y1=TERM;  /* end mark! */

 status= cnf_tsput(job_tuple,tpname,X,sizeof(JOB_UNIT));
 printf("\n\n --------------------------------------------- ");
 printf("\n Master wait for outputs, Should be %d JOBS:",XRES);
 printf("\n ----------------------------------------------\n");
 OUT;



/*************************************************************/
/*        Receive the output here!!                          */
/*************************************************************/

 received=0;

/* open the file to store the tracing result */
 fp=fopen("/home/yoda/cfu/synray/povray.out","w");
 if (!fp) {
              printf("\n output file error!");
               exit(0);
           }
 data[1][0]=XRES/256;           /* RESOULTIONS of the image */
 data[1][1]=XRES%256;
 data[1][2]=YRES/256;
 data[1][3]=YRES%256;

 for(i=0;i<=3;i++) fputc(data[1][i],fp);
 fflush(fp);  

 while( received < YRES)
 {
 strcpy(tpname,"*");
 status=cnf_tsget(output_tuple,tpname,data[1],0);
 received++;
 
 printf("\n-->  %d/%d Master get: %d, ",received,YRES,data[1][0]*256+data[1][1]);
 printf("(%d Bytes) ",status);
 OUT;
 for (i=0;i<status;i++)
 putc(data[1][i],fp);
 }
 fclose(fp);
 cnf_close(job_tuple);
 cnf_close(output_tuple);
 time(&t1);
 printf("\n\n\n\n ----- Spent %ld sec  with CHUNK %d ----- \n\n\n",t1-t0,CHUNK);
 OUT;
 fp=fopen(SNG_REC,"a");
 fprintf(fp,"\n XRES: %4d, YRES: %4d, CHUNK: %3d, TIME: %5d",
         XRES,YRES,CHUNK,t1-t0);
 fclose(fp);
 cnf_term();
 return 1;
}




