/****************************************************************************
*                triangle.c
*
*  This module implements primitives for triangles and smooth triangles.
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

METHODS Triangle_Methods = 
  {
  All_Triangle_Intersections,
  Inside_Triangle, Triangle_Normal,
  Copy_Triangle,
  Translate_Triangle, Rotate_Triangle,
  Scale_Triangle, Transform_Triangle, Invert_Triangle, Destroy_Triangle
  };

METHODS Smooth_Triangle_Methods = 
  {
  All_Triangle_Intersections,
  Inside_Triangle, Smooth_Triangle_Normal,
  Copy_Smooth_Triangle,
  Translate_Smooth_Triangle, Rotate_Smooth_Triangle,
  Scale_Smooth_Triangle, Transform_Smooth_Triangle, 
  Invert_Smooth_Triangle, Destroy_Triangle
  };

extern RAY *CM_Ray;
extern long Ray_Triangle_Tests, Ray_Triangle_Tests_Succeeded;

#define max3(x,y,z) ((x>y)?((x>z)?1:3):((y>z)?2:3))

#define MAX3(x,y,z) (((x)>(y))?(((x)>(z))?(x):(z)):(((y)>(z))?(y):(z)))
#define MIN3(x,y,z) (((x)<(y))?(((x)<(z))?(x):(z)):(((y)<(z))?(y):(z)))

void Find_Triangle_Dominant_Axis(Triangle)
TRIANGLE *Triangle;
  {
  DBL x, y, z;

  x = fabs(Triangle->Normal_Vector.x);
  y = fabs (Triangle->Normal_Vector.y);
  z = fabs (Triangle->Normal_Vector.z);
  switch (max3(x, y, z)) 
  {
  case 1: 
    Triangle->Dominant_Axis = X_AXIS;
    break;
  case 2: 
    Triangle->Dominant_Axis = Y_AXIS;
    break;
  case 3: 
    Triangle->Dominant_Axis = Z_AXIS;
    break;
  }
  }

void Compute_Smooth_Triangle (Triangle)
SMOOTH_TRIANGLE *Triangle;
  {
  VECTOR P3MinusP2, VTemp1, VTemp2;
  DBL x, y, z, uDenominator, Proj;

  VSub (P3MinusP2, Triangle->P3, Triangle->P2);
  x = fabs (P3MinusP2.x);
  y = fabs (P3MinusP2.y);
  z = fabs (P3MinusP2.z);

  switch (max3 (x, y, z)) 
  {
  case 1:  
    Triangle->vAxis = X_AXIS;
    Triangle->BaseDelta = P3MinusP2.x;
    break;

  case 2:  
    Triangle->vAxis = Y_AXIS;
    Triangle->BaseDelta = P3MinusP2.y;
    break;

  case 3:  
    Triangle->vAxis = Z_AXIS;
    Triangle->BaseDelta = P3MinusP2.z;
    break;
  }   

  VSub (VTemp1, Triangle->P2, Triangle->P3);
  VNormalize (VTemp1, VTemp1);
  VSub (VTemp2, Triangle->P1, Triangle->P3);
  VDot (Proj, VTemp2, VTemp1);
  VScaleEq (VTemp1, Proj);
  VSub (Triangle->Perp, VTemp1, VTemp2);
  VNormalize (Triangle->Perp, Triangle->Perp);
  VDot (uDenominator, VTemp2, Triangle->Perp);
  VInverseScaleEq (Triangle->Perp, -uDenominator);
  }

int Compute_Triangle (Triangle,Smooth)
TRIANGLE *Triangle;
int Smooth;
  {
  VECTOR V1, V2, Temp;
  DBL Length, T1, T2, T3;

  VSub (V1, Triangle->P1, Triangle->P2);
  VSub (V2, Triangle->P3, Triangle->P2);
  VCross (Triangle->Normal_Vector, V1, V2);
  VLength (Length, Triangle->Normal_Vector);
  /* Set up a flag so we can ignore degenerate triangles */
  if (Length < 1.0e-9)
    {
    Triangle->Degenerate_Flag = TRUE;
    return (0);
    }

  /* Normalize the normal vector. */
  VInverseScaleEq (Triangle->Normal_Vector, Length);

  VDot (Triangle->Distance, Triangle->Normal_Vector, Triangle->P1);
  Triangle->Distance *= -1.0;
  Find_Triangle_Dominant_Axis(Triangle);

  switch (Triangle->Dominant_Axis) 
  {
  case X_AXIS:
    if ((Triangle->P2.y - Triangle->P3.y)*(Triangle->P2.z - Triangle->P1.z) <
      (Triangle->P2.z - Triangle->P3.z)*(Triangle->P2.y - Triangle->P1.y)) 
      {
      Temp = Triangle->P2;
      Triangle->P2 = Triangle->P1;
      Triangle->P1 = Temp;
      if (Smooth) 
        {
        Temp = ((SMOOTH_TRIANGLE *) Triangle)->N2;
        ((SMOOTH_TRIANGLE *) Triangle)->N2 = ((SMOOTH_TRIANGLE *) Triangle)->N1;
        ((SMOOTH_TRIANGLE *) Triangle)->N1 = Temp;
        }
      }
    break;

  case Y_AXIS:
    if ((Triangle->P2.x - Triangle->P3.x)*(Triangle->P2.z - Triangle->P1.z) <
      (Triangle->P2.z - Triangle->P3.z)*(Triangle->P2.x - Triangle->P1.x)) 
      {
      Temp = Triangle->P2;
      Triangle->P2 = Triangle->P1;
      Triangle->P1 = Temp;
      if (Smooth) 
        {
        Temp = ((SMOOTH_TRIANGLE *) Triangle)->N2;
        ((SMOOTH_TRIANGLE *) Triangle)->N2 = ((SMOOTH_TRIANGLE *) Triangle)->N1;
        ((SMOOTH_TRIANGLE *) Triangle)->N1 = Temp;
        }
      }
    break;

  case Z_AXIS:
    if ((Triangle->P2.x - Triangle->P3.x)*(Triangle->P2.y - Triangle->P1.y) <
      (Triangle->P2.y - Triangle->P3.y)*(Triangle->P2.x - Triangle->P1.x)) 
      {
      Temp = Triangle->P2;
      Triangle->P2 = Triangle->P1;
      Triangle->P1 = Temp;
      if (Smooth) 
        {
        Temp = ((SMOOTH_TRIANGLE *) Triangle)->N2;
        ((SMOOTH_TRIANGLE *) Triangle)->N2 = ((SMOOTH_TRIANGLE *) Triangle)->N1;
        ((SMOOTH_TRIANGLE *) Triangle)->N1 = Temp;
        }
      }
    break;
  }

  if (Smooth)
    Compute_Smooth_Triangle((SMOOTH_TRIANGLE *) Triangle);

  /* Build the bounding information from the vertices */
  /* Use temps so macro not too big. */
  T1 = MIN3(Triangle->P1.x, Triangle->P2.x, Triangle->P3.x);
  T2 = MIN3(Triangle->P1.y, Triangle->P2.y, Triangle->P3.y);
  T3 = MIN3(Triangle->P1.z, Triangle->P2.z, Triangle->P3.z);
  Make_Vector(&Triangle->Bounds.Lower_Left,T1,T2,T3);

  T1 = MAX3(Triangle->P1.x, Triangle->P2.x, Triangle->P3.x);
  T2 = MAX3(Triangle->P1.y, Triangle->P2.y, Triangle->P3.y);
  T3 = MAX3(Triangle->P1.z, Triangle->P2.z, Triangle->P3.z);
  Make_Vector(&Triangle->Bounds.Lengths,T1,T2,T3);

  VSub(Triangle->Bounds.Lengths, Triangle->Bounds.Lengths,
    Triangle->Bounds.Lower_Left);
  Triangle->Bounds.Lengths.x += EPSILON;
  Triangle->Bounds.Lengths.y += EPSILON;
  Triangle->Bounds.Lengths.z += EPSILON;
  return (1);
  }

