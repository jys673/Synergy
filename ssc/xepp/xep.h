#define XRES 600 
#define YRES 600 

char        tpname[128];
double  ituple[7];                           /* Vectorized data structure */
        /* [0]: grain size
           [1]: x (screen coord)
           [2]: ix (world coord)
           [3]: iy
           [4]: xstep
           [5]: ystep
           [6]: iteration limit */

struct otupl {	
	char	host[128];
	double  index[YRES*XRES+2];
	} otuple;
