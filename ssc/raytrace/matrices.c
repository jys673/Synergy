/****************************************************************************
*                matrices.c
*
*  This module contains code to manipulate 4x4 matrices.
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

void MZero (result)
MATRIX *result;
  {
  /* Initialize the matrix to the following values:
   0.0   0.0   0.0   0.0
   0.0   0.0   0.0   0.0
   0.0   0.0   0.0   0.0
   0.0   0.0   0.0   0.0
*/
  register int i, j;

  for (i = 0 ; i < 4 ; i++)
    for (j = 0 ; j < 4 ; j++)
    (*result)[i][j] = 0.0;
  }

void MIdentity (result)
MATRIX *result;
  {
  /* Initialize the matrix to the following values:
   1.0   0.0   0.0   0.0
   0.0   1.0   0.0   0.0
   0.0   0.0   1.0   0.0
   0.0   0.0   0.0   1.0
*/
  register int i, j;

  for (i = 0 ; i < 4 ; i++)
    for (j = 0 ; j < 4 ; j++)
    if (i == j)
      (*result)[i][j] = 1.0;
    else
      (*result)[i][j] = 0.0;
  }

void MTimes (result, matrix1, matrix2)
MATRIX *result, *matrix1, *matrix2;
  {
  register int i, j, k;
  MATRIX temp_matrix;

  for (i = 0 ; i < 4 ; i++)
    for (j = 0 ; j < 4 ; j++) 
    {
    temp_matrix[i][j] = 0.0;
    for (k = 0 ; k < 4 ; k++)
      temp_matrix[i][j] += (*matrix1)[i][k] * (*matrix2)[k][j];
    }

  for (i = 0 ; i < 4 ; i++)
    for (j = 0 ; j < 4 ; j++)
    (*result)[i][j] = temp_matrix[i][j];
  }

/*  AAC - These are not used, so they are commented out to save code space...

void MAdd (result, matrix1, matrix2)
   MATRIX *result, *matrix1, *matrix2;
   {
   register int i, j;

   for (i = 0 ; i < 4 ; i++)
      for (j = 0 ; j < 4 ; j++)
         (*result)[i][j] = (*matrix1)[i][j] + (*matrix2)[i][j];
   }

void MSub (result, matrix1, matrix2)
   MATRIX *result, *matrix1, *matrix2;
   {
   register int i, j;

   for (i = 0 ; i < 4 ; i++)
      for (j = 0 ; j < 4 ; j++)
         (*result)[i][j] = (*matrix1)[i][j] - (*matrix2)[i][j];
   }

void MScale (result, matrix1, amount)
MATRIX *result, *matrix1;
DBL amount;
{
   register int i, j;

   for (i = 0 ; i < 4 ; i++)
      for (j = 0 ; j < 4 ; j++)
	 if (amount == 1.0)
	    (*result)[i][j] = (*matrix1)[i][j]; * just copy *
	 else
            (*result)[i][j] = (*matrix1)[i][j] * amount;
   return;
}
... up to here! */

void MTranspose (result, matrix1)
MATRIX *result, *matrix1;
  {
  register int i, j;
  MATRIX temp_matrix;

  for (i = 0 ; i < 4 ; i++)
    for (j = 0 ; j < 4 ; j++)
    temp_matrix[i][j] = (*matrix1)[j][i];

  for (i = 0 ; i < 4 ; i++)
    for (j = 0 ; j < 4 ; j++)
    (*result)[i][j] = temp_matrix[i][j];
  }


void MTransPoint (result, vector, transform)
VECTOR *result, *vector;
TRANSFORM *transform;
  {
  register int i;
  DBL answer_array[4];
  MATRIX *matrix;

  matrix = (MATRIX *) transform -> matrix;

  for (i = 0 ; i < 4 ; i++)
    answer_array[i] = vector -> x * (*matrix)[0][i]
    + vector -> y * (*matrix)[1][i]
    + vector -> z * (*matrix)[2][i]
    + (*matrix)[3][i];

  result -> x  = answer_array[0];
  result -> y  = answer_array[1];
  result -> z  = answer_array[2];
  }

