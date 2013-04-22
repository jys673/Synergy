/*
 * lapsig.h
 *
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define MAXI 120
#define MAXJ 120

#define GRAIN 20

float VP1 = 10000;
int IP1 = 1, JP1 = 41;

float VP2 = -10000;
int IP2 = 1, JP2 = 80;

int ITER = 10000;

int TLEV = 100, TLFT = 41, TRGT = 80;

float URAX = 4.0;

float DVP[20] = {100, -100, 200, -200, 500, -500, 1000, -1000, 5000, -5000};

int NDVP = 10; /* = the # of elements in DVP */

float SIGMA_f(float RI, float RJ);
float max(float x, float y);
float min(float x, float y);

