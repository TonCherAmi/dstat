VERSION := 0.1

PREFIX := /usr/bin

CC := cc

XLIB := -lxcb -lxcb-icccm

# comment out to disable mpd support
MPDFLAGS := -DMPD
MPDLIB := -lmpdclient

LIB := ${XLIB} ${MPDLIB}

# flags

CFLAGS := -std=c99 -pedantic -Wall ${MPDFLAGS}
LDFLAGS := ${LIB}