void MInvTransPoint (result, vector, transform)
VECTOR *result, *vector;
TRANSFORM *transform;
  {
  register int i;
  DBL answer_array[4];
  MATRIX *matrix;

  matrix = (MATRIX *) transform -> inverse;

  for (i = 0 ; i < 4 ; i++)
    answer_array[i] = vector -> x * (*matrix)[0][i]
    + vector -> y * (*matrix)[1][i]
    + vector -> z * (*matrix)[2][i]
    + (*matrix)[3][i];

  result -> x  = answer_array[0];
  result -> y  = answer_array[1];
  result -> z  = answer_array[2];
  }

void MTransDirection (result, vector, transform)
VECTOR *result, *vector;
TRANSFORM *transform;
  {
  register int i;
  DBL answer_array[4];
  MATRIX *matrix;

  matrix = (MATRIX *) transform -> matrix;

  for (i = 0 ; i < 4 ; i++)
    answer_array[i] = vector -> x * (*matrix)[0][i]
    + vector -> y * (*matrix)[1][i]
    + vector -> z * (*matrix)[2][i];

  result -> x  = answer_array[0];
  result -> y  = answer_array[1];
  result -> z  = answer_array[2];
  }

void MInvTransDirection (result, vector, transform)
VECTOR *result, *vector;
TRANSFORM *transform;
  {
  register int i;
  DBL answer_array[4];
  MATRIX *matrix;

  matrix = (MATRIX *) transform -> inverse;

  for (i = 0 ; i < 4 ; i++)
    answer_array[i] = vector -> x * (*matrix)[0][i]
    + vector -> y * (*matrix)[1][i]
    + vector -> z * (*matrix)[2][i];

  result -> x  = answer_array[0];
  result -> y  = answer_array[1];
  result -> z  = answer_array[2];
  }

void MTransNormal (result, vector, transform)
VECTOR *result, *vector;
TRANSFORM *transform;
  {
  register int i;
  DBL answer_array[3];
  MATRIX *matrix;

  matrix = (MATRIX *) transform -> inverse;

  for (i = 0 ; i < 3 ; i++)
    answer_array[i] = vector -> x * (*matrix)[i][0]
    + vector -> y * (*matrix)[i][1]
    + vector -> z * (*matrix)[i][2];

  result -> x  = answer_array[0];
  result -> y  = answer_array[1];
  result -> z  = answer_array[2];
  }

void MInvTransNormal (result, vector, transform)
VECTOR *result, *vector;
TRANSFORM *transform;
  {
  register int i;
  DBL answer_array[3];
  MATRIX *matrix;

  matrix = (MATRIX *) transform -> matrix;

  for (i = 0 ; i < 3 ; i++)
    answer_array[i] = vector -> x * (*matrix)[i][0]
    + vector -> y * (*matrix)[i][1]
    + vector -> z * (*matrix)[i][2];

  result -> x  = answer_array[0];
  result -> y  = answer_array[1];
  result -> z  = answer_array[2];
  }

void Compute_Scaling_Transform (result, vector)
TRANSFORM *result;
VECTOR *vector;
  {
  MIdentity ((MATRIX *)result -> matrix);
  (result -> matrix)[0][0] = vector -> x;
  (result -> matrix)[1][1] = vector -> y;
  (result -> matrix)[2][2] = vector -> z;

  MIdentity ((MATRIX *)result -> inverse);
  (result -> inverse)[0][0] = 1.0 / vector -> x;
  (result -> inverse)[1][1]= 1.0 / vector -> y;
  (result -> inverse)[2][2] = 1.0 / vector -> z;
  }

/* AAC - This is not used, so it's commented out...

void Compute_Inversion_Transform (result)
   TRANSFORM *result;
   {
   MIdentity ((MATRIX *)result -> matrix);
   (result -> matrix)[0][0] = -1.0;
   (result -> matrix)[1][1] = -1.0;
   (result -> matrix)[2][2] = -1.0;
   (result -> matrix)[3][3] = -1.0;


   (result -> inverse)[0][0] = -1.0;
   (result -> inverse)[1][1] = -1.0;
   (result -> inverse)[2][2] = -1.0;
   (result -> inverse)[3][3] = -1.0;
   }
... up to here! */

