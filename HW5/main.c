#include "func.c"

int main(int argc, char *argv[]){

int A[N][N], B[N][N], C[N][N];
int i, j, k;

// naive 
for (i=0; i<N; i++)
  for (j=0; j<N; j++)
    for (k=0; k<N; k++)
      C[i][j] += A[i][k]+B[k][j];

// smart
for (i=0; i<N; i++)
  for (k=0; k<N; k++)
    for (j=0; j<N; j++)
      C[i][j] += A[i][k]+B[k][j];

  return 0;
  
}
