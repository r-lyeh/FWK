@if     "%1"=="-impdef" @%~dp0\tcc\tcc -m64 %*
@if not "%1"=="-impdef" @%~dp0\tcc\tcc -I %~dp0\tcc-winapi\winapi -I %~dp0\tcc-winapi\ -m64 %*
