/****************************************************************************
*                   bezier.c
*
*  This module implements the code for Bezier bicubic patch shapes
*
*  This file was written by Alexander Enzmann.	He wrote the code for
*  bezier bicubic patches and generously provided us these enhancements.
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

METHODS Bicubic_Patch_Methods =
  { 
  All_Bicubic_Patch_Intersections,
  Inside_Bicubic_Patch, Bicubic_Patch_Normal, Copy_Bicubic_Patch,
  Translate_Bicubic_Patch, Rotate_Bicubic_Patch,
  Scale_Bicubic_Patch, Transform_Bicubic_Patch, Invert_Bicubic_Patch,
  Destroy_Bicubic_Patch
  };

extern long Ray_Bicubic_Tests, Ray_Bicubic_Tests_Succeeded;
extern unsigned int Options;
extern RAY *CM_Ray;
extern int Shadow_Test_Flag;

int max_depth_reached;

static int InvertMatrix PARAMS((VECTOR in[3], VECTOR out[3]));
static void bezier_value PARAMS((VECTOR (*Control_Points)[4][4], DBL u0, DBL v0,
VECTOR *P, VECTOR *N));
static int intersect_subpatch PARAMS((BICUBIC_PATCH *, RAY *, VECTOR [3],
DBL [3], DBL [3], DBL *, VECTOR *, VECTOR *, DBL *, DBL *));
static void find_average PARAMS((int, VECTOR *, VECTOR *, DBL *));
static int spherical_bounds_check PARAMS((RAY *, VECTOR *, DBL));
static int intersect_bicubic_patch0 PARAMS((RAY *, BICUBIC_PATCH *, DBL *));
static int intersect_bicubic_patch1 PARAMS((RAY *, BICUBIC_PATCH *, DBL *));
static DBL point_plane_distance PARAMS((VECTOR *, VECTOR *, DBL *));
static DBL determine_subpatch_flatness PARAMS((VECTOR (*)[4][4]));
static int flat_enough PARAMS((BICUBIC_PATCH *, VECTOR (*)[4][4]));
static void bezier_bounding_sphere PARAMS((VECTOR (*)[4][4], VECTOR *,DBL *));
static void bezier_subpatch_intersect PARAMS((RAY *, BICUBIC_PATCH *,
VECTOR (*)[4][4], DBL, DBL, DBL, DBL,
int *, DBL *));
static void bezier_split_left_right PARAMS((VECTOR (*)[4][4],VECTOR (*)[4][4],
VECTOR (*)[4][4]));
static void bezier_split_up_down PARAMS((VECTOR (*)[4][4], VECTOR (*)[4][4],
VECTOR (*)[4][4]));
static void bezier_subdivider PARAMS((RAY *, BICUBIC_PATCH *,VECTOR (*)[4][4],
DBL, DBL, DBL, DBL, int, int *, DBL *));
static void bezier_tree_deleter PARAMS((BEZIER_NODE *Node));
static BEZIER_NODE *bezier_tree_builder PARAMS((BICUBIC_PATCH *Object,
VECTOR(*Patch)[4][4], DBL u0, DBL u1,
DBL v0, DBL v1, int depth));
static void bezier_tree_walker PARAMS((RAY *, BICUBIC_PATCH *, BEZIER_NODE *,
int, int *, DBL *));
static BEZIER_NODE *create_new_bezier_node PARAMS((void));
static BEZIER_VERTICES *create_bezier_vertex_block PARAMS((void));
static BEZIER_CHILDREN *create_bezier_child_block PARAMS((void));
static int subpatch_normal PARAMS((VECTOR *v1, VECTOR *v2, VECTOR *v3,
VECTOR *Result, DBL *d));

  static DBL C[4] = {
  1.0, 3.0, 3.0, 1.0
  };

#define BEZIER_EPSILON 1.0e-10
#define BEZIER_TOLERANCE 1.0e-5

static BEZIER_NODE *create_new_bezier_node()
  {
  BEZIER_NODE *Node = (BEZIER_NODE *)malloc(sizeof(BEZIER_NODE));
  if (Node == NULL) 
    MAError("bezier node");
  Node->Data_Ptr = NULL;
  return Node;
  }

static BEZIER_VERTICES *create_bezier_vertex_block()
  {
  BEZIER_VERTICES *Vertices = (BEZIER_VERTICES *)malloc(sizeof(BEZIER_VERTICES));
  if (Vertices == NULL) 
    MAError("bezier vertices");
  return Vertices;
  }

static BEZIER_CHILDREN *create_bezier_child_block()
  {
  BEZIER_CHILDREN *Children = (BEZIER_CHILDREN *)malloc(sizeof(BEZIER_CHILDREN));
  if (Children == NULL) 
    MAError("bezier children");
  return Children;
  }

static BEZIER_NODE *bezier_tree_builder(Object, Patch, u0, u1, v0, v1, depth)
BICUBIC_PATCH *Object;
VECTOR (*Patch)[4][4];
DBL u0, u1, v0, v1;
int depth;
  {
  VECTOR Lower_Left[4][4], Lower_Right[4][4];
  VECTOR Upper_Left[4][4], Upper_Right[4][4];
  BEZIER_CHILDREN *Children;
  BEZIER_VERTICES *Vertices;
  BEZIER_NODE *Node = create_new_bezier_node();

  if (depth > max_depth_reached) max_depth_reached = depth;

  /* Build the bounding sphere for this subpatch */
  bezier_bounding_sphere(Patch, &(Node->Center), &(Node->Radius_Squared));

  /* If the patch is close to being flat, then just perform a ray-plane
      intersection test. */
  if (flat_enough(Object, Patch)) 
    {
    /* The patch is now flat enough to simply store the corners */
    Node->Node_Type = BEZIER_LEAF_NODE;
    Vertices = create_bezier_vertex_block();
    Vertices->Vertices[0] = (*Patch)[0][0];
    Vertices->Vertices[1] = (*Patch)[0][3];
    Vertices->Vertices[2] = (*Patch)[3][3];
    Vertices->Vertices[3] = (*Patch)[3][0];
    Vertices->uvbnds[0] = u0;
    Vertices->uvbnds[1] = u1;
    Vertices->uvbnds[2] = v0;
    Vertices->uvbnds[3] = v1;
    Node->Data_Ptr = (void *)Vertices;
    }
  else if (depth >= Object->U_Steps)
    if (depth >= Object->V_Steps) 
      {
      /* We are at the max recursion depth. Just store corners. */
      Node->Node_Type = BEZIER_LEAF_NODE;
      Vertices = create_bezier_vertex_block();
      Vertices->Vertices[0] = (*Patch)[0][0];
      Vertices->Vertices[1] = (*Patch)[0][3];
      Vertices->Vertices[2] = (*Patch)[3][3];
      Vertices->Vertices[3] = (*Patch)[3][0];
      Vertices->uvbnds[0] = u0;
      Vertices->uvbnds[1] = u1;
      Vertices->uvbnds[2] = v0;
      Vertices->uvbnds[3] = v1;
      Node->Data_Ptr = (void *)Vertices;
      }
    else 
      {
      bezier_split_up_down(Patch, (VECTOR (*)[4][4])Lower_Left,
        (VECTOR (*)[4][4])Upper_Left);
      Node->Node_Type = BEZIER_INTERIOR_NODE;
      Children = create_bezier_child_block();
      Children->Children[0] =
      bezier_tree_builder(Object, (VECTOR (*)[4][4])Lower_Left,
        u0, u1, v0, (v0 + v1) / 2.0, depth+1);
      Children->Children[1] =
      bezier_tree_builder(Object, (VECTOR (*)[4][4])Upper_Left,
        u0, u1, (v0 + v1) / 2.0, v1, depth+1);
      Node->Count = 2;
      Node->Data_Ptr = (void *)Children;
      }
  else if (depth >= Object->V_Steps) 
    {
    bezier_split_left_right(Patch, (VECTOR (*)[4][4])Lower_Left,
      (VECTOR (*)[4][4])Lower_Right);
    Node->Node_Type = BEZIER_INTERIOR_NODE;
    Children = create_bezier_child_block();
    Children->Children[0] =
    bezier_tree_builder(Object, (VECTOR (*)[4][4])Lower_Left,
      u0, (u0 + u1) / 2.0, v0, v1, depth+1);
    Children->Children[1] =
    bezier_tree_builder(Object, (VECTOR (*)[4][4])Lower_Right,
      (u0 + u1) / 2.0, u1, v0, v1, depth+1);
    Node->Count = 2;
    Node->Data_Ptr = (void *)Children;
    }
  else 
    {
    bezier_split_left_right(Patch, (VECTOR (*)[4][4])Lower_Left,
      (VECTOR (*)[4][4])Lower_Right);
    bezier_split_up_down((VECTOR (*)[4][4])Lower_Left,
      (VECTOR (*)[4][4])Lower_Left,
      (VECTOR (*)[4][4])Upper_Left);
    bezier_split_up_down((VECTOR (*)[4][4])Lower_Right,
      (VECTOR (*)[4][4])Lower_Right,
      (VECTOR (*)[4][4])Upper_Right);
    Node->Node_Type = BEZIER_INTERIOR_NODE;
    Children = create_bezier_child_block();
    Children->Children[0] =
    bezier_tree_builder(Object, (VECTOR (*)[4][4])Lower_Left,
      u0, (u0 + u1) / 2.0, v0, (v0 + v1) / 2.0, depth+1);
    Children->Children[1] =
    bezier_tree_builder(Object, (VECTOR (*)[4][4])Upper_Left,
      u0, (u0 + u1) / 2.0, (v0 + v1) / 2.0, v1, depth+1);
    Children->Children[2] =
    bezier_tree_builder(Object, (VECTOR (*)[4][4])Lower_Right,
      (u0 + u1) / 2.0, u1, v0, (v0 + v1) / 2.0, depth+1);
    Children->Children[3] =
    bezier_tree_builder(Object, (VECTOR (*)[4][4])Upper_Right,
      (u0 + u1) / 2.0, u1, (v0 + v1) / 2.0, v1, depth+1);
    Node->Count = 4;
    Node->Data_Ptr = (void *)Children;
    }
  return Node;
  }

