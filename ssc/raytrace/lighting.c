/****************************************************************************
*                   lighting.c
*
*  This module calculates lighting properties like ambient, diffuse, specular,
*  reflection, refraction, etc.
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

extern int Trace_Level;
extern FRAME Frame;
extern OBJECT *Root_Object;
extern unsigned int Options;
extern int Use_Slabs;
extern unsigned long Quality_Flags;
extern int Shadow_Test_Flag;
extern long Shadow_Ray_Tests, Shadow_Rays_Succeeded, Shadow_Cache_Hits;
extern long Reflected_Rays_Traced, Refracted_Rays_Traced;
extern long Transmitted_Rays_Traced;

extern short *hashTable;
extern unsigned short crctab[256];
#define rand3d(a,b) crctab[(int)(hashTable[(int)(hashTable[(int)((a)&0xfff)]^(b))&0xfff])&0xff]

#define COORDINATE_LIMIT 1.0e17
#define BLACK_LEVEL 0.003

/* "Small_Tolerance" is just too tight for higher order polynomial equations.
   this value should probably be a variable of some sort, but for now just
   use a reasonably small value.  If people render real small objects real
   close to each other then there may be some shading problems.  Otherwise
   having SHADOW_TOLERANCE as large as this won't affect images. */
#define SHADOW_TOLERANCE 1.0e-3

static void do_light PARAMS((LIGHT_SOURCE *Light_Source,
DBL *Light_Source_Depth, RAY *Light_Source_Ray, VECTOR *IPoint,
COLOUR *Light_Colour));
static int do_blocking PARAMS((INTERSECTION *Local_Intersection,
COLOUR *Light_Colour, ISTACK *Local_Stack));
static void do_phong PARAMS((FINISH *Finish, RAY *Light_Source_Ray,
VECTOR *Eye, VECTOR *Layer_Normal, COLOUR *Colour, COLOUR *Light_Colour,
COLOUR *Layer_Pigment_Colour));
static void do_specular PARAMS((FINISH *Finish, RAY *Light_Source_Ray,
VECTOR *REye, VECTOR *Layer_Normal, COLOUR *Colour, COLOUR *Light_Colour,
COLOUR *Layer_Pigment_Colour));
static void do_diffuse PARAMS((FINISH *Finish, RAY *Light_Source_Ray,
VECTOR *Layer_Normal, COLOUR *Colour, COLOUR *Light_Colour,
COLOUR *Layer_Pigment_Colour, DBL Attenuation));
static void Block_Area_Light PARAMS((LIGHT_SOURCE *Light_Source,
DBL Light_Source_Depth, RAY *Light_Source_Ray_Ptr, VECTOR *IPoint,
COLOUR *Light_Colour, int u1, int v1, int u2, int v2, int Level));
static void Block_Point_Light PARAMS((LIGHT_SOURCE *Light_Source,
DBL Light_Source_Depth, RAY *Light_Source_Ray_Ptr, COLOUR *Light_Colour));

static void Block_Point_Light (Light_Source, Light_Source_Depth, Light_Source_Ray_Ptr, Light_Colour)
LIGHT_SOURCE *Light_Source;
DBL Light_Source_Depth;
RAY *Light_Source_Ray_Ptr;
COLOUR *Light_Colour;
  {
  OBJECT *Blocking_Object;
  int Quit_Looking, Not_Found_Shadow, Cache_Me;
  INTERSECTION *Local_Intersection, Bounded_Intersection;
  ISTACK *Local_Stack;
  RAY Local_Ray;
  RAY Light_Source_Ray;

  Light_Source_Ray = *Light_Source_Ray_Ptr;

  Local_Stack = open_istack ();
  Quit_Looking = FALSE;

  /* Test the cached object first */
  /* Made changes so that semi-transparent objects never get cached */
  if (Light_Source->Shadow_Cached_Object != NULL) 
    {
    Shadow_Ray_Tests++;

    if (Ray_In_Bounds (&Light_Source_Ray, Light_Source->Shadow_Cached_Object->Bound))
      {
      if (All_Intersections (Light_Source->Shadow_Cached_Object, &Light_Source_Ray, Local_Stack))
        while ((Local_Intersection=pop_entry(Local_Stack)) != NULL)
          {
          if ((!Local_Intersection->Object->No_Shadow_Flag) && 
            (Local_Intersection->Depth < Light_Source_Depth-Small_Tolerance) && 
            (Local_Intersection->Depth > SHADOW_TOLERANCE))
            if (do_blocking(Local_Intersection, Light_Colour, Local_Stack))
              {
              Quit_Looking = TRUE;
              Shadow_Cache_Hits++;
              break;
              }
          }
      }
    }

  if (Quit_Looking) 
    {
    close_istack (Local_Stack);
    return;
    }

  Not_Found_Shadow = TRUE;   
  Cache_Me = FALSE;
  if (!Use_Slabs)
    {
    for (Blocking_Object = Frame.Objects;
    Blocking_Object != NULL;
    Blocking_Object = Blocking_Object->Sibling)
      {
      if (Blocking_Object == Light_Source->Shadow_Cached_Object)
        continue;

      Shadow_Ray_Tests++;

      if (!Ray_In_Bounds (&Light_Source_Ray, Blocking_Object->Bound))
        continue;

      if (!All_Intersections (Blocking_Object, &Light_Source_Ray, Local_Stack))
        continue;

      while ((Local_Intersection = pop_entry(Local_Stack)) != NULL)
        if ((!Local_Intersection->Object->No_Shadow_Flag) && 
          (Local_Intersection->Depth < Light_Source_Depth-Small_Tolerance) && 
          (Local_Intersection->Depth > SHADOW_TOLERANCE))
          {
          if (do_blocking(Local_Intersection, Light_Colour, Local_Stack))
            {
            Cache_Me = Not_Found_Shadow;
            Quit_Looking = TRUE;
            break; /* from while */
            }
          Not_Found_Shadow = FALSE;
          }
      if (Quit_Looking)
        break; /* from for */
      }
    }
  else   /* Use bounding slabs to look for shadows */
    {
    Local_Ray = Light_Source_Ray;
    while (!Quit_Looking) 
      {
      Shadow_Ray_Tests++;
      Local_Ray.Quadric_Constants_Cached = 0;
      Bounded_Intersection.Depth = Light_Source_Depth;
      if (Bounds_Intersect(Root_Object, &Local_Ray, &Bounded_Intersection,
        &Blocking_Object)) 
        {
        if (Bounded_Intersection.Depth > Light_Source_Depth) 
          break; /* Intersection was beyond the light */

        if (!(Bounded_Intersection.Object->No_Shadow_Flag))
          if (Blocking_Object != Light_Source->Shadow_Cached_Object)
            {
            Shadow_Rays_Succeeded++;
            Filter_Shadow_Ray(&Bounded_Intersection, Light_Colour);
            if ((fabs(Light_Colour->Red) < BLACK_LEVEL) &&
              (fabs(Light_Colour->Green) < BLACK_LEVEL) &&
              (fabs(Light_Colour->Blue) < BLACK_LEVEL))
              {
              Cache_Me = Not_Found_Shadow;
              Quit_Looking = TRUE;
              break; /* from while */
              }
            }
        /* Move the ray to the point of intersection, plus some */
        Light_Source_Depth -= Bounded_Intersection.Depth;
        Local_Ray.Initial = Bounded_Intersection.IPoint;
        Not_Found_Shadow = FALSE;
        }
      else /* No intersections in the direction of the ray */
        {
        break;
        }
      } /*endwhile*/
    } /*endelse*/
  if (Cache_Me) 
    Light_Source->Shadow_Cached_Object = Blocking_Object;
  close_istack (Local_Stack);
  }


