/****************************************************************************
*                   frame.h
*
*  This header file is included by all C modules in POV-Ray. It defines all
*  globally-accessible types and constants.
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

/* Generic header for all modules */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "config.h"


/* These are used by POVRAY.C and the machine specific modules */

#define POV_RAY_VERSION "2.0"

/* This message is for the personal distribution release. */
#define DISTRIBUTION_MESSAGE_1 "This is an unofficial version compiled by:"
#define DISTRIBUTION_MESSAGE_2 " C.S. FU & ported for Synergy Project!"
#define DISTRIBUTION_MESSAGE_3 "The POV-Ray Team is not responsible for supporting this version."


#ifndef READ_ENV_VAR_BEFORE 
#define READ_ENV_VAR_BEFORE 
#endif
#ifndef READ_ENV_VAR_AFTER
#define READ_ENV_VAR_AFTER if ((Option_String_Ptr = getenv("POVRAYOPT")) != NULL) read_options(Option_String_Ptr);   
#endif

#ifndef CONFIG_MATH
#define CONFIG_MATH
#endif

#ifndef EPSILON
#define EPSILON 1.0e-10
#endif

#ifndef FILE_NAME_LENGTH
#define FILE_NAME_LENGTH 150
#endif

#ifndef HUGE_VAL
#define HUGE_VAL 1.0e+17
#endif

#ifndef BOUND_HUGE
#define BOUND_HUGE 1.0e30
#endif

#ifndef DBL_FORMAT_STRING
#define DBL_FORMAT_STRING "%lf"
#endif

#ifndef DEFAULT_OUTPUT_FORMAT
#define DEFAULT_OUTPUT_FORMAT	'd'
#endif

#ifndef RED_RAW_FILE_EXTENSION
#define RED_RAW_FILE_EXTENSION ".red"
#endif

#ifndef GREEN_RAW_FILE_EXTENSION
#define GREEN_RAW_FILE_EXTENSION ".grn"
#endif

#ifndef BLUE_RAW_FILE_EXTENSION
#define BLUE_RAW_FILE_EXTENSION ".blu"
#endif

#ifndef FILENAME_SEPARATOR
#define FILENAME_SEPARATOR "/"
#endif

/* 0==yes 1==no 2==opt */
#ifndef CASE_SENSITIVE_DEFAULT
#define CASE_SENSITIVE_DEFAULT 0
#endif

#ifndef READ_FILE_STRING
#define READ_FILE_STRING "rb"
#endif

#ifndef WRITE_FILE_STRING
#define WRITE_FILE_STRING "wb"
#endif

#ifndef APPEND_FILE_STRING
#define APPEND_FILE_STRING "ab"
#endif

#ifndef NORMAL
#define NORMAL '0'
#endif

#ifndef GREY
#define GREY   'G'
#endif

#ifndef START_TIME
#define START_TIME time(&tstart);     
#endif

#ifndef STOP_TIME
#define STOP_TIME  time(&tstop);
#endif

#ifndef TIME_ELAPSED
#define TIME_ELAPSED difftime (tstop, tstart);
#endif

#ifndef STARTUP_POVRAY
#define STARTUP_POVRAY
#endif

#ifndef PRINT_OTHER_CREDITS
#define PRINT_OTHER_CREDITS
#endif

#ifndef TEST_ABORT
#define TEST_ABORT
#endif

#ifndef FINISH_POVRAY
#define FINISH_POVRAY
#endif

#ifndef COOPERATE
#define COOPERATE 
#endif

#ifndef DBL
#define DBL double
#endif

#ifndef ACOS
#define ACOS acos
#endif

#ifndef SQRT
#define SQRT sqrt
#endif

#ifndef POW
#define POW pow
#endif

#ifndef COS
#define COS cos
#endif

#ifndef SIN
#define SIN sin
#endif

#ifndef labs
#define labs(x) (long) ((x<0)?-x:x)
#endif

#ifndef max
#define max(x,y) ((x<y)?y:x)
#endif

#ifndef STRLN
#define STRLN(x) x
#endif

#ifndef PARAMS
#define PARAMS(x) x
#endif

#ifndef ANSIFUNC
#define ANSIFUNC 1
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef IFF_SWITCH_CAST
#define IFF_SWITCH_CAST (int)
#endif

#ifndef PRINT_CREDITS
#define PRINT_CREDITS print_credits();
#endif

