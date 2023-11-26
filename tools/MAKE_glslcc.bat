#!/bin/bash 2>nul || goto :windows

# linux + osx -----------------------------------------------------------------
cd `dirname $0`

git clone https://github.com/septag/glslcc && mkdir glslcc/.build
cd glslcc/.build
cmake .. -B . -DCMAKE_BUILD_TYPE=Release
make -j 8
# cp -f glslcc/.build/src/Release/glslcc.exe .
cd ".."

exit


:windows -----------------------------------------------------------------------
@echo off
cd "%~dp0"

if not exist glslcc (
git clone https://github.com/septag/glslcc
)

if "%1"=="debug" (
	if not exist glslcc\.debug md glslcc\.debug
	pushd glslcc\.debug
	cmake .. -B . && (make || msbuild glslcc.sln)
	popd
	xcopy /y glslcc\.debug\src\Debug\glslcc.exe
) else (
	if not exist glslcc\.build md glslcc\.build
	pushd glslcc\.build
	cmake .. -B . -DCMAKE_BUILD_TYPE=Release && (make || msbuild glslcc.sln -m -p:Configuration=Release)
	popd
	xcopy /y glslcc\.build\src\Release\glslcc.exe
)