/* Determine the position and normal at a single coordinate point (u, v)
   on a Bezier patch */
static void bezier_value(Control_Points, u0, v0, P, N)
VECTOR (*Control_Points)[4][4];
DBL u0, v0;
VECTOR *P, *N;
  {
  DBL c, t, ut, vt;
  DBL u[4], uu[4], v[4], vv[4];
  DBL du[4], duu[4], dv[4], dvv[4];
  int i, j;
  VECTOR U, V, T;

  /* Calculate binomial coefficients times coordinate positions */
  u[0]  = 1.0; uu[0] = 1.0; du[0] = 0.0; duu[0] = 0.0;
  v[0]  = 1.0; vv[0] = 1.0; dv[0] = 0.0; dvv[0] = 0.0;
  for (i=1;i<4;i++) 
    {
    u[i]   = u[i-1] * u0;
    uu[i]  = uu[i-1] * (1.0 - u0);
    v[i]   = v[i-1] * v0;
    vv[i]  = vv[i-1] * (1.0 - v0);
    du[i]  =  i * u[i-1];
    duu[i] = -i * uu[i-1];
    dv[i]  =  i * v[i-1];
    dvv[i] = -i * vv[i-1];
    }

  /* Now evaluate position and tangents based on control points */
  Make_Vector(P, 0, 0, 0);
  Make_Vector(&U, 0, 0, 0);
  Make_Vector(&V, 0, 0, 0);
  for (i=0;i<4;i++)
    for (j=0;j<4;j++) 
    {
    c = C[i] * C[j];
    ut = u[i] * uu[3 - i];
    vt = v[j] * vv[3 - j];
    t = c * ut * vt;
    VScale(T, (*Control_Points)[i][j], t);
    VAddEq(*P, T);
    t = c * vt * (du[i] * uu[3-i] + u[i] * duu[3-i]);
    VScale(T, (*Control_Points)[i][j], t);
    VAddEq(U, T);
    t = c * ut * (dv[j] * vv[3-j] + v[j] * dvv[3-j]);
    VScale(T, (*Control_Points)[i][j], t);
    VAddEq(V, T);
    }

  /* Make the normal from the cross product of the tangents */
  VCross(*N, U, V);
  VDot(t, *N, *N);
  if (t > BEZIER_EPSILON) 
    {
    t = 1.0 / sqrt(t);
    VScaleEq(*N, t);
    }
  else
    Make_Vector(N, 1, 0, 0)
      }

