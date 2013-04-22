/****************************************************************************
*                bound.c
*
*  This module implements the bounding slab calculations.
*  This file was written by Alexander Enzmann.    He wrote the code for
*  POV-Ray's bounding slabs and generously provided us these enhancements.
*  The slab intersection code was further hacked by Eric Haines to speed it up.
*
*  Just so everyone knows where this came from, the code is VERY heavily
*  based on the slab code from Mark VandeWettering's MTV raytracer.
*  POV-Ray is just joining the crowd of admirers of Mark's contribution to
*  the public domain. [ARE]
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

  typedef struct {
  int x,y,z ;
  } 
VECTORI, *pVECTORI ;

  typedef struct {
  VECTOR slab_num ;
  VECTOR slab_den ;
  VECTORI nonzero ;
  VECTORI positive ;
  } 
RAYINFO, *pRAYINFO ;


extern FRAME Frame;
extern long Bounds_Threshold;
extern int Use_Slabs;
static int Axis = 0;
static unsigned long maxprimcount = 0;
  typedef struct t_qelem {
  DBL     q_key;
  OBJECT *q_obj;
  } 
Qelem;

static int FindAxis PARAMS((OBJECT **Prims, unsigned long first,
unsigned long last));
static COMPOSITE *Create_Composite PARAMS((void));
static int SortAndSplit PARAMS((OBJECT **Root, OBJECT **Prims,
unsigned long *nPrims, unsigned long first, unsigned long last));
static void PriorityQueueInsert PARAMS((Qelem *Queue, unsigned *Qsize,
DBL key, OBJECT *obj));
static void CheckAndEnqueue PARAMS((Qelem *Queue, unsigned *Qsize,
OBJECT *obj, RAYINFO *rayinfo));
static void PriorityQueueDelete PARAMS((Qelem *Queue, unsigned *Qsize,
DBL *key, OBJECT **obj));

/* Should move these out of here... */
unsigned long totalQueues = 0;
unsigned long totalQueueResets = 0;
unsigned long nChecked = 0;
unsigned long nEnqueued = 0;

unsigned MAXQUEUE = 256;

METHODS Composite_Methods =
  { 
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, Destroy_Composite 
};

void Destroy_Composite (Object)
OBJECT *Object;
  {
  free (Object);
  }

static COMPOSITE *Create_Composite ()
  {
  COMPOSITE *New;

  if ((New = (COMPOSITE *) malloc (sizeof (COMPOSITE))) == NULL)
    MAError ("composite");

  INIT_OBJECT_FIELDS(New, COMPOSITE_OBJECT, &Composite_Methods)
    return New;
  }

void
recompute_bbox(bbox, trans)
BBOX *bbox;
TRANSFORM *trans;
  {
  VECTOR lower_left, lengths, corner;
  VECTOR mins, maxs;
  int i;

  lower_left = bbox->Lower_Left;
  lengths    = bbox->Lengths;
  Make_Vector(&mins,  BOUND_HUGE,  BOUND_HUGE,  BOUND_HUGE);
  Make_Vector(&maxs, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE);
  for (i=1;i<=8;i++) 
    {
    corner = lower_left;
    corner.x += ((i & 1) ? lengths.x : 0.0);
    corner.y += ((i & 2) ? lengths.y : 0.0);
    corner.z += ((i & 4) ? lengths.z : 0.0);
    MTransPoint(&corner, &corner, trans);
    if (corner.x < mins.x) mins.x = corner.x;
    if (corner.x > maxs.x) maxs.x = corner.x;
    if (corner.y < mins.y) mins.y = corner.y;
    if (corner.y > maxs.y) maxs.y = corner.y;
    if (corner.z < mins.z) mins.z = corner.z;
    if (corner.z > maxs.z) maxs.z = corner.z;
    }
  bbox->Lower_Left = mins;
  VSub(bbox->Lengths, maxs, mins);
  }

