# vi:filetype=

use lib 'lib';
use Test::Nginx::Socket; # 'no_plan';

# repeat_each(2);

plan tests => repeat_each() * 3 * blocks();

no_long_string();
#no_diff;

run_tests();


__DATA__

=== TEST 1: Test a file with a link - no error if database setup correctly
--- init
system("cp t/test.db /tmp/amber_nginx_test_links_01.db");
--- config
location /foo {
	amber on;
	amber_db /tmp/amber_nginx_test_links_01.db;
}
--- user_files
>>> foo/foo.html
<html><head><title>Doghouse</title></head><body>
Welcome to the <a href="http://dog.com">doghouse</a>
</body></html>
--- request
GET /foo/foo.html
--- response_body
<html><head><title>Doghouse</title></head><body>
Welcome to the <a href="http://dog.com">doghouse</a>
</body></html>
--- no_error_log
AMBER error

=== TEST 2: Test a file with a link - return annotated result with popup (UP) 
--- init
system("cp t/test.db /tmp/amber_nginx_test_links_02.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'cache/XYZ', 99, 'text/html', 100);\" | sqlite3 /tmp/amber_nginx_test_links_02.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 1, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_links_02.db");
--- config
location /foo {
	amber on;
	amber_db /tmp/amber_nginx_test_links_02.db;
	amber_behavior_up popup;

	subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';
}
--- user_files
>>> foo/foo.html
<html><head><title>Doghouse</title></head><body>
Welcome to the <a href="http://dog.com">doghouse</a>
</body></html>
--- request
GET /foo/foo.html
--- response_body
<html><head><title>Doghouse</title><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head><body>
Welcome to the <a data-versionurl='http://localhost:1984/cache/XYZ' data-versiondate='1970-01-01T00:01:39+0000' data-amber-behavior='up popup' href="http://dog.com">doghouse</a>
</body></html>
--- no_error_log
AMBER error

=== TEST 3: Test a file with a link - return annotated result with hover (UP)
--- init
system("cp t/test.db /tmp/amber_nginx_test_links_03.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'cache/XYZ', 99, 'text/html', 100);\" | sqlite3 /tmp/amber_nginx_test_links_03.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 1, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_links_03.db");
--- config
location /foo {
	amber on;
	amber_db /tmp/amber_nginx_test_links_03.db;
	amber_behavior_up hover;
	amber_hover_delay_up 4;
	subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';
}
--- user_files
>>> foo/foo.html
<html><head><title>Doghouse</title></head><body>
Welcome to the <a href="http://dog.com">doghouse</a>
</body></html>
--- request
GET /foo/foo.html
--- response_body
<html><head><title>Doghouse</title><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head><body>
Welcome to the <a data-versionurl='http://localhost:1984/cache/XYZ' data-versiondate='1970-01-01T00:01:39+0000' data-amber-behavior='up hover:4' href="http://dog.com">doghouse</a>
</body></html>
--- no_error_log
AMBER error

=== TEST 4: Test a file with a link - return annotated result with hover (DOWN)
--- init
system("cp t/test.db /tmp/amber_nginx_test_links_04.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'cache/XYZ', 99, 'text/html', 100);\" | sqlite3 /tmp/amber_nginx_test_links_04.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 0, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_links_04.db");
--- config
location /foo {
	amber on;
	amber_db /tmp/amber_nginx_test_links_04.db;
	amber_behavior_down hover;
	amber_hover_delay_down 3;
	subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';
}
--- user_files
>>> foo/foo.html
<html><head><title>Doghouse</title></head><body>
Welcome to the <a href="http://dog.com">doghouse</a>
</body></html>
--- request
GET /foo/foo.html
--- response_body
<html><head><title>Doghouse</title><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head><body>
Welcome to the <a data-versionurl='http://localhost:1984/cache/XYZ' data-versiondate='1970-01-01T00:01:39+0000' data-amber-behavior='down hover:3' href="http://dog.com">doghouse</a>
</body></html>
--- no_error_log
AMBER error


