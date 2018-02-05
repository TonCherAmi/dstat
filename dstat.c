#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <time.h>

#ifdef MPD
#include <mpd/client.h>
#endif

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>

#define LENGTH(X) (sizeof X  / sizeof X[0])

/* module info functions */

/**
 * writes current date and time to the buffer
 */
static int date(char *modbuf, size_t n);

#ifdef MPD
/**
 * writes current mpd status to the buffer
 */
static int mpd(char *modbuf, size_t n);
#endif

/* config */

#include "config.h"

static void write_modules(char *statbuf, size_t n);

/* module helper functions */

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

    if (xcb_connection_has_error(c) != 0) {
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

        sleep(1);
    }

    xcb_disconnect(c);

    return EXIT_SUCCESS;
}

int date(char *modbuf, size_t n)
{
    time_t now = time(NULL);
    const char *fmt = "[ %a %b %e ] ( %I:%M:%S )";

    return !strftime(modbuf, n, fmt, localtime(&now));
}

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

void write_modules(char *statbuf, size_t n)
{
    char modbuf[DSTAT_MAX_MODBUFSIZE];

    for (int i = 0; i < LENGTH(modules); i++) {
        memset(modbuf, 0, DSTAT_MAX_MODBUFSIZE);

        int errno = modules[i](modbuf, DSTAT_MAX_MODBUFSIZE);

        if (errno) {
            break;
        }

        strncat(statbuf, modbuf, DSTAT_MAX_MODBUFSIZE - 1);

        if (i != LENGTH(modules) - 1 && strlen(modbuf)) {
            strcat(statbuf, separator);
        }
    }
}

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
