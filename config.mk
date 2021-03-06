# dstat - a simple status bar
# Copyright © 2018 Vasili Karaev
#
# This file is part of dstat.
#
# dstat is free software: you can redistribute  it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# dstat is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHENTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with dstat. If not, see <http://www.gnu.org/licenses/>.

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
