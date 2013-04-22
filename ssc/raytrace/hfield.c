/****************************************************************************
*                   hfield.c
*
*    This file implements the height field shape primitive.  The shape is
*    implemented as a collection of triangles which are calculated as
*    needed.  The basic intersection routine first computes the rays
*    intersection with the box marking the limits of the shape, then
*    follows the line from one intersection point to the other, testing the
*    two triangles which form the pixel for an intersection with the ray at
*    each step.
*        height field added by Doug Muir
*        with lots of advice and support from David Buck 
*            and Drew Wells.
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

#define sign(x) (((x) > 0.0) ? 1: (((x) < 0.0) ? -1: 0))

#ifndef min_value
#define min_value(x,y) ((x) > (y) ? (y) : (x))
#endif
#ifndef max_value
#define max_value(x,y) ((x) < (y) ? (y) : (x))
#endif

METHODS Height_Field_Methods = 
  {
  All_HeightFld_Intersections,
  Inside_HeightFld, HeightFld_Normal,
  Copy_HeightFld,    Translate_HeightFld, Rotate_HeightFld,
  Scale_HeightFld, Transform_HeightFld, Invert_HeightFld,Destroy_HeightFld
};

METHODS Csg_Height_Field_Methods = 
  {
  All_Csg_HeightFld_Intersections,
  Inside_HeightFld, HeightFld_Normal,
  Copy_HeightFld,
  Translate_HeightFld, Rotate_HeightFld,
  Scale_HeightFld, Transform_HeightFld, Invert_HeightFld,Destroy_HeightFld
};

extern long Ray_Ht_Field_Tests, Ray_Ht_Field_Tests_Succeeded;
extern long Ray_Ht_Field_Box_Tests, Ray_HField_Box_Tests_Succeeded;
extern int Options;

int isdx, isdz, X_Dom;
DBL Gdx, Gdy, Gdz, Block_Size, Inv_Blk_Size;
DBL Myx, Mxz, Mzx, Myz;
ISTACK *Hf_Stack;
RAY *RRay;
DBL mindist, maxdist;

#define Get_Height(x, z, H_Field) ((DBL)(H_Field)->Map[(z)][(x)])

static DBL Normalize PARAMS(( VECTOR *A, VECTOR *B ));
static DBL stretch PARAMS((DBL x));
static int Intersect_Csg_Sub_Block PARAMS((HF_BLOCK *Block, RAY *Ray,
HEIGHT_FIELD *H_Field, VECTOR *start, VECTOR *end));
static int Intersect_Csg_Hf_Node PARAMS((RAY *Ray, HEIGHT_FIELD *H_Field,
VECTOR *start, VECTOR *end));
static DBL Normalize PARAMS(( VECTOR *A, VECTOR *B ));
static int add_single_normal PARAMS((HF_val **data, int xsize, int zsize,
int x0, int z0,int x1, int z1,int x2, int z2,VECTOR *N));
static void smooth_height_field PARAMS((HEIGHT_FIELD *hf, int xsize, int zsize));

static DBL
Normalize(A, B)
VECTOR *A, *B;
  {
  DBL VTemp = sqrt(B->x * B->x + B->y * B->y + B->z * B->z);
  if (fabs(VTemp) > EPSILON) 
    {
    A->x = B->x / VTemp;
    A->y = B->y / VTemp;
    A->z = B->z / VTemp;
    }
  else 
    {
    A->x = 0.0;
    A->y = 1.0;
    A->z = 0.0;
    }
  return VTemp;
  }

  int Intersect_Pixel(x, z, Ray, H_Field, height1, height2)
    int x;
int z;
RAY *Ray;
HEIGHT_FIELD *H_Field;
DBL height1;
DBL height2;
  {
  VECTOR T1V1,T1V2,T1V3,T2V1,T2V2,T2V3,Local_Normal,N1;
  DBL pos1,pos2,dot,depth1,depth2,s,t,y1,y2,y3,y4;
  DBL max_height, min_height;
  int Found = FALSE;

  y1 = Get_Height(x,z,H_Field);
  y2 = Get_Height(x+1,z,H_Field);
  y3 = Get_Height(x,z+1,H_Field);
  y4 = Get_Height(x+1,z+1,H_Field);

  Make_Vector(&T1V1,(DBL)x,y1,(DBL)z);
  Make_Vector(&T1V2,1.0,y2-y1,0.0);
  Make_Vector(&T1V3,0.0,y3-y1,1.0);
  Make_Vector(&T2V1,(DBL)(x+1),y4,(DBL)(z+1));
  Make_Vector(&T2V2,-1.0,y3-y4,0.0);
  Make_Vector(&T2V3,0.0,y2-y4,-1.0);

  /*
     * first, we check to see if it is even possible for the ray to
     * intersect the triangle.
     */

  max_height = max_value(y1,max_value(y2,y3));
  min_height = min_value(y1,min_value(y2,y3));
  if((max_height >= height1) && (min_height <= height2))
    {
    VCross(Local_Normal,T1V3,T1V2);
    VDot(dot,Local_Normal,Ray->Direction);

    if((dot > EPSILON) || (dot < -EPSILON))
      {
      VDot(pos1,Local_Normal,T1V1);
      VDot(pos2,Local_Normal,Ray->Initial);

      pos1 -= pos2;

      depth1 = pos1 / dot;

      if((depth1 >= mindist) && (depth1 <= maxdist)) 
        {
        s = Ray->Initial.x+(depth1*Ray->Direction.x)-(DBL)x;
        t = Ray->Initial.z+(depth1*Ray->Direction.z)-(DBL)z;

        if((s>=-0.0001) && (t>=-0.0001) && ((s+t)<=1.0001)) 
          {
          if (!H_Field->Smoothed) 
            {
            N1 = Local_Normal;
            if (H_Field->cache_pos < HF_CACHE_SIZE) 
              {
              H_Field->Normal_Vector[H_Field->cache_pos].normal = N1;
              H_Field->Normal_Vector[H_Field->cache_pos].x = x + s;
              H_Field->Normal_Vector[H_Field->cache_pos].z = z + t;
              H_Field->cache_pos += 1;
              }
            }

          VScale (T1V1, RRay -> Direction, depth1);
          VAddEq (T1V1, RRay -> Initial);
          if (Point_In_Clip (&T1V1, H_Field->Clip))
            {
            push_entry(depth1,T1V1,(OBJECT *)H_Field,Hf_Stack);
            Found = TRUE;
            Ray_Ht_Field_Tests_Succeeded++;
            }
          }
        }
      }
    }

  /*
	 * first, we check to see if it is even possible for the ray to
	 * intersect the triangle.
	   Rewritten to get around Code Builder FP stack problem.
	   Original code:
              if((max_value(y4,max_value(y2,y3)) >= height1) && 
	      (min_value(y4,min_value(y2,y3)) <= height2))            */

  max_height = max_value(y4,max_value(y2,y3));
  min_height = min_value(y4,min_value(y2,y3));
  if((max_height >= height1) && (min_height <= height2))
    {
    VCross(Local_Normal,T2V3,T2V2);
    VDot(dot,Local_Normal,Ray->Direction);

    if((dot > EPSILON) || (dot < -EPSILON))
      {
      VDot(pos1,Local_Normal,T2V1);

      VDot(pos2,Local_Normal,Ray->Initial);
      pos1 -= pos2;

      depth2 = pos1 / dot;

      if((depth2 >=mindist) && (depth2 <=maxdist))
        {
        s = Ray->Initial.x+(depth2*Ray->Direction.x)-(DBL)x;
        t = Ray->Initial.z+(depth2*Ray->Direction.z)-(DBL)z;

        if((s<=1.0001) && (t<=1.0001) && ((s+t)>0.9999)) 
          {
          if (!H_Field->Smoothed) 
            {
            N1 = Local_Normal;
            if (H_Field->cache_pos < HF_CACHE_SIZE) 
              {
              H_Field->Normal_Vector[H_Field->cache_pos].normal = N1;
              H_Field->Normal_Vector[H_Field->cache_pos].x = x + s;
              H_Field->Normal_Vector[H_Field->cache_pos].z = z + t;
              H_Field->cache_pos += 1;
              }
            }

          VScale (T1V1, RRay -> Direction, depth2);
          VAddEq (T1V1, RRay -> Initial);
          if (Point_In_Clip (&T1V1, H_Field->Clip))
            {
            push_entry(depth2,T1V1,(OBJECT *)H_Field,Hf_Stack);
            Found = TRUE;
            Ray_Ht_Field_Tests_Succeeded++;
            }
          }
        }
      }
    }

  return(Found);
  }

