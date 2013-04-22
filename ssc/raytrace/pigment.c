/****************************************************************************
*                pigment.c
*
*  This module implements solid texturing functions that modify the color
*  transparency of an object's surface.
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

/*
   Some texture ideas garnered from SIGGRAPH '85 Volume 19 Number 3, 
   "An Image Synthesizer" By Ken Perlin.
   Further Ideas Garnered from "The RenderMan Companion" (Addison Wesley).
*/

#include "frame.h"
#include "vector.h"
#include "povproto.h"
#include "texture.h"

COLOUR_MAP_ENTRY Black_White_Entries[2]=
   {{0.0, {0.0,0.0,0.0,0.0}},
    {1.0, {1.0,1.0,1.0,0.0}}};

COLOUR_MAP Gray_Default_Map =
  { 2, FALSE, -1, Black_White_Entries};

COLOUR_MAP_ENTRY Bozo_Entries[6]=
   {{0.4, {1.0,1.0,1.0,0.0}},
    {0.4, {0.0,1.0,0.0,0.0}},
    {0.6, {0.0,1.0,0.0,0.0}},
    {0.6, {0.0,0.0,1.0,0.0}},
    {0.8, {0.0,0.0,1.0,0.0}},
    {0.8, {1.0,0.0,0.0,0.0}}};

COLOUR_MAP Bozo_Default_Map =
  { 6, FALSE, -1, Bozo_Entries};

COLOUR_MAP_ENTRY Wood_Entries[2]=
   {{0.6, {0.666,0.312, 0.2,  0.0}},
    {0.6, {0.4,  0.1333,0.066,0.0}}};

COLOUR_MAP Wood_Default_Map =
  { 2, FALSE, -1, Wood_Entries};

COLOUR_MAP_ENTRY Mandel_Entries[5]=
   {{0.001, {0.0,0.0,0.0,0.0}},
    {0.001, {0.0,1.0,1.0,0.0}},
    {0.012, {1.0,1.0,0.0,0.0}},
    {0.015, {1.0,0.0,1.0,0.0}},
    {0.1,   {0.0,1.0,1.0,0.0}}};

COLOUR_MAP Mandel_Default_Map =
  { 5, FALSE, -1, Mandel_Entries};

COLOUR_MAP_ENTRY Agate_Entries[6]=
   {{0.0, {1.0, 1.0, 1.0, 0.0}},
    {0.5, {0.95,0.75,0.5, 0.0}},
    {0.5, {0.9, 0.7, 0.5, 0.0}},
    {0.6, {0.9, 0.7, 0.4, 0.0}},
    {0.6, {1.0, 0.7, 0.4, 0.0}},
    {1.0, {0.6, 0.3, 0.0, 0.0}}};

COLOUR_MAP Agate_Default_Map =
  { 6, FALSE, -1, Agate_Entries};

COLOUR_MAP_ENTRY Radial_Entries[4]=
   {{0.0,   {0.0,1.0,1.0,0.0}},
    {0.333, {1.0,1.0,0.0,0.0}},
    {0.666, {1.0,0.0,1.0,0.0}},
    {1.0,   {0.0,1.0,1.0,0.0}}};

COLOUR_MAP Radial_Default_Map =
  { 4, FALSE, -1, Radial_Entries};

COLOUR_MAP_ENTRY Marble_Entries[3]=
   {{0.0,   {0.9,0.8, 0.8, 0.0}},
    {0.9,   {0.9,0.08,0.08,0.0}},
    {0.9,   {0.0,0.0,0.0,0.0}}};

COLOUR_MAP Marble_Default_Map =
  { 3, FALSE, -1, Marble_Entries};

void agate (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  register DBL noise, turb;
  COLOUR New_Colour;

  turb = Turbulence(x, y, z,Pigment->omega,
                    Pigment->lambda,Pigment->Octaves) 
           * Pigment->Agate_Turb_Scale;
  noise = 0.5 * (cycloidal(1.3 * turb + 1.1 * z) + 1.0);
  if (noise <= 0.0)
     noise = 0.0;
  else 
    {
     noise = (noise > 1.0 ? 1.0 : noise);
     noise = pow(noise, 0.77);
    }

  Compute_Colour (&New_Colour, Pigment, noise);
  colour -> Red += New_Colour.Red;
  colour -> Green += New_Colour.Green;
  colour -> Blue += New_Colour.Blue;
  colour -> Filter += New_Colour.Filter;
  }

void bozo (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  register DBL noise;
  COLOUR New_Colour;

  noise = Noise (x, y, z);

  Compute_Colour (&New_Colour, Pigment, noise);
  colour -> Red += New_Colour.Red;
  colour -> Green += New_Colour.Green;
  colour -> Blue += New_Colour.Blue;
  colour -> Filter += New_Colour.Filter;
  }

