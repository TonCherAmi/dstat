include config.mk

PROGNAME := dstat

SRC := dstat.c
OBJ := ${SRC:.c=.o}

all:
	${CC} -c ${SRC} ${CFLAGS}
	${CC} -o ${PROGNAME} ${OBJ} ${LIB}
