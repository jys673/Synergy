/****************************************************************************
*                povray.c
*
*  This module contains the entry routine for the raytracer and the code to
*  parse the parameters on the command line.
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

#include <ctype.h>
#include <time.h>          /* BP */
#include "frame.h"		/* common to ALL modules in this program */
#include "povproto.h"

#define MAX_FILE_NAMES 1
unsigned int Options;
unsigned long Quality_Flags;
int Quality,Use_Slabs;
int Case_Sensitive_Flag = CASE_SENSITIVE_DEFAULT;

extern FRAME Frame;
OBJECT *Root_Object;

char Input_File_Name[FILE_NAME_LENGTH], Output_File_Name[FILE_NAME_LENGTH], Stat_File_Name[FILE_NAME_LENGTH];

#define MAX_LIBRARIES 10
char *Library_Paths[MAX_LIBRARIES];
int Library_Path_Index;
int Max_Symbols = 1000;

FILE_HANDLE *Output_File_Handle;
int File_Buffer_Size;
static int Number_Of_Files;
static int inflag, outflag; 
DBL VTemp;
DBL Antialias_Threshold;
int First_Line, Last_Line;
int First_Column, Last_Column;
DBL First_Column_Temp, Last_Column_Temp;
DBL First_Line_Temp, Last_Line_Temp;
int Display_Started = FALSE;
int Shadow_Test_Flag = FALSE;
DBL Clock_Value = 0.0;
DBL Language_Version = 2.0;
long Bounds_Threshold = 25;
long AntialiasDepth = 3;
DBL JitterScale = 1.0;

/* Quality constants */
  long Quality_Values[10]=
    {
    QUALITY_0, QUALITY_1, QUALITY_2, QUALITY_3, QUALITY_4,
    QUALITY_5, QUALITY_6, QUALITY_7, QUALITY_8, QUALITY_9
    };
/* Stats kept by the ray tracer: */
long Number_Of_Pixels, Number_Of_Rays, Number_Of_Pixels_Supersampled;
long Ray_Sphere_Tests, Ray_Sphere_Tests_Succeeded;
long Ray_Box_Tests, Ray_Box_Tests_Succeeded;    
long Ray_Blob_Tests, Ray_Blob_Tests_Succeeded;  
long Ray_Cone_Tests, Ray_Cone_Tests_Succeeded;
long Ray_Disc_Tests, Ray_Disc_Tests_Succeeded;
long Ray_Plane_Tests, Ray_Plane_Tests_Succeeded;
long Ray_Plane_Tests, Ray_Plane_Tests_Succeeded;
long Ray_Triangle_Tests, Ray_Triangle_Tests_Succeeded;
long Ray_Quadric_Tests, Ray_Quadric_Tests_Succeeded;
long Ray_Poly_Tests, Ray_Poly_Tests_Succeeded;
long Ray_Bicubic_Tests, Ray_Bicubic_Tests_Succeeded;
long Ray_Ht_Field_Tests, Ray_Ht_Field_Tests_Succeeded;
long Ray_Ht_Field_Box_Tests, Ray_HField_Box_Tests_Succeeded;
long Bounding_Region_Tests, Bounding_Region_Tests_Succeeded;
long Clipping_Region_Tests, Clipping_Region_Tests_Succeeded;
long Calls_To_Noise, Calls_To_DNoise;
/* SJA */
long Shadow_Ray_Tests, Shadow_Rays_Succeeded, Shadow_Cache_Hits;
/* SJA */
long Reflected_Rays_Traced, Refracted_Rays_Traced;
long Transmitted_Rays_Traced;
long Istack_overflows;
int Number_of_istacks;
int Max_Intersections;
ISTACK *free_istack;

DBL    tused;             /* Trace timer variables. - BP */ 
time_t tstart, tstop;

char DisplayFormat, OutputFormat, VerboseFormat, PaletteOption, Color_Bits;

struct Constant_Struct Constants[MAX_CONSTANTS];
#define NOCMDLINE 1     /* Should be in header file, but Put it here....
                           change later! */
#ifdef NOCMDLINE	/* a main() by any other name... */
#ifdef ALTMAIN
void alt_main()
#else
  void main()
#endif
#else
#ifdef ALTMAIN
  void alt_main(argc, argv)
#else
  void main(argc, argv)
#endif
    int argc;
