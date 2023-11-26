/* game framework.
*  - rlyeh, public domain
*
* ## FWK License
*
* This software is available under 3 licenses. Choose whichever you prefer.
* ------------------------------------------------------------------------------
* ALTERNATIVE A - Public Domain (https://unlicense.org/)
* ------------------------------------------------------------------------------
* This is free and unencumbered software released into the public domain.
*
* Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
* software, either in source code form or as a compiled binary, for any purpose,
* commercial or non-commercial, and by any means.
*
* In jurisdictions that recognize copyright laws, the author or authors of this
* software dedicate any and all copyright interest in the software to the public
* domain. We make this dedication for the benefit of the public at large and to
* the detriment of our heirs and successors. We intend this dedication to be an
* overt act of relinquishment in perpetuity of all present and future rights to
* this software under copyright law.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* ------------------------------------------------------------------------------
* ALTERNATIVE B - 0-BSD License (https://opensource.org/licenses/FPL-1.0.0)
* ------------------------------------------------------------------------------
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
* REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
* FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
* INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
* LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
* OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
* PERFORMANCE OF THIS SOFTWARE.
* ------------------------------------------------------------------------------
* ALTERNATIVE C - MIT-0 (No Attribution clause)
* ------------------------------------------------------------------------------
* Permission is hereby granted, free of charge, to any person obtaining a copy of this
* software and associated documentation files (the "Software"), to deal in the Software
* without restriction, including without limitation the rights to use, copy, modify,
* merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* ## License: Contributed Code ------------------------------------------------
*
* Dear Contributor,
*
* In order to ensure this project remains completely free and unencumbered by
* anyone's copyright monopoly, it is advisable that you dedicate your code-base
* contributions to the three licensing terms above. This removes any possible
* ambiguity as to what terms somebody might have thought they were contributing
* under, in case of a future dispute. These concerns are not unique to public
* domain software. Most large, established open-source projects have a
* Contributor License Agreement (CLA) process, of varying degrees of formality.
*
* Please add yourself to the list below before contributing.
* Thanks.
*
* --
*
* "I dedicate any and all copyright interest in this software to the three
* licensing terms listed above. I make this dedication for the benefit of the
* public at large and to the detriment of my heirs and successors. I intend
* this dedication to be an overt act of relinquishment in perpetuity of all
* present and future rights to this software under copyright law."
*
* Author (name)              I agree (YES/NO)    Files/Features (optional)
* ------------------------------------------------------------------------------
* @r-lyeh                    YES                 Initial codebase
* @zak@v4.games              YES                 N/A
* ------------------------------------------------------------------------------
*/

#ifndef FWK_H
#define FWK_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// Headers

#line 1 "fwk_config.h"
// -----------------------------------------------------------------------------
// config directives

#ifndef ENABLE_FASTCALL_LUA
#define ENABLE_FASTCALL_LUA     1 ///+
#endif

#ifndef ENABLE_PROFILER
#define ENABLE_PROFILER         ifdef(retail, 0, 1)  ///+
#endif

#ifndef ENABLE_SELFIES
#define ENABLE_SELFIES          0 ///+
#endif

#ifndef ENABLE_MEMORY_POISON
#define ENABLE_MEMORY_POISON    ifdef(debug, 1, 0) ///+
#endif

#ifndef ENABLE_MEMORY_LEAKS
#define ENABLE_MEMORY_LEAKS     0 ///+
#endif

#ifndef ENABLE_LINUX_CALLSTACKS
#define ENABLE_LINUX_CALLSTACKS 0 ///+
#endif

#ifndef ENABLE_AUTOTESTS
#define ENABLE_AUTOTESTS        ifdef(debug, ifndef(ems, 1, 0), 0) ///+
#endif

#ifndef ENABLE_RETAIL
#define ENABLE_RETAIL           0 // ifdef(retail, 1, 0) ///+
#endif

#ifndef ENABLE_RPMALLOC
#define ENABLE_RPMALLOC         0 // ifdef(tcc, 0, 1) // forbidden on tcc because of lacking TLS support
#endif

// -----------------------------------------------------------------------------
// if/n/def hell

#define ifdef(macro, yes, /*no*/...)   ifdef_##macro(yes, __VA_ARGS__)
#define ifndef(macro, yes, /*no*/...)  ifdef_##macro(__VA_ARGS__, yes)
#define is(macro)                      ifdef_##macro(1,0)
#define isnt(macro)                    ifdef_##macro(0,1)
#define ifdef_true(yes, /*no*/...)     yes
#define ifdef_false(yes, /*no*/...)    __VA_ARGS__

#ifdef _MSC_VER
#define ifdef_gcc                      ifdef_false
#define ifdef_mingw                    ifdef_false
#define ifdef_tcc                      ifdef_false
#define ifdef_cl                       ifdef_true
#elif defined __TINYC__
#define ifdef_gcc                      ifdef_false
#define ifdef_mingw                    ifdef_false
#define ifdef_tcc                      ifdef_true
#define ifdef_cl                       ifdef_false
#elif defined __MINGW64__ || defined __MINGW32__
#define ifdef_gcc                      ifdef_true
#define ifdef_mingw                    ifdef_true
#define ifdef_tcc                      ifdef_false
#define ifdef_cl                       ifdef_false
#else // also __clang__
#define ifdef_gcc                      ifdef_true
#define ifdef_mingw                    ifdef_false
#define ifdef_tcc                      ifdef_false
#define ifdef_cl                       ifdef_false
#endif

#ifdef __cplusplus
#define ifdef_cpp                      ifdef_true
#define ifdef_c                        ifdef_false
#else
#define ifdef_c                        ifdef_true
#define ifdef_cpp                      ifdef_false
#endif

#if defined _WIN32
#define ifdef_win32                    ifdef_true
#define ifdef_linux                    ifdef_false
#define ifdef_osx                      ifdef_false
#define ifdef_bsd                      ifdef_false
#define ifdef_ems                      ifdef_false
#elif defined __linux__
#define ifdef_win32                    ifdef_false
#define ifdef_linux                    ifdef_true
#define ifdef_osx                      ifdef_false
#define ifdef_bsd                      ifdef_false
#define ifdef_ems                      ifdef_false
#elif defined __APPLE__
#define ifdef_win32                    ifdef_false
#define ifdef_linux                    ifdef_false
#define ifdef_osx                      ifdef_true
#define ifdef_bsd                      ifdef_false
#define ifdef_ems                      ifdef_false
#elif defined __EMSCRIPTEN__
#define ifdef_win32                    ifdef_false
#define ifdef_linux                    ifdef_false
#define ifdef_osx                      ifdef_false
#define ifdef_bsd                      ifdef_false
#define ifdef_ems                      ifdef_true
#else // __FreeBSD__ || @todo: __ANDROID_API__
#define ifdef_win32                    ifdef_false
#define ifdef_linux                    ifdef_false
#define ifdef_osx                      ifdef_false
#define ifdef_bsd                      ifdef_true
#define ifdef_ems                      ifdef_false
#endif

#ifdef NDEBUG // rely on NDEBUG as the official/portable way to disable asserts
#define ifdef_debug                    ifdef_false
#define ifdef_release                  ifdef_true
#else
#define ifdef_debug                    ifdef_true
#define ifdef_release                  ifdef_false
#endif

#if ENABLE_RETAIL // NDEBUG>=2 ?
#define ifdef_retail                   ifdef_true
#else
#define ifdef_retail                   ifdef_false
#endif

#if   defined NDEBUG && NDEBUG >= 3 // we use NDEBUG=[0,1,2,3] to signal the compiler optimization flags O0,O1,O2,O3
#define ifdef_O3                       ifdef_true
#define ifdef_O2                       ifdef_false
#define ifdef_O1                       ifdef_false
#define ifdef_O0                       ifdef_false
#elif defined NDEBUG && NDEBUG >= 2
#define ifdef_O3                       ifdef_false
#define ifdef_O2                       ifdef_true
#define ifdef_O1                       ifdef_false
#define ifdef_O0                       ifdef_false
#elif defined NDEBUG && NDEBUG >= 1
#define ifdef_O3                       ifdef_false
#define ifdef_O2                       ifdef_false
#define ifdef_O1                       ifdef_true
#define ifdef_O0                       ifdef_false
#else
#define ifdef_O3                       ifdef_false
#define ifdef_O2                       ifdef_false
#define ifdef_O1                       ifdef_false
#define ifdef_O0                       ifdef_true
#endif

#include <stdint.h>
#if (defined INTPTR_MAX && INTPTR_MAX == INT64_MAX) || defined(_M_X64) || defined(__amd64__) || defined(__x86_64__) || defined(__ppc64__) || __WORDSIZE == 64
#define ifdef_64                       ifdef_true
#define ifdef_32                       ifdef_false
#else
#define ifdef_64                       ifdef_false
#define ifdef_32                       ifdef_true
#endif

// -----------------------------------------------------------------------------
// new C keywords

#define countof(x)       (int)(sizeof (x) / sizeof 0[x])

#define concat(a,b)      conc4t(a,b)
#define conc4t(a,b)      a##b ///-

#define macro(name)      concat(name, __LINE__)
#define unique(name)     concat(concat(concat(name,concat(_L,__LINE__)),_),__COUNTER__)
#define defer(begin,end) for(int macro(i) = ((begin), 0); !macro(i); macro(i) = ((end), 1))
#define scope(end)       defer((void)0, end)
#define benchmark        for(double macro(i) = 1, macro(t) = (time_ss(),-time_ss()); macro(i); macro(t)+=time_ss(), macro(i)=0, printf("%.4fs %2.f%% (" FILELINE ")\n", macro(t), macro(t)*100/0.0166667 ))
#define benchmark_ms     for(double macro(i) = 1, macro(t) = (time_ss(),-time_ss()); macro(i); macro(t)+=time_ss(), macro(i)=0, printf("%.2fms %2.f%% (" FILELINE ")\n", macro(t)*1000, macro(t)*100/0.016666667 ))
#define do_once          static int macro(once) = 0; for(;!macro(once);macro(once)=1)

#if is(cl)
#define __thread         __declspec(thread)
#elif is(tcc) && is(win32)
#define __thread         __declspec(thread) // compiles fine apparently, but does not work
#elif is(tcc)
#define __thread
#endif

// usage: bool static(audio_is_init) = audio_init();
//#define static(var)    static var; do_once var

//-----------------------------------------------------------------------------
// new C macros

#if ENABLE_RETAIL
#define ASSERT(expr, ...)          (void)0
#define ASSERT_ONCE(expr, ...)     (void)0
#else
#define ASSERT(expr, ...)          do { int fool_msvc[] = {0,}; if(!(expr)) { fool_msvc[0]++; alert(va("!Expression failed: " #expr " " FILELINE "\n" __VA_ARGS__)), breakpoint(); } } while(0)
#define ASSERT_ONCE(expr, ...)     do { int fool_msvc[] = {0,}; if(!(expr)) { fool_msvc[0]++; static int seen = 0; if(!seen) seen = 1, alert(va("!Expression failed: " #expr " " FILELINE "\n" __VA_ARGS__)), breakpoint(); } } while(0)
#endif
#define STATIC_ASSERT(EXPR)        typedef struct { unsigned macro(static_assert_on_L) : !!(EXPR); } unique(static_assert_on_L)

#define FILELINE                   __FILE__ ":" STRINGIZE(__LINE__)
#define STRINGIZE(x)               STRINGIZ3(x)
#define STRINGIZ3(x)               #x ///-

#define EXPAND(name, ...)          EXPAND_QUOTE(EXPAND_JOIN(name, EXPAND_COUNT_ARGS(__VA_ARGS__)), (__VA_ARGS__))
#define EXPAND_QUOTE(x, y)         x y ///-
#define EXPAND_JOIN(name, count)   EXPAND_J0IN(name, count) ///-
#define EXPAND_J0IN(name, count)   EXPAND_J01N(name, count) ///-
#define EXPAND_J01N(name, count)   name##count ///-
#define EXPAND_COUNT_ARGS(...)     EXPAND_ARGS((__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)) ///-
#define EXPAND_ARGS(args)          EXPAND_RETURN_COUNT args ///-
#define EXPAND_RETURN_COUNT(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, count, ...) count ///-

// expands to the first argument
#define VA_FIRST(...) VA_F1RST(__VA_ARGS__, throwaway)
#define VA_F1RST(first, ...) first ///-
// if there's only one argument, expands to nothing.  if there is more
// than one argument, expands to a comma followed by everything but
// the first argument.  only supports up to 9 arguments but can be expanded.
#define VA_REST(...) VA_R3ST(VA_NUM(__VA_ARGS__), __VA_ARGS__)
#define VA_R3ST(qty, ...) VA_R3S7(qty, __VA_ARGS__) ///-
#define VA_R3S7(qty, ...) VA_R3S7_##qty(__VA_ARGS__) ///-
#define VA_R3S7_ONE(first) ///-
#define VA_R3S7_TWOORMORE(first, ...) , __VA_ARGS__ ///-
#define VA_NUM(...) VA_SELECT_10TH(__VA_ARGS__, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, ONE, throwaway) ///-
#define VA_SELECT_10TH(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, ...) A10
// VA_SPLIT() expands to A) 1 item OR B) 1 item + ',' + va_args[1..N]
#define VA_SPLIT(...) VA_FIRST(__VA_ARGS__) VA_REST(__VA_ARGS__)
// VA_COUNT() counts number of va args
#define VA_COUNT(...)  (int)(sizeof((int[]){0, ##__VA_ARGS__})/sizeof(int)-1)

#if is(cl) && !is(cpp)
#define INLINE __inline
#else
#define INLINE inline
#endif

#if is(cl)
#define FORCE_INLINE __forceinline
#elif is(gcc)
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE INLINE
#endif

#if is(cl) && (_MSC_VER <= 1700)
#define FINITE _finite
#else
#define FINITE isfinite
#endif

// usage: #define vec2(...) C_CAST(vec2, __VA_ARGS__)
// typedef union vec2 { float X,Y; }; vec2 a = {0,1}, b = vec2(0,1);
#define C_CAST(type, ...)  ( ifdef(c,(type),type) { __VA_ARGS__ } )

// create a WARNING(...) macro
// usage: WARNING("this is displayed at compile time")
#if is(gcc)
#   define WARNING(msg) WARN1NG( message( msg ) )
#   define WARN1NG(msg) _Pragma(#msg)
#elif is(cl)
#   define WARNING(msg) __pragma( message( msg ) )
#else
#   define WARNING(msg)
#endif

// document todos and fixmes via compiler warnings
#define TODO(str)  ifdef(debug,WARNING("TO DO: " str " (" FILELINE ")"))
#define FIXME(str) ifdef(debug,WARNING("FIXME: " str " (" FILELINE ")"))

// -----------------------------------------------------------------------------
// autorun initializers for C
// - rlyeh, public domain
//
// note: based on code by Joe Lowe (public domain).
// note: XIU for C initializers, XCU for C++ initializers, XTU for C deinitializers

#define AUTORUN AUTORUN_( unique(fn) )
#ifdef __cplusplus
#define AUTORUN_(fn) \
    static void fn(void); \
    static const int concat(fn,__1) = (fn(), 1); \
    static void fn(void)
#elif defined _MSC_VER && !defined(__clang__) // cl, but not clang-cl
#define AUTORUN_(fn) \
    static void fn(void); \
    static int concat(fn,__1) (){ fn(); return 0; } \
    __pragma(section(".CRT$XIU", long, read)) \
    __declspec(allocate(".CRT$XIU")) \
    static int(* concat(fn,__2) )() = concat(fn,__1); \
    static void fn(void)
#elif defined __TINYC__ // tcc...
#define AUTORUN_(fn) \
    __attribute__((constructor)) \
    static void fn(void)
#else // gcc,clang,clang-cl...
#define AUTORUN_(fn) \
    __attribute__((constructor(__COUNTER__+101))) \
    static void fn(void)
#endif

// -----------------------------------------------------------------------------
// build info

#ifndef BUILD_VERSION
#define BUILD_VERSION ""
#endif

// -----------------------------------------------------------------------------
// visibility

// win32 users would need to -DAPI=EXPORT/IMPORT as needed when building/using FWK as DLL.

#define IMPORT ifdef(win32, ifdef(gcc, __attribute__ ((dllimport)), __declspec(dllimport)))
#define EXPORT ifdef(win32, ifdef(gcc, __attribute__ ((dllexport)), __declspec(dllexport)))
#define STATIC

#ifndef API
#define API STATIC
#endif

// -----------------------------------------------------------------------------
// system headers

#ifndef _GNU_SOURCE
#define _GNU_SOURCE    ///- for linux
#endif

#if is(cl) && is(win32) // for VC IDE
#define _CRT_SECURE_NO_WARNINGS ///-
#define _CRT_NONSTDC_NO_DEPRECATE ///-
#define _WINSOCK_DEPRECATED_NO_WARNINGS ///-
#define _WIN32_WINNT 0x0600  ///- 0x0502 // GetInfoAddrW/FreeAddrInfoW for X86
#endif

#if is(cl)
#include <omp.h>      // compile with /openmp to speed up some computations
#endif
#include <assert.h>
//#include <float.h>
//#include <limits.h>
#include <math.h>     // NAN
#include <stdarg.h>   // va_*(), ...
#include <stdbool.h>  // bool,true,false
#include <stdint.h>   // u/int8/16/32/64_t
#include <stdio.h>    // FILE,NULL
#include <stdlib.h>   // malloc,free,exit,
#include <string.h>   // strlen,memset,memcpy,

#if is(tcc) && is(win32) && defined(__x86_64)
#include <tgmath.h>

// @fixme workarounds on `tcc0.9.27 -m64` (win) for fmod()/trunc() functions. test: 00-easing broken otherwise
//#define trunc(x) ((double)(int64_t)(x))
//#define fmod(x,y) ((x) - trunc((x) / (y)) * (y))

// @fixme workarounds on `tcc0.9.27 -m64` (win) for all functions with ending bool argument. test: 00-anims crashes otherwise
#undef  bool
typedef char bool;

// missing libm symbols on tinycc HEAD repo (tcc-x64 pre-0.9.28)
//#define fabsf fabs
#define sqrtf sqrt
#define sinf sin
#define asinf asin
#define cosf cos
#define acosf acos
#define tanf tan
#define atan2f atan2
#define powf pow
#define floorf floor
#define logf log
#define ceilf ceil
#define copysignf copysign
//#define ldexpf ldexp
#define expf exp
//#define frexpf frexp
#define fmodf fmod
#define log10f log10
//#define logf log
#define hypotf hypot

#endif

// -----------------------------------------------------------------------------
// pragma libs

#if is(win32) && (is(cl) || is(tcc))
#pragma comment(lib, "advapi32")
#pragma comment(lib, "comdlg32")
#pragma comment(lib, "dbghelp")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "ole32")
#pragma comment(lib, "shell32")
#pragma comment(lib, "user32")
#pragma comment(lib, "winmm")
#pragma comment(lib, "wininet")
#pragma comment(lib, "ws2_32")
#endif

#if is(linux) && is(tcc)
#pragma comment(lib, "dl")
#pragma comment(lib, "m")
#pragma comment(lib, "pthread")
#endif
#line 0

#line 1 "fwk_ds.h"
// data structures and utils: array, set, map, hash, sort.
// - rlyeh, public domain

// -----------------------------------------------------------------------------
// less

API int less_64(uint64_t a, uint64_t b);
API int less_int(int a, int b);
API int less_ptr(void *a, void *b);
API int less_str(char *a, char *b);

// -----------------------------------------------------------------------------
// qsort

API int less_64_ptr(const void *a, const void *b);
API int less_int_ptr(const void *a, const void *b);

// -----------------------------------------------------------------------------
// un/hash

API uint32_t unhash_32(uint32_t x);
API uint32_t hash_32(uint32_t x);
API uint64_t hash_64(uint64_t x);
API uint64_t hash_flt(double x);
API uint64_t hash_int(int key);
API uint64_t hash_ptr(const void* ptr);
API uint64_t hash_bin(const void* ptr, unsigned len);
API uint64_t hash_str(const char* str);

// -----------------------------------------------------------------------------
// bits

API uint64_t popcnt64(uint64_t x);

// -----------------------------------------------------------------------------
// vector based allocator (x1.75 enlarge factor)

API void* vrealloc( void* p, size_t sz );
API size_t vlen( void* p );

// -----------------------------------------------------------------------------
// arrays

#if is(cpp)
#define array_cast(x) (decltype x)
#else
#define array_cast(x) (void *)
#endif

#define array(t) t*
#define array_init(t) ( (t) = 0 )
#define array_resize(t, n) ( array_c_ = array_count(t), array_n_ = (n), array_realloc_((t),array_n_), (array_n_>array_c_? memset(array_c_+(t),0,(array_n_-array_c_)*sizeof(0[t])) : (void*)0), (t) )
#define array_push(t, ...) ( array_realloc_((t),array_count(t)+1), (t)[ array_count(t) - 1 ] = (__VA_ARGS__) )
#define array_pop(t) ( array_realloc_((t), array_count(t)-1) )
#define array_back(t) ( &(t)[ array_count(t)-1 ] ) // ( (t) ? &(t)[ array_count(t)-1 ] : NULL )
#define array_data(t) (t)
#define array_at(t,i) (t[i])
#define array_count(t) (int)( (t) ? array_vlen_(t) / sizeof(0[t]) : 0u )
#define array_bytes(t) (int)( (t) ? array_vlen_(t) : 0u )
#define array_sort(t, cmpfunc) qsort( t, array_count(t), sizeof(0[t]), (uintptr_t)cmpfunc == (uintptr_t)strcmp ? (void*)strcmp_qsort : (void*)cmpfunc )
#define array_empty(t) ( !array_count(t) )

#define array_push_front(arr,x) \
    (array_resize((arr), array_count(arr)+1), memmove((arr)+1, (arr), sizeof(0[arr])*array_count(arr)), 0[arr] = (x))
#define array_pop_front(arr) ( \
    (array_count(arr) > 1 ? memmove((arr), (arr)+1, sizeof(0[arr])*(array_count(arr)-1)) : (void*)0), \
    (array_count(arr) > 0 ? array_resize(arr, array_count(arr) - 1 ) : array_resize( arr, 0 ) ) )

static __thread unsigned array_c_;
static __thread unsigned array_n_;

#if 0 // original: no reserve support
#define array_reserve(t, n) ((void)0) // not implemented
#define array_clear(t) ( array_realloc_((t), 0), (t) = 0 )
#define array_vlen_(t)  ( vlen(t) - 0 )
#define array_realloc_(t,n)  ( (t) = array_cast(t) vrealloc((t), ((n)+0) * sizeof(0[t])) )
#define array_free(t) array_clear(t)
#else // new: with reserve support (@todo: check for bugs?)
#define array_reserve(t, n) ( array_realloc_((t),(n)), array_clear(t) )
#define array_clear(t) ( array_realloc_((t),0) ) // -1
#define array_vlen_(t)  ( vlen(t) - sizeof(0[t]) ) // -1
#define array_realloc_(t,n)  ( (t) = array_cast(t) vrealloc((t), ((n)+1) * sizeof(0[t])) ) // +1
#define array_free(t) ( array_realloc_((t), -1), (t) = 0 ) // -1
#endif

#define array_reverse(t) \
    do if( array_count(t) ) { \
        for(int l = array_count(t), e = l-1, i = (array_push(t, 0[t]), 0); i <= e/2; ++i ) \
            { l[t] = i[t]; i[t] = (e-i)[t]; (e-i)[t] = l[t]; } \
        array_pop(t); \
    } while(0)

#define array_foreach(t,val_t,v) for each_array(t,val_t,v)
#define each_array(a,val_t,v) \
    ( array(val_t) a_ = (a); a_; a_ = 0 ) \
    for( int i_ = 0, e_ = array_count(a_); i_ < e_; ++i_ ) \
        for( val_t v = i_[a_], *v_ = (void*)(uintptr_t)&v; v_; v_ = 0 )

#define array_foreach_ptr(t,val_t,v) for each_array_ptr(t,val_t,v)
#define each_array_ptr(a,val_t,v) \
    ( array(val_t) a_ = (a); a_; a_ = 0 ) \
    for( int i_ = 0, e_ = array_count(a_); i_ < e_; ++i_ ) \
        for( val_t *v = (val_t*)&i_[a_]; v; v = 0 )

#define array_search(t, key, cmpfn) /* requires sorted array beforehand */ \
    bsearch(&key, t, array_count(t), sizeof(t[0]), cmpfn )

#define array_insert(t, i, n) do { \
    int ac = array_count(t); \
    if( i >= ac ) { \
        array_push(t, n); \
    } else { \
        array_push(t, array_back(t)); \
        memmove( &(t)[(i)+1], &(t)[i], (ac - (i)) * sizeof(t[0]) ); \
        (t)[ i ] = (n); \
    } \
} while(0)

