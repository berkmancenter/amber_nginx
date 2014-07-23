#include <stdio.h>
#include <string.h>
#include <time.h>
#include "amber_utils.h"


#define AMBER_MAX_BEHAVIOR_STRING 20
#define AMBER_MAX_DATE_STRING 30

/* Create a string containing attributes to be added to the HREF

    amber_options_t *options : configuration settings
    char *out               : buffer to where the attribute is written
    chatr *locatino         : location of the cached copy
    int status              : whether the site is up or down
    time_t date             : when the cache was generated (unix epoch)

    returns 0 on success
*/
int amber_build_attribute(amber_options_t *options, unsigned char *out, char *location, int status, time_t date)
{
    unsigned char behavior[AMBER_MAX_BEHAVIOR_STRING];
    char date_string[AMBER_MAX_DATE_STRING];

    int rc = amber_get_behavior(options, behavior, status);
    if (!rc) {
        struct tm *timeinfo = localtime(&date);
        strftime(date_string,AMBER_MAX_DATE_STRING,"%FT%T%z",timeinfo);
        snprintf((char *)out,
                 AMBER_MAX_ATTRIBUTE_STRING,
                 "data-cache='/%s %s' data-amber-behavior='%s' ",
                 location,
                 date_string,
                 behavior
                 );
         }

    return rc;
}

/* Get the contents of behavior attribute based on the status of the link
   and the configuration settings.

   amber_options_t *options : configuration settings
   char *out               : buffer to where the attribute is written
   int status              : whether the site is up or down

   returns 0 on success

   TODO: Country-specific behavior
   */
int amber_get_behavior(amber_options_t *options, unsigned char *out, int status) {
    if (!options || !out) {
        return 1;
    }
    if (status == AMBER_STATUS_UP) {
        switch (options->behavior_up) {
            case AMBER_ACTION_HOVER:
                snprintf((char *)out,
                         AMBER_MAX_ATTRIBUTE_STRING,
                         "up hover:%d",
                         options->hover_delay_up);
                break;
            case AMBER_ACTION_POPUP:
                snprintf((char *)out, AMBER_MAX_ATTRIBUTE_STRING, "up popup");
                break;
            case AMBER_ACTION_CACHE:
                snprintf((char *)out, AMBER_MAX_ATTRIBUTE_STRING, "up cache");
                break;
            case AMBER_ACTION_NONE:
                break;
        }
    } else if (status == AMBER_STATUS_DOWN) {
        switch (options->behavior_down) {
            case AMBER_ACTION_HOVER:
                snprintf((char *)out,
                         AMBER_MAX_ATTRIBUTE_STRING,
                         "down hover:%d",
                         options->hover_delay_down);
                break;
            case AMBER_ACTION_POPUP:
                snprintf((char *)out, AMBER_MAX_ATTRIBUTE_STRING, "down popup");
                break;
            case AMBER_ACTION_CACHE:
                snprintf((char *)out, AMBER_MAX_ATTRIBUTE_STRING, "down cache");
                break;
            case AMBER_ACTION_NONE:
                break;

        }
    }
    if (strlen(options->country)) {
        char country_attribute[AMBER_MAX_ATTRIBUTE_STRING];
        if (status == AMBER_STATUS_UP) {
            switch (options->country_behavior_up) {
                case AMBER_ACTION_HOVER:
                    snprintf(country_attribute,
                             AMBER_MAX_ATTRIBUTE_STRING,
                             ",%s up hover:%d",
                             options->country,
                             options->country_hover_delay_up);
                    break;
                case AMBER_ACTION_POPUP:
                    snprintf(country_attribute, AMBER_MAX_ATTRIBUTE_STRING, ",%s up popup", options->country);
                    break;
                case AMBER_ACTION_CACHE:
                    snprintf(country_attribute, AMBER_MAX_ATTRIBUTE_STRING, ",%s up cache", options->country);
                    break;
                case AMBER_ACTION_NONE:
                    break;
            }
        } else if (status == AMBER_STATUS_DOWN) {
            switch (options->country_behavior_down) {
                case AMBER_ACTION_HOVER:
                    snprintf(country_attribute,
                             AMBER_MAX_ATTRIBUTE_STRING,
                             ",%s down hover:%d",
                             options->country,
                             options->country_hover_delay_down);
                    break;
                case AMBER_ACTION_POPUP:
                    snprintf(country_attribute, AMBER_MAX_ATTRIBUTE_STRING, ",%s down popup", options->country);
                    break;
                case AMBER_ACTION_CACHE:
                    snprintf(country_attribute, AMBER_MAX_ATTRIBUTE_STRING, ",%s down cache", options->country);
                    break;
                case AMBER_ACTION_NONE:
                    break;
            }
        }
        if (strlen(country_attribute)) {
            strncat((char *) out, country_attribute, AMBER_MAX_ATTRIBUTE_STRING - strlen((char *)out));
        }
    }

    return 0;
}