char **argv;
#endif			/* ...would be a lot less hassle!! :-) AAC */
  {
  register int i;
  FILE *stat_file;

  STARTUP_POVRAY

  PRINT_CREDITS

  PRINT_OTHER_CREDITS

  /* Parse the command line parameters */
#ifndef NOCMDLINE
  if (argc == 1)
    usage();
#endif

  init_vars();

  Output_File_Name[0]='\0';

  Library_Paths[0] = NULL;
  Library_Path_Index = 0;

  /* Read the default parameters from povray.def */
  get_defaults();

#ifndef NOCMDLINE
  for (i = 1 ; i < argc ; i++ )
    if ((*argv[i] == '+') || (*argv[i] == '-'))
      parse_option(argv[i]);
    else
      parse_file_name(argv[i]);

#endif

  if (Last_Line == -1)
    Last_Line = Frame.Screen_Height;

  if (Last_Column == -1)
    Last_Column = Frame.Screen_Width-1;

  if (Options & DISKWRITE) 
    {
    switch (OutputFormat) 
    {
    case '\0':
    case 'd':
    case 'D':
      if ((Output_File_Handle = Get_Dump_File_Handle()) == NULL) 
        {
        close_all();
        exit(1);
        }
      break;
      /*
         case 'i':
         case 'I':
                   if ((Output_File_Handle = Get_Iff_File_Handle()) == NULL) {
                      close_all();
                      exit(1);
                      }
                   break;

*/
    case 'r':
    case 'R':
      if ((Output_File_Handle = Get_Raw_File_Handle()) == NULL) 
        {
        close_all();
        exit(1);
        }
      break;

    case 't':
    case 'T':
      if ((Output_File_Handle = Get_Targa_File_Handle()) == NULL) 
        {
        close_all();
        exit(1);
        }
      break;

    default:
      fprintf (stderr, "Unrecognized output file format %c\n", OutputFormat);
      close_all(); 
      exit(1);
    }
    if (Output_File_Name[0] == '\0')
      strcpy (Output_File_Name, Default_File_Name (Output_File_Handle));
    }

  Print_Options();

  Initialize_Tokenizer(Input_File_Name);
  fprintf (stderr,"Parsing...");
  if (Options & VERBOSE_FILE)
    {
    stat_file = fopen(Stat_File_Name,"w+t");
    fprintf (stat_file, "Parsing...\n");
    fclose(stat_file);
    }

  Parse ();
  Terminate_Tokenizer();

  if (Use_Slabs) 
    {
    fprintf (stderr, "Preprocessing...\n");  /* Added */
    BuildBoundingSlabs(&Root_Object);        /* Added */
    }

  if (Options & DISPLAY)
    {
    printf ("Displaying...\n");
    display_init(Frame.Screen_Width, Frame.Screen_Height);
    Display_Started = TRUE;
    }

  /* Get things ready for ray tracing */
  if (Options & DISKWRITE)
    if (Options & CONTINUE_TRACE) 
    {
    if (Open_File (Output_File_Handle, Output_File_Name,
      &Frame.Screen_Width, &Frame.Screen_Height, File_Buffer_Size,
      READ_MODE) != 1) 
      {
      fprintf (stderr, "Error opening continue trace output file\n");
      fprintf (stderr, "Opening new output file %s.\n",Output_File_Name);
      Options &= ~CONTINUE_TRACE; /* Turn off continue trace */

      if (Open_File (Output_File_Handle, Output_File_Name,
        &Frame.Screen_Width, &Frame.Screen_Height, File_Buffer_Size,
        WRITE_MODE) != 1) 
        {
        fprintf (stderr, "Error opening output file\n");
        close_all();
        exit(1);
        }
      }

    Initialize_Renderer();
    if (Options & CONTINUE_TRACE) 
      Read_Rendered_Part();
    }
    else 
    {
    if (Open_File (Output_File_Handle, Output_File_Name,
      &Frame.Screen_Width, &Frame.Screen_Height, File_Buffer_Size,
      WRITE_MODE) != 1) 
      {
      fprintf (stderr, "Error opening output file\n");
      close_all();
      exit(1);
      }

    Initialize_Renderer();
    }
  else
    Initialize_Renderer();

  Initialize_Noise();

  START_TIME  /* Store start time for trace. Timer macro in CONFIG.H */


  /* Ok, go for it - trace the picture */
  if ((Options & VERBOSE) && (VerboseFormat !='1'))
    printf ("Rendering...\n");
  else if ((Options & VERBOSE) && (VerboseFormat=='1'))
    fprintf (stderr,"POV-Ray rendering %s to %s :\n",Input_File_Name,Output_File_Name);
  if (Options & VERBOSE_FILE)
    {
    stat_file = fopen(Stat_File_Name,"w+t");
    fprintf (stat_file,"Parsed ok. Now rendering %s to %s :\n",Input_File_Name,Output_File_Name);
    fclose(stat_file);
    }
  CONFIG_MATH               /* Macro for setting up any special FP options */
  Start_Tracing ();

  if (Options & VERBOSE && VerboseFormat=='1')
    fprintf (stderr,"\n");

  /* Record the time so well spent... */
  STOP_TIME                  /* Get trace done time. */
  tused = TIME_ELAPSED       /* Calc. elapsed time. Define TIME_ELAPSED as */
  /* 0 in your specific CONFIG.H if unsupported */

  /* Clean up and leave */
  display_finished();

  close_all ();

  PRINT_STATS

  if (Options & VERBOSE_FILE)
    {
    stat_file = fopen(Stat_File_Name,"a+t");
    fprintf (stat_file,"Done Tracing\n");
    fclose(stat_file);
    }

  FINISH_POVRAY
  }

