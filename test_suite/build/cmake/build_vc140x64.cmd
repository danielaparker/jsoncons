call rmdir build_vc140x64 /S/Q
call mkdir build_vc140x64  & pushd build_vc140x64
call cmake -G "Visual Studio 14 2015 Win64" ..
call popd
call cmake --build build_vc140x64 --config Debug

