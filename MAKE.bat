#!/bin/bash 2>nul || goto :windows

# linux + osx -----------------------------------------------------------------
cd `dirname $0`

if [ "$(uname)" != "Darwin" ]; then
    # setup (linux)
     sudo apt-get update
     sudo apt-get install g++ ffmpeg libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev  # absolute minimum
    #sudo apt-get install g++ ffmpeg xorg-dev                                                           # memorable, around 100 mib
    #sudo apt-get install g++ ffmpeg xorg-dev libglfw3-dev libassimp-dev clang                          # initial revision

    # pipeline
    #cc art/tools/ass2iqe.c   -o art/tools/ass2iqe  -lm -ldl -lpthread -w -g -lassimp
    #cc art/tools/iqe2iqm.cpp -o art/tools/iqe2iqm  -lm -ldl -lpthread -w -g -lstdc++
    #cc art/tools/mid2wav.c   -o art/tools/mid2wav  -lm -ldl -lpthread -w -g
    #cc art/tools/xml2json.c  -o art/tools/xml2json -lm -ldl -lpthread -w -g

    # framework
    echo fwk            && cc -c fwk.c -w -g

    # demos
    echo demo           && cc -o demo           demo.c           fwk.o -lm -ldl -lpthread -w -g &
    echo demo_cubemap   && cc -o demo_cubemap   demo_cubemap.c   fwk.o -lm -ldl -lpthread -w -g &
    echo demo_collide   && cc -o demo_collide   demo_collide.c   fwk.o -lm -ldl -lpthread -w -g &
    echo demo_model     && cc -o demo_model     demo_model.c     fwk.o -lm -ldl -lpthread -w -g &
    echo demo_scene     && cc -o demo_scene     demo_scene.c     fwk.o -lm -ldl -lpthread -w -g &
    echo demo_shadertoy && cc -o demo_shadertoy demo_shadertoy.c fwk.o -lm -ldl -lpthread -w -g &
    echo demo_sprite    && cc -o demo_sprite    demo_sprite.c    fwk.o -lm -ldl -lpthread -w -g &
    echo demo_video     && cc -o demo_video     demo_video.c     fwk.o -lm -ldl -lpthread -w -g &
    echo demo_script    && cc -o demo_script    demo_script.c    fwk.o -lm -ldl -lpthread -w -g &
fi

if [ "$(uname)" = "Darwin" ]; then
    # setup (osx)
    export SDKROOT=$(xcrun --show-sdk-path)

    # pipeline
    #cc art/tools/ass2iqe.c   -o art/tools/ass2iqe  -w -g -lassimp
    #cc art/tools/iqe2iqm.cpp -o art/tools/iqe2iqm  -w -g -lstdc++
    #cc art/tools/mid2wav.c   -o art/tools/mid2wav  -w -g
    #cc art/tools/xml2json.c  -o art/tools/xml2json -w -g

    # framework
    echo fwk            && cc -c -ObjC fwk.c -w -g

    # demos
    echo demo           && cc -o demo           demo.c           fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_cubemap   && cc -o demo_cubemap   demo_cubemap.c   fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_collide   && cc -o demo_collide   demo_collide.c   fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_model     && cc -o demo_model     demo_model.c     fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_scene     && cc -o demo_scene     demo_scene.c     fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_shadertoy && cc -o demo_shadertoy demo_shadertoy.c fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_sprite    && cc -o demo_sprite    demo_sprite.c    fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_video     && cc -o demo_video     demo_video.c     fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_script    && cc -o demo_script    demo_script.c    fwk.o -framework cocoa -framework iokit -w -g &
fi

exit



:windows -----------------------------------------------------------------------
@echo off

rem setup
if "%Platform%"=="" (
    set Platform=x64
           if exist "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat"
    ) else if exist "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat"
    ) else if exist "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (
              @call "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat"
    ) else if exist "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (
              @call "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat"
    ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat"
    ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat"
    ) else (
        set Platform=mingw64
        echo Warning: Could not find x64 environment variables for Visual Studio 2019/2017/2015/2013. Trying MingW64...

        cd "%~dp0"
        gcc.exe 1> nul 2> nul
        if "%ERRORLEVEL%"=="9009" (
            set Platform=tcc
            echo Warning: Mingw64 not found. Trying tcc...
        )
    )
)

