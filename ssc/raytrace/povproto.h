/****************************************************************************
*                   povproto.h
*
*  This module defines the prototypes for all system-independent functions.
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

/* Prototypes for functions defined in povray.c */
void usage PARAMS((void));
void init_vars PARAMS((void));
void close_all PARAMS((void));
void get_defaults PARAMS((void));
void read_options PARAMS((char *File_Name));
void parse_option PARAMS((char *Option_String));
void Print_Options PARAMS((void));
void parse_file_name PARAMS((char *File_Name));
void print_stats PARAMS((void));
FILE *Locate_File PARAMS((char *filename, char *mode));
void print_credits PARAMS((void));

/* Prototypes for functions defined in render.c */
void Create_Ray PARAMS((RAY *ray, int width, int height, DBL x, DBL y));
void Supersample PARAMS((COLOUR *result, int x, int y, int Width,int Height));
void Read_Rendered_Part PARAMS((void));
void Start_Tracing PARAMS((void));
void Initialize_Renderer PARAMS((void));
void Trace PARAMS((RAY *Ray, COLOUR *Colour));
void Check_User_Abort PARAMS((int Do_Stats));

/* Prototypes for functions defined in bound.c */
void Destroy_Composite PARAMS((OBJECT *Object));
void BuildBoundingSlabs PARAMS((OBJECT **Root));
void recompute_bbox PARAMS((BBOX *bbox, TRANSFORM *trans));
void Recompute_Inverse_BBox PARAMS((BBOX *bbox, TRANSFORM *trans));
int Bounds_Intersect PARAMS((OBJECT *Root, RAY *ray, INTERSECTION *Best_Intersection, OBJECT **Best_Object));

/* Prototypes for functions defined in tokenize.c */
void Initialize_Tokenizer PARAMS((char *filename));
void Terminate_Tokenizer PARAMS((void));
void Get_Token PARAMS((void));
void Unget_Token PARAMS((void));
int Skip_Spaces PARAMS((DATA_FILE *Data_File));
int Parse_C_Comments PARAMS((DATA_FILE *Data_File));
void Begin_String PARAMS((void));
void Stuff_Character PARAMS((int c, DATA_FILE *Data_File));
void End_String PARAMS((DATA_FILE *Data_File));
int Read_Float PARAMS((DATA_FILE *Data_File));
void Parse_String PARAMS((DATA_FILE *Data_File));
int Read_Symbol PARAMS((DATA_FILE *Data_File));
int Find_Reserved PARAMS((void));
int Find_Symbol PARAMS((void));
void Token_Error PARAMS((DATA_FILE *Data_File, char *str));

/* Prototypes for functions defined in parse.c */
void Parse_Error PARAMS((TOKEN Token_Id));
void Parse_Error_Str PARAMS((char *str));
void Parse_Begin PARAMS((void));
void Parse_End PARAMS((void));
void Parse_Colour PARAMS((COLOUR *Colour));
COLOUR_MAP *Parse_Colour_Map PARAMS((void));
COLOUR_MAP *Parse_Colour_List PARAMS((int MinCount));
void Parse_Comma PARAMS((void));
DBL Parse_Float PARAMS((void));
void Parse_Vector PARAMS((VECTOR *Vector));
void Parse_Vector_Float PARAMS((VECTOR *Vector));
void Parse_Vector PARAMS((VECTOR *Vector));
void Parse_Scale_Vector PARAMS((VECTOR *Vector));
void Parse PARAMS((void));
void Write_Token PARAMS((TOKEN Token_Id, DATA_FILE *Data_File));
void Error PARAMS((char *str));
void MAError PARAMS((char *str));
void Warn PARAMS((char *str, DBL Level));

