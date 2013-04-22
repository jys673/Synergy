
/*
 *           PVM 3.2:  Parallel Virtual Machine System 3.2
 *               University of Tennessee, Knoxville TN.
 *           Oak Ridge National Laboratory, Oak Ridge TN.
 *                   Emory University, Atlanta GA.
 *      Authors:  A. L. Beguelin, J. J. Dongarra, G. A. Geist,
 *          R. J. Manchek, B. K. Moore, and V. S. Sunderam
 *                   (C) 1992 All Rights Reserved
 *
 *                              NOTICE
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted
 * provided that the above copyright notice appear in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * Neither the Institutions (Emory University, Oak Ridge National
 * Laboratory, and University of Tennessee) nor the Authors make any
 * representations about the suitability of this software for any
 * purpose.  This software is provided ``as is'' without express or
 * implied warranty.
 *
 * PVM 3.2 was funded in part by the U.S. Department of Energy, the
 * National Science Foundation and the State of Tennessee.
 */

/*------------------------------------------------------------------*
 *			xepclnt.c
 *
 *	Display pixmap calculated by tiled workers in an X window.
 *
 *	Nov 92  R. J. Manchek
 *
 *-------------------------------------------------------------------*
 *	        	XEP Revision Project
 *
 *		Directed by  Dr. Yuan Shi	December 1993
 * 	Code Revised by   Sudhakar Nelamangala		 
 *    
 *		Synergy Distributed Systems	
 *		      Temple University
 *
 *-------------------------------------------------------------------*
 *
 *		Modifications		
 *
 *	1. start_worker() :
 *	2. stop_worker()  :
 *	3. pick() :
 *		Button1 : variable size box
 *		Button3 : fixed size box
 *	4. refresh_region():
 *	5. start_recalc() :
 *	6. claim_tile():
 *
 *		functions added
 *	1. search_host() :
 *	2. participants() :
 *	3. hamdahl() :
 *	
 *	 	default settings
 *	COLOR = 0  original color
 *	COLOR = 1 artificial color
 *	CAPTURE = 1 turn on  beta,speedup  calculation
 *	CAPTURE = 0 turn off bata, speedup calculation
 *
 */


#include <stdio.h>
#include <math.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Form.h>
#include <X11/Xutil.h>
#include "bfunc.h"
#include "bars.xbm"
#include "neww.xbm"
#include "quit.xbm"
#include "redo.xbm"
#include "zout.xbm"
#include "xep.h" 

#ifndef	min
#define	min(a,b)	((a)<(b)?(a):(b))
#endif
#ifndef	max
#define	max(a,b)	((a)>(b)?(a):(b))
#endif

#define	TALLOC(n,t)	(t*)malloc((n)*sizeof(t))

#define	NCMV	64
#define	byteswide(w) (((w)+7)/8)
#define	TILEHEIGHT	16

typedef	unsigned int IBIT32;

/* describes an image canvas (currently only one) */

struct canvas {
	Widget cn_wgt;			/* widget */
	Window cn_win;			/* window */
	int cn_wd;			/* window size */
	int cn_ht;
	u_char *cn_dat;			/* image data */
	XImage *cn_xim;			/* ximage struct */
	int cn_zoom;			/* display mag */
	int cn_ox;			/* offset for zoom */
	int cn_oy;
	double cn_re1;			/* corner coords */
	double cn_im1;
	double cn_re2;
	double cn_im2;
	int cn_x1;			/* pick coords */
	int cn_y1;
	int cn_x2;
	int cn_y2;
};


void pick();
void canvas_ev();
void redo_cb();
void zout_cb();



/***************
 **  Globals  **
 **           **
 ***************/

Display *xDisp;
XtAppContext context;
Widget topLevel;		/* main widget */
int xScrn;
Window xRootW;
int isMono;			/* monochrome display */
int isCmap;			/* display colormapped */
int nPlanes;			/* display depth */
int revByte;			/* X server byte order is opposite ours */
int bimbo;			/* bitmap bit order */
int xBpp;			/* ximage bits per pixel */
int xBypp;			/* ximage bytes per pixel */
int redMask;
int redShift;
int greenMask;
int greenShift;
int blueMask;
int blueShift;
Colormap xCmap;
int cmapInd[NCMV];
Visual *defVis;
IBIT32 fclutr[256];		/* pseudo-color lut for true-color display */
IBIT32 fclutg[256];
IBIT32 fclutb[256];
GC rubGc;			/* rubberbox gc */
GC canGc;			/* canvas painting gc */
Cursor crossCr;
struct canvas imCan;

int redoing = 0;		/* already calculating */
int dobars = 0;			
char *workerfile = 0;
int received;
float f;
int P;
int t = 1;
int G;
int R;
double xmin,ymin,xmax,ymax,xstep,ystep,x,y;
int tsd,res;
int status,tpnamsz,tplength;
int ix,iy;
long t0,t1,t2;
double rate,te;
long int flops;
int iterat;
FILE *fd,*fd2;
char buff[132];
u_char *temp;
int h_base = 50;     		/* half the side of the pick box */
int COLOR = 1;			

int CAPTURE= 0;

struct wlist {
	char worker[128];
	int density;
  } ;

struct wlist work_list[20];
int next_worker;
int Wcount;
int search_host();
add_density();
void participants();
void Hamdahl();

/***************
 **  Xt Gorp  **
 **           **
 ***************/

char canvasTl[] =
"*canvas.translations:\
<Btn1Down>:pick(start)\\n\
<Btn1Motion>:pick(adjust)\\n\
<Btn1Up>:pick(end)\\n\
<Btn3Down>:pick(modify)\\n\
<Btn3Motion>:pick(default)\\n\
<Btn3Up>:pick(default)\\n\
";


/* Widget default values */

static char *fallbacks[] = {
	"*allowShellResize:true",
	"*quitButton.label:Quit",
	"*recalcButton.label:Redo",
	"*workersButton.label:NewWorkers",
	canvasTl,
	0
};

/* To get custom resources */

