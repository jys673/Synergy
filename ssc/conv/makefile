CFLAGS = -O -DWNDX -DTIME
LIBS = -lm -L$(SNG_PATH)/obj -lsng
CC = cc
OBJS = bconvert.c bfile.c trans.c
OBJS2 = convlnt.c bfile.c trans.c
OBJS3 = convwrk.c 
all: bconvert convlnt convwrk copy

.c.o: 
	$(CC) $(CFLAGS) -c $@

bconvert: bconvert.c
	$(CC) $(CFLAGS) $(OBJS) -o bconvert $(LIBS)
convlnt:  convlnt.c
	$(CC) $(CFLAGS) $(OBJS2) -o convlnt $(LIBS)
convwrk:  convwrk.c
	$(CC) $(CFLAGS) $(OBJS3) -o convwrk $(LIBS)

copy::
	cp convwrk $(HOME)/bin
	cp convlnt $(HOME)/bin