/* Prototypes for functions defined in objects.c */
int Intersection PARAMS((INTERSECTION *Ray_Intersection, OBJECT *Object, RAY *Ray));
int Ray_In_Bounds PARAMS((RAY *Ray, OBJECT *Bounds));
int Point_In_Clip PARAMS((VECTOR *IPoint, OBJECT *Clip));
OBJECT *Copy_Bound_Clip PARAMS((OBJECT *Old));
OBJECT *Copy_Object PARAMS((OBJECT *Old));
void Translate_Object PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Object PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Object PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Object PARAMS((OBJECT *Object, TRANSFORM *Trans));
int Inside_Object PARAMS((VECTOR *IPoint, OBJECT *Vector));
void Invert_Object PARAMS((OBJECT *Object));
void Destroy_Object PARAMS((OBJECT *Object));
void create_istack PARAMS((void));
ISTACK *open_istack PARAMS((void));
void close_istack PARAMS((ISTACK *istk));
void incstack PARAMS((ISTACK *istk));

/* Prototypes for functions defined in spheres.c */
int All_Sphere_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int All_Ellipsoid_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int Intersect_Sphere PARAMS((RAY *Ray, SPHERE *Sphere, DBL *Depth1, DBL *Depth2));
int Inside_Sphere PARAMS((VECTOR *IPoint, OBJECT *Object));
int Inside_Ellipsoid PARAMS((VECTOR *IPoint, OBJECT *Object));
void Sphere_Normal PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void Ellipsoid_Normal PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Sphere PARAMS((OBJECT *Object));
void Translate_Sphere PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Sphere PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Sphere PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Sphere PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Invert_Sphere PARAMS((OBJECT *Object));
void Destroy_Sphere PARAMS((OBJECT *Object));
SPHERE *Create_Sphere PARAMS((void));

/* Prototypes for functions defined in quadrics.c */
int All_Quadric_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int Intersect_Quadric PARAMS((RAY *Ray, QUADRIC *Quadric, DBL *Depth1, DBL *Depth2));
int Inside_Quadric PARAMS((VECTOR *IPoint, OBJECT *Object));
void Quadric_Normal PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Quadric PARAMS((OBJECT *Object));
void Quadric_To_Matrix PARAMS((QUADRIC *Quadric, MATRIX *Matrix));
void Matrix_To_Quadric PARAMS((MATRIX *Matrix, QUADRIC *Quadric));
void Translate_Quadric PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Quadric PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Quadric PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Quadric PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Invert_Quadric PARAMS((OBJECT *Object));
void Destroy_Quadric PARAMS((OBJECT *Object));
QUADRIC *Create_Quadric PARAMS((void));

/* Prototypes for functions defined in poly.c */
int All_Poly_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int Inside_Poly PARAMS((VECTOR *IPoint, OBJECT *Object));
void Poly_Normal PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Poly PARAMS((OBJECT *Object));
void Translate_Poly PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Poly PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Poly PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Poly PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Invert_Poly PARAMS((OBJECT *Object));
void Destroy_Poly PARAMS((OBJECT *Object));
POLY *Create_Poly PARAMS((int Order));

/* Prototypes for functions defined in bezier.c */
void Precompute_Patch_Values PARAMS((BICUBIC_PATCH *Shape));
int All_Bicubic_Patch_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int Inside_Bicubic_Patch PARAMS((VECTOR *IPoint, OBJECT *Object));
void Bicubic_Patch_Normal PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Bicubic_Patch PARAMS((OBJECT *Object));
void Translate_Bicubic_Patch PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Bicubic_Patch PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Bicubic_Patch PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Bicubic_Patch PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Invert_Bicubic_Patch PARAMS((OBJECT *Object));
BICUBIC_PATCH *Create_Bicubic_Patch PARAMS((void));
void Destroy_Bicubic_Patch PARAMS((OBJECT *Object));

/* Prototypes for functions defined in boxes.c */
int All_Box_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int Intersect_Boxx PARAMS((RAY *Ray, BOX *box, DBL *Depth1, DBL *Depth2));
int Inside_Box PARAMS((VECTOR *point, OBJECT *Object));
void Box_Normal PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Box PARAMS((OBJECT *Object));
void Translate_Box PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Box PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Box PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Box PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Invert_Box PARAMS((OBJECT *Object));
BOX *Create_Box PARAMS((void));
void Destroy_Box PARAMS((OBJECT *Object));

