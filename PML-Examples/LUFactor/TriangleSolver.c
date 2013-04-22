#include "blu.h"

void TriangleSolver(float LU[M][M], float inSubMat[M][M], float outSubMat[M][M], 
                    int col, int LUtype)
{
    int i, j, k1, k2, p;

    for (i = 0; i < M; i++)
    {
        for (j = 0; j < M; j++)
        {
            outSubMat[i][j] = inSubMat[i][j];
        }
    }
    /* LUtype = 1, lower triangle */
    if (LUtype == 1)
    {
        for (k1 = 0; k1 < M-1; k1++)
        {
            for (k2 = k1+1; k2 < M; k2++)
            {
                for (p = 0; p < M; p++)
                {
                    outSubMat[k2][p] = outSubMat[k2][p] - LU[k2][k1] * outSubMat[k1][p];
                }
            } 
        }
    }
    /* LUtype = 2, upper triangle */
    if (LUtype == 2)
    {
        for (k1 = 0; k1 < M; k1++)
        {
            for (p = 0; p < M; p++)
            {
                outSubMat[p][k1] = outSubMat[p][k1] / LU[k1][k1];
            }

            for (k2 = k1+1; k2 < M; k2++)
            {
                for (p = 0; p < M; p++)
                {
                    outSubMat[p][k2] = outSubMat[p][k2] - LU[k1][k2] * outSubMat[p][k1];
                }
            } 
        }
    }
}
