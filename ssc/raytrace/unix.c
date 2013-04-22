/****************************************************************************
*                unix.c
*
*  This module implements UNIX specific routines.
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


#include <math.h>
#include "config.h"

void unix_init_POVRAY PARAMS ((void))
   {
   }

#ifdef UNDERFLOW
int matherr (x)
   struct exception *x;
   {
   switch(x->type) 
     {
     case DOMAIN:
     case OVERFLOW:
        x->retval = 1.0e17;
        break;

     case SING:
     case UNDERFLOW:
        x->retval = 0.0;
        break;

     default:
        break;
     }
   return(1);
   }
#endif

void display_finished ()
   {
   }

void display_init ()
   {
   }

void display_close ()
   {
   }

void display_plot (x, y, Red, Green, Blue)
   int x, y;
   char Red, Green, Blue;
   {
   }
