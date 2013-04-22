CFLAGS = -O -o
OBJS =-L../../obj  -lsng

all: tsp_seq tspclnt tspwrkr 
 
tsp_seq : tsp_seq.c
	cc $(CFLAGS) tsp_seq tsp_seq.c $(OBJS)

tspclnt : tspclnt.c
	cc $(CFLAGS) tspclnt tspclnt.c $(OBJS)

tspwrkr : tspwrkr.c 
	cc $(CFLAGS) tspwrkr tspwrkr.c $(OBJS)
