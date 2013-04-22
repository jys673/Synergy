#include <stdio.h>
#include <stdlib.h>

int main()
{
 FILE *fp;
 int j; 
 char file[100];
 int i;
  unsigned char data;
 printf("\n\n\n File name:");
 scanf("%s",file);

 fp=fopen(file,"r");
 if (!fp) { printf("\n FILE ERR\n"); exit(0);}
 printf("\n File %s opened! \n",file);
 j=0;
 while(!feof(fp))
{

 data=getc(fp);
 printf("%d",
 j++;
} 
 printf("\n length %d",j);
 printf("\n\n");
 fclose(fp);
}

