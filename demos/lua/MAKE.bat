#!/bin/bash 2>nul || goto :windows

sh ../../MAKE.bat dll
sh ../../MAKE.bat bind

./luajit.osx   hello.lua
./luajit.linux hello.lua
python hello.py

exit
:windows

call ..\..\make.bat dll
call ..\..\make.bat bind

luajit hello.lua
python hello.py
