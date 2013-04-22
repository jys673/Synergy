/************************************************************************
 *
 *    Fractal client.
 *    Builts up the tuples, and is responsible for all displaying.
 *
 *    Kostas   Blathras
 *    Temple U.
 *    April  90
 *    revised by Yuan Shi, May 1993
 *    Revised by Yuan Shi for automatic load balancing, Oct. 1994
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
#define    DISPLAY 	1

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

double second() {
#include <sys/time.h>
#include <sys/resource.h>
	struct timeval tp;
	struct timezone tzp;
	double t;

	gettimeofday(&tp, &tzp);
	t = (tzp.tz_minuteswest*60 + tp.tv_sec+tp.tv_sec)*1.0e6 + 
		(tp.tv_usec+tp.tv_usec)*1.0;
	return t;
}
double index_m[100]; /* at most 100 processors */
double longest = 0, sum = 0;

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

    long t0,t1;				/* for timing purpose */
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

    fd = fopen("fractal.time", "a");
    xstep = (xmax-xmin) / (double)XRES;
    ystep = (ymax-ymin) / (double)YRES;
    x = xmin;

    tsd = cnf_open("coords",0);
    res = cnf_open("colors",0);
    P = cnf_getP();

    R = XRES;
    tplength = 9 * sizeof(double);   /*   In bytes   */
    ix = 0; 
    /* use f as test grain size */
    G = cnf_getf(); 
printf(" frclnt G= (%d)\n",G);
    if (G > 0) goto manual_partition;
    G = cnf_gett();
    for (i=0; i < P; i++) /* send out test tuples */
    {
	ituple[0] = (double)G;
	ituple[1] = (double)ix;
	ituple[2] = xmin;
	ituple[3] = xmax;
	ituple[4] = ymin;
	ituple[5] = ymax;
	ituple[6] = (double)(XRES);
	ituple[7] = (double)(YRES);
	ituple[8] = (double)iterat;
	index_m[i] = (double)second();
printf("frclnt send out time P%d (%.3f) \n",i, index_m[i]);
	sprintf( tpname, "T%d\0",i);
	tpnamsz = strlen(tpname); 
	status = cnf_tsput(tsd,tpname,ituple,tplength);
    }
    for (i=0; i < P; i++)
    {
	strcpy(tpname,"T*");
	clen = cnf_tsget(res,tpname, ituple, 0);
printf("frclnt received a test tuple (%s) \n",tpname);
	ix = (int) ituple[0]; 	/* get back the processor index */
	index_m[ix] = second() - index_m[ix];
	printf(" frclnt received P%d clock:(%.3f) \n", ix,
		index_m[ix]);
	if (index_m[ix] > longest) longest = (double) index_m[ix];
    } 
    for (i=0; i < P; i++)
    {
	index_m[i] = longest/index_m[i];	
	sum = sum + index_m[i];
	printf(" fractal index P%d : %.3f \n",i,index_m[i]);
	fprintf(fd,"P%d: %.3f\n",i,index_m[i]);
    }
    if ((sum == 0)&&(P > 1)) sum = P; /* forces P partition */
    else sum = 10; /* fixed partition on a single processor */
    G = (int)(XRES/sum);

manual_partition:

    fprintf(fd,"G = (%d) R=(%d) sum (%f) \n",G, R, sum);
    R = XRES;
    x = xmin;
    ix = 0;
    printf(" G = (%d) R(%d) sum(%f) \n", G, R, sum);

    tplength = 9 * sizeof(double);   /*   In bytes   */
    clen = 0; /* count for IO total */
    while ((R > 0)) {
	if (R >= G) ituple[0] = (double)G;
	else ituple[0] = (double) R;
	ituple[1] = (double)ix;
	ituple[2] = xmin;
	ituple[3] = xmax;
	ituple[4] = ymin;
	ituple[5] = ymax;
	ituple[6] = (double)(XRES);
	ituple[7] = (double)(YRES);
	ituple[8] = (double)iterat;
	sprintf( tpname, "W%d\0",ix);
	tpnamsz = strlen(tpname); 
		printf("Put in tuple (%s) G(%d) ix(%d) x(%f) R(%d)\n",
			tpname,G,ix,x,R);
	status = cnf_tsput(tsd,tpname,ituple,tplength);
	clen = clen + 9*8;
	if (R >= G) {
		x += G*xstep; 
		ix +=G;
	} else {
		x += R*xstep;
		ix +=R;
	}	
	R = R - G; 
    }
    /* insert the last zero size tuple as termination signal */
    ituple[0] = (float) 0;
    sprintf( tpname, "W%d\0", XRES);
    tpnamsz = strlen(tpname);
    status = cnf_tsput(tsd,tpname,ituple,tplength);
    clen = clen + 9*8; 

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
    t1 = time((long *)0) - t0;
    printf("Elapsed time: (%d) p(%d) G(%d)\n",t1, P,G);
    fprintf(fd, 
	"Te(%d) P(%d) G(%d) xmin(%f)xmax(%f)ymin(%f)ymax(%f)XRES(%d)YRES(%d) Iterat(%d) DISP(%d) tm(%s) alpha(%d) gamma(%d) MFLOPS (%d)\n",
	 t1,P,G,xmin,xmax,ymin,ymax,XRES,YRES,iterat,DISPLAY,ctime(&start_time), (int)flops/(XRES*YRES), (int)clen/(XRES*YRES), (int)flops/t1/1000000);
    fflush(fd);
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
