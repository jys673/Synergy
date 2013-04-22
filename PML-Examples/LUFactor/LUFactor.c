#include "blu.h"

void LUFactor(float inMat[M][M], float outMat[M][M], int n)
{
    int i, j, k, row, col;

    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            outMat[i][j] = inMat[i][j];
        }
    }

    for (k = 0; k < n-1; k++)
    {
        for (row = k+1; row < n; row++)
        {
            outMat[row][k] = outMat[row][k] / outMat[k][k];
            for (col = k+1; col < n; col++)
            {
                outMat[row][col] = outMat[row][col] - outMat[row][k] * outMat[k][col];
            }
        }
    }
}
