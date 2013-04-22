/****************************************************************************
*                   csg.c
*
*  This module implements routines for constructive solid geometry.
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

METHODS CSG_Union_Methods =
  { 
  All_CSG_Union_Intersections,
  Inside_CSG_Union, NULL /*Normal*/,
  Copy_CSG,
  Translate_CSG, Rotate_CSG,
  Scale_CSG, Transform_CSG, Invert_CSG_Union, Destroy_CSG
};

METHODS CSG_Merge_Methods =
  { 
  All_CSG_Merge_Intersections,
  Inside_CSG_Union, NULL /*Normal*/,
  Copy_CSG,
  Translate_CSG, Rotate_CSG,
  Scale_CSG, Transform_CSG, Invert_CSG_Union, Destroy_CSG
};

METHODS CSG_Intersection_Methods =
  { 
  All_CSG_Intersect_Intersections,
  Inside_CSG_Intersection, NULL /*Normal*/,
  Copy_CSG,
  Translate_CSG, Rotate_CSG,
  Scale_CSG, Transform_CSG, Invert_CSG_Intersection, Destroy_CSG
};

extern RAY *VP_Ray;

int All_CSG_Union_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  register int Found;
  OBJECT *Current_Sib, *Clip;
  ISTACK *Local_Stack;
  INTERSECTION *Sibling_Intersection;

  Found = FALSE;

  if ((Clip = Object->Clip) == NULL) /* Use shortcut if no clip */
    {
    for (Current_Sib = ((CSG *)Object)->Children;
         Current_Sib != NULL ;
         Current_Sib = Current_Sib->Sibling)
      if (Ray_In_Bounds (Ray, Current_Sib->Bound))
        if (All_Intersections (Current_Sib, Ray, Depth_Stack))
          Found = TRUE;
    }
  else
    {
    Local_Stack = open_istack ();

    for (Current_Sib = ((CSG *)Object)->Children;
         Current_Sib != NULL ;
         Current_Sib = Current_Sib->Sibling)
      if (Ray_In_Bounds (Ray, Current_Sib->Bound))
        if (All_Intersections (Current_Sib, Ray, Local_Stack))
          while ((Sibling_Intersection = pop_entry(Local_Stack)) != NULL)
            if (Point_In_Clip (&Sibling_Intersection->IPoint, Clip))
              {
              push_copy (Depth_Stack, Sibling_Intersection);
              Found = TRUE;
              }
    close_istack (Local_Stack);
    }
  return (Found);
  }

int All_CSG_Intersect_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  int Maybe_Found, Found;
  OBJECT *Current_Sib, *Inside_Sib;
  ISTACK *Local_Stack;
  INTERSECTION *Sibling_Intersection;

  Local_Stack = open_istack ();

  Found = FALSE;

  for (Current_Sib = ((CSG *)Object)->Children;
  Current_Sib != NULL;
  Current_Sib = Current_Sib->Sibling) 
    {
    if (Ray_In_Bounds (Ray, Current_Sib->Bound))
      if (All_Intersections (Current_Sib, Ray, Local_Stack))
        while ((Sibling_Intersection = pop_entry(Local_Stack)) != NULL)
          {
          Maybe_Found = TRUE;
          for (Inside_Sib = ((CSG *)Object)->Children;
          Inside_Sib != NULL;
          Inside_Sib = Inside_Sib->Sibling)
            if (Inside_Sib != Current_Sib)
              if (!Inside_Object (&Sibling_Intersection->IPoint, Inside_Sib)) 
                {
                Maybe_Found = FALSE;
                break;
                }
          if (Maybe_Found)
            if (Point_In_Clip (&Sibling_Intersection->IPoint, Object->Clip))
              {
              push_copy(Depth_Stack, Sibling_Intersection);
              Found = TRUE;
              }
          }
    }
  close_istack (Local_Stack);
  return (Found);
  }

int All_CSG_Merge_Intersections (Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  register int Found, inside_flag;
  OBJECT *Sib1, *Sib2;
  ISTACK *Local_Stack;
  INTERSECTION *Sibling_Intersection;

  Found = FALSE;

  Local_Stack = open_istack ();

  for (Sib1 = ((CSG *)Object)->Children;
       Sib1 != NULL ;
       Sib1 = Sib1->Sibling)
    if (Ray_In_Bounds (Ray, Sib1->Bound))
      if (All_Intersections (Sib1, Ray, Local_Stack))
        while ((Sibling_Intersection = pop_entry (Local_Stack)) !=  NULL)
          if (Point_In_Clip (&Sibling_Intersection->IPoint,Object->Clip)) 
          {
          inside_flag = TRUE;
          for (Sib2 = ((CSG *)Object)->Children;
               Sib2 != NULL && inside_flag == TRUE;
               Sib2 = Sib2->Sibling)
            if (Sib1 != Sib2)
              if (Inside_Object(&Sibling_Intersection->IPoint,Sib2))
                inside_flag = FALSE;
          if (inside_flag == TRUE) 
            {
            Found = TRUE;
            push_copy (Depth_Stack, Sibling_Intersection);
            }
          }
  close_istack (Local_Stack);
  return (Found);
  }

int Inside_CSG_Union (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  OBJECT *Current_Sib;

  for (Current_Sib = ((CSG *)Object)->Children;
  Current_Sib != NULL ;
  Current_Sib = Current_Sib->Sibling)
    if (Inside_Object (IPoint, Current_Sib))
      return (TRUE);

  return (FALSE);
  }

