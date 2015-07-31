# Amber nginx module #

[![Build Status](https://travis-ci.org/berkmancenter/amber_nginx.png?branch=master)](https://travis-ci.org/berkmancenter/amber_nginx)

This is Amber, an Nginx module that provides an alternative route to information when content would otherwise be unavailable. Amber is useful for virtually any organization or individual that has an interest in preserving the content to which their website links.

If you’d like to join the private beta, we welcome critiques and feedback as we progress through testing. As part of the beta, the Berkman Center will incorporate your suggestions to see what works, what doesn't, and what can be improved. You will also receive personal help and support from our team of devs and sysadmins in running Amber on Nginx.

Indicate your interest by contacting amber@cyber.law.harvard.edu.

## System Requirements ##

* git
* sqlite3
* PHP 5.3 or higher
* cURL
* php-fpm

## Installation (Ubuntu) ##

The Amber module consists of two components:

* An **Nginx module** that identifies pages to be cached, schedules them for caching, and links to cached pages
* A **caching script** that runs periodically to cache new pages and check on the status of existing pages

#### Representations ####
This documentation contains the following representations:
* **$WEBROOT** - We represent where the root of your homepage is as $WEBROOT. (For example, in our install our webroot was /usr/local/nginx/html)
* **$NGINXCONFDIR** -We also represent the Nginx configuration directory as $NGINXCONFDIR. (For example, in our install our Nginx configuration directory was /usr/local/nginx)
* **$BUILDDIR** -We represent the build directory as $BUILDDIR. (For example, in our install our build directory was /usr/local/src)
* **$DATADIR** -We represent the data directory as $DATADIR. (For example, in our install our data directory was /var/lib)
* **$LOGDIR** -We represent the log directory as $LOGDIR. (For example, in our install our log directory was /var/log)
* **$WEBROLE** -We run all services (nginx, FastCGI, database file owner, & cron), as a single user. We represent this user as $WEBROLE. (For example, in our install our web role user was www-data)

Note that the Amber module reserves the /amber top-level directory. If you already have a top-level directory called Amber, you must modify the instructions.

### Install procedure ###

Install prerequisites

    sudo apt-get update
    sudo apt-get -y install git make curl libpcre3 libpcre3-dev sqlite3 libsqlite3-dev php5-cli php5-common php5-sqlite php5-curl php5-fpm zlib1g-dev

Get code
    
    cd $BUILDDIR
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

    sudo mkdir $DATADIR/amber $WEBROOT/amber $WEBROOT/amber/cache
    sudo touch $LOGDIR/amber
    sudo ln -s $BUILDDIR/amber_common/src/admin $WEBROOT/amber/admin
    sudo cp -r $BUILDDIR/amber_common/src/css $BUILDDIR/amber_common/src/js $WEBROOT/amber
    sudo cp $BUILDDIR/amber_nginx/amber.conf $NGINXCONFDIR/conf
    sudo cp $BUILDDIR/amber_nginx/amber-cache.conf $NGINXCONFDIR/conf

Create amber database

    sudo sqlite3 $DATADIR/amber/amber.db < ../amber_common/src/amber.sql

Update permissions

    sudo chgrp -R $WEBROLE $DATADIR/amber $WEBROOT/amber
    sudo chmod -R g+w $DATADIR/amber $WEBROOT/amber/cache
    sudo chmod +x $BUILDDIR/amber_common/deploy/nginxg/vagrant/cron-cache.sh $BUILDDIR/amber_common/deploy/nginx/vagrant/cron-check.sh
    sudo chown $WEBROLE $LOGDIR/amber
    sudo chgrp $WEBROLE $LOGDIR/amber
    
Test that amber cron scripts run without failure

    sudo -u $WEBROLE $BUILDDIR/amber_common/deploy/nginx/vagrant/cron-cache.sh
    sudo -u $WEBROLE $BUILDDIR/amber_common/deploy/nginx/vagrant/cron-check.sh

Create Amber cron jobs

    sudo cat > /etc/cron.d/amber << EOF
    */5 * * * * $WEBROLE /bin/sh $BUILDDIR/amber_common/deploy/nginx/vagrant/cron-cache.sh --ini=$BUILDDIR/amber_common/src/amber-nginx.ini 2>> $LOGDIR/amber >> $LOGDIR/amber
    15 3 * * *  $WEBROLE /bin/sh $BUILDDIR/amber_common/deploy/nginx/vagrant/cron-check.sh --ini=$BUILDDIR/amber_common/src/amber-nginx.ini 2>> $LOGDIR/amber >> $LOGDIR/amber
    EOF

Edit nginx.conf to enable Amber. Edit the root location directive to include amber.conf and amber-cache.conf

    location / {
        [...]
        include amber-cache.conf
        include amber.conf
    }

Start nginx

    sudo $NGINXCONFDIR/sbin/nginx

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

Display Javascript and CSS in Farsi

    subs_filter '</head>' '<script type="text/javascript">var amber_locale="fa";</script><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"><link rel="stylesheet" type="text/css" href="/amber/css/amber_fa.css"></head>';

Allow access to the Amber Admin page to multiple users

    Coming soon!

Create user agent name custom to your domain

    Coming soon!

Modify cron processes (keep caches as-is, or update regularly in "live mirror" format)

    Coming soon!

## Security configuration ##

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

## Optional configuration ##
Display Farsi version of Javascript and CSS

    `subs_filter '</head>' '<script type="text/javascript">var amber_locale="fa";</script><script type="text/javascript"     src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"><link rel="stylesheet" type="text/css" href="/amber/css/amber_fa.css"></head>';`
