/*=========================================================================*/
/* Function : Initialise the X-window Display                              */
/*                                                                         */
/* Arguments :                                                             */
/*   int  xres, yres :  Desired Screen resolution                          */
/*   title : The name of the window                                        */
/*                                                                         */
/*   This Initialization Routine opens a Window , and allocates space      */
/*   for a pixmap, and an Image of N rows wide.                            */
/*   ( N is defined in the inx.h file ).                                   */
/*   The pixmap should be used to store the picture and refresh the        */
/*   picture after an expose event.                                        */
/*   The Image should be used in the following fashion :                   */
/*   Each time we have the colors allocated for a pixel, then we should    */
/*   write it in the pixel pointer, until we reach the limit               */
/*   xres*_NROWS.  At that point the Image should be copied in the         */
/*   Pixmap,  and on the Window.                                           */
/*   There is also a global variable progress, that indicates the rows     */
/*   that have been filled up to now.                                      */
/*                                                                         */
/*              Kostas Blathras, IBM , T.J. Watson                         */
/*=========================================================================*/
#define MAXNRES 1000
#define MAXCOLS 180
int pxl[MAXCOLS];

#include "inx.h"
#include <math.h>
void HsvToRgb( double h,double s,double v,double *r, double *g, double *b);



void Init_X( xres, yres, width,  title, argc, argv)
    int  xres, yres, argc, width;
    char  *title, *argv[];
{
    int i, icl, n, ir, ib, ig;
    int  black, white;
    char  aa[2][20];
    Window root;
    double r, g, b, h, s, v, dh;
    int cstep;
/*------------------------------------------------------------------------*/
/*   Open Display and get other default resources                         */
/*------------------------------------------------------------------------*/
    if  (( dpy=XOpenDisplay(NULL))==NULL) {
        perror(" Can't open display");
        exit(0);
    }
    root = RootWindow( dpy, DefaultScreen(dpy) );
    cmap = DefaultColormap( dpy, DefaultScreen( dpy ) );
    gc = XCreateGC( dpy, root, 0L, NULL );
    black = BlackPixel( dpy, DefaultScreen(dpy) );
    white = WhitePixel( dpy, DefaultScreen(dpy) );

/*------------------------------------------------------------------------*/
/*  Setup Geometry and create the window                                  */
/*------------------------------------------------------------------------*/
    printf("Opening a window %d * %d \n", xres, yres );
    xsh.x = 100;
    xsh.y = 100;
    xsh.min_width = xres;
    xsh.min_height = yres;
    xsh.width = xres;
    xsh.height = yres;
    bw = 3;
    bd = 1;
    bg = 0;
    win = XCreateSimpleWindow(  dpy, root,
                                xsh.x, xsh.y, xsh.width, xsh.height,
                                bw, black, white );
    xsh.flags = PSize | PMinSize;
    strcpy( aa[0], title );
    XSetStandardProperties( dpy, win, title, title, NULL, argv, argc, &xsh );
    XSelectInput( dpy, win, event_mask );
    XMapRaised( dpy, win );
    pixels = (char *)malloc( xres*yres );
    visual = DefaultVisual( dpy, DefaultScreen( dpy ) );
    image = XCreateImage( dpy, visual, 8, ZPixmap, 0, pixels,
                          xres, yres, 8, 0 );
    XGetWindowAttributes( dpy, win, &war );
    pix = XCreatePixmap( dpy, win, xsh.width, xsh.height, war.depth );
    XMaskEvent( dpy, event_mask, &event );
/*------------------------------------------------------------------------*/
/*  Create blue-red  colormap                                             */
/*------------------------------------------------------------------------*/
    n = MAXCOLS;
    icl = 0;
    for (i=0; i<MAXCOLS/4; i++) {
        XColor color;
        ir = 0;   ig = i*4; ib = 255;
        color.red = ir << 8;
        color.green = ig << 8;
        color.blue = ib << 8;
        color.flags = DoRed | DoGreen | DoBlue; 
        if (!XAllocColor(dpy, cmap, &color)) {
            printf("out of color table entries %d\n", i);
            exit(-1);
        }
        table[icl] = color.pixel;
        icl++;
    }
    for (i=MAXCOLS/4-1; i>=0; i--) {
        XColor color;
        ir = 0;   ig = 255; ib = i*4;
        color.red = ir << 8;
        color.green = ig << 8;
        color.blue = ib << 8;
        color.flags = DoRed | DoGreen | DoBlue; 
        if (!XAllocColor(dpy, cmap, &color)) {
            printf("out of color table entries %d\n", i);
            exit(-1);
        }
        table[icl] = color.pixel;
        icl++;
    }

    for (i=0; i<MAXCOLS/4; i++) {
        XColor color;
        ir = i*4;   ig = 255; ib = 0;
        color.red = ir << 8;
        color.green = ig << 8;
        color.blue = ib << 8;
        color.flags = DoRed | DoGreen | DoBlue; 
        if (!XAllocColor(dpy, cmap, &color)) {
            printf("out of color table entries %d\n", i);
            exit(-1);
        }
        table[icl] = color.pixel;
        icl++;
    }
    for (i=MAXCOLS/4-1; i>=0; i--) {
        XColor color;
        ir = 255;   ig = i*4; ib = 0;
        color.red = ir << 8;
        color.green = ig << 8;
        color.blue = ib << 8;
        color.flags = DoRed | DoGreen | DoBlue; 
        if (!XAllocColor(dpy, cmap, &color)) {
            printf("out of color table entries %d\n", i);
            exit(-1);
        }
        table[icl] = color.pixel;
        icl++;
    }

}


