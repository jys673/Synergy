#include "parallel.h"

 
/* <reference> */
 
# include "lapsig.h"
 
/* </reference> */
 

 
/* <parallel appname="lapsig"> */
float * _tp_V_123;
int _x0_123;
int _x1_123;
int _y0_123;
int _y1_123;
int _I_start = 0;
int _I_stop = 0;
int _I_step = 0;
int _J_start = 0;
int _J_stop = 0;
int _J_step = 0;
 

int main()
{
 
/* <reference> */
 
	int I,J,K, COUNT;
	float V[MAXI+1][MAXJ+2];
	float R1S, R2S, R3, A, B, C, URAXP4, RI, RJ;
	float SIGPX, SIGMX, SIGPY, SIGMY;
 
/* </reference> */
 

        long t0, t1;              /* start and end elapsed time */

	char FIELD[MAXI][MAXJ];
	char MARK[20];

	strcpy(MARK,"abcdefghijklmnopqrst");

/* 10000,1,41	------ initialized in the header file
	printf("($V1, I,J -)\n");
	scanf("%f, %d, %d", &VP1, &IP1, &JP1);
	printf("+%f, %d, %d\n", VP1, IP1, JP1);
	RIP1 = (float)IP1;
	RJP1 = (float)JP1;
*/

/* -10000,1,80	------ initialized in the header file
	printf("($V2,I,J -)\n");
	scanf("%f, %d, %d", &VP2, &IP2, &JP2);
	printf("+%f, %d, %d)\n", VP2, IP2, JP2);
	RIP2=(float)IP2;
	RJP2=(float)JP2;
*/

/* 10000	------ initialized in the header file
	printf("(Iterations-)\n");
	scanf("%d", &ITER);
	printf("(+%d\n", ITER);
*/

/* 100,41,80	------ initialized in the header file
	printf("$Enter table level and left and right coordinates -\n");
	scanf("%d %d %d", &TLEV, &TLFT, &TRGT);
	printf("(+%d %d %d\n", TLEV, TLFT, TRGT);
*/

/* 4.0	------ initialized in the header file
	printf("$Under-relaxation factor -\n");
	scanf("%f", &URAX);
	printf("(+%f\n", URAX);
*/
	URAXP4 = URAX + 4.0;

/*	------ initialized in the header file
	for (K=1; K<=20; K++)
	{
		printf("($Voltage Line: %d -\n", K);
		scanf("%f", &DVP[K-1]);
		if (DVP[K-1] == 0)
			break;
		printf("(+%f - (%c)\n", DVP[K-1], MARK[K-1]);
	}
	NDVP=K-1;
*/

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
			
	for (J=TLFT; J<=TRGT; J++)
	{
		V[TLEV][J] = 0.0;
	}
/*
	for (I=1; I<=MAXI; I++)
	{
		for (J=1; J<=MAXJ; J++)
		{
			printf(" %f ", V[I][J]);
		}
		printf("\n");
	}
*/

        t0 = time ((long*)0);

         
/* <master id="123"> */
_distributor = _open_space("distributor", 0, "123");
_constructor = _open_space("constructor", 0, "123");
 

	for (COUNT=1; COUNT<=ITER; COUNT++)
	{
		for (J=1; J<=MAXJ; J++)
		{
			V[0][J]=V[1][J];
		}
		for (I=0; I<=MAXI-1; I++) 
		{
			V[I][0]=V[I][1];
			V[I][MAXJ+1]=V[I][MAXJ];
		}

                 
_cleanup_space(_distributor, "123");
_cleanup_space(_constructor, "123");
    /* <send var="V" type="float[MAXI+1][MAXJ+2]" opt="ONCE"/> */
    sprintf(_tp_name, "float(%d)(%d):V#%s", (MAXI+1), (MAXJ+2), "123");
    _tp_size = ((MAXI+1) * (MAXJ+2)) * sizeof(float);
    _tp_V_123 = (float *)malloc(_tp_size);
    for (_x0_123 = 0; _x0_123 < (MAXI+1); _x0_123 +=1) {
        for (_x1_123 = 0; _x1_123 < (MAXJ+2); _x1_123 +=1) {

            _tp_V_123[_x0_123 * (MAXJ+2) + _x1_123] = V[_x0_123][_x1_123];
        }
    }

    _status  = _send_data(_distributor, _tp_name, (char *)_tp_V_123, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_V_123);

 

                 
    /* <token action="SET" idxset="(I)(J)"/> */
    sprintf(_tp_name, "token#%s", "123");
    sprintf(_tp_token, "=(I:%d~%d,%d:#%d)(J:%d~%d,%d:#%d)", 1, MAXI, 1, GRAIN, 1, MAXJ+1, 1, GRAIN);
    _tp_size = sizeof(_tp_token);
    _tokens  = _set_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tokens < 0) exit(-1);


    /* <send var="V" type="float[MAXI+1(0   ~1     )][MAXJ+2(0     ~MAXJ+1)]" opt="XCHG"/> */
    sprintf(_tp_name, "float(%d)(%d):V#%s[%d~%d,%d][%d~%d,%d]@%d", (MAXI+1), (MAXJ+2), "123", (0   ), (1     ), 1, (0     ), (MAXJ+1), 1, sizeof(float));
    _tp_size = ((((1     ) - (0   ) - 1) / 1 + 1) * (((MAXJ+1) - (0     ) - 1) / 1 + 1)) * sizeof(float);
    _tp_V_123 = (float *)malloc(_tp_size);
    for (_x0_123 = (0   ), _y0_123 =0; _x0_123 < (1     ); _x0_123 +=1, _y0_123 ++) {
        for (_x1_123 = (0     ), _y1_123 =0; _x1_123 < (MAXJ+1); _x1_123 +=1, _y1_123 ++) {

            _tp_V_123[_y0_123 * (((MAXJ+1) - (0     ) - 1) / 1 + 1) + _y1_123] = V[_x0_123][_x1_123];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_V_123, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_V_123);

 
                 
    /* <send var="V" type="float[MAXI+1(0   ~MAXI  )][MAXJ+2(MAXJ+1~MAXJ+2)]" opt="XCHG"/> */
    sprintf(_tp_name, "float(%d)(%d):V#%s[%d~%d,%d][%d~%d,%d]@%d", (MAXI+1), (MAXJ+2), "123", (0   ), (MAXI  ), 1, (MAXJ+1), (MAXJ+2), 1, sizeof(float));
    _tp_size = ((((MAXI  ) - (0   ) - 1) / 1 + 1) * (((MAXJ+2) - (MAXJ+1) - 1) / 1 + 1)) * sizeof(float);
    _tp_V_123 = (float *)malloc(_tp_size);
    for (_x0_123 = (0   ), _y0_123 =0; _x0_123 < (MAXI  ); _x0_123 +=1, _y0_123 ++) {
        for (_x1_123 = (MAXJ+1), _y1_123 =0; _x1_123 < (MAXJ+2); _x1_123 +=1, _y1_123 ++) {

            _tp_V_123[_y0_123 * (((MAXJ+2) - (MAXJ+1) - 1) / 1 + 1) + _y1_123] = V[_x0_123][_x1_123];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_V_123, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_V_123);

 
                 
    /* <send var="V" type="float[MAXI+1(MAXI~MAXI+1)][MAXJ+2(1     ~MAXJ+2)]" opt="XCHG"/> */
    sprintf(_tp_name, "float(%d)(%d):V#%s[%d~%d,%d][%d~%d,%d]@%d", (MAXI+1), (MAXJ+2), "123", (MAXI), (MAXI+1), 1, (1     ), (MAXJ+2), 1, sizeof(float));
    _tp_size = ((((MAXI+1) - (MAXI) - 1) / 1 + 1) * (((MAXJ+2) - (1     ) - 1) / 1 + 1)) * sizeof(float);
    _tp_V_123 = (float *)malloc(_tp_size);
    for (_x0_123 = (MAXI), _y0_123 =0; _x0_123 < (MAXI+1); _x0_123 +=1, _y0_123 ++) {
        for (_x1_123 = (1     ), _y1_123 =0; _x1_123 < (MAXJ+2); _x1_123 +=1, _y1_123 ++) {

            _tp_V_123[_y0_123 * (((MAXJ+2) - (1     ) - 1) / 1 + 1) + _y1_123] = V[_x0_123][_x1_123];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_V_123, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_V_123);

 
                 
    /* <send var="V" type="float[MAXI+1(1   ~MAXI+1)][MAXJ+2(0     ~1     )]" opt="XCHG"/> */
    sprintf(_tp_name, "float(%d)(%d):V#%s[%d~%d,%d][%d~%d,%d]@%d", (MAXI+1), (MAXJ+2), "123", (1   ), (MAXI+1), 1, (0     ), (1     ), 1, sizeof(float));
    _tp_size = ((((MAXI+1) - (1   ) - 1) / 1 + 1) * (((1     ) - (0     ) - 1) / 1 + 1)) * sizeof(float);
    _tp_V_123 = (float *)malloc(_tp_size);
    for (_x0_123 = (1   ), _y0_123 =0; _x0_123 < (MAXI+1); _x0_123 +=1, _y0_123 ++) {
        for (_x1_123 = (0     ), _y1_123 =0; _x1_123 < (1     ); _x1_123 +=1, _y1_123 ++) {

            _tp_V_123[_y0_123 * (((1     ) - (0     ) - 1) / 1 + 1) + _y1_123] = V[_x0_123][_x1_123];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_V_123, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_V_123);

 

                  

                 
    /* <read var="V" type="float[MAXI+1][MAXJ+2]"/> */
    sprintf(_tp_name, "float(%d)(%d):V#%s", (MAXI+1), (MAXJ+2), "123");
    _tp_size = ((MAXI+1) * (MAXJ+2)) * sizeof(float);
    _tp_V_123 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_V_123, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123 = 0; _x0_123 < (MAXI+1); _x0_123 +=1) {
        for (_x1_123 = 0; _x1_123 < (MAXJ+2); _x1_123 +=1) {

            V[_x0_123][_x1_123] = _tp_V_123[_x0_123 * (MAXJ+2) + _x1_123];
        }
    }

    free(_tp_V_123);

 

/*
		for (I=1; I<=MAXI; I++)
		{
			for (J=1; J<=MAXJ; J++)
			{
				printf(" %f ", V[I][J]);
			}
			printf("\n");
		}
*/
	}

         
_close_space(_constructor, "123", 1);
_close_space(_distributor, "123", 1);
/* </master> */
 
				
	for (I=1; I<=MAXI-1; I++)
	{
		strcpy(FIELD[I-1] , " ");
		for (J=1; J<=MAXJ; J++)
		{
			for (K=1; K<=NDVP; K++)
			{
				if (abs(V[I][J]-DVP[K-1]) < abs(DVP[K-1]/20.0))
					FIELD[I-1][J-1] = MARK[K-1];
			}
		}
	}

	FIELD[IP1-1][JP1-1]='*';
	FIELD[IP2-1][JP2-1]='*';
/* ???? */
	strcpy(&FIELD[TLEV-1][TLFT-1],"1**********************************************");

	for (J=1; J<=MAXJ; J++)
	{
		FIELD[MAXI-1][J-1]='*';
	}

	for (I=1; I<=MAXI; I++)
	{
		for (J=MAXJ; J>=2; J--)
		{
			if (FIELD[I-1][J-1] != ' ')
				break;
		}
/* ???? */
		printf("( %s)\n", FIELD[I-1]);

	}

        t1 = time((long *)0) - t0;

        printf ("\nTime: (%d), MAXI(J): (%d), Iters: (%d)\n", 
                 t1, MAXI, ITER);

	printf("End of Lapsig.c\n");
	exit(0);
}

 
/* </parallel> */
 


 
/* <reference> */
 
float SIGMA_f(float RI,float RJ)
{
	float RIP1, RJP1, RIP2, RJP2;
	float D1,D2;
	float SIGMA;

	RIP1 = (float)IP1;
	RJP1 = (float)JP1;

	RIP2 = (float)IP2;
	RJP2 = (float)JP2;

	D1=sqrt(pow((RI-RIP1),2)+pow((RJ-RJP1),2));
	D2=sqrt(pow((RI-RIP2),2)+pow((RJ-RJP2),2));
	SIGMA=1.0/max(1.0,min(D1,D2));

	return (SIGMA);
}

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
 
/* </reference> */
 
	 