void
Recompute_Inverse_BBox(bbox, trans)
BBOX *bbox;
TRANSFORM *trans;
  {
  VECTOR lower_left, lengths, corner;
  VECTOR mins, maxs;
  int i;

  lower_left = bbox->Lower_Left;
  lengths = bbox->Lengths;
  Make_Vector(&mins,  BOUND_HUGE,  BOUND_HUGE,  BOUND_HUGE);
  Make_Vector(&maxs, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE);
  for (i=1;i<=8;i++) 
    {
    corner = lower_left;
    corner.x += ((i & 1) ? lengths.x : 0.0);
    corner.y += ((i & 2) ? lengths.y : 0.0);
    corner.z += ((i & 4) ? lengths.z : 0.0);
    MInvTransPoint(&corner, &corner, trans);
    if (corner.x < mins.x) mins.x = corner.x;
    if (corner.x > maxs.x) maxs.x = corner.x;
    if (corner.y < mins.y) mins.y = corner.y;
    if (corner.y > maxs.y) maxs.y = corner.y;
    if (corner.z < mins.z) mins.z = corner.z;
    if (corner.z > maxs.z) maxs.z = corner.z;
    }
  bbox->Lower_Left = mins;
  VSub(bbox->Lengths, maxs, mins);
  }

int CDECL compslabs(in_a, in_b)
void *in_a;
void *in_b;
  {

  OBJECT **a, **b;
  DBL am, bm;

  a = (OBJECT **)in_a;
  b = (OBJECT **)in_b;

  switch (Axis) 
  {
  case 0:
    am = 2.0 * (*a)->Bounds.Lower_Left.x + (*a)->Bounds.Lengths.x;
    bm = 2.0 * (*b)->Bounds.Lower_Left.x + (*b)->Bounds.Lengths.x;
    break;
  case 1:
    am = 2.0 * (*a)->Bounds.Lower_Left.y + (*a)->Bounds.Lengths.y;
    bm = 2.0 * (*b)->Bounds.Lower_Left.y + (*b)->Bounds.Lengths.y;
    break;
  case 2:
    am = 2.0 * (*a)->Bounds.Lower_Left.z + (*a)->Bounds.Lengths.z;
    bm = 2.0 * (*b)->Bounds.Lower_Left.z + (*b)->Bounds.Lengths.z;
    break;
  default:
    Error("Bad axis in compslabs\n");
  }

  if (am < bm)
    return -1;
  else if (am == bm)
    return 0;
  else
    return 1;
  }

static int
FindAxis(Prims, first, last)
OBJECT **Prims;
unsigned long first, last;
  {
  BBOX *bbox;
  VECTOR mins, maxs;
  unsigned long i;
  int which;
  DBL d = -BOUND_HUGE, e;

  Make_Vector(&mins, BOUND_HUGE, BOUND_HUGE, BOUND_HUGE);
  Make_Vector(&maxs, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE);

  for (i=first;i<last;i++) 
    {
    bbox = &(Prims[i]->Bounds);
    if (bbox->Lower_Left.x < mins.x)
      mins.x = bbox->Lower_Left.x;
    if (bbox->Lower_Left.x + bbox->Lengths.x > maxs.x)
      maxs.x = bbox->Lower_Left.x;
    if (bbox->Lower_Left.y < mins.y)
      mins.y = bbox->Lower_Left.y;
    if (bbox->Lower_Left.y + bbox->Lengths.y > maxs.y)
      maxs.y = bbox->Lower_Left.y;
    if (bbox->Lower_Left.z < mins.z)
      mins.z = bbox->Lower_Left.z;
    if (bbox->Lower_Left.z + bbox->Lengths.z > maxs.z)
      maxs.z = bbox->Lower_Left.z;
    }

  e = maxs.x - mins.x;
  if (e > d) 
    { 
    d = e; which = 0; 
  }
  e = maxs.y - mins.y;
  if (e > d) 
    { 
    d = e; which = 1; 
  }
  e = maxs.z - mins.z;
  if (e > d) 
    { 
    d = e; which = 2; 
  }

  return which;
  }

