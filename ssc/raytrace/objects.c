/****************************************************************************
*                objects.c
*
*  This module implements the methods for objects and composite objects.
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

extern RAY *VP_Ray;
extern long Bounding_Region_Tests, Bounding_Region_Tests_Succeeded;
extern long Clipping_Region_Tests, Clipping_Region_Tests_Succeeded;
extern unsigned int Options;
extern long Istack_overflows;
extern int Number_of_istacks;
extern int Max_Intersections;
extern ISTACK *free_istack;

int Intersection (Ray_Intersection, Object, Ray)
INTERSECTION *Ray_Intersection;
OBJECT *Object;
RAY *Ray;
  {
  ISTACK *Depth_Stack;
  INTERSECTION *Local;
  DBL Closest = HUGE_VAL;

  if (Object == NULL)
    return (FALSE);

  if(!Ray_In_Bounds (Ray,Object->Bound))
    return (FALSE);

  Depth_Stack = open_istack ();

  if (All_Intersections (Object, Ray, Depth_Stack))
    {
    while ((Local = pop_entry(Depth_Stack)) != NULL)
      if (Local->Depth < Closest)
        {
        *Ray_Intersection = *Local;
        Closest = Local->Depth;
        }
    close_istack (Depth_Stack);
    return (TRUE);
    }
  else
    {
    close_istack (Depth_Stack);
    return (FALSE);
    }
  }

int Ray_In_Bounds (Ray,Bounds)
RAY *Ray;
OBJECT *Bounds;
  {
  OBJECT *Bound;
  INTERSECTION Local;

  for (Bound = Bounds;
  Bound != NULL;
  Bound = Bound->Sibling)
    {
    Bounding_Region_Tests++;

    if (!Intersection (&Local, Bound, Ray))
      if (!Inside_Object(&Ray->Initial,Bound))
        return (FALSE);

    Bounding_Region_Tests_Succeeded++;
    }

  return (TRUE);
  }

int Point_In_Clip (IPoint, Clip)
VECTOR *IPoint;
OBJECT *Clip;
  {
  OBJECT *Local_Clip;

  for (Local_Clip = Clip;
  Local_Clip != NULL;
  Local_Clip = Local_Clip->Sibling)
    {
    Clipping_Region_Tests++;
    if (!Inside_Object(IPoint, Local_Clip)) 
      return (FALSE);

    Clipping_Region_Tests_Succeeded++;
    }
  return (TRUE);
  }

OBJECT *Copy_Bound_Clip (Old)
OBJECT *Old;
  {
  OBJECT *Current, *New, *Prev, *First;

  First = Prev = NULL;

  for (Current = Old;
  Current != NULL ;
  Current = Current->Sibling) 
    {
    New = Copy_Object (Current);
    if (First == NULL)
      First = New;
    if (Prev != NULL)
      Prev->Sibling = New;
    Prev = New;
    }
  return (First);
  }

OBJECT *Copy_Object (Old)
OBJECT *Old;
  {
  OBJECT *New;

  if (Old == NULL)
    return (NULL);

  New = Copy (Old);

  New->Methods = Old->Methods;
  New->Type    = Old->Type;
  New->Sibling = Old->Sibling;
  New->Texture = Old->Texture;
  New->Bound   = Old->Bound;
  New->Clip    = Old->Clip;
  New->Bounds  = Old->Bounds;
  New->No_Shadow_Flag = Old->No_Shadow_Flag;

  New->Sibling = NULL;          /* Important */

  New->Texture = Copy_Textures (Old->Texture);
  New->Bound   = Copy_Bound_Clip (Old->Bound);
  if (Old->Bound != Old->Clip)
    New->Clip  = Copy_Bound_Clip (Old->Clip);
  else
    New->Clip  = New->Bound;

  return (New);
  }   

void Translate_Object (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  OBJECT *Sib;

  if (Object == NULL)
    return;

  for (Sib = Object->Bound;
  Sib != NULL;
  Sib = Sib->Sibling)
    Translate_Object (Sib, Vector);

  if (Object->Clip != Object->Bound)
    for (Sib = Object->Clip;
  Sib != NULL;
  Sib = Sib->Sibling)
    Translate_Object (Sib, Vector);

  Translate_Textures (Object->Texture,Vector);

  Translate (Object,Vector);
  }