#ifndef PRINT_STATS
#define PRINT_STATS print_stats();
#endif

#ifndef MAX_CONSTANTS
#define MAX_CONSTANTS 1000
#endif

#ifndef WAIT_FOR_KEYPRESS
#define WAIT_FOR_KEYPRESS
#endif

#ifndef CDECL
#define CDECL
#endif

#ifndef MAX_BUFSIZE
#define MAX_BUFSIZE INT_MAX
#endif

/* If compiler version is undefined, then make it 'u' for unknown */
#ifndef COMPILER_VER
#define COMPILER_VER ".u"
#endif

/* These values determine the minumum and maximum distances
   that qualify as ray-object intersections */
#define Small_Tolerance 0.001
#define Max_Distance 1.0e7

typedef struct istk_entry INTERSECTION;
typedef struct Vector_Struct VECTOR;
typedef DBL MATRIX [4][4];
typedef struct Bounding_Box_Struct BBOX;
typedef struct Colour_Struct COLOUR;
typedef struct Colour_Map_Entry COLOUR_MAP_ENTRY;
typedef struct Colour_Map_Struct COLOUR_MAP;
typedef struct Transform_Struct TRANSFORM;
typedef struct Image_Struct IMAGE;
typedef struct Texture_Struct TEXTURE;
typedef struct Material_Texture_Struct MATERIAL;
typedef struct Tiles_Texture_Struct TILES;
typedef struct Pattern_Struct TPATTERN;
typedef struct Pigment_Struct PIGMENT;
typedef struct Tnormal_Struct TNORMAL;
typedef struct Finish_Struct FINISH;
typedef struct Method_Struct METHODS;
typedef struct Camera_Struct CAMERA;
typedef struct Object_Struct OBJECT;
typedef struct Composite_Struct COMPOSITE;
typedef struct Sphere_Struct SPHERE;
typedef struct Quadric_Struct QUADRIC;
typedef struct Poly_Struct POLY;
typedef struct Disc_Struct DISC;
typedef struct Cone_Struct CYLINDER;
typedef struct Cone_Struct CONE;
typedef struct Light_Source_Struct LIGHT_SOURCE;
typedef struct Bicubic_Patch_Struct BICUBIC_PATCH;
typedef struct Triangle_Struct TRIANGLE;
typedef struct Smooth_Triangle_Struct SMOOTH_TRIANGLE;
typedef struct Plane_Struct PLANE;
typedef struct CSG_Struct CSG;
typedef struct Box_Struct BOX;
typedef struct Blob_Struct BLOB;
typedef struct Ray_Struct RAY;
typedef struct Frame_Struct FRAME;
typedef struct istack_struct ISTACK;
typedef int TOKEN;
typedef int CONSTANT;
typedef struct Chunk_Header_Struct CHUNK_HEADER;
typedef struct Data_File_Struct DATA_FILE;
typedef struct complex_block complex;
typedef struct Height_Field_Struct HEIGHT_FIELD;
typedef short WORD;

struct Vector_Struct
  {
   DBL x, y, z;
  };
#define Destroy_Vector(x) if ((x)!=NULL) free(x)
#define Destroy_Float(x) if ((x)!=NULL) free(x)

struct Colour_Struct
  {
   DBL Red, Green, Blue, Filter;
  };

#define Make_Colour(c,r,g,b) {(c)->Red=(r);(c)->Green=(g);(c)->Blue=(b);(c)->Filter=0.0;}
#define Make_ColourA(c,r,g,b,a) {(c)->Red=(r);(c)->Green=(g);(c)->Blue=(b);(c)->Filter=(a);}
#define Make_Vector(v,a,b,c) { (v)->x=(a);(v)->y=(b);(v)->z=(c); }
#define Destroy_Colour(x) if ((x)!=NULL) free(x)
#define MAX_COLOUR_MAP_ENTRIES 40

struct Colour_Map_Entry
  {
   DBL value;
   COLOUR Colour;
  };

struct Colour_Map_Struct
  {
   int Number_Of_Entries, Transparency_Flag, Users;
   COLOUR_MAP_ENTRY *Colour_Map_Entries;
  };

struct Transform_Struct
  {
   MATRIX matrix;
   MATRIX inverse;
  };

#define Destroy_Transform(x) if ((x)!=NULL) free(x)

/* Types for reading IFF files. */
typedef struct {unsigned short Red, Green, Blue, Filter;} IMAGE_COLOUR;

