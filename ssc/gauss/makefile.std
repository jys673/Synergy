CFLAGS = -g -o
OBJS =-L../../obj -lsng 

all: ga_master ga_wrk ga_seq copy               

ga_master : ga_master.c
	cc $(CFLAGS) ga_master ga_master.c $(OBJS)

ga_wrk : ga_wrk.c
	cc $(CFLAGS) ga_wrk ga_wrk.c $(OBJS)

ga_seq : ga_seq.c
	cc $(CFLAGS) ga_seq ga_seq.c $(OBJS)

copy : ga_master ga_wrk
	cp ga_master $(HOME)/bin
	cp ga_wrk $(HOME)/bin