int All_Triangle_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  DBL Depth;
  VECTOR IPoint;

  if (Intersect_Triangle (Ray, (TRIANGLE *)Object, &Depth))
    {
    VScale (IPoint, Ray -> Direction, Depth);
    VAddEq (IPoint, Ray -> Initial);
    if (Point_In_Clip(&IPoint,Object->Clip))
      {
      push_entry(Depth,IPoint,Object,Depth_Stack);
      return (TRUE);
      }
    }
  return (FALSE);
  }

int Intersect_Triangle (Ray, Triangle, Depth)
RAY *Ray;
TRIANGLE *Triangle;
DBL *Depth;
  {
  DBL NormalDotOrigin, NormalDotDirection;
  DBL s, t;

  Ray_Triangle_Tests++;
  if(Triangle->Degenerate_Flag)
    return(FALSE);

  if (Ray == CM_Ray) 
    {
    if (!Triangle->CMCached) 
      {
      VDot (Triangle->CMNormDotOrigin, Triangle->Normal_Vector, Ray->Initial);
      Triangle->CMNormDotOrigin += Triangle->Distance;
      Triangle->CMNormDotOrigin *= -1.0;
      Triangle->CMCached = TRUE;
      }

    VDot (NormalDotDirection, Triangle->Normal_Vector, Ray->Direction);
    if ((NormalDotDirection < Small_Tolerance) &&
      (NormalDotDirection > -Small_Tolerance))
      return (FALSE);

    *Depth = Triangle->CMNormDotOrigin / NormalDotDirection;
    }
  else 
    {
    VDot (NormalDotOrigin, Triangle->Normal_Vector, Ray->Initial);
    NormalDotOrigin += Triangle->Distance;
    NormalDotOrigin *= -1.0;

    VDot (NormalDotDirection, Triangle->Normal_Vector, Ray->Direction);
    if ((NormalDotDirection < Small_Tolerance) &&
      (NormalDotDirection > -Small_Tolerance))
      return (FALSE);

    *Depth = NormalDotOrigin / NormalDotDirection;
    }

  if ((*Depth < Small_Tolerance) || (*Depth > Max_Distance))
    return (FALSE);

  switch (Triangle->Dominant_Axis) 
  {
  case X_AXIS:
    s = Ray->Initial.y + *Depth * Ray->Direction.y;
    t = Ray->Initial.z + *Depth * Ray->Direction.z;

    if ((Triangle->P2.y - s)*(Triangle->P2.z - Triangle->P1.z) <
      (Triangle->P2.z - t)*(Triangle->P2.y - Triangle->P1.y))
      return (FALSE);

    if ((Triangle->P3.y - s)*(Triangle->P3.z - Triangle->P2.z) <
      (Triangle->P3.z - t)*(Triangle->P3.y - Triangle->P2.y))
      return (FALSE);

    if ((Triangle->P1.y - s)*(Triangle->P1.z - Triangle->P3.z) <
      (Triangle->P1.z - t)*(Triangle->P1.y - Triangle->P3.y))
      return (FALSE);

    Ray_Triangle_Tests_Succeeded++;
    return (TRUE);

  case Y_AXIS:
    s = Ray->Initial.x + *Depth * Ray->Direction.x;
    t = Ray->Initial.z + *Depth * Ray->Direction.z;

    if ((Triangle->P2.x - s)*(Triangle->P2.z - Triangle->P1.z) <
      (Triangle->P2.z - t)*(Triangle->P2.x - Triangle->P1.x))
      return (FALSE);

    if ((Triangle->P3.x - s)*(Triangle->P3.z - Triangle->P2.z) <
      (Triangle->P3.z - t)*(Triangle->P3.x - Triangle->P2.x))
      return (FALSE);

    if ((Triangle->P1.x - s)*(Triangle->P1.z - Triangle->P3.z) <
      (Triangle->P1.z - t)*(Triangle->P1.x - Triangle->P3.x))
      return (FALSE);

    Ray_Triangle_Tests_Succeeded++;
    return (TRUE);

  case Z_AXIS:
    s = Ray->Initial.x + *Depth * Ray->Direction.x;
    t = Ray->Initial.y + *Depth * Ray->Direction.y;

    if ((Triangle->P2.x - s)*(Triangle->P2.y - Triangle->P1.y) <
      (Triangle->P2.y - t)*(Triangle->P2.x - Triangle->P1.x))
      return (FALSE);

    if ((Triangle->P3.x - s)*(Triangle->P3.y - Triangle->P2.y) <
      (Triangle->P3.y - t)*(Triangle->P3.x - Triangle->P2.x))
      return (FALSE);

    if ((Triangle->P1.x - s)*(Triangle->P1.y - Triangle->P3.y) <
      (Triangle->P1.y - t)*(Triangle->P1.x - Triangle->P3.x))
      return (FALSE);

    Ray_Triangle_Tests_Succeeded++;
    return (TRUE);
  }
  return (FALSE);
  }

