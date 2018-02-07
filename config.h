/**
 * dstat - a simple status bar
 * Copyright © 2018 Vasili Karaev
 *
 * This file is part of dstat.
 *
 * dstat is free software: you can redistribute  it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * dstat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHENTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with dstat. If not, see <http://www.gnu.org/licenses/>.
 */

/* maximum total status size */
#define DSTAT_MAX_STATBUFSIZE 500

/* maximum module length */
#define DSTAT_MAX_MODBUFSIZE 100

static const struct timespec interval = { .tv_sec = 0, .tv_nsec = 500000000 };

/* module separator */
static const char separator[] = " :: ";

/* alsa configuration variables */
const char *alsa_soundcard = "hw:3";
const char *alsa_mixer = "PCM";

/* mpd configuration variables */
const char *mpd_host = "localhost";
const unsigned mpd_port = 6600;
const unsigned mpd_timeout = 0;

/* module array */
static int (*modules[])(char *, size_t) = { mpd, alsa, date };
