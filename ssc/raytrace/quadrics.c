/****************************************************************************
*                quadrics.c
*
*  This module implements the code for the quadric shape primitive.
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

METHODS Quadric_Methods =
  { 
  All_Quadric_Intersections,
  Inside_Quadric, Quadric_Normal,
  Copy_Quadric,
  Translate_Quadric, Rotate_Quadric,
  Scale_Quadric, Transform_Quadric, Invert_Quadric,
  Destroy_Quadric
};

extern RAY *CM_Ray;
extern long Ray_Quadric_Tests, Ray_Quadric_Tests_Succeeded;

int All_Quadric_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  DBL Depth1, Depth2;
  VECTOR IPoint;
  register int Intersection_Found;

  Intersection_Found = FALSE;

  if (Intersect_Quadric (Ray, (QUADRIC *) Object, &Depth1, &Depth2))
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

int Intersect_Quadric (Ray, Quadric, Depth1, Depth2)
RAY *Ray;
QUADRIC *Quadric;
DBL *Depth1, *Depth2;
  {
  register DBL Square_Term, Linear_Term, Constant_Term, Temp_Term;
  register DBL Determinant, Determinant_2, A2, BMinus;

  Ray_Quadric_Tests++;
  if (!Ray->Quadric_Constants_Cached)
    Make_Ray(Ray);

  if (Quadric->Non_Zero_Square_Term)
    {
    VDot (Square_Term, Quadric->Square_Terms, Ray->Direction_2);
    VDot (Temp_Term, Quadric->Mixed_Terms, Ray->Mixed_Dir_Dir);
    Square_Term += Temp_Term;
    }
  else
    Square_Term = 0.0;

  VDot (Linear_Term, Quadric->Square_Terms, Ray->Initial_Direction);
  Linear_Term *= 2.0;
  VDot (Temp_Term, Quadric->Terms, Ray->Direction);
  Linear_Term += Temp_Term;
  VDot (Temp_Term, Quadric->Mixed_Terms, Ray->Mixed_Init_Dir);
  Linear_Term += Temp_Term;

  if (Ray == CM_Ray)
    if (!Quadric->Constant_Cached)
      {
      VDot (Constant_Term, Quadric->Square_Terms, Ray->Initial_2);
      VDot (Temp_Term, Quadric->Terms, Ray->Initial);
      Constant_Term +=  Temp_Term + Quadric->Constant;
      Quadric->CM_Constant = Constant_Term;
      Quadric->Constant_Cached = TRUE;
      }
    else
      Constant_Term = Quadric->CM_Constant;
  else
    {
    VDot (Constant_Term, Quadric->Square_Terms, Ray->Initial_2);
    VDot (Temp_Term, Quadric->Terms, Ray->Initial);
    Constant_Term += Temp_Term + Quadric->Constant;
    }

  VDot (Temp_Term, Quadric->Mixed_Terms, 
    Ray->Mixed_Initial_Initial);
  Constant_Term += Temp_Term;

  if (Square_Term != 0.0)
    {
    /* The equation is quadratic - find its roots */

    Determinant_2 = Linear_Term * Linear_Term - 4.0 * Square_Term * Constant_Term;

    if (Determinant_2 < 0.0)
      return (FALSE);

    Determinant = sqrt (Determinant_2);
    A2 = Square_Term * 2.0;
    BMinus = Linear_Term * -1.0;

    *Depth1 = (BMinus + Determinant) / A2;
    *Depth2 = (BMinus - Determinant) / A2;
    }
  else
    {
    /* There are no quadratic terms.  Solve the linear equation instead. */
    if (Linear_Term == 0.0)
      return (FALSE);

    *Depth1 = Constant_Term * -1.0 / Linear_Term;
    *Depth2 = *Depth1;
    }

  if ((*Depth1 < Small_Tolerance) || (*Depth1 > Max_Distance))
    if ((*Depth2 < Small_Tolerance) || (*Depth2 > Max_Distance))
      return (FALSE);
    else
      *Depth1 = *Depth2;
  else
    if ((*Depth2 < Small_Tolerance) || (*Depth2 > Max_Distance))
      *Depth2 = *Depth1;

  Ray_Quadric_Tests_Succeeded++;
  return (TRUE);
  }

int Inside_Quadric (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  QUADRIC *Quadric = (QUADRIC *) Object;
  VECTOR New_Point;
  register DBL Result, Linear_Term, Square_Term;

  VDot (Linear_Term, *IPoint, Quadric->Terms);
  Result = Linear_Term + Quadric->Constant;
  VSquareTerms (New_Point, *IPoint);
  VDot (Square_Term, New_Point, Quadric->Square_Terms);
  Result += Square_Term;
  Result += Quadric->Mixed_Terms.x * (IPoint->x) * (IPoint->y)
    + Quadric->Mixed_Terms.y * (IPoint->x) * (IPoint->z)
      + Quadric->Mixed_Terms.z * (IPoint->y) * (IPoint->z);

  if (Result < Small_Tolerance)
    return (TRUE);

  return (FALSE);
  }

