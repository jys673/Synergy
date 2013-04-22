
/* From: kelvin@autodesk.com (John Walker)
 * Message-Id: <9206091233.AA22915@throop>
 * To: 73767.1244@uunet.uu.net
 * Subject: Extensions to XWIN.C module of POV-Ray

 * Attached is a modified version of the XWIN.C module included in the
 * BETA 0.5 distribution of POV-Ray.  While the display is still dithered for
 * a monochrome screen (I, too, have only a monochrome monitor on my X
 * machine), it now uses the Floyd-Steinberg error diffusion algorithm rather
 * than a fixed 8x8 dithering matrix, which results in *much* better looking
 * images.  There is one slight drawback to this algorithm; it requires that
 * you buffer an entire line of pixels before displaying them so the image
 * now appears line-by-line rather that dot-by-dot.  I don't find this
 * objectionable, and it does concentrate the display update context switches
 * into one burst rather than a steady drone.

 * This code also includes optional gamma correction, which you can enable
 * by defining the tag Gamma to the desired floating point value.  If Gamma is
 * undefined (as in the attached code), the gamma correction is entirely
 * disabled.

 * Thank you all for creating such a wonderful tool as POV-Ray.  I hope this
 * contribution proves useful.

 */
/*------------------------------------ Cut here ------------------------- xwin.c */
/****************************************************************************
*                xwindows.c
*
*  This module implements X-windows specific routines.
*
*  from Persistence of Vision Raytracer
*  Copyright 1993 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other 
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file. If 
*  POVLEGAL.DOC is not available or for more info please contact the POV-Ray
*  Team Coordinator by leaving a message in CompuServe's Graphics Developer's
*  Forum.  The latest version of POV-Ray may be found there as well.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
*****************************************************************************/


  /*
        Here are some routines I wrote which implement +d option on
        unix computers running X-windows. For now, only black
        and white output is supported. If I get access to a computer
        with a color monitor, I'll probably add support for colors to
        my routines.

        In future I'll probably add some more dithering methods.
        I have tested these routines on SUN 3 and SUN 4. I'm using
        the tvtwm window manager.

        If you have some suggestions to my source code or if you change
        anything, please let me know. I can be reached at the following
        address: Marek Rzewuski, Avstikkeren 11, 1156 Oslo 11, Norway or
        marekr@ifi.uio.no on Internet.

                                    * * *

        On  9th May 1992, I replaced the original 8x8 dithering matrix
        in this module with Floyd-Steinberg error diffusion mapping of
        the  YIQ-derived  grey  scale  into  a monochrome bitmap.  The
        Floyd-Steinberg code was based on the algorithm  used  in  the
        "pgmtopbm" module of Jef Poskanzer's raster toolkit (PBMPLUS).

        In addition, I added optional gamma correction (with a default
        factor  of  0.5),  to darken the picture somewhat and increase
        the level  of  perceived  detail.   The  gamma  correction  is
        disabled  in the standard version, but you can turn it back on
        by uncommenting the  declaration  of  Gamma  if  you  find  it
        improves the quality of the pictures you're making.
                -- John Walker
                   Autodesk Neuchatel
                   Internet: kelvin@Autodesk.com

*/

#include <stdio.h>
#include <X11/Xlib.h>           /* some Xlib stuff */
#include <X11/Xutil.h>
#include "theIcon"

#include "frame.h"
#include "povproto.h"

#define         BORDER_WIDTH    2
#define EV_MASK (ButtonPressMask | \
                 KeyPressMask    | \
                 ExposureMask    | \
                 StructureNotifyMask)

Display         *theDisplay;
int             theScreen;
int             theDepth;
unsigned long   theBlackPixel;
unsigned long   theWhitePixel;
XEvent          theEvent;
Window          theWindow, openWindow();
GC              theGC;
unsigned char   *bitmap;        /* pointer to our picture bitmap */
unsigned char   *bitmapPos;     /* position to the last drawn pixel in our bitm
ap */


