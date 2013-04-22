/****************************************************************************
*                normal.c
*
*  This module implements solid texturing functions that perturb the surface
*  normal to create a bumpy effect. 
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

extern unsigned short crctab[256];

void ripples (x, y, z, Tnormal, normal)
DBL x, y, z;
TNORMAL *Tnormal;
VECTOR *normal;
  {
  register int i;
  VECTOR point;
  register DBL length, scalar, index;

  if (Options & DEBUGGING)
    printf ("ripples %g %g %g", x, y, z);

  for (i = 0 ; i < NUMBER_OF_WAVES ; i++) 
    {
    point.x = x;
    point.y = y;
    point.z = z;
    VSub (point, point, Wave_Sources[i]);
    VDot (length, point, point);
    if (length == 0.0)
      length = 1.0;

    length = sqrt(length);
    index = length*Tnormal->Frequency
    + Tnormal -> Phase;
    scalar = cycloidal (index) * Tnormal -> Amount;

    if (Options & DEBUGGING)
      printf (" index %g scalar %g length %g\n", index, scalar, length);

    VScale (point, point, scalar/length/(DBL)NUMBER_OF_WAVES);
    VAdd (*normal, *normal, point);
    }
  VNormalize (*normal, *normal);
  }

void waves (x, y, z, Tnormal, normal)
DBL x, y, z;
TNORMAL *Tnormal;
VECTOR *normal;
  {
  register int i;
  VECTOR point;
  register DBL length, scalar, index, sinValue ;

  if (Options & DEBUGGING)
    printf ("waves %g %g %g\n", x, y, z);

  for (i = 0 ; i < NUMBER_OF_WAVES ; i++) 
    {
    point.x = x;
    point.y = y;
    point.z = z;
    VSub (point, point, Wave_Sources[i]);
    VDot (length, point, point);
    if (length == 0.0)
      length = 1.0;

    length = sqrt(length);
    index = (length * Tnormal -> Frequency * frequency[i])
      + Tnormal -> Phase;
    sinValue = cycloidal (index);

    scalar =  sinValue * Tnormal -> Amount /
    frequency[i];
    VScale (point, point, scalar/length/(DBL)NUMBER_OF_WAVES);
    VAdd (*normal, *normal, point);
    }
  VNormalize (*normal, *normal);
  }


void bumps (x, y, z, Tnormal, normal)
DBL x, y, z;
TNORMAL *Tnormal;
VECTOR *normal;
  {
  VECTOR bump_turb;

  if (Tnormal -> Amount == 0.0)
    return;                            /* why are we here?? */

  if (Options & DEBUGGING)
    printf ("bumps %g %g %g\n", x, y, z);

  DNoise (&bump_turb, x, y, z);         /* Get Normal Displacement Val. */
  VScale(bump_turb, bump_turb, Tnormal->Amount);
  VAdd (*normal, *normal, bump_turb);   /* displace "normal" */
  VNormalize (*normal, *normal);        /* normalize normal! */
  return;
  }

/*
   dents is similar to bumps, but uses noise() to control the amount of
   dnoise() perturbation of the object normal...
*/

void dents (x, y, z, Tnormal, normal)
DBL x, y, z;
TNORMAL *Tnormal;
VECTOR *normal;
  {
  VECTOR stucco_turb;
  DBL noise;

  if (Tnormal -> Amount == 0.0)
    return;                           /* why are we here?? */

  noise = Noise (x, y, z);

  noise =  noise * noise * noise * Tnormal->Amount;

  if (Options & DEBUGGING)
    printf ("dents %g %g %g noise %g\n", x, y, z, noise);

  DNoise (&stucco_turb, x, y, z);       /* Get Normal Displacement Val. */

  VScale (stucco_turb, stucco_turb, noise);
  VAdd (*normal, *normal, stucco_turb); /* displace "normal" */
  VNormalize (*normal, *normal);        /* normalize normal! */
  return;
  }




