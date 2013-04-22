CFLAGS = -g -o
OBJS =-L../../obj -lsng

all: bfs-master bfs-worker bfs-seq

bfs-master : bfs-master.c
	cc -I/home/local/X11R5/include $(CFLAGS) bfs-master \
	-L/home/local/X11R5/lib   bfs-master.c -lX11 $(OBJS)

bfs-worker: bfs-worker.c
	cc $(CFLAGS) bfs-worker  bfs-worker.c $(OBJS)

bfs-seq : bfs-seq.c 
	cc -I/home/local/X11R5/include $(CFLAGS) bfs-seq  \
	-L/home/local/X11R5/lib  bfs-seq.c $(OBJS) -lX11
