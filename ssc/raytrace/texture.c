/****************************************************************************
*                texture.c
*
*  This module implements texturing functions such as noise, turbulence and
*  texture transformation functions. The actual texture routines are in the
*  files pigment.c & normal.c.
*  The noise function used here is the one described by Ken Perlin in
*  "Hypertexture", SIGGRAPH '89 Conference Proceedings page 253.
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
   Further Ideas Garnered from "The RenderMan Companion" (Addison Wesley)
*/

#include "frame.h"
#include "vector.h"
#include "povproto.h"
#include "texture.h"

DBL *sintab;
DBL frequency[NUMBER_OF_WAVES];
VECTOR Wave_Sources[NUMBER_OF_WAVES];
DBL *RTable;
short *hashTable;

unsigned short crctab[256] =
  {
  0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
  0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
  0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
  0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
  0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
  0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
  0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
  0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
  0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
  0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
  0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
  0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
  0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
  0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
  0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
  0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
  0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
  0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
  0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
  0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
  0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
  0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
  0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
  0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
  0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
  0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
  0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
  0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
  0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
  0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
  0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
  0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
  };

void Compute_Colour (Colour, Pigment, value)
COLOUR *Colour;
PIGMENT *Pigment;
DBL value;
  {
  COLOUR_MAP *Colour_Map = Pigment->Colour_Map;
  int max_colors = Colour_Map->Number_Of_Entries-1;
  COLOUR_MAP_ENTRY *Cur, *Prev;
  register DBL fraction;

  value = fmod(value * Pigment->Frequency + Pigment->Phase,1.00001);
  if (value < 0.0)         /* allow negative Frequency */
    value -=floor(value);

  /* if greater than last, use last. */
  if (value >= Colour_Map->Colour_Map_Entries[max_colors].value)
    {
    *Colour = Colour_Map->Colour_Map_Entries[max_colors].Colour;
    return;
    }

  Prev = Cur = &(Colour_Map->Colour_Map_Entries[0]);
  while (value >= Cur->value)
    Prev = Cur++;

  /* if stopped on first entry, use first. */
  if (Prev == Cur)
    {
    *Colour = Cur->Colour;
    return;
    }

  fraction = (value - Prev->value) / (Cur->value - Prev->value);
  Colour->Red   = Prev->Colour.Red   + fraction * (Cur->Colour.Red   - Prev->Colour.Red);
  Colour->Green = Prev->Colour.Green + fraction * (Cur->Colour.Green - Prev->Colour.Green);
  Colour->Blue  = Prev->Colour.Blue  + fraction * (Cur->Colour.Blue  - Prev->Colour.Blue);
  Colour->Filter = Prev->Colour.Filter + fraction * (Cur->Colour.Filter - Prev->Colour.Filter);

  return;
  }

void Initialize_Noise ()
  {
  register int i = 0;
  VECTOR point;

  InitRTable();

  if ((sintab = (DBL *)malloc(SINTABSIZE * sizeof(DBL))) == NULL) 
    MAError ("sine table");

  for (i = 0 ; i < SINTABSIZE ; i++)
    sintab[i] = sin(i/(DBL)SINTABSIZE * (3.14159265359 * 2.0));

  for (i = 0 ; i < NUMBER_OF_WAVES ; i++)
    {
    DNoise (&point, (DBL) i, 0.0, 0.0);
    VNormalize (Wave_Sources[i], point);
    frequency[i] = (rand() & RNDMASK) / RNDDIVISOR + 0.01;
    }
  }

void InitTextureTable()
  {
  int i, j, temp;

  srand(0);

  if ((hashTable = (short int *) malloc(4096*sizeof(short int))) == NULL) 
    MAError ("hash table");

  for (i = 0; i < 4096; i++)
    hashTable[i] = i;
  for (i = 4095; i >= 0; i--) 
    {
    j = rand() % 4096;
    temp = hashTable[i];
    hashTable[i] = hashTable[j];
    hashTable[j] = temp;
    }
  }

/* modified by AAC to work properly with little bitty integers (16 bits) */

void InitRTable()
  {
  int i;
  VECTOR rp;

  InitTextureTable();

  if ((RTable = (DBL *)malloc(MAXSIZE * sizeof(DBL))) == NULL) 
    MAError ("RTable");

  for (i = 0; i < MAXSIZE; i++)
    {
    rp.x = rp.y = rp.z = (DBL)i;
    RTable[i] = (unsigned int) R(&rp) * REALSCALE - 1.0;
    }
  }

int R(v)
VECTOR *v;
  {
  v->x *= .12345;
  v->y *= .12345;
  v->z *= .12345;

  return (Crc16((char *) v, sizeof(VECTOR)));
  }

