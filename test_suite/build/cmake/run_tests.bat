
@echo off

set DIR=%~dp0

cd %DIR%..\..
%DIR%jsoncons_tests.exe

cd %DIR%