/* Calculate the normal to a subpatch (triangle) return the vector
<1.0 0.0 0.0> if the triangle is degenerate. */
static int subpatch_normal(v1, v2, v3, Result, d)
VECTOR *v1, *v2, *v3, *Result;
DBL *d;
  {
  VECTOR V1, V2;
  DBL Length;

  VSub(V1, *v1, *v2);
  VSub(V2, *v3, *v2);
  VCross(*Result, V1, V2);
  VLength(Length, *Result);
  if (Length < BEZIER_EPSILON) 
    {
    Result->x = 1.0;
    Result->y = 0.0;
    Result->z = 0.0;
    *d = -1.0 * v1->x;
    return 0;
    }
  else 
    {
    VInverseScale(*Result, *Result, Length);
    VDot(*d, *Result, *v1);
    *d = 0.0 - *d;
    return 1;
    }
  }

static int
InvertMatrix(in, out)
VECTOR in[3], out[3];
  {
  int i;
  DBL det;

  out[0].x =  (in[1].y * in[2].z - in[1].z * in[2].y);
  out[1].x = -(in[0].y * in[2].z - in[0].z * in[2].y);
  out[2].x =  (in[0].y * in[1].z - in[0].z * in[1].y);

  out[0].y = -(in[1].x * in[2].z - in[1].z * in[2].x);
  out[1].y =  (in[0].x * in[2].z - in[0].z * in[2].x);
  out[2].y = -(in[0].x * in[1].z - in[0].z * in[1].x);

  out[0].z =  (in[1].x * in[2].y - in[1].y * in[2].x);
  out[1].z = -(in[0].x * in[2].y - in[0].y * in[2].x);
  out[2].z =  (in[0].x * in[1].y - in[0].y * in[1].x);

  det = 
  in[0].x * in[1].y * in[2].z +
  in[0].y * in[1].z * in[2].x +
  in[0].z * in[1].x * in[2].y -
  in[0].z * in[1].y * in[2].x -
  in[0].x * in[1].z * in[2].y -
  in[0].y * in[1].x * in[2].z;

  if (det > -BEZIER_EPSILON && det < BEZIER_EPSILON)
    return 0;

  det = 1.0 / det;

  for (i=0;i<3;i++)
    VScaleEq(out[i], det)

      return 1;
  }

static int
intersect_subpatch(Shape, ray, V, uu, vv, Depth, P, N, u, v)
BICUBIC_PATCH *Shape;
RAY *ray;
VECTOR V[3];
DBL uu[3], vv[3], *Depth;
VECTOR *P, *N;
DBL *u, *v;
  {
  VECTOR B[3], IB[3], NN[3], Q, T;
  DBL d, n, a, b, r;

  VSub(B[0], V[1], V[0]);
  VSub(B[1], V[2], V[0]);
  VCross(B[2], B[0], B[1]);
  VDot(d, B[2], B[2]);
  if (d < BEZIER_EPSILON)
    return 0;
  d = 1.0 / sqrt(d);
  VScaleEq(B[2], d);

  if (!InvertMatrix(B, IB))
    /* Degenerate triangle */
    return 0;

  VDot(d, ray->Direction, IB[2]);
  if (d > -BEZIER_EPSILON && d < BEZIER_EPSILON)
    return 0;

  VSub(Q, V[0], ray->Initial);
  VDot(n, Q, IB[2]);
  *Depth = n / d;
  if (*Depth < BEZIER_TOLERANCE)
    return 0;
  VScale(T, ray->Direction, *Depth);
  VAdd(*P, ray->Initial, T);
  VSub(Q, *P, V[0]);

  VDot(a, Q, IB[0]);
  VDot(b, Q, IB[1]);
  if (a < 0.0 || b < 0.0 || a + b > 1.0) 
    return 0;

  r = 1.0 - a - b;

  Make_Vector(N, 0.0, 0.0, 0.0);
  bezier_value((VECTOR (*)[4][4])&Shape->Control_Points,
    uu[0], vv[0], &T, &NN[0]);
  bezier_value((VECTOR (*)[4][4])&Shape->Control_Points,
    uu[1], vv[1], &T, &NN[1]);
  bezier_value((VECTOR (*)[4][4])&Shape->Control_Points,
    uu[2], vv[2], &T, &NN[2]);
  VScale(T, NN[0], r); VAdd(*N, *N, T);
  VScale(T, NN[1], a); VAdd(*N, *N, T);
  VScale(T, NN[2], b); VAdd(*N, *N, T);
  *u = r * uu[0] + a * uu[1] + b * uu[2];
  *v = r * vv[0] + a * vv[1] + b * vv[2];
  VDot(d, *N, *N);
  if (d > BEZIER_EPSILON) 
    {
    d = 1.0 / sqrt(d);
    VScaleEq(*N, d);
    }
  else
    Make_Vector(N, 1, 0, 0);
  return 1;
  }

