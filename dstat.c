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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>

#ifdef ALSA
#include <alsa/asoundlib.h>
#endif

#ifdef MPD
#include <mpd/client.h>
#endif

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>


#define LENGTH(X) (sizeof X  / sizeof X[0])


/* writes module info to the status buffer */
static void write_modules(char *statbuf, size_t n);


/* module functions */

/**
 * writes current date and time to the buffer
 */
static int date(char *modbuf, size_t n);

#ifdef ALSA
/**
 * writes current alsa volume level to the buffer
 */
static int alsa(char *modbuf, size_t n);
#endif

#ifdef MPD
/**
 * writes current mpd status to the buffer
 */
static int mpd(char *modbuf, size_t n);
#endif


/* config */

#include "config.h"


/* module helper functions */

#ifdef ALSA
/**
 * initializes internal alsa stuff
 */
int alsa_init(snd_mixer_t **mixer, snd_mixer_elem_t **elem);
#endif

#ifdef MPD
/**
 * writes info on currently playing song to the buffer
 */
static void mpd_cur_song_info(struct mpd_connection *c,
                              char *songbuf,
                              size_t n);
#endif


/* xcb helper functions */

/**
 * returns the screen of display
 */
static xcb_screen_t *x_get_screen(xcb_connection_t *c, int nscreen);

/**
 * sets WM_NAME of a window
 */
static void x_set_wm_name(xcb_connection_t *c, xcb_window_t wid, const char *str);


int main()
{
    int nscreen;
    xcb_connection_t *c = xcb_connect(NULL, &nscreen);

    if (xcb_connection_has_error(c)) {
        return EXIT_FAILURE;
    }

    xcb_screen_t *screen = x_get_screen(c, nscreen);

    if (!screen) {
        return EXIT_FAILURE;
    }

    xcb_window_t root = screen->root;

    char statbuf[DSTAT_MAX_STATBUFSIZE];

    while (1) {
        memset(statbuf, 0, DSTAT_MAX_STATBUFSIZE);

        write_modules(statbuf, DSTAT_MAX_STATBUFSIZE);

        x_set_wm_name(c, root, statbuf);

        nanosleep(&interval, NULL);
    }

    xcb_disconnect(c);

    return EXIT_SUCCESS;
}

void write_modules(char *statbuf, size_t n)
{
    char modbuf[DSTAT_MAX_MODBUFSIZE];

    for (int i = 0; i < LENGTH(modules); i++) {
        memset(modbuf, 0, DSTAT_MAX_MODBUFSIZE);

        int err = modules[i](modbuf, DSTAT_MAX_MODBUFSIZE);

        if (err) {
            continue;
        }

        strncat(statbuf, modbuf, DSTAT_MAX_MODBUFSIZE - 1);

        if (i != LENGTH(modules) - 1 && strlen(modbuf)) {
            strcat(statbuf, separator);
        }
    }
}

int date(char *modbuf, size_t n)
{
    time_t now = time(NULL);
    const char *fmt = "[ %a %b %e ] ( %I:%M:%S )";

    return !strftime(modbuf, n, fmt, localtime(&now));
}

#ifdef ALSA
int alsa(char *modbuf, size_t n)
{
    static snd_mixer_t *mixer = NULL;
    static snd_mixer_elem_t *elem = NULL;

    int err = 0;

    if (!mixer && !elem) {
        err = alsa_init(&mixer, &elem);

        if (err) {
            goto out;
        }
    }

    if (snd_mixer_wait(mixer, 25)) {
        err = 1;
        goto out;
    }

    if (snd_mixer_handle_events(mixer) < 0) {
        err = 1;
        goto out;
    }

    long vol_min;
    long vol_max;

    if (snd_mixer_selem_get_playback_volume_range(elem, &vol_min, &vol_max)) {
        err = 2;
        goto free_mixer;
    }

    int nchan = 0;
    long vol_total = 0;

    for (snd_mixer_selem_channel_id_t chan_id = SND_MIXER_SCHN_FRONT_LEFT;
         chan_id <= SND_MIXER_SCHN_LAST;
         chan_id++) {
        if (snd_mixer_selem_has_playback_channel(elem, chan_id)) {
            long vol_current;

            if (snd_mixer_selem_get_playback_volume(elem,
                                                    chan_id,
                                                    &vol_current)) {
                err = 3;
                goto free_mixer;
            }

            nchan++;
            vol_total += vol_current;
        }
    }

    snprintf(modbuf, n, "< %ld >", vol_total / nchan);

    goto out;

free_mixer:
    snd_mixer_close(mixer);

    mixer = NULL;
    elem = NULL;
out:
    return err;
}
#endif