void Quadric_Normal (Result, Object, IPoint)
VECTOR *Result, *IPoint;
OBJECT *Object;
  {
  QUADRIC *Intersection_Quadric = (QUADRIC *) Object;
  VECTOR Derivative_Linear;
  DBL Len;

  VScale (Derivative_Linear, Intersection_Quadric->Square_Terms, 2.0);
  VEvaluate (*Result, Derivative_Linear, *IPoint);
  VAdd (*Result, *Result, Intersection_Quadric->Terms);

  Result->x += 
  Intersection_Quadric->Mixed_Terms.x * IPoint->y +
  Intersection_Quadric->Mixed_Terms.y * IPoint->z;


  Result->y +=
  Intersection_Quadric->Mixed_Terms.x * IPoint->x +
  Intersection_Quadric->Mixed_Terms.z * IPoint->z;

  Result->z +=
  Intersection_Quadric->Mixed_Terms.y * IPoint->x +
  Intersection_Quadric->Mixed_Terms.z * IPoint->y;

  Len = Result->x * Result->x + Result->y * Result->y + Result->z * Result->z;
  Len = sqrt(Len);
  if (Len == 0.0) 
    {
    /* The normal is not defined at this point of the surface.  Set it
         to any arbitrary direction. */
    Result->x = 1.0;
    Result->y = 0.0;
    Result->z = 0.0;
    }
  else 
    {
    Result->x /= Len;		/* normalize the normal */
    Result->y /= Len;
    Result->z /= Len;
    }
  }

  void Transform_Quadric (Object, Trans)
    OBJECT *Object;
TRANSFORM *Trans;
  {
  QUADRIC *Quadric=(QUADRIC *)Object;
  MATRIX Quadric_Matrix, Transform_Transposed;

  Quadric_To_Matrix (Quadric, (MATRIX *) &Quadric_Matrix[0][0]);
  MTimes ((MATRIX *) &Quadric_Matrix[0][0], (MATRIX *) &(Trans->inverse[0][0]), (MATRIX *) &Quadric_Matrix[0][0]);
  MTranspose ((MATRIX *) &Transform_Transposed[0][0], (MATRIX *) &(Trans->inverse[0][0]));
  MTimes ((MATRIX *) &Quadric_Matrix[0][0], (MATRIX *) &Quadric_Matrix[0][0], (MATRIX *) &Transform_Transposed[0][0]);
  Matrix_To_Quadric ((MATRIX *) &Quadric_Matrix[0][0], Quadric);
  }

void Quadric_To_Matrix (Quadric, Matrix)
QUADRIC *Quadric;
MATRIX *Matrix;
  {
  MZero (Matrix);
  (*Matrix)[0][0] = Quadric->Square_Terms.x;
  (*Matrix)[1][1] = Quadric->Square_Terms.y;
  (*Matrix)[2][2] = Quadric->Square_Terms.z;
  (*Matrix)[0][1] = Quadric->Mixed_Terms.x;
  (*Matrix)[0][2] = Quadric->Mixed_Terms.y;
  (*Matrix)[0][3] = Quadric->Terms.x;
  (*Matrix)[1][2] = Quadric->Mixed_Terms.z;
  (*Matrix)[1][3] = Quadric->Terms.y;
  (*Matrix)[2][3] = Quadric->Terms.z;
  (*Matrix)[3][3] = Quadric->Constant;
  }

void Matrix_To_Quadric (Matrix, Quadric)
MATRIX *Matrix;
QUADRIC *Quadric;
  {
  Quadric->Square_Terms.x = (*Matrix)[0][0];
  Quadric->Square_Terms.y = (*Matrix)[1][1];
  Quadric->Square_Terms.z = (*Matrix)[2][2];
  Quadric->Mixed_Terms.x = (*Matrix)[0][1] + (*Matrix)[1][0];
  Quadric->Mixed_Terms.y = (*Matrix)[0][2] + (*Matrix)[2][0];
  Quadric->Terms.x = (*Matrix)[0][3] + (*Matrix)[3][0];
  Quadric->Mixed_Terms.z = (*Matrix)[1][2] + (*Matrix)[2][1];
  Quadric->Terms.y = (*Matrix)[1][3] + (*Matrix)[3][1];
  Quadric->Terms.z = (*Matrix)[2][3] + (*Matrix)[3][2];
  Quadric->Constant = (*Matrix)[3][3];
  }

void Translate_Quadric (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Translation_Transform (&Trans, Vector);
  Transform_Quadric (Object, &Trans);

  }

void Rotate_Quadric (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Rotation_Transform (&Trans, Vector);
  Transform_Quadric (Object, &Trans);
  }

void Scale_Quadric (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Scaling_Transform (&Trans, Vector);
  Transform_Quadric (Object, &Trans);
  }

void Invert_Quadric (Object)
OBJECT *Object;
  {
  QUADRIC *Quadric = (QUADRIC *) Object;

  VScaleEq (Quadric->Square_Terms, -1.0);
  VScaleEq (Quadric->Mixed_Terms, -1.0);
  VScaleEq (Quadric->Terms, -1.0);
  Quadric->Constant *= -1.0;
  }

QUADRIC *Create_Quadric()
  {
  QUADRIC *New;

  if ((New = (QUADRIC *) malloc (sizeof (QUADRIC))) == NULL)
    MAError ("quadric");

  INIT_OBJECT_FIELDS(New, QUADRIC_OBJECT, &Quadric_Methods)
    Make_Vector (&(New->Square_Terms), 1.0, 1.0, 1.0);
  Make_Vector (&(New->Mixed_Terms), 0.0, 0.0, 0.0);
  Make_Vector (&(New->Terms), 0.0, 0.0, 0.0);
  New->Constant = 1.0;
  New->CM_Constant = HUGE_VAL;
  New->Constant_Cached = FALSE;
  New->Non_Zero_Square_Term = FALSE;
  return (New);
  }

void *Copy_Quadric (Object)
OBJECT *Object;
  {
  QUADRIC *New;

  New = Create_Quadric ();
  *New = *((QUADRIC *) Object);

  return (New);
  }

void Destroy_Quadric (Object)
OBJECT *Object;
  {
  free (Object);
  }
