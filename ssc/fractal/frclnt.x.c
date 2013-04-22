/************************************************************************
 *
 *    Fractal client.
 *    Builts up the tuples, does the display.
 *
 *    Kostas   Blathras
 *    Temple U.
 *    April  90
 *    revised by Yuan Shi for factoring & fixed chunking, May 1993
 *    revised by Feijian Sun for XDR, August 1994
 *
 ************************************************************************
 */
#include  <stdio.h>
#include  <X11/Xlib.h>
#include  <X11/Xutil.h>
#include  "fractal.h"

#define    NMAX 	512
#define    DISPLAY 	0
/*
 *   Global   X-Windowing Stuff
 */
Display    *dpy;
Window      win;
GC          gc;
XWindowAttributes   war;
unsigned long     fg, bg, bd, bw;   /* foreground, back.,  borderwidth*/
Colormap    cmap;
XGCValues   gcv;
XEvent      event;
Pixmap      pix;
XSizeHints  xsh;     /* Size hints for window manager */
unsigned long     pixellist[NMAX];

main()
{
    double xstep, ystep,x;	/* World Coord */
    int    ix, iy, i, j, ip, flops, clen;
    int    status, tsd, tpnamsz, tplength, res, received;
    float  f; 					/* (0-1], scheduling factor */
    int    G,					/* Tuple grain size */
           t=1,					/* cutoff level */
	   R,					/* Remainder cols */
	   P; 					/* number of processors */ 
    char buff[132];

/*  Initialise X-Windows */ 
    if (DISPLAY) 
    {
	init_X();
	XNextEvent( dpy, &event );
    }

    xstep = (xmax-xmin) / (double)XRES;
    ystep = (ymax-ymin) / (double)YRES;
    x = xmin;

    tsd = cnf_open("coords",0);
    res = cnf_open("colors",0);

    f = (float)cnf_getf();
    f = f/100.0;
    P = cnf_getP();
    t = cnf_gett();

    tplength = 9 * sizeof(double);   /*   In bytes   */
    R = XRES;
    G = 1;
    ix = 0; 
    clen = 0; /* count for IO total */
    while ((R > t) && (G>0)) {
	G = (int) R*f/P;
	R = R - G * P;		/* Pass along the remainder to next partition */
	if (G>0) {
	    for (i=0; i<P; i++) { 
		ituple[0] = (double)G;
		ituple[1] = (double)ix;
		ituple[2] = xmin;
		ituple[3] = xmax;
		ituple[4] = ymin;
		ituple[5] = ymax;
		ituple[6] = (double)(XRES);
		ituple[7] = (double)(YRES);
		ituple[8] = (double)iterat;
		sprintf( tpname, "i%d\0",ix);
		tpnamsz = strlen(tpname); 
		status = cnf_xdr_tsput(tsd, tpname, (char *)ituple, tplength, 5); 
		clen = clen + 9*8;
		x += G*xstep; 
		ix +=G;
	    }
	}
    }
    ituple[0] = (double) R;
    ituple[1] = (double) ix;
    ituple[2] = xmin;
    ituple[3] = xmax;
    ituple[4] = ymin;
    ituple[5] = ymax;
    ituple[6] = (double) XRES;
    ituple[7] = (double) YRES; 
    ituple[8] = (double) iterat;
    sprintf( tpname, "i%d\0",ix);
    tpnamsz = strlen(tpname); 

    status = cnf_xdr_tsput(tsd, tpname, (char *)ituple, tplength, 5);
    clen = clen + 9*8; 
    if (R>0) { 	/* insert the last zero size tuple as termination signal */
	ituple[0] = (float) 0;
	sprintf( tpname, "i%d\0", XRES);
	tpnamsz = strlen(tpname);
	status = cnf_xdr_tsput(tsd, tpname, (char *)ituple, tplength, 5);
	clen = clen + 9*8; 
    }
    /* Now receive the results */ 
    received = 0;
    tplength = 100000; 
    flops = 0;
    while ( received < XRES ) {
        strcpy( tpname, "*" );
	status = cnf_xdr_tsget(res, tpname, (char *)otuple, 0, 5);
        printf("Master received %s tuple length(%d)\n", tpname,status);   
	fflush(stdout);
	ix = (int) otuple[1];
	G = (int) otuple[0];
	iy = 2;
	printf(" Master received X(%d) G(%d)\n",ix,G);
	clen = clen + status; 
	for (i=0; i<G; i++) {
		received ++;
		for  ( j= 0; j<YRES; j++ ) {
			ip = (int)otuple[iy];
			flops = flops + 6 + 9*ip;
			if (DISPLAY)
			{
				XSetForeground( dpy, gc, ip );
				XDrawPoint( dpy, win, gc, ix, j );
			}
			iy++;
		}
		ix ++;
	}
    }
    cnf_close(tsd);
    cnf_close(res);
    if (DISPLAY)
    {
	EventH();
	XFlush( dpy ); 
    }
    cnf_term();
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
    xsh.x = 500;
    xsh.y = 500;
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
 *  to be moved elsewhere..
 *      Process events...
 */
     while(1) {
         XNextEvent( dpy, &event );
     }
}