void brick (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {

  /* Disabled.  Needs work.

   DBL xr, yr, zr;

     xr = fabs(fmod(x, 1.0));
     yr = fabs(fmod(y, 1.0));
     zr = fabs(fmod(z, 1.0));

   *colour = *Pigment -> Colour2;

    if (xr > 0 && xr < Pigment-> Mortar) {
       *colour = *Pigment -> Colour1;
       return;
       }
    if (yr > 0 && yr < Pigment-> Mortar) {
       *colour = *Pigment -> Colour1;
       return;
       }
    if (zr > 0 && zr < Pigment-> Mortar)
       *colour = *Pigment -> Colour1;

*/
  return;

  }

void checker (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  DBL value;
  COLOUR_MAP_ENTRY *Cur;
  int Num_Entries = Pigment->Colour_Map->Number_Of_Entries;
  int Fudge = (int)((DBL)Num_Entries/Pigment->Frequency);

  x += Small_Tolerance;
  y += Small_Tolerance;
  z += Small_Tolerance;

  x -= FLOOR(x/Fudge) * Fudge-Small_Tolerance;
  y -= FLOOR(y/Fudge) * Fudge-Small_Tolerance;
  z -= FLOOR(z/Fudge) * Fudge-Small_Tolerance;

  value = (FLOOR(x)+FLOOR(y)+FLOOR(z))* Pigment->Frequency;

  value = fmod(FLOOR(value + (int)(Pigment->Phase)),
    (DBL) Num_Entries);

  Cur = &(Pigment->Colour_Map->Colour_Map_Entries[0]);

  while (value > Cur->value)
    Cur++;

  colour->Red   += Cur->Colour.Red;
  colour->Green += Cur->Colour.Green;
  colour->Blue  += Cur->Colour.Blue;
  colour->Filter += Cur->Colour.Filter;

  }


/*
   Color Gradient Pigment - gradient based on the fractional values of x, y or
   z, based on whether or not the given directional vector is a 1.0 or a 0.0.
   Note - ONLY works with colour maps, preferably one that is circular - i.e.
   the last defined colour (value 1.001) is the same as the first colour (with
   a value of 0.0) in the map.  The basic concept of this is from DBW Render,
   but Dave Wecker's only supports simple Y axis gradients.
*/

void gradient (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  COLOUR New_Colour;
  DBL value = 0.0;

  if (Pigment -> Colour_Gradient.x != 0.0)
    {
    x = FABS(x);
    value += x - FLOOR(x);        /* obtain fractional X component */
    }
  if (Pigment -> Colour_Gradient.y != 0.0)
    {
    y = FABS(y);
    value += y - FLOOR(y);        /* obtain fractional Y component */
    }
  if (Pigment -> Colour_Gradient.z != 0.0)
    {
    z = FABS(z);
    value += z - FLOOR(z);        /* obtain fractional Z component */
    }
  value = ((value > 1.0) ? fmod(value, 1.0) : value); /* clamp to 1.0 */

  Compute_Colour (&New_Colour, Pigment, value);
  colour -> Red += New_Colour.Red;
  colour -> Green += New_Colour.Green;
  colour -> Blue += New_Colour.Blue;
  colour -> Filter += New_Colour.Filter;
  }


/*
   Granite - kind of a union of the "spotted" and the "dented" textures,
   using a 1/f fractal noise function for color values.  Typically used
   w/ small scaling values.  Should work with colour maps for pink granite...
*/


void granite (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  register int i;
  register DBL temp, noise = 0.0, freq = 1.0;
  COLOUR New_Colour;

  for (i = 0; i < 6 ; freq *= 2.0, i++)
    {
    temp = 0.5 - Noise (x * 4 * freq, y * 4 * freq, z * 4 * freq);
    temp = FABS(temp);
    noise += temp / freq;
    }

  Compute_Colour (&New_Colour, Pigment, noise);
  colour -> Red += New_Colour.Red;
  colour -> Green += New_Colour.Green;
  colour -> Blue += New_Colour.Blue;
  colour -> Filter += New_Colour.Filter;
  }

void mandel (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {  
  DBL a,b,cf,a2,b2;
  int it_max,col;
  COLOUR thepoint;

  a = x; a2 = a*a;
  b = y; b2 = b*b;
  it_max = Pigment->Iterations;
  for (col=0;col<it_max;col++) 
    {
     b  = 2*a*b + y;
     a  = a2 - b2 + x;
     a2 = a*a;
     b2 = b*b;
     if (a2 + b2 > 4.0)
       break;
    }
  cf = (DBL)col / it_max;

  Compute_Colour (&thepoint, Pigment, cf);
  colour->Red   +=thepoint.Red;
  colour->Green +=thepoint.Green;
  colour->Blue  +=thepoint.Blue;
  colour->Filter +=thepoint.Filter;
  }


void marble (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  register DBL noise, turb;
  COLOUR New_Colour;

  if ((Pigment->Flags) & HAS_TURB)
    turb =  Turbulence(x, y, z,Pigment->omega,Pigment->lambda,Pigment->Octaves) *
    Pigment->Turbulence.x;
  else
    turb = 0.0;

  noise = Triangle_Wave(x + turb);

  Compute_Colour (&New_Colour, Pigment, noise);
  colour -> Red += New_Colour.Red;
  colour -> Green += New_Colour.Green;
  colour -> Blue += New_Colour.Blue;
  colour -> Filter += New_Colour.Filter;
  }

void radial (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  register DBL value;
  COLOUR New_Colour;

  if ( (fabs(x)<0.001) && (fabs(z)<0.001))
    value = 0.25;
  else
    value = 0.25+(atan2(x,z)+M_PI)/(2*M_PI);

  Compute_Colour (&New_Colour, Pigment, value);
  colour -> Red += New_Colour.Red;
  colour -> Green += New_Colour.Green;
  colour -> Blue += New_Colour.Blue;
  colour -> Filter += New_Colour.Filter;
  }


/*        
   With a little reflectivity and brilliance, can look like organ pipe
   metal.   With tiny scaling values can look like masonry or concrete.
   Works with color maps.
*/

void spotted (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  register DBL noise;
  COLOUR New_Colour;

  noise = Noise (x, y, z);

  Compute_Colour (&New_Colour, Pigment, noise);
  colour -> Red += New_Colour.Red;
  colour -> Green += New_Colour.Green;
  colour -> Blue += New_Colour.Blue;
  colour -> Filter += New_Colour.Filter;
  }


void wood (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  register DBL noise, length;
  VECTOR WoodTurbulence;
  VECTOR point;
  COLOUR New_Colour;

  DTurbulence (&WoodTurbulence, x, y, z,Pigment->omega,Pigment->lambda,Pigment->Octaves);

  point.x = cycloidal((x + WoodTurbulence.x)
    * Pigment -> Turbulence.x);
  point.y = cycloidal((y + WoodTurbulence.y)
    * Pigment -> Turbulence.y);
  point.z = 0.0;

  point.x += x;
  point.y += y;

  /*  point.z += z;       Deleted per David Buck --  BP 7/91 */

  VLength (length, point);

  noise = Triangle_Wave(length);

  Compute_Colour (&New_Colour, Pigment, noise);
  colour -> Red += New_Colour.Red;
  colour -> Green += New_Colour.Green;
  colour -> Blue += New_Colour.Blue;
  colour -> Filter += New_Colour.Filter;
  }

/* Two new pigments by Scott Taylor LEOPARD & ONION */

void leopard (x, y, z, Pigment, colour)      /* SWT 7/18/91 */
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  /* The variable noise is not used as noise in this function */
  register DBL noise,temp1,temp2,temp3;
  COLOUR New_Colour;

  /* This form didn't work with Zortech 386 compiler */
  /* noise = Sqr((sin(x)+sin(y)+sin(z))/3); */
  /* So we break it down. */
  temp1 = sin(x);
  temp2 = sin(y);
  temp3 = sin(z);
  noise = Sqr((temp1+temp2+temp3)/3);

  Compute_Colour (&New_Colour, Pigment, noise);
  colour -> Red += New_Colour.Red;
  colour -> Green += New_Colour.Green;
  colour -> Blue += New_Colour.Blue;
  colour -> Filter += New_Colour.Filter;
  }

void onion (x, y, z, Pigment, colour)      /* SWT 7/18/91 */
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  /* The variable noise is not used as noise in this function */
  register DBL noise;
  COLOUR New_Colour;

  /* This ramp goes 0-1,1-0,0-1,1-0...
   noise = (fmod(SQRT(Sqr(x)+Sqr(y)+Sqr(z)),2.0)-1.0);
   if (noise<0.0) {noise = 0.0-noise;}
   */

  /* This ramp goes 0-1,0-1,0-1,0-1... */
  noise = (fmod(SQRT(Sqr(x)+Sqr(y)+Sqr(z)),1.0));

  Compute_Colour (&New_Colour, Pigment, noise);
  colour -> Red += New_Colour.Red;
  colour -> Green += New_Colour.Green;
  colour -> Blue += New_Colour.Blue;
  colour -> Filter += New_Colour.Filter;
  }