struct Image_Line
  {
   unsigned char *red, *green, *blue;
  };

typedef struct Image_Line IMAGE_LINE;

/* Legal image attributes */
#define GIF_FILE   1
#define POT_FILE   2
#define DUMP_FILE  4
#define IFF_FILE   8
#define TGA_FILE  16
#define GRAD_FILE 32

#define IMAGE_FILE    GIF_FILE+DUMP_FILE+IFF_FILE+GRAD_FILE+TGA_FILE
#define NORMAL_FILE   GIF_FILE+DUMP_FILE+IFF_FILE+GRAD_FILE+TGA_FILE
#define MATERIAL_FILE GIF_FILE+DUMP_FILE+IFF_FILE+GRAD_FILE+TGA_FILE
#define HF_FILE       GIF_FILE+POT_FILE+TGA_FILE

struct Image_Struct
  {
   int Map_Type;
   int File_Type;
   int Interpolation_Type;
   short Once_Flag;
   short Use_Colour_Flag;
   VECTOR Gradient;
   DBL width, height;
   int iwidth, iheight;
   short Colour_Map_Size;
   IMAGE_COLOUR *Colour_Map;
   union 
    {
     IMAGE_LINE *rgb_lines;
     unsigned char **map_lines;
    } data;  
  };

/* Texture types */
#define PNF_TEXTURE     0
#define TILE_TEXTURE    1
#define MAT_TEXTURE     2

/* Image/Bump Map projection types */
#define PLANAR_MAP      0
#define SPHERICAL_MAP   1
#define CYLINDRICAL_MAP 2
#define PARABOLIC_MAP   3
#define HYPERBOLIC_MAP  4
#define TORUS_MAP       5
#define PIRIFORM_MAP    6
#define OLD_MAP         7

/* Bit map interpolation types */
#define NO_INTERPOLATION 0
#define NEAREST_NEIGHBOR 1
#define BILINEAR         2
#define CUBIC_SPLINE     3
#define NORMALIZED_DIST  4

/* Coloration pigment list */
#define NO_PIGMENT               0
#define COLOUR_PIGMENT           1
#define BOZO_PIGMENT             2
#define MARBLE_PIGMENT           3
#define WOOD_PIGMENT             4
#define CHECKER_PIGMENT          5
#define SPOTTED_PIGMENT          6
#define AGATE_PIGMENT            7
#define GRANITE_PIGMENT          8
#define GRADIENT_PIGMENT         9
#define IMAGE_MAP_PIGMENT       10
#define PAINTED1_PIGMENT        11 
#define PAINTED2_PIGMENT        12 
#define PAINTED3_PIGMENT        13 
#define ONION_PIGMENT           14 
#define LEOPARD_PIGMENT         15 
#define BRICK_PIGMENT           16
#define MANDEL_PIGMENT          17
#define HEXAGON_PIGMENT         18
#define RADIAL_PIGMENT          19


/* Normal perturbation (bumpy) texture list  */
#define NO_NORMAL  0
#define WAVES      1
#define RIPPLES    2
#define WRINKLES   3
#define BUMPS      4
#define DENTS      5
#define BUMPY1     6
#define BUMPY2     7
#define BUMPY3     8
#define BUMP_MAP   9

/* Pattern flags */
#define NO_FLAGS      0
#define HAS_FILTER    1
#define FULL_BLOCKING 2
#define HAS_TURB      4
#define POST_DONE     8

#define TPATTERN_FIELDS int Type, Octaves, Flags; VECTOR Turbulence;  \
  DBL omega, lambda, Frequency, Phase; IMAGE *Image; TRANSFORM *Trans;

#define INIT_TPATTERN_FIELDS(p,t) p->Type=t; p->Octaves=6; p->Image=NULL; \
 p->Frequency=1.0; p->Phase=0.0;\
 p->Trans=NULL; p->Flags=NO_FLAGS; p->omega=0.5;p->lambda=2.0; \
 Make_Vector(&(p->Turbulence),0.0,0.0,0.0);

/* This is an abstract structure that is never actually used.
   Pigment and Tnormal are descendents of this primative type */

struct Pattern_Struct
  {
   TPATTERN_FIELDS
  };

