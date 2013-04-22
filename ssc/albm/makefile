CFLAGS = -O
OBJS =-L../../../obj -lsng 

all: test

test :  mtclnt1 mtwrkr1 copy

mtclnt1 : mtclnt.c
	cc -I../../../include $(CFLAGS) -o mtclnt mtclnt.c $(OBJS)

mtwrkr1 : mtwrk.c
	cc -I../../../include $(CFLAGS) -o mtwrk mtwrk.c $(OBJS)

copy ::
	 cp mtclnt $(HOME)/bin
	 cp mtwrk  $(HOME)/bin