/* Prototypes for functions defined in blob.c */
void Set_Blob_Solver PARAMS((OBJECT *obj, int Sturm_Flag));
int All_Blob_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
void BlobDelete PARAMS((OBJECT *obj));
int Inside_Blob PARAMS((VECTOR *point, OBJECT *Object));
void Blob_Normal PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Blob PARAMS((OBJECT *Object));
void Translate_Blob PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Blob PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Blob PARAMS((OBJECT *Object, VECTOR *Vector));
void Invert_Blob PARAMS((OBJECT *Object));
void Transform_Blob PARAMS((OBJECT *Object, TRANSFORM *Trans));
BLOB *Create_Blob PARAMS((void));
void Destroy_Blob PARAMS((OBJECT *Object));
void MakeBlob PARAMS((BLOB *blob, DBL threshold, blobstackptr bloblist, 
                      int npoints, int sflag));

/* Prototypes for functions defined in cones.c */
int All_Cone_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int Inside_Cone PARAMS((VECTOR *point, OBJECT *Object));
void Cone_Normal PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Cone PARAMS((OBJECT *Object));
void Translate_Cone PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Cone PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Cone PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Cone PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Invert_Cone PARAMS((OBJECT *Object));
CONE *Create_Cone PARAMS((void));
CONE *Create_Cylinder PARAMS((void));
void Compute_Cone_Data PARAMS((OBJECT *Object));
void Compute_Cylinder_Data PARAMS((OBJECT *Object));
void Destroy_Cone PARAMS((OBJECT *Object));

/* Prototypes for functions defined in cones.c */
int All_Disc_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int Intersect_Disc PARAMS((RAY *Ray, DISC *Disc, DBL *Depth));
int Inside_Disc PARAMS((VECTOR *point, OBJECT *Object));
void Disc_Normal PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Disc PARAMS((OBJECT *Object));
void Translate_Disc PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Disc PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Disc PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Disc PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Invert_Disc PARAMS((OBJECT *Object));
DISC *Create_Disc PARAMS((void));
void Destroy_Disc PARAMS((OBJECT *Object));

/* Prototypes for functions defined in hfield.c */
void Find_Hf_Min_Max PARAMS((HEIGHT_FIELD *H_Field, IMAGE *Image));
int Intersect_Sub_Block PARAMS((HF_BLOCK *Block, RAY *Ray, HEIGHT_FIELD *H_Field,
	VECTOR *start, VECTOR *end));
int Intersect_Hf_Node PARAMS((RAY *Ray, HEIGHT_FIELD *H_Field, VECTOR *start, VECTOR *end));
int Intersect_Box PARAMS((HEIGHT_FIELD *H_Field,RAY *Ray,DBL *depth1,DBL *depth2));
DBL Get_Height PARAMS((int x,int y,HEIGHT_FIELD *H_Field));
int Intersect_Pixel PARAMS((int x,int z,RAY *Ray,HEIGHT_FIELD *H_Field,DBL height1,DBL height2));
int All_HeightFld_Intersections PARAMS((OBJECT *Object,RAY *Ray,ISTACK *Depth_Stack));
int All_Csg_HeightFld_Intersections PARAMS((OBJECT *Object,RAY *Ray,ISTACK *Depth_Stack));
int Intersect_HeightFld PARAMS((RAY *Ray,HEIGHT_FIELD *H_Field,DBL *Depth));
int Inside_HeightFld PARAMS((VECTOR *IPoint,OBJECT *Object));
void HeightFld_Normal PARAMS((VECTOR *Result,OBJECT *Object,VECTOR *IPoint));
void *Copy_HeightFld PARAMS((OBJECT *Object));
void Translate_HeightFld PARAMS((OBJECT *Object,VECTOR *Vector));
void Rotate_HeightFld PARAMS((OBJECT *Object,VECTOR *Vector));
void Scale_HeightFld PARAMS((OBJECT *Object,VECTOR *Vector));
void Transform_HeightFld PARAMS((OBJECT *Object,TRANSFORM *Trans));
void Destroy_HeightFld PARAMS((OBJECT *Object));
void Invert_HeightFld PARAMS((OBJECT *Object));
HEIGHT_FIELD *Create_Height_Field PARAMS((void));

