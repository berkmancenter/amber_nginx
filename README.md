# Amber nginx plugin #

[![Build Status](https://travis-ci.org/berkmancenter/robustness_nginx.png?branch=master)](https://travis-ci.org/berkmancenter/robustness_nginx)

The Amber plugin consists of two components:

* An nginx module that identifies pages to be cached, schedules them for caching, and links to cached pages
* A caching script that run periodically to cache new pages and check on the status of existing pages

## Installation ##

Coming soon..

## Configuration - Nginx plugin ##

The Amber nginx plugin uses the following configuration directives. See the provided cayl.conf for examples. 

Enable Amber

    amber <on|off>

The location of the sqlite3 database file used by Amber to keep track of cached links

    amber_db <filename>;

The behavior for cached links that appear to be down

    amber_behavior_down <hover|popup|cache>;

The behavior for cached links that appear to be up

    amber_behavior_up <hover|popup|cache>;

If the behavior is "hover", the delay in seconds before displaying the hover popup. There are separate configurations for liks that are up or down

    amber_hover_delay_up <time in seconds>;
    amber_hover_delay_down <time in seconds>;

Specify additional content mime-types to be processed by Amber (default is text/html)

    amber_filter <content-type>

Insert Javascript and CSS required for Amber to function. `Required`

    subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';

Display Farsi version of Javascript and CSS 

    subs_filter '</head>' '<script type="text/javascript">var amber_locale="fa";</script><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"><link rel="stylesheet" type="text/css" href="/amber/css/amber_fa.css"></head>';

## Configuration - Caching ##

The caching process is configured through ```amber.ini``` - full documentation is available within the sample configuration file [here](https://github.com/berkmancenter/robustness_common/blob/master/src/amber.ini) 

