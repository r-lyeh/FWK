//
// Copyright 2018 Sepehr Taghdisian (septag@github). All rights reserved.
// License: https://github.com/septag/sx#license-bsd-2-clause
//
#pragma once

// indicates that the library is a shared-dll
// If you are using the shared sx library, you should define SX_CONFIG_SHARED_LIB=1
// If you are building the sx library itself, there is no need to modify this, it must be defined by
// SX_SHARED_LIB cmake option
#ifndef SX_CONFIG_SHARED_LIB
#   define SX_CONFIG_SHARED_LIB 0
#endif

// Debug is forced to be off, so we undef _DEBUG if it's already defined
#if defined(SX_DEBUG) && !SX_DEBUG
#    ifdef _DEBUG
#        undef _DEBUG
#    endif
#endif

#if defined(_DEBUG) || (defined(SX_DEBUG) && SX_DEBUG)
#    ifndef SX_CONFIG_DEBUG_ALLOCATOR
#        define SX_CONFIG_DEBUG_ALLOCATOR 1
#    endif

// There is an issue with msvc+clang_c2 where NDEBUG (and some other release flags) are always
// defined
#    ifdef NDEBUG
#        undef NDEBUG
#    endif

#    ifndef SX_DEBUG
#        define SX_DEBUG 1
#    endif
#endif

#ifndef SX_DEBUG
#    define SX_DEBUG 0
#endif

#ifndef SX_CONFIG_DEBUG_ALLOCATOR
#    define SX_CONFIG_DEBUG_ALLOCATOR 0
#endif

// Natural aligment is the default memory alignment for each platform
// All memory allocators aligns pointers to this value if 'align' parameter is less than natural
// alignment.
// MacOS/iOS devices seems to be 16-byte aligned by default: 
// https://developer.apple.com/library/archive/documentation/Performance/Conceptual/ManagingMemory/Articles/MemoryAlloc.html
#ifndef SX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT
#    if defined(__APPLE__) && defined(__MACH__)
#        define SX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT 16
#    else
#        define SX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT 8
#    endif
#endif    // BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT

// Inserts code for hash-table debugging, used only for efficiency tests, see hash.h
#ifndef SX_CONFIG_HASHTBL_DEBUG
#    define SX_CONFIG_HASHTBL_DEBUG 1
#endif

// Use stdc math lib for basic math functions, see math.h
#ifndef SX_CONFIG_STDMATH
#    define SX_CONFIG_STDMATH 1
#endif

#ifndef SX_CONFIG_HANDLE_GEN_BITS
#   define SX_CONFIG_HANDLE_GEN_BITS 14
#endif

#ifndef SX_CONFIG_SIMD_DISABLE
#   define SX_CONFIG_SIMD_DISABLE 0
#endif

#if defined(_MSC_VER) && 0
// Macros for stdint.h definitions
// There are some problems with intellisense+gcc and I had to define these (only works in editor,
// the compiler defines them by default)
// TODO: make some of them with cmake --config
#    ifndef __INT32_TYPE__
#        define __INT32_TYPE__ int
#    endif

#    ifndef __UINT32_TYPE__
#        define __UINT32_TYPE__ unsigned int
#    endif

#    ifndef __INT64_TYPE__
#        ifdef _MSC_VER
#            define __INT64_TYPE__ __int64
#        else
#            define __INT64_TYPE__ long long
#        endif
#    endif

#    ifndef __UINT64_TYPE__
#        ifdef _MSC_VER
#            define __UINT64_TYPE__ unsigned __int64
#        else
#            define __UINT64_TYPE__ unsigned long long
#        endif
#    endif

#    ifndef __INT8_TYPE__
#        define __INT8_TYPE__ char
#    endif

#    ifndef __UINT8_TYPE__
#        define __UINT8_TYPE__ unsigned char
#    endif

#    ifndef __INT16_TYPE__
#        define __INT16_TYPE__ short
#    endif

#    ifndef __UINT16_TYPE__
#        define __UINT16_TYPE__ unsigned short
#    endif

#    ifndef __INTPTR_WIDTH__
#        if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || \
            defined(__64BIT__) || defined(__LP64__)
#            define uintptr_t uint64_t
#            define intptr_t int64_t
#        else
#            define uintptr_t uint32_t
#            define intptr_t int32_t
#        endif
#    endif

#    ifndef __INT_MAX__
#        define __INT_MAX__ 2147483647
#    endif

#    ifndef __LONG_MAX__
#        if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || \
            defined(__64BIT__) || defined(__LP64__)
#            define __LONG_MAX__ 9223372036854775807L
#        else
#            define __LONG_MAX__ 2147483647L
#        endif
#    endif
#endif
