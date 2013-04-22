//   	DISK$USER:[KAPPS.STATIC_INC]LAPSIG.FOR;3
// 
//	PROGRAM LAPLACE
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

//	PARAMETER (MAXI=120,MAXJ=120)
#define MAXI 120
#define MAXJ 120
// Prototype for SIGMA 
float SIGMA_f(float RI, float RJ);
float max(float x, float y);
float min(float x, float y);

// Globals
float RIP1, RJP1, RIP2, RJP2;

//	INTEGER IP1,JP1,IP2,JP2
//	REAL RIP1,RJP1,RIP2,RJP2
//	COMMON /WIRES/RIP1,RJP1,RIP2,RJP2
//	INTEGER I,J,K,ITER,COUNT,NDVP
//	INTEGER TLEV,TLFT,TRGT
//	REAL V(0:MAXI,0:MAXJ+1),VP1,VP2,DVP(1:20)
//	REAL R1S,R2S,R3,A,B,C,URAX,URAXP4,RI,RJ
//	REAL SIGMA,SIGPX,SIGMX,SIGPY,SIGMY
//	CHARACTER*(MAXJ) FIELD(MAXI)
//	CHARACTER*20 MARK
//	MARK='abcdefghijklmnopqrst'
int main()
{
	int IP1, JP1, IP2, JP2;
	int I,J,K, ITER, COUNT, NDVP;
	int TLEV, TLFT, TRGT;
	float V[MAXI][MAXJ+1], VP1, VP2, DVP[20];
	float R1S, R2S, R3, A, B, C, URAX, URAXP4, RI, RJ;
	float SIGPX, SIGMX, SIGPY, SIGMY;
	char FIELD[MAXI][MAXJ];
	char MARK[20];

	strcpy(MARK,"abcdefghijklmnopqrst");

//	TYPE '(''$V1,I,J - '')'
//	ACCEPT *,VP1,IP1,JP1
	printf("($V1, I,J -)\n");
	scanf("%f, %d, %d", &VP1, &IP1, &JP1);
	printf("+%f, %d, %d\n", VP1, IP1, JP1);
//	TYPE '(''+''F8.0,I4,I4)',VP1,IP1,JP1
//	RIP1=REAL(IP1)
//	RJP1=REAL(JP1)
	RIP1 = (float)IP1;
	RJP1 = (float)JP1;
//	TYPE '(''$V2,I,J - '')'
	printf("($V2,I,J -)\n");
//	ACCEPT *,VP2,IP2,JP2
	scanf("%f, %d, %d", &VP2, &IP2, &JP2);
//	TYPE '(''+''F8.0,I4,I4)',VP2,IP2,JP2
	printf("+%f, %d, %d)\n", VP2, IP2, JP2);
//	RIP2=REAL(IP2)
//	RJP2=REAL(JP2)
	RIP2=(float)IP2;
	RJP2=(float)JP2;
//	TYPE '(''$Iterations - '')'
	printf("(Iterations-)\n");
//	ACCEPT *,ITER
	scanf("%d", &ITER);
//	TYPE '(''+''I7)',ITER
	printf("(+%d\n", ITER);
//	TYPE '(''$Enter table level and left and right coordinates - '')'
	printf("$Enter table level and left and right coordinates -\n");
//	ACCEPT *,TLEV,TLFT,TRGT
	scanf("%d %d %d", &TLEV, &TLFT, &TRGT);
//	TYPE '(''+''3I5)',TLEV,TLFT,TRGT
	printf("(+%d %d %d\n", TLEV, TLFT, TRGT);
//	TYPE '(''$Under-relaxation factor - '')'
	printf("$Under-relaxation factor -\n");
//	ACCEPT *,URAX
	scanf("%f", &URAX);
//	TYPE '(''+''F6.2)',URAX
	printf("(+%f\n", URAX);
//	URAXP4=URAX+4.0
	URAXP4 = URAX + 4.0;
//	DO 5 K=1,20
//	   TYPE '(''$Voltage Line '',I2,''  - '')',K
//	   ACCEPT *,DVP(K)
//	   IF(DVP(K).EQ.0.0)GO TO 6
//	   TYPE '(''+''F8.0,'' - '',A)',DVP(K),MARK(K:K)
//5	CONTINUE
	for (K=1; K<=20; K++)
	{
		printf("($Voltage Line: %d -\n", K);
		scanf("%f", &DVP[K]);
		printf("(+%f - (%c)\n", DVP[K], MARK[K]);
		if (DVP[K] == 0) break;
	}
//6	NDVP=K-1
//	DO 20 I=1,MAXI
//	   DO 10 J=1,MAXJ
//	      R1S=SQRT(REAL((IP1-I)**2+(JP1-J)**2))
//	      R2S=SQRT(REAL((IP2-I)**2+(JP2-J)**2))
//	      R3=REAL(MAXI-I)
//	      A=R1S*R2S
//	      B=R1S*R3
//	      C=R2S*R3
//	      V(I,J)=(VP2*B+VP1*C)/(A+B+C)
//10	   CONTINUE
//20	CONTINUE
	NDVP=K-1;
	for (I=1; I<=MAXI; I++)
	{
		for (J=1; J<=MAXJ; J++)
		{
	      		R1S=sqrt((float)(pow((IP1-I),2)+pow((JP1-J),2)));
			R2S=sqrt((float)(pow((IP2-I),2)+pow((JP2-J),2)));
	      		R3=(float)(MAXI-I);
	      		A=R1S*R2S;
	      		B=R1S*R3;
	      		C=R2S*R3;
	      		V[I][J]=(VP2*B+VP1*C)/(A+B+C);
		}
	}
			
//	DO 25 J=TLFT,TRGT
//	   V(TLEV,J)=0.0
//25	CONTINUE
	for (J=TLFT; J<=TRGT; J++) V[TLEV][J] = 0.0;
//C
//D	   PRINT '(1X,20F5.1)',((V(I,J),J=1,MAXJ),I=1,MAXI)
	for (I=1; I<=MAXI; I++)
	{
		for (J=1; J<=MAXJ; J++) printf(" %f ", V[I][J]);
		printf("\n");
	}
//	DO 70 COUNT=1,ITER
//	   DO 30 J=1,MAXJ
//	      V(0,J)=V(1,J)
//30	   CONTINUE
//	   DO 40 I=0,MAXI-1
//	      V(I,0)=V(I,1)
//	      V(I,MAXJ+1)=V(I,MAXJ)
//40	   CONTINUE
//	   DO 60 I=1,MAXI-1
//	      DO 50 J=1,MAXJ
//	         IF(((I.EQ.IP1).AND.(J.EQ.JP1)).OR.
//	1           ((I.EQ.IP2).AND.(J.EQ.JP2)).OR.
//	2           ((I.EQ.TLEV).AND.(J.GE.TLFT).AND.(J.LE.TRGT)))
//	3              GO TO 50
//	         RI=REAL(I)
//	         RJ=REAL(J)
//	         SIGPX=SIGMA(RI+0.5,RJ)
//	         SIGMX=SIGMA(RI-0.5,RJ)
//	         SIGPY=SIGMA(RI,RJ+0.5)
//	         SIGMY=SIGMA(RI,RJ-0.5)
//	         V(I,J)=
//	1          (URAX*V(I,J)+
//	2          ( SIGMY*V(I,J-1)+SIGPY*V(I,J+1)+
//	3          SIGMX*V(I-1,J)+SIGPX*V(I+1,J) ))
//	4          /(URAX+SIGPX+SIGMX+SIGPY+SIGMY)
//50	      CONTINUE
//60	   CONTINUE
//D	   PRINT '(1X,20F5.1)',((V(I,J),J=1,MAXJ),I=1,MAXI)
//70	CONTINUE
	for (COUNT=1; COUNT<=ITER; COUNT ++)
	{
		for (J=1; J<=MAXJ; J++) V[0][J]=V[I][J];
		for (I=0; I<=MAXI-1; I++) 
		{
			V[I][0]=V[I][1];
			V[I][MAXJ+1]=V[I][MAXJ];
		}
		for (I=1; I<=MAXI-1; I++)
		{
			for (J=1; J<=MAXJ; J++)
			{
				if (((I==IP1) && (J==JP1)) || ((I==IP2) && (J==JP2)) ||
					((I==TLEV) && (J>=TLFT) && (J<=TRGT)))
					continue;
	         		RI=(float)(I);
	         		RJ=(float)(J);
	         		SIGPX=SIGMA_f(RI+0.5,RJ);
	         		SIGMX=SIGMA_f(RI-0.5,RJ);
	         		SIGPY=SIGMA_f(RI,RJ+0.5);
	         		SIGMY=SIGMA_f(RI,RJ-0.5);
	         		V[I][J]= (URAX*V[I][J]+( SIGMY*V[I][J-1]+SIGPY*V[I][J+1]
					+SIGMX*V[I-1][J]+SIGPX*V[I+1][J]))/(URAX+SIGPX+SIGMX+SIGPY+SIGMY);
			}
		}
	}
				
//	DO 90 I=1,MAXI-1
//	   FIELD(I)=' '
//	   DO 80 J=1,MAXJ
//	      DO 75 K=1,NDVP
//	         IF(ABS(V(I,J)-DVP(K)).LT.ABS(DVP(K)/20.0))
//	1              FIELD(I)(J:J)=MARK(K)
//75	      CONTINUE
//80	   CONTINUE
//90	CONTINUE
	for (I=1; I<=MAXI-1; I++)
	{
		strcpy(FIELD[I] , " ");
		for (J=1; J<=MAXJ; J++)
		{
			for (K=1; K<=NDVP; K++)
			if (abs(V[I][J]-DVP[K]) < abs(DVP[K]/20.0)) FIELD[I][J] = MARK[K];
		}
	}
//	FIELD(IP1)(JP1:JP1)='*'
//	FIELD(IP2)(JP2:JP2)='*'
//	FIELD(TLEV)(TLFT:TRGT)=
//	1'**************************************************'
	FIELD[IP1][JP1]='*';
	FIELD[IP2][JP2]='*';
	strcpy(&FIELD[TLEV][TLFT],"1**********************************************");
//	DO 100 J=1,MAXJ
//	   FIELD(MAXI)(J:J)='*'
//100	CONTINUE
	for (J=1; J<=MAXJ; J++) FIELD[MAXI][J]='*';

//	DO 130 I=1,MAXI
//	   DO 110 J=MAXJ,2,-1
//	      IF(FIELD(I)(J:J).NE.' ')GO TO 120
//110	   CONTINUE
//120	   PRINT '(1X,A)',FIELD(I)(1:J)
//130	CONTINUE
	for (I=1;I<=MAXI; I++)
	{
		for (J=MAXJ; J>=2; J--) if (FIELD[I][J] != ' ') continue;
		printf("( %s)\n", FIELD[I]);
	}
//	STOP
//	END
	printf("End of Lapsig.c\n");
	exit(0);
}