static void Block_Area_Light (Light_Source, Light_Source_Depth,
Light_Source_Ray_Ptr,IPoint, Light_Colour, u1, v1, u2, v2, Level)
LIGHT_SOURCE *Light_Source;
DBL Light_Source_Depth;
RAY *Light_Source_Ray_Ptr;
VECTOR *IPoint;
COLOUR *Light_Colour;
int u1, v1, u2, v2, Level;
  {
  COLOUR Sample_Colour[4], Dummy_Colour;
  VECTOR Center_Save, NewAxis1, NewAxis2;
  int i, j, u, v, New_u1, New_v1, New_u2, New_v2;

  DBL Jitter_u, Jitter_v, ScaleFactor;
  RAY Light_Source_Ray;

  Light_Source_Ray = *Light_Source_Ray_Ptr;

  /* First call, initialize */
  if (u1 == 0 && v1 == 0 && u2 == 0 && v2 == 0) 
    {
    /* Flag uncalculated points with a negative value for Red */
    for (i = 0; i < Light_Source->Area_Size1; i++) 
      {
      for (j = 0; j < Light_Source->Area_Size2; j++)
        Light_Source->Light_Grid[i][j].Red = -1.0;
      }

    u1 = 0;
    v1 = 0;
    u2 = Light_Source->Area_Size1 - 1;
    v2 = Light_Source->Area_Size2 - 1;
    }

  /* Save the light source center since we'll be fiddling with it */
  Center_Save=Light_Source->Center;

  /* Sample the four corners of the region */
  for (i = 0; i < 4; i++) 
    {
    switch (i) 
    {
    case 0: 
      u = u1; v = v1; break;
    case 1: 
      u = u2; v = v1; break;
    case 2: 
      u = u1; v = v2; break;
    case 3: 
      u = u2; v = v2; break;
    }

    if (Light_Source -> Light_Grid[u][v].Red >= 0.0)
      /* We've already calculated this point, reuse it */
      Sample_Colour[i]=Light_Source->Light_Grid[u][v];
    else 
      {
      Jitter_u = (DBL)u;
      Jitter_v = (DBL)v;

      if (Light_Source -> Jitter) 
        {
        Jitter_u += (rand() % 4096)/4096.0 - 0.5;
        Jitter_v += (rand() % 4096)/4096.0 - 0.5;
        /*
  Not sure if    jx = IPoint->x + 100*u;
  this works    jy = IPoint->y + 100*v;
  yet
        Jitter_u += ((rand3d(jx, jy) & 0x7FFF)/32768.0) - 0.5;
        Jitter_v += ((rand3d(jx+10, jy+10) & 0x7FFF)/32768.0) - 0.5;
*/
        }

      if (Light_Source -> Area_Size1 > 1) 
        {
        ScaleFactor = Jitter_u/(DBL)(Light_Source->Area_Size1 - 1) - 0.5;
        VScale (NewAxis1, Light_Source->Axis1, ScaleFactor)
          }
      else
        Make_Vector (&NewAxis1, 0.0, 0.0, 0.0);

      if (Light_Source -> Area_Size2 > 1) 
        {
        ScaleFactor = Jitter_v/(DBL)(Light_Source->Area_Size2 - 1) - 0.5;
        VScale (NewAxis2, Light_Source->Axis2, ScaleFactor)
        }
      else
        Make_Vector (&NewAxis2, 0.0, 0.0, 0.0);

      Light_Source->Center=Center_Save;
      VAdd  (Light_Source->Center, Light_Source->Center, NewAxis1);
      VAdd  (Light_Source->Center, Light_Source->Center, NewAxis2);

      /* Recalculate the light source ray but not the colour */
      do_light (Light_Source, &Light_Source_Depth, &Light_Source_Ray,
        IPoint, &Dummy_Colour);

      Sample_Colour[i]=*Light_Colour;

      Block_Point_Light (Light_Source, Light_Source_Depth,
        &Light_Source_Ray, &Sample_Colour[i]);

      Light_Source->Light_Grid[u][v]=Sample_Colour[i];
      }
    }

  Light_Source->Center=Center_Save;

  if ( (u2 - u1 > 1 || v2 - v1 > 1) && (Level < Light_Source -> Adaptive_Level ||
    Colour_Distance (&Sample_Colour[0], &Sample_Colour[1]) > 0.1 ||
    Colour_Distance (&Sample_Colour[1], &Sample_Colour[3]) > 0.1 ||
    Colour_Distance (&Sample_Colour[3], &Sample_Colour[2]) > 0.1 ||
    Colour_Distance (&Sample_Colour[2], &Sample_Colour[0]) > 0.1) )
    {
    for (i = 0; i < 4; i++) 
      {
      switch (i) 
      {
      case 0: 
        New_u1 = u1;
        New_v1 = v1;
        New_u2 = (int)floor ((u1 + u2)/2.0);
        New_v2 = (int)floor ((v1 + v2)/2.0);
        break;

      case 1: 
        New_u1 = (int)ceil  ((u1 + u2)/2.0);
        New_v1 = v1;
        New_u2 = u2;
        New_v2 = (int)floor ((v1 + v2)/2.0);
        break;

      case 2: 
        New_u1 = u1;
        New_v1 = (int)ceil  ((v1 + v2)/2.0);
        New_u2 = (int)floor ((u1 + u2)/2.0);
        New_v2 = v2;
        break;

      case 3: 
        New_u1 = (int)ceil ((u1 + u2)/2.0);
        New_v1 = (int)ceil ((v1 + v2)/2.0);
        New_u2 = u2;
        New_v2 = v2;
        break;
      }

      /* Recalculate the light source ray but not the colour */
      do_light (Light_Source, &Light_Source_Depth, &Light_Source_Ray,
        IPoint, &Dummy_Colour);

      Sample_Colour[i]=*Light_Colour;

      Block_Area_Light (Light_Source, Light_Source_Depth,
        &Light_Source_Ray, IPoint,
        &Sample_Colour[i],
        New_u1, New_v1, New_u2, New_v2, Level+1);
      }
    }

  /* Add up the light contributions */
  Make_Colour (Light_Colour, 0.0, 0.0, 0.0);

  for (i = 0; i < 4; i++) 
    {
    Scale_Colour (&Sample_Colour[i], &Sample_Colour[i], 0.25);
    Add_Colour (Light_Colour, Light_Colour, &Sample_Colour[i]);
    }
  }