void Compute_Translation_Transform (transform, vector)
TRANSFORM *transform;
VECTOR *vector;
  {
  MIdentity ((MATRIX *)transform -> matrix);
  (transform -> matrix)[3][0] = vector -> x;
  (transform -> matrix)[3][1] = vector -> y;
  (transform -> matrix)[3][2] = vector -> z;

  MIdentity ((MATRIX *)transform -> inverse);
  (transform -> inverse)[3][0] = 0.0 - vector -> x;
  (transform -> inverse)[3][1] = 0.0 - vector -> y;
  (transform -> inverse)[3][2] = 0.0 - vector -> z;
  }

void Compute_Rotation_Transform (transform, vector)
TRANSFORM *transform;
VECTOR *vector;
  {
  MATRIX Matrix;
  VECTOR Radian_Vector;
  register DBL cosx, cosy, cosz, sinx, siny, sinz;

  VScale (Radian_Vector, *vector, M_PI/180.0);
  MIdentity ((MATRIX *)transform -> matrix);
  cosx = cos (Radian_Vector.x);
  sinx = sin (Radian_Vector.x);
  cosy = cos (Radian_Vector.y);
  siny = sin (Radian_Vector.y);
  cosz = cos (Radian_Vector.z);
  sinz = sin (Radian_Vector.z);

  (transform -> matrix) [1][1] = cosx;
  (transform -> matrix) [2][2] = cosx;
  (transform -> matrix) [1][2] = sinx;
  (transform -> matrix) [2][1] = 0.0 - sinx;
  MTranspose ((MATRIX *)transform -> inverse, (MATRIX *)transform -> matrix);

  MIdentity ((MATRIX *)Matrix);
  Matrix [0][0] = cosy;
  Matrix [2][2] = cosy;
  Matrix [0][2] = 0.0 - siny;
  Matrix [2][0] = siny;
  MTimes ((MATRIX *)transform -> matrix, (MATRIX *)transform -> matrix, (MATRIX *)Matrix);
  MTranspose ((MATRIX *)Matrix, (MATRIX *)Matrix);
  MTimes ((MATRIX *)transform -> inverse, (MATRIX *)Matrix, (MATRIX *)transform -> inverse);

  MIdentity ((MATRIX *)Matrix);
  Matrix [0][0] = cosz;
  Matrix [1][1] = cosz;
  Matrix [0][1] = sinz;
  Matrix [1][0] = 0.0 - sinz;
  MTimes ((MATRIX *)transform -> matrix, (MATRIX *)transform -> matrix, (MATRIX *)Matrix);
  MTranspose ((MATRIX *)Matrix, (MATRIX *)Matrix);
  MTimes ((MATRIX *)transform -> inverse, (MATRIX *)Matrix, (MATRIX *)transform -> inverse);
  }

/* AAC - This is not used so it's commented out...

void Compute_Look_At_Transform (result, Look_At, Up, Right)
   TRANSFORM *result;
   VECTOR *Look_At, *Up, *Right;
   {
   MIdentity ((MATRIX *)result -> inverse);
   (result -> matrix)[0][0] = Right->x;
   (result -> matrix)[0][1] = Right->y;
   (result -> matrix)[0][2] = Right->z;
   (result -> matrix)[1][0] = Up->x;
   (result -> matrix)[1][1] = Up->y;
   (result -> matrix)[1][2] = Up->z;
   (result -> matrix)[2][0] = Look_At->x;
   (result -> matrix)[2][1] = Look_At->y;
   (result -> matrix)[2][2] = Look_At->z;

   MIdentity ((MATRIX *)result -> matrix);
   MTranspose ((MATRIX *)result -> matrix, (MATRIX *)result -> inverse);   
   }

... up to here! */

void Compose_Transforms (Original_Transform, New_Transform)
TRANSFORM *Original_Transform, *New_Transform;
  {
  MTimes ((MATRIX *)Original_Transform -> matrix,
    (MATRIX *)Original_Transform -> matrix,
    (MATRIX *)New_Transform -> matrix);

  MTimes ((MATRIX *)Original_Transform -> inverse,
    (MATRIX *)New_Transform -> inverse,
    (MATRIX *)Original_Transform -> inverse);
  }

