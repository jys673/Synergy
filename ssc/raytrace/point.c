/****************************************************************************
*                point.c
*
*  This module implements the point & spot light source primitive.
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

METHODS Light_Source_Methods =
  { 
  All_Light_Source_Intersections,
  Inside_Light_Source, Light_Source_Normal,
  Copy_Light_Source,
  Translate_Light_Source, Rotate_Light_Source,
  Scale_Light_Source, Transform_Light_Source, Invert_Light_Source,
  Destroy_Light_Source
};

static DBL cubic_spline PARAMS(( DBL low,DBL high,DBL pos));

int All_Light_Source_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  if (((LIGHT_SOURCE *)Object)->Children != NULL)
    if (Ray_In_Bounds (Ray, ((LIGHT_SOURCE *)Object)->Children->Bound))
      if (All_Intersections (((LIGHT_SOURCE *)Object)->Children, Ray, Depth_Stack))
        return (TRUE);

  return (FALSE);
  }

int Inside_Light_Source (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  if (((LIGHT_SOURCE *)Object)->Children != NULL)
    if (Inside_Object (IPoint, ((LIGHT_SOURCE *)Object)->Children))
      return (TRUE);

  return (FALSE);
  }

void Light_Source_Normal (Result, Object, IPoint)
OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  if (((LIGHT_SOURCE *)Object)->Children != NULL)
    Normal (Result, ((LIGHT_SOURCE *)Object)->Children,IPoint);
  }

void Translate_Light_Source (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  VAddEq (((LIGHT_SOURCE *) Object)->Center, *Vector);
  VAddEq (((LIGHT_SOURCE *) Object)->Points_At, *Vector);

  Translate_Object (((LIGHT_SOURCE *)Object)->Children, Vector);
  }

void Rotate_Light_Source (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Rotation_Transform (&Trans, Vector);
  Transform_Light_Source(Object, &Trans);
  }

void Scale_Light_Source (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Scaling_Transform (&Trans, Vector);
  Transform_Light_Source(Object, &Trans);
  }

void Invert_Light_Source (Object)
OBJECT *Object;
  {
  Invert_Object (((LIGHT_SOURCE *)Object)->Children);
  }

void Transform_Light_Source (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  MTransPoint (&((LIGHT_SOURCE *) Object)->Center,
    &((LIGHT_SOURCE *) Object)->Center, Trans);
  MTransPoint (&((LIGHT_SOURCE *) Object)->Points_At,
    &((LIGHT_SOURCE *) Object)->Points_At, Trans);
  MTransPoint (&((LIGHT_SOURCE *) Object)->Axis1,
    &((LIGHT_SOURCE *) Object)->Axis1, Trans);
  MTransPoint (&((LIGHT_SOURCE *) Object)->Axis2,
    &((LIGHT_SOURCE *) Object)->Axis2, Trans);
  Transform_Object (((LIGHT_SOURCE *)Object)->Children, Trans);
  }

void Destroy_Light_Source (Object)
OBJECT *Object;
  {
  int i;

  if (((LIGHT_SOURCE *)Object)->Light_Grid != NULL) 
    { 
    for (i = 0; i < ((LIGHT_SOURCE *)Object)->Area_Size1; i++)
      free(((LIGHT_SOURCE *)Object)->Light_Grid[i]);

    free(((LIGHT_SOURCE *)Object)->Light_Grid);
    } 

  Destroy_Object (((LIGHT_SOURCE *)Object)->Children);
  free (Object);
  }

COLOUR **Create_Light_Grid (Size1, Size2)
int Size1, Size2;
  {
  COLOUR **New;
  int i;

  New = (COLOUR **)malloc (Size1 * sizeof (COLOUR *));
  if (New == NULL)
    MAError ("area light");

  for (i = 0; i < Size1; i++) 
    {
    New[i] = (COLOUR *)malloc (Size2 * sizeof (COLOUR));
    if (New[i] == NULL)
      MAError ("area light");
    }

  return (New);
  }

LIGHT_SOURCE *Create_Light_Source ()
  {
  LIGHT_SOURCE *New;

  if ((New = (LIGHT_SOURCE *) malloc (sizeof (LIGHT_SOURCE))) == NULL)
    MAError ("light_source");

  INIT_OBJECT_FIELDS(New, LIGHT_OBJECT, &Light_Source_Methods)
    New->Children = NULL;
  New->No_Shadow_Flag = TRUE;

  Make_Colour(&New->Colour,1.0,1.0,1.0);
  Make_Vector(&New->Center,0.0,0.0,0.0);
  Make_Vector(&New->Points_At,0.0,0.0,1.0);
  Make_Vector(&New->Axis1,0.0,0.0,1.0);
  Make_Vector(&New->Axis2,0.0,1.0,0.0);
  New->Coeff   = 10.0;
  New->Radius  = 0.35;
  New->Falloff = 0.35;
  New->Next_Light_Source    = NULL;
  New->Shadow_Cached_Object = NULL;
  New->Light_Grid           = NULL;
  New->Light_Type = POINT_SOURCE;
  New->Area_Light = FALSE;
  New->Jitter     = FALSE;
  New->Track      = FALSE;
  New->Area_Size1 = 0;
  New->Area_Size2 = 0;
  New->Adaptive_Level = 100;
  return (New);
  }

void *Copy_Light_Source (Old)
OBJECT *Old;
  {
  LIGHT_SOURCE *New;
  int i, j;

  New = Create_Light_Source ();
  *New = *(LIGHT_SOURCE *)Old;

  New->Next_Light_Source = NULL;

  New->Children = Copy_Object (((LIGHT_SOURCE *)Old)->Children);

  if (((LIGHT_SOURCE *)Old)->Light_Grid != NULL) 
    { 
    New->Light_Grid = Create_Light_Grid (((LIGHT_SOURCE *)Old)->Area_Size1,
      ((LIGHT_SOURCE *)Old)->Area_Size2);

    for (i = 0; i < ((LIGHT_SOURCE *)Old)->Area_Size1; i++)
      for (j = 0; j < ((LIGHT_SOURCE *)Old)->Area_Size2; j++)
      New->Light_Grid[i][j] = ((LIGHT_SOURCE *)Old)->Light_Grid[i][j];
    } 

  return (New);
  }

/* Cubic spline that has tangents of slope 0 at x == low and at x == high.
   For a given value "pos" between low and high the spline value is returned */