struct Pigment_Struct
  {
   TPATTERN_FIELDS
   COLOUR *Colour1;
   COLOUR Quick_Colour;
   COLOUR_MAP *Colour_Map;
   VECTOR Colour_Gradient;
   DBL Mortar, Agate_Turb_Scale;
   int Iterations; /* mhs 10/92 for fractal textures */
  };

struct Tnormal_Struct
  {
   TPATTERN_FIELDS
   DBL Amount;
  };

struct Finish_Struct
  {
   DBL Reflection, Ambient, Diffuse, Brilliance, Index_Of_Refraction;
   DBL Refraction, Specular, Roughness, Phong, Phong_Size;
   DBL Crand;
   short Metallic_Flag;
  };

#define Destroy_Finish(x) if ((x)!=NULL) free(x)

#define TEXTURE_FIELDS unsigned char Type,Flags; TEXTURE *Next_Material; \
 TEXTURE *Next_Layer;
#define TRANS_TEXTURE_FIELDS TEXTURE_FIELDS TRANSFORM *Trans;

struct Texture_Struct
  {
   TEXTURE_FIELDS
   PIGMENT *Pigment;
   TNORMAL *Tnormal;
   FINISH *Finish;
  };

struct Tiles_Texture_Struct
  {
   TRANS_TEXTURE_FIELDS
   TEXTURE *Tile1;
   TEXTURE *Tile2;
  };

struct Material_Texture_Struct
  {
   TRANS_TEXTURE_FIELDS
   TEXTURE *Materials;
   IMAGE *Image;
   int Num_Of_Mats;
  };

/* Object types */
#define BASIC_OBJECT            0
#define PATCH_OBJECT            1   /* Has no inside, no inverse */
#define TEXTURED_OBJECT         2   /* Has texture, possibly in children */

#define CHILDREN_FLAGS (PATCH_OBJECT+TEXTURED_OBJECT)
                                    /* Reverse inherited flags */

#define COMPOUND_OBJECT         4   /* Has children field */
#define STURM_OK_OBJECT         8   /* STRUM legal */
#define WATER_LEVEL_OK_OBJECT  16   /* WATER_LEVEL legal */
#define LIGHT_SOURCE_OBJECT    32   /* link me in frame.light_sources */
#define BOUNDING_OBJECT        64   /* This is a holder for bounded object */
#define SMOOTH_OK_OBJECT      128   /* SMOOTH legal */
#define IS_CHILD_OBJECT       256   /* Object is inside a COMPOUND */

#define COMPOSITE_OBJECT       (BOUNDING_OBJECT)
#define SPHERE_OBJECT          (BASIC_OBJECT)
#define PLANE_OBJECT           (BASIC_OBJECT)
#define QUADRIC_OBJECT         (BASIC_OBJECT)
#define BOX_OBJECT             (BASIC_OBJECT)
#define CONE_OBJECT            (BASIC_OBJECT)
#define DISC_OBJECT            (BASIC_OBJECT)
#define HEIGHT_FIELD_OBJECT    (BASIC_OBJECT+WATER_LEVEL_OK_OBJECT+SMOOTH_OK_OBJECT)
#define TRIANGLE_OBJECT        (PATCH_OBJECT)
#define SMOOTH_TRIANGLE_OBJECT (PATCH_OBJECT)
#define BICUBIC_PATCH_OBJECT   (PATCH_OBJECT)
#define UNION_OBJECT           (COMPOUND_OBJECT)
#define MERGE_OBJECT           (COMPOUND_OBJECT)
#define INTERSECTION_OBJECT    (COMPOUND_OBJECT)
#define CUBIC_OBJECT           (STURM_OK_OBJECT)
#define QUARTIC_OBJECT         (STURM_OK_OBJECT)
#define POLY_OBJECT            (STURM_OK_OBJECT)
#define BLOB_OBJECT            (STURM_OK_OBJECT)
#define LIGHT_OBJECT           (COMPOUND_OBJECT+PATCH_OBJECT+LIGHT_SOURCE_OBJECT)

typedef int (*ALL_INTERSECTIONS_METHOD)PARAMS((OBJECT *, RAY *, ISTACK *));
typedef int (*INSIDE_METHOD)PARAMS((VECTOR *, OBJECT *));
typedef void (*NORMAL_METHOD)PARAMS((VECTOR *, OBJECT *, VECTOR *));
typedef void *(*COPY_METHOD)PARAMS((OBJECT *));
typedef void (*TRANSLATE_METHOD)PARAMS((OBJECT *, VECTOR *));
typedef void (*ROTATE_METHOD)PARAMS((OBJECT *, VECTOR *));
typedef void (*SCALE_METHOD)PARAMS((OBJECT *, VECTOR *));
typedef void (*TRANSFORM_METHOD)PARAMS((OBJECT *, TRANSFORM *));
typedef void (*INVERT_METHOD)PARAMS((OBJECT *));
typedef void (*DESTROY_METHOD)PARAMS((OBJECT *));

