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



Install prerequisites

    sudo apt-get update
    sudo apt-get -y install git make curl libpcre3 libpcre3-dev sqlite3 libsqlite3-dev php5-cli php5-common php5-sqlite php5-curl php5-fpm zlib1g-dev

Get code
    
    cd /usr/local/src
    sudo git clone https://github.com/berkmancenter/amber_common.git
    sudo git clone https://github.com/berkmancenter/amber_nginx.git
    sudo git clone https://github.com/yaoweibin/ngx_http_substitutions_filter_module

Build nginx

    sudo wget http://nginx.org/download/nginx-1.6.0.tar.gz
    sudo tar xzf nginx-1.6.0.tar.gz
    cd nginx-1.6.0
    sudo ./configure --add-module=../ngx_http_substitutions_filter_module --add-module=../amber_nginx
    sudo make && sudo make install

Create amber directories and install supporting files

    sudo mkdir /var/lib/amber /usr/local/nginx/html/amber /usr/local/nginx/html/amber/cache
    sudo touch /var/log/amber
    sudo ln -s /usr/local/src/amber_common/src/admin /usr/local/nginx/html/amber/admin
    sudo cp -r /usr/local/src/amber_common/src/css /usr/local/src/amber_common/src/js /usr/local/nginx/html/amber
    sudo cp /usr/local/src/amber_nginx/amber.conf /usr/local/nginx/conf
    sudo cp /usr/local/src/amber_nginx/amber-cache.conf /usr/local/nginx/conf


Create amber database and cron jobs

    sudo sqlite3 /var/lib/amber/amber.db < ../amber_common/src/amber.sql
    sudo cat > /etc/cron.d/amber << EOF
    */5 * * * * www-data /bin/sh /usr/local/src/amber_common/deploy/nginx/vagrant/cron-cache.sh --ini=/usr/local/src/amber_common/src/amber-nginx.ini 2>> /var/log/amber >> /var/log/amber
    15 3 * * *  www-data /bin/sh /usr/local/src/amber_common/deploy/nginx/vagrant/cron-check.sh --ini=/usr/local/src/amber_common/src/amber-nginx.ini 2>> /var/log/amber >> /var/log/amber
    EOF

Update permissions

    sudo chgrp -R www-data /var/lib/amber /usr/local/nginx/html/amber
    sudo chmod -R g+w /var/lib/amber /usr/local/nginx/html/amber/cache
    sudo chmod +x /usr/local/src/amber_common/deploy/nginxg/vagrant/cron-cache.sh /usr/local/src/amber_common/deploy/nginx/vagrant/cron-check.sh
    sudo chown www-data /var/log/amber
    sudo chgrp www-data /var/log/amber

Edit nginx.conf to enable amber. Edit the root location directive to include amber.conf and amber-cache.conf

    location / {
        [...]
        include amber-cache.conf
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

## Configuration - Sandboxing ##

For improved security, you must next configure Amber to serve cached content from a separate domain. We recommend that you establish a subdomain for this purpose.

Here is a sample configuration where the site is running at www.amber.com, while cached content is served from sandbox.amber.com.

    http {
        [...]
        server {
            server_name sandbox.amber.com;
            root html;
            include amber-cache.conf;
        }
        server {
            server_name www.amber.com;
            root html;
            location /amber/cache {
                return 301 $scheme://sandbox.amber.org$request_uri;
            }
            location / {
                include amber.conf;
                [...]
            }
        }
    }

## Configuration - Caching script ##

The caching process is configured through ```amber.ini``` - full documentation is available within the sample configuration file [here](https://github.com/berkmancenter/amber_common/blob/master/src/amber-nginx.ini) 