static int
SortAndSplit(Root, Prims, nPrims, first, last)
OBJECT **Root;
OBJECT **Prims;
unsigned long *nPrims;
unsigned long first;
unsigned long last;
  {
  COMPOSITE *cd;
  unsigned long size, i, j, m;
  DBL dmin, dmax, tmin, tmax;

  Axis = FindAxis(Prims, first, last);
  size = last - first;

  /* Actually, we could do this faster in several ways. we could use a
      logn algorithm to find the median along the given axis, and then a
      linear algorithm to partition along the axis. Oh well. */

  qsort((char *) (Prims + first), (int)size, sizeof(OBJECT *), compslabs);

  if (size <= BUNCHING_FACTOR) 
    {
    cd = Create_Composite();
    cd->Entries = (unsigned short)size;

    for (i=0;i<size;i++) 
      {
      cd->Objects[i] = Prims[first+i];
      /*
printf("Extent of object %ld/%d: <%g, %g, %g> -> <%g, %g, %g>\n",
       first+i, cd->Objects[i]->Type,
       cd->Objects[i]->Bounds.Lower_Left.x,
       cd->Objects[i]->Bounds.Lower_Left.y,
       cd->Objects[i]->Bounds.Lower_Left.z,
       cd->Objects[i]->Bounds.Lower_Left.x + cd->Objects[i]->Bounds.Lengths.x,
       cd->Objects[i]->Bounds.Lower_Left.y + cd->Objects[i]->Bounds.Lengths.y,
       cd->Objects[i]->Bounds.Lower_Left.z + cd->Objects[i]->Bounds.Lengths.z);
*/
      }

    /* Check bounds in each direction */
    /* First along the x axis */
    dmin = BOUND_HUGE; dmax = -BOUND_HUGE;
    for (j=0;j<size;j++) 
      {
      tmin = cd->Objects[j]->Bounds.Lower_Left.x;
      tmax = tmin + cd->Objects[j]->Bounds.Lengths.x;
      if (tmin < dmin) dmin = tmin;
      if (tmax > dmax) dmax = tmax;
      }
    cd->Bounds.Lower_Left.x = dmin;
    cd->Bounds.Lengths.x = dmax - dmin;

    /* Now along the y axis */
    dmin = BOUND_HUGE; dmax = -BOUND_HUGE;
    for (j=0;j<size;j++) 
      {
      tmin = cd->Objects[j]->Bounds.Lower_Left.y;
      tmax = tmin + cd->Objects[j]->Bounds.Lengths.y;
      if (tmin < dmin) dmin = tmin;
      if (tmax > dmax) dmax = tmax;
      }
    cd->Bounds.Lower_Left.y = dmin;
    cd->Bounds.Lengths.y = dmax - dmin;

    /* Lastly along the z axis */
    dmin = BOUND_HUGE; dmax = -BOUND_HUGE;
    for (j=0;j<size;j++) 
      {
      tmin = cd->Objects[j]->Bounds.Lower_Left.z;
      tmax = tmin + cd->Objects[j]->Bounds.Lengths.z;
      if (tmin < dmin) dmin = tmin;
      if (tmax > dmax) dmax = tmax;
      }
    cd->Bounds.Lower_Left.z = dmin;
    cd->Bounds.Lengths.z = dmax - dmin;

    *Root = (OBJECT *)cd;
    if (*nPrims <= maxprimcount) 
      {
      Prims[*nPrims] = (OBJECT *)cd;
      *nPrims += 1;
      return 1;
      }
    else
      Error("Too many primitives\n");
    }
  else 
    {
    m = (first + last) / 2;
    SortAndSplit(Root, Prims, nPrims, first, m);
    SortAndSplit(Root, Prims, nPrims, m , last);
    return 0;
    }
  return -1;
  }

  void
  BuildBoundingSlabs(Root)
    OBJECT **Root;
  {
  OBJECT **Prims, **prim, *head;
  unsigned long nPrims;
  unsigned long low, high;

  /* We have to start by counting how many frame level object there are */
  head   = Frame.Objects;
  nPrims = 0;
  while (head != NULL) 
    {
    nPrims++;
    head = head->Sibling;
    }

  /* The total # of prims inflates around 150% when bounding objects are
      generated.  If the 1.8 below proves to be too small, use 2.0.  The
      inflation is never 200%. */
  maxprimcount = (unsigned long)(1.8 * (nPrims + 1));

  /* Now allocate an array to hold references to these prims & any new
     composite objects we may generate */
  Prims = (OBJECT **)malloc((unsigned)maxprimcount * sizeof(OBJECT *));
  if (Prims == NULL)
    Error("Failed to allocate bounding slab reference information\n");

  /* Copy pointers to the objects into the array */
  prim = Prims;
  for (head=Frame.Objects;head!=NULL;head=head->Sibling) 
    {
    if (head->Type & LIGHT_SOURCE_OBJECT) 
      {
      /* Only bother with lights if they have an attached shape */
      if (((LIGHT_SOURCE *)head)->Children != NULL)
        *prim++ = ((LIGHT_SOURCE *)head)->Children;
      else
        nPrims--;
      }
    else
      /* Normal sort of object - add it to the list */
      *prim++ = head;
    }

  /* Now do a sort on the objects, with the end result being a tree of
      objects sorted along the x, y, and z axes */
  low  = 0;
  high = nPrims;
  while (SortAndSplit(Root, Prims, &nPrims, low, high) == 0) 
    {
    low  = high;
    high = nPrims;
    }

  Use_Slabs = (nPrims >= Bounds_Threshold);

  /* Test */
  /*
printf("Extent of scene: <%g, %g, %g> -> <%g, %g, %g>\n",
       (*Root)->Bounds.Lower_Left.x,
       (*Root)->Bounds.Lower_Left.y,
       (*Root)->Bounds.Lower_Left.z,
       (*Root)->Bounds.Lower_Left.x + (*Root)->Bounds.Lengths.x,
       (*Root)->Bounds.Lower_Left.y + (*Root)->Bounds.Lengths.y,
       (*Root)->Bounds.Lower_Left.z + (*Root)->Bounds.Lengths.z);
*/

  /* Now we can get rid of the Prim array, and just use Root */
  free(Prims);
  }

