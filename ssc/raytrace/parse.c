/****************************************************************************
*                parse.c
*
*  This module implements a parser for the scene description files.
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
extern int Use_Slabs;
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
extern METHODS CSG_Union_Methods;

static void Parse_Image_Map PARAMS((PIGMENT *Pigment));
static void Parse_Bump_Map PARAMS((TNORMAL *Tnormal));
static void Parse_Pigment PARAMS((PIGMENT **Pigment_Ptr));
static void Parse_Tnormal PARAMS((TNORMAL **Tnormal_Ptr));
static void Parse_Finish PARAMS((FINISH **Finish_Ptr));
static TEXTURE *Parse_Texture PARAMS((void));
static void Token_Init PARAMS((void));
static void Frame_Init PARAMS((void));
static void Parse_Coeffs PARAMS((int order, DBL *Coeffs));
static IMAGE *Parse_Image PARAMS((int Legal));
static TRANSFORM *Parse_Transform PARAMS((void));
static void Parse_Object_Mods PARAMS((OBJECT *Object));
static OBJECT *Parse_Bicubic_Patch PARAMS((void));
static OBJECT *Parse_Blob PARAMS((void));
static OBJECT *Parse_Box PARAMS((void));
static OBJECT *Parse_Cone PARAMS((void));
static OBJECT *Parse_Cylinder PARAMS((void));
static OBJECT *Parse_Disc PARAMS((void));
static OBJECT *Parse_Height_Field PARAMS((void));
static OBJECT *Parse_Plane PARAMS((void));
static OBJECT *Parse_Poly PARAMS((int order));
static OBJECT *Parse_Quadric PARAMS((void));
static OBJECT *Parse_Smooth_Triangle PARAMS((void));
static OBJECT *Parse_Sphere PARAMS((void));
static OBJECT *Parse_Torus PARAMS((void));
static OBJECT *Parse_Triangle PARAMS((void));
static OBJECT *Parse_CSG PARAMS((int CSG_Type));
static OBJECT *Parse_Light_Source PARAMS((void));
static OBJECT *Parse_Object PARAMS((void));
static void Parse_Fog PARAMS((void));
static void Parse_Frame PARAMS((void));
static void Parse_Camera PARAMS((CAMERA **Camera_Ptr));
static void Parse_Declare PARAMS((void));
static void Link PARAMS((OBJECT *New_Object,OBJECT **Field,OBJECT **Old_Object_List));
static void Link_Textures PARAMS((TEXTURE **Old_Texture, TEXTURE *New_Texture));
static char *Get_Token_String PARAMS((TOKEN Token_Id));
static void Where_Error PARAMS((void));
static int Test_Redefine PARAMS((int a));
static OBJECT *Parse_Bound_Clip PARAMS((void));
static void Found_Instead PARAMS((void));
/*static void Parse_Warn PARAMS((TOKEN Token_Id));*/
static void Warn_State PARAMS((TOKEN Token_Id, TOKEN Type));
static void Post_Process PARAMS((OBJECT *Object, OBJECT *Parent));
static void Destroy_Constants PARAMS((void));
static OBJECT *Parse_Object_Id PARAMS((void));
static void Link_To_Frame PARAMS((OBJECT *Object));

extern struct Constant_Struct Constants[MAX_CONSTANTS];

int Number_Of_Constants;
int Previous;
short Have_Vector;
short Not_In_Default;

TOKEN *Brace_Stack;
int Brace_Index;

TEXTURE *Default_Texture;
CAMERA *Default_Camera;

/* Parse the file. */
void Parse ()
  {
   Build_Entries  = NULL;
   if ((Brace_Stack = (TOKEN *) malloc(MAX_BRACES*sizeof (TOKEN))) == NULL)
     MAError ("brace stack");
   Brace_Index = 0;
   Token_Init ();
   Default_Camera = Create_Camera();
   Default_Texture = Create_PNF_Texture();
   Default_Texture->Pigment = Create_Pigment();
   Default_Texture->Tnormal = NULL;
   Default_Texture->Finish  = Create_Finish();
   Not_In_Default = TRUE;
   Frame_Init ();
   Parse_Frame ();
   if (Frame.Objects==NULL)
     Error("No objects in scene");
   Destroy_Constants ();
   Destroy_Textures(Default_Texture);
   Destroy_Camera(Default_Camera);
   if (Build_Entries != NULL)
     free (Build_Entries);
   free (Brace_Stack);
  }

static void Token_Init ()
  {
   Number_Of_Constants = 0;
  }

/* Set up the fields in the frame to default values. */
static
void Frame_Init ()
  {
   Frame.Camera = Copy_Camera(Default_Camera);
   Frame.Light_Sources = NULL;
   Frame.Objects = NULL;
   Frame.Atmosphere_IOR = 1.0;
   Frame.Antialias_Threshold = Antialias_Threshold;
   Frame.Fog_Distance = 0.0;
   Make_Colour (&(Frame.Fog_Colour), 0.0, 0.0, 0.0);
  }

void Parse_Begin ()
  {
   char *front;

   Brace_Stack[++Brace_Index]=Token.Token_Id;
   Get_Token ();

   if (Token.Token_Id == LEFT_CURLY_TOKEN)
     return;

   Where_Error ();

   front = Get_Token_String (Brace_Stack[Brace_Index]);
   fprintf (stderr, "Missing { after %s, ", front);
   Found_Instead ();
   exit (1);
  }

void Parse_End ()
  {
   char *front;

   Get_Token ();

   if (Token.Token_Id == RIGHT_CURLY_TOKEN)
     {
      Brace_Index--;
      return;
     }

   Where_Error ();

   front = Get_Token_String (Brace_Stack[Brace_Index]);
   fprintf (stderr, "No matching } in %s,", front);
   Found_Instead ();
   exit (1);
  }

static OBJECT *Parse_Object_Id ()
  {
   OBJECT *Object;
   
   EXPECT
     CASE (OBJECT_ID_TOKEN)
       Warn_State(OBJECT_ID_TOKEN, OBJECT_TOKEN);
       Object = Copy_Object((OBJECT *) Token.Constant_Data);
       Parse_Object_Mods (Object);
       EXIT
     END_CASE

     OTHERWISE
       Object = NULL;
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   return (Object);
  }

void Parse_Comma ()
  {
   Get_Token();
   if (Token.Token_Id != COMMA_TOKEN)
     {
      UNGET;
     }
  }

static void Parse_Coeffs(order, Coeffs)
  int order;
  DBL *Coeffs;
  {
   int i;

   EXPECT
     CASE (LEFT_ANGLE_TOKEN)
       Coeffs[0] = Parse_Float();
       for (i = 1; i < term_counts(order); i++)
         {
          Parse_Comma();
          Coeffs[i] = Parse_Float();
         }
       GET (RIGHT_ANGLE_TOKEN);
       EXIT
     END_CASE

     OTHERWISE
       Parse_Error (LEFT_ANGLE_TOKEN);
     END_CASE
   END_EXPECT
  }

static
IMAGE *Parse_Image (Legal)
  int Legal;
  {
   IMAGE *Image;
   VECTOR Local_Vector;

   Image = Create_Image ();

   if (Legal & GRAD_FILE)
     {
      EXPECT
        CASE_VECTOR
          Warn("Should use map_type keyword and/or eliminate orientation.",1.5);
          Have_Vector = FALSE;
          Parse_Vector_Float (&Local_Vector);
          if (Have_Vector)
            Image->Gradient = Local_Vector;
          else
            Image->Map_Type = (int)Local_Vector.x;
        END_CASE

        OTHERWISE
          UNGET
          EXIT
        END_CASE
      END_EXPECT
     }

   EXPECT
     CASE (IFF_TOKEN)
       Image->File_Type = IFF_FILE;
       GET (STRING_TOKEN);
       Read_Iff_Image (Image, Token.Token_String);
       EXIT
     END_CASE

     CASE (GIF_TOKEN)
       Image->File_Type = GIF_FILE;
       GET (STRING_TOKEN);
       Read_Gif_Image(Image, Token.Token_String);
       EXIT
     END_CASE

     CASE (POT_TOKEN)
       Image->File_Type = POT_FILE;
       GET (STRING_TOKEN);
       Read_Gif_Image(Image, Token.Token_String);
       EXIT
     END_CASE

     CASE (DUMP_TOKEN)
       Image->File_Type = DUMP_FILE;
       GET (STRING_TOKEN);
       Read_Dump_Image(Image, Token.Token_String);
       EXIT
     END_CASE

     CASE (TGA_TOKEN)
       Image->File_Type = TGA_FILE;
       GET (STRING_TOKEN);
       Read_Targa_Image(Image, Token.Token_String);
       EXIT
     END_CASE

     OTHERWISE
       Parse_Error_Str ("map file spec");
     END_CASE
   END_EXPECT

   if (!(Image->File_Type & Legal))
     Error ("File type not supported here");
   return (Image);
  }

static void Parse_Image_Map (Pigment)
  PIGMENT *Pigment;
  {
   int reg;

   Pigment->Type = IMAGE_MAP_PIGMENT;

   Parse_Begin ();

   Pigment->Image = Parse_Image (IMAGE_FILE);
   Pigment->Image->Use_Colour_Flag = TRUE;

   EXPECT                   /* Look for image_attribs */
     CASE (ONCE_TOKEN)
       Pigment->Image->Once_Flag=TRUE;
     END_CASE

     CASE (INTERPOLATE_TOKEN)
       Pigment->Image->Interpolation_Type = (int)Parse_Float();
     END_CASE

     CASE (MAP_TYPE_TOKEN)
       Pigment->Image->Map_Type = (int) Parse_Float ();
     END_CASE

     CASE (USE_COLOUR_TOKEN)
       Pigment->Image->Use_Colour_Flag = TRUE;
     END_CASE

     CASE (USE_INDEX_TOKEN)
       Pigment->Image->Use_Colour_Flag = FALSE;
     END_CASE

     CASE (ALPHA_TOKEN)
       Warn("Keyword ALPHA discontinued.  Use FILTER instead.",1.55);
         
     CASE (FILTER_TOKEN)
       EXPECT
         CASE (ALL_TOKEN)
           {
            DBL filter;
            filter = Parse_Float();
            for (reg = 0 ; reg < Pigment->Image->Colour_Map_Size ; reg++)
              Pigment->Image->Colour_Map[reg].Filter
                  = (unsigned short) (filter *255.0);
           }
           EXIT
         END_CASE

         OTHERWISE
           UNGET
           reg = (int)(Parse_Float() + 0.01);
           if (Pigment->Image->Colour_Map == NULL)
             Error ("Can't apply FILTER to a non colour-mapped image\n");
           if ((reg < 0) || (reg >= Pigment->Image->Colour_Map_Size))
             Error ("FILTER colour register value out of range.\n");

           Parse_Comma();
           Pigment->Image->Colour_Map[reg].Filter
                  = (unsigned short) (255.0 * Parse_Float());
           EXIT
         END_CASE

       END_EXPECT
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   Parse_End ();
}