#define array_copy(t, src) do { \
    array_free(t); \
    (t) = array_realloc_( (t), array_count(src)); \
    memcpy( (t), src, array_count(src) * sizeof(0[t])); \
} while(0)

#define array_erase_fast(t, i) do { /*alters ordering*/ \
    memcpy( &(t)[i], &(t)[array_count(t) - 1], sizeof(0[t])); \
    array_pop(t); \
} while(0)

#define array_erase_slow(t, i) do { /*preserves ordering*/ \
    memmove( &(t)[i], &(t)[i + 1], sizeof(0[t])*(array_count(t) - i - 1)); \
    array_pop(t); \
} while(0)

#define array_unique(t, cmpfunc) do { /*@todo: optimize me. requires array_sort() beforehand*/ \
    int cnt = array_count(t), cnt_bak = cnt; \
     if( cnt > 1 ) { \
        for( int i = 1; i < cnt; ++i ) { \
            while( i < cnt && !cmpfunc(&(t)[i-1], &(t)[i]) ) { \
                memmove(&(t)[i-1], &(t)[i], (cnt - 1 - i) * sizeof((t)[0]) ) ; \
                --cnt; \
            } \
         } \
        if( cnt_bak != cnt ) array_resize((t), cnt); \
     } \
 } while(0)

#if 0 // snippet below does work
#define array_unique(t, cmpfunc) \
    array_sort(t, cmpfunc); \
    for( int i = 0, end = array_count(t) - 1; i < end; ) { \
        if( !strcmp(t[i], t[i+1]) ) { \
            /* array_erase(t, i+1); */ \
            memmove(&(t)[i+1], &(t)[i+2], (end - 1 - i) * sizeof((t)[0]) ); \
            array_pop(t); \
            --end; \
        } else { \
            ++i; \
        } \
    }
#endif

#define array_shuffle(t) do { /* https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle */ \
    void* tmp = stack(sizeof(0[t])); \
    for( int i = 0, n = array_count(t); i < n; ++i ) { \
        int j = randi(i, n); /* j random integer such that [i,n) i<=j<n */ \
        memcpy(tmp, &j[t], sizeof(0[t])); \
        memcpy(&j[t], &i[t], sizeof(0[t])); \
        memcpy(&i[t], tmp, sizeof(0[t])); \
    } \
} while(0)

// -----------------------------------------------------------------------------
// set<K>
// ideas from: https://en.wikipedia.org/wiki/Hash_table
// ideas from: https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/
// ideas from: http://www.idryman.org/blog/2017/05/03/writing-a-damn-fast-hash-table-with-tiny-memory-footprints/

// config
#ifndef SET_HASHSIZE
#define SET_HASHSIZE (4096 << 4)
#endif
#ifndef SET_DONT_ERASE
#define SET_DONT_ERASE 1
#endif

// public api
#define set(K) \
    struct { set base; struct { set_item p; K key; } tmp, *ptr; K *tmpval; \
        int (*typed_cmp)(K, K); uint64_t (*typed_hash)(K); } *

#define set_init(m, cmpfn, hashfn) ( \
    (m) = set_cast(m) REALLOC(0, sizeof(*m)), \
    set_init(&(m)->base), \
    (m)->base.cmp = (int(*)(void*,void*))( (m)->typed_cmp = set_cast(cmpfn) cmpfn ), \
    (m)->base.hash = (uint64_t(*)(void*))( (m)->typed_hash = set_cast(hashfn) hashfn ) \
    )

#define set_free(m) ( \
    set_clear(m), \
    set_free(&(m)->base), \
    (m) = set_cast(m) REALLOC((m), 0), \
    (m) = 0 \
    )

#define set_insert(m, k) ( \
    (m)->ptr = set_cast((m)->ptr) REALLOC(0, sizeof((m)->tmp)), \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    set_insert(&(m)->base, &(m)->ptr->p, &(m)->ptr->key, (m)->ptr->p.keyhash, (m)->ptr), \
    &(m)->ptr->key \
    )

#define set_find(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    (m)->ptr = set_cast((m)->ptr) set_find(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash), \
    (m)->ptr ? &(m)->ptr->key : 0 \
    )

#define set_find_or_add(m, k) ( \
    (m)->tmp.key = (k), \
    (m)->tmpval = set_find((m), ((m)->tmp.key)), \
    (m)->tmpval = (m)->tmpval ? (m)->tmpval : set_insert((m), ((m)->tmp.key)) \
    )

#define set_find_or_add_allocated_key(m, k) ( \
    (m)->tmp.key = (k), \
    (m)->tmpval = set_find((m), ((m)->tmp.key)), \
    (m)->tmpval = (m)->tmpval ? FREE((m)->tmp.key), (m)->tmpval : set_insert((m), ((m)->tmp.key)) \
    )

#define set_erase(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    set_erase(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash) \
    )

#define set_foreach for each_set
#define each_set(m,key_t,k) \
    ( int i_ = (m)->base.count ? 0 : SET_HASHSIZE; i_ < SET_HASHSIZE; ++i_) \
        for( set_item *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t k = *(key_t *)cur_->key; on_; on_ = 0 )

#define set_foreach_ptr for each_set_ptr
#define each_set_ptr(m,key_t,k) \
    ( int i_ = (m)->base.count ? 0 : SET_HASHSIZE; i_ < SET_HASHSIZE; ++i_) \
        for( set_item *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t *k = (key_t *)cur_->key; on_; on_ = 0 )

#define set_clear(m) ( \
    set_clear(&(m)->base) \
    )

#define set_isempty(m)      set_isempty(&(m)->base)
#define set_count(m)        set_count(&(m)->base)
#define set_gc(m)           set_gc(&(m)->base)

#ifndef set_init_int
#define set_init_int(m) set_init((m), less_int, hash_64) // hash_int)
#define set_init_str(m) set_init((m), less_str, hash_str)
#define set_init_ptr(m) set_init((m), less_ptr, hash_ptr)
#endif

// private:

#if is(cpp)
#define set_cast(t) (decltype(t))
#else
#define set_cast(t) (void *)
#endif

typedef struct set_item {
    struct set_item *next;

    uint64_t keyhash;
    void *key;
    void *super;
} set_item;

typedef struct set {
    array(set_item*) array;
    int (*cmp)(void *, void *);
    uint64_t (*hash)(void *);
    int count;
} set;

API void  (set_init)(set *m);
API void  (set_free)(set *m);

API void  (set_insert)(set *m, set_item *p, void *key, uint64_t keyhash, void *super);
API void  (set_erase)(set *m, void *key, uint64_t keyhash);
API void* (set_find)(const set *m, void *key, uint64_t keyhash);
API int   (set_isempty)(const set *m);
API int   (set_count)(const set *m);
API void  (set_gc)(set *m); // only if using SET_DONT_ERASE
API void  (set_clear)(set* m);

// -----------------------------------------------------------------------------
// map<K,V>
// ideas from: https://en.wikipedia.org/wiki/Hash_table
// ideas from: https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/
// ideas from: http://www.idryman.org/blog/2017/05/03/writing-a-damn-fast-hash-table-with-tiny-memory-footprints/

// config
#ifndef MAP_HASHSIZE
#define MAP_HASHSIZE (4096 << 4)
#endif
#ifndef MAP_DONT_ERASE
#define MAP_DONT_ERASE 1
#endif

// public api
#define map(K,V) \
    struct { map base; struct { pair p; K key; V val; } tmp, *ptr; V* tmpval; \
        int (*typed_cmp)(K, K); uint64_t (*typed_hash)(K); } *

#define map_init(m, cmpfn, hashfn) ( \
    (m) = map_cast(m) REALLOC(0, sizeof(*(m))), \
    map_init(&(m)->base), \
    (m)->base.cmp = (int(*)(void*,void*))( (m)->typed_cmp = map_cast((m)->typed_cmp) cmpfn), \
    (m)->base.hash = (uint64_t(*)(void*))( (m)->typed_hash = map_cast((m)->typed_hash) hashfn ) \
    )

#define map_free(m) ( \
    map_free(&(m)->base), \
    map_cast(m) REALLOC((m), sizeof(*(m))), (m) = 0 \
    )

#define map_insert(m, k, v) ( \
    (m)->ptr = map_cast((m)->ptr) REALLOC(0, sizeof((m)->tmp)), \
    (m)->ptr->val = (v), \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    map_insert(&(m)->base, &(m)->ptr->p, &(m)->ptr->key, &(m)->ptr->val, (m)->ptr->p.keyhash, (m)->ptr), \
    &(m)->ptr->val \
    )

#define map_find(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    (m)->ptr = map_cast((m)->ptr) map_find(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash), \
    (m)->ptr ? &(m)->ptr->val : 0 \
    )

#define map_find_or_add(m, k, v) ( \
    (m)->tmp.key = (k), (m)->tmp.val = (v), \
    (m)->tmpval = map_find((m), ((m)->tmp.key)), \
    (m)->tmpval = (m)->tmpval ? (m)->tmpval : map_insert((m), ((m)->tmp.key), ((m)->tmp.val)) \
    )

#define map_find_or_add_allocated_key(m, k, v) ( \
    (m)->tmp.key = (k), (m)->tmp.val = (v), \
    (m)->tmpval = map_find((m), ((m)->tmp.key)), \
    (m)->tmpval = (m)->tmpval ? FREE((m)->tmp.key), (m)->tmpval : map_insert((m), ((m)->tmp.key), ((m)->tmp.val)) \
    )

#define map_erase(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    map_erase(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash) \
    )

#define map_foreach for each_map
#define each_map(m,key_t,k,val_t,v) \
    ( int i_ = (m)->base.count ? 0 : MAP_HASHSIZE; i_ < MAP_HASHSIZE; ++i_) \
        for( pair *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t k = *(key_t *)cur_->key; on_; ) \
                for( val_t v = *(val_t *)cur_->value; on_; on_ = 0 )

#define map_foreach_ptr for each_map_ptr
#define each_map_ptr(m,key_t,k,val_t,v) \
    ( int i_ = (m)->base.count ? 0 : MAP_HASHSIZE; i_ < MAP_HASHSIZE; ++i_) \
        for( pair *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t *k = (key_t *)cur_->key; on_; ) \
                for( val_t *v = (val_t *)cur_->value; on_; on_ = 0 )

#define map_foreach_ptr_sorted for each_map_ptr_sorted
#define each_map_ptr_sorted(m,key_t,k,val_t,v) \
    ( int i_ = (map_sort(&(m)->base), 0); i_ < array_count((m)->base.sorted); ++i_) \
        for( pair *cur_ = (m)->base.sorted[i_]; cur_; ) \
            for( key_t *k = (key_t *)cur_->key; cur_; ) \
                for( val_t *v = (val_t *)cur_->value; cur_; cur_ = 0 )

#define map_clear(m) ( \
    map_clear(&(m)->base) \
    )

#define map_isempty(m)      map_isempty(&(m)->base)
#define map_count(m)        map_count(&(m)->base)
#define map_gc(m)           map_gc(&(m)->base)

// aliases:

#ifndef map_init_int
#define map_init_int(m)     map_init((m), less_int, hash_64) // hash_int
#define map_init_str(m)     map_init((m), less_str, hash_str)
#define map_init_ptr(m)     map_init((m), less_ptr, hash_ptr)
#endif

// private:

#if is(cpp)
#define map_cast(t) (decltype(t))
#else
#define map_cast(t) (void *)
#endif

typedef struct pair {
    struct pair *next;

    uint64_t keyhash;
    void *key;
    void *value;
    void *super;
} pair;

typedef struct map {
    array(pair*) array;
    int (*cmp)(void *, void *);
    uint64_t (*hash)(void *);
    int count:31;

    int is_sorted:1;
    array(pair*) sorted;
} map;

API void  (map_init)(map *m);
API void  (map_free)(map *m);

API void  (map_insert)(map *m, pair *p, void *key, void *value, uint64_t keyhash, void *super);
API void  (map_erase)(map *m, void *key, uint64_t keyhash);
API void* (map_find)(map *m, void *key, uint64_t keyhash);
API int   (map_isempty)(map *m);
API int   (map_count)(map *m);
API void  (map_gc)(map *m); // only if using MAP_DONT_ERASE
API bool  (map_sort)(map* m);
API void  (map_clear)(map* m);
#line 0

#line 1 "fwk_math.h"
// -----------------------------------------------------------------------------
// math framework: rand, ease, vec2, vec3, vec4, quat, mat2, mat33, mat34, mat4
// - rlyeh, public domain
//
// Credits: @ands+@krig+@vurtun (PD), @datenwolf (WTFPL2), @evanw+@barerose (CC0), @sgorsten (Unlicense).

#define C_EPSILON  (1e-6)
#define C_PI       (3.14159265358979323846f) // (3.141592654f)
#define TO_RAD     (C_PI/180)
#define TO_DEG     (180/C_PI)

// ----------------------------------------------------------------------------

//#define ptr(type)         0[&(type).x]
#define vec2i(x, y     )  C_CAST(vec2i,(int)(x),   (int)(y)                          )
#define vec3i(x, y, z  )  C_CAST(vec3i,(int)(x),   (int)(y),   (int)(z)              )
#define vec2(x, y      )  C_CAST(vec2, (float)(x), (float)(y)                        )
#define vec3(x, y, z   )  C_CAST(vec3, (float)(x), (float)(y), (float)(z),           )
#define vec4(x, y, z, w)  C_CAST(vec4, (float)(x), (float)(y), (float)(z), (float)(w))
#define quat(x, y, z, w)  C_CAST(quat, (float)(x), (float)(y), (float)(z), (float)(w))
#define axis(x, y, z)     C_CAST(axis, (float)(x), (float)(y), (float)(z))
#define mat33(...)        C_CAST(mat33, __VA_ARGS__ )
#define mat34(...)        C_CAST(mat34, __VA_ARGS__ )
#define mat44(...)        C_CAST(mat44, __VA_ARGS__ )

typedef union vec2i{ struct { int X,Y; };       struct { int x,y; }; struct { int r,g; }; struct { int w,h; }; struct { int min,max; }; struct { int from,to; }; struct { int src,dst; }; int v2[2]; int array[1]; } vec2i;
typedef union vec3i{ struct { int X,Y,Z; };     struct { int x,y,z; }; struct { int r,g,b; }; struct { int w,h,d; }; struct { int min,max; }; struct { int from,to,step; }; struct { int src,dst; }; int v3[3]; int array[1]; } vec3i;
typedef union vec2 { struct { float X,Y; };     struct { float x,y; }; struct { float r,g; }; struct { float w,h; }; struct { float min,max; }; struct { float from,to; }; struct { float src,dst; }; float v2[2]; float array[1]; } vec2;
typedef union vec3 { struct { float X,Y,Z; };   struct { float x,y,z; }; struct { float r,g,b; }; struct { float min,max; }; struct { float from,to; }; vec2 xy; vec2 rg; vec2 wh; float v3[3]; float array[1]; } vec3;
typedef union vec4 { struct { float X,Y,Z,W; }; struct { float x,y,z,w; }; struct { float r,g,b,a; }; struct { float min,max; }; struct { float from,to; }; vec2 xy; vec3 xyz; vec2 rg; vec3 rgb; vec2 wh; vec3 whd; float v4[4]; float array[1]; } vec4;
typedef union quat { struct { float X,Y,Z,W; }; struct { float x,y,z,w; }; vec3 xyz; vec4 xyzw; float v4[4]; float array[1]; } quat;
typedef float mat33[9];
typedef float mat34[12];
typedef float mat44[16];

// ----------------------------------------------------------------------------

API void     randset(uint64_t state);
API uint64_t rand64(void);
API double   randf(void); // [0, 1) interval
API int      randi(int mini, int maxi); // [mini, maxi) interval

// ----------------------------------------------------------------------------

API float simplex1( float x );
API float simplex2( vec2 xy );
API float simplex3( vec3 xyz );
API float simplex4( vec4 xyzw );

// ----------------------------------------------------------------------------

API float deg      (float radians);
API float rad      (float degrees);

API int   mini     (int    a, int    b);
API int   maxi     (int    a, int    b);
API int   absi     (int    a          );
API int   clampi   (int v,int a,int b);

API float minf     (float  a, float  b);
API float maxf     (float  a, float  b);
API float absf     (float  a          );
API float pmodf    (float  a, float  b);
API float signf    (float  a)           ;
API float clampf   (float v,float a,float b);
API float mixf     (float a,float b,float t);
API float fractf   (float a);

// ----------------------------------------------------------------------------

API vec2  ptr2     (const float *a    );
//
API vec2  neg2     (vec2   a          );
API vec2  add2     (vec2   a, vec2   b);
API vec2  sub2     (vec2   a, vec2   b);
API vec2  mul2     (vec2   a, vec2   b);
API vec2  div2     (vec2   a, vec2   b);
API vec2  inc2     (vec2   a, float  b);
API vec2  dec2     (vec2   a, float  b);
API vec2  scale2   (vec2   a, float  b);
API vec2  pmod2    (vec2   a, float  b);
API vec2  min2     (vec2   a, vec2   b);
API vec2  max2     (vec2   a, vec2   b);
API vec2  abs2     (vec2   a          );
API vec2  floor2   (vec2   a          );
API vec2  fract2   (vec2   a          );
API vec2  ceil2    (vec2   a          );
API float dot2     (vec2   a, vec2   b);
API vec2  refl2    (vec2   a, vec2   b);
API float cross2   (vec2   a, vec2   b);
API float len2sq   (vec2   a          );
API float len2     (vec2   a          );
API vec2  norm2    (vec2   a          );
API int   finite2  (vec2   a          );
API vec2  mix2  (vec2 a,vec2 b,float t);
API vec2  clamp2(vec2 v,vec2 a,vec2 b);
API vec2  clamp2f(vec2 v,float a,float b);
// ----------------------------------------------------------------------------

API vec3  rnd3     (void); // @todo: rnd2,rnd4,rndq
API vec3  ptr3     (const float *a    );
API vec3  vec23    (vec2   a, float z );
//
API vec3  neg3     (vec3   a          );
API vec3  add3     (vec3   a, vec3   b);
API vec3  sub3     (vec3   a, vec3   b);
API vec3  mul3     (vec3   a, vec3   b);
API vec3  div3     (vec3   a, vec3   b);
API vec3  inc3     (vec3   a, float  b);
API vec3  dec3     (vec3   a, float  b);
API vec3  scale3   (vec3   a, float  b);
API vec3  pmod3    (vec3   a, float  b);
API vec3  min3     (vec3   a, vec3   b);
API vec3  max3     (vec3   a, vec3   b);
API vec3  abs3     (vec3   a          );
API vec3  floor3   (vec3   a          );
API vec3  fract3   (vec3   a          );
API vec3  ceil3    (vec3   a          );
API vec3  cross3   (vec3   a, vec3   b);
API float dot3     (vec3   a, vec3   b);
API vec3  refl3    (vec3   a, vec3   b);
API float len3sq   (vec3   a          );
API float len3     (vec3   a          );
API vec3  norm3    (vec3   a          );
API vec3  norm3sq  (vec3   a          );
API int   finite3  (vec3   a          );
API vec3  mix3  (vec3 a,vec3 b,float t);
API vec3  clamp3(vec3 v,vec3 a,vec3 b);
API vec3  clamp3f(vec3 v,float a,float b);
//vec3 tricross3 (vec3 a, vec3 b, vec3 c);
API void  ortho3   (vec3 *left, vec3 *up, vec3 v);

API vec3  rotatex3 (vec3 dir, float degrees);
API vec3  rotatey3 (vec3 dir, float degrees);
API vec3  rotatez3 (vec3 dir, float degrees);

// ----------------------------------------------------------------------------

API vec4  ptr4     (const float *a    );
API vec4  vec34    (vec3   a, float w );
//
API vec4  neg4     (vec4   a          );
API vec4  add4     (vec4   a, vec4   b);
API vec4  sub4     (vec4   a, vec4   b);
API vec4  mul4     (vec4   a, vec4   b);
API vec4  div4     (vec4   a, vec4   b);
API vec4  inc4     (vec4   a, float  b);
API vec4  dec4     (vec4   a, float  b);
API vec4  scale4   (vec4   a, float  b);
API vec4  pmod4    (vec4   a, float  b);
API vec4  min4     (vec4   a, vec4   b);
API vec4  max4     (vec4   a, vec4   b);
API vec4  abs4     (vec4   a          );
API vec4  floor4   (vec4   a          );
API vec4  fract4   (vec4   a          );
API vec4  ceil4    (vec4   a          );
API float dot4     (vec4   a, vec4   b);
API vec4  refl4    (vec4   a, vec4   b);
API float len4sq   (vec4   a          );
API float len4     (vec4   a          );
API vec4  norm4    (vec4   a          );
API vec4  norm4sq  (vec4   a          );
API int   finite4  (vec4   a          );
API vec4  mix4  (vec4 a,vec4 b,float t);
API vec4  clamp4(vec4 v,vec4 a,vec4 b);
API vec4  clamp4f(vec4 v,float a,float b);
// vec4 cross4(vec4 v0, vec4 v1);

// ----------------------------------------------------------------------------

API quat  idq      (                  );
API quat  ptrq     (const float *a    );
API quat  vec3q    (vec3   a, float w );
API quat  vec4q    (vec4   a          );
//
API quat  negq     (quat   a          );
API quat  conjq    (quat   a          );
API quat  addq     (quat   a, quat   b);
API quat  subq     (quat   a, quat   b);
API quat  mulq     (quat   p, quat   q);
API quat  scaleq   (quat   a, float  s);
API quat  normq    (quat   a          );
API float dotq     (quat   a, quat   b);
API quat  mixq(quat a, quat b, float t);
/* quat lerpq(quat a, quat b, float s);
    return norm(quat((1-s)*a.x + s*b.x, (1-s)*a.y + s*b.y, (1-s)*a.z + s*b.z, (1-s)*a.w + s*b.w));
}*/
API quat slerpq(quat a, quat b, float s);

API quat  rotationq(float deg,float x,float y,float z);
API quat  mat44q   (mat44 M);

API vec3 rotate3q_2(vec3 v, quat q);
API vec3 rotate3q(vec3 v, quat r);

// euler <-> quat
API vec3  euler    (quat q);
API quat  eulerq   (vec3 pyr_degrees);

// ----------------------------------------------------------------------------

API void scaling33(mat33 m, float x, float y, float z);
API void scale33(mat33 m, float x, float y, float z);
API void id33(mat33 m);
API void extract33(mat33 m, const mat44 m4);
API void copy33(mat33 m, const mat33 a);//
API vec3 mulv33(mat33 m, vec3 v);
API void multiply33x2(mat33 m, const mat33 a, const mat33 b);

API void rotation33(mat33 m, float degrees, float x,float y,float z);
API void rotationq33(mat33 m, quat q);
API void rotate33(mat33 r, float degrees, float x,float y,float z);
API void compose33(mat33 m, quat r, vec3 s);

// ----------------------------------------------------------------------------

API void id34(mat34 m);
API void copy34(mat34 m, const mat34 a);
API void scale34(mat34 m, float s);
API void add34(mat34 m, mat34 n);
API void muladd34(mat34 m, mat34 n, float s);
API void add34x2(mat34 m, mat34 n, mat34 o);
API void lerp34(mat34 m, mat34 n, mat34 o, float alpha); // mix34?
API void multiply34x2(mat34 m, const mat34 m0, const mat34 m1);
API void multiply34(mat34 m, const mat34 a);
API void multiply34x3(mat34 m, const mat34 a, const mat34 b, const mat34 c);
API void compose34(mat34 m, vec3 t, quat q, vec3 s);
API void invert34(mat34 m, const mat34 o);

// ----------------------------------------------------------------------------

API void id44(mat44 m);
API void identity44(mat44 m);
API void copy44(mat44 m, const mat44 a);
API void multiply44x2(mat44 m, const mat44 a, const mat44 b);
API void multiply44x3(mat44 m, const mat44 a, const mat44 b, const mat44 c);
API void multiply44(mat44 m, const mat44 a);
// ---
API void ortho44(mat44 m, float l, float r, float b, float t, float n, float f);
API void frustum44(mat44 m, float l, float r, float b, float t, float n, float f);
API void perspective44(mat44 m, float fovy_degrees, float aspect, float nearp, float farp);
API void lookat44(mat44 m, vec3 eye, vec3 center, vec3 up);
// ---
API void translation44(mat44 m, float x, float y, float z);
API void translate44(mat44 m, float x, float y, float z);
API void relocate44(mat44 m, float x, float y, float z);
API void rotationq44(mat44 m, quat q);
API void rotation44(mat44 m, float degrees, float x, float y, float z);
API void rotate44(mat44 m, float degrees, float x, float y, float z);
API void scaling44(mat44 m, float x, float y, float z);
API void scale44(mat44 m, float x, float y, float z);
// ---
API void transpose44(mat44 m, const mat44 a);
API float det44(const mat44 M);
API bool invert44(mat44 T, const mat44 M);

API void compose44(mat44 m, vec3 t, quat q, vec3 s);