/*
 * Note that passing a VECTOR array to Crc16 and interpreting it as
 * an array of chars means that machines with different floating-point
 * representation schemes will evaluate Noise(point) differently.
 */

int Crc16(buf, count)
register char *buf;
register int  count;
  {
  register unsigned short crc = 0;

  while (count--)
    crc = (crc >> 8) ^ crctab[ (unsigned char) (crc ^ *buf++) ];

  return ((int) crc);
  }


/*
        Robert's Skinner's Perlin-style "Noise" function - modified by AAC
        to ensure uniformly distributed clamped values between 0 and 1.0...
*/


DBL Noise(x, y, z)
DBL x, y, z;
  {
  DBL *mp;
  long ix, iy, iz, jx, jy, jz;
  int ixiy_hash, ixjy_hash, jxiy_hash, jxjy_hash;

  DBL sx, sy, sz, tx, ty, tz;
  DBL sum;

  DBL x_ix, x_jx, y_iy, y_jy, z_iz, z_jz, txty, sxty, txsy, sxsy;

  Calls_To_Noise++;

  /*setup_lattice(&x, &y, &z, &ix, &iy, &iz, &jx, &jy, &jz, &sx, &sy, &sz, &tx, &ty, &tz);*/
  x -= MINX;
  y -= MINY;
  z -= MINZ;

  /* its equivalent integer lattice point. */
  ix = (long)x; iy = (long)y; iz = (long)z;
  jx = ix + 1; jy = iy + 1; jz = iz + 1;

  sx = SCURVE(x - ix); sy = SCURVE(y - iy); sz = SCURVE(z - iz);

  /* the complement values of sx,sy,sz */
  tx = 1.0 - sx; ty = 1.0 - sy; tz = 1.0 - sz;

  /*
    *  interpolate!
    */
  x_ix = x - ix;
  x_jx = x - jx;
  y_iy = y - iy;
  y_jy = y - jy;
  z_iz = z - iz;
  z_jz = z - jz;
  txty = tx * ty;
  sxty = sx * ty;
  txsy = tx * sy;
  sxsy = sx * sy;
  ixiy_hash = Hash2d ( ix, iy );
  jxiy_hash = Hash2d ( jx, iy );
  ixjy_hash = Hash2d ( ix, jy );
  jxjy_hash = Hash2d ( jx, jy );

  mp = &RTable[(int) Hash1d(ixiy_hash, iz ) & 0xFF];
  sum = INCRSUMP(mp,(txty*tz), x_ix, y_iy, z_iz);

  mp = &RTable[(int) Hash1d( jxiy_hash, iz ) & 0xFF];
  sum += INCRSUMP(mp,(sxty*tz), x_jx, y_iy, z_iz);

  mp = &RTable[(int) Hash1d( ixjy_hash, iz ) & 0xFF];
  sum += INCRSUMP(mp,(txsy*tz), x_ix, y_jy, z_iz);

  mp = &RTable[(int) Hash1d( jxjy_hash, iz ) & 0xFF];
  sum += INCRSUMP(mp,(sxsy*tz), x_jx, y_jy, z_iz);

  mp = &RTable[(int) Hash1d( ixiy_hash, jz ) & 0xFF];
  sum += INCRSUMP(mp,(txty*sz), x_ix, y_iy, z_jz);

  mp = &RTable[(int) Hash1d( jxiy_hash, jz ) & 0xFF];
  sum += INCRSUMP(mp,(sxty*sz), x_jx, y_iy, z_jz);

  mp = &RTable[(int) Hash1d( ixjy_hash, jz ) & 0xFF];
  sum += INCRSUMP(mp,(txsy*sz), x_ix, y_jy, z_jz);

  mp = &RTable[(int) Hash1d( jxjy_hash, jz ) & 0xFF];
  sum += INCRSUMP(mp,(sxsy*sz), x_jx, y_jy, z_jz);

  sum = sum + 0.5;          /* range at this point -0.5 - 0.5... */

  if (sum < 0.0)
    sum = 0.0;
  if (sum > 1.0)
    sum = 1.0;

  return (sum);
  }


/*
       Vector-valued version of "Noise"
*/

