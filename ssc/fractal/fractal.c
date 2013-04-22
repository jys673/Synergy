/************************************************************************
 *
 *    Program to Generate Fractal images on  SUN X Windows
 *
 *    Kostas   Blathras
 *    Temple U.
 *    December  1989
 *
 ************************************************************************
 */
#include  <stdio.h>
#include  <time.h>
#include  <X11/Xos.h>
#include  <X11/Xlib.h>
#include  <X11/Xutil.h>
#include  <X11/Xatom.h>
#include "fractal.h"

#define    TRUE  1
#define    FALSE 0
#define    NMAX  1024
#define    DISPLAY 1

/*
 *   Global   X-Windowing Stuff
 */
Display    *dpy;
Window      win;
GC          gc;
XWindowAttributes   war;
unsigned long     fg, bg, bd, bw;   /* foreground, back., border, borderwidth*/
Colormap    cmap;
XGCValues   gcv;
XEvent      event;
Pixmap      pix;
XSizeHints  xsh;     /* Size hints for window manager */

short int   screen[XRES][YRES];      /*   Screen buffer    */
unsigned long     pixellist[NMAX];
double  xstep, ystep, tstart[2];
double wall_clock();

main()
{
    int     ix, iy, i, j, inten, status, ip, ops; 
    double  flops ;
    double  x, y, tnew[2], told[2], retval, sigma[2], temp;
    double  time, rate;
    double  t0, t1;
    char    host[128];
    FILE    *fd;
    
    fd = fopen("fractal.seq.time", "a");
    xstep = (xmax-xmin) / (float)XRES;
    ystep = (ymax-ymin) / (float)YRES;
    x = xmin - xstep;
/*
 *  Initialise X-Windows
 */
    if (DISPLAY)
    {
    	init_X() ;
    	XNextEvent( dpy, &event );
    }
    t0 = wall_clock();
    flops = 0;
    for  ( ix=0; ix<XRES; ix++ ) {
         x += xstep;
         ops = 0;
         y = ymin - ystep;
         for  ( iy=0; iy<YRES; iy++ ) {
              y += ystep;
              told[0] = 0;
              told[1] = 0;
              tnew[0] = 0;
              tnew[1] = 0;
              sigma[0] = x;
              sigma[1] = y;
/*------------------------------------------------------------------------*/
/*  Iterate until point diverges , or                                     */
/*  index of iteration is gt an upper limit                               */
/*------------------------------------------------------------------------*/
              i = 0;
              while  ( (i <= iterat) &&
                       ( (tnew[0]*tnew[0]+tnew[1]*tnew[1]) < 4  ) ) {
                  i++;
                  tnew[0] = told[0]*told[0] - told[1]*told[1] + sigma[0];
                  tnew[1] = 2 * told[0]*told[1] + sigma[1];
                  told[0] = tnew[0];
                  told[1] = tnew[1];
              }
/*-----------------------------------------------------------------------*/
/*  Set foreground color and draw the pixel                              */
/*-----------------------------------------------------------------------*/
                flops = flops + i;
		ops = ops + i;
		if (DISPLAY)
		{
			XSetForeground( dpy, gc, i );
			XDrawPoint( dpy, win, gc, ix, iy );
/*
			printf("x(%f)ix(%d) y(%f)iy(%d) c(%d)\n",x,ix,y,iy,i);
*/
		}
         }
    }
    t1 = wall_clock();
    if (DISPLAY)
    {
    	XFlush( dpy );
    }
    time = (double) (t1 - t0)/1000000.0;
    status = gethostname(host,sizeof(host));
    printf("Sequential time(%f) host(%s) (%d,%d)(%d)(%f,%f)(%f,%f)(%d) \n",
	time, host, XRES, YRES, iterat, xmin, ymin, xmax, ymax, (int)flops/(XRES*YRES));
    fprintf(fd, "Sequential time(%f) host(%s) (%d,%d)(%d)(%f,%f)(%f,%f)(%d)",
	time, host, XRES, YRES, iterat, xmin, ymin, xmax, ymax, (int)flops/(XRES*YRES));
    flops = flops * 10 ;
    rate = flops / ( 1000000 * time );
    printf("\nEffective speed of sequential program is : %f Mflops \n", rate);
    fprintf(fd," Effective speed : %f Mflops \n", rate);
    if (DISPLAY) EventH();  
}

/*==========================================================================
 *
 *   Function Init_X
 *      Initialises X-Window for Mandelbrot Graph
 *
 *==========================================================================
 */
init_X()
{
   int i, numpix, nomax, nomin;
/*
 *   Open Display
 */
    if  (( dpy=XOpenDisplay(NULL))==NULL) {
        perror(" Can't open display");
        exit(0);
    }
/*
 *   Get default Colormap
 */
    cmap = DefaultColormap( dpy, DefaultScreen( dpy ) );
/*
 *  Setup Geometry for the window
 *
 */
    xsh.x = 100;
    xsh.y = 100;
    xsh.width = XRES;
    xsh.height = YRES;
    bw = 3;
    bd = 1;
    bg = 0;
/*
 *  Create the window
 */
    win = XCreateSimpleWindow(  dpy, DefaultRootWindow(dpy),
                                xsh.x, xsh.y, xsh.width, xsh.height,
                                bw, bd, bg );
    XGetWindowAttributes( dpy, win, &war );
    pix = XCreatePixmap( dpy, win, xsh.width, xsh.height, war.depth );
/*
 *  Create a graphics context, for the Pixel color
 */
    gc = XCreateGC( dpy, win, 0L, &gcv );
    numpix = 1<<war.depth;
    nomax = 0;
    nomin = NMAX;
    pixellist[0] = 1;
    for  ( i=1; i<NMAX; i++ ) {
       pixellist[i] = (2 + ((i-nomin)*(numpix-2))/(nomax-nomin)) % numpix ;
    }
/*
 *  Select the input and map the window
 */
    XSelectInput( dpy, win, ExposureMask );
    XMapWindow( dpy, win );
}

/*====================================================================
 *   Subrourtine Event Handler
 *====================================================================
 */
EventH()
{
/*
 *  to be moved elsewhere...
 *      Process events...
 */
     while(1) {
         XNextEvent( dpy, &event );
     }
}