typedef struct {
	Bool mono;			/* force monochrome display */
	String worker;		/* worker a.out name */
	Bool bars;			/* display processor id bars */
	int n;				/* number of workers */
} app_res_t, *app_resp_t;

static app_res_t app_res;

static XtResource res_list[] = {
	{ "worker", "Worker", XtRString, sizeof(String),
		XtOffset(app_resp_t, worker), XtRString, "mtile" },
	{ "monochrome", "Monochrome", XtRBool, sizeof(Bool),
		XtOffset(app_resp_t, mono), XtRString, "off" },
	{ "bars", "Bars", XtRBool, sizeof(Bool),
		XtOffset(app_resp_t, bars), XtRString, "on" },
	{ "nWorkers", "NWorkers", XtRInt, sizeof(int),
		XtOffset(app_resp_t, n), XtRString, "-1" },
};

static XrmOptionDescRec knownargs[] = {
	{ "-mono", ".monochrome", XrmoptionNoArg, "on" },
	{ "+mono", ".monochrome", XrmoptionNoArg, "off" },
	{ "-worker", ".worker", XrmoptionSepArg, 0 },
	{ "-bars", ".bars", XrmoptionNoArg, "on" },
	{ "+bars", ".bars", XrmoptionNoArg, "off" },
	{ "-n", ".nWorkers", XrmoptionSepArg, 0 },
};

static XtCallbackRec callback[2] = { { 0, 0 }, { 0, 0 } };
static Arg args[16];

static XtActionsRec actbl[] = {
	{"pick", pick},
};

u_char ditclass[8][8] = {
	2,   130, 34,  162, 10,  138, 42,  170,
	194, 66,  226, 98,  202, 74,  234, 106,
	50,  178, 18,  146, 58,  186, 26,  154,
	242, 114, 210, 82,  250, 122, 218, 90,
	14,  142, 46,  174, 6,   134, 38,  166,
	206, 78,  238, 110, 198, 70,  230, 102,
	62,  190, 30,  158, 54,  182, 22,  150,
	254, 126, 222, 94,  246, 118, 214, 86
};


main(argc, argv)
	int argc;
	char **argv;
{
	int n;
	XGCValues xgcv;

	n = 0;
	fprintf(stdout," enter toplevel \n");	
	topLevel = XtAppInitialize(&context, "xep", 
			knownargs, XtNumber(knownargs), 
			&argc, argv, 
			fallbacks, 
			args, n);


	fprintf(stdout," exit toplevel \n");	
	if (argc > 1) {
		for (n = 1; n < argc; n++)
			fprintf(stderr, "unknown option <%s>\n", argv[n]);
		fputs("options:\n", stderr);
		fputs("  -worker filename  set calculation task name\n", stderr);
		fputs("  -/+mono           set/unset forced mono mode\n", stderr);
		fputs("  -/+bars           display/don't display processor bars\n", stderr);
		fputs("  -n count          set number of workers\n", stderr);
		exit(1);
	}

	XtGetApplicationResources(topLevel, (caddr_t)&app_res,
		res_list, XtNumber(res_list), 0, 0);

//	dobars = app_res.bars;
//	workerfile = app_res.worker;

	fprintf(stderr,"start worker\n");

 	start_workers();
	fprintf(stderr," exit start worker ppppp\n");

	XtAppAddActions(context, actbl, XtNumber(actbl));

	xDisp = XtDisplay(topLevel);
	xScrn = DefaultScreen(xDisp);
	xRootW = RootWindow(xDisp, xScrn);

	crossCr = XCreateFontCursor(xDisp, XC_tcross);
	setup_color(app_res.mono);

	xgcv.function = GXxor;
	xgcv.background = BlackPixel(xDisp, xScrn);
	xgcv.foreground = ~(~0 << nPlanes);
	rubGc = XCreateGC(xDisp, xRootW,
			GCBackground|GCForeground|GCFunction, &xgcv);

	xgcv.function = GXcopy;
	xgcv.background = BlackPixel(xDisp, xScrn);
	xgcv.foreground = WhitePixel(xDisp, xScrn);
	canGc = XCreateGC(xDisp, xRootW,
			GCBackground|GCForeground|GCFunction, &xgcv);
	printf("create\n");
	create_xep_widget();

	XtAppMainLoop(context);

}

/*
*	start workers
*	open tuple spaces for communication
*
*/
start_workers()
{
	printf(" Inside START WORKER \n");
	tsd = cnf_open("coord",0);
	res = cnf_open("color",0);
	fd = fopen("xep_stat.s93","a");
	fd2 = fopen("xep_cap.s93","a");
 
	
	f=(float)cnf_getf();
	f = f/100;
	t = cnf_gett();
	P = cnf_getP();
	printf(" exiting  START WORKER  \n");

}

/*	stop workers			
*	
*	terminates communication by closing the tuplespaces	
*/
stop_workers()
{
	fclose(fd);	
	fclose(fd2);	
	cnf_close(tsd);
	cnf_close(res);

}