cd "%~dp0"
echo @%~dp0\art\tools\tcc-win\tcc -I %~dp0\art\tools\tcc-win\include_mingw\winapi -I %~dp0\art\tools\tcc-win\include_mingw\ %%* > tcc.bat

if "%Platform%"=="x64" (
    rem pipeline
    rem cl art/tools/ass2iqe.c   /Feart/tools/ass2iqe.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL assimp.lib
    rem cl art/tools/iqe2iqm.cpp /Feart/tools/iqe2iqm.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL
    rem cl art/tools/mid2wav.c   /Feart/tools/mid2wav.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL
    rem cl art/tools/xml2json.c  /Feart/tools/xml2json.exe /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL

    rem framework
    cl fwk.c /nologo /openmp /Zi /c

    rem demos
    cl demo.c           fwk.obj /nologo /openmp /Zi
    cl demo_cubemap.c   fwk.obj /nologo /openmp /Zi
    cl demo_collide.c   fwk.obj /nologo /openmp /Zi
    cl demo_model.c     fwk.obj /nologo /openmp /Zi
    cl demo_scene.c     fwk.obj /nologo /openmp /Zi
    cl demo_shadertoy.c fwk.obj /nologo /openmp /Zi
    cl demo_sprite.c    fwk.obj /nologo /openmp /Zi
    cl demo_video.c     fwk.obj /nologo /openmp /Zi
    cl demo_script.c    fwk.obj /nologo /openmp /Zi
) else if "%Platform%"=="mingw64" (
    rem pipeline
    rem gcc art/tools/ass2iqe.c   -o art/tools/ass2iqe.exe  -w -lassimp
    rem gcc art/tools/iqe2iqm.cpp -o art/tools/iqe2iqm.exe  -w -lstdc++
    rem gcc art/tools/mid2wav.c   -o art/tools/mid2wav.exe  -w
    rem gcc art/tools/xml2json.c  -o art/tools/xml2json.exe -w

    rem framework
    echo fwk            && gcc -c fwk.c -std=c99 -w -g

    rem demos
    echo demo           && gcc -o demo           demo.c           fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_cubemap   && gcc -o demo_cubemap   demo_cubemap.c   fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_collide   && gcc -o demo_collide   demo_collide.c   fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_model     && gcc -o demo_model     demo_model.c     fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_scene     && gcc -o demo_scene     demo_scene.c     fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_shadertoy && gcc -o demo_shadertoy demo_shadertoy.c fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_sprite    && gcc -o demo_sprite    demo_sprite.c    fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_video     && gcc -o demo_video     demo_video.c     fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_script    && gcc -o demo_script    demo_script.c    fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
) else (
    rem pipeline
    rem gcc art/tools/ass2iqe.c   -o art/tools/ass2iqe.exe  -w -lassimp
    rem gcc art/tools/iqe2iqm.cpp -o art/tools/iqe2iqm.exe  -w -lstdc++
    rem gcc art/tools/mid2wav.c   -o art/tools/mid2wav.exe  -w
    rem gcc art/tools/xml2json.c  -o art/tools/xml2json.exe -w

    rem framework
    echo fwk            && tcc -c fwk.c -w

    rem demos
    echo demo           && tcc demo.c           fwk.o -w
    echo demo_cubemap   && tcc demo_cubemap.c   fwk.o -w
    echo demo_collide   && tcc demo_collide.c   fwk.o -w
    echo demo_model     && tcc demo_model.c     fwk.o -w
    echo demo_scene     && tcc demo_scene.c     fwk.o -w
    echo demo_shadertoy && tcc demo_shadertoy.c fwk.o -w
    echo demo_sprite    && tcc demo_sprite.c    fwk.o -w
    echo demo_video     && tcc demo_video.c     fwk.o -w
    echo demo_script    && tcc demo_script.c    fwk.o -w
)

pause
exit /b
