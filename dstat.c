#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <time.h>

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>

#define LENGTH(X) (sizeof X  / sizeof X[0])

/* custom types */

/**
 * represents a module argument
 */
typedef union {
    const void *v;
} arg_t;

/**
 * represents a module
 */
typedef struct {
    const char *(*func)(const arg_t *);
    const arg_t arg;
} module_t;

/* module info functions */

/**
 * returns a date string
 */
static const char *date(const arg_t *arg);

/* config */

#include "config.h"

/* xcb helper functions. */

/**
 * returns the screen of display
 */
static xcb_screen_t *x_get_screen(xcb_connection_t *c, int nscreen);

/**
 * sets WM_NAME of a window
 */
static void x_set_wm_name(xcb_connection_t *c, xcb_window_t wid, const char *str);

int main() {
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

    while (1) {
        char statbuff[MAX_STATSIZE];

        memset(statbuff, 0, sizeof statbuff);

        for (int i = 0; i < LENGTH(modules); i++) {
            strcat(statbuff, modules[i].func(&(modules[i].arg)));
        }

        if (i != LENGTH(modules) - 1) {
            strcat(statbuff, separator);
        }

        x_set_wm_name(c, root, statbuff);

        sleep(1);
    }

    xcb_disconnect(c);

    return EXIT_SUCCESS;
}

const char *date(const arg_t *arg) {
    static char datebuff[MAX_BUFFSIZE];
    time_t now = time(NULL);
    const char *fmt = (const char *)arg->v;

    if (strftime(datebuff, MAX_BUFFSIZE, fmt, localtime(&now))) {
        return datebuff;
    }

    return "unable to get date";
}

xcb_screen_t *x_get_screen(xcb_connection_t *c, int nscreen) {
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

void x_set_wm_name(xcb_connection_t *c, xcb_window_t wid, const char *str) {
    xcb_icccm_set_wm_name(c,
                          wid,
                          XCB_ATOM_STRING,
                          8,
                          strlen(str),
                          str);

    xcb_flush(c);
}