setup_color(mono)
	int mono;
{
	int i, j;
	XColor colr;
	IBIT32 rbowr[NCMV], rbowg[NCMV], rbowb[NCMV];
	IBIT32 mbo = 0x04030201;	/* to test machine byte order */

	nPlanes = DefaultDepth(xDisp, xScrn);
	defVis = DefaultVisual(xDisp, xScrn);

	revByte = (ImageByteOrder(xDisp) == MSBFirst) ? 1 : 0;
	if (*(char*)&mbo == 4)
		revByte = !revByte;

	bimbo = BitmapBitOrder(xDisp);

	if (!mono && nPlanes > 1) {
		isMono = 0;

		if (defVis->class == TrueColor || defVis->class == DirectColor) {
			XPixmapFormatValues *pfv;
			int i;

			isCmap = 0;
			if (!(pfv = XListPixmapFormats(xDisp, &i))) {
				fprintf(stderr, "can't get pixmap format list for screen\n");
				exit(1);
			}
			while (--i >= 0)
				if (pfv[i].depth == nPlanes) break;
			if (i < 0) {
				fprintf(stderr, "no pixmap format matches screen depth?\n");
				exit(1);
			}
			xBpp = pfv[i].bits_per_pixel;
			xBypp = xBpp / 8;
			redMask = defVis->red_mask;
			redShift = ffs(redMask & ~(redMask >> 1)) - 8;
			greenMask = defVis->green_mask;
			greenShift = ffs(greenMask & ~(greenMask >> 1)) - 8;
			blueMask = defVis->blue_mask;
			blueShift = ffs(blueMask & ~(blueMask >> 1)) - 8;
			mkrbow(fclutr, fclutg, fclutb, 256);

		} else {
			isCmap = 1;
			xCmap = DefaultColormap(xDisp, xScrn);
			j = 17 - ffs(NCMV);
/*
			for (i = 0; i < NCMV; i++) {
				colr.red = colr.green = colr.blue = i << j;
				if (!XAllocColor(xDisp, xCmap, &colr))
					goto forcebw;
				cmapInd[i] = colr.pixel;
			}
*/
			mkrbow(rbowr, rbowg, rbowb, NCMV);
			for (i = 0; i < NCMV; i++) {
				colr.red = rbowr[i] << j;
				colr.green = rbowg[i] << j;
				colr.blue = rbowb[i] << j;
				if (!XAllocColor(xDisp, xCmap, &colr))
					goto forcebw;
				cmapInd[i] = colr.pixel;
			}
		}
		return;
	}

forcebw:
	isMono = 1;
	fputs("display is binary, it's not gonna look great...\n", stderr);
}


/*	mkrbow()
*
*	Generate a rainbow lut.  0 is black, n-1 is white, and entries
*	between those two go through the spectrum from red to violet.
*	Output values are from 0 to n-1.
*/

mkrbow(r, g, b, n)
	IBIT32 *r, *g, *b;	/* red, grn, blu lut return */
	int n;				/* number of entries (length of r, g, b) */
{
	int i, j;
	double d, e;

	for (i = 1; i < n - 1; i++) {
		j = n - 1 - i;
		d = (d = cos((double)((j - n * 0.16) * (3.1415926535 / n)))) < 0.0
			? 0.0 : d;
		b[i] = d * n;
		d = (d = cos((double)((j - n * 0.52) * (3.1415926535 / n)))) < 0.0
			? 0.0 : d;
		g[i] = d * n;
		d = (d = cos((double)((j - n * .83) * (3.1415926535 / n)))) < 0.0
			? 0.0 : d;
		e = (e = cos((double)(j * (3.1415926535 / n)))) < 0.0
			? 0.0 : e;
		r[i] = d * n + e * (n / 2);
	}
	r[i] = g[i] = b[i] = i;
	r[0] = g[0] = b[0] = 0;
}


void
quit_cb(wgt, cli, cd)
	Widget wgt;
	caddr_t cli;
	caddr_t cd;
{
	tplength = 7*8;
	ituple[0] = (float) -1;
	sprintf(tpname, "i%d\0",imCan.cn_ht + 1);
 	tpnamsz = strlen(tpname);
	status = cnf_tsput(tsd,tpname,ituple,tplength);	
	stop_workers();
	cnf_term();
	exit(0);
}


void
zout_cb(wgt, cli, cd)
	Widget wgt;
	caddr_t cli;
	caddr_t cd;
{
	double d;

	if (redoing)
		return;
	redoing = 1;

	d = (imCan.cn_re2 - imCan.cn_re1) / 2;
	imCan.cn_re1 -= d;
	imCan.cn_re2 += d;
	d = (imCan.cn_im2 - imCan.cn_im1) / 2;
	imCan.cn_im1 -= d;
	imCan.cn_im2 += d;

	splat_out(imCan.cn_dat, imCan.cn_wd, imCan.cn_ht, 1);

	if(!COLOR)
		repaint_region(&imCan, 0, 0, imCan.cn_wd - 1, imCan.cn_ht - 1);
	refresh_region(&imCan, 0, 0, imCan.cn_wd - 1, imCan.cn_ht - 1); 
	imCan.cn_x1 = 0;
	imCan.cn_y1 = 0;
	imCan.cn_x2 = 0;
	imCan.cn_y2 = 0;
	
	start_recalc();
}


void
redo_cb(wgt, cli, cd)
	Widget wgt;
	caddr_t cli;
	caddr_t cd;
{
	double re1, im1, re2, im2;
	int wd = imCan.cn_wd;
	int ht = imCan.cn_ht;
	int x1 = imCan.cn_x1;
	int y1 = imCan.cn_y1;
	int x2 = imCan.cn_x2;
	int y2 = imCan.cn_y2;
	int t;

	if (x1 != x2 && y1 != y2) {

		if (redoing)
			return;
		redoing = 1;

		if (x1 > x2) {
			t = x1;
			x1 = x2;
			x2 = t;
		}
		if (y1 > y2) {
			t = y1;
			y1 = y2;
			y2 = t;
		}
		re1 = imCan.cn_re1 + x1 * (imCan.cn_re2 - imCan.cn_re1) / wd;
		im1 = imCan.cn_im1 + y1 * (imCan.cn_im2 - imCan.cn_im1) / ht;
		re2 = re1 + (x2 - x1) * (imCan.cn_re2 - imCan.cn_re1) / wd;
		im2 = im1 + (x2 - x1) * (imCan.cn_im2 - imCan.cn_im1) / wd;
		imCan.cn_re1 = re1;
		imCan.cn_im1 = im1;
		imCan.cn_re2 = re2;
		imCan.cn_im2 = im2;

		splat_out(imCan.cn_dat, imCan.cn_wd, imCan.cn_ht, 1);
		if(!COLOR)
			repaint_region(&imCan, 0, 0, imCan.cn_wd - 1, imCan.cn_ht - 1); 
		refresh_region(&imCan, 0, 0, imCan.cn_wd - 1, imCan.cn_ht - 1);  
		imCan.cn_x1 = 0;
		imCan.cn_y1 = 0;
		imCan.cn_x2 = 0;
		imCan.cn_y2 = 0;
		
		start_recalc();
	}
}