struct Method_Struct
  {
   ALL_INTERSECTIONS_METHOD All_Intersections_Method;
   INSIDE_METHOD Inside_Method;
   NORMAL_METHOD Normal_Method;
   COPY_METHOD Copy_Method;
   TRANSLATE_METHOD Translate_Method;
   ROTATE_METHOD Rotate_Method;
   SCALE_METHOD Scale_Method;
   TRANSFORM_METHOD Transform_Method;
   INVERT_METHOD Invert_Method;
   DESTROY_METHOD Destroy_Method;
  };

#define All_Intersections(x,y,z) ((*((x)->Methods->All_Intersections_Method)) (x,y,z))
#define Inside(x,y) ((*((y)->Methods->Inside_Method)) (x,y))
#define Normal(x,y,z) ((*((y)->Methods->Normal_Method)) (x,y,z))
#define Copy(x) ((*((x)->Methods->Copy_Method)) (x))
#define Translate(x,y) ((*((x)->Methods->Translate_Method)) (x,y))
#define Scale(x,y) ((*((x)->Methods->Scale_Method)) (x,y))
#define Rotate(x,y) ((*((x)->Methods->Rotate_Method)) (x,y))
#define Transform(x,y) ((*((x)->Methods->Transform_Method)) (x,y))
#define Invert(x) ((*((x)->Methods->Invert_Method)) (x))
#define Destroy(x) ((*((x)->Methods->Destroy_Method)) (x))

#define Destroy_Camera(x) if ((x)!=NULL) free(x)

struct Camera_Struct
  {
   VECTOR Location;
   VECTOR Direction;
   VECTOR Up;
   VECTOR Right;
   VECTOR Sky;
  };

struct Bounding_Box_Struct {
   VECTOR Lower_Left, Lengths;
   };

/* These fields are common to all objects */

#define OBJECT_FIELDS \
 METHODS *Methods;\
 int Type;\
 OBJECT *Sibling;\
 TEXTURE *Texture;\
 OBJECT *Bound;\
 OBJECT *Clip;\
 BBOX Bounds;\
 short No_Shadow_Flag;

/* These fields are common to all compound objects */
#define COMPOUND_FIELDS \
 OBJECT_FIELDS \
 OBJECT *Children;

#define INIT_OBJECT_FIELDS(o,t,m)\
 o->Type=t;o->Methods=m;o->Sibling=NULL;o->Texture=NULL;\
 o->Bound=NULL;o->Clip=NULL;o->No_Shadow_Flag=FALSE;\
 Make_Vector(&o->Bounds.Lower_Left, -BOUND_HUGE/2, -BOUND_HUGE/2, -BOUND_HUGE/2)\
 Make_Vector(&o->Bounds.Lengths, BOUND_HUGE, BOUND_HUGE, BOUND_HUGE)

/* This is an abstract structure that is never actually used.
   All other objects are descendents of this primative type */

struct Object_Struct
  {
   OBJECT_FIELDS
  };

struct CSG_Struct
  {
   COMPOUND_FIELDS
  };

struct Light_Source_Struct
  {
   COMPOUND_FIELDS
   COLOUR Colour;
   VECTOR Center, Points_At, Axis1, Axis2;
   DBL Coeff, Radius, Falloff;
   LIGHT_SOURCE *Next_Light_Source;
   unsigned char Light_Type, Area_Light, Jitter, Track;
   int    Area_Size1, Area_Size2;
   int    Adaptive_Level;
   COLOUR **Light_Grid;
   OBJECT *Shadow_Cached_Object;
  };

/* Light source types */
#define POINT_SOURCE     1
#define SPOT_SOURCE      2

#define BUNCHING_FACTOR 4
struct Composite_Struct
  {
   OBJECT_FIELDS
   unsigned short int Entries;
   OBJECT *Objects[BUNCHING_FACTOR];
  };