/* Prototypes for functions defined in triangle.c */
void Find_Triangle_Dominant_Axis PARAMS((TRIANGLE *Triangle));
int Compute_Triangle  PARAMS((TRIANGLE *Triangle, int Smooth));
void Compute_Smooth_Triangle  PARAMS((SMOOTH_TRIANGLE *Triangle));
int All_Triangle_Intersections  PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int Intersect_Triangle  PARAMS((RAY *Ray, TRIANGLE *Triangle, DBL *Depth));
int Inside_Triangle  PARAMS((VECTOR *IPoint, OBJECT *Object));
void Triangle_Normal  PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Triangle  PARAMS((OBJECT *Object));
void Translate_Triangle  PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Triangle  PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Triangle  PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Triangle  PARAMS((OBJECT *Object, TRANSFORM *Trans));
TRIANGLE *Create_Triangle PARAMS((void));
void Invert_Triangle  PARAMS((OBJECT *Object));
void Smooth_Triangle_Normal  PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Smooth_Triangle PARAMS((OBJECT *Object));
void Translate_Smooth_Triangle  PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Smooth_Triangle  PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Smooth_Triangle  PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Smooth_Triangle  PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Invert_Smooth_Triangle  PARAMS((OBJECT *Object));
SMOOTH_TRIANGLE *Create_Smooth_Triangle PARAMS((void));
void Destroy_Triangle  PARAMS((OBJECT *Object));

/* Prototypes for functions defined in vect.c */
int solve_quadratic PARAMS((DBL *x, DBL *y));
int solve_cubic PARAMS((DBL *x, DBL *y));
int solve_quartic PARAMS((DBL *x, DBL *y));
int polysolve PARAMS((int order, DBL *Coeffs, DBL *roots));

/* Prototypes for functions defined in lighting.c */
void Add_Pigment PARAMS((COLOUR *Colour, PIGMENT *Pigment, VECTOR *IPoint));
void Perturb_Normal PARAMS((VECTOR *Layer_Normal, TNORMAL *Tnormal, VECTOR *IPoint));
void Diffuse PARAMS((FINISH *Finish, VECTOR *IPoint, RAY *Eye, VECTOR *Layer_Normal, COLOUR *Layer_Colour, COLOUR *Colour,DBL Attenuation));
void Reflect PARAMS((DBL Reflection, VECTOR *IPoint, RAY *Ray, VECTOR *Layer_Normal, COLOUR *Colour));
void Refract PARAMS((TEXTURE *Texture, VECTOR *IPoint, RAY *Ray, VECTOR *Layer_Normal, COLOUR *Colour));
void Fog PARAMS((DBL Distance, COLOUR *Fog_Colour, DBL Fog_Distance, COLOUR *Colour));
void Compute_Reflected_Colour PARAMS ((RAY *Ray, FINISH *Finish,\
 INTERSECTION *Ray_Intersection, COLOUR *Layer_Colour, COLOUR *Filter_Colour,COLOUR *Colour, VECTOR *Layer_Normal));
void Determine_Apparent_Colour PARAMS ((INTERSECTION *Ray_Intersection, COLOUR *Colour, RAY *Ray));
void Filter_Shadow_Ray PARAMS ((INTERSECTION *Ray_Intersection, COLOUR *Colour));

/* Prototypes for functions defined in point.c */
int All_Light_Source_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int Inside_Light_Source PARAMS((VECTOR *point, OBJECT *Object));
void Light_Source_Normal PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Light_Source PARAMS((OBJECT *Object));
void Translate_Light_Source PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Light_Source PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Light_Source PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Light_Source PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Invert_Light_Source PARAMS((OBJECT *Object));
void Destroy_Light_Source PARAMS((OBJECT *Object));
LIGHT_SOURCE *Create_Light_Source PARAMS((void));
DBL Attenuate_Light PARAMS((LIGHT_SOURCE *Light_Source, RAY *Light_Source_Ray));
COLOUR **Create_Light_Grid PARAMS((int Size1, int Size2));