static void do_light(Light_Source, Light_Source_Depth, Light_Source_Ray, IPoint, Light_Colour)
LIGHT_SOURCE *Light_Source;
DBL *Light_Source_Depth;
RAY *Light_Source_Ray;
VECTOR *IPoint;
COLOUR *Light_Colour;
  {
  DBL Attenuation = 1.0;

  /* Get the light source colour. */
  *Light_Colour = Light_Source->Colour;

  Light_Source_Ray->Initial = *IPoint;
  Light_Source_Ray->Quadric_Constants_Cached = FALSE;

  VSub (Light_Source_Ray->Direction,
    Light_Source->Center,
    *IPoint);

  VLength (*Light_Source_Depth, Light_Source_Ray->Direction);

  VScale (Light_Source_Ray->Direction, Light_Source_Ray->Direction,
    1.0/(*Light_Source_Depth));

  Attenuation = Attenuate_Light(Light_Source, Light_Source_Ray);

  /* Now scale the color by the attenuation */
  Light_Colour->Red   *= Attenuation;
  Light_Colour->Green *= Attenuation;
  Light_Colour->Blue  *= Attenuation;


  return;
  }

static int do_blocking(Local_Intersection, Light_Colour, Local_Stack)
INTERSECTION *Local_Intersection;
COLOUR *Light_Colour;
ISTACK *Local_Stack;
  {
  Shadow_Rays_Succeeded++;

  Filter_Shadow_Ray (Local_Intersection, Light_Colour);

  if ((fabs(Light_Colour->Red) < BLACK_LEVEL) && 
    (fabs(Light_Colour->Green) < BLACK_LEVEL) && 
    (fabs(Light_Colour->Blue) < BLACK_LEVEL)) 
    {
    while ((Local_Intersection = pop_entry(Local_Stack)) != NULL)
      {
      }
    return(TRUE);
    }
  return(FALSE);
  }

static void do_phong(Finish, Light_Source_Ray, Eye, Layer_Normal, Colour, Light_Colour, Layer_Pigment_Colour)
FINISH *Finish;
RAY *Light_Source_Ray;
VECTOR *Layer_Normal, *Eye;
COLOUR *Colour, *Light_Colour, *Layer_Pigment_Colour;
  {
  DBL Cos_Angle_Of_Incidence, Normal_Length, Intensity;
  VECTOR Local_Normal, Normal_Projection, Reflect_Direction;

  VDot(Cos_Angle_Of_Incidence, *Eye, *Layer_Normal);

  if (Cos_Angle_Of_Incidence < 0.0)
    {
    Local_Normal = *Layer_Normal;
    Cos_Angle_Of_Incidence = -Cos_Angle_Of_Incidence;
    }
  else
    {
    VScale (Local_Normal, *Layer_Normal, -1.0);
    }

  VScale (Normal_Projection, Local_Normal, Cos_Angle_Of_Incidence);
  VScale (Normal_Projection, Normal_Projection, 2.0);
  VAdd (Reflect_Direction, *Eye, Normal_Projection);

  VDot (Cos_Angle_Of_Incidence, Reflect_Direction, Light_Source_Ray->Direction);
  VLength (Normal_Length, Light_Source_Ray->Direction);

  if (Normal_Length == 0.0)
    Cos_Angle_Of_Incidence = 0.0;
  else 
    Cos_Angle_Of_Incidence /= Normal_Length;

  if (Cos_Angle_Of_Incidence < 0.0)
    Cos_Angle_Of_Incidence = 0;

  if (Finish->Phong_Size != 1.0)
    Intensity = pow(Cos_Angle_Of_Incidence, Finish->Phong_Size);
  else
    Intensity = Cos_Angle_Of_Incidence;

  Intensity *= Finish->Phong;

  if (Finish->Metallic_Flag) 
    {
    Colour->Red+=Intensity*(Layer_Pigment_Colour->Red)*(Light_Colour->Red);    
    Colour->Green+=Intensity*(Layer_Pigment_Colour->Green)*(Light_Colour->Green);
    Colour->Blue+=Intensity*(Layer_Pigment_Colour->Blue)*(Light_Colour->Blue);  
    }
  else 
    {
    Colour->Red+=Intensity*(Light_Colour->Red);
    Colour->Green+=Intensity*(Light_Colour->Green);
    Colour->Blue+=Intensity*(Light_Colour->Blue);  
    }
  }

