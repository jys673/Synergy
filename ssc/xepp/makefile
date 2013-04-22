CFLAGS = -O -o
OBJS =-L$(SNG_PATH)/obj -lsng 
XCFLAGS =-I/home/local/X11R5/include -L/home/local/X11R5/lib
XLIBS =-lXm -lXt -lX11 -lXaw -lXmu -lXext
 

all: xepwrk  xepclnt copy 

xepclnt : xepclnt.c
	cc $(XCFLAGS)  $(CFLAGS) xepclnt xepclnt.c \
	 $(OBJS) $(XLIBS) -lm 
xepwrk : xepwrk.c
	cc $(CFLAGS) xepwrk xepwrk.c $(OBJS)
copy:   xepclnt xepwrk
	cp xepclnt $(HOME)/bin
	cp xepwrk $(HOME)/bin
