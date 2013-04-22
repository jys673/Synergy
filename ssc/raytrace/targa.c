/****************************************************************************
*                targa.c
*
*  This module contains the code to read and write the Targa output file
*  format.
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
#include "povproto.h"

static int Targa_Line_Number;
static unsigned char idbuf[256];

extern int First_Line;
static void convert_targa_color PARAMS((IMAGE_COLOUR *, int, unsigned char *));

FILE_HANDLE *Get_Targa_File_Handle()
  {
  FILE_HANDLE *handle;

  if ((handle = (FILE_HANDLE *) malloc(sizeof(FILE_HANDLE))) == NULL) 
    {
    fprintf (stderr, "Cannot allocate memory for output file handle\n");
    return(NULL);
    }

  handle->Default_File_Name_p = Default_Targa_File_Name;
  handle->Open_File_p = Open_Targa_File;
  handle->Write_Line_p = Write_Targa_Line;
  handle->Read_Line_p = Read_Targa_Line;
  handle->Read_Image_p = Read_Targa_Image;
  handle->Close_File_p = Close_Targa_File;
  handle->file = NULL;
  handle->buffer_size = 0;
  handle->buffer = NULL;
  return (handle);
  }

char *Default_Targa_File_Name()
  {
  return ("data.tga");
  }

int Open_Targa_File (handle, name, width, height, buffer_size, mode)
FILE_HANDLE *handle;
char *name;
int *width;
int *height;
int buffer_size;
int mode;
  {
  int data1, data2, i;

  handle->mode = mode;
  handle->filename = name;
  Targa_Line_Number = 0;

  switch (mode) 
  {
  case READ_MODE:
    if ((handle->file = fopen (name, READ_FILE_STRING)) == NULL)
      return(0);

    if (buffer_size != 0) 
      {
      if ((handle->buffer = malloc (buffer_size)) == NULL)
        return(0);

      setvbuf (handle->file, handle->buffer, _IOFBF, buffer_size);
      }

    for (i = 0 ; i < 12 ; i++)
      if (getc(handle->file) == EOF)
        return(0);

    if (((data1 = getc(handle->file)) == EOF)
      || ((data2 = getc(handle->file)) == EOF))
      return(0);

    *width  = data2 * 256 + data1;

    if (((data1 = getc(handle->file)) == EOF)
      || ((data2 = getc(handle->file)) == EOF))
      return(0);

    for (i = 0 ; i < 2 ; i++)
      if (getc(handle->file) == EOF)
        return(0);

    *height = data2 * 256 + data1;
    handle->width = *width;
    handle->height = *height;
    handle->buffer_size = buffer_size;
    break;

  case WRITE_MODE:
    if ((handle->file = fopen (name, WRITE_FILE_STRING)) == NULL)
      return(0);

    if (buffer_size != 0) 
      {
      if ((handle->buffer = malloc (buffer_size)) == NULL)
        return(0);

      setvbuf (handle->file, handle->buffer, _IOFBF, buffer_size);
      }

    for (i = 0; i < 10; i++)	/* 00, 00, 02, then 7 00's... */
      if (i == 2)
        putc(i, handle->file);
      else
        putc(0, handle->file);

    putc(First_Line % 256, handle->file); /* y origin set to "First_Line" */
    putc(First_Line / 256, handle->file);

    putc(*width % 256, handle->file);  /* write width and height */
    putc(*width / 256, handle->file);
    putc(*height % 256, handle->file);
    putc(*height / 256, handle->file);
    putc(24, handle->file);  /* 24 bits/pixel (16 million colors!) */
    putc(32, handle->file);  /* Bitmask, pertinent bit: top-down raster */

    handle->width = *width;
    handle->height = *height;
    handle->buffer_size = buffer_size;

    break;

  case APPEND_MODE:
    if ((handle->file = fopen (name, APPEND_FILE_STRING)) == NULL)
      return(0);

    if (buffer_size != 0) 
      {
      if ((handle->buffer = malloc (buffer_size)) == NULL)
        return(0);

      setvbuf (handle->file, handle->buffer, _IOFBF, buffer_size);
      }

    break;
  }
  return(1);
  }

