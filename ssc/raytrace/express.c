/****************************************************************************
*                express.c
*
*  This module implements an expression parser for the floats, vectors and
*  colours in scene description files.
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
#include "parse.h"

/* This file implements a simple recursive-descent parser for reading the
input file.  */

extern DBL Max_Trace_Level;
extern char VerboseFormat;
extern unsigned int Options;
extern char Stat_File_Name[FILE_NAME_LENGTH];

extern struct Reserved_Word_Struct Reserved_Words [];
extern DBL Antialias_Threshold;

extern struct Token_Struct Token;
extern char String[MAX_STRING_INDEX];

extern COLOUR_MAP_ENTRY *Build_Entries;
extern FRAME Frame;
extern DBL Clock_Value;
extern char **Symbol_Table;
extern int Max_Intersections;
extern DBL Language_Version;
extern METHODS Csg_Height_Field_Methods;

static DBL Parse_Float_Factor PARAMS((void));
static DBL Parse_Float_Term PARAMS((void));
static void Parse_Vector_Factor PARAMS((VECTOR *Vector));
static void Parse_Vector_Term PARAMS((VECTOR *Vector));

extern struct Constant_Struct Constants[MAX_CONSTANTS];

extern int Number_Of_Constants;
extern int Previous;
extern short Have_Vector;
extern short Not_In_Default;

extern TOKEN *Brace_Stack;
extern int Brace_Index;

static DBL Parse_Float_Factor ()
  {
   DBL Local_Float;

   EXPECT
     CASE (FLOAT_TOKEN)
       Local_Float = Token.Token_Float;
       EXIT
     END_CASE

     CASE (FLOAT_ID_TOKEN)
       Local_Float = *((DBL *) Token.Constant_Data);
       EXIT
     END_CASE

     CASE (CLOCK_TOKEN)
       Local_Float = Clock_Value;
       EXIT
     END_CASE

     CASE (PLUS_TOKEN)
     END_CASE

     CASE (DASH_TOKEN)
       Local_Float = - Parse_Float_Factor();
       EXIT
     END_CASE

     CASE (LEFT_PAREN_TOKEN)
       Local_Float = Parse_Float();
       GET(RIGHT_PAREN_TOKEN);
       EXIT
     END_CASE

     CASE (VERSION_TOKEN)
       Local_Float = Language_Version;
       EXIT
     END_CASE

     OTHERWISE
       Parse_Error_Str ("float factor");
     END_CASE
   END_EXPECT

   return (Local_Float);
  }

static DBL Parse_Float_Term ()
  {
   DBL Local_Float;

   Local_Float = Parse_Float_Factor();

   EXPECT
     CASE (STAR_TOKEN)
       Local_Float *= Parse_Float_Factor();
     END_CASE

     CASE (SLASH_TOKEN)
       Local_Float /= Parse_Float_Factor();
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   return (Local_Float);
  }

DBL Parse_Float ()
  {
   DBL Local_Float;

   if (Language_Version < 1.5)
     return(Parse_Float_Factor());

   Local_Float = Parse_Float_Term();

   EXPECT
     CASE (PLUS_TOKEN)
       Local_Float += Parse_Float_Term();
     END_CASE

     CASE (DASH_TOKEN)
       Local_Float -= Parse_Float_Term();
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   return (Local_Float);
  }

static void Parse_Vector_Factor (Vector)
  VECTOR *Vector;
  {

   EXPECT
     CASE (LEFT_ANGLE_TOKEN)
       Have_Vector = TRUE;
       Vector -> x = Parse_Float();   Parse_Comma();
       Vector -> y = Parse_Float();   Parse_Comma();
       Vector -> z = Parse_Float();
       GET (RIGHT_ANGLE_TOKEN);
       EXIT
     END_CASE

     CASE (VECTOR_ID_TOKEN)
       Have_Vector = TRUE;
       *Vector = *((VECTOR *) Token.Constant_Data);
       EXIT
     END_CASE

     CASE (X_TOKEN)
       Have_Vector = TRUE;
       Make_Vector(Vector,1.0,0.0,0.0)
       EXIT
     END_CASE

     CASE (Y_TOKEN)
       Have_Vector = TRUE;
       Make_Vector(Vector,0.0,1.0,0.0)
       EXIT
     END_CASE

     CASE (Z_TOKEN)
       Have_Vector = TRUE;
       Make_Vector(Vector,0.0,0.0,1.0)
       EXIT
     END_CASE

     CASE (PLUS_TOKEN)              /* uniary plus */
     END_CASE

     CASE (DASH_TOKEN)              /* uniary minus */
       Parse_Vector_Factor(Vector);
       Vector -> x *= -1.0;
       Vector -> y *= -1.0;
       Vector -> z *= -1.0;
       EXIT
     END_CASE

     CASE (LEFT_PAREN_TOKEN)
       Parse_Vector_Float(Vector);
       GET(RIGHT_PAREN_TOKEN);
       EXIT
     END_CASE

     CASE4 (FLOAT_TOKEN, FLOAT_ID_TOKEN, CLOCK_TOKEN, VERSION_TOKEN)
       UNGET
       (Vector->x) =
       (Vector->y) =
       (Vector->z) = Parse_Float_Factor ();
       EXIT
     END_CASE

     OTHERWISE
       Parse_Error_Str ("vector factor");
     END_CASE
   END_EXPECT

  }

