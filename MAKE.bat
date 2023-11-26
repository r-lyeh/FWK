#!/bin/bash 2>nul || goto :windows

# linux + osx -----------------------------------------------------------------
cd `dirname $0`

# copy demos to root folder. local changes are preserved
# cp -n demos/*.c .

# rem tests
# clang editor.c -I. -lm -lX11 -g -fsanitize=address,undefined && ./a.out
# cl editor.c -I. -fsanitize=address /DEBUG /Zi && editor

# tidy environment
if [ "$1" = "tidy" ]; then
    rm 0?-* 2> /dev/null
    rm fwk.o 2> /dev/null
    rm .art*.zip 2> /dev/null
    rm demos/lua/.art*.zip 2> /dev/null
    rm demos/lua/libfwk* 2> /dev/null
    rm demos/html5/.art*.zip 2> /dev/null
    rm fwk_*.* 2> /dev/null
    rm 3rd_*.* 2> /dev/null
    rm libfwk* 2> /dev/null
    rm -rf *.dSYM 2> /dev/null
    rm *.png 2> /dev/null
    rm *.mp4 2> /dev/null
    rm *.log 2> /dev/null
    rm editor 2> /dev/null
    rm temp_* 2> /dev/null
    rm hello 2> /dev/null
    exit
fi
# shortcuts for split & join scripts
if [ "$1" = "split" ]; then
    sh tools/split.bat
    exit
fi
if [ "$1" = "join" ]; then
    sh tools/join.bat
    exit
fi
# cook
if [ "$1" = "cook" ]; then
    cc -o cook tools/cook.c -Iengine
    ./cook
    exit
fi
# sync
if [ "$1" = "sync" ]; then
    git reset --hard HEAD^1 && git pull
    sh MAKE.bat tidy
    exit
fi

export dll=dll
export build=dev
export args=
export cc=cc