/* Print out usage error message */

void usage ()
  {
  fprintf (stdout,"\nUsage: POVRAY  [+/-] Option1 [+/-] Option2 ...");
  fprintf (stdout,"\n  Example: +L\\povray\\include +Iscene.pov +Oscene.tga +W320 +H200");
  fprintf (stdout,"\n  Example: +Iscene.pov +Oscene.tga +W160 +H200 +V -D +X ");
  fprintf (stdout,"\n");
  fprintf (stdout,"\n[ Paused for keypress... ]\n");
  WAIT_FOR_KEYPRESS;
  fprintf (stdout,"\n\n\n\n\n Options:");
  fprintf (stdout,"\n    Dxy = display in format x, using palette option y");
  fprintf (stdout,"\n    V   = verbose messages on");
  fprintf (stdout,"\n    P  = pause before exit");
  fprintf (stdout,"\n    X  = enable early exit by key hit");
  fprintf (stdout,"\n    Fx = write output file in format x");
  fprintf (stdout,"\n         FT - Uncompressed Targa-24  |  FD - DKB/QRT Dump  | FR - RGB Raw Files");
  fprintf (stdout,"\n    C  = continue aborted trace");
  fprintf (stdout,"\n    Qx = image quality 0=rough, 9=full");
  fprintf (stdout,"\n    A0.x = perform antialiasing");
  fprintf (stdout,"\n    Bxxx = Use xxx KB for output file buffer");
  fprintf (stdout,"\n    Jx.x = set aa-jitter amount");
  fprintf (stdout,"\n    Kx.x = set frame clocK to x.x");
  fprintf (stdout,"\n    MBxxx = use slabs if more than xxx objects");
  fprintf (stdout,"\n    MSxxx = set max symbol table size to xxx");
  fprintf (stdout,"\n    MVx.x = set compability to version x.x");
  fprintf (stdout,"\n    Rn   = set aa-depth (use n X n rays/pixel)");
  fprintf (stdout,"\n    SRxx = start at row xxx         |  SR0.xx start row at x%% of screen");
  fprintf (stdout,"\n    ERxx = end   at row xxx         |  ER0.xx end   row at x%% of screen");
  fprintf (stdout,"\n    SCxx = start at col xxx         |  SC0.xx start col at x%% of screen");
  fprintf (stdout,"\n    ECxx = end   at col xxx         |  EC0.xx end   col at x%% of screen");
  fprintf (stdout,"\n    I<filename> = input file name   |  O<filename> = output file name");
  fprintf (stdout,"\n    L<pathname> = library path prefix");
  fprintf (stdout,"\n");
  exit(1);
  }
void init_vars()
  {
  Output_File_Handle = NULL;
  File_Buffer_Size = 0;
  Options = JITTER;
  Quality_Flags = QUALITY_9;
  Quality = 9;
  Use_Slabs=TRUE;
  Number_Of_Files = 0;
  Frame.Screen_Height = 100;
  Frame.Screen_Width = 100;
  First_Line = 0;
  Last_Line = -1;
  First_Column = 0;
  Last_Column = -1;

  First_Line_Temp = (DBL) First_Line;
  Last_Line_Temp = (DBL) Last_Line;
  First_Column_Temp = (DBL) First_Column ;
  Last_Column_Temp = (DBL) Last_Column;

  Color_Bits = 8;

  Number_Of_Pixels = 0L;
  Number_Of_Rays = 0L;
  Number_Of_Pixels_Supersampled = 0L;
  Ray_Ht_Field_Tests = 0L;
  Ray_Ht_Field_Tests_Succeeded = 0L;
  Ray_Ht_Field_Box_Tests = 0L;
  Ray_HField_Box_Tests_Succeeded = 0L;
  Ray_Bicubic_Tests = 0L;
  Ray_Bicubic_Tests_Succeeded = 0L;
  Ray_Blob_Tests = 0L;
  Ray_Blob_Tests_Succeeded = 0L;
  Ray_Box_Tests = 0L;
  Ray_Box_Tests_Succeeded = 0L;
  Ray_Disc_Tests = 0L;
  Ray_Disc_Tests_Succeeded = 0L;
  Ray_Cone_Tests = 0L;
  Ray_Cone_Tests_Succeeded = 0L;
  Ray_Sphere_Tests = 0L;
  Ray_Sphere_Tests_Succeeded = 0L;
  Ray_Plane_Tests = 0L;
  Ray_Plane_Tests_Succeeded = 0L;
  Ray_Triangle_Tests = 0L;
  Ray_Triangle_Tests_Succeeded = 0L;
  Ray_Quadric_Tests = 0L;
  Ray_Quadric_Tests_Succeeded = 0L;
  Ray_Poly_Tests = 0L;
  Ray_Poly_Tests_Succeeded = 0L;
  Bounding_Region_Tests = 0L;
  Bounding_Region_Tests_Succeeded = 0L;
  Clipping_Region_Tests = 0L;
  Clipping_Region_Tests_Succeeded = 0L;
  Calls_To_Noise = 0L;
  Calls_To_DNoise = 0L;
  Shadow_Ray_Tests = 0L;
  /* SJA */
  Shadow_Cache_Hits = 0L;
  /* SJA */
  Shadow_Rays_Succeeded = 0L;
  Reflected_Rays_Traced = 0L;
  Refracted_Rays_Traced = 0L;
  Transmitted_Rays_Traced = 0L;
  Istack_overflows = 0L;
  Number_of_istacks = 0;
  free_istack = NULL;
  Max_Intersections = 64; /*128*/
  Antialias_Threshold = 0.3;
  strcpy (Input_File_Name, "object.pov");
  return;
  }

