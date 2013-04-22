/****************************************************************************
*                discs.c
*
*  This module implements the disc primitive.
*  This file was written by Alexander Enzmann.	He wrote the code for
*  discs and generously provided us these enhancements.
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

METHODS Disc_Methods =
  { 
  All_Disc_Intersections,
  Inside_Disc, Disc_Normal,
  Copy_Disc, Translate_Disc, Rotate_Disc, Scale_Disc, Transform_Disc,
  Invert_Disc, Destroy_Disc
};

extern long Ray_Disc_Tests, Ray_Disc_Tests_Succeeded;

int All_Disc_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  DBL Depth;
  VECTOR IPoint;
  register int Intersection_Found;

  Intersection_Found = FALSE;

  if (Intersect_Disc (Ray, (DISC *)Object, &Depth))
    {
    VScale (IPoint, Ray->Direction, Depth);
    VAddEq (IPoint, Ray->Initial);

    if (Point_In_Clip (&IPoint, Object->Clip))
      {
      push_entry(Depth,IPoint,Object,Depth_Stack);
      Intersection_Found = TRUE;
      }
    }
  return (Intersection_Found);
  }

int Intersect_Disc (Ray, disc, Depth)
RAY *Ray;
DISC *disc;
DBL *Depth;
  {
  DBL t1, t2, len, tmpf;
  VECTOR P, D, tmpv;

  Ray_Disc_Tests++;

  /* Transform the point into the discs space */
  MInvTransPoint(&P, &Ray->Initial, disc->Trans);
  MInvTransDirection(&D, &Ray->Direction, disc->Trans);

  *Depth = HUGE_VAL;

  VLength(len, D);
  VInverseScaleEq(D, len);

  /* Do the normal ray-plane intersection */
  VDot(t1, disc->normal, D);
  if (fabs(t1) < 1.0e-10)
    return 0;
  VDot(t2, disc->normal, P);
  t2 = -(t2 + disc->d) / t1;
  if (t2 < 0)
    return 0;
  VScale(tmpv, D, t2);
  VAddEq(tmpv, P);
  VSub(tmpv, tmpv, disc->center);
  VDot(tmpf, tmpv, tmpv);
  if (tmpf < disc->iradius2 || tmpf > disc->oradius2)
    return 0;
  else
    *Depth = t2 / len;

  if (*Depth < Small_Tolerance || *Depth > Max_Distance)
    return FALSE;

  Ray_Disc_Tests_Succeeded++;
  return TRUE;
  }

int Inside_Disc (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  VECTOR New_Point;
  DISC *disc = (DISC *) Object;
  DBL tmpf, offset = (disc->Inverted ? -EPSILON : EPSILON);

  /* Transform the point into the discs space */
  MInvTransPoint(&New_Point, IPoint, disc->Trans);

  VDot(tmpf, New_Point, disc->normal);
  tmpf += disc->d;
  if (tmpf+offset > 0)
    return disc->Inverted;
  else
    return 1 - disc->Inverted;
  }

void Disc_Normal (Result, Object, IPoint)
OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  DISC *disc = (DISC *) Object;
  MTransNormal(Result, &disc->normal, disc->Trans);
  VNormalize(*Result, *Result);
  }

void *Copy_Disc (Object)
OBJECT *Object;
  {
  DISC *New;

  New  = Create_Disc();
  *New = *((DISC *) Object);

  New->Trans = Copy_Transform(((DISC *)Object)->Trans);

  return (New);
  }

void Translate_Disc (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Translation_Transform(&Trans, Vector);
  Transform_Disc(Object, &Trans);
  }

void Rotate_Disc (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;
  Compute_Rotation_Transform(&Trans, Vector);
  Transform_Disc(Object, &Trans);
  }

void Scale_Disc (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Scaling_Transform(&Trans, Vector);
  Transform_Disc(Object, &Trans);
  }

void Invert_Disc (Object)
OBJECT *Object;
  {
  ((DISC *)Object)->Inverted = 1 - ((DISC *)Object)->Inverted;
  }

void Transform_Disc (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  DISC *Disc = (DISC *)Object;
  VECTOR lengths;
  DBL len;

  Compose_Transforms(Disc->Trans, Trans);

  /* Recalculate the bounds */
  len = sqrt(Disc->oradius2);
  Make_Vector(&lengths, len, len, len);
  VSub(Disc->Bounds.Lower_Left, Disc->center, lengths);
  VScale(Disc->Bounds.Lengths, lengths, 2.0);
  recompute_bbox(&Disc->Bounds, Disc->Trans);
  }

DISC *Create_Disc ()
  {
  DISC *New;

  if ((New = (DISC *) malloc (sizeof (DISC))) == NULL)
    MAError ("disc");

  INIT_OBJECT_FIELDS(New, DISC_OBJECT, &Disc_Methods)
    Make_Vector (&(New->center), 0.0, 0.0, 0.0);
  Make_Vector (&(New->normal), 0.0, 1.0, 0.0);
  New->iradius2 = 0.0;
  New->oradius2 = 1.0;
  New->d = 0.0;
  New->Trans = Create_Transform();
  New->Inverted = 0;

  /* Default bounds */
  Make_Vector(&New->Bounds.Lower_Left, -1.0, -EPSILON, -1.0);
  Make_Vector(&New->Bounds.Lengths, 2.0, 2*EPSILON, 2.0);

  return New;
  }

void Destroy_Disc (Object)
OBJECT *Object;
  {
  Destroy_Transform(((DISC *)Object)->Trans);
  free (Object);
  }