/* Find a sphere that contains all of the points in the list "vectors" */
static void find_average(vector_count, vectors, center, radius)
int vector_count;
VECTOR *vectors, *center;
DBL *radius;
  {
  DBL r0, r1, xc=0, yc=0, zc=0;
  DBL x0, y0, z0;
  int i;
  for (i=0;i<vector_count;i++) 
    {
    xc += vectors[i].x;
    yc += vectors[i].y;
    zc += vectors[i].z;
    }
  xc /= (DBL)vector_count;
  yc /= (DBL)vector_count;
  zc /= (DBL)vector_count;
  r0 = 0.0;
  for (i=0;i<vector_count;i++) 
    {
    x0 = vectors[i].x - xc;
    y0 = vectors[i].y - yc;
    z0 = vectors[i].z - zc;
    r1 = x0 * x0 + y0 * y0 + z0 * z0;
    if (r1 > r0) r0 = r1;
    }
  center->x = xc; center->y = yc; center->z = zc;
  *radius = r0;
  }

static int spherical_bounds_check(Ray, center, radius)
RAY *Ray;
VECTOR *center;
DBL radius;
  {
  DBL x, y, z, dist1, dist2;
  x = center->x - Ray->Initial.x;
  y = center->y - Ray->Initial.y;
  z = center->z - Ray->Initial.z;
  dist1 = x * x + y * y + z * z;
  if (dist1 < radius)
    /* ray starts inside sphere - assume it intersects. */
    return 1;
  else 
    {
    dist2 = x*Ray->Direction.x + y*Ray->Direction.y + z*Ray->Direction.z;
    dist2 = dist2 * dist2;
    if (dist2 > 0 && (dist1 - dist2 < radius))
      return 1;
    }
  return 0;
  }

/* Find a sphere that bounds all of the control points of a Bezier patch.
   The values returned are: the center of the bounding sphere, and the
   square of the radius of the bounding sphere. */
static void bezier_bounding_sphere(Patch, center, radius)
VECTOR (*Patch)[4][4], *center;
DBL *radius;
  {
  DBL r0, r1, xc=0, yc=0, zc=0;
  DBL x0, y0, z0;
  int i, j;
  for (i=0;i<4;i++) 
    {
    for (j=0;j<4;j++) 
      {
      xc += (*Patch)[i][j].x;
      yc += (*Patch)[i][j].y;
      zc += (*Patch)[i][j].z;
      }
    }
  xc /= 16.0;
  yc /= 16.0;
  zc /= 16.0;
  r0 = 0.0;
  for (i=0;i<4;i++) 
    {
    for (j=0;j<4;j++) 
      {
      x0 = (*Patch)[i][j].x - xc;
      y0 = (*Patch)[i][j].y - yc;
      z0 = (*Patch)[i][j].z - zc;
      r1 = x0 * x0 + y0 * y0 + z0 * z0;
      if (r1 > r0) r0 = r1;
      }
    }
  center->x = xc; center->y = yc; center->z = zc;
  *radius = r0;
  }

/* Precompute grid points and normals for a bezier patch */
void Precompute_Patch_Values(Shape)
BICUBIC_PATCH *Shape;
  {
  int i, j;
  VECTOR Control_Points[16];
  VECTOR (*Patch_Ptr)[4][4] = (VECTOR (*)[4][4]) Shape->Control_Points;

  /* Calculate the bounding sphere for the entire patch. */
  for (i=0;i<4;i++) for (j=0;j<4;j++)
    Control_Points[4*i+j] = Shape->Control_Points[i][j];
  find_average(16, &Control_Points[0], &Shape->Bounding_Sphere_Center,
    &Shape->Bounding_Sphere_Radius);

  if (Shape->Patch_Type == 0)
    return;
  else 
    {
    if (Shape->Node_Tree != NULL)
      bezier_tree_deleter(Shape->Node_Tree);
    Shape->Node_Tree = bezier_tree_builder(Shape, Patch_Ptr,
      0.0, 1.0, 0.0, 1.0, 0);
    return;
    }
  }

/* Determine the distance from a point to a plane. */
static DBL point_plane_distance(p, n, d)
VECTOR *p, *n;
DBL *d;
  {
  DBL temp1, temp2;

  VDot(temp1, *p, *n);
  temp1 += *d;
  VLength(temp2, *n);
  if (fabs(temp2) < EPSILON) return 0;
  temp1 /= temp2;
  return temp1;
  }