/* TriHex pattern -- Ernest MacDougal Campbell III (EMC3) 11/23/92
 *
 * Creates a hexagon pattern in the XZ plane.
 *
 * This algorithm is hard to explain.  First it scales the point to make
 * a few of the later calculations easier, then maps some points to be
 * closer to the Origin.  A small area in the first quadrant is subdivided
 * into a 6 x 6 grid.  The position of the point mapped into that grid
 * determines its color.  For some points, just the grid location is enough,
 * but for others, we have to calculate which half of the block it's in 
 * (this is where the atan2() function comes in handy).
 */

#define xfactor 0.5;         /* each triangle is split in half for the grid */
#define zfactor 0.866025404; /* sqrt(3)/2 -- Height of an equilateral triangle */

void hexagon (x, y, z, Pigment, colour)
DBL x, y, z;
PIGMENT *Pigment;
COLOUR *colour;
  {
  int xm, zm;
  DBL xs, zs, xl, zl, value;
  int brkindx;
  COLOUR_MAP_ENTRY *Cur;

  /* Keep all numbers positive.  Also, if z is negative, map it in such a
 * way as to avoid mirroring across the x-axis.  The value 5.196152424
 * is (sqrt(3)/2) * 6 (because the grid is 6 blocks high)
 */

  x = fabs(x);
  z = z<0?5.196152424 - fabs(z):z;  /* avoid mirroring across x-axis */

  xs = x/xfactor;               /* scale point to make calcs easier */
  zs = z/zfactor;

  xs -= floor(xs/6) * 6;        /* map points into the 6 x 6 grid where  */
  zs -= floor(zs/6) * 6;        /* the basic formula works               */

  xm = (int) FLOOR(xs) % 6;     /* Get a block in the 6 x 6 grid */
  zm = (int) FLOOR(zs) % 6;

  switch (xm)
    {                 /* These are easy cases:           */
  case 0:                     /* color depends only on xm and zm */
  case 5:
    switch (zm)
      {
    case 0:
    case 5:
      value = 0;
      break;
    case 1:
    case 2:
      value = 1;
      break;
    case 3:
    case 4:
      value = 2;
      break;
    }
    break;
  case 2:
  case 3:
    switch (zm)
      {
    case 0:
    case 1:
      value = 2;
      break;
    case 2:
    case 3:
      value = 0;
      break;
    case 4:
    case 5:
      value = 1;
      break;
    }
    break;

    /* These cases are harder.  These blocks are divided diagonally
      * by the angled edges of the hexagons.  Some slope positive, and
      * others negative.  We flip the x value of the negatively sloped
      * pieces.  Then we check to see if the point in question falls
      * in the upper or lower half of the block.  That info, plus the
      * z status of the block determines the color.
      */

  case 1:
  case 4:
    xl = xs-xm; /* map the point into the block at the origin */
    zl = zs-zm;

    if (((xm+zm) % 2) == 1)   /* These blocks have negative slopes */
      xl = 1 - xl;            /* so we flip it horizontally        */

    if (xl == 0) 
      xl = .0001;             /* avoid a divide-by-zero error */

    /* is the angle less-than or greater-than 45 degrees? */

    brkindx = (zl/xl) < 1;

    /* was...
         * brkindx = (atan2(zl,xl) < (45 * M_PI/180));  
         * ...but because of the mapping, it's easier and cheaper, 
         * CPU-wise, to just use a good ol' slope.
         */

    switch (brkindx) 
      {
    case TRUE:
      switch (zm) 
        {
      case 0:
      case 3:
        value = 0;
        break;
      case 2:
      case 5:
        value = 1;
        break;
      case 1:
      case 4:
        value = 2;
        break;
      }
      break;
    case FALSE:
      switch (zm) 
        {
      case 0:
      case 3:
        value = 2;
        break;
      case 2:
      case 5:
        value = 0;
        break;
      case 1:
      case 4:
        value = 1;
        break;
      }
      }
    }
  value = fmod (value+(int)Pigment->Phase,3);

  Cur = &(Pigment->Colour_Map->Colour_Map_Entries[0]);
  while (value > Cur->value)
    Cur++;

  colour->Red   += Cur->Colour.Red;
  colour->Green += Cur->Colour.Green;
  colour->Blue  += Cur->Colour.Blue;
  colour->Filter += Cur->Colour.Filter;

  return;
  }

