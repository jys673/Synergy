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
MACHINE_OBJ	= unix.$(OBJ)
CFLAGS=		-c -O
LFLAGS =	-o povray -O
CC =            cc

# Make's implicit rules for making a .o file from a .c file...
#
.c.o :
	$(CC) $(CFLAGS) $*.c


POVOBJS = povray.$(OBJ) bezier.$(OBJ) blob.$(OBJ) bound.$(OBJ) boxes.$(OBJ)  \
	  camera.$(OBJ) colour.$(OBJ) cones.$(OBJ) csg.$(OBJ) discs.$(OBJ)   \
	  dump.$(OBJ) express.$(OBJ) gif.$(OBJ) gifdecod.$(OBJ)              \
	  hfield.$(OBJ) iff.$(OBJ) image.$(OBJ) lighting.$(OBJ)              \
	  matrices.$(OBJ) normal.$(OBJ) objects.$(OBJ) parse.$(OBJ)          \
	  pigment.$(OBJ) planes.$(OBJ) point.$(OBJ) poly.$(OBJ)              \
	  quadrics.$(OBJ) raw.$(OBJ) ray.$(OBJ) render.$(OBJ) spheres.$(OBJ) \
	  targa.$(OBJ) texture.$(OBJ) tokenize.$(OBJ) triangle.$(OBJ)        \
	  txttest.$(OBJ) vect.$(OBJ) $(MACHINE_OBJ)


povray:	$(POVOBJS)
	$(CC) $(LFLAGS) $(POVOBJS) -lm

