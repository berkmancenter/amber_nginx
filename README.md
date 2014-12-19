# Amber nginx module #

[![Build Status](https://travis-ci.org/berkmancenter/amber_nginx.png?branch=master)](https://travis-ci.org/berkmancenter/amber_nginx)

This is Amber, an Nginx module that provides an alternative route to information when content would otherwise be unavailable. Amber is useful for virtually any organization or individual that has an interest in preserving the content to which their website links.

If you’d like to join the private beta, we welcome critiques and feedback as we progress through testing. As part of the beta, the Berkman Center will incorporate your suggestions to see what works, what doesn't, and what can be improved. You will also receive personal help and support from our team of devs and sysadmins in running Amber on Nginx.

Indicate your interest by contacting amber@cyber.law.harvard.edu.

## System Requirements ##

* sqlite3
* PHP 5.3 or higher
* cURL
* php-fpm

## Installation (Ubuntu) ##

The Amber module consists of two components:

* An **Nginx module** that identifies pages to be cached, schedules them for caching, and links to cached pages
* A **caching script** that runs periodically to cache new pages and check on the status of existing pages


Ensure you have GIT installed. If not, you can install (Ubuntu) with through apt-get.
    
    sudo apt-get install git

Get code.
    
    cd /usr/local/src
    sudo git clone https://github.com/berkmancenter/amber_nginx.git

Install amber using the amber-install.sh script. This will download the prerequisites, create the database and configure the paths for the amber module. Use the "-skip-nginx-install" option to skip the download and installation of a new instance of nginx.

    cd /usr/local/src/amber_nginx
    sudo ./amber-install.sh

Edit nginx.conf to enable amber. Edit the root location directive to include amber.conf

    location / {
        [...]
        include amber.conf
    }

Start nginx

    sudo /usr/local/nginx/sbin/nginx

## Configuration - Nginx module ##

The Amber nginx module uses the following configuration directives. See the provided amber.conf for examples. 

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

## Configuration - Caching script ##

The caching process is configured through ```amber.ini``` - full documentation is available within the sample configuration file [here](https://github.com/berkmancenter/amber_common/blob/master/src/amber-nginx.ini) 