// ----------------------------------------------------------------------------

API vec3 transformq(const quat q, const vec3 v);
API vec3 transform33(const mat33 m, vec3 p);
API vec3 transform344(const mat44 m, const vec3 p);
API vec4 transform444(const mat44 m, const vec4 p);
API bool unproject44(vec3 *out, vec3 xyd, vec4 viewport, mat44 mvp);

// ----------------------------------------------------------------------------
// debugging and utils

API void print2i( vec2i v );
API void print3i( vec3i v );
API void print2( vec2 v );
API void print3( vec3 v );
API void print4( vec4 v );
API void printq( quat q );
API void print33( float *m );
API void print34( float *m );
API void print44( float *m );
#line 0

#line 1 "fwk_obj.h"
// -----------------------------------------------------------------------------
// factory of handle ids

// convert between hard refs (pointers) and weak refs (ids)
API uintptr_t id_make(void *ptr);
API void *     id_handle(uintptr_t id);
API void       id_dispose(uintptr_t id);
API bool        id_valid(uintptr_t id);

// configuration:
// ideally, these two should be 32 each. they were changed to fit our OBJHEADER bits
#ifndef ID_INDEX_BITS
#define ID_INDEX_BITS 16
#endif
#ifndef ID_COUNT_BITS
#define ID_COUNT_BITS  3
#endif

// C objects framework
// - rlyeh, public domain.
//
// ## object limitations
// - 8-byte overhead per object
// - XX-byte overhead per object-entity
// - 32 components max per object-entity
// - 256 classes max per game
// - 256 references max per object
// - 1024K bytes max per object
// - 8 generations + 64K IDs per running instance (19-bit IDs)
// - support for pragma pack(1) structs not enabled by default.

/* /!\ if you plan to use pragma pack(1) on any struct, you need #define OBJ_MIN_PRAGMAPACK_BITS 0 at the expense of max class size /!\ */
#ifndef   OBJ_MIN_PRAGMAPACK_BITS
//#define OBJ_MIN_PRAGMAPACK_BITS 3 // allows pragma packs >= 8. objsizew becomes 8<<3, so 2048 bytes max per class (default)
#define   OBJ_MIN_PRAGMAPACK_BITS 2 // allows pragma packs >= 4. objsizew becomes 8<<2, so 1024 bytes max per class
//#define OBJ_MIN_PRAGMAPACK_BITS 1 // allows pragma packs >= 2. objsizew becomes 8<<1, so  512 bytes max per class
//#define OBJ_MIN_PRAGMAPACK_BITS 0 // allows pragma packs >= 1. objsizew becomes 8<<0, so  256 bytes max per class
#endif

#define OBJHEADER \
    struct { \
        ifdef(debug, const char *objname;) \
        union { \
            uintptr_t objheader; \
            struct {  \
            uintptr_t objtype:8; \
            uintptr_t objsizew:8; \
            uintptr_t objrefs:8; \
            uintptr_t objheap:1; \
            uintptr_t objcomps:1; /* << can be removed? check payload ptr instead? */ \
            uintptr_t objunused:64-8-8-8-1-1-ID_INDEX_BITS-ID_COUNT_BITS; /*19*/ \
            uintptr_t objid:ID_INDEX_BITS+ID_COUNT_BITS; /*16+3*/ \
            }; \
        }; \
        array(struct obj*) objchildren; \
    };

#define OBJ \
    OBJHEADER

// ----------------------------------------------------------------------------
// syntax sugars

#ifdef OBJTYPE
#undef OBJTYPE
#endif

#define OBJTYPE(T) \
    OBJTYPE_##T

#define OBJTYPEDEF(NAME,N) \
     enum { OBJTYPE(NAME) = N }; \
     STATIC_ASSERT( N <= 255 ); \
     STATIC_ASSERT( sizeof(NAME) == ((sizeof(NAME)>>OBJ_MIN_PRAGMAPACK_BITS)<<OBJ_MIN_PRAGMAPACK_BITS) ); // (sizeof(NAME) & ((1<<OBJ_MIN_PRAGMAPACK_BITS)-1)) == 0 );

// ----------------------------------------------------------------------------
// objects

#define TYPEDEF_STRUCT(NAME,N,...) \
    typedef struct NAME { OBJ \
        __VA_ARGS__ \
        char payload[0]; \
    } NAME; OBJTYPEDEF(NAME,N);

// TYPEDEF_STRUCT(obj,0);
    typedef struct obj { OBJ } obj;

// ----------------------------------------------------------------------------
// entities

#define OBJCOMPONENTS_MAX 32
#define OBJCOMPONENTS_ALL_ENABLED 0xAAAAAAAAAAAAAAAAULL
#define OBJCOMPONENTS_ALL_FLAGGED 0x5555555555555555ULL
#define COMPONENTS_ONLY(x) ((x) & ~OBJCOMPONENTS_ALL_FLAGGED)

#define ENTITY \
    struct { OBJHEADER union { struct { uintptr_t objenabled:OBJCOMPONENTS_MAX, objflagged:OBJCOMPONENTS_MAX; }; uintptr_t cflags; }; void *c[OBJCOMPONENTS_MAX]; };

#define TYPEDEF_ENTITY(NAME,N,...) \
    typedef struct NAME { ENTITY \
        __VA_ARGS__ \
        char payload[0]; \
    } NAME; OBJTYPEDEF(NAME,N);

// OBJTYPEDEF(entity,1)
    typedef struct entity { ENTITY } entity;

#define entity_new(TYPE, ...)             OBJ_CTOR(TYPE, #TYPE, 1, 0, __VA_ARGS__)
#define entity_new_ext(TYPE, NAME, ...)   OBJ_CTOR(TYPE,  NAME, 1, 0, __VA_ARGS__)

// ----------------------------------------------------------------------------
// heap/stack ctor/dtor

static __thread obj *objtmp;
#define OBJ_CTOR_HDR(PTR,HEAP,SIZEOF_OBJ,OBJ_TYPE) ( \
        (PTR)->objheader = HEAP ? id_make(PTR) : 0, /*should assign to .objid instead. however, id_make() returns shifted bits already*/ \
        (PTR)->objtype = (OBJ_TYPE), \
        (PTR)->objheap = (HEAP), \
        (PTR)->objsizew = (SIZEOF_OBJ>>OBJ_MIN_PRAGMAPACK_BITS))
#define OBJ_CTOR_PTR(PTR,HEAP,SIZEOF_OBJ,OBJ_TYPE) ( \
        OBJ_CTOR_HDR(PTR,HEAP,SIZEOF_OBJ,OBJ_TYPE), \
        obj_ctor(PTR))
#define OBJ_CTOR(TYPE, NAME, HEAP, PAYLOAD_SIZE, ...) (TYPE*)( \
        objtmp = (HEAP ? MALLOC(sizeof(TYPE)+(PAYLOAD_SIZE)) : ALLOCA(sizeof(TYPE)+(PAYLOAD_SIZE))), \
        *(TYPE*)objtmp = ((TYPE){ {0,}, __VA_ARGS__}), \
        ((PAYLOAD_SIZE) ? memset((char*)objtmp + sizeof(TYPE), 0, (PAYLOAD_SIZE)) : objtmp), \
        ( OBJTYPES[ OBJTYPE(TYPE) ] = #TYPE ), \
        OBJ_CTOR_PTR(objtmp, HEAP,sizeof(TYPE),OBJTYPE(TYPE)), \
        ifdef(debug, (obj_printf)(objtmp, va("%s", callstack(+16))), 0), \
        obj_setname(objtmp, NAME))

#define obj(TYPE, ...)                *OBJ_CTOR(TYPE, #TYPE, 0, 0, __VA_ARGS__)
#define obj_new(TYPE, ...)             OBJ_CTOR(TYPE, #TYPE, 1, 0, __VA_ARGS__)
#define obj_new_ext(TYPE, NAME, ...)   OBJ_CTOR(TYPE,  NAME, 1, 0, __VA_ARGS__)

void*   obj_malloc(unsigned sz);
void*   obj_free(void *o);

// ----------------------------------------------------------------------------
// obj generics. can be extended.

#define obj_ctor(o,...) obj_method(ctor, o, ##__VA_ARGS__)
#define obj_dtor(o,...) obj_method(dtor, o, ##__VA_ARGS__)

#define obj_save(o,...) obj_method(save, o, ##__VA_ARGS__)
#define obj_load(o,...) obj_method(load, o, ##__VA_ARGS__)

#define obj_test(o,...) obj_method(test, o, ##__VA_ARGS__)

#define obj_init(o,...) obj_method(init, o, ##__VA_ARGS__)
#define obj_quit(o,...) obj_method(quit, o, ##__VA_ARGS__)
#define obj_tick(o,...) obj_method(tick, o, ##__VA_ARGS__)
#define obj_draw(o,...) obj_method(draw, o, ##__VA_ARGS__)

#define obj_lerp(o,...) obj_method(lerp, o, ##__VA_ARGS__)
#define obj_edit(o,...) obj_method(edit, o, ##__VA_ARGS__)
#define obj_menu(o,...) obj_method(menu, o, ##__VA_ARGS__)
#define obj_aabb(o,...) obj_method(aabb, o, ##__VA_ARGS__)
#define obj_icon(o,...) obj_method(icon, o, ##__VA_ARGS__)

// --- syntax sugars

#define obj_extend(T,method)       (obj_##method[OBJTYPE(T)] = (void*)T##_##method)
#define obj_extend_t(T,method)     (obj_##method[OBJTYPE(T##_t)] = (void*)T##_##method)
#define obj_method(method,o,...)   (obj_##method[((struct obj*)(o))->objtype](o,##__VA_ARGS__)) // (obj_##method[((struct obj*)(o))->objtype]((o), ##__VA_ARGS__))
#define obj_hasmethod(o,method)    (obj_typeid(o)[obj_##method])

#define obj_vtable(method,RC,...)   RC macro(obj_##method)(){ __VA_ARGS__ }; RC (*obj_##method[256])() = { REPEAT256(macro(obj_##method)) };
#define obj_vtable_null(method,RC)  RC (*obj_##method[256])() = { 0 }; // null virtual table. will crash unless obj_extend'ed

#define REPEAT16(f)  f,f,f,f,f,f,f,f,f,f,f,f,f,f,f,f ///-
#define REPEAT64(f)  REPEAT16(f),REPEAT16(f),REPEAT16(f),REPEAT16(f) ///-
#define REPEAT256(f) REPEAT64(f),REPEAT64(f),REPEAT64(f),REPEAT64(f) ///-

#undef  EXTEND
#define EXTEND(...) EXPAND(EXTEND, __VA_ARGS__)
#define EXTEND2(o,F1) obj_extend(o,F1) ///-
#define EXTEND3(o,F1,F2) obj_extend(o,F1), obj_extend(o,F2) ///-
#define EXTEND4(o,F1,F2,F3) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3) ///-
#define EXTEND5(o,F1,F2,F3,F4) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4) ///-
#define EXTEND6(o,F1,F2,F3,F4,F5) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4), obj_extend(o,F5) ///-
#define EXTEND7(o,F1,F2,F3,F4,F5,F6) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4), obj_extend(o,F5), obj_extend(o,F6) ///-
#define EXTEND8(o,F1,F2,F3,F4,F5,F6,F7) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4), obj_extend(o,F5), obj_extend(o,F6), obj_extend(o,F7) ///-
#define EXTEND9(o,F1,F2,F3,F4,F5,F6,F7,F8) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4), obj_extend(o,F5), obj_extend(o,F6), obj_extend(o,F7), obj_extend(o,F8) ///-
#define EXTEND10(o,F1,F2,F3,F4,F5,F6,F7,F8,F9) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4), obj_extend(o,F5), obj_extend(o,F6), obj_extend(o,F7), obj_extend(o,F8), obj_extend(o,F9) ///-

#define EXTEND_T(...) EXPAND(EXTEND_T, __VA_ARGS__)
#define EXTEND_T2(o,F1) obj_extend_t(o,F1) ///-
#define EXTEND_T3(o,F1,F2) obj_extend_t(o,F1), obj_extend_t(o,F2) ///-
#define EXTEND_T4(o,F1,F2,F3) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3) ///-
#define EXTEND_T5(o,F1,F2,F3,F4) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4) ///-
#define EXTEND_T6(o,F1,F2,F3,F4,F5) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4), obj_extend_t(o,F5) ///-
#define EXTEND_T7(o,F1,F2,F3,F4,F5,F6) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4), obj_extend_t(o,F5), obj_extend_t(o,F6) ///-
#define EXTEND_T8(o,F1,F2,F3,F4,F5,F6,F7) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4), obj_extend_t(o,F5), obj_extend_t(o,F6), obj_extend_t(o,F7) ///-
#define EXTEND_T9(o,F1,F2,F3,F4,F5,F6,F7,F8) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4), obj_extend_t(o,F5), obj_extend_t(o,F6), obj_extend_t(o,F7), obj_extend_t(o,F8) ///-
#define EXTEND_T10(o,F1,F2,F3,F4,F5,F6,F7,F8,F9) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4), obj_extend_t(o,F5), obj_extend_t(o,F6), obj_extend_t(o,F7), obj_extend_t(o,F8), obj_extend_t(o,F9) ///-

// --- declare vtables

API extern void  (*obj_ctor[256])(); ///-
API extern void  (*obj_dtor[256])(); ///-

API extern char* (*obj_save[256])(); ///-
API extern bool  (*obj_load[256])(); ///-
API extern int   (*obj_test[256])(); ///-

API extern int   (*obj_init[256])(); ///-
API extern int   (*obj_quit[256])(); ///-
API extern int   (*obj_tick[256])(); ///-
API extern int   (*obj_draw[256])(); ///-
API extern int   (*obj_lerp[256])(); ///-

API extern int   (*obj_aabb[256])(); ///-
API extern int   (*obj_edit[256])(); ///-
API extern int   (*obj_menu[256])(); ///-
API extern char* (*obj_icon[256])(); ///-

API extern const char*OBJTYPES[256]; ///-

// ----------------------------------------------------------------------------
// core

API uintptr_t   obj_header(const void *o);
API uintptr_t   obj_id(const void *o);

API const char* obj_type(const void *o);
API unsigned    obj_typeid(const void *o);

API int         obj_sizeof(const void *o);
API int         obj_size(const void *o); // size of all members together in struct. may include padding bytes.

API char*       obj_data(void *o); // pointer to the first member in struct
API const char* obj_datac(const void *o); // const pointer to the first struct member

API void*       obj_payload(const void *o); // pointer right after last member in struct
API void*       obj_zero(void *o); // reset all object members

// ----------------------------------------------------------------------------
// refcounting

API void*       obj_ref(void *oo);
API void*       obj_unref(void *oo);

// ----------------------------------------------------------------------------
// scene tree

#define each_objchild(p,T,o) /*non-recursive*/ \
    (array(struct obj*)* children = obj_children(p); children; children = 0) \
        for(int _i = 1, _end = array_count(*children); _i < _end; ++_i) \
            for(T o = (T)((*children)[_i]); o && (obj_parent(o) == p); o = 0)

API obj*        obj_detach(void *c);
API obj*        obj_attach(void *o, void *c);

API obj*        obj_root(const void *o);
API obj*        obj_parent(const void *o);
API array(obj*)*obj_children(const void *o); // child[0]: parent, child[1]: 1st child, child[2]: 2nd child...
API array(obj*)*obj_siblings(const void *o); // child[0]: grandpa, child[1]: sibling1, child[2]: sibling2...

API int         obj_dumptree(const void *o);

// ----------------------------------------------------------------------------
// metadata

API void*       obj_setmeta(void *o, const char *key, const char *value);
API const char* obj_meta(const void *o, const char *key);

API void*       obj_setname(void *o, const char *name);
API const char* obj_name(const void *o);

// ----------------------------------------------------------------------------
// stl

API void*       obj_swap(void *dst, void *src);
API void*       obj_copy_fast(void *dst, const void *src);
API void*       obj_copy(void *dst, const void *src);

API int         obj_comp_fast(const void *a, const void *b);
API int         obj_comp(const void *a, const void *b);
API int         obj_lesser(const void *a, const void *b);
API int         obj_greater(const void *a, const void *b);
API int         obj_equal(const void *a, const void *b);

API uint64_t    obj_hash(const void *o);

// ----------------------------------------------------------------------------
// debug

API bool        obj_hexdump(const void *oo);
API int         obj_print(const void *o);

API int         obj_printf(const void *o, const char *text);
API int         obj_console(const void *o); // obj_output() ?

#define         obj_printf(o, ...) obj_printf(o, va(__VA_ARGS__))

// ----------------------------------------------------------------------------
// serialization

API char*       obj_saveini(const void *o);
API obj*        obj_mergeini(void *o, const char *ini);
API obj*        obj_loadini(void *o, const char *ini);

API char*       obj_savejson(const void *o);
API obj*        obj_mergejson(void *o, const char *json);
API obj*        obj_loadjson(void *o, const char *json);

API char*       obj_savebin(const void *o);
API obj*        obj_mergebin(void *o, const char *sav);
API obj*        obj_loadbin(void *o, const char *sav);

API char*       obj_savempack(const void *o); // @todo
API obj*        obj_mergempack(void *o, const char *sav); // @todo
API obj*        obj_loadmpack(void *o, const char *sav); // @todo

API int         obj_push(const void *o);
API int         obj_pop(void *o);

// ----------------------------------------------------------------------------
// components

API bool        obj_addcomponent(entity *e, unsigned c, void *ptr);
API bool        obj_hascomponent(entity *e, unsigned c);
API void*       obj_getcomponent(entity *e, unsigned c);
API bool        obj_delcomponent(entity *e, unsigned c);
API bool        obj_usecomponent(entity *e, unsigned c);
API bool        obj_offcomponent(entity *e, unsigned c);

API char*       entity_save(entity *self);

// ----------------------------------------------------------------------------
// reflection

#define each_objmember(oo,TYPE,NAME,PTR) \
    (array(reflect_t) *found_ = members_find(obj_type(oo)); found_; found_ = 0) \
        for(int it_ = 0, end_ = array_count(*found_); it_ != end_; ++it_ ) \
            for(reflect_t *R = &(*found_)[it_]; R; R = 0 ) \
                for(const char *NAME = R->name, *TYPE = R->type; NAME || TYPE; ) \
                    for(void *PTR = ((char*)oo) + R->sz ; NAME || TYPE ; NAME = TYPE = 0 )

API void*       obj_clone(const void *src);
API void*       obj_merge(void *dst, const void *src); // @testme
API void*       obj_mutate(void *dst, const void *src);
API void*       obj_make(const char *str);

// built-ins

typedef enum OBJTYPE_BUILTINS {
    OBJTYPE_obj    =  0,
    OBJTYPE_entity =  1,
    OBJTYPE_vec2   =  2,
    OBJTYPE_vec3   =  3,
    OBJTYPE_vec4   =  4,
    OBJTYPE_quat   =  5,
    OBJTYPE_mat33  =  6,
    OBJTYPE_mat34  =  7,
    OBJTYPE_mat44  =  8,
    OBJTYPE_vec2i  =  9,
    OBJTYPE_vec3i  = 10,
} OBJTYPE_BUILTINS;
#line 0


#line 1 "fwk_ai.h"
// AI framework
// - rlyeh, public domain.
//
// [src] original A-star code by @mmozeiko (PD) - https://gist.github.com/mmozeiko/68f0a8459ef2f98bcd879158011cc275
// [src] original swarm/boids code by @Cultrarius (UNLICENSE) - https://github.com/Cultrarius/Swarmz

// pathfinding -----------------------------------------------------------------

API int pathfind_astar(int width, int height, const unsigned* map, vec2i src, vec2i dst, vec2i* path, size_t maxpath);

// ----------------------------------------------------------------------------
// Behavior trees: decision planning and decision making.
// Supersedes finite state-machines (FSM) and hierarchical finite state-machines (HFSM).

typedef int (*bt_func)();

typedef struct bt_t {
    uint64_t type;
    int (*action)();
    union {
        int argi;
        float argf;
    };
    array(struct bt_t) children;
} bt_t;

API bt_t    bt(const char *ini_file, unsigned flags);
API int     bt_run(bt_t *b);
API void    bt_addfun(const char *name, int(*func)());
API bt_func bt_findfun(const char *name);
API char   *bt_funcname(bt_func fn);

API int ui_bt(bt_t *b);

// boids/swarm -----------------------------------------------------------------

typedef enum SWARM_DISTANCE {
    SWARM_DISTANCE_LINEAR,
    SWARM_DISTANCE_INVERSE_LINEAR,
    SWARM_DISTANCE_QUADRATIC,
    SWARM_DISTANCE_INVERSE_QUADRATIC
} SWARM_DISTANCE;

#define boid(...) C_CAST(boid_t, __VA_ARGS__)

typedef struct boid_t {
    vec3 position;
    vec3 velocity;
    vec3 acceleration;
    vec3 prev_position;
} boid_t;

typedef struct swarm_t {
    array(boid_t) boids;

    float perception_radius; // determines the vision radius of each boid. Only boids within this distance influence each other.

    float separation_weight; // how much boids repel each other
    SWARM_DISTANCE separation_type;

    float alignment_weight; // how much boids want go in the same direction
    float cohesion_weight; // how much boids want to be in the center of the swarm

    float steering_weight;
    array(vec3) steering_targets;
    SWARM_DISTANCE steering_target_type;

    float blindspot_angledeg;
    float max_acceleration; // how fast each boid can change its direction
    float max_velocity; // how fast each boid can move

    // private:
    map(vec3*, array(boid_t*)) voxel_cache_;
    float blindspot_angledeg_compare_value_;
} swarm_t;

API swarm_t swarm();
API void    swarm_update(swarm_t *self, float delta); // acc,vel,pos
API void    swarm_update_acceleration_only(swarm_t *self); // acc
API void    swarm_update_acceleration_and_velocity_only(swarm_t *self, float delta); // acc,vel

API int     ui_swarm(swarm_t *self);
#line 0

#line 1 "fwk_audio.h"
// -----------------------------------------------------------------------------
// audio framework
// - rlyeh, public domain
//
// fixme: leaks, audio_delete
// @todo: audio_volume_fx, audio_volume_bgm, audio_volume_master instead?
// @todo: destroystream()    if( ss->type == WAV ) drwav_uninit(&ss->wav);
// @todo: destroystream()    if( ss->type == MOD ) jar_mod_unload(&ss->mod);
// @todo: destroystream()    if( ss->type == XM && ss->xm ) jar_xm_free_context(ss->xm);

// midi interface

API void    midi_send(unsigned midi_msg);

// audio interface

typedef struct audio_handle* audio_t;

API audio_t audio_clip( const char *pathfile );
API audio_t audio_stream( const char *pathfile );
API int     audio_play( audio_t s, int flags );
API int     audio_play_gain( audio_t a, int flags, float gain/*0*/ );
API int     audio_play_gain_pitch( audio_t a, int flags, float gain, float pitch/*1*/ );
API int     audio_play_gain_pitch_pan( audio_t a, int flags, float gain, float pitch, float pan/*0*/ );
API int     audio_stop( audio_t a );
API void    audio_loop( audio_t a, bool loop );
API bool    audio_playing( audio_t a );

API float   audio_volume_clip(float gain);   // set     fx volume if gain is in [0..1] range. returns current     fx volume in any case
API float   audio_volume_stream(float gain); // set    bgm volume if gain is in [0..1] range. returns current    bgm volume in any case
API float   audio_volume_master(float gain); // set master volume if gain is in [0..1] range. returns current master volume in any case

API int     audio_mute(int mute);
API int     audio_muted();

API int ui_audio();

enum AUDIO_FLAGS {
    AUDIO_1CH = 0, // default
    AUDIO_2CH = 1,

    AUDIO_8 = 2,
    AUDIO_16 = 0, // default
    AUDIO_32 = 4,
    AUDIO_FLOAT = 8,

    AUDIO_8KHZ = 16,
    AUDIO_11KHZ = 32,
    AUDIO_22KHZ = 0, // default
    AUDIO_32KHZ = 64,
    AUDIO_44KHZ = 128,

    AUDIO_MIXER_GAIN = 0, // default
    AUDIO_IGNORE_MIXER_GAIN = 256,

    AUDIO_MULTIPLE_INSTANCES = 0, // default
    AUDIO_SINGLE_INSTANCE = 512,
};

API int audio_queue( const void *samples, int num_samples, int flags );
#line 0

#line 1 "fwk_collide.h"
// -----------------------------------------------------------------------------
// original code by @vurtun (PD) and @barerose (CC0).
// [src] https://gist.github.com/vurtun/95f088e4889da2474ad1ce82d7911fee
// - rlyeh, public domain.

#ifndef GJK_H
#define GJK_H

#define GJK_MAX_ITERATIONS 20