/* Prototypes for functions defined in texture.c */
void Compute_Colour PARAMS((COLOUR *Colour,PIGMENT *Pigment, DBL value));
void Initialize_Noise PARAMS((void));
void InitTextureTable PARAMS((void));
void InitRTable PARAMS((void));
int R PARAMS((VECTOR *v));
int Crc16 PARAMS((char *buf, int count));
DBL Noise PARAMS((DBL x, DBL y, DBL z));
void DNoise PARAMS((VECTOR *result, DBL x, DBL y, DBL z));
DBL Turbulence PARAMS((DBL x, DBL y, DBL z, DBL omega, DBL lambda, int octaves));
void DTurbulence PARAMS((VECTOR *result, DBL x, DBL y, DBL z, DBL omega, DBL lambda, int octaves));
DBL cycloidal PARAMS((DBL value));
DBL Triangle_Wave PARAMS((DBL value));
void Translate_Textures PARAMS((TEXTURE *Textures, VECTOR *Vector));
void Rotate_Textures PARAMS((TEXTURE *Textures, VECTOR *Vector));
void Scale_Textures PARAMS((TEXTURE *Textures, VECTOR *Vector));
void Transform_Textures PARAMS((TEXTURE *Textures, TRANSFORM *Trans));
void Destroy_Textures PARAMS((TEXTURE *Textures));
void Post_Textures PARAMS((TEXTURE *Textures));
FINISH *Create_Finish PARAMS((void));
FINISH *Copy_Finish PARAMS((FINISH *Old));
TEXTURE *Create_PNF_Texture PARAMS((void));
TILES *Create_Tiles_Texture PARAMS((void));
MATERIAL *Create_Material_Texture PARAMS((void));
TEXTURE *Copy_Textures PARAMS((TEXTURE *Textures));
TEXTURE *Create_Texture PARAMS((void));
TEXTURE *Copy_Materials PARAMS((TEXTURE *Old));

/* Prototypes for functions defined in pigment.c */
void agate PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void bozo PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void brick PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void checker PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void gradient PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void granite PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void marble PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void mandel PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void radial PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void spotted PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void wood PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void leopard PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));   /* SWT 7/18/91 */
void onion PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
void hexagon PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
PIGMENT *Create_Pigment PARAMS((void));
PIGMENT *Copy_Pigment PARAMS((PIGMENT *Old));
void Translate_Pigment PARAMS((PIGMENT *Pigment, VECTOR *Vector));
void Rotate_Pigment PARAMS((PIGMENT *Pigment, VECTOR *Vector));
void Scale_Pigment PARAMS((PIGMENT *Pigment, VECTOR *Vector));
void Transform_Pigment PARAMS((PIGMENT *Pigment, TRANSFORM *Trans));
void Destroy_Pigment PARAMS((PIGMENT *Pigment));
void Post_Pigment PARAMS((PIGMENT *Pigment));

/* Prototypes for functions defined in normal.c */
void ripples PARAMS((DBL x, DBL y, DBL z, TNORMAL *Tnormal, VECTOR *Vector));
void waves PARAMS((DBL x, DBL y, DBL z, TNORMAL *Tnormal, VECTOR *Vector));
void bumps PARAMS((DBL x, DBL y, DBL z, TNORMAL *Tnormal, VECTOR *normal));
void dents PARAMS((DBL x, DBL y, DBL z, TNORMAL *Tnormal, VECTOR *normal));
void wrinkles PARAMS((DBL x, DBL y, DBL z, TNORMAL *Tnormal, VECTOR *normal));
TNORMAL *Create_Tnormal PARAMS((void));
TNORMAL *Copy_Tnormal PARAMS((TNORMAL *Old));
void Translate_Tnormal PARAMS((TNORMAL *Tnormal, VECTOR *Vector));
void Rotate_Tnormal PARAMS((TNORMAL *Tnormal, VECTOR *Vector));
void Scale_Tnormal PARAMS((TNORMAL *Tnormal, VECTOR *Vector));
void Transform_Tnormal PARAMS((TNORMAL *Tnormal, TRANSFORM *Trans));
void Destroy_Tnormal PARAMS((TNORMAL *Tnormal));
void Post_Tnormal PARAMS((TNORMAL *Tnormal));

