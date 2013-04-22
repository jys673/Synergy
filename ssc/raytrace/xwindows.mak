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

# Added for colorx addition.  You _do_ have $X11 defined by your .cshrc,
# don't you?
X11 = /usr
CFLAGS=		-c -O 
LFLAGS =	-O  $(X11)/lib/libXt.a $(X11)/lib/libXext.a -lX11 \
                -o povray
CC =		cc

OBJ	= o
MACHINE_OBJ	= xwindows.$(OBJ)

# Make's implicit rules for making a .o file from a .c file...
#
.c.o :
	$(CC) $(CFLAGS) $*.c

OBJS		= bezier.$(OBJ) blob.$(OBJ) bound.$(OBJ) boxes.$(OBJ) camera.$(OBJ) \
		  colour.$(OBJ) cones.$(OBJ) csg.$(OBJ) discs.$(OBJ) dump.$(OBJ) \
		  express.$(OBJ) gif.$(OBJ) gifdecod.$(OBJ) hfield.$(OBJ) \
		  iff.$(OBJ) image.$(OBJ) lighting.$(OBJ) matrices.$(OBJ) normal.$(OBJ) \
		  objects.$(OBJ) parse.$(OBJ) pigment.$(OBJ) planes.$(OBJ) point.$(OBJ) \
		  poly.$(OBJ) povray.$(OBJ) quadrics.$(OBJ) raw.$(OBJ) ray.$(OBJ) \
		  render.$(OBJ) spheres.$(OBJ) targa.$(OBJ) texture.$(OBJ) tokenize.$(OBJ) \
		  triangle.$(OBJ) txttest.$(OBJ) vect.$(OBJ) $(MACHINE_OBJ)


povray:	$(OBJS)
	cc $(OBJS) -lm $(LFLAGS)

xwindows.$(OBJ): xwindows.c xpov.icn frame.h povproto.h config.h
	$(CC) $(CFLAGS) -I$(X11)/include xwindows.c