typedef struct gjk_support {
    int aid, bid;
    vec3 a;
    vec3 b;
} gjk_support;
typedef struct gjk_vertex {
    vec3 a;
    vec3 b;
    vec3 p;
    int aid, bid;
} gjk_vertex;
typedef struct gjk_simplex {
    int max_iter, iter;
    int hit, cnt;
    gjk_vertex v[4];
    float bc[4], D;
} gjk_simplex;
typedef struct gjk_result {
    int hit;
    vec3 p0;
    vec3 p1;
    float distance_squared;
    int iterations;
} gjk_result;

int gjk(gjk_simplex *s, const gjk_support *sup, vec3 *dv);
gjk_result gjk_analyze(const gjk_simplex *s);
gjk_result gjk_quad(float a_radius, float b_radius);

#endif

//typedef struct gjk_result gjk_result;

typedef struct line     { vec3 a, b;                                                  } line;
typedef struct sphere   { vec3 c; float r;                                            } sphere;
typedef struct aabb     { vec3 min, max;                                              } aabb;
typedef struct plane    { vec3 p, n;                                                  } plane;
typedef struct capsule  { vec3 a, b; float r;                                         } capsule;
typedef struct ray      { vec3 p, d;                                                  } ray;
typedef struct triangle { vec3 p0,p1,p2;                                              } triangle;
typedef struct poly     { vec3* verts; int cnt;                                       } poly;
typedef union  frustum  { struct { vec4 l, r, t, b, n, f; }; vec4 pl[6]; float v[24]; } frustum;

#define line(...)       C_CAST(line, __VA_ARGS__)
#define sphere(...)     C_CAST(sphere, __VA_ARGS__)
#define aabb(...)       C_CAST(aabb, __VA_ARGS__)
#define plane(...)      C_CAST(plane, __VA_ARGS__)
#define capsule(...)    C_CAST(capsule, __VA_ARGS__)
#define ray(p,normdir)  C_CAST(ray, p, normdir)
#define triangle(...)   C_CAST(triangle, __VA_ARGS__)
#define poly(...)       C_CAST(poly, __VA_ARGS__)
#define frustum(...)    C_CAST(frustum, __VA_ARGS__)

// ----------------------------------------------------------------------------

typedef struct hit {
    union {
        // general case
        float depth;
        // rays only: penetration (t0) and extraction (t1) points along ray line
        struct { float t0, t1; };
        // gjk only
        struct { int hits; vec3 p0, p1; float distance2; int iterations; };
    };
    union { vec3 p; vec3 contact_point; };
    union { vec3 n; vec3 normal; };
} hit;

#define hit(...)        C_CAST(hit, __VA_ARGS__)

// ----------------------------------------------------------------------------

/* line/segment */
API float   line_distance2_point(line l, vec3 p);
API vec3    line_closest_point(line l, vec3 p);
/* ray */
API float   ray_test_plane(ray r, vec4 p4);
API float   ray_test_triangle(ray r, triangle t);
API int     ray_test_sphere(float *t0, float *t1, ray r, sphere s);
API int     ray_test_aabb(float *t0, float *t1, ray r, aabb a);
API hit*    ray_hit_plane(ray r, plane p);
API hit*    ray_hit_triangle(ray r, triangle t);
API hit*    ray_hit_sphere(ray r, sphere s);
API hit*    ray_hit_aabb(ray r, aabb a);
/* sphere */
API vec3    sphere_closest_point(sphere s, vec3 p);
API hit*    sphere_hit_aabb(sphere s, aabb a);
API hit*    sphere_hit_capsule(sphere s, capsule c);
API hit*    sphere_hit_sphere(sphere a, sphere b);
API int     sphere_test_aabb(sphere s, aabb a);
API int     sphere_test_capsule(sphere s, capsule c);
API int     sphere_test_poly(sphere s, poly p);
API int     sphere_test_sphere(sphere a, sphere b);
/* aabb */
API vec3    aabb_closest_point(aabb a, vec3 p);
API float   aabb_distance2_point(aabb a, vec3 p);
API int     aabb_contains_point(aabb a, vec3 p);
API hit*    aabb_hit_aabb(aabb a, aabb b);
API hit*    aabb_hit_capsule(aabb a, capsule c);
API hit*    aabb_hit_sphere(aabb a, sphere s);
API int     aabb_test_aabb(aabb a, aabb b);
API int     aabb_test_capsule(aabb a, capsule c);
API int     aabb_test_poly(aabb a, poly p);
API int     aabb_test_sphere(aabb a, sphere s);
/* capsule */
API float   capsule_distance2_point(capsule c, vec3 p);
API vec3    capsule_closest_point(capsule c, vec3 p);
API hit*    capsule_hit_aabb(capsule c, aabb a);
API hit*    capsule_hit_capsule(capsule a, capsule b);
API hit*    capsule_hit_sphere(capsule c, sphere s);
API int     capsule_test_aabb(capsule c, aabb a);
API int     capsule_test_capsule(capsule a, capsule b);
API int     capsule_test_poly(capsule c, poly p);
API int     capsule_test_sphere(capsule c, sphere s);
/* poly: query */
API int     poly_test_sphere(poly p, sphere s);
API int     poly_test_aabb(poly p, aabb a);
API int     poly_test_capsule(poly p, capsule c);
API int     poly_test_poly(poly a, poly b);
/* poly: query transformed */
API int     poly_test_sphere_transform(poly p, vec3 pos3, mat33 rot33, sphere s);
API int     poly_test_aabb_transform(poly p, vec3 apos3, mat33 arot33, aabb a);
API int     poly_test_capsule_transform(poly p, vec3 pos3, mat33 rot33, capsule c);
API int     poly_test_poly_transform(poly a, vec3 apos3, mat33 arot33, poly b, vec3 bpos3, mat33 brot33);
/* poly: gjk result */
API int     poly_hit_sphere(struct gjk_result *res, poly p, sphere s);
API int     poly_hit_aabb(struct gjk_result *res, poly p, aabb a);
API int     poly_hit_capsule(struct gjk_result *res, poly p, capsule c);
API int     poly_hit_poly(struct gjk_result *res, poly a, poly b);
/* poly: gjk result transformed */
API int     poly_hit_sphere_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, sphere s);
API int     poly_hit_aabb_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, aabb a);
API int     poly_hit_capsule_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, capsule c);
API int     poly_hit_poly_transform(struct gjk_result *res, poly a, vec3 at3, mat33 ar33, poly b, vec3 bt3, mat33 br33);

API vec4    plane4(vec3 p, vec3 n);

API frustum frustum_build(mat44 projview);
API int     frustum_test_sphere(frustum f, sphere s);
API int     frustum_test_aabb(frustum f, aabb a);

API poly    poly_alloc(int cnt);
API void    poly_free(poly *p);

API poly    pyramid(vec3 from, vec3 to, float size); // poly_free() required
API poly    diamond(vec3 from, vec3 to, float size); // poly_free() required

API void    collide_demo(); // debug draw collisions
#line 0

#line 1 "fwk_cook.h"
// -----------------------------------------------------------------------------
// asset pipeline framework
// - rlyeh, public domain.
//
// all cooked assets are stored inside zip file at root folder, which acts as an asset database.
// during game boot, the database gets rebuilt as follows: (note: step 0 is an optional optimization)
// 0. for N given cores, split list of infiles into N zipfiles. then, parallelize cooks.
// 1. compare local disk files against file in zip database. for each mismatch do:
// 2. - invalidate its entry in database, if local file was removed from disk.
// 3. - write its *cooked* contents into database, if local file was created or modified from disk.
// 4. mount any existing zipfile(s) after cooking.
//
// notes: meta-datas from every raw asset are stored into comment field, inside .cook.zip archive.
// @todo: fix leaks
// @todo: symlink exact files
// @todo: idle threads should steal jobs from busy threads (maybe use jobs/coroutines for this?) ...

enum COOK_FLAGS {
    COOK_SYNC = 0,
    COOK_ASYNC = 1,
    COOK_CANCELABLE = 2,
    COOK_DEBUGLOG = 4, // log all cooking commands to a batch file
};

API void cook_config( const char *path_to_cook_ini ); // "tools/cook.ini"
API bool cook_start( const char *path_to_cook_ini, const char *masks, int flags ); // COOK_INI, "**"
API void cook_stop();
API void cook_cancel();
API int  cook_jobs();     // [0..N]
API int  cook_progress(); // [0..100]

// utils
API bool have_tools();
#line 0

#line 1 "fwk_data.h"
// -----------------------------------------------------------------------------
// data framework (json5, xml, compression) @todo:kvdb
// - rlyeh, public domain
//
// @todo: vec2,vec3,vec4

typedef union json_t { char* s; double f; int64_t i; uintptr_t p; array(union json_t) arr; } json_t;

// json api

API bool            json_push(const char *json_content);
API const char*         json_key(const char *keypath);
API json_t*             json_find(const char *type_keypath);
API json_t              json_get(const char *type_keypath);
API int                 json_count(const char *keypath);
#define                 json_int(...)    (json_get(va("i" __VA_ARGS__)).i)
#define                 json_float(...)  (json_get(va("f" __VA_ARGS__)).f)
#define                 json_string(...) (json_get(va("s" __VA_ARGS__)).s)
#define                 json_key(...)     json_key(va(__VA_ARGS__))
#define                 json_count(...)   json_count(va(__VA_ARGS__))
API bool            json_pop();

// xml api

API int             xml_push(const char *xml_content);
API const char *        xml_string(char *key);
API unsigned            xml_count(char *key);
API array(char)         xml_blob(char *key);
#define                 xml_string(...) xml_string(va(__VA_ARGS__))       // syntax sugar: string
#define                 xml_int(...)    atoi(xml_string(__VA_ARGS__))     // syntax sugar: int
#define                 xml_float(...)  atof(xml_string(__VA_ARGS__))     // syntax sugar: float
#define                 xml_blob(...)   xml_blob(va(__VA_ARGS__))         // syntax sugar: base64 blob
#define                 xml_count(...)  xml_count(va(__VA_ARGS__))        // syntax sugar: count nodes
API void            xml_pop();

API bool data_tests();
#line 0

#line 1 "fwk_extend.h"
// dll ------------------------------------------------------------------------

/// !!! `filename` must contain extension
/// load dynamic library `file` and search for `symbol`
/// return: NULL if not found, found symbol otherwise.
/// filename: path to dynamic library file. must contain extension.
/// symbol: symbol name. must not be NULL.
/// see: dlopen^, dlclose^
/// > bool (*plugin_init)(void) = dll("plugin.dll", "init");
/// > assert(plugin_init());
API void* dll(const char *filename, const char *symbol);

// -----------------------------------------------------------------------------
// script framework

API void script_init();
API void script_run(const char *script);
API void script_runfile(const char *pathfile);

API void script_bind_class(const char *objname, int num_methods, const char **c_names, void **c_functions);
API void script_bind_function(const char *c_name, void *c_function);
API void script_call(const char *lua_function);

API bool script_tests();

// -----------------------------------------------------------------------------
// script framework

enum {
    SCRIPT_LUA = 1,
    SCRIPT_DEBUGGER = 2,
};

API void *script_init_env(unsigned flags);
#line 0

#line 1 "fwk_file.h"
// -----------------------------------------------------------------------------
// files, cache and virtual filesystem (registered directories and/or compressed zip archives).
// - rlyeh, public domain.
//
// - note: vfs_mount() order matters (last mounts have higher priority).
// - note: directory/with/trailing/slash/ as mount_point, or zip/tar/pak archive otherwise.
//
// @todo: file_mmap
// @todo: file_find() from first file_scan()

// physical filesystem. files

API array(char*) file_list( const char *pathmasks ); // folder/*.ico;**.png;*.c
API bool         file_write( const char *file, const void *ptr, int len );
API bool         file_append( const char *file, const void *ptr, int len );
API char *       file_read(const char *filename);
API char *       file_load(const char *filename, int *len);
API uint64_t     file_size(const char *pathfile);
API bool         file_directory(const char *pathfile);

API char *       file_pathabs(const char *pathfile); // ../dir/./file.ext -> c:/prj/dir/file.ext
API char *       file_path(const char *pathfile); // c:/prj/dir/file.ext -> c:/prj/dir/
API char *       file_name(const char *pathfile); // c:/prj/dir/file.ext -> file.ext
API char *       file_base(const char *pathfile); // c:/prj/dir/file.ext -> file
API char *       file_ext(const char *pathfile); // c:/prj/dir/file.ext -> .ext
API char *       file_id(const char *pathfile); // c:/prj/dir/file.ext -> file/dir/prj (name then alphabetical)
API char *       file_normalize(const char *pathfile); // c:/prj/dir/file.ext -> c/prj/dir/file_ext
//API char *     file_normalize_with_folder(const char *pathfile); // c:/prj/dir/file.ext -> dir/file_ext
API char *       file_counter(const char *pathfile); // in: fwk.ini -> out: fwk(001).ini -> out: fwk(002).ini [-> etc...]

API uint64_t     file_stamp(const char *pathfile); // 1616153596 (seconds since unix epoch)
API uint64_t     file_stamp10(const char *pathfile); // 20210319113316 (absolute datetime in base10)

API bool         file_exist(const char *pathfile);
API bool         file_delete(const char *pathfile);
API bool         file_copy(const char *src, const char *dst);
API bool         file_move(const char *src, const char *dst);

API FILE*        file_temp();
API char*        file_tempname();

API void*        file_md5(const char *file); // 16 bytes
API void*        file_sha1(const char *file); // 20 bytes
API void*        file_crc32(const char *file); // 4 bytes

// compressed files

API array(char*) file_zip_list(const char *zipfile);
API array(char)  file_zip_extract(const char *zipfile, const char *filename);
API bool         file_zip_append(const char *zipfile, const char *filename, int clevel);
API bool         file_zip_appendmem(const char *zipfile, const char *entryname, const void *ptr, unsigned len, int clevel);

// storage (emscripten only)

// Mounts local storage folder for writing. Useful for Emscripten only. @path_folder: "/save" for example
// Reads local storage to memory. Usually call it one time only, after mount. Useful for Emscripten only.
// Writes memory contents to local storage. Usually call it after all fclose
API void         storage_mount(const char* folder);
API void         storage_read();
API void         storage_flush();

// virtual filesystem

API bool         vfs_mount(const char *mount_point);
API array(char*) vfs_list(const char *masks); // **.png;*.c

API char *       vfs_read(const char *pathfile);
API char *       vfs_load(const char *pathfile, int *size);
API int          vfs_size(const char *pathfile);

API void         vfs_reload();
API const char * vfs_resolve(const char *fuzzyname); // guess best match. @todo: fuzzy path
//API const char*vfs_extract(const char *pathfile); // extracts vfs file into local filesystem (temporary file), so it can be read by foreign/3rd party libs
API FILE*        vfs_handle(const char *pathfile); // same as above, but returns file handle instead. preferred way, will clean descriptors at exit

// cache

API void *       cache_insert(const char *key, void *value, int size);
API void *       cache_lookup(const char *key, int *size);

// ini

// @todo: evaluate alt api #1
// char *ini(filename, section.key, default);
// float inif(filename, section.key, default);
// @todo: evaluate alt api #2
// char *val = ini(filename, section_key);
// int count = ini_count(filename);
// char *key = ini_key_id(filename, id);
// char *val = ini_val_id(filename, id);

typedef map(char*,char*) ini_t;

API ini_t        ini(const char *filename);
API ini_t        ini_from_mem(const char *data);
API void         ini_destroy(ini_t);

API bool         ini_write(const char *filename, const char *section, const char *key, const char *value);
#line 0

#line 1 "fwk_font.h"
// -----------------------------------------------------------------------------
// font framework
// - rlyeh, public domain

// font size tags
#define FONT_H1 "\1" // largest
#define FONT_H2 "\2"
#define FONT_H3 "\3"
#define FONT_H4 "\4"
#define FONT_H5 "\5"
#define FONT_H6 "\6" // smallest

// font color tags
#define FONT_COLOR1   "\x1a"
#define FONT_COLOR2   "\x1b"
#define FONT_COLOR3   "\x1c"
#define FONT_COLOR4   "\x1d"
#define FONT_COLOR5   "\x1e"
#define FONT_COLOR6   "\x1f"

// font face tags
#define FONT_FACE1    "\x10"
#define FONT_FACE2    "\x11"
#define FONT_FACE3    "\x12"
#define FONT_FACE4    "\x13"
#define FONT_FACE5    "\x14"
#define FONT_FACE6    "\x15"
#define FONT_FACE7    "\x16"
#define FONT_FACE8    "\x17" // editor may override this one
#define FONT_FACE9    "\x18" // editor may override this one
#define FONT_FACE10   "\x19" // editor may override this one

// font align tags
#define FONT_LEFT     "\\<"
#define FONT_CENTER   "\\|"
#define FONT_RIGHT    "\\>"
#define FONT_TOP      "\\^"
#define FONT_MIDDLE   "\\-"
#define FONT_BASELINE "\\_"
#define FONT_BOTTOM   "\\v"

// font flags
enum FONT_FLAGS {
    // font atlas size
    FONT_512 = 0x0,
    FONT_1024 = 0x1,
    FONT_2048 = 0x2,
    FONT_4096 = 0x4,

    // font oversampling
    FONT_NO_OVERSAMPLE = 0x0,
    FONT_OVERSAMPLE_X = 0x08,
    FONT_OVERSAMPLE_Y = 0x10,

    // unicode ranges
    FONT_ASCII = 0x800, // Compatible charset
    FONT_AR = 0x001000, // Arabic and Arabic-Indic digits
    FONT_ZH = 0x002000, // Chinese Simplified (@todo: add ZH_FULL)
    FONT_EL = 0x004000, // Greek, Coptic, modern Georgian, Svan, Mingrelian, Ancient Greek
    FONT_EM = 0x008000, // Emoji
    FONT_EU = 0x010000, // Eastern/western Europe, IPA, Latin ext A/B
    FONT_HE = 0x020000, // Hebrew, Yiddish, Ladino, and other diaspora languages
    FONT_JP = 0x040000, // Hiragana, Katakana, Punctuations, Half-width chars
    FONT_KR = 0x080000, // Korean, Hangul
    FONT_RU = 0x100000, // Cyrillic + ext A/B
    FONT_TH = 0x200000, // Thai
    FONT_VI = 0x400000, // Vietnamese
    FONT_CJK = FONT_ZH|FONT_JP|FONT_KR,

    // FONT_DEFAULTS = FONT_512 | FONT_NO_OVERSAMPLE | FONT_ASCII,
};

typedef struct font_metrics_t {
    float ascent;   // max distance above baseline for all glyphs
    float descent;  // max distance below baseline for all glyphs
    float linegap;  // distance betwen ascent of next line and descent of current line
    float linedist; // distance between the baseline of two lines (ascent - descent + linegap)
} font_metrics_t;

// configures
API void  font_face(const char *face_tag, const char *filename_ttf, float font_size, unsigned flags);
API void  font_face_from_mem(const char *tag, const void *ttf_buffer, unsigned ttf_len, float font_size, unsigned flags);
API void  font_scales(const char *face_tag, float h1, float h2, float h3, float h4, float h5, float h6);
API void  font_color(const char *color_tag, uint32_t color);

// commands
API vec2           font_xy();
API void           font_goto(float x, float y);
API vec2           font_print(const char *text);
API vec2           font_rect(const char *text);
API font_metrics_t font_metrics(const char *text);
//  void  font_clip(vec2 topleft, vec2 bottomright);
//  void  font_wrap(vec2 topleft, vec2 bottomright);

// syntax highlighting
API void* font_colorize(const char *text, const char *comma_types, const char *comma_keywords); // comma separated tokens. expensive, please cache result.
API vec2  font_highlight(const char *text, const void *colors);

// ui
API void  ui_font();
#line 0

#line 1 "fwk_input.h"
// -----------------------------------------------------------------------------
// input framework
// - rlyeh, public domain
//
// @todo: window
// @todo: for extra savings (168->72 bytes), promote bits to real bits (/8 %8) & normalized floats [-1,+1] to shorts or chars
// @todo: GAMEPAD_A|2, MOUSE_L|1, KEY_C|3
// @todo: load/save
// @todo: send virtual presses & outputs (rumble, light, led, text, etc)
// @todo: fix if logger !60 hz
// @tofo: fix click2/repeat edge cases

API int         input_use( int controller_id ); // [0..3]

// -- basic polling api (read input at current frame)

API float       input( int vk );
API vec2        input2( int vk );
API float       input_diff( int vk ); // @todo: rename diff->delta
API vec2        input_diff2( int vk ); // @todo: rename diff2->delta2
API const char* input_string( int vk );

// -- extended polling api (read input at Nth frame ago)

API float       input_frame( int vk, int Nth_frame );
API vec2        input_frame2( int vk, int Nth_frame );

// -- events api

API int         input_up( int vk ); // ON -> OFF (release)
API int         input_down( int vk ); // OFF -> ON (trigger)
API int         input_held( int vk ); // ON -> ON (pressed)
API int         input_idle( int vk ); // OFF -> OFF

API int         input_click( int vk, int ms ); // OFF -> ON -> OFF
API int         input_click2( int vk, int ms ); // OFF -> ON -> OFF -> ON -> OFF
API int         input_repeat( int vk, int ms ); // [...] ON -> ON -> ON

API int         input_chord2( int vk1, int vk2 ); // all vk1 && vk2 are ON
API int         input_chord3( int vk1, int vk2, int vk3 ); // all vk1 && vk2 && vk3 are ON
API int         input_chord4( int vk1, int vk2, int vk3, int vk4 ); // all vk1 && vk2 && vk3 && vk4 are ON

// -- 1d/2d filters

API float       input_filter_positive( float v ); // [-1..1] -> [0..1]
API vec2        input_filter_positive2( vec2 v ); // [-1..1] -> [0..1]
API vec2        input_filter_deadzone( vec2 v, float deadzone_treshold );
API vec2        input_filter_deadzone_4way( vec2 v, float deadzone_treshold );

// -- multi-touch

enum TOUCH_BUTTONS {
    TOUCH_0,    // defaults to left screen area. input_touch_area() to override
    TOUCH_1,    // defaults to right screen area. input_touch_area() to override
};

API void        input_touch_area(unsigned button, vec2 begin_coord_ndc, vec2 end_coord_ndc);
API vec2        input_touch(unsigned button, float sensitivity);                   // absolute position in 2d coords
API vec2        input_touch_delta(unsigned button, float sensitivity);             // delta from previous position
API vec2        input_touch_delta_from_origin(unsigned button, float sensitivity); // relative position from initial touch
API bool        input_touch_active();

// -- utils

API void        input_mappings(const char *filename); // update gamepad mappings (usually "gamecontrollerdb.txt" file)
API char        input_keychar(unsigned code); // Converts keyboard code to its latin char (if any)
API int         input_enum(const char *sym);
API int         input_anykey();
API int         input_eval(const char *expression); // "down(X)*input(CTRL)"

// inject state
API void        input_send( int vk ); // @todo
// load/save input
API array(char) save_input(); // @todo
API bool        load_input(array(char) replay); // @todo

// visualize input
API int         ui_keyboard();
API int         ui_mouse();
API int         ui_gamepad(int id);
API int         ui_gamepads();

// --

enum INPUT_ENUMS {
    // -- bits: x104 keyboard, x3 mouse, x15 gamepad, x7 window
    // keyboard gaming keys (53-bit): first-class keys for gaming
    KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,   KEY_TICK,KEY_BS,           KEY_ESC,
    KEY_TAB,   KEY_Q,KEY_W,KEY_E,KEY_R,KEY_T,KEY_Y,KEY_U,KEY_I,KEY_O,KEY_P,
    KEY_CAPS,     KEY_A,KEY_S,KEY_D,KEY_F,KEY_G,KEY_H,KEY_J,KEY_K,KEY_L, KEY_ENTER,
    KEY_LSHIFT,       KEY_Z,KEY_X,KEY_C,KEY_V,KEY_B,KEY_N,KEY_M,        KEY_RSHIFT,            KEY_UP,
    KEY_LCTRL,KEY_LALT,               KEY_SPACE,                KEY_RALT,KEY_RCTRL,  KEY_LEFT,KEY_DOWN,KEY_RIGHT,

    // for completeness, secondary keys below (52-bit). beware!
    KEY_INS,KEY_HOME,KEY_PGUP,KEY_DEL,KEY_END,KEY_PGDN, // beware: different behavior win/osx (also, osx: no home/end).
    KEY_LMETA,KEY_RMETA,KEY_MENU,KEY_PRINT,KEY_PAUSE,KEY_SCROLL,KEY_NUMLOCK, // beware: may trigger unexpected OS behavior. (@todo: add RSHIFT here for win?)
    KEY_MINUS,KEY_EQUAL,KEY_LSQUARE,KEY_RSQUARE,KEY_SEMICOLON,KEY_QUOTE,KEY_HASH,KEY_BAR,KEY_COMMA,KEY_DOT,KEY_SLASH, // beware: non-us keyboard layouts
    KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12, // beware: complicated on laptops/osx
    KEY_PAD1,KEY_PAD2,KEY_PAD3,KEY_PAD4,KEY_PAD5,KEY_PAD6,KEY_PAD7,KEY_PAD8,KEY_PAD9,KEY_PAD0, // beware: complicated on laptops
    KEY_PADADD,KEY_PADSUB,KEY_PADMUL,KEY_PADDIV,KEY_PADDOT,KEY_PADENTER, // beware: complicated on laptops

