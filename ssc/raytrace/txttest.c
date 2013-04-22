/****************************************************************************
*                txttest.c
*
*  This module implements "fill-in-the-blank" pre-programmed texture 
*  functions for easy modification and testing. Create new textures here.
*
*  from Persistence of Vision Raytracer
*  Copyright 1993 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other 
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file. If 
*  POVLEGAL.DOC is not available or for more info please contact the POV-Ray
*  Team Coordinator by leaving a message in CompuServe's Graphics Developer's
*  Forum.  The latest version of POV-Ray may be found there as well.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
*****************************************************************************/

/*
   Some texture ideas garnered from SIGGRAPH '85 Volume 19 Number 3, 
   "An Image Synthesizer" By Ken Perlin.
   Further Ideas Garnered from "The RenderMan Companion" (Addison Wesley)
*/

#include "frame.h"
#include "vector.h"
#include "povproto.h"
#include "texture.h"

/* Test new textures in the routines that follow */

/* The painted routines take an x,y,z point on an object and a pointer to the*/
/* object's texture description and return the color at that point           */
/* Similar routines are granite, agate, marble. See txtcolor.c for examples. */ 

void painted1 (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  /* YOUR NAME HERE */
  VECTOR Colour_Vector;   

  if (Options & DEBUGGING)
    printf ("painted1 %g %g %g\n", x, y, z);

  DNoise(&Colour_Vector,x,y,z);
  colour ->Red += Colour_Vector.x;
  colour ->Green += Colour_Vector.y;
  colour ->Blue += Colour_Vector.z;
  return;
  }
void painted2 (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  int brkindx;
  COLOUR Colour1, Colour2;

  /* You could change the parser to take two colors after PAINTED2,           */
  /* but since the colormap is already parsed it's easier to use it during    */
  /* testing. If the texture works out right you can change the parser later. */
  if (Pigment -> Colour_Map != NULL)
    {
    Compute_Colour (&Colour1, Pigment, 0.1);
    Compute_Colour (&Colour2, Pigment, 0.9);
    }
  else
    {
    Make_Colour (&Colour1, 1.0, 1.0, 1.0);
    Colour1.Filter = 0.0;
    Make_Colour (&Colour2, 0.0, 1.0, 0.0);
    Colour2.Filter = 0.0;
    }



  brkindx = (int) FLOOR(x) + (int) FLOOR(z);

  if (Options & DEBUGGING)
    printf ("checker %g %g %g\n", x, y, z);

  if (brkindx & 1)
    {
    colour->Red = Colour1.Red;
    colour->Green = Colour1.Green;
    colour->Blue = Colour1.Blue;
    colour->Filter = Colour1.Filter;
    }
    else{
    colour->Red = Colour2.Red;
    colour->Green = Colour2.Green;
    colour->Blue = Colour2.Blue;
    colour->Filter = Colour2.Filter;
    }
  return;
  }
void painted3 (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  /* YOUR NAME HERE */
  ;
  }

/* The bumpy routines take a point on an object,  a pointer to the          */
/* object's texture description and the surface normal at that point and    */
/* return a peturb surface normal to create the illusion that the surface   */
/* has been displaced.                                                      */
/* Similar routines are ripples, dents, bumps. See txtbump.c for examples.  */ 

void bumpy1 (x, y, z, Tnormal, normal)
DBL x, y, z;
TNORMAL *Tnormal;
VECTOR *normal;
  {
  /* YOUR NAME HERE */

  }

void bumpy2 (x, y, z, Tnormal, normal)
DBL x, y, z;
TNORMAL *Tnormal;
VECTOR *normal;
  {
  /* YOUR NAME HERE */
  ;
  }
void bumpy3 (x, y, z, Tnormal, normal)
DBL x, y, z;
TNORMAL *Tnormal;
VECTOR *normal;
  {
  ;
  }