int Inside_CSG_Intersection (IPoint, Object)
OBJECT *Object;
VECTOR *IPoint;
  {
  OBJECT *Current_Sib;

  for (Current_Sib = ((CSG *)Object)->Children;
  Current_Sib != NULL ;
  Current_Sib = Current_Sib->Sibling)
    if (!Inside_Object (IPoint, Current_Sib))
      return (FALSE);

  return (TRUE);
  }

void *Copy_CSG (Object)
OBJECT *Object;
  {
  CSG *New;
  OBJECT *Old_Sib, *New_Sib, *Prev_Sib;

  if ((New = (CSG *) malloc (sizeof (CSG))) == NULL)
    MAError ("CSG");

  New->Children = Prev_Sib = NULL;

  for (Old_Sib = ((CSG *)Object)->Children;
  Old_Sib != NULL ;
  Old_Sib = Old_Sib->Sibling) 
    {
    New_Sib = Copy_Object (Old_Sib);

    if (New->Children == NULL)
      New->Children = New_Sib;

    if (Prev_Sib != NULL)
      Prev_Sib->Sibling = New_Sib;

    Prev_Sib = New_Sib;
    }

  return (New);
  }

void Translate_CSG (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  OBJECT *Sib;

  for (Sib = ((CSG *) Object)->Children;
  Sib != NULL ;
  Sib = Sib->Sibling)
    Translate_Object (Sib, Vector);   
  Compute_CSG_Bounds(Object);
  }

void Rotate_CSG (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  OBJECT *Sib;

  for (Sib = ((CSG *) Object)->Children;
  Sib != NULL ;
  Sib = Sib->Sibling)
    Rotate_Object (Sib, Vector);   
  Compute_CSG_Bounds(Object);
  }

void Scale_CSG (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  OBJECT *Sib;

  for (Sib = ((CSG *) Object)->Children;
  Sib != NULL ;
  Sib = Sib->Sibling)
    Scale_Object (Sib, Vector);   
  Compute_CSG_Bounds(Object);
  }

void Transform_CSG (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  OBJECT *Sib;

  for (Sib = ((CSG *) Object)->Children;
  Sib != NULL ;
  Sib = Sib->Sibling)
    Transform_Object (Sib, Trans);   
  Compute_CSG_Bounds(Object);
  }

void Destroy_CSG (Object)
OBJECT *Object;
  {
  Destroy_Object (((CSG *) Object)->Children);
  free (Object);
  }

void Invert_CSG_Union (Object)
OBJECT *Object;
  {
  OBJECT *Sib;

  Object->Methods = &CSG_Intersection_Methods;

  for (Sib = ((CSG *)Object)->Children;
  Sib != NULL ;
  Sib = Sib->Sibling)
    Invert_Object (Sib);
  }

void Invert_CSG_Intersection (Object)
OBJECT *Object;
  {
  OBJECT *Sib;

  Object->Methods = &CSG_Union_Methods;

  for (Sib = ((CSG *)Object)->Children;
  Sib != NULL ;
  Sib = Sib->Sibling)
    Invert_Object (Sib);   
  }

CSG *Create_CSG_Union ()
  {
  CSG *New;

  if ((New = (CSG *) malloc (sizeof (CSG))) == NULL)
    MAError ("union");

  INIT_OBJECT_FIELDS(New, UNION_OBJECT, &CSG_Union_Methods)

    New->Children = NULL;
  return (New);
  }

CSG *Create_CSG_Merge ()
  {
  CSG *New;

  if ((New = (CSG *) malloc (sizeof (CSG))) == NULL)
    MAError ("merge");

  INIT_OBJECT_FIELDS(New, MERGE_OBJECT, &CSG_Merge_Methods)

    New->Children = NULL;
  return (New);
  }

CSG *Create_CSG_Intersection ()
  {
  CSG *New;

  if ((New = (CSG *) malloc (sizeof (CSG))) == NULL)
    MAError ("intersection");

  INIT_OBJECT_FIELDS(New, INTERSECTION_OBJECT, &CSG_Intersection_Methods)

    New->Children = NULL;
  return (New);
  }

void Compute_CSG_Bounds (Object)
OBJECT *Object;
  {
  VECTOR mins, maxs;
  DBL tmin, tmax;
  OBJECT *Sib;

  Make_Vector(&mins,  BOUND_HUGE,  BOUND_HUGE,  BOUND_HUGE);
  Make_Vector(&maxs, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE);

  for (Sib = ((CSG *) Object)->Children;
  Sib != NULL ;
  Sib = Sib->Sibling) 
    {
    tmin = Sib->Bounds.Lower_Left.x;
    tmax = Sib->Bounds.Lower_Left.x + Sib->Bounds.Lengths.x;
    if (tmin < mins.x) mins.x = tmin;
    if (tmax > maxs.x) maxs.x = tmax;
    tmin = Sib->Bounds.Lower_Left.y;
    tmax = Sib->Bounds.Lower_Left.y + Sib->Bounds.Lengths.y;
    if (tmin < mins.y) mins.y = tmin;
    if (tmax > maxs.y) maxs.y = tmax;
    tmin = Sib->Bounds.Lower_Left.z;
    tmax = Sib->Bounds.Lower_Left.z + Sib->Bounds.Lengths.z;
    if (tmin < mins.z) mins.z = tmin;
    if (tmax > maxs.z) maxs.z = tmax;
    }
  Object->Bounds.Lower_Left = mins;
  VSub(Object->Bounds.Lengths, maxs, mins);
  }