    MOUSE_L, MOUSE_M, MOUSE_R, // @todo: MOUSE_CLICKS,
    GAMEPAD_CONNECTED, GAMEPAD_A, GAMEPAD_B, GAMEPAD_X, GAMEPAD_Y,
    GAMEPAD_UP, GAMEPAD_DOWN, GAMEPAD_LEFT, GAMEPAD_RIGHT, GAMEPAD_MENU, GAMEPAD_START,
    GAMEPAD_LB, GAMEPAD_RB, GAMEPAD_LTHUMB, GAMEPAD_RTHUMB,
    WINDOW_BLUR, WINDOW_FOCUS, WINDOW_CLOSE, WINDOW_MINIMIZE, WINDOW_MAXIMIZE, WINDOW_FULLSCREEN, WINDOW_WINDOWED, // MINI/MAXI/RESTORED, SHOWN/HIDDEN

    // -- floats: x7 gamepad, x3 mouse, x4 touch, x4 window
    GAMEPAD_LPAD, GAMEPAD_LPADX = GAMEPAD_LPAD, GAMEPAD_LPADY,
    GAMEPAD_RPAD, GAMEPAD_RPADX = GAMEPAD_RPAD, GAMEPAD_RPADY,
    GAMEPAD_LTRIGGER, GAMEPAD_LT = GAMEPAD_LTRIGGER, GAMEPAD_RTRIGGER, GAMEPAD_RT = GAMEPAD_RTRIGGER, GAMEPAD_BATTERY,
    MOUSE, MOUSE_X = MOUSE, MOUSE_Y, MOUSE_W,
    TOUCH_X1, TOUCH_Y1, TOUCH_X2, TOUCH_Y2,
    WINDOW_RESIZE, WINDOW_RESIZEX = WINDOW_RESIZE, WINDOW_RESIZEY, WINDOW_ORIENTATION, WINDOW_BATTERY,

    // -- strings: x2 gamepad
    GAMEPAD_GUID, GAMEPAD_NAME,
};
// these aliases do check both left and right counterparts
enum INPUT_ALIASES {
    KEY_SHIFT = KEY_LSHIFT,
    KEY_ALT = KEY_LALT,
    KEY_CTRL = KEY_LCTRL,
};
#line 0

#line 1 "fwk_memory.h"
// -----------------------------------------------------------------------------
// memory framework
// - rlyeh, public domain

// memory leaks detector
#if ENABLE_MEMORY_LEAKS
#define WATCH(ptr,sz) watch((ptr), (sz))
#define FORGET(ptr)   forget(ptr)
#else
#define WATCH(ptr,sz) (ptr)
#define FORGET(ptr)   (ptr)
#endif

// default allocator (aborts on out-of-mem)
API void*  xrealloc(void* p, size_t sz);
API size_t xsize(void* p);
API char*  xstats(void);

// stack based allocator (negative bytes does rewind stack, like when entering new frame)
API void*  stack(int bytes);

// memory leaks api (this is already integrated as long as you compile with -DENABLE_MEMORY_LEAKS)
API void*  watch( void *ptr, int sz );
API void*  forget( void *ptr );

// memory api
#define ALLOCSIZE(p)   xsize(p)
#define MALLOC(n)      REALLOC_(0,(n))
#define FREE(p)        REALLOC_((p), 0)
#define REALLOC(p,n)   REALLOC_((p),(n))
#define CALLOC(m,n)    CALLOC_((m),(n))
#define STRDUP(s)      STRDUP_(s)
#define ALLOCA(n)      ifdef(gcc, __builtin_alloca(n), _alloca(n))

static FORCE_INLINE void *(REALLOC_)(void *p, size_t n) { return n ? WATCH(xrealloc(p,n),n) : xrealloc(FORGET(p),0); } ///-
static FORCE_INLINE void *(CALLOC_)(size_t m, size_t n) { return n *= m, memset(REALLOC(0,n),0,n); } ///-
static FORCE_INLINE char *(STRDUP_)(const char *s) { size_t n = strlen(s)+1; return ((char*)memcpy(REALLOC(0,n), s, n)); } ///-
#line 0

#line 1 "fwk_network.h"
// -----------------------------------------------------------------------------
// network framework
// - rlyeh, public domain

API array(char) download( const char *url );
API int         download_file( FILE *out, const char *url );
API int         portname( const char *service_name, unsigned retries );

API bool        network_tests();

// -----------------------------------------------------------------------------
// udp wrapper
// - rlyeh, public domain.

// server
API int   udp_bind(const char *address, const char *port);

// client
API int   udp_open(const char *address, const char *port);

// common
API int   udp_send(int, const void *buf, int len ); // <0 error, >0 bytes sent ok
API int   udp_sendto(int, const char *ip, const char *port, const void *buf, int len ); // <0 error, >0 bytes sent ok
API int   udp_recv(int, void *buf, int len ); // <0 error, 0 orderly shutdown, >0 received bytes
API int   udp_peek(int); // <0 error, 0 timeout, >0 data

// -----------------------------------------------------------------------------
// tcp wrapper
// - rlyeh, public domain

// client
API int   tcp_open(const char *address, const char *port);

// server
API int   tcp_bind(const char *interface_, const char *port, int queue);
API int   tcp_peek(int, int(*callback)(int));

// common
API int   tcp_send(int, const void* buf, int len);
API int   tcp_recv(int, void* buf, int len);
API char* tcp_host(int); // info
API char* tcp_port(int); // info

API int   tcp_close(int);

// extras
API int   tcp_debug(int); // toggle traffic monitoring on/off for given socket
//API int   tcp_printf(int, const char *fmt, ...); // printf message in remote end
//API int   tcp_crypt(int,uint64_t);               // set shared secret
#line 0

#line 1 "fwk_netsync.h"
// high-level, socket-less networking api. inspired by Quake, MPI and RenderBuckets theories.
// - rlyeh, public domain
//
// Usage:
// 1. configure networked memory buffers with flags (world, player1, player2, etc). network_buffer();
// 2. then during game loop:
//    - modify your buffers as much as needed.
//    - sync buffers at least once per frame. network_sync();
//    - render your world
// 3. optionally, monitor network status & variables. network_get();
//
// @todo: maybe network_send(msg) + msg *network_recv(); instead of event queue of network_sync() ?

//enum { NETWORK_HANDSHAKE, NETWORK_ENCRYPT, NETWORK_VERSIONED, NETWORK_CHECKSUM }; // negotiation
//enum { NETWORK_TCP, NETWORK_UDP, NETWORK_KCP, NETWORK_ENET, NETWORK_WEBSOCKET }; // transport, where

enum { NETWORK_BIND = 2, NETWORK_CONNECT = 4, NETWORK_NOFAIL = 8 };
API void   network_create(unsigned max_clients, const char *ip, const char *port, unsigned flags); // both ip and port can be null

//enum { NETWORK_LOSSY, NETWORK_COMPRESS }; // post-processes
//enum { NETWORK_PREDICT, NETWORK_RECONCILE, NETWORK_INTERPOLATE, NETWORK_COMPENSATE }; // time authority, when
//enum { NETWORK_LAGS, NETWORK_DROPS, NETWORK_THROTTLES, NETWORK_DUPES }; // quality sim, how much
//enum { NETWORK_CONST = 1, NETWORK_64,NETWORK_32,NETWORK_16,NETWORK_8, NETWORK_FLT, NETWORK_STR, NETWORK_BLOB }; // type, what
enum { NETWORK_SEND = 2, NETWORK_RECV = 4 };
enum { NETWORK_UNRELIABLE = 8, NETWORK_UNORDERED = 16/*, NETWORK_PRIORITY = 32*/ };
API void*   network_buffer(void *ptr, unsigned sz, uint64_t flags, int64_t rank); // configures a shared/networked buffer
API char**  network_sync(unsigned timeout_ms); // syncs all buffers & returns null-terminated list of network events

enum {
    NETWORK_EVENT_CONNECT,
    NETWORK_EVENT_DISCONNECT,
    NETWORK_EVENT_RECEIVE,
    NETWORK_EVENT_DISCONNECT_TIMEOUT,

    /* offset from internal networking events */
    NETWORK_EVENT_RPC = 10,
    NETWORK_EVENT_RPC_RESP,
};

/* errcode and errstr are optional arguments, pass NULL to ignore them,
    errstr is filled by va() */
API int network_event(const char *msg, int *errcode, char **errstr);

enum { NETWORK_RANK = 0 }; // [0..N] where 0 is server
enum { NETWORK_PING = 1 }; // NETWORK_BANDWIDTH, NETWORK_QUALITY };
enum { NETWORK_PORT = 2, NETWORK_IP, NETWORK_LIVE };
enum { NETWORK_SEND_MS = 4 };
enum { NETWORK_BUF_CLEAR_ON_JOIN = 5 };
enum { NETWORK_USERID = 7, /*NETWORK_SALT,*/ NETWORK_COUNT/*N users*/ /*...*/, NETWORK_CAPACITY };
API int64_t network_get(uint64_t key);
API int64_t network_put(uint64_t key, int64_t value);

API void network_rpc(const char *signature, void *function);
API void network_rpc_send_to(int64_t rank, unsigned id, const char *cmdline);
API void network_rpc_send(unsigned id, const char *cmdline);

// -----------------------------------------------------------------------------
// low-level api (sockets based)

API bool   server_bind(int max_clients, int port);
API char** server_poll(unsigned timeout_ms);
API char** client_poll(unsigned timeout_ms);
API void   server_broadcast_bin_flags(const void *ptr, int len, uint64_t flags);
API void   server_broadcast_bin(const void *ptr, int len);
API void   server_broadcast_flags(const char *msg, uint64_t flags);
API void   server_broadcast(const char *msg);
API void   server_terminate();
API void   server_send(int64_t handle, const char *msg);
API void   server_send_bin(int64_t handle, const void *ptr, int len);
API void   server_drop(int64_t handle);

API int64_t  client_join(const char *ip, int port);
#define      client_send_flags(msg,flags) server_broadcast(msg, flags)
#define      client_send(msg) server_broadcast(msg)
#define      client_send_bin_flags(ptr,len,flags) server_broadcast_bin(ptr, len, flags)
#define      client_send_bin(ptr,len) server_broadcast_bin(ptr, len)
#define      client_terminate() server_terminate()

#define ANYHOST_IPV4 "0.0.0.0"
#define ANYHOST_IPV6 "::0"

#define LOCALHOST_IPV4 "127.0.0.1"
#define LOCALHOST_IPV6 "::1"
#line 0

#line 1 "fwk_pack.h"
// ----------------------------------------------------------------------------
// compression api

enum COMPRESS_FLAGS {
    COMPRESS_RAW     = 0,
    COMPRESS_PPP     = (1<<4),
    COMPRESS_ULZ     = (2<<4),
    COMPRESS_LZ4     = (3<<4),
    COMPRESS_CRUSH   = (4<<4),
    COMPRESS_DEFLATE = (5<<4),
    COMPRESS_LZP1    = (6<<4),
    COMPRESS_LZMA    = (7<<4),
    COMPRESS_BALZ    = (8<<4),
    COMPRESS_LZW3    = (9<<4),
    COMPRESS_LZSS    = (10<<4),
    COMPRESS_BCM     = (11<<4),
    COMPRESS_ZLIB    = (12<<4), // same as deflate with header
};

API unsigned zbounds(unsigned inlen, unsigned flags);
API unsigned zencode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags);
API unsigned zexcess(unsigned flags);
API unsigned zdecode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags);

// ----------------------------------------------------------------------------
// array de/interleaving
//
// results:
// R0G0B0   R1G1B1   R2G2B2...   -> R0R1R2... B0B1B2... G0G1G2...
// R0G0B0A0 R1G1B1A1 R2G2B2A2... -> R0R1R2... A0A1A2... B0B1B2... G0G1G2...

API void *interleave( void *out, const void *list, int list_count, int sizeof_item, unsigned columns );

// ----------------------------------------------------------------------------
// cobs en/decoder

API unsigned cobs_bounds(unsigned len);
API unsigned cobs_encode(const void *in, unsigned inlen, void *out, unsigned outlen);
API unsigned cobs_decode(const void *in, unsigned inlen, void *out, unsigned outlen);

// ----------------------------------------------------------------------------
// base92 en/decoder

API unsigned base92_encode(const void *in, unsigned inlen, void* out, unsigned outlen);
API unsigned base92_decode(const void *in, unsigned inlen, void* out, unsigned outlen);
API unsigned base92_bounds(unsigned inlen);

// ----------------------------------------------------------------------------
// netstring en/decoder

API unsigned netstring_bounds(unsigned inlen);
API unsigned netstring_encode(const char *in, unsigned inlen, char *out, unsigned outlen);
API unsigned netstring_decode(const char *in, unsigned inlen, char *out, unsigned outlen);

// ----------------------------------------------------------------------------
// delta en/decoder

API void delta8_encode(void *buffer, unsigned count);
API void delta8_decode(void *buffer, unsigned count);

API void delta16_encode(void *buffer, unsigned count);
API void delta16_decode(void *buffer, unsigned count);

API void delta32_encode(void *buffer, unsigned count);
API void delta32_decode(void *buffer, unsigned count);

API void delta64_encode(void *buffer, unsigned count);
API void delta64_decode(void *buffer, unsigned count);

// ----------------------------------------------------------------------------
// zigzag en/decoder

API uint64_t zig64( int64_t value ); // convert sign|magnitude to magnitude|sign
API int64_t zag64( uint64_t value ); // convert magnitude|sign to sign|magnitude

API uint32_t enczig32u( int32_t n);
API uint64_t enczig64u( int64_t n);
API  int32_t deczig32i(uint32_t n);
API  int64_t deczig64i(uint64_t n);

// ----------------------------------------------------------------------------
// arc4 en/decryptor

API void *arc4( void *buffer, unsigned buflen, const void *pass, unsigned passlen );

// ----------------------------------------------------------------------------
// crc64

API uint64_t crc64(uint64_t h, const void *ptr, uint64_t len);

// ----------------------------------------------------------------------------
// entropy encoder

API void entropy( void *buf, unsigned n );

// -----------------------------------------------------------------------------
// semantic versioning in a single byte (octal)

API int semver( int major, int minor, int patch );
API int semvercmp( int v1, int v2 );

#define SEMVER(major,minor,patch) (0100 * (major) + 010 * (minor) + (patch))
#define SEMVERCMP(v1,v2) (((v1) & 0110) - ((v2) & 0110))
#define SEMVERFMT "%03o"

// -----------------------------------------------------------------------------
// storage types. refer to vec2i/3i, vec2/3/4 if you plan to do math operations

typedef struct byte2 { uint8_t x,y; } byte2;
typedef struct byte3 { uint8_t x,y,z; } byte3;
typedef struct byte4 { uint8_t x,y,z,w; } byte4;

typedef struct int2 { int x,y; } int2;
typedef struct int3 { int x,y,z; } int3;
typedef struct int4 { int x,y,z,w; } int4;

typedef struct uint2 { unsigned int x,y; } uint2;
typedef struct uint3 { unsigned int x,y,z; } uint3;
typedef struct uint4 { unsigned int x,y,z,w; } uint4;

typedef struct float2 { float x,y; } float2;
typedef struct float3 { float x,y,z; } float3;
typedef struct float4 { float x,y,z,w; } float4;

typedef struct double2 { double x,y; } double2;
typedef struct double3 { double x,y,z; } double3;
typedef struct double4 { double x,y,z,w; } double4;

#define byte2(x,y)       C_CAST(byte2, (uint8_t)(x), (uint8_t)(y) )
#define byte3(x,y,z)     C_CAST(byte3, (uint8_t)(x), (uint8_t)(y), (uint8_t)(z) )
#define byte4(x,y,z,w)   C_CAST(byte4, (uint8_t)(x), (uint8_t)(y), (uint8_t)(z), (uint8_t)(w) )

#define int2(x,y)        C_CAST(int2, (int)(x), (int)(y) )
#define int3(x,y,z)      C_CAST(int3, (int)(x), (int)(y), (int)(z) )
#define int4(x,y,z,w)    C_CAST(int4, (int)(x), (int)(y), (int)(z), (int)(w) )

#define uint2(x,y)       C_CAST(uint2, (unsigned)(x), (unsigned)(y) )
#define uint3(x,y,z)     C_CAST(uint3, (unsigned)(x), (unsigned)(y), (unsigned)(z) )
#define uint4(x,y,z,w)   C_CAST(uint4, (unsigned)(x), (unsigned)(y), (unsigned)(z), (unsigned)(w) )

#define float2(x,y)      C_CAST(float2, (float)(x), (float)(y) )
#define float3(x,y,z)    C_CAST(float3, (float)(x), (float)(y), (float)(z) )
#define float4(x,y,z,w)  C_CAST(float4, (float)(x), (float)(y), (float)(z), (float)(w) )

#define double2(x,y)     C_CAST(double2, (double)(x), (double)(y) )
#define double3(x,y,z)   C_CAST(double3, (double)(x), (double)(y), (double)(z) )
#define double4(x,y,z,w) C_CAST(double4, (double)(x), (double)(y), (double)(z), (double)(w) )

// -----------------------------------------------------------------------------
// compile-time fourcc, eightcc

API char *cc4str(unsigned cc);
API char *cc8str(uint64_t cc);

enum {
#   define _(a,b,c,d,e) cc__##a, cc__##b, cc__##c, cc__##d, cc__##e
    cc__1 = '1', _(2,3,4,5,6),_(7,8,9,0,_), cc__ = ' ',
    cc__A = 'A', _(B,C,D,E,F),_(G,H,I,J,K),_(L,M,N,O,P),_(Q,R,S,T,U),_(V,W,X,Y,Z),
    cc__a = 'a', _(b,c,d,e,f),_(g,h,i,j,k),_(l,m,n,o,p),_(q,r,s,t,u),_(v,w,x,y,z),
#   undef _
};

#ifdef BIG
#define cc4(a,b,c,d) ((uint32_t)(cc__##a<<24) | (cc__##b<<16) | (cc__##c<<8) | (cc__##d<<0))
#define cc8(a,b,c,d,e,f,g,h) (((uint64_t)cc4(a,b,c,d) << 32ULL) | cc4(e,f,g,h))
#else
#define cc4(a,b,c,d) ((uint32_t)(cc__##d<<24) | (cc__##c<<16) | (cc__##b<<8) | (cc__##a<<0))
#define cc8(a,b,c,d,e,f,g,h) (((uint64_t)cc4(e,f,g,h) << 32ULL) | cc4(a,b,c,d))
#endif

#define cc3(a,b,c) cc4(,a,b,c)
#define cc5(a,b,c,d,e) cc6(,a,b,c,d,e)
#define cc6(a,b,c,d,e,f) cc7(,a,b,c,d,e,f)
#define cc7(a,b,c,d,e,f,g) cc8(,a,b,c,d,e,f,g)

// ----------------------------------------------------------------------------
// text conversions

API char* ftoa1(float v);
API char* ftoa2(vec2  v);
API char* ftoa3(vec3  v);
API char* ftoa4(vec4  v);

API float atof1(const char *s);
API vec2  atof2(const char *s);
API vec3  atof3(const char *s);
API vec4  atof4(const char *s);

API char* itoa1(int   v);
API char* itoa2(vec2i v);
API char* itoa3(vec3i v);

API int   atoi1(const char *s);
API vec2i atoi2(const char *s);
API vec3i atoi3(const char *s);

// ----------------------------------------------------------------------------
// endianness

API int         is_big();
API int         is_little();

API uint16_t    swap16( uint16_t x );
API uint32_t    swap32( uint32_t x );
API uint64_t    swap64( uint64_t x );
API float       swap32f(float n);
API double      swap64f(double n);
API void        swapf(float *a, float *b);
API void        swapf2(vec2 *a, vec2 *b);
API void        swapf3(vec3 *a, vec3 *b);
API void        swapf4(vec4 *a, vec4 *b);

API uint16_t    lil16(uint16_t n); // swap16 as lil
API uint32_t    lil32(uint32_t n); // swap32 as lil
API uint64_t    lil64(uint64_t n); // swap64 as lil
API float       lil32f(float n);   // swap32 as lil
API double      lil64f(double n);  // swap64 as lil

API uint16_t    big16(uint16_t n); // swap16 as big
API uint32_t    big32(uint32_t n); // swap32 as big
API uint64_t    big64(uint64_t n); // swap64 as big
API float       big32f(float n);   // swap32 as big
API double      big64f(double n);  // swap64 as big

API uint16_t*   lil16p(void *p, int sz);
API uint32_t*   lil32p(void *p, int sz);
API uint64_t*   lil64p(void *p, int sz);
API float   *   lil32pf(void *p, int sz);
API double  *   lil64pf(void *p, int sz);

API uint16_t*   big16p(void *p, int sz);
API uint32_t*   big32p(void *p, int sz);
API uint64_t*   big64p(void *p, int sz);
API float   *   big32pf(void *p, int sz);
API double  *   big64pf(void *p, int sz);

#if is(cl)
#define swap16 _byteswap_ushort
#define swap32 _byteswap_ulong
#define swap64 _byteswap_uint64
#elif is(gcc)
#define swap16 __builtin_bswap16
#define swap32 __builtin_bswap32
#define swap64 __builtin_bswap64
#endif

#define hton16 big16
#define ntoh16 big16
#define hton32 big32
#define ntoh32 big32
#define hton64 big64
#define ntoh64 big64

#define IS_BIG    ((*(uint16_t *)"\0\1") == 1)
#define IS_LITTLE ((*(uint16_t *)"\0\1") != 1)

// ----------------------------------------------------------------------------
// half packing

typedef uint16_t half;
API float half_to_float(half value);
API half  float_to_half(float value);

// ----------------------------------------------------------------------------
// int packing

// pack16i() -- store a 16-bit int into a char buffer (like htons())
// pack32i() -- store a 32-bit int into a char buffer (like htonl())
// pack64i() -- store a 64-bit int into a char buffer (like htonl())

API void pack16i(uint8_t *buf, uint16_t i, int swap);
API void pack32i(uint8_t *buf, uint32_t i, int swap);
API void pack64i(uint8_t *buf, uint64_t i, int swap);

// unpack16i() -- unpack a 16-bit int from a char buffer (like ntohs())
// unpack32i() -- unpack a 32-bit int from a char buffer (like ntohl())
// unpack64i() -- unpack a 64-bit int from a char buffer (like ntohl())
// changes unsigned numbers to signed if needed.

API int16_t unpack16i(const uint8_t *buf, int swap);
API int32_t unpack32i(const uint8_t *buf, int swap);
API int64_t unpack64i(const uint8_t *buf, int swap);

// ----------------------------------------------------------------------------
// float un/packing: 8 (micro), 16 (half), 32 (float), 64 (double) types

#define pack754_8(f)    (  pack754((f),  8,  4))
#define pack754_16(f)   (  pack754((f), 16,  5))
#define pack754_32(f)   (  pack754((f), 32,  8))
#define pack754_64(f)   (  pack754((f), 64, 11))
#define unpack754_8(u)  (unpack754((u),  8,  4))
#define unpack754_16(u) (unpack754((u), 16,  5))
#define unpack754_32(u) (unpack754((u), 32,  8))
#define unpack754_64(u) (unpack754((u), 64, 11))

API    uint64_t pack754(long double f, unsigned bits, unsigned expbits);
API long double unpack754(uint64_t i, unsigned bits, unsigned expbits);

// ----------------------------------------------------------------------------
// variable-length integer packing

API uint64_t pack64uv( uint8_t *buffer, uint64_t value );
API uint64_t unpack64uv( const uint8_t *buffer, uint64_t *value );
API uint64_t pack64iv( uint8_t *buffer, int64_t value_ );
API uint64_t unpack64iv( const uint8_t *buffer, int64_t *value );

// ----------------------------------------------------------------------------
// msgpack v5, schema based struct/buffer bitpacking

// api v2

API int  msgpack(const char *fmt, ... );                // va arg pack "n,b,u,d/i,s,p,f/g,e,[,{". returns number of written bytes
API int  msgunpack(const char *fmt, ... );              // va arg pack "n,b,u,d/i,s,p,f/g,e,[,{". returns number of parsed args

// api v1

