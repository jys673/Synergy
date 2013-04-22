/****************************************************************************
*                spheres.c
*
*  This module implements the sphere primitive.
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

#ifndef Sphere_Tolerance
#define Sphere_Tolerance 1.0e-8
#endif

METHODS Sphere_Methods =
  { 
  All_Sphere_Intersections,
  Inside_Sphere, Sphere_Normal,
  Copy_Sphere,
  Translate_Sphere, Rotate_Sphere,
  Scale_Sphere, Transform_Sphere, Invert_Sphere,
  Destroy_Sphere
};

METHODS Ellipsoid_Methods =
  { 
  All_Ellipsoid_Intersections,
  Inside_Ellipsoid, Ellipsoid_Normal,
  Copy_Sphere,
  Translate_Sphere, Rotate_Sphere,
  Scale_Sphere, Transform_Sphere, Invert_Sphere,
  Destroy_Sphere
};

extern RAY *CM_Ray;
extern long Ray_Sphere_Tests, Ray_Sphere_Tests_Succeeded;

int All_Sphere_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  DBL Depth1, Depth2;
  VECTOR IPoint;
  register int Intersection_Found;

  Intersection_Found = FALSE;

  if (Intersect_Sphere (Ray, (SPHERE*) Object, &Depth1, &Depth2))
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

int All_Ellipsoid_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  DBL Depth1, Depth2, len;
  VECTOR IPoint, dv;
  register int Intersection_Found;
  RAY New_Ray;

  /* Transform the ray into the sphere's space */
  MInvTransPoint(&New_Ray.Initial, &Ray->Initial, ((SPHERE *)Object)->Trans);
  MInvTransDirection(&New_Ray.Direction, &Ray->Direction, ((SPHERE *)Object)->Trans);

  VDot(len, New_Ray.Direction, New_Ray.Direction);
  if (len == 0.0)
    return 0;
  len = 1.0 / sqrt(len);
  VScaleEq(New_Ray.Direction, len);

  Intersection_Found = FALSE;

  if (Intersect_Sphere (&New_Ray, (SPHERE*) Object, &Depth1, &Depth2))
    {
    VScale (IPoint, New_Ray.Direction, Depth1);
    VAddEq (IPoint, New_Ray.Initial);
    if (((SPHERE *)Object)->Trans != NULL)
      MTransPoint(&IPoint, &IPoint, ((SPHERE *)Object)->Trans);

    VSub(dv, IPoint, Ray->Initial);
    VLength(len, dv);

    if (Point_In_Clip (&IPoint, Object->Clip))
      {
      push_entry(len,IPoint,Object,Depth_Stack);
      Intersection_Found = TRUE;
      }

    if (Depth2 != Depth1)
      {
      VScale (IPoint, New_Ray.Direction, Depth2);
      VAddEq (IPoint, New_Ray.Initial);
      if (((SPHERE *)Object)->Trans != NULL)
        MTransPoint(&IPoint, &IPoint, ((SPHERE *)Object)->Trans);

      VSub(dv, IPoint, Ray->Initial);
      VLength(len, dv);

      if (Point_In_Clip (&IPoint, Object->Clip))
        {
        push_entry(len,IPoint,Object,Depth_Stack);
        Intersection_Found = TRUE;
        }
      }
    }
  return (Intersection_Found);
  }

