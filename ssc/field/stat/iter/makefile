OPT = -O
XLIBPATH = -L/usr/local/lib
SNGLIB = -L$(HOME)/synergy/obj
INCL = -I$(HOME)/synergy/include
#LIBS = -lm -lsng -lbsd -lc
LIBS = -lsng -lm -lc
XLIBS = -lX11


all: fieldcl fieldwrk copy


fieldcl : fieldcl.o geom.o  xdisp.o 
	cc $(OPT) -o fieldcl  fieldcl.o geom.o xdisp.o $(XLIBPATH) \
	-lX11 $(SNGLIB) $(LIBS)

xdisp.o : xdisp.c inx.h fieldcl.h
	cc $(OPT) -c xdisp.c

fieldcl.o: fieldcl.c fieldcl.h 
	cc $(INCL) $(OPT) -c fieldcl.c

geom.o: geom.c   fieldcl.h
	cc $(INCL) $(OPT) -c geom.c

fieldwrk: fieldwrk.o
	cc $(OPT) -o fieldwrk fieldwrk.o $(SNGLIB) $(LIBS)

fieldwrk.o : fieldwrk.c fieldwrk.h 
	cc $(INCL) $(OPT) -c fieldwrk.c

copy: fieldwrk fieldcl
	cp fieldwrk $(HOME)/bin
	cp fieldcl $(HOME)/bin
