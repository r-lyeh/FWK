@echo off
cd /d "%~dp0"

if "%1"=="tidy" (
    del *.exe 1>nul 2>nul
    del *.dll 1>nul 2>nul
    del *.lib 1>nul 2>nul
    del *.obj 1>nul 2>nul
    del *.exp 1>nul 2>nul
    del *.ilk 1>nul 2>nul
    del *.pdb 1>nul 2>nul
    del *.def 1>nul 2>nul
    del call_*.h 1>nul 2>nul
    rd /q /s lua 1>nul 2>nul
    exit /b
)

if "%1"=="dll" (

    if not exist lua     git clone https://github.com/lua/lua
    if not exist lua.dll (
        cl lua\onelua.c -Ilua /Felua.exe

        cl lua\onelua.c -Ilua /Felua.dll /DLUA_BUILD_AS_DLL /LD
        echo LIBRARY LUA > lua.def
        echo EXPORTS >> lua.def
        for /f "skip=19 tokens=4" %%A in ('dumpbin /exports lua.dll') do echo %%A >> lua.def
        lib /def:lua.def /out:lua.lib /machine:x64
    )

    lua dynasm\dynasm.lua -LNE -D X32WIN        -o call_x86.h    call_x86.dasc
    lua dynasm\dynasm.lua -LNE -D X64           -o call_x64.h    call_x86.dasc
    lua dynasm\dynasm.lua -LNE -D X64 -D X64WIN -o call_x64win.h call_x86.dasc
    lua dynasm\dynasm.lua -LNE                  -o call_arm.h    call_arm.dasc

    set COMMON=/nologo /Zi /D_CRT_SECURE_NO_DEPRECATE /DLUA_FFI_BUILD_AS_DLL

    if "%1"=="rel" (
    set OPTIONS=/MD /Ox %COMMON%
    ) else (
    set OPTIONS=/MDd /Od %COMMON%
    )

    CL %OPTIONS% /I. /Ilua /DLUA_DLL_NAME="lua.dll" call.c ctype.c ffi.c parser.c lua.lib /Feffi.dll /LD

    CL %OPTIONS% /Gd test.c /Fe"test_cdecl.dll" /LD
    CL %OPTIONS% /Gz test.c /Fe"test_stdcall.dll" /LD
    CL %OPTIONS% /Gr test.c /Fe"test_fastcall.dll" /LD

    lua test.lua

    del *.exp *.ilk *.pdb *.obj *.manifest 2> nul 1> nul

    exit /b
)

if "%1"=="3rd" (

    echo #ifndef LUAFFI_H  > 3rd_luaffi.h
    echo #define LUAFFI_H >> 3rd_luaffi.h
    echo. >> 3rd_luaffi.h

        type ffi.h >> 3rd_luaffi.h
        echo. >> 3rd_luaffi.h

        type "dynasm\dasm_proto.h" >> 3rd_luaffi.h
        echo. >> 3rd_luaffi.h

    echo #endif >> 3rd_luaffi.h
    echo. >> 3rd_luaffi.h


    echo #ifdef LUAFFI_C  >> 3rd_luaffi.h
    echo. >> 3rd_luaffi.h

        echo static cfunction compile(Dst_DECL, lua_State* L, cfunction func, int ref^); >> 3rd_luaffi.h
        echo. >> 3rd_luaffi.h

        echo #if defined __arm__ ^|^| defined __arm ^|^| defined __ARM__ ^|^| defined __ARM ^|^| defined ARM ^|^| defined _ARM_ ^|^| defined ARMV4I ^|^| defined _M_ARM >> 3rd_luaffi.h
        type "dynasm\dasm_arm.h" >> 3rd_luaffi.h
        echo #else >> 3rd_luaffi.h
        type "dynasm\dasm_x86.h" >> 3rd_luaffi.h
        echo #endif >> 3rd_luaffi.h
        echo. >> 3rd_luaffi.h

        echo #if defined __arm__ ^|^| defined __arm ^|^| defined __ARM__ ^|^| defined __ARM ^|^| defined ARM ^|^| defined _ARM_ ^|^| defined ARMV4I ^|^| defined _M_ARM >> 3rd_luaffi.h
        type "call_arm.h" >> 3rd_luaffi.h
        echo #elif defined _WIN64 >> 3rd_luaffi.h
        type "call_x64win.h" >> 3rd_luaffi.h
        echo #elif defined __amd64__ >> 3rd_luaffi.h
        type "call_x64.h" >> 3rd_luaffi.h
        echo #else >> 3rd_luaffi.h
        type "call_x86.h" >> 3rd_luaffi.h
        echo #endif >> 3rd_luaffi.h
        echo. >> 3rd_luaffi.h

        type call.c >> 3rd_luaffi.h
        type ctype.c >> 3rd_luaffi.h
        type parser.c >> 3rd_luaffi.h
        type ffi.c >> 3rd_luaffi.h
        echo. >> 3rd_luaffi.h

    echo #endif >> 3rd_luaffi.h
    echo. >> 3rd_luaffi.h

    ..\fart -- 3rd_luaffi.h "#pragma once" "//#pragma once"
    ..\fart -- 3rd_luaffi.h "#include \"" "//#include \""
    ..\fart -- 3rd_luaffi.h "# include" "//# include"
    ..\fart -- 3rd_luaffi.h "dasm_State*" "struct dasm_State*"
    ..\fart -- 3rd_luaffi.h "EXPORT" "LUAFFI_EXPORT"
    ..\fart -- 3rd_luaffi.h "ALIGN_UP" "LUAFFI_ALIGN_UP"
    ..\fart -- 3rd_luaffi.h "get_int" "LUAFFI_get_int"
)

if not exist "..\fart.exe"  echo ..\fart.exe not found && exit /b
if not exist "call_x86.h"   call make dll
if not exist "3rd_luaffi.h" call make 3rd
if exist "3rd_luaffi.h"     move /y 3rd_luaffi.h ..\..\engine\split && call make tidy