void DNoise(result, x, y, z)
VECTOR *result;
DBL x, y, z;
  {
  DBL *mp;
  long ix, iy, iz, jx, jy, jz;
  int ixiy_hash, ixjy_hash, jxiy_hash, jxjy_hash;
  DBL px, py, pz, s;
  DBL sx, sy, sz, tx, ty, tz;
  DBL txty, sxty, txsy, sxsy;

  Calls_To_DNoise++;

  /*setup_lattice(&x, &y, &z, &ix, &iy, &iz, &jx, &jy, &jz, &sx, &sy, &sz, &tx, &ty, &tz);*/
  x -= MINX;
  y -= MINY;
  z -= MINZ;

  /* its equivalent integer lattice point. */
  ix = (long)x; iy = (long)y; iz = (long)z;
  jx = ix + 1; jy = iy + 1; jz = iz + 1;

  sx = SCURVE(x - ix); sy = SCURVE(y - iy); sz = SCURVE(z - iz);

  /* the complement values of sx,sy,sz */
  tx = 1.0 - sx; ty = 1.0 - sy; tz = 1.0 - sz;

  /*
    *  interpolate!
    */
  txty = tx * ty;
  sxty = sx * ty;
  txsy = tx * sy;
  sxsy = sx * sy;
  ixiy_hash = Hash2d ( ix, iy );
  jxiy_hash = Hash2d ( jx, iy );
  ixjy_hash = Hash2d ( ix, jy );
  jxjy_hash = Hash2d ( jx, jy );

  mp = &RTable[(int) Hash1d( ixiy_hash, iz ) & 0xFF];
  px = x-ix;  py = y-iy;  pz = z-iz;
  s = txty*tz;
  result->x = INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->y = INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->z = INCRSUMP(mp,s,px,py,pz);

  mp = &RTable[(int) Hash1d( jxiy_hash, iz ) & 0xFF];
  px = x-jx;
  s = sxty*tz;
  result->x += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->y += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->z += INCRSUMP(mp,s,px,py,pz);

  mp = &RTable[(int) Hash1d( jxjy_hash, iz ) & 0xFF];
  py = y-jy;
  s = sxsy*tz;
  result->x += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->y += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->z += INCRSUMP(mp,s,px,py,pz);

  mp = &RTable[(int) Hash1d( ixjy_hash, iz ) & 0xFF];
  px = x-ix;
  s = txsy*tz;
  result->x += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->y += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->z += INCRSUMP(mp,s,px,py,pz);

  mp = &RTable[(int) Hash1d( ixjy_hash, jz ) & 0xFF];
  pz = z-jz;
  s = txsy*sz;
  result->x += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->y += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->z += INCRSUMP(mp,s,px,py,pz);

  mp = &RTable[(int) Hash1d( jxjy_hash, jz ) & 0xFF];
  px = x-jx;
  s = sxsy*sz;
  result->x += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->y += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->z += INCRSUMP(mp,s,px,py,pz);

  mp = &RTable[(int) Hash1d( jxiy_hash, jz ) & 0xFF];
  py = y-iy;
  s = sxty*sz;
  result->x += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->y += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->z += INCRSUMP(mp,s,px,py,pz);

  mp = &RTable[(int) Hash1d( ixiy_hash, jz ) & 0xFF];
  px = x-ix;
  s = txty*sz;
  result->x += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->y += INCRSUMP(mp,s,px,py,pz);
  mp += 4;
  result->z += INCRSUMP(mp,s,px,py,pz);
  }

DBL Turbulence (x, y, z, omega, lambda, octaves)
DBL x, y, z,omega, lambda;
int octaves;
  {
  int i;
  DBL l, o, value, tempx, tempy, tempz;

  value = Noise(x, y, z);
  l = lambda;
  o = omega;
  for (i = 2; i <= octaves; i++)
    {
    tempx = l * x;
    tempy = l * y;
    tempz = l * z;
    value += o * Noise(tempx, tempy, tempz);
    if (i < octaves)
      {
      l *= lambda;
      o *= omega;
      }
    }
  return (value);
  }

void DTurbulence (result, x, y, z, omega, lambda, octaves)
VECTOR  *result;
DBL x, y, z, omega, lambda;
int octaves;
  {
  int i;
  DBL l, o;
  VECTOR value, temp;

  result -> x = 0.0;
  result -> y = 0.0;
  result -> z = 0.0;

  value.x = value.y = value.z = 0.0;

  DNoise(result, x,y,z);

  l = lambda;
  o = omega;
  for (i = 2; i <= octaves; i++)
    {
    temp.x = l * x;
    temp.y = l * y;
    temp.z = l * z;

    DNoise(&value, temp.x, temp.y, temp.z);
    result->x += o * value.x;
    result->y += o * value.y;
    result->z += o * value.z;
    if (i < octaves)
      {
      l *= lambda;
      o *= omega;
      }
    }
  }

