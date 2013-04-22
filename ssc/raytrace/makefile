# Makefile for Persistence of Vision Raytracer
# This file is released to the public domain.
#
#
# MAKE Macros and Such...
#

#***************************************************************
#*
#*                      UNIX Makefile
#*
#***************************************************************

# The exact options may depend on your compiler.  Feel free to modify
# these as required.
# The cc compiler on a HP9000 likes the -Aa option for all files except unix.c
# The gcc compiler is usually satisfied with these options.


OBJ	= o
SNGOBJ    = -L/usr/local/synergy/obj -lsng $(SYS_LIB) -lm
MACHINE_OBJ	= unix.$(OBJ)
CFLAGS=		-c -O
LFLAGS =	-o povray -O
LFLAGS1 =       -o wrk1 -O
LFLAGS2 =       sngray1.c -o sngray1
LFLAGS3 =  	-o wrk2 -O
LFLAGS4 = 	sngray2.c -o sngray2
CC =            gcc

# Make's implicit rules for making a .o file from a .c file...
#
.c.o :
	$(CC) $(CFLAGS) $*.c

all: povray wrk1 sngray1 wrk2 sngray2 showout copy

POVOBJS = povray.$(OBJ) bezier.$(OBJ) blob.$(OBJ) bound.$(OBJ) boxes.$(OBJ)  \
	  camera.$(OBJ) colour.$(OBJ) cones.$(OBJ) csg.$(OBJ) discs.$(OBJ)   \
	  dump.$(OBJ) express.$(OBJ) gif.$(OBJ) gifdecod.$(OBJ)              \
	  hfield.$(OBJ) iff.$(OBJ) image.$(OBJ) lighting.$(OBJ)              \
	  matrices.$(OBJ) normal.$(OBJ) objects.$(OBJ) parse.$(OBJ)          \
	  pigment.$(OBJ) planes.$(OBJ) point.$(OBJ) poly.$(OBJ)              \
	  quadrics.$(OBJ) raw.$(OBJ) ray.$(OBJ) spheres.$(OBJ) \
	  targa.$(OBJ) texture.$(OBJ) tokenize.$(OBJ) triangle.$(OBJ)        \
	  txttest.$(OBJ) vect.$(OBJ) $(MACHINE_OBJ)

REND  = render.$(OBJ)
REND1 = render1.$(OBJ)
REND2 = render2.$(OBJ)


# The original sequential pov-ray-tracing program

povray:	$(POVOBJS) $(REND)
	$(CC) $(LFLAGS) $(POVOBJS) $(REND) -lm



# The worker for fixed chunk Synergy POV-ray-tracing 

wrk1: $(POVOBJS) $(REND1)
	$(CC) $(LFLAGS1) $(POVOBJS) $(REND1) $(SNGOBJ)



# The Master for fixed chunk Synergy Pov-ray-tracing

sngray1: sngray1.c 
	$(CC) $(LFLAGS2) $(SNGOBJ) 



# The worker for fixed chunck Synergy Pov with the chunk decided by
# the speed of each CPU

wrk2: $(POVOBJS) $(REND2) 
	$(CC) $(LFLAGS3) $(POVOBJS) $(REND2) $(SNGOBJ)



# The Master for fixed chunk Synergy Pov with the chunk decided by
# the speed of each CPU

sngray2: sngray2.c
	$(CC) $(LFLAGS4) $(SNGOBJ)


# In order to view the result file, we need the viewer program

showout: show.c draw.c
	$(CC) show.c -o showout -lX11 -lm


copy : wrk1 sngray1 wrk2 sngray2
	cp wrk1 $(HOME)/bin
	cp sngray1 $(HOME)/bin
	cp wrk2 $(HOME)/bin
	cp sngray2 $(HOME)/bin
