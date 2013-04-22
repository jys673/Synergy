/****************************************************************************
*                cones.c
*
*  This module implements the cone primitive.
*  This file was written by Alexander Enzmann.    He wrote the code for
*  cones and generously provided us these enhancements.
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

METHODS Cone_Methods =
  { 
  All_Cone_Intersections,
  Inside_Cone, Cone_Normal,
  Copy_Cone, Translate_Cone, Rotate_Cone, Scale_Cone, Transform_Cone,
  Invert_Cone, Destroy_Cone
  };

extern long Ray_Cone_Tests, Ray_Cone_Tests_Succeeded;
static int Intersect_Cone PARAMS((RAY *Ray, CONE *Cone, DBL *Depths));

#ifndef Cone_Tolerance
#define Cone_Tolerance 1.0e-6
#endif

#define close(x, y) (fabs(x-y) < EPSILON ? 1 : 0)

int All_Cone_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  DBL Depths[4];
  VECTOR IPoint;
  int Intersection_Found, cnt, i;

  Intersection_Found = FALSE;

  if ((cnt=Intersect_Cone (Ray, (CONE *)Object, Depths))!=0)
    for (i=0; i<cnt; i++)
    {
    VScale (IPoint, Ray->Direction, Depths[i]);
    VAddEq (IPoint, Ray->Initial);

    if (Point_In_Clip (&IPoint, Object->Clip))
      {
      push_entry(Depths[i],IPoint,Object,Depth_Stack);
      Intersection_Found = TRUE;
      }
    }
  return (Intersection_Found);
  }

static int Intersect_Cone (Ray, Cone, Depths)
RAY *Ray;
CONE *Cone;
DBL *Depths;
  {
  DBL a, b, c, z, t1, t2, len;
  DBL d;
  VECTOR P, D;
  int i=0;

  Ray_Cone_Tests++;

  /* Transform the ray into the cones space */
  MInvTransPoint(&P, &Ray->Initial, Cone->Trans);
  MInvTransDirection(&D, &Ray->Direction, Cone->Trans);

  VLength(len, D);
  VInverseScaleEq(D, len);

  if (Cone->cyl_flag) 
    {
    /* Solve intersections with a cylinder */
    a = D.x * D.x + D.y * D.y;
    if (a > EPSILON) 
      {
      b = P.x * D.x + P.y * D.y;
      c = P.x * P.x + P.y * P.y - 1.0;
      d = b * b - a * c;
      if (d >= 0.0)
        {
        d = sqrt(d);
        t1 = (-b + d) / a;
        t2 = (-b - d) / a;
        z = P.z + t1 * D.z;
        if (t1 > Cone_Tolerance && t1 < Max_Distance && z >= 0.0 && z <= 1.0)
          Depths[i++] = t1/len;
        z = P.z + t2 * D.z;
        if (t2 > Cone_Tolerance && t1 < Max_Distance && z >= 0.0 && z <= 1.0)
          Depths[i++] = t2/len;
        }
      }
    }
  else
    {
    /* Solve intersections with a cone */
    a = D.x * D.x + D.y * D.y - D.z * D.z;
    b = D.x * P.x + D.y * P.y - D.z * P.z;
    c = P.x * P.x + P.y * P.y - P.z * P.z;

    if (fabs(a) < EPSILON)
      {
      if (fabs(b) > EPSILON)
        {
        /* One intersection */
        t1 = -0.5 * c / b;
        z = P.z + t1 * D.z;
        if (t1 > Cone_Tolerance && t1 < Max_Distance && z >= Cone->dist && z <= 1.0)
          Depths[i++] = t1/len;
        }
      }
    else
      {
      /* Check hits against the side of the cone */
      d = b * b - a * c;
      if (d >= 0.0)
        {
        d = sqrt(d);
        t1 = (-b - d) / a;
        t2 = (-b + d) / a;
        z = P.z + t1 * D.z;
        if (t1 > Cone_Tolerance && t1 < Max_Distance && z >= Cone->dist && z <= 1.0)
          Depths[i++] = t1/len;
        z = P.z + t2 * D.z;
        if (t2 > Cone_Tolerance && t1 < Max_Distance && z >= Cone->dist && z <= 1.0)
          Depths[i++] = t2/len;
        }
      }
    }

  if (Cone->closed)
    {
    d = (1.0 - P.z) / D.z;
    a = (P.x + d * D.x);
    b = (P.y + d * D.y);
    if ((a * a + b * b) <= 1.0 && d > Cone_Tolerance && d < Max_Distance)
      Depths[i++] = d/len;
    d = (Cone->dist - P.z) / D.z;
    a = (P.x + d * D.x);
    b = (P.y + d * D.y);
    if ((a * a + b * b) <= (Cone->cyl_flag ? 1.0 : Cone->dist*Cone->dist)
      && d > Cone_Tolerance && d < Max_Distance)
      Depths[i++] = d/len;
    }

  Ray_Cone_Tests_Succeeded +=i;

  return(i);
  }

