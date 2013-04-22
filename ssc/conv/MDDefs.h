/* @(#)MDDefs.h	1.35     7/26/94 */

#define	CALLOC(n, x)    	(x *) calloc(n, sizeof(x))
#define	RECALLOC(ptr, n, x) (ptr = (x *) realloc((void *) ptr,(n)*sizeof(x)))
#define FREE(ptr)			free((void *) ptr);	

#define     MAX_PATH_NAME_LEN   2048

#define 	UNSCALED	0
#define		SCALED		1
#define 	RIGHT		0
#define 	LEFT		1
#define 	DECREASE	0
#define 	INCREASE	1
#define 	MDOUT			0
#define 	MDIN		1
#define 	NO			0
#define 	YES			1
#define 	OFF			0
#define 	ON			1

#define		LINEAR		1
#define		CUBIC		2

#define		BLACK		-1
#define		WHITE		-2
#define		INVISABLE	-10

#define		EXCLUDED	-1
#define		COMBINED	0
#define		INCLUDED	1

#define     MDFALSE               0
#define     MDTRUE                1

#define		MDNoJump	-1

#define		MDSTATDEF(x)	(depend_data[x].stats_defined)
#define		MDDO(x)			(depend_data[x].data_offset)

#define		MDSTAT_SUM		(1L<<0)
#define		MDSTAT_NUM		(1L<<1)
#define		MDSTAT_MIN		(1L<<2)
#define		MDSTAT_MAX		(1L<<3)
#define		MDSTAT_SUMSQ	(1L<<4)

#define		MD0(x)	((MDSTATDEF(x)>>0) & 1L)	
#define		MD1(x)	(MD0(x)+((MDSTATDEF(x)>>1) & 1L))	
#define		MD2(x)	(MD1(x)+((MDSTATDEF(x)>>2) & 1L))
#define		MD3(x)	(MD2(x)+((MDSTATDEF(x)>>3) & 1L))	
#define		MD4(x)	(MD3(x)+((MDSTATDEF(x)>>4) & 1L))

#define		MDSum(x)	(MDSTATDEF(x) & MDSTAT_SUM ? MD0(x)-1+MDDO(x) : -1)
#define		MDNum(x)	(MDSTATDEF(x) & MDSTAT_NUM ? MD1(x)-1+MDDO(x) : -1)
#define		MDMin(x)	(MDSTATDEF(x) & MDSTAT_MIN ? MD2(x)-1+MDDO(x) : -1)	
#define		MDMax(x)	(MDSTATDEF(x) & MDSTAT_MAX ? MD3(x)-1+MDDO(x) : -1)
#define		MDSumSq(x)	(MDSTATDEF(x) & MDSTAT_SUMSQ ? MD4(x)-1+MDDO(x) : -1)

#define		MDDataSize	(MDDO(num_depend-1) + MD4(num_depend-1))

#define		MDSymbolPNo	4

#define		HORIZONTAL	0
#define		VERTICAL	1

#define		PT_PER_IN	72.0

#define		AXIS_RAMP	0
#define		AXIS_TEXT1	1
#define		AXIS_TEXT2	2
#define		AXIS_TEXT3	3
#define		AXIS_TEXT4	4
#define		AXIS_TEXT5	5
#define		AXIS_TEXT6	6

#define		RECTANGLE		1
#define		PARALLELOGRAM	2
#define		CIRCLE			3
#define		V_BARS			4
#define		H_BARS			5

#define		LEVEL_CELL_SCALE	0
#define		GLOBAL_CELL_SCALE	1

#define		MDCELL			0
#define		MDSYMBOL		1

#define		MD_II			0
#define		MD_IE			1
#define		MD_EI			2
#define		MD_EE			3

#define		NORM			0
#define		PERM			1

#define		MD_NO_RECT		0
#define		MD_DATA_RECT	1
#define		MD_HAXIS_RECT	2
#define		MD_VAXIS_RECT	3
#define		MD_WIGET_RECT	4
#define		MD_TITLE_RECT	5
#define		MD_GRAPH_RECT	6

#define		MDSUBSPACEIN	0
#define		MDSUBSPACEOUT	1
#define		MDANIMATE		2
#define		MDSUBSET		3
#define		MDRESTORESET	4
#define		MDDECIMATE		5
#define		MDUNDECIMATE	6
#define		MDCOARSESTGRAIN	7
#define		MDCOARSERGRAIN	8
#define		MDFINESTGRAIN	9
#define		MDSORTZOOMED	10
#define		MDSETUPORDER	11

#define		MDSYMBOLSON		0
#define		MDSYMBOLSOFF	1
#define		MDSELECTBINS	2
#define		MDRESETBINS		3

#define		MDSAVENEEDED	0
#define		MDSAVENOTNEEDED	1

#define		MDSolidFill		0
#define		MDThreeQFill	1
#define		MDTwoQFill		2
#define		MDOneQFill		3
#define		MDNoneFill 		4