/* Prototypes for functions defined in txttest.c */
void painted1 PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour)); /* CdW 7/2/91 */
void painted2 PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour)); /* CdW 7/2/91 */
void painted3 PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour)); /* CdW 7/2/91 */
void bumpy1 PARAMS((DBL x, DBL y, DBL z, TNORMAL *Tnormal, VECTOR *normal)); /* CdW 7/2/91*/
void bumpy2 PARAMS((DBL x, DBL y, DBL z, TNORMAL *Tnormal, VECTOR *normal)); /* CdW 7/2/91*/
void bumpy3 PARAMS((DBL x, DBL y, DBL z, TNORMAL *Tnormal, VECTOR *normal)); /* CdW 7/2/91*/

/* Prototypes for functions defined in image.c */
void image_map PARAMS((DBL x, DBL y, DBL z, PIGMENT *Pigment, COLOUR *colour));
TEXTURE *material_map PARAMS((VECTOR *IPoint, MATERIAL *Texture));
TEXTURE *tiles_texture PARAMS((VECTOR *IPoint, TILES *Texture));
void bump_map PARAMS((DBL x, DBL y, DBL z, TNORMAL *Tnormal, VECTOR *normal));/* CdW 7/8/91*/
void gouge_map PARAMS((DBL x, DBL y, DBL z, TNORMAL *Tnormal, VECTOR *normal));
int map PARAMS((DBL x,DBL y,DBL z, TPATTERN *Turb ,IMAGE *Image,DBL *xcoor, DBL *ycoor));
IMAGE *Copy_Image PARAMS ((IMAGE *Old));
IMAGE *Create_Image PARAMS ((void));
void Destroy_Image PARAMS ((IMAGE *Image));

/* Prototypes for functions defined in csg.c */
int All_CSG_Union_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int All_CSG_Merge_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int All_CSG_Intersect_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack)); 
int Inside_CSG_Union PARAMS((VECTOR *point, OBJECT *Object));
int Inside_CSG_Intersection PARAMS((VECTOR *point, OBJECT *Object));
void *Copy_CSG PARAMS((OBJECT *Object));
void Translate_CSG PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_CSG PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_CSG PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_CSG PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Destroy_CSG PARAMS((OBJECT *Object));
void Invert_CSG_Union PARAMS((OBJECT *Object));
void Invert_CSG_Intersection PARAMS((OBJECT *Object));
CSG *Create_CSG_Union PARAMS((void));
CSG *Create_CSG_Merge PARAMS((void));
CSG *Create_CSG_Intersection PARAMS((void));
void Compute_CSG_Bounds PARAMS((OBJECT *Object));

/* Prototypes for functions defined in colour.c */
COLOUR *Create_Colour PARAMS((void));
COLOUR *Copy_Colour PARAMS((COLOUR *Old));
COLOUR_MAP_ENTRY *Create_CMap_Entries PARAMS((int Map_Size));
COLOUR_MAP_ENTRY *Copy_CMap_Entries PARAMS((COLOUR_MAP_ENTRY *Old,int Map_Size));
COLOUR_MAP *Create_Colour_Map PARAMS((void));
COLOUR_MAP *Copy_Colour_Map PARAMS((COLOUR_MAP *Old));
DBL Colour_Distance PARAMS((COLOUR *colour1, COLOUR *colour2));
void Add_Colour PARAMS((COLOUR *result, COLOUR *colour1, COLOUR *colour2));
void Scale_Colour PARAMS((COLOUR *result, COLOUR *colour, DBL factor));
void Clip_Colour PARAMS((COLOUR *result, COLOUR *colour));
void Destroy_Colour_Map PARAMS((COLOUR_MAP *CMap));