static void
bezier_subpatch_intersect(ray, Shape, Patch, u0, u1, v0, v1,
depth_count, Depths)
RAY *ray;
BICUBIC_PATCH *Shape;
VECTOR (*Patch)[4][4];
DBL u0, u1, v0, v1;
int *depth_count;
DBL *Depths;
  {
  int tcnt = Shape->Intersection_Count;
  VECTOR V[3];
  DBL u, v, Depth, uu[3], vv[3];
  VECTOR P, N;

  if (tcnt + *depth_count >= MAX_BICUBIC_INTERSECTIONS) return;
  V[0] = (*Patch)[0][0];
  V[1] = (*Patch)[0][3];
  V[2] = (*Patch)[3][0];

  uu[0] = u0; uu[1] = u0; uu[2] = u1;
  vv[0] = v0; vv[1] = v1; vv[2] = v1;

  if (intersect_subpatch(Shape, ray, V, uu, vv, &Depth, &P, &N, &u, &v)) 
    {
    Shape->IPoint[tcnt + *depth_count] = P;
    Shape->Normal_Vector[tcnt + *depth_count] = N;
    Depths[*depth_count] = Depth;
    *depth_count += 1;
    }

  if (tcnt + *depth_count >= MAX_BICUBIC_INTERSECTIONS) return;

  V[1] = V[2];
  V[2] = (*Patch)[3][0];
  uu[1] = uu[2]; uu[2] = u1;
  vv[1] = vv[2]; vv[2] = v0;

  if (intersect_subpatch(Shape, ray, V, uu, vv, &Depth, &P, &N, &u, &v)) 
    {
    Shape->IPoint[tcnt + *depth_count] = P;
    Shape->Normal_Vector[tcnt + *depth_count] = N;
    Depths[*depth_count] = Depth;
    *depth_count += 1;
    }
  }


static void bezier_split_left_right(Patch, Left_Patch, Right_Patch)
VECTOR (*Patch)[4][4], (*Left_Patch)[4][4], (*Right_Patch)[4][4];
  {
  VECTOR Temp1[4], Temp2[4], Half;
  int i, j;
  for (i=0;i<4;i++) 
    {
    Temp1[0] = (*Patch)[0][i];
    VHalf(Temp1[1], (*Patch)[0][i], (*Patch)[1][i]);
    VHalf(Half, (*Patch)[1][i], (*Patch)[2][i]);
    VHalf(Temp1[2], Temp1[1], Half);
    VHalf(Temp2[2], (*Patch)[2][i], (*Patch)[3][i]);
    VHalf(Temp2[1], Half, Temp2[2]);
    VHalf(Temp1[3], Temp1[2], Temp2[1]);
    Temp2[0] = Temp1[3];
    Temp2[3] = (*Patch)[3][i];
    for (j=0;j<4;j++) 
      {
      (*Left_Patch)[j][i]  = Temp1[j];
      (*Right_Patch)[j][i] = Temp2[j];
      }
    }
  }

static void bezier_split_up_down(Patch, Bottom_Patch, Top_Patch)
VECTOR (*Patch)[4][4], (*Top_Patch)[4][4], (*Bottom_Patch)[4][4];
  {
  VECTOR Temp1[4], Temp2[4], Half;
  int i, j;

  for (i=0;i<4;i++) 
    {
    Temp1[0] = (*Patch)[i][0];
    VHalf(Temp1[1], (*Patch)[i][0], (*Patch)[i][1]);
    VHalf(Half, (*Patch)[i][1], (*Patch)[i][2]);
    VHalf(Temp1[2], Temp1[1], Half);
    VHalf(Temp2[2], (*Patch)[i][2], (*Patch)[i][3]);
    VHalf(Temp2[1], Half, Temp2[2]);
    VHalf(Temp1[3], Temp1[2], Temp2[1]);
    Temp2[0] = Temp1[3];
    Temp2[3] = (*Patch)[i][3];
    for (j=0;j<4;j++) 
      {
      (*Bottom_Patch)[i][j] = Temp1[j];
      (*Top_Patch)[i][j]    = Temp2[j];
      }
    }
  }

/* See how close to a plane a subpatch is, the patch must have at least
   three distinct vertices. A negative result from this function indicates
   that a degenerate value of some sort was encountered. */
static DBL determine_subpatch_flatness(Patch)
VECTOR (*Patch)[4][4];
  {
  VECTOR vertices[4], n, TempV;
  DBL d, dist, temp1;
  int i, j;

  vertices[0] = (*Patch)[0][0];
  vertices[1] = (*Patch)[0][3];
  VSub(TempV, vertices[0], vertices[1]);
  VLength(temp1, TempV);
  if (fabs(temp1) < EPSILON) 
    {
    /* Degenerate in the V direction for U = 0. This is ok if the other
         two corners are distinct from the lower left corner - I'm sure there
         are cases where the corners coincide and the middle has good values,
         but that is somewhat pathalogical and won't be considered. */
    vertices[1] = (*Patch)[3][3];
    VSub(TempV, vertices[0], vertices[1]);
    VLength(temp1, TempV);
    if (fabs(temp1) < EPSILON) return -1.0;
    vertices[2] = (*Patch)[3][0];
    VSub(TempV, vertices[0], vertices[1]);
    VLength(temp1, TempV);
    if (fabs(temp1) < EPSILON) return -1.0;
    VSub(TempV, vertices[1], vertices[2]);
    VLength(temp1, TempV);
    if (fabs(temp1) < EPSILON) return -1.0;
    }
  else 
    {
    vertices[2] = (*Patch)[3][0];
    VSub(TempV, vertices[0], vertices[1]);
    VLength(temp1, TempV);
    if (fabs(temp1) < EPSILON) 
      {
      vertices[2] = (*Patch)[3][3];
      VSub(TempV, vertices[0], vertices[2]);
      VLength(temp1, TempV);
      if (fabs(temp1) < EPSILON)
        return -1.0;
      VSub(TempV, vertices[1], vertices[2]);
      VLength(temp1, TempV);
      if (fabs(temp1) < EPSILON)
        return -1.0;
      }
    else 
      {
      VSub(TempV, vertices[1], vertices[2]);
      VLength(temp1, TempV);
      if (fabs(temp1) < EPSILON)
        return -1.0;
      }
    }
  /* Now that a good set of candidate points has been found, find the
      plane equations for the patch */
  if (subpatch_normal(&vertices[0], &vertices[1], &vertices[2], &n, &d)) 
    {
    /* Step through all vertices and see what the maximum distance from the
             plane happens to be. */
    dist = 0.0;
    for (i=0;i<4;i++) 
      {
      for (j=0;j<4;j++) 
        {
        temp1 = fabs(point_plane_distance(&((*Patch)[i][j]), &n, &d));
        if (temp1 > dist)
          dist = temp1;
        }
      }
    return dist;
    }
  else 
    {
    if (Options & DEBUGGING) 
      {
      printf("Subpatch normal failed in determine_subpatch_flatness\n");
      fflush(stdout);
      }
    return -1.0;
    }
  }