void Rotate_Object (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  OBJECT *Sib;

  if (Object == NULL)
    return;

  for (Sib = Object->Bound;
  Sib != NULL;
  Sib = Sib->Sibling)
    Rotate_Object (Sib, Vector);

  if (Object->Clip != Object->Bound)
    for (Sib = Object->Clip;
  Sib != NULL;
  Sib = Sib->Sibling)
    Rotate_Object (Sib, Vector);

  Rotate_Textures (Object->Texture,Vector);

  Rotate (Object,Vector);
  }

void Scale_Object (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  OBJECT *Sib;

  if (Object == NULL)
    return;

  for (Sib = Object->Bound;
  Sib != NULL;
  Sib = Sib->Sibling)
    Scale_Object (Sib, Vector);

  if (Object->Clip != Object->Bound)
    for (Sib = Object->Clip;
  Sib != NULL;
  Sib = Sib->Sibling)
    Scale_Object (Sib, Vector);

  Scale_Textures (Object->Texture,Vector);

  Scale (Object,Vector);
  }

int Inside_Object (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  OBJECT *Sib;

  /* removed 7/19/92 CEY   
   for (Sib = Object->Bound;
        Sib != NULL;
        Sib = Sib->Sibling)
     if (!Inside_Object(IPoint, Sib))
       return(FALSE);
*/
  for (Sib = Object->Clip;
  Sib != NULL;
  Sib = Sib->Sibling)
    if (!Inside_Object(IPoint, Sib))
      return(FALSE);

  return (Inside(IPoint,Object));
  }

void Invert_Object (Object)
OBJECT *Object;
  {
  /*   OBJECT *Sib; */

  if (Object == NULL)
    return;

  /* removed 3/29/93 CEY
   for (Sib = Object->Clip;
        Sib != NULL;
        Sib = Sib->Sibling)
     Invert_Object (Sib);
*/
  Invert (Object);
  }

void Destroy_Object (Object)
OBJECT *Object;
  {
  OBJECT *Sib;

  while (Object != NULL)
    {
    Destroy_Textures (Object->Texture);
    Destroy_Object (Object->Bound);
    if (Object->Bound != Object->Clip)
      Destroy_Object (Object->Clip);
    Sib = Object->Sibling;
    Destroy(Object);
    Object = Sib;
    }
  }   

void Transform_Object (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  OBJECT *Sib;

  if (Object == NULL)
    return;

  for (Sib = Object->Bound;
  Sib != NULL;
  Sib = Sib->Sibling)
    Transform_Object (Sib, Trans);

  if (Object->Clip != Object->Bound)
    for (Sib = Object->Clip;
  Sib != NULL;
  Sib = Sib->Sibling)
    Transform_Object (Sib, Trans);

  Transform_Textures (Object->Texture,Trans);

  Transform (Object,Trans);
  }

void create_istack ()
  {
  ISTACK *New;

  if ((New = (ISTACK *) malloc (sizeof (ISTACK))) == NULL) 
    {
    fprintf (stderr, "\nOut of memory. Cannot allocate istack");
    close_all();
    exit(1);
    }

  New->next = free_istack;
  free_istack = New;

  if ((New->istack = (INTERSECTION *)
    malloc (Max_Intersections * sizeof (INTERSECTION))) == NULL) 
    {
    fprintf (stderr, "\nOut of memory. Cannot allocate istack entries");
    close_all();
    exit(1);
    }
  Number_of_istacks++;
  }

ISTACK *open_istack()
  {
  ISTACK *istk;

  if (free_istack == NULL) 
    create_istack ();

  istk = free_istack;
  free_istack = istk->next;
  istk->top_entry = 0;

  return (istk);
  }

void close_istack (istk)
ISTACK *istk;
  {
  istk->next = free_istack;
  free_istack = istk;
  }

void incstack(istk)
ISTACK *istk;
  {
  if (++istk->top_entry >= (unsigned int)Max_Intersections)
    {
    istk->top_entry--;
    Istack_overflows++;
    }
  }