/* global POV-Ray variables */

extern FRAME Frame;
extern unsigned int Options;
extern char DisplayFormat;
extern int First_Line;

enum pixval { BLACK, WHITE };

/* #define Gamma      0.5 */          /* Screen gamma correction factor */
#define Fthreshold 0.5

static unsigned char *scrline;
#ifdef Gamma
static unsigned char gammamap[256];
#endif
static enum pixval *pixrow;
static long *thiserr, *nexterr, *temperr;
static long threshval, sum;
static int ncols;                     /* Screen width */
#define FS_SCALE 1024
#define HALF_FS_SCALE 512
static int fs_direction;

void unix_init_POVRAY PARAMS ((void))
{
}


/* Sets up a connection to the X server and stores informations about the envir
oment */

initX()
{
  theDisplay = XOpenDisplay(NULL);
  if (theDisplay == NULL) {
    fprintf(stderr,"ERROR: Cannot establish a connection to the X server %s\n",
            XDisplayName(NULL));
    exit(1);
  }
  theScreen = DefaultScreen(theDisplay);
  theDepth  = DefaultDepth(theDisplay, theScreen);
  theWhitePixel = WhitePixel(theDisplay, theScreen);
  theBlackPixel = BlackPixel(theDisplay, theScreen);
}

/* This procedure will do the following things:
   1)   Set up attributes desired for the window
   2)   Set up an icon to our window.
   3)   Send hints to the window manager.
   4)   Open a window on the display
   5)   Tell the X to place the window on the screen
   6)   Flush out all the queued up X requests to the X server */

Window openWindow(x,y,width,height,flag,theNewGC)
int     x,y;
int     width,height;
int     flag;
GC      *theNewGC;
{
  XSetWindowAttributes  theWindowAttributes;
  XSizeHints            theSizeHints;
  unsigned      long    theWindowMask;
  Window                theNewWindow;
  Pixmap                theIconPixmap;
  XWMHints              theWMHints;


  /* Set up some attributes for the window. Override_redirect tells
     the window manager to deal width the window or leave it alone */

  theWindowAttributes.border_pixel      = theBlackPixel;
  theWindowAttributes.background_pixel  = theWhitePixel;
  theWindowAttributes.override_redirect = False;
  theWindowMask = CWBackPixel | CWBorderPixel | CWOverrideRedirect;

  /* Now, open out window */

  theNewWindow = XCreateWindow(theDisplay,
                               RootWindow(theDisplay,theScreen),
                               x,y,
                               width, height,
                               BORDER_WIDTH,
                               theDepth,
                               InputOutput,
                               CopyFromParent,
                               theWindowMask,
                               &theWindowAttributes);

  /* Create one iconbitmap */

  theIconPixmap = XCreateBitmapFromData(theDisplay,
                                        theNewWindow,
                                        theIcon_bits,
                                        theIcon_width,
                                        theIcon_height);

  /* Now tell the window manager where on screen we should place our
     window. */

  theWMHints.icon_pixmap        = theIconPixmap;
  theWMHints.initial_state      = NormalState;          /* we don't want an ico
nized window when it's created */
  theWMHints.flags              = IconPixmapHint | StateHint;

  XSetWMHints(theDisplay,theNewWindow,&theWMHints);

  theSizeHints.flags            = PPosition | PSize;
  theSizeHints.x                = x;
  theSizeHints.y                = y;
  theSizeHints.width            = width;
  theSizeHints.height           = height;
 /* theSizeHints.min_width      = width;
  theSizeHints.min_height       = height;
  theSizeHints.max_width        = width;
  theSizeHints.max_height       = height; */

  XSetNormalHints(theDisplay,theNewWindow,&theSizeHints);


  if (createGC(theNewWindow, theNewGC) == 0) {
    XDestroyWindow(theDisplay, theNewWindow);
    return((Window) 0);
  }

  /* Make a name for our window */

  XStoreName(theDisplay, theNewWindow, "Persistence Of Vision Raytracer\0");

  /* Now, could we please see the window on the screen?
     Until now, we have dealt with a window which has been created
     but not appeared on the screen. Maping the window places it visibly

     on the screen */

  XMapWindow(theDisplay,theNewWindow);
  XFlush(theDisplay);
  return(theNewWindow);
}

