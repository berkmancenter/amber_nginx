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


