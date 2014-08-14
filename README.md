# Amber nginx plugin #

[![Build Status](https://travis-ci.org/berkmancenter/robustness_nginx.png?branch=master)](https://travis-ci.org/berkmancenter/robustness_nginx)

The Amber plugin consists of two components:

* An nginx module that identifies pages to be cached, schedules them for caching, and links to cached pages
* A caching script that runs periodically to cache new pages and check on the status of existing pages

## System Requirements ##

* sqlite3
* PHP 5.3 or higher
* cURL
* php-fpm

## Installation (Ubuntu) ##

Install prerequisites

    sudo apt-get update
    sudo apt-get -y install git make curl libpcre3 libpcre3-dev sqlite3 libsqlite3-dev php5-cli php5-common php5-sqlite php5-curl php5-fpm zlib1g-dev

Get code

    cd /usr/local/src
    git clone https://github.com/berkmancenter/robustness_common.git
    git clone https://github.com/berkmancenter/robustness_nginx.git
    git clone https://github.com/yaoweibin/ngx_http_substitutions_filter_module

Build nginx

    curl http://nginx.org/download/nginx-1.6.0.tar.gz > nginx-1.6.0.tar.gz
    tar xzf nginx-1.6.0.tar.gz
    cd nginx-1.6.0
    ./configure --add-module=../ngx_http_substitutions_filter_module --add-    module=../robustness_nginx
    make
    sudo make install

More coming soon....


## Configuration - Nginx plugin ##

The Amber nginx plugin uses the following configuration directives. See the provided amber.conf for examples. 

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

Specific behavior can be specified for an additional country. The country must be identified, using the ISO code (e.g. US, DE). 

    amber_country <country_code>

The behavior for the country is set using the usual directives, prefixed by ```country_```

    amber_country_behavior_down <hover|popup|cache>;
    amber_country_behavior_up <hover|popup|cache>;
    amber_country_hover_delay_up <time in seconds>;
    amber_country_hover_delay_down <time in seconds>;

Specify additional content mime-types to be processed by Amber (default is text/html)

    amber_filter <content-type>

Insert Javascript and CSS required for Amber to function. `Required`

    subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';

Display Farsi version of Javascript and CSS 

    subs_filter '</head>' '<script type="text/javascript">var amber_locale="fa";</script><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"><link rel="stylesheet" type="text/css" href="/amber/css/amber_fa.css"></head>';

## Configuration - Caching ##

The caching process is configured through ```amber.ini``` - full documentation is available within the sample configuration file [here](https://github.com/berkmancenter/robustness_common/blob/master/src/amber.ini) 

