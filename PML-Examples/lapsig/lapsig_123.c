#include "parallel.h"

/* <reference> */
 
# include "lapsig.h"
 
/* </reference> */

/* <reference> */
 
	int I,J,K, COUNT;
	float V[MAXI+1][MAXJ+2];
	float R1S, R2S, R3, A, B, C, URAXP4, RI, RJ;
	float SIGPX, SIGMX, SIGPY, SIGMY;
 
/* </reference> */

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

main(int argc, char **argv[])
{

/* <worker id="123"> */
_distributor = _open_space("distributor", 0, "123");
_constructor = _open_space("constructor", 0, "123");
 

                 
while (1)
{

    /* <token action="GET" idxset="(I)(J)"/> */
    sprintf(_tp_name, "token#%s", "123");
    _tp_size = 0;
    _tp_size = _get_token(_distributor, _tp_name, (char *)_tp_token, _tp_size);
    if (_tp_size < 0) exit(-1);
    if (_tp_token[0] == '!') break;
    sscanf(_tp_token, "%d@(I:%d~%d,%d)(J:%d~%d,%d)", &_tokens, &_I_start, &_I_stop, &_I_step, &_J_start, &_J_stop, &_J_step);


    /* <read var="V" type="float[MAXI+1][MAXJ+2]"/> */
    sprintf(_tp_name, "float(%d)(%d):V#%s", (MAXI+1), (MAXJ+2), "123");
    _tp_size = ((MAXI+1) * (MAXJ+2)) * sizeof(float);
    _tp_V_123 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_distributor, _tp_name, (char *)_tp_V_123, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123 = 0; _x0_123 < (MAXI+1); _x0_123 +=1) {
        for (_x1_123 = 0; _x1_123 < (MAXJ+2); _x1_123 +=1) {

            V[_x0_123][_x1_123] = _tp_V_123[_x0_123 * (MAXJ+2) + _x1_123];
        }
    }

    free(_tp_V_123);

 

                 
    /* <read var="V" type="float[MAXI+1(I:$L-1)][MAXJ+2(J)     ]" opt="XCHG"/> */
    sprintf(_tp_name, "float(%d)(%d):V#%s[%d~%d,%d][%d~%d,%d]@%d", (MAXI+1), (MAXJ+2), "123", (_I_start-1), _I_start, 1, _J_start, _J_stop, 1, sizeof(float));
    _tp_size = (((_I_start - (_I_start-1) - 1) / 1 + 1) * ((_J_stop - _J_start - 1) / 1 + 1)) * sizeof(float);
    _tp_V_123 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_V_123, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123 = (_I_start-1), _y0_123 =0; _x0_123 < _I_start; _x0_123 +=1, _y0_123 ++) {
        for (_x1_123 = _J_start, _y1_123 =0; _x1_123 < _J_stop; _x1_123 +=1, _y1_123 ++) {

            V[_x0_123][_x1_123] = _tp_V_123[_y0_123 * ((_J_stop - _J_start - 1) / 1 + 1) + _y1_123];
        }
    }

    free(_tp_V_123);

 
                 
    /* <read var="V" type="float[MAXI+1(I)     ][MAXJ+2(J:$L-1)]" opt="XCHG"/> */
    sprintf(_tp_name, "float(%d)(%d):V#%s[%d~%d,%d][%d~%d,%d]@%d", (MAXI+1), (MAXJ+2), "123", _I_start, _I_stop, 1, (_J_start-1), _J_start, 1, sizeof(float));
    _tp_size = (((_I_stop - _I_start - 1) / 1 + 1) * ((_J_start - (_J_start-1) - 1) / 1 + 1)) * sizeof(float);
    _tp_V_123 = (float *)malloc(_tp_size);
    _tp_size = _read_data(_constructor, _tp_name, (char *)_tp_V_123, _tp_size);
    if (_tp_size < 0) exit(-1);
    for (_x0_123 = _I_start, _y0_123 =0; _x0_123 < _I_stop; _x0_123 +=1, _y0_123 ++) {
        for (_x1_123 = (_J_start-1), _y1_123 =0; _x1_123 < _J_start; _x1_123 +=1, _y1_123 ++) {

            V[_x0_123][_x1_123] = _tp_V_123[_y0_123 * ((_J_start - (_J_start-1) - 1) / 1 + 1) + _y1_123];
        }
    }

    free(_tp_V_123);

 

                 
/* <target index ="I" order ="1" limits="(1,MAXI,1)" chunk ="GRAIN"> */
for (I = _I_start; I < _I_stop; I +=_I_step) 
/* </target> */
 
		{
                     
/* <target index ="J" order ="1" limits="(1,MAXJ+1,1)" chunk ="GRAIN"> */
for (J = _J_start; J < _J_stop; J +=_J_step) 
/* </target> */
 
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

	         		V[I][J]= (URAX*V[I][J]
					+(SIGMY*V[I][J-1]+SIGPY*V[I][J+1]
					+SIGMX*V[I-1][J]+SIGPX*V[I+1][J]))
					/(URAX+SIGPX+SIGMX+SIGPY+SIGMY);
			}
		}


                 
    /* <send var="V" type="float[MAXI+1(I)     ][MAXJ+2(J)     ]"/> */
    sprintf(_tp_name, "float(%d)(%d):V#%s[%d~%d,%d][%d~%d,%d]@%d", (MAXI+1), (MAXJ+2), "123", _I_start, _I_stop, 1, _J_start, _J_stop, 1, sizeof(float));
    _tp_size = (((_I_stop - _I_start - 1) / 1 + 1) * ((_J_stop - _J_start - 1) / 1 + 1)) * sizeof(float);
    _tp_V_123 = (float *)malloc(_tp_size);
    for (_x0_123 = _I_start, _y0_123 =0; _x0_123 < _I_stop; _x0_123 +=1, _y0_123 ++) {
        for (_x1_123 = _J_start, _y1_123 =0; _x1_123 < _J_stop; _x1_123 +=1, _y1_123 ++) {

            _tp_V_123[_y0_123 * ((_J_stop - _J_start - 1) / 1 + 1) + _y1_123] = V[_x0_123][_x1_123];
        }
    }

    _status  = _send_data(_constructor, _tp_name, (char *)_tp_V_123, _tp_size);
    if (_status < 0) exit(-1);
    free(_tp_V_123);

 

                 
}

_close_space(_constructor, "123", 0);
_close_space(_distributor, "123", 0);
/* </worker> */

exit(0);
}

/* </parallel> */