static void do_specular(Finish, Light_Source_Ray, REye, Layer_Normal, Colour, Light_Colour, Layer_Pigment_Colour)
FINISH *Finish;
RAY *Light_Source_Ray;
VECTOR *Layer_Normal, *REye;
COLOUR *Colour, *Light_Colour, *Layer_Pigment_Colour;
  {
  DBL Cos_Angle_Of_Incidence, Normal_Length, Intensity, Halfway_Length;
  VECTOR Halfway;

  VHalf (Halfway, *REye, Light_Source_Ray->Direction);
  VLength (Normal_Length, *Layer_Normal);
  VLength (Halfway_Length, Halfway);
  VDot (Cos_Angle_Of_Incidence, Halfway, *Layer_Normal);

  if (Normal_Length == 0.0 || Halfway_Length == 0.0)
    Cos_Angle_Of_Incidence = 0.0;
  else
    Cos_Angle_Of_Incidence /= (Normal_Length * Halfway_Length);

  if (Cos_Angle_Of_Incidence < 0.0)
    Cos_Angle_Of_Incidence = 0.0;


  if (Finish->Roughness != 1.0)
    Intensity = pow(Cos_Angle_Of_Incidence, Finish->Roughness);
  else
    Intensity = Cos_Angle_Of_Incidence;

  Intensity *= Finish->Specular;
  if (Finish->Metallic_Flag) 
    {
    Colour->Red+=Intensity*(Layer_Pigment_Colour->Red)*(Light_Colour->Red);    
    Colour->Green+=Intensity*(Layer_Pigment_Colour->Green)*(Light_Colour->Green);
    Colour->Blue+=Intensity*(Layer_Pigment_Colour->Blue)*(Light_Colour->Blue);  
    }
  else 
    {
    Colour->Red+=Intensity*(Light_Colour->Red);
    Colour->Green+=Intensity*(Light_Colour->Green);
    Colour->Blue+=Intensity*(Light_Colour->Blue);
    }
  }

static void do_diffuse(Finish, Light_Source_Ray, Layer_Normal, Colour, Light_Colour, Layer_Pigment_Colour, Attenuation)
FINISH *Finish;
RAY *Light_Source_Ray;
VECTOR *Layer_Normal;
COLOUR *Colour, *Light_Colour, *Layer_Pigment_Colour;
DBL Attenuation;
  {
  DBL Cos_Angle_Of_Incidence, Intensity;

  VDot (Cos_Angle_Of_Incidence, *Layer_Normal, Light_Source_Ray->Direction);
  if (Cos_Angle_Of_Incidence < 0.0)
    Cos_Angle_Of_Incidence = -Cos_Angle_Of_Incidence;

  if (Finish->Brilliance != 1.0)
    Intensity = pow(Cos_Angle_Of_Incidence, Finish->Brilliance);
  else
    Intensity = Cos_Angle_Of_Incidence;

  Intensity *= Finish->Diffuse * Attenuation;

  if (Finish->Crand > 0.0)
    Intensity -= ((rand()&0x7FFF)/(DBL) 0x7FFF) * Finish->Crand;

  Colour->Red += Intensity * (Layer_Pigment_Colour->Red) * (Light_Colour->Red);
  Colour->Green += Intensity * (Layer_Pigment_Colour->Green) * (Light_Colour->Green);
  Colour->Blue += Intensity * (Layer_Pigment_Colour->Blue) * (Light_Colour->Blue);
  return;
  }