void Init_X_RealCols( xres, yres, width,  title, argc, argv)
    int  xres, yres, argc, width;
    char  *title, *argv[];
{
   int i, n;
   int  black, white;
   char  aa[2][20];
   Window root;
   XVisualInfo vinfo;
   int colornum, status, scrn;
   XSetWindowAttributes   attr;
   unsigned long     plane_masks[8];
   XColor      coltab[MAXCOLS];
/*------------------------------------------------------------------------*/
/*   Open Display and get other default resources                         */
/*------------------------------------------------------------------------*/
    if  (( dpy=XOpenDisplay(NULL))==NULL) {
        perror(" Can't open display");
        exit(0);
    }
    scrn = DefaultScreen(dpy);
/*------------------------------------------------------------------------*/
/*  Setup Geometry and create the window                                  */
/*------------------------------------------------------------------------*/
    printf("Opening a window %d * %d \n", xres, yres );
    xsh.x = 100;
    xsh.y = 100;
    xsh.min_width = xres;
    xsh.min_height = yres;
    xsh.width = xres;
    xsh.height = yres;
    bw = 3;
    bd = 1;
    bg = 0;

    if  ( !XMatchVisualInfo( dpy, scrn, DefaultDepth( dpy, scrn ),
          PseudoColor, &vinfo ) ) {
          perror(" Matching Visual Info");
          exit(0);
    }
    cmap = XCreateColormap( dpy, RootWindow( dpy, scrn ),
                            vinfo.visual, AllocNone );
    if ( cmap == 0 ) {
        perror(" Cant create Colormap");
        exit( 0 );
    }
/* */
/* Create window */
/* */
    attr.event_mask = KeyPressMask | ButtonPressMask | ExposureMask |
                      StructureNotifyMask | ColormapChangeMask;
    attr.colormap = cmap;
    win = XCreateWindow(  dpy, RootWindow(dpy,scrn),
                          xsh.x, xsh.y, xsh.width, xsh.height,
                          bw, vinfo.depth, InputOutput, vinfo.visual,
                          CWEventMask | CWColormap, &attr );
    XGetWindowAttributes( dpy, win, &war );
    pix = XCreatePixmap( dpy, win, xsh.width, xsh.height, war.depth );
    gc = XCreateGC( dpy, win, 0L, &gcv );

    XSelectInput( dpy, win, event_mask );
    XMapRaised( dpy, win );
    XSetWindowColormap( dpy, win, cmap);
    XInstallColormap( dpy, cmap );
    if  ( ( status = XAllocColorCells( dpy, cmap, True, plane_masks,
                     0, pxl, 256 ) ) == 0 ) {
        perror(" Allocating Color Cells ");
        exit( 0 );
    }


    pixels = (char *)malloc( xres*yres );
    visual = DefaultVisual( dpy, DefaultScreen( dpy ) );
    image = XCreateImage( dpy, visual, 8, ZPixmap, 0, pixels,
                          xres, yres, 8, 0 );
    printf("Image Created \n\n");

    XMaskEvent( dpy, event_mask, &event );
/*------------------------------------------------------------------------*/
/*  Create blue-red  colormap                                             */
/*------------------------------------------------------------------------*/
    for (i=0; i<MAXCOLS; ++i){        /* red */
        colornum = i;
        coltab[colornum].pixel = pxl[colornum];
        coltab[colornum].red = 256 << 8;
        coltab[colornum].green = i << 8;
        coltab[colornum].blue = 256 << 8;
        coltab[colornum].flags = DoRed | DoGreen | DoBlue;
    }
    if ( XStoreColors( dpy, cmap, coltab, MAXCOLS )== 0 ) perror(" Colors");

}