while [ $# -ge 1 ]; do
    if [ "$1" = "help" ]; then
        echo sh MAKE.bat
        echo sh MAKE.bat [gcc,clang,tcc] [dbg,dev,rel] [dll,static]
        echo sh MAKE.bat [tidy]
        echo sh MAKE.bat [split,join]
        echo sh MAKE.bat [cook]
        echo sh MAKE.bat [proj]
        exit
    fi
    if [ "$1" = "dll" ]; then
        export dll=dll
    fi
    if [ "$1" = "static" ]; then
        export dll=static
    fi
    if [ "$1" = "dbg" ]; then
        export build=dbg
        export flags="-O0 -g"
    fi
    if [ "$1" = "dev" ]; then
        export build=dev
        export flags="-O1 -g -DNDEBUG=1"
    fi
    if [ "$1" = "rel" ]; then
        export build=rel
        export flags="-O2    -DNDEBUG=2"
    fi
    if [ "$1" = "ret" ]; then
        export build=ret
        export flags="-O3    -DNDEBUG=3 -DENABLE_RETAIL"
    fi
    if [ "$1" = "gcc" ]; then
        export cc=gcc
    fi
    if [ "$1" = "clang" ]; then
        export cc=clang
    fi
    if [ "$1" = "tcc" ]; then
        export cc="tcc -D__STDC_NO_VLA__"
    fi
    if [ "$1" = "proj" ]; then
        if [ "$(uname)" != "Darwin" ]; then
            chmod +x tools/premake5.linux
            tools/premake5.linux gmake
            tools/premake5.linux ninja
            exit
        fi
        if [ "$(uname)" = "Darwin" ]; then
            chmod +x tools/premake5.osx
            tools/premake5.osx xcode4
            tools/premake5.osx ninja
            exit
        fi
    fi
    if [ "$1" = "--" ]; then
        shift
        export args=$*
        shift $#
    fi
    if [ $# -ge 1 ]; then
        shift
    fi
done

if [ "$(uname)" != "Darwin" ]; then

    # setup (ArchLinux)
    [ ! -f ".setup" ] && sudo pacman -S --noconfirm tcc && echo>.setup
    # setup (Debian, Ubuntu, etc)
    [ ! -f ".setup" ] && sudo apt-get -y update
    [ ! -f ".setup" ] && sudo apt-get -y install tcc libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev && echo>.setup       # absolute minimum
    #                      sudo apt-get -y install clang xorg-dev                                                                             # memorable, around 100 mib
    #                      sudo apt-get -y install clang xorg-dev libglfw3-dev libassimp-dev gcc                                              # initial revision
    #                      sudo apt-get -y install ffmpeg || (sudo apt-get install snapd && sudo snap install ffmpeg)                         # variant

    # pipeline
    #cc tools/ass2iqe.c   -o tools/ass2iqe.linux  -lm -ldl -lpthread -w -g -lassimp
    #cc tools/iqe2iqm.cpp -o tools/iqe2iqm.linux  -lm -ldl -lpthread -w -g -lstdc++
    #cc tools/mid2wav.c   -o tools/mid2wav.linux  -lm -ldl -lpthread -w -g

    # change permissions of precompiled tools binaries because of 'Permission denied' runtime error (@procedural)
    chmod +x tools/ass2iqe.linux
    chmod +x tools/ase2ini.linux
    chmod +x tools/cook.linux
    chmod +x tools/cuttlefish.linux
    chmod +x tools/ffmpeg.linux
    chmod +x tools/furnace.linux
    chmod +x tools/iqe2iqm.linux
    chmod +x tools/mid2wav.linux
    chmod +x tools/mod2wav.linux
    chmod +x tools/PVRTexToolCLI.linux
    chmod +x tools/sfxr2wav.linux
    chmod +x tools/xlsx2ini.linux
    chmod +x tools/premake5.linux
    chmod +x tools/ninja.linux
    chmod +x tools/ase2ini.linux
    chmod +x tools/ark.linux
    chmod +x demos/lua/luajit.linux

    export args="-lm -ldl -lpthread -lX11 -w -Iengine/ $args"
    echo build=$build, type=$dll, cc=$cc, args=$args

    # framework (as dynamic library)
    if [ "$dll" = "dll" ]; then
        echo libfwk.so  && $cc -o libfwk.so engine/fwk.c -shared -fPIC $flags $args
        cp libfwk.so demos/lua/
        export import="libfwk.so -Wl,-rpath,./"
    else
    # framework (static)
        echo fwk        && $cc -c engine/fwk.c -w    $flags $args
        export import=fwk.o
    fi

    # editor
    echo editor        && $cc -o editor        engine/editor.c       $flags $import $args &

    # demos
    echo hello         && $cc -o hello         hello.c               $flags         $args &
    echo 00-loop       && $cc -o 00-loop       demos/00-loop.c       $flags $import $args &
    echo 00-script     && $cc -o 00-script     demos/00-script.c     $flags $import $args &
    echo 01-demo2d     && $cc -o 01-demo2d     demos/01-demo2d.c     $flags $import $args &
    echo 01-ui         && $cc -o 01-ui         demos/01-ui.c         $flags $import $args &
    echo 01-easing     && $cc -o 01-easing     demos/01-easing.c     $flags $import $args &
    echo 01-font       && $cc -o 01-font       demos/01-font.c       $flags $import $args &
    echo 02-ddraw      && $cc -o 02-ddraw      demos/02-ddraw.c      $flags $import $args &
    echo 02-frustum    && $cc -o 02-frustum    demos/02-frustum.c    $flags $import $args &
    echo 03-anims      && $cc -o 03-anims      demos/03-anims.c      $flags $import $args &
    echo 04-actor      && $cc -o 04-actor      demos/04-actor.c      $flags $import $args &
    echo 06-scene      && $cc -o 06-scene      demos/06-scene.c      $flags $import $args &
    echo 07-netsync    && $cc -o 07-netsync    demos/07-netsync.c    $flags $import $args &
    echo 06-material   && $cc -o 06-material   demos/06-material.c   $flags $import $args &
    echo 07-network    && $cc -o 07-network    demos/07-network.c    $flags $import $args &
    echo 08-audio      && $cc -o 08-audio      demos/08-audio.c      $flags $import $args &
    echo 08-video      && $cc -o 08-video      demos/08-video.c      $flags $import $args &
    echo 09-cubemap    && $cc -o 09-cubemap    demos/09-cubemap.c    $flags $import $args &
    echo 09-shadertoy  && $cc -o 09-shadertoy  demos/09-shadertoy.c  $flags $import $args
fi

if [ "$(uname)" = "Darwin" ]; then
    # setup (osx)
    export SDKROOT=$(xcrun --show-sdk-path)
    # brew install glfw

    # pipeline
    #cc tools/ass2iqe.c   -o tools/ass2iqe.osx  -w -g -lassimp
    #cc tools/iqe2iqm.cpp -o tools/iqe2iqm.osx  -w -g -lstdc++
    #cc tools/mid2wav.c   -o tools/mid2wav.osx  -w -g

    # change permissions of precompiled tools binaries because of 'Permission denied' runtime error (@procedural)
    chmod +x tools/ass2iqe.osx
    chmod +x tools/ase2ini.osx
    chmod +x tools/cook.osx
    chmod +x tools/ark.osx
    chmod +x tools/cuttlefish.osx
    chmod +x tools/ffmpeg.osx
    chmod +x tools/furnace.osx
    chmod +x tools/iqe2iqm.osx
    chmod +x tools/mid2wav.osx
    chmod +x tools/mod2wav.osx
    chmod +x tools/PVRTexToolCLI.osx
    chmod +x tools/sfxr2wav.osx
    chmod +x tools/xlsx2ini.osx
    chmod +x tools/premake5.osx
    chmod +x tools/ninja.osx
    chmod +x demos/lua/luajit.osx

    export args="-w -Iengine/ -framework cocoa -framework iokit -framework CoreFoundation -framework CoreAudio -framework AudioToolbox $args"
    echo build=$build, type=$dll, cc=$cc, args=$args

    # framework (as dynamic library)
    if [ "$dll" = "dll" ]; then
        echo libfwk    && cc -ObjC -dynamiclib -o libfwk.dylib engine/fwk.c $flags $args
        cp libfwk.dylib demos/lua
        export import=libfwk.dylib
    else
    # framework
        echo fwk       && cc -c -ObjC engine/fwk.c $flags $args
        export import=fwk.o
    fi

    # editor
    echo editor        && cc -o editor        engine/editor.c       $import $flags $args &

    # demos
    echo hello         && cc -o hello -ObjC   hello.c                       $flags $args &
    echo 00-loop       && cc -o 00-loop       demos/00-loop.c       $import $flags $args &
    echo 00-script     && cc -o 00-script     demos/00-script.c     $import $flags $args &
    echo 01-demo2d     && cc -o 01-demo2d     demos/01-demo2d.c     $import $flags $args &
    echo 01-ui         && cc -o 01-ui         demos/01-ui.c         $import $flags $args &
    echo 01-easing     && cc -o 01-easing     demos/01-easing.c     $import $flags $args &
    echo 01-font       && cc -o 01-font       demos/01-font.c       $import $flags $args &
    echo 02-ddraw      && cc -o 02-ddraw      demos/02-ddraw.c      $import $flags $args &
    echo 02-frustum    && cc -o 02-frustum    demos/02-frustum.c    $import $flags $args &
    echo 03-anims      && cc -o 03-anims      demos/03-anims.c      $import $flags $args &
    echo 04-actor      && cc -o 04-actor      demos/04-actor.c      $import $flags $args &
    echo 06-scene      && cc -o 06-scene      demos/06-scene.c      $import $flags $args &
    echo 07-netsync    && cc -o 07-netsync    demos/07-netsync.c    $import $flags $args &
    echo 06-material   && cc -o 06-material   demos/06-material.c   $import $flags $args &
    echo 07-network    && cc -o 07-network    demos/07-network.c    $import $flags $args &
    echo 08-audio      && cc -o 08-audio      demos/08-audio.c      $import $flags $args &
    echo 08-video      && cc -o 08-video      demos/08-video.c      $import $flags $args &
    echo 09-cubemap    && cc -o 09-cubemap    demos/09-cubemap.c    $import $flags $args &
    echo 09-shadertoy  && cc -o 09-shadertoy  demos/09-shadertoy.c  $import $flags $args
fi

exit


:: -----------------------------------------------------------------------------
:windows

@echo off
cd /d "%~dp0"

rem show help
if "%1"=="-?"     goto showhelp
if "%1"=="-h"     goto showhelp
if "%1"=="--help" goto showhelp
if "%1"=="-help"  goto showhelp
if "%1"=="help" (
    :showhelp
    echo %0                   ; compile hello. uses `dll dev` settings
    echo %0 [all]             ; build everything
    echo %0 [run]             ; run everything built
    echo %0 [bind]            ; generate lua bindings
    echo %0 [cook]            ; generate cooked zip files. uses tools/cook.ini cookbook
    echo %0 [docs]            ; generate engine/fwk.html file
    echo %0 [fuse]            ; fuse all binaries and cooked zipfiles found together
    echo %0 [help]            ; show this screen
    echo %0 [proj]            ; generate a xcode/gmake/ninja/visual studio solution
    echo %0 [sync]            ; sync repo to latest
    echo %0 [tidy]            ; clean up temp files
    echo %0 [test]            ; perform different checks
    echo %0 [join]            ; merge engine/* ^<- engine/joint/* ^<- engine/split/*
    echo %0 [split]           ; merge engine/* -^> engine/joint/* -^> engine/split/*
    echo %0 [joint]           ; merge engine/* -^> engine/joint/*
    echo %0 [cl^|tcc^|cc^|gcc^|clang^|clang-cl] [dbg^|dev^|rel^|ret] [static^|dll] [nofwk^|nodemos^|editor] [-- args]
    echo    cl       \
    echo    tcc      ^|
    echo    cc       ^| select compiler. must be accessible in PATH
    echo    gcc      ^| (autodetected if no option is provided^)
    echo    clang    ^|
    echo    clang-cl /
    echo    dbg      \   debug build: [x] ASAN [x] poison [x] asserts [x] profiler [x] suite [x] symbols                    [ ] zero optimizations
    echo    dev      ^| develop build: [ ] ASAN [x] poison [x] asserts [x] profiler [ ] suite [x] symbols                    [*] some optimizations (default^)
    echo    rel      ^| release build: [ ] ASAN [ ] poison [ ] asserts [ ] profiler [ ] suite [x] symbols (cl,clang-cl only^) [x] many optimizations
    echo    ret      /  retail build: same than release build above plus no-console, no-printf, no-profiler, no-debug-panel, no-cook, no-editor, no-title-stats...
    echo    static   \ link fwk as static library
    echo    dll      / link fwk as dynamic library (dll^) (default^)
    echo    fwk      \ compile framework
    echo    nofwk    / do not compile framework
    echo    demos    \ compile demos
    echo    nodemos  / do not compile demos
    echo    editor   \ compile editor
    echo    noeditor / do not compile editor
    echo    all      ^> compile everything: same than `fwk demos editor` setting
    echo    args     ^> after `--` separator is found, pass all remaining arguments to compiler as-is
    echo.
    exit /b
)

rem sync repo to latest
if "%1"=="sync" (
    call MAKE.bat tidy
    git reset --hard HEAD~1 && git pull
    exit /b
)

rem cook asset files
if "%1"=="cook" (
    echo Cooking assets...
    rem generate cooker twice: use multi-threaded version if available (cl). then cook.
    rem call tools\tcc tools\cook.c -Iengine engine\fwk.c
    rem             cl tools\cook.c -Iengine engine\fwk.c
    rem cook
    del cook*.csv 2> nul
    tools\cook --cook-stats
    (type *.csv | sort /R > cook.csv) 2> nul

    exit /b
)
rem generate bindings
if "%1"=="bind" (
    rem luajit
    tools\luajit tools\luajit_make_bindings.lua > fwk.lua
    move /y fwk.lua engine\
    copy /y engine\fwk.lua demos\lua\
    copy /y engine\fwk.py demos\lua\

    exit /b
)
rem generate documentation
if "%1"=="docs" (
    rem set symbols...
    git describe --tags --abbrev=0 > info.obj
    set /p VERSION=<info.obj
    git rev-list --count --first-parent HEAD > info.obj
    set /p GIT_REVISION=<info.obj
    git rev-parse --abbrev-ref HEAD > info.obj
    set /p GIT_BRANCH=<info.obj
    date /t > info.obj
    set /p LAST_MODIFIED=<info.obj

    rem ...and generate docs
    cl tools\docs\docs.c engine\fwk.c -Iengine %2
    docs engine\fwk.h --excluded=3rd_glad.h,fwk.h,fwk_compat.h, > fwk.html
    move /y fwk.html engine\

    exit /b
)
rem run all generated executables
if "%1"=="run" (
    for %%i in (*.exe) do start /wait %%i
    exit /b
)

rem generate single-header distribution
if "%1"=="joint" (
    echo // This file is intended to be consumed by a compiler. Do not read.  > fwk.h
    echo // **Browse to any of the sources in engine/split/ folder instead** >> fwk.h
    echo // ---------------------------------------------------------------- >> fwk.h
    echo // #define FWK_IMPLEMENTATION early in **one** C file to unroll the >> fwk.h
    echo // implementation. The symbol must be defined in a C (not C++^) file>> fwk.h
    echo // ---------------------------------------------------------------- >> fwk.h
    echo #pragma once                                                        >> fwk.h
    type engine\split\3rd_icon_md.h                                          >> fwk.h
    type engine\split\3rd_glad.h                                             >> fwk.h
    type engine\fwk.h                                                        >> fwk.h
    echo #ifdef FWK_IMPLEMENTATION                                           >> fwk.h
    echo #define FWK_3RD                                                     >> fwk.h
    type engine\fwk                                                          >> fwk.h
    type engine\fwk.c                                                        >> fwk.h
    echo #endif // FWK_IMPLEMENTATION                                        >> fwk.h
    move /y fwk.h engine\joint
    exit /b
)

rem generate prior files to a github release
if "%1"=="github" (

    call make.bat joint
    call make.bat split

    rem call make.bat dll
    call make.bat docs
    call make.bat bind

rem rd /q /s engine\split
rem md engine\split
rem move /y fwk_*.? engine\split\
rem move /y 3rd_*.? engine\split\

rem pushd tools
rem cl  cook.c      -I..\engine /openmp /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
rem del cook.exp
rem del cook.lib
rem del cook.obj
rem del cook.pdb
rem popd

    call make.bat tidy

    exit /b
)

rem shortcuts for split & join scripts
if "%1"=="split" (
    call tools\split
    call make joint
    exit /b
)
if "%1"=="join" (
    call tools\join
    call make joint
    exit /b
)

rem fuse binaries and zipfiles
if "%1"=="fuse" (
    if not exist *.exe (
        echo No binaries to fuse.
    ) else (
        setlocal enableDelayedExpansion
        del *.zip 2> nul 1> nul & tools\cook --cook-jobs=1
        md _fused 2> nul 1> nul
        for %%i in (*.exe) do set "var=%%i" && ( copy /y !var! _fused\fused_!var! 2> nul 1> nul & tools\ark _fused\fused_!var! *.zip )
    )
    exit /b
)

rem check memory api calls
if "%1"=="test" (
    findstr /RNC:"[^_xv]realloc[(]" engine\fwk.c engine\split\fwk*
    findstr /RNC:"[^_xv]malloc[(]"  engine\fwk.c engine\split\fwk*
    findstr /RNC:"[^_xv]free[(]"    engine\fwk.c engine\split\fwk*
    findstr /RNC:"[^_xv]calloc[(]"  engine\fwk.c engine\split\fwk*
    findstr /RNC:"[^_xv]strdup[(]"  engine\fwk.c engine\split\fwk*
    exit /b
)

rem copy demos to root folder. local changes are preserved
rem echo n | copy /-y demos\*.c 1> nul 2> nul

rem tidy environment
if "%1"=="tidy" (
    move /y ??-*.png demos          > nul 2> nul
    move /y ??-*.c demos            > nul 2> nul
    del demos\lua\fwk.dll           > nul 2> nul
    del demos\lua\*.zip             > nul 2> nul
    rd /q /s demos\lua\__pycache__  > nul 2> nul
    del .temp*.*                    > nul 2> nul
    del *.zip                       > nul 2> nul
    del *.mem                       > nul 2> nul
    del *.exp                       > nul 2> nul
    del *.lib                       > nul 2> nul
    del *.exe                       > nul 2> nul
    del *.obj                       > nul 2> nul
    del *.o                         > nul 2> nul
    del *.a                         > nul 2> nul
    del *.pdb                       > nul 2> nul
    del *.ilk                       > nul 2> nul
    del *.png                       > nul 2> nul
    del *.mp4                       > nul 2> nul
    del *.def                       > nul 2> nul
    del *.dll                       > nul 2> nul
    del *.log                       > nul 2> nul
    del 3rd_*.*                     > nul 2> nul
    del fwk_*.*                     > nul 2> nul
rem del ??-*.*                      > nul 2> nul
    del temp_*.*                    > nul 2> nul
    rd /q /s .vs                    > nul 2> nul
    rd /q /s _cache                 > nul 2> nul
    rd /q /s _debug                 > nul 2> nul
    rd /q /s _devel                 > nul 2> nul
    rd /q /s _release               > nul 2> nul
rem rd /q /s _project               > nul 2> nul
    rd /q /s _fused                 > nul 2> nul
    del tcc.bat                     > nul 2> nul
    del sh.bat                      > nul 2> nul
    del cook*.csv                   > nul 2> nul
    rem for /R %%i in (*@animlist.txt) do del %%i > nul 2> nul
    exit /b
)

rem Compiler detection
rem set vs=00
rem set cc=%cc%
rem check args
rem if "%cc%"=="" (
rem     SET "_FullString=%*"
rem     SET "_Search=tcc"
rem     CALL SET "_result=%%_FullString:%_Search%=%%"
rem     If /i "%_result%"=="%_FullString%" (Echo String not found) ELSE (set cc=tcc)
rem )
rem detect setup
if "%cc%"=="" (
    echo Detecting VS 2022/2019/2017/2015/2013 x64 ...
    set "cc=cl" && where /q cl.exe || (
               if exist "%VS170COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
                  @call "%VS170COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul && set "vs=22"
        ) else if exist "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
                  @call "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul && set "vs=19"
        ) else if exist "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
                  @call "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul && set "vs=17"
        ) else if exist "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (
                  @call "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" >nul && set "vs=15"
        ) else if exist "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (
                  @call "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" >nul && set "vs=13"
        ) else if exist "%ProgramFiles%/microsoft visual studio/2022/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
                  @call "%ProgramFiles%/microsoft visual studio/2022/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul && set "vs=22"
        ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
                  @call "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul && set "vs=19"
        ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
                  @call "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul && set "vs=17"
        ) else (
            echo Detecting Mingw64 ...
            set "cc=gcc" && where /q gcc.exe || (
                echo Detecting TCC ... && set "cc=tcc"
            )
        )
    )
)
if "%cc%"=="cl" (
           if exist "%VS170COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (set "vs=22"
    ) else if exist "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (set "vs=19"
    ) else if exist "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (set "vs=17"
    ) else if exist "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (set "vs=15"
    ) else if exist "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (set "vs=13"
    ) else if exist "%ProgramFiles%/microsoft visual studio/2022/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (set "vs=22"
    ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (set "vs=19"
    ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (set "vs=17"
    ) else set "vs=00"
)

setlocal enableDelayedExpansion

rem ASK what to build if double-clicked from Windows explorer
if "%1"=="" (((echo.%cmdcmdline%)|%WINDIR%\system32\find.exe /I "%~0")>nul) && (
    set "bak=!cc!"
    for /L %%j in (0,0,1) do (
        echo Menu:
        echo ^(H^)ello intro ^(!cc!^)
        echo ^(E^)ditor ^(!cc!^)
        echo ^(B^)uild everything ^(!cc!^)
        echo ^(C^)ook everything
        echo ^(R^)un everything
        echo ^(D^)ocumentation
        echo ^(F^)use retail binaries ^(!cc!^)
        echo ^(T^)idy folders
        echo ^(S^)ync to latest
        echo ^(1^)Open Visual Studio ^(!vs!^)
        echo ^(2^)Open Visual Studio Code
        echo ^(3^)Open Explorer
        echo ^(G^)Toggle compiler ^(!bak! ^<=^> tcc^)
        echo ^(Q^)uit
        choice /C HEBCRDFTS123GQ /M "Select"
        set choice=!errorlevel!
        if "!choice!"== "1" call make hello.c !cc! && start hello
        if "!choice!"== "2" ( if exist editor.exe ( start editor ) else ( call make editor static !cc! -- -DUI_FONT_SMALL && start editor ) )
        if "!choice!"== "3" call make all static !cc!
        if "!choice!"== "4" call make cook
        if "!choice!"== "5" call make run
        if "!choice!"== "6" start engine/fwk.html && rem start "" "https://bit.ly/fwk2023"
        if "!choice!"== "7" call make all retail static !cc! && make fuse
        if "!choice!"== "8" call make tidy
        if "!choice!"== "9" choice /C YN /M "Local changes may be overwritten. Continue" && if "!errorlevel!"=="1" ( call make sync && start make && exit )
        if "!choice!"=="10" call make proj && (tasklist | find /i "devenv.exe") >nul 2>nul && (echo.) || (start _project\project.sln)
        if "!choice!"=="11" start /min cmd /C "code ."
        if "!choice!"=="12" start .
        if "!choice!"=="13" if "!cc!"=="tcc" (set "cc=!bak!") else (set "cc=tcc")
        if "!choice!"=="14" exit
    )
)

set dll=dll
set build=dev
set args=-Iengine
set other=
set fwk=yes
set hello=yes
set demos=no
set editor=no
set proj=no
set rc=0

:parse_args
    if "%1"=="--"       shift && goto parse_compiler_args

    if "%1"=="dll"      set "dll=%1" && goto loop
    if "%1"=="static"   set "dll=%1" && goto loop

    if "%1"=="dbg"      set "build=%1" && goto loop
    if "%1"=="dev"      set "build=%1" && goto loop
    if "%1"=="rel"      set "build=%1" && goto loop
    if "%1"=="ret"      set "build=%1" && goto loop

    if "%1"=="debug"       set "build=dbg" && goto loop
    if "%1"=="devel"       set "build=dev" && goto loop
    if "%1"=="develop"     set "build=dev" && goto loop
    if "%1"=="developer"   set "build=dev" && goto loop
    if "%1"=="development" set "build=dev" && goto loop
    if "%1"=="release"     set "build=rel" && goto loop
    if "%1"=="retail"      set "build=ret" && goto loop

    if "%1"=="all"      set "fwk=yes" && set "demos=yes" && set "editor=yes" && set "hello=yes" && goto loop
    if "%1"=="demos"    set "demos=yes" && set "hello=no" && goto loop
    if "%1"=="nodemos"  set "demos=no" && goto loop
    if "%1"=="editor"   set "editor=yes" && set "hello=no" && goto loop
    if "%1"=="noeditor" set "editor=no" && goto loop
    if "%1"=="fwk"      set "fwk=yes" && set "hello=no" && goto loop
    if "%1"=="nofwk"    set "fwk=no" && goto loop

    if "%1"=="tcc"      set "cc=%1" && goto loop
    if "%1"=="cl"       set "cc=%1" && goto loop
    if "%1"=="vc"       set "cc=cl" && goto loop
    if "%1"=="cc"       set "cc=%1" && goto loop
    if "%1"=="gcc"      set "cc=%1" && goto loop
    if "%1"=="clang"    set "cc=%1" && goto loop
    if "%1"=="clang-cl" set "cc=%1" && goto loop

    if "%1"=="proj"     set "proj=yes" && goto loop

    if not "%1"==""     set "other=!other! %1" && set "editor=no" && set "demos=no" && set "hello=no"

:loop
    if not "%1"==""     shift && goto parse_args

:parse_compiler_args
    if not "%1"==""     set "args=!args! %1" && shift && goto parse_compiler_args

rem solution. @todo: lin/osx
if "!proj!"=="yes" pushd tools && premake5 vs20!vs! & popd
if "!proj!"=="yes" pushd tools && premake5 ninja    & popd
if "!proj!"=="yes" pushd tools && premake5 gmake    & popd & exit /b

rem --- pipeline
rem cl tools/ass2iqe.c   /Fetools/ass2iqe.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG assimp.lib
rem cl tools/iqe2iqm.cpp /Fetools/iqe2iqm.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG
rem cl tools/mid2wav.c   /Fetools/mid2wav.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG
rem cl tools/xml2json.c  /Fetools/xml2json.exe /nologo /openmp /O2 /Oy /MT /DNDEBUG
rem --- pipeline
rem gcc tools/ass2iqe.c   -o tools/ass2iqe.exe  -w -lassimp
rem gcc tools/iqe2iqm.cpp -o tools/iqe2iqm.exe  -w -lstdc++
rem gcc tools/mid2wav.c   -o tools/mid2wav.exe  -w
rem gcc tools/xml2json.c  -o tools/xml2json.exe -w
rem --- different strategies for release builds
rem  4.6s 6.9MiB (default)
rem 33.7s 6.6MiB             /Ox     /Oy /MT /DNDEBUG
rem 35.8s 5.3MiB                 /O2 /Oy /MT /DNDEBUG
rem 17.9s 4.6MiB                 /O1     /MT /DNDEBUG /GL /GF     /arch:AVX2
rem 17.8s 4.6MiB         /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF     /arch:AVX2
rem 18.8s 4.6MiB         /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF /Gw            /link /OPT:ICF /LTCG
rem 18.0s 4.6MiB /openmp /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG

if "!cc!"=="cl" (

    if "!dll!"=="static" (
        set export=/c
        set import=fwk.obj
    ) else (
        set export=/DAPI=EXPORT /LD
        set import=/DAPI=IMPORT fwk.lib
    )

    if "!build!"=="ret" (
        set args=-DENABLE_RETAIL -Dmain=WinMain !args!
        set args=/nologo /Zi /MT /openmp /DNDEBUG=3 !args!        /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
    )
    if "!build!"=="rel" (
        set args=/nologo /Zi /MT /openmp /DNDEBUG=2 !args!        /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
    )
    if "!build!"=="dev" (
        set args=/nologo /Zi /MT /openmp /DNDEBUG=1 !args!        && REM /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2
    )
    if "!build!"=="dbg" (
        set args=/nologo /Zi /MT         /DEBUG   !args!          /Od  /fsanitize=address
        rem make -- /RTC1, or make -- /Zi /fsanitize=address /DEBUG
    )

    set o=/Fe:
    set echo=REM

) else if "!cc!"=="clang-cl" (

    if "!dll!"=="static" (
        set export=/c
        set import=fwk.obj
    ) else (
        set export=/DAPI=EXPORT /LD
        set import=/DAPI=IMPORT fwk.lib
    )

    set warnings_fwkc=-Wno-deprecated-declarations -Wno-tautological-constant-out-of-range-compare
    set warnings_demos=-Wno-empty-body -Wno-format-security -Wno-pointer-sign
    set warnings=!warnings_fwkc! !warnings_demos!

    if "!build!"=="ret" (
        set args=-DENABLE_RETAIL -Dmain=WinMain !args!
        set args=!warnings! /nologo /Zi /MT /openmp /DNDEBUG=3 !args!        /Os /Ox /O2 /Oy /GF /Gw /arch:AVX2
    )
    if "!build!"=="rel" (
        set args=!warnings! /nologo /Zi /MT /openmp /DNDEBUG=2 !args!        /Os /Ox /O2 /Oy /GF /Gw /arch:AVX2
    )
    if "!build!"=="dev" (
        set args=!warnings! /nologo /Zi /MT /openmp /DNDEBUG=1 !args!        && REM /Os /Ox /O2 /Oy /GF /Gw /arch:AVX2
    )
    if "!build!"=="dbg" (
        set args=!warnings! /nologo /Zi /MT         /DEBUG     !args!        /Od  /fsanitize=address
    )

    set o=-o
    set echo=echo

) else if "!cc!"=="tcc" (

    if "!dll!"=="static" (
        set export=-c
        set import=fwk.o
    ) else (
        set export=-DAPI=EXPORT -shared
        set import=-DAPI=IMPORT fwk.def
    )

    if "!build!"=="ret" (
        set args=-DENABLE_RETAIL -Dmain=WinMain !args!
        set args=-O3 -DNDEBUG=3    !args!
    )
    if "!build!"=="rel" (
        set args=-O2 -DNDEBUG=2    !args!
    )
    if "!build!"=="dev" (
        set args=-O1 -DNDEBUG=1 -g !args!
    )
    if "!build!"=="dbg" (
        set args=-O0            -g !args!
    )

    set o=-o
    set echo=echo

) else ( rem if "!cc!"=="gcc" or "clang"

    set libs=-lws2_32 -lgdi32 -lwinmm -ldbghelp -lole32 -lshell32 -lcomdlg32

    if "!dll!"=="static" (
        set export=-c
        set import=fwk.o -Wl,--allow-multiple-definition
    ) else (
        set export=-DAPI=EXPORT -shared -o fwk.dll -Wl,--out-implib,fwk.a
        set import=-DAPI=IMPORT fwk.a
    )

    set args=-Wno-implicit-function-declaration !libs! !args!

    if "!build!"=="ret" (
        set args=-DENABLE_RETAIL   !args!
        set args=-O3 -DNDEBUG=3    !args!
    )
    if "!build!"=="rel" (
        rem @todo see: https://stackoverflow.com/questions/866721/how-to-generate-gcc-debug-symbol-outside-the-build-target
        set args=-O2 -DNDEBUG=2    !args!
    )
    if "!build!"=="dev" (
        set args=-O1 -DNDEBUG=1 -g !args!
    )
    if "!build!"=="dbg" (
        set args=-O0            -g !args!
    )

    set o=-o
    set echo=echo
)

rem detect whether user-defined sources use single-header distro
rem if so, remove API=IMPORT flags and also do not produce fwk.dll by default
if "!other!!demos!!editor!!hello!"=="nonoyes" (
    set other=hello.c
    set hello=no
)
if not "!other!"=="" (
    >nul find "FWK_IMPLEMENTATION" !other! && (
      set import=
      set fwk=no
    )
)

rem visualize vars
echo cc=!cc!, build=!build!, type=!dll!, other=!other!, args=!args!
echo export=!export!
echo import=!import!

rem set BUILD_VERSION symbol
git describe --tags --abbrev=0 > info.obj
set /p VERSION=<info.obj
git rev-list --count --first-parent HEAD > info.obj
set /p GIT_REVISION=<info.obj
git rev-parse --abbrev-ref HEAD > info.obj
set /p GIT_BRANCH=<info.obj
date /t > info.obj
set /p LAST_MODIFIED=<info.obj
set args=-DBUILD_VERSION="\"!GIT_BRANCH!-!GIT_REVISION!-!build!-!dll!-!USERNAME!\"" !args!

if "!cc!"=="tcc" set "cc=call tools\tcc"

rem framework
if "!fwk!"=="yes" (

    tools\file2hash engine\fwk.c engine\fwk.h engine\fwk. engine\joint\fwk.h -- !build! !import! !export! !args! !dll! > nul
    set cache=_cache\.!errorlevel!
    md _cache 2>nul >nul

    rem cache for `make rel` cl:48s->25s, tcc:3.3s->1.8s
    if exist !cache!.o   copy /y !cache!.o   fwk.o   2>nul >nul
    if exist !cache!.obj copy /y !cache!.obj fwk.obj 2>nul >nul
    if exist !cache!.lib copy /y !cache!.lib fwk.lib 2>nul >nul
    if exist !cache!.dll copy /y !cache!.dll fwk.dll 2>nul >nul
    if exist !cache!.def copy /y !cache!.def fwk.def 2>nul >nul
    if exist !cache!.pdb copy /y !cache!.pdb fwk.pdb 2>nul >nul

    if not exist "!cache!" (
        !echo! fwk          && !cc! engine\fwk.c !export! !args!   && if "!dll!"=="dll" copy /y fwk.dll demos\lua  > nul || set rc=1
        echo. > !cache!
        if exist fwk.o   copy /y fwk.o   !cache!.o   2>nul >nul
        if exist fwk.obj copy /y fwk.obj !cache!.obj 2>nul >nul
        if exist fwk.lib copy /y fwk.lib !cache!.lib 2>nul >nul
        if exist fwk.dll copy /y fwk.dll !cache!.dll 2>nul >nul
        if exist fwk.def copy /y fwk.def !cache!.def 2>nul >nul
        if exist fwk.pdb copy /y fwk.pdb !cache!.pdb 2>nul >nul
    ) else (
        rem cached. do not compile...
        echo fwk.c ^(cached^)
        if "!dll!"=="dll" copy /y !cache!.dll demos\lua\fwk.dll > nul || set rc=1
    )
)

rem editor
if "!editor!"=="yes" (
set edit=-DCOOK_ON_DEMAND -DUI_FONT_SMALL !edit! && REM -DUI_ICONS_SMALL -DUI_LESSER_SPACING -- directives wont work at this point unless fwk.dll is rebuilt
!echo! editor        && !cc! !o! editor.exe engine\editor.c !edit!          !import! !args! || set rc=1
)

rem demos
if "!demos!"=="yes" (
!echo! 00-loop       && !cc! !o! 00-loop.exe        demos\00-loop.c         !import! !args! || set rc=1
!echo! 00-script     && !cc! !o! 00-script.exe      demos\00-script.c       !import! !args! || set rc=1
!echo! 01-demo2d     && !cc! !o! 01-demo2d.exe      demos\01-demo2d.c       !import! !args! || set rc=1
!echo! 01-ui         && !cc! !o! 01-ui.exe          demos\01-ui.c           !import! !args! || set rc=1
!echo! 01-easing     && !cc! !o! 01-easing.exe      demos\01-easing.c       !import! !args! || set rc=1
!echo! 01-font       && !cc! !o! 01-font.exe        demos\01-font.c         !import! !args! || set rc=1
!echo! 02-ddraw      && !cc! !o! 02-ddraw.exe       demos\02-ddraw.c        !import! !args! || set rc=1
!echo! 02-frustum    && !cc! !o! 02-frustum.exe     demos\02-frustum.c      !import! !args! || set rc=1
!echo! 03-anims      && !cc! !o! 03-anims.exe       demos\03-anims.c        !import! !args! || set rc=1
!echo! 04-actor      && !cc! !o! 04-actor.exe       demos\04-actor.c        !import! !args! || set rc=1
!echo! 06-scene      && !cc! !o! 06-scene.exe       demos\06-scene.c        !import! !args! || set rc=1
!echo! 06-material   && !cc! !o! 06-material.exe    demos\06-material.c     !import! !args! || set rc=1
!echo! 07-network    && !cc! !o! 07-network.exe     demos\07-network.c      !import! !args! || set rc=1
!echo! 07-netsync    && !cc! !o! 07-netsync.exe     demos\07-netsync.c      !import! !args! || set rc=1
!echo! 08-audio      && !cc! !o! 08-audio.exe       demos\08-audio.c        !import! !args! || set rc=1
!echo! 08-video      && !cc! !o! 08-video.exe       demos\08-video.c        !import! !args! || set rc=1
!echo! 09-cubemap    && !cc! !o! 09-cubemap.exe     demos\09-cubemap.c      !import! !args! || set rc=1
!echo! 09-shadertoy  && !cc! !o! 09-shadertoy.exe   demos\09-shadertoy.c    !import! !args! || set rc=1

!echo! 99-bt         && !cc! !o! 99-bt.exe          demos\99-bt.c           !import! !args! || set rc=1
!echo! 99-controller && !cc! !o! 99-controller.exe  demos\99-controller.c   !import! !args! || set rc=1
!echo! 99-demo       && !cc! !o! 99-demo.exe        demos\99-demo.c         !import! !args! || set rc=1
!echo! 99-lod        && !cc! !o! 99-lod.exe         demos\99-lod.c          !import! !args! || set rc=1
!echo! 99-pbr        && !cc! !o! 99-pbr.exe         demos\99-pbr.c          !import! !args! || set rc=1
!echo! 99-spine      && !cc! !o! 99-spine.exe       demos\99-spine.c        !import! !args! || set rc=1
!echo! 99-sprite     && !cc! !o! 99-sprite.exe      demos\99-sprite.c       !import! !args! || set rc=1
!echo! 99-sprite3d   && !cc! !o! 99-sprite3d.exe    demos\99-sprite3d.c     !import! !args! || set rc=1
!echo! 99-geom       && !cc! !o! 99-geom.exe        demos\99-geom.c         !import! !args! || set rc=1
!echo! 99-compute    && !cc! !o! 99-compute.exe     demos\99-compute.c      !import! !args! || set rc=1
!echo! 99-pathfind   && !cc! !o! 99-pathfind.exe    demos\99-pathfind.c     !import! !args! || set rc=1
!echo! 99-sponza     && !cc! !o! 99-sponza.exe      demos\99-sponza.c       !import! !args! || set rc=1
!echo! 99-gui        && !cc! !o! 99-gui.exe         demos\99-gui.c          !import! !args! || set rc=1
!echo! 99-lmap       && !cc! !o! 99-lmap.exe        demos\99-lmap.c         !import! !args! || set rc=1
!echo! 99-steam      && !cc! !o! 99-steam.exe       demos\99-steam.c        !import! !args! || set rc=1
)

rem hello
if "!hello!"=="yes" (
!echo! hello         && !cc! !o! hello.exe          hello.c                          !args! || set rc=1
)

rem user-defined apps
if not "!other!"=="" (
!echo! !other! && !cc! !other! !import! !args! || set rc=1
)

rem PAUSE only if double-clicked from Windows explorer
(((echo.%cmdcmdline%)|%WINDIR%\system32\find.exe /I "%~0")>nul)&&pause

cmd /c exit !rc!
