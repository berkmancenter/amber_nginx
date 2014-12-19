!/bin/bash

sudo apt-get update
sudo apt-get -y install git make curl libpcre3 libpcre3-dev sqlite3 libsqlite3-dev php5-cli php5-common php5-sqlite php5-curl php5-fpm zlib1g-dev

cd /usr/local/src
sudo git clone https://github.com/berkmancenter/amber_common.git
#sudo git clone https://github.com/berkmancenter/amber_nginx.git
sudo git clone https://github.com/yaoweibin/ngx_http_substitutions_filter_module

if [ "$1" == "-skip-nginx"]
then
	echo "NGINX must be recompiled to complete installation. This can be performed with the following commands:"
echo "sudo wget http://nginx.org/download/nginx-1.6.0.tar.gz"
echo "sudo tar xzf nginx-1.6.0.tar.gz"
echo "cd nginx-1.6.0"
echo "sudo ./configure --add-module=../ngx_http_substitutions_filter_module --add-module=../amber_nginx"
echo "sudo make && sudo make install"

else
sudo wget http://nginx.org/download/nginx-1.6.0.tar.gz
sudo tar xzf nginx-1.6.0.tar.gz
cd nginx-1.6.0
sudo ./configure --add-module=../ngx_http_substitutions_filter_module --add-module=../amber_nginx
sudo make && sudo make install
fi

sudo mkdir /var/lib/amber /usr/local/nginx/html/amber /usr/local/nginx/html/amber/cache
sudo touch /var/log/amber
sudo ln -s /usr/local/src/amber_common/src/admin /usr/local/nginx/html/amber/admin
sudo cp -r /usr/local/src/amber_common/src/css /usr/local/src/amber_common/src/js /usr/local/nginx/html/amber
sudo cp /usr/local/src/amber_nginx/amber.conf /usr/local/nginx/conf

sudo sqlite3 /var/lib/amber/amber.db < ../amber_common/src/amber.sql
sudo cat > /etc/cron.d/amber << EOF
*/5 * * * * www-data /bin/sh /usr/local/src/amber_common/deploy/nginx/vagrant/cron-cache.sh --ini=/usr/local/src/amber_common/src/amber-nginx.ini 2>> /var/log/amber >> /var/log/amber
15 3 * * *  www-data /bin/sh /usr/local/src/amber_common/deploy/nginx/vagrant/cron-check.sh --ini=/usr/local/src/amber_common/src/amber-nginx.ini 2>> /var/log/amber >> /var/log/amber
EOF

sudo chgrp -R www-data /var/lib/amber /usr/local/nginx/html/amber
sudo chmod -R g+w /var/lib/amber /usr/local/nginx/html/amber/cache
sudo chmod +x /usr/local/src/amber_common/deploy/nginx/vagrant/cron-cache.sh /usr/local/src/amber_common/deploy/nginx/vagrant/cron-check.sh
sudo chown www-data /var/log/amber
sudo chgrp www-data /var/log/amber