void
bars_cb(wgt, cli, cd)
	Widget wgt;
	caddr_t cli;
	caddr_t cd;
{
	if(redoing)
		return;
 
		redoing = 1;
		refresh_region(&imCan,0,0,imCan.cn_wd - 1,imCan.cn_ht - 1);	
		redoing = 0;

}


void
workers_cb(wgt, cli, cd)
	Widget wgt;
	caddr_t cli;
	caddr_t cd;
{
	if (redoing)
		return;

	stop_workers();
	start_workers();
}


create_xep_widget()
{
	Widget box;
	Widget w;
	int n;
	char buf[128];
	Pixmap pm;

	n = 0;
	box = XtCreateManagedWidget("", formWidgetClass, topLevel, args, n);

	n = 0;
	XtSetArg(args[n], XtNleft, XtChainLeft); n++;
	XtSetArg(args[n], XtNright, XtChainLeft); n++;
	XtSetArg(args[n], XtNtop, XtChainTop); n++;
	XtSetArg(args[n], XtNbottom, XtChainTop); n++;
	callback[0].callback = quit_cb;
	callback[0].closure = (caddr_t)0;
	XtSetArg(args[n], XtNcallback, callback); n++;
	pm = XCreatePixmapFromBitmapData(xDisp, xRootW,
			quit_bits, quit_width, quit_height, 1, 0, 1);
	XtSetArg(args[n], XtNbitmap, (XtArgVal)pm); n++;
	w =  XtCreateManagedWidget("quitButton", commandWidgetClass,
			box, args, n);

	n = 0;
	XtSetArg(args[n], XtNleft, XtChainLeft); n++;
	XtSetArg(args[n], XtNright, XtChainLeft); n++;
	XtSetArg(args[n], XtNtop, XtChainTop); n++;
	XtSetArg(args[n], XtNbottom, XtChainTop); n++;
	callback[0].callback = workers_cb;
	callback[0].closure = (caddr_t)0;
	XtSetArg(args[n], XtNcallback, callback); n++;
	XtSetArg(args[n], XtNfromHoriz, w); n++;
	pm = XCreatePixmapFromBitmapData(xDisp, xRootW,
			neww_bits, neww_width, neww_height, 1, 0, 1);
	XtSetArg(args[n], XtNbitmap, (XtArgVal)pm); n++;
	w =  XtCreateManagedWidget("workersButton", commandWidgetClass,
			box, args, n);

	n = 0;
	XtSetArg(args[n], XtNleft, XtChainLeft); n++;
	XtSetArg(args[n], XtNright, XtChainLeft); n++;
	XtSetArg(args[n], XtNtop, XtChainTop); n++;
	XtSetArg(args[n], XtNbottom, XtChainTop); n++;
	callback[0].callback = redo_cb;
	callback[0].closure = (caddr_t)0;
	XtSetArg(args[n], XtNcallback, callback); n++;
	XtSetArg(args[n], XtNfromHoriz, w); n++;
	pm = XCreatePixmapFromBitmapData(xDisp, xRootW,
			redo_bits, redo_width, redo_height, 1, 0, 1);
	XtSetArg(args[n], XtNbitmap, (XtArgVal)pm); n++;
	w =  XtCreateManagedWidget("recalcButton", commandWidgetClass,
			box, args, n);

	n = 0;
	XtSetArg(args[n], XtNleft, XtChainLeft); n++;
	XtSetArg(args[n], XtNright, XtChainLeft); n++;
	XtSetArg(args[n], XtNtop, XtChainTop); n++;
	XtSetArg(args[n], XtNbottom, XtChainTop); n++;
	callback[0].callback = zout_cb;
	callback[0].closure = (caddr_t)0;
	XtSetArg(args[n], XtNcallback, callback); n++;
	XtSetArg(args[n], XtNfromHoriz, w); n++;
	pm = XCreatePixmapFromBitmapData(xDisp, xRootW,
			zout_bits, zout_width, zout_height, 1, 0, 1);
	XtSetArg(args[n], XtNbitmap, (XtArgVal)pm); n++;
	w =  XtCreateManagedWidget("zoomoutButton", commandWidgetClass,
			box, args, n);

	n = 0;
	XtSetArg(args[n], XtNleft, XtChainLeft); n++;
	XtSetArg(args[n], XtNright, XtChainLeft); n++;
	XtSetArg(args[n], XtNtop, XtChainTop); n++;
	XtSetArg(args[n], XtNbottom, XtChainTop); n++;
	callback[0].callback = bars_cb;
	callback[0].closure = (caddr_t)0;
	XtSetArg(args[n], XtNcallback, callback); n++;
	XtSetArg(args[n], XtNfromHoriz, w); n++;
	XtSetArg(args[n], XtNstate, dobars); n++;
	pm = XCreatePixmapFromBitmapData(xDisp, xRootW,
			bars_bits, bars_width, bars_height, 1, 0, 1);
	XtSetArg(args[n], XtNbitmap, (XtArgVal)pm); n++;
	w =  XtCreateManagedWidget("barsButton", toggleWidgetClass,
			box, args, n);

	imCan.cn_re1 = -2.0;
	imCan.cn_im1 = -2.0;
	imCan.cn_re2 = 2.0;
	imCan.cn_im2 = 2.0;

	imCan.cn_wd = 600;
	imCan.cn_ht = 600;
	imCan.cn_x1 = 0;
	imCan.cn_y1 = 0;
	imCan.cn_x2 = 600;
	imCan.cn_y2 = 600;
	imCan.cn_zoom = 1;
	imCan.cn_ox = 0;
	imCan.cn_oy = 0;
	imCan.cn_dat = TALLOC(imCan.cn_wd * imCan.cn_ht, u_char);
	splat_out(imCan.cn_dat, imCan.cn_wd, imCan.cn_ht, 1);
	cre_xim(&imCan);
	if(!COLOR)
		repaint_region(&imCan, 0, 0, imCan.cn_wd - 1, imCan.cn_ht - 1);

	n = 0;
	XtSetArg(args[n], XtNtop, XtChainTop); n++;
	XtSetArg(args[n], XtNright, XtChainRight); n++;
	XtSetArg(args[n], XtNwidth, imCan.cn_wd); n++;
	XtSetArg(args[n], XtNheight, imCan.cn_ht); n++;
	XtSetArg(args[n], XtNfromVert, w); n++;
	imCan.cn_wgt = XtCreateManagedWidget("canvas", widgetClass,
		box, args, n);

	XtRealizeWidget(topLevel);

	imCan.cn_win = XtWindow(imCan.cn_wgt);
	XDefineCursor(xDisp, imCan.cn_win, crossCr);

	XtAddEventHandler(imCan.cn_wgt, StructureNotifyMask|ExposureMask,
		False, canvas_ev, 0);
}


