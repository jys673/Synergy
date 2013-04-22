#include <stdio.h>
#include <sys/time.h>
#include "matrix.h"

float outMat[N][N];
double wall_clock();
void LUFactor(float [][], float [][], int);

main()
{
  int i, j, dist, k1, k2, p1, p2, q1, q2, q3;
  int subdist, rowdist, coldist;
  int idA, idB, num_recvd, recv_length;
  int tsd, res, status; 
  
  float inSubMat[M][M], outSubMat[M][M], LU[M][M];
  float mat1[M][M], mat2[M][M], res_mat[M][M];
 
  double t0, t1;

  FILE *fd;
 
  t0 = wall_clock();

  if ((msgA = (struct msg *) malloc(tpAlength)) == NULL) exit(1);
  if ((msgB = (struct res_msg *) malloc(tpBlength)) == NULL) exit(1);

  for(i=0; i<N; i++)
  {
    for(j=0; j<N; j++)
    {
      if(i==j)
      {
        outMat[i][j] = N;
      }
      else
      {
        outMat[i][j]= 1;
      }
    }
  }
  tsd = cnf_open("problem",0);
  res = cnf_open("result",0);
 
  i = 0;
  while(i < N)
  {
    j = i + M - 1;
    dist = M;
    if(j > N-1) 
    {
      j = N-1;
      dist = N - i;
    }
    
    // LU factors for submatrix   
    for(k1 = 0; k1 < dist; k1++)
    {
      for(k2 = 0; k2 < dist; k2++)
      {
        inSubMat[k1][k2] = outMat[i+k1][i+k2];
      }
    }
    
    LUFactor(inSubMat, outSubMat, dist);
   
    //update
    for(k1 = 0; k1 < dist; k1++)
    {
      for(k2 = 0; k2 < dist; k2++)
      {
        outMat[i+k1][i+k2] = outSubMat[k1][k2];
        LU[k1][k2] = outSubMat[k1][k2];
        (*msgA).mat1[k1][k2] = outSubMat[k1][k2];
      }
    }
   
    //LZ, WU solver
    k1 = j + 1;
    idA = 0;
    while(k1 < N)
    {
      k2 = k1 + M - 1;
      subdist = M;
      if(k2 > N-1)
      {
        k2 = N - 1;
        subdist = N - k1;
      }
      
      //Solve LZ
      (*msgA).type = 1;
      (*msgA).x = i;
      (*msgA).y = k1;
      (*msgA).row = M;
      (*msgA).col = subdist;
      for(p1 = i; p1 < i+M; p1++)
      {
        for(p2 = k1; p2 <= k2; p2++)
        {
          (*msgA).mat2[p1-i][p2-k1] = outMat[p1][p2];
        }
      }
      
      //put LZ=Z' to the tuple space
      sprintf(tpname,"A%d\0",idA);
      idA++;
      status = cnf_tsput(tsd, tpname, msgA, tpAlength);

      //Solve WU
      (*msgA).type = 2;
      (*msgA).x = k1;
      (*msgA).y = i;
      (*msgA).row = subdist;
      (*msgA).col = M;
      for(p1 = i; p1 < i+M; p1++)
      {
        for(p2 = k1; p2 <= k2; p2++)
        {
          (*msgA).mat2[p2-k1][p1-i] = outMat[p2][p1];
        }
      }            
      //put WU=W' to the tuple space
      sprintf(tpname,"A%d\0",idA);
      idA++;
      status = cnf_tsput(tsd, tpname, msgA, tpAlength);

      k1 = k1 + M;      
    }
    
    //receive result from worker
    for(num_recvd = 0; num_recvd < idA; num_recvd++)
    {
      strcpy(tpname,"B*");
      recv_length = cnf_tsget(res, tpname, msgB, 0);
      
     // printf("result LZ, WU:\n");
      //printf("%d, %d\n", msgB.x, msgB.y); 
      //put the sub_result into the whole picture
      for(k1 = 0; k1 < (*msgB).row; k1++)
      {
        for(k2 = 0; k2 < (*msgB).col; k2++)
        {
  //printf("%5.2f  ", msgB.res_mat[k1][k2]);
          outMat[(*msgB).x + k1][(*msgB).y + k2] = (*msgB).res_mat[k1][k2];
        }
//printf("\n");
      }     
    }
        
    //A = A - WZ
    idA = 0;
    k1 = j + 1;
    while(k1 < N)
    {
      k2 = k1 + M - 1;
      rowdist = M;
      if(k2 > N-1)
      {
        k2 = N - 1;
        rowdist = N - k1;
      }
      
      (*msgA).type = 3;
      (*msgA).x = k1;
      (*msgA).row = rowdist;
      
      for(q1 = 0; q1 < rowdist; q1++)
      {
          for(q2 = 0; q2 < M; q2++)
          {
            (*msgA).mat1[q1][q2] = outMat[k1+q1][i+q2];
          }
      }    

      p1 = j + 1;
      while(p1 < N)
      {
        p2 = p1 + M - 1;
        coldist = M;
        if(p2 > N-1)
        {
          p2 = N - 1;
          coldist = N - p1;
        }
  
        (*msgA).y = p1;
        (*msgA).col = coldist;
        
        for(q1 = 0; q1 < M; q1++)
        {
          for(q2 = 0; q2 < coldist; q2++)
          {
            (*msgA).mat2[q1][q2] = outMat[i+q1][p1+q2];
          }
        }        
        sprintf(tpname,"A%d\0",idA);
        idA++;
        status = cnf_tsput(tsd, tpname, msgA, tpAlength);

        p1 = p1 + M;  
      }
      k1 = k1 + M;
    }
    
    //receive result from worker
    for(num_recvd = 0; num_recvd < idA; num_recvd++)
    {
      strcpy(tpname,"B*");
      recv_length = cnf_tsget(res, tpname, msgB, 0);
      
      //put the sub_result into the whole picture
      for(k1 = 0; k1 < (*msgB).row; k1++)
      {
        for(k2 = 0; k2 < (*msgB).col; k2++)
        {
          outMat[(*msgB).x + k1][(*msgB).y + k2] = outMat[(*msgB).x + k1][(*msgB).y + k2] - (*msgB).res_mat[k1][k2];
        }
      }     
    }              
    i = i + M;
  }

  //for(i=0; i<N; i++)
  //{
    //for(j=0; j<N; j++)
    //{
      //printf("%6.3f ", outMat[i][j]);
    //}
    //printf("\n");
 //} 

  //terminate tuple space
  idA = N*N;
  sprintf(tpname,"A%d\0",idA);
  (*msgA).type = 4;
  printf("msgA.row = %d\n",(*msgA).row); 
  status = cnf_tsput(tsd, tpname, msgA, tpAlength);
  printf("termination status = %d\n", status);
  
  t1 = wall_clock() - t0;
  if (t1>0) printf(" (%f) MFLOPS.\n", (float) 2*N*N*N/3/t1);
  else printf(" MFLOPS: Not measured.\n");

  printf("elapse time = %10.6f\n", t1/1000000);

        fd = fopen("matrix.par.time", "a");
        //fprintf(fd, "nXDR chunked: (%s) (%f)sec. P(%d) f(%d) n(%d) ",
        //              host, t1/1000000, P, G,  N*1);
        if (t1>0) fprintf(fd, " (%f) MFLOPS.\n", (float) N*N*N/t1);
        else fprintf(fd, " MFLOPS: Not measured.\n");
        fclose(fd);


  free(msgA);
  free(msgB);

  cnf_term();
}