refreshWindow(theExposedWindow)
Window  theExposedWindow;
{
  int   i, x, y;
  unsigned char *dummy;
  dummy = bitmap;
  i = 0; x= 0; y = First_Line;
  while (dummy < bitmapPos) {
    if (*dummy)
      XDrawPoint(theDisplay, theWindow, theGC,x,y);
    if (x == Frame.Screen_Width) {
      x = 0;
      y++;
    } else {
      dummy++;
      x++;
      i++;
    }
  }
  XFlush(theDisplay);
}

/* Creates a new graphics context */

createGC(theNewWindow, theNewGC)
Window  theNewWindow;
GC      *theNewGC;
{
  XGCValues theGCValues;
  *theNewGC = XCreateGC(theDisplay,
                        theNewWindow,
                        (unsigned long) 0,
                        &theGCValues);

  if (*theNewGC == 0) {
    return(0); /*error*/
  } else { /* set foreground and background defaults for the new GC */
    XSetForeground(theDisplay,
                   *theNewGC,
                   theBlackPixel);

    XSetBackground(theDisplay,
                  *theNewGC,
                  theWhitePixel);

    return(1); /* OK */
  }
}

initEvents(theWindow)
Window theWindow;
{
  XSelectInput(theDisplay,
               theWindow,
               EV_MASK);
}

void display_finished ()
{
}

void display_init ()
{
  int i;
  int cols;

  /* Allocate error propagation vectors for Floyd-Steinberg algorithm. */

  ncols = Frame.Screen_Width;
  cols = ncols + 2;
  scrline = (unsigned char *) malloc(ncols * sizeof(unsigned char));
  pixrow = (enum pixval *) malloc(ncols * sizeof(enum pixval));
  thiserr = (long *) malloc(cols * sizeof(long));
  nexterr = (long *) malloc(cols * sizeof(long));
  if (scrline == NULL || pixrow == NULL ||
      thiserr == NULL || nexterr == NULL) {
    printf("XWIN error: Failed to allocate Floyd-Steinberg error vectors.\n");
    exit(0);
  }
  srandom((int) (time(NULL) ^ getpid()));
  /* (Random errors in [-FS_SCALE/8 .. FS_SCALE/8]) */
  for (i = 0; i < cols; i++) {
      thiserr[i] = (random() % FS_SCALE - HALF_FS_SCALE) / 4;
  }
  fs_direction = 1;
  threshval = Fthreshold * FS_SCALE;

  /* Initialise gamma correction table. */

#ifdef Gamma
  for (i = 0; i < 256; i++) {
    int gc = 255.0 * pow((((double) i) / 255.0), (1.0 / Gamma)) + 0.5;

    gammamap[i] = (gc > 255) ? 255 : gc;
  }
#endif

  /* Set some room for a bitmap for our picture.  I've got to
     "remember" the whole picture because of resizing of the window,
     overlapping etc.  Then I've got to refresh the picture.  This
     should be easy to convert to an "color version" in future */

  bitmap = (unsigned char *) malloc(sizeof(unsigned char) *
                                 (Frame.Screen_Width * Frame.Screen_Height));
  bitmapPos = bitmap;
  if (bitmap == NULL) {
    printf("XWIN error: Can not allocate the buffer..\n");
    exit(0);
  }

  for (i = 0; i < (Frame.Screen_Width*Frame.Screen_Height); i++) {
    *bitmapPos++ = 0;
  }
  bitmapPos = bitmap;
  initX();
  theWindow = openWindow(0,0,Frame.Screen_Width,Frame.Screen_Height,0,&theGC);
  initEvents(theWindow);
  XFlush(theDisplay);
#ifdef NEEDED
  /* On my Sun SPARCStation 2 running OpenWindows 3.0, this XNextEvent call
     sometimes makes the window go to sleep until some event is artificially
     generated for it (for example, by clicking the mouse in it).  I removed
     the call and everything seems to work just fine.  -- JW */
  XNextEvent(theDisplay,&theEvent);
  XFlush(theDisplay);
#endif
} /* end of display initialisation */

