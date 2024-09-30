@echo off
cd /d "%~dp0"

REM Build fuser
cl fuser.c /MT /O2 /DNDEBUG /link setargv.obj

REM Build ase2ini
cl ase2ini.c -I ..\engine\split -I ..\engine\3rd /DNDEBUG /O2 /Ox /MT

REM Build ASSIMP
call MAKE_assimp.bat

REM Build cook
cl cook.c -I..\engine /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG

REM Build file2hash
cl file2hash.c /O2 /Oy /MT /DNDEBUG /link setargv.obj

REM Build iqe2iqm
cl iqe2iqm.cpp /O2 /Oy /MT /DNDEBUG

REM Build mid2wav
cl mid2wav.c -I ..\engine\split -I ..\engine\3rd /Os /Ox /O2 /Oy /MT /DFINAL /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG

REM Build mod2wav
cl mod2wav.cc -I ..\engine\split -I ..\engine\3rd /EHsc /O2 /Oy /DNDEBUG /MT

REM Build sfxr2wav
cl sfxr2wav.c /EHsc /O2 /Oy /DNDEBUG /MT

REM Build xlsx2ini
cl xlsx2ini.cc /MD /Ox /Oy /DNDEBUG /EHsc

REM Build docs
cl   docs\docs.c ..\engine\fwk.c -I..\engine /Od /DNDEBUG

cd ..
