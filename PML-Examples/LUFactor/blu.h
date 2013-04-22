#include <stdio.h>
#include <sys/time.h>

#define N 200
#define M 100

float outMat[N][N];

void LUFactor(float [][], float [][], int);
void TriangleSolver(float [][], float [][], float [][], int, int);
double wall_clock();
