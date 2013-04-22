/*
  This is the render program for SNGRAY1,
  I modify it from the render.c.
  C FU
*/


/****************************************************************************
*                   render.c
*
*  This module implements the main raytracing loop.
*
* 08/07/92 lsk    Changed the normal antialiasing function to use a loop 
*                 where the number of rays per pixel when antialiasing can 
*                 be sepcified.
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
******************************************************************************/

#include "syn-ray.h"
#include "syn-ray.c"

#include "frame.h"
#include "vector.h"
#include "povproto.h"

extern FILE_HANDLE *Output_File_Handle;
extern char Output_File_Name[FILE_NAME_LENGTH];
extern char Input_File_Name[FILE_NAME_LENGTH];
extern char Stat_File_Name[FILE_NAME_LENGTH];
extern char OutputFormat, Color_Bits, PaletteOption;
extern char VerboseFormat;
extern unsigned int Options;
extern int File_Buffer_Size;
extern int Use_Slabs;
volatile int Stop_Flag;
extern int First_Line, Last_Line;
extern int First_Column, Last_Column;
extern long Number_Of_Pixels, Number_Of_Rays, Number_Of_Pixels_Supersampled;
extern short *hashTable;
extern unsigned short crctab[256];
extern OBJECT *Root_Object;
extern long AntialiasDepth;
extern DBL JitterScale;

#define rand3d(a,b) crctab[(int)(hashTable[(int)(hashTable[(int)((a)&0xfff)]^(b))&0xfff])&0xff]

FRAME Frame;
RAY *CM_Ray;
int Trace_Level, SuperSampleCount;

DBL Max_Trace_Level = 5;
DBL maxclr;

static void check_stats PARAMS((int y));
static void do_anti_aliasing PARAMS((int x, int y, COLOUR *Colour));
static void output_line PARAMS((int y));

COLOUR *Previous_Line, *Current_Line;

char *Previous_Line_Antialiased_Flags, *Current_Line_Antialiased_Flags;
RAY Ray;

void Create_Ray (ray, width, height, x, y)
RAY *ray;
int width, height;
DBL x, y;
  {
  register DBL X_Scalar, Y_Scalar;
  VECTOR Temp_Vect_1, Temp_Vect_2;

  /* Convert the X Coordinate to be a DBL from 0.0 to 1.0 */
  X_Scalar = (x - (DBL) width / 2.0) / (DBL) width;

  /* Convert the Y Coordinate to be a DBL from 0.0 to 1.0 */
  Y_Scalar = (( (DBL)(Frame.Screen_Height - 1) - y) -
    (DBL) height / 2.0) / (DBL) height;

  VScale (Temp_Vect_1, Frame.Camera->Up, Y_Scalar);
  VScale (Temp_Vect_2, Frame.Camera->Right, X_Scalar);
  VAdd (ray->Direction, Temp_Vect_1, Temp_Vect_2);
  VAdd (ray->Direction, ray->Direction, Frame.Camera->Direction);
  VNormalize (ray->Direction, ray->Direction);
  Initialize_Ray_Containers (ray);
  ray->Quadric_Constants_Cached = FALSE;
  }

void Read_Rendered_Part()
  {
  int rc, x, line_number;
  unsigned char Red, Green, Blue;
  DBL grey;

  maxclr = (DBL)(1 << Color_Bits) - 1.0;
  while ((rc = Read_Line(Output_File_Handle, Previous_Line, &line_number)) == 1) 
    {
    if (Options & DISPLAY)
      for (x = 0 ; x < Frame.Screen_Width ; x++) 
      {
      if (PaletteOption == GREY) 
        {
        grey = Previous_Line[x].Red * 0.287 +
        Previous_Line[x].Green * 0.589 +
        Previous_Line[x].Blue * 0.114;
        Red = Green = Blue = (unsigned char)(grey * maxclr);
        }
      else 
        {
        Red = (unsigned char) (Previous_Line[x].Red * maxclr);
        Green = (unsigned char) (Previous_Line[x].Green * maxclr);
        Blue = (unsigned char) (Previous_Line[x].Blue * maxclr);
        }
      display_plot (x, line_number, Red, Green, Blue);
      COOPERATE     /* Moved inside loop JLN 12/91 */
        }
      }

    First_Line = line_number+1;

  if (rc == 0) 
    {
    Close_File(Output_File_Handle);
    if (Open_File (Output_File_Handle, Output_File_Name,
      &Frame.Screen_Width, &Frame.Screen_Height, File_Buffer_Size,
      APPEND_MODE) != 1) 
      {
      fprintf (stderr, "Error opening output file\n");
      fflush(stdout);
      close_all();
      exit(1);
      }
    return;
    }

  fprintf (stderr, "Error reading aborted data file\n");
  }