/* End trihex() */

PIGMENT *Create_Pigment ()
  {
  PIGMENT *New;

  if ((New = (PIGMENT *) malloc (sizeof (PIGMENT))) == NULL)
    MAError ("pigment");

  INIT_TPATTERN_FIELDS(New,NO_PIGMENT)
    New->Colour1 = NULL;
  Make_Colour(&(New->Quick_Colour), 0.5,0.5,0.5) ;
  New->Colour_Map = NULL;
  Make_Vector (&(New->Colour_Gradient), 0.0, 0.0, 0.0);
  New->Image = NULL;
  New->Mortar = 0.2;
  New->Agate_Turb_Scale = 1.0;
  New->Iterations = 0;
  return (New);
  }

PIGMENT *Copy_Pigment (Old)
PIGMENT *Old;
  {
  PIGMENT *New;

  if (Old != NULL)
    {
    New = Create_Pigment ();
    *New = *Old;

    New->Trans = Copy_Transform (Old->Trans);
    New->Colour1 = Copy_Colour (Old->Colour1);
    New->Image = Copy_Image (Old->Image);
    New->Colour_Map = Copy_Colour_Map (Old->Colour_Map);
    }
  else
    New = NULL;

  return (New);   
  }

void Translate_Pigment(Pigment,Vector)
PIGMENT *Pigment;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  if (Pigment == NULL)
    return;

  Compute_Translation_Transform (&Trans, Vector);
  Transform_Pigment (Pigment, &Trans);
  }

