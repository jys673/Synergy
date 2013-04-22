/****************************************************************************
*                blob.c
*
*  This module contains the code for the blob shape.
*
*  This file was written by Alexander Enzmann.	He wrote the code for
*  blobs and generously provided us these enhancements.
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

#ifndef min
#define min(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif

METHODS Blob_Methods =
  { 
  All_Blob_Intersections,
  Inside_Blob, Blob_Normal,
  Copy_Blob,
  Translate_Blob, Rotate_Blob, Scale_Blob, Transform_Blob,
  Invert_Blob, Destroy_Blob
};

extern long Ray_Blob_Tests, Ray_Blob_Tests_Succeeded;
extern int Shadow_Test_Flag;

static int determine_influences PARAMS((VECTOR *P, VECTOR *D, BLOB *blob, DBL mindist));
static DBL calculate_field_value PARAMS ((OBJECT *obj, VECTOR *Pos));

#ifndef Blob_Tolerance 
#define Blob_Tolerance 1.0e-3
#endif

#define COEFF_LIMIT 1.0e-20
#define INSIDE_TOLERANCE 1.0e-6

/* Starting with the density function: (1-r^2)^2, we have a field
   that varies in strength from 1 at r = 0 to 0 at r = 1.  By
   substituting r/rad for r, we can adjust the range of influence
   of a particular component.  By multiplication by coeff, we can
   adjust the amount of total contribution, giving the formula:
      coeff * (1 - (r/rad)^2)^2
   This varies in strength from coeff at r = 0, to 0 at r = rad. */
void
MakeBlob(blob, threshold, bloblist, npoints, sflag)
BLOB *blob;
DBL threshold;
blobstackptr bloblist;
int npoints;
int sflag;
  {
  unsigned i;
  DBL rad, coeff;
  blobstackptr temp;
  VECTOR mins, maxs;

  if (npoints < 1) 
    Error("Need at least one component in a blob.");

  blob->threshold = threshold;
  blob->list = (Blob_Element **)malloc(npoints*sizeof(Blob_Element *));
  if (blob->list == NULL) 
    MAError("blob data");
  for (i=0;i < (unsigned)npoints;i++) 
    {
    blob->list[i] = (Blob_Element *)malloc(sizeof(Blob_Element));
    if (blob->list[i] == NULL)
      MAError("blob data");
    }

  blob->count = npoints;
  blob->Sturm_Flag = sflag;

  /* Initialize the blob data */
  for(i=0;i < (unsigned)npoints;i++) 
    {
    temp = bloblist;
    if (fabs(temp->elem.coeffs[2]) < EPSILON ||
      temp->elem.radius2 < EPSILON) 
      {
      perror("Degenerate blob element\n");
      }
    /* Store blob specific information */
    rad = temp->elem.radius2;
    rad *= rad;
    coeff = temp->elem.coeffs[2];
    blob->list[i]->radius2   = rad;
    blob->list[i]->coeffs[2] = coeff;
    blob->list[i]->coeffs[1] = -(2.0 * coeff) / rad;
    blob->list[i]->coeffs[0] = coeff / (rad * rad);
    blob->list[i]->pos.x = temp->elem.pos.x;
    blob->list[i]->pos.y = temp->elem.pos.y;
    blob->list[i]->pos.z = temp->elem.pos.z;

    rad = temp->elem.radius2;
    if (i == 0) 
      {
      /* First component, just set the bounds */
      Make_Vector(&mins,
        temp->elem.pos.x-rad,
        temp->elem.pos.y-rad,
        temp->elem.pos.z-rad)
        Make_Vector(&maxs,
          temp->elem.pos.x+rad,
          temp->elem.pos.y+rad,
          temp->elem.pos.z+rad)
          }
    else 
      {
      /* Check min/max on the bounds */
        mins.x = min(mins.x, temp->elem.pos.x - rad);
      mins.y = min(mins.y, temp->elem.pos.y - rad);
      mins.z = min(mins.z, temp->elem.pos.z - rad);
      maxs.x = max(maxs.x, temp->elem.pos.x + rad);
      maxs.y = max(maxs.y, temp->elem.pos.y + rad);
      maxs.z = max(maxs.z, temp->elem.pos.z + rad);
      }

    bloblist = bloblist->next;
    free(temp);
    }

  blob->Bounds.Lower_Left = mins;
  VSub(blob->Bounds.Lengths, maxs, mins);

  /*  Allocate memory for intersection intervals */
  npoints *= 2;
  blob->intervals = (Blob_Interval *)malloc(npoints*sizeof(Blob_Interval));
  if (blob->intervals == NULL) 
    MAError("blob data");
  }