static void
PriorityQueueInsert(Queue, Qsize, key, obj)
Qelem *Queue;
unsigned *Qsize;
DBL key;
OBJECT *obj;
  {
  unsigned size;
  int i;
  Qelem tmp;

  totalQueues++;
  (*Qsize)++;
  size = *Qsize;
  /* if (size > maxQueueSize) maxQueueSize = size; */
  if (size >= MAXQUEUE)
    Error("Priority queue overflow");
  Queue[size].q_key = key;
  Queue[size].q_obj = obj;

  i = size;
  while (i > 1 && Queue[i].q_key < Queue[i/2].q_key) 
    {
    tmp = Queue[i];
    Queue[i] = Queue[i/2];
    Queue[i/2] = tmp;
    i = i / 2;
    }
  }

static void
PriorityQueueDelete(Queue, Qsize, key, obj)
Qelem *Queue;
unsigned *Qsize;
DBL *key;
OBJECT **obj;
  {
  Qelem tmp;
  int i, j;
  unsigned size;

  if (*Qsize == 0)
    Error("priority queue is empty");

  *key = Queue[1].q_key;
  *obj = Queue[1].q_obj;
  Queue[1] = Queue[*Qsize];
  (*Qsize)--;
  size = *Qsize;

  i = 1 ;

  while (2 * i <= (int)size) 
    {
    if (2 * i == (int)size)
      j = 2 * i;
    else if (Queue[2*i].q_key < Queue[2*i+1].q_key)
      j = 2 * i;
    else
      j = 2 * i + 1;

    if (Queue[i].q_key > Queue[j].q_key) 
      {
      tmp = Queue[i];
      Queue[i] = Queue[j];
      Queue[j] = tmp;
      i = j;
      }
    else
      break;
    }
  }