/* Given a 3d point and a pigment, accumulate colour from that layer */
/* Formerly called "Colour_At" */
void Add_Pigment (Colour, Pigment, IPoint)
COLOUR *Colour;
PIGMENT *Pigment;
VECTOR *IPoint;
  {
  register DBL x, y, z;
  VECTOR TPoint,PTurbulence;

  if (Pigment->Trans != NULL) 
    MInvTransPoint (&TPoint, IPoint, Pigment->Trans);
  else 
    TPoint = *IPoint;

  x = TPoint.x;
  y = TPoint.y;
  z = TPoint.z;
  if(Pigment->Type != WOOD_PIGMENT &&
    Pigment->Type != MARBLE_PIGMENT &&
    Pigment->Type != NO_PIGMENT &&
    Pigment->Type != COLOUR_PIGMENT &&
    /*      Pigment->Type != SPOTTED_PIGMENT && */ /*maybe?*/
    Pigment->Flags & HAS_TURB)
    {
    DTurbulence (&PTurbulence, x, y, z,
      Pigment->omega,Pigment->lambda,Pigment->Octaves);
    x += PTurbulence.x * Pigment->Turbulence.x;
    y += PTurbulence.y * Pigment->Turbulence.y;
    z += PTurbulence.z * Pigment->Turbulence.z;
    }


  if (x > COORDINATE_LIMIT)
    x = COORDINATE_LIMIT;
  else
    if (x < -COORDINATE_LIMIT)
      x = -COORDINATE_LIMIT;

  if (y > COORDINATE_LIMIT)
    y = COORDINATE_LIMIT;
  else
    if (y < -COORDINATE_LIMIT)
      y = -COORDINATE_LIMIT;

  if (z > COORDINATE_LIMIT)
    z = COORDINATE_LIMIT;
  else
    if (z < -COORDINATE_LIMIT)
      z = -COORDINATE_LIMIT;

  switch (Pigment->Type) 
  {
  case NO_PIGMENT:
    /* No colouring pigment has been specified - make it black. */
    Make_Colour (Colour, 0.0, 0.0, 0.0);
    Colour -> Filter  = 0.0;
    break;

  case COLOUR_PIGMENT:
    Colour -> Red += Pigment->Colour1->Red;
    Colour -> Green += Pigment->Colour1->Green;
    Colour -> Blue += Pigment->Colour1->Blue;
    Colour -> Filter += Pigment->Colour1->Filter;
    break;

  case BOZO_PIGMENT: 
    bozo (x, y, z, Pigment, Colour);
    break;

  case MARBLE_PIGMENT:
    marble (x, y, z, Pigment, Colour);
    break;

  case WOOD_PIGMENT:
    wood (x, y, z, Pigment, Colour);
    break;

  case CHECKER_PIGMENT:
    checker (x, y, z, Pigment, Colour);
    break;

  case SPOTTED_PIGMENT:
    spotted (x, y, z, Pigment, Colour);
    break;

  case AGATE_PIGMENT:
    agate (x, y, z, Pigment, Colour);
    break;

  case GRANITE_PIGMENT:
    granite (x, y, z, Pigment, Colour);
    break;

  case GRADIENT_PIGMENT:
    gradient (x, y, z, Pigment, Colour);
    break;

  case HEXAGON_PIGMENT:
    hexagon (x, y, z, Pigment, Colour);
    break;

  case RADIAL_PIGMENT:
    radial (x, y, z, Pigment, Colour);
    break;

  case MANDEL_PIGMENT:
    mandel (x, y, z, Pigment, Colour);
    break;

  case IMAGE_MAP_PIGMENT:
    image_map (x, y, z, Pigment, Colour);
    break;

  case ONION_PIGMENT:
    onion (x, y, z, Pigment, Colour);
    break;

  case LEOPARD_PIGMENT:
    leopard (x, y, z, Pigment, Colour);
    break;

  case PAINTED1_PIGMENT:
    painted1 (x, y, z, Pigment, Colour);
    break;

  case PAINTED2_PIGMENT:
    painted2 (x, y, z, Pigment, Colour);
    break;

  case PAINTED3_PIGMENT:
    painted3 (x, y, z, Pigment, Colour);
    break;
  }
  }


void Perturb_Normal(Layer_Normal, Tnormal, IPoint)
VECTOR *Layer_Normal, *IPoint;
TNORMAL *Tnormal;
  {
  register DBL x, y, z;
  VECTOR TPoint,NTurbulence;

  if (Tnormal->Trans != NULL) 
    MInvTransPoint (&TPoint, IPoint, Tnormal->Trans);
  else 
    TPoint = *IPoint;

  x = TPoint.x;
  y = TPoint.y;
  z = TPoint.z;

  if(Tnormal->Flags && HAS_TURB)
    {
    DTurbulence (&NTurbulence, x, y, z,
      Tnormal->omega,Tnormal->lambda,Tnormal->Octaves);
    x += NTurbulence.x * Tnormal->Turbulence.x;
    y += NTurbulence.y * Tnormal->Turbulence.y;
    z += NTurbulence.z * Tnormal->Turbulence.z;
    }


  switch (Tnormal->Type) 
  {

  case WAVES: 
    waves (x, y, z, Tnormal, Layer_Normal);
    break;

  case RIPPLES: 
    ripples (x, y, z, Tnormal, Layer_Normal);
    break;

  case WRINKLES: 
    wrinkles (x, y, z, Tnormal, Layer_Normal);
    break;

  case BUMPS: 
    bumps (x, y, z, Tnormal, Layer_Normal);
    break;

  case DENTS: 
    dents (x, y, z, Tnormal, Layer_Normal);
    break; 

  case BUMPY1: 
    bumpy1 (x, y, z, Tnormal, Layer_Normal);
    break;

  case BUMPY2: 
    bumpy2 (x, y, z, Tnormal, Layer_Normal);
    break;

  case BUMPY3: 
    bumpy3 (x, y, z, Tnormal, Layer_Normal);
    break;

  case BUMP_MAP: 
    bump_map (x, y, z, Tnormal, Layer_Normal);
    break;
  }
  return;
  }

void Diffuse (Finish, IPoint, Eye, Layer_Normal, Layer_Pigment_Colour, Colour, Attenuation)
FINISH *Finish;
VECTOR *IPoint, *Layer_Normal;
COLOUR *Layer_Pigment_Colour;
COLOUR *Colour;
RAY    *Eye;
DBL    Attenuation;
  {
  DBL Light_Source_Depth, Cos_Shadow_Angle;
  RAY Light_Source_Ray;
  LIGHT_SOURCE *Light_Source;
  VECTOR REye;
  COLOUR Light_Colour;

  if ((Finish->Diffuse == 0.0) && (Finish->Specular == 0.0) && (Finish->Phong == 0.0))
    return;

  if (Finish->Specular != 0.0)
    {
    REye.x = -Eye->Direction.x;
    REye.y = -Eye->Direction.y;
    REye.z = -Eye->Direction.z;
    }

  for (Light_Source = Frame.Light_Sources ; 
  Light_Source != NULL;
  Light_Source = Light_Source->Next_Light_Source)
    {
    /* Get a colour and a ray */

    do_light(Light_Source,       &Light_Source_Depth, 
      &Light_Source_Ray,  IPoint,
      &Light_Colour);

    /* Don't calculate spotlights when outside of the light's cone */
    if (fabs(Light_Colour.Red) < BLACK_LEVEL && 
      fabs(Light_Colour.Green) < BLACK_LEVEL && 
      fabs(Light_Colour.Blue) < BLACK_LEVEL)
      continue;

    /* See if light on far side of surface from camera. */
    VDot(Cos_Shadow_Angle,*Layer_Normal,Light_Source_Ray.Direction);

    if (Cos_Shadow_Angle < 0.0)
      continue;

    /* If light source was not blocked by any intervening object, then
      calculate it's contribution to the object's overall illumination */

    Shadow_Test_Flag = TRUE;
    if (Quality_Flags & Q_SHADOW)
      {
      if ((Light_Source->Area_Light) && (Quality_Flags & Q_AREA_LIGHT))
        Block_Area_Light (Light_Source, Light_Source_Depth,
          &Light_Source_Ray, IPoint,
          &Light_Colour, 0, 0, 0, 0, 0);
      else
        Block_Point_Light (Light_Source, Light_Source_Depth,
          &Light_Source_Ray, &Light_Colour);
      }
    Shadow_Test_Flag = FALSE;

    if (fabs(Light_Colour.Red)  > BLACK_LEVEL || 
      fabs(Light_Colour.Green) > BLACK_LEVEL || 
      fabs(Light_Colour.Blue) > BLACK_LEVEL) 
      {
      if (Finish->Phong > 0.0) 
        do_phong(Finish,&Light_Source_Ray,&Eye->Direction,Layer_Normal,Colour,&Light_Colour, Layer_Pigment_Colour);

      if (Finish->Specular > 0.0) 
        do_specular(Finish,&Light_Source_Ray,&REye,Layer_Normal,Colour,&Light_Colour, Layer_Pigment_Colour);

      if (Finish->Diffuse > 0.0) 
        do_diffuse(Finish,&Light_Source_Ray,Layer_Normal,Colour,&Light_Colour,Layer_Pigment_Colour, Attenuation);
      }
    }
  return;
  }

