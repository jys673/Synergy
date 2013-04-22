
Init_X( xres, yres, width,  title, argc, argv)
    int  xres, yres, argc, width;
    char  *title, *argv[];
{
   int i, n;
   int  black, white;
   char  aa[2][20];
   Window root;
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
    n = 256;
    for (i=0; i<n; i++) {
        XColor color;
        color.red = i << 8;
        color.green = 0 << 8;
        color.blue = (256 - i) << 8;
        if (!XAllocColor(dpy, cmap, &color)) {
            printf("out of color table entries \n");
            exit(-1);
        }
        table[i] = color.pixel;
    }
}
