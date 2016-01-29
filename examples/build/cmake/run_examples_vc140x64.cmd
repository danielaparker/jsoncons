@echo off

set DIR=%~dp0

cd %DIR%..\..

call "build/cmake/build_vc140x64/Debug/jsoncons_examples.exe"

cd %DIR%