/* Make a sorted list of points along the ray that the various blob
   components start and stop adding their influence.  It would take
   a very complex blob (with many components along the current ray)
   to warrant the overhead of using a faster sort technique. */
static int
determine_influences(P, D, blob, mindist)
VECTOR *P, *D;
BLOB *blob;
DBL mindist;
  {
  int i, j, k, cnt;
  DBL b, t, t0, t1, disc;
  VECTOR V;
  Blob_Interval *intervals = blob->intervals;

  cnt = 0;
  for (i=0;i<blob->count;i++) 
    {
    /* Use standard sphere intersection routine
         to determine where the ray hits the volume
         of influence of each component of the blob. */
    VSub(V, blob->list[i]->pos, *P);
    VDot(b, V, *D);
    VDot(t, V, V);
    disc = b * b - t + blob->list[i]->radius2;
    if (disc < EPSILON)
      continue;
    disc = sqrt(disc);
    t1 = b + disc;
    if (t1 < mindist) t1 = 0.0;
    t0 = b - disc;
    if (t0 < mindist) t0 = 0.0;
    if (t1 == t0) continue;
    else if (t1 < t0) 
      {
      disc = t0;
      t0 = t1;
      t1 = disc;
      }

    /* Store the points of intersection of this
         blob with the ray.  Keep track of: whether
         this is the start or end point of the hit,
         which component was pierced by the ray,
         and the point along the ray that the
         hit occured at. */
    for (k=0;k<cnt && t0 > intervals[k].bound;k++);
    if (k<cnt) 
      {
      /* This hit point is smaller than one that
            already exists - bump the rest and insert
            it here */
      for (j=cnt;j>k;j--)
        memcpy(&intervals[j], &intervals[j-1],
          sizeof(Blob_Interval));
      intervals[k].type  = 0;
      intervals[k].index = i;
      intervals[k].bound = t0;
      cnt++;
      for (k=k+1;k<cnt && t1 > intervals[k].bound;k++);
      if (k<cnt) 
        {
        for (j=cnt;j>k;j--)
          memcpy(&intervals[j], &intervals[j-1],
            sizeof(Blob_Interval));
        intervals[k].type  = 1;
        intervals[k].index = i;
        intervals[k].bound = t1;
        }
      else 
        {
        intervals[cnt].type  = 1;
        intervals[cnt].index = i;
        intervals[cnt].bound = t1;
        }
      cnt++;
      }
    else 
      {
      /* Just plop the start and end points at
            the end of the list */
      intervals[cnt].type  = 0;
      intervals[cnt].index = i;
      intervals[cnt].bound = t0;
      cnt++;
      intervals[cnt].type  = 1;
      intervals[cnt].index = i;
      intervals[cnt].bound = t1;
      cnt++;
      }
    }
  return cnt;
  }

  /* Calculate the field value of a blob - the position vector
   "Pos" must already have been transformed into blob space. */
  static DBL
  calculate_field_value(obj, Pos)
    OBJECT *obj;