void Reflect (Reflection, IPoint, Ray, Layer_Normal, Colour)
DBL Reflection;
VECTOR *IPoint;
RAY *Ray;
VECTOR *Layer_Normal;
COLOUR *Colour;
  {
  RAY New_Ray;
  COLOUR Temp_Colour;
  VECTOR Local_Normal;
  VECTOR Normal_Projection;
  VECTOR Surface_Offset;
  register DBL Normal_Component;

  if (Reflection != 0.0)
    {
    Reflected_Rays_Traced++;
    VDot (Normal_Component, Ray -> Direction, *Layer_Normal);
    if (Normal_Component < 0.0) 
      {
      Local_Normal = *Layer_Normal;
      Normal_Component *= -1.0;
      }
    else
      VScale (Local_Normal, *Layer_Normal, -1.0);

    VScale (Normal_Projection, Local_Normal, Normal_Component);
    VScale (Normal_Projection, Normal_Projection, 2.0);
    VAdd (New_Ray.Direction, Ray -> Direction, Normal_Projection);
    New_Ray.Initial = *IPoint;

    /* ARE 08/25/91 */

    VScale(Surface_Offset, New_Ray.Direction, 2.0 * Small_Tolerance); 
    VAdd(New_Ray.Initial, New_Ray.Initial, Surface_Offset);           

    Copy_Ray_Containers (&New_Ray, Ray);
    Trace_Level++;
    Make_Colour (&Temp_Colour, 0.0, 0.0, 0.0);
    New_Ray.Quadric_Constants_Cached = FALSE;
    Trace (&New_Ray, &Temp_Colour);
    Trace_Level--;

    Colour -> Red   += Temp_Colour.Red   * Reflection;
    Colour -> Green += Temp_Colour.Green * Reflection;
    Colour -> Blue  += Temp_Colour.Blue  * Reflection;

    }
  }

void Refract (Texture, IPoint, Ray, Top_Normal, Colour)
TEXTURE *Texture;
VECTOR *IPoint;
RAY *Ray;
VECTOR *Top_Normal;
COLOUR *Colour;
  {
  RAY New_Ray;
  COLOUR Temp_Colour;
  VECTOR Local_Normal;
  VECTOR Ray_Direction;
  register DBL Normal_Component, Temp_IOR;
  DBL temp, ior;
  /*   int inside; */

  if (Top_Normal == NULL) 
    {
    New_Ray.Initial = *IPoint;
    New_Ray.Direction = Ray->Direction;

    Copy_Ray_Containers (&New_Ray, Ray);
    Trace_Level++;
    Transmitted_Rays_Traced++;
    Make_Colour (&Temp_Colour, 0.0, 0.0, 0.0);
    New_Ray.Quadric_Constants_Cached = FALSE;
    Trace (&New_Ray, &Temp_Colour);
    Trace_Level--;
    (Colour -> Red) += Temp_Colour.Red;
    (Colour -> Green) += Temp_Colour.Green;
    (Colour -> Blue) += Temp_Colour.Blue;
    }
  else 
    {
    Refracted_Rays_Traced++;
    VDot (Normal_Component, Ray -> Direction, *Top_Normal);
    if (Normal_Component <= 0.0)
      {
      Local_Normal.x = Top_Normal -> x;
      Local_Normal.y = Top_Normal -> y;
      Local_Normal.z = Top_Normal -> z;
      Normal_Component *= -1.0;
      /*     inside = FALSE;*/
      }
    else
      {
      VScale (Local_Normal, *Top_Normal, -1.0);
      /*     inside = TRUE;*/
      }


    Copy_Ray_Containers (&New_Ray, Ray);

    if (Ray -> Containing_Index == -1)
      {
      /* The ray is entering from the atmosphere */
      Ray_Enter (&New_Ray, Texture);
      ior = (Frame.Atmosphere_IOR)/(Texture->Finish->Index_Of_Refraction);
      }
else
  {
  /* The ray is currently inside an object */
  if (New_Ray.Containing_Textures [New_Ray.Containing_Index] == Texture) 
    /*         if (inside) */
    {
    /* The ray is leaving the current object */
    Ray_Exit (&New_Ray);
    if (New_Ray.Containing_Index == -1)
      /* The ray is leaving into the atmosphere */
    Temp_IOR = Frame.Atmosphere_IOR;
    else
      /* The ray is leaving into another object */
      Temp_IOR = New_Ray.Containing_IORs [New_Ray.Containing_Index];

    ior =  (Texture->Finish->Index_Of_Refraction)/Temp_IOR;
    }
    else
      {
      /* The ray is entering a new object */
      Temp_IOR = New_Ray.Containing_IORs [New_Ray.Containing_Index];
      Ray_Enter (&New_Ray, Texture);

      ior =  Temp_IOR / (Texture->Finish->Index_Of_Refraction);
      }
    }

  temp = 1.0 + ior * ior * (Normal_Component * Normal_Component - 1.0);
  if (temp < 0.0) 
      {
    Reflect ((1.0 - Texture->Finish->Reflection), IPoint, 
    Ray, Top_Normal, Colour);
    return;
    }

  temp = ior*Normal_Component - sqrt(temp);
  VScale (Local_Normal, Local_Normal, temp);
  VScale (Ray_Direction, Ray->Direction, ior);
  VAdd (New_Ray.Direction, Local_Normal, Ray_Direction);
  VNormalize (New_Ray.Direction, New_Ray.Direction);

  New_Ray.Initial = *IPoint;
  Trace_Level++;
  Make_Colour (&Temp_Colour, 0.0, 0.0, 0.0);
  New_Ray.Quadric_Constants_Cached = FALSE;

  Trace (&New_Ray, &Temp_Colour);
  Trace_Level--;

  (Colour -> Red) += (Temp_Colour.Red)
    * (Texture -> Finish->Refraction);
  (Colour -> Green) += (Temp_Colour.Green)
    * (Texture -> Finish->Refraction);
  (Colour -> Blue) += (Temp_Colour.Blue)
    * (Texture -> Finish->Refraction);
  }
}