splat_out(ba, wd, ht, dir)
	u_char *ba;
	int wd, ht;
	int dir;
{
	int x = wd;
	int o = 0;
	int n = wd * ht;

	dir = dir ? 7 : 1;
	while (--n >= 0) {
		*ba++ = x & 7 ? 0 : 255;
		if (--x <= o) {
			o = (o + dir) & 7;
			x = wd + o;
		}
	}
}


void
canvas_ev(wgt, cli, xev, rem)
	Widget wgt;
	XtPointer cli;
	XEvent *xev;
	Boolean *rem;
{
	int wd, ht;				/* size of new canvas */
	double re1, im1, re2, im2;
	int ox, oy;				/* offset of new image in old */
	u_char *dat;			/* new canvas image */
	u_char *b1, *b2;		/* for copying image data */
	int w, h;
	

	switch (xev->type) {

	case ConfigureNotify:
		wd = xev->xconfigure.width;
		ht = xev->xconfigure.height;

		ox = (imCan.cn_wd - wd) / 2;
		re1 = imCan.cn_re1
				+ ((imCan.cn_re2 - imCan.cn_re1) * ox) / imCan.cn_wd;
		re2 = imCan.cn_re1
				+ ((imCan.cn_re2 - imCan.cn_re1) * (ox + wd)) / imCan.cn_wd;
		oy = (imCan.cn_ht - ht) / 2;
		im1 = imCan.cn_im1
				+ ((imCan.cn_re2 - imCan.cn_re1) * oy) / imCan.cn_wd;
		im2 = imCan.cn_im1
				+ ((imCan.cn_re2 - imCan.cn_re1) * (oy + ht)) / imCan.cn_wd;
		imCan.cn_re1 = re1;
		imCan.cn_re2 = re2;
		imCan.cn_im1 = im1;
		imCan.cn_im2 = im2;
/*
	printf("%dx%d  %e,%e  %e,%e\n", wd, ht, re1, im1, re2, im2);
	printf("%e : %e\n", (re2 - re1) / wd, (im2 - im1) / ht);
*/

		dat = TALLOC(wd * ht, u_char);
		splat_out(dat, wd, ht, 0);

		w = min(imCan.cn_wd, wd);
		h = min(imCan.cn_ht, ht);
		b1 = imCan.cn_dat + max(0, ox) + max(0, oy) * imCan.cn_wd;
		b2 = dat + max(0, -ox) + max(0, -oy) * wd;
		while (h-- > 0) {
			BCOPY(b1, b2, w);
			b1 += imCan.cn_wd;
			b2 += wd;
		}
		free(imCan.cn_dat);

		imCan.cn_dat = dat;
		imCan.cn_wd = wd;
		imCan.cn_ht = ht;
		imCan.cn_x1 = 0;
		imCan.cn_y1 = 0;
		imCan.cn_x2 = 0;
		imCan.cn_y2 = 0;
		cre_xim(&imCan);
		if(!COLOR)
			repaint_region(&imCan, 0, 0, imCan.cn_wd - 1, imCan.cn_ht - 1);
		refresh_region(&imCan, 0, 0, imCan.cn_wd - 1, imCan.cn_ht - 1);  
		break;

	case Expose:
		{
			static int once = 1;

			if (once) {
				redo_cb((Widget)0, (caddr_t)0, (caddr_t)0);
				once = 0;
			}
		}
		rubbox(imCan.cn_win, imCan.cn_x1, imCan.cn_y1,
				imCan.cn_x2, imCan.cn_y2);
		refresh_region(&imCan, xev->xexpose.x, xev->xexpose.y,
				xev->xexpose.x + xev->xexpose.width - 1,
				xev->xexpose.y + xev->xexpose.height - 1);   
		rubbox(imCan.cn_win, imCan.cn_x1, imCan.cn_y1,
				imCan.cn_x2, imCan.cn_y2);
		break;

	default:
		break;
	}
}


/*	pick()
*
*	s - start
*	a - adjust
*	e - end
*	m - modify
*/

