//
// Copyright 2018 Sepehr Taghdisian (septag@github). All rights reserved.
// License: https://github.com/septag/sx#license-bsd-2-clause
//
// parts of this code is copied from bx library: https://github.com/bkaradzic/bx
// Copyright 2011-2019 Branimir Karadzic. All rights reserved.
// License: https://github.com/bkaradzic/bx#license-bsd-2-clause
//
// allocator.h - v1.0 - Memory allocation routines and helper functions
//      sx_alloc: main allocation primitive, all memory allocation function must accept this object
//      instead of malloc/free
//                Custom implementaion is only one 'alloc_cb' function (see sx_alloc), and a
//                user_data that is to store any custom data for user allocators. 'alloc_cb'
//                provides all three main allocation function inside of itself which is
//                malloc/realloc/free Here's an example minimal custom implementation:
//                      void my_alloc(void* ptr, size_t size, uint32_t align, const char* file,
//                      uint32_t line, void* user_data) {
//                          // 'file' can be NULL in none-debug builds
//                          // if 'align' is zero or less than
//                          SX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT, no alignment is needed if (size
//                          == 0) { free(ptr);  return NULL; } else if (ptr == NULL)   { return
//                          malloc(size); }    // TODO: handle alignment cases else { return
//                          realloc(ptr, size); }    // TODO: handle alignment cases
//                      }
//
//     sx_alloc, sx_realloc, sx_free, sx_aligned_alloc, sx_aligned_realloc, sx_aligned_free:
//          These macros provide helper to allocation functions, they all require a valid allocator
//          They also skip passing __FILE__ and __LINE__ if SX_CONFIG_DEBUG_ALLOCATOR=0s
//
//     sx_is_aligned: checks if pointer is aligned to specified alignment
//     sx_align_ptr:  offsets the pointer forward to aligned address with 'extra' bytes reserved
//     sx_alloc_malloc: normal heap (malloc/free) allocator
//     sx_alloc_malloc_leak_detect: normal heap (malloc/free) allocator but with extra leak
//                                  detection use sx_dump_leaks and the end of the program to dump
//                                  possible memory leaks
//
#pragma once

#include "sx.h"

// Define anything here, for out of memory exceptions (exit/assert/...)
#ifndef sx_out_of_memory
#    include <stdlib.h>
#    define sx_out_of_memory()                    \
        do {                                      \
            sx_assert_rel(0 && "Out of memory!"); \
            exit(-1);                             \
        } while (0)
#endif

#if SX_CONFIG_DEBUG_ALLOCATOR
#    define sx_malloc(_allocator, _size) \
        sx__malloc(_allocator, _size, 0, __FILE__, __FUNCTION__, __LINE__)
#    define sx_realloc(_allocator, _ptr, _size) \
        sx__realloc(_allocator, _ptr, _size, 0, __FILE__, __FUNCTION__, __LINE__)
#    define sx_free(_allocator, _ptr) \
        sx__free(_allocator, _ptr, 0, __FILE__, __FUNCTION__, __LINE__)
#    define sx_aligned_malloc(_allocator, _size, _align) \
        sx__malloc(_allocator, _size, _align, __FILE__, __FUNCTION__, __LINE__)
#    define sx_aligned_realloc(_allocator, _ptr, _size, _align) \
        sx__realloc(_allocator, _ptr, _size, _align, __FILE__, __FUNCTION__, __LINE__)
#    define sx_aligned_free(_allocator, _ptr, _align) \
        sx__free(_allocator, _ptr, _align, __FILE__, __FUNCTION__, __LINE__)
#else
#    define sx_malloc(_allocator, _size) sx__malloc(_allocator, _size, 0, NULL, NULL, 0)
#    define sx_realloc(_allocator, _ptr, _size) \
        sx__realloc(_allocator, _ptr, _size, 0, NULL, NULL, 0)
#    define sx_free(_allocator, _ptr) sx__free(_allocator, _ptr, 0, NULL, NULL, 0)
#    define sx_aligned_malloc(_allocator, _size, _align) \
        sx__malloc(_allocator, _size, _align, NULL, NULL, 0)
#    define sx_aligned_realloc(_allocator, _ptr, _size, _align) \
        sx__realloc(_allocator, _ptr, _size, _align, NULL, NULL, 0)
#    define sx_aligned_free(_allocator, _ptr, _align) \
        sx__free(_allocator, _ptr, _align, NULL, NULL, 0)