/* Close all the stuff that has been opened. */
void close_all ()
  {
  if ((Options & DISPLAY) && Display_Started)
    display_close();

  if (Output_File_Handle)
    Close_File (Output_File_Handle);
  }

/* Read the default parameters from povray.def */
void get_defaults()
  {
  FILE *defaults_file;
  char Option_String[256], *Option_String_Ptr;
  /* READ_ENV_VAR_? should be defined in config.h */
  /* Only one READ_ENV_VAR_? should ever be defined. */
  /* This allows some machines to read environment variable before */
  /* reading povray.def and others to do it after depending on the */
  /* operating system. IBM-PC is before. Default is after if not */
  /* defined in config.h. CDW 2/92 */
  /* Set Diskwrite as default */
  Options |= DISKWRITE;
  OutputFormat = DEFAULT_OUTPUT_FORMAT;

  READ_ENV_VAR_BEFORE
  if ((defaults_file = Locate_File("/home/yoda/cfu/synray/povray.def", "r")) != NULL) 
    {
    printf("\n Get the Defalut povray.def file ");
    while (fgets(Option_String, 256, defaults_file) != NULL)
      read_options(Option_String);
    fclose (defaults_file);
    }
  READ_ENV_VAR_AFTER
  }

void read_options (Option_Line)
char *Option_Line;
  {
  register int c, String_Index, Option_Started;
  short Option_Line_Index = 0;
  char Option_String[80];

  String_Index = 0;
  Option_Started = FALSE;
  while ((c = Option_Line[Option_Line_Index++]) != '\0')
    {
    if (Option_Started)
      if (isspace(c))
        {
        Option_String[String_Index] = '\0';
        parse_option (Option_String);
        Option_Started = FALSE;
        String_Index = 0;
        }
      else
        Option_String[String_Index++] = (char) c;

    else /* Option_Started */
      if ((c == (int) '-') || (c == (int) '+'))
        {
        String_Index = 0;
        Option_String[String_Index++] = (char) c;
        Option_Started = TRUE;
        }
      else
        if (!isspace(c))
          {
          fprintf (stderr, 
            "\nCommand line or .DEF error. Bad character (%c), val: %d.\n", 
            (char) c, c);
          exit (1);
          }
    }

  if (Option_Started)
    {
    Option_String[String_Index] = '\0';
    parse_option (Option_String);
    }
  }

