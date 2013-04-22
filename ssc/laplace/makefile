CFLAGS = -O
OBJS =-L/usr/local/synergy/obj -lsng 

all: nxdr lap copy

xdr :  lapclnt2 lapwrkr2 

lapclnt2 : lapclnt.x.c
	cc $(CFLAGS) -o lapclnt lapclnt.x.c $(OBJS)

lapwrkr2 : lapwrk.x.c
	cc $(CFLAGS) -o lapwrk lapwrk.x.c $(OBJS)

nxdr :  lapclnt1 lapwrkr1 

lapclnt1 : lapclnt.c
	cc $(CFLAGS) -o lapclnt lapclnt.c $(OBJS)

lapwrkr1 : lapwrk.c
	cc $(CFLAGS) -o lapwrk lapwrk.c $(OBJS)

lap : lap.c 
	cc $(CFLAGS) -o lap lap.c $(OBJS) 

copy : lapclnt lapwrk
	cp lapclnt $(HOME)/bin
	cp lapwrk $(HOME)/bin