VECTOR *Pos;
  {
  int i;
  DBL len, density;
  VECTOR V;
  Blob_Element *ptr;
  BLOB *blob = (BLOB *)obj;

  density = 0.0;
  for (i=0;i<blob->count;i++) 
    {
    ptr = blob->list[i];
    VSub(V, ptr->pos, *Pos);
    VDot(len, V, V);
    if (len < ptr->radius2) 
      {
      /* Inside the radius of influence of this
            component, add it's contribution */
      density += len * (len * ptr->coeffs[0] +
        ptr->coeffs[1]) +
      ptr->coeffs[2];
      }
    }
  return density;
  }

/* Generate intervals of influence of each component.  After these
   are made, determine their aggregate effect on the ray.  As the
   individual intervals are checked, a quartic is generated
   that represents the density at a particular point on the ray.

   After making the substitutions in MakeBlob, there is a formula
   for each component that has the form:
   
      c0 * r^4 + c1 * r^2 + c2.
   
   In order to determine the influence on the ray of all of the
   individual components, we start by determining the distance
   from any point on the ray to the specified point.  This can
   be found using the pythagorean theorem, using C as the center
   of this component, P as the start of the ray, and D as the
   direction of travel of the ray:

      r^2 = (t * D + P - C) . (t * D + P - C)

   we insert this equation for each appearance of r^2 in the
   components' formula, giving:

      r^2 = D.D t^2 + 2 t D . (P - C) + (P - C) . (P - C)

   Since the direction vector has been normalized, D.D = 1.
   Using the substitutions:

      t0 = (P - C) . (P - C),
      t1 = D . (P - C)

   We can write the formula as:

      r^2 = t0 + 2 t t1 + t^2

   Taking r^2 and substituting into the formula for this component
   of the blob we get the formula:

      density = c0 * (r^2)^2 + c1 * r^2 + c2,

   or:

      density = c0 * (t0 + 2 t t1 + t^2)^2 +
                c1 * (t0 + 2 t t1 + t^2) +
                c2

   Expanding terms and collecting with respect to "t" gives:
      t^4 * c0 +
      t^3 * 4 c0 t1 +
      t^2 * (c1 + 2 * c0 t0 + 4 c0 t1^2)
      t   * 2 (c1 t1 + 2 c0 t0 t1) +
            c2 + c1*t0 + c0*t0^2

   This formula can now be solved for "t" by any of the quartic
   root solvers that are available.
*/
int All_Blob_Intersections(Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  BLOB *blob = (BLOB *)Object;
  DBL dist, len, *tcoeffs, coeffs[5], roots[4];
  int i, j, cnt;
  VECTOR P, D, V;
  int root_count, in_flag;
  Blob_Element *element;
  DBL t0, t1, c0, c1, c2;
  VECTOR IPoint, dv;
  Blob_Interval *intervals = blob->intervals;
  int Intersection_Found = FALSE;

  Ray_Blob_Tests++;

  /* Transform the ray into the blob space */
  if (blob->Trans != NULL) 
    {
    MInvTransPoint(&P, &Ray->Initial, blob->Trans);
    MInvTransDirection(&D, &Ray->Direction, blob->Trans);
    }
  else 
    {
    P = Ray->Initial;
    D = Ray->Direction;
    }

    len = sqrt(D.x * D.x + D.y * D.y + D.z * D.z);
  if (len == 0.0)
    return 0;
  else 
    {
    D.x /= len;
    D.y /= len;
    D.z /= len;
    }

    /* Figure out the intervals along the ray where each
   component of the blob has an effect. */
    if ((cnt = determine_influences(&P, &D, blob, 0.01)) == 0)
      /* Ray doesn't hit the sphere of influence of any of
      its component elements */
      return 0;

  /* Clear out the coefficients */
  for (i=0;i<4;i++) coeffs[i] = 0.0;
  coeffs[4] = -blob->threshold;

  /* Step through the list of influence points, adding the
      influence of each blob component as it appears */
  for (i=0,in_flag=0;i<cnt;i++) 
    {
    if (intervals[i].type == 0) 
      {
      /* Something is just starting to influence the ray,
            so calculate its coefficients and add them
         into the pot. */
      in_flag++;
      element = blob->list[intervals[i].index];

      VSub(V, P, element->pos);
      c0 = element->coeffs[0];
      c1 = element->coeffs[1];
      c2 = element->coeffs[2];
      VDot(t0, V, V);
      VDot(t1, V, D);
      tcoeffs = &(element->tcoeffs[0]);

      tcoeffs[0] = c0;
      tcoeffs[1] = 4.0 * c0 * t1;
      tcoeffs[2] = 2.0 * c0 * (2.0 * t1 * t1 + t0) + c1;
      tcoeffs[3] = 2.0 * t1 * (2.0 * c0 * t0 + c1);
      tcoeffs[4] = c0 * t0 * t0 + c1 * t0 + c2;

      for (j=0;j<5;j++) coeffs[j] += tcoeffs[j];
      }
    else 
      {
      /* We are losing the influence of a component, so
            subtract off its coefficients */
      tcoeffs = &(blob->list[intervals[i].index]->tcoeffs[0]);
      for (j=0;j<5;j++) coeffs[j] -= tcoeffs[j];
      if (--in_flag == 0)
        /* None of the components are currently affecting
               the ray - skip ahead. */
        continue;
      }

    /* Figure out which root solver to use */
    if (blob->Sturm_Flag == 0)
      /* Use Ferrari's method */
      root_count = solve_quartic(coeffs, &roots[0]);
    else
      /* Sturm sequences */
      if (fabs(coeffs[0]) < COEFF_LIMIT)
        if (fabs(coeffs[1]) < COEFF_LIMIT)
          root_count = solve_quadratic(&coeffs[2], &roots[0]);
        else
          root_count = polysolve(3, &coeffs[1], &roots[0]);
      else
        root_count = polysolve(4, coeffs, &roots[0]);

    /* See if any of the roots are valid */
    for(j=0;j<root_count;j++) 
      {
      dist = roots[j];
      /* First see if the root is in the interval of influence of
            the currently active components of the blob */
      if ((dist >= intervals[i].bound) &&
        (dist <= intervals[i+1].bound) &&
        (dist > Blob_Tolerance)) 
        {
        VScale(IPoint, D, dist);
        VAdd(IPoint, IPoint, P);
        /* Transform the point into world space */
        if (blob->Trans != NULL)
          MTransPoint(&IPoint, &IPoint, blob->Trans);
        VSub(dv, IPoint, Ray->Initial);
        VLength(len, dv);
        if (Point_In_Clip(&IPoint, Object->Clip)) 
          {
          push_entry(len,IPoint,Object,Depth_Stack);
          Intersection_Found = TRUE;
          }
        }
      }
    }
  if(Intersection_Found)
    Ray_Blob_Tests_Succeeded++;
  return Intersection_Found;
  }

