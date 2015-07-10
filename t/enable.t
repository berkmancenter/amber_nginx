# vi:filetype=

use lib 'lib';
use Test::Nginx::Socket; # 'no_plan';

# repeat_each(2);

plan tests => repeat_each() * 2 * blocks();

no_long_string();
no_diff;

run_tests();

__DATA__

=== TEST 1: Make sure the test environment is working
--- config
location /foo {
}
--- user_files
>>> foo/foo.html
Hello, world!
--- request
GET /foo/foo.html
--- response_body
Hello, world!

=== TEST 2: Enable Amber
--- config
location /foo {
    amber on;
}
--- user_files
>>> foo/foo.html
Hello, world!
--- request
GET /foo/foo.html
--- response_body
Hello, world!

=== TEST 3: Disable Amber
--- config
location /foo {
    amber off;
}
--- user_files
>>> foo/foo.html
Hello, world!
--- request
GET /foo/foo.html
--- response_body
Hello, world!

=== TEST 4: Amber configuration settings (without country)
--- config
location /foo {
    amber on;
    amber_db '/var/lib/amber/amber.db';
    amber_behavior_up popup;
    amber_behavior_down popup;
    amber_hover_delay_up 1;
    amber_hover_delay_down 1;
}
--- user_files
>>> foo/foo.html
Hello, world!
--- request
GET /foo/foo.html
--- response_body
Hello, world!

=== TEST 5: Amber configuration settings (with country)
--- config
location /foo {
    amber on;
    amber_db '/var/lib/amber/amber.db';
    amber_behavior_up popup;
    amber_behavior_down popup;
    amber_hover_delay_up 1;
    amber_hover_delay_down 1;
    amber_country US;
    amber_country_behavior_up hover;
    amber_country_behavior_down hover;
    amber_country_hover_delay_up 3;
    amber_country_hover_delay_down 3;
}
--- user_files
>>> foo/foo.html
Hello, world!
--- request
GET /foo/foo.html
--- response_body
Hello, world!

