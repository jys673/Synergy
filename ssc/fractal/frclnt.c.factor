/************************************************************************
 *
 *    Fractal client.
 *    Builts up the tuples, and is responsible for all displaying.
 *
 *    Kostas   Blathras
 *    Temple U.
 *    April  90
 *    revised by Yuan Shi, May 1993
 *
 ************************************************************************
 */
#include  <stdio.h>
#include  <fcntl.h>
#include  <math.h>
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

int cnf_tsget();

main()
{
    double xstep, ystep,x;	/* World Coord */
    int    ix, iy, i, j, ip; 
    double flops, clen;
    int    status, tsd, tpnamsz, tplength, res, received;
    float  f; 					/* (0-1], scheduling factor */
    int    G,					/* Tuple grain size */
	   t=1,					/* cutoff level */
	   R,					/* Remainder cols */
	   P; 					/* number of processors */ 

    double t0,t1;				/* for timing purpose */
    double start_time;
    FILE  *fd;					/* for stat collection */
    char buff[132];

    time(&start_time);
    t0 = time((long *)0);
/*  Initialise X-Windows */ 
    if (DISPLAY) 
    {
	init_X();
	XNextEvent( dpy, &event );
    }
/*  Get Fractal Parameters 
    printf(" \n  Please enter Number of Processor: ");
    scanf("%d" , &P );
    printf(" \n  Please enter Factor Value (0-1]: ");
    scanf("%f", &f);
    printf(" \n  Please enter Cutoff Value (1-%d): ",XRES);
    scanf("%d", &t);
*/

    fd = fopen("fr_stat.s93", "a");
    xstep = (xmax-xmin) / (double)XRES;
    ystep = (ymax-ymin) / (double)YRES;
    x = xmin;

    tsd = cnf_open("coords",0);
    res = cnf_open("colors",0);

    f = (float)cnf_getf();
    f = f/100.0;
    P = cnf_getP();
    t = cnf_gett();
printf(" get f(%f) P(%d) t(%d) \n",f,P,t);

    tplength = 9 * sizeof(double);   /*   In bytes   */
    R = XRES;
    ix = 0; 
    G = 1;
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
/*
		printf("Put in tuple (%s) G(%d) ix(%d) x(%f) R(%d)\n",
			tpname,G,ix,x,R);
*/
		status = cnf_tsput(tsd,tpname,ituple,tplength);
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
/*
    printf("Put in tuple (%s) G(%d) ix(%d) x(%d)\n",
			tpname,G,ix,x);
*/
    status = cnf_tsput(tsd,tpname,ituple,tplength);
    clen = clen + 9*8; 
    if (R>0) { 	/* insert the last zero size tuple as termination signal */
	ituple[0] = (float) 0;
	sprintf( tpname, "i%d\0", XRES);
	tpnamsz = strlen(tpname);
    	status = cnf_tsput(tsd,tpname,ituple,tplength);
	clen = clen + 9*8; 
    }
    /* Now receive the results */ 
    received = 0;
    tplength = 100000; 
    flops = 0;
    while ( received < XRES ) {
        strcpy( tpname, "*\0" );
printf("Master wants (%s) \n",tpname); 
        status = cnf_tsget( res, tpname, otuple, 0 );
printf("Master received (%s) length(%d) total count(%d)\n", tpname,status,received);   
	ix = (int) otuple[1];
	G = (int) otuple[0];
printf("Master received G(%d)\n",G);
	iy = 2;
	clen = clen + status; 
	for (i=0; i<G; i++) {
printf("MASTER loop 1........................................(%d,%d)\n",i,G);
		received ++;
		for  ( j= 0; j<YRES; j++ ) {
/*
printf("master looping 2 ... j(%d) Y(%d) ---i(%d) G(%d)\n",j,YRES,i,G);	
*/
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
    t1 = time((long *)0) - t0;
    printf("Elapsed time: (%d) p(%d) f(%f) t(%d)\n",t1, P,f,t);
    fprintf(fd, 
	"Te(%d) P(%d) f(%f) t(%d) xmin(%f)xmax(%f)ymin(%f)ymax(%f)XRES(%d)YRES(%d) Iterat(%d) DISP(%d) tm(%s) (%f) (%f) \n",
	 t1,P,f,t,xmin,xmax,ymin,ymax,XRES,YRES,iterat,DISPLAY,ctime(&start_time), (float)flops/(XRES*YRES), (float)clen/(XRES*YRES));
    fclose(fd);
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