void Fog (Distance, Fog_Colour, Fog_Distance, Colour)
DBL Distance, Fog_Distance;
COLOUR *Fog_Colour, *Colour;
  {
  DBL Fog_Factor, Fog_Factor_Inverse;

  Fog_Factor = exp(-1.0 * Distance/Fog_Distance);
  Fog_Factor_Inverse = 1.0 - Fog_Factor;
  Colour->Red = Colour->Red*Fog_Factor + Fog_Colour->Red*Fog_Factor_Inverse;
  Colour->Green = Colour->Green*Fog_Factor + Fog_Colour->Green*Fog_Factor_Inverse;
  Colour->Blue = Colour->Blue*Fog_Factor + Fog_Colour->Blue*Fog_Factor_Inverse;
  }

void Compute_Reflected_Colour (Ray, Finish, Ray_Intersection, Layer_Pigment_Colour, Filter_Colour, Colour, Layer_Normal)
RAY *Ray;
FINISH *Finish;
INTERSECTION *Ray_Intersection;
COLOUR *Layer_Pigment_Colour;
COLOUR *Filter_Colour;
COLOUR *Colour;
VECTOR *Layer_Normal;
  {
  DBL Attenuation, Ambient;
  COLOUR Emitted_Colour;

  /* This variable keeps track of how much colour comes from the surface
      of the object and how much is transmited through. */
  Make_Colour (&Emitted_Colour, 0.0, 0.0, 0.0);

  if (Quality_Flags & Q_FULL_AMBIENT) 
    {
    Layer_Pigment_Colour->Filter = 0.0;

    Colour->Red   += Layer_Pigment_Colour->Red * Filter_Colour->Filter;
    Colour->Green += Layer_Pigment_Colour->Green * Filter_Colour->Filter;
    Colour->Blue  += Layer_Pigment_Colour->Blue * Filter_Colour->Filter;
    return;
    }

  Attenuation = Filter_Colour->Filter * (1.0 - Layer_Pigment_Colour->Filter);

  if ((Ambient = Finish->Ambient*Attenuation) != 0.0)
    {
    Emitted_Colour.Red += Layer_Pigment_Colour->Red * Ambient;
    Emitted_Colour.Green += Layer_Pigment_Colour->Green * Ambient;
    Emitted_Colour.Blue += Layer_Pigment_Colour->Blue * Ambient;
    }

  Diffuse (Finish, &Ray_Intersection ->IPoint, Ray,
    Layer_Normal, Layer_Pigment_Colour, &Emitted_Colour, Attenuation);

  Colour->Red   += Emitted_Colour.Red;
  Colour->Green += Emitted_Colour.Green;
  Colour->Blue  += Emitted_Colour.Blue;

  if (Quality_Flags & Q_REFLECT)
    Reflect (Finish->Reflection, &Ray_Intersection -> IPoint, Ray,
      Layer_Normal, Colour); 
  }

/* Given an intersection point, a ray, & a shadow flag, add that point's
  color to the given colour and return it. */