#ifdef MPD
int mpd(char *modbuf, size_t n)
{
    static struct mpd_connection *c = NULL;

    if (!c) {
        c = mpd_connection_new(mpd_host, mpd_port, mpd_timeout);
    }

    if (mpd_connection_get_error(c) != MPD_ERROR_SUCCESS) {
        if (c) {
            mpd_connection_free(c);
            c = NULL;
        }

        return 1;
    }

    struct mpd_status *status = mpd_run_status(c);

    if (!status) {
        return 1;
    }

    enum mpd_state state = mpd_status_get_state(status);

    if (state == MPD_STATE_PLAY || state == MPD_STATE_PAUSE) {
        char songbuf[n - 20];

        mpd_cur_song_info(c, songbuf, n - 20);
        snprintf(modbuf,
                 n,
                 "%u:%02u / %u:%02u . %s",
                 mpd_status_get_elapsed_time(status) / 60,
                 mpd_status_get_elapsed_time(status) % 60,
                 mpd_status_get_total_time(status) / 60,
                 mpd_status_get_total_time(status) % 60,
                 songbuf);
    }

    mpd_status_free(status);

    return 0;
}
#endif

#ifdef ALSA
int alsa_init(snd_mixer_t **mixer, snd_mixer_elem_t **elem)
{
    int err = 0;

    snd_mixer_t *m;

    if (snd_mixer_open(&m, 1)) {
        err = 1;
        goto out;
    }

    snd_config_update_free_global();

    if (snd_mixer_attach(m, alsa_soundcard)) {
        err = 2;
        goto free_mixer;
    }

    if (snd_mixer_selem_register(m, NULL, NULL)) {
        err = 3;
        goto free_mixer;
    }

    if (snd_mixer_load(m)) {
        err = 4;
        goto free_mixer;
    }

    snd_mixer_selem_id_t *selem_id = NULL;

    if (snd_mixer_selem_id_malloc(&selem_id)) {
        err = 5;
        goto free_mixer;
    }

    snd_mixer_selem_id_set_index(selem_id, 0);
    snd_mixer_selem_id_set_name(selem_id, alsa_mixer);

    snd_mixer_elem_t *e;

    if ((e = snd_mixer_find_selem(m, selem_id)) == NULL) {
        err = 6;
        goto free_selem_id;
    }

    *mixer = m;
    *elem = e;

    snd_mixer_selem_id_free(selem_id);

    goto out;

free_selem_id:
    snd_mixer_selem_id_free(selem_id);
free_mixer:
    snd_mixer_close(m);
out:
    return err;
}
#endif

#ifdef MPD
void mpd_cur_song_info(struct mpd_connection *c, char *songbuf, size_t n)
{
    struct mpd_song *song = mpd_run_current_song(c);

    if (!song) {
        return;
    }

    const char *artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
    const char *title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);

    if (artist && title) {
        snprintf(songbuf, n, "%s - %s", artist, title);
    } else {
        snprintf(songbuf, n, "%s", mpd_song_get_uri(song));
    }

    mpd_song_free(song);
}
#endif

xcb_screen_t *x_get_screen(xcb_connection_t *c, int nscreen)
{
    xcb_screen_t *screen = NULL;
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(xcb_get_setup(c));

    while (it.rem) {
        if (nscreen == 0) {
            screen = it.data;
        }

        nscreen--;
        xcb_screen_next(&it);
    }

    return screen;
}

void x_set_wm_name(xcb_connection_t *c, xcb_window_t wid, const char *str)
{
    xcb_icccm_set_wm_name(c,
                          wid,
                          XCB_ATOM_STRING,
                          8,
                          strlen(str),
                          str);

    xcb_flush(c);
}
