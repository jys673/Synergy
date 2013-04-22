#include  <stdio.h>
#include  <X11/X.h>
#include  <X11/Xos.h>
#include  <X11/Xlib.h>
#include  <X11/Xutil.h>
#include  <X11/Xatom.h>

#define  _NROWS 15

Display           *dpy;
Window            win;
Pixmap            pix;
Visual            *visual;
XImage            *image;

GC                  gc;
XWindowAttributes   war;
unsigned long       fg, bg, bd, bw, whitepixel, attribmask;
XSetWindowAttributes  attrib;
XStandardColormap   best_map;
Colormap            cmap;
XGCValues           gcv;
XEvent              event;
XSizeHints          xsh;     /* Size hints for window manager */
char                *pixels;
int                 ix, iy, itimes, icols, progress;

unsigned long       IaxFlag, XDisplay;
unsigned long       event_mask = ExposureMask | ButtonPressMask;
int                table[256]; /* Store the colormap indexes for grays val */