API int msgpack_new(uint8_t *w, size_t l);
API int msgpack_nil();                                  // write null
API int msgpack_chr(bool n);                            // write boolean
API int msgpack_uns(uint64_t n);                        // write unsigned integer
API int msgpack_int(int64_t n);                         // write integer
API int msgpack_str(const char *s);                     // write string
API int msgpack_bin(const char *s, size_t n);           // write binary pointer
API int msgpack_flt(double g);                          // write real
API int msgpack_ext(uint8_t key, void *val, size_t n);  // write extension type
API int msgpack_arr(uint32_t n);                        // write array mark for next N items
API int msgpack_map(uint32_t n);                        // write map mark for next N pairs (N keys + N values)
API int msgpack_eof();                                  // write full?
API int msgpack_err();                                  // write error?

API bool msgunpack_new( const void *opaque_or_FILE, size_t bytes );
API bool msgunpack_nil();
API bool msgunpack_chr(bool *chr);
API bool msgunpack_uns(uint64_t *uns);
API bool msgunpack_int(int64_t *sig);
API bool msgunpack_str(char **str);
API bool msgunpack_bin(void **bin, uint64_t *len);
API bool msgunpack_flt(float *flt);
API bool msgunpack_dbl(double *dbl);
API bool msgunpack_ext(uint8_t *key, void **val, uint64_t *len);
API bool msgunpack_arr(uint64_t *len);
API bool msgunpack_map(uint64_t *len);
API bool msgunpack_eof();
API bool msgunpack_err();

// ----------------------------------------------------------------------------
// Based on code by Brian "Beej Jorgensen" Hall (public domain) [1].
// Based on code by Ginger Bill's half<->float (public domain) [2].
// - rlyeh, public domain.
//
// pack.c  -- perl/python-ish pack/unpack functions
// like printf and scanf, but for binary data.
//
// format flags:
//  (<) little endian       (>) big endian (! also)     (=) native endian
//  (c) 8-bit  char         (b) 8-bit  byte
//  (h) 16-bit half         (w) 16-bit word
//  (i) 32-bit integer      (u) 32-bit unsigned         (f) 32-bit float
//  (l) 64-bit long         (q) 64-bit quad             (d) 64-bit double
//  (v) varint
//  (s) string   (64-bit varint length prepended)
//  (S) string   (32-bit fixed  length prepended)
//  (m) memblock (64-bit varint length prepended)
//  (M) memblock (32-bit fixed  length prepended)
//  (z) memblock (zeroed)
//  (#) number of arguments processed (only when unpacking)
//
// @todo:
// - (x) document & test flag
// @totest:
// - (s) string   (64-bit variable length automatically prepended)
// - (S) string   (32-bit fixed    length automatically prepended)
// - (m) memblock (64-bit variable length automatically prepended)
// - (M) memblock (32-bit fixed    length automatically prepended)
// - (z) memblock (zeroed)
// - (#) number of arguments processed (only when unpacking)

// - save data dictated by the format string from the buffer. return: number of bytes written, or 0 if error.
//   if first argument is zero, returns number of bytes required for packing.

API int savef(FILE *file, const char *format, ...);
API int saveb(unsigned char *buf, const char *format, ...);

// - load data dictated by the format string into the buffer. return: number of bytes read, or 0 if error.
//   if first argument is zero, returns number of bytes required for unpacking.

API int loadf(FILE *file, const char *format, ...);
API int loadb(const unsigned char *buf, const char *format, ...);

#line 0

#line 1 "fwk_profile.h"
// -----------------------------------------------------------------------------
// profiler & stats (@fixme: threadsafe)

#if !ENABLE_PROFILER
#   define profile(section)             for(int macro(i) = 1; macro(i); macro(i) = 0)
#   define profile_incstat(name, accum) do {} while(0)
#   define profile_setstat(name, value) do {} while(0)
#   define profiler_init()              do {} while(0)
#   define profiler_enable(x)           0
#   define ui_profiler()                do {} while(0)
#else
#   define profile(section) for( \
        struct profile_t *found = profiler_enabled ? \
            map_find_or_add(profiler, section "@" FILELINE, (struct profile_t){NAN} ) : NULL, \
        *doit = found + ( found ? found->cost = -time_us(), 1 : 1 ); doit; \
        doit = found ? found->cost += time_us(), found->avg = found->cost * 0.25 + found->avg * 0.75, NULL : NULL)  ///+
#   define profile_incstat(name, accum) for( \
        struct profile_t *found = profiler_enabled ? map_find_or_add(profiler, name, (struct profile_t){0}) : NULL; \
        found; found->stat += accum, found = NULL) ///+
#   define profile_setstat(name, value) for( \
        struct profile_t *found = profiler_enabled ? map_find_or_add(profiler, name, (struct profile_t){0}) : NULL; \
        found; found->stat = value, found = NULL) ///+
API int profiler_enable(bool on);

struct profile_t { double stat; int32_t cost, avg; }; ///-
typedef map(char *, struct profile_t) profiler_t; ///-
extern API profiler_t profiler; ///-
extern API int profiler_enabled; ///-
#endif
#line 0

#line 1 "fwk_reflect.h"
// C reflection: enums, functions, structs, members and anotations.
// - rlyeh, public domain
//
// @todo: nested structs? pointers in members?
// @todo: declare TYPEDEF(vec3, float[3]), TYPEDEF(mat4, vec4[4]/*float[16]*/)

#ifndef OBJTYPE
#define OBJTYPE(T) 0
#endif

typedef struct reflect_t {
    unsigned id, objtype;
    union {
    unsigned sz;
    unsigned member_offset;
    unsigned enum_value;
    };
    const char *name;
    const char *info;
    void *addr;
    unsigned parent;
    const char *type;
    unsigned bytes;
} reflect_t;

// inscribe api

