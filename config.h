/* maximum total status size */
#define MAX_STATSIZE 500

/* maximum module length */
#define MAX_BUFFSIZE 100

/* module separator */
static const char separator[] = " :: ";

/* see man strftime for the list of available tokens */
static const char date_format[] = "[ %a %b %e ] ( %I:%M:%S )";

module_t modules[] = {
    /* module name      argument */
    { date,             { .v = datefmt } },
};
