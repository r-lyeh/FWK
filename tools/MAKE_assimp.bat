#!/bin/bash 2>nul || goto :windows

# linux + osx -----------------------------------------------------------------
cd `dirname $0`
 
git clone https://github.com/assimp/assimp

cd assimp && git checkout 05115b07
cmake -DCMAKE_BUILD_TYPE=Release -DASSIMP_BUILD_TESTS=OFF -D_FORTIFY_SOURCE=0 .
make -j 8
cd ..

if [ "$(uname)" != "Darwin" ]; then

cp assimp/bin/libassimp.so libassimp.so
cp assimp/bin/libassimp.so libassimp.so.5
cp assimp/bin/libassimp.so libassimp.so.5.1.4
cc ass2iqe.c -o ass2iqe.linux -O2 -I. -I ../engine/split -I assimp/include/ libassimp.so    -lm
cc iqe2iqm.cpp -o iqe2iqm.linux -O2 -I. -I ../engine/split -lm -lstdc++

else

cp assimp/bin/libassimp.dylib libassimp.dylib
cp assimp/bin/libassimp.dylib libassimp.5.dylib
cp assimp/bin/libassimp.dylib libassimp.5.0.1.dylib
cc ass2iqe.c -o ass2iqe.osx   -O2 -I. -I ../engine/split -I assimp/include/ libassimp.dylib -lm
cc iqe2iqm.cpp -o iqe2iqm.osx   -O2 -I. -I ../engine/split -lm -lstdc++	

fi

exit


:windows -----------------------------------------------------------------------
@echo off
cd "%~dp0"

if not exist assimp-vc14?-mt.lib (
where /q "fart.exe" || (echo cannot find fart.exe tool in path && exit /b)

if not exist assimp (
git clone https://github.com/assimp/assimp && md assimp\.build && pushd assimp\.build && git checkout 05115b07 && popd
)

if not exist assimp (
	echo assimp/ folder not found && exit /b
)

pushd assimp\.build
cmake .. -DCMAKE_BUILD_TYPE=Release && (make || msbuild assimp.sln -m -p:Configuration=Release)
popd

xcopy /y assimp\.build\bin\release\*.dll
xcopy /y assimp\.build\lib\release\*.lib

copy /y assimp\include\assimp\config.h.in assimp\config.h && fart -- assimp\config.h "cmakedefine" "//#define"
)

cl ass2iqe.c -I. -I ..\engine\split -I assimp\include assimp-vc14?-mt.lib /O2 /Oy /MT /DNDEBUG

cl iqe2iqm.cpp /O2 /Oy /MT /DNDEBUG