static void
CheckAndEnqueue(Queue, Qsize, obj, rayinfo)
Qelem *Queue;
unsigned *Qsize;
OBJECT *obj;
RAYINFO *rayinfo;
  {
  DBL tmin, tmax;
  DBL dmin, dmax ;

  nChecked++;

  if (rayinfo->nonzero.x ) 
    {
    if (rayinfo->positive.x ) 
      {
      dmin = (obj->Bounds.Lower_Left.x - rayinfo->slab_num.x) *
      rayinfo->slab_den.x;
      dmax = dmin + (obj->Bounds.Lengths.x  * rayinfo->slab_den.x);
      if ( dmax < EPSILON ) return ;
      } 
    else 
      {
      dmax = (obj->Bounds.Lower_Left.x - rayinfo->slab_num.x) *
        rayinfo->slab_den.x;
      if ( dmax < EPSILON ) return ;
      dmin = dmax + (obj->Bounds.Lengths.x  * rayinfo->slab_den.x);
      }
    if ( dmin > dmax ) return ;
    }
  else 
    {
    if ( ( rayinfo->slab_num.x < obj->Bounds.Lower_Left.x ) ||
      ( rayinfo->slab_num.x >
      obj->Bounds.Lengths.x + obj->Bounds.Lower_Left.x ) ) return ;
    dmin = -BOUND_HUGE; dmax = BOUND_HUGE;
    }

  if (rayinfo->nonzero.y ) 
    {
    if (rayinfo->positive.y ) 
      {
      tmin = (obj->Bounds.Lower_Left.y - rayinfo->slab_num.y) *
      rayinfo->slab_den.y;
      tmax = tmin + (obj->Bounds.Lengths.y  * rayinfo->slab_den.y);
      } 
    else 
      {
      tmax = (obj->Bounds.Lower_Left.y - rayinfo->slab_num.y) *
        rayinfo->slab_den.y;
      tmin = tmax + (obj->Bounds.Lengths.y  * rayinfo->slab_den.y);
      }
    /* unwrap the logic - do the dmin and dmax checks only when tmin and
	  tmax actually affect anything, also try to escape ASAP.  Better
	  yet, fold the logic below into the two branches above so as to
	  compute only what is needed.
	*/
    /* you might even try tmax < EPSILON first (instead of second) for an
          early quick out
	*/
    if ( tmax < dmax ) 
      {
      if ( tmax < EPSILON ) return;
      /* check bounds only if tmax changes dmax */
      if ( tmin > dmin ) 
        {
        if ( tmin > tmax ) return ;
        /* do this last in case it's not needed! */
        dmin = tmin ;
        } 
      else 
        {
        if ( dmin > tmax ) return ;
        }
      /* do this last in case it's not needed! */
      dmax = tmax ;
      } 
    else 
      {
      if ( tmin > dmin ) 
        {
        if ( tmin > dmax ) return ;
        /* do this last in case it's not needed! */
        dmin = tmin ;
        } /* else nothing needs to happen, since dmin and dmax did not
	       change! */
             
      }
    } 
  else 
    {
    if (rayinfo->slab_num.y < obj->Bounds.Lower_Left.y ||
      rayinfo->slab_num.y >
      obj->Bounds.Lengths.y + obj->Bounds.Lower_Left.y ) return ;
    }

  if (rayinfo->nonzero.z ) 
    {
    if (rayinfo->positive.z ) 
      {
      tmin = (obj->Bounds.Lower_Left.z - rayinfo->slab_num.z) *
      rayinfo->slab_den.z;
      tmax = tmin + (obj->Bounds.Lengths.z  * rayinfo->slab_den.z);
      } 
    else 
      {
      tmax = (obj->Bounds.Lower_Left.z - rayinfo->slab_num.z) *
        rayinfo->slab_den.z;
      tmin = tmax + (obj->Bounds.Lengths.z  * rayinfo->slab_den.z);
      }
    if ( tmax < dmax ) 
      {
      if ( tmax < EPSILON ) return;
      /* check bounds only if tmax changes dmax */
      if ( tmin > dmin ) 
        {
        if ( tmin > tmax ) return ;
        /* do this last in case it's not needed! */
        dmin = tmin ;
        } 
      else 
        {
        if ( dmin > tmax ) return ;
        }
      /* do this last in case it's not needed! */
      dmax = tmax ;
      } 
    else 
      {
      if ( tmin > dmin ) 
        {
        if ( tmin > dmax ) return ;
        /* do this last in case it's not needed! */
        dmin = tmin ;
        } /* else nothing needs to happen, since dmin and dmax did not
	       change! */
             
      }
    } 
  else 
    {
    if (rayinfo->slab_num.z < obj->Bounds.Lower_Left.z ||
      rayinfo->slab_num.z >
      obj->Bounds.Lengths.z + obj->Bounds.Lower_Left.z ) return ;
    }

  PriorityQueueInsert(Queue, Qsize, dmin, obj);
  nEnqueued++;
  }