static int flat_enough(Object, Patch)
BICUBIC_PATCH *Object;
VECTOR (*Patch)[4][4];
  {
  DBL Dist;

  Dist = determine_subpatch_flatness(Patch);
  if (Dist < 0.0)
    return 0;
  else if (Dist < Object->Flatness_Value)
    return 1;
  else
    return 0;
  }

static void bezier_subdivider(Ray, Object, Patch, u0, u1, v0, v1,
recursion_depth, depth_count, Depths)
RAY *Ray;
BICUBIC_PATCH *Object;
VECTOR (*Patch)[4][4];
DBL u0, u1, v0, v1;
int recursion_depth, *depth_count;
DBL *Depths;
  {
  VECTOR Lower_Left[4][4], Lower_Right[4][4];
  VECTOR Upper_Left[4][4], Upper_Right[4][4];
  VECTOR center;
  DBL ut, vt, radius;
  int tcnt = Object->Intersection_Count;

  /* Don't waste time if there are already too many intersections */
  if (tcnt >= MAX_BICUBIC_INTERSECTIONS) return;

  /* Make sure the ray passes through a sphere bounding the control points of
      the patch */
  bezier_bounding_sphere(Patch, &center, &radius);
  if (!spherical_bounds_check(Ray, &center, radius))
    return;

  /* If the patch is close to being flat, then just perform a ray-plane
      intersection test. */
  if (flat_enough(Object, Patch))
    bezier_subpatch_intersect(Ray, Object, Patch, u0, u1, v0, v1,
      depth_count, Depths);

  if (recursion_depth >= Object->U_Steps)
    if (recursion_depth >= Object->V_Steps)
      bezier_subpatch_intersect(Ray, Object, Patch, u0, u1, v0, v1,
        depth_count, Depths);
    else 
      {
      bezier_split_up_down(Patch, (VECTOR (*)[4][4])Lower_Left,
        (VECTOR (*)[4][4])Upper_Left);
      vt = (v1 - v0) / 2.0;
      bezier_subdivider(Ray, Object, (VECTOR (*)[4][4])Lower_Left,
        u0, u1, v0, vt,
        recursion_depth+1, depth_count, Depths);
      bezier_subdivider(Ray, Object, (VECTOR (*)[4][4])Upper_Left,
        u0, u1, vt, v1,
        recursion_depth+1, depth_count, Depths);
      }
  else if (recursion_depth >= Object->V_Steps) 
    {
    bezier_split_left_right(Patch, (VECTOR (*)[4][4])Lower_Left,
      (VECTOR (*)[4][4])Lower_Right);
    ut = (u1 - u0) / 2.0;
    bezier_subdivider(Ray, Object, (VECTOR (*)[4][4])Lower_Left,
      u0, ut, v0, v1,
      recursion_depth+1, depth_count, Depths);
    bezier_subdivider(Ray, Object, (VECTOR (*)[4][4])Lower_Right,
      ut, u1, v0, v1,
      recursion_depth+1, depth_count, Depths);
    }
  else 
    {
    ut = (u1 - u0) / 2.0;
    vt = (v1 - v0) / 2.0;
    bezier_split_left_right(Patch, (VECTOR (*)[4][4])Lower_Left,
      (VECTOR (*)[4][4])Lower_Right);
    bezier_split_up_down((VECTOR (*)[4][4])Lower_Left,
      (VECTOR (*)[4][4])Lower_Left,
      (VECTOR (*)[4][4])Upper_Left);
    bezier_split_up_down((VECTOR (*)[4][4])Lower_Right,
      (VECTOR (*)[4][4])Lower_Right,
      (VECTOR (*)[4][4])Upper_Right);
    bezier_subdivider(Ray, Object, (VECTOR (*)[4][4])Lower_Left,
      u0, ut, v0, vt,
      recursion_depth+1, depth_count, Depths);
    bezier_subdivider(Ray, Object, (VECTOR (*)[4][4])Upper_Left,
      u0, ut, vt, v1,
      recursion_depth+1, depth_count, Depths);
    bezier_subdivider(Ray, Object, (VECTOR (*)[4][4])Lower_Right,
      ut, u1, v0, vt,
      recursion_depth+1, depth_count, Depths);
    bezier_subdivider(Ray, Object, (VECTOR (*)[4][4])Upper_Right,
      ut, u1, vt, v1,
      recursion_depth+1, depth_count, Depths);
    }
  }