/*
   Ideas garnered from the April 89 Byte Graphics Supplement on RenderMan,
   refined from "The RenderMan Companion, by Steve Upstill of Pixar, (C) 1990
   Addison-Wesley.
*/


/*
   wrinkles - This is my implementation of the dented() routine, using
   a surface iterative fractal derived from DTurbulence.  This is a 3-D vers.
   (thanks to DNoise()...) of the usual version using the singular Noise()...
   Seems to look a lot like wrinkles, however... (hmmm)
*/

void wrinkles (x, y, z, Tnormal, normal)
DBL x, y, z;
TNORMAL *Tnormal;
VECTOR *normal;
  {
  register int i;
  register DBL scale = 1.0;
  VECTOR result, value;

  if (Tnormal -> Amount == 0.0)
    return;                                /* why are we here?? */

  if (Options & DEBUGGING)
    printf ("wrinkles %g %g %g\n", x, y, z);

  result.x = 0.0;
  result.y = 0.0;
  result.z = 0.0;

  for (i = 0; i < 10 ; scale *= 2.0, i++)
    {
    DNoise(&value, x * scale, y * scale, z * scale);   /* * scale,*/
    result.x += FABS (value.x / scale);
    result.y += FABS (value.y / scale);
    result.z += FABS (value.z / scale);
    }

  VScale(result, result, Tnormal->Amount);
  VAdd (*normal, *normal, result);             /* displace "normal" */
  VNormalize (*normal, *normal);               /* normalize normal! */
  return;
  }

TNORMAL *Create_Tnormal ()
  {
  TNORMAL *New;

  if ((New = (TNORMAL *) malloc (sizeof (TNORMAL))) == NULL)
    MAError ("normal");

  INIT_TPATTERN_FIELDS(New,NO_NORMAL);
  New->Amount = 0.0;
  return (New);
  }

TNORMAL *Copy_Tnormal (Old)
TNORMAL *Old;
  {
  TNORMAL *New;

  if (Old != NULL)
    {
    New = Create_Tnormal ();
    *New = *Old;

    New->Image = Copy_Image (Old->Image);
    New->Trans = Copy_Transform (Old->Trans);
    }
  else
    New = NULL;

  return (New);
  }

void Translate_Tnormal(Tnormal,Vector)
TNORMAL *Tnormal;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  if (Tnormal == NULL)
    return;

  Compute_Translation_Transform (&Trans, Vector);
  Transform_Tnormal (Tnormal, &Trans);
  }

void Rotate_Tnormal(Tnormal,Vector)
TNORMAL *Tnormal;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  if (Tnormal == NULL)
    return;

  Compute_Rotation_Transform (&Trans, Vector);
  Transform_Tnormal (Tnormal, &Trans);
  }

void Scale_Tnormal(Tnormal,Vector)
TNORMAL *Tnormal;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  if (Tnormal == NULL)
    return;

  Compute_Scaling_Transform (&Trans, Vector);
  Transform_Tnormal (Tnormal, &Trans);
  }

void Transform_Tnormal(Tnormal,Trans)
TNORMAL *Tnormal;
TRANSFORM *Trans;
  {
  if (Tnormal == NULL)
    return;

  if (!Tnormal->Trans)
    Tnormal->Trans = Create_Transform ();

  Compose_Transforms (Tnormal->Trans, Trans);
  }

void Destroy_Tnormal(Tnormal)
TNORMAL *Tnormal;
  {
  if (Tnormal == NULL)
    return;

  Destroy_Image (Tnormal->Image);
  Destroy_Transform (Tnormal->Trans);
  free (Tnormal);
  }

void Post_Tnormal (Tnormal)
TNORMAL *Tnormal;
  {
  if (Tnormal == NULL)
    return;

  if (Tnormal->Type == NO_NORMAL)
    Error("No normal type given");

  Tnormal->Flags |= POST_DONE;

  return;   
  }