void Start_Tracing ()
  {
  COLOUR Colour;
  register int x, y;
  unsigned char Red, Green, Blue;
  DBL grey;

  int job_tuple,output_tuple,status;
  char tpname[128];
  JOB_UNIT X;
  unsigned char *data;
  int XRES,YRES;
  int CNT; /* count the pixels */ 
  int RUN=1;
  
  getjob(&XRES,&YRES);
  printf("\n JOB: %d %d",XRES,YRES);
  data=malloc((2+3*XRES)*sizeof(char));
  if (!data) 
  {
    printf("\n memory error on worker!");
    exit(0);
  }
  
  job_tuple=cnf_open("job",0);
  output_tuple=cnf_open("output",0);

while(RUN)
{ 
 strcpy(tpname,"*");
 status=cnf_tsget(job_tuple,tpname,&X,0);
 printf("\n Worker get Jobs: %d ~ %d ",X.y1,X.y2);
 OUT;
 if (X.y1==TERM)    /* I define this is the END condiction */
 {
  sprintf(tpname,"i%d\0",0);
  status=cnf_tsput(job_tuple,tpname,X,sizeof(JOB_UNIT));
  printf("\n Worker die!");
  OUT;
  cnf_term();
  RUN=0;
 } 
 else  /* if not END, do the job! :-( */
 {
 for (y=X.y1;y<=X.y2;y++)
    {
    int x_count; 

    data[0]=y/256;
    data[1]=y%256;
    x_count=2;
  
    printf("\n Process Y: %d,  X1: %d,  X2: %d ",y,First_Column,Last_Column); 
    check_stats(y);

      for (x = 1; x<=XRES; x++)
      {

      Check_User_Abort(1);
      
      Number_Of_Pixels++;

      Create_Ray (CM_Ray, Frame.Screen_Width, Frame.Screen_Height, (DBL) x, (DBL) y);
      Trace_Level = 0;
      Trace (&Ray, &Colour);
      Clip_Colour (&Colour, &Colour);

      Current_Line[x] = Colour;

      if (Options & ANTIALIAS)      
        do_anti_aliasing(x, y, &Colour); 

      if (y != First_Line-1) 
        {

        if (PaletteOption == GREY) 
          {
          grey = Colour.Red * 0.287 +
          Colour.Green * 0.589 +
          Colour.Blue * 0.114;
          Red = Green = Blue = (unsigned char)(grey * maxclr);
          }
        else 
          {
          Red = (unsigned char) (Colour.Red * maxclr);
          Green = (unsigned char) (Colour.Green * maxclr);
          Blue = (unsigned char) (Colour.Blue * maxclr);
          }
        if (Options & DISPLAY)
          display_plot (x, y, Red, Green, Blue);
        data[x_count++]=Red;
        data[x_count++]=Green;
        data[x_count++]=Blue;
          } /* Y!=first_line.. */
      }  /* finish a line */
    
/* Put the calc. result back ! */
   
     sprintf(tpname,"i%d\0",data[1]); /* Just a name */
     status=
     cnf_tsput(output_tuple,tpname,data,(2+3*XRES)*sizeof(char));


 
    output_line(y);
    }

  if (Options & DISKWRITE) 
    {
    Write_Line (Output_File_Handle, Previous_Line, Last_Line - 1);
    }
  }
}
}

static void check_stats(y)
register int y;
  {
  FILE *stat_file;

  /* New verbose options CdW */
  if (Options & VERBOSE && VerboseFormat=='0')
    {
    printf ("POV-Ray rendering %s to %s",Input_File_Name,Output_File_Name);
    if((First_Line != 0) || (Last_Line != Frame.Screen_Height))
      printf(" from %4d to %4d:\n",First_Line, Last_Line);
    else
      printf (":\n");
    printf ("Res %4d X %4d. Calc line %4d of %4d",Frame.Screen_Width, Frame.Screen_Height, (y-First_Line)+1, Last_Line-First_Line);
    if (!(Options & ANTIALIAS))
      printf(".");
    }
  if (Options & VERBOSE_FILE)
    {
    stat_file = fopen(Stat_File_Name,"w+t");
    fprintf (stat_file,"Line %4d.\n", y);
    fclose(stat_file);
    }

  /* Use -vO for Old style verbose */
  if (Options & VERBOSE && (VerboseFormat=='O')) 
    {
    printf ("Line %4d", y);
    }
  if (Options & VERBOSE && VerboseFormat=='1')
    {
    fprintf (stderr,"Res %4d X %4d. Calc line %4d of %4d",Frame.Screen_Width, Frame.Screen_Height, (y-First_Line)+1, Last_Line-First_Line);
    if (!(Options & ANTIALIAS))
      fprintf(stderr,".");
    }

  if (Options & ANTIALIAS)
    SuperSampleCount = 0;
  }

