#include <stdio.h>
#include <stdlib.h>

int main()
{
 FILE *fp;
 int i;
 unsigned char c,b,d,a;

 int XRES,YRES;
 fp=fopen("povray.out","r");

 a=fgetc(fp);
 b=fgetc(fp);
 c=fgetc(fp);
 d=fgetc(fp);
  XRES=256*a+b;
/*
 printf("\n\n\n\n XRES: %d   YRES: %d\n\n",a*256+b,c*256+d);
 for(i=1;i<=3000;i++)
 {
 if (i==753) printf("\n");
 c=fgetc(fp);
 printf("%d,",c);
 }
*/
 printf("\n XRES:%d, %d, %d, %d, %d,",XRES,a,b,c,d);
 YRES=XRES*3+2;
 for (i=0;i<=3500;i++)
 {
 if (i%YRES==0) printf("\n");
 printf("%d,",fgetc(fp));
 }
 fclose(fp);
}
