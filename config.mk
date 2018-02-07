VERSION := 0.1

PREFIX := /usr/bin

CC := cc

XLIB := -lxcb -lxcb-icccm

# comment out to disable alsa support
ALSAFLAGS := -DALSA
ALSALIB := -lasound

# comment out to disable mpd support
MPDFLAGS := -DMPD
MPDLIB := -lmpdclient

MODFLAGS := ${ALSAFLAGS} ${MPDFLAGS}
LIB := ${XLIB} ${ALSALIB} ${MPDLIB}

# flags

CFLAGS := -std=c99 -pedantic -Wall -D_POSIX_C_SOURCE=200809L ${MODFLAGS}
LDFLAGS := ${LIB}