static void bezier_tree_deleter(Node)
BEZIER_NODE *Node;
  {
  BEZIER_CHILDREN *Children;
  int i;

  /* If this is an interior node then continue the descent */
  if (Node->Node_Type == BEZIER_INTERIOR_NODE) 
    {
    Children = (BEZIER_CHILDREN *)Node->Data_Ptr;
    for (i=0;i<Node->Count;i++)
      bezier_tree_deleter(Children->Children[i]);
    free((void *)Children);
    }
  else if (Node->Node_Type == BEZIER_LEAF_NODE) 
    {
    /* Free the memory used for the vertices. */
    free((void *)Node->Data_Ptr);
    }
  /* Free the memory used for the node. */
  free((void *)Node);
  }

static void bezier_tree_walker(Ray, Shape, Node, depth, depth_count, Depths)
RAY *Ray;
BICUBIC_PATCH *Shape;
BEZIER_NODE *Node;
int depth, *depth_count;
DBL *Depths;
  {
  BEZIER_CHILDREN *Children;
  BEZIER_VERTICES *Vertices;
  VECTOR N, P, V[3];
  DBL Depth, u, v, uu[3], vv[3];
  int i, tcnt = Shape->Intersection_Count;

  /* Don't waste time if there are already too many intersections */
  if (tcnt >= MAX_BICUBIC_INTERSECTIONS) return;

  /* Make sure the ray passes through a sphere bounding the control points of
      the patch */
  if (!spherical_bounds_check(Ray, &(Node->Center), Node->Radius_Squared))
    return;

  /* If this is an interior node then continue the descent, else
      do a check against the vertices. */
  if (Node->Node_Type == BEZIER_INTERIOR_NODE) 
    {
    Children = (BEZIER_CHILDREN *)Node->Data_Ptr;
    for (i=0;i<Node->Count;i++)
      bezier_tree_walker(Ray, Shape, Children->Children[i],
        depth+1, depth_count, Depths);
    }
  else if (Node->Node_Type == BEZIER_LEAF_NODE) 
    {
    Vertices = (BEZIER_VERTICES *)Node->Data_Ptr;
    V[0] = Vertices->Vertices[0];
    V[1] = Vertices->Vertices[1];
    V[2] = Vertices->Vertices[2];

    uu[0] = Vertices->uvbnds[0];
    uu[1] = Vertices->uvbnds[0];
    uu[2] = Vertices->uvbnds[1];
    vv[0] = Vertices->uvbnds[2];
    vv[1] = Vertices->uvbnds[3];
    vv[2] = Vertices->uvbnds[3];

    /* Triangulate this subpatch, then check for intersections in
         the triangles. */
    if (intersect_subpatch(Shape, Ray, V, uu, vv, &Depth, &P, &N, &u, &v)) 
      {
      Shape->IPoint[tcnt + *depth_count] = P;
      Shape->Normal_Vector[tcnt + *depth_count] = N;
      Depths[*depth_count] = Depth;
      *depth_count += 1;
      }
    if (*depth_count + tcnt >= MAX_BICUBIC_INTERSECTIONS) return;

    V[1] = V[2];
    V[2] = Vertices->Vertices[3];
    uu[1] = uu[2]; uu[2] = Vertices->uvbnds[1];
    vv[1] = vv[2]; vv[2] = Vertices->uvbnds[2];

    if (intersect_subpatch(Shape, Ray, V, uu, vv, &Depth, &P, &N, &u, &v)) 
      {
      Shape->IPoint[tcnt + *depth_count] = P;
      Shape->Normal_Vector[tcnt + *depth_count] = N;
      Depths[*depth_count] = Depth;
      *depth_count += 1;
      }
    }
  else 
    {
    printf("Bad Node type at depth %d\n", depth);
    }
  }

static int intersect_bicubic_patch0(Ray, Shape, Depths)
RAY *Ray;
BICUBIC_PATCH *Shape;
DBL *Depths;
  {
  int cnt = 0;
  VECTOR (*Patch)[4][4] = (VECTOR (*)[4][4]) Shape->Control_Points;

  bezier_subdivider(Ray, Shape, Patch, 0.0, 1.0, 0.0, 1.0,
    0, &cnt, Depths);
  return cnt;
  }

static int intersect_bicubic_patch1(Ray, Shape, Depths)
RAY *Ray;
BICUBIC_PATCH *Shape;
DBL *Depths;
  {
  int cnt = 0;
  bezier_tree_walker(Ray, Shape, Shape->Node_Tree, 0, &cnt, Depths);
  return cnt;
  }

int All_Bicubic_Patch_Intersections(Object, Ray, Depth_Stack)
OBJECT *Object;
RAY *Ray;
ISTACK *Depth_Stack;
  {
  DBL Depths[MAX_BICUBIC_INTERSECTIONS];
  VECTOR IPoint;
  int cnt, tcnt, i, Found;

  Found = FALSE;
  Ray_Bicubic_Tests++;

  if (Ray == CM_Ray)
    ((BICUBIC_PATCH *)Object)->Intersection_Count = 0;

  tcnt = ((BICUBIC_PATCH *)Object)->Intersection_Count;

  switch (((BICUBIC_PATCH *)Object)->Patch_Type)
  {
  case 0: 
    cnt = intersect_bicubic_patch0(Ray, ((BICUBIC_PATCH *)Object), &Depths[0]);
    break;
  case 1: 
    cnt = intersect_bicubic_patch1(Ray, ((BICUBIC_PATCH *)Object), &Depths[0]);
    break;
  default: 
    Error("Bad patch type\n");
  }

  if (cnt > 0) Ray_Bicubic_Tests_Succeeded++;
  for (i=0;i<cnt;i++) 
    {
    if (!Shadow_Test_Flag)
      ((BICUBIC_PATCH *)Object)->Intersection_Count++;
    IPoint = ((BICUBIC_PATCH *)Object)->IPoint[tcnt + i];
    if (Point_In_Clip(&IPoint,Object->Clip))
      {
      push_entry(Depths[i], IPoint, Object, Depth_Stack);
      Found = TRUE;
      }
    }
  return (Found);
  }

