/*
   This is the drawing program for the result of the ray tracing program.
   It will allocate a image area so that we can re-draw the scene fast
   after a WINDOW EXPOSE event. 

   C FU
*/




#include  <stdio.h>
#include  <math.h>
#include  <signal.h>
#include <stdlib.h>
#include  <string.h>
#include  <ctype.h>
#include  <X11/Xos.h>
#include  <X11/Xlib.h>
#include  <X11/Xutil.h>
#include  <X11/Xatom.h>
#define XRES 1024
#define YRES 1024
#define    TRUE  1
#define    FALSE 0
#define    NMAX  1024
#define    DISPLAY 1 
XWindowAttributes   war;
unsigned long     fg, bg, bd, bw;   /* foreground, back., border, borderwidth*/
Colormap    cmap;
Pixmap      pix;
XColor C[256];

Visual *visual;
unsigned int depth;
int bitmap_pad;
XImage *image;
Drawable drawable;

int format;
int WIDTH,HEIGHT;


double OP=0.;   /* total Float s */
short int   scren[XRES][YRES];      /*   Screen buffer    */
unsigned long     pixellist[NMAX];
double  xstep, ystep,
        tstart[2];
char get_level(unsigned char);

draw_pic(char filename1[30])
{
    int width,height;
    unsigned char c1,c2,c3,c4;
    int     ix, iy, i, j, inten, status, ip;
    double   x, y, tnew[2], told[2], retval, sigma[2], temp;
    double   rate, t0, t1, time;
    FILE *fd;
    char host[128];
    unsigned char r,g,b;
    int c;
    int screen;
    int ops;
    double flops;
    char filename[80];
    init_color();

    
     fd=fopen(filename1,"r");
     if (!fd) 
     fd=fopen("povray.out","r");
     if (!fd) { printf("\n file open error!"); exit(1); }
     c1=fgetc(fd); c2=fgetc(fd);
     c3=fgetc(fd); c4=fgetc(fd); 
     
     WIDTH=width=c1*256+c2; 
     HEIGHT=height=c3*256+c4;
     fflush(stdout);

/**  Image mapping ***/

    screen=DefaultScreen( mydisplay );
    depth=DefaultDepth(mydisplay, screen);
    if (depth==1)
    {
    format=XYPixmap;
    bitmap_pad=32;
    }
    else 
    {
     format=ZPixmap;
     bitmap_pad=8;
     if (depth>8) bitmap_pad=32;
    }
     image=XCreateImage(mydisplay, DefaultVisual(mydisplay,screen),
                        depth, format, 0, 0,
                        width, height, bitmap_pad,0);

     if (image==0)
      { printf("\n IMAGE structure allocate error!");
      }
     else
     {
      image->data=malloc(image->bytes_per_line * height);
      if (image->data ==0)
      printf("\n IMAGE memory allocate error!");
     }
     


/* ---- get image ----^^^^ */

                        for(i=1;i<=height;i++)
                       { 
                        int LINE_Y;
                        c1=fgetc(fd);
                        c2=fgetc(fd);
                        LINE_Y=c1*256+c2;
                        for(j=1;j<=width;j++)
                        {
                        int cc;
                        r=fgetc(fd); 
                        g=fgetc(fd);  
                        b=fgetc(fd);   
                        r=get_level(r);
                        g=get_level(g);
                        b=get_level(b);
                        cc=r*6*6+g*6+b;
                        c=scren[j][i]=cc;
			XSetForeground( mydisplay,
                                        mygc, C[c].pixel);
			XDrawPoint( mydisplay, 
                                    mywindow,
                                    mygc, j,LINE_Y );
                        XPutPixel(image,j,LINE_Y, C[c].pixel);
                         }
                       }

    	XFlush( mydisplay );
        XPutImage(mydisplay,mywindow,mygc,image,
                  1,1,1,1,width,height);
}



init_color()
{
    int i;
    int OFF;
    char r,g,b;
    cmap = DefaultColormap( mydisplay,
                            DefaultScreen( mydisplay ) );
    i=0; OFF=10200;
    for (r=0;r<=5;r++)
    {
     for (g=0;g<=5;g++)
     {
      for (b=0;b<=5;b++)
       {
         C[i].red =r*OFF;
         C[i].green=g*OFF;
         C[i].blue=b*OFF;
    if (XAllocColor (mydisplay, cmap, &C[i]) == 0)
    { printf("\n Alloc Color %d Error ",i); exit(1); } 
         i++;
     }
          
    }
   }
}
char get_level(unsigned char x)
{
 if (x<42) return 0;
 else if (x<84) return 1;
 else if (x<126) return 2;
 else if (x<168) return 3;
 else if (x<210) return 4;
 else return 5;
}
