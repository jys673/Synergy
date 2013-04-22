/*----------------------------------------------------------------------*/
/* Geometry constructing Module                                         */
/*----------------------------------------------------------------------*/
#include "field.h"

void GeomPrint(int il)
{
    int i,j;

    for  (i=0; i<YRES; i++) {
        for (j=0; j<XRES; j++) {
            printf(" %d", geom[il][i][j]);
        }
        printf("\n");
    }
}
/*-----------------------------------------------------------------------*/
/*  RectangGeom function generates the geometry for a rectangular magnet */
/*  The mesh is divided in the following 5 regions :                     */
/*     0 - Inside the magnet                                             */
/*     2 - North and south poles (magnetic sides)                        */
/*     1 - Non-magnetic sides                                            */
/*     4 - Mesh points outside the magnet                                */
/*     5 - Border of the mesh (must be 0 at all times)                   */
/*     3 - Corners of the magnet                                         */
/*-----------------------------------------------------------------------*/
void RectangGeom(int il)
{
    int i, j, horiz, vert;
    float x, y, xleft, xright, ytop, ybot;

    xleft = size/2.0 - width/2.0;
    xright = size/2.0 + width/2.0;
    ytop = size/2.0 - length/2.0;
    ybot = size/2.0 + length/2.0;
/*---------------------------------------------------------------------*/
/*  Set border to 4. Then scan the mesh and put 4 outside the box and  */
/*  0 inside.                                                          */
/*---------------------------------------------------------------------*/
    for  (i=0; i<XRES; i++) {
        geom[il][i][0] = 5;
        geom[il][i][XRES-1] = 5;
        geom[il][0][i] = 5;
        geom[il][YRES-1][i] = 5;
    }

    y = dy;
    for (i=1; i<YRES-1; i++ ){
        x=dx;                /* The border has already been assigbned */
        for  (j=1; j<XRES-1; j++ ) {
            geom[il][i][j] = 4;
            if  (  ( (x>xleft) && (x<xright) ) &&
                   ( (y>ytop) &&  (y<ybot) ) ) geom[il][i][j] = 0;
            x += dx;
        }
        y += dy;
    }
/*---------------------------------------------------------------------*/
/*  Scan the mesh to locate the border of the magnet                   */
/*---------------------------------------------------------------------*/
    for  (i=1; i<YRES-1; i++) {
        horiz = 0;
        vert = 0;
        for  (j=1; j<XRES-1; j++) {
            if (!horiz) {
                if  ((geom[il][i][j]==4)&&(geom[il][i][j+1]==0)) {
                    geom[il][i][j] = 1;
                    horiz = 1;
                }
            } else {
                if  ((geom[il][i][j-1]==0)&&(geom[il][i][j]==4)) {
                    geom[il][i][j] = 1;
                    horiz = 0;
                }
            }
/* The vertical scan */
            if (!vert) {
                if  ((geom[il][j][i]==4)&&(geom[il][j+1][i]==0)) {
                    geom[il][j][i] = 2;
                    vert = 1;
                }
            } else {
                if  ((geom[il][j-1][i]==0)&&(geom[il][j][i]==4)) {
                    geom[il][j][i] = 2;
                    vert = 0;
                }
            }
        }
    }
/* Scan for the corners */
    for  (i=1; i<YRES-1; i++) {
        for  (j=1; j<XRES-1; j++) {
            if  ((geom[il][i][j+1]==2) && (geom[il][i+1][j]==1) && 
                 (geom[il][i][j]==4)) geom[il][i][j] =3;
            if  ((geom[il][i][j-1]==2) && (geom[il][i+1][j]==1) &&
                 (geom[il][i][j]==4)) geom[il][i][j] =3;
            if  ((geom[il][i][j-1]==2) && (geom[il][i-1][j]==1) &&
                 (geom[il][i][j]==4)) geom[il][i][j] =3;
            if  ((geom[il][i][j+1]==2) && (geom[il][i-1][j]==1) &&
                 (geom[il][i][j]==4)) geom[il][i][j] =3;
        }
    }
}


void fill()
{
    int i,j;

/*------------------------------------------------------------------------*/
/*  Setup the magnetic values                                             */
/*------------------------------------------------------------------------*/
    for  (i=0; i<YRES; i++) {
        for (j=0; j<XRES; j++) {
            switch (geom[ilevel][i][j]) {
            case 0:
                phiold[i][j] = 0.0;
                break; 
            case 1:
                phiold[i][j] = 10.0;
                break; 
            case 2:
                phiold[i][j] = 20.0;
                break; 
            case 3:
                phiold[i][j] = 30.0;
                break; 
            case 4:
                phiold[i][j] = 250.0;
                break; 
            case 5:        /*outer border of mesh */
                phiold[i][j] = 0.0;
                break; 
            }
/*            phiold[i][j] = 100; */
        }
    }
}
