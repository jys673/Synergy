CFLAGS = -O -o
OBJS =-L$(SNG_PATH)/obj -lsng 

all: fix_chunk fractal copy

fix_chunk : frwrk frclnt
frclnt : frclnt.c 
	cc -I/home/local/X11R5/include $(CFLAGS) frclnt frclnt.c \
	-L/home/local/X11R5/lib -lX11 $(OBJS)
frwrk : frwrk.c
	cc $(CFLAGS) frwrk frwrk.c $(OBJS)

fractal : fractal.c 
	cc -I/home/local/X11R5/include $(CFLAGS) fractal  \
	-L/home/local/X11R5/lib  fractal.c $(OBJS) -lX11

copy : frclnt frwrk
	cp frclnt $(HOME)/bin
	cp frwrk $(HOME)/bin
