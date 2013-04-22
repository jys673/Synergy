CFLAGS = -O -o
OBJS =-L../../obj -lsng 

all: subwrk subclnt sub_seq move

subclnt : subclnt.c
	cc $(CFLAGS) subclnt subclnt.c $(OBJS) 

subwrk : subwrk.c
	cc $(CFLAGS) subwrk subwrk.c $(OBJS)

sub_seq : sub_seq.c 
	cc $(CFLAGS) sub_seq sub_seq.c $(OBJS) 

move : subclnt subwrk
	cp subclnt $(HOME)/bin
	cp subwrk $(HOME)/bin
