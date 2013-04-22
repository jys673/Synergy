/****************************************************************************
*                ray.c
*
*  This module implements the code pertaining to rays.
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

#include "frame.h"
#include "vector.h"
#include "povproto.h"

#define Mix(a,b,c)   {   \
   (a).x = (b).x * (c).y; \
   (a).y = (b).x * (c).z; \
   (a).z = (b).y * (c).z; }

void Make_Ray(r)
RAY *r;
  {
  VECTOR Temp_Init_Dir;

  VSquareTerms (r -> Initial_2, r -> Initial);
  VSquareTerms (r -> Direction_2, r -> Direction);
  VEvaluate (r -> Initial_Direction, r -> Initial, r -> Direction);
  Mix (r -> Mixed_Initial_Initial, r -> Initial, r -> Initial);
  Mix (r -> Mixed_Dir_Dir, r -> Direction, r -> Direction);
  Mix (Temp_Init_Dir, r -> Initial, r -> Direction);
  Mix (r -> Mixed_Init_Dir, r -> Direction, r -> Initial);
  VAdd (r -> Mixed_Init_Dir, r -> Mixed_Init_Dir, Temp_Init_Dir);
  r -> Quadric_Constants_Cached = TRUE;
  }

void Initialize_Ray_Containers (Ray)
RAY *Ray;
  {
  Ray -> Containing_Index = -1;
  }

void Copy_Ray_Containers (Dest_Ray, Source_Ray)
RAY *Dest_Ray, *Source_Ray;
  {
  register int i;

  if ((Dest_Ray -> Containing_Index = Source_Ray -> Containing_Index)
    >= MAX_CONTAINING_OBJECTS) 
    {
    fprintf (stderr, "ERROR - Containing Index too high\n");
    close_all();
    exit (1);
    }

  for (i = 0 ; i < MAX_CONTAINING_OBJECTS ; i++) 
    {
    Dest_Ray -> Containing_Textures[i] = Source_Ray -> Containing_Textures[i];
    Dest_Ray -> Containing_IORs[i] = Source_Ray -> Containing_IORs[i];
    }
  }

void Ray_Enter (ray, texture)
RAY *ray;
TEXTURE *texture;
  {
  register int index;

  if ((index = ++(ray -> Containing_Index)) >= MAX_CONTAINING_OBJECTS) 
    {
    fprintf (stderr, "Too many nested refracting objects\n");
    close_all();
    exit(1);
    }

  ray -> Containing_Textures [index] = texture;
  ray -> Containing_IORs [index] = texture->Finish->Index_Of_Refraction;
  }

void Ray_Exit (ray)
RAY *ray;
  {
  if (--(ray -> Containing_Index) < -1) 
    {
    fprintf (stderr, "Too many exits from refractions\n");
    close_all();
    exit(1);
    }
  }
