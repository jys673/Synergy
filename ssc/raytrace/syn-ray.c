
/*
   syn-ray.c

   This program provide the function to
   read in the jobs informations.

   C FU

*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define OUT fflush(stdout)
#define STR_MAX 128

int parse_value(char BUF[STR_MAX],int,char,char);


getjob(int *x,int *y)
{
 FILE *fp;
 char BUF[STR_MAX];

 int i=0,j;
 int k,l,m;

 /* I define the input file will always be this one */

 fp=fopen("/home/yoda/cfu/synray/povray.def","r");

 if (!fp) { printf("\n FILE ERR!"); exit(1); }
 while(!feof(fp))
 {
 BUF[i++]= fgetc(fp);
 }
 fclose(fp); 
 
 (*x)=parse_value(BUF,i,'W','w');
 (*y)=parse_value(BUF,i,'H','h');

}

int parse_value(char BUF[STR_MAX],int length,char ex_l, char ex_u)
{
  int j,k,l,m;
 for (j=1;j<length;j++)
 {
  if ((BUF[j]== '+') || (BUF[j]=='-'))
  {
    if ((BUF[j+1]==ex_l) || (BUF[j+1]==ex_u))
    {
      j+=2;
      k=l=0;
      while(BUF[j]>='0')
      { k++; j++; }
      for (m=1;m<=k;m++)
          {
             l=(BUF[j-m]-48 ) * (int) pow (10,m-1) +l;
          }
      return l;
    }
   }
  }
 }
     