static DBL cubic_spline(low, high, pos)
DBL low, high, pos;
  {
  /* Check to see if the position is within the proper boundaries */
  if (pos < low)
    return 0.0;
  else if (pos > high)
    return 1.0;
  if (high == low)
    return 0.0;

  /* Normalize to the interval 0->1 */
  pos = (pos - low) / (high - low);

  /* See where it is on the cubic curve */
  return (3 - 2 * pos) * pos * pos;
  }

DBL Attenuate_Light (Light_Source, Light_Source_Ray)
LIGHT_SOURCE *Light_Source;
RAY *Light_Source_Ray;
  {
  DBL Len,costheta;
  DBL Attenuation = 1.0;
  VECTOR Spot_Direction;

  /* If this is a spotlight then attenuate based on the incidence angle */
  if (Light_Source->Light_Type == SPOT_SOURCE) 
    {
    VSub(Spot_Direction, Light_Source->Points_At, Light_Source->Center);
    VLength(Len, Spot_Direction);
    if (Len > 0.0) 
      {
      VInverseScale(Spot_Direction, Spot_Direction, Len);
      VDot(costheta, Light_Source_Ray->Direction, Spot_Direction);
      costheta *= -1.0;
      if (costheta > 0.0) 
        {
        Attenuation = pow(costheta, Light_Source->Coeff);
        /* If there is a soft falloff region associated with the light then
               do an interpolation of values between the hot center and the
               direction at which light falls to nothing. */
        if (Light_Source->Radius > 0.0)
          Attenuation *= cubic_spline(Light_Source->Falloff,
            Light_Source->Radius,
            costheta);
        /* printf("Atten: %lg\n", Attenuation); */
        }
      else
        Attenuation = 0.0;
      }
    else
      Attenuation = 0.0;
    } 
  return(Attenuation);
  }    