/* A patch is not a solid, so an inside test doesn't make sense. */
int Inside_Bicubic_Patch (IPoint, Object)
VECTOR *IPoint;
OBJECT *Object;
  {
  return 0;
  }

void Bicubic_Patch_Normal (Result, Object, IPoint)
OBJECT *Object;
VECTOR *Result, *IPoint;
  {
  BICUBIC_PATCH *Patch = (BICUBIC_PATCH *)Object;
  int i;

  /* If all is going well, the normal was computed at the time the intersection
      was computed.  Look on the list of associated intersection points and normals */
  for (i=0;i<Patch->Intersection_Count;i++)
    if (IPoint->x == Patch->IPoint[i].x &&
      IPoint->y == Patch->IPoint[i].y &&
      IPoint->z == Patch->IPoint[i].z) 
      {
      Result->x = Patch->Normal_Vector[i].x;
      Result->y = Patch->Normal_Vector[i].y;
      Result->z = Patch->Normal_Vector[i].z;
      return;
      }
  if (Options & DEBUGGING) 
    {
    printf("Bicubic patch normal for unknown intersection point\n");
    fflush(stdout);
    }
  Result->x = 1.0;
  Result->y = 0.0;
  Result->z = 0.0;
  }

void Translate_Bicubic_Patch (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  BICUBIC_PATCH *Patch = (BICUBIC_PATCH *) Object;
  int i, j;
  for (i=0;i<4;i++) for (j=0;j<4;j++)
    VAdd (Patch->Control_Points[i][j], Patch->Control_Points[i][j], *Vector)
      Precompute_Patch_Values(Patch);
  }

void Rotate_Bicubic_Patch (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  TRANSFORM Trans;

  Compute_Rotation_Transform (&Trans, Vector);
  Transform_Bicubic_Patch (Object,&Trans);
  }

void Scale_Bicubic_Patch (Object, Vector)
OBJECT *Object;
VECTOR *Vector;
  {
  BICUBIC_PATCH *Patch = (BICUBIC_PATCH *) Object;
  int i, j;
  for (i=0;i<4;i++) for (j=0;j<4;j++)
    VEvaluate(Patch->Control_Points[i][j],
      Patch->Control_Points[i][j], *Vector);
  Precompute_Patch_Values(Patch);
  }


/* Inversion of a patch really doesn't make sense. */
void Invert_Bicubic_Patch (Object)
OBJECT *Object;
  {
  ;
  }

BICUBIC_PATCH *Create_Bicubic_Patch()
  {
  BICUBIC_PATCH *New;

  if ((New = (BICUBIC_PATCH *) malloc (sizeof (BICUBIC_PATCH))) == NULL)
    MAError ("bicubic patch");

  INIT_OBJECT_FIELDS(New,BICUBIC_PATCH_OBJECT,&Bicubic_Patch_Methods)

    New->Patch_Type = -1;
  New->U_Steps = 0;
  New->V_Steps = 0;
  New->Intersection_Count = 0;
  New->Flatness_Value = 0.0;
  New->Node_Tree = NULL;

  /* NOTE: Control_Points[4][4] is initialized in Parse_Bicubic_Patch.
   Bounding_Sphere_Center,Bounding_Sphere_Radius, Normal_Vector[], and 
   IPoint[] are initialized in Precompute_Patch_Values. 
 */

  return (New);
  }

void *Copy_Bicubic_Patch (Object)
OBJECT *Object;
  {
  BICUBIC_PATCH *New;
  int i,j;

  New = Create_Bicubic_Patch ();

  /*  Do not do *New = *Old so that Precompute works right */

  New->Patch_Type = ((BICUBIC_PATCH *)Object)->Patch_Type;
  New->U_Steps    = ((BICUBIC_PATCH *)Object)->U_Steps;
  New->V_Steps    = ((BICUBIC_PATCH *)Object)->V_Steps;
  for (i=0;i<4;i++) for (j=0;j<4;j++)
    New->Control_Points[i][j]
    = ((BICUBIC_PATCH *)Object)->Control_Points[i][j];

  New->Flatness_Value     = ((BICUBIC_PATCH *)Object)->Flatness_Value;
  New->Intersection_Count = ((BICUBIC_PATCH *)Object)->Intersection_Count;

  Precompute_Patch_Values(New);

  return (New);
  }

void Transform_Bicubic_Patch (Object, Trans)
OBJECT *Object;
TRANSFORM *Trans;
  {
  BICUBIC_PATCH *Patch = (BICUBIC_PATCH *) Object;
  int i, j;

  for (i=0;i<4;i++) for (j=0;j<4;j++)
    MTransPoint(&(Patch->Control_Points[i][j]),
      &(Patch->Control_Points[i][j]),
      Trans);
  Precompute_Patch_Values(Patch);
  }

void Destroy_Bicubic_Patch (Object)
OBJECT *Object;
  {

  /* Need to add code to free() all malloc'ed memory created for this
     object. */
  free (Object);     
  }