/* Calculate the density at this point, then compare to
   the threshold to see if we are in or out of the blob */
int Inside_Blob (Test_Point, Object)
VECTOR *Test_Point;
OBJECT *Object;
  {
  VECTOR New_Point;
  BLOB *blob = (BLOB *) Object;

  /* Transform the point into blob space */
  if (blob->Trans != NULL)
    MInvTransPoint(&New_Point, Test_Point, blob->Trans);
  else
    New_Point = *Test_Point;

  if (calculate_field_value(Object, &New_Point) >
    blob->threshold-INSIDE_TOLERANCE)
    return ((int) 1-blob->Inverted);
  else
    return ((int) blob->Inverted);
  }

void Blob_Normal (Result, Object, IPoint)
OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  VECTOR New_Point, V;
  int i;
  DBL dist, val;
  BLOB *blob = (BLOB *) Object;
  Blob_Element *temp;

  /* Transform the point into the blobs space */
  if (blob->Trans != NULL)
    MInvTransPoint(&New_Point, IPoint, blob->Trans);
  else
    New_Point = *IPoint;

  Make_Vector(Result, 0, 0, 0);

  /* For each component that contributes to this point, add
      its bit to the normal */
  for(i=0;i<blob->count;i++) 
    {
    temp = blob->list[i];
    V.x = New_Point.x - temp->pos.x;
    V.y = New_Point.y - temp->pos.y;
    V.z = New_Point.z - temp->pos.z;
    dist = (V.x * V.x + V.y * V.y + V.z * V.z);

    if (dist <= temp->radius2) 
      {
      val = -2.0 * (2.0 * temp->coeffs[0] * dist +
        temp->coeffs[1]);
      Result->x += val * V.x;
      Result->y += val * V.y;
      Result->z += val * V.z;
      }
    }
  val = (Result->x * Result->x + Result->y * Result->y +
    Result->z * Result->z);
  if (val < EPSILON) 
    {
    Result->x = 1.0;
    Result->y = 0.0;
    Result->z = 0.0;
    }
  else 
    {
    val = 1.0 / sqrt(val);
    Result->x *= val;
    Result->y *= val;
    Result->z *= val;
    }

    /* Transform back to world space */
    if (blob->Trans != NULL)
      MTransNormal(Result, Result, blob->Trans);
  VNormalize(*Result, *Result);
  }

