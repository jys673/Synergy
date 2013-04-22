CFLAGS = -g
OBJS =-L$(SNG_PATH)/obj -lsng

#SIZE = $(MAT_SIZE)
#G= $(G_MAT)  
SIZE = 500 #$(MAT_SIZE)
G= 100#$(G_MAT)  

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