int Inside_Triangle (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  return (FALSE);
  }

void Triangle_Normal (Result, Object, IPoint)
OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  *Result = ((TRIANGLE *)Object)->Normal_Vector;
  }

void *Copy_Triangle (Object)
OBJECT *Object;
  {
  TRIANGLE *New;

  New = Create_Triangle ();
  *New = * ((TRIANGLE *)Object);

  return (New);
  }

void Translate_Triangle (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRIANGLE *Triangle = (TRIANGLE *) Object;
  VECTOR Translation;

  VEvaluate (Translation, Triangle->Normal_Vector, *Vector);
  Triangle->Distance -= Translation.x + Translation.y + Translation.z;
  VAddEq (Triangle->P1, *Vector)
    VAddEq (Triangle->P2, *Vector)
      VAddEq (Triangle->P3, *Vector)

        /* Recalculate the bounds */
        VAddEq(Object->Bounds.Lower_Left, *Vector);
  }

void Rotate_Triangle (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Rotation_Transform (&Trans, Vector);
  Transform_Triangle (Object, &Trans);
  }

void Scale_Triangle (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRIANGLE *Triangle = (TRIANGLE *) Object;
  DBL Length,T1,T2,T3;

  Triangle->Normal_Vector.x = Triangle->Normal_Vector.x / Vector->x;
  Triangle->Normal_Vector.y = Triangle->Normal_Vector.y / Vector->y;
  Triangle->Normal_Vector.z = Triangle->Normal_Vector.z / Vector->z;

  VLength(Length, Triangle->Normal_Vector);
  VInverseScaleEq (Triangle->Normal_Vector, Length);
  Triangle->Distance /= Length;

  VEvaluateEq (Triangle->P1, *Vector);
  VEvaluateEq (Triangle->P2, *Vector);
  VEvaluateEq (Triangle->P3, *Vector);

  /* Recompute the bounds */
  /* Use temps so macro not too big. */
  T1 = MIN3(Triangle->P1.x, Triangle->P2.x, Triangle->P3.x);
  T2 = MIN3(Triangle->P1.y, Triangle->P2.y, Triangle->P3.y);
  T3 = MIN3(Triangle->P1.z, Triangle->P2.z, Triangle->P3.z);
  Make_Vector(&Triangle->Bounds.Lower_Left,T1,T2,T3);

  T1 = MAX3(Triangle->P1.x, Triangle->P2.x, Triangle->P3.x);
  T2 = MAX3(Triangle->P1.y, Triangle->P2.y, Triangle->P3.y);
  T3 = MAX3(Triangle->P1.z, Triangle->P2.z, Triangle->P3.z);
  Make_Vector(&Triangle->Bounds.Lengths,T1,T2,T3);

  VSub(Triangle->Bounds.Lengths, Triangle->Bounds.Lengths,
    Triangle->Bounds.Lower_Left);
  Triangle->Bounds.Lengths.x += EPSILON;
  Triangle->Bounds.Lengths.y += EPSILON;
  Triangle->Bounds.Lengths.z += EPSILON;
  }