DBL cycloidal (value)
DBL value;
  {
  register int indx;

  if (value >= 0.0)
    {
    indx = (int)((value - floor (value)) * SINTABSIZE);
    return (sintab [indx]);
    }
  else
    {
    indx = (int)((0.0 - (value + floor (0.0 - value))) * SINTABSIZE);
    return (0.0 - sintab [indx]);
    }
  }

DBL Triangle_Wave (value)
DBL value;
  {
  register DBL offset,temp1;

  if (value >= 0.0) offset = value - floor(value);
  else 
    {
    temp1 = -1.0 - floor(fabs(value));
    offset = value - temp1;
    }
  if (offset >= 0.5) return (2.0 * (1.0 - offset));
  else return (2.0 * offset);
  }

  void Translate_Textures (Textures, Vector)
    TEXTURE *Textures;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Translation_Transform (&Trans, Vector);
  Transform_Textures (Textures, &Trans);
  }

void Rotate_Textures (Textures, Vector)
TEXTURE *Textures;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Rotation_Transform (&Trans, Vector);
  Transform_Textures (Textures, &Trans);
  }

void Scale_Textures (Textures, Vector)
TEXTURE *Textures;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Scaling_Transform (&Trans, Vector);
  Transform_Textures (Textures, &Trans);
  }

void Transform_Textures (Textures, Trans)
TEXTURE *Textures;
TRANSFORM *Trans;
  {
  TEXTURE *Layer, *Material;

  for (Layer = Textures;
  Layer != NULL;
  Layer = Layer->Next_Layer)
    switch (Layer->Type)
    {
    case PNF_TEXTURE:
      Transform_Pigment (Layer->Pigment, Trans);
      Transform_Tnormal (Layer->Tnormal, Trans);
      break;

    case TILE_TEXTURE:
      if (((TILES *)Layer)->Trans == NULL)
        ((TILES *)Layer)->Trans = Create_Transform ();
      Compose_Transforms (((TILES *)Layer)->Trans, Trans);
      Transform_Textures (((TILES *)Layer)->Tile1, Trans);
      Transform_Textures (((TILES *)Layer)->Tile2, Trans);
      break;

    case MAT_TEXTURE:
      if (((MATERIAL *)Layer)->Trans == NULL)
        ((MATERIAL *)Layer)->Trans = Create_Transform ();
      Compose_Transforms (((MATERIAL *)Layer)->Trans, Trans);
      for (Material = ((MATERIAL *)Layer)->Materials;
      Material != NULL;
      Material = Material->Next_Material)
        Transform_Textures (Material, Trans);
      break;
    }
  }

FINISH *Create_Finish ()
  {
  FINISH *New;

  if ((New = (FINISH *) malloc (sizeof (FINISH))) == NULL)
    MAError ("finish");

  New->Reflection = 0.0;
  New->Ambient    = 0.1;
  New->Diffuse    = 0.6;
  New->Brilliance = 1.0;
  New->Index_Of_Refraction = 1.0;
  New->Refraction = 0.0;
  New->Specular   = 0.0;
  New->Roughness  = 1.0/0.05; /* CEY 12/92 */
  New->Phong      = 0.0;
  New->Phong_Size = 40.0;
  New->Crand = 0.0;
  New->Metallic_Flag = FALSE;

  return (New);
  }

FINISH *Copy_Finish (Old)
FINISH *Old;
  {
  FINISH *New;

  if (Old != NULL)
    {
    New  = Create_Finish ();
    *New = *Old;
    }
  else
    New = NULL;
  return (New);
  }

TEXTURE *Create_PNF_Texture ()
  {
  TEXTURE *New;

  if ((New = (TEXTURE *) malloc (sizeof (TEXTURE))) == NULL)
    MAError ("texture");

  New->Type    = PNF_TEXTURE;
  New->Flags   = NO_FLAGS;
  New->Pigment = NULL;
  New->Tnormal = NULL;
  New->Finish  = NULL;
  New->Next_Layer = NULL;
  New->Next_Material = NULL;

  return (New);
  }

TILES *Create_Tiles_Texture ()
  {
  TILES *New;

  if ((New = (TILES *) malloc (sizeof (TILES))) == NULL)
    MAError ("checker texture");

  New->Type  = TILE_TEXTURE;
  New->Flags = NO_FLAGS;
  New->Tile1 = NULL;
  New->Tile2 = NULL;
  New->Trans  = NULL;
  New->Next_Layer = NULL;
  New->Next_Material = NULL;

  return (New);
  }

