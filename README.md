# Amber nginx plugin #

## Installation ##

## Configuration ##

The Amber plugin uses the following configuration directives. See the provided cayl.conf for examples. 

Enable Amber

    cayl <on|off>

The location of the sqlite3 database file used by Amber to keep track of cached links

    cayl_db <filename>;

The behavior for cached links that appear to be down

    cayl_behavior_down <hover|popup|cache>;

The behavior for cached links that appear to be up

    cayl_behavior_up <hover|popup|cache>;

If the behavior is "hover", the delay in seconds before displaying the hover popup. There are separate configurations for liks that are up or down

    cayl_hover_delay_up <time in seconds>;
    cayl_hover_delay_down <time in seconds>;

Specify additional content mime-types to be processed by Amber (default is text/html)

    cayl_filter <content-type>

Insert Javascript and CSS required for Amber to function. `Required`

    subs_filter '</head>' '<script type="text/javascript" src="/cayl/js/cayl.js"></script><link rel="stylesheet" type="text/css" href="/cayl/css/cayl.css"></head>';

Display Farsi version of Javascript and CSS 

    subs_filter '</head>' '<script type="text/javascript">var cayl_locale="fa";</script><script type="text/javascript" src="/cayl/js/cayl.js"></script><link rel="stylesheet" type="text/css" href="/cayl/css/cayl.css"><link rel="stylesheet" type="text/css" href="/cayl/css/cayl_fa.css"></head>';