static void do_anti_aliasing(x, y, Colour)
register int x, y;
COLOUR *Colour;
  {
  char Antialias_Center_Flag = 0;

  Current_Line_Antialiased_Flags[x] = 0;

  if (x != 0) 
    {
    if (Colour_Distance (&Current_Line[x-1], &Current_Line[x])
      >= Frame.Antialias_Threshold) 
      {
      Antialias_Center_Flag = 1;
      if (!(Current_Line_Antialiased_Flags[x-1])) 
        {
        Supersample (&Current_Line[x-1],
          x-1, y, Frame.Screen_Width, Frame.Screen_Height);
        Current_Line_Antialiased_Flags[x-1] = 1;
        SuperSampleCount++;
        }
      }
    }

  if (y != First_Line-1) 
    {
    if (Colour_Distance (&Previous_Line[x], &Current_Line[x])
      >= Frame.Antialias_Threshold) 
      {
      Antialias_Center_Flag = 1;
      if (!(Previous_Line_Antialiased_Flags[x])) 
        {
        Supersample (&Previous_Line[x],
          x, y-1, Frame.Screen_Width, Frame.Screen_Height);
        Previous_Line_Antialiased_Flags[x] = 1;
        SuperSampleCount++;
        }
      }
    }

  if (Antialias_Center_Flag) 
    {
    Supersample (&Current_Line[x],
      x, y, Frame.Screen_Width, Frame.Screen_Height);
    Current_Line_Antialiased_Flags[x] = 1;
    *Colour = Current_Line[x];
    SuperSampleCount++;
    }

  return;
  }


void Initialize_Renderer PARAMS((void))
  {
  register int i;

  CM_Ray = &Ray;

  maxclr = (DBL)(1 << Color_Bits) - 1.0;

  /* These malloc's are never freed! Why ? Need a Deinit_Renderer() ?*/
  Previous_Line = (COLOUR *) malloc (sizeof (COLOUR)*(Frame.Screen_Width + 1));
  Current_Line = (COLOUR *) malloc (sizeof (COLOUR)*(Frame.Screen_Width + 1));

  for (i = 0 ; i <= Frame.Screen_Width ; i++) 
    {
    Previous_Line[i].Red = 0.0;
    Previous_Line[i].Green = 0.0;
    Previous_Line[i].Blue = 0.0;
    Current_Line[i].Red = 0.0;
    Current_Line[i].Green = 0.0;
    Current_Line[i].Blue = 0.0;
    }

  if (Options & ANTIALIAS) 
    {
    Previous_Line_Antialiased_Flags =
    (char *) malloc (sizeof (char)*(Frame.Screen_Width + 1));
    Current_Line_Antialiased_Flags =
    (char *)  malloc (sizeof (char)*(Frame.Screen_Width + 1));

    for (i = 0 ; i <= Frame.Screen_Width ; i++) 
      {
      (Previous_Line_Antialiased_Flags)[i] = 0;
      (Current_Line_Antialiased_Flags)[i] = 0;
      }
    }

  Ray.Initial = Frame.Camera->Location;
  return;
  }

static void output_line (y)
register int y;
  {
  COLOUR *Temp_Colour_Ptr;
  char *Temp_Char_Ptr;

  if (Options & DISKWRITE)
    if (y > First_Line) 
    {
    Write_Line (Output_File_Handle, Previous_Line, y-1);
    }

  if (Options & VERBOSE)
    {
    if (Options & ANTIALIAS && VerboseFormat != '1')
      printf (" supersampled %d times.", SuperSampleCount);

    if (Options & ANTIALIAS && VerboseFormat == '1')
      {
      fprintf (stderr," supersampled %d times.", SuperSampleCount);

      }
    if (VerboseFormat == '1')
      fprintf (stderr,"\r");
    else
      fprintf (stderr,"\n");
    }
  Temp_Colour_Ptr = Previous_Line;
  Previous_Line = Current_Line;
  Current_Line = Temp_Colour_Ptr;

  Temp_Char_Ptr = Previous_Line_Antialiased_Flags;
  Previous_Line_Antialiased_Flags = Current_Line_Antialiased_Flags;
  Current_Line_Antialiased_Flags = Temp_Char_Ptr;

  return;
  }