/*========================================================================*/
/*  Show displays (xarr x yarr) an array of MAXNRES (must be #defined)    */
/*  on a Window of xres x yres                                            */
/*========================================================================*/
void ShowDbl( int xarr, int yarr, int xres, int yres, 
         double arr[MAXNRES][MAXNRES]) {
    int index, ixp, iyp, i, j, x, y;
    unsigned char color, cl;
    double min, max, scale, xp, yp;

/* */
/*    find min and max, and then scale arr entries into pixel array. */
/*    printf(" Array %d * %d, Display %d*%d \n",xarr,yarr,x res, yres); */
/*    printf("Finding Min and Max"); */
/* */
    min = max = arr[0][0];
    for  (x=0; x<xarr; x++) {
        for  (y=0; y<yarr; y++ ){
            if  ( arr[x][y] > max ) max = arr[x][y];
            if  ( arr[x][y] < min ) min = arr[x][y];
        }
 /*       printf("."); */
/*        fflush(stdout); */
    }
    if ((max-min)<0.000001) min = max-1.0;
/* */
/*  Scale is for color scaling into our MAXCOLS available colors   */
/*  ixp : Number of pixels per array entry                    */

    scale = (float)(MAXCOLS-1) / (max - min);

    xp = (float)xres/(float)xarr;
    yp = (float)yres/(float)yarr;
    ixp = (int) xp+0.1;
    iyp = (int) yp+0.1;
/**
    printf("%d %d ixp %d  iyp %d   min %f  max %f  scale %f \n", 
           xres, xarr, ixp, iyp, min, max, scale);
**/
    for  (x=0; x<xarr; x++) {
        for  (y=0; y<yarr; y++ ){
            for  (i=0; i<ixp; i++) {   
                for  (j=0; j<iyp; j++) {
                    index = x*xres*iyp + y*ixp + (i*yres) +j;
                    cl =  scale * (arr[x][y]-min);
/*                    cl = y%255;   */
                    pixels[index] = (unsigned char)table[cl]; 
/*                    pixels[index] = (char) table[cl];  */
                }
            }
        }
    }
    
    XPutImage( dpy, pix, gc, image, 0, 0, 0, 0, xres, yres );      
    XCopyArea( dpy, pix, win, gc, 0, 0, xres, yres, 0, 0 );
    XFlush( dpy );
        
}

/*========================================================================*/
/*   Event Handler Subroutine, Skeleton Version.                          */
/*   Handles :                                                            */
/*     Exposure events and Move Events                                    */
/*========================================================================*/
void EventH( done, xres, yres )
    int *done, xres, yres;
{
     int  x, y, width, height;

     while ( XCheckMaskEvent( dpy, event_mask, &event ) == True ) {
         switch ( event.type ) {
             case Expose:
                 x = event.xexpose.x;
                 y = event.xexpose.y;
                 width = event.xexpose.width;
                 height = event.xexpose.height;
                 if  ( event.xexpose.window ==win ) {
                    if (  y+height > progress )  height = progress - y;
                 }
                 XCopyArea( dpy, pix, win, gc, 0, 0, xres, yres, 0, 0 );
/*             XCopyArea( dpy, pix, win, gc, x, y, width, height, x, y ); */
                 break;
             case ButtonPress :
                 if  ( event.xbutton.window == win ) {
                     *done = 1;
                 }
                 break;
             default:
                 break;
         } /* endswitch */
     } /* endwhile */
}

void HsvToRgb( double  h, double  s, double v, 
          double *r, double *g, double *b)
{
    int i;
    double f, p, q, t;

    if  ( s==0 ) {  /* No color.. just gray level */
        *r=v; *g=v; *b=v;
    } else { 
       h = h / 60.0;
       i = (int) h;
       f = h - (double) i;
       p = v*(1-s);
       q = v*(1-(s*f));
       t = v*(1-s*(1-f));

       switch (i) {
       case 0:
           *r = v; *g = t; *b = p;
           break;
       case 1:
           *r = q; *g = v; *b = p;
           break;
       case 2:
           *r = p; *g = v; *b = t;
           break;
       case 3:
           *r = p; *g = q; *b = v;
           break;
       case 4:
           *r = t; *g = p; *b = v;
           break;
       case 5:
           *r = v; *g = p; *b = q;
           break;
       }        
    }
}