static void Parse_Vector_Term (Vector)
  VECTOR *Vector;
  {
   VECTOR Local_Vector;

   Parse_Vector_Factor(Vector);

   EXPECT
     CASE (STAR_TOKEN)
       Parse_Vector_Factor(&Local_Vector);
       Vector->x *= Local_Vector.x;
       Vector->y *= Local_Vector.y;
       Vector->z *= Local_Vector.z;
     END_CASE

     CASE (SLASH_TOKEN)
       Parse_Vector_Factor(&Local_Vector);
       Vector->x /= Local_Vector.x;
       Vector->y /= Local_Vector.y;
       Vector->z /= Local_Vector.z;
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

  }

void Parse_Vector_Float (Vector)
  VECTOR *Vector;
  {
   VECTOR Local_Vector;

   Parse_Vector_Term(Vector);

   EXPECT
     CASE (PLUS_TOKEN)
       Parse_Vector_Term(&Local_Vector);
       Vector->x += Local_Vector.x;
       Vector->y += Local_Vector.y;
       Vector->z += Local_Vector.z;
     END_CASE

     CASE (DASH_TOKEN)
       Parse_Vector_Term(&Local_Vector);
       Vector->x -= Local_Vector.x;
       Vector->y -= Local_Vector.y;
       Vector->z -= Local_Vector.z;
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

  }

void Parse_Scale_Vector (Vector)
  VECTOR *Vector;
  {
   Parse_Vector_Float(Vector);

   if (Vector->x == 0.0)
    {
     Vector->x = 1.0;
     Warn("Illegal Value: Scale X by 0.0. Changed to 1.0\n",0.0);
    }
   if (Vector->y == 0.0)
    {
     Vector->y = 1.0;
     Warn("Illegal Value: Scale Y by 0.0. Changed to 1.0\n",0.0);
    }
   if (Vector->z == 0.0)
    {
     Vector->z = 1.0;
     Warn("Illegal Value: Scale Z by 0.0. Changed to 1.0\n",0.0);
    }
  }

void Parse_Vector (Vector)
  VECTOR *Vector;
  {
   Have_Vector = FALSE;

   if (Language_Version < 1.5)
     {
      Parse_Vector_Factor(Vector);
     }
   else
     {
      Parse_Vector_Float (Vector);
      if (!Have_Vector)
         Error ("Vector expected but float only expression found");
     }
  }

void Parse_Colour (Colour)
  COLOUR *Colour;
  {
   EXPECT
     CASE (COLOUR_ID_TOKEN)
       *Colour = *((COLOUR *) Token.Constant_Data);
       EXIT
     END_CASE

     CASE (RGB_TOKEN)
       GET (LEFT_ANGLE_TOKEN);
       (Colour -> Red)   = Parse_Float();    Parse_Comma();
       (Colour -> Green) = Parse_Float();    Parse_Comma();
       (Colour -> Blue)  = Parse_Float();
       (Colour -> Filter) = 0.0;
       GET (RIGHT_ANGLE_TOKEN);
       EXIT
     END_CASE

     CASE (RGBF_TOKEN)
       GET (LEFT_ANGLE_TOKEN);
       (Colour -> Red)   = Parse_Float();    Parse_Comma();
       (Colour -> Green) = Parse_Float();    Parse_Comma();
       (Colour -> Blue)  = Parse_Float();    Parse_Comma();
       (Colour -> Filter) = Parse_Float();
       GET (RIGHT_ANGLE_TOKEN);
       EXIT
     END_CASE

     OTHERWISE
       Make_Colour (Colour, 0.0, 0.0, 0.0);
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   EXPECT
     CASE (COLOUR_ID_TOKEN)
       *Colour = *((COLOUR *) Token.Constant_Data);
       Warn("Previous color overwritten.",1.5);
     END_CASE

     CASE (RED_TOKEN)
       (Colour -> Red) = Parse_Float();
     END_CASE

     CASE (GREEN_TOKEN)
       (Colour -> Green) = Parse_Float();
     END_CASE

     CASE (BLUE_TOKEN)
       (Colour -> Blue) = Parse_Float();
     END_CASE

     CASE (ALPHA_TOKEN)
       Warn("Keyword ALPHA discontinued.  Use FILTER instead.",1.55);
       
     CASE (FILTER_TOKEN)
       (Colour -> Filter) = Parse_Float();
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT
  }