//
//	REAL FUNCTION SIGMA(RI,RJ)
//	REAL RI,RJ
//	REAL RIP1,RJP1,RIP2,RJP2
//	COMMON /WIRES/RIP1,RJP1,RIP2,RJP2
//	REAL AMAX1,AMIN1,SQRT,REAL,D1,D2
//	D1=SQRT((RI-RIP1)**2+(RJ-RJP1)**2)
//	D2=SQRT((RI-RIP2)**2+(RJ-RJP2)**2)
//	SIGMA=1.0/AMAX1(1.0,AMIN1(D1,D2))
//	RETURN
//	END

float max(float x, float y)
{
	if (x > y) return x;
	return y;
}

float min(float x, float y)
{
	if (x < y) return x;
	return y;
}
	 

float SIGMA_f(float RI,float RJ)
{
//	float RIP1,RJP1,RIP2,RJP2;
//	COMMON /WIRES/RIP1,RJP1,RIP2,RJP2
	float D1,D2;
	float SIGMA;

	D1=sqrt(pow((RI-RIP1),2)+pow((RJ-RJP1),2));
	D2=sqrt(pow((RI-RIP2),2)+pow((RJ-RJP2),2));
	SIGMA=1.0/max(1.0,min(D1,D2));
	return (SIGMA);
}	
