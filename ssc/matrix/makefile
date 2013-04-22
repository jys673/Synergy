CFLAGS = -I$(SNG_PATH)/include -O3
OBJS =-L$(SNG_PATH)/obj -lsng

ifndef SIZE
  SIZE = 500
endif

all: nxdr copy matrix

nxdr :  mtclnt1 mtwrkr1

mtclnt1 : mtclnt.c
	cc $(CFLAGS) -DN=$(SIZE) -o mtclnt mtclnt.c $(OBJS)
	echo $(MAT_SIZE)

mtwrkr1 : mtwrk.c
	cc $(CFLAGS) -DN=$(SIZE) -o mtwrk mtwrk.c $(OBJS)

matrix : matrix.c
	cc $(CFLAGS) -DN=$(SIZE) -o matrix matrix.c $(OBJS)

copy : mtclnt mtwrk
	cp mtclnt $(HOME)/bin
	cp mtwrk $(HOME)/bin
