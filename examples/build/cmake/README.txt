To build the jsoncons examples with cmake:

Windows

To build the tests with vc140,

build_vc140x64.cmd

and to run them

run_examples_vc140x64.cmd

UNIX

From the examples/build/cmake directory

mkdir -p debug
cd debug
cmake -DCMAKE_BUILD_TYPE=debug -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DARCHIVE_INSTALL_DIR=. -G "Unix Makefiles" ..
make

From the examples directory

./build/cmake/debug/jsoncons_examples
