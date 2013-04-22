/****************************************************************************
*                boxes.c
*
*  This module implements the box primitive.
*  This file was written by Alexander Enzmann.	He wrote the code for
*  boxes and generously provided us these enhancements.
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

METHODS Box_Methods =
  { 
  All_Box_Intersections,
  Inside_Box, Box_Normal,
  Copy_Box, Translate_Box, Rotate_Box, Scale_Box, Transform_Box,
  Invert_Box, Destroy_Box
};

extern long Ray_Box_Tests, Ray_Box_Tests_Succeeded;

#define close(x, y) (fabs(x-y) < EPSILON ? 1 : 0)

int All_Box_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  DBL Depth1, Depth2;
  VECTOR IPoint;
  register int Intersection_Found;

  Intersection_Found = FALSE;

  if (Intersect_Boxx (Ray, (BOX *)Object, &Depth1, &Depth2))
    {
    VScale (IPoint, Ray->Direction, Depth1);
    VAddEq (IPoint, Ray->Initial);

    if (Point_In_Clip (&IPoint, Object->Clip))
      {
      push_entry(Depth1,IPoint,Object,Depth_Stack);
      Intersection_Found = TRUE;
      }

    if (Depth2 != Depth1)
      {
      VScale (IPoint, Ray->Direction, Depth2);
      VAddEq (IPoint, Ray->Initial);

      if (Point_In_Clip (&IPoint, Object->Clip))
        {
        push_entry(Depth2,IPoint,Object,Depth_Stack);
        Intersection_Found = TRUE;
        }
      }
    }
  return (Intersection_Found);
  }

int Intersect_Boxx (Ray, box, Depth1, Depth2)
RAY *Ray;
BOX *box;
DBL *Depth1, *Depth2;
  {
  DBL t, tmin, tmax;
  VECTOR P, D;

  Ray_Box_Tests++;

  /* Transform the point into the boxes space */
  if (box->Trans != NULL) 
    {
    MInvTransPoint(&P, &Ray->Initial, box->Trans);
    MInvTransDirection(&D, &Ray->Direction, box->Trans);
    }
  else 
    {
    P.x = Ray->Initial.x;
    P.y = Ray->Initial.y;
    P.z = Ray->Initial.z;
    D.x = Ray->Direction.x;
    D.y = Ray->Direction.y;
    D.z = Ray->Direction.z;
    }

    tmin = 0.0;
  tmax = HUGE_VAL;

  /* Sides first */
  if (D.x < -EPSILON) 
    {
    t = (box->bounds[0].x - P.x) / D.x;
    if (t < tmin)
      return 0;
    if (t <= tmax)
      tmax = t;
    t = (box->bounds[1].x - P.x) / D.x;
    if (t >= tmin) 
      {
      if (t > tmax)
        return 0;
      tmin = t;
      }
    }
  else if (D.x > EPSILON) 
    {
    t = (box->bounds[1].x - P.x) / D.x;
    if (t < tmin)
      return 0;
    if (t <= tmax)
      tmax = t;
    t = (box->bounds[0].x - P.x) / D.x;
    if (t >= tmin) 
      {
      if (t > tmax)
        return 0;
      tmin = t;
      }
    }
  else if (P.x < box->bounds[0].x || P.x > box->bounds[1].x)
    return 0;

  /* Check Top/Bottom */
  if (D.y < -EPSILON) 
    {
    t = (box->bounds[0].y - P.y) / D.y;
    if (t < tmin)
      return 0;
    if (t <= tmax)
      tmax = t;
    t = (box->bounds[1].y - P.y) / D.y;
    if (t >= tmin) 
      {
      if (t > tmax)
        return 0;
      tmin = t;
      }
    }
  else if (D.y > EPSILON) 
    {
    t = (box->bounds[1].y - P.y) / D.y;
    if (t < tmin)
      return 0;
    if (t <= tmax)
      tmax = t;
    t = (box->bounds[0].y - P.y) / D.y;
    if (t >= tmin) 
      {
      if (t > tmax)
        return 0;
      tmin = t;
      }
    }
  else if (P.y < box->bounds[0].y || P.y > box->bounds[1].y)
    return 0;

  /* Now front/back */
  if (D.z < -EPSILON) 
    {
    t = (box->bounds[0].z - P.z) / D.z;
    if (t < tmin)
      return 0;
    if (t <= tmax)
      tmax = t;
    t = (box->bounds[1].z - P.z) / D.z;
    if (t >= tmin) 
      {
      if (t > tmax)
        return 0;
      tmin = t;
      }
    }
  else if (D.z > EPSILON) 
    {
    t = (box->bounds[1].z - P.z) / D.z;
    if (t < tmin)
      return 0;
    if (t <= tmax)
      tmax = t;
    t = (box->bounds[0].z - P.z) / D.z;
    if (t >= tmin) 
      {
      if (t > tmax)
        return 0;
      tmin = t;
      }
    }
  else if (P.z < box->bounds[0].z || P.z > box->bounds[1].z)
    return 0;

  *Depth1 = tmin;
  *Depth2 = tmax;

  /* printf("Box intersects: %g, %g\n", *Depth1, *Depth2); */
  if ((*Depth1 < Small_Tolerance) || (*Depth1 > Max_Distance))
    if ((*Depth2 < Small_Tolerance) || (*Depth2 > Max_Distance))
      return (FALSE);
    else
      *Depth1 = *Depth2;
  else
    if ((*Depth2 < Small_Tolerance) || (*Depth2 > Max_Distance))
      *Depth2 = *Depth1;

  Ray_Box_Tests_Succeeded++;
  return (TRUE);
  }