int Intersect_Sub_Block(Block, Ray, H_Field, start, end)
HF_BLOCK *Block;
RAY *Ray;
HEIGHT_FIELD *H_Field;
VECTOR *start, *end;
  {
  DBL y1, y2;
  DBL sx, sy, sz, ex, ez, f, tx, tz;
  int ix, iz, length, i;

  if(min_value(start->y,end->y) > (DBL)Block->max_y)
    return(FALSE);

  if(max_value(start->y,end->y) < (DBL)Block->min_y)
    return(FALSE);

  sx = start->x;
  ex = end->x;
  sz = start->z;
  ez = end->z;
  sy = start->y;

  if(X_Dom) 
    {
    if(isdx >= 0) 
      {
      f = floor(sx) - sx;
      sx = floor(sx);
      sy += Myx * f;
      sz += Mzx * f;
      ex = ceil(ex) - 1.0;
      ix = (int)sx;
      }
    else 
      {
      f = ceil(sx) - sx;
      sx = ceil(sx) - 1.0;
      sy += Myx * f;
      sz += Mzx * f;
      ex = floor(ex);
      ix = (int)sx;
      }

      length = (int)abs((int)ex - (int)sx);

    if (isdz >= 0) 
      {
      tz = floor(start->z) + 1.0;
      iz = (int)start->z;
      f = sz - tz;
      }
    else 
      {
      tz = ceil(start->z) - 1.0;
      iz = (int)tz;
      f = tz - sz;
      }

      if(Gdy >= 0.0) 
      {
      y1 = sy;
      y2 = sy + Gdy;
      }
      else 
      {
      y1 = sy + Gdy;
      y2 = sy;
      }

      for(i=0;i<=length;i++) 
      {
      if(Intersect_Pixel(ix,iz,Ray,H_Field,y1,y2))
        return(TRUE);
      f += Gdz;
      if(f>0.0) 
        {
        iz += isdz;
        if(Intersect_Pixel(ix,iz,Ray,H_Field,y1,y2))
          return(TRUE);
        f -= 1.0;
        }
      ix += isdx;
      y1 += Gdy;
      y2 += Gdy;
      }
    }
  else 
    {
    if(isdz >= 0) 
      {
      f = floor(sz) - sz;
      sz = floor(sz);
      sy += Myz * f;
      sx += Mxz * f;
      ez = ceil(ez) - 1.0;
      iz = (int)sz;
      }
    else 
      {
      f = ceil(sz) - sz;
      sz = ceil(sz) - 1.0;
      sy += Myz * f;
      sx += Mxz * f;
      ez = floor(ez);
      iz = (int)sz;
      }             

      length = (int)abs((int)ez - (int)sz);

    if (isdx >= 0) 
      {
      tx = floor(start->x) + 1.0;
      ix = (int)start->x;
      f = sx - tx;
      }
    else 
      {
      tx = ceil(start->x) - 1.0;
      ix = (int)tx;
      f = tx - sx;
      }

      if(Gdy >= 0.0) 
      {
      y1 = sy;
      y2 = sy + Gdy;
      }
      else 
      {
      y1 = sy + Gdy;
      y2 = sy;
      }

      for(i=0;i<=length;i++) 
      {
      if(Intersect_Pixel(ix,iz,Ray,H_Field,y1,y2))
        return(TRUE);
      f += Gdx;
      if(f>0.0) 
        {
        ix += isdx;
        if(Intersect_Pixel(ix,iz,Ray,H_Field,y1,y2))
          return(TRUE);
        f -= 1.0;
        }
      iz += isdz;
      y1 += Gdy;
      y2 += Gdy;
      }
    }
  return (FALSE);
  }

static int Intersect_Csg_Sub_Block(Block, Ray, H_Field, start, end)
HF_BLOCK *Block;
RAY *Ray;
HEIGHT_FIELD *H_Field;
VECTOR *start, *end;
  {
  DBL y1, y2;
  DBL sx, sy, sz, ex, ez, f, tx, tz;
  int ix, iz, length, i, retval;

  if(min_value(start->y,end->y) > (DBL)Block->max_y)
    return(FALSE);

  if(max_value(start->y,end->y) < (DBL)Block->min_y)
    return(FALSE);

  retval = FALSE;
  sx = start->x;
  ex = end->x;
  sz = start->z;
  ez = end->z;
  sy = start->y;

  if(X_Dom) 
    {
    if(isdx >= 0) 
      {
      f = floor(sx) - sx;
      sx = floor(sx);
      sy += Myx * f;
      sz += Mzx * f;
      ex = ceil(ex) - 1.0;
      ix = (int)sx;
      }
    else 
      {
      f = ceil(sx) - sx;
      sx = ceil(sx) - 1.0;
      sy += Myx * f;
      sz += Mzx * f;
      ex = floor(ex);
      ix = (int)sx;
      }

      length = (int) abs((int)ex - (int)sx);

    if (isdz >= 0) 
      {
      tz = floor(start->z) + 1.0;
      iz = (int)start->z;
      f = sz - tz;
      }
    else 
      {
      tz = ceil(start->z) - 1.0;
      iz = (int)tz;
      f = tz - sz;
      }

      if(Gdy >= 0.0) 
      {
      y1 = sy;
      y2 = sy + Gdy;
      }
      else 
      {
      y1 = sy + Gdy;
      y2 = sy;
      }

      for(i=0;i<=length;i++) 
      {
      if(Intersect_Pixel(ix,iz,Ray,H_Field,y1,y2))
        retval = TRUE;
      f += Gdz;
      if(f>0.0) 
        {
        iz += isdz;
        if(Intersect_Pixel(ix,iz,Ray,H_Field,y1,y2))
          retval = TRUE;
        f -= 1.0;
        }
      ix += isdx;
      y1 += Gdy;
      y2 += Gdy;
      }
    }
  else 
    {
    if(isdz >= 0) 
      {
      f = floor(sz) - sz;
      sz = floor(sz);
      sy += Myz * f;
      sx += Mxz * f;
      ez = ceil(ez) - 1.0;
      iz = (int)sz;
      }
    else 
      {
      f = ceil(sz) - sz;
      sz = ceil(sz) - 1.0;
      sy += Myz * f;
      sx += Mxz * f;
      ez = floor(ez);
      iz = (int)sz;
      }             

      length = (int)abs((int)ez - (int)sz);

    if (isdx >= 0) 
      {
      tx = floor(start->x) + 1.0;
      ix = (int)start->x;
      f = sx - tx;
      }
    else 
      {
      tx = ceil(start->x) - 1.0;
      ix = (int)tx;
      f = tx - sx;
      }

      if(Gdy >= 0.0) 
      {
      y1 = sy;
      y2 = sy + Gdy;
      }
      else 
      {
      y1 = sy + Gdy;
      y2 = sy;
      }

      for(i=0;i<=length;i++) 
      {
      if(Intersect_Pixel(ix,iz,Ray,H_Field,y1,y2))
        retval = TRUE;
      f += Gdx;
      if(f>0.0) 
        {
        ix += isdx;
        if(Intersect_Pixel(ix,iz,Ray,H_Field,y1,y2))
          retval = TRUE;
        f -= 1.0;
        }
      iz += isdz;
      y1 += Gdy;
      y2 += Gdy;
      }
    }
  return (retval);
  }