void display_close ()
{
  sleep(10);                            /* an simple delay. 10 seconds. */
  XDestroyWindow(theDisplay,theWindow);
  XFlush(theDisplay);
  XCloseDisplay(theDisplay);
  free(bitmap);
}

void display_plot (x, y, Red, Green, Blue)
int x, y;
unsigned char Red, Green, Blue;
{
  int                   numEvents;
  /* lets find if there are some events waiting for us */

  numEvents = XPending(theDisplay);
  if (numEvents > 0) {                  /* now deal with the events.. */
    XNextEvent(theDisplay,&theEvent);

    switch (theEvent.type) {
    case Expose:
      /*printf("Window is exposed.\n");*/
      refreshWindow(theEvent.xany.window);
      break;

    case MapNotify:
      /*printf("The window is mapped.\n");*/
      refreshWindow(theEvent.xany.window);
      break;

    case ButtonPress:
      /*printf("A mouse button was pressed.\n");*/
      break;

    case ConfigureNotify:
      /*printf("The window configuration has been changed\n");*/
      refreshWindow(theEvent.xany.window);
      break;
    }
  }
  scrline[x] =
#ifdef Gamma
               gammamap[
#endif
                        (int) (0.299 * Red + 0.587 * Green  + 0.114 * Blue)
#ifdef Gamma
                       ]
#endif
                        ;

  if (x >= (ncols - 1)) {
    int col, limitcol;
    unsigned char *pixelp;            /* Grey scale pixel pointer */
    enum pixval *bitmapp;             /* Monochrome bitmap pointer */

    for (col = 0; col < ncols + 2; ++col) {
        nexterr[col] = 0;
    }
    if (fs_direction) {
        col = 0;
        limitcol = ncols;
        pixelp = scrline;
        bitmapp = pixrow;
    } else {
        col = ncols - 1;
        limitcol = -1;
        pixelp = &(scrline[col]);
        bitmapp = &(pixrow[col]);
    }
    do {
        sum = ((long) *pixelp * FS_SCALE) / 255 + thiserr[col + 1];
        if (sum >= threshval) {
            *bitmapp = WHITE;
            sum = sum - threshval - HALF_FS_SCALE;
        } else {
            *bitmapp = BLACK;
        }

        if (fs_direction) {
            thiserr[col + 2] += (sum * 7) / 16;
            nexterr[col    ] += (sum * 3) / 16;
            nexterr[col + 1] += (sum * 5) / 16;
            nexterr[col + 2] += (sum    ) / 16;

            ++col;
            ++pixelp;
            ++bitmapp;
        } else {
            thiserr[col    ] += (sum * 7) / 16;
            nexterr[col + 2] += (sum * 3) / 16;
            nexterr[col + 1] += (sum * 5) / 16;
            nexterr[col    ] += (sum    ) / 16;

            --col;
            --pixelp;
            --bitmapp;
        }
    } while (col != limitcol);
    temperr = thiserr;                /* Swap error vectors */
    thiserr = nexterr;
    nexterr = temperr;
    fs_direction = ! fs_direction;

    for (col = 0; col < ncols; col++) {
      *bitmapPos++ = !pixrow[col];
      if (!pixrow[col]) {
        XDrawPoint(theDisplay, theWindow, theGC, col, y);
      }
    }
  }
  /*XFlush(theDisplay); Let's be nice to the network, OK? */
}
