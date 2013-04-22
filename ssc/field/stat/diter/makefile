OPT = -O
XLIBPATH = -L/usr/X11R6/lib
SNGLIB = -L$(SNG_PATH)/obj
INCL = -I$(SNG_PATH)/include
#LIBS = -lm -lsng -lbsd -lc
LIBS = -lm -lsng -lbsd -lc 
XLIBS = -lX11


all: fieldcl fieldwrk copy


fieldcl : fieldcl.o geom.o  xdisp.o 
	cc $(OPT) -o dfieldcl  fieldcl.o geom.o xdisp.o $(XLIBPATH) \
	-lX11 $(SNGLIB) $(LIBS)

xdisp.o : xdisp.c inx.h fieldcl.h
	cc $(OPT) -c xdisp.c

fieldcl.o: fieldcl.c fieldcl.h 
	cc $(INCL) $(OPT) -c fieldcl.c

geom.o: geom.c   fieldcl.h
	cc $(INCL) $(OPT) -c geom.c

fieldwrk: fieldwrk.o
	cc $(OPT) -o dfieldwrk fieldwrk.o $(SNGLIB) $(LIBS)

fieldwrk.o : fieldwrk.c fieldwrk.h 
	cc $(INCL) $(OPT) -c fieldwrk.c

copy: fieldwrk fieldcl
	rm *.o
	cp dfieldwrk $(HOME)/bin
	cp dfieldcl $(HOME)/bin
