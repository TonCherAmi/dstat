CC = cc
LIB = -lxcb -lxcb-icccm

all:
	${CC} -c dstat.c -std=c99
	${CC} -o dstat dstat.o ${LIB}
