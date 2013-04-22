/****************************************************************************
*                camera.c
*
*  This module implements methods for managing the viewpoint.
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

void Translate_Camera (Camera, Vector)
CAMERA *Camera;
VECTOR *Vector;
  {
  VAdd (((CAMERA *) Camera) -> Location, 
    ((CAMERA *) Camera) -> Location,
    *Vector);
  }

void Rotate_Camera (Camera, Vector)
CAMERA *Camera;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Rotation_Transform(&Trans, Vector);
  Transform_Camera (Camera, &Trans);
  }

void Scale_Camera (Camera, Vector)
CAMERA *Camera;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Scaling_Transform(&Trans, Vector);
  Transform_Camera (Camera, &Trans);
  }

void Transform_Camera (Camera, Trans)
CAMERA *Camera;
TRANSFORM *Trans;
  {
  MTransPoint (&(Camera -> Location), &(Camera -> Location), Trans);

  MTransPoint (&(Camera -> Direction), &(Camera -> Direction), Trans);

  MTransPoint (&(Camera -> Up), &(Camera -> Up), Trans);

  MTransPoint (&(Camera -> Right), &(Camera -> Right), Trans);
  }

CAMERA *Copy_Camera (Old)
CAMERA *Old;
  {
  CAMERA *New;

  if (Old != NULL)
    {
    New = Create_Camera ();
    *New = *Old;
    }
  else
    New = NULL;

  return (New);   
  }

CAMERA *Create_Camera ()
  {
  CAMERA *New;

  if ((New = (CAMERA *) malloc (sizeof (CAMERA))) == NULL)
    MAError ("camera");

  Make_Vector (&(New->Location), 0.0, 0.0, 0.0);
  Make_Vector (&(New->Direction), 0.0, 0.0, 1.0);
  Make_Vector (&(New->Up), 0.0, 1.0, 0.0);
  Make_Vector (&(New->Right), 1.33, 0.0, 0.0);
  Make_Vector (&(New->Sky), 0.0, 1.0, 0.0);
  return (New);
  }

