#this is a sample makefile for synergy applications
SYNERGY_DIR = ../..
all: split sort merge 

split:	split.c
	cc $(SYS_TYPE) $(SYS_LINK) -g split.c -o split -L${SYNERGY_DIR}/obj -lsng $(SYS_LIB)

sort:	sort.c
	cc $(SYS_TYPE) $(SYS_LINK) -g sort.c -o sort -L${SYNERGY_DIR}/obj -lsng $(SYS_LIB)

merge:	merge.c
	cc $(SYS_TYPE) $(SYS_LINK) -g merge.c -o merge -L${SYNERGY_DIR}/obj -lsng $(SYS_LIB)

