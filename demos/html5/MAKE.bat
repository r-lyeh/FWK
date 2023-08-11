#!/bin/bash 2>nul || goto :windows

if [ "$1" = "" ]; then
	sh MAKE.bat demo_ui.c
	exit
fi

## clone emscripten sdk
git clone https://github.com/emscripten-core/emsdk ../../../../../emsdk
pushd ../../../../../emsdk
	./emsdk install  3.0.0 ## latest
	./emsdk activate 3.0.0 ## latest
	source ./emsdk_env.sh
popd

## cook art
if [ "$(uname)" = "Darwin" ]; then
    chmod +x ../../tools/cook.osx
    ../../tools/cook.osx   --cook-jobs=1 --cook-ini=../../tools/cook.ini
    cp .art[00].zip index.data
else
    chmod +x ../../tools/cook.linux
    ../../tools/cook.linux --cook-jobs=1 --cook-ini=../../tools/cook.ini
    cp .art[00].zip index.data
fi

## host webserver, compile and launch
python -m http.server --bind 127.0.0.1 8000 1> /dev/null 2> /dev/null &
emcc $@ -g ../../engine/fwk.c -I../../engine -o index.html -s FULL_ES3 -s USE_PTHREADS -s USE_GLFW=3 -s SINGLE_FILE=1 -s PRECISE_F32=1 -s TOTAL_MEMORY=256mb -s ENVIRONMENT=worker,web --shell-file template.html -Wfatal-errors --preload-file .art[00].zip -s ALLOW_MEMORY_GROWTH=1 -lidbfs.js && xdg-open http://localhost:8000/index.html

exit

:windows

if "%1"=="" MAKE.bat demo_collide.c

rem clone emscripten sdk
if not exist "emsdk" (
	git clone https://github.com/emscripten-core/emsdk emsdk
	pushd emsdk
		call emsdk install  3.0.0 && rem latest
		call emsdk activate 3.0.0 && rem latest
	popd
)
if "%EMSDK%"=="" call emsdk\emsdk_env.bat --system

rem cook art
..\..\tools\cook.exe --cook-jobs=1 --cook-ini=..\..\tools\cook.ini
copy .art[00].zip index.data

rem host webserver:8000 if not open , compile and launch
netstat /ano | find /i "listening" | find ":8000" >nul 2>nul && (
	rem start python -m http.server --bind 127.0.0.1 8000
) || (
	start python -m http.server --bind 127.0.0.1 8000
)

emcc %* -g ..\..\engine\fwk.c -I..\..\engine -o index.html -pthread -s FULL_ES3 -s USE_PTHREADS -s USE_GLFW=3 -s SINGLE_FILE=1 -s PRECISE_F32=1 -s TOTAL_MEMORY=256mb -s ENVIRONMENT=worker,web --shell-file template.html -Wfatal-errors --preload-file .art[00].zip -s ALLOW_MEMORY_GROWTH=1 -lidbfs.js && start "" http://localhost:8000/index.html