COLOUR_MAP *Parse_Colour_Map ()
  {
   COLOUR_MAP *New;
   short Flag;
   int i,j,c,p;

   Parse_Begin ();

   EXPECT
     CASE (COLOUR_MAP_ID_TOKEN)
       New = Copy_Colour_Map ((COLOUR_MAP *) Token.Constant_Data);
       EXIT
     END_CASE

     OTHERWISE
       UNGET
       if (Build_Entries == NULL)
         Build_Entries = Create_CMap_Entries(MAX_COLOUR_MAP_ENTRIES);
       i = 0;
       j = 1;
       Flag = FALSE;

       EXPECT
         CASE (LEFT_SQUARE_TOKEN)
           Build_Entries[i].value = Parse_Float();  Parse_Comma();

           EXPECT
             CASE (COLOUR_TOKEN);
               Parse_Colour (&(Build_Entries[i].Colour));
               Flag |= (Build_Entries[i].Colour.Filter == 0.0);
               i++;
               j++;
               EXIT
             END_CASE
             
             OTHERWISE
               UNGET
               Build_Entries[j].value = Parse_Float();

               GET (COLOUR_TOKEN);
               Parse_Colour (&(Build_Entries[i].Colour));
               Flag |= (Build_Entries[i].Colour.Filter == 0.0);

               GET (COLOUR_TOKEN);
               Parse_Colour (&(Build_Entries[j].Colour));
               Flag |= (Build_Entries[j].Colour.Filter == 0.0);
               i += 2;
               j += 2;
               EXIT
             END_CASE
           END_EXPECT
             
           if (j > MAX_COLOUR_MAP_ENTRIES)
             Error ("Colour_Map too long");

           GET (RIGHT_SQUARE_TOKEN);
         END_CASE

         OTHERWISE
           UNGET
           if (i < 1)
             Error ("Must have at least one color in colour map");

           /* Eliminate duplicates */
           for (c = 1, p = 0; c<i; c++)
             {
              if (memcmp(&(Build_Entries[p]),
                         &(Build_Entries[c]),sizeof(COLOUR_MAP_ENTRY)) == 0)
                p--;

              Build_Entries[++p] = Build_Entries[c];
             }
           p++;
           New = Create_Colour_Map ();
           New->Number_Of_Entries = p;
           New->Colour_Map_Entries = Copy_CMap_Entries (Build_Entries,p);
           New->Transparency_Flag = Flag;
           EXIT
         END_CASE
       END_EXPECT
       EXIT
     END_CASE
   END_EXPECT

   Parse_End ();

   return (New);
  }

COLOUR_MAP *Parse_Colour_List (MinCount)
  int MinCount;
  {
   COLOUR_MAP *New;
   short Flag;
   int i;

   if (Build_Entries == NULL)
      Build_Entries = Create_CMap_Entries(MAX_COLOUR_MAP_ENTRIES);
   i = 0;
   Flag = FALSE;

   EXPECT
     CASE (COLOUR_TOKEN);
       Parse_Colour (&(Build_Entries[i].Colour));
       Build_Entries[i].value = (DBL)i;
       Flag |= (Build_Entries[i].Colour.Filter == 0.0);
       i++;
     END_CASE
             
     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT
             
   if (i > MAX_COLOUR_MAP_ENTRIES)
      Error ("Too many colors");
   
   /* the follow code assumes MinCount of 2 or 3 */
   
   if (i < MinCount)
     {
      if (MinCount == 3)
        {
         Make_Colour(&(Build_Entries[2].Colour),1.0,0.0,0.0);
         Build_Entries[2].value = 2.0;
        }
      if (i < 2)
        {
         Make_Colour(&(Build_Entries[1].Colour),0.0,1.0,0.0);
         Build_Entries[1].value = 1.0;
        }
      if (i == 0)
        {
         Make_Colour(&(Build_Entries[0].Colour),0.0,0.0,1.0);
         Build_Entries[0].value = 0.0;
        }
      i=MinCount;
     }

   if (i == 0)
      return (NULL);
   
   New = Create_Colour_Map ();
   New->Number_Of_Entries = i;
   New->Colour_Map_Entries = Copy_CMap_Entries (Build_Entries,i);
   New->Transparency_Flag = Flag;

   return (New);
  }

