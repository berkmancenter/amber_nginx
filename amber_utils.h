#define AMBER_ACTION_NONE     0
#define AMBER_ACTION_HOVER    1
#define AMBER_ACTION_POPUP    2
#define AMBER_ACTION_CACHE    3
#define AMBER_STATUS_DOWN     0
#define AMBER_STATUS_UP       1
#define AMBER_MAX_ATTRIBUTE_STRING 250


typedef struct {
    int        behavior_up;      /* Default behaviour when site is up */
    int        behavior_down;    /* Default behaviour when site is down */
    int        hover_delay_up;   /* Hover delay when site is up */
    int        hover_delay_down; /* Hover delay when site is down */
    char       country[2];
    int        country_behavior_up;      /* Default behaviour when site is up */
    int        country_behavior_down;    /* Default behaviour when site is down */
    int        country_hover_delay_up;   /* Hover delay when site is up */
    int        country_hover_delay_down; /* Hover delay when site is down */
} amber_options_t;

int amber_get_behavior(amber_options_t *options, unsigned char *out, int status);
int amber_build_attribute(amber_options_t *options, unsigned char *out, char *location, int status, time_t date);