void Transform_Triangle (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  TRIANGLE *Triangle = (TRIANGLE *) Object;

  MTransPoint (&Triangle->Normal_Vector,
    &Triangle->Normal_Vector, Trans);
  MTransPoint (&Triangle->P1, &Triangle->P1, Trans);
  MTransPoint (&Triangle->P2, &Triangle->P2, Trans);
  MTransPoint (&Triangle->P3, &Triangle->P3, Trans);
  Compute_Triangle (Triangle,FALSE);
  }

TRIANGLE *Create_Triangle()
  {
  TRIANGLE *New;

  if ((New = (TRIANGLE *) malloc (sizeof (TRIANGLE))) == NULL)
    MAError ("triangle");

  INIT_OBJECT_FIELDS(New,TRIANGLE_OBJECT,&Triangle_Methods)

    Make_Vector (&(New -> Normal_Vector), 0.0, 1.0, 0.0);
  New -> Distance = 0.0;
  New -> CMNormDotOrigin = 0.0;
  New -> CMCached = FALSE;
  Make_Vector (&(New -> P1), 0.0, 0.0, 0.0);
  Make_Vector (&(New -> P2), 1.0, 0.0, 0.0);
  Make_Vector (&(New -> P3), 0.0, 1.0, 0.0);
  New -> Degenerate_Flag = FALSE;

  /* NOTE: Dominant_Axis is computed when Parse_Triangle calls
   Compute_Triangle.  vAxis is used only for smooth triangles */

  return (New);
  }