void
pick(wgt, xev, par, nump)
	Widget wgt;
	XEvent *xev;
	String *par;
	int *nump;
{
	Window w = XtWindow(wgt);
	int wd = imCan.cn_wd;
	int ht = imCan.cn_ht;
	int x1 = imCan.cn_x1;
	int y1 = imCan.cn_y1;
	int x2 = imCan.cn_x2;
	int y2 = imCan.cn_y2;
	int x, y;

	if (*nump == 1 && get_event_xy(xev, &x, &y)) {
		switch (par[0][0]) {

		case 's':
			if (x1 != -1) 
				rubbox(w, x1, y1, x2, y2);
			x2 = x1 = x;
			y2 = y1 = y;
			printf(" X1( %d) Y1( %d  )\n", x1, y1);
			rubbox(w, x1, y1, x2, y2); 

			break;

		case 'a':
			  if (x1 != -1) {
				rubbox(w, x1, y1, x2, y2);
				x2 = (abs(y - y1) * wd) / ht;
				x2 = x - x1 > 0 ? x1 + x2 : x1 - x2;
				y2 = (abs(x - x1) * ht) / wd;
				y2 = y - y1 > 0 ? y1 + y2 : y1 - y2;
				if (abs(x2 - x1) > abs(x - x1))
					y2 = y;
				else
					x2 = x;
				rubbox(w, x1, y1, x2, y2);
				printf(" X2( %d ) Y2( %d )\n", x2, y2);
			}
			
			break;

		case 'e':
			if (x1 != -1) {
				x2 = (abs(y - y1) * wd) / ht;
				x2 = x - x1 > 0 ? x1 + x2 : x1 - x2;
				y2 = (abs(x - x1) * ht) / wd;
				y2 = y - y1 > 0 ? y1 + y2 : y1 - y2;
				if (abs(x2 - x1) > abs(x - x1))
					y2 = y;
				else
					x2 = x;

				printf(" X1( %d ) Y1( %d ) <--> X2( %d ) Y2( %d )\n", x1, y1, x2, y2);

			}
			break;

		case 'm':
			if (x1 != -1) {

				printf(" X1( %d) Y1( %d  )\n", x1, y1);
				rubbox(w, x1, y1, x2, y2);
				if( (x > h_base) && ((wd - x) > h_base) && (y > h_base) && ((ht - y) > h_base) ) {
				x1 = x - h_base;
				y1 = y - h_base;
				x2 = x + h_base;
				y2 = y + h_base;

				rubbox(w, x1, y1, x2, y2);
				printf(" X( %d) %d, %d -> %d, %d\n", x, x1, y1, x2, y2);
				} else {
					x1 = x2;
					y1 = y2;
				}			
				printf(" X1( %d ) Y1( %d ) <--> X2( %d ) Y2( %d )\n", x1, y1, x2, y2);

		/*		rubbox(w, x1, y1, x2, y2);
				if (abs(x - x1) < abs(x - x2))
					x1 = x2;
				if (abs(y - y1) < abs(y - y2))
					y1 = y2;
				x2 = (abs(y - y1) * wd) / ht;
				x2 = x - x1 > 0 ? x1 + x2 : x1 - x2;
				y2 = (abs(x - x1) * ht) / wd;
				y2 = y - y1 > 0 ? y1 + y2 : y1 - y2;
				if (abs(x2 - x1) > abs(x - x1))
					y2 = y;
				else
					x2 = x;
				rubbox(w, x1, y1, x2, y2);
	   			printf("%d, %d -> %d, %d\n", x1, y1, x2, y2);
		*/
			}
			break;

		default:
			break;
		}
		imCan.cn_x1 = x1;
		imCan.cn_y1 = y1;
		imCan.cn_x2 = x2;
		imCan.cn_y2 = y2;
	}
}


int
get_event_xy(xev, xp, yp)
	XEvent *xev;
	int *xp, *yp;
{
	switch (xev->type) {
	case KeyPress:
	case KeyRelease:
		*xp = xev->xkey.x;
		*yp = xev->xkey.y;
		return 1;
		break;

	case ButtonPress:
	case ButtonRelease:
		*xp = xev->xbutton.x;
		*yp = xev->xbutton.y;
		return 1;
		break;

	case MotionNotify:
		*xp = xev->xmotion.x;
		*yp = xev->xmotion.y;
		return 1;
		break;

	case EnterNotify:
	case LeaveNotify:
		*xp = xev->xcrossing.x;
		*yp = xev->xcrossing.y;
		return 1;
		break;

	default:
		return 0;
	}
}


/*	rubbox()
*
*	Draw a rubberband box XOR in a window.  Call again to undraw.
*/

rubbox(w, x1, y1, x2, y2)
	Window w;
	int x1, x2, y1, y2;
{
	register int x, y, h, v;

	if (x1 < x2)
		{ x = x1; h = x2 - x1; }
	else
		{ x = x2; h = x1 - x2; }
	if (y1 < y2)
		{ y = y1; v = y2 - y1; }
	else
		{ y = y2; v = y1 - y2; }

	XDrawRectangle(xDisp, w, rubGc, x, y, h, v);
}


/*	sw4()
*
*	Byteswap a 32-bit integer.
*/

IBIT32
sw4(x)
	IBIT32 x;
{
	u_char *cp = (u_char*)&x;
	u_char c;

	c = cp[0];
	cp[0] = cp[3];
	cp[3] = c;
	c = cp[1];
	cp[1] = cp[2];
	cp[2] = c;
	return x;
}


/*	repaint_region()
*
*	Replot XImage from its underlying image.
*/