int Inside_Cone (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  VECTOR New_Point;
  CONE *Cone = (CONE *) Object;
  DBL w2, z2, offset = (Cone->Inverted ? -EPSILON : EPSILON);

  /* Transform the point into the cones space */
  MInvTransPoint(&New_Point, IPoint, Cone->Trans);

  /* Test to see if we are inside the cone */
  w2 = New_Point.x * New_Point.x + New_Point.y * New_Point.y;
  if (Cone->cyl_flag) 
    {
    /* Check to see if we are inside a cylinder */
    if (w2 > 1.0 + offset ||
      New_Point.z < 0.0 - offset ||
      New_Point.z > 1.0 + offset)
      return Cone->Inverted;
    else
      return 1 - Cone->Inverted;
    }
  else 
    {
    /* Check to see if we are inside a cone */
    z2 = New_Point.z * New_Point.z;
    if (w2 > z2 + offset ||
      New_Point.z < Cone->dist - offset ||
      New_Point.z > 1.0+offset)
      return Cone->Inverted;
    else
      return 1 - Cone->Inverted;
    }
  }

void Cone_Normal (Result, Object, IPoint)
OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  CONE *Cone = (CONE *) Object;

  /* Transform the point into the cones space */
  MInvTransPoint(Result, IPoint, Cone->Trans);

  /* Calculating the normal is real simple in canonical cone space */
  if (Result->z > (1-EPSILON))
    Make_Vector(Result,0.0,0.0,1.0)
else
  if (Result->z < (Cone->dist+EPSILON))
    Make_Vector(Result,0.0,0.0,-1.0)
else
  if (Cone->cyl_flag)
    Result->z = 0.0;
  else
    Result->z = -Result->z;

/* Transform the point out of the cones space */
MTransNormal(Result, Result, Cone->Trans);
VNormalize(*Result, *Result);
}

void *Copy_Cone (Object)
OBJECT *Object;
{
CONE *New;

New  = Create_Cone();
*New = *((CONE *) Object);

New->Trans = Copy_Transform(((CONE *)Object)->Trans);

return (New);
}

