// -----------------------------------------------------------------------------
// config directives

#ifndef ENABLE_FASTCALL_LUA
#define ENABLE_FASTCALL_LUA     1 ///+
#endif

#ifndef ENABLE_PROFILER
#define ENABLE_PROFILER         ifdef(debug, 1, 0)  ///+
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
// @todo: autorun (needed?)

#define countof(x)       (int)(sizeof (x) / sizeof 0[x])

#define concat(a,b)      conc4t(a,b)
#define conc4t(a,b)      a##b

#define macro(name)      concat(name, __LINE__)
#define defer(begin,end) for(int macro(i) = ((begin), 0); !macro(i); macro(i) = ((end), 1))
#define scope(end)       defer((void)0, end)
#define benchmark        for(double macro(t) = -time_ss(); macro(t) < 0; printf("%.2fs (" FILELINE ")\n", macro(t)+=time_ss()))
#define do_once          static int macro(once) = 0; for(;!macro(once);macro(once)=1)

#if is(cl)
#define __thread         __declspec(thread)
#elif is(tcc) && is(win32)
#define __thread         __declspec(thread) // compiles fine, but does not work apparently
#elif is(tcc)
#define __thread
#endif

// usage: bool static(audio_is_init) = audio_init();
//#define static(var)    static var; do_once var

//-----------------------------------------------------------------------------
// new C macros

#define ASSERT(expr, ...)          do { int fool_msvc[] = {0,}; if(!(expr)) { fool_msvc[0]++; breakpoint(va("!Expression failed: " #expr " " FILELINE "\n" __VA_ARGS__)); } } while(0)
#define ASSERT_ONCE(expr, ...)     do { int fool_msvc[] = {0,}; if(!(expr)) { fool_msvc[0]++; static int seen = 0; if(!seen) seen = 1, breakpoint(va("!Expression failed: " #expr " " FILELINE "\n" __VA_ARGS__)); } } while(0)
#define STATIC_ASSERT(EXPR)        typedef struct { unsigned macro(static_assert_on_line_) : !!(EXPR); } macro(static_assert_on_line_)

#define FILELINE                   __FILE__ ":" STRINGIZE(__LINE__)
#define STRINGIZE(x)               STRINGIZ3(x)
#define STRINGIZ3(x)               #x

#define EXPAND(name, ...)          EXPAND_QUOTE(EXPAND_JOIN(name, EXPAND_COUNT_ARGS(__VA_ARGS__)), (__VA_ARGS__))
#define EXPAND_QUOTE(x, y)         x y
#define EXPAND_JOIN(name, count)   EXPAND_J0IN(name, count)
#define EXPAND_J0IN(name, count)   EXPAND_J01N(name, count)
#define EXPAND_J01N(name, count)   name##count
#define EXPAND_COUNT_ARGS(...)     EXPAND_ARGS((__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define EXPAND_ARGS(args)          EXPAND_RETURN_COUNT args
#define EXPAND_RETURN_COUNT(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, count, ...) count

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
#if is(cpp)
#define C_CAST(type, ...)  ( type { __VA_ARGS__ } )
#else
#define C_CAST(type, ...)  ((type){ __VA_ARGS__ } )
#endif

// -----------------------------------------------------------------------------
// visibility

// win32 users would need to -DAPI=IMPORT/EXPORT as needed when using/building FWK as DLL.

#if is(win32)
#define IMPORT ifdef(gcc, __attribute__ ((dllimport)), __declspec(dllimport))
#define EXPORT ifdef(gcc, __attribute__ ((dllexport)), __declspec(dllexport))
#define STATIC
#else
#define IMPORT    
#define EXPORT    
#define STATIC
#endif

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