repaint_region(cnp, x1, y1, x2, y2)
	struct canvas *cnp;
	int x1, y1, x2, y2;				/* image coordinates */
{
	char *ximbuf;					/* ximage data */
	u_char *ribuf = cnp->cn_dat;	/* src image data */
	int sx, sy;						/* source x, y counter */
	int dx, dy;						/* dest x, y counter */
	int xa, ya;
	unsigned char *sa;
	char *da;
	IBIT32 pixv;
	IBIT32 pixr, pixg, pixb;
	int lbs;
	int lbm;

	if (x1 < cnp->cn_ox)
		x1 = cnp->cn_ox;
	if (y1 < cnp->cn_oy)
		y1 = cnp->cn_oy;
	if (++x2 > cnp->cn_wd)
		x2 = cnp->cn_wd;
	if (++y2 > cnp->cn_ht)
		y2 = cnp->cn_ht;

	ximbuf = cnp->cn_xim->data;
	if (!isMono) {
		if (isCmap) {	/* display is colormapped */
			lbs = 8 - ffs(NCMV) + 1;	/* shift make byte into cmap depth */
			lbm = 0xff >> lbs;			/* mask significant */
			sy = y1;
			dy = (y1 - cnp->cn_oy) * cnp->cn_zoom;
			ya = 0;
			while (sy < y2 && dy < cnp->cn_ht) {
				sx = x1;
				dx = (x1 - cnp->cn_ox) * cnp->cn_zoom;
				xa = 0;
				sa = ribuf + sy * cnp->cn_wd + sx;
				da = ximbuf + dy * cnp->cn_xim->bytes_per_line + dx;
				while (sx < x2 && dx < cnp->cn_wd) {
					*da = cmapInd[lbm & (*sa >> lbs)];
					da++;
					dx++;
					if (++xa >= cnp->cn_zoom) {
						xa = 0;
						sx++;
						sa++;
					}
				}
				dy++;
				if (++ya >= cnp->cn_zoom) {
					ya = 0;
					sy++;
				}
			}

		} else {	/* display is true color */

			lbm = 0xff;

			sy = y1;
			dy = (y1 - cnp->cn_oy) * cnp->cn_zoom;
			ya = 0;
			while (sy < y2 && dy < cnp->cn_ht) {
				sx = x1;
				dx = (x1 - cnp->cn_ox) * cnp->cn_zoom;
				xa = 0;
				sa = ribuf + sy * cnp->cn_wd + sx;
				da = ximbuf + dy * cnp->cn_xim->bytes_per_line + dx * sizeof(IBIT32);
				while (sx < x2 && dx < cnp->cn_wd) {
					pixv = lbm & *sa;
					pixr = fclutr[pixv];
					pixg = fclutg[pixv];
					pixb = fclutb[pixv];
					pixv = (redMask & (pixr << redShift))
							| (greenMask & (pixg << greenShift))
							| (blueMask & (pixb << blueShift));
					if (revByte)
						*((IBIT32*)da) = sw4(pixv);
					else
						*((IBIT32*)da) = pixv;
		
					da += sizeof(IBIT32);
					dx++;
					if (++xa >= cnp->cn_zoom) {
						xa = 0;
						sx++;
						sa++;
					}
				}
				dy++;
				if (++ya >= cnp->cn_zoom) {
					ya = 0;
					sy++;
				}
			}
		}

	} else {	/* binary display  XXX region not done yet */
		u_char pxa;
		u_char msks[8];

		for (lbs = 0; lbs < 8; lbs++)
			if (bimbo == LSBFirst)
				msks[lbs] = 1 << lbs;
			else
				msks[7 - lbs] = 1 << lbs;
		sy = cnp->cn_oy;
		dy = 0;
		ya = 0;
		while (sy < cnp->cn_ht && dy < cnp->cn_ht) {
			sx = cnp->cn_ox;
			dx = 0;
			xa = 0;
			sa = ribuf + sy * cnp->cn_wd + sx;
			da = ximbuf + dy * cnp->cn_xim->bytes_per_line;
			pxa = 0;
			while (sx < cnp->cn_wd && dx < cnp->cn_wd) {
				if (*sa > ditclass[dx & 7][dy & 7])
					pxa |= msks[dx & 7];
				if (!(++dx & 7)) {
					*da++ = pxa;
					pxa = 0;
				}
				if (++xa >= cnp->cn_zoom) {
					xa = 0;
					sx++;
					sa++;
				}
			}
			if (dx & 7)
				*da++ = pxa;
			dy++;
			if (++ya >= cnp->cn_zoom) {
				ya = 0;
				sy++;
			}
		}
	}
}


/*	cre_xim()
*
*	Create (or change) XImage for a canvas.
*/

cre_xim(cnp)
	struct canvas *cnp;
{
	char *ximbuf;
	int wd = cnp->cn_wd;
	int ht = cnp->cn_ht;
	int nb;

	if (cnp->cn_xim)
		XDestroyImage(cnp->cn_xim);

	if (!isMono) {
		nb = wd * ht;
		if (!isCmap) {
			ximbuf = TALLOC(nb * xBypp, char);
			cnp->cn_xim = XCreateImage(xDisp, defVis, nPlanes, ZPixmap, 0,
				ximbuf, wd, ht, xBpp, wd * xBypp);

		} else {
			ximbuf = TALLOC(nb, char);

			cnp->cn_xim = XCreateImage(xDisp, defVis, nPlanes, ZPixmap, 0,
				ximbuf, wd, ht, 8, wd);
		}

	} else {
		nb = byteswide(wd) * ht;
		ximbuf = TALLOC(nb, char);

		cnp->cn_xim = XCreateImage(xDisp, defVis, 1, XYBitmap, 0,
			ximbuf, wd, ht, 8, byteswide(wd));
	}
}


/*	refresh_region()
*
*	Refresh a canvas window.
*/