int Inside_Box (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  VECTOR New_Point;
  BOX *box = (BOX *) Object;

  /* Transform the point into the boxes space */
  if (box->Trans != NULL)
    MInvTransPoint(&New_Point, IPoint, box->Trans);
  else
    New_Point = *IPoint;

  /* Test to see if we are inside the box */
  if (New_Point.x < box->bounds[0].x || New_Point.x > box->bounds[1].x)
    return ((int) box->Inverted);
  if (New_Point.y < box->bounds[0].y || New_Point.y > box->bounds[1].y)
    return ((int) box->Inverted);
  if (New_Point.z < box->bounds[0].z || New_Point.z > box->bounds[1].z)
    return ((int)box->Inverted);
  /* Inside the box */
  return 1-box->Inverted;
  }

void Box_Normal (Result, Object, IPoint)
OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  VECTOR New_Point;
  BOX *box = (BOX *) Object;

  /* Transform the point into the boxes space */
  if (box->Trans != NULL)
    MInvTransPoint(&New_Point, IPoint, box->Trans);
  else 
    {
    New_Point.x = IPoint->x;
    New_Point.y = IPoint->y;
    New_Point.z = IPoint->z;
    }

    Result->x = 0.0; Result->y = 0.0; Result->z = 0.0;
  if (close(New_Point.x, box->bounds[1].x))
    Result->x = 1.0;
  else if (close(New_Point.x, box->bounds[0].x))
    Result->x = -1.0;
  else if (close(New_Point.y, box->bounds[1].y))
    Result->y = 1.0;
  else if (close(New_Point.y, box->bounds[0].y))
    Result->y = -1.0;
  else if (close(New_Point.z, box->bounds[1].z))
    Result->z = 1.0;
  else if (close(New_Point.z, box->bounds[0].z))
    Result->z = -1.0;
  else 
    {
    /* Bad result, should we do something with it? */
      Result->x = 1.0;
    }

  /* Transform the point into the boxes space */
  if (box->Trans != NULL) 
    {
    MTransNormal(Result, Result, box->Trans);
    VNormalize(*Result, *Result);
    }
  }

void *Copy_Box (Object)
OBJECT *Object;
  {
  BOX *New;

  New  = Create_Box();
  *New = *((BOX *) Object);

  New->Trans = Copy_Transform(((BOX *)Object)->Trans);

  return (New);
  }

void Translate_Box (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  if (((BOX *)Object)->Trans == NULL)
    {
    VAddEq(((BOX *)Object)->bounds[0], *Vector);
    VAddEq(((BOX *)Object)->bounds[1], *Vector);
    Object->Bounds.Lower_Left = ((BOX *)Object)->bounds[0];
    }
  else
    {
    Compute_Translation_Transform(&Trans, Vector);
    Transform_Box(Object, &Trans);
    }
  }

void Rotate_Box (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;
  Compute_Rotation_Transform(&Trans, Vector);
  Transform_Box(Object, &Trans);
  }

void Scale_Box (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  if (((BOX *)Object)->Trans == NULL)
    {
    VEvaluateEq(((BOX *)Object)->bounds[0], *Vector);
    VEvaluateEq(((BOX *)Object)->bounds[1], *Vector);
    Object->Bounds.Lower_Left = ((BOX *)Object)->bounds[0];
    VSub(Object->Bounds.Lengths, ((BOX *)Object)->bounds[1],
      ((BOX *)Object)->bounds[0]);
    }
  else
    {
    Compute_Scaling_Transform(&Trans, Vector);
    Transform_Box(Object, &Trans);
    }
  }

void Invert_Box (Object)
OBJECT *Object;
  {
  ((BOX *)Object)->Inverted = 1 - ((BOX *)Object)->Inverted;
  }

void Transform_Box (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  BOX *box = (BOX *)Object;
  if (box->Trans == NULL)
    box->Trans = Create_Transform();
  Compose_Transforms(box->Trans, Trans);
  Object->Bounds.Lower_Left = box->bounds[0];
  VSub(Object->Bounds.Lengths, box->bounds[1], box->bounds[0]);
  recompute_bbox(&Object->Bounds, box->Trans);
  }

BOX *Create_Box ()
  {
  BOX *New;

  if ((New = (BOX *) malloc (sizeof (BOX))) == NULL)
    MAError ("box");

  INIT_OBJECT_FIELDS(New, BOX_OBJECT, &Box_Methods)

    Make_Vector (&(New->bounds[0]), -1.0, -1.0, -1.0);
  Make_Vector (&(New->bounds[1]),  1.0,  1.0,  1.0);
  /* Recalculate the bounds */
  Make_Vector(&New->Bounds.Lower_Left, -1.0, -1.0, -1.0);
  Make_Vector(&New->Bounds.Lengths, 2.0, 2.0, 2.0);
  /* Unlike HField, we don't always have a trans here */
  New->Trans = NULL;
  New->Inverted = FALSE;
  return (New);
  }

void Destroy_Box (Object)
OBJECT *Object;
  {
  Destroy_Transform(((BOX *)Object)->Trans);
  free (Object);
  }