void Trace (Ray, Colour)
RAY *Ray;
COLOUR *Colour;
  {
  OBJECT *Object;
  INTERSECTION Best_Intersection, New_Intersection;
  register int Intersection_Found;

  COOPERATE
  Number_Of_Rays++;
  Make_Colour (Colour, 0.0, 0.0, 0.0);

  if (Trace_Level > (int) Max_Trace_Level) 
    return;

  Intersection_Found = FALSE;
  Best_Intersection.Depth = BOUND_HUGE;

    /* What objects does this ray intersect? */
  if (!Use_Slabs)
    for (Object = Frame.Objects ; 
         Object != NULL ;
         Object = Object -> Sibling) 
    {
    if (Intersection (&New_Intersection, Object, Ray))
      if (New_Intersection.Depth < Best_Intersection.Depth) 
        {
        Best_Intersection = New_Intersection;
        Intersection_Found = TRUE;
        }
    }
  else
    Intersection_Found = Bounds_Intersect(Root_Object, Ray,
      &Best_Intersection,&Object);

  if (Intersection_Found)
    Determine_Apparent_Colour (&Best_Intersection, Colour, Ray);
  else
    if (Frame.Fog_Distance > 0.0)
      *Colour = Frame.Fog_Colour;
    else
      *Colour = Frame.Background_Colour;
  }

/* exit with error if image not completed/user abort*/
void Check_User_Abort (Do_Stats)
int Do_Stats;
  {
  TEST_ABORT
  if (Stop_Flag) 
    {
    close_all();
    if (Do_Stats)
      {
      PRINT_STATS
      }
    exit(2);
    }
  }

/*---------------  Standard sampling in loop  -----------------------*/

unsigned short JRanges[] = {1,1,1,1,3,2,5,3,7,4}; /* LSK */

void Supersample (result, x, y, Width, Height)
COLOUR *result;
int x, y, Width, Height;
  {
  COLOUR colour;
  register DBL dx, dy, Jitter_X, Jitter_Y;
  register int Jitt_Offset;
  unsigned char Red, Green, Blue;
  int JRange;                               /* LSK */
  int JSteps;                               /* LSK */
  DBL JScale;                               /* LSK */
  DBL JSize,JOffset;                        /* LSK */
  int i,j;                                  /* LSK */

  dx = (DBL) x;                             /* LSK */
  dy = (DBL) y;                             /* LSK */
  Jitt_Offset = 10;

  Number_Of_Pixels_Supersampled++;

  Make_Colour (result, 0.0, 0.0, 0.0);

  if (AntialiasDepth>1)                                           /* LSK */
    {                                                             /* LSK */
    /* JSize is the size of the jitter scattering area */
    JSize = 1.0/AntialiasDepth;                                   /* LSK */

    /* JOffset is the 'radius' of the jitter scatter area */
    JOffset = JSize/2.0;                                        /* LSK */

    /* JSteps is either 1 or 2 depending on whether the number of samples
        is odd or even. This is because the loop need to either run through
        or over 0
     */
    JSteps = 2-(AntialiasDepth % 2);                              /* LSK */

    /* JRange is the range that the loop will run through. I couldn't
        come up with a function describing the values, so I used an array
        for 2x2 up to 9x9.
     */
    JRange = JRanges[AntialiasDepth];                             /* LSK */

    /* JScale is the scaling value for the color resulting from the
        ray before adding to the resultant color
     */
    JScale = 1.0/(DBL)(AntialiasDepth*AntialiasDepth);              /* LSK */

    for (i=-JRange;i<=JRange;i+=JSteps)
      for (j=-JRange;j<=JRange;j+=JSteps)
      {
      if (Options & JITTER)
        {
        Jitter_X = (rand3d(x+Jitt_Offset, y) & 0x7FFF) / 32768.0 * JSize - JOffset;
        Jitt_Offset++;
        Jitter_Y = (rand3d(x+Jitt_Offset, y) & 0x7FFF) / 32768.0 * JSize - JOffset;
        Jitt_Offset++;
        }
      else
        {
        Jitter_X=Jitter_Y=0.0;
        }
      Jitter_X*=JitterScale;
      Jitter_Y*=JitterScale;

      Create_Ray (CM_Ray, Frame.Screen_Width, Frame.Screen_Height,
        dx + Jitter_X + i * JSize/JSteps,
        dy + Jitter_Y + j * JSize/JSteps );

      Trace_Level = 0;
      Trace (CM_Ray, &colour);
      Clip_Colour (&colour, &colour);
      Scale_Colour (&colour, &colour, JScale );
      Add_Colour (result, result, &colour);

      }
    }                                       /* LSK */
  else   /* 1x1 specified! */
    {
    Create_Ray (CM_Ray, Frame.Screen_Width, Frame.Screen_Height,dx,dy );

    Trace_Level = 0;
    Trace (CM_Ray, &colour);
    Clip_Colour (&colour, &colour);
    Add_Colour (result, result, &colour);
    Jitt_Offset += 10;
    }

  if ((y != First_Line - 1) && (Options & DISPLAY)) 
    {
    Red = (unsigned char)(result->Red * maxclr);
    Green = (unsigned char)(result->Green * maxclr);
    Blue = (unsigned char)(result->Blue * maxclr);
    display_plot (x, y, Red, Green, Blue);
    }

  }