void Write_Targa_Line (handle, line_data, line_number)
FILE_HANDLE *handle;
COLOUR *line_data;
int line_number;
  {
  register int x;

  for (x = 0; x < handle->width; x++) 
    {
    putc((int) floor (line_data[x].Blue * 255.0), handle->file);
    putc((int) floor (line_data[x].Green * 255.0), handle->file);
    putc((int) floor (line_data[x].Red * 255.0), handle->file);
    }

  if (handle->buffer_size == 0) 
    {
    fflush(handle->file);                       /* close and reopen file for */
    handle->file = freopen(handle->filename, APPEND_FILE_STRING,
      handle->file);                /* integrity in case we crash*/
    }
  }

int Read_Targa_Line (handle, line_data, line_number)
FILE_HANDLE *handle;
COLOUR *line_data;
int *line_number;
  {
  int x, data;

  for (x = 0; x < handle->width; x++) 
    {

    /* Read the BLUE data byte.  If EOF is reached on the first character read,
      then this line hasn't been rendered yet.  Return 0.  If an EOF occurs
      somewhere within the line, this is an error - return -1. */

    if ((data = getc(handle->file)) == EOF)
      if (x == 0)
        return (0);
      else
        return (-1);

    line_data[x].Blue = (DBL) data / 255.0;

    /* Read the GREEN data byte. */
    if ((data = getc(handle->file)) == EOF)
      return (-1);
    line_data[x].Green = (DBL) data / 255.0;


    /* Read the RED data byte. */
    if ((data = getc(handle->file)) == EOF)
      return (-1);
    line_data[x].Red = (DBL) data / 255.0;
    }

  *line_number = Targa_Line_Number++;
  return(1);
  }

void Close_Targa_File (handle)
FILE_HANDLE *handle;
  {
  if(handle->file)
    fclose (handle->file);
  if (handle->buffer != NULL)
    free (handle->buffer);
  }

static void
convert_targa_color(tcolor, pixelsize, bytes)
IMAGE_COLOUR *tcolor;
int pixelsize;
unsigned char *bytes;
  {
  switch (pixelsize) 
  {
  case 1:
    tcolor->Red   = bytes[0];
    tcolor->Green = bytes[0];
    tcolor->Blue  = bytes[0];
    tcolor->Filter = 0;
    break;
  case 2:
    tcolor->Red   = ((bytes[1] & 0x7c) << 1);
    tcolor->Green = (((bytes[1] & 0x03) << 3) |
      ((bytes[0] & 0xe0) >> 5)) << 3;
    tcolor->Blue  = (bytes[0] & 0x1f) << 3;
    tcolor->Filter = (bytes[1] & 0x80 ? 255 : 0);
    break;
  case 3:
    tcolor->Red   = bytes[2];
    tcolor->Green = bytes[1];
    tcolor->Blue  = bytes[0];
    tcolor->Filter = 0;
    break;
  case 4:
    tcolor->Red   = bytes[2];
    tcolor->Green = bytes[1];
    tcolor->Blue  = bytes[0];
    tcolor->Filter = bytes[3];
    break;
  default:
    fprintf(stderr, "Bad pixelsize in Targa color\n");
    close_all();
    exit(1);
  }
  }

/* Reads a Targa image into an RGB image buffer.  Handles 8, 16, 24, 32 bit
   formats.  Raw or color mapped. Simple raster and RLE compressed pixel
   encoding. Right side up or upside down orientations. */