int Intersect_Hf_Node(Ray, H_Field, start, end)
RAY *Ray;
HEIGHT_FIELD *H_Field;
VECTOR *start, *end;
  {
  VECTOR *curr, *next, *temp, temp1, temp2;
  DBL sx, sy, sz, ex, ey, ez, x, y, z;
  DBL tnear, tfar, t, bsx, bsz, bex, bez;
  int ix, iz, x_size, z_size, length, i;

  x = sx = start->x;
  y = sy = start->y;
  z = sz = start->z;
  ex = end->x;
  ey = end->y;
  ez = end->z;

  bsx = sx * Inv_Blk_Size;
  bsz = sz * Inv_Blk_Size;
  bex = ex * Inv_Blk_Size;
  bez = ez * Inv_Blk_Size;

  if (isdx >= 0) 
    {
    bsx = floor(bsx);
    bex = ceil(bex) - 1.0;
    }
  else 
    {
    bsx = ceil(bsx) - 1.0;
    bex = floor(bex);
    }

    if (isdz >= 0) 
    {
    bsz = floor(bsz);
    bez = ceil(bez) - 1.0;
    }
    else 
    {
    bsz = ceil(bsz) - 1.0;
    bez = floor(bez);
    }

    x_size = abs((int)bex - (int)bsx);
  z_size = abs((int)bez - (int)bsz);

  length = x_size + z_size;

  curr = &temp1;
  next = &temp2;
  Make_Vector(curr, x, y, z);
  t = 0.0;

  if(X_Dom) 
    {
    if(isdx >= 0) 
      {
      ix = (int)floor(sx*Inv_Blk_Size);
      tnear = Block_Size*(ix+1) - sx;

      if (isdz >= 0) 
        {
        iz = (int)floor(sz*Inv_Blk_Size);
        tfar = Gdx * (Block_Size*(iz+1) - sz);
        }
      else 
        {
        iz = (int)ceil(sz*Inv_Blk_Size) - 1;
        tfar = Gdx * (sz - Block_Size*(iz));
        }                        
      for (i = 0; i < length; i++) 
        {
        if(tfar < tnear) 
          {
          t = tfar;
          x = sx + t;
          y = sy + Myx * t;
          z = sz + Mzx * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            return(TRUE);
          temp = curr;
          curr = next;
          next = temp;
          iz += isdz;
          if (isdz >= 0) 
            tfar = Gdx * (Block_Size*(iz+1) - sz);
          else 
            tfar = Gdx * (sz - Block_Size*(iz));
          }
        else 
          {
          t = tnear;
          x = sx + t;
          y = sy + Myx * t;
          z = sz + Mzx * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            return(TRUE);
          temp = curr;
          curr = next;
          next = temp;
          ix++;
          tnear = Block_Size*(ix+1) - sx;
          }
        }
      }
    else 
      {                          
      ix = (int)ceil(sx*Inv_Blk_Size) - 1;
      tnear = sx - Block_Size*(ix);

        if (isdz >= 0) 
        {
        iz = (int)floor(sz*Inv_Blk_Size);
        tfar = Gdx * (Block_Size*(iz+1) - sz);
        }
        else 
        {
        iz = (int)ceil(sz*Inv_Blk_Size) - 1;
        tfar = Gdx * (sz - Block_Size*(iz));
        }

        for (i = 0; i < length; i++) 
        {
        if(tfar < tnear) 
          {
          t = tfar;
          x = sx - t;
          y = sy - Myx * t;
          z = sz - Mzx * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            return(TRUE);
          temp = curr;
          curr = next;
          next = temp;
          iz += isdz;
          if (isdz >= 0) 
            tfar = Gdx * (Block_Size*(iz+1) - sz);
          else 
            tfar = Gdx * (sz - Block_Size*(iz));
          }
        else 
          {
          t = tnear;
          x = sx - t;
          y = sy - Myx * t;
          z = sz - Mzx * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            return(TRUE);
          temp = curr;
          curr = next;
          next = temp;
          ix--;
          tnear = sx - Block_Size*(ix);
          }
        }
      }                             
    }
  else 
    {
    if(isdz >= 0) 
      {
      iz = (int)floor(sz*Inv_Blk_Size);
      tnear = Block_Size*(iz+1) - sz;

      if (isdx >= 0) 
        {
        ix = (int)floor(sx*Inv_Blk_Size);       
        tfar = Gdz * (Block_Size*(ix+1) - sx);
        }
      else 
        {
        ix = (int)ceil(sx*Inv_Blk_Size) - 1;
        tfar = Gdz * (sx - Block_Size*(ix));
        }
      for (i = 0; i < length; i++) 
        {
        if(tfar < tnear) 
          {
          t = tfar;
          z = sz + t;
          y = sy + Myz * t;
          x = sx + Mxz * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            return(TRUE);
          temp = curr;
          curr = next;
          next = temp;
          ix += isdx;
          if (isdx >= 0) 
            tfar = Gdz * (Block_Size*(ix+1) - sx);
          else 
            tfar = Gdz * (sx - Block_Size*(ix));
          }
        else 
          {
          t = tnear;
          z = sz + t;
          y = sy + Myz * t;
          x = sx + Mxz * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            return(TRUE);
          temp = curr;
          curr = next;
          next = temp;
          iz++;
          tnear = Block_Size*(iz+1) - sz;
          }
        }
      }
    else 
      {                          
      iz = (int)ceil(sz*Inv_Blk_Size) - 1;
      tnear = sz - Block_Size*(iz);

        if (isdx >= 0) 
        {
        ix = (int)floor(sx*Inv_Blk_Size);
        tfar = Gdz * (Block_Size*(ix+1) - sx);
        }
        else 
        {
        ix = (int)ceil(sx*Inv_Blk_Size) - 1;
        tfar = Gdz * (sx - Block_Size*(ix));
        }                                   
      for (i = 0; i < length; i++) 
        {
        if(tfar < tnear) 
          {
          t = tfar;
          z = sz - t;
          y = sy - Myz * t;
          x = sx - Mxz * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            return(TRUE);
          temp = curr;
          curr = next;
          next = temp;
          ix += isdx;
          if (isdx >= 0) 
            tfar = Gdz * (Block_Size*(ix+1) - sx);
          else
            tfar = Gdz * (sx - Block_Size*(ix));
          }
        else 
          {
          t = tnear;
          z = sz - t;
          y = sy - Myz * t;
          x = sx - Mxz * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            return(TRUE);
          temp = curr;
          curr = next;
          next = temp;
          iz--;
          tnear = sz - Block_Size*iz;
          }
        }
      }                                     
    }
  Make_Vector(next,ex,ey,ez);
  if(isdx >= 0)
    ix = (int)floor(ex*Inv_Blk_Size);
  else
    ix = (int)ceil(ex*Inv_Blk_Size) - 1;
  if(isdz >= 0)
    iz = (int)floor(ez*Inv_Blk_Size);
  else
    iz = (int)ceil(ez*Inv_Blk_Size) - 1;
  if(Intersect_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
    return(TRUE);
  return (FALSE);
  }

static int Intersect_Csg_Hf_Node(Ray, H_Field, start, end)
RAY *Ray;
HEIGHT_FIELD *H_Field;
VECTOR *start, *end;
  {
  VECTOR *curr, *next, *temp, temp1, temp2;
  DBL sx, sy, sz, ex, ey, ez, x, y, z;
  DBL tnear, tfar, t, bsx, bsz, bex, bez;
  int ix, iz, x_size, z_size, length, i, retval;

  retval = FALSE;
  x = sx = start->x;
  y = sy = start->y;
  z = sz = start->z;
  ex = end->x;
  ey = end->y;
  ez = end->z;

  bsx = sx * Inv_Blk_Size;
  bsz = sz * Inv_Blk_Size;
  bex = ex * Inv_Blk_Size;
  bez = ez * Inv_Blk_Size;

  if (isdx >= 0) 
    {
    bsx = floor(bsx);
    bex = ceil(bex) - 1.0;
    }
  else 
    {
    bsx = ceil(bsx) - 1.0;
    bex = floor(bex);
    }

    if (isdz >= 0) 
    {
    bsz = floor(bsz);
    bez = ceil(bez) - 1.0;
    }
    else 
    {
    bsz = ceil(bsz) - 1.0;
    bez = floor(bez);
    }

    x_size = abs((int)bex - (int)bsx);
  z_size = abs((int)bez - (int)bsz);

  length = x_size + z_size;

  curr = &temp1;
  next = &temp2;
  Make_Vector(curr, x, y, z);
  t = 0.0;

  if(X_Dom) 
    {
    if(isdx >= 0) 
      {
      ix = (int)floor(sx*Inv_Blk_Size);
      tnear = Block_Size*(ix+1) - sx;

      if (isdz >= 0) 
        {
        iz = (int)floor(sz*Inv_Blk_Size);
        tfar = Gdx * (Block_Size*(iz+1) - sz);
        }
      else 
        {
        iz = (int)ceil(sz*Inv_Blk_Size) - 1;
        tfar = Gdx * (sz - Block_Size*(iz));
        }                        
      for (i = 0; i < length; i++) 
        {
        if(tfar < tnear) 
          {
          t = tfar;
          x = sx + t;
          y = sy + Myx * t;
          z = sz + Mzx * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Csg_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            retval = TRUE;
          temp = curr;
          curr = next;
          next = temp;
          iz += isdz;
          if (isdz >= 0) 
            tfar = Gdx * (Block_Size*(iz+1) - sz);
          else 
            tfar = Gdx * (sz - Block_Size*(iz));
          }
        else 
          {
          t = tnear;
          x = sx + t;
          y = sy + Myx * t;
          z = sz + Mzx * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Csg_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            retval = TRUE;
          temp = curr;
          curr = next;
          next = temp;
          ix++;
          tnear = Block_Size*(ix+1) - sx;
          }
        }
      }
    else 
      {                          
      ix = (int)ceil(sx*Inv_Blk_Size) - 1;
      tnear = sx - Block_Size*(ix);

        if (isdz >= 0) 
        {
        iz = (int)floor(sz*Inv_Blk_Size);
        tfar = Gdx * (Block_Size*(iz+1) - sz);
        }
        else 
        {
        iz = (int)ceil(sz*Inv_Blk_Size) - 1;
        tfar = Gdx * (sz - Block_Size*(iz));
        }

        for (i = 0; i < length; i++) 
        {
        if(tfar < tnear) 
          {
          t = tfar;
          x = sx - t;
          y = sy - Myx * t;
          z = sz - Mzx * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Csg_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            retval = TRUE;
          temp = curr;
          curr = next;
          next = temp;
          iz += isdz;
          if (isdz >= 0) 
            tfar = Gdx * (Block_Size*(iz+1) - sz);
          else 
            tfar = Gdx * (sz - Block_Size*(iz));
          }
        else 
          {
          t = tnear;
          x = sx - t;
          y = sy - Myx * t;
          z = sz - Mzx * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Csg_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            retval = TRUE;
          temp = curr;
          curr = next;
          next = temp;
          ix--;
          tnear = sx - Block_Size*(ix);
          }
        }
      }                             
    }
  else 
    {
    if(isdz >= 0) 
      {
      iz = (int)floor(sz*Inv_Blk_Size);
      tnear = Block_Size*(iz+1) - sz;

      if (isdx >= 0) 
        {
        ix = (int)floor(sx*Inv_Blk_Size);       
        tfar = Gdz * (Block_Size*(ix+1) - sx);
        }
      else 
        {
        ix = (int)ceil(sx*Inv_Blk_Size) - 1;
        tfar = Gdz * (sx - Block_Size*(ix));
        }
      for (i = 0; i < length; i++) 
        {
        if(tfar < tnear) 
          {
          t = tfar;
          z = sz + t;
          y = sy + Myz * t;
          x = sx + Mxz * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Csg_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            retval = TRUE;
          temp = curr;
          curr = next;
          next = temp;
          ix += isdx;
          if (isdx >= 0) 
            tfar = Gdz * (Block_Size*(ix+1) - sx);
          else 
            tfar = Gdz * (sx - Block_Size*(ix));
          }
        else 
          {
          t = tnear;
          z = sz + t;
          y = sy + Myz * t;
          x = sx + Mxz * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Csg_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            retval = TRUE;
          temp = curr;
          curr = next;
          next = temp;
          iz++;
          tnear = Block_Size*(iz+1) - sz;
          }
        }
      }
    else 
      {                          
      iz = (int)ceil(sz*Inv_Blk_Size) - 1;
      tnear = sz - Block_Size*(iz);

        if (isdx >= 0) 
        {
        ix = (int)floor(sx*Inv_Blk_Size);
        tfar = Gdz * (Block_Size*(ix+1) - sx);
        }
        else 
        {
        ix = (int)ceil(sx*Inv_Blk_Size) - 1;
        tfar = Gdz * (sx - Block_Size*(ix));
        }                                   
      for (i = 0; i < length; i++) 
        {
        if(tfar < tnear) 
          {
          t = tfar;
          z = sz - t;
          y = sy - Myz * t;
          x = sx - Mxz * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Csg_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            retval = TRUE;
          temp = curr;
          curr = next;
          next = temp;
          ix += isdx;
          if (isdx >= 0) 
            tfar = Gdz * (Block_Size*(ix+1) - sx);
          else 
            tfar = Gdz * (sx - Block_Size*(ix));
          }
        else 
          {
          t = tnear;
          z = sz - t;
          y = sy - Myz * t;
          x = sx - Mxz * t;
          Make_Vector(next, x, y, z);
          if(Intersect_Csg_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
            retval = TRUE;
          temp = curr;
          curr = next;
          next = temp;
          iz--;
          tnear = sz - Block_Size*iz;
          }
        }
      }                                     
    }
  Make_Vector(next,ex,ey,ez);
  if(isdx >= 0)
    ix = (int)floor(ex*Inv_Blk_Size);
  else
    ix = (int)ceil(ex*Inv_Blk_Size) - 1;
  if(isdz >= 0)
    iz = (int)floor(ez*Inv_Blk_Size);
  else
    iz = (int)ceil(ez*Inv_Blk_Size) - 1;
  if(Intersect_Csg_Sub_Block(&(H_Field->Block[ix][iz]),Ray,H_Field,curr,next))
    retval = TRUE;
  return (retval);
  }

static int
add_single_normal(data, xsize, zsize, x0, z0, x1, z1, x2, z2, N)
  HF_val **data;
  int xsize,zsize,x0,z0,x1,z1,x2,z2;
  VECTOR *N;
  {
  VECTOR v0, v1, v2, t0, t1, Nt;

  if (x0 < 0 || z0 < 0 ||
    x1 < 0 || z1 < 0 ||
    x2 < 0 || z2 < 0 ||
    x0 > xsize || z0 > zsize ||
    x1 > xsize || z1 > zsize ||
    x2 > xsize || z2 > zsize) 
    {
    return 0;
    }
  else 
    {
    Make_Vector(&v0, x0, (DBL)data[z0][x0], z0);
    Make_Vector(&v1, x1, (DBL)data[z1][x1], z1);
    Make_Vector(&v2, x2, (DBL)data[z2][x2], z2);
    VSub(t0, v2, v0);
    VSub(t1, v1, v0);
    VCross(Nt, t0, t1);
    Normalize(&Nt, &Nt);
    if(Nt.y < 0.0) 
      {
      VScale(Nt,Nt,-1.0);
      }
    VAdd(*N, *N, Nt);
    return 1;
    }
  }

/* Given a height field that only contains an elevation grid, this
   routine will walk through the data and produce averaged normals
   for all points on the grid. */
static void
smooth_height_field(hf, xsize, zsize)
  HEIGHT_FIELD *hf;
  int xsize;
  int zsize;
  {
  int i, j, k;
  VECTOR N;
  HF_val **map = hf->Map;

  /* First off, allocate all the memory needed to store the
      normal information */
  hf->Normals = (HF_Normals **)malloc((zsize+1) * sizeof(HF_Normals *));
  if (hf->Normals == NULL) 
    {
    fprintf(stderr, "Failed to allocate hf->norm\n");
    exit(1);
    }
  for (i=0; i<=zsize; i++) 
    {
    hf->Normals[i] = (HF_Normals *)malloc((xsize+1) * sizeof(HF_Normals));
    if (hf->Normals[i] == NULL) 
      {
      fprintf(stderr, "Failed to allocate hf->norm[%d]\n", i);
      exit(1);
      }
    }

  /* For now we will do it the hard way - by generating the normals
      individually for each elevation point */
  for (i=0;i<=zsize;i++) 
    {
    COOPERATE 
    if((i%(int)Block_Size) == 0) fprintf(stderr,".");
    for (j=0;j<=xsize;j++) 
      {
      Make_Vector(&N, 0.0, 0.0, 0.0);
      k = 0;
      /*
         k += add_single_normal(map, xsize, zsize, j, i, j+1, i, j, i+1, &N);
         k += add_single_normal(map, xsize, zsize, j+1, i+1, j, i+1, j+1, i, &N);
         k += add_single_normal(map, xsize, zsize, j, i+1, j-1, i+1, j, i, &N);
         k += add_single_normal(map, xsize, zsize, j-1, i, j, i, j-1, i+1, &N);
         k += add_single_normal(map, xsize, zsize, j, i, j-1, i, j, i-1, &N);
         k += add_single_normal(map, xsize, zsize, j-1, i-1, j, i-1, j-1, i, &N);
         k += add_single_normal(map, xsize, zsize, j, i-1, j+1, i-1, j, i, &N);
         k += add_single_normal(map, xsize, zsize, j+1, i, j, i, j+1, i-1, &N);
*/
      k += add_single_normal(map, xsize, zsize, j, i, j+1, i, j, i+1, &N);
      k += add_single_normal(map, xsize, zsize, j, i, j, i+1, j-1, i, &N);
      k += add_single_normal(map, xsize, zsize, j, i, j-1, i, j, i-1, &N);
      k += add_single_normal(map, xsize, zsize, j, i, j, i-1, j+1, i, &N);

      if (k == 0) 
        {
        fprintf(stderr, "Failed to find any normals at: (%d, %d)\n", i, j);
        exit(1);
        }
      Normalize(&N, &N);
      hf->Normals[i][j][0] = (short)(32767 * N.x);
      hf->Normals[i][j][1] = (short)(32767 * N.y);
      hf->Normals[i][j][2] = (short)(32767 * N.z);
      /* printf("n[%d,%d]: <%g %g %g>\n", j, i, N.x, N.y, N.z); */
      }
    }
  }   

void Find_Hf_Min_Max(H_Field, Image)
HEIGHT_FIELD *H_Field;
IMAGE *Image;
  {
  int n, i, i2, j, j2, x, z, w, h, max_x, max_z, temp1, temp2;
  DBL size;
  HF_val temp_y;

  max_x = Image->iwidth;
  if(Image->File_Type == POT_FILE) max_x = max_x/2;
  max_z = Image->iheight;

  size = (DBL)max_value(max_x, max_z);
  H_Field->Block_Size  = Block_Size = ceil(sqrt(size+1.0));
  H_Field->Inv_Blk_Size = Inv_Blk_Size = 1.0/Block_Size;
  n = (int)Block_Size;

  w = (int)ceil((max_x+1.0)*Inv_Blk_Size);
  h = (int)ceil((max_z+1.0)*Inv_Blk_Size);

  H_Field->Map = (HF_val **)calloc(max_z+1, sizeof(HF_val *));
  if (H_Field->Map == NULL)
    fprintf(stderr,"Cannot allocate memory for height field\n");

  H_Field->Block = (HF_BLOCK **)calloc(w,sizeof(HF_BLOCK *));
  if(H_Field->Block == NULL)
    fprintf(stderr, "Cannot allocate memory for height field buffer\n");
  for(i=0; i<w; i++) 
    {
    H_Field->Block[i] = (HF_BLOCK *)calloc(h,sizeof(HF_BLOCK));
    if (H_Field->Block[i] == NULL)
      fprintf(stderr, "Cannot allocate memory for height field buffer line\n"
        );
    for(j=0; j<h; j++) 
      {
      H_Field->Block[i][j].min_y = 65535;
      H_Field->Block[i][j].max_y = 0;
      }
    }

  H_Field->Map[0] = (HF_val *)calloc(max_x+1,sizeof(HF_val));
  if (H_Field->Map[0] == NULL)
    fprintf(stderr,"Cannot allocate memory for height field\n");

  for(j=0; j < h; j++)
    {
    for(j2=0;(j2 <= n) && (j*n+j2 <= max_z);j2++)
      {
      z = j*n+j2;
      if(j2!=0)
        {
        H_Field->Map[z] = (HF_val *)calloc(max_x+1,sizeof(HF_val));
        if (H_Field->Map[z] == NULL)
          fprintf(stderr, "Cannot allocate memory for height field\n");
        }

      COOPERATE 
      for(i=0; i < w; i++)
        {
        for(i2=0;(i2 <= n)&&(i*n+i2 <= max_x);i2++)
          {
          x = i*n+i2;
          if((x >= 0) && (x < max_x) && (z >= 0) && (z < max_z)) 
            {
            switch(Image->File_Type) 
            {
            case GIF_FILE:
              temp1 = Image->data.map_lines[max_z - z - 1][x];
              temp_y = (HF_val)(256*temp1);
              break;
            case POT_FILE:
              temp1 = Image->data.map_lines[max_z - z - 1][x];
              temp2 = Image->data.map_lines[max_z - z - 1][x + max_x];
              temp_y = (HF_val)(256*temp1 + temp2);
              break;
            case TGA_FILE:
              if (Image->data.rgb_lines != NULL) 
                {
                temp1 = Image->data.rgb_lines[max_z - z - 1].red[x];
                temp2 = Image->data.rgb_lines[max_z - z - 1].green[x];
                }
              else 
                {
                temp1 = Image->data.map_lines[max_z - z - 1][x];
                temp2 = 0;
                }
              temp_y = (HF_val)(256*temp1 + temp2);
              break;
            }
            H_Field->Map[z][x] = temp_y;
            }
          else 
            {
            if (z == max_z) 
              {
              H_Field->Map[z][x] = H_Field->Map[z-1][x];
              }
            if (x == max_x) 
              {
              H_Field->Map[z][x] = H_Field->Map[z][x-1];
              }
            temp_y = H_Field->Map[z][x];
            }

          if(temp_y < H_Field->Block[i][j].min_y)
            H_Field->Block[i][j].min_y = temp_y;
          if(temp_y > H_Field->Block[i][j].max_y)
            H_Field->Block[i][j].max_y = temp_y;
          }
        }
      if((z >= 0) && (z < max_z) && (j2!=n)) 
        {
        switch (Image->File_Type) 
        {
        case GIF_FILE: 
          free(Image->data.map_lines[max_z - z - 1]); break;
        case POT_FILE: 
          free(Image->data.map_lines[max_z - z - 1]); break;
        case TGA_FILE:
          if (Image->data.rgb_lines != NULL) 
            {
            free(Image->data.rgb_lines[max_z - z - 1].blue);
            free(Image->data.rgb_lines[max_z - z - 1].green);
            free(Image->data.rgb_lines[max_z - z - 1].red);
            }
          else 
            {
            free(Image->data.map_lines[max_z - z - 1]);
            }
          break;
        }
        }
      }
    }

    /* If this is a smoothed height field, then allocate storage for
      the normals & compute them */
    if (H_Field->Smoothed)
      smooth_height_field(H_Field, max_x, max_z);

  }

int All_HeightFld_Intersections(Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  VECTOR Temp1, Temp2;
  RAY Temp_Ray;
  DBL depth1, depth2;
  int ret_val = FALSE;
  HEIGHT_FIELD *H_Field = (HEIGHT_FIELD *) Object;

  Ray_Ht_Field_Tests++;

  MInvTransPoint(&(Temp_Ray.Initial),&(Ray->Initial),H_Field->Trans);
  MInvTransDirection(&(Temp_Ray.Direction),&(Ray->Direction),H_Field->Trans);

  if(!Intersect_Boxx(&Temp_Ray,H_Field->bounding_box,&depth1,&depth2))
    return(FALSE);     

  H_Field->cache_pos = 0;
  Block_Size = H_Field->Block_Size;
  Inv_Blk_Size = H_Field->Inv_Blk_Size;

  if( depth1 == depth2) 
    {
    depth1 = Small_Tolerance;
    VScale(Temp1,Temp_Ray.Direction,depth1);
    VAddEq(Temp1,Temp_Ray.Initial);
    VScale(Temp2,Temp_Ray.Direction,depth2);
    VAddEq(Temp2,Temp_Ray.Initial);
    }
  else 
    {
    VScale(Temp1,Temp_Ray.Direction,depth1);
    VAddEq(Temp1,Temp_Ray.Initial);
    VScale(Temp2,Temp_Ray.Direction,depth2);
    VAddEq(Temp2,Temp_Ray.Initial);        
    }

  mindist = depth1;
  maxdist = depth2;

  if(fabs(Temp_Ray.Direction.x) > EPSILON) 
    {
    Mzx = Temp_Ray.Direction.z/Temp_Ray.Direction.x;
    Myx = Temp_Ray.Direction.y/Temp_Ray.Direction.x;
    }
  else 
    {
    Mzx = Temp_Ray.Direction.z/EPSILON;
    Myx = Temp_Ray.Direction.y/EPSILON;
    }
  if(fabs(Temp_Ray.Direction.z) > EPSILON) 
    {
    Mxz = Temp_Ray.Direction.x/Temp_Ray.Direction.z;
    Myz = Temp_Ray.Direction.y/Temp_Ray.Direction.z;
    }
  else 
    {
    Mxz = Temp_Ray.Direction.x/EPSILON;
    Myz = Temp_Ray.Direction.y/EPSILON;
    }

  Hf_Stack = Depth_Stack;
  RRay = Ray;

  isdx = sign(Temp_Ray.Direction.x);
  isdz = sign(Temp_Ray.Direction.z);

  X_Dom = FALSE;
  if(fabs(Temp_Ray.Direction.x) >= fabs(Temp_Ray.Direction.z))
    X_Dom = TRUE;

  Gdx = fabs(Mxz);
  Gdz = fabs(Mzx);
  if(X_Dom) 
    {
    Gdy = Myx * (DBL)isdx;
    }
  else 
    {
    Gdy = Myz * (DBL)isdz;
    }

  if(Intersect_Hf_Node(&Temp_Ray, H_Field, &Temp1, &Temp2))
    ret_val = TRUE;
  return(ret_val);
  }

int All_Csg_HeightFld_Intersections(Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;  
ISTACK *Depth_Stack;
  {
  VECTOR Temp1, Temp2;
  RAY Temp_Ray;
  DBL depth1, depth2;
  int ret_val = FALSE;
  HEIGHT_FIELD *H_Field = (HEIGHT_FIELD *) Object;

  Ray_Ht_Field_Tests++;

  MInvTransPoint(&(Temp_Ray.Initial),&(Ray->Initial),H_Field->Trans);
  MInvTransDirection(&(Temp_Ray.Direction),&(Ray->Direction),H_Field->Trans);

  if(!Intersect_Boxx(&Temp_Ray,H_Field->bounding_box,&depth1,&depth2))
    return(FALSE);     

  H_Field->cache_pos = 0;          
  Block_Size = H_Field->Block_Size;
  Inv_Blk_Size = H_Field->Inv_Blk_Size;

  if( depth1 == depth2) 
    {
    depth1 = Small_Tolerance;
    VScale(Temp1,Temp_Ray.Direction,depth1);
    VAddEq(Temp1,Temp_Ray.Initial);
    VScale(Temp2,Temp_Ray.Direction,depth2);
    VAddEq(Temp2,Temp_Ray.Initial);
    }
  else 
    {
    VScale(Temp1,Temp_Ray.Direction,depth1);
    VAddEq(Temp1,Temp_Ray.Initial);
    VScale(Temp2,Temp_Ray.Direction,depth2);
    VAddEq(Temp2,Temp_Ray.Initial);             
    }

    mindist = depth1;
  maxdist = depth2;

  if(fabs(Temp_Ray.Direction.x) > EPSILON) 
    {
    Mzx = Temp_Ray.Direction.z/Temp_Ray.Direction.x;
    Myx = Temp_Ray.Direction.y/Temp_Ray.Direction.x;
    }
  else 
    {
    Mzx = Temp_Ray.Direction.z/EPSILON;
    Myx = Temp_Ray.Direction.y/EPSILON;
    }
  if(fabs(Temp_Ray.Direction.z) > EPSILON) 
    {
    Mxz = Temp_Ray.Direction.x/Temp_Ray.Direction.z;
    Myz = Temp_Ray.Direction.y/Temp_Ray.Direction.z;
    }
  else 
    {
    Mxz = Temp_Ray.Direction.x/EPSILON;
    Myz = Temp_Ray.Direction.y/EPSILON;
    }

    Hf_Stack = Depth_Stack;
  RRay = Ray;

  isdx = sign(Temp_Ray.Direction.x);
  isdz = sign(Temp_Ray.Direction.z);

  X_Dom = FALSE;
  if(fabs(Temp_Ray.Direction.x) >= fabs(Temp_Ray.Direction.z))
    X_Dom = TRUE;

  Gdx = fabs(Mxz);
  Gdz = fabs(Mzx);
  if(X_Dom) 
    {
    Gdy = Myx * (DBL)isdx;
    }
  else 
    {
    Gdy = Myz * (DBL)isdz;
    }

    if(Intersect_Csg_Hf_Node(&Temp_Ray, H_Field, &Temp1, &Temp2))
      ret_val = TRUE;
  return(ret_val);
  }

int Inside_HeightFld (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  HEIGHT_FIELD *H_Field = (HEIGHT_FIELD *) Object;
  int px, pz;
  DBL x,z,y1,y2,y3,water, dot1, dot2;
  VECTOR Local_Origin, Temp1, Temp2, Local_Normal, Test;

  MInvTransPoint(&Test, IPoint, H_Field->Trans);

  water = H_Field->bounding_box->bounds[0].y;
  if ((Test.x < 0.0) || (Test.x >= H_Field->bounding_box->bounds[1].x) || (Test.z < 0.0) || (Test.z >= H_Field->bounding_box->bounds[1].z))
    return (H_Field->Inverted);

  if (Test.y >= H_Field->bounding_box->bounds[1].y)
    return (H_Field->Inverted);

  if (Test.y < water)
    return (H_Field->Inverted ^ TRUE);

  px = (int)Test.x;
  pz = (int)Test.z;
  x = Test.x - (DBL)px;
  z = Test.z - (DBL)pz;

  if((x+z)<1.0) 
    {
    y1 = max_value(Get_Height(px,pz,H_Field),water);
    y2 = max_value(Get_Height(px+1,pz,H_Field),water);
    y3 = max_value(Get_Height(px,pz+1,H_Field),water);
    Make_Vector(&Local_Origin,(DBL)px,y1,(DBL)pz);
    Temp1.x = 1.0;
    Temp1.z = 0.0;
    Temp1.y = y2 - y1;
    Temp2.x = 0.0;
    Temp2.z = 1.0;
    Temp2.y = y3 - y1;
    }
  else 
    {
    px = (int)ceil(Test.x);
    pz = (int)ceil(Test.z);
    y1 = max_value(Get_Height(px,pz,H_Field),water);
    y2 = max_value(Get_Height(px-1,pz,H_Field),water);
    y3 = max_value(Get_Height(px,pz-1,H_Field),water);
    Make_Vector(&Local_Origin,(DBL)px,y1,(DBL)pz);
    Temp1.x = -1.0;
    Temp1.z = 0.0;
    Temp1.y = y2 - y1;
    Temp2.x = 0.0;
    Temp2.z = -1.0;
    Temp2.y = y3 - y1;
    }
  VCross(Local_Normal,Temp2,Temp1);
  VDot(dot1,Test,Local_Normal);
  VDot(dot2,Local_Origin,Local_Normal);
  if(dot1 < dot2)
    return(TRUE^H_Field->Inverted);
  return(FALSE^H_Field->Inverted);
  }

static
DBL stretch (x)
DBL x;
  {
  if(x<=0.5) 
    {
    x = 2 * x*x;
    }
  else 
    {
    x = 1.0 - (2 * (1.0-x)*(1.0-x));
    }
  return x;
  }

  void HeightFld_Normal (Result, Object, IPoint)
    OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  HEIGHT_FIELD *H_Field = (HEIGHT_FIELD *) Object;
  int px,pz, i, code;
  DBL x,z,y1,y2,y3,u,v;
  VECTOR Local_Origin, Temp1, Temp2;
  VECTOR n[5];

  MInvTransPoint(&Local_Origin, IPoint, H_Field->Trans);

  for(i=0;i<H_Field->cache_pos;i++) 
    {
    if(((float)Local_Origin.x == H_Field->Normal_Vector[i].x) &&
      ((float)Local_Origin.z == H_Field->Normal_Vector[i].z)) 
      {
      *Result = H_Field->Normal_Vector[i].normal;
      MTransNormal(Result,Result,H_Field->Trans);
      VNormalize(*Result,*Result);
      return;
      }
    }

  px = (int)Local_Origin.x;
  pz = (int)Local_Origin.z;
  x = Local_Origin.x - (DBL)px;
  z = Local_Origin.z - (DBL)pz;

  px = (int)Local_Origin.x;
  pz = (int)Local_Origin.z;
  x = Local_Origin.x - (DBL)px;
  z = Local_Origin.z - (DBL)pz;
  if ((x+z) <= 1.0)
    code = LOWER_TRI;
  else 
    code = UPPER_TRI;

  if (H_Field->Smoothed) 
    {
    n[0].x = H_Field->Normals[pz][px][0];
    n[0].y = H_Field->Normals[pz][px][1];
    n[0].z = H_Field->Normals[pz][px][2];
    n[1].x = H_Field->Normals[pz][px+1][0];
    n[1].y = H_Field->Normals[pz][px+1][1];
    n[1].z = H_Field->Normals[pz][px+1][2];
    n[2].x = H_Field->Normals[pz+1][px][0];
    n[2].y = H_Field->Normals[pz+1][px][1];
    n[2].z = H_Field->Normals[pz+1][px][2];
    n[3].x = H_Field->Normals[pz+1][px+1][0];
    n[3].y = H_Field->Normals[pz+1][px+1][1];
    n[3].z = H_Field->Normals[pz+1][px+1][2];
    x = stretch(x);
    z = stretch(z);
    u = (1.0 - x);
    v = (1.0 - z);

    /* 	 n[4].x = u*n[0].x + x*n[1].x;
	 n[4].y = u*n[0].y + x*n[1].y;
	 n[4].z = u*n[0].z + x*n[1].z;

	 n[5].x = u*n[2].x + x*n[3].x;
	 n[5].y = u*n[2].y + x*n[3].y;
	 n[5].z = u*n[2].z + x*n[3].z;

	 n[6].x = v*n[0].x + z*n[2].x;
	 n[6].y = v*n[0].y + z*n[2].y;
	 n[6].z = v*n[0].z + z*n[2].z;

	 n[7].x = v*n[1].x + z*n[3].x;
	 n[7].y = v*n[1].y + z*n[3].y;
	 n[7].z = v*n[1].z + z*n[3].z;

	 Result->x = u*n[6].x + x*n[7].x + v*n[4].x + z*n[5].x;
	 Result->y = u*n[6].y + x*n[7].y + v*n[4].y + z*n[5].y;
	 Result->z = u*n[6].z + x*n[7].z + z*n[4].z + v*n[5].z;
*/
    Result->x = u*v*n[0].x + x*v*n[1].x + u*z*n[2].x + x*z*n[3].x;	 
    Result->y = u*v*n[0].y + x*v*n[1].y + u*z*n[2].y + x*z*n[3].y;	 
    Result->z = u*v*n[0].z + x*v*n[1].z + u*z*n[2].z + x*z*n[3].z;	 
    }
  else if (code == LOWER_TRI) 
    {
    y1 = Get_Height(px,pz,H_Field);
    y2 = Get_Height(px+1,pz,H_Field);
    y3 = Get_Height(px,pz+1,H_Field);
    Temp1.x = 1.0;
    Temp1.z = 0.0;
    Temp1.y = y2 - y1;
    Temp2.x = 0.0;
    Temp2.z = 1.0;
    Temp2.y = y3 - y1;
    VCross(*Result,Temp2,Temp1);
    }
  else 
    {
    y1 = Get_Height(px+1,pz+1,H_Field);
    y2 = Get_Height(px,pz+1,H_Field);
    y3 = Get_Height(px+1,pz,H_Field);
    Temp1.x = -1.0;
    Temp1.z = 0.0;
    Temp1.y = y2 - y1;
    Temp2.x = 0.0;
    Temp2.z = -1.0;
    Temp2.y = y3 - y1;
    VCross(*Result,Temp2,Temp1);
    }
  MTransNormal(Result,Result,H_Field->Trans);
  VNormalize(*Result,*Result);
  return;
  }

  void *Copy_HeightFld (Object)
    OBJECT *Object;
  {
  HEIGHT_FIELD *New;

  New = Create_Height_Field ();

  /*  Create_Height_Field creates a transform and a box as 
    does Copy_Transform and Copy_Box so destroy these.
*/

  Destroy_Transform(New->Trans);
  Destroy_Box((OBJECT *)(New->bounding_box));

  *New = *((HEIGHT_FIELD *)Object);

  New->Trans        = Copy_Transform (((HEIGHT_FIELD *)Object)->Trans);
  New->bounding_box = Copy_Box ((OBJECT *)(((HEIGHT_FIELD *)Object)->bounding_box));

  /* Note: For the time being we will not support copying the Block and Map
   arrays.  We will only copy the pointers.  This means Destroy_HeightFld
   must not destroy these arrays.  Actually it cannot because we don't
   really know how big they are!  
*/

  return (New);
  }

void Translate_HeightFld (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Translation_Transform(&Trans,Vector);
  Compose_Transforms(((HEIGHT_FIELD *) Object)->Trans,&Trans);
  }

void Rotate_HeightFld (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Rotation_Transform(&Trans,Vector);
  Compose_Transforms(((HEIGHT_FIELD *) Object)->Trans,&Trans);
  }

void Scale_HeightFld (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Scaling_Transform(&Trans,Vector);
  Compose_Transforms(((HEIGHT_FIELD *)Object)->Trans,&Trans);
  }

void Invert_HeightFld (Object)
OBJECT *Object;
  {
  ((HEIGHT_FIELD *)Object)->Inverted ^= TRUE;
  }

void Transform_HeightFld (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  Compose_Transforms(((HEIGHT_FIELD *)Object)->Trans, Trans);
  }

/* Allocate and intialize a Height Field */
HEIGHT_FIELD *Create_Height_Field()
  {
  HEIGHT_FIELD *New;

  if((New = (HEIGHT_FIELD *) malloc (sizeof(HEIGHT_FIELD))) == NULL)
    MAError ("height field");

  INIT_OBJECT_FIELDS(New, HEIGHT_FIELD_OBJECT, &Height_Field_Methods)

    /* Always uses Trans so always create one. */  
    New->Trans = Create_Transform (); 
  New->bounding_box = Create_Box (); 
  New->Block_Size   = 1.0;
  New->Inv_Blk_Size = 1.0;
  New->Block = NULL;
  New->Map   = NULL;
  New->Inverted = FALSE;
  New->cache_pos = 0;
  New->Smoothed = FALSE;
  New->Normals  = NULL;
  return(New);
  }

void Destroy_HeightFld (Object)
OBJECT *Object;
  {
  Destroy_Transform (((HEIGHT_FIELD *)Object)->Trans);
  Destroy_Box ((OBJECT *)((HEIGHT_FIELD *)Object)->bounding_box);
  free (Object);
  }