int Intersect_Sphere (Ray, Sphere, Depth1, Depth2)
RAY *Ray;
SPHERE *Sphere;
DBL *Depth1, *Depth2;
  {
  VECTOR Origin_To_Center;
  DBL OCSquared, t_Closest_Approach, Half_Chord, t_Half_Chord_Squared;
  short inside;

  Ray_Sphere_Tests++;
  if (Ray == CM_Ray) 
    {
    if (!Sphere->CMCached) 
      {
      VSub (Sphere->CMOtoC, Sphere->Center, Ray->Initial);
      VDot (Sphere->CMOCSquared, Sphere->CMOtoC, Sphere->CMOtoC);
      Sphere->CMinside = (Sphere->CMOCSquared < Sphere->Radius_Squared);
      Sphere->CMCached = TRUE;
      }
    VDot (t_Closest_Approach, Sphere->CMOtoC, Ray->Direction);
    if (!Sphere->CMinside && (t_Closest_Approach < Sphere_Tolerance))
      return (FALSE);      
    t_Half_Chord_Squared = Sphere->Radius_Squared - Sphere->CMOCSquared +
    (t_Closest_Approach * t_Closest_Approach);
    }
  else 
    {
    VSub (Origin_To_Center, Sphere->Center, Ray->Initial);
    VDot (OCSquared, Origin_To_Center, Origin_To_Center);
    inside = (OCSquared < Sphere->Radius_Squared);
    VDot (t_Closest_Approach, Origin_To_Center, Ray->Direction);
    if (!inside && (t_Closest_Approach < Sphere_Tolerance))
      return (FALSE);

    t_Half_Chord_Squared = Sphere->Radius_Squared - OCSquared +
    (t_Closest_Approach * t_Closest_Approach);
    }

  if (t_Half_Chord_Squared < Sphere_Tolerance)
    return (FALSE);

  Half_Chord = sqrt (t_Half_Chord_Squared);
  *Depth1 = t_Closest_Approach + Half_Chord;
  *Depth2 = t_Closest_Approach - Half_Chord;

  if ((*Depth1 < Sphere_Tolerance) || (*Depth1 > Max_Distance))
    if ((*Depth2 < Sphere_Tolerance) || (*Depth2 > Max_Distance))
      return (FALSE);
    else
      *Depth1 = *Depth2;
  else
    if ((*Depth2 < Sphere_Tolerance) || (*Depth2 > Max_Distance))
      *Depth2 = *Depth1;

  Ray_Sphere_Tests_Succeeded++;
  return (TRUE);
  }

int Inside_Sphere (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  VECTOR Origin_To_Center;
  DBL OCSquared;

  VSub (Origin_To_Center, ((SPHERE *)Object)->Center, *IPoint);
  VDot (OCSquared, Origin_To_Center, Origin_To_Center);

  if (((SPHERE *)Object)->Inverted)
    return (OCSquared - ((SPHERE *)Object)->Radius_Squared > Sphere_Tolerance);
  else
    return (OCSquared - ((SPHERE *)Object)->Radius_Squared < Sphere_Tolerance);
  }

int Inside_Ellipsoid (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  VECTOR Origin_To_Center;
  DBL OCSquared;
  VECTOR New_Point;

  /* Transform the point into the sphere's space */
  MInvTransPoint(&New_Point, IPoint, ((SPHERE *)Object)->Trans);
  VSub (Origin_To_Center, ((SPHERE *)Object)->Center, New_Point);
  VDot (OCSquared, Origin_To_Center, Origin_To_Center);

  if (((SPHERE *)Object)->Inverted)
    return (OCSquared - ((SPHERE *)Object)->Radius_Squared > Sphere_Tolerance);
  else
    return (OCSquared - ((SPHERE *)Object)->Radius_Squared < Sphere_Tolerance);
  }

void Sphere_Normal (Result, Object, IPoint)
OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  VSub (*Result, *IPoint, ((SPHERE *)Object)->Center);
  VScaleEq (*Result, ((SPHERE *)Object)->Inverse_Radius);
  }

void Ellipsoid_Normal (Result, Object, IPoint)
OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  VECTOR New_Point;

  /* Transform the point into the sphere's space */
  MInvTransPoint(&New_Point, IPoint, ((SPHERE *)Object)->Trans);

  VSub (*Result, New_Point, ((SPHERE *)Object)->Center);
  VScaleEq (*Result, ((SPHERE *)Object)->Inverse_Radius);

  MTransNormal(Result, Result, ((SPHERE *)Object)->Trans);
  VNormalize(*Result, *Result);
  }

void *Copy_Sphere (Object)
OBJECT *Object;
  {
  SPHERE *New;

  New = Create_Sphere ();
  *New = *((SPHERE *) Object);

  New->Trans = Copy_Transform(((SPHERE *)Object)->Trans);
  return (New);
  }