void Invert_Triangle (Object)
OBJECT *Object;
  {
  return;
  }

/* Calculate the Phong-interpolated vector within the triangle
   at the given intersection point. The math for this is a bit
   bizarre:

    -         P1
    |        /|\ \
    |       / |Perp\
    |      /  V  \   \
    |     /   |    \   \
  u |    /____|_____PI___\
    |   /     |       \    \
    -  P2-----|--------|----P3
              Pbase    PIntersect
        |-------------------|
                       v

   Triangle->Perp is a unit vector from P1 to Pbase. We calculate

   u = (PI - P1) DOT Perp / ((P3 - P1) DOT Perp).

   We then calculate where the line from P1 to PI intersects the line P2 to P3:
   PIntersect = (PI - P1)/u.

   We really only need one coordinate of PIntersect.  We then calculate v as:

      v = PIntersect.x / (P3.x - P2.x)
 or   v = PIntersect.y / (P3.y - P2.y)
 or   v = PIntersect.z / (P3.z - P2.z)

   depending on which calculation will give us the best answers.

   Once we have u and v, we can perform the normal interpolation as:

     NTemp1 = N1 + u(N2 - N1);
     NTemp2 = N1 + u(N3 - N1);
     Result = normalize (NTemp1 + v(NTemp2 - NTemp1))

   As always, any values which are constant for the triangle are cached
   in the triangle.
*/

void Smooth_Triangle_Normal (Result, Object, IPoint)
OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  SMOOTH_TRIANGLE *Triangle = (SMOOTH_TRIANGLE *) Object;
  VECTOR PIMinusP1, NTemp1, NTemp2;
  DBL u = 0.0, v = 0.0;

  VSub (PIMinusP1, *IPoint, Triangle->P1);
  VDot (u, PIMinusP1, Triangle->Perp);
  if (u < 1.0e-9) 
    {
    *Result = Triangle->N1;
    return;
    }

  /* BaseDelta contains P3.x-P2.x,  P3.y-P2.y, or P3.z-P2.z depending on the
      value of vAxis. */

  switch (Triangle->vAxis) 
  {
  case X_AXIS:  
    v = (PIMinusP1.x/u + Triangle->P1.x - Triangle->P2.x) / Triangle->BaseDelta;
    break;

  case Y_AXIS:  
    v = (PIMinusP1.y/u + Triangle->P1.y - Triangle->P2.y) / Triangle->BaseDelta;
    break;

  case Z_AXIS:  
    v = (PIMinusP1.z/u + Triangle->P1.z - Triangle->P2.z)/ Triangle->BaseDelta;
    break;
  }

  VSub (NTemp1, Triangle->N2, Triangle->N1);
  VScaleEq (NTemp1, u);
  VAddEq (NTemp1, Triangle->N1);
  VSub (NTemp2, Triangle->N3, Triangle->N1);
  VScaleEq (NTemp2, u);
  VAddEq (NTemp2, Triangle->N1);
  VSub (*Result, NTemp2, NTemp1);
  VScaleEq (*Result, v);
  VAddEq (*Result, NTemp1); 
  VNormalize (*Result, *Result);
  }

