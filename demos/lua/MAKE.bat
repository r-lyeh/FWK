#!/bin/bash 2>nul || goto :windows

sh ../../MAKE.bat editor
sh ../../MAKE.bat bind

./luajit.osx   hello.lua
./luajit.linux hello.lua
python hello.py

exit
:windows

if "%1"=="tidy" pushd . && call ..\..\make.bat tidy    && popd && exit /b
if "%1"=="dll"  pushd . && call ..\..\make.bat dll fwk && popd && exit /b
if "%1"=="bind" pushd . && call ..\..\make.bat bind    && popd && exit /b

luajit hello.lua
python hello.py