void Determine_Apparent_Colour (Ray_Intersection, Colour, Ray)
INTERSECTION *Ray_Intersection;
COLOUR *Colour;
RAY *Ray;
  {
  COLOUR Layer_Pigment_Colour, Refracted_Colour, Filter_Colour;
  TEXTURE *Layer, *Texture;
  FINISH *Finish;
  VECTOR Layer_Normal, Raw_Normal, Top_Normal;
  DBL Normal_Direction;
  int layer_number;

#define QColour Texture->Pigment->Quick_Colour

  Normal (&Raw_Normal, Ray_Intersection->Object, &Ray_Intersection->IPoint);
  /* Now, we perform the lighting calculations. */

  /* We assume here that Post_Process has propagated all parent
   textures to the object itself and that everything has some texture.
   Redirrect to the proper texture if its a material or checker texture */

  for (Texture = Ray_Intersection->Object->Texture;
  Texture->Type != PNF_TEXTURE;)
    switch (Texture->Type)
    {
    case TILE_TEXTURE:
      Texture = tiles_texture(&Ray_Intersection->IPoint,((TILES *)Texture));
      break;
    case MAT_TEXTURE:
      Texture = material_map(&Ray_Intersection->IPoint,((MATERIAL *)Texture));
      break;
    default:
      fprintf(stderr, "Bad texture type: %d\n", Texture->Type);
      close_all(); 
      exit(1);
    };

  Make_ColourA (&Filter_Colour, 1.0, 1.0, 1.0, 1.0);
  for (layer_number=1 , Layer = Texture;
  (Layer != NULL) && (fabs(Filter_Colour.Filter) > BLACK_LEVEL);
  layer_number++, Layer = Layer->Next_Layer)
    {
    Make_Colour (&Layer_Pigment_Colour, 0.0, 0.0, 0.0);
    if (Quality_Flags & Q_QUICKC)
      Layer_Pigment_Colour = QColour;
    else
      Add_Pigment (&Layer_Pigment_Colour, Layer->Pigment, &Ray_Intersection->IPoint);

    Layer_Normal = Raw_Normal;
    if ((Quality_Flags & Q_NORMAL) && (Texture->Tnormal != NULL))
      Perturb_Normal (&Layer_Normal, Texture->Tnormal,
        &Ray_Intersection->IPoint);

    /* If the surface normal points away, flip its direction. */
    VDot (Normal_Direction, Layer_Normal, Ray->Direction);
    if (Normal_Direction > 0.0) 
      {
      VScaleEq (Layer_Normal, -1.0);
      }
    if (layer_number == 1)
      Top_Normal = Layer_Normal;

    Compute_Reflected_Colour (Ray,
      Layer->Finish,
      Ray_Intersection,
      &Layer_Pigment_Colour,
      &Filter_Colour,
      Colour, &Layer_Normal);

    Filter_Colour.Red   *= Layer_Pigment_Colour.Red;
    Filter_Colour.Green *= Layer_Pigment_Colour.Green;
    Filter_Colour.Blue  *= Layer_Pigment_Colour.Blue;
    Filter_Colour.Filter *= Layer_Pigment_Colour.Filter;
    }

  Finish = Texture->Finish;

  if ((fabs(Filter_Colour.Filter) > BLACK_LEVEL) && (Quality_Flags & Q_REFRACT))
    {
    Make_Colour (&Refracted_Colour, 0.0, 0.0, 0.0);

    if (Finish->Refraction > 0.0)
      Refract (Texture, &Ray_Intersection -> IPoint, Ray,
        &Top_Normal, &Refracted_Colour);
    else
      Refract (Texture, &Ray_Intersection->IPoint, Ray,
        NULL, &Refracted_Colour);

    Colour->Red += Filter_Colour.Red * Refracted_Colour.Red * Filter_Colour.Filter;
    Colour->Green += Filter_Colour.Green * Refracted_Colour.Green * Filter_Colour.Filter;
    Colour->Blue += Filter_Colour.Blue * Refracted_Colour.Blue * Filter_Colour.Filter;
    }

  if (Frame.Fog_Distance != 0.0)
    Fog (Ray_Intersection->Depth, &Frame.Fog_Colour, Frame.Fog_Distance,
      Colour);
  }

void Filter_Shadow_Ray (Ray_Intersection, Colour)
INTERSECTION *Ray_Intersection;
COLOUR *Colour;
  {
  COLOUR Layer_Pigment_Colour, Filter_Colour;
  TEXTURE *Layer, *Texture;
  FINISH *Finish;
  int layer_number;

#define QColour Texture->Pigment->Quick_Colour

  if (!(Quality_Flags & Q_SHADOW))
    return;

  /* Now, we perform the lighting calculations. */

  /* We assume here that Post_Process has propagated all parent
   textures to the object itself and that everything has some texture.
   Redirrect to the proper texture if its a material or checker texture */

  for (Texture = Ray_Intersection->Object->Texture;
  Texture->Type != PNF_TEXTURE;)
    switch (Texture->Type)
    {
    case TILE_TEXTURE:
      Texture = tiles_texture(&Ray_Intersection->IPoint,((TILES *)Texture));
      break;
    case MAT_TEXTURE:
      Texture = material_map(&Ray_Intersection->IPoint,((MATERIAL *)Texture));
      break;
    default:
      fprintf(stderr, "Bad texture type: %d\n", Texture->Type);
      close_all(); 
      exit(1);
    };

  Make_ColourA (&Filter_Colour, 1.0, 1.0, 1.0, 1.0);
  for (layer_number=1 , Layer = Texture;
  (Layer != NULL) && (fabs(Filter_Colour.Filter) > BLACK_LEVEL);
  layer_number++, Layer = Layer->Next_Layer)
    {
    if (Quality_Flags & Q_QUICKC)
      Layer_Pigment_Colour = QColour;
    else
      {
      Make_Colour (&Layer_Pigment_Colour, 0.0, 0.0, 0.0);
      Add_Pigment (&Layer_Pigment_Colour, Layer->Pigment, &Ray_Intersection->IPoint);
      }

    Filter_Colour.Red   *= Layer_Pigment_Colour.Red;
    Filter_Colour.Green *= Layer_Pigment_Colour.Green;
    Filter_Colour.Blue  *= Layer_Pigment_Colour.Blue;
    Filter_Colour.Filter *= Layer_Pigment_Colour.Filter;
    }

  Finish = Texture->Finish;

  /* For shadow rays, we have the filter colour now - time to return */
  if (fabs(Filter_Colour.Filter) < BLACK_LEVEL) 
    {
    Make_Colour (Colour, 0.0, 0.0, 0.0);
    return;
    }

  if (Finish->Refraction > 0.0) 
    {
    Colour->Red *= Filter_Colour.Red * Finish->Refraction * Filter_Colour.Filter;
    Colour->Green *= Filter_Colour.Green * Finish->Refraction * Filter_Colour.Filter;
    Colour->Blue *= Filter_Colour.Blue * Finish->Refraction * Filter_Colour.Filter;
    }
  else 
    {
    Colour->Red *= Filter_Colour.Red * Filter_Colour.Filter;
    Colour->Green *= Filter_Colour.Green * Filter_Colour.Filter;
    Colour->Blue *= Filter_Colour.Blue * Filter_Colour.Filter;
    }
  return;

  }

