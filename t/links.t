# vi:filetype=

use lib 'lib';
use Test::Nginx::Socket; # 'no_plan';

# repeat_each(2);

plan tests => repeat_each() * 2 * blocks();

no_long_string();
no_diff;

run_tests();

__DATA__

=== TEST 1: Test a file with a link
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

=== TEST 2: Default substitution filter behaves as expected
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