void Translate_Cone (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
{
TRANSFORM Trans;

Compute_Translation_Transform(&Trans, Vector);
Transform_Cone(Object, &Trans);
}

void Rotate_Cone (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
{
TRANSFORM Trans;
Compute_Rotation_Transform(&Trans, Vector);
Transform_Cone(Object, &Trans);
}

void Scale_Cone (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
{
TRANSFORM Trans;

Compute_Scaling_Transform(&Trans, Vector);
Transform_Cone(Object, &Trans);
}

void Invert_Cone (Object)
OBJECT *Object;
{
((CONE *)Object)->Inverted = 1 - ((CONE *)Object)->Inverted;
}

void Transform_Cone (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
{
CONE *Cone = (CONE *)Object;
Compose_Transforms(Cone->Trans, Trans);

/* Recalculate the bounds */
Make_Vector(&Cone->Bounds.Lower_Left, -1.0, -1.0, 0.0);
Make_Vector(&Cone->Bounds.Lengths, 2.0, 2.0, 1.0);
recompute_bbox(&Cone->Bounds, Cone->Trans);
}

CONE *Create_Cone ()
{
CONE *New;

if ((New = (CONE *) malloc (sizeof (CONE))) == NULL)
MAError ("cone");

INIT_OBJECT_FIELDS(New, CONE_OBJECT, &Cone_Methods)
Make_Vector (&(New->apex), 0.0, 0.0, 1.0);
Make_Vector (&(New->base), 0.0, 0.0, 0.0);
New->apex_radius = 1.0;
New->base_radius = 0.0;
New->dist = 0.0;
New->Trans = Create_Transform();
New->Inverted = FALSE;
New->cyl_flag = 0; /* This is a Cone */
New->closed   = 1; /* Has capped ends*/

/* Default bounds */
Make_Vector(&New->Bounds.Lower_Left, -1.0, -1.0, 0.0);
Make_Vector(&New->Bounds.Lengths, 2.0, 2.0, 1.0);

return New;
}

CONE *Create_Cylinder ()
{
CONE *New;

if ((New = (CONE *) malloc (sizeof (CONE))) == NULL)
MAError ("cone");

INIT_OBJECT_FIELDS(New, CONE_OBJECT, &Cone_Methods)
Make_Vector (&(New->apex), 0.0, 0.0, 1.0);
Make_Vector (&(New->base), 0.0, 0.0, 0.0);
New->apex_radius = 1.0;
New->base_radius = 1.0;
New->dist = 0.0;
New->Trans = Create_Transform();
New->Inverted = FALSE;
New->cyl_flag = 1; /* This is a Cylinder */
New->closed   = 1; /* Has capped ends*/

/* Default bounds */
Make_Vector(&New->Bounds.Lower_Left, -1.0, -1.0, 0.0);
Make_Vector(&New->Bounds.Lengths, 2.0, 2.0, 1.0);

return New;
}

void Compute_Cone_Data(Object)
OBJECT *Object;
{
DBL tlen, len, tmpf;
VECTOR tmpv, axis, origin;
CONE *Cone = (CONE *)Object;

/* Process the primitive specific information */
if(Cone->apex_radius < Cone->base_radius) 
  {
  /* Want the bigger end at the top */
  tmpv = Cone->base;
  Cone->base = Cone->apex;
  Cone->apex = tmpv;
  tmpf = Cone->base_radius;
  Cone->base_radius = Cone->apex_radius;
  Cone->apex_radius = tmpf;
  }
else if (fabs(Cone->apex_radius - Cone->base_radius) < EPSILON) 
  {
  /* What we are dealing with here is really a cylinder */
  Cone->cyl_flag = 1;
  Compute_Cylinder_Data(Object);
  return;
  }

/* Find the axis and axis length */
VSub(axis, Cone->apex, Cone->base);
VLength(len, axis);
if (len < EPSILON)
Error("Degenerate cone/cylinder\n");
else
  VInverseScaleEq(axis, len)

    /* Determine alignment */
    tmpf = Cone->base_radius *
    len / (Cone->apex_radius - Cone->base_radius);
VScale(origin, axis, tmpf);
VSub(origin, Cone->base, origin);
tlen = tmpf + len;
Cone->dist = tmpf / tlen;
Compute_Coordinate_Transform(Cone->Trans, &origin, &axis,
Cone->apex_radius, tlen);

/* Recalculate the bounds */
Make_Vector(&Cone->Bounds.Lower_Left, -1.0, -1.0, 0.0);
Make_Vector(&Cone->Bounds.Lengths, 2.0, 2.0, 1.0);
recompute_bbox(&Cone->Bounds, Cone->Trans);
}

void Compute_Cylinder_Data(Object)
OBJECT *Object;
{
CONE *Cone = (CONE *)Object;
VECTOR axis;
DBL tmpf;

VSub(axis, Cone->apex, Cone->base);
VLength(tmpf, axis);
if (tmpf < EPSILON)
Error("Degenerate cylinder, base point = apex point\n");
else
  VInverseScaleEq(axis, tmpf)
    Compute_Coordinate_Transform(Cone->Trans, &Cone->base, &axis,
      Cone->apex_radius, tmpf);

Cone->dist = 0.0;
/* Recalculate the bounds */
Make_Vector(&Cone->Bounds.Lower_Left, -1.0, -1.0, 0.0);
Make_Vector(&Cone->Bounds.Lengths, 2.0, 2.0, 1.0);
recompute_bbox(&Cone->Bounds, Cone->Trans);
}


void Destroy_Cone (Object)
OBJECT *Object;
{
Destroy_Transform(((CONE *)Object)->Trans);
free (Object);
}
