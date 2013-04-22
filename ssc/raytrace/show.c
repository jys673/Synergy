
/*

  This program will read in the result of of the ray-tracing,
  and draw it to a image memory. It will be call the 'draw.c'
  program.

  C FU
*/



#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

char hello[]={"Synergy-POV-Raytracing"};   /* title of the window */


Display *mydisplay;
Window mywindow;
GC mygc;
XEvent myevent;
KeySym mykey; 


#include "draw.c"  /* The drawing program */

int main(argc,argv)
    int  argc;
    char ** argv;
{

   char filename[30];
   XSizeHints myhint;

   int myscreen;

   unsigned long myforeground,mybackground;

   int i;
  
   char text[10];
   XColor my_back_is_in_pain;
   int done;

   FILE *fp;
   unsigned char c1,c2;
   strcpy(filename,argv[1]);
   fp=fopen(filename,"r");
   if (!fp) 
   {  printf("\n open failure, use defaults!");
   fp=fopen("povray.out","r");
   } 
   if (!fp) {printf("\n FILE ERR!"); exit(1);}
   c1=fgetc(fp);
   c2=fgetc(fp);
   myhint.width=c1*256+c2;
   c1=fgetc(fp);
   c2=fgetc(fp);
   myhint.height=c1*256+c2;
   fclose(fp);  

   /* initialization! */

   
   mydisplay=XOpenDisplay("");
   if (! mydisplay) printf("\n Can't open DISPLAY! \n");

   myscreen=DefaultScreen(mydisplay);

 

   /* default Pixel values */

   mybackground=WhitePixel(mydisplay,myscreen);

   myforeground=BlackPixel(mydisplay,myscreen);

   my_back_is_in_pain.red=200;
   my_back_is_in_pain.green=40000;
   my_back_is_in_pain.blue=65000;    /* should be light bule.... */

   
   /* default program -specified windows */

   myhint.x=20; myhint.y=20;

 /*  myhint.width=350; myhint.height=250; */

   myhint.flags=PPosition | PSize;

 

   /* window creation */

   mywindow=XCreateSimpleWindow(mydisplay,

				DefaultRootWindow(mydisplay),

				myhint.x,myhint.y,myhint.width,

				myhint.height,5,myforeground,

				mybackground);

   XSetStandardProperties(mydisplay,mywindow,hello,hello,

			  None,argv,argc,&myhint);

 

   /* Gc creation */

   mygc=XCreateGC(mydisplay,mywindow,0,0);

   XSetBackground(mydisplay,mygc,mybackground);

   XSetForeground(mydisplay,mygc,myforeground);

   XSetWindowBackground(mydisplay,mywindow,
                       (unsigned long) 0x0000ffff);  

   /* input event selection */

   XSelectInput(mydisplay,mywindow,

       ButtonPressMask | KeyPressMask | ExposureMask );

 

   /* window mapping */

    XMapRaised (mydisplay,mywindow);

    draw_pic(filename); 

    /* main event loop */

 

   done=0;

   while (done==0)

   {

    /* readnext event */

    XNextEvent(mydisplay,&myevent);

    switch(myevent.type)

    {

    case Expose:

	  /*  if (myevent.xexpose.count ==0)  */
            XPutImage(mydisplay,mywindow,mygc,image,
                      1,1,1,1,myhint.width,myhint.height);
            XFlush(mydisplay);

	  break;

 
    case MappingNotify:

 

	 XRefreshKeyboardMapping(&myevent);

	 break;


    case ButtonPress:

	 break;

 
    case KeyPress:

 

	 i=XLookupString (&myevent, text,10,&mykey,0);
         printf("\n in keypress case ");
	 if (i==1 && text[0] =='q') done=1;

	 break;
	}

      }

 

      XFreeGC(mydisplay,mygc);

      XDestroyWindow(mydisplay,mywindow);

      XCloseDisplay(mydisplay);

      exit(0);

     }

 

 
 

 