void LUFactor(float inMat[M][M], float outMat[M][M], int n)
{
  int i, j, k, row, col;

  //copy inMat to outMat
  for(i=0; i<n; i++)
  {
    for(j=0; j<n; j++)
    {
      outMat[i][j] = inMat[i][j];
    }
  }

  for(k=0; k<n-1; k++)
  {
    for(row=k+1; row<n; row++)
    {
      outMat[row][k] = outMat[row][k] / outMat[k][k];
      for(col=k+1; col<n; col++)
      {
        outMat[row][col] = outMat[row][col] - outMat[row][k] * outMat[k][col];
      }
    }
  }
}

double wall_clock()
{
  struct timeval tp;
  struct timezone tzp;
  double t;
  gettimeofday(&tp, &tzp);
  t = (tzp.tz_minuteswest*60 + tp.tv_sec)*1.0e6 + (tp.tv_usec)*1.0;
  return t;
}




--------------------------------------------------------------------------------


#include <stdio.h>
#include "matrix.h"

void TriangleSolver(float [][], float [][], float [][], int, int, int);
void matmul(float [][], float [][], float [][], int, int);

main()
{
  int i, j, row, col, tsd, res, ix, status, recv_length;

  float mat1[M][M], mat2[M][M], res_mat[M][M];

  if ((msgA = (struct msg *) malloc(tpAlength)) == NULL) exit(1);
  if ((msgB = (struct res_msg *) malloc(tpBlength)) == NULL) exit(1);
  
  tsd = cnf_open("problem",0);
  res = cnf_open("result",0);  
  
  while(1)
  {
    strcpy(tpname,"A*");
    recv_length = cnf_tsget(tsd, tpname, msgA, 0);
    
    if(recv_length <= 0)
    {
cnf_term();
        exit(0);
    }
 
    //decide whether it is a termination tuple
    if((*msgA).type == 4)
    {
      printf("recv_length = %d, tpAlength = %d \n", recv_length, tpAlength);
      status = cnf_tsput(tsd, tpname, msgA, tpAlength);
      printf("Worker received terminate tuple, status = %d\n", status);
      //free(msgA);
      //free(msgB);
      cnf_term();
      return;
    }

    (*msgB).type = (*msgA).type;
    (*msgB).x = (*msgA).x;
    (*msgB).y = (*msgA).y;
    (*msgB).row = (*msgA).row;
    (*msgB).col = (*msgA).col;
   
    //printf("type = %d \n", (*msgA).type); 
    row = (*msgA).row;
    col = (*msgA).col;

    //printf("matrix received:\n");
    for(i = 0; i < M; i++)
    {
      for(j = 0; j < M; j++)
      {
        mat1[i][j] = (*msgA).mat1[i][j];
        mat2[i][j] = (*msgA).mat2[i][j];
//printf("%5.2f  ", (*msgA).mat1[i][j]);
      }
      //printf("\n");    
    }
    
    if((*msgA).type == 1)
    {
      TriangleSolver(mat1, mat2, res_mat, row, col, 1);    
    }
    else if((*msgA).type == 2)
    {
      TriangleSolver(mat1, mat2, res_mat, row, col, 2);       
    }
    else
    {
      matmul(mat1, mat2, res_mat, row, col);
    }
      
    //send sub_solution to master
    ix = atoi(&tpname[1]);
    sprintf(tpname,"B%d\0",ix);
    for(i = 0; i < row; i++)
    {
      for(j = 0; j < col; j++)
      {
        (*msgB).res_mat[i][j] = res_mat[i][j];
      }
    }
    status = cnf_tsput(res, tpname, msgB, tpBlength);
  }
  exit(0);
}