/* parse the command line parameters */
void parse_option (Option_String)
char *Option_String;
  {
  register int Add_Option;
  unsigned int Option_Number;
  long bounds_thresh;
  DBL threshold;

  inflag = outflag = FALSE;   /* if these flags aren't immediately used, reset them on next -/+ option! */
  if (*(Option_String++) == '-')
    Add_Option = FALSE;
  else
    Add_Option = TRUE;

  Option_Number = 0;

  switch (*Option_String)
  {
  case 'B':
  case 'b':  
    sscanf (&Option_String[1], "%d", &File_Buffer_Size);
    File_Buffer_Size *= 1024;
    if (File_Buffer_Size < BUFSIZ)   /* system default MIN */
      File_Buffer_Size = BUFSIZ;
    if (File_Buffer_Size > MAX_BUFSIZE)    /* unsigned short MAX */
      File_Buffer_Size = MAX_BUFSIZE;
    break;

  case 'C':
  case 'c':  
    Option_Number = CONTINUE_TRACE;
    break;

  case 'D':
  case 'd':  
    Option_Number = DISPLAY;
    DisplayFormat = '0';
    PaletteOption = '3';
    if (Option_String[1] != '\0')
      DisplayFormat = (char)toupper(Option_String[1]);

    if (Option_String[1] != '\0' && Option_String[2] != '\0')
      PaletteOption = (char)toupper(Option_String[2]);
    break;

  case '@':  
    Option_Number = VERBOSE_FILE;
    if(Option_String[1] == '\0')
      strcpy(Stat_File_Name, "POVSTAT.OUT");
    else
      strncpy (Stat_File_Name, &Option_String[1], FILE_NAME_LENGTH);
    break;

  case 'V':
  case 'v':  
    Option_Number = VERBOSE;
    VerboseFormat = (char)toupper(Option_String[1]);
    if (VerboseFormat == '\0')
      VerboseFormat = '1';
    break;

  case 'W':
  case 'w':  
    sscanf (&Option_String[1], "%d", &Frame.Screen_Width);
    break;

  case 'H':
  case 'h':  
    sscanf (&Option_String[1], "%d", &Frame.Screen_Height);
    break;

  case 'F':
  case 'f':  
    Option_Number = DISKWRITE;
    if (isupper(Option_String[1]))
      OutputFormat = (char)tolower(Option_String[1]);
    else
      OutputFormat = Option_String[1];

    /* Default the output format to the default in the config file */
    if (OutputFormat == '\0')
      OutputFormat = DEFAULT_OUTPUT_FORMAT;
    break;

  case 'P':
  case 'p':  
    Option_Number = PROMPTEXIT;
    break;

  case 'I':
  case 'i':  
    if (Option_String[1] == '\0')
      inflag = TRUE;
    else 
      strncpy (Input_File_Name, &Option_String[1], FILE_NAME_LENGTH);
    break;

  case 'O':
  case 'o':  
    if (Option_String[1] == '\0')
      outflag = TRUE;
    else
      strncpy (Output_File_Name, &Option_String[1], FILE_NAME_LENGTH);
    break;

  case 'A':
  case 'a':  
    Option_Number = ANTIALIAS;
    if (sscanf (&Option_String[1], DBL_FORMAT_STRING, &threshold) != EOF)
      Antialias_Threshold = threshold;
    break;

  case 'J':
  case 'j':
    Option_Number = JITTER;
    if (sscanf (&Option_String[1], DBL_FORMAT_STRING, &threshold) != EOF)
       JitterScale = threshold;
    if (JitterScale<=0.0)
       Add_Option = FALSE;
    break; 

  case 'R':
  case 'r':  
    sscanf (&Option_String[1], "%ld", &AntialiasDepth);
    if (AntialiasDepth < 1)
      AntialiasDepth = 1;
    if (AntialiasDepth > 9)
      AntialiasDepth = 9;
    break;

  case 'X':
  case 'x':  
    Option_Number = EXITENABLE;
    break;

  case 'L':
  case 'l':  
    if (Library_Path_Index >= MAX_LIBRARIES) 
      {
      fprintf (stderr, "Too many library directories specified\n");
      exit(1);
      }
    Library_Paths [Library_Path_Index] = malloc (strlen(Option_String));
    if (Library_Paths [Library_Path_Index] == NULL) 
      {
      fprintf (stderr, "Out of memory. Cannot allocate memory for library pathname\n");
      exit(1);
      }
    strcpy (Library_Paths [Library_Path_Index], &Option_String[1]);
    Library_Path_Index++;
    break;

  case 'T':
  case 't':  
    switch (toupper(Option_String[1]))
    {
    case 'Y':
      Case_Sensitive_Flag = 0;
      break;
    case 'N':
      Case_Sensitive_Flag = 1;
      break;
    case 'O':
      Case_Sensitive_Flag = 2;
      break;
    default:
      Case_Sensitive_Flag = 2;
      break;
    }  
    break;

  case 'S':
  case 's':  
    switch (Option_String[1])
    {
    case 'c': 
    case 'C':
      sscanf (&Option_String[2], DBL_FORMAT_STRING, &First_Column_Temp);
      break;

    case 'r': 
    case 'R':
      sscanf (&Option_String[2], DBL_FORMAT_STRING, &First_Line_Temp);
      break;

    default:
      sscanf (&Option_String[1], DBL_FORMAT_STRING, &First_Line_Temp);
      break;
    }

    if(First_Column_Temp > 0.0 && First_Column_Temp < 1.0)
      First_Column = (int) (Frame.Screen_Width * First_Column_Temp);
    else
      First_Column = (int) First_Column_Temp;

    if(First_Line_Temp > 0.0 && First_Line_Temp < 1.0)
      First_Line = (int) (Frame.Screen_Height * First_Line_Temp);
    else
      First_Line = (int) First_Line_Temp;

    if (First_Column < 0)
      First_Column = 0;

    if (First_Line < 0)
      First_Line = 0;

    break;

  case 'E':
  case 'e': 
    switch (Option_String[1])
    {
    case 'c': 
    case 'C':
      sscanf (&Option_String[2], DBL_FORMAT_STRING, &Last_Column_Temp);
      break;

    case 'r': 
    case 'R':
      sscanf (&Option_String[2], DBL_FORMAT_STRING, &Last_Line_Temp);
      break;

    default:
      sscanf (&Option_String[1], DBL_FORMAT_STRING, &Last_Line_Temp);
      break;
    }

    if(Last_Column_Temp > 0.0 && Last_Column_Temp < 1.0)
      Last_Column = (int) (Frame.Screen_Width * Last_Column_Temp);
    else
      Last_Column = (int) Last_Column_Temp;

    if(Last_Line_Temp > 0.0 && Last_Line_Temp < 1.0)
      Last_Line = (int) (Frame.Screen_Height * Last_Line_Temp);
    else
      Last_Line = (int) Last_Line_Temp;

    if (Last_Column < 0 || Last_Column >= Frame.Screen_Width)
      Last_Column = Frame.Screen_Width - 1;

    if (Last_Line > Frame.Screen_Height)
      Last_Line = Frame.Screen_Height;

    break;

  case 'M': /* Switch used so other max values can be inserted easily */
  case 'm':  
    switch (Option_String[1])
    {
    case 's': /* Max Symbols */
    case 'S':
      sscanf (&Option_String[2], "%d", &Max_Symbols);
      break;
    case 'v': /* Max Version */
    case 'V':
      sscanf (&Option_String[2], DBL_FORMAT_STRING, &Language_Version);
      break;
    case 'b': /* Min Bounded */
    case 'B':
      if (sscanf (&Option_String[2], "%d", &bounds_thresh) != EOF)
        Bounds_Threshold=bounds_thresh;
      Use_Slabs = Add_Option;
      break;
    default:
      break;
    }
    break;

  case 'Q':
  case 'q':
    sscanf (&Option_String[1], "%d", &Quality);
    if ((Quality < 0) || (Quality > 9))
      Error("Illegal +Q switch setting");
    Quality_Flags = Quality_Values[Quality];
    break;

    /* Turn on debugging print statements. */
  case 'Z':
  case 'z':  
    Option_Number = DEBUGGING;
    break;

    /* +Y switch to remain undocumented.  Add to +Q later */
  case 'Y':
  case 'y':  
    Use_Slabs = Add_Option;
    break;

  case 'K':
  case 'k': 
    sscanf (&Option_String[1], DBL_FORMAT_STRING, &Clock_Value);
    break;

  default:   
    fprintf (stderr, "\nInvalid option: %s\n\n", --Option_String);
  }

  if (Option_Number != 0)
    if (Add_Option)
      Options |= Option_Number;
    else Options &= ~Option_Number;
  }

  void Print_Options()
    {
    int i;

    fprintf (stdout,"\nPOV-Ray Options in effect: ");

    if (Options & CONTINUE_TRACE)
      fprintf (stdout,"+c ");

    if (Options & DISPLAY)
      fprintf (stdout,"+d%c%c ", DisplayFormat, PaletteOption);

    if (Options & VERBOSE)
      fprintf (stdout,"+v%c ", VerboseFormat);

    if (Options & VERBOSE_FILE)
      fprintf (stdout,"+@%s ", Stat_File_Name);

    if (Options & DISKWRITE)
      fprintf (stdout,"+f%c ", OutputFormat);

    if (Options & PROMPTEXIT)
      fprintf (stdout,"+p ");

    if (Options & EXITENABLE)
      fprintf (stdout,"+x ");

    if (Use_Slabs)
      fprintf (stdout,"+mb%d ", Bounds_Threshold);
    else
      fprintf (stdout,"-mb ");

    if (Options & DEBUGGING)
      fprintf (stdout,"+z ");

    if (File_Buffer_Size != 0)
      fprintf (stdout,"-b%d ", File_Buffer_Size/1024);

    if (Options & ANTIALIAS)
      {
      fprintf (stdout,"+a%.3f ", Antialias_Threshold);
      if (Options & JITTER)
        fprintf (stdout,"+j%.3f ", JitterScale);
      fprintf (stdout,"+r%ld ",AntialiasDepth);
      }

    /* quality flags rewrite in progress by CEY */

    fprintf (stdout,"-q%d -w%d -h%d -s%d -e%d\n",Quality,
      Frame.Screen_Width, Frame.Screen_Height, First_Line, Last_Line);

    fprintf (stdout, "-k%.3f -mv%.1f -i%s ", Clock_Value, Language_Version, 
      Input_File_Name);

    if (Options & DISKWRITE)
      fprintf (stdout,"-o%s ", Output_File_Name);

    for (i = 0 ; i < Library_Path_Index ; i++)
      fprintf (stdout,"-l%s ", Library_Paths[i]);

    fprintf (stdout,"\n");
    }

