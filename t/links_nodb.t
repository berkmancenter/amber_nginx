# vi:filetype=

use lib 'lib';
use Test::Nginx::Socket; # 'no_plan';

# repeat_each(2);

plan tests => repeat_each() * 3 * blocks();

no_long_string();
no_diff;

run_tests();

__DATA__

=== TEST 1: Test a file with a link - should work, but log error about DB configuration
--- config
location /foo {
	amber on;
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
--- error_log: AMBER error creating sqlite prepared statement 

=== TEST 2: Default substitution filter behaves as expected - should work, but log error about DB configuration
--- config
location /foo {
	amber on;
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
Welcome to the <a href="http://dog.com">doghouse</a>
</body></html>
--- error_log: AMBER error creating sqlite prepared statement 

=== TEST 3: Test a file with a link - where database is not writeable
--- config
location /foo {
	amber on;
	amber_db /foo/bar/doesnotexist.db;
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
--- error_log: AMBER error opening sqlite database 