struct Sphere_Struct
  {
   OBJECT_FIELDS
   TRANSFORM *Trans; 
   VECTOR  Center;
   DBL     Radius;
   DBL     Radius_Squared;
   DBL     Inverse_Radius;
   VECTOR  CMOtoC;
   DBL     CMOCSquared;
   short   CMinside, CMCached, Inverted;
  };

struct Quadric_Struct
  {
   OBJECT_FIELDS
   VECTOR  Square_Terms;
   VECTOR  Mixed_Terms;
   VECTOR  Terms;
   DBL Constant;
   DBL CM_Constant;
   short Constant_Cached;
   short Non_Zero_Square_Term;
  };

typedef unsigned short HF_val;

typedef struct {
   HF_val min_y, max_y;
} HF_BLOCK;

typedef struct {
        float x, z;
        VECTOR normal;
} Cached_Normals;

typedef short HF_Normals[3];
#define HF_CACHE_SIZE 16
#define LOWER_TRI 0
#define UPPER_TRI 1



struct Height_Field_Struct
  {
   OBJECT_FIELDS 
   TRANSFORM *Trans; 
   BOX *bounding_box;
   DBL Block_Size;
   DBL Inv_Blk_Size;
   HF_BLOCK **Block;
   HF_val **Map;
   int Inverted;
   int cache_pos;
   Cached_Normals Normal_Vector[HF_CACHE_SIZE];
   int Smoothed;
   HF_Normals **Normals;
   };

struct Box_Struct
  {
   OBJECT_FIELDS 
   TRANSFORM *Trans; 
   VECTOR bounds[2];
   short Inverted;
  };

#define MAX_ORDER 15

#define STURM_FIELDS  OBJECT_FIELDS int Sturm_Flag;

/* Number of coefficients of a three variable polynomial of order x */
#define term_counts(x) (((x)+1)*((x)+2)*((x)+3)/6)

struct Poly_Struct
  {
   STURM_FIELDS
   TRANSFORM *Trans;
   short Inverted;
   int Order;
   DBL *Coeffs;
  };

struct Disc_Struct {
   OBJECT_FIELDS
   TRANSFORM *Trans; /* Transformation of a Disc object */
   VECTOR center;    /* Center of the disc */
   VECTOR normal;    /* Direction perpendicular to the disc (plane normal) */
   DBL d;            /* The constant part of the plane equation */
   DBL iradius2;     /* Distance from center to inner circle of the disc */
   DBL oradius2;     /* Distance from center to outer circle of the disc */
   short Inverted;
   };

struct Cone_Struct {
   OBJECT_FIELDS
   TRANSFORM *Trans;   /* Transformation of a Cone object */
   short int cyl_flag; /* Is this a cone or a cylinder? */
   short int closed;   /* Currently unused - for making caps on the cone */
   VECTOR apex;        /* Center of the top of the cone */
   VECTOR base;        /* Center of the bottom of the cone */
   DBL apex_radius;    /* Radius of the cone at the top */
   DBL base_radius;    /* Radius of the cone at the bottom */
   DBL dist;           /* Distance to end of cone in canonical coords */
   short Inverted;
   };

typedef struct Bezier_Node_Struct BEZIER_NODE;
typedef struct Bezier_Child_Struct BEZIER_CHILDREN;
typedef struct Bezier_Vertices_Struct BEZIER_VERTICES;

struct Bezier_Child_Struct
  {
   BEZIER_NODE *Children[4];
  };

struct Bezier_Vertices_Struct
  {
   float uvbnds[4];
   VECTOR Vertices[4];
  };

struct Bezier_Node_Struct
  {
   int Node_Type;      /* Is this an interior node, or a leaf */
   VECTOR Center;      /* Center of sphere bounding the (sub)patch */
   DBL Radius_Squared; /* Radius of bounding sphere (squared) */
   int Count;          /* # of subpatches associated with this node */
   void *Data_Ptr;     /* Either pointer to vertices or pointer to children */
  };

#define BEZIER_INTERIOR_NODE 0
#define BEZIER_LEAF_NODE 1
#define MAX_BICUBIC_INTERSECTIONS 32

#define MAX_PATCH_TYPE 4

