The test_suite requires boost

To build the x64 boost libraries with, for example, vc140 (Visual Studio 2015):

bootstrap.bat
b2 toolset=msvc-14.0 --build-type=complete --abbreviate-paths architecture=x86 address-model=64 install -j4 stage

To build the jsoncons test_suite with cmake:

Set the environmental variables:

BOOST_ROOT
BOOST_INCLUDEDIR       - Preferred include directory 
BOOST_LIBRARYDIR       - Preferred library directory e.g. <prefix>/lib/vc140x64

To build the tests with vc140,

build_vc140x64.cmd

and to run them

run_tests_vc140x64.cmd

