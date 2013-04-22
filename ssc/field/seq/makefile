OPT = 

field: main.o geom.o jacobi.o gs.o  fft.o multi.o
	cc $(OPT) -o field  main.o geom.o jacobi.o gs.o multi.o  \
	fft.o -lm  -L$(SNG_PATH)/obj -lsng

xdisp.o : xdisp.c inx.h field.h
	cc $(OPT) -c xdisp.c

main.o: main.c field.h
	cc $(OPT) -c main.c -I$(SNG_PATH)/include \
	-L$(SNG_PATH)/obj -lsng

geom.o: geom.c   field.h
	cc $(OPT) -c geom.c

jacobi.o: jacobi.c   field.h
	cc $(OPT) -c jacobi.c

gs.o: gs.c   field.h
	cc $(OPT) -c gs.c

multi.o: multi.c   field.h
	cc $(OPT) -c multi.c