/* Rotation about an arbitrary axis - formula from:
      "Computational Geometry for Design and Manufacture",
      Faux & Pratt
   Note that the angles for this transform are specified in radians.
*/
void Compute_Axis_Rotation_Transform (transform, V, angle)
TRANSFORM *transform;
VECTOR *V;
DBL angle;
  {
  DBL l, cosx, sinx;

  VLength(l, *V);
  VInverseScaleEq(*V, l);

  MIdentity(&transform->matrix);
  cosx = cos(angle);
  sinx = sin(angle);
  transform->matrix[0][0] = V->x * V->x + cosx * (1.0 - V->x * V->x);
  transform->matrix[0][1] = V->x * V->y * (1.0 - cosx) + V->z * sinx;
  transform->matrix[0][2] = V->x * V->z * (1.0 - cosx) - V->y * sinx;
  transform->matrix[1][0] = V->x * V->y * (1.0 - cosx) - V->z * sinx;
  transform->matrix[1][1] = V->y * V->y + cosx * (1.0 - V->y * V->y);
  transform->matrix[1][2] = V->y * V->z * (1.0 - cosx) + V->x * sinx;
  transform->matrix[2][0] = V->x * V->z * (1.0 - cosx) + V->y * sinx;
  transform->matrix[2][1] = V->y * V->z * (1.0 - cosx) - V->x * sinx;
  transform->matrix[2][2] = V->z * V->z + cosx * (1.0 - V->z * V->z);
  MTranspose(&transform->inverse, &transform->matrix);   
  }

/* Given a point and a direction and a radius, find the transform
   that brings these into a canonical coordinate system */
void
Compute_Coordinate_Transform(trans, origin, up, radius, length)
TRANSFORM *trans;
VECTOR *origin;
VECTOR *up;
DBL radius;
DBL length;
  {
  TRANSFORM trans2;
  VECTOR tmpv;

  Make_Vector(&tmpv, radius, radius, length);
  Compute_Scaling_Transform(trans, &tmpv);
  if (fabs(up->z) == 1.0)
    Make_Vector(&tmpv, 1.0, 0.0, 0.0)
else
  Make_Vector(&tmpv, -up->y, up->x, 0.0)
    Compute_Axis_Rotation_Transform(&trans2, &tmpv, acos(up->z));
Compose_Transforms(trans, &trans2);
Compute_Translation_Transform(&trans2, origin);
Compose_Transforms(trans, &trans2);
}

TRANSFORM *Create_Transform()
{
TRANSFORM *New;

if ((New = (TRANSFORM *) malloc (sizeof (TRANSFORM))) == NULL)
MAError ("transform");

MIdentity ((MATRIX *) &(New -> matrix[0][0]));
MIdentity ((MATRIX *) &(New -> inverse[0][0]));
return (New);
}

TRANSFORM *Copy_Transform (Old)
TRANSFORM *Old;
{
TRANSFORM *New;
if (Old != NULL)
  {
  New  = Create_Transform ();
  *New = *Old;
  }
else New = NULL;
return (New);
}

VECTOR *Create_Vector ()
{
VECTOR *New;

if ((New = (VECTOR *) malloc (sizeof (VECTOR))) == NULL)
MAError ("vector");

Make_Vector (New, 0.0, 0.0, 0.0);

return (New);
}

VECTOR *Copy_Vector (Old)
VECTOR *Old;
{
VECTOR *New;
if (Old != NULL)
  {
  New  = Create_Vector ();
  *New = *Old;
  }
else New = NULL;
return (New);
}

DBL *Create_Float ()
{
DBL *New_Float;

if ((New_Float = (DBL *) malloc (sizeof (DBL))) == NULL)
MAError ("float");

*New_Float = 0.0;
return (New_Float);
}

DBL *Copy_Float (Old)
DBL *Old;
{
DBL *New;
if (Old)
  {
  New  = Create_Float ();
  *New = *Old;
  }
else New = NULL;
return (New);
}