void parse_file_name (File_Name)
char *File_Name;
  {
  FILE *defaults_file;
  char Option_String[256];

  if (inflag)   /* file names may now be separated by spaces from cmdline option */
    {
    strncpy (Input_File_Name, File_Name, FILE_NAME_LENGTH);
    inflag = FALSE;
    return;
    }

  if (outflag)  /* file names may now be separated by spaces from cmdline option */
    {
    strncpy (Output_File_Name, File_Name, FILE_NAME_LENGTH);
    outflag = FALSE;
    return;
    }


  if (++Number_Of_Files > MAX_FILE_NAMES)
    {
    fprintf (stderr, "\nOnly %d option file names are allowed in a command line.", 
      MAX_FILE_NAMES);
    exit(1);
    }

  if ((defaults_file = Locate_File(File_Name, "r")) != NULL) 
    {
    while (fgets(Option_String, 256, defaults_file) != NULL)
      read_options(Option_String);
    fclose (defaults_file);
    }
  else
    printf("\nError opening option file %s.",File_Name);  
  }

void print_stats()
  {
  long hours,min;
  DBL sec;
  FILE *stat_out;
  long Pixels_In_Image;

  if (Options & VERBOSE_FILE)
    stat_out = fopen(Stat_File_Name,"w+t");
  else
    stat_out = stdout;  

  Pixels_In_Image = (long)Frame.Screen_Width * (long)Frame.Screen_Height;


  fprintf (stat_out,"\n%s statistics\n",Input_File_Name);
  if(Pixels_In_Image > Number_Of_Pixels)
    fprintf (stat_out,"  Partial Image Rendered");

  fprintf (stat_out,"--------------------------------------\n");
  fprintf (stat_out,"Resolution %d x %d\n",Frame.Screen_Width, Frame.Screen_Height);
  fprintf (stat_out,"# Rays:  %10ld    # Pixels:  %10ld  # Pixels supersampled: %10ld\n",
    Number_Of_Rays, Number_Of_Pixels, Number_Of_Pixels_Supersampled);

  fprintf (stat_out,"  Ray->Shape Intersection Tests:\n");
  fprintf (stat_out,"   Type             Tests    Succeeded   Percentage\n");
  fprintf (stat_out,"  -----------------------------------------------------------\n");
  if(Ray_Sphere_Tests)
    fprintf (stat_out,"  Sphere       %10ld  %10ld  %10.2f\n", Ray_Sphere_Tests, Ray_Sphere_Tests_Succeeded, ( ((DBL)Ray_Sphere_Tests_Succeeded/(DBL)Ray_Sphere_Tests) *100.0 ) );
  if(Ray_Plane_Tests)
    fprintf (stat_out,"  Plane        %10ld  %10ld  %10.2f\n", Ray_Plane_Tests, Ray_Plane_Tests_Succeeded, ( ((DBL)Ray_Plane_Tests_Succeeded/(DBL)Ray_Plane_Tests) *100.0 ));
  if(Ray_Triangle_Tests)
    fprintf (stat_out,"  Triangle     %10ld  %10ld  %10.2f\n", Ray_Triangle_Tests, Ray_Triangle_Tests_Succeeded, ( ((DBL)Ray_Triangle_Tests_Succeeded/(DBL)Ray_Triangle_Tests) *100.0 ));
  if(Ray_Quadric_Tests)
    fprintf (stat_out,"  Quadric      %10ld  %10ld  %10.2f\n", Ray_Quadric_Tests, Ray_Quadric_Tests_Succeeded, ( ((DBL)Ray_Quadric_Tests_Succeeded/(DBL)Ray_Quadric_Tests) *100.0 ));
  if(Ray_Blob_Tests)
    fprintf (stat_out,"  Blob         %10ld  %10ld  %10.2f\n", Ray_Blob_Tests, Ray_Blob_Tests_Succeeded, ( ((DBL)Ray_Blob_Tests_Succeeded/(DBL)Ray_Blob_Tests) *100.0 ));
  if(Ray_Box_Tests)
    fprintf (stat_out,"  Box          %10ld  %10ld  %10.2f\n", Ray_Box_Tests, Ray_Box_Tests_Succeeded, ( ((DBL)Ray_Box_Tests_Succeeded/(DBL)Ray_Box_Tests) *100.0 ));
  if(Ray_Cone_Tests)
    fprintf (stat_out,"  Cone         %10ld  %10ld  %10.2f\n", Ray_Cone_Tests, Ray_Cone_Tests_Succeeded, ( ((DBL)Ray_Cone_Tests_Succeeded/(DBL)Ray_Cone_Tests) *100.0 ) );
  if(Ray_Disc_Tests)
    fprintf (stat_out,"  Disc         %10ld  %10ld  %10.2f\n", Ray_Disc_Tests, Ray_Disc_Tests_Succeeded, ( ((DBL)Ray_Disc_Tests_Succeeded/(DBL)Ray_Disc_Tests) *100.0 ) );
  if(Ray_Poly_Tests)
    fprintf (stat_out,"  Quartic\\Poly %10ld  %10ld  %10.2f\n", Ray_Poly_Tests, Ray_Poly_Tests_Succeeded, ( ((DBL)Ray_Poly_Tests_Succeeded/(DBL)Ray_Poly_Tests) *100.0 ));
  if(Ray_Bicubic_Tests)
    fprintf (stat_out,"  Bezier Patch %10ld  %10ld  %10.2f\n", Ray_Bicubic_Tests, Ray_Bicubic_Tests_Succeeded, ( ((DBL)Ray_Bicubic_Tests_Succeeded/(DBL)Ray_Bicubic_Tests) *100.0 ));
  if(Ray_Ht_Field_Tests)
    fprintf (stat_out,"  Height Fld   %10ld  %10ld  %10.2f\n", Ray_Ht_Field_Tests, Ray_Ht_Field_Tests_Succeeded, ( ((DBL)Ray_Ht_Field_Tests_Succeeded/(DBL)Ray_Ht_Field_Tests) *100.0 ));
  if(Ray_Ht_Field_Box_Tests)
    fprintf (stat_out,"  Hght Fld Box %10ld  %10ld  %10.2f\n", Ray_Ht_Field_Box_Tests, Ray_HField_Box_Tests_Succeeded, ( ((DBL)Ray_HField_Box_Tests_Succeeded/(DBL)Ray_Ht_Field_Box_Tests) *100.0 ));
  if(Bounding_Region_Tests)
    fprintf (stat_out,"  Bounds       %10ld  %10ld  %10.2f\n", Bounding_Region_Tests, Bounding_Region_Tests_Succeeded, ( ((DBL)Bounding_Region_Tests_Succeeded/(DBL)Bounding_Region_Tests) *100.0 ));
  if(Clipping_Region_Tests)
    fprintf (stat_out,"  Clips        %10ld  %10ld  %10.2f\n", Clipping_Region_Tests, Clipping_Region_Tests_Succeeded, ( ((DBL)Clipping_Region_Tests_Succeeded/(DBL)Clipping_Region_Tests) *100.0 ));

  if(Calls_To_Noise) 

    fprintf (stat_out,"  Calls to Noise:   %10ld\n", Calls_To_Noise);
  if(Calls_To_DNoise)
    fprintf (stat_out,"  Calls to DNoise:  %10ld\n", Calls_To_DNoise);
  if(Shadow_Ray_Tests)
    fprintf (stat_out,"  Shadow Ray Tests: %10ld     Blocking Objects Found:  %10ld\n",
      Shadow_Ray_Tests, Shadow_Rays_Succeeded);
  if(Reflected_Rays_Traced)
    fprintf (stat_out,"  Reflected Rays:   %10ld\n", Reflected_Rays_Traced);
  if(Refracted_Rays_Traced)
    fprintf (stat_out,"  Refracted Rays:   %10ld\n", Refracted_Rays_Traced);
  if(Transmitted_Rays_Traced)
    fprintf (stat_out,"  Transmitted Rays: %10ld\n", Transmitted_Rays_Traced);
  if(Istack_overflows)
    fprintf (stat_out,"  I-Stack overflows:%10ld\n", Istack_overflows);

  if(tused==0) 
    {
    STOP_TIME                  /* Get trace done time. */
    tused = TIME_ELAPSED       /* Calc. elapsed time. Define TIME_ELAPSED as */
    /* 0 in your specific CONFIG.H if unsupported */
    }
  if (tused != 0)
    {
    /* Convert seconds to hours, min & sec. CdW */
    hours = (long) tused/3600;
    min = (long) (tused - hours*3600)/60;
    sec = tused - (DBL) (hours*3600 + min*60);
    fprintf (stat_out,"  Time For Trace:   %2ld hours %2ld minutes %4.2f seconds\n", hours,min,sec); 
    }
  if (Options & VERBOSE_FILE)
    fclose(stat_out);

  }