MATERIAL *Create_Material_Texture ()
  {
  MATERIAL *New;

  if ((New = (MATERIAL *) malloc (sizeof (MATERIAL))) == NULL)
    MAError ("material texture");

  New->Type      = MAT_TEXTURE;
  New->Flags     = NO_FLAGS;
  New->Materials = NULL;
  New->Num_Of_Mats = 0;
  New->Trans  = NULL;
  New->Next_Layer = NULL;
  New->Next_Material = NULL;

  return (New);
  }

TEXTURE *Copy_Textures (Textures)
TEXTURE *Textures;
  {
  TEXTURE *New, *First, *Previous, *Layer;

  Previous = First = NULL;

  for (Layer = Textures;
  Layer != NULL;
  Layer = Layer->Next_Layer)
    {
    switch (Layer->Type)
    {
    case PNF_TEXTURE:
      New = Create_PNF_Texture ();
      New->Pigment = Copy_Pigment (Layer->Pigment);
      New->Tnormal = Copy_Tnormal (Layer->Tnormal);
      New->Finish  = Copy_Finish  (Layer->Finish);
      break;

    case TILE_TEXTURE:
      New = (TEXTURE *) Create_Tiles_Texture ();
      ((TILES *)New)->Tile1 = Copy_Textures (((TILES *)Layer)->Tile1);
      ((TILES *)New)->Tile2 = Copy_Textures (((TILES *)Layer)->Tile2);
      ((TILES *)New)->Trans = Copy_Transform (((TILES *)Layer)->Trans);
      break;

    case MAT_TEXTURE:
      New = (TEXTURE *) Create_Material_Texture ();
      ((MATERIAL *)New)->Materials = Copy_Materials (((MATERIAL *)Layer)->Materials);
      ((MATERIAL *)New)->Trans = Copy_Transform (((MATERIAL *)Layer)->Trans);
      ((MATERIAL *)New)->Num_Of_Mats = (((MATERIAL *)Layer)->Num_Of_Mats);
      break;
    }

    if (First == NULL)
      First = New;
    if (Previous != NULL)
      Previous->Next_Layer = New;
    Previous = New;
    }
  return (First);
  }

TEXTURE *Copy_Materials (Old)
TEXTURE *Old;
  {
  TEXTURE *New, *First, *Previous, *Material;

  Previous = First = NULL;

  for (Material = Old;
  Material != NULL;
  Material = Material->Next_Material)
    {
    New = Copy_Textures (Material);

    if (First == NULL)
      First = New;

    if (Previous != NULL)
      Previous->Next_Material = New;

    Previous = New;
    }
  return (First);
  }

void Destroy_Textures (Textures)
TEXTURE *Textures;
  {
  TEXTURE *Layer=Textures;
  TEXTURE *Mats;
  TEXTURE *Temp;

  while (Layer != NULL)
    {
    Mats = Layer->Next_Material;
    while (Mats != NULL)
      {
      Temp = Mats->Next_Material;
      Destroy_Textures (Mats);
      Mats = Temp;
      }
    switch (Layer->Type)
    {
    case PNF_TEXTURE:
      Destroy_Pigment (Layer->Pigment);
      Destroy_Tnormal (Layer->Tnormal);
      Destroy_Finish (Layer->Finish);
      break;

    case TILE_TEXTURE:
      Destroy_Transform (((TILES *)Layer)->Trans);
      Destroy_Textures (((TILES *)Layer)->Tile1);
      Destroy_Textures (((TILES *)Layer)->Tile2);
      break;

    case MAT_TEXTURE:
      Destroy_Transform (((MATERIAL *)Layer)->Trans);
      Destroy_Textures (((MATERIAL *)Layer)->Materials);
      Destroy_Image (((MATERIAL *)Layer)->Image);
      break;
    }
    Temp = Layer->Next_Layer;
    free (Layer);
    Layer = Temp;
    }
  }  

void Post_Textures (Textures)
TEXTURE *Textures;
  {
  TEXTURE *Layer, *Material;

  if (Textures == NULL)
    return;

  for (Layer = Textures;
  Layer != NULL;
  Layer = Layer->Next_Layer)
    {
    if (!((Layer->Flags) & POST_DONE))
      switch (Layer->Type)
      {
      case PNF_TEXTURE:
        Post_Pigment (Layer->Pigment);
        Post_Tnormal (Layer->Tnormal);
        break;

      case TILE_TEXTURE:
        Post_Textures (((TILES *)Layer)->Tile1);
        Post_Textures (((TILES *)Layer)->Tile2);
        break;

      case MAT_TEXTURE:
        for (Material = ((MATERIAL *)Layer)->Materials;
        Material != NULL;
        Material = Material->Next_Material)
          Post_Textures(Material);
        break;
      }
    }
  return;
  }

