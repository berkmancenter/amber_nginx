# vi:filetype=

use lib 'lib';
use Test::Nginx::Socket; # 'no_plan';

# repeat_each(2);

plan tests => repeat_each() * 4 * blocks();

no_long_string();
#no_diff;

run_tests();


__DATA__

=== TEST 1: Deliver cached item with Memento header
--- init
system("cp t/test.db /tmp/amber_nginx_test_delivery_01.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'amber/cache/XYZ', 99, 'text/html', 100);\" | sqlite3 /tmp/amber_nginx_test_delivery_01.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 1, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_delivery_01.db");
--- config
location /amber/cache {
  	amber off; 
  	amber_cache_delivery on;
	amber_db /tmp/amber_nginx_test_delivery_01.db;
  	default_type  text/html;	
}
--- user_files
>>> amber/cache/XYZ
I am the walrus
--- request
GET /amber/cache/XYZ
--- response_body
I am the walrus
--- response_headers
Memento-Datetime: Thu, 01 Jan 1970 00:01:39 GMT
--- no_error_log
AMBER error

=== TEST 2: Deliver cached item with different content-type
--- init
system("cp t/test.db /tmp/amber_nginx_test_delivery_01.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'amber/cache/XYZ', 99, 'bear/trap', 100);\" | sqlite3 /tmp/amber_nginx_test_delivery_01.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 1, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_delivery_01.db");
--- config
location /amber/cache {
  	amber off; 
  	amber_cache_delivery on;
	amber_db /tmp/amber_nginx_test_delivery_01.db;
  	default_type  text/html;	
}
--- user_files
>>> amber/cache/XYZ
I am the walrus
--- request
GET /amber/cache/XYZ
--- response_body
I am the walrus
--- response_headers
Content-Type: bear/trap
--- no_error_log
AMBER error