/* Find a file in the search path. */

FILE *Locate_File (filename, mode)
char *filename, *mode;
  {
  FILE *f;
  int i;
  char pathname[FILE_NAME_LENGTH];

  /* Check the current directory first. */
  if ((f = fopen (filename, mode)) != NULL)
    return (f);

  for (i = 0 ; i < Library_Path_Index ; i++) 
    {
    strcpy (pathname, Library_Paths[i]);
    if (FILENAME_SEPARATOR != NULL)
      strcat (pathname, FILENAME_SEPARATOR);
    strcat (pathname, filename);
    if ((f = fopen (pathname, mode)) != NULL)
      return (f);
    }

  return (NULL);
  }
void print_credits()
  {
  fprintf (stderr,"\n");
  fprintf (stderr,"  Persistence of Vision Raytracer Ver %s%s\n",POV_RAY_VERSION,COMPILER_VER);
  fprintf (stderr,"    %s\n",DISTRIBUTION_MESSAGE_1);
  fprintf (stderr,"     %s\n",DISTRIBUTION_MESSAGE_2);
  fprintf (stderr,"     %s\n",DISTRIBUTION_MESSAGE_3);
  fprintf (stderr,"  Copyright 1993 POV-Team\n");
  fprintf (stderr,"  ----------------------------------------------------------------------\n");
  fprintf (stderr,"  POV-Ray is based on DKBTrace 2.12 by David K. Buck & Aaron A. Collins.\n");
  fprintf (stderr,"    Contributing Authors: (Alphabetically)\n");
  fprintf (stderr,"       Steve Anger        Steve A. Bennett   David K. Buck\n");
  fprintf (stderr,"       Aaron A. Collins   Alexander Enzmann  Dan Farmer\n");
  fprintf (stderr,"       Douglas Muir       Bill Pulver        Robert Skinner\n");
  fprintf (stderr,"       Scott Taylor       Drew Wells         Chris Young\n");
  fprintf (stderr,"    Other contributors listed in the documentation.\n");
  fprintf (stderr,"  ----------------------------------------------------------------------\n");
  }