struct Bicubic_Patch_Struct
  {
   OBJECT_FIELDS
   int Patch_Type, U_Steps, V_Steps;
   VECTOR Control_Points[4][4];
   VECTOR Bounding_Sphere_Center;
   DBL Bounding_Sphere_Radius;
   DBL Flatness_Value;
   int Intersection_Count;
   VECTOR Normal_Vector[MAX_BICUBIC_INTERSECTIONS];
   VECTOR IPoint[MAX_BICUBIC_INTERSECTIONS];
   VECTOR **Interpolated_Grid, **Interpolated_Normals, **Smooth_Normals;
   DBL **Interpolated_D;
   BEZIER_NODE *Node_Tree;
  };
   
#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

struct Triangle_Struct
  {
   OBJECT_FIELDS
   VECTOR  Normal_Vector;
   DBL     Distance;
   DBL     CMNormDotOrigin;
   unsigned int  CMCached:1;
   unsigned int  Dominant_Axis:2;
   unsigned int  vAxis:2;  /* used only for smooth triangles */
   VECTOR  P1, P2, P3;
   short int Degenerate_Flag;
  };

struct Smooth_Triangle_Struct
  {
   OBJECT_FIELDS
   VECTOR  Normal_Vector;
   DBL     Distance;
   DBL     CMNormDotOrigin;
   unsigned int  CMCached:1;
   unsigned int  Dominant_Axis:2;
   unsigned int  vAxis:2;         /* used only for smooth triangles */
   VECTOR  P1, P2, P3;
   short int Degenerate_Flag;
   VECTOR  N1, N2, N3, Perp;
   DBL  BaseDelta;
  };

struct Plane_Struct
  {
   OBJECT_FIELDS
   VECTOR  Normal_Vector;
   DBL     Distance;
   DBL     CMNormDotOrigin;
   int     CMCached;
  };

typedef struct {
   VECTOR pos;
   DBL radius2;
   DBL coeffs[3];
   DBL tcoeffs[5];
   } Blob_Element;

typedef struct blob_list_struct *blobstackptr;
struct blob_list_struct {
   Blob_Element elem;
   blobstackptr next;
   };

typedef struct {
   int type, index;
   DBL bound;
   } Blob_Interval;

struct Blob_Struct
   {
   STURM_FIELDS
   TRANSFORM *Trans;
   short Inverted;
   int count;
   DBL threshold;
   Blob_Element **list;
   Blob_Interval *intervals;
};

#define MAX_CONTAINING_OBJECTS 10

struct Ray_Struct
  {
   VECTOR Initial;               /*  Xo  Yo  Zo  */
   VECTOR Direction;             /*  Xv  Yv  Zv  */
   VECTOR Initial_2;             /*  Xo^2  Yo^2  Zo^2  */
   VECTOR Direction_2;           /*  Xv^2  Yv^2  Zv^2  */
   VECTOR Initial_Direction;     /*  XoXv  YoYv  ZoZv  */
   VECTOR Mixed_Initial_Initial; /*  XoYo  XoZo  YoZo  */
   VECTOR Mixed_Dir_Dir;         /*  XvYv  XvZv  YvZv  */
   VECTOR Mixed_Init_Dir;        /*  XoYv+XvYo  XoZv+XvZo  YoZv+YvZo  */
   int Containing_Index;
   TEXTURE *Containing_Textures [MAX_CONTAINING_OBJECTS];
   DBL Containing_IORs [MAX_CONTAINING_OBJECTS];
   int Quadric_Constants_Cached;
  };

struct Frame_Struct
  {
   CAMERA *Camera;
   int Screen_Height, Screen_Width;
   LIGHT_SOURCE *Light_Sources;
   OBJECT *Objects;
   DBL Atmosphere_IOR, Antialias_Threshold;
   DBL Fog_Distance;
   COLOUR Fog_Colour;
   COLOUR Background_Colour;
  };

#define DISPLAY 1
#define VERBOSE 2
#define DISKWRITE 4
#define PROMPTEXIT 8
#define ANTIALIAS 16
#define DEBUGGING 32
#define RGBSEPARATE 64
#define EXITENABLE 128
#define CONTINUE_TRACE 256
#define VERBOSE_FILE 512
#define JITTER 1024

/* Definitions for ISTACK structure */

struct istk_entry
  {
   DBL Depth;
   VECTOR IPoint;
   OBJECT *Object;
  };

struct istack_struct
  {
   struct istack_struct *next;
   struct istk_entry *istack;
   unsigned int top_entry;
  };

#define itop(i) i->istack[i->top_entry]
#define push_entry(d,v,o,i) itop(i).Depth=d; itop(i).IPoint=v; \
 itop(i).Object=o; incstack(i);
