CFLAGS = -O
OBJS = -L/usr/local/synergy/obj -lsng

all: nxdr xdr queen copy 

nxdr : qcl1 qw1

qcl1:	quclnt.c
	cc $(CFLAGS) -o quclnt quclnt.c $(OBJS)

qw1:	quwrk.c
	cc $(CFLAGS) -o quwrk quwrk.c $(OBJS)

xdr : qcl2 qw2

qcl2 : quclnt.x.c
	cc $(CFLAGS) -o quclnt quclnt.x.c $(OBJS)

qw2 :	quwrk.x.c
	cc $(CFLAGS) -o quwrk quwrk.x.c $(OBJS)

queen:	queen.c
	cc $(CFLAGS) -o queen queen.c $(OBJS)

copy : quclnt quwrk
	cp quclnt $(HOME)/bin
	cp quwrk $(HOME)/bin
