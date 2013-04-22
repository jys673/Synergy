/****************************************************************************
*                   colour.c
*
*  This module implements routines to manipulate colours.
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

#define FABS(x) ((x) < 0.0 ? (0.0 - (x)) : (x))

COLOUR_MAP_ENTRY *Build_Entries;

COLOUR *Create_Colour ()
  {
  COLOUR *New;

  if ((New = (COLOUR *) malloc (sizeof (COLOUR))) == NULL)
    MAError ("color");

  Make_Colour (New, 0.0, 0.0, 0.0);
  return (New);
  }

COLOUR *Copy_Colour (Old)
COLOUR *Old;
  {
  COLOUR *New;
  if (Old != NULL)
    {
    New  = Create_Colour ();
    *New = *Old;
    }
  else New = NULL;
  return (New);
  }

  COLOUR_MAP_ENTRY *Create_CMap_Entries (Map_Size)
    int Map_Size;
  {
  COLOUR_MAP_ENTRY *New;

  if ((New = (COLOUR_MAP_ENTRY *)
    malloc(Map_Size * sizeof (COLOUR_MAP_ENTRY))) == NULL)
    MAError ("colour map entry");
  return (New);
  }

COLOUR_MAP_ENTRY *Copy_CMap_Entries (Old,Map_Size)
COLOUR_MAP_ENTRY *Old;
int Map_Size;
  {
  COLOUR_MAP_ENTRY *New;
  register int i;

  if (Old != NULL)
    {
    New = Create_CMap_Entries (Map_Size);
    for (i = 0; i < Map_Size; i++)
      New[i] = Old[i];
    }
  else
    New = NULL;
  return (New);
  }

COLOUR_MAP *Create_Colour_Map ()
  {
  COLOUR_MAP *New;

  if ((New = (COLOUR_MAP *) malloc (sizeof (COLOUR_MAP))) == NULL)
    MAError ("colour map");

  New->Users = 0;
  New->Number_Of_Entries = 0;
  New->Colour_Map_Entries = NULL;
  New->Transparency_Flag = FALSE;
  return (New);
  }   

COLOUR_MAP *Copy_Colour_Map (Old)
COLOUR_MAP *Old;
  {
  COLOUR_MAP *New;

  New = Old;
  if (New != NULL)
    New->Users++;

  return (New);
  }

DBL Colour_Distance (colour1, colour2)
COLOUR *colour1, *colour2;
  {
  return (FABS (colour1->Red - colour2->Red)
    + FABS (colour1->Green - colour2->Green)
    + FABS (colour1->Blue - colour2->Blue));
  }

void Add_Colour (result, colour1, colour2)
COLOUR *result, *colour1, *colour2;
  {
  result->Red = colour1->Red + colour2->Red;
  result->Green = colour1->Green + colour2->Green;
  result->Blue = colour1->Blue + colour2->Blue;
  result->Filter = colour1->Filter + colour2->Filter;
  }

void Scale_Colour (result, colour, factor)
COLOUR *result, *colour;
DBL factor;
  {
  result->Red = colour->Red * factor;
  result->Green = colour->Green * factor;
  result->Blue = colour->Blue * factor;
  result->Filter = colour->Filter * factor;
  }

void Clip_Colour (result, colour)
COLOUR *result, *colour;
  {
  if (colour -> Red > 1.0)
    result -> Red = 1.0;
  else if (colour -> Red < 0.0)
    result -> Red = 0.0;
  else result -> Red = colour -> Red;

    if (colour -> Green > 1.0)
      result -> Green = 1.0;
    else if (colour -> Green < 0.0)
      result -> Green = 0.0;
    else result -> Green = colour -> Green;

    if (colour -> Blue > 1.0)
      result -> Blue = 1.0;
    else if (colour -> Blue < 0.0)
      result -> Blue = 0.0;
    else result -> Blue = colour -> Blue;

    if (colour -> Filter > 1.0)
      result -> Filter = 1.0;
    else if (colour -> Filter < 0.0)
      result -> Filter = 0.0;
    else result -> Filter = colour -> Filter;
  }

  void Destroy_Colour_Map (CMap)
    COLOUR_MAP *CMap;
  {
  if (CMap == NULL)
    return;

  if (CMap->Users < 0)
    return;

  CMap->Users--;

  if (CMap->Users >= 0)
    return;

  free (CMap->Colour_Map_Entries);
  free (CMap);
  }