#define ENUM(V, ...) \
    enum_inscribe(#V,V, "E" __VA_ARGS__ " ("FILELINE")")

#define FUNCTION(F, ...) \
    function_inscribe(#F,(void*)F, "F" __VA_ARGS__ " ("FILELINE")")

#define STRUCT(T, type, member, ...) \
    struct_inscribe(#T,sizeof(T),OBJTYPE(T),"S" " ("FILELINE")"), \
    type_inscribe(#type,sizeof(((T){0}).member),"T" __VA_ARGS__ " ("FILELINE")"), \
    member_inscribe(#T, #member,(uintptr_t)&((T*)0)->member, "M" __VA_ARGS__ " ("FILELINE")", #type, sizeof(((T){0}).member) )

// find api

API unsigned           enum_find(const char *E);
API void *             function_find(const char *F);

API reflect_t          member_find(const char *T, const char *M); /// find specific member
API void *             member_findptr(void *obj, const char *T, const char *M); // @deprecate
API array(reflect_t)*  members_find(const char *T);

// iterate members in a struct

#define each_member(T,R) \
    (array(reflect_t) *found_ = members_find(T); found_; found_ = 0) \
        for(int it_ = 0, end_ = array_count(*found_); it_ != end_; ++it_ ) \
            for(reflect_t *R = &(*found_)[it_]; R; R = 0 )

// private api, still exposed

API void               type_inscribe(const char *TY,unsigned TYsz,const char *infos);
API void               enum_inscribe(const char *E,unsigned Eval,const char *infos);
API void               struct_inscribe(const char *T,unsigned Tsz,unsigned OBJTYPEid, const char *infos);
API void               member_inscribe(const char *T, const char *M,unsigned Msz, const char *infos, const char *type, unsigned bytes);
API void               function_inscribe(const char *F,void *func,const char *infos);
API const char*        symbol_naked(const char *s);

API int                ui_reflect(const char *mask); // *, model* or NULL
#line 0

#line 1 "fwk_render.h"
// -----------------------------------------------------------------------------
// naive rendering framework
// - rlyeh, public domain
//
// IQM skeletal meshes by @lsalzman (public domain) - https://bit.ly/2OQh0Me
// SH code by @ands (public domain) - https://github.com/ands/spherical_harmonics_playground
// SHM code by @jarikomppa (unlicensed) - https://github.com/jarikomppa/shadertoolkit

typedef unsigned handle; // GLuint

// -----------------------------------------------------------------------------
// colors

API unsigned rgba( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
API unsigned bgra( uint8_t b, uint8_t g, uint8_t r, uint8_t a );
API unsigned rgbaf( float r, float g, float b, float a );
API unsigned bgraf( float b, float g, float r, float a );
API unsigned alpha( unsigned rgba );

#define RGBX(rgb,x)   ( ((rgb)&0xFFFFFF) | (((unsigned)(x))<<24) )
#define RGB3(r,g,b)   ( (255<<24) | ((b)<<16) | ((g)<<8) | (r) )
#define RGB4(r,g,b,a) ( ((a)<<24) | ((b)<<16) | ((g)<<8) | (r) )

#define BLACK   RGBX(0x000000,255)
#define WHITE   RGBX(0xE8F1FF,255)

#define RED     RGB3(   255, 0,48 )
#define GREEN   RGB3(  144,255,48 )
#define CYAN    RGB3(   0,192,255 )
#define ORANGE  RGB3(  255,144,48 )
#define PURPLE  RGB3(  102,77,102 ) // 178,128,255 )
#define YELLOW  RGB3(   255,224,0 )
#define GRAY    RGB3(    32,32,32 ) // dark gray
#define SILVER  RGB3( 149,149,149 ) // dark white, gray-ish
#define PINK    RGB3(  255,48,144 )
#define AQUA    RGB3(  48,255,144 )

#define BLUE    RGBX(0xB55A06,255)

API unsigned atorgba(const char *s);
API char *   rgbatoa(unsigned rgba);

// -----------------------------------------------------------------------------
// images

/// flags when constructing the image_t type. see: image, image_from_mem
/// IMAGE_R: 1-channel image (R)
/// IMAGE_RG: 2-channel image (R,G)
/// IMAGE_RGB: 3-channel image (R,G,B)
/// IMAGE_RGBA: 4-channel image (R,G,B,A)
/// IMAGE_FLIP: Flip image vertically
/// IMAGE_FLOAT: Float pixel components
enum IMAGE_FLAGS {
    IMAGE_R     = 0x01000,
    IMAGE_RG    = 0x02000,
    IMAGE_RGB   = 0x04000,
    IMAGE_RGBA  = 0x08000,
    IMAGE_FLIP  = 0x10000,
    IMAGE_FLOAT = 0x20000,
};

/// type that holds linear uncompressed bitmap of any given dimensions.
/// w,h: image dimensions in pixels. `x,y` alias.
/// comps: number of components per pixel. `n` alias.
/// pixels: untyped pointer to linear bitmap data. typed pointers use `pixels8/16/32/f` aliases.
/// see: texture_t
typedef struct image_t {
    union { unsigned x, w; };
    union { unsigned y, h; };
    union { unsigned n, comps; };
    union { void *pixels; uint8_t *pixels8; uint16_t *pixels16; uint32_t *pixels32; float *pixelsf; };
} image_t;

API image_t image(const char *pathfile, int flags);
API image_t image_from_mem(const void *ptr, int len, int flags);
API void    image_destroy(image_t *img);

// -----------------------------------------------------------------------------
// textures

enum TEXTURE_FLAGS {
    // UNIT[0..7]

    TEXTURE_BC1 = 8,  // DXT1, RGB with 8:1 compression ratio (+ optional 1bpp for alpha)
    TEXTURE_BC2 = 16, // DXT3, RGBA with 4:1 compression ratio (BC1 for RGB + 4bpp for alpha)
    TEXTURE_BC3 = 32, // DXT5, RGBA with 4:1 compression ratio (BC1 for RGB + BC4 for A)
//  TEXTURE_BC4,     // Alpha

    TEXTURE_NEAREST = 0,
    TEXTURE_LINEAR = 64,
    TEXTURE_MIPMAPS = 128,

    TEXTURE_CLAMP = 0,
    TEXTURE_BORDER = 0x100,
    TEXTURE_REPEAT = 0x200,

    TEXTURE_BYTE = 0,
    TEXTURE_FLOAT = IMAGE_FLOAT,

    TEXTURE_COLOR = 0,
    TEXTURE_DEPTH = 0x800,

    TEXTURE_R = IMAGE_R,
    TEXTURE_RG = IMAGE_RG,
    TEXTURE_RGB = IMAGE_RGB,
    TEXTURE_RGBA = IMAGE_RGBA,
    TEXTURE_FLIP = IMAGE_FLIP,

    // @fixme
    TEXTURE_SRGB = 1 << 24,
    TEXTURE_BGR = 1 << 25,
    TEXTURE_BGRA = TEXTURE_BGR,
    TEXTURE_ARRAY = 1 << 26,
};

typedef struct texture_t {
    union { unsigned x, w; };
    union { unsigned y, h; };
    union { unsigned z, d; };
    union { unsigned n, bpp; };
    handle id, unit;
    unsigned texel_type;
    unsigned flags;
    char* filename;
    bool transparent;
    unsigned fbo; // for texture recording
    union { unsigned userdata, delay; };
} texture_t;

API texture_t texture_compressed(const char *filename, unsigned flags);
API texture_t texture_compressed_from_mem(const void *data, int len, unsigned flags);

API texture_t texture(const char* filename, int flags);
API texture_t texture_from_mem(const void* ptr, int len, int flags);
API texture_t texture_create(unsigned w, unsigned h, unsigned n, const void *pixels, int flags);
API texture_t texture_checker();
API void      texture_destroy(texture_t *t);
// textureLod(filename, dir, lod);
// void texture_add_loader( int(*loader)(const char *filename, int *w, int *h, int *bpp, int reqbpp, int flags) );
API unsigned  texture_update(texture_t *t, unsigned w, unsigned h, unsigned n, const void *pixels, int flags);

API bool      texture_rec_begin(texture_t *t, unsigned w, unsigned h); // texture_rec
API void      texture_rec_end(texture_t *t); // texture_rec

// -----------------------------------------------------------------------------
// brdf

API texture_t brdf_lut();

// -----------------------------------------------------------------------------
// pbr materials

typedef struct colormap_t {
    vec4 color;
    texture_t *texture;
} colormap_t;

API bool colormap( colormap_t *cm, const char *pbr_material_type, bool load_as_srgb );

typedef struct pbr_material_t {
    char* name;
    colormap_t diffuse;
    colormap_t normals;
    colormap_t specular;
    colormap_t albedo;
    colormap_t roughness;
    colormap_t metallic;
    colormap_t ao;
    colormap_t ambient;
    colormap_t emissive;

    float specular_shininess;
} pbr_material_t;

API bool pbr_material(pbr_material_t *pbr, const char *material);
API void pbr_material_destroy(pbr_material_t *m);

// -----------------------------------------------------------------------------
// fullscreen quads

API void fullscreen_quad_rgb( texture_t texture_rgb, float gamma );
API void fullscreen_quad_rgb_flipped( texture_t texture, float gamma );
API void fullscreen_quad_ycbcr( texture_t texture_YCbCr[3], float gamma );
API void fullscreen_quad_ycbcr_flipped( texture_t texture_YCbCr[3], float gamma );

// -----------------------------------------------------------------------------
// cubemaps

typedef struct cubemap_t {
    unsigned id;    // texture id
    vec3 sh[9];     // precomputed spherical harmonics coefficients
} cubemap_t;

API cubemap_t  cubemap( const image_t image, int flags ); // 1 equirectangular panorama
API cubemap_t  cubemap6( const image_t images[6], int flags ); // 6 cubemap faces
API void       cubemap_destroy(cubemap_t *c);
API cubemap_t* cubemap_get_active();

// -----------------------------------------------------------------------------
// fbos

API unsigned fbo( unsigned texture_color, unsigned texture_depth, int wr_flags );
API void     fbo_bind(unsigned id);
API void     fbo_unbind();
API void     fbo_destroy(unsigned id);

// -----------------------------------------------------------------------------
// shadowmaps

// #ifndef VSMCUBE
// #define VSMCUBE 0
// #endif
// #ifndef VSMBLUR
// #define VSMBLUR 1
// #endif

typedef struct shadowmap_t {
    mat44  shadowmatrix;
    mat44  mvp;
    mat44  mv;
    mat44  proj;
    vec4   light_position;
    int    saved_fb;
    int    saved_viewport[4];
    handle fbo, texture;
    int texture_width;
} shadowmap_t;

API shadowmap_t shadowmap(int texture_width); // = 1024
API void shadowmap_destroy(shadowmap_t *s);

API void shadowmap_set_shadowmatrix(shadowmap_t *s, vec3 aLightPos, vec3 aLightAt, vec3 aLightUp, const mat44 projection);
API void shadowmap_begin(shadowmap_t *s);
API void shadowmap_end(shadowmap_t *s);

// shadowmap utils

API void shadowmatrix_proj(mat44 shm_proj, float aLightFov, float znear, float zfar);
API void shadowmatrix_ortho(mat44 shm_proj, float left, float right, float bottom, float top, float znear, float zfar);

// -----------------------------------------------------------------------------
// shaders

API unsigned shader(const char *vs, const char *fs, const char *attribs, const char *fragcolor, const char *defines);
API unsigned shader_geom(const char *gs, const char *vs, const char *fs, const char *attribs, const char *fragcolor, const char *defines);
API unsigned shader_bind(unsigned program);
API     void shader_bool(const char *uniform, bool i );
API     void shader_int(const char *uniform, int i);
API     void shader_uint(const char *uniform, unsigned i );
API     void shader_float(const char *uniform, float f);
API     void shader_vec2(const char *uniform, vec2 v);
API     void shader_vec3(const char *uniform, vec3 v);
API     void shader_vec3v(const char *uniform, int count, vec3 *v);
API     void shader_vec4(const char *uniform, vec4 v);
API     void shader_mat44(const char *uniform, mat44 m);
API     void shader_texture(const char *sampler, texture_t texture);
API     void shader_texture_unit(const char *sampler, unsigned texture, unsigned unit);
API     void shader_colormap(const char *name, colormap_t cm);
API unsigned shader_get_active();
API void     shader_destroy(unsigned shader);

// reflection. [0..N] are shader properties

API unsigned     shader_properties(unsigned shader);
API char**       shader_property(unsigned shader, unsigned property_no);

API void         shader_apply_param(unsigned shader, unsigned param_no);
API void         shader_apply_params(unsigned shader, const char *parameter_mask);

API int          ui_shader(unsigned shader);
API int          ui_shaders();

// compute shaders
enum BUFFER_MODE {
    BUFFER_READ,
    BUFFER_WRITE,
    BUFFER_READ_WRITE
};

/// Loads the compute shader and compiles a GL program.
/// return: GL program, 0 if failed.
/// cs: shader source code
API unsigned compute(const char *cs);

/// Runs the compute program with provided global workgroup size on x y z grid.
/// wx: global workgroup size x
/// wy: global workgroup size y
/// wz: global workgroup size z
API void compute_dispatch(unsigned wx, unsigned wy, unsigned wz);

/// Binds a texture to the program
/// !!! Set `layer` to -1 to disable layered access.
/// t: texture to bind
/// unit: texture unit bind index
/// level: texture level access (MIP0, MIP1, ...)
/// layer: bind layer
/// access: texture access policy
/// see: BUFFER_MODE
API void shader_image(texture_t t, unsigned unit, unsigned level, int layer, unsigned access);

/// Binds a texture to the program
/// !!! Set `layer` to -1 to disable layered access.
/// texture: GL texture handle
/// unit: texture unit bind index
/// level: texture level access (MIP0, MIP1, ...)
/// layer: bind layer
/// texel_type: image texel format (RGBA8, RGBA32F, ...)
/// access: texture access policy
/// see: BUFFER_MODE
API void shader_image_unit(unsigned texture, unsigned unit, unsigned level, int layer, unsigned texel_type, unsigned access);

// gpu memory barriers

/// Blocks main thread until all memory operations are done by the GPU.
API void write_barrier();

/// Blocks main thread until all image operations are done by the GPU.
API void write_barrier_image();

// ssbo
/// `STATIC`, `DYNAMIC` AND `STREAM` specify the frequency at which we intend to access the data.
/// `DRAW` favors CPU->GPU operations.
/// `READ` favors GPU->CPU operations.
/// `COPY` favors CPU->GPU->CPU operations.
enum SSBO_USAGE {
    STATIC_DRAW,
    STATIC_READ,
    STATIC_COPY,

    DYNAMIC_DRAW,
    DYNAMIC_READ,
    DYNAMIC_COPY,

    STREAM_DRAW,
    STREAM_READ,
    STREAM_COPY
};

enum SSBO_ACCESS {
    SSBO_READ = BUFFER_READ,
    SSBO_WRITE = BUFFER_WRITE,
    SSBO_READ_WRITE = BUFFER_READ_WRITE
};

/// Create Shader Storage Buffer Object
/// !!! `data` can be NULL
/// data: optional pointer to data to upload
/// len: buffer size, must not be 0
/// usage: buffer usage policy
/// see: SSBO_USAGE
API unsigned ssbo_create(const void *data, int len, unsigned usage);

/// Destroys an SSBO resource
API void ssbo_destroy(unsigned ssbo);

/// Updates an existing SSBO
/// !!! `len` can not exceed the original buffer size specified in `ssbo_create` !
/// offset: offset to buffer memory
/// len: amount of data to write
/// data: pointer to data we aim to write, can not be NULL
API void ssbo_update(int offset, int len, const void *data);

/// Bind an SSBO resource to the provided bind unit index
/// ssbo: resource object
/// unit: bind unit index
API void ssbo_bind(unsigned ssbo, unsigned unit);

/// Map an SSBO resource to the system memory
/// !!! Make sure to `ssbo_unmap` the buffer once done working with it.
/// access: buffer access policy
/// return: pointer to physical memory of the buffer
/// see: SSBO_ACCESS
API void *ssbo_map(unsigned access);

/// Unmaps an SSBO resource
/// !!! Pointer provided by `ssbo_map` becomes invalid.
API void ssbo_unmap();

/// Unbinds an SSBO resource
API void ssbo_unbind();

// -----------------------------------------------------------------------------
// meshes (@fixme: deprecate?)

enum MESH_FLAGS {
    MESH_STATIC = 0, // STATIC, DYNAMIC, STREAM // zero|single|many updates per frame
    MESH_STREAM = 1,
    MESH_TRIANGLE_STRIP = 2,
};

typedef struct mesh_t {
    handle vao, vbo, ibo;
    unsigned vertex_count;
    unsigned index_count;
    unsigned flags;

    array(int) lod_collapse_map; // to which neighbor each vertex collapses. ie, [10] -> 7 (used by LODs) @leak

    // @leaks: following members are totally unused. convenient for end-users to keep their custom datas somewhere while processing.
    union {
    array(unsigned) in_index;
    array(vec3i)    in_index3;
    };
    union {
    array(unsigned) out_index;
    array(vec3i)    out_index3;
    };
    union {
    array(float) in_vertex;
    array(vec3) in_vertex3;
    };
    union {
    array(float) out_vertex;
    array(vec3) out_vertex3;
    };
} mesh_t;

API mesh_t mesh();
API   void mesh_update(mesh_t *m, const char *format, int vertex_stride,int vertex_count,const void *interleaved_vertex_data, int index_count,const void *index_data, int flags);
API   void mesh_render(mesh_t *m);
API   void mesh_render_prim(mesh_t *sm, unsigned prim);
API   void mesh_destroy(mesh_t *m);
API   aabb mesh_bounds(mesh_t *m);

// -----------------------------------------------------------------------------
// materials

enum MATERIAL_ENUMS {
    MAX_CHANNELS_PER_MATERIAL = 8
};

typedef struct material_t {
    char *name;

    int count;
    struct material_layer_t {
        char   texname[32];
        handle texture;
        float  value;
        vec4   color; // uint32_t
    } layer[MAX_CHANNELS_PER_MATERIAL];

} material_t;

// -----------------------------------------------------------------------------
// shadertoys

enum {
    SHADERTOY_FLIP_Y = 2,
    SHADERTOY_IGNORE_FBO = 4,
    SHADERTOY_IGNORE_MOUSE = 8,
};

typedef struct shadertoy_t {
    handle vao, program;
    int uniforms[32];
    int texture_channels[4];
    int frame;
    uint64_t t;
    texture_t tx;
    vec2i dims;
    int flags;
    vec4 mouse;
} shadertoy_t;

API shadertoy_t  shadertoy( const char *shaderfile, unsigned flags );
API shadertoy_t* shadertoy_render( shadertoy_t *s, float delta );

// -----------------------------------------------------------------------------
// anims

enum ANIM_FLAGS {
    ANIM_LOOP = 1,
    ANIM_DONT_RESET_AFTER_USE = 2,
};

typedef struct anim_t {
    int        from;
    int        to;
    float      blendtime;
    unsigned   flags;
    float      curframe;

    unsigned   easing;
    float      alpha; // refreshed at every tick
    float      timer; // private
    bool       active;

    vec3       pose; // private
    char*      name; // debug
} anim_t;

API anim_t clip(float minframe, float maxframe, float blendtime, unsigned flags);
API anim_t loop(float minframe, float maxframe, float blendtime, unsigned flags);
//API array(anim_t) animlist(const char *filename); // @todo

// -----------------------------------------------------------------------------
// models

enum MODEL_FLAGS {
    MODEL_NO_ANIMATIONS = 1,
    MODEL_NO_MESHES = 2,
    MODEL_NO_TEXTURES = 4,
    MODEL_MATCAPS = 8,
    MODEL_RIMLIGHT = 16
};

//@todo: make this data-driven
// enum SHADING_MODE {
//     SHADING_NONE,
//     SHADING_PHONG,
//     SHADING_CARTOON,
//     // SHADING_PBR,
// };

typedef struct model_t {
    struct iqm_t *iqm; // private

    unsigned num_textures;
    handle *textures;
    char **texture_names;
    array(material_t) materials;

    texture_t lightmap;
    float *lmdata;

    unsigned num_meshes;
    unsigned num_triangles;
    unsigned num_joints; // num_poses;
    unsigned num_anims;
    unsigned num_frames;
    handle program;
    float curframe;
    mat44 pivot;

    int stride; // usually 68 bytes for a p3 u2 u2 n3 t4 i4B w4B c4B vertex stream
    void *verts;
    int num_verts;
    void *tris;
    int num_tris;
    handle vao, ibo, vbo, vao_instanced;

    unsigned flags;
    unsigned billboard;

    float *instanced_matrices;
    unsigned num_instances;
} model_t;

enum BILLBOARD_MODE {
    BILLBOARD_X = 0x1,
    BILLBOARD_Y = 0x2,
    BILLBOARD_Z = 0x4,

    BILLBOARD_CYLINDRICAL = BILLBOARD_X|BILLBOARD_Z,
    BILLBOARD_SPHERICAL = BILLBOARD_X|BILLBOARD_Y|BILLBOARD_Z
};

API model_t  model(const char *filename, int flags);
API model_t  model_from_mem(const void *mem, int sz, int flags);
API float    model_animate(model_t, float curframe);
API float    model_animate_clip(model_t, float curframe, int minframe, int maxframe, bool loop);
API float    model_animate_blends(model_t m, anim_t *primary, anim_t *secondary, float delta);
API aabb     model_aabb(model_t, mat44 transform);
API void     model_render(model_t, mat44 proj, mat44 view, mat44 model, int shader);
API void     model_render_skeleton(model_t, mat44 model);
API void     model_render_instanced(model_t, mat44 proj, mat44 view, mat44 *models, int shader, unsigned count);
API void     model_set_texture(model_t, texture_t t);
API bool     model_get_bone_pose(model_t m, unsigned joint, mat34 *out);
API void     model_destroy(model_t);

API vec3     pose(bool forward, float curframe, int minframe, int maxframe, bool loop, float *opt_retframe);

// -----------------------------------------------------------------------------
// model animations

typedef struct anims_t {
    int   inuse; // animation number in use
    float speed; // x1.00
    array(anim_t) anims; // [begin,end,flags] frames of every animation in set
} anims_t;

API anims_t animations(const char *pathfile, int flags);

// -----------------------------------------------------------------------------
// lightmapping utils
// @fixme: support xatlas uv packing

typedef struct lightmap_t {
    struct lm_context *ctx; // private
    bool ready;
    int w, h;
    int atlas_w, atlas_h; //@fixme: implement
    texture_t atlas; //@fixme: implement this
    array(model_t*) models;
    unsigned shader;
} lightmap_t;

API lightmap_t lightmap(int hmsize /*64*/, float near, float far, vec3 color /*1,1,1 for AO*/, int passes /*2*/, float threshold /*0.01f*/, float distmod /*0.0f*/);
API void       lightmap_setup(lightmap_t *lm, int w, int h);
API void          lightmap_bake(lightmap_t *lm, int bounces, void (*drawscene)(lightmap_t *lm, model_t *m, float *view, float *proj, void *userdata), void (*progressupdate)(float progress), void *userdata);
API void       lightmap_destroy(lightmap_t *lm);

// -----------------------------------------------------------------------------
// skyboxes

typedef struct skybox_t {
    handle program;
    mesh_t geometry;
    cubemap_t cubemap;
    int flags;

    // mie
    int framebuffers[6];
    int textures[6];
    float *pixels;
} skybox_t;

API skybox_t skybox(const char *panorama_or_cubemap_folder, int flags);
API int      skybox_render(skybox_t *sky, mat44 proj, mat44 view);
API void     skybox_destroy(skybox_t *sky);
API void     skybox_mie_calc_sh(skybox_t *sky, float sky_intensity);
API void     skybox_sh_reset(skybox_t *sky);
API void     skybox_sh_add_light(skybox_t *sky, vec3 light, vec3 dir, float strength);

API int      skybox_push_state(skybox_t *sky, mat44 proj, mat44 view); // @to deprecate
API int      skybox_pop_state(); // @to deprecate

// -----------------------------------------------------------------------------
// post-fxs

API void     viewport_clear(bool color, bool depth);
API void     viewport_clip(vec2 from, vec2 to);

API int      fx_load(const char *file);
API int      fx_load_from_mem(const char *nameid, const char *content);
API void     fx_begin();
API void     fx_begin_res(int w, int h);
API void     fx_end();
API void     fx_enable(int pass, int enabled);
API int      fx_enabled(int pass);
API void     fx_enable_all(int enabled);
API char *   fx_name(int pass);
API int      fx_find(const char *name);

API int      ui_fx(int pass);
API int      ui_fxs();

// -----------------------------------------------------------------------------
// utils

API void*    screenshot(int components); // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA
API void*    screenshot_async(int components); // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA
#line 0

#line 1 "fwk_renderdd.h"
// -----------------------------------------------------------------------------
// debugdraw framework
// - rlyeh, public domain.
//
// Credits: Based on work by @glampert https://github.com/glampert/debug-draw (PD)
// [x] grid, axis, frustum, cube, sphere, triangle, square, pentagon, hexagon, circle, normal.
// [x] arrow, point, text, capsule, aabb, plane, flotilla-style locator, boid, bone, ring
// [x] line batching
// [*] line width and stipple
// [*] (proper) gizmo,
// [ ] camera, light bulb, light probe,

API void ddraw_color(unsigned rgb);
API void ddraw_color_push(unsigned rgb);
API void ddraw_color_pop();
//
API void ddraw_ontop(int enabled);
API void ddraw_ontop_push(int enabled);
API void ddraw_ontop_pop();
//
API void ddraw_push_2d();
API void ddraw_pop_2d();
//
API void ddraw_aabb(vec3 minbb, vec3 maxbb);
API void ddraw_aabb_corners(vec3 minbb, vec3 maxbb);
API void ddraw_arrow(vec3 begin, vec3 end);
API void ddraw_axis(float units);
API void ddraw_boid(vec3 pos, vec3 dir);
API void ddraw_bone(vec3 center, vec3 end); // @todo: use me
API void ddraw_bounds(const vec3 points[8]);
API void ddraw_box(vec3 center, vec3 extents);
API void ddraw_capsule(vec3 from, vec3 to, float radius);
API void ddraw_circle(vec3 pos, vec3 n, float radius);
API void ddraw_ring(vec3 pos, vec3 n, float radius);
API void ddraw_cone(vec3 center, vec3 top, float radius);
API void ddraw_cube(vec3 center, float radius);
API void ddraw_cube33(vec3 center, vec3 radius, mat33 M);
API void ddraw_diamond(vec3 from, vec3 to, float size);
API void ddraw_frustum(float projview[16]);
API void ddraw_ground(float scale);
API void ddraw_grid(float scale);
API void ddraw_hexagon(vec3 pos, float radius);
API void ddraw_line(vec3 from, vec3 to);
API void ddraw_line_dashed(vec3 from, vec3 to);
API void ddraw_line_thin(vec3 from, vec3 to);
API void ddraw_normal(vec3 pos, vec3 n);
API void ddraw_pentagon(vec3 pos, float radius);
API void ddraw_plane(vec3 p, vec3 n, float scale);
API void ddraw_point(vec3 from);
API void ddraw_position(vec3 pos, float radius);
API void ddraw_position_dir(vec3 pos, vec3 dir, float radius);
API void ddraw_pyramid(vec3 center, float height, int segments);
API void ddraw_cylinder(vec3 center, float height, int segments);
API void ddraw_sphere(vec3 pos, float radius);
API void ddraw_square(vec3 pos, float radius);
API void ddraw_text(vec3 pos, float scale, const char *text);
API void ddraw_text2d(vec2 pos, const char *text);
API void ddraw_triangle(vec3 p1, vec3 p2, vec3 p3);
//
API void ddraw_prism(vec3 center, float radius, float height, vec3 normal, int segments);
//
API void ddraw_demo();
API void ddraw_flush();
API void ddraw_flush_projview(mat44 proj, mat44 view);

// transform gizmos

API int  gizmo(vec3 *pos, vec3 *rot, vec3 *sca);
API bool gizmo_active();
API bool gizmo_hover();
#line 0

#line 1 "fwk_scene.h"
// -----------------------------------------------------------------------------
// scene framework
// - rlyeh, public domain

// camera

typedef struct camera_t {
    mat44 view, proj;
    vec3 position, updir, lookdir;
    float yaw, pitch; // mirror of (x,y) lookdir in deg;
    float speed, fov; // fov in deg(45)

    float move_friction, move_damping;
    float look_friction, look_damping;
    vec2 last_look; vec3 last_move; // used for friction and damping
    bool damping;

    bool orthographic; // 0 perspective, 1 orthographic; when ortho: dimetric[if pitch == -30º], isometric[if pitch == 35.264º]
    float distance;    // distance to pivot, when orbiting
    // vec2 polarity = { +1,-1 }; // @todo
    // vec2 sensitivity = { 2,2 }; // @todo
} camera_t;

API camera_t camera();
API void camera_teleport(camera_t *cam, vec3 pos);
API void camera_moveby(camera_t *cam, vec3 inc);
API void camera_fov(camera_t *cam, float fov);
API void camera_fps(camera_t *cam, float yaw, float pitch);
API void camera_orbit(camera_t *cam, float yaw, float pitch, float inc_distance);
API void camera_lookat(camera_t *cam, vec3 target);
API void camera_enable(camera_t *cam);
API camera_t *camera_get_active();

API int  ui_camera(camera_t *cam);
API void ddraw_camera(camera_t *cam);

// object

typedef struct object_t {
    uint64_t renderbucket;
    mat44 transform;
    quat rot;
    vec3 sca, pos, euler, pivot;
    array(handle) textures;
    model_t model;
    aabb bounds;
    unsigned billboard; // [0..7] x(4),y(2),z(1) masks
    bool light_cached; //< used by scene to update light data
} object_t;

API object_t object();
API void object_rotate(object_t *obj, vec3 euler);
API void object_pivot(object_t *obj, vec3 euler);
API void object_teleport(object_t *obj, vec3 pos);
API void object_move(object_t *obj, vec3 inc);
API vec3 object_position(object_t *obj);
API void object_scale(object_t *obj, vec3 sca);
//
API void object_model(object_t *obj, model_t model);
API void object_diffuse(object_t *obj, texture_t tex);
API void object_diffuse_push(object_t *obj, texture_t tex);
API void object_diffuse_pop(object_t *obj);
API void object_billboard(object_t *obj, unsigned mode);

// object_pose(transform); // @todo


// light
enum LIGHT_TYPE {
    LIGHT_DIRECTIONAL,
    LIGHT_POINT,
    LIGHT_SPOT,
};

enum LIGHT_FLAGS {
    LIGHT_CAST_SHADOWS = 1,
};

typedef struct light_t {
    char type;
    vec3 diffuse, specular, ambient;
    vec3 pos, dir;
    struct {
        float constant, linear, quadratic;
    } falloff;
    float specularPower;
    float innerCone, outerCone;
    //@todo: cookie, flare

    // internals
    bool cached; //< used by scene to invalidate cached light data
} light_t;

API light_t light();
// API void    light_flags(int flags);
API void    light_type(light_t* l, char type);
API void    light_diffuse(light_t* l, vec3 color);
API void    light_specular(light_t* l, vec3 color);
API void    light_ambient(light_t* l, vec3 color);
API void    light_teleport(light_t* l, vec3 pos);
API void    light_dir(light_t* l, vec3 dir);
API void    light_power(light_t* l, float power);
API void    light_falloff(light_t* l, float constant, float linear, float quadratic);
API void    light_cone(light_t* l, float innerCone, float outerCone);
API void    light_update(unsigned num_lights, light_t *lv);

// scene

enum SCENE_FLAGS {
    SCENE_WIREFRAME = 1,
    SCENE_CULLFACE = 2,
    SCENE_BACKGROUND = 4,
    SCENE_FOREGROUND = 8,
    SCENE_UPDATE_SH_COEF = 16,
};

typedef struct scene_t {
    array(object_t) objs;
    array(light_t) lights;

    // special objects below:
    skybox_t skybox;
    int u_coefficients_sh;
} scene_t;

API scene_t*  scene_push();
API void      scene_pop();
API scene_t*  scene_get_active();

API int       scene_merge(const char *source);
API void      scene_render(int flags);

API object_t* scene_spawn();
API unsigned  scene_count();
API object_t* scene_index(unsigned index);

API light_t*  scene_spawn_light();
API unsigned  scene_count_light();
API light_t*  scene_index_light(unsigned index);
#line 0

#line 1 "fwk_string.h"
// string framework
// - rlyeh, public domain

// string: temporary api (stack)
API char*   tempvl(const char *fmt, va_list);
API char*   tempva(const char *fmt, ...);
#define     va(...) (((&printf) || printf(__VA_ARGS__), tempva(__VA_ARGS__)))  // vs2015 check trick
#define     vac (const char*)va

// string: allocated api (heap). FREE() after use
API char*   strcatf(char **s, const char *buf);
#define     strcatf(s,fmt,...)  strcatf((s), va(fmt, __VA_ARGS__))
#define     stringf(fmt,...)    STRDUP(va(fmt, __VA_ARGS__)) // (strcatf)(0, va(fmt, __VA_ARGS__))


#if is(cl) || (is(tcc) && is(win32))
#if!is(cl)
char* strtok_s(char* str,const char* delimiters,char** context); // tcc misses this in <string.h>
#endif
#define strtok_r strtok_s
#endif

#if 1
#define each_substring(str, delims, keyname) \
    ( char *str_ = (char*)(str); str_; str_ = 0 ) \
    for( int len_ = strlen(str_) + 1, heap_ = len_ < 1024; len_ > 1; len_ = 0 ) \
    for( char *ptr_ = (heap_ ? REALLOC(0, len_) : ALLOCA(len_)), *cpy_ = (snprintf(ptr_, len_, "%s", str_), ptr_); ptr_; (heap_ ? REALLOC(ptr_, 0) : 0), ptr_ = 0 ) \
    for( char *next_token = 0, *keyname = strtok_r(cpy_, delims, &next_token); keyname; keyname = strtok_r(NULL, delims, &next_token) )
#else
#define each_substring(str, delims, keyname) \
    ( char** tokens_ = strsplit((str), (delims)), *keyname = 0; tokens_; tokens_ = 0) \
    for( int i_ = 0, end_ = array_count(tokens_); i_ < (keyname = tokens_[i_], end_); ++i_ )
#endif

// utils

API int          strmatch(const char *s, const char *wildcard);
API int          strmatchi(const char *s, const char *wildcard);

API int          strcmp_qsort(const void *a, const void *b);
API int          strcmpi_qsort(const void *a, const void *b);

API bool         strbeg(const char *src, const char *sub); // returns true if both strings match at beginning. case sensitive
API bool         strend(const char *src, const char *sub); // returns true if both strings match at end. case sensitive

API bool         strbegi(const char *src, const char *sub);  // returns true if both strings match at beginning. case insensitive
API bool         strendi(const char *src, const char *sub);  // returns true if both strings match at end. case insensitive
API const char * strstri(const char *src, const char *sub);  // returns find first substring in string. case insensitive.
#define          strcmpi  ifdef(cl, _stricmp, strcasecmp)

API char *       strupper(const char *str);
API char *       strlower(const char *str);

API char *       strrepl(char **copy, const char *target, const char *replace); // replace any 'target' as 'repl' in 'copy'. 'copy' may change (heap). returns 'copy'
API char *       strswap(char *copy, const char *target, const char *replace);  // replaced inline only if repl is shorter than target. no allocations.
API char *       strcut(char *copy, const char *target);                        // remove any 'target' in 'copy'. returns 'copy'

API const char * strlerp(unsigned numpairs, const char **pairs, const char *str); // using key-value pairs, null-terminated

#ifndef __APPLE__ // BSD provides these
API size_t       strlcat(char *dst, const char *src, size_t dstcap); // concat 2 strings safely. always NUL terminates. may truncate.
API size_t       strlcpy(char *dst, const char *src, size_t dstcap); // copy 2 strings safely. always NUL terminates. truncates if retval>=dstcap
#endif

/// split `string` after any of `delimiters` character is found.
/// returns temporary array of split strings. see: strjoin
/// > array(char*) tokens = strsplit("hello! world!", " !"); // [0]="hello",[1]="world",
API array(char*)    strsplit(const char *string, const char *delimiters);

/// concatenate all elements within `list`, with `separator` string in between.
/// returns: temporary joint string. see: strsplit
/// > array(char*) tokens = strsplit("hello! world!", " !"); // [0]="hello",[1]="world",
/// > char *joint = strjoin(tokens, "+"); // joint="hello+world"
API char*           strjoin(array(char*) list, const char *separator);

API char*           string8(const wchar_t *str);  /// convert from wchar16(win) to utf8/ascii
API array(uint32_t) string32( const char *utf8 ); /// convert from utf8 to utf32

API const char*     codepoint_to_utf8(unsigned cp);

// -----------------------------------------------------------------------------
// ## string interning (quarks)
// - rlyeh, public domain.

API unsigned    intern( const char *string );
API const char *quark( unsigned key );

typedef struct quarks_db {
    array(char) blob;
    array(vec2i) entries;
} quarks_db;

API unsigned    quark_intern( quarks_db*, const char *string );
API const char *quark_string( quarks_db*, unsigned key );

// -----------------------------------------------------------------------------
// ## localization kit (I18N, L10N)

API bool  kit_load( const char *filename ); // load translations file (xlsx)
API bool  kit_merge( const char *filename ); // merge translations file into existing context
API void  kit_insert( const char *id, const char *translation ); // insert single translation unit
API void  kit_clear(); // delete all translations

API void  kit_set( const char *variable, const char *value ); // set context variable
API void  kit_reset(); // reset all variables in context
API void  kit_dump_state( FILE *fp ); // debug

API char* kit_translate2( const char *id, const char *langcode_iso639_1 ); // perform a translation given explicit locale

API void  kit_locale( const char *langcode_iso639_1 ); // set current locale: enUS, ptBR, esES, ...
API char* kit_translate( const char *id ); // perform a translation, given current locale
#line 0

#line 1 "fwk_sprite.h"
// -----------------------------------------------------------------------------
// sprites

typedef enum SPRITE_FLAGS {
    SPRITE_PROJECTED = 1,
    SPRITE_ADDITIVE = 2,
    SPRITE_CENTERED = 4,
    SPRITE_RESOLUTION_INDEPENDANT = 128,
} SPRITE_FLAGS;

// texture id, position(x,y,depth sort), tint color, rotation angle
API void sprite( texture_t texture, float position[3], float rotation /*0*/, unsigned color /*~0u*/, unsigned flags);

// texture id, rect(x,y,w,h) is [0..1] normalized, then: pos(xyz,z-index), (scale.xy,offset.xy), rotation (degrees), color (rgba)
API void sprite_rect( texture_t t, vec4 rect, vec4 pos, vec4 scaleoff, float tilt_deg, unsigned tint_rgba, unsigned flags);

// texture id, sheet(frameNumber,X,Y) (frame in a X*Y spritesheet), position(x,y,depth sort), rotation angle, offset(x,y), scale(x,y), is_additive, tint color
API void sprite_sheet( texture_t texture, float sheet[3], float position[3], float rotation, float offset[2], float scale[2], unsigned rgba, unsigned flags);

API void sprite_flush();

// -----------------------------------------------------------------------------
// tilemaps

typedef struct tileset_t {
    texture_t tex;            // spritesheet
    unsigned tile_w, tile_h;  // dimensions per tile in pixels
    unsigned cols, rows;      // tileset num_cols, num_rows
    unsigned selected;        // active tile (while editing)
} tileset_t;

API tileset_t tileset(texture_t tex, unsigned tile_w, unsigned tile_h, unsigned cols, unsigned rows);

API int       ui_tileset( tileset_t t );

typedef struct tilemap_t {
    int blank_chr;                // transparent tile
    unsigned cols, rows;          // map dimensions (in tiles)
    array(int) map;

    vec3 position;                // x,y,scale
    float zindex;
    float tilt;
    unsigned tint;
    bool is_additive;
} tilemap_t;

API tilemap_t tilemap(const char *map, int blank_chr, int linefeed_chr);
API void      tilemap_render( tilemap_t m, tileset_t style );
API void      tilemap_render_ext( tilemap_t m, tileset_t style, float zindex, float xy_zoom[3], float tilt, unsigned tint, bool is_additive );

// -----------------------------------------------------------------------------
// tiled maps

typedef struct tiled_t {
    char *map_name;
    unsigned first_gid, tilew, tileh, w, h;

    bool parallax;
    vec3 position;
    array(bool) visible;
    array(tilemap_t) layers;
    array(tileset_t) sets;
    array(char*) names;
} tiled_t;

API tiled_t tiled(const char *file_tmx);
API void    tiled_render(tiled_t tmx, vec3 pos);

API void    ui_tiled(tiled_t *t);

// -----------------------------------------------------------------------------
// spines

typedef struct spine_t spine_t;

API spine_t*spine(const char *file_json, const char *file_atlas, unsigned flags);
API void    spine_skin(spine_t *p, unsigned skin);
API void    spine_render(spine_t *p, vec3 offset, unsigned flags);
API void    spine_animate(spine_t *p, float delta);

API void    ui_spine(spine_t *p);

// ----------------------------------------------------------------------------
// atlas api
typedef struct atlas_frame_t {
    unsigned delay;
    vec4 sheet;
    vec2 anchor; // @todo
    array(vec3i) indices;
    array(vec2) coords;
    array(vec2) uvs;
} atlas_frame_t;

typedef struct atlas_anim_t {
    unsigned name;
    array(unsigned) frames;
} atlas_anim_t;

typedef struct atlas_slice_frame_t {
    vec4 bounds;
    bool has_9slice;
    vec4 core;
    vec2 pivot;
    unsigned color;
    char *text;
} atlas_slice_frame_t;

typedef struct atlas_slice_t {
    unsigned name;
    array(unsigned) frames;
} atlas_slice_t;

typedef struct atlas_t {
    texture_t tex;

    array(atlas_frame_t) frames;
    array(atlas_anim_t)  anims;
    array(atlas_slice_t) slices;
    array(atlas_slice_frame_t) slice_frames;

    quarks_db db;
} atlas_t;

API atlas_t atlas_create(const char *inifile, unsigned flags);
API int         ui_atlas(atlas_t *a);
API int         ui_atlas_frame(atlas_frame_t *f);
API void    atlas_destroy(atlas_t *a);

// ----------------------------------------------------------------------------
// sprite v2 api

typedef struct sprite_t { OBJ
    vec4 gamepad; // up,down,left,right
    vec2 fire;    // a,b

    vec4 pos;
    vec2 sca;
    float tilt;
    unsigned tint;
    unsigned frame;
    unsigned timer, timer_ms;
    unsigned flip_, flipped;
    unsigned play;
    bool paused;
    // array(unsigned) play_queue; or unsigned play_next;
    struct atlas_t *a; // shared
    //atlas_t own; // owned
} sprite_t;

OBJTYPEDEF(sprite_t,10);
API void     sprite_ctor(sprite_t *s);
API void     sprite_dtor(sprite_t *s);
API void     sprite_tick(sprite_t *s);
API void     sprite_draw(sprite_t *s);
API void     sprite_edit(sprite_t *s);

API sprite_t*sprite_new(const char *ase, int bindings[6]);
API void     sprite_del(sprite_t *s);
API void     sprite_setanim(sprite_t *s, unsigned name);
#line 0
#line 1 "fwk_gui.h"
// ----------------------------------------------------------------------------
// game ui

typedef struct guiskin_t {
    void (*drawrect)(void* userdata, const char *skin, const char *fallback, vec4 rect);
    void (*getskinsize)(void* userdata, const char *skin, const char *fallback, vec2 *size);
    void (*getskincolor)(void* userdata, const char *skin, const char *fallback, unsigned *color);
    void (*getscissorrect)(void* userdata, const char *skin, const char *fallback, vec4 rect, vec4 *dims);
    bool (*ismouseinrect)(void* userdata, const char *skin, const char *fallback, vec4 rect);
    void (*free)(void* userdata);
    void *userdata;
} guiskin_t;

API void    gui_pushskin(guiskin_t skin);
API void*       gui_userdata();
API vec2        gui_getskinsize(const char *skin, const char *fallback);
API unsigned    gui_getskincolor(const char *skin, const char *fallback);
API bool        gui_ismouseinrect(const char *skin, const char *fallback, vec4 rect);
API vec4        gui_getscissorrect(const char *skin, const char *fallback, vec4 rect);
// --
API void        gui_panel_id(int id, vec4 rect, const char *skin);
API void            gui_rect_id(int id, vec4 rect, const char *skin);
API void            gui_label_id(int id, const char *skin, const char *text, vec4 rect);
API bool            gui_button_id(int id, vec4 rect, const char *skin);
API bool            gui_button_label_id(int id, const char *text, vec4 rect, const char *skin);
API bool            gui_slider_id(int id, vec4 rect, const char *skin, float min, float max, float step, float *value);
API bool            gui_slider_label_id(int id, const char *text, vec4 rect, const char *skin, float min, float max, float step, float *value);
API void        gui_panel_end();
API void    gui_popskin();

// helpers
#define gui_panel(...) gui_panel_id(__LINE__, __VA_ARGS__)
#define gui_rect(...) gui_rect_id(__LINE__, __VA_ARGS__)
#define gui_label(...) gui_label_id(__LINE__, __VA_ARGS__)
#define gui_button(...) gui_button_id(__LINE__, __VA_ARGS__)
#define gui_button_label(...) gui_button_label_id(__LINE__, __VA_ARGS__)
#define gui_slider(...) gui_slider_id(__LINE__, __VA_ARGS__)
#define gui_slider_label(...) gui_slider_label_id(__LINE__, __VA_ARGS__)

// default renderers

typedef struct skinned_t {
    atlas_t atlas;
    float scale;
} skinned_t;

// The skinning engine depends on an Aseprite asset with slices set up.
//   While you can specify your own skins for various GUI widgets, some
//   skin variants are hardcoded and expected to be present in your asset:
//
//   gui_panel():
//    - "panel" (overridable)
//   gui_button():
//    - "button" (base overridable)
//      - "_hover" (ex. "scarybtn_hover")
//      - "_press"
//   gui_rect():
//    - no defaults, always pass your own skin/slice name
//   gui_slider():
//    - "slider" (overridable)
//    - "slider_cursor" (partially overridable, ex. "bigslider_cursor")
//      - "_hover" (ex. "slider_cursor_hover")
//      - "_press"
//
API guiskin_t gui_skinned(const char *asefile, float scale);
#line 0

#line 1 "fwk_steam.h"
// ----------------------------------------------------------------------------
// steam framework

API bool steam_init(unsigned app_id);
API void steam_tick();
API void steam_trophy(const char *trophy_id, bool redeem);
API void steam_screenshot();
API void steam_destroy();

API int  ui_steam();
#line 0

#line 1 "fwk_system.h"
// -----------------------------------------------------------------------------
// system framework utils
// - rlyeh, public domain.
//
// Note: Windows users add `/Zi` compilation flags, else add `-g` and/or `-ldl` flags
// Note: If you are linking your binary using GNU ld you need to add --export-dynamic

API void*       thread( int (*thread_func)(void* user_data), void* user_data );
API void        thread_destroy( void *thd );

API int         argc();
API char*       argv(int);
API void        argvadd(const char *arg);

API int         flag(const char *commalist); // --arg // app_flag?
API const char* option(const char *commalist, const char *defaults); // --arg=string or --arg string
API int         optioni(const char *commalist, int defaults); // --arg=integer or --arg integer  // argvi() ?
API float       optionf(const char *commalist, float defaults); // --arg=float or --arg float    // flagf() ?

API void        tty_attach();
API void        tty_detach();
API void        tty_color(unsigned color);
API void        tty_reset();

API const char* app_exec(const char *command); // returns ("%15d %s", retcode, output_last_line)
API int         app_spawn(const char *command);
API int         app_cores();
API int         app_battery(); /// returns battery level [1..100]. also positive if charging (+), negative if discharging (-), and 0 if no battery is present.

API const char* app_name();
API const char* app_path();
API const char* app_cache();
API const char* app_temp();
API const char* app_cmdline();

API void        app_beep();
API void        app_hang();
API void        app_crash();
API void        app_singleton(const char *guid);
API bool        app_open(const char *folder_file_or_url);

API const char* app_loadfile();
API const char* app_savefile();


API char*       callstack( int traces ); // write callstack into a temporary string. <0 traces to invert order. do not free().
API int         callstackf( FILE *fp, int traces ); // write callstack to file. <0 traces to invert order.

API void        die(const char *message);
API void        alert(const char *message);
API void        hexdump( const void *ptr, unsigned len );
API void        hexdumpf( FILE *fp, const void *ptr, unsigned len, int width );
API void        breakpoint();
API bool        has_debugger();

API void        trap_install(void);
API const char *trap_name(int signal);      // helper util
API void        trap_on_ignore(int signal); // helper util
API void        trap_on_quit(int signal);   // helper util
API void        trap_on_abort(int signal);  // helper util
API void        trap_on_debug(int signal);  // helper util

#define PANIC(...)   PANIC(va(""__VA_ARGS__), __FILE__, __LINE__) // die() ?
API int (PANIC)(const char *error, const char *file, int line);

#define PRINTF(...)  PRINTF(va(""__VA_ARGS__), 1[""#__VA_ARGS__] == '!' ? callstack(+48) : "", __FILE__, __LINE__, __FUNCTION__)
API int (PRINTF)(const char *text, const char *stack, const char *file, int line, const char *function);

#define test(expr) test(__FILE__,__LINE__,#expr,!!(expr))
API int (test)(const char *file, int line, const char *expr, bool result);

#if ENABLE_AUTOTESTS
#define AUTOTEST AUTORUN
#else
#define AUTOTEST static void concat(concat(concat(disabled_test_, __LINE__), _), __COUNTER__)()
#endif

// AUTOTEST { test(1<2); }

#if ENABLE_RETAIL
#undef  PRINTF
#define PRINTF(...) 0
#undef  test
#define test(expr)  0
#endif
#line 0

#line 1 "fwk_time.h"
// -----------------------------------------------------------------------------
// time framework utils

API uint64_t    date();        // YYYYMMDDhhmmss
API uint64_t    date_epoch();  // linux epoch
API char*       date_string(); // "YYYY-MM-DD hh:mm:ss"
API double      time_hh();
API double      time_mm();
API double      time_ss();
API uint64_t    time_ms();
API uint64_t    time_us();
API uint64_t    time_ns();
API void        sleep_ss(double ss);
API void        sleep_ms(double ms);
API void        sleep_us(double us);
API void        sleep_ns(double us);

API unsigned    timer(unsigned ms, unsigned (*callback)(unsigned ms, void *arg), void *arg);
API void        timer_destroy(unsigned timer_handle);

// time sortable unique identifier (similar to ksuid/tuid; others: sno/xid/cuid/ulid)
// - rlyeh, public domain.
//
// also similar to a mongo object id, 12 bytes as follows:
// - 4-byte timestamp (ss). epoch: Tuesday, 12 September 2023 6:06:56
// - 2-byte (machine, hash or app id)
// - 2-byte (thread-id)
// - 4-byte (rand counter, that gets increased at every id creation)

typedef vec3i guid;

API guid        guid_create();

/*
AUTORUN {
    guid g1 = guid_create();
    guid g2 = guid_create();
    print3i(g1);
    hexdump(&g1, sizeof(g1));
    print3i(g2);
    hexdump(&g2, sizeof(g2));
}
*/

// ----------------------------------------------------------------------------
// ease

API float ease_zero(float t);
API float ease_one(float t);
API float ease_linear(float t);

API float ease_out_sine(float t);
API float ease_out_quad(float t);
API float ease_out_cubic(float t);
API float ease_out_quart(float t);
API float ease_out_quint(float t);
API float ease_out_expo(float t);
API float ease_out_circ(float t);
API float ease_out_back(float t);
API float ease_out_elastic(float t);
API float ease_out_bounce(float t);

API float ease_in_sine(float t);
API float ease_in_quad(float t);
API float ease_in_cubic(float t);
API float ease_in_quart(float t);
API float ease_in_quint(float t);
API float ease_in_expo(float t);
API float ease_in_circ(float t);
API float ease_in_back(float t);
API float ease_in_elastic(float t);
API float ease_in_bounce(float t);

API float ease_inout_sine(float t);
API float ease_inout_quad(float t);
API float ease_inout_cubic(float t);
API float ease_inout_quart(float t);
API float ease_inout_quint(float t);
API float ease_inout_expo(float t);
API float ease_inout_circ(float t);
API float ease_inout_back(float t);
API float ease_inout_elastic(float t);
API float ease_inout_bounce(float t);

API float ease_inout_perlin(float t);

enum EASE_FLAGS {
    EASE_SINE,
    EASE_QUAD,
    EASE_CUBIC,
    EASE_QUART,
    EASE_QUINT,
    EASE_EXPO,
    EASE_CIRC,
    EASE_BACK,
    EASE_ELASTIC,
    EASE_BOUNCE,

    EASE_IN,
    EASE_OUT = 0,
    EASE_INOUT = EASE_IN * 2,

    EASE_ZERO = EASE_INOUT | (EASE_BOUNCE + 1),
    EASE_ONE,
    EASE_LINEAR,
    EASE_INOUT_PERLIN,

    EASE_NUM
};

API float ease(float t01, unsigned fn); // / 0-to-1
API float ease_pong(float t01, unsigned fn); // \ 1-to-0
API float ease_ping_pong(float t, unsigned fn1, unsigned fn2); // /\ 0-to-1-to-0
API float ease_pong_ping(float t, unsigned fn1, unsigned fn2); // \/ 1-to-0-to-1

API const char *ease_enum(unsigned fn);
API const char**ease_enums();

// ----------------------------------------------------------------------------
// tween

typedef struct tween_keyframe_t {
    float t;
    vec3 v;
    unsigned ease;
} tween_keyframe_t;

typedef struct tween_t {
    array(tween_keyframe_t) keyframes;

    vec3 result;
    float time;
    float duration;
} tween_t;

API tween_t tween();
API void      tween_setkey(tween_t *tw, float t, vec3 v, unsigned easing_mode);
API void        tween_delkey(tween_t *tw, float t);
API float     tween_update(tween_t *tw, float dt);
API void      tween_reset(tween_t *tw);
API void    tween_destroy(tween_t *tw);

// ----------------------------------------------------------------------------
// curve

typedef struct curve_t {
    array(float) lengths;
    array(unsigned) colors;
    array(vec3)  samples;
    array(vec3)  points;
    array(int)   indices;
} curve_t;

API curve_t curve();
API void      curve_add(curve_t *c, vec3 p);
API void      curve_end(curve_t *c, int num_points);
API vec3       curve_eval(curve_t *c, float dt, unsigned *color);
API void    curve_destroy(curve_t *c);
#line 0

#line 1 "fwk_ui.h"
// -----------------------------------------------------------------------------
// immediate ui framework
// - rlyeh, public domain
//
// @todo: logger/console
// @todo: surround-adaptive window resizing. ie, surrounding windows adapting theirselves to fit dragged active window

enum PANEL_FLAGS {
    PANEL_OPEN = 1,
};

API int ui_notify(const char *title, const char *body);
API int ui_window(const char *title, int *enabled);
API int  ui_panel(const char *title, int flags); // may be embedded inside a window, or standalone
API int   ui_collapse(const char *label, const char *id);
API int   ui_collapseo(const char *label, const char *id);
API int   ui_contextual();
API int    ui_section(const char *title);
API int    ui_int(const char *label, int *value);
API int    ui_bool(const char *label, bool *value);
API int    ui_short(const char *label, short *value);
API int    ui_float(const char *label, float *value);
API int    ui_float2(const char *label, float value[2]);
API int    ui_float3(const char *label, float value[3]);
API int    ui_float4(const char *label, float value[4]);
API int    ui_mat33(const char *label, float mat33[9]);
API int    ui_mat34(const char *label, float mat34[12]);
API int    ui_mat44(const char *label, float mat44[16]);
API int    ui_double(const char *label, double *value);
API int    ui_buffer(const char *label, char *buffer, int buflen);
API int    ui_string(const char *label, char **string);
API int    ui_color3(const char *label, unsigned *color); //[0..255]
API int    ui_color3f(const char *label, float color[3]); //[0..1]
API int    ui_color4(const char *label, unsigned *color); //[0..255]
API int    ui_color4f(const char *label, float color[4]); //[0..1]
API int    ui_unsigned(const char *label, unsigned *value);
API int    ui_unsigned2(const char *label, unsigned *value);
API int    ui_unsigned3(const char *label, unsigned *value);
API int    ui_button(const char *label);
API int    ui_button_transparent(const char *label);
API int    ui_buttons(int buttons, /*labels*/...);
API int    ui_toolbar(const char *options); // int choice = ui_toolbar("A;B;C;D");
API int    ui_submenu(const char *options); // int choice = ui_submenu("A;B;C;D");
API int    ui_browse(const char **outfile, bool *inlined); // may be embedded inside a window or inside a panel
API int    ui_toggle(const char *label, bool *value);
API int    ui_dialog(const char *title, const char *text, int choices, bool *show); // @fixme: return
API int    ui_list(const char *label, const char **items, int num_items, int *selector);
API int    ui_radio(const char *label, const char **items, int num_items, int *selector);
API int    ui_texture(const char *label, texture_t t);
API int    ui_subtexture(const char *label, texture_t t, unsigned x, unsigned y, unsigned w, unsigned h);
API int    ui_image(const char *label, handle id, unsigned w, unsigned h); //(w,h) can be 0
API int    ui_subimage(const char *label, handle id, unsigned iw, unsigned ih, unsigned sx, unsigned sy, unsigned sw, unsigned sh);
API int    ui_colormap(const char *label, colormap_t *cm); // returns num member changed: 1 for color, 2 for texture map
API int    ui_separator();
API int    ui_bitmask8(const char *label, uint8_t *bits);
API int    ui_bitmask16(const char *label, uint16_t *bits);
API int    ui_console();
API int    ui_clampf(const char *label, float *value, float minf, float maxf);
API int    ui_label(const char *label);
API int    ui_label2(const char *label, const char *caption);
API int    ui_label2_bool(const char *label, bool enabled);
API int    ui_label2_float(const char *label, float value);
API int    ui_label2_toolbar(const char *label, const char *icons);
API int    ui_slider(const char *label, float *value);
API int    ui_slider2(const char *label, float *value, const char *caption);
API int   ui_contextual_end(int close);
API int   ui_collapse_clicked();
API int   ui_collapse_end();
API int  ui_panel_end();
API int ui_window_end();

API int  ui_show(const char *panel_or_window_title, int enabled);
API int  ui_dims(const char *panel_or_window_title, float width, float height);
API int  ui_visible(const char *panel_or_window_title); // @todo: include ui_collapse() items that are open as well?
API vec2 ui_get_dims();

API int  ui_enable();
API int  ui_enabled();
API int  ui_disable();

API int ui_has_menubar();
API int ui_menu(const char *items); // semicolon-separated or comma-separated items
API int ui_menu_editbox(char *buf, int bufcap);
API int ui_item();

API int ui_popups(); // ui_any_popup()? ui_has_popups()?
API int ui_hover(); // ui_is_hover()?
API int ui_active(); // ui_is_active()?

API int ui_demo(int do_windows);
API void *ui_handle();
#line 0

#line 1 "fwk_video.h"
// -----------------------------------------------------------------------------
// video decoder (mpeg)
// - rlyeh, public domain
//
// [ref] https://github.com/phoboslab/pl_mpeg/blob/master/pl_mpeg_player.c
// [use] ffmpeg -i infile.mp4 -c:v mpeg1video -c:a mp2 -format mpeg outfile.mpg

enum VIDEO_FLAGS {
    VIDEO_YCBCR = 0,
    VIDEO_RGB = 2,

    VIDEO_AUDIO = 0,
    VIDEO_NO_AUDIO = 4,

    VIDEO_LOOP = 8,
};

typedef struct video_t video_t;

API video_t*   video( const char *filename, int flags );
API texture_t*  video_decode( video_t *v ); // decodes next frame, returns associated texture(s)
API texture_t*  video_textures( video_t *v ); // returns last video textures. does not perform any decoding.
API int         video_has_finished(video_t *v);
API double      video_duration(video_t *v);
API int         video_seek(video_t *v, double seek_to);
API double      video_position(video_t *v);
API void        video_pause(video_t *v, bool paused);
API bool        video_is_paused(video_t *v);
API bool        video_is_rgb(video_t *v);
API void       video_destroy( video_t *v );

// -----------------------------------------------------------------------------
// video recorder (uses external ffmpeg and fallbacks to built-in mpeg1 encoder)
// - rlyeh, public domain
//
// @fixme: MSAA can cause some artifacts with Intel PBOs: either use glDisable(GL_MULTISAMPLE) before recording or do not create window with WINDOW_MSAA at all.

API bool       record_start(const char *outfile_mp4);
API bool        record_active();
API void       record_stop(void);
#line 0

#line 1 "fwk_window.h"
// -----------------------------------------------------------------------------
// window framework
// - rlyeh, public domain
//
// @todo: window_cursor(ico);
// @todo: if WINDOW_PORTRAIT && exist portrait monitor, use that instead of primary one
// @todo: WINDOW_TRAY

enum WINDOW_FLAGS {
    WINDOW_MSAA2 = 0x02,
    WINDOW_MSAA4 = 0x04,
    WINDOW_MSAA8 = 0x08,

    WINDOW_SQUARE = 0x20,
    WINDOW_PORTRAIT = 0x40,
    WINDOW_LANDSCAPE = 0x80,
    WINDOW_ASPECT = 0x100, // keep aspect
    WINDOW_FIXED = 0x200, // disable resizing
    WINDOW_TRANSPARENT = 0x400,
    WINDOW_BORDERLESS = 0x800,

    WINDOW_VSYNC = 0,
    WINDOW_VSYNC_ADAPTIVE = 0x1000,
    WINDOW_VSYNC_DISABLED = 0x2000,
};

API bool     window_create(float scale, unsigned flags);
API bool     window_create_from_handle(void *handle, float scale, unsigned flags);
API void     window_reload();

API int      window_frame_begin();
API void     window_frame_end();
API void     window_frame_swap();
API int      window_swap(); // single function that combines above functions (desktop only)

API void     window_loop(void (*function)(void* loopArg), void* loopArg ); // run main loop function continuously (emscripten only)
API void     window_loop_exit(); // exit from main loop function (emscripten only)

API void     window_title(const char *title);
API void     window_color(unsigned color);
API vec2     window_canvas();
API void*    window_handle();
API char*    window_stats();

API uint64_t window_frame();
API int      window_width();
API int      window_height();
API double   window_time();
API double   window_delta();

// API bool  window_hook(void (*func)(), void* userdata); // deprecated
// API void  window_unhook(void (*func)()); // deprecated

API void     window_focus(); // window attribute api using haz catz language for now
API int      window_has_focus();
API void     window_fullscreen(int enabled);
API int      window_has_fullscreen();
API void     window_cursor(int visible);
API int      window_has_cursor();
API void     window_pause(int paused);
API int      window_has_pause();
API void     window_visible(int visible);
API int      window_has_visible();
API void     window_maximize(int enabled);
API int      window_has_maximize();
API void     window_transparent(int enabled);
API int      window_has_transparent();
API void     window_icon(const char *file_icon);
API int      window_has_icon();

API double   window_aspect();
API void     window_aspect_lock(unsigned numer, unsigned denom);
API void     window_aspect_unlock();

API double   window_fps();
API double   window_fps_target();
API void     window_fps_lock(float fps);
API void     window_fps_unlock();

API void     window_screenshot(const char* outfile_png); // , bool record_cursor
API int      window_record(const char *outfile_mp4); // , bool record_cursor

API vec2     window_dpi();

enum CURSOR_SHAPES {
    CURSOR_NONE,
    CURSOR_HW_ARROW,  // default
    CURSOR_HW_IBEAM,  // i-beam text cursor
    CURSOR_HW_HDRAG,  // horizontal drag/resize
    CURSOR_HW_VDRAG,  // vertical drag/resize
    CURSOR_HW_HAND,   // hand, clickable
    CURSOR_HW_CROSS,  // crosshair
    CURSOR_SW_AUTO,   // software cursor, ui driven. note: this is the only icon that may be recorded or snapshotted
};

API void     window_cursor_shape(unsigned shape);

API const char *window_clipboard();
API void        window_setclipboard(const char *text);
#line 0

// ----

#line 1 "fwk_editor.h"
// -----------------------------------------------------------------------------
// in-game editor
// - rlyeh, public domain.

#define EDITOR_VERSION "2023.10"

// ----------------------------------------------------------------------------
// editor bindings

typedef struct editor_bind_t {
    const char *command;
    const char *bindings;
    void (*fn)();
} editor_bind_t;

API void editor_addbind(editor_bind_t bind);

#define EDITOR_BIND(CMD,KEYS,...) \
    void macro(editor_bind_##CMD##_fn_)() { __VA_ARGS__ }; AUTORUN { array_push(editor_binds, ((editor_bind_t){#CMD,KEYS,macro(editor_bind_##CMD##_fn_)}) ); }

// ----------------------------------------------------------------------------
// editor properties

#define EDITOR_PROPERTYDEF(T,property_name) \
    typedef map(void*,T) editor_##property_name##_map_t; \
API editor_##property_name##_map_t *editor_##property_name##_map(); \
API T editor_##property_name(const void *obj); \
API void editor_set##property_name(const void *obj, T value); \
API void editor_alt##property_name(const void *obj); \
API void editor_no##property_name(void *obj);

EDITOR_PROPERTYDEF(int,  open);     ///- whether object is tree opened in tree editor
EDITOR_PROPERTYDEF(int,  selected); ///- whether object is displaying a contextual popup or not
EDITOR_PROPERTYDEF(int,  changed);  ///- whether object is displaying a contextual popup or not
EDITOR_PROPERTYDEF(int,  popup);    ///- whether object is displaying a contextual popup or not
EDITOR_PROPERTYDEF(int,  bookmarked); ///-
EDITOR_PROPERTYDEF(int,  visible); ///-
EDITOR_PROPERTYDEF(int,  script); ///-
EDITOR_PROPERTYDEF(int,  event); ///-
EDITOR_PROPERTYDEF(char*,iconinstance); ///-
EDITOR_PROPERTYDEF(char*,iconclass); ///-
EDITOR_PROPERTYDEF(int,  treeoffsety); ///-

API void editor_destroy_properties(void *o);
API void editor_load_on_boot(void);
API void editor_save_on_quit(void);

// ----------------------------------------------------------------------------
// editor ui

enum EDITOR_MODE {
    EDITOR_PANEL,
    EDITOR_WINDOW,
    EDITOR_WINDOW_NK,
    EDITOR_WINDOW_NK_SMALL,
};

API int editor_begin(const char *title, int mode);
API int editor_end(int mode);

// ----------------------------------------------------------------------------------------
// editor selection

API int editor_filter();
API void editor_select(const char *mask);
API void editor_unselect(); // same than editor_select("!**");

API void editor_select_aabb(aabb box);
API void editor_selectgroup(obj *first, obj *last);
API void* editor_first_selected();
API void* editor_last_selected();

// ----------------------------------------------------------------------------------------
// editor instancing

API void editor_addtoworld(obj *o);
API void editor_watch(const void *o);
API void* editor_spawn(const char *ini); // deprecate?
API void editor_spawn1();

API void editor_destroy_selected();
API void editor_inspect(obj *o);

// ----------------------------------------------------------------------------------------
// editor utils

//API void  editor();
//API bool  editor_active();
API vec3   editor_pick(float mouse_x, float mouse_y);
API char*  editor_path(const char *path);

API void editor_setmouse(int x, int y);
API vec2 editor_glyph(int x, int y, const char *style, unsigned codepoint);
API vec2 editor_glyphs(int x, int y, const char *style, const char *utf8);
API void editor_gizmos(int dim);

// ----------------------------------------------------------------------------------------
// editor loop

API int         editor_send(const char *cmd); // returns job-id
API const char* editor_recv(int jobid, double timeout_ss);

API void editor_pump();
API void editor_frame( void (*game)(unsigned, float, double) );

// ----------------------------------------------------------------------------------------
// engine section: @todo: expand me

API float* engine_getf(const char *key);
API int*   engine_geti(const char *key);
API char** engine_gets(const char *key);
API int    engine_send(const char *cmd, const char *optional_value);

API int    ui_engine();
#line 0

// ----

#if is(cpp)
} // extern "C"
#endif

// expose glfw/glad apis
#if is(ems)
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #define gladLoadGL(func) (glewExperimental = true, glewInit() == GLEW_OK) ///-
#else
    #if is(win32) /*&& is(tcc)*/ // && ENABLE_DLL
    #ifdef GLAD_API_CALL
    #undef GLAD_API_CALL
    #endif
    #define GLAD_API_CALL extern API ///-
    #endif
    #ifndef GLAD_GL_H_
    #include "fwk"
    #endif
#endif

#endif // FWK_H
