# define  TRUE 1
# define  FALSE 0
# define  NLIMIT  100
# define  NUM_QUEEN 14

typedef   int  rowcheck[NLIMIT];
typedef   int  diagonalcheck1[2*NLIMIT-1];
typedef   int  diagonalcheck2[2*NLIMIT-1];

rowcheck        r;
diagonalcheck1  d1;
diagonalcheck2  d2;

char            tpname[128];
int             ituple[NLIMIT],iituple[2];