refresh_region(cnp, x1, y1, x2, y2)
	struct canvas *cnp;
	int x1, y1, x2, y2;
{
	int lx,ly,wd;
	 wd = y2-y1+1;
        if(!COLOR) {
        //        printf("enter  x1(%d) y1(%d) x2(%d) y2(%d)  \n",x1,y1,x2,y2);
                XPutImage(xDisp, cnp->cn_win, canGc, cnp->cn_xim,
                        x1, y1, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
        //        printf("exit  x1(%d) y1(%d) x2(%d) y2(%d)  \n",x1,y1,x2,y2);
        } else {
                for(ly = y1; ly <= y2+1; ly++) {
                        for(lx = x1; lx <= x2+1;lx++) {
                                XSetForeground(xDisp,canGc,cnp->cn_dat[ly * wd + lx]);
                                XDrawPoint(xDisp,cnp->cn_win,canGc,lx,ly);
                        }
                }
        }


}



/*	start_recalc()
*
*	Start calculation by sending off tiles.
*	Scheduling used : factoring
*/

start_recalc()
{
	int ht = imCan.cn_ht;
	int wd = imCan.cn_wd;
	int y1, y2;
	int i,j;

	xmin = imCan.cn_re1;
	xmax = imCan.cn_re2;
	ymin = imCan.cn_im1;
	ymax = imCan.cn_im2;
	
	xstep = (xmax - xmin) / (double)wd;
	ystep = (ymax - ymin) / (double)ht;
	y = ymin;

	tplength = 7*8;
	iterat = 1024;
	R = wd;
	iy = 0;
	G = 1;
	t0 = time((long *)0);

	while ( (R > t) && (G>0) ) {
		G = (int)R*f/P;
		R = R - G*P;
		if (G>0) {
		for(i=0;i<P;i++) {
			ituple[0] = (double) G;
			ituple[1] = (double) iy;
			ituple[2] = y;
			ituple[3] = xmin;
			ituple[4] = xstep;
			ituple[5] = ystep;
			ituple[6] = (float) iterat;		
			sprintf( tpname, "%d\0",iy);
			tpnamsz = strlen(tpname);
	
		//	 printf("put in tuple (%s) G(%d) iy(%d) y(%f) R(%d) \n",tpname,G,iy,y,R);

	 		status = cnf_tsput(tsd,tpname,ituple,tplength);
			y += G*xstep;
			iy += G;
		}
		}
	}

	if (R > 0) 
	{ 	
		ituple[0] = (double) R;
		ituple[1] = (double) iy;
		ituple[2] = y;
		ituple[3] = xmin;
		ituple[4] = xstep;
		ituple[5] = ystep;
		ituple[6] = (double) iterat;		
		sprintf( tpname, "%d\0",iy);
		tpnamsz = strlen(tpname);
		status = cnf_tsput(tsd,tpname,ituple,tplength);
		printf("put in tuple (%s) G(%d) iy(%d) y(%f) R(%d) \n",tpname,R,iy,y,R);
	}
 
/*		if(R>0) {
			ituple[0] = (float) 0;
			sprintf( tpname, "%d\0",wd);
			tpnamsz = strlen(tpname);
			status = cnf_tsput(tsd,tpname,ituple,tplength);
			printf("put in tuple (%s) G(%d) iy(%d) y(%f) R(%d) \n",tpname,G,iy,y,R);
		}	
*/

	received = 0;
	flops = 0;
	printf(" claim tiles\n");
	Wcount = 0; /* # of workers responded so far is 0 */
	
	while (received < ht)
		claim_tile();


	 t2 = time((long *)0); 
	
	flops = flops * 15;
	te = t1-t0;
	if (te > 0) rate = flops/(1000000 * te);
	else rate = 0.0;
	redoing = 0;
	printf("claimed all tiles \n");
	printf("TE1( %f ), TE2( %f ) P( %d ), f( %f )\n",(t1-t0),(t2-t0),P,f);

	fprintf(fd2,"TE( %f ) MF( %f ) P( %d ) f(%1.2f ) XRES( %d ) YRES( %d ) \
		 xmin( %f ) ymin( %f ) xmax(%f ) ymax( %f ) IT( %d ) COLOR( %d ) CAPTURE( %d ) \n",
		te/1000,rate,P,f,imCan.cn_wd,imCan.cn_ht,xmin,ymin,xmax,ymax,iterat,
		COLOR,CAPTURE);
	
	participants();
	
	if(CAPTURE) {
		Hamdahl();	
		fprintf(fd2,"-------------------------------------------------------------------------------\n");	
	}
}

claim_tile()
{
	int wd = imCan.cn_wd;
	int ht = imCan.cn_ht;
	int y1, y2;						/* tile start, end rows */
	int x1,x2;
	int j;							
	int lx,ly;
	int tplen;
	
	strcpy(tpname,"*\0");
	tpnamsz = strlen(tpname);
	status = cnf_tsget(res,tpname,&otuple,0);


	if(CAPTURE) {	 
		printf("received %s tuple from( %s ) \n",tpname,otuple.host);
		next_worker = search_host(work_list,otuple.host);
	}
	t1 = time((long *)0);
	
	G = (int) otuple.index[0];
	y1= (int) otuple.index[1];

	y2 = y1 + G;
	x1 = 0;
	x2 = XRES;
	
	tplen = (2 + G*XRES);
		temp = imCan.cn_dat + y1*wd;
		
		j=2;
		if(COLOR) { 
                	for(ly = y1; ly <= y2 - 1; ly++) {
                        	for(lx = x1; lx <= x2 - 1;lx++) {
 					*temp = (int)otuple.index[j];
					flops += (int)otuple.index[j];
					if(CAPTURE)		
						work_list[next_worker].density += (int)otuple.index[j];	
                                	XSetForeground(xDisp,canGc,*temp);
                                	XDrawPoint(xDisp,imCan.cn_win,canGc,lx,ly);
					temp++;
					j+=1;
                        	}
                	}
		} else {
			for(j = 2; j <= tplen; j++) { 
 			*temp = ~(int)otuple.index[j];
			flops += (int)otuple.index[j];
			if(CAPTURE)		
				work_list[next_worker].density += (int)otuple.index[j];	
			temp++;
			}
			repaint_region(&imCan, 0, y1, imCan.cn_wd - 1, y2 - 1); 
			refresh_region(&imCan, 0, y1, imCan.cn_wd - 1, y2 - 1); 
		}

	received += G;
}

int search_host(work_list,hostname)
struct wlist work_list[20];
char hostname[128];
{
	int id,done,r;
	
	id = 1;
	done = 0;
	while((id <= Wcount) && (done != 1)) {
		if(strcmp(work_list[id].worker,hostname) == 0) {
			done = 1;
			r = id;
		}
		id += 1;
	}
	
	if(!done) {
		strcpy(work_list[id].worker,hostname);
		Wcount += 1;
	 	r = Wcount;	
		work_list[next_worker].density = 0;
		}
	return r;	
}

/* print out the Worker names who participated */ 

void participants()
{ 
	int j;
	for(j=1;j<=Wcount;j++) {
		printf(" Worker ( %s ) has done ( %d ) # of operations\n",
			work_list[j].worker,work_list[j].density*15);
		fprintf(fd2," Worker ( %s ) has done ( %d ) # of operations\n",
			work_list[j].worker,work_list[j].density*15);
	}
}

/* Speedup calculation  */
void Hamdahl() 
{
	int i,Maxid,sigma_density,max_density;
	double BETA,speed_up;
	
	sigma_density = 0;
	max_density = -1;
	for(i=1;i<=Wcount;i++) {
		sigma_density += work_list[i].density;
		if(work_list[i].density > max_density) {
			max_density = work_list[i].density;
			Maxid = i;
		}
	}
	
	printf(" sigma_density ==> %d, max density %d  \n",sigma_density*15,max_density*15);
	max_density = max_density * 15;
	sigma_density = sigma_density *15;
	BETA = (double)max_density / (double)sigma_density;
	speed_up = (double) P / ( BETA * (double) P + (1 - BETA) );
	printf("beta( %f ) speed up( %f ) \n",BETA,speed_up);
	fprintf(fd2,"beta( %f ) speed up( %f ) \n",BETA,speed_up);
}