void Translate_Blob (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;
  Compute_Translation_Transform(&Trans, Vector);
  Transform_Blob(Object, &Trans);
  }

void Rotate_Blob (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;
  Compute_Rotation_Transform(&Trans, Vector);
  Transform_Blob(Object, &Trans);
  }

void Scale_Blob (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;
  Compute_Scaling_Transform(&Trans, Vector);
  Transform_Blob(Object, &Trans);
  }

void Transform_Blob(Object,Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  if (((BLOB *)Object)->Trans == NULL)
    ((BLOB *)Object)->Trans = Create_Transform();
  recompute_bbox(&Object->Bounds, Trans);
  Compose_Transforms(((BLOB *)Object)->Trans, Trans);
  }

void Invert_Blob(Object)
OBJECT *Object;
  {
  ((BLOB *) Object)->Inverted = 1 - ((BLOB *)Object)->Inverted;
  }

void *Copy_Blob (Object)
OBJECT *Object;
  {
  BLOB *New;
  unsigned i, cnt;

  New = Create_Blob();
  *New = * ((BLOB *)Object);

  New->Trans = Copy_Transform(New->Trans);

  cnt = ((BLOB *)Object)->count;
  New->list = (Blob_Element **)malloc(cnt * sizeof(Blob_Element *));
  if (New->list == NULL) 
    MAError("blob data");
  for (i=0;i<cnt;i++) 
    {
    New->list[i] = (Blob_Element *)malloc(sizeof(Blob_Element));
    if (New->list[i] == NULL)
      MAError("blob data");
    memcpy(New->list[i], ((BLOB *)Object)->list[i], sizeof(Blob_Element));
    }

  New->intervals = (Blob_Interval *)malloc(2*New->count*sizeof(Blob_Interval));
  if (New->intervals == NULL)
    MAError("blob data");

  return (New);
  }

/* Allocate a blob. */
BLOB *Create_Blob()
  {
  BLOB *New;

  if ((New = (BLOB *) malloc (sizeof (BLOB))) == NULL)
    MAError ("blob");

  INIT_OBJECT_FIELDS(New, BLOB_OBJECT, &Blob_Methods)

    New->Trans = NULL;
  New->Inverted = FALSE;
  New->count = 0;
  New->threshold = 0.0;
  New->list = NULL;
  New->intervals = NULL;
  New->Sturm_Flag = FALSE;

  return (New);
  }

void Destroy_Blob (Object)
OBJECT *Object;
  {
  unsigned i;

  Destroy_Transform(((BLOB *)Object)->Trans);
  for (i=0;i < (unsigned)((BLOB *)Object)->count;i++)
    free (((BLOB *)Object)->list[i]);
  free (((BLOB *)Object)->list);
  free (((BLOB *)Object)->intervals);
  free (Object);
  }
