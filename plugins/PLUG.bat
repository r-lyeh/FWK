@echo off
setlocal enableDelayedExpansion
cd /d "%~dp0"

rem check first char of 2nd argument. process filename if it starts with '@'
IF NOT "%~2"=="" (
    SET firstChar=%2
    SET firstChar=!firstChar:~0,1!
    if "!firstChar!"=="@" (
        SET filename=%2
        SET filename=!filename:~1!
        for /f "tokens=* delims=" %%a in (!filename!) do (
            call PLUG.bat %1 %%a
        )
        exit /b
    )
)

rem add plugin into disk
if "%1"=="add" if not "%2" == "" (
    if not exist "%2" (
        echo adding %2
        git clone https://github.com/%2 "%2"
    )
    exit /b
)
rem remove plugin from disk
if "%1"=="del" if not "%2" == "" (
    if exist "%2" (
        echo deleting %2
        rd /q /s "%2" && rem 1>nul 2>nul
        rd "%2\.." && rem 1>nul 2>nul
    )
    exit /b
)
rem update plugin to latest
if "%1"=="upd" if not "%2" == "" (
    if exist "%2" (
        pushd "%2"
            git fetch
            git rev-list --count HEAD..@{u} > repo.0
            findstr /m "0" "repo.0" >nul
            if !errorlevel!==0 (
                echo already satisfied %2
            ) else (
                echo updating %2
                git pull
            )
            del repo.0
        popd
    )
    exit /b
)

if not "%1"=="dir" ( echo plug ^[dir^] && echo plug ^[add^|del^|upd^] user/repo && echo plug ^[add^|del^|upd^] @filelist && exit /b )
set fwk_done=no
goto dir_fwk


rem list plugins and descriptions
:dir_v4k
curl -s "https://api.github.com/search/repositories?q=v4k+topic:v4k+language:c+language:cpp+language:lua&sort=created&order=asc" > plugs.0
goto process_plugs

:dir_fwk
curl -s "https://api.github.com/search/repositories?q=fwk+topic:fwk+language:c+language:cpp+language:lua&sort=created&order=asc" > plugs.0


:process_plugs
findstr /C:"\"description\"" /C:"\"clone_url\"" plugs.0 > plugs.1 && rem /C:"\"name\"" 

rem remove keys, commas and quotes
if exist plugs.x del plugs.x
for /f "tokens=*" %%i in (plugs.1) do (
    set var=%%i
    set var=!var:,=!
    set var=!var:"name": =!
    set var=!var:"clone_url": =!
    set var=!var:"description": =!
    set var=!var:https://github.com/=!
    set var=!var:.git=!
    set var=!var:"=!
    echo !var! >> plugs.x
)


rem determine number of lines
for /f %%i in ('find /c /v "" ^< plugs.x') do set "cnt=%%i"


rem read the file into an array
<plugs.x (
    for /l %%i in (1 1 %cnt%) do (
        set "str.%%i="
        set /p "str.%%i="
    )
)


rem combine odd/even lines
for /l %%i in (1 2 %cnt%) do (
    set /A from=%%i+1
    set /A to=%%i+2
    for /l %%u in (!from! 2 !to!) do (

        rem display the array values
        if exist "./!str.%%u!" ( echo [x] !str.%%u!: !str.%%i! ) else ( echo [ ] !str.%%u!: !str.%%i! )
    )
)


rem clean up
del plugs.*

if "!fwk_done!"=="no" (
    set fwk_done=yes
    goto dir_v4k
)