void Rotate_Pigment(Pigment,Vector)
PIGMENT *Pigment;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  if (Pigment == NULL)
    return;

  Compute_Rotation_Transform (&Trans, Vector);
  Transform_Pigment (Pigment, &Trans);
  }

void Scale_Pigment(Pigment,Vector)
PIGMENT *Pigment;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  if (Pigment == NULL)
    return;

  Compute_Scaling_Transform (&Trans, Vector);
  Transform_Pigment (Pigment, &Trans);
  }

void Transform_Pigment(Pigment,Trans)
PIGMENT *Pigment;
TRANSFORM *Trans;
  {
  if (Pigment == NULL)
    return;

  if (!Pigment->Trans)
    Pigment->Trans = Create_Transform ();

  Compose_Transforms (Pigment->Trans, Trans);
  }

void Destroy_Pigment (Pigment)
PIGMENT *Pigment;
  {
  if (Pigment == NULL)
    return;

  Destroy_Colour (Pigment->Colour1);
  Destroy_Colour_Map (Pigment->Colour_Map);
  Destroy_Image (Pigment->Image);
  Destroy_Transform (Pigment->Trans);
  free (Pigment);
  }

void Post_Pigment (Pigment)
PIGMENT *Pigment;
  {
  if (Pigment == NULL)
    Error("Missing pigment");

  if (Pigment->Flags & POST_DONE)
    return;

  if (Pigment->Type == NO_PIGMENT)
    {
    Pigment->Type = COLOUR_PIGMENT;
    Pigment->Colour1 = Create_Colour ();
    Warn("No pigment type given",1.5);
    }

  Pigment->Flags |= POST_DONE;

  switch (Pigment->Type)
  {
  case COLOUR_PIGMENT:
    Destroy_Transform (Pigment->Trans);
    Pigment->Trans = NULL;
    Make_Vector(&(Pigment->Turbulence),0.0,0.0,0.0);
    break;

  case BOZO_PIGMENT:
    if (Pigment->Colour_Map == NULL)
      Pigment->Colour_Map = &Bozo_Default_Map;
    break;

  case WOOD_PIGMENT:
    if (Pigment->Colour_Map == NULL)
      Pigment->Colour_Map = &Wood_Default_Map;
    break;

  case MANDEL_PIGMENT:
    if (Pigment->Colour_Map == NULL)
      Pigment->Colour_Map = &Mandel_Default_Map;
    break;

  case RADIAL_PIGMENT:
    if (Pigment->Colour_Map == NULL)
      Pigment->Colour_Map = &Radial_Default_Map;
    break;

  case AGATE_PIGMENT:
    if (Pigment->Colour_Map == NULL)
      Pigment->Colour_Map = &Agate_Default_Map;
    break;

  case MARBLE_PIGMENT:
    if (Pigment->Colour_Map == NULL)
      Pigment->Colour_Map = &Marble_Default_Map;
    break;

  case SPOTTED_PIGMENT:
  case GRADIENT_PIGMENT:
  case GRANITE_PIGMENT:
  case ONION_PIGMENT:
  case LEOPARD_PIGMENT:
    if (Pigment->Colour_Map == NULL)
      Pigment->Colour_Map = &Gray_Default_Map;
    break;
  }

  return;   
  }