void TriangleSolver(float LU[M][M], float inSubMat[M][M], float outSubMat[M][M], int row, int col, int LUtype)
{
  int i, j, k1, k2, p;
  for(i = 0; i < M; i++)
  {
    for(j = 0; j < M; j++)
    {
      outSubMat[i][j] = inSubMat[i][j];
    }
  }
  //LUtype=1, lower triangle
  if(LUtype == 1)
  {
    for(k1 = 0; k1 < M-1; k1++)
    {
      for(k2 = k1 + 1; k2 < M; k2++)
      {
        for(p = 0; p < M; p++)
        {
          outSubMat[k2][p] = outSubMat[k2][p] - LU[k2][k1] * outSubMat[k1][p];
        }
      } 
    }
  }
  
  //LUtype=2, upper triangle
  if(LUtype == 2)
  {
    for(k1 = 0; k1 < M; k1++)
    { 
      for(p = 0; p < M; p++)
      {
        outSubMat[p][k1] = outSubMat[p][k1]/LU[k1][k1];
      }
      
      k2 = k1 + 1;
      while(k2 < M)
      {
        for(p = 0; p < M; p++)
        {
          outSubMat[p][k2] = outSubMat[p][k2] - LU[k1][k2] * outSubMat[p][k1];
        }
        k2++;
      } 
    }
  }
}

void matmul(float mat1[M][M], float mat2[M][M], float res[M][M], int row, int col)
{
  int i, j, k;
  for(i = 0; i < row; i++)
  {
    for(j = 0; j < col; j++)
    {
      res[i][j] = 0;
      for(k = 0; k < M; k++)
      {
        res[i][j] = res[i][j] + mat1[i][k] * mat2[k][j];
      }
    }
  }
}