int
Bounds_Intersect(Root, ray, Best_Intersection, Best_Object)
OBJECT *Root;
RAY *ray;
INTERSECTION *Best_Intersection;
OBJECT **Best_Object;
  {
  Qelem *Queue;
  unsigned Qsize = 0;
  int i;
  OBJECT *cobj;
  COMPOSITE *cdp;
  RAYINFO rayinfo;
  DBL t, key;
  INTERSECTION New_Intersection;
  int Intersection_Found = 0;

  Queue = (Qelem *)malloc(MAXQUEUE * sizeof(Qelem));
  if (Queue == NULL)
    Error("Failed to allocate priority queue\n");

  /* Create the direction vectors for this ray */
  rayinfo.slab_num.x = ray->Initial.x;
  rayinfo.slab_num.y = ray->Initial.y;
  rayinfo.slab_num.z = ray->Initial.z;
  if ( rayinfo.nonzero.x = ((t = ray->Direction.x) != 0.0) ) 
    {
    rayinfo.slab_den.x = 1.0 / t;
    rayinfo.positive.x = ( ray->Direction.x > 0.0 ) ;
    }
  if ( rayinfo.nonzero.y = ((t = ray->Direction.y) != 0.0) ) 
    {
    rayinfo.slab_den.y = 1.0 / t;
    rayinfo.positive.y = ( ray->Direction.y > 0.0 ) ;
    }
  if ( rayinfo.nonzero.z = ((t = ray->Direction.z) != 0.0) ) 
    {
    rayinfo.slab_den.z = 1.0 / t;
    rayinfo.positive.z = ( ray->Direction.z > 0.0 ) ;
    }

  /* start with an empty priority queue */
  Qsize = 0;
  totalQueueResets++;

  CheckAndEnqueue(Queue, &Qsize, Root, &rayinfo );

  for (;;)
    {
    if (Qsize == 0)
      break;

    PriorityQueueDelete(Queue, &Qsize, &key, &cobj);

    if (key > Best_Intersection->Depth)
      break;
    else
      if (cobj->Type & BOUNDING_OBJECT)
        {
        cdp = (COMPOSITE *)cobj;
        for (i=0;(unsigned short)i < cdp->Entries;i++)
          CheckAndEnqueue(Queue, &Qsize, cdp->Objects[i],
            &rayinfo) ;
        }
      else
        {
        if (Intersection(&New_Intersection, cobj, ray))
          if (New_Intersection.Depth < Best_Intersection->Depth)
            {
            *Best_Intersection = New_Intersection;
            *Best_Object = cobj;
            Intersection_Found = TRUE;
            }
        }
    }

  free(Queue);
  return Intersection_Found;
  }