#endif    // SX_CONFIG_ALLOCATOR_DEBUG

typedef struct sx_alloc {
    void* (*alloc_cb)(void* ptr, size_t size, uint32_t align, const char* file, const char* func,
                      uint32_t line, void* user_data);
    void* user_data;
} sx_alloc;

// Default allocator: allocate from heap
SX_API const sx_alloc* sx_alloc_malloc();

// Leak checking allocator, useful for debug and SX_CONFIG_DEBUG_ALLOCATOR=1
// sx_alloc* sx_alloc_malloc_leak_detect();
SX_API const sx_alloc* sx_alloc_malloc_leak_detect();

typedef void (*sx_dump_leak_cb)(const char* formatted_msg, const char* file, const char* func,
                                int line, size_t size, void* ptr);
SX_API void sx_dump_leaks(sx_dump_leak_cb dump_leak_fn);

static inline bool sx_is_aligned(const void* ptr, uint32_t align) {
    union {
        const void* ptr;
        uintptr_t   addr;
    } un;
    un.ptr = ptr;
    return 0 == (un.addr & (align - 1));
}

static inline void* sx_align_ptr(void* ptr, size_t extra, uint32_t align) {
    union {
        void*     ptr;
        uintptr_t addr;
    } un;
    un.ptr = ptr;
    uintptr_t unaligned = un.addr + extra;    // space for header
    uintptr_t mask = align - 1;
    uintptr_t aligned = sx_align_mask(unaligned, mask);
    un.addr = aligned;
    return un.ptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal
static inline void* sx__malloc(const sx_alloc* alloc, size_t size, uint32_t align, const char* file,
                               const char* func, uint32_t line) {
    return alloc->alloc_cb(NULL, size, align, file, func, line, alloc->user_data);
}

static inline void sx__free(const sx_alloc* alloc, void* ptr, uint32_t align, const char* file,
                            const char* func, uint32_t line) {
    alloc->alloc_cb(ptr, 0, align, file, func, line, alloc->user_data);
}

static inline void* sx__realloc(const sx_alloc* alloc, void* ptr, size_t size, uint32_t align,
                                const char* file, const char* func, uint32_t line) {
    return alloc->alloc_cb(ptr, size, align, file, func, line, alloc->user_data);
}

static inline void* sx__aligned_alloc(const sx_alloc* alloc, size_t size, uint32_t align,
                                      const char* file, const char* func, uint32_t line) {
    align = sx_max((int)align, SX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT);
    const size_t total = size + align + sizeof(uint32_t);
    uint8_t*     ptr = (uint8_t*)sx__malloc(alloc, total, 0, file, func, line);
    sx_assert(ptr);
    uint8_t*  aligned = (uint8_t*)sx_align_ptr(ptr, sizeof(uint32_t), align);
    uint32_t* header = (uint32_t*)aligned - 1;
    *header = (uint32_t)(uintptr_t)(aligned - ptr);
    return aligned;
}

static inline void sx__aligned_free(const sx_alloc* alloc, void* ptr,
                                    const char* file, const char* func, uint32_t line) {
    uint8_t*  aligned = (uint8_t*)ptr;
    uint32_t* header = (uint32_t*)aligned - 1;
    ptr = aligned - *header;
    sx__free(alloc, ptr, 0, file, func, line);
}

static inline void* sx__aligned_realloc(const sx_alloc* alloc, void* ptr, size_t size,
                                        uint32_t align, const char* file, const char* func,
                                        uint32_t line) {
    if (ptr == NULL)
        return sx__aligned_alloc(alloc, size, align, file, func, line);

    uint8_t* aligned = (uint8_t*)ptr;
    uint32_t offset = *((uint32_t*)aligned - 1);
    ptr = aligned - offset;

    align = sx_max((int)align, SX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT);
    const size_t total = size + align + sizeof(uint32_t);
    ptr = sx__realloc(alloc, ptr, total, 0, file, func, line);
    sx_assert(ptr);
    uint8_t* new_aligned = (uint8_t*)sx_align_ptr(ptr, sizeof(uint32_t), align);

    if (new_aligned == aligned)
        return aligned;

    aligned = (uint8_t*)ptr + offset;
    sx_memmove(new_aligned, aligned, size);
    uint32_t* header = (uint32_t*)new_aligned - 1;
    *header = (uint32_t)(new_aligned - (uint8_t*)ptr);
    return new_aligned;
}