/* Prototypes for functions defined in camera.c */
void Translate_Camera PARAMS((CAMERA *Cm, VECTOR *Vector));
void Rotate_Camera PARAMS((CAMERA *Cm, VECTOR *Vector));
void Scale_Camera PARAMS((CAMERA *Cm, VECTOR *Vector));
void Transform_Camera PARAMS((CAMERA *Cm, TRANSFORM *Trans));
CAMERA *Copy_Camera PARAMS((CAMERA *Old));
CAMERA *Create_Camera PARAMS((void));

/* Prototypes for functions defined in ray.c */
void Make_Ray PARAMS((RAY *r));
void Initialize_Ray_Containers PARAMS((RAY *Ray));
void Copy_Ray_Containers PARAMS((RAY *Dest_Ray, RAY *Source_Ray));
void Ray_Enter PARAMS((RAY *ray, TEXTURE *texture));
void Ray_Exit PARAMS((RAY *ray));

/* Prototypes for functions defined in planes.c */
int All_Plane_Intersections PARAMS((OBJECT *Object, RAY *Ray, ISTACK *Depth_Stack));
int Intersect_Plane PARAMS((RAY *Ray, PLANE *Plane, DBL *Depth));
int Inside_Plane PARAMS((VECTOR *point, OBJECT *Object));
void Plane_Normal PARAMS((VECTOR *Result, OBJECT *Object, VECTOR *IPoint));
void *Copy_Plane PARAMS((OBJECT *Object));
void Translate_Plane PARAMS((OBJECT *Object, VECTOR *Vector));
void Rotate_Plane PARAMS((OBJECT *Object, VECTOR *Vector));
void Scale_Plane PARAMS((OBJECT *Object, VECTOR *Vector));
void Transform_Plane PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Invert_Plane PARAMS((OBJECT *Object));
void Destroy_Plane PARAMS((OBJECT *Object));
PLANE *Create_Plane PARAMS((void));

/* Prototypes for functions defined in iff.c */
void iff_error PARAMS((void));
int read_byte PARAMS((FILE *f));
int read_word PARAMS((FILE *f));
long read_long PARAMS((FILE *f));
void Read_Chunk_Header PARAMS((FILE *f, CHUNK_HEADER *dest)); 
void Read_Iff_Image PARAMS((IMAGE *Image, char *filename));

/* Prototypes for functions defined in gif.c */
int out_line PARAMS((unsigned char *pixels, int linelen));
int get_byte PARAMS((void));
void Read_Gif_Image PARAMS((IMAGE *Image, char *filename));

/* Prototypes for functions defined in gifdecod.c */
void cleanup_gif_decoder PARAMS((void));
WORD init_exp PARAMS((int i_size));   /* changed param to int to avoid
					 problems with 32bit int ANSI
					 compilers. */
WORD get_next_code PARAMS((void));
WORD decoder PARAMS((int i_linewidth)); /* same as above */

/* Prototypes for machine specific functions defined in "computer".c (ibm.c amiga.c unix.c etc.)*/
void display_finished PARAMS((void));
void display_init PARAMS((int width, int height));
void display_close PARAMS((void));
void display_plot PARAMS((int x, int y, unsigned char Red, unsigned char Green, unsigned char Blue));