=== TEST 5: Test a file with a link - return annotated result with popup (DOWN)
--- init
system("cp t/test.db /tmp/amber_nginx_test_links_05.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'cache/XYZ', 99, 'text/html', 100);\" | sqlite3 /tmp/amber_nginx_test_links_05.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 0, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_links_05.db");
--- config
location /foo {
	amber on;
	amber_db /tmp/amber_nginx_test_links_05.db;
	amber_behavior_down popup;
	subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';
}
--- user_files
>>> foo/foo.html
<html><head><title>Doghouse</title></head><body>
Welcome to the <a href="http://dog.com">doghouse</a>
</body></html>
--- request
GET /foo/foo.html
--- response_body
<html><head><title>Doghouse</title><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head><body>
Welcome to the <a data-versionurl='http://localhost:1984/cache/XYZ' data-versiondate='1970-01-01T00:01:39+0000' data-amber-behavior='down popup' href="http://dog.com">doghouse</a>
</body></html>
--- no_error_log
AMBER error

=== TEST 6: Test a file with a link - return annotated result with country-specific-behavior of popup (DOWN)
--- init
system("cp t/test.db /tmp/amber_nginx_test_links_06.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'cache/XYZ', 99, 'text/html', 100);\" | sqlite3 /tmp/amber_nginx_test_links_06.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 0, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_links_06.db");
--- config
location /foo {
	amber on;
	amber_db /tmp/amber_nginx_test_links_06.db;
	amber_behavior_down hover;
	amber_hover_delay_down 3;
	amber_country_behavior_down popup;
	amber_country IR;
	subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';
}
--- user_files
>>> foo/foo.html
<html><head><title>Doghouse</title></head><body>
Welcome to the <a href="http://dog.com">doghouse</a>
</body></html>
--- request
GET /foo/foo.html
--- response_body
<html><head><title>Doghouse</title><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head><body>
Welcome to the <a data-versionurl='http://localhost:1984/cache/XYZ' data-versiondate='1970-01-01T00:01:39+0000' data-amber-behavior='down hover:3,IR down popup' href="http://dog.com">doghouse</a>
</body></html>
--- no_error_log
AMBER error


=== TEST 7: Test a file with a link - return annotated result with country-specific-behavior of hover (DOWN)
--- init
system("cp t/test.db /tmp/amber_nginx_test_links_07.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'cache/XYZ', 99, 'text/html', 100);\" | sqlite3 /tmp/amber_nginx_test_links_07.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 0, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_links_07.db");
--- config
location /foo {
	amber on;
	amber_db /tmp/amber_nginx_test_links_07.db;
	amber_behavior_down hover;
	amber_hover_delay_down 3;
	amber_country_behavior_down hover;
	amber_country_hover_delay_down 9;
	amber_country IR;
	subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';
}
--- user_files
>>> foo/foo.html
<html><head><title>Doghouse</title></head><body>
Welcome to the <a href="http://dog.com">doghouse</a>
</body></html>
--- request
GET /foo/foo.html
--- response_body
<html><head><title>Doghouse</title><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head><body>
Welcome to the <a data-versionurl='http://localhost:1984/cache/XYZ' data-versiondate='1970-01-01T00:01:39+0000' data-amber-behavior='down hover:3,IR down hover:9' href="http://dog.com">doghouse</a>
</body></html>
--- no_error_log
AMBER error

=== TEST 8: Test a file with a link - return annotated result with country-specific-behavior of hover (UP)
--- init
system("cp t/test.db /tmp/amber_nginx_test_links_08.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'cache/XYZ', 99, 'text/html', 100);\" | sqlite3 /tmp/amber_nginx_test_links_08.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 1, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_links_08.db");
--- config
location /foo {
	amber on;
	amber_db /tmp/amber_nginx_test_links_08.db;
	amber_behavior_up hover;
	amber_hover_delay_up 3;
	amber_country_behavior_up hover;
	amber_country_hover_delay_up 1;
	amber_country IR;
	subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';
}
--- user_files
>>> foo/foo.html
<html><head><title>Doghouse</title></head><body>
Welcome to the <a href="http://dog.com">doghouse</a>
</body></html>
--- request
GET /foo/foo.html
--- response_body
<html><head><title>Doghouse</title><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head><body>
Welcome to the <a data-versionurl='http://localhost:1984/cache/XYZ' data-versiondate='1970-01-01T00:01:39+0000' data-amber-behavior='up hover:3,IR up hover:1' href="http://dog.com">doghouse</a>
</body></html>
--- no_error_log
AMBER error