void *Copy_Smooth_Triangle (Object)
OBJECT *Object;
  {
  SMOOTH_TRIANGLE *New;

  New = Create_Smooth_Triangle ();
  *New = * ((SMOOTH_TRIANGLE *)Object);

  return (New);
  }

void Translate_Smooth_Triangle (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  SMOOTH_TRIANGLE *Triangle = (SMOOTH_TRIANGLE *) Object;
  VECTOR Translation;

  VEvaluate (Translation, Triangle->Normal_Vector, *Vector);
  Triangle->Distance -= Translation.x + Translation.y + Translation.z;
  VAddEq (Triangle->P1, *Vector)
    VAddEq (Triangle->P2, *Vector)
      VAddEq (Triangle->P3, *Vector)
        Compute_Triangle ((TRIANGLE *) Triangle,TRUE);
  }

void Rotate_Smooth_Triangle (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Rotation_Transform (&Trans, Vector);
  Transform_Smooth_Triangle (Object, &Trans);
  }

void Scale_Smooth_Triangle (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  SMOOTH_TRIANGLE *Triangle = (SMOOTH_TRIANGLE *) Object;
  DBL Length;

  Triangle->Normal_Vector.x = Triangle->Normal_Vector.x / Vector->x;
  Triangle->Normal_Vector.y = Triangle->Normal_Vector.y / Vector->y;
  Triangle->Normal_Vector.z = Triangle->Normal_Vector.z / Vector->z;

  VLength(Length, Triangle->Normal_Vector);
  VScaleEq (Triangle->Normal_Vector, 1.0 / Length);
  Triangle->Distance /= Length;

  VEvaluateEq (Triangle->P1, *Vector);
  VEvaluateEq (Triangle->P2, *Vector);
  VEvaluateEq (Triangle->P3, *Vector);
  Compute_Triangle ((TRIANGLE *) Triangle,TRUE);
  }

void Transform_Smooth_Triangle (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  SMOOTH_TRIANGLE *Triangle = (SMOOTH_TRIANGLE *) Object;

  MTransPoint (&Triangle->Normal_Vector,
    &Triangle->Normal_Vector, Trans);
  MTransPoint (&Triangle->P1, &Triangle->P1, Trans);
  MTransPoint (&Triangle->P2, &Triangle->P2, Trans);
  MTransPoint (&Triangle->P3, &Triangle->P3, Trans);
  MTransPoint (&Triangle->N1, &Triangle->N1, Trans);
  MTransPoint (&Triangle->N2, &Triangle->N2, Trans);
  MTransPoint (&Triangle->N3, &Triangle->N3, Trans);
  Compute_Triangle ((TRIANGLE *) Triangle,TRUE);
  }

void Invert_Smooth_Triangle (Object)
OBJECT *Object;
  {
  return;
  }

SMOOTH_TRIANGLE *Create_Smooth_Triangle()
  {
  SMOOTH_TRIANGLE *New;

  if ((New = (SMOOTH_TRIANGLE *) malloc (sizeof (SMOOTH_TRIANGLE))) == NULL)
    MAError ("smooth triangle");

  INIT_OBJECT_FIELDS(New,SMOOTH_TRIANGLE_OBJECT,&Smooth_Triangle_Methods)

    Make_Vector (&(New->Normal_Vector), 0.0, 1.0, 0.0);
  New->Distance = 0.0;
  New -> CMNormDotOrigin = 0.0;
  New -> CMCached = FALSE;
  Make_Vector (&(New -> P1), 0.0, 0.0, 0.0);
  Make_Vector (&(New -> P2), 1.0, 0.0, 0.0);
  Make_Vector (&(New -> P3), 0.0, 1.0, 0.0);
  Make_Vector (&(New -> N1), 0.0, 1.0, 0.0);
  Make_Vector (&(New -> N2), 0.0, 1.0, 0.0);
  Make_Vector (&(New -> N3), 0.0, 1.0, 0.0);
  New -> BaseDelta = 0.0;
  New -> Degenerate_Flag = FALSE;

  /* NOTE: Dominant_Axis and vAxis are computed when Parse_Triangle calls
   Compute_Triangle.  */

  return (New);
  }

void Destroy_Triangle (Object)
OBJECT *Object;
  {
  free (Object);
  }