/* Prototypes for functions defined in matrices.c */
void MZero PARAMS((MATRIX *result));
void MIdentity PARAMS((MATRIX *result));
void MTimes PARAMS((MATRIX *result, MATRIX *matrix1, MATRIX *matrix2));
void MAdd PARAMS((MATRIX *result, MATRIX *matrix1, MATRIX *matrix2));
void MSub PARAMS((MATRIX *result, MATRIX *matrix1, MATRIX *matrix2));
void MScale PARAMS((MATRIX *result, MATRIX *matrix1, DBL amount));
void MTranspose PARAMS((MATRIX *result, MATRIX *matrix1));
void MTransPoint PARAMS((VECTOR *result, VECTOR *vector, TRANSFORM *trans));
void MInvTransPoint PARAMS((VECTOR *result, VECTOR *vector, TRANSFORM *trans));
void MTransDirection PARAMS((VECTOR *result, VECTOR *vector, TRANSFORM *trans));
void MInvTransDirection PARAMS((VECTOR *result, VECTOR *vector, TRANSFORM *trans));
void MTransNormal PARAMS((VECTOR *result, VECTOR *vector, TRANSFORM *trans));
void MInvTransNormal PARAMS((VECTOR *result, VECTOR *vector, TRANSFORM *trans));
void Compute_Scaling_Transform PARAMS((TRANSFORM *result, VECTOR *vector));
void Compute_Inversion_Transform PARAMS((TRANSFORM *result));
void Compute_Translation_Transform PARAMS((TRANSFORM *transform, VECTOR *vector));
void Compute_Rotation_Transform PARAMS((TRANSFORM *transform, VECTOR *vector));
void Compute_Look_At_Transform PARAMS((TRANSFORM *transform, VECTOR *Look_At, VECTOR *Up, VECTOR *Right));
void Compose_Transforms PARAMS((TRANSFORM *Original_Transform, TRANSFORM *New_Transform));
void Compute_Axis_Rotation_Transform PARAMS((TRANSFORM *transform, VECTOR *V, DBL angle));
void Compute_Coordinate_Transform PARAMS((TRANSFORM *trans, VECTOR *origin, VECTOR *up, DBL r, DBL len));
TRANSFORM *Create_Transform PARAMS((void));
TRANSFORM *Copy_Transform PARAMS((TRANSFORM *Old));
VECTOR *Create_Vector PARAMS((void));
VECTOR *Copy_Vector PARAMS((VECTOR *Old));
DBL *Create_Float PARAMS((void));
DBL *Copy_Float PARAMS((DBL *Old));

/* Prototypes for functions defined in dump.c */
FILE_HANDLE *Get_Dump_File_Handle PARAMS((void));
char *Default_Dump_File_Name PARAMS((void));
int Open_Dump_File PARAMS((FILE_HANDLE *handle, char *name,
                           int *width, int *height, int buffer_size, int mode));
void Write_Dump_Line PARAMS((FILE_HANDLE *handle, COLOUR *line_data, int line_number));
int Read_Dump_Line PARAMS((FILE_HANDLE *handle, COLOUR *line_data, int *line_number));
int Read_Dump_Int_Line PARAMS((FILE_HANDLE *handle, IMAGE_LINE *line_data, int *line_number));
void Read_Dump_Image PARAMS((IMAGE *Image, char *filename));
void Close_Dump_File PARAMS((FILE_HANDLE *handle));

/* Prototypes for functions defined in targa.c */
FILE_HANDLE *Get_Targa_File_Handle PARAMS((void));
char *Default_Targa_File_Name PARAMS((void));
int Open_Targa_File PARAMS((FILE_HANDLE *handle, char *name,
                           int *width, int *height, int buffer_size, int mode));
void Write_Targa_Line PARAMS((FILE_HANDLE *handle, COLOUR *line_data, int line_number));
int Read_Targa_Line PARAMS((FILE_HANDLE *handle, COLOUR *line_data, int *line_number));
void Close_Targa_File PARAMS((FILE_HANDLE *handle));
void Read_Targa_Image PARAMS((IMAGE *Image, char *filename));

/* Prototypes for functions defined in Raw.c */
FILE_HANDLE *Get_Raw_File_Handle PARAMS((void));
char *Default_Raw_File_Name PARAMS((void));
int Open_Raw_File PARAMS((FILE_HANDLE *handle, char *name,
                           int *width, int *height, int buffer_size, int mode));
void Write_Raw_Line PARAMS((FILE_HANDLE *handle, COLOUR *line_data, int line_number));
int Read_Raw_Line PARAMS((FILE_HANDLE *handle, COLOUR *line_data, int *line_number));
void Close_Raw_File PARAMS((FILE_HANDLE *handle));

