#define CAYL_ACTION_NONE     0
#define CAYL_ACTION_HOVER    1
#define CAYL_ACTION_POPUP    2
#define CAYL_ACTION_CACHE    3
#define CAYL_STATUS_DOWN     0
#define CAYL_STATUS_UP       1
#define CAYL_MAX_ATTRIBUTE_STRING 200


typedef struct {
    int        behavior_up;      /* Default behaviour when site is up */
    int        behavior_down;    /* Default behaviour when site is down */
    int        hover_delay_up;   /* Hover delay when site is up */
    int        hover_delay_down; /* Hover delay when site is down */
} cayl_options_t;

int cayl_get_behavior(cayl_options_t *options, unsigned char *out, int status);
int cayl_build_attribute(cayl_options_t *options, unsigned char *out, char *location, int status, time_t date);
