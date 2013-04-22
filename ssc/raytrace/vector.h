/****************************************************************************
*                vector.h
*
*  This module contains macros to perform operations on vectors.
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

/* Misc. Vector Math Macro Definitions */

extern DBL VTemp;

/* Vector Add */
#define VAdd(a, b, c) {(a).x=(b).x+(c).x;(a).y=(b).y+(c).y;(a).z=(b).z+(c).z;}
#define VAddEq(a, b) {(a).x+=(b).x;(a).y+=(b).y;(a).z+=(b).z;}

/* Vector Subtract */
#define VSub(a, b, c) {(a).x=(b).x-(c).x;(a).y=(b).y-(c).y;(a).z=(b).z-(c).z;}
#define VSubEq(a, b) {(a).x-=(b).x;(a).y-=(b).y;(a).z-=(b).z;}

/* Scale - Multiply Vector by a Scalar */
#define VScale(a, b, k) {(a).x=(b).x*(k);(a).y=(b).y*(k);(a).z=(b).z*(k);}
#define VScaleEq(a, k) {(a).x*=(k);(a).y*=(k);(a).z*=(k);}

/* Inverse Scale - Divide Vector by a Scalar */
#define VInverseScale(a, b, k) {(a).x=(b).x/(k);(a).y=(b).y/(k);(a).z=(b).z/(k);}
#define VInverseScaleEq(a, k) {(a).x/=(k);(a).y/=(k);(a).z/=(k);}

/* Dot Product - Gives Scalar angle (a) between two vectors (b) and (c) */
#define VDot(a, b, c) {a=(b).x*(c).x+(b).y*(c).y+(b).z*(c).z;}

/* Cross Product - returns Vector (a) = (b) x (c) 
   WARNING:  a must be different from b and c.*/
#define VCross(a,b,c) {(a).x=(b).y*(c).z-(b).z*(c).y; \
                       (a).y=(b).z*(c).x-(b).x*(c).z; \
                       (a).z=(b).x*(c).y-(b).y*(c).x;}

/* Evaluate - returns Vector (a) = Multiply Vector (b) by Vector (c) */
#define VEvaluate(a, b, c) {(a).x=(b).x*(c).x;(a).y=(b).y*(c).y;(a).z=(b).z*(c).z;}
#define VEvaluateEq(a, b) {(a).x*=(b).x;(a).y*=(b).y;(a).z*=(b).z;}

/* Divide - returns Vector (a) = Divide Vector (b) by Vector (c) */
#define VDiv(a, b, c) {(a).x=(b).x/(c).x;(a).y=(b).y/(c).y;(a).z=(b).z/(c).z;}
#define VDivEq(a, b) {(a).x/=(b).x;(a).y/=(b).y;(a).z/=(b).z;}

/* Square a Vector */
#define	VSqr(a) {(a).x*(a).x;(a).y*(a).y;(a).z*(a).z;}

/* Simple Scalar Square Macro */
#define	Sqr(a)	((a)*(a))

/* Square a Vector (b) and Assign to another Vector (a) */
#define VSquareTerms(a, b) {(a).x=(b).x*(b).x;(a).y=(b).y*(b).y;(a).z=(b).z*(b).z;}

/* Vector Length - returs Scalar Euclidean Length (a) of Vector (b) */
#define VLength(a, b) {a=sqrt((b).x*(b).x+(b).y*(b).y+(b).z*(b).z);}

/* Normalize a Vector - returns a vector (length of 1) that points at (b) */
#define VNormalize(a,b) {VTemp=sqrt((b).x*(b).x+(b).y*(b).y+(b).z*(b).z);(a).x=(b).x/VTemp;(a).y=(b).y/VTemp;(a).z=(b).z/VTemp;}

/* Compute a Vector (a) Halfway Between Two Given Vectors (b) and (c) */
#define VHalf(a, b, c) {(a).x=0.5*((b).x+(c).x);(a).y=0.5*((b).y+(c).y);(a).z=0.5*((b).z+(c).z);}