void
Read_Targa_Image(Image, name)
IMAGE *Image;
char *name;
  {
  FILE *filep;
  IMAGE_LINE *line_data;
  IMAGE_COLOUR *cmap, pixel;
  int h;
  unsigned i, j, k;
  int temp;
  unsigned char cflag, *map_line, bytes[4], tgaheader[18];
  unsigned ftype, idlen, cmlen, cmsiz, psize, orien;
  unsigned width, height;

  /* Start by trying to open the file */
  if ((filep = Locate_File(name, READ_FILE_STRING)) == NULL) 
    {
    fprintf (stderr, "Cannot open Targa file %s\n", name);
    close_all();
    exit(1);
    }
  if (fread(tgaheader, 18, 1, filep) != 1) 
    {
    fprintf(stderr, "Error reading header of Targa image: %s\n", name);
    close_all();
    exit(1);
    }

  /* Decipher the header information */
  idlen  = tgaheader[ 0];
  ftype  = tgaheader[ 2];
  cmlen  = tgaheader[ 5] + (tgaheader[ 6] << 8);
  cmsiz  = tgaheader[ 7] / 8;
  width  = tgaheader[12] + (tgaheader[13] << 8);
  height = tgaheader[14] + (tgaheader[15] << 8);
  psize  = tgaheader[16] / 8;
  orien  = tgaheader[17] & 0x20; /* Right side up ? */

  Image->iwidth  = width;
  Image->iheight = height;
  Image->width   = (DBL)width;
  Image->height  = (DBL)height;
  Image->Colour_Map_Size = cmlen;
  Image->Colour_Map = NULL;

  /* Determine if this is a supported Targa type */
  if (ftype == 9 || ftype == 10)
    cflag = 1;
  else if (ftype == 1 || ftype == 2 || ftype == 3)
    cflag = 0;
  else 
    {
    fprintf(stderr, "Image file %s is an unsupported Targa type: %d\n",
      name, ftype);
    close_all();
    exit(1);
    }

  /* Skip over the picture ID information */
  if (idlen > 0 && fread(idbuf, idlen, 1, filep) != 1) 
    {
    fprintf(stderr, "reading identification field of %s\n", name);
    close_all();
    exit(1);
    }

  /* Read in the the color map (if any) */
  if (cmlen > 0) 
    {
    if (psize != 1) 
      {
      fprintf(stderr, "Can't support %d bits in a color map index\n",
        psize * 8);
      close_all();
      exit(1);
      }
    if ((cmap = (IMAGE_COLOUR *)malloc(cmlen * sizeof(IMAGE_COLOUR))) == NULL) 
      {
      fprintf(stderr, "Failed to allocate memory for image: %s\n", name);
      close_all();
      exit(1);
      }
    for (i=0;i<cmlen;i++) 
      {
      for (j=0;j<cmsiz;j++)
        if ((temp = fgetc(filep)) == EOF) 
        {
        fprintf(stderr, "Premature EOF in image file: %s\n", name);
        close_all();
        exit(1);
        }
        else
          bytes[j] = (unsigned char)temp;
      convert_targa_color(&cmap[i], cmsiz, bytes);
      }
    Image->Colour_Map = cmap;
    }
  else
    Image->Colour_Map = NULL;

  /* Allocate the buffer for the image */
  if (cmlen > 0) 
    {
    if ((Image->data.map_lines = (unsigned char **)
      malloc(height * sizeof(unsigned char *)))==NULL) 
      {
      fprintf (stderr, "Cannot allocate memory for image: %s\n", name);
      close_all(); 
      exit(1);
      }
    }
  else if ((Image->data.rgb_lines = (struct Image_Line *)
    malloc(height * sizeof(struct Image_Line))) == NULL) 
    {
    fprintf (stderr, "Cannot allocate memory for image: %s\n", name);
    close_all(); 
    exit(1);
    }
  for (i=0;i<height;i++) 
    {
    if (cmlen > 0) 
      {
      k = width * sizeof(unsigned char);
      map_line = (unsigned char *)malloc(k);
      if (map_line == NULL) 
        {
        fprintf(stderr, "Cannot allocate memory for image: %s\n", name);
        close_all();
        exit(1);
        }
      Image->data.map_lines[i] = map_line;
      }
    else 
      {
      line_data = &Image->data.rgb_lines[i];
      k = width * sizeof(unsigned char);
      line_data->red   = (unsigned char *)malloc(k);
      line_data->green = (unsigned char *)malloc(k);
      line_data->blue  = (unsigned char *)malloc(k);
      if (line_data->red == NULL || line_data->green == NULL ||
        line_data->blue == NULL) 
        {
        fprintf(stderr, "Cannot allocate memory for image: %s\n", name);
        close_all();
        exit(1);
        }
      }
    }

  /* Read the image into the buffer */
  if (cflag) 
    {
    /* RLE compressed images */
    if (cmlen > 0)
      if (orien)
        map_line = Image->data.map_lines[0];
      else
        map_line = Image->data.map_lines[height-1];
    else
      if (orien)
        line_data = &Image->data.rgb_lines[0];
      else
        line_data = &Image->data.rgb_lines[height-1];
    i = 0; /* row counter */
    j = 0; /* column counter */
    while (i < height) 
      {
      /* Grab a header */
      if ((h = fgetc(filep)) == EOF) 
        {
        fprintf(stderr, "Premature EOF in image file: %s\n", name);
        close_all();
        exit(1);
        }
      if (h & 0x80) 
        {
        /* Repeat buffer */
        h &= 0x7F;
        for (k=0;k<psize;k++)
          if ((temp = fgetc(filep)) == EOF) 
          {
          fprintf(stderr, "Premature EOF in image file: %s\n", name);
          close_all();
          exit(1);
          }
          else
            bytes[k] = (unsigned char)temp;
        if (cmlen == 0)
          convert_targa_color(&pixel, psize, bytes);
        for ( ; h >= 0; h--) 
          {
          if (cmlen > 0)
            map_line[j] = bytes[0];
          else 
            {
            line_data->red[j]   = (unsigned char)pixel.Red;
            line_data->green[j] = (unsigned char)pixel.Green;
            line_data->blue[j]  = (unsigned char)pixel.Blue;
            }
          if (++j == width) 
            {
            i++;
            if (cmlen > 0) 
              {
              if (orien)
                map_line = Image->data.map_lines[i];
              else
                map_line = Image->data.map_lines[height-i-1];
              }
            else
              line_data += (orien ? 1 : -1);
            j = 0;
            }
          }
        }
      else 
        {
        /* Copy buffer */
          for (;h>=0;h--) 
          {
          for (k=0;k<psize;k++)
            if ((temp = fgetc(filep)) == EOF) 
            {
            fprintf(stderr, "Premature EOF in image file: %s\n", name);
            close_all();
            exit(1);
            }
            else
              bytes[k] = (unsigned char)temp;
          if (cmlen > 0)
            map_line[j] = bytes[0];
          else 
            {
            convert_targa_color(&pixel, psize, bytes);
            line_data->red[j]   = (unsigned char)pixel.Red;
            line_data->green[j] = (unsigned char)pixel.Green;
            line_data->blue[j]  = (unsigned char)pixel.Blue;
            }
          if (++j == width) 
            {
            i++;
            if (cmlen > 0) 
              {
              if (orien)
                map_line = Image->data.map_lines[i];
              else
                map_line = Image->data.map_lines[height-i-1];
              }
            else
              line_data += (orien ? 1 : -1);
            j = 0;
            }
          }
        }
      }
    }
  else 
    {
    /* Simple raster image file, read in all of the pixels */
      if (cmlen == 0) 
      {
      if (orien)
        line_data = &Image->data.rgb_lines[0];
      else
        line_data = &Image->data.rgb_lines[height-1];
      }
    for (i=0;i<height;i++) 
      {
      if (cmlen > 0) 
        {
        if (orien)
          map_line = Image->data.map_lines[i];
        else
          map_line = Image->data.map_lines[height-i-1];
        }
      for (j=0;j<width;j++) 
        {
        for (k=0;k<psize;k++)
          if ((temp = fgetc(filep)) == EOF) 
          {
          fprintf(stderr, "Premature EOF in image file: %s\n", name);
          close_all();
          exit(1);
          }
          else
            bytes[k] = (unsigned char)temp;
        if (cmlen > 0)
          map_line[j] = bytes[0];
        else 
          {
          convert_targa_color(&pixel, psize, bytes);
          line_data->red[j]   = (unsigned char)pixel.Red;
          line_data->green[j] = (unsigned char)pixel.Green;
          line_data->blue[j]  = (unsigned char)pixel.Blue;
          }
        }
      if (cmlen == 0)
        line_data += (orien ? 1 : -1);
      }
    }
  /* Any data following the image is ignored. */

  /* Close the image file */
  fclose(filep);
  }
