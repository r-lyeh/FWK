#!/bin/bash 2>nul || goto :windows

# linux + osx -----------------------------------------------------------------
cd `dirname $0`

git clone https://github.com/assimp/assimp && cd assimp && git checkout 05115b07
cmake -DCMAKE_BUILD_TYPE=Release -DASSIMP_BUILD_TESTS=OFF .
make -j 8
cp bin/libassimp.so ../libassimp.so
cp bin/libassimp.so ../libassimp.so.5
cp bin/libassimp.so ../libassimp.so.5.1.4
cd ..
cc ass2iqe.c -o ass2iqe.linux -O2 -I assimp/include/ libassimp.so -lm

exit


:windows -----------------------------------------------------------------------
@echo off
cd "%~dp0"

git clone https://github.com/assimp/assimp && md assimp\.build && pushd assimp\.build
rem fart -- ..\CMakeLists.txt assimp-${ASSIMP_MSVC_VERSION}-mt.dll assimp.dll
rem fart -- ..\CMakeLists.txt assimp-${ASSIMP_MSVC_VERSION}-mt.lib assimp.lib
rem cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 14 2015" && msbuild assimp.sln -m -p:Configuration=Release -p:PlatformToolset=140
cmake .. -DCMAKE_BUILD_TYPE=Release && (make || msbuild assimp.sln -m -p:Configuration=Release)
popd

xcopy /y assimp\.build\bin\release\*.dll  
xcopy /y assimp\.build\lib\release\*.lib 

copy /y assimp\include\assimp\config.h.in assimp\config.h && fart -- assimp\config.h "cmakedefine" "//#define"
cl ass2iqe.c -I . -I assimp\include assimp-vc142-mt.lib /O2 /Oy /MT /DNDEBUG

rem xcopy assimp\.build\bin\release\*.dll x64\ /s
rem xcopy assimp\.build\lib\release\*.lib x64\ /s
rem rd /q /s assimp
rem fart -r -- * "assimp/" "3rd/3rd_assimp/"