#define push_copy(i,e) itop(i)=*e; incstack(i);
#define pop_entry(i) (i->top_entry > 0)?&(i->istack[--i->top_entry]):NULL

#define MAX_STRING_INDEX 41 

struct Reserved_Word_Struct
  {
   TOKEN Token_Number;
   char *Token_Name;
  };

/* Here's where you dump the information on the current token (fm. PARSE.C) */

struct Token_Struct
  {
   TOKEN Token_Id;
   int Token_Line_No;
   char *Token_String;
   DBL Token_Float;
   TOKEN Begin_Id;
   int Constant_Index;
   int Unget_Token, End_Of_File;
   char *Filename, *Constant_Data;
  };

/* Types of constants allowed in DECLARE statement (fm. PARSE.C) */

#define COLOUR_CONSTANT         0
#define VECTOR_CONSTANT         1
#define FLOAT_CONSTANT          2
#define PIGMENT_CONSTANT        3
#define TNORMAL_CONSTANT        4
#define FINISH_CONSTANT         5
#define TEXTURE_CONSTANT        6
#define OBJECT_CONSTANT         7
#define COLOUR_MAP_CONSTANT     8
#define TRANSFORM_CONSTANT      9
#define CAMERA_CONSTANT        10

/* CSG types */
#define CSG_UNION_TYPE             1
#define CSG_INTERSECTION_TYPE      2
#define CSG_DIFFERENCE_TYPE        4
#define CSG_MERGE_TYPE             8
#define CSG_SINGLE_TYPE           16

struct Constant_Struct
  {
   int Identifier_Number;
   CONSTANT Constant_Type;
   char *Constant_Data;
  };

struct Chunk_Header_Struct 
  {
   long name;
   long size;
  };

struct Data_File_Struct 
  {
   FILE *File;
   char *Filename;
   int Line_Number;
  };

struct complex_block 
  {
   DBL r, c;
  };

#define READ_MODE 0
#define WRITE_MODE 1
#define APPEND_MODE 2

struct file_handle_struct 
  {
   char *filename;
   int  mode;
   int width, height;
   int buffer_size;
   char *buffer;
   FILE *file;
   char *(*Default_File_Name_p) PARAMS((void));
   int  (*Open_File_p) PARAMS((struct file_handle_struct *handle,
		   char *name, int *width, int *height, int buffer_size,
		   int mode));
   void (*Write_Line_p) PARAMS((struct file_handle_struct *handle,
		   COLOUR *line_data, int line_number));
   int  (*Read_Line_p) PARAMS((struct file_handle_struct *handle,
		   COLOUR *line_data, int *line_number));
   void (*Read_Image_p) PARAMS((IMAGE *Image, char *filename));
   void (*Close_File_p) PARAMS((struct file_handle_struct *handle));
  };

typedef struct file_handle_struct FILE_HANDLE;

#define Default_File_Name(h) ((*((h)->Default_File_Name_p)) ())
#define Open_File(h,n,wd,ht,sz,m) ((*((h)->Open_File_p)) (h,n,wd,ht,sz,m))
#define Write_Line(h,l,n) ((*((h)->Write_Line_p)) (h, l, n))
#define Read_Line(h,l,n) ((*((h)->Read_Line_p)) (h, l, n))
#define Read_Image(h,i) ((*((h)->Read_Image_p)) (h, i))
#define Close_File(h) ((*((h)->Close_File_p)) (h))


#define Q_FULL_AMBIENT 1
#define Q_QUICKC       2
#define Q_SHADOW       4
#define Q_AREA_LIGHT   8
#define Q_REFRACT     16
#define Q_REFLECT     32
#define Q_NORMAL      64

#define QUALITY_0 Q_QUICKC+Q_FULL_AMBIENT
#define QUALITY_1 QUALITY_0
#define QUALITY_2 QUALITY_1-Q_FULL_AMBIENT
#define QUALITY_3 QUALITY_2
#define QUALITY_4 QUALITY_3+Q_SHADOW
#define QUALITY_5 QUALITY_4+Q_AREA_LIGHT
#define QUALITY_6 QUALITY_5-Q_QUICKC+Q_REFRACT
#define QUALITY_7 QUALITY_6
#define QUALITY_8 QUALITY_7+Q_REFLECT+Q_NORMAL
#define QUALITY_9 QUALITY_8
