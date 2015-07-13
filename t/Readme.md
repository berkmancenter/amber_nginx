# Unit Testing #

We use Test::Nginx for unit testing: https://github.com/gitpan/Test-Nginx

To run the unit tests:

* Build the plugin 
* Install Test::Nginx using CPAN
* Run the the tests, adding the nginx executable to the path and specifying the path to the directory to where Test::Nginx is installed. For example:

```TZ=GMT PATH=nginx-1.6.0/objs:$PATH prove -r t -I ~/perl5/lib/perl5```

See the .travis.yml file for an example