static void Parse_Bump_Map (Tnormal)
  TNORMAL *Tnormal;
  {
   Tnormal->Type = BUMP_MAP;

   Parse_Begin ();

   Tnormal->Image = Parse_Image (NORMAL_FILE);

   Tnormal->Image->Use_Colour_Flag = TRUE;

   EXPECT
     CASE (ONCE_TOKEN)
       Tnormal->Image->Once_Flag=TRUE;
     END_CASE

     CASE (MAP_TYPE_TOKEN)
       Tnormal->Image->Map_Type = (int) Parse_Float ();
     END_CASE

     CASE (INTERPOLATE_TOKEN)
       Tnormal->Image->Interpolation_Type = (int)Parse_Float();
     END_CASE

     CASE (BUMP_SIZE_TOKEN)
       Tnormal->Amount = Parse_Float ();
     END_CASE

     CASE (USE_COLOUR_TOKEN)
       Tnormal->Image->Use_Colour_Flag = TRUE;
     END_CASE

     CASE (USE_INDEX_TOKEN)
       Tnormal->Image->Use_Colour_Flag = FALSE;
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT
   Parse_End ();
}

static void Parse_Pigment (Pigment_Ptr)
  PIGMENT **Pigment_Ptr;
  {
   PIGMENT *New;
   VECTOR Local_Vector;

   Parse_Begin ();

   EXPECT            /* Look for [pigment_id] */
     CASE (PIGMENT_ID_TOKEN)
       Destroy_Pigment(*Pigment_Ptr);
       *Pigment_Ptr = Copy_Pigment ((PIGMENT *) Token.Constant_Data);
       EXIT
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT    /* End pigment_id */

   New = *Pigment_Ptr;

   EXPECT
     CASE (AGATE_TOKEN)
       New->Type = AGATE_PIGMENT;
       EXIT
     END_CASE

     CASE (BOZO_TOKEN)
       New->Type = BOZO_PIGMENT;
       EXIT
     END_CASE

     CASE (GRANITE_TOKEN)
       New->Type = GRANITE_PIGMENT;
       EXIT
     END_CASE

     CASE (LEOPARD_TOKEN)
       New->Type = LEOPARD_PIGMENT;
       EXIT
     END_CASE

     CASE (MARBLE_TOKEN)
       New->Type = MARBLE_PIGMENT;
       EXIT
     END_CASE

     CASE (MANDEL_TOKEN)
       New->Type = MANDEL_PIGMENT;
       New->Iterations = (int)Parse_Float();
       EXIT
     END_CASE

     CASE (ONION_TOKEN)
       New->Type = ONION_PIGMENT;
       EXIT
     END_CASE

     CASE (PAINTED1_TOKEN)
       New->Type = PAINTED1_PIGMENT;
       EXIT
     END_CASE

     CASE (PAINTED2_TOKEN)
       New->Type = PAINTED2_PIGMENT;
       EXIT
     END_CASE

     CASE (PAINTED3_TOKEN)
       New->Type = PAINTED2_PIGMENT;
       EXIT
     END_CASE

     CASE (SPOTTED_TOKEN)
       New->Type = SPOTTED_PIGMENT;
       EXIT
     END_CASE

     CASE (WOOD_TOKEN)
       New->Type = WOOD_PIGMENT;
       EXIT
     END_CASE

     CASE (GRADIENT_TOKEN)
       New->Type = GRADIENT_PIGMENT;
       Parse_Vector (&(New->Colour_Gradient));
       EXIT
     END_CASE

     CASE (RADIAL_TOKEN)
       New->Type = RADIAL_PIGMENT;
     END_CASE

     CASE (COLOUR_TOKEN)
       New->Type = COLOUR_PIGMENT;
       New->Colour1 = Create_Colour ();
       Parse_Colour (New->Colour1);
       New->Quick_Colour = *New->Colour1;
       EXIT
     END_CASE

     CASE5 (COLOUR_ID_TOKEN, RGB_TOKEN, RGBF_TOKEN, RED_TOKEN, BLUE_TOKEN)
     CASE3 (GREEN_TOKEN, ALPHA_TOKEN, FILTER_TOKEN)
       UNGET
       New->Type = COLOUR_PIGMENT;
       New->Colour1 = Create_Colour ();
       Parse_Colour (New->Colour1);
       New->Quick_Colour = *New->Colour1;
       EXIT
     END_CASE

     CASE (CHECKER_TOKEN)
       New->Type = CHECKER_PIGMENT;
       New->Colour_Map = Parse_Colour_List(2);
       EXIT
     END_CASE

     CASE (HEXAGON_TOKEN)
       New->Type = HEXAGON_PIGMENT;
       New->Colour_Map = Parse_Colour_List(3);
       EXIT
     END_CASE

     CASE (IMAGE_MAP_TOKEN)
       Parse_Image_Map (New);
       EXIT
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT     /* Concludes pigment_body */

   EXPECT         /* Look for pigment_modifier */
     CASE (TURBULENCE_TOKEN)
       Parse_Vector_Float(&(New->Turbulence));
       if ((New->Turbulence.x !=0.0) || (New->Turbulence.y !=0.0) ||
           (New->Turbulence.z !=0.0))
          New->Flags |= HAS_TURB;
     END_CASE

     CASE (COLOUR_MAP_TOKEN)
       if (New->Type == CHECKER_PIGMENT ||
           New->Type == HEXAGON_PIGMENT ||
           New->Type == COLOUR_PIGMENT ||
           New->Type == IMAGE_MAP_PIGMENT)
         Warn ("Cannot use color map with this pigment type",1.5);
       New->Colour_Map = Parse_Colour_Map ();
     END_CASE

     CASE (QUICK_COLOUR_TOKEN)
       Parse_Colour (&New->Quick_Colour);
     END_CASE

     CASE (OCTAVES_TOKEN)
       New->Octaves = (int)Parse_Float();
         if(New->Octaves < 1)
            New->Octaves = 1;
         if(New->Octaves > 10)  /* Avoid DOMAIN errors */
            New->Octaves = 10;
     END_CASE

     CASE (OMEGA_TOKEN)
       New->omega = Parse_Float();
     END_CASE

     CASE (LAMBDA_TOKEN)
       New->lambda = Parse_Float();
     END_CASE

     CASE (FREQUENCY_TOKEN)
       New->Frequency = Parse_Float();
     END_CASE

     CASE (PHASE_TOKEN)
       New->Phase = Parse_Float();
     END_CASE

     CASE (AGATE_TURB_TOKEN)
       if (Not_In_Default && (New->Type != AGATE_PIGMENT))
          Warn("Attempt to use agate_turb on non-agate",1.9);
       New->Agate_Turb_Scale = Parse_Float();
     END_CASE

     CASE (TRANSLATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Translate_Pigment (New, &Local_Vector);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Rotate_Pigment (New, &Local_Vector);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (&Local_Vector);
       Scale_Pigment (New, &Local_Vector);
     END_CASE

     CASE (TRANSFORM_TOKEN)
       GET(TRANSFORM_ID_TOKEN)
       Transform_Pigment (New, (TRANSFORM *)Token.Constant_Data);
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   if (Not_In_Default && (New->Type == NO_PIGMENT))
     Warn("Pigment type unspecified or not 1st item",1.7);

   Parse_End ();
  }

static void Parse_Tnormal (Tnormal_Ptr)
  TNORMAL **Tnormal_Ptr;
  {
   TNORMAL *New;
   VECTOR Local_Vector;

   Parse_Begin ();

   EXPECT            /* Look for [tnormal_id] */
     CASE (TNORMAL_ID_TOKEN)
       Destroy_Tnormal(*Tnormal_Ptr);
       *Tnormal_Ptr = Copy_Tnormal ((TNORMAL *) Token.Constant_Data);
       EXIT
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT    /* End [tnormal_id] */

   if (*Tnormal_Ptr == NULL)
     if ((Default_Texture->Tnormal) != NULL)
       *Tnormal_Ptr = Copy_Tnormal ((Default_Texture->Tnormal));
     else
       *Tnormal_Ptr = Create_Tnormal ();

   New = *Tnormal_Ptr;

   EXPECT  /* [tnormal_body] */
     CASE (BUMPS_TOKEN)
       New->Type = BUMPS;
       New->Amount = Parse_Float ();
       EXIT
     END_CASE

     CASE (BUMPY1_TOKEN)
       New->Type = BUMPY1;
       New->Amount = Parse_Float ();
       EXIT
     END_CASE

     CASE (BUMPY2_TOKEN)
       New->Type = BUMPY2;
       New->Amount = Parse_Float ();
       EXIT
     END_CASE

     CASE (BUMPY3_TOKEN)
       New->Type = BUMPY3;
       New->Amount = Parse_Float ();
       EXIT
     END_CASE

     CASE (DENTS_TOKEN)
       New->Type = DENTS;
       New->Amount = Parse_Float ();
       EXIT
     END_CASE

     CASE (RIPPLES_TOKEN)
       New->Type = RIPPLES;
       New->Amount = Parse_Float ();
       EXIT
     END_CASE

     CASE (WAVES_TOKEN)
       New->Type = WAVES;
       New->Amount = Parse_Float ();
       EXIT
     END_CASE

     CASE (WRINKLES_TOKEN)
       New->Type = WRINKLES;
       New->Amount = Parse_Float ();
       EXIT
     END_CASE

     CASE (BUMP_MAP_TOKEN)
       Parse_Bump_Map (New);
       EXIT
     END_CASE

     OTHERWISE
       if (Not_In_Default && (New->Type == NO_NORMAL))
         Parse_Error_Str("normal body");
       UNGET
       EXIT
     END_CASE
   END_EXPECT    /* End of tnormal_body */

   EXPECT        /* Look for tnormal_mods */

     CASE (TURBULENCE_TOKEN)
       Parse_Vector_Float(&(New->Turbulence));
       if ((New->Turbulence.x !=0.0) || (New->Turbulence.y !=0.0) ||
           (New->Turbulence.z !=0.0))
          New->Flags |= HAS_TURB;
     END_CASE

     CASE (OCTAVES_TOKEN)
       New->Octaves = (int)Parse_Float();
     END_CASE

     CASE (OMEGA_TOKEN)
       New->omega = Parse_Float();
     END_CASE

     CASE (LAMBDA_TOKEN)
       New->lambda = Parse_Float();
     END_CASE

     CASE (FREQUENCY_TOKEN)
       if (!(New->Type == RIPPLES || New->Type == WAVES))
         if (Language_Version >= 1.5)
           Warn ("Cannot use frequency with this normal",1.5);
       New->Frequency = Parse_Float();
     END_CASE

     CASE (PHASE_TOKEN)
       if (!(New->Type == RIPPLES || New->Type == WAVES))
         if (Language_Version >= 1.5)
            Warn ("Cannot use phase with this normal",1.5);
       New->Phase = Parse_Float();
     END_CASE

     CASE (TRANSLATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Translate_Tnormal (New, &Local_Vector);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Rotate_Tnormal (New, &Local_Vector);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (&Local_Vector);
       Scale_Tnormal (New, &Local_Vector);
     END_CASE

     CASE (TRANSFORM_TOKEN)
       GET(TRANSFORM_ID_TOKEN)
       Transform_Tnormal (New, (TRANSFORM *)Token.Constant_Data);
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT    /* End of tnormal_mods */

   Parse_End ();
  }

static void Parse_Finish (Finish_Ptr)
  FINISH **Finish_Ptr;
  {
   FINISH *New;

   Parse_Begin ();

   EXPECT        /* Look for zero or one finish_id */
     CASE (FINISH_ID_TOKEN)
       Destroy_Finish(*Finish_Ptr);
       *Finish_Ptr = Copy_Finish ((FINISH *) Token.Constant_Data);
       EXIT
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT    /* End finish_id */

   New = *Finish_Ptr;

   EXPECT        /* Look for zero or more finish_body */
     CASE (AMBIENT_TOKEN)
       New->Ambient = Parse_Float ();
     END_CASE

     CASE (BRILLIANCE_TOKEN)
       New->Brilliance = Parse_Float ();
     END_CASE

     CASE (DIFFUSE_TOKEN)
       New->Diffuse = Parse_Float ();
     END_CASE

     CASE (REFLECTION_TOKEN)
       New->Reflection = Parse_Float ();
     END_CASE

     CASE (REFRACTION_TOKEN)
       New->Refraction = Parse_Float ();
     END_CASE

     CASE (IOR_TOKEN)
       New->Index_Of_Refraction = Parse_Float ();
     END_CASE

     CASE (PHONG_TOKEN)
       New->Phong = Parse_Float ();
     END_CASE

     CASE (PHONG_SIZE_TOKEN)
       New->Phong_Size = Parse_Float ();
/*     if (New->Phong_Size < 1.0)
           New->Phong_Size = 1.0;
       if (New->Phong_Size > 100)
           New->Phong_Size = 100; */
     END_CASE

     CASE (SPECULAR_TOKEN)
       New->Specular = Parse_Float ();
     END_CASE

     CASE (ROUGHNESS_TOKEN)
       New->Roughness = Parse_Float ();
/*     if (New->Roughness > 1.0)
           New->Roughness = 1.0;
       if (New->Roughness < 0.001)
           New->Roughness = 0.001;  */
       New->Roughness = 1.0/New->Roughness; /* CEY 12/92 */
     END_CASE

     CASE (METALLIC_TOKEN)
       New->Metallic_Flag = TRUE;
     END_CASE

     CASE (CRAND_TOKEN)
       New->Crand = Parse_Float();
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT    /* End of finish_body */

   EXPECT        /* Look for finish_mods */

/*   CASE none implemented
     END_CASE     */

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT    /* End of finish_mods */

   Parse_End ();
  }

#define ADD_TNORMAL if (Tnormal == NULL) {if ((Default_Texture->Tnormal) != NULL) \
 Tnormal = Copy_Tnormal ((Default_Texture->Tnormal)); else Tnormal = Create_Tnormal ();\
 Texture->Tnormal=Tnormal;};

static
TEXTURE *Parse_Texture ()
  {
   VECTOR Local_Vector;
   TEXTURE *Texture, *Local_Texture;
   PIGMENT *Pigment;
   TNORMAL *Tnormal;
   FINISH *Finish;

   Parse_Begin ();

   EXPECT                      /* Look for texture_body */
     CASE (TILES_TOKEN)
       Parse_Begin ();

       Texture = (TEXTURE *)Create_Tiles_Texture ();

       EXPECT
         CASE (TEXTURE_TOKEN)
           Local_Texture = Parse_Texture ();
           Link_Textures(&(((TILES *)Texture)->Tile1),Local_Texture);
         END_CASE

         OTHERWISE
           UNGET
           EXIT
         END_CASE
       END_EXPECT

       GET (TILE2_TOKEN);

       EXPECT
         CASE (TEXTURE_TOKEN)
           Local_Texture = Parse_Texture ();
           Link_Textures(&(((TILES *)Texture)->Tile2),Local_Texture);
         END_CASE

         OTHERWISE
           UNGET
           EXIT
         END_CASE
       END_EXPECT
       Parse_End ();
       EXIT
     END_CASE

     CASE (MATERIAL_MAP_TOKEN)
       Parse_Begin ();

       Texture = (TEXTURE *)Create_Material_Texture ();

       ((MATERIAL *)Texture)->Image = Parse_Image(MATERIAL_FILE);
       ((MATERIAL *)Texture)->Image->Use_Colour_Flag = FALSE;

       EXPECT
         CASE (ONCE_TOKEN)
           ((MATERIAL *)Texture)->Image->Once_Flag=TRUE;
         END_CASE

         CASE (INTERPOLATE_TOKEN)
           ((MATERIAL *)Texture)->Image->Interpolation_Type=(int)Parse_Float();
         END_CASE

         CASE (MAP_TYPE_TOKEN)
           ((MATERIAL *)Texture)->Image->Map_Type = (int) Parse_Float ();
         END_CASE

         OTHERWISE
           UNGET
           EXIT
         END_CASE
       END_EXPECT

       GET (TEXTURE_TOKEN)                /* First material */
       ((MATERIAL *)Texture)->Materials = Local_Texture = Parse_Texture ();
       ((MATERIAL *)Texture)->Num_Of_Mats++;

       EXPECT                             /* Subsequent materials */
         CASE (TEXTURE_TOKEN)
           Local_Texture->Next_Material = Parse_Texture ();
           Local_Texture = Local_Texture->Next_Material;
           ((MATERIAL *)Texture)->Num_Of_Mats++;
         END_CASE

         OTHERWISE
           UNGET
           EXIT
         END_CASE
       END_EXPECT
       Parse_End ();
       EXIT
     END_CASE

     OTHERWISE  /* Look for [pnf_texture] */
       UNGET

       Texture = Copy_Textures (Default_Texture);

       EXPECT   /* Look for [tpnf_ids] */
         CASE (TEXTURE_ID_TOKEN)
           Destroy_Textures(Texture);
           Texture = Copy_Textures((TEXTURE *) Token.Constant_Data);
         END_CASE

         CASE (PIGMENT_ID_TOKEN)
           Destroy_Pigment(Texture->Pigment);
           Texture->Pigment = Copy_Pigment ((PIGMENT *) Token.Constant_Data);
         END_CASE

         CASE (TNORMAL_ID_TOKEN)
           Destroy_Tnormal(Texture->Tnormal);
           Texture->Tnormal = Copy_Tnormal ((TNORMAL *) Token.Constant_Data);
         END_CASE

         CASE (FINISH_ID_TOKEN)
           Destroy_Finish(Texture->Finish);
           Texture->Finish = Copy_Finish ((FINISH *) Token.Constant_Data);
         END_CASE

         OTHERWISE
           UNGET
           EXIT
         END_CASE
       END_EXPECT

       Pigment = Texture->Pigment;
       Tnormal = Texture->Tnormal;
       Finish  = Texture->Finish;

       EXPECT
         CASE (PIGMENT_TOKEN)
           Parse_Pigment ( &(Texture->Pigment) );
         END_CASE

         CASE (TNORMAL_TOKEN)
           Parse_Tnormal ( &(Texture->Tnormal) );
         END_CASE

         CASE (FINISH_TOKEN)
           Parse_Finish ( &(Texture->Finish) );
         END_CASE

/***********************************************************************
 PIGMENT STUFF OUTSIDE PIGMENT{}
***********************************************************************/
         CASE (AGATE_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = AGATE_PIGMENT;
         END_CASE

         CASE (BOZO_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = BOZO_PIGMENT;
         END_CASE

         CASE (GRANITE_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = GRANITE_PIGMENT;
         END_CASE

         CASE (LEOPARD_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = LEOPARD_PIGMENT;
         END_CASE

         CASE (MARBLE_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = MARBLE_PIGMENT;
         END_CASE

         CASE (MANDEL_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = MANDEL_PIGMENT;
           Pigment->Iterations = (int)Parse_Float();
         END_CASE

         CASE (ONION_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = ONION_PIGMENT;
         END_CASE

         CASE (PAINTED1_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = PAINTED1_PIGMENT;
         END_CASE

         CASE (PAINTED2_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = PAINTED2_PIGMENT;
         END_CASE

         CASE (PAINTED3_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = PAINTED2_PIGMENT;
         END_CASE

         CASE (SPOTTED_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = SPOTTED_PIGMENT;
         END_CASE

         CASE (WOOD_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = WOOD_PIGMENT;
         END_CASE

         CASE (GRADIENT_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = GRADIENT_PIGMENT;
           Parse_Vector (&(Pigment->Colour_Gradient));
         END_CASE

         CASE (COLOUR_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = COLOUR_PIGMENT;
           Pigment->Colour1 = Create_Colour ();
           Parse_Colour (Pigment->Colour1);
           Pigment->Quick_Colour = *Pigment->Colour1;
         END_CASE

         CASE5 (COLOUR_ID_TOKEN, RGB_TOKEN, RGBF_TOKEN, RED_TOKEN, BLUE_TOKEN)
         CASE3 (GREEN_TOKEN, ALPHA_TOKEN, FILTER_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           UNGET
           Pigment->Type = COLOUR_PIGMENT;
           Pigment->Colour1 = Create_Colour ();
           Parse_Colour (Pigment->Colour1);
           Pigment->Quick_Colour = *Pigment->Colour1;
         END_CASE

         CASE (CHECKER_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = CHECKER_PIGMENT;
           Pigment->Colour_Map = Parse_Colour_List(2);
         END_CASE

         CASE (HEXAGON_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Pigment->Type = HEXAGON_PIGMENT;
           Pigment->Colour_Map = Parse_Colour_List(3);
         END_CASE

         CASE (IMAGE_MAP_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Parse_Image_Map (Pigment);
         END_CASE

         CASE (TURBULENCE_TOKEN)
           Parse_Vector_Float(&(Pigment->Turbulence));
           if ((Pigment->Turbulence.x !=0.0) ||
               (Pigment->Turbulence.y !=0.0) ||
               (Pigment->Turbulence.z !=0.0))
             Pigment->Flags |= HAS_TURB;
         END_CASE

         CASE (COLOUR_MAP_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);
           if (Pigment->Type == CHECKER_PIGMENT ||
               Pigment->Type == HEXAGON_PIGMENT ||
               Pigment->Type == COLOUR_PIGMENT ||
               Pigment->Type == IMAGE_MAP_PIGMENT)
             Warn ("Cannot use color map with this pigment type",1.5);
           Pigment->Colour_Map = Parse_Colour_Map ();
         END_CASE

         CASE (QUICK_COLOUR_TOKEN)
           Warn_State(Token.Token_Id, PIGMENT_TOKEN);           
           Parse_Colour (&Pigment->Quick_Colour);
         END_CASE

         CASE (OCTAVES_TOKEN)
           Pigment->Octaves = (int)Parse_Float();
             if(Pigment->Octaves < 1)
                Pigment->Octaves = 1;
             if(Pigment->Octaves > 10)  /* Avoid DOMAIN errors */
                Pigment->Octaves = 10;
         END_CASE

         CASE (OMEGA_TOKEN)
           Pigment->omega = Parse_Float();
         END_CASE

         CASE (LAMBDA_TOKEN)
           Pigment->lambda = Parse_Float();
         END_CASE

/***********************************************************************
TNORMAL STUFF OUTSIDE NORMAL{}
***********************************************************************/
         CASE (BUMPS_TOKEN)
           Warn_State(Token.Token_Id, TNORMAL_TOKEN);           
           ADD_TNORMAL
           Tnormal->Type = BUMPS;
           Tnormal->Amount = Parse_Float ();
         END_CASE

         CASE (BUMPY1_TOKEN)
           Warn_State(Token.Token_Id, TNORMAL_TOKEN);           
           ADD_TNORMAL
           Tnormal->Type = BUMPY1;
           Tnormal->Amount = Parse_Float ();
         END_CASE

         CASE (BUMPY2_TOKEN)
           Warn_State(Token.Token_Id, TNORMAL_TOKEN);           
           ADD_TNORMAL
           Tnormal->Type = BUMPY2;
           Tnormal->Amount = Parse_Float ();
         END_CASE

         CASE (BUMPY3_TOKEN)
           Warn_State(Token.Token_Id, TNORMAL_TOKEN);           
           ADD_TNORMAL
           Tnormal->Type = BUMPY3;
           Tnormal->Amount = Parse_Float ();
         END_CASE

         CASE (DENTS_TOKEN)
           Warn_State(Token.Token_Id, TNORMAL_TOKEN);           
           ADD_TNORMAL
           Tnormal->Type = DENTS;
           Tnormal->Amount = Parse_Float ();
         END_CASE

         CASE (RIPPLES_TOKEN)
           Warn_State(Token.Token_Id, TNORMAL_TOKEN);           
           ADD_TNORMAL
           Tnormal->Type = RIPPLES;
           Tnormal->Amount = Parse_Float ();
         END_CASE

         CASE (WAVES_TOKEN)
           Warn_State(Token.Token_Id, TNORMAL_TOKEN);           
           ADD_TNORMAL
           Tnormal->Type = WAVES;
           Tnormal->Amount = Parse_Float ();
         END_CASE

         CASE (WRINKLES_TOKEN)
           Warn_State(Token.Token_Id, TNORMAL_TOKEN);           
           ADD_TNORMAL
           Tnormal->Type = WRINKLES;
           Tnormal->Amount = Parse_Float ();
         END_CASE

         CASE (BUMP_MAP_TOKEN)
           Warn_State(Token.Token_Id, TNORMAL_TOKEN);           
           ADD_TNORMAL
           Parse_Bump_Map (Tnormal);
         END_CASE

         CASE (FREQUENCY_TOKEN)
           Warn_State(Token.Token_Id, TNORMAL_TOKEN);           
           ADD_TNORMAL
           if (!(Tnormal->Type == RIPPLES || Tnormal->Type == WAVES))
             if (Language_Version >= 1.5)
               Warn ("Cannot use frequency with this normal",1.5);
           Tnormal->Frequency = Parse_Float();
         END_CASE

         CASE (PHASE_TOKEN)
           Warn_State(Token.Token_Id, TNORMAL_TOKEN);           
           ADD_TNORMAL
           if (!(Tnormal->Type == RIPPLES || Tnormal->Type == WAVES))
             if (Language_Version >= 1.5)
               Warn ("Cannot use phase with this normal",1.5);
           Tnormal->Phase = Parse_Float();
         END_CASE


/***********************************************************************
FINISH STUFF OUTSIDE FINISH{}
***********************************************************************/
         CASE (AMBIENT_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Ambient = Parse_Float ();
         END_CASE

         CASE (BRILLIANCE_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Brilliance = Parse_Float ();
         END_CASE

         CASE (DIFFUSE_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Diffuse = Parse_Float ();
         END_CASE

         CASE (REFLECTION_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Reflection = Parse_Float ();
         END_CASE

         CASE (REFRACTION_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Refraction = Parse_Float ();
         END_CASE

         CASE (IOR_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Index_Of_Refraction = Parse_Float ();
         END_CASE

         CASE (PHONG_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Phong = Parse_Float ();
         END_CASE

         CASE (PHONG_SIZE_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Phong_Size = Parse_Float ();
    /*     if (Finish->Phong_Size < 1.0)
               Finish->Phong_Size = 1.0;
           if (Finish->Phong_Size > 100)
               Finish->Phong_Size = 100; */
         END_CASE

         CASE (SPECULAR_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Specular = Parse_Float ();
         END_CASE

         CASE (ROUGHNESS_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Roughness = Parse_Float ();
    /*     if (Finish->Roughness > 1.0)
               Finish->Roughness = 1.0;
           if (Finish->Roughness < 0.001)
               Finish->Roughness = 0.001;  */
           Finish->Roughness = 1.0/Finish->Roughness; /* CEY 12/92 */
         END_CASE

         CASE (METALLIC_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Metallic_Flag = TRUE;
         END_CASE

         CASE (CRAND_TOKEN)
           Warn_State(Token.Token_Id, FINISH_TOKEN);           
           Finish->Crand = Parse_Float();
         END_CASE

         CASE_FLOAT
           Finish->Crand = Parse_Float();
           Warn("Should use crand keyword in finish statement.",1.5);           
         END_CASE

         CASE (TRANSLATE_TOKEN)
           Parse_Vector (&Local_Vector);
           Translate_Textures (Texture, &Local_Vector);
         END_CASE

         CASE (ROTATE_TOKEN)
           Parse_Vector (&Local_Vector);
           Rotate_Textures (Texture, &Local_Vector);
         END_CASE

         CASE (SCALE_TOKEN)
           Parse_Scale_Vector (&Local_Vector);
           Scale_Textures (Texture, &Local_Vector);
         END_CASE

         CASE (TRANSFORM_TOKEN)
           GET(TRANSFORM_ID_TOKEN)
           Transform_Textures (Texture, (TRANSFORM *)Token.Constant_Data);
         END_CASE

         CASE (TEXTURE_ID_TOKEN)
           Warn("Texture identifier overwriting previous values.",0);
           Destroy_Textures(Texture);
           Texture = Copy_Textures((TEXTURE *) Token.Constant_Data);
           Pigment = Texture->Pigment;
           Tnormal = Texture->Tnormal;
           Finish  = Texture->Finish;
         END_CASE

         OTHERWISE
           UNGET
           EXIT
         END_CASE
/***********************************************************************/

       END_EXPECT

       if (Not_In_Default && (Texture->Pigment->Type == NO_PIGMENT) &&
           !(Language_Version < 1.5))
         Parse_Error(PIGMENT_ID_TOKEN);

       EXIT
     END_CASE        /* End of pnf texture */

   END_EXPECT       /* End of texture_body */

   EXPECT            /* Look for texture_mods */
     CASE (TRANSLATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Translate_Textures (Texture, &Local_Vector);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Rotate_Textures (Texture, &Local_Vector);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (&Local_Vector);
       Scale_Textures (Texture, &Local_Vector);
     END_CASE

     CASE (TRANSFORM_TOKEN)
       GET(TRANSFORM_ID_TOKEN)
       Transform_Textures (Texture, (TRANSFORM *)Token.Constant_Data);
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT        /* End of texture */

   Parse_End ();
   return (Texture);
  }

static
OBJECT *Parse_Bound_Clip ()
  {
   VECTOR Local_Vector;
   OBJECT *First, *Current, *Prev;

   First = Prev = NULL;

   while ((Current = Parse_Object ()) != NULL)
     {
      if (Current->Type & (TEXTURED_OBJECT+PATCH_OBJECT))
        Error ("Illegal texture or patch in clip or bound");
      if (First == NULL)
        First = Current;
      if (Prev != NULL)
        Prev->Sibling = Current;
      Prev = Current;
     }

   EXPECT
     CASE (TRANSLATE_TOKEN)
       Parse_Vector (&Local_Vector);
       for (Current = First;
            Current != NULL;
            Current = Current->Sibling)
         Translate_Object (Current, &Local_Vector);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (&Local_Vector);
       for (Current = First;
            Current != NULL;
            Current = Current->Sibling)
         Rotate_Object (Current, &Local_Vector);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (&Local_Vector);
       for (Current = First;
            Current != NULL;
            Current = Current->Sibling)
         Scale_Object (Current, &Local_Vector);
     END_CASE

     CASE (TRANSFORM_TOKEN)
       GET(TRANSFORM_ID_TOKEN)
       for (Current = First;
            Current != NULL;
            Current = Current->Sibling)
       Transform_Object (Current, (TRANSFORM *)Token.Constant_Data);
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   return (First);
  }

static void Parse_Object_Mods (Object)
  OBJECT *Object;
  {
   VECTOR Local_Vector;
   TEXTURE *Local_Texture;
   COLOUR Local_Colour;

   EXPECT
     CASE (COLOUR_TOKEN)
       Parse_Colour (&Local_Colour);
       if (Language_Version < 1.5)
         if (Object->Texture != NULL) 
           if (Object->Texture->Type == PNF_TEXTURE)
             {
              Object->Texture->Pigment->Quick_Colour = Local_Colour;
              break;  /* acts like END_CASE */
             }
       Warn("Quick color belongs in texture.  Color ignored.",0.0);
     END_CASE

     CASE (TRANSLATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Translate_Object (Object, &Local_Vector);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Rotate_Object (Object, &Local_Vector);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (&Local_Vector);
       Scale_Object (Object, &Local_Vector);
     END_CASE

     CASE (TRANSFORM_TOKEN)
       GET(TRANSFORM_ID_TOKEN)
       Transform_Object (Object, (TRANSFORM *)Token.Constant_Data);
     END_CASE

     CASE (BOUNDED_BY_TOKEN)
       Parse_Begin ();
       if (Object->Bound != NULL)
         Error ("Cannot have more than one BOUNDED_BY {} per object");

       EXPECT
         CASE (CLIPPED_BY_TOKEN)
           Object->Bound = Object->Clip;
           EXIT
         END_CASE

         OTHERWISE
           UNGET
           Object->Bound = Parse_Bound_Clip ();
           EXIT
         END_CASE
       END_EXPECT
       
       Parse_End ();
     END_CASE

     CASE (CLIPPED_BY_TOKEN)
       Parse_Begin ();
       if (Object->Clip != NULL)
         Error ("Cannot have more than one CLIPPED_BY {} per object");

       EXPECT
         CASE (BOUNDED_BY_TOKEN)
           Object->Clip = Object->Bound;
           EXIT
         END_CASE

         OTHERWISE
           UNGET
           Object->Clip = Parse_Bound_Clip ();
           EXIT
         END_CASE
       END_EXPECT

       Parse_End ();
     END_CASE

     CASE (TEXTURE_TOKEN)
       Object->Type |= TEXTURED_OBJECT;
       Local_Texture = Parse_Texture ();
       Link_Textures(&(Object->Texture), Local_Texture);
     END_CASE

     CASE3 (PIGMENT_TOKEN, TNORMAL_TOKEN, FINISH_TOKEN)
       Object->Type |= TEXTURED_OBJECT;
       if (Object->Texture == NULL)
         Object->Texture = Copy_Textures(Default_Texture);
       else
         if (Object->Texture->Type != PNF_TEXTURE)
           Link_Textures(&(Object->Texture), Copy_Textures(Default_Texture));
       UNGET
       EXPECT
         CASE (PIGMENT_TOKEN)
           Parse_Pigment ( &(Object->Texture->Pigment) );
         END_CASE

         CASE (TNORMAL_TOKEN)
           Parse_Tnormal ( &(Object->Texture->Tnormal) );
         END_CASE

         CASE (FINISH_TOKEN)
           Parse_Finish ( &(Object->Texture->Finish) );
         END_CASE

         OTHERWISE
           UNGET
           EXIT
         END_CASE
       END_EXPECT
     END_CASE

     CASE (INVERSE_TOKEN)
       if (Object->Type & PATCH_OBJECT)
         Warn ("Cannot invert a patch object",0.0);
       Invert_Object (Object);
     END_CASE

     CASE (STURM_TOKEN)
       if (!(Object->Type & STURM_OK_OBJECT))
         Error ("Cannot use STRUM here");
       ((POLY *) Object)->Sturm_Flag = TRUE;
     END_CASE

     CASE (WATER_LEVEL_TOKEN)
       if (!(Object->Type & WATER_LEVEL_OK_OBJECT))
         Error ("Cannot use WATER_LEVEL here");
       ((HEIGHT_FIELD *) Object)->bounding_box->bounds[0].y = 65536.0 * Parse_Float();
     END_CASE

     CASE (SMOOTH_TOKEN)
       if (!(Object->Type & SMOOTH_OK_OBJECT))
         Error ("Cannot use SMOOTH here");
       ((HEIGHT_FIELD *) Object)->Smoothed = TRUE;
     END_CASE

     CASE (NO_SHADOW_TOKEN)
       Object->No_Shadow_Flag = TRUE;
     END_CASE

     CASE (LIGHT_SOURCE_TOKEN)
       Error("Light source must be defined using new syntax");
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   if (Object->Bound != NULL)
     {
      Object->Bounds.Lower_Left = Object->Bound->Bounds.Lower_Left;
      Object->Bounds.Lengths    = Object->Bound->Bounds.Lengths;
     }
   Parse_End ();
  }

static
OBJECT *Parse_Sphere ()
  {
   SPHERE *Object;

   Parse_Begin ();

   if ( (Object = (SPHERE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Sphere();

   Parse_Vector(&(Object -> Center));   Parse_Comma();
   Object -> Radius = Parse_Float();
   Object -> Radius_Squared = Object -> Radius * Object -> Radius;
   Object -> Inverse_Radius = 1.0 / Object -> Radius;

   Make_Vector(&Object->Bounds.Lower_Left,
	       Object->Center.x - Object->Radius,
	       Object->Center.y - Object->Radius,
	       Object->Center.z - Object->Radius);
   Make_Vector(&Object->Bounds.Lengths,
	       2.0 * Object->Radius,
	       2.0 * Object->Radius,
	       2.0 * Object->Radius);

   Parse_Object_Mods ((OBJECT *) Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Plane ()
  {
   PLANE *Object;

   Parse_Begin ();

   if ( (Object = (PLANE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Plane();

   Parse_Vector(&(Object -> Normal_Vector));   Parse_Comma();
   VNormalize(Object->Normal_Vector, Object->Normal_Vector);
   Object->Distance = -Parse_Float();

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Height_Field ()
  {
   HEIGHT_FIELD *Object;
   VECTOR Local_Vector;
   IMAGE *Image;

   Parse_Begin ();

   if ( (Object = (HEIGHT_FIELD *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Height_Field();

   Image = Parse_Image (HF_FILE);
   Image->Use_Colour_Flag = FALSE;

   Object->bounding_box->bounds[0].x = 1.0;
   Object->bounding_box->bounds[0].y = 0.0;
   Object->bounding_box->bounds[0].z = 1.0;
   if (Image->File_Type == POT_FILE)
     {
      Object->bounding_box->bounds[1].x = Image -> width/2.0 - 2.0;
      Make_Vector(&Local_Vector,2.0/Image->width,1.0/65536.0,1.0/Image->height);
     }
   else
     {
      Object->bounding_box->bounds[1].x = Image -> width - 2.0;
      Make_Vector(&Local_Vector,1.0/(Image->width),1.0/65536.0,1.0/(Image->height));
     }
   Object->bounding_box->bounds[1].y = 65536.0;
   Object->bounding_box->bounds[1].z = Image -> height - 2.0;
   Compute_Scaling_Transform(Object->Trans,&Local_Vector);

   Parse_Object_Mods ((OBJECT *)Object);

   Find_Hf_Min_Max(Object, Image);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Triangle ()
  {
   TRIANGLE *Object;

   Parse_Begin ();

   if ( (Object = (TRIANGLE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Triangle();

   Parse_Vector (&Object->P1);    Parse_Comma();
   Parse_Vector (&Object->P2);    Parse_Comma();
   Parse_Vector (&Object->P3);
   if (!Compute_Triangle (Object,FALSE))
     fprintf (stdout, "Degenerate triangle on line %d.  Please remove.\n",
              Token.Token_Line_No+1);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Smooth_Triangle ()
  {
   SMOOTH_TRIANGLE *Object;
   short degen;                                                   /* LSK */
   DBL vlen;                                                      /* LSK */

   degen=FALSE;

   Parse_Begin ();

   if ( (Object = (SMOOTH_TRIANGLE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   Object = Create_Smooth_Triangle();

   Parse_Vector (&Object->P1);    Parse_Comma();
   Parse_Vector (&Object->N1);    Parse_Comma();

   VLength(vlen,Object->N1);                                     /* LSK */
   if (vlen<1E-09)                                               /* LSK */
     degen=TRUE;                                                 /* LSK */
   else                                                          /* LSK */
     VNormalize (Object->N1, Object->N1);

   Parse_Vector (&Object->P2);    Parse_Comma();
   Parse_Vector (&Object->N2);    Parse_Comma();

   VLength(vlen,Object->N2);                                     /* LSK */
   if (vlen<1E-09)                                               /* LSK */
     degen=TRUE;                                                 /* LSK */
   else                                                          /* LSK */
     VNormalize (Object->N2, Object->N2);

   Parse_Vector (&Object->P3);    Parse_Comma();
   Parse_Vector (&Object->N3);

   VLength(vlen,Object->N3);                                     /* LSK */
   if (vlen<1E-09)                                               /* LSK */
     degen=TRUE;                                                 /* LSK */
   else                                                          /* LSK */
     VNormalize (Object->N3, Object->N3);

   if (!degen) {                                                 /* LSK */
     degen=!Compute_Triangle ((TRIANGLE *) Object,TRUE);         /* LSK */
   }

   if (degen)                                                    /* LSK */
     fprintf (stdout, "Degenerate triangle on line %d.  Please remove.\n",
              Token.Token_Line_No+1);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
 }

static
OBJECT *Parse_Quadric ()
  {
   QUADRIC *Object;

   Parse_Begin ();

   if ( (Object = (QUADRIC *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Quadric();

   Parse_Vector(&(Object -> Square_Terms));     Parse_Comma();
   Parse_Vector(&(Object -> Mixed_Terms));      Parse_Comma();
   Parse_Vector(&(Object -> Terms));            Parse_Comma();
   Object -> Constant = Parse_Float();
   Object -> Non_Zero_Square_Term =
     !( (Object -> Square_Terms.x == 0.0)
     && (Object -> Square_Terms.y == 0.0)
     && (Object -> Square_Terms.z == 0.0)
     && (Object -> Mixed_Terms.x == 0.0)
     && (Object -> Mixed_Terms.y == 0.0)
     && (Object -> Mixed_Terms.z == 0.0));

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Box ()
  {
   BOX *Object;

   Parse_Begin ();

   if ( (Object = (BOX *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Box();

   Parse_Vector(&(Object->bounds[0]));     Parse_Comma();
   Parse_Vector(&(Object->bounds[1]));
   
   Object->Bounds.Lower_Left=Object->bounds[0];
   VSub(Object->Bounds.Lengths, Object->bounds[1],Object->bounds[0]);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Disc ()
  {
   DISC *Object;
   DBL tmpf;
   VECTOR lengths;

   Parse_Begin ();

   if ( (Object = (DISC *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Disc();

   Parse_Vector(&(Object->center)); Parse_Comma ();
   Parse_Vector(&(Object->normal)); Parse_Comma ();
   VNormalize(Object->normal, Object->normal);

   tmpf = Parse_Float(); Parse_Comma ();
   Object->oradius2 = tmpf * tmpf;

   EXPECT
     CASE_FLOAT
       tmpf = Parse_Float();
       Object->iradius2 = tmpf * tmpf;
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   /* Calculate info needed for ray-disc intersections */
   VDot(tmpf, Object->center, Object->normal);
   Object->d = -tmpf;

   /* Calculate the bounds */
   tmpf = sqrt(Object->oradius2);
   Make_Vector(&lengths, tmpf, tmpf, tmpf);
   VSub(Object->Bounds.Lower_Left, Object->center, lengths);
   VScale(Object->Bounds.Lengths, lengths, 2.0);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Cylinder ()
  {
   CONE *Object;

   Parse_Begin ();

   if ( (Object = (CONE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Cylinder();

   Parse_Vector(&(Object->apex));  Parse_Comma ();
   Parse_Vector(&(Object->base));  Parse_Comma ();
   Object->apex_radius = Parse_Float();
   Object->base_radius = Object->apex_radius;

   EXPECT
     CASE(OPEN_TOKEN)
       Object->closed = 0;
       EXIT
     END_CASE
     
     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   Compute_Cylinder_Data((OBJECT *)Object);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Cone ()
  {
   CONE *Object;

   Parse_Begin ();

   if ( (Object = (CONE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Cone();

   Parse_Vector(&(Object->apex));  Parse_Comma ();
   Object->apex_radius = Parse_Float();  Parse_Comma ();

   Parse_Vector(&(Object->base));  Parse_Comma ();
   Object->base_radius = Parse_Float();
   
   EXPECT
     CASE(OPEN_TOKEN)
       Object->closed = 0;
       EXIT
     END_CASE
     
     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   /* Compute run-time values for the cone */
   Compute_Cone_Data((OBJECT *)Object);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Blob ()
  {
   BLOB *Object;
   DBL threshold;
   int npoints;
   blobstackptr blob_components, blob_component;

   Parse_Begin ();

   if ( (Object = (BLOB *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Blob();

   blob_components = NULL;
   npoints = 0;
   threshold = 1.0;

   EXPECT
     CASE (THRESHOLD_TOKEN)
       threshold = Parse_Float();
     END_CASE

     CASE (COMPONENT_TOKEN)
       blob_component = (blobstackptr) malloc(sizeof(struct blob_list_struct));
       if (blob_component == NULL)
          MAError("blob component");
       blob_component->elem.coeffs[2] = Parse_Float(); Parse_Comma();
       blob_component->elem.radius2   = Parse_Float(); Parse_Comma();
       Parse_Vector(&blob_component->elem.pos);
       blob_component->next = blob_components;
       blob_components = blob_component;
       npoints++;
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   /* Finally, process the information */
   MakeBlob(Object, threshold, blob_components, npoints, 0);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Torus ()
  {
   POLY *Object;
   DBL iradius, oradius, *Coeffs;

   Parse_Begin ();

   if ( (Object = (POLY *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Poly(4);

   /* Read in the two radii */
   iradius = Parse_Float(); /* Big radius */
   Parse_Comma();
   oradius = Parse_Float(); /* Little radius */

   /* Build the coefficients of a torus lying in the x-z plane */
   Coeffs = Object->Coeffs;
   Coeffs[ 0] =  1.0;
   Coeffs[ 4] =  2.0;
   Coeffs[ 7] =  2.0;
   Coeffs[ 9] = -2.0 * (iradius * iradius + oradius * oradius);
   Coeffs[20] =  1.0;
   Coeffs[23] =  2.0;
   Coeffs[25] =  2.0 * (iradius * iradius - oradius * oradius);
   Coeffs[30] =  1.0;
   Coeffs[32] = -2.0 * (iradius * iradius + oradius * oradius);
   Coeffs[34] = (iradius * iradius - oradius * oradius) *
	        (iradius * iradius - oradius * oradius);

   Make_Vector(&Object->Bounds.Lower_Left, -(iradius + oradius),
	       -iradius, -(iradius + oradius))
   Make_Vector(&Object->Bounds.Lengths, 2.0 * (iradius + oradius),
	       2.0 * iradius, 2.0 * (iradius + oradius));

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Poly (order)
  int order;
  {
   POLY *Object;

   Parse_Begin ();

   if ( (Object = (POLY *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   if (order == 0)
     {
      order = (int)Parse_Float();      Parse_Comma();
      if (order < 2 || order > MAX_ORDER)
        Error("Order of poly is out of range");
     }

   Object = Create_Poly(order);

   Parse_Coeffs(Object->Order, &(Object->Coeffs[0]));

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Bicubic_Patch ()
  {
   BICUBIC_PATCH *Object;
   int i, j;

   Parse_Begin ();

   if ( (Object = (BICUBIC_PATCH *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Bicubic_Patch();

   EXPECT
     CASE_FLOAT
       Warn("Should use keywords for bicubic parameters.",1.5);
       Object->Patch_Type = (int)Parse_Float();
       if (Object->Patch_Type == 2 ||
           Object->Patch_Type == 3)
           Object->Flatness_Value = Parse_Float();
         else
           Object->Flatness_Value = 0.1;
       Object->U_Steps = (int)Parse_Float();
       Object->V_Steps = (int)Parse_Float();
       EXIT
     END_CASE       
       
     CASE (TYPE_TOKEN)
       Object->Patch_Type = (int)Parse_Float();
     END_CASE

     CASE (FLATNESS_TOKEN)
       Object->Flatness_Value = Parse_Float();
     END_CASE

     CASE (V_STEPS_TOKEN)
       Object->V_Steps = (int)Parse_Float();
     END_CASE

     CASE (U_STEPS_TOKEN)
       Object->U_Steps = (int)Parse_Float();
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   if (Object->Patch_Type > 1)
     {
      Object->Patch_Type = 1;
      Warn("Patch type no longer supported. Using type 1.",0.0);
     }

   if ((Object->Patch_Type < 0) || (Object->Patch_Type > MAX_PATCH_TYPE))
     Error("Undefined bicubic patch type");

   Parse_Comma();
   for (i=0;i<4;i++)
     for (j=0;j<4;j++)
       {
        Parse_Vector(&(Object -> Control_Points[i][j]));
        if (!((i==3)&&(j==3)))
          Parse_Comma();
       };
   Precompute_Patch_Values(Object); /* interpolated mesh coords */

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_CSG (CSG_Type)
  int CSG_Type;
  {
   CSG *Object;
   OBJECT *Local;
   int Object_Count = 0;

   Parse_Begin ();

   if ( (Object = (CSG *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   if (CSG_Type & CSG_UNION_TYPE)
     Object = Create_CSG_Union ();
   else
     if (CSG_Type & CSG_MERGE_TYPE)
       Object = Create_CSG_Merge ();
     else
       Object = Create_CSG_Intersection ();

   Object->Children = NULL;

   while ((Local = Parse_Object ()) != NULL)
     {
      if ((CSG_Type & CSG_INTERSECTION_TYPE) && (Local->Type & PATCH_OBJECT))
        Warn ("Patch objects not allowed in intersection",0.0);
      Object_Count++;
      if ((CSG_Type & CSG_DIFFERENCE_TYPE) && (Object_Count > 1))
        Invert_Object (Local);
      Object->Type |=  (Local->Type & CHILDREN_FLAGS);
      Local->Type |= IS_CHILD_OBJECT;
      Link(Local, &Local->Sibling, &Object->Children);
     };

   if ((Object_Count < 2) && (Language_Version >= 1.5))
     Warn ("Should have at least 2 objects in csg",1.5);
   Compute_CSG_Bounds((OBJECT *)Object);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }

static
OBJECT *Parse_Light_Source ()
  {
   VECTOR Local_Vector;
   LIGHT_SOURCE *Object;

   Parse_Begin ();

   if ( (Object = (LIGHT_SOURCE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Light_Source ();

   Parse_Vector(&Object->Center);

   GET (COLOUR_TOKEN)

   Parse_Colour (&Object->Colour);

   EXPECT
     CASE (LOOKS_LIKE_TOKEN)
       if (Object->Children != NULL)
         Error("Only one looks_like allowed per light_source");
       Parse_Begin ();
       Object->Type &= ~(int)PATCH_OBJECT;
       if ((Object->Children = Parse_Object ()) == NULL)
         Parse_Error_Str ("object");
       Translate_Object (Object->Children, &Object->Center);
       Parse_Object_Mods (Object->Children);
       Object->Children->No_Shadow_Flag = TRUE;
       Object->No_Shadow_Flag = TRUE;
       Object->Type |= (Object->Children->Type & CHILDREN_FLAGS);
     END_CASE

     CASE (SPOTLIGHT_TOKEN)
       Object->Light_Type = SPOT_SOURCE;
     END_CASE

     CASE (POINT_AT_TOKEN)
       if (Object->Light_Type == SPOT_SOURCE)
         Parse_Vector(&Object->Points_At);
       else
         Error("Spotlight param illegal in standard light source");
     END_CASE

     CASE (TIGHTNESS_TOKEN)
       if (Object->Light_Type == SPOT_SOURCE)
         Object->Coeff = Parse_Float();
       else
         Error("Spotlight param illegal in standard light source");
     END_CASE

     CASE (RADIUS_TOKEN)
       if (Object->Light_Type == SPOT_SOURCE)
         Object->Radius = cos(Parse_Float() * M_PI / 180.0);
       else
         Error("Spotlight param illegal in standard light source");
     END_CASE

     CASE (FALLOFF_TOKEN)
       if (Object->Light_Type == SPOT_SOURCE)
         Object->Falloff = cos(Parse_Float() * M_PI / 180.0);
       else
         Error("Spotlight param illegal in standard light source");
     END_CASE

     CASE (AREA_LIGHT_TOKEN)
       Object -> Area_Light = TRUE;
       Parse_Vector (&(Object -> Axis1)); Parse_Comma ();
       Parse_Vector (&(Object -> Axis2)); Parse_Comma ();
       Object -> Area_Size1 = (int)Parse_Float(); Parse_Comma ();
       Object -> Area_Size2 = (int)Parse_Float();
       Object -> Light_Grid = Create_Light_Grid (Object -> Area_Size1,
            Object -> Area_Size2);
     END_CASE

     CASE (JITTER_TOKEN)
       Object -> Jitter = TRUE;
     END_CASE

     CASE (TRACK_TOKEN)
       Object -> Track = TRUE;
     END_CASE

     CASE (ADAPTIVE_TOKEN)
       Object -> Adaptive_Level = (int)Parse_Float();
     END_CASE

     CASE (TRANSLATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Translate_Object ((OBJECT *)Object, &Local_Vector);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Rotate_Object ((OBJECT *)Object, &Local_Vector);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (&Local_Vector);
       Scale_Object ((OBJECT *)Object, &Local_Vector);
     END_CASE

     CASE (TRANSFORM_TOKEN)
       GET(TRANSFORM_ID_TOKEN)
       Transform_Object ((OBJECT *)Object, (TRANSFORM *)Token.Constant_Data);
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   Parse_End ();

   return ((OBJECT *)Object);
  }


static
OBJECT *Parse_Object ()
  {
   OBJECT *Object = NULL;

   EXPECT
     CASE (SPHERE_TOKEN)
       Object = Parse_Sphere ();
       EXIT
     END_CASE

     CASE (PLANE_TOKEN)
       Object = Parse_Plane ();
       EXIT
     END_CASE

     CASE (CONE_TOKEN)
       Object = Parse_Cone ();
       EXIT
     END_CASE

     CASE (CYLINDER_TOKEN)
       Object = Parse_Cylinder ();
       EXIT
     END_CASE

     CASE (DISC_TOKEN)
       Object = Parse_Disc ();
       EXIT
     END_CASE

     CASE (QUADRIC_TOKEN)
       Object = Parse_Quadric ();
       EXIT
     END_CASE

     CASE (CUBIC_TOKEN)
       Object = Parse_Poly (3);
       EXIT
     END_CASE

     CASE (QUARTIC_TOKEN)
       Object = Parse_Poly (4);
       EXIT
     END_CASE

     CASE (POLY_TOKEN)
       Object = Parse_Poly (0);
       EXIT
     END_CASE

     CASE (TORUS_TOKEN)
       Object = Parse_Torus ();
       EXIT
     END_CASE

     CASE (OBJECT_ID_TOKEN)
       Object = Copy_Object((OBJECT *) Token.Constant_Data);
       EXIT
     END_CASE

     CASE (UNION_TOKEN)
       Object = Parse_CSG (CSG_UNION_TYPE);
       EXIT
     END_CASE

     CASE (COMPOSITE_TOKEN)
       Warn("Use union instead of composite",1.5);
       Object = Parse_CSG (CSG_UNION_TYPE);
       EXIT
     END_CASE

     CASE (MERGE_TOKEN)
       Object = Parse_CSG (CSG_MERGE_TYPE);
       EXIT
     END_CASE

     CASE (INTERSECTION_TOKEN)
       Object = Parse_CSG (CSG_INTERSECTION_TYPE);
       EXIT
     END_CASE

     CASE (DIFFERENCE_TOKEN)
       Object = Parse_CSG (CSG_DIFFERENCE_TYPE+CSG_INTERSECTION_TYPE);
       EXIT
     END_CASE

     CASE (BICUBIC_PATCH_TOKEN)
       Object = Parse_Bicubic_Patch ();
       EXIT
     END_CASE

     CASE (TRIANGLE_TOKEN)
       Object = Parse_Triangle ();
       EXIT
     END_CASE

     CASE (SMOOTH_TRIANGLE_TOKEN)
       Object = Parse_Smooth_Triangle ();
       EXIT
     END_CASE

     CASE (HEIGHT_FIELD_TOKEN)
       Object = Parse_Height_Field ();
       EXIT
     END_CASE

     CASE (BOX_TOKEN)
       Object = Parse_Box ();
       EXIT
     END_CASE

     CASE (BLOB_TOKEN)
       Object = Parse_Blob ();
       EXIT
     END_CASE

     CASE (LIGHT_SOURCE_TOKEN)
       Object = Parse_Light_Source ();
       EXIT
     END_CASE

     CASE (OBJECT_TOKEN)
       Parse_Begin ();
       Object = Parse_Object ();
       if (!Object)
         Parse_Error_Str ("object");
       Parse_Object_Mods ((OBJECT *)Object);
       EXIT
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   return ((OBJECT *) Object);
  }

static void Parse_Fog ()
  {
   Parse_Begin ();

   EXPECT
     CASE (COLOUR_TOKEN)
       Parse_Colour (&Frame.Fog_Colour);
     END_CASE

     CASE (DISTANCE_TOKEN)
       Frame.Fog_Distance = Parse_Float ();
     END_CASE

     CASE_FLOAT
       Warn("Should use distance keyword.",1.5);
       Frame.Fog_Distance = Parse_Float ();
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT
   Parse_End ();
  }

static void Parse_Frame ()
  {
   OBJECT *Object;
   TEXTURE *Local_Texture;
   PIGMENT *Local_Pigment;
   TNORMAL *Local_Tnormal;
   FINISH  *Local_Finish;

   EXPECT
     CASE (FOG_TOKEN)
       Parse_Fog();
     END_CASE

     CASE (BACKGROUND_TOKEN)
       Parse_Begin();
       GET (COLOUR_TOKEN)
       Parse_Colour (&Frame.Background_Colour);
       Parse_End();
     END_CASE

     CASE (CAMERA_TOKEN)
       Parse_Camera (&Frame.Camera);
     END_CASE

     CASE (DECLARE_TOKEN)
       Parse_Declare ();
     END_CASE

     CASE (MAX_TRACE_LEVEL_TOKEN)
       Max_Trace_Level = Parse_Float ();
     END_CASE

     CASE (VERSION_TOKEN)
       Language_Version = Parse_Float ();
     END_CASE

     CASE (MAX_INTERSECTIONS)
       Max_Intersections = (int)Parse_Float ();
     END_CASE

     CASE (DEFAULT_TOKEN)
       Not_In_Default = FALSE;
       Parse_Begin();
       EXPECT
         CASE (TEXTURE_TOKEN)
           Local_Texture = Default_Texture;
           Default_Texture = Parse_Texture();
           if (Default_Texture->Type != PNF_TEXTURE)
             Error("Default texture cannot be material map or tiles");
           if (Default_Texture->Next_Layer != NULL)
             Error("Default texture cannot be layered");
           Destroy_Textures(Local_Texture);
         END_CASE

         CASE (PIGMENT_TOKEN)
           Local_Pigment = Copy_Pigment((Default_Texture->Pigment));
           Parse_Pigment (&Local_Pigment);
           Destroy_Pigment(Default_Texture->Pigment);
           Default_Texture->Pigment = Local_Pigment;
         END_CASE

         CASE (TNORMAL_TOKEN)
           Local_Tnormal = Copy_Tnormal((Default_Texture->Tnormal));
           Parse_Tnormal (&Local_Tnormal);
           Destroy_Tnormal(Default_Texture->Tnormal);
           Default_Texture->Tnormal = Local_Tnormal;
         END_CASE

         CASE (FINISH_TOKEN)
           Local_Finish = Copy_Finish((Default_Texture->Finish));
           Parse_Finish (&Local_Finish);
           Destroy_Finish(Default_Texture->Finish);
           Default_Texture->Finish = Local_Finish;
         END_CASE

         CASE (CAMERA_TOKEN)
           Parse_Camera (&Default_Camera);
         END_CASE

         OTHERWISE
           UNGET
           EXIT
         END_CASE
       END_EXPECT
       Parse_End();
       Not_In_Default = TRUE;
     END_CASE

     CASE (END_OF_FILE_TOKEN)
       EXIT
     END_CASE

     OTHERWISE
       UNGET
       Object = Parse_Object();
       if (Object == NULL)
         Parse_Error_Str ("object or directive");
       Post_Process (Object, NULL);
       Link_To_Frame (Object);
     END_CASE
   END_EXPECT
  }

static void Parse_Camera (Camera_Ptr)
  CAMERA **Camera_Ptr;
  {
   VECTOR Local_Vector, Temp_Vector;
   DBL Direction_Length, Up_Length, Right_Length, Handedness;
   CAMERA *New;

   Parse_Begin ();

   EXPECT
     CASE (CAMERA_ID_TOKEN)
       Destroy_Camera(*Camera_Ptr);
       *Camera_Ptr = Copy_Camera ((CAMERA *) Token.Constant_Data);
       EXIT
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   New = *Camera_Ptr;

   EXPECT
     CASE (LOCATION_TOKEN)
       Parse_Vector(&(New->Location));
     END_CASE

     CASE (DIRECTION_TOKEN)
       Parse_Vector(&(New->Direction));
     END_CASE

     CASE (UP_TOKEN)
       Parse_Vector(&(New->Up));
     END_CASE

     CASE (RIGHT_TOKEN)
       Parse_Vector(&(New->Right));
     END_CASE

     CASE (SKY_TOKEN)
       Parse_Vector(&(New->Sky));
     END_CASE

     CASE (LOOK_AT_TOKEN)
       VLength (Direction_Length, New->Direction);
       VLength (Up_Length,        New->Up);
       VLength (Right_Length,     New->Right);
       VCross  (Temp_Vector,      New->Direction, New->Up);
       VDot    (Handedness,       Temp_Vector,   New->Right);

       Parse_Vector (&New->Direction);

       VSub       (New->Direction, New->Direction, New->Location);
       VNormalize (New->Direction, New->Direction);
       VCross     (New->Right,     New->Direction, New->Sky);
       VNormalize (New->Right,     New->Right);
       VCross     (New->Up,        New->Right,     New->Direction);
       VScale     (New->Direction, New->Direction, Direction_Length);

       if (Handedness >= 0.0)
         VScale (New->Right, New->Right, Right_Length)
       else
         VScale (New->Right, New->Right, -Right_Length);

       VScale (New->Up, New->Up, Up_Length);
     END_CASE

     CASE (TRANSLATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Translate_Camera (New, &Local_Vector);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Rotate_Camera (New, &Local_Vector);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (&Local_Vector);
       Scale_Camera (New, &Local_Vector);
     END_CASE

     CASE (TRANSFORM_TOKEN)
       GET(TRANSFORM_ID_TOKEN)
       Transform_Camera (New, (TRANSFORM *)Token.Constant_Data);
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT
   Parse_End ();
  }

static
TRANSFORM *Parse_Transform ()
  {
   TRANSFORM *New, Local_Trans;
   VECTOR Local_Vector;

   Parse_Begin ();
   New = Create_Transform ();

   EXPECT
     CASE(TRANSFORM_ID_TOKEN)
       Compose_Transforms (New, (TRANSFORM *)Token.Constant_Data);
       EXIT
     END_CASE

     CASE (TRANSLATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Compute_Translation_Transform(&Local_Trans, &Local_Vector);
       Compose_Transforms (New, &Local_Trans);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (&Local_Vector);
       Compute_Rotation_Transform(&Local_Trans, &Local_Vector);
       Compose_Transforms (New, &Local_Trans);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (&Local_Vector);
       Compute_Scaling_Transform(&Local_Trans, &Local_Vector);
       Compose_Transforms (New, &Local_Trans);
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   Parse_End ();
   return (New);
  }

static void Parse_Declare ()
  {
  VECTOR Local_Vector;
  COLOUR *Local_Colour;
  PIGMENT *Local_Pigment;
  TNORMAL *Local_Tnormal;
  FINISH *Local_Finish;
  TEXTURE *Local_Texture;
  COLOUR_MAP *Local_Colour_Map;
  TRANSFORM *Local_Trans;
  OBJECT *Local_Object;
  CAMERA *Local_Camera;

  struct Constant_Struct *Constant_Ptr;

  EXPECT
    CASE (IDENTIFIER_TOKEN)
      if (++Number_Of_Constants >= MAX_CONSTANTS)
        Error ("Too many constants \"DECLARED\"");
      else
        Constant_Ptr = &(Constants[Number_Of_Constants]);
      EXIT
    END_CASE

    CASE4 (COLOUR_ID_TOKEN, VECTOR_ID_TOKEN, FLOAT_ID_TOKEN, PIGMENT_ID_TOKEN)
    CASE4 (TNORMAL_ID_TOKEN, FINISH_ID_TOKEN, TEXTURE_ID_TOKEN, OBJECT_ID_TOKEN)
    CASE3 (COLOUR_MAP_ID_TOKEN, TRANSFORM_ID_TOKEN, CAMERA_ID_TOKEN)
      Constant_Ptr = &(Constants[Token.Constant_Index]);
      EXIT
    END_CASE

    OTHERWISE
      Parse_Error(IDENTIFIER_TOKEN);
    END_CASE
  END_EXPECT

  Previous = Token.Token_Id;

  GET (EQUALS_TOKEN);

  EXPECT
    CASE (COLOUR_TOKEN)
      if (Test_Redefine(COLOUR_ID_TOKEN))
        Destroy_Colour((COLOUR *)Constant_Ptr->Constant_Data);
      Local_Colour = Create_Colour();
      Parse_Colour (Local_Colour);
      Constant_Ptr -> Constant_Data = (char *) Local_Colour;
      Constant_Ptr -> Constant_Type = COLOUR_CONSTANT;
      EXIT
    END_CASE

    CASE_VECTOR
      Have_Vector = FALSE;
      Parse_Vector_Float (&Local_Vector);
      if (Have_Vector)
        {
         if (Test_Redefine(VECTOR_ID_TOKEN))
           Destroy_Vector((VECTOR *)Constant_Ptr->Constant_Data);
         Constant_Ptr -> Constant_Type = VECTOR_CONSTANT;
         Constant_Ptr -> Constant_Data = (char *) Create_Vector();
         *((VECTOR *)Constant_Ptr -> Constant_Data) = Local_Vector;
        }
      else
        {
         if (Test_Redefine(FLOAT_ID_TOKEN))
           Destroy_Float((DBL *)Constant_Ptr->Constant_Data);
         Constant_Ptr -> Constant_Type = FLOAT_CONSTANT;
         Constant_Ptr -> Constant_Data = (char *) Create_Float();
         *((DBL *) Constant_Ptr -> Constant_Data) = Local_Vector.x;
        }
      EXIT
    END_CASE

    CASE (PIGMENT_TOKEN)
      if (Test_Redefine(PIGMENT_ID_TOKEN))
        Destroy_Pigment((PIGMENT *)Constant_Ptr->Constant_Data);
      Local_Pigment = Copy_Pigment((Default_Texture->Pigment));
      Parse_Pigment (&Local_Pigment);
      Constant_Ptr -> Constant_Type = PIGMENT_CONSTANT;
      Constant_Ptr -> Constant_Data = (char *)Local_Pigment;
      EXIT
    END_CASE

    CASE (TNORMAL_TOKEN)
      if (Test_Redefine(TNORMAL_ID_TOKEN))
        Destroy_Tnormal((TNORMAL *)Constant_Ptr->Constant_Data);
      Local_Tnormal = Copy_Tnormal((Default_Texture->Tnormal));
      Parse_Tnormal (&Local_Tnormal);
      Constant_Ptr -> Constant_Type = TNORMAL_CONSTANT;
      Constant_Ptr -> Constant_Data = (char *) Local_Tnormal;
      EXIT
    END_CASE

    CASE (FINISH_TOKEN)
      if (Test_Redefine(FINISH_ID_TOKEN))
        Destroy_Finish((FINISH *)Constant_Ptr->Constant_Data);
      Local_Finish = Copy_Finish((Default_Texture->Finish));
      Parse_Finish (&Local_Finish);
      Constant_Ptr -> Constant_Type = FINISH_CONSTANT;
      Constant_Ptr -> Constant_Data = (char *) Local_Finish;
      EXIT
    END_CASE

    CASE (CAMERA_TOKEN)
      if (Test_Redefine(CAMERA_ID_TOKEN))
        Destroy_Camera((CAMERA *)Constant_Ptr->Constant_Data);
      Local_Camera = Copy_Camera(Default_Camera);
      Parse_Camera (&Local_Camera);
      Constant_Ptr -> Constant_Type = CAMERA_CONSTANT;
      Constant_Ptr -> Constant_Data = (char *) Local_Camera;
      EXIT
    END_CASE

    CASE (TEXTURE_TOKEN)
      if (Test_Redefine(TEXTURE_ID_TOKEN))
        Destroy_Textures((TEXTURE *)Constant_Ptr->Constant_Data);
      Local_Texture = Parse_Texture ();
      Constant_Ptr -> Constant_Type = TEXTURE_CONSTANT;
      Constant_Ptr -> Constant_Data = NULL;
      Link_Textures((TEXTURE **) &Constant_Ptr->Constant_Data, Local_Texture);
      EXPECT
        CASE (TEXTURE_TOKEN)
          Local_Texture = Parse_Texture ();
          Link_Textures((TEXTURE **) &Constant_Ptr->Constant_Data, Local_Texture);
        END_CASE

        OTHERWISE
          UNGET
          EXIT
        END_CASE
      END_EXPECT
      EXIT
    END_CASE

    CASE (COLOUR_MAP_TOKEN)
      if (Test_Redefine(COLOUR_MAP_ID_TOKEN))
        Destroy_Colour_Map((COLOUR_MAP *)Constant_Ptr->Constant_Data);
      Local_Colour_Map = Parse_Colour_Map ();
      Constant_Ptr -> Constant_Type = COLOUR_MAP_CONSTANT;
      Constant_Ptr -> Constant_Data = (char *) Local_Colour_Map;
      EXIT
    END_CASE

    CASE (TRANSFORM_TOKEN)
      if (Test_Redefine(TRANSFORM_ID_TOKEN))
        Destroy_Transform((TRANSFORM *)Constant_Ptr->Constant_Data);
      Local_Trans = Parse_Transform ();
      Constant_Ptr -> Constant_Type = TRANSFORM_CONSTANT;
      Constant_Ptr -> Constant_Data = (char *) Local_Trans;
      EXIT
    END_CASE

    OTHERWISE
      UNGET
      if (Test_Redefine(OBJECT_ID_TOKEN))
        Destroy_Object((OBJECT *)Constant_Ptr->Constant_Data);
      Local_Object = Parse_Object ();
      Constant_Ptr -> Constant_Type = OBJECT_CONSTANT;
      Constant_Ptr -> Constant_Data = (char *) Local_Object;
      EXIT
    END_CASE

  END_EXPECT
  }

static void Link (New_Object, Field, Old_Object_List)
  OBJECT *New_Object, **Field, **Old_Object_List;
  {
  *Field = *Old_Object_List;
  *Old_Object_List = New_Object;
  }

static void Link_Textures (Old_Textures, New_Textures)
  TEXTURE **Old_Textures;
  TEXTURE  *New_Textures;
  {
   TEXTURE *Layer;

   for (Layer = New_Textures ;
        Layer->Next_Layer != NULL ;
        Layer = Layer->Next_Layer)
     {}
        Layer->Next_Layer = *Old_Textures;
        *Old_Textures = New_Textures;
  }

static
char *Get_Token_String (Token_Id)
  TOKEN Token_Id;
  {
  register int i;

  for (i = 0 ; i < LAST_TOKEN ; i++)
     if (Reserved_Words[i].Token_Number == Token_Id)
        return (Reserved_Words[i].Token_Name);
  return ("");
  }

static
void Where_Error ()
  {
  fprintf (stderr, "\nError in file %s line %d\n", Token.Filename,
                                                 Token.Token_Line_No+1);
  }

static int Test_Redefine(a)
  int a;
  {
  char *old, *new;

  if (Previous == IDENTIFIER_TOKEN)
    return (FALSE);
  if (Previous != a)
    {old = Get_Token_String (Previous);
     new = Get_Token_String (a);
     Where_Error ();
     fprintf (stderr, "Attempted to redefine %s as %s", old, new);
     exit (1);
    }
  return (TRUE);
  }

void Parse_Error (Token_Id)
  TOKEN Token_Id;
  {
  char *expected;

  expected = Get_Token_String (Token_Id);
  Parse_Error_Str(expected);
  }

void Parse_Error_Str (str)
  char *str;
  {
   Where_Error ();
   fprintf (stderr, "%s expected but", str);
   Found_Instead ();
   exit (1);
  }

static void Found_Instead ()
  {
  char *found;

  if (Token.Token_Id == IDENTIFIER_TOKEN)
    fprintf (stderr,
      " undeclared identifier '%s' found instead.\n", Token.Token_String);
  else
   {
    found = Get_Token_String (Token.Token_Id);
    fprintf (stderr, " %s found instead.\n", found);
   }
  }
/*
static void Parse_Warn (Token_Id)
  TOKEN Token_Id;
  {
  char *expected;

  fprintf (stderr, "\nWarning in file %s line %d\n", Token.Filename,
                                                   Token.Token_Line_No+1);
  expected = Get_Token_String (Token_Id);
  fprintf (stderr, "%s expected but", expected);
  Found_Instead ();
  }
*/
static void Warn_State (Token_Id,Type)
  TOKEN Token_Id, Type;
  {
  char *found;
  char *should;

  if (Language_Version < 1.5)
     return;

  fprintf (stderr, "\nWarning in file %s line %d\n", Token.Filename,
                                                   Token.Token_Line_No+1);
  found = Get_Token_String (Token_Id);
  should = Get_Token_String (Type);
  fprintf (stderr, "Found %s that should be in %s statement", found, should);
  }

void Warn (str,Level)
  char *str;
  DBL Level;
  {
  if (Language_Version < Level)
    return;
    
  fprintf (stdout, "\nWarning in file %s line %d\n", Token.Filename,
                                                   Token.Token_Line_No+1);
  fputs (str, stdout);
  }

void Error (str)
  char *str;
  {
  Where_Error ();
  fputs (str, stderr);
  exit (1);
  }

void MAError (str)
  char *str;
  {
  Where_Error ();
  fprintf (stderr, "Out of memory.  Cannot allocate %s.\n",str);
  exit (1);
  }

/* Write a token out to the token file */

void Write_Token (Token_Id, Data_File)
  TOKEN Token_Id;
  DATA_FILE *Data_File;

  {
   Token.Token_Id = Token_Id;
   Token.Token_Line_No = Data_File->Line_Number;
   Token.Filename = Data_File->Filename;
   Token.Token_String = String;
   Token.Constant_Data = NULL;
   Token.Constant_Index = (int) Token.Token_Id - (int) LAST_TOKEN;

   if (Token.Constant_Index >= 0)
     {if (Token.Constant_Index <= Number_Of_Constants)
        {Token.Constant_Data = Constants[Token.Constant_Index].Constant_Data;
         switch (Constants[Token.Constant_Index].Constant_Type)
           {CASEID(COLOUR_CONSTANT,     COLOUR_ID_TOKEN)
            CASEID(VECTOR_CONSTANT,     VECTOR_ID_TOKEN)
            CASEID(FLOAT_CONSTANT,      FLOAT_ID_TOKEN)
            CASEID(PIGMENT_CONSTANT,    PIGMENT_ID_TOKEN)
            CASEID(TNORMAL_CONSTANT,    TNORMAL_ID_TOKEN)
            CASEID(FINISH_CONSTANT,     FINISH_ID_TOKEN)
            CASEID(TEXTURE_CONSTANT,    TEXTURE_ID_TOKEN)
            CASEID(OBJECT_CONSTANT,     OBJECT_ID_TOKEN)
            CASEID(COLOUR_MAP_CONSTANT, COLOUR_MAP_ID_TOKEN)
            CASEID(TRANSFORM_CONSTANT,  TRANSFORM_ID_TOKEN)
            CASEID(CAMERA_CONSTANT,     CAMERA_ID_TOKEN)
           }
        }
      else Token.Token_Id = IDENTIFIER_TOKEN;
     }
  }

static void Post_Process (Object,Parent)
  OBJECT *Object, *Parent;
  {
   OBJECT *Sib;

   if (Object == NULL)
     return;

   if (Parent != NULL)
     {
      if (Object->Texture == NULL)
        Object->Texture = Parent->Texture;
/*
      else
        if (Parent->Texture != NULL)
          {Local_Texture = Copy_Textures (Parent->Texture);
           Link_Textures (&(Object->Texture), Local_Texture);
          }
*/ /* Removed for backward compat with 1.0.  May put back in. CEY 12/92 */
      Object->No_Shadow_Flag |= Parent->No_Shadow_Flag;
     }
     
   if (     (Object->Texture == NULL) 
        && !(Object->Type & TEXTURED_OBJECT) 
        && !(Object->Type & LIGHT_SOURCE_OBJECT))
     Object->Texture = Copy_Textures(Default_Texture);

   if (Object->Type & COMPOUND_OBJECT)
     {
      if (Object->Type & LIGHT_SOURCE_OBJECT)
        {
         ((LIGHT_SOURCE *)Object)->Next_Light_Source = Frame.Light_Sources;
         Frame.Light_Sources = (LIGHT_SOURCE *)Object;
        }
      for (Sib = ((CSG *)Object)->Children;
           Sib != NULL;
           Sib = Sib->Sibling)
        Post_Process(Sib, Object);
     }
   else
     if (Object->Texture == NULL)
       Object->Texture = Copy_Textures(Default_Texture);
   Post_Textures (Object->Texture);
   if ((Object->Type & WATER_LEVEL_OK_OBJECT) &&
       (Object->Type & IS_CHILD_OBJECT))
     Object->Methods = &Csg_Height_Field_Methods;
  }

static void Destroy_Constants ()
  {
   int i;
   char *Ptr;

   for (i=1; i <= Number_Of_Constants; i++)
     {
      Ptr = Constants[i].Constant_Data;
      switch (Constants[i].Constant_Type)
        {
         case COLOUR_CONSTANT:
           Destroy_Colour((COLOUR *)Ptr);
           break;
         case VECTOR_CONSTANT:
           Destroy_Vector((VECTOR *)Ptr);
           break;
         case FLOAT_CONSTANT:
           Destroy_Float((DBL *)Ptr);
           break;
         case PIGMENT_CONSTANT:
           Destroy_Pigment((PIGMENT *)Ptr);
           break;
         case TNORMAL_CONSTANT:
           Destroy_Tnormal((TNORMAL *)Ptr);
           break;
         case FINISH_CONSTANT:
           Destroy_Finish((FINISH *)Ptr);
           break;
         case TEXTURE_CONSTANT:
           Destroy_Textures((TEXTURE *)Ptr);
           break;
         case OBJECT_CONSTANT:
           Destroy_Object((OBJECT *)Ptr);
           break;
         case COLOUR_MAP_CONSTANT:
           Destroy_Colour_Map((COLOUR_MAP *)Ptr);
           break;
         case TRANSFORM_CONSTANT:
           Destroy_Transform((TRANSFORM *)Ptr);
           break;
         case CAMERA_CONSTANT:
           Destroy_Camera((CAMERA *)Ptr);
           break;
        }
     }
  }

static void Link_To_Frame (Object)
 OBJECT *Object;
 {
  OBJECT *This_Sib, *Next_Sib;
  
  if ((Object->Methods != &CSG_Union_Methods) ||
      (Object->Bound != NULL) ||
      (Object->Clip != NULL) ||
      (!Use_Slabs))
    {
     Link(Object, &(Object -> Sibling), &(Frame.Objects));
     return;
    }
  
  for (This_Sib = ((CSG *)Object)->Children;
       This_Sib != NULL;
       This_Sib = Next_Sib)
       {
        Next_Sib = This_Sib->Sibling; /*L2F changes Sibling so save it */
        Link_To_Frame (This_Sib);
       }
  Object->Texture = NULL;
  Object->Sibling = NULL;
  ((CSG *)Object)->Children = NULL;
  Destroy_Object (Object);
 }
