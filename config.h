/* maximum total status size */
#define DSTAT_MAX_STATBUFSIZE 500

/* maximum module length */
#define DSTAT_MAX_MODBUFSIZE 100

/* module separator */
static const char separator[] = " :: ";

/* mpd configuration variables */
const char *mpd_host = "localhost";
const unsigned mpd_port = 6600;
const unsigned mpd_timeout = 0;

/* module array */
static int (*modules[])(char *, size_t) = { mpd, date };
