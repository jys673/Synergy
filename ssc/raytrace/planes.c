/****************************************************************************
*                planes.c
*
*  This module implements functions that manipulate planes.
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

METHODS Plane_Methods =
  { 
  All_Plane_Intersections,
  Inside_Plane, Plane_Normal,
  Copy_Plane,
  Translate_Plane, Rotate_Plane,
  Scale_Plane, Transform_Plane, Invert_Plane, Destroy_Plane
};

extern RAY *CM_Ray;
extern long Ray_Plane_Tests, Ray_Plane_Tests_Succeeded;

#ifndef Plane_Tolerance
#define Plane_Tolerance 1.0e-8
#endif

int All_Plane_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  DBL Depth;
  VECTOR IPoint;

  if (Intersect_Plane (Ray, (PLANE *)Object, &Depth))
    if (Depth > Plane_Tolerance)
      {
      VScale (IPoint, Ray -> Direction, Depth);
      VAddEq (IPoint, Ray -> Initial);
      if (Point_In_Clip (&IPoint, Object->Clip))
        {
        push_entry(Depth,IPoint,Object,Depth_Stack);
        return (TRUE);
        }
      }
  return (FALSE);
  }

int Intersect_Plane (Ray, Plane, Depth)
RAY *Ray;
PLANE *Plane;
DBL *Depth;
  {
  DBL NormalDotOrigin, NormalDotDirection;

  Ray_Plane_Tests++;
  if (Ray == CM_Ray) 
    {
    VDot (NormalDotDirection, Plane->Normal_Vector, Ray->Direction);
    if ((NormalDotDirection < Plane_Tolerance) &&
      (NormalDotDirection > -Plane_Tolerance))
      return (FALSE);

    if (!Plane->CMCached) 
      {
      VDot (Plane->CMNormDotOrigin, Plane->Normal_Vector, Ray->Initial);
      Plane->CMNormDotOrigin += Plane->Distance;
      Plane->CMNormDotOrigin *= -1.0;
      Plane->CMCached = TRUE;
      }

    *Depth = Plane->CMNormDotOrigin / NormalDotDirection;
    if ((*Depth >= Plane_Tolerance) && (*Depth <= Max_Distance)) 
      {
      Ray_Plane_Tests_Succeeded++;
      return (TRUE);
      }
    else
      return (FALSE);
    }
  else 
    {
    VDot (NormalDotDirection, Plane->Normal_Vector, Ray->Direction);
    if ((NormalDotDirection < Plane_Tolerance) &&
      (NormalDotDirection > -Plane_Tolerance))
      return (FALSE);

    VDot (NormalDotOrigin, Plane->Normal_Vector, Ray->Initial);
    NormalDotOrigin += Plane->Distance;
    NormalDotOrigin *= -1.0;

    *Depth = NormalDotOrigin / NormalDotDirection;
    if ((*Depth >= Plane_Tolerance) && (*Depth <= Max_Distance)) 
      {
      Ray_Plane_Tests_Succeeded++;
      return (TRUE);
      }
    else
      return (FALSE);
    }
  }

int Inside_Plane (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  DBL Temp;

  VDot (Temp, *IPoint, ((PLANE *)Object)->Normal_Vector);
  return ((Temp + ((PLANE *)Object)->Distance) <= Plane_Tolerance);
  }

void Plane_Normal (Result, Object, IPoint)
OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  *Result = ((PLANE *)Object)->Normal_Vector;
  }

void Translate_Plane (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  VECTOR Translation;

  VEvaluate (Translation, ((PLANE *)Object)->Normal_Vector, *Vector);
  ((PLANE *)Object)->Distance -= Translation.x + Translation.y + Translation.z;
  }

void Rotate_Plane (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Rotation_Transform (&Trans, Vector);
  Transform_Plane (Object, &Trans);
  }

void Scale_Plane (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  DBL Length;

  PLANE *Plane = (PLANE  *) Object;

  VDivEq(Plane->Normal_Vector, *Vector);

  VLength(Length, ((PLANE *)Object)->Normal_Vector);
  VScaleEq (((PLANE *)Object)->Normal_Vector, 1.0 / Length);
  ((PLANE *)Object)->Distance /= Length;
  }

void Invert_Plane (Object)
OBJECT *Object;
  {
  VScaleEq (((PLANE *) Object)->Normal_Vector, -1.0);
  ((PLANE *) Object)->Distance *= -1.0;
  }

PLANE *Create_Plane()
  {
  PLANE *New;

  if ((New = (PLANE *) malloc (sizeof (PLANE))) == NULL)
    MAError ("plane");

  INIT_OBJECT_FIELDS(New,PLANE_OBJECT,&Plane_Methods)

    Make_Vector (&(New -> Normal_Vector), 0.0, 1.0, 0.0);
  New -> Distance = 0.0;
  New -> CMNormDotOrigin = 0.0;
  New -> CMCached = 0;
  return (New);
  }

void *Copy_Plane (Object)
OBJECT *Object;
  {
  PLANE *New;

  New = Create_Plane ();
  *New = * ((PLANE *)Object);

  return (New);
  }

void Transform_Plane (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  MTransPoint (&((PLANE *) Object)->Normal_Vector,
    &((PLANE *) Object)->Normal_Vector, Trans);
  }

void Destroy_Plane (Object)
OBJECT *Object;
  {
  free (Object);
  }
