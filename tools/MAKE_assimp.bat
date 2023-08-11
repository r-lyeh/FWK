#!/bin/bash 2>nul || goto :windows

# linux + osx -----------------------------------------------------------------
cd `dirname $0`

git clone https://github.com/assimp/assimp && cd assimp && git checkout 05115b07
cmake -DCMAKE_BUILD_TYPE=Release -DASSIMP_BUILD_TESTS=OFF .
make -j 8
cp bin/libassimp.so    ../libassimp.so
cp bin/libassimp.so    ../libassimp.so.5
cp bin/libassimp.so    ../libassimp.so.5.1.4
cp bin/libassimp.dylib ../libassimp.dylib
cp bin/libassimp.dylib ../libassimp.5.dylib
cp bin/libassimp.dylib ../libassimp.5.0.1.dylib
cd ..

cc ass2iqe.c -o ass2iqe.linux -O2 -I assimp/include/ libassimp.so    -lm
cc ass2iqe.c -o ass2iqe.osx   -O2 -I assimp/include/ libassimp.dylib -lm

exit


:windows -----------------------------------------------------------------------
@echo off
cd "%~dp0"

git clone https://github.com/assimp/assimp && md assimp\.build && pushd assimp\.build && git checkout 05115b07
cmake .. -DCMAKE_BUILD_TYPE=Release && (make || msbuild assimp.sln -m -p:Configuration=Release)
popd

xcopy /y assimp\.build\bin\release\*.dll  
xcopy /y assimp\.build\lib\release\*.lib 

copy /y assimp\include\assimp\config.h.in assimp\config.h && fart -- assimp\config.h "cmakedefine" "//#define"
cl ass2iqe.c -I . -I assimp\include assimp-vc14?-mt.lib /O2 /Oy /MT /DNDEBUG
