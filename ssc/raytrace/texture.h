/****************************************************************************
*                   texture.h
*
*  This file contains defines and variables for the txt*.c files
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

extern long Calls_To_Noise, Calls_To_DNoise;

#define MINX	-10000		/* Ridiculously large scaling values */
#define MINY	MINX
#define MINZ	MINX

#define MAXSIZE 267
#define RNDMASK 0x7FFF
#define RNDDIVISOR (DBL) RNDMASK
#define NUMBER_OF_WAVES 10
#define SINTABSIZE 1000

#define FLOOR(x) ((x) >= 0.0 ? floor(x) : (0.0 - floor(0.0 - (x)) - 1.0))
#define FABS(x) ((x) < 0.0 ? (0.0 - x) : (x))
#define SCURVE(a) ((a)*(a)*(3.0-2.0*(a)))
#define REALSCALE ( 2.0 / 65535.0 )
#define Hash3d(a,b,c) hashTable[(int)(hashTable[(int)(hashTable[(int)((a) & 0xfffL)] \
						 ^ ((b) & 0xfffL))] ^ ((c) & 0xfffL))]
#define Hash2d(a,b) hashTable[(int)(hashTable[(int)((a) & 0xfffL)] ^ ((b) & 0xfffL))]
#define Hash1d(a,b) hashTable[(int)(a) ^ ((b) & 0xfffL)]  
#define INCRSUM(m,s,x,y,z)	((s)*(RTable[m]*0.5		\
					+ RTable[m+1]*(x)	\
					+ RTable[m+2]*(y)	\
					+ RTable[m+3]*(z)))

#define INCRSUMP(mp,s,x,y,z) ((s)*((mp[0])*0.5 + (mp[1])*(x) + (mp[2])*(y) + (mp[3])*(z)))

extern int Options;
extern DBL *sintab;
extern DBL frequency[NUMBER_OF_WAVES];
extern VECTOR Wave_Sources[NUMBER_OF_WAVES];
extern DBL *RTable;
extern short *hashTable;
extern unsigned short crctab[256];