void Translate_Sphere (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  if (((SPHERE *)Object)->Trans == NULL)
    {
    VAddEq (((SPHERE *) Object)->Center, *Vector);
    VAddEq(Object->Bounds.Lower_Left, *Vector);
    }
  else
    {
    Compute_Translation_Transform(&Trans, Vector);
    Transform_Sphere(Object, &Trans);
    }
  }

void Rotate_Sphere (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;
  SPHERE *Sphere = (SPHERE *) Object;

  Compute_Rotation_Transform (&Trans, Vector);

  if (Sphere->Trans == NULL)
    {
    MTransPoint(&Sphere->Center, &Sphere->Center, &Trans);
    Make_Vector(&Sphere->Bounds.Lower_Left,
      Sphere->Center.x - Sphere->Radius,
      Sphere->Center.y - Sphere->Radius,
      Sphere->Center.z - Sphere->Radius);
    Make_Vector(&Sphere->Bounds.Lengths,
      2.0 * Sphere->Radius,
      2.0 * Sphere->Radius,
      2.0 * Sphere->Radius);
    }
  else
    {
    Transform_Sphere (Object, &Trans);
    }
  }

void Scale_Sphere (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  SPHERE *Sphere = (SPHERE *) Object;
  TRANSFORM Trans;

  if ((Vector->x != Vector->y) || (Vector->x != Vector->z))
    if (Sphere->Trans == NULL)
      {
      Sphere->Methods = &Ellipsoid_Methods;
      Sphere->Trans = Create_Transform();
      }

  if (Sphere->Trans == NULL)
    {
    VScaleEq (Sphere->Center, Vector->x);
    Sphere->Radius *= Vector->x;
    Sphere->Radius_Squared = Sphere->Radius * Sphere->Radius;
    Sphere->Inverse_Radius = 1.0 / Sphere->Radius;
    Make_Vector(&Sphere->Bounds.Lower_Left,
      Sphere->Center.x - Sphere->Radius,
      Sphere->Center.y - Sphere->Radius,
      Sphere->Center.z - Sphere->Radius);
    Make_Vector(&Sphere->Bounds.Lengths,
      2.0 * Sphere->Radius,
      2.0 * Sphere->Radius,
      2.0 * Sphere->Radius);
    }
  else
    {
    Compute_Scaling_Transform(&Trans, Vector);
    Transform_Sphere(Object, &Trans);
    }
  }

void Invert_Sphere (Object)
OBJECT *Object;
  {
  ((SPHERE *) Object)->Inverted ^= TRUE;
  }

SPHERE *Create_Sphere ()
  {
  SPHERE *New;

  if ((New = (SPHERE *) malloc (sizeof (SPHERE))) == NULL)
    MAError ("sphere");

  INIT_OBJECT_FIELDS(New, SPHERE_OBJECT, &Sphere_Methods)
    Make_Vector (&(New->Center), 0.0, 0.0, 0.0);
  New->Radius = 1.0;
  New->Radius_Squared = 1.0;
  New->Inverse_Radius = 1.0;
  New->CMCached = FALSE;
  New->Trans = NULL;
  New->Inverted = FALSE;
  /* CMOtoC, CMOtoCSquared and CMinside are only valid when CMCached */
  return (New);
  }

void Transform_Sphere (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  SPHERE *Sphere = (SPHERE *)Object;

  if (Sphere->Trans == NULL)
    {
    Sphere->Methods = &Ellipsoid_Methods;
    Sphere->Trans = Create_Transform();
    }

  Compose_Transforms(Sphere->Trans, Trans);

  Make_Vector(&Sphere->Bounds.Lower_Left,
    Sphere->Center.x - Sphere->Radius,
    Sphere->Center.y - Sphere->Radius,
    Sphere->Center.z - Sphere->Radius);
  Make_Vector(&Sphere->Bounds.Lengths,
    2.0 * Sphere->Radius,
    2.0 * Sphere->Radius,
    2.0 * Sphere->Radius);

  recompute_bbox(&Object->Bounds, Sphere->Trans);
  }

void Destroy_Sphere (Object)
OBJECT *Object;
  {
  Destroy_Transform(((SPHERE *)Object)->Trans);
  free (Object);
  }