=== TEST 9: Test a file with a link - return annotated result with country-specific-behavior of popup (UP)
--- init
system("cp t/test.db /tmp/amber_nginx_test_links_09.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'cache/XYZ', 99, 'text/html', 100);\" | sqlite3 /tmp/amber_nginx_test_links_09.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 1, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_links_09.db");
--- config
location /foo {
	amber on;
	amber_db /tmp/amber_nginx_test_links_09.db;
	amber_behavior_up hover;
	amber_hover_delay_up 3;
	amber_country_behavior_up popup;
	amber_country IR;
	subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';
}
--- user_files
>>> foo/foo.html
<html><head><title>Doghouse</title></head><body>
Welcome to the <a href="http://dog.com">doghouse</a>
</body></html>
--- request
GET /foo/foo.html
--- response_body
<html><head><title>Doghouse</title><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head><body>
Welcome to the <a data-versionurl='http://localhost:1984/cache/XYZ' data-versiondate='1970-01-01T00:01:39+0000' data-amber-behavior='up hover:3,IR up popup' href="http://dog.com">doghouse</a>
</body></html>
--- no_error_log
AMBER error

=== TEST 10: Test a file with a link, with single-quotes 
--- init
system("cp t/test.db /tmp/amber_nginx_test_links_10.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'cache/XYZ', 99, 'text/html', 100);\" | sqlite3 /tmp/amber_nginx_test_links_10.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 1, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_links_10.db");
--- config
location /foo {
	amber on;
	amber_db /tmp/amber_nginx_test_links_10.db;
	amber_behavior_up popup;

	subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';
}
--- user_files
>>> foo/foo.html
<html><head><title>Doghouse</title></head><body>
Welcome to the <a href='http://dog.com'>doghouse</a>
</body></html>
--- request
GET /foo/foo.html
--- response_body
<html><head><title>Doghouse</title><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head><body>
Welcome to the <a data-versionurl='http://localhost:1984/cache/XYZ' data-versiondate='1970-01-01T00:01:39+0000' data-amber-behavior='up popup' href='http://dog.com'>doghouse</a>
</body></html>
--- no_error_log
AMBER error

=== TEST 11: Test a file with a link, with additional attributes 
--- init
system("cp t/test.db /tmp/amber_nginx_test_links_10.db");
system("echo \"insert into amber_cache values('XYZ', 'http://dog.com', 'cache/XYZ', 99, 'text/html', 100);\" | sqlite3 /tmp/amber_nginx_test_links_10.db");
system("echo \"insert into amber_check values('XYZ', 'http://dog.com', 1, 100000, 200000, '');\" | sqlite3 /tmp/amber_nginx_test_links_10.db");
--- config
location /foo {
	amber on;
	amber_db /tmp/amber_nginx_test_links_10.db;
	amber_behavior_up popup;

	subs_filter '</head>' '<script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head>';
}
--- user_files
>>> foo/foo.html
<html><head><title>Doghouse</title></head><body>
Welcome to the <a data-foo='bar' href="http://dog.com">doghouse</a>
</body></html>
--- request
GET /foo/foo.html
--- response_body
<html><head><title>Doghouse</title><script type="text/javascript" src="/amber/js/amber.js"></script><link rel="stylesheet" type="text/css" href="/amber/css/amber.css"></head><body>
Welcome to the <a data-foo='bar' data-versionurl='http://localhost:1984/cache/XYZ' data-versiondate='1970-01-01T00:01:39+0000' data-amber-behavior='up popup' href="http://dog.com">doghouse</a>
</body></html>
--- no_error_log
AMBER error

