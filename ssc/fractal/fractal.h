#define XRES 600 
#define YRES 600 

double  xmin = -2,
	ymin = -2,
	xmax = 2,
	ymax = 2;

int	iterat = 1024;

char        tpname[128];
double  ituple[8];                           /* Vectorized data structure */
        /* [0]: grain size
           [1]: x (screen coord)
           [2]: xmin (world)
           [3]: xmax 
           [4]: ymin 
           [5]: ymax
	   [6]: Xres
	   [7]: Yres
           [8]: iteration limit */
double  otuple[YRES*XRES+2];                  /* returned color indices */
        /* [0]: grain size
           [1]: column index (x)
           ...: color indices */
