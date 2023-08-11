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
* ALTERNATIVE B - MIT-0 (No Attribution clause)
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
* ------------------------------------------------------------------------------
* ALTERNATIVE C - Zero BSD License (https://opensource.org/licenses/FPL-1.0.0)
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
* license terms listed above. I make this dedication for the benefit of the
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
#include "fwk.h"
#endif

#ifndef FWK_3RD
#define FWK_3RD
#include "fwk"
#endif

//-----------------------------------------------------------------------------
// C files

#if is(win32) // hack to boost exit time. there are no critical systems that need to shutdown properly on Windows,
#define atexit(...) // however Linux needs proper atexit() to deinitialize 3rd_miniaudio.h; likely OSX as well.
#endif

#line 1 "fwk_ds.c"

// -----------------------------------------------------------------------------
// sort/less

int sort_64(const void *a, const void *b) {
    return 0[(uint64_t*)a] - 0[(uint64_t*)b];
}

int less_int(int a, int b) {
    return a - b;
}
int less_64(uint64_t a, uint64_t b) {
    return a > b ? +1 : -!!(a - b);
}
int less_ptr(void *a, void *b) {
    return (uintptr_t)a > (uintptr_t)b ? +1 : -!!((uintptr_t)a - (uintptr_t)b);
}
int less_str(char *a, char *b) {
    return strcmp((const char *)a, (const char *)b);
}

// -----------------------------------------------------------------------------
// un/hash

uint32_t unhash_32(uint32_t x) {
    // Thomas Mueller at https://stackoverflow.com/questions/664014/ - says no collisions for 32bits!
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = (x >> 16) ^ x;
    return x;
}
uint32_t hash_32(uint32_t x) {
    // Thomas Mueller at https://stackoverflow.com/questions/664014/ - says no collisions for 32bits!
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}
uint64_t hash_64(uint64_t x) {
#if 1
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
#else
    // should we just use x2 hash_32?
    uint32_t hi = (x >> 32ull), lo = (x & ~0u);
    return (hash_32(hi) << 32ull) | hash_32(lo);
#endif
}
uint64_t hash_flt(double x) {
    union { double d; uint64_t i; } c;
    return c.d = x, hash_64(c.i);
}
uint64_t hash_str(const char* str) {
    uint64_t hash = 14695981039346656037ULL; // hash(0),mul(131) faster than fnv1a, a few more collisions though
    while( *str ) hash = ( (unsigned char)*str++ ^ hash ) * 0x100000001b3ULL;
    return hash;
}
uint64_t hash_bin(const void* ptr, unsigned len) {
    uint64_t hash = 14695981039346656037ULL; // hash(0),mul(131) faster than fnv1a, a few more collisions though
    for( unsigned char *str = (unsigned char *)ptr; len--; )
    hash = ( (unsigned char)*str++ ^ hash ) * 0x100000001b3ULL;
    return hash;
}
uint64_t hash_int(int key) {
    return hash_32((uint32_t)key);
}
uint64_t hash_ptr(const void *ptr) {
    uint64_t key = (uint64_t)(uintptr_t)ptr;
    return hash_64(key); // >> 3? needed?
}

// -----------------------------------------------------------------------------
// utils

uint64_t popcnt64(uint64_t x) {
    // [src] https://en.wikipedia.org/wiki/Hamming_weight
    x -= (x >> 1) & 0x5555555555555555ULL;                                //put count of each 2 bits into those 2 bits
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL); //put count of each 4 bits into those 4 bits
    x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;                           //put count of each 8 bits into those 8 bits
    return (x * 0x0101010101010101ULL) >> 56;                             //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...
}

// -----------------------------------------------------------------------------
// vector based allocator (x1.75 enlarge factor)

void* vrealloc( void* p, size_t sz ) {
    if( !sz ) {
        if( p ) {
            size_t *ret = (size_t*)p - 2;
            ret[0] = 0;
            ret[1] = 0;
            REALLOC( ret, 0 );
        }
        return 0;
    } else {
        size_t *ret;
        if( !p ) {
            ret = (size_t*)REALLOC( 0, sizeof(size_t) * 2 + sz );
            ret[0] = sz;
            ret[1] = 0;
        } else {
            ret = (size_t*)p - 2;
            size_t osz = ret[0];
            size_t ocp = ret[1];
            if( sz <= (osz + ocp) ) {
                ret[0] = sz;
                ret[1] = ocp - (sz - osz);
            } else {
                ret = (size_t*)REALLOC( ret, sizeof(size_t) * 2 + sz * 1.75 );
                ret[0] = sz;
                ret[1] = (size_t)(sz * 1.75) - sz;
            }
        }
        return &ret[2];
    }
}
size_t vlen( void* p ) {
    return p ? 0[ (size_t*)p - 2 ] : 0;
}

// -----------------------------------------------------------------------------

enum { MAP_GC_SLOT = MAP_HASHSIZE };
typedef int map_is_pow2_assert[ !(MAP_HASHSIZE & (MAP_HASHSIZE - 1)) ? 1:-1];

static int map_get_index(uint64_t hkey1) {
    return hkey1 & (MAP_HASHSIZE-1);
}

void (map_init)(map* m) {
    map c = {0};
    *m = c;

    array_resize(m->array, (MAP_HASHSIZE+1));
    memset(m->array, 0, (MAP_HASHSIZE+1) * sizeof(m->array[0]) ); // array_resize() just did memset()
}

void (map_insert)(map* m, pair *p, void *key, void *value, uint64_t keyhash, void *super) {
    p->keyhash = keyhash;
    p->key = key;
    p->value = value;
    p->super = super;

    /* Insert onto the beginning of the list */
    int index = map_get_index(p->keyhash);
    p->next = m->array[index];
    m->array[index] = p;
    m->is_sorted = 0;

    ++m->count;
}

void* (map_find)(map* m, void *key, uint64_t keyhash) {
    int index = map_get_index(keyhash);
    for( pair *cur = m->array[index]; cur; cur = cur->next ) {
        if( cur->keyhash == keyhash ) {
            char **c = (char **)cur->key;
            char **k = (char **)key;
            if( !m->cmp(c[0], k[0]) ) {
                return cur->super;
            }
        }
    }
    return 0;
}

void (map_erase)(map* m, void *key, uint64_t keyhash) {
    int index = map_get_index(keyhash);
    for( pair *prev = 0, *cur = m->array[index]; cur; (prev = cur), (cur = cur->next) ) {
        if( cur->keyhash == keyhash ) {
            char **c = (char **)cur->key;
            char **k = (char **)key;
            if( !m->cmp(c[0], k[0]) ) {
                if( prev ) prev->next = cur->next; else m->array[index] = cur->next ? cur->next : 0;
#if MAP_DONT_ERASE
                /* Insert onto the beginning of the GC list */
                cur->next = m->array[MAP_GC_SLOT];
                m->array[MAP_GC_SLOT] = cur;
#else
                REALLOC(cur,0);
#endif
                --m->count;
                m->is_sorted = 0;
                return;
            }
        }
    }
}

int (map_count)(map* m) { // clean deferred GC_SLOT only
    return m->count;

    int counter = 0;
    for( int i = 0; i < MAP_HASHSIZE; ++i) {
        for( pair *cur = m->array[i]; cur; cur = cur->next ) {
            ++counter;
        }
    }
    return counter;
}

int (map_isempty)(map* m) { // clean deferred GC_SLOT only
    return !m->count;
}

void (map_gc)(map* m) { // clean deferred GC_SLOT only
#if MAP_DONT_ERASE
    for( pair *next, *cur = m->array[MAP_GC_SLOT]; cur; cur = next ) {
        next = cur->next;
        REALLOC(cur,0);
    }
    m->array[MAP_GC_SLOT] = 0;
#endif
}

void (map_clear)(map* m) {
    for( int i = 0; i <= MAP_HASHSIZE; ++i) {
        for( pair *next, *cur = m->array[i]; cur; cur = next ) {
            next = cur->next;
            REALLOC(cur,0);
        }
        m->array[i] = 0;
    }
    m->count = 0;
    m->is_sorted = 0;
}

bool (map_sort)(map* m) {
    if( m->is_sorted ) return false;

    array_clear(m->sorted);
//    array_reserve(m->sorted, m->count);

    for( int i = 0; i < array_count(m->array); ++i) {
        for( pair *cur = m->array[i]; cur; cur = cur->next ) {
            array_push(m->sorted, cur);
        }
    }

#if 0
    array_sort(m->sorted, m->cmp);
#else
    // @fixme: do better than bubble sort below
    for( int i = 0; i < array_count(m->sorted) - 1; ++i)
    for( int j = i+1; j < array_count(m->sorted); ++j) {
        pair *curi = m->sorted[i];
        pair *curj = m->sorted[j];
        char **c = (char **)curi->key;
        char **k = (char **)curj->key;
        if( m->cmp(c[0], k[0]) > 0 ) {
            pair *swap = m->sorted[i];
            m->sorted[i] = m->sorted[j];
            m->sorted[j] = swap;
        }
    }
#endif

    return m->is_sorted = true;
}

void (map_free)(map* m) {
    (map_clear)(m);

    array_free(m->array);
    m->array = 0;

    map c = {0};
    *m = c;
}

// -----------------------------------------------------------------------------

enum { set_GC_SLOT = SET_HASHSIZE };
typedef int set_is_pow2_assert[ !(SET_HASHSIZE & (SET_HASHSIZE - 1)) ? 1:-1];

static int set_get_index(uint64_t hkey1) {
    return hkey1 & (SET_HASHSIZE-1);
}

void (set_init)(set* m) {
    set zero = {0};
    *m = zero;

    array_resize(m->array, (SET_HASHSIZE+1));
    memset(m->array, 0, (SET_HASHSIZE+1) * sizeof(m->array[0]) ); // array_resize() just did memset()
}

void (set_insert)(set* m, set_item *p, void *key, uint64_t keyhash, void *super) {
    p->keyhash = keyhash;
    p->key = key;
    p->super = super;

    /* Insert onto the beginning of the list */
    int index = set_get_index(p->keyhash);
    p->next = m->array[index];
    m->array[index] = p;

    ++m->count;
}

void* (set_find)(const set* m, void *key, uint64_t keyhash) {
    int index = set_get_index(keyhash);
    for( const set_item *cur = m->array[index]; cur; cur = cur->next ) {
        if( cur->keyhash == keyhash ) {
            char **c = (char **)cur->key;
            char **k = (char **)key;
            if( !m->cmp(c[0], k[0]) ) {
                return cur->super;
            }
        }
    }
    return 0;
}

void (set_erase)(set* m, void *key, uint64_t keyhash) {
    int index = set_get_index(keyhash);
    for( set_item *prev = 0, *cur = m->array[index]; cur; (prev = cur), (cur = cur->next) ) {
        if( cur->keyhash == keyhash ) {
            char **c = (char **)cur->key;
            char **k = (char **)key;
            if( !m->cmp(c[0], k[0]) ) {
                if (prev) prev->next = cur->next; else m->array[index] = cur->next ? cur->next : 0;
#if SET_DONT_ERASE
                /* Insert onto the beginning of the GC list */
                cur->next = m->array[set_GC_SLOT];
                m->array[set_GC_SLOT] = cur;
#else
                REALLOC(cur,0);
#endif
                --m->count;
                return;
            }
        }
    }
}

int (set_count)(const set* m) { // does not include GC_SLOT
    return m->count;

    int counter = 0;
    for( int i = 0; i < SET_HASHSIZE; ++i) {
        for( const set_item *cur = m->array[i]; cur; cur = cur->next ) {
            ++counter;
        }
    }
    return counter;
}

int (set_isempty)(const set *m) { // clean deferred GC_SLOT only
    return !m->count;
}

void (set_gc)(set* m) { // clean deferred GC_SLOT only
#if SET_DONT_ERASE
    for( set_item *next, *cur = m->array[set_GC_SLOT]; cur; cur = next ) {
        next = cur->next;
        REALLOC(cur,0);
    }
    m->array[set_GC_SLOT] = 0;
#endif
}

void (set_clear)(set* m) { // include GC_SLOT
    for( int i = 0; i <= SET_HASHSIZE; ++i) {
        for( set_item *next, *cur = m->array[i]; cur; cur = next ) {
            next = cur->next;
            REALLOC(cur,0);
        }
        m->array[i] = 0;
    }
    m->count = 0;
}

void (set_free)(set* m) {
    (set_clear)(m);

    array_free(m->array);
    m->array = 0;

    set zero = {0};
    *m = zero;
}

char *cc4str(unsigned x) {
    static __thread char type[4+1] = {0};
    type[3] = (x >> 24ULL) & 255;
    type[2] = (x >> 16ULL) & 255;
    type[1] = (x >>  8ULL) & 255;
    type[0] = (x >>  0ULL) & 255;
    return type;
}
char *cc8str(uint64_t x) {
    static __thread char type[8+1] = {0};
    type[7] = (x >> 56ULL) & 255;
    type[6] = (x >> 48ULL) & 255;
    type[5] = (x >> 40ULL) & 255;
    type[4] = (x >> 32ULL) & 255;
    type[3] = (x >> 24ULL) & 255;
    type[2] = (x >> 16ULL) & 255;
    type[1] = (x >>  8ULL) & 255;
    type[0] = (x >>  0ULL) & 255;
    return type;
}

#line 0

#line 1 "fwk_string.c"
#include <stdarg.h>

char* tempvl(const char *fmt, va_list vl) {
    va_list copy;
    va_copy(copy, vl);
    int sz = /*stbsp_*/vsnprintf( 0, 0, fmt, copy ) + 1;
    va_end(copy);

    int reqlen = sz;
#if 0
    enum { STACK_ALLOC = 16384 };
    static __thread char buf[STACK_ALLOC];
#else
    enum { STACK_ALLOC = 128*1024 };
    static __thread char *buf = 0; if(!buf) buf = REALLOC(0, STACK_ALLOC); // @leak
#endif
    static __thread int cur = 0, len = STACK_ALLOC - 1; //printf("string stack %d/%d\n", cur, STACK_ALLOC);

    assert(reqlen < STACK_ALLOC && "no stack enough, increase STACK_ALLOC variable above");
    char* ptr = buf + (cur *= (cur+reqlen) < len, (cur += reqlen) - reqlen);

    /*stbsp_*/vsnprintf( ptr, sz, fmt, vl );
    return (char *)ptr;
}
char* tempva(const char *fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    char *s = tempvl(fmt, vl);
    va_end(vl);
    return s;
}

char* (strcatf)(char **src_, const char *buf) {
    char *src = *src_;
        if(!buf) return src;
        // if(!buf[0]) return src;
        int srclen = (src ? strlen(src) : 0), buflen = strlen(buf);
        src = (char*)REALLOC(src, srclen + buflen + 1 );
        memcpy(src + srclen, buf, buflen + 1 );
    *src_ = src;
    return src;
}

// -----------------------------------------------------------------------------
// string utils

int strmatch(const char *s, const char *wildcard) {
    // returns true if wildcard matches
    if( *wildcard=='\0' ) return !*s;
    if( *wildcard=='*' )  return strmatch(s, wildcard+1) || (*s && strmatch(s+1, wildcard));
    if( *wildcard=='?' )  return *s && (*s != '.') && strmatch(s+1, wildcard+1);
    return (*s == *wildcard) && strmatch(s+1, wildcard+1);
}
int strmatchi(const char *s, const char *wildcard) {
    return strmatch(strlower(s), strlower(wildcard));
}

int strcmp_qsort(const void *a, const void *b) {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
}
int strcmpi_qsort(const void *a, const void *b) {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmpi(*ia, *ib);
}

bool strbeg(const char *a, const char *b) { // returns true if both strings match at beginning. case sensitive
    return strncmp(a, b, strlen(b)) ? false : true; // strstr(a,b) == a
}
bool strend(const char *a, const char *b) { // returns true if both strings match at end. case sensitive
    int la = strlen(a), lb = strlen(b);
    if( la < lb ) return false;
    return strncmp(a + la - lb, b, lb) ? false : true;
}
/*
int main() {
    printf("strbeg(abc abc) = %d\n", strbeg("abc", "abc"));
    printf("strbeg(abc abcd) = %d\n", strbeg("abc", "abcd"));
    printf("strbeg(abcd abc) = %d\n", strbeg("abcd", "abc"));
    printf("strbeg(abc (empty)) = %d\n", strbeg("abc", ""));
    printf("strbeg((empty) abc) = %d\n", strbeg("", "abc"));
    printf("strbeg(123 abcd) = %d\n", strbeg("123", "abcd"));
    printf("strbeg(abcd 123) = %d\n", strbeg("abcd", "123"));
    puts("---");
    printf("strend(abc abc) = %d\n", strend("abc", "abc"));
    printf("strend(abc 0abc) = %d\n", strend("abc", "0abc"));
    printf("strend(abcd bcd) = %d\n", strend("abcd", "bcd"));
    printf("strend(abc (empty)) = %d\n", strend("abc", ""));
    printf("strend((empty) abc) = %d\n", strend("", "abc"));
    printf("strend(123 abcd) = %d\n", strend("123", "abcd"));
    printf("strend(abcd 123) = %d\n", strend("abcd", "123"));
}
*/

bool strbegi(const char *a, const char *b) { // returns true if both strings match at beginning. case insensitive
    int la = strlen(a), lb = strlen(b);
    if( la < lb ) return 0;
    if( lb == 0 ) return 1;
    int len = la < lb ? la : lb;
    for( int i = 0; i < len; ++i ) {
        if( tolower(a[i]) != tolower(b[i]) ) {
            return false;
        }
    }
    return true;
}
bool strendi(const char *src, const char *sub) { // returns true if both strings match at end. case insensitive
    int srclen = strlen(src);
    int sublen = strlen(sub);
    if( sublen > srclen ) return 0;
    return !strcmpi(src + srclen - sublen, sub);
}


// Find substring in string, case insensitive. Alias for strcasestr()
// Returns first char of coincidence, or NULL.
const char *strstri( const char *src, const char *sub ){
    while( *src++ ) {
        for( const char *s = src-1, *f = sub, *c = s; ; ++f, ++c) {
            if(!*f) return s;
            if(!*c) return NULL;
            if(tolower(*c) != tolower(*f)) break;
        }
    }
    return NULL;
}

char *strupper(const char *str) {
    char *s = va("%s", str), *bak = s;
    while(*s++) s[-1] = toupper(s[-1]);
    return bak;
}
char *strlower(const char *str) {
    char *s = va("%s", str), *bak = s;
    while(*s++) s[-1] = tolower(s[-1]);
    return bak;
}

#ifndef __APPLE__ // BSD provides these
// Safely concatenate two strings. Always NUL terminates (unless dstcap == 0).
// Returns length of operation; if retval >= dstcap, truncation occurred.
size_t strlcat(char *dst, const char *src, size_t dstcap) {
    int dl = strlen(dst), sl = strlen(src);
    if( dstcap ) snprintf(dst + dl, dstcap - dl, "%s", src);
    return dl + sl;
}
// Safely copy two strings. Always NUL terminates (unless dstcap == 0).
// Copy src to string dst of size dstcap. Copies at most dstcap-1 characters.
// Returns length of input; if retval >= dstcap, truncation occurred.
size_t strlcpy(char *dst, const char *src, size_t dstcap) {
    int sl = strlen(src);
    if( dstcap ) snprintf(dst, dstcap, "%*s", sl, src);
    return sl;// count does not include NUL
}
#endif

char *string8(const wchar_t *str) { // from wchar16(win) to utf8/ascii
    int i = 0, n = wcslen(str) * 6 - 1;
    static __thread char error[1] = {0}, buffer[2048]; assert( n < 2048 );
    while( *str ) {
       if (*str < 0x80) {
          if (i+1 > n) return error;
          buffer[i++] = (char) *str++;
       } else if (*str < 0x800) {
          if (i+2 > n) return error;
          buffer[i++] = 0xc0 + (*str >> 6);
          buffer[i++] = 0x80 + (*str & 0x3f);
          str += 1;
       } else if (*str >= 0xd800 && *str < 0xdc00) {
          uint32_t c;
          if (i+4 > n) return error;
          c = ((str[0] - 0xd800) << 10) + ((str[1]) - 0xdc00) + 0x10000;
          buffer[i++] = 0xf0 + (c >> 18);
          buffer[i++] = 0x80 + ((c >> 12) & 0x3f);
          buffer[i++] = 0x80 + ((c >>  6) & 0x3f);
          buffer[i++] = 0x80 + ((c      ) & 0x3f);
          str += 2;
       } else if (*str >= 0xdc00 && *str < 0xe000) {
          return error;
       } else {
          if (i+3 > n) return error;
          buffer[i++] = 0xe0 + (*str >> 12);
          buffer[i++] = 0x80 + ((*str >> 6) & 0x3f);
          buffer[i++] = 0x80 + ((*str     ) & 0x3f);
          str += 1;
       }
    }
    buffer[i] = 0;
    return va("%s", buffer);
}

char *strrepl(char **string, const char *target, const char *replace) { // may reallocate input string if needed
    //if new text is shorter than old one,then no need to heap, replace inline
    int rlen = strlen(replace), tlen = strlen(target), diff = tlen - rlen;
    if( diff >= 0 ) return strswap(*string, target, replace);

    char *buf = 0, *aux = *string;
    for( int tgtlen = tlen; tgtlen && aux[0]; ) {
        char *found = strstr(aux, target);
        if( found ) {
            strcatf(&buf, "%.*s%s", (int)(found - aux), aux, replace);
            aux += (found - aux) + tgtlen;
        } else {
            strcatf(&buf, "%s", aux);
            break;
        }
    }
    if( buf ) {
        // strcpy(*string, buf);
        char *s = *string;
        if(s) s[0] = 0;
        strcatf(&s, "%s", buf);
        *string = s;

        FREE( buf );
    }
    return *string;
}
char *strswap(char *copy, const char *target, const char *replace) { // replaced only if new text is shorter than old one
    int rlen = strlen(replace), diff = strlen(target) - rlen;
    if( diff >= 0 ) {
        for( char *s = copy, *e = s + strlen(copy); /*s < e &&*/ 0 != (s = strstr(s, target)); ) {
            if( rlen ) s = (char*)memcpy( s, replace, rlen ) + rlen;
            if( diff ) memmove( s, s + diff, (e - (s + diff)) + 1 );
        }
    }
    return copy;
}
char *strcut(char *copy, const char *target) {
    return strswap(copy, target, "");
}

const char *strlerp(unsigned numpairs, const char **pairs, const char *str) { // using key-value pairs, null-terminated
    if( !pairs[0] ) {
        return str;
    }
    // find & replace all tokens; @fixme: optimize me
    char *buf = REALLOC(0, 128*1024); strcpy(buf, str);
    for( unsigned i = 0; i < numpairs; ++i ) {
        const char *token = pairs[i*2+0];
        const char *repl = pairs[i*2+1];
        while(strstr(buf, token)) {
            strrepl(&buf, token, repl);
        }
    }
    char *ret = va("%s", buf);
    FREE(buf);
    return ret;
}

array(char*) strsplit(const char *str, const char *separators) {
    static __thread int slot = 0;
    static __thread char *buf[16] = {0};
    static __thread array(char*) list[16] = {0};

    slot = (slot+1) % 16;
    array_resize(list[slot], 0);
    *(buf[slot] = REALLOC(buf[slot], strlen(str)*2+1)) = '\0'; // *2 to backup pathological case where input str is only separators && include == 1

    for(char *dst = buf[slot]; str && *str; ) {
        // count literal run && terminators
        int run = strcspn(str, separators);
        int end = strspn(str + run, separators);

        // append literal run
        if( run ) {
            array_push(list[slot], dst);
            memmove(dst,str,run); dst[run] = '\0'; //strncpy(dst, str, run)
            dst += run + 1;
        }
#if defined SPLIT_INCLUDE_SEPARATORS && SPLIT_INCLUDE_SEPARATORS
        // mode: append all separators: "1++2" -> "1" "+" "+" "2"
        for( int i = 0; i < end; ++i ) {
            array_push(list[slot], dst);
            dst[0] = str[ run + i ];
            dst[1] = '\0';
            dst += 2;
        }
#endif

        // skip both
        str += run + end;
    }

    return list[slot];
}
char* strjoin(array(char*) list, const char *separator) {
    static __thread int slot = 0;
    static __thread char* mems[16] = {0};

    slot = (slot+1) % 16;

    int num_list = array_count(list);
    int len = 0, inc = 0, seplen = strlen(separator);
    for( int i = 0; (num_list > 0 ? i < num_list : !!list[i]); ++i ) {
        len += strlen(list[i]) + inc;
        inc = seplen;
    }

    mems[slot] = REALLOC(mems[slot], len+1);
    char *p = mems[slot]; *p = 0;
    const char *sep = "";
    for( int i = 0; (num_list > 0 ? i < num_list : !!list[i]); ++i ) {
        p += sprintf(p, "%s%s", sep, list[i]);
        sep = separator;
    }

    return mems[slot];
}

static
const char *extract_utf32(const char *s, uint32_t *out) {
    /**/ if( (s[0] & 0x80) == 0x00 ) return *out = (s[0]), s + 1;
    else if( (s[0] & 0xe0) == 0xc0 ) return *out = (s[0] & 31) <<  6 | (s[1] & 63), s + 2;
    else if( (s[0] & 0xf0) == 0xe0 ) return *out = (s[0] & 15) << 12 | (s[1] & 63) <<  6 | (s[2] & 63), s + 3;
    else if( (s[0] & 0xf8) != 0xf0 ) return *out = (s[0] &  7) << 18 | (s[1] & 63) << 12 | (s[2] & 63) << 8 | (s[3] & 63), s + 4;
    return *out = 0, s + 0;
}
array(uint32_t) string32( const char *utf8 ) {
    static __thread int slot = 0; slot = (slot+1) % 16;
    static __thread array(uint32_t) out[16] = {0}; array_resize(out[slot], 0);

    //int worstlen = strlen(utf8) + 1; array_reserve(out, worstlen);
    while( *utf8 ) {
        uint32_t unicode = 0;
        utf8 = extract_utf32( utf8, &unicode );
        array_push(out[slot], unicode);
    }
    return out[slot];
}

#line 0

#line 1 "fwk_compat.c"
//-----------------------------------------------------------------------------
// compat (unix & stdio.h)

#if is(tcc) && is(win32) // add missing symbols
const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;        /* :: */
const struct in6_addr in6addr_loopback;   /* ::1 */
#endif

#if is(win32)
#include <io.h>
#if is(mingw)
#include <unistd.h>
#endif
#else
#include <unistd.h>
#include <sched.h> // sched_setaffinity(), CPU_ZERO(), CPU_COUNT()
#include <sys/ioctl.h>
#endif

#if is(ems)
//#define unlink(x) ((void)(x), 0)
#endif

#if is(win32)
//#define alloca        _alloca
#define atoi64        _atoi64
#define popen         _popen
#define pclose        _pclose
//#define strncasecmp   _strnicmp
#define mkdir(p,m)    mkdir(p)
#define chdir         ifdef(cl, _chdir, chdir)
#if is(cl) || is(tcc)
#define ftruncate     _chsize_s
#endif
#else // gcc
//#include <alloca.h> // mingw64 does not have it
#include <strings.h> // strncasecmp
#define atoi64        atoll
//#define strstri       strcasestr
//#define strcmpi       strcasecmp
#endif

#if   defined MAX_PATH
#define DIR_MAX MAX_PATH
#elif defined PATH_MAX
#define DIR_MAX PATH_MAX
#else
#define DIR_MAX 260
#endif

#if is(win32) // _MSC_VER and __MINGW64__
#include <stdio.h>
#include <windows.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
FILE *fmemopen(void *buf, size_t len, const char *type) {
    int fd = -1;
    char temppath[DIR_MAX - 14], filename[DIR_MAX + 1];
    if( GetTempPathA(sizeof(temppath), temppath) )
    if( GetTempFileNameA(temppath, "fwk_temp", 0, filename) )
    if( !_sopen_s(&fd, filename, _O_CREAT | _O_SHORT_LIVED | _O_TEMPORARY | _O_RDWR | _O_BINARY | _O_NOINHERIT, _SH_DENYRW, _S_IREAD | _S_IWRITE) )
    for( FILE *fp = fd != -1 ? _fdopen(fd, "w+b") : NULL; fp; )
    return fwrite(buf, len, 1, fp), rewind(fp), unlink(filename), fp; // no need to _close. fclose(on the returned FILE*) also _closes the file descriptor.
    return fd != -1 ? _close(fd), NULL : NULL;
}
#endif


#if 0
#if !is(cl)
#define tmpfile file_temp
#endif

#define tmpnam(x) file_tempname()
#endif

#if 0
static
const char *pathfile_from_handle(FILE *fp) {
#if is(win32)
    int fd = fileno(fp);
    HANDLE handle = (HANDLE)_get_osfhandle( fd ); // <io.h>
    DWORD size = GetFinalPathNameByHandleW(handle, NULL, 0, VOLUME_NAME_DOS);
    wchar_t name[DIR_MAX] = L"";
    size = GetFinalPathNameByHandleW(handle, name, size, VOLUME_NAME_DOS);
    name[size] = L'\0';
    return wchar16to8(name + 4); // skip \\?\ header
#else
    // In Linux, you can use readlink on /proc/self/fd/NNN where NNN is the file descriptor
    // In OSX:
    //     #include <sys/syslimits.h>
    //     #include <fcntl.h>
    //     char filePath[DIR_MAX];
    //     if (fcntl(fd, F_GETPATH, filePath) != -1) {
    //         // do something with the file path
    //     }
    return 0;
#endif
}
#endif

// -----------------------------------------------------------------------------
// new C macros

#define cast(T)          ifdef(c, void *, decltype(T))
#define literal(T)       ifdef(c, T, (T))

// -----------------------------------------------------------------------------

void fwk_init();
static void fwk_pre_init();
static void fwk_post_init(float);
#line 0

#line 1 "fwk_audio.c"
// @fixme: really shutdown audio & related threads before quitting. drwav crashes.


#if is(win32) && !is(gcc)
#include <windows.h>
#include <mmeapi.h> // midi
static HMIDIOUT midi_out_handle = 0;
#elif is(osx)
static AudioUnit midi_out_handle = 0;
#endif

static void midi_init() {
#if is(win32) && !is(gcc)
    if( midiOutGetNumDevs() != 0 ) {
        midiOutOpen(&midi_out_handle, 0, 0, 0, 0);        
    }
#elif is(osx)
    AUGraph graph;
    AUNode outputNode, mixerNode, dlsNode;
    NewAUGraph(&graph);
    AudioComponentDescription output = {'auou','ahal','appl',0,0};
    AUGraphAddNode(graph, &output, &outputNode);
    AUGraphOpen(graph);
    AUGraphInitialize(graph);
    AUGraphStart(graph);
    AudioComponentDescription dls = {'aumu','dls ','appl',0,0};
    AUGraphAddNode(graph, &dls, &dlsNode);
    AUGraphNodeInfo(graph, dlsNode, NULL, &midi_out_handle);
    AudioComponentDescription mixer = {'aumx','smxr','appl',0,0};
    AUGraphAddNode(graph, &mixer, &mixerNode);
    AUGraphConnectNodeInput(graph,mixerNode,0,outputNode,0);
    AUGraphConnectNodeInput(graph,dlsNode,0,mixerNode,0);
    AUGraphUpdate(graph,NULL);
#endif
}

static void midi_quit() {
#if is(win32) && !is(gcc)
    if (midi_out_handle) midiOutClose(midi_out_handle);
#endif
    // @fixme: osx
    // https://developer.apple.com/library/archive/samplecode/PlaySoftMIDI/Listings/main_cpp.html#//apple_ref/doc/uid/DTS40008635-main_cpp-DontLinkElementID_4
}

void midi_send(unsigned midi_msg) {
#if is(win32) && !is(gcc)
    if( midi_out_handle ) {
        midiOutShortMsg(midi_out_handle, midi_msg);
    }
#elif is(osx)
    if( midi_out_handle ) {
        MusicDeviceMIDIEvent(midi_out_handle, (midi_msg) & 0xFF, (midi_msg >> 8) & 0xFF, (midi_msg >> 16) & 0xFF, 0);
    }
#endif
}

// encapsulate drwav,drmp3,stbvorbis and some buffer with the sts_mixer_stream_t
enum { UNK, WAV, OGG, MP1, MP3 };
typedef struct {
    int type;
    union {
        drwav wav;
        stb_vorbis *ogg;
        void *opaque;
        drmp3           mp3_;
    };
    sts_mixer_stream_t  stream;             // mixer stream
    union {
    int32_t             data[4096*2];       // static sample buffer
    float               dataf[4096*2];
    };
    bool rewind;
} mystream_t;

static void downsample_to_mono_flt( int channels, float *buffer, int samples ) {
    if( channels > 1 ) {
        float *output = buffer;
        while( samples-- > 0 ) {
            float mix = 0;
            for( int i = 0; i < channels; ++i ) mix += *buffer++;
            *output++ = (float)(mix / channels);
        }
    }
}
static void downsample_to_mono_s16( int channels, short *buffer, int samples ) {
    if( channels > 1 ) {
        short *output = buffer;
        while( samples-- > 0 ) {
            float mix = 0;
            for( int i = 0; i < channels; ++i ) mix += *buffer++;
            *output++ = (short)(mix / channels);
        }
    }
}

// the callback to refill the (stereo) stream data
static void refill_stream(sts_mixer_sample_t* sample, void* userdata) {
    mystream_t* stream = (mystream_t*)userdata;
    switch( stream->type ) {
        default:
        break; case WAV: {
            int sl = sample->length / 2; /*sample->channels*/;
            if( stream->rewind ) stream->rewind = 0, drwav_seek_to_pcm_frame(&stream->wav, 0);
            if (drwav_read_pcm_frames_s16(&stream->wav, sl, (short*)stream->data) < sl) {
                drwav_seek_to_pcm_frame(&stream->wav, 0);
            }
        }
        break; case MP3: {
            int sl = sample->length / 2; /*sample->channels*/;
            if( stream->rewind ) stream->rewind = 0, drmp3_seek_to_pcm_frame(&stream->mp3_, 0);
            if (drmp3_read_pcm_frames_f32(&stream->mp3_, sl, stream->dataf) < sl) {
                drmp3_seek_to_pcm_frame(&stream->mp3_, 0);
            }
        }
        break; case OGG: {
            stb_vorbis *ogg = (stb_vorbis*)stream->ogg;
            if( stream->rewind ) stream->rewind = 0, stb_vorbis_seek(stream->ogg, 0);
            if( stb_vorbis_get_samples_short_interleaved(ogg, 2, (short*)stream->data, sample->length) == 0 )  {
                stb_vorbis_seek(stream->ogg, 0);
            }
        }
    }
}
static void reset_stream(mystream_t* stream) {
    if( stream ) memset( stream->data, 0, sizeof(stream->data) ), stream->rewind = 1;
}

// load a (stereo) stream
static bool load_stream(mystream_t* stream, const char *filename) {
    int datalen;
    char *data = vfs_load(filename, &datalen); if(!data) return false;

    int error;
    int HZ = 44100;
    stream->type = UNK;
    if( stream->type == UNK && (stream->ogg = stb_vorbis_open_memory((const unsigned char *)data, datalen, &error, NULL)) ) {
        stb_vorbis_info info = stb_vorbis_get_info(stream->ogg);
        if( info.channels != 2 ) { puts("cannot stream ogg file. stereo required."); goto end; } // @fixme: upsample
        stream->type = OGG;
        stream->stream.sample.frequency = info.sample_rate;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_16;
    }
    if( stream->type == UNK && drwav_init_memory(&stream->wav, data, datalen, NULL)) {
        if( stream->wav.channels != 2 ) { puts("cannot stream wav file. stereo required."); goto end; } // @fixme: upsample
        stream->type = WAV;
        stream->stream.sample.frequency = stream->wav.sampleRate;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_16;
    }
    drmp3_config mp3_cfg = { 2, HZ };
    if( stream->type == UNK && (drmp3_init_memory(&stream->mp3_, data, datalen, NULL/*&mp3_cfg*/) != 0) ) {
        stream->type = MP3;
        stream->stream.sample.frequency = stream->mp3_.sampleRate;
        stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_FLOAT;
    }

    if( stream->type == UNK ) {
        return false;
    }

    end:;
    stream->stream.userdata = stream;
    stream->stream.callback = refill_stream;
    stream->stream.sample.length = sizeof(stream->data) / sizeof(stream->data[0]);
    stream->stream.sample.data = stream->data;
    refill_stream(&stream->stream.sample, stream);

    return true;
}

// load a (mono) sample
static bool load_sample(sts_mixer_sample_t* sample, const char *filename) {
    int datalen;
    char *data = vfs_load(filename, &datalen); if(!data) return false;

    int error;
    int channels = 0;

    if( !channels ) for( drwav w = {0}, *wav = &w; wav && drwav_init_memory(wav, data, datalen, NULL); wav = 0 ) {
        channels = wav->channels;
        sample->frequency = wav->sampleRate;
        sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        sample->length = wav->totalPCMFrameCount;
        sample->data = REALLOC(0, sample->length * sizeof(short) * channels);
        drwav_read_pcm_frames_s16(wav, sample->length, (short*)sample->data);
        drwav_uninit(wav);
    }
    if( !channels ) for( stb_vorbis *ogg = stb_vorbis_open_memory((const unsigned char *)data, datalen, &error, NULL); ogg; ogg = 0 ) {
        stb_vorbis_info info = stb_vorbis_get_info(ogg);
        channels = info.channels;
        sample->frequency = info.sample_rate;
        sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        sample->length = (int)stb_vorbis_stream_length_in_samples(ogg);
        stb_vorbis_close(ogg);

        short *buffer;
        int sample_rate;
        stb_vorbis_decode_memory((const unsigned char *)data, datalen, &channels, &sample_rate, (short **)&buffer);
        sample->data = buffer;
    }
    drmp3_config mp3_cfg = { 2, 44100 };
    drmp3_uint64 mp3_fc;
    if( !channels ) for( short *fbuf = drmp3_open_memory_and_read_pcm_frames_s16(data, datalen, &mp3_cfg, &mp3_fc, NULL); fbuf ; fbuf = 0 ) {
        channels = mp3_cfg.channels;
        sample->frequency = mp3_cfg.sampleRate;
        sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        sample->length = mp3_fc; //  / sizeof(float) / mp3_cfg.channels;
        sample->data = fbuf;
    }
    if( !channels ) {
        short *output = 0;
        int outputSize, hz, mp1channels;
        bool ok = jo_read_mp1(data, datalen, &output, &outputSize, &hz, &mp1channels);
        if( ok ) {
            channels = mp1channels;
            sample->frequency = hz;
            sample->audio_format = STS_MIXER_SAMPLE_FORMAT_16;
            sample->length = outputSize / sizeof(int16_t) / channels;
            sample->data = output; // REALLOC(0, sample->length * sizeof(int16_t) * channels );
            // memcpy( sample->data, output, outputSize );
        }
    }

    if( !channels ) {
        return false;
    }

    if( channels > 1 ) {
        if( sample->audio_format == STS_MIXER_SAMPLE_FORMAT_FLOAT ) {
            downsample_to_mono_flt( channels, sample->data, sample->length );
            sample->data = REALLOC( sample->data, sample->length * sizeof(float));
        }
        else
        if( sample->audio_format == STS_MIXER_SAMPLE_FORMAT_16 ) {
            downsample_to_mono_s16( channels, sample->data, sample->length );
            sample->data = REALLOC( sample->data, sample->length * sizeof(short));
        }
        else {
            puts("error!"); // @fixme
        }
    }

    return true;
}

// -----------------------------------------------------------------------------

static ma_device  device;
static ma_context context;
static sts_mixer_t mixer;

// This is the function that's used for sending more data to the device for playback.
static ma_uint32 audio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    int len = frameCount;
    sts_mixer_mix_audio(&mixer, pOutput, len / (sizeof(int32_t) / 4));
    (void)pDevice; (void)pInput;
    return len / (sizeof(int32_t) / 4);
}

void audio_drop(void) {
    ma_device_stop(&device);
    ma_device_uninit(&device);
    ma_context_uninit(&context);
}

int audio_init( int flags ) {
    atexit(audio_drop);

    // init sts_mixer
    sts_mixer_init(&mixer, 44100, STS_MIXER_SAMPLE_FORMAT_32);

    // The prioritization of backends can be controlled by the application. You need only specify the backends
    // you care about. If the context cannot be initialized for any of the specified backends ma_context_init()
    // will fail.
    ma_backend backends[] = {
#if 1
        ma_backend_wasapi, // Higest priority.
        ma_backend_dsound,
        ma_backend_winmm,
        ma_backend_pulseaudio,
        ma_backend_alsa,
        ma_backend_oss,
        ma_backend_jack,
        ma_backend_opensl,
        //ma_backend_webaudio,
        //ma_backend_openal,
        //ma_backend_sdl,
        ma_backend_null    // Lowest priority.
#else
        // Highest priority
        ma_backend_wasapi,      // WASAPI      |  Windows Vista+
        ma_backend_dsound,      // DirectSound |  Windows XP+
        ma_backend_winmm,       // WinMM       |  Windows XP+ (may work on older versions, but untested)
        ma_backend_coreaudio,   // Core Audio  |  macOS, iOS 
        ma_backend_pulseaudio,  // PulseAudio  |  Cross Platform (disabled on Windows, BSD and Android)
        ma_backend_alsa,        // ALSA        |  Linux 
        ma_backend_oss,         // OSS         |  FreeBSD 
        ma_backend_jack,        // JACK        |  Cross Platform (disabled on BSD and Android)
        ma_backend_opensl,      // OpenSL ES   |  Android (API level 16+)
        ma_backend_webaudio,    // Web Audio   |  Web (via Emscripten)
        ma_backend_sndio,       // sndio       |  OpenBSD 
        ma_backend_audio4,      // audio(4)    |  NetBSD, OpenBSD 
        ma_backend_aaudio,      // AAudio      |  Android 8+
        ma_backend_custom,      // Custom      |  Cross Platform 
        ma_backend_null,        // Null        |  Cross Platform (not used on Web)
        // Lowest priority
#endif
    };

    if (ma_context_init(backends, countof(backends), NULL, &context) != MA_SUCCESS) {
        PRINTF("%s\n", "Failed to initialize audio context.");
        return false;
    }

    ma_device_config config = ma_device_config_init(ma_device_type_playback); // Or ma_device_type_capture or ma_device_type_duplex.
    config.playback.pDeviceID = NULL; // &myPlaybackDeviceID; // Or NULL for the default playback device.
    config.playback.format    = ma_format_s32;
    config.playback.channels  = 2;
    config.sampleRate         = 44100;
    config.dataCallback       = (void*)audio_callback; //< @r-lyeh add void* cast
    config.pUserData          = NULL;

    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.");
        ma_context_uninit(&context);
        return false;
    }

    (void)flags;
    ma_device_start(&device);
    return true;
}

typedef struct audio_handle {
    bool is_clip;
    bool is_stream;
    union {
    sts_mixer_sample_t clip;
    mystream_t         stream;
    };
} audio_handle;

static array(audio_handle*) audio_instances;

audio_t audio_clip( const char *pathfile ) {
    audio_handle *a = REALLOC(0, sizeof(audio_handle) );
    memset(a, 0, sizeof(audio_handle));
    a->is_clip = load_sample( &a->clip, pathfile );
    array_push(audio_instances, a);
    return a;
}
audio_t audio_stream( const char *pathfile ) {
    audio_handle *a = REALLOC(0, sizeof(audio_handle) );
    memset(a, 0, sizeof(audio_handle));
    a->is_stream = load_stream( &a->stream, pathfile );
    array_push(audio_instances, a);
    return a;
}


static float volume_clip = 1, volume_stream = 1, volume_master = 1;
float audio_volume_clip(float gain) {
    if( gain >= 0 && gain <= 1 ) volume_clip = gain * gain;
        // patch all live clips
        for(int i = 0, active = 0; i < STS_MIXER_VOICES; ++i) {
            if(mixer.voices[i].state != STS_MIXER_VOICE_STOPPED) // is_active?
            if( mixer.voices[i].sample ) // is_sample?
                mixer.voices[i].gain = volume_clip;
        }
    return sqrt( volume_clip );
}
float audio_volume_stream(float gain) {
    if( gain >= 0 && gain <= 1 ) volume_stream = gain * gain;
        // patch all live streams
        for(int i = 0, active = 0; i < STS_MIXER_VOICES; ++i) {
            if(mixer.voices[i].state != STS_MIXER_VOICE_STOPPED) // is_active?
            if( mixer.voices[i].stream ) // is_stream?
                mixer.voices[i].gain = volume_stream;
        }
    return sqrt( volume_stream );
}
float audio_volume_master(float gain) {
    if( gain >= 0 && gain <= 1 ) volume_master = gain * gain;
        // patch global mixer
        mixer.gain = volume_master;
    return sqrt( volume_master );
}

int audio_play_gain_pitch_pan( audio_t a, int flags, float gain, float pitch, float pan ) {
    static bool muted = 0; do_once muted = flag("--mute") || flag("--muted");
    if(muted) return 1;

    if( flags & AUDIO_IGNORE_MIXER_GAIN ) {
        // do nothing, gain used as-is
    } else {
        // apply mixer gains on top
        gain += a->is_clip ? volume_clip : volume_stream;
    }

    if( flags & AUDIO_SINGLE_INSTANCE ) {
        audio_stop( a );
    }

    // gain: [0..+1], pitch: (0..N], pan: [-1..+1]

    if( a->is_clip ) {
        int voice = sts_mixer_play_sample(&mixer, &a->clip, gain, pitch, pan);
        if( voice == -1 ) return 0; // all voices busy
    }
    if( a->is_stream ) {
        int voice = sts_mixer_play_stream(&mixer, &a->stream.stream, gain);
        if( voice == -1 ) return 0; // all voices busy
    }
    return 1;
}

int audio_play_gain_pitch( audio_t a, int flags, float gain, float pitch ) {
    return audio_play_gain_pitch_pan(a, flags, gain, pitch, 0);
}

int audio_play_gain( audio_t a, int flags, float gain ) {
    return audio_play_gain_pitch(a, flags, gain, 1.f);
}

int audio_play( audio_t a, int flags ) {
    return audio_play_gain(a, flags & ~AUDIO_IGNORE_MIXER_GAIN, 0.f);
}

int audio_stop( audio_t a ) {
    if( a->is_clip ) {
        sts_mixer_stop_sample(&mixer, &a->clip);
    }
    if( a->is_stream ) {
        sts_mixer_stop_stream(&mixer, &a->stream.stream);
        reset_stream(&a->stream);
    }
    return 1;
}

// -----------------------------------------------------------------------------
// audio queue

#ifndef AUDIO_QUEUE_BUFFERING_MS
#define AUDIO_QUEUE_BUFFERING_MS 50 // 10 // 100
#endif
#ifndef AUDIO_QUEUE_MAX
#define AUDIO_QUEUE_MAX 2048
#endif
#ifndef AUDIO_QUEUE_TIMEOUT
#define AUDIO_QUEUE_TIMEOUT ifdef(win32, THREAD_QUEUE_WAIT_INFINITE, 500)
#endif

typedef struct audio_queue_t {
    int cursor;
    int avail;
    unsigned flags;
    char data[0];
} audio_queue_t;

static thread_queue_t queue_mutex;

static void audio_queue_init() {
    static void* audio_queues[AUDIO_QUEUE_MAX] = {0};
    do_once thread_queue_init(&queue_mutex, countof(audio_queues), audio_queues, 0);
}

static void audio_queue_callback(sts_mixer_sample_t* sample, void* userdata) {
    (void)userdata;

    int sl = sample->length / 2; // 2 ch
    int bytes = sl * 2 * (sample->audio_format == STS_MIXER_SAMPLE_FORMAT_16 ? 2 : 4);
    char *dst = sample->data;

    static audio_queue_t *aq = 0;

    do {
        while( !aq ) aq = (audio_queue_t*)thread_queue_consume(&queue_mutex, THREAD_QUEUE_WAIT_INFINITE);

        int len = aq->avail > bytes ? bytes : aq->avail;
        memcpy(dst, (char*)aq->data + aq->cursor, len);
        dst += len;
        bytes -= len;
        aq->cursor += len;
        aq->avail -= len;

        if( aq->avail <= 0 ) {
            FREE(aq); // @fixme: mattias' original thread_queue_consume() implementation crashes here on tcc+win because of a double free on same pointer. using mcmp for now
            aq = 0;
        }
    } while( bytes > 0 );
}

static int audio_queue_voice = -1;
void audio_queue_clear() {
    do_once audio_queue_init();
    sts_mixer_stop_voice(&mixer, audio_queue_voice);
    audio_queue_voice = -1;
}
int audio_queue( const void *samples, int num_samples, int flags ) {
    do_once audio_queue_init();

    float gain = 1; // [0..1]
    float pitch = 1; // (0..N]
    float pan = 0; // [-1..1]

    int bits = flags & AUDIO_8 ? 8 : flags & (AUDIO_32|AUDIO_FLOAT) ? 32 : 16;
    int channels = flags & AUDIO_2CH ? 2 : 1;
    int bytes_per_sample = channels * (bits / 8);
    int bytes = num_samples * bytes_per_sample;

    static sts_mixer_stream_t q = { 0 };
    if( audio_queue_voice < 0 ) {
        void *reuse_ptr = q.sample.data;
        q = ((sts_mixer_stream_t){0});
        q.sample.data = reuse_ptr;

        q.callback = audio_queue_callback;
        q.sample.frequency = flags & AUDIO_8KHZ ? 8000 : flags & AUDIO_11KHZ ? 11025 : flags & AUDIO_44KHZ ? 44100 : flags & AUDIO_32KHZ ? 32000 : 22050;
        q.sample.audio_format = flags & AUDIO_FLOAT ? STS_MIXER_SAMPLE_FORMAT_FLOAT : STS_MIXER_SAMPLE_FORMAT_16;
        q.sample.length = q.sample.frequency / (1000 / AUDIO_QUEUE_BUFFERING_MS); // num_samples;
        int bytes = q.sample.length * 2 * (flags & AUDIO_FLOAT ? 4 : 2);
        q.sample.data = memset(REALLOC(q.sample.data, bytes), 0, bytes);
        audio_queue_voice = sts_mixer_play_stream(&mixer, &q, gain * 1.f);
        if( audio_queue_voice < 0 ) return 0;
    }

    audio_queue_t *aq = MALLOC(sizeof(audio_queue_t) + (bytes << (channels == 1))); // dupe space if going to be converted from mono to stereo
    aq->cursor = 0;
    aq->avail = bytes;
    aq->flags = flags;
    if( !samples ) {
        memset(aq->data, 0, bytes);
    } else {
        // @todo: convert from other source formats to target format in here: add AUDIO_8, AUDIO_32
        if( channels == 1 ) {
            // mixer accepts stereo samples only; so resample mono to stereo if needed
            for( int i = 0; i < num_samples; ++i ) {
                memcpy((char*)aq->data + (i*2+0) * bytes_per_sample, (char*)samples + i * bytes_per_sample, bytes_per_sample );
                memcpy((char*)aq->data + (i*2+1) * bytes_per_sample, (char*)samples + i * bytes_per_sample, bytes_per_sample );
            }
        } else {
            memcpy(aq->data, samples, bytes);
        }
    }

    while( !thread_queue_produce(&queue_mutex, aq, THREAD_QUEUE_WAIT_INFINITE) ) {}

    return audio_queue_voice;
}
#line 0

#line 1 "fwk_collide.c"
/* poly */
poly poly_alloc(int cnt) {
    poly p = {0};
    p.cnt = cnt;
    p.verts = REALLOC(p.verts, sizeof(p.verts[0]) * cnt); // array_resize(p.verts, cnt);
    return p;
}

void poly_free(poly *p) {
    REALLOC(p->verts, 0); // array_free(p->verts);
    poly z = {0};
    *p = z;
}

/* plane */
vec4 plane4(vec3 p, vec3 n) {
    return vec34(n, -dot3(n,p));
}

/* pyramid */
poly pyramid(vec3 from, vec3 to, float size) {
    /* calculate axis */
    vec3 up, right, forward = norm3( sub3(to, from) );
    ortho3(&right, &up, forward);

    /* calculate extend */
    vec3 xext = scale3(right, size);
    vec3 yext = scale3(up, size);
    vec3 nxext = scale3(right, -size);
    vec3 nyext = scale3(up, -size);

    /* calculate base vertices */
    poly p = {0};
    p.verts = REALLOC(p.verts, sizeof(p.verts[0]) * (5+1)); p.cnt = 5; /*+1 for diamond case*/ // array_resize(p.verts, 5+1); p.cnt = 5;
    p.verts[0] = add3(add3(from, xext), yext); /*a*/
    p.verts[1] = add3(add3(from, xext), nyext); /*b*/
    p.verts[2] = add3(add3(from, nxext), nyext); /*c*/
    p.verts[3] = add3(add3(from, nxext), yext); /*d*/
    p.verts[4] = to; /*r*/
    return p;
}

/* pyramid */
poly diamond(vec3 from, vec3 to, float size) {
    vec3 mid = add3(from, scale3(sub3(to, from), 0.5f));
    poly p = pyramid(mid, to, size);
    p.verts[5] = from; p.cnt = 6;
    return p;
}

// ---

static void transform_(vec3 *r, vec3 v, const float *r33, vec3 t3) {
    for (int i = 0; i < 3; ++i) {
        i[&r->x]  = i[&v.x] * r33[i*3+0];
        i[&r->x] += i[&v.x] * r33[i*3+1];
        i[&r->x] += i[&v.x] * r33[i*3+2];
        i[&r->x] += i[&t3.x];
    }
}
static void transformS(vec3 *v, const float *r33, vec3 t3) {
    vec3 tmp = *v;
    transform_(v, tmp, r33, t3);
}
static void transformT(vec3 *r, vec3 v, const float *r33, vec3 t3) {
    for (int i = 0; i < 3; ++i) {
        float p = i[&v.x] - i[&t3.x];
        i[&r->x]  = p * r33[0*3+i];
        i[&r->x] += p * r33[1*3+i];
        i[&r->x] += p * r33[2*3+i];
    }
}
static void transformST(vec3 *v, const float *r33, vec3 t3) {
    vec3 tmp = *v;
    transformT(v, tmp, r33, t3);
}

/* ============================================================================
 *
 *                                  COLLISION
 *
 * =========================================================================== */

static __thread hit hits[16] = {0};
static __thread int hit_index = -1;
#define hit_next() &hits[ (++hit_index) & 15 ]

static float line_closest_line_(float *t1, float *t2, vec3 *c1, vec3 *c2, line l, line m) {
    vec3 r, d1, d2;
    d1 = sub3(l.b, l.a); /* direction vector segment s1 */
    d2 = sub3(m.b, m.a); /* direction vector segment s2 */
    r = sub3(l.a, m.a);

    float i = dot3(d1, d1);
    float e = dot3(d2, d2);
    float f = dot3(d2, r);

    if (i <= C_EPSILON && e <= C_EPSILON) {
        /* both segments degenerate into points */
        vec3 d12;
        *t1 = *t2 = 0.0f;
        *c1 = l.a;
        *c2 = m.a;
        d12 = sub3(*c1, *c2);
        return dot3(d12,d12);
    }
    if (i > C_EPSILON) {
        float c = dot3(d1,r);
        if (e > C_EPSILON) {
            /* non-degenerate case */
            float b = dot3(d1,d2);
            float denom = i*e - b*b;

            /* compute closest point on L1/L2 if not parallel else pick any t2 */
            if (denom != 0.0f)
                *t1 = clampf(0.0f, (b*f - c*e) / denom, 1.0f);
            else *t1 = 0.0f;

            /* cmpute point on L2 closest to S1(s) */
            *t2 = (b*(*t1) + f) / e;
            if (*t2 < 0.0f) {
                *t2 = 0.0f;
                *t1 = clampf(0.0f, -c/i, 1.0f);
            } else if (*t2 > 1.0f) {
                *t2 = 1.0f;
                *t1 = clampf(0.0f, (b-c)/i, 1.0f);
            }
        } else {
            /* second segment degenerates into a point */
            *t1 = clampf(0.0f, -c/i, 1.0f);
            *t2 = 0.0f;
        }
    } else {
        /* first segment degenerates into a point */
        *t2 = clampf(0.0f, f/e, 1.0f);
        *t1 = 0.0f;
    }
    /* calculate closest points */
    vec3 n, d12;
    n = scale3(d1, *t1);
    *c1 = add3(l.a, n);
    n = scale3(d2, *t2);
    *c2 = add3(m.a, n);

    /* calculate squared distance */
    d12 = sub3(*c1, *c2);
    return dot3(d12,d12);
}

vec3 line_closest_point(line l, vec3 p) {
    vec3 ab = sub3(l.b,l.a), pa = sub3(p,l.a);
    float t = dot3(pa,ab) / dot3(ab,ab);
    return add3(l.a, scale3(ab, t < 0 ? 0 : t > 1 ? 1 : t));
}
float line_distance2_point(line l, vec3 p) {
    vec3 ab = sub3(l.a,l.b), ap = sub3(l.a,p), bp = sub3(l.b,p);
    /* handle cases p proj outside ab */
    float e = dot3(ap,ab); if (e <= 0) return dot3(ap,ap);
    float f = dot3(ab,ab); if (e >= f) return dot3(bp,bp);
    return dot3(ap,ap) - (e*e)/f;
}
float ray_test_plane(ray r, vec4 plane) {
    /* Ray: P = origin + rd * t
     * Plane: plane_normal * P + d = 0
     *
     * Substitute:
     *      normal * (origin + rd*t) + d = 0
     *
     * Solve for t:
     *      plane_normal * origin + plane_normal * rd*t + d = 0
     *      -(plane_normal*rd*t) = plane_normal * origin + d
     *
     *                  plane_normal * origin + d
     *      t = -1 * -------------------------
     *                  plane_normal * rd
     *
     * Result:
     *      Behind: t < 0
     *      Infront: t >= 0
     *      Parallel: t = 0
     *      Intersection point: ro + rd * t
     */
    vec3 p = ptr3(&plane.x);
    float n = -(dot3(p,r.p) + plane.w);
    if (fabs(n) < 0.0001f) return 0.0f;
    return n/(dot3(p,r.d));
}
float ray_test_triangle(ray r, triangle tr) {
    float t = 0;
    vec3 di0, di1, di2;
    vec3 d21, d02, in;
    vec3 n, d10, d20;
    vec3 in0, in1, in2;

    /* calculate triangle normal */
    d10 = sub3(tr.p1,tr.p0);
    d20 = sub3(tr.p2,tr.p0);
    d21 = sub3(tr.p2,tr.p1);
    d02 = sub3(tr.p0,tr.p2);
    n = cross3(d10,d20);

    /* check for plane intersection */
    vec4 p = plane4(tr.p0, n);
    t = ray_test_plane(r, p);
    if (t <= 0.0f) return t;

    /* intersection point */
    in = scale3(r.d,t);
    in = add3(in,r.p);

    /* check if point inside triangle in plane */
    di0 = sub3(in, tr.p0);
    di1 = sub3(in, tr.p1);
    di2 = sub3(in, tr.p2);

    in0 = cross3(d10, di0);
    in1 = cross3(d21, di1);
    in2 = cross3(d02, di2);

    if (dot3(in0,n) < 0.0f)
        return -1;
    if (dot3(in1,n) < 0.0f)
        return -1;
    if (dot3(in2,n) < 0.0f)
        return -1;
    return t;
}
int ray_test_sphere(float *t0, float *t1, ray r, sphere s) {
    vec3 a;
    float tc,td,d2,r2;
    a = sub3(s.c,r.p);
    tc = dot3(r.d,a);
    if (tc < 0) return 0;

    r2 = s.r*s.r;
    d2 = dot3(a,a) - tc*tc;
    if (d2 > r2) return 0;
    td = sqrtf(r2 - d2);

    *t0 = tc - td;
    *t1 = tc + td;
    return 1;
}
int ray_test_aabb(float *t0, float *t1, ray r, aabb a) {
    float t0x = (a.min.x - r.p.x) / r.d.x;
    float t0y = (a.min.y - r.p.y) / r.d.y;
    float t0z = (a.min.z - r.p.z) / r.d.z;
    float t1x = (a.max.x - r.p.x) / r.d.x;
    float t1y = (a.max.y - r.p.y) / r.d.y;
    float t1z = (a.max.z - r.p.z) / r.d.z;

    float tminx = minf(t0x, t1x);
    float tminy = minf(t0y, t1y);
    float tminz = minf(t0z, t1z);
    float tmaxx = maxf(t0x, t1x);
    float tmaxy = maxf(t0y, t1y);
    float tmaxz = maxf(t0z, t1z);
    if (tminx > tmaxy || tminy > tmaxx)
        return 0;

    *t0 = maxf(tminx, tminy);
    *t1 = minf(tmaxy, tmaxx);
    if (*t0 > tmaxz || tminz> *t1)
        return 0;

    *t0 = maxf(*t0, tminz);
    *t1 = minf(*t1, tmaxz);
    return 1;
}
vec3 sphere_closest_point(sphere s, vec3 p) {
    vec3 d = norm3(sub3(p, s.c));
    return add3(s.c, scale3(d,s.r));
}
int sphere_test_sphere(sphere a, sphere b) {
    vec3 d = sub3(b.c, a.c);
    float r = a.r + b.r;
    if (dot3(d,d) > r*r)
        return 0;
    return 1;
}
hit *sphere_hit_sphere(sphere a, sphere b) {
    vec3 d = sub3(b.c, a.c);
    float r = a.r + b.r;
    float d2 = dot3(d,d);
    if (d2 > r*r) return 0;

    hit *m = hit_next();
    float l = sqrtf(d2);
    float linv = 1.0f / ((l != 0) ? l: 1.0f);
    m->normal = scale3(d, linv);
    m->depth = r - l;
    d = scale3(m->normal, b.r);
    m->contact_point = sub3(b.c, d);
    return m;
}
int sphere_test_aabb(sphere s, aabb a) {
    return aabb_test_sphere(a, s);
}
hit *sphere_hit_aabb(sphere s, aabb a) {
    /* find closest aabb point to sphere center point */
    vec3 ap = aabb_closest_point(a, s.c);
    vec3 d = sub3(s.c, ap);
    float d2 = dot3(d, d);
    if (d2 > s.r*s.r) return 0;

    hit *m = hit_next();
    /* calculate distance vector between sphere and aabb center points */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));
    d = sub3(ac, s.c);

    /* normalize distance vector */
    float l2 = dot3(d,d);
    float l = l2 != 0.0f ? sqrtf(l2): 1.0f;
    float linv = 1.0f/l;
    d = scale3(d, linv);

    m->normal = d;
    m->contact_point = scale3(m->normal, s.r);
    m->contact_point = add3(s.c, m->contact_point);

    /* calculate penetration depth */
    vec3 sp = sphere_closest_point(s, ap);
    d = sub3(sp, ap);
    m->depth = sqrtf(dot3(d,d)) - l;
    return m;
}
int sphere_test_capsule(sphere s, capsule c) {
    return capsule_test_sphere(c, s);
}
hit *sphere_hit_capsule(sphere s, capsule c) {
#if 0
        // original code
        /* find closest capsule point to sphere center point */
        hit *m = hit_next();
        vec3 cp = capsule_closest_point(c, s.c);
        m->normal = sub3(cp, s.c);
        float d2 = dot3(m->normal, m->normal);
        if (d2 > s.r*s.r) return 0;

        /* normalize hit normal vector */
        m->normal = norm3(m->normal);

        /* calculate penetration depth */
        m->depth = d2 - s.r*s.r;
        m->depth = m->depth != 0.0f ? sqrtf(m->depth): 0.0f;
        m->contact_point = add3(s.c, scale3(m->normal, s.r));
        return m;
#else
        // aproximation of I would expect this function to return instead
        vec3 l = sub3(c.a,c.b); float len = len3(l);
        vec3 d = norm3(l);
        ray r = ray(add3(c.a,scale3(d,-2*len)), d);
        s.r += c.r;
        hit *h = ray_hit_sphere(r, s);
        if(!h) return 0;
        s.r -= c.r;
        h->contact_point = add3(s.c,scale3(norm3(sub3(h->contact_point,s.c)),s.r));
        return h;
#endif
}
int sphere_test_poly(sphere s, poly p) {
    return poly_test_sphere(p, s);
}
void aabb_rebalance_transform(aabb *b, aabb a, mat33 m, vec3 t) {
    for (int i = 0; i < 3; ++i) {
        i[&b->min.x] = i[&b->max.x] = i[&t.x];
        for (int j = 0; j < 3; ++j) {
            float e = m[i*3+j] * j[&a.min.x];
            float f = m[i*3+j] * j[&a.max.x];
            if (e < f) {
                i[&b->min.x] += e;
                i[&b->max.x] += f;
            } else {
                i[&b->min.x] += f;
                i[&b->max.x] += e;
            }
        }
    }
}
vec3 aabb_closest_point(aabb a, vec3 p) {
    vec3 res;
    for (int i = 0; i < 3; ++i) {
        float v = i[&p.x];
        if (v < i[&a.min.x]) v = i[&a.min.x];
        if (v > i[&a.max.x]) v = i[&a.max.x];
        i[&res.x] = v;
    }
    return res;
}
float aabb_distance2_point(aabb a, vec3 p) {
    float r = 0;
    for (int i = 0; i < 3; ++i) {
        float v = i[&p.x];
        if (v < i[&a.min.x]) r += (i[&a.min.x]-v) * (i[&a.min.x]-v);
        if (v > i[&a.max.x]) r += (v-i[&a.max.x]) * (v-i[&a.max.x]);
    } return r;
}
int aabb_contains_point(aabb a, vec3 p) {
    if (p.x < a.min.x || p.x > a.max.x) return 0;
    if (p.y < a.min.y || p.y > a.max.y) return 0;
    if (p.z < a.min.z || p.z > a.max.z) return 0;
    return 1;
}
int aabb_test_aabb(aabb a, aabb b) {
    if (a.max.x < b.min.x || a.min.x > b.max.x) return 0;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return 0;
    if (a.max.z < b.min.z || a.min.z > b.max.z) return 0;
    return 1;
}
hit *aabb_hit_aabb(aabb a, aabb b) {
    if (!aabb_test_aabb(a, b))
        return 0;

    hit *m = hit_next();
    /* calculate distance vector between both aabb center points */
    vec3 ac, bc, d;
    ac = sub3(a.max, a.min);
    bc = sub3(b.max, b.min);

    ac = scale3(ac, 0.5f);
    bc = scale3(bc, 0.5f);

    ac = add3(a.min, ac);
    bc = add3(b.min, bc);
    d = sub3(bc, ac);

    /* normalize distance vector */
    float l2 = dot3(d,d);
    float l = l2 != 0.0f ? sqrtf(l2): 1.0f;
    float linv = 1.0f/l;
    d = scale3(d, linv);

    /* calculate contact point */
    m->normal = d;
    m->contact_point = aabb_closest_point(a, bc);
    d = sub3(m->contact_point, ac);

    /* calculate penetration depth */
    float r2 = dot3(d,d);
    float r = sqrtf(r2);
    m->depth = r - l;
    return m;
}
int aabb_test_sphere(aabb a, sphere s) {
    /* compute squared distance between sphere center and aabb */
    float d2 = aabb_distance2_point(a, s.c);
    /* intersection if distance is smaller/equal sphere radius*/
    return d2 <= s.r*s.r;
}
hit *aabb_hit_sphere(aabb a, sphere s) {
    /* find closest aabb point to sphere center point */
    hit *m = hit_next();
    m->contact_point = aabb_closest_point(a, s.c);
    vec3 d = sub3(s.c, m->contact_point);
    float d2 = dot3(d, d);
    if (d2 > s.r*s.r) return 0;

    /* calculate distance vector between aabb and sphere center points */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));
    d = sub3(s.c, ac);

    /* normalize distance vector */
    float l2 = dot3(d,d);
    float l = l2 != 0.0f ? sqrtf(l2): 1.0f;
    float linv = 1.0f/l;
    d = scale3(d, linv);

    /* calculate penetration depth */
    m->normal = d;
    d = sub3(m->contact_point, ac);
    m->depth = sqrtf(dot3(d,d));
    return m;
}
int aabb_test_capsule(aabb a, capsule c) {
    return capsule_test_aabb(c, a);
}
hit *aabb_hit_capsule(aabb a, capsule c) {
    /* calculate aabb center point */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));

    /* calculate closest point from aabb to point on capsule and check if inside aabb */
    vec3 cp = capsule_closest_point(c, ac);
    if (!aabb_contains_point(a, cp))
        return 0;

    hit *m = hit_next();
    /* vector and distance between both capsule closests point and aabb center*/
    vec3 d; float d2;
    d = sub3(cp, ac);
    d2 = dot3(d,d);

    /* calculate penetration depth from closest aabb point to capsule */
    vec3 ap = aabb_closest_point(a, cp);
    vec3 dt = sub3(ap, cp);
    m->depth = sqrtf(dot3(dt,dt));

    /* calculate normal */
    float l = sqrtf(d2);
    float linv = 1.0f / ((l != 0.0f) ? l: 1.0f);
    m->normal = scale3(d, linv);
    m->contact_point = ap;
    return m;
}
int aabb_test_poly(aabb a, poly p) {
    return poly_test_aabb(p, a);
}
float capsule_distance2_point(capsule c, vec3 p) {
    float d2 = line_distance2_point(line(c.a,c.b), p);
    return d2 - (c.r*c.r);
}
vec3 capsule_closest_point(capsule c, vec3 p) {
    /* calculate closest point to internal capsule segment */
    vec3 pp = line_closest_point(line(c.a,c.b), p);

    /* extend point out by radius in normal direction */
    vec3 d = norm3(sub3(p,pp));
    return add3(pp, scale3(d, c.r));
}
int capsule_test_capsule(capsule a, capsule b) {
    float t1, t2;
    vec3 c1, c2;
    float d2 = line_closest_line_(&t1, &t2, &c1, &c2, line(a.a,a.b), line(b.a,b.b));
    float r = a.r + b.r;
    return d2 <= r*r;
}
hit *capsule_hit_capsule(capsule a, capsule b) {
    float t1, t2;
    vec3 c1, c2;
    float d2 = line_closest_line_(&t1, &t2, &c1, &c2, line(a.a,a.b), line(b.a,b.b));
    float r = a.r + b.r;
    if (d2 > r*r) return 0;

    hit *m = hit_next();
    /* calculate normal from both closest points for each segement */
    vec3 cp, d;
    m->normal = sub3(c2, c1);
    m->normal = norm3(m->normal);

    /* calculate contact point from closest point and depth */
    m->contact_point = capsule_closest_point(a, c2);
    cp = capsule_closest_point(b, c1);
    d = sub3(c1, cp);
    m->depth = sqrtf(dot3(d,d));
    return m;
}
int capsule_test_sphere(capsule c, sphere s) {
    /* squared distance bwetween sphere center and capsule line segment */
    float d2 = line_distance2_point(line(c.a,c.b), s.c);
    float r = s.r + c.r;
    return d2 <= r * r;
}
hit *capsule_hit_sphere(capsule c, sphere s) {
    /* find closest capsule point to sphere center point */
    hit *m = hit_next();
    m->contact_point = capsule_closest_point(c, s.c);
    m->normal = sub3(s.c, m->contact_point);
    float d2 = dot3(m->normal, m->normal);
    if (d2 > s.r*s.r) return 0;

    /* normalize hit normal vector */
    float l = d2 != 0.0f ? sqrtf(d2): 1;
    float linv = 1.0f/l;
    m->normal = scale3(m->normal, linv);

    /* calculate penetration depth */
    m->depth = d2 - s.r*s.r;
    m->depth = m->depth != 0.0f ? sqrtf(m->depth): 0.0f;
    return m;
}
int capsule_test_aabb(capsule c, aabb a) {
    /* calculate aabb center point */
    vec3 ac = scale3(sub3(a.max, a.min), 0.5f);

    /* calculate closest point from aabb to point on capsule and check if inside aabb */
    vec3 p = capsule_closest_point(c, ac);
    return aabb_contains_point(a, p);
}
hit *capsule_hit_aabb(capsule c, aabb a) {
    /* calculate aabb center point */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));

    /* calculate closest point from aabb to point on capsule and check if inside aabb */
    vec3 cp = capsule_closest_point(c, ac);
    if (!aabb_contains_point(a, cp))
        return 0;

    hit *m = hit_next();
    /* vector and distance between both capsule closests point and aabb center*/
    vec3 d; float d2;
    d = sub3(ac, cp);
    d2 = dot3(d,d);

    /* calculate penetration depth from closest aabb point to capsule */
    vec3 ap = aabb_closest_point(a, cp);
    vec3 dt = sub3(ap, cp);
    m->depth = sqrtf(dot3(dt,dt));

    /* calculate normal */
    float l = sqrtf(d2);
    float linv = 1.0f / ((l != 0.0f) ? l: 1.0f);
    m->normal = scale3(d, linv);
    m->contact_point = cp;
    return m;
}
int capsule_test_poly(capsule c, poly p) {
    return poly_test_capsule(p, c);
}
int line_support(vec3 *support, vec3 d, vec3 a, vec3 b) {
    int i = 0;
    float adot = dot3(a, d);
    float bdot = dot3(b, d);
    if (adot < bdot) {
        *support = b;
        i = 1;
    } else *support = a;
    return i;
}
int poly_support(vec3 *support, vec3 d, poly p) {
    int imax = 0;
    float dmax = dot3(*p.verts, d);
    for (int i = 1; i < p.cnt; ++i) {
        /* find vertex with max dot product in direction d */
        float dot = dot3(p.verts[i], d);
        if (dot < dmax) continue;
        imax = i, dmax = dot;
    } *support = p.verts[imax];
    return imax;
}
int poly_hit_sphere(struct gjk_result *res,
    poly p,
    sphere s) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *p.verts;
    gs.b = s.c;
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        gs.aid = poly_support(&gs.a, n, p);
        d = sub3(gs.b, gs.a);
    }
    /* check distance between closest points */
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= s.r*s.r;
}
int poly_hit_sphere_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, sphere s) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *p.verts;
    gs.b = s.c;
    transformS(&gs.a, rot33, pos3);
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        vec3 da; transformT(&da, n, rot33, pos3);

        gs.aid = poly_support(&gs.a, da, p);
        transformS(&gs.a, rot33, pos3);
        d = sub3(gs.b, gs.a);
    }
    /* check distance between closest points */
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= s.r*s.r;
}
int poly_test_sphere(poly p, sphere s) {
    struct gjk_result res;
    return poly_hit_sphere(&res, p, s);
}
int poly_test_sphere_transform(poly p, vec3 pos3, mat33 rot33, sphere s) {
    struct gjk_result res;
    return poly_hit_sphere_transform(&res, p, pos3, rot33, s);
}
int poly_hit_capsule(struct gjk_result *res, poly p, capsule c) {
    /* initial guess */
    vec3 d = {0};
    gjk_support s = {0};
    s.a = *p.verts;
    s.b = c.a;
    d = sub3(s.b, s.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &s, &d)) {
        vec3 n = scale3(d, -1);
        s.aid = poly_support(&s.a, n, p);
        s.bid = line_support(&s.b, d, c.a, c.b);
        d = sub3(s.b, s.a);
    }
    /* check distance between closest points */
    assert(gsx.iter < gsx.max_iter);
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= c.r*c.r;
}
int poly_test_capsule(poly p, capsule c) {
    struct gjk_result res;
    return poly_hit_capsule(&res, p, c);
}
int poly_hit_capsule_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, capsule c) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *p.verts;
    gs.b = c.a;
    transformS(&gs.a, rot33, pos3);
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        vec3 da; transformT(&da, n, rot33, pos3);

        gs.aid = poly_support(&gs.a, da, p);
        gs.bid = line_support(&gs.b, d, c.a, c.b);
        transformS(&gs.a, rot33, pos3);
        d = sub3(gs.b, gs.a);
    }
    /* check distance between closest points */
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= c.r*c.r;
}
int poly_test_capsule_transform(poly p, vec3 pos3, mat33 rot33, capsule c) {
    struct gjk_result res;
    return poly_hit_capsule_transform(&res, p, pos3, rot33, c);
}
int poly_hit_poly_transform(struct gjk_result *res,
    poly a, vec3 at3, mat33 ar33,
    poly b, vec3 bt3, mat33 br33) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *a.verts;
    gs.b = *b.verts;
    transformS(&gs.a, ar33, at3);
    transformS(&gs.b, br33, bt3);
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        /* transform direction */
        vec3 n = scale3(d, -1);
        vec3 da; transformT(&da, n, ar33, at3);
        vec3 db; transformT(&db, d, br33, bt3);
        /* run support function on tranformed directions  */
        gs.aid = poly_support(&gs.a, da, a);
        gs.bid = poly_support(&gs.b, db, b);
        /* calculate distance vector on transformed points */
        transformS(&gs.a, ar33, at3);
        transformS(&gs.b, br33, bt3);
        d = sub3(gs.b, gs.a);
    }
    *res = gjk_analyze(&gsx);
    return gsx.hit;
}
int poly_hit_poly(struct gjk_result *res,
    poly a,
    poly b) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *a.verts;
    gs.b = *b.verts;
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        gs.aid = poly_support(&gs.a, n, a);
        gs.bid = poly_support(&gs.b, d, b);
        d = sub3(gs.b, gs.a);
    }
    *res = gjk_analyze(&gsx);
    return gsx.hit;
}
int poly_test_poly(poly a, poly b) {
    struct gjk_result res;
    return poly_hit_poly(&res, a, b);
}
int poly_test_poly_transform(poly a, vec3 apos3, mat33 arot33,
    poly b, vec3 bpos3, mat33 brot33) {
    struct gjk_result res;
    return poly_hit_poly_transform(&res, a, apos3, arot33, b, bpos3, brot33);
}
int poly_hit_aabb(struct gjk_result *res, poly p, aabb a) {
    vec3 box[8];
    box[0] = vec3(a.min.x, a.min.y, a.min.z),
    box[1] = vec3(a.min.x, a.min.y, a.max.z);
    box[2] = vec3(a.min.x, a.max.y, a.min.z);
    box[3] = vec3(a.min.x, a.max.y, a.max.z);
    box[4] = vec3(a.max.x, a.min.y, a.min.z);
    box[5] = vec3(a.max.x, a.min.y, a.max.z);
    box[6] = vec3(a.max.x, a.max.y, a.min.z);
    box[7] = vec3(a.max.x, a.max.y, a.max.z);
    return poly_hit_poly(res, p, poly(&box[0], 8));
}
int poly_hit_aabb_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, aabb a) {
    vec3 zero = {0};
    vec3 id[3] = {{1,0,0},{0,1,0},{0,0,1}};
    vec3 box[8];
    box[0] = vec3(a.min.x, a.min.y, a.min.z),
    box[1] = vec3(a.min.x, a.min.y, a.max.z);
    box[2] = vec3(a.min.x, a.max.y, a.min.z);
    box[3] = vec3(a.min.x, a.max.y, a.max.z);
    box[4] = vec3(a.max.x, a.min.y, a.min.z);
    box[5] = vec3(a.max.x, a.min.y, a.max.z);
    box[6] = vec3(a.max.x, a.max.y, a.min.z);
    box[7] = vec3(a.max.x, a.max.y, a.max.z);
    return poly_hit_poly_transform(res, p, pos3, rot33, poly(&box[0], 8), zero, id[0].v3);
}
int poly_test_aabb(poly p, aabb a) {
    struct gjk_result res;
    return poly_hit_aabb(&res, p, a);
}
int poly_test_aabb_transform(poly p, vec3 apos3, mat33 arot33, aabb a) {
    struct gjk_result res;
    return poly_hit_aabb_transform(&res, p, apos3, arot33, a);
}

/* ============================================================================
 *
 *                              COLLISION VOLUME
 *
 * =========================================================================== */

hit *ray_hit_plane(ray r, plane p) {
    vec4 pf = plane4(p.p, p.n);
    float t = ray_test_plane(r, pf);
    if (t <= 0.0f) return 0;
    hit *o = hit_next();
    o->p = add3(r.p, scale3(r.d, t));
    o->t0 = o->t1 = t;
    o->n = scale3(p.n, -1.0f);
    return o;
}
hit *ray_hit_triangle(ray r, triangle tr) {
    float t = ray_test_triangle(r, tr);
    if (t <= 0) return 0;

    hit *o = hit_next();
    o->t0 = o->t1 = t;
    o->p = add3(r.p, scale3(r.d, t));
    o->n = norm3(cross3(sub3(tr.p1,tr.p0),sub3(tr.p2,tr.p0)));
    return o;
}
hit *ray_hit_sphere(ray r, sphere s) {
    hit *o = hit_next();
    if (!ray_test_sphere(&o->t0, &o->t1, r, s))
        return 0;
    o->p = add3(r.p, scale3(r.d, minf(o->t0,o->t1)));
    o->n = norm3(sub3(o->p, s.c));
    return o;
}
hit *ray_hit_aabb(ray r, aabb a) {
    hit *o = hit_next();

    vec3 pnt, ext, c;
    float d, min;
    if (!ray_test_aabb(&o->t0, &o->t1, r, a))
        return 0;

    o->p = add3(r.p, scale3(r.d, minf(o->t0,o->t1)));
    ext = sub3(a.max, a.min);
    c = add3(a.min, scale3(ext,0.5f));
    pnt = sub3(o->p, c);

    min = fabs(ext.x - fabs(pnt.x));
    o->n = scale3(vec3(1,0,0), signf(pnt.x));
    d = fabs(ext.y - fabs(pnt.y));
    if (d < min) {
        min = d;
        o->n = scale3(vec3(0,1,0), signf(pnt.y));
    }
    d = fabs(ext.z - fabs(pnt.z));
    if (d < min)
        o->n = scale3(vec3(0,0,1), signf(pnt.z));
    return o;
}

frustum frustum_build(mat44 pv) {
    frustum f;
    f.l = vec4(pv[ 3]+pv[ 0], pv[ 7]+pv[ 4], pv[11]+pv[ 8], pv[15]+pv[12]);
    f.r = vec4(pv[ 3]-pv[ 0], pv[ 7]-pv[ 4], pv[11]-pv[ 8], pv[15]-pv[12]);
    f.t = vec4(pv[ 3]-pv[ 1], pv[ 7]-pv[ 5], pv[11]-pv[ 9], pv[15]-pv[13]);
    f.b = vec4(pv[ 3]+pv[ 1], pv[ 7]+pv[ 5], pv[11]+pv[ 9], pv[15]+pv[13]);
    f.n = vec4(pv[ 3]+pv[ 2], pv[ 7]+pv[ 6], pv[11]+pv[10], pv[15]+pv[14]);
    f.f = vec4(pv[ 3]-pv[ 2], pv[ 7]-pv[ 6], pv[11]-pv[10], pv[15]-pv[14]);
    for (int i = 0; i < 6; i++) f.pl[i] = scale4(f.pl[i], 1 / len3(f.pl[i].xyz));
    return f;
}
int frustum_test_sphere(frustum f, sphere s) {
    for(int i = 0; i < 6; i++) {
        if((dot3(f.pl[i].xyz, s.c) + f.pl[i].w + s.r) < 0) return 0;
    }
    return 1;
}
int frustum_test_aabb(frustum f, aabb a) {
    for(int i = 0; i < 6; i++) {
        vec3 v = vec3(f.pl[i].x > 0 ? a.max.x : a.min.x, f.pl[i].y > 0 ? a.max.y : a.min.y, f.pl[i].z > 0 ? a.max.z : a.min.z);
        if((dot3(f.pl[i].xyz, v) + f.pl[i].w) < 0) return 0;
    }
    return 1;
}

void collide_demo() { // debug draw collisions // @fixme: fix leaks: poly_free()

    // animation
    static float dx = 0, dy = 0;
    float delta = (0.25f / 60.f);
    dx = dx + delta * 2.0f;
    dy = dy + delta * 0.8f;

#if 0
    // 3D
    glEnable(GL_DEPTH_TEST);

    // grid
    ddraw_grid(0);
#endif

    {
        // Triangle-Ray Intersection*/
        vec3 ro, rd;
        int suc;

        triangle tri = { vec3(-9,1,28), vec3(-10,0,28), vec3(-11,1,28) };

        // ray
        ro = vec3(-10,-1,20);
        rd = vec3(-10+0.4f*sin(dx), 2.0f*cos(dy), 29.81023f);
        rd = sub3(rd, ro);
        rd = norm3(rd);

        ray r = ray(ro, rd);
        hit *hit = ray_hit_triangle(r, tri);
        if (hit) {
            // point of intersection
            ddraw_color(RED);
            ddraw_box(hit->p, vec3(0.10f, 0.10f, 0.10f));

            // intersection normal
            ddraw_color(BLUE);
            vec3 v = add3(hit->p, hit->n);
            ddraw_arrow(hit->p, v);
        }

        // line
        ddraw_color(RED);
        rd = scale3(rd,10);
        rd = add3(ro,rd);
        ddraw_line(ro, rd);

        // triangle
        if (hit) ddraw_color(RED);
        else ddraw_color(WHITE);
        ddraw_triangle(tri.p0,tri.p1,tri.p2);
    }
    {
        // Plane-Ray Intersection*/
        vec3 ro, rd;
        mat33 rot;

        // ray
        static float d = 0;
        d += delta * 2.0f;
        ro = vec3(0,-1,20);
        rd = vec3(0.1f, 0.5f, 9.81023f);
        rd = sub3(rd, ro);
        rd = norm3(rd);

        // rotation
        rotation33(rot, deg(d), 0,1,0);
        rd = mulv33(rot, rd);

        // intersection
        ray r = ray(ro, rd);
        plane pl = plane(vec3(0,0,28), vec3(0,0,1));
        hit *hit = ray_hit_plane(r, pl);
        if (hit) {
            // point of intersection
            ddraw_color(RED);
            ddraw_box(hit->p, vec3(0.10f, 0.10f, 0.10f));

            // intersection normal
            ddraw_color(BLUE);
            vec3 v = add3(hit->p, hit->n);
            ddraw_arrow(hit->p, v);
            ddraw_color(RED);
        }
        // line
        ddraw_color(RED);
        rd = scale3(rd,9);
        rd = add3(ro,rd);
        ddraw_line(ro, rd);

        // plane
        if (hit) ddraw_color(RED);
        else ddraw_color(WHITE);
        ddraw_plane(vec3(0,0,28), vec3(0,0,1), 3.0f);
    }
    {
        // Sphere-Ray Intersection*/
        vec3 ro, rd;
        sphere s;

        // ray
        ro = vec3(0,-1,0);
        rd = vec3(0.4f*sin(dx), 2.0f*cos(dy), 9.81023f);
        rd = sub3(rd, ro);
        rd = norm3(rd);

        ray r = ray(ro, rd);
        s = sphere(vec3(0,0,8), 1);
        hit *hit = ray_hit_sphere(r, s);
        if(hit) {
            // points of intersection
            vec3 in = add3(ro,scale3(rd,hit->t0));

            ddraw_color(GREEN);
            ddraw_box(in, vec3(0.05f, 0.05f, 0.05f));

            in = add3(ro,scale3(rd,hit->t1));

            ddraw_color(YELLOW);
            ddraw_box(in, vec3(0.05f, 0.05f, 0.05f));

            // intersection normal
            ddraw_color(BLUE);
            vec3 v = add3(hit->p, hit->n);
            ddraw_arrow(hit->p, v);
            ddraw_color(RED);
        }
        // line
        ddraw_color(RED);
        rd = scale3(rd,10);
        rd = add3(ro,rd);
        ddraw_line(ro, rd);

        // sphere
        if (hit) ddraw_color(RED);
        else ddraw_color(WHITE);
        ddraw_sphere(vec3(0,0,8), 1);
    }
    {   // ray-aabb
        aabb bounds = aabb(vec3(10-0.5f,-0.5f,7.5f), vec3(10.5f,0.5f,8.5f));

        vec3 ro = vec3(10,-1,0);
        vec3 rd = vec3(10+0.4f*sin(dx), 2.0f*cos(dy), 9.81023f);
        rd = norm3(sub3(rd, ro));
        ray r = ray(ro, rd);

        hit *hit = ray_hit_aabb(r, bounds);
        if(hit) {
            // points of intersection
            vec3 in;
            in = scale3(rd,hit->t0);
            in = add3(ro,in);

            ddraw_color(RED);
            ddraw_box(in, vec3(0.05f, 0.05f, 0.05f));

            in = scale3(rd,hit->t1);
            in = add3(ro,in);

            ddraw_color(RED);
            ddraw_box(in, vec3(0.05f, 0.05f, 0.05f));

            // intersection normal
            ddraw_color(BLUE);
            vec3 v = add3(hit->p, hit->n);
            ddraw_arrow(hit->p, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);
        ddraw_box(vec3(10,0,8), vec3(1,1,1));

        // line
        ddraw_color(RED);
        rd = scale3(rd,10);
        rd = add3(ro,rd);
        ddraw_line(ro, rd);
    }
    {
        // Sphere-Sphere intersection*/
        sphere a = sphere(vec3(-10,0,8), 1);
        sphere b = sphere(vec3(-10+0.6f*sin(dx), 3.0f*cos(dy),8), 1);
        hit *m = sphere_hit_sphere(a, b);
        if (m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);

        ddraw_sphere(a.c, 1);
        ddraw_sphere(b.c, 1);
    }
    {
        // AABB-AABB intersection*/
        const float x = 10+0.6f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = 20.0f;

        aabb a = aabb(vec3(10-0.5f,-0.5f,20-0.5f), vec3(10+0.5f,0.5f,20.5f));
        aabb b = aabb(vec3(x-0.5f,y-0.5f,z-0.5f), vec3(x+0.5f,y+0.5f,z+0.5f));
        hit *m = aabb_hit_aabb(a, b);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);

        ddraw_box(vec3(10,0,20), vec3(1,1,1));
        ddraw_box(vec3(x,y,z), vec3(1,1,1));
    }
    {
        // Capsule-Capsule intersection*/
        const float x = 20+0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = 28.5f;

        capsule a = capsule(vec3(20.0f,-1.0f,28.0f), vec3(20.0f,1.0f,28.0f), 0.2f);
        capsule b = capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        hit *m = capsule_hit_capsule(a, b);
        if( m ) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);
        ddraw_capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        ddraw_capsule(vec3(20.0f,-1.0f,28.0f), vec3(20.0f,1.0f,28.0f), 0.2f);
    }
    {
        // AABB-Sphere intersection*/
        aabb a = aabb(vec3(20-0.5f,-0.5f,7.5f), vec3(20.5f,0.5f,8.5f));
        sphere s = sphere(vec3(20+0.6f*sin(dx), 3.0f*cos(dy),8), 1);
        hit *m  = aabb_hit_sphere(a, s);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);

        ddraw_box(vec3(20,0,8), vec3(1,1,1));
        ddraw_sphere(s.c, 1);
    }
    {
        // Sphere-AABB intersection*/
        const float x = 10+0.6f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -8.0f;

        sphere s = sphere(vec3(10,0,-8), 1);
        aabb a = aabb(vec3(x-0.5f,y-0.5f,z-0.5f), vec3(x+0.5f,y+0.5f,z+0.5f));
        hit *m = sphere_hit_aabb(s, a);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);

        ddraw_box(vec3(x,y,z), vec3(1,1,1));
        ddraw_sphere(s.c, 1);
    }
    {
        // Capsule-Sphere intersection*/
        capsule c = capsule(vec3(-20.5f,-1.0f,7.5f), vec3(-20+0.5f,1.0f,8.5f), 0.2f);
        sphere b = sphere(vec3(-20+0.6f*sin(dx), 3.0f*cos(dy),8), 1);
        hit *m = capsule_hit_sphere(c, b);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);
        ddraw_sphere(b.c, 1);
        ddraw_capsule(vec3(-20.5f,-1.0f,7.5f), vec3(-20+0.5f,1.0f,8.5f), 0.2f);
    }
    {
        // Sphere-Capsule intersection*/
        const float x = 20+0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -8;

        sphere s = sphere(vec3(20,0,-8), 1);
        capsule c = capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        hit *m = sphere_hit_capsule(s, c);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);

        ddraw_capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        ddraw_sphere(s.c, 1);
    }
    {
        // Capsule-AABB intersection*/
        const float x = -20+0.6f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = 28.0f;

        capsule c = capsule(vec3(-20.5f,-1.0f,27.5f), vec3(-20+0.5f,1.0f,28.5f), 0.2f);
        aabb b = aabb(vec3(x-0.5f,y-0.5f,z-0.5f), vec3(x+0.5f,y+0.5f,z+0.5f));
        hit *m = capsule_hit_aabb(c, b);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);
        ddraw_box(vec3(x,y,z), vec3(1,1,1));
        ddraw_capsule(vec3(-20.5f,-1.0f,27.5f), vec3(-20+0.5f,1.0f,28.5f), 0.2f);
    }
    {
        // AABB-Capsule intersection*/
        const float x = 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -8;

        aabb a = aabb(vec3(-0.5f,-0.5f,-8.5f), vec3(0.5f,0.5f,-7.5f));
        capsule c = capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        hit *m = aabb_hit_capsule(a, c);
        if(m) {
            ddraw_color(RED);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            ddraw_arrow(m->contact_point, add3(m->contact_point, m->normal));
        } else ddraw_color(WHITE);

        ddraw_capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        ddraw_box(vec3(0,0,-8.0f), vec3(1,1,1));
    }
    {
        // poly(Pyramid)-Sphere (GJK) intersection*/
        sphere s = sphere(vec3(-10+0.6f*sin(dx), 3.0f*cos(dy),-8), 1);
        poly pyr = pyramid(vec3(-10.5f,-0.5f,-7.5f), vec3(-10.5f,1.0f,-7.5f), 1.0f);

        gjk_result gjk;
        if (poly_hit_sphere(&gjk, pyr, s))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_sphere(s.c, 1);
        ddraw_pyramid(vec3(-10.5f,-0.5f,-7.5f), 0.5f/*vec3(-10.5f,1.0f,-7.5f)*/, 1.0f);

        poly_free(&pyr);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }
    {
        // poly(Diamond)-Sphere (GJK) intersection*/

        sphere s = sphere(vec3(-20+0.6f*sin(dx), 3.0f*cos(dy),-8), 1);
        poly dmd = diamond(vec3(-20.5f,-0.5f,-7.5f), vec3(-20.5f,1.0f,-7.5f), 0.5f);

        gjk_result gjk;
        if (poly_hit_sphere(&gjk, dmd, s))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_sphere(s.c, 1);
        ddraw_diamond(vec3(-20.5f,-0.5f,-7.5f), vec3(-20.5f,1.0f,-7.5f), 0.5f);

        poly_free(&dmd);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }
    {
        // poly(Pyramid)-Capsule (GJK) intersection*/

        const float x = 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -15;

        capsule c = capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z), 0.2f);
        poly pyr = pyramid(vec3(-0.5f,-0.5f,-15.5f), vec3(-0.5f,1.0f,-15.5f), 1.0f);

        gjk_result gjk;
        if (poly_hit_capsule(&gjk, pyr, c))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_capsule(c.a, c.b, c.r);
        ddraw_pyramid(vec3(-0.5f,-0.5f,-15.5f), 0.5f/*vec3(-0.5f,1.0f,-15.5f)*/, 1.0f);

        poly_free(&pyr);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }

    {
        // poly(Diamond)-Capsule (GJK) intersection*/

        const float x = -10 + 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -15;

        capsule c = capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z), 0.2f);
        poly dmd = diamond(vec3(-10.5f,-0.5f,-15.5f), vec3(-10.5f,1.0f,-15.5f), 0.5f);

        gjk_result gjk;
        if (poly_hit_capsule(&gjk, dmd, c))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_capsule(c.a, c.b, c.r);
        ddraw_diamond(vec3(-10.5f,-0.5f,-15.5f), vec3(-10.5f,1.0f,-15.5f), 0.5f);

        poly_free(&dmd);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }

    {
        // poly(Diamond)-poly(Pyramid) (GJK) intersection*/

        const float x = -20 + 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -15;

        poly pyr = pyramid(vec3(x,y-0.5f,z), vec3(x,y+1,z), 0.8f);
        poly dmd = diamond(vec3(-20.5f,-0.5f,-15.5f), vec3(-20.5f,1.0f,-15.5f), 0.5f);

        gjk_result gjk;
        if (poly_hit_poly(&gjk, dmd, pyr))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_pyramid(vec3(x,y-0.5f,z), 1/*vec3(x,y+1,z)*/, 1/*0.8f*/);
        ddraw_diamond(vec3(-20.5f,-0.5f,-15.5f), vec3(-20.5f,1.0f,-15.5f), 0.5f);

        poly_free(&dmd);
        poly_free(&pyr);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }
    {
        // poly(Pyramid)-poly(Diamond) (GJK) intersection*/

        const float x = 10 + 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -15;

        poly dmd = diamond(vec3(x,y-0.5f,z), vec3(x,y+1,z), 0.5f);
        poly pyr = pyramid(vec3(10.5f,-0.5f,-15.5f), vec3(10.5f,1.0f,-15.5f), 1.0f);

        gjk_result gjk;
        if (poly_hit_poly(&gjk, dmd, pyr))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_diamond(vec3(x,y-0.5f,z), vec3(x,y+1,z), 0.5f);
        ddraw_pyramid(vec3(10.5f,-0.5f,-15.5f), 0.5f/*vec3(10.5f,1.0f,-15.5f)*/, 1.0f);

        poly_free(&dmd);
        poly_free(&pyr);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }
    {
        // poly(Diamond)-AABB (GJK) intersection*/

        const float x = 20 + 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -15;

        poly dmd = diamond(vec3(x,y-0.5f,z), vec3(x,y+1,z), 0.5f);
        aabb a = aabb(vec3(19.5f,-0.5f,-14.5f), vec3(20.5f,0.5f,-15.5f));

        gjk_result gjk;
        if (poly_hit_aabb(&gjk, dmd, a))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        poly_free(&dmd);

        ddraw_diamond(vec3(x,y-0.5f,z), vec3(x,y+1,z), 0.5f);
        ddraw_box(vec3(20,0,-15), vec3(1,1,1));

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }
}
#line 0

#line 1 "fwk_cooker.c"
// data pipeline
// - rlyeh, public domain.
// ----------------------------------------------------------------------------
// @todo: threads should steal workloads from job queue
// @todo: restore errno/errorlevel checks
// @todo: +=, -=, that_asset.ini
// @todo: @dae FLAGS+=-U
// @todo: SF2_SOUNDBANK=TOOLS/soundbank.sf2
// @fixme: leaks (worth?)
// -----------------------------------------------------------------------------

const char *ART = "art/";
const char *TOOLS = "tools/bin/";
const char *EDITOR = "tools/editor/";
const char *COOK_INI = "tools/cook.ini";

static unsigned ART_SKIP_ROOT; // number of chars to skip the base root in ART folder
static unsigned ART_LEN;       // dupe

typedef struct cook_subscript_t {
    char *infile;
    char *outfile;    // can be either infile, or a totally different file
    char *script;
    char *outname;
    int compress_level;
} cook_subscript_t;

typedef struct cook_script_t {
    cook_subscript_t cs[8];

    int num_passes;
} cook_script_t;

static
cook_script_t cook_script(const char *rules, const char *infile, const char *outfile) {
    cook_script_t mcs = { 0 };

    // pass loop: some asset rules may require multiple cook passes
    for( int pass = 0; pass < countof(mcs.cs); ++pass ) {
        // by default, assume:
        // - no script is going to be generated (empty script)
        // - if no script is going to be generated, output is in fact input file.
        // - no compression is going to be required.
        cook_subscript_t cs = { 0 };

            // reuse script heap from last call if possible (optimization)
            static __thread char *script = 0;
            if(script) script[0] = 0;

            // reuse parsing maps if possible (optimization)
            static __thread map(char*, char*) symbols = 0;   if(!symbols) map_init_str(symbols);
            static __thread map(char*, char*) groups = 0;    if(!groups)  map_init_str(groups);
            static __thread set(char*) passes = 0;           if(!passes)  set_init_str(passes);
            map_clear(symbols);
            map_clear(groups);

            map_find_or_add(symbols, "INFILE", STRDUP(infile));
            map_find_or_add(symbols, "INPUT", STRDUP(infile));
            map_find_or_add(symbols, "PRETTY", STRDUP(infile + ART_SKIP_ROOT)); // pretty (truncated) input (C:/prj/FWK/art/file.wav -> file.wav)
            map_find_or_add(symbols, "OUTPUT", STRDUP(outfile));
            map_find_or_add(symbols, "TOOLS", STRDUP(TOOLS));
            map_find_or_add(symbols, "EDITOR", STRDUP(EDITOR));
            map_find_or_add(symbols, "PROGRESS", STRDUP(va("%03d", cook_progress())));

        // clear pass counter
        set_clear(passes);

        // start parsing. parsing is enabled by default
        int enabled = 1;
        array(char*)lines = strsplit(rules, "\r\n");
        for( int i = 0, end = array_count(lines); i < end; ++i ) {
            // skip blanks
            int blanks = strspn(lines[i], " \t");
            char *line = lines[i] + blanks;

            // discard full comments
            if( line[0] == ';' ) continue;
            // truncate inline comments
            if( strstr(line, ";") ) *strstr(line, ";") = 0;
            // trim ending spaces
            char *eos = line + strlen(line); while(eos > line && eos[-1] == ' ' ) *--eos = 0;
            // discard non-specific lines
            if( line[0] == '@' ) {
                int with_wine = flag("--cook-wine") && !!strstr(line, "@win");
                int parse = 0
                    | ifdef(win32, (!!strstr(line, "@win")), 0)
                    | ifdef(linux, (!!strstr(line, "@lin") ? 1 : with_wine), 0)
                    | ifdef(osx,   (!!strstr(line, "@osx") ? 1 : with_wine), 0);

                if( !parse ) continue;

                line = strchr(line+1, ' ');
                if(!line) continue;
                line += strspn(line, " \t");
            }
            // execute `shell` commands
            if( line[0] == '`' ) {
                char *eos = strrchr(++line, '`');
                if( eos ) *eos = 0;

                    // replace all symbols
                    char* nl = STRDUP(line); // @leak
                    for each_map(symbols, char*, key, char*, val) {
                        strrepl(&nl, key, val);
                    }
                    lines[i] = line = nl;

                static thread_mutex_t lock, *init = 0; if(!init) thread_mutex_init(init = &lock);
                thread_mutex_lock( &lock );
                    system(line); // strcatf(&script, "%s\n", line);
                thread_mutex_unlock( &lock );

                continue;
            }
            // process [sections]
            if( line[0] == '[' ) {
                enabled = 1;
                int is_cook = !!strstr(line, "[cook]");
                int is_compress = !!strstr(line, "[compress]");
                if( !is_cook && !is_compress ) { // if not a special section...
                    // remove hint cook tag if present. that's informative only.
                    if(strbegi(line, "[cook ") ) memcpy(line+1, "    ", 4); // line += 6;

                    // start parsing expressions like `[media && !avi && mp3]`
                    array(char*) tags = strsplit(line, " []&");

                    // let's check whether INPUT belongs to tags above
                    char **INPUT = map_find(symbols, "INPUT");
                    bool found_in_set = true;

                    for( int i = 0, end = array_count(tags); i < end; ++i) {
                        bool negate = false;
                        char *tag = tags[i];
                        while(*tag == '!') negate ^= 1, ++tag;

                        // find tag in groups map
                        // either a group or an extension
                        char **is_group = map_find(groups, tag);
                        if( is_group ) {
                            char *list = *is_group;
                            char *INPUT_EXT = file_ext(infile); INPUT_EXT = strrchr(INPUT_EXT, '.'); // .ext1.ext -> .ext
                            char *ext = INPUT_EXT; ext += ext[0] == '.'; // dotless
                            bool in_list = strbegi(list, ext) || strendi(list, va(",%s",ext)) || strstri(list, va(",%s,",ext));
                            if( !in_list ^ negate ) { found_in_set = false; break; }
                        } else {
                            char *ext = va(".%s", tag);
                            bool found = !!strendi(*INPUT, ext);
                            if( !found ^ negate ) { found_in_set = false; break; }
                        }
                    }
                    if( found_in_set ) {
                        // inc pass
                        set_find_or_add(passes, STRDUP(*tags)); // @leak
                        // check whether we keep searching
                        int num_passes = set_count(passes);
                        found_in_set = ( pass == (num_passes-1) );
                    }
                    //
                    enabled = found_in_set ? 1 : 0;
                }
            }
            // either SYMBOL=, group=, or regular script line
            if( enabled && line[0] != '[' ) {
                enum { group, symbol, regular } type = regular;
                int tokenlen = strspn(line, "-+_.|0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
                char *token = va("%.*s", tokenlen, line);
                char *equal = strchr(line, '=');
                if( equal ) {
                    if( equal == &line[tokenlen] ) { // if key=value expression found
                        // discriminate: symbols are uppercase and never begin with digits. groups are [0-9]+[|][a-z].
                        type = strcmp(strupper(token), token) || isdigit(token[0]) ? group : symbol;
                    }
                }
                if( type == group  ) map_find_or_add(groups, token, STRDUP(equal+1));
                if( type == symbol ) {
                    // @todo: perform the replacement/union/intersection on set here
                    bool is_add = strendi(token, "+");
                    bool is_del = strendi(token, "-");

                    // if present, remove last sign from token -> (FLAGS1+)=, (FLAGS1-)=
                    if(is_add || is_del) token[strlen(token) - 1] = 0;

                    map_find_or_add(symbols, token, STRDUP(equal+1));
                }
                // for each_map(symbols, char*, key, char*, val) printf("%s=%s,", key, val); puts("");
                // for each_map(groups, char*, key, char*, val) printf("%s=%s,", key, val); puts("");
                // if( type != regular ) printf("%s found >> %s\n", type == group ? "group" : "symbol", line);

                if( type == regular ) {
                    char** INPUT = map_find(symbols, "INPUT");
                    char** OUTPUT = map_find(symbols, "OUTPUT");

                    // parse return code
                    char *has_errorlevel = strstr(line, "=="); //==N form
                    int errorlevel = has_errorlevel ? atoi(has_errorlevel + 2) : 0;
                    if( has_errorlevel ) memcpy(has_errorlevel, "   ", 3);

                    // detect if newer extension or filename is present, and thus update OUTPUT if needed
                    char *newer_extension = strstr(line, "->"); if(newer_extension) {
                        *newer_extension = 0;
                        newer_extension += 2 + strspn(newer_extension + 2, " ");

                        if( strchr(newer_extension, '.') ) {
                            // newer filename
                            cs.outname = stringf("%s@%s", cs.outname ? cs.outname : infile, newer_extension); // @leak // special char (multi-pass cooks)
                            newer_extension = NULL;
                        } else {
                            strcatf(&*OUTPUT, ".%s", newer_extension);
                        }
                    }

                    // replace all symbols
                    char* nl = STRDUP(line); // @leak
                    for each_map(symbols, char*, key, char*, val) {
                        strrepl(&nl, key, val);
                    }
                    lines[i] = line = nl;

                    // convert slashes
                    ifdef(win32,
                        strswap(line, "/", "\\")
                    , // else
                        strswap(line, "\\", "/")
                    );

                    // append line
                    strcatf(&script, "%s\n", line);

                    // handle return code here
                    // if(has_errorlevel)
                    // strcatf(&script, "IF NOT '%%ERRORLEVEL%%'=='%d' echo ERROR!\n", errorlevel);

                    // rename output->input for further chaining, in case it is needed
                    if( newer_extension ) {
                        *INPUT[0] = 0;
                        strcatf(&*INPUT, "%s", *OUTPUT);
                    }
                }
            }
        }

        char** OUTPUT = map_find(symbols, "OUTPUT");
        int ext_num_groups = 0;

        // compression
        if( 1 ) {
            char* ext = file_ext(infile); ext = strrchr(ext, '.'); ext += ext[0] == '.'; // dotless INPUT_EXT
            char* belongs_to = 0;
            for each_map(groups, char*, key, char*, val) {
                if( !isdigit(key[0]) ) {
                    char *comma = va(",%s,", ext);
                    if( !strcmpi(val,ext) || strbegi(val, comma+1) || strstri(val, comma) || strendi(val, va(",%s", ext))) {
                        belongs_to = key;
                        ext_num_groups++;
                    }
                }
            }
            char *compression = 0;
            for each_map(groups, char*, key, char*, val) {
                if( isdigit(key[0]) ) {
                    char *comma = va(",%s,", ext);
                    if( !strcmpi(val,ext) || strbegi(val, comma+1) || strstri(val, comma) || strendi(val, va(",%s", ext))) {
                        compression = key;
                    }
                    comma = va(",%s,", belongs_to);
                    if( !strcmpi(val,ext) || strbegi(val, comma+1) || strstri(val, comma) || strendi(val, va(",%s", ext))) {
                        compression = key;
                    }
                }
            }

            cs.compress_level = 0;
            if( compression ) {
                // last chance to optionally override the compressor at command-line level
                static const char *compressor_override;
                do_once compressor_override = option("--cook-compressor", "");
                if( compressor_override[0] ) compression = (char*)compressor_override;

                /**/ if(strstri(compression, "PPP"))  cs.compress_level = atoi(compression) | PPP;
                else if(strstri(compression, "ULZ"))  cs.compress_level = atoi(compression) | ULZ;
                else if(strstri(compression, "LZ4"))  cs.compress_level = atoi(compression) | LZ4X;
                else if(strstri(compression, "CRSH")) cs.compress_level = atoi(compression) | CRSH;
                else if(strstri(compression, "DEFL")) cs.compress_level = isdigit(compression[0]) ? atoi(compression) : 6 /*| DEFL*/;
                //else if(strstri(compression, "LZP"))  cs.compress_level = atoi(compression) | LZP1; // not supported
                else if(strstri(compression, "LZMA")) cs.compress_level = atoi(compression) | LZMA;
                else if(strstri(compression, "BALZ")) cs.compress_level = atoi(compression) | BALZ;
                else if(strstri(compression, "LZW"))  cs.compress_level = atoi(compression) | LZW3;
                else if(strstri(compression, "LZSS")) cs.compress_level = atoi(compression) | LZSS;
                else if(strstri(compression, "BCM"))  cs.compress_level = atoi(compression) | BCM;
                else                                  cs.compress_level = isdigit(compression[0]) ? atoi(compression) : 6 /*| DEFL*/;
            }
        }

        // if script was generated...
        if( script && script[0]) {
            // update outfile
            cs.outfile = *OUTPUT;

            // amalgamate script
            array(char*) lines = strsplit(script, "\r\n");

            #if is(win32)
                char *joint = strjoin(lines, " && ");
                cs.script = joint;
            #else
                if( flag("--cook-wine") ) {
                    // dear linux/osx/bsd users:
                    // tools going wrong for any reason? cant compile them maybe?
                    // small hack to use win32 pipeline tools instead
                    char *joint = strjoin(lines, " && wine " );
                    cs.script = va("wine %s", /*TOOLS,*/ joint);
                } else {
                    char *joint = strjoin(lines, " && " );
                    cs.script = va("export LD_LIBRARY_PATH=%s && %s", TOOLS, joint);
                }
            #endif
        } else {
            // ... else bypass infile->outfile
            char** INFILE = map_find(symbols, "INFILE");
            cs.outfile = *INFILE;

            // and return an empty script
            cs.script = "";
        }

        cs.outname = cs.outname ? cs.outname : (char*)infile;

        ASSERT(mcs.num_passes < countof(mcs.cs));
        mcs.cs[mcs.num_passes++] = cs;

        bool next_pass_required = mcs.num_passes < ext_num_groups;
        if( !next_pass_required ) break;
    }

    return mcs;
}

// ----------------------------------------------------------------------------

struct fs {
    char *fname, status;
    uint64_t stamp;
    uint64_t bytes;
};

static array(struct fs) fs_now;
static __thread array(char*) added;
static __thread array(char*) changed;
static __thread array(char*) deleted;
static __thread array(char*) uncooked;

static
array(struct fs) zipscan_filter(int threadid, int numthreads) {
    // iterate all previously scanned files
    array(struct fs) fs = 0;
    for( int i = 0, end = array_count(fs_now); i < end; ++i ) {
        // during workload distribution, we assign random files to specific thread buckets.
        // we achieve this by hashing the basename of the file. we used to hash also the path 
        // long time ago but that is less resilient to file relocations across the repository. 
        // excluding the file extension from the hash also helps from external file conversions.
        char *fname = file_name(fs_now[i].fname);
        char *sign = strrchr(fname, '@'); if(sign) *sign = '\0'; // special char (multi-pass cooks)
        char *dot = strrchr(fname, '.'); if(dot) *dot = '\0';

        // skip if list item does not belong to this thread bucket
        uint64_t hash = hash_str(fname);
        unsigned bucket = (hash >> 32) % numthreads;
        if(bucket != threadid) continue;

        array_push(fs, fs_now[i]);
    }
    return fs;
}

static
int zipscan_diff( zip* old, array(struct fs) now ) {
    array_free(added);
    array_free(changed);
    array_free(deleted);
    array_free(uncooked);

    // if not zipfile is present, all files are new and must be added
    if( !old ) {
        for( int i = 0; i < array_count(now); ++i ) {
            array_push(uncooked, STRDUP(now[i].fname));
        }
        return 1;
    }

    // compare for new & changed files
    for( int i = 0; i < array_count(now); ++i ) {
        int found = zip_find(old, now[i].fname);
        if( found < 0 ) {
            array_push(added, STRDUP(now[i].fname));
            array_push(uncooked, STRDUP(now[i].fname));
        } else {
            uint64_t oldsize = atoi64(zip_comment(old,found)); // zip_size(old, found); returns sizeof processed asset. return original size of unprocessed asset, which we store in comment section
            uint64_t oldstamp = atoi64(zip_modt(old,found)+20); // format is "YYYY/MM/DD hh:mm:ss", then +20 chars later a hidden epoch timestamp in base10 can be found
            int64_t diffstamp = oldstamp < now[i].stamp ? now[i].stamp - oldstamp : oldstamp - now[i].stamp;
            if( oldsize != now[i].bytes || diffstamp > 1 ) { // @fixme: should use hash instead. hashof(tool) ^ hashof(args used) ^ hashof(rawsize) ^ hashof(rawdate)
                printf("%s:\t%u vs %u, %llu vs %llu\n", now[i].fname, (unsigned)oldsize,(unsigned)now[i].bytes, oldstamp,now[i].stamp);
                array_push(changed, STRDUP(now[i].fname));
                array_push(uncooked, STRDUP(now[i].fname));
            }
        }
    }
    // compare for deleted files
    for( int i = 0; i < zip_count(old); ++i ) {
        char *oldname = zip_name(old, i);
        //if( strchr(oldname, '@') ) oldname = va("%*.s", (int)(strchr(oldname, '@') - oldname), oldname ); // special char (multi-pass cooks)

        int idx = zip_find(old, oldname); // find latest versioned file in zip
        unsigned oldsize = zip_size(old, idx);
        if (!oldsize) continue;

        struct fs *found = 0; // zipscan_locate(now, oldname);
        for(int j = 0; j < array_count(now); ++j) {
            if( !strcmp(now[j].fname,oldname)) {
                found = &now[j];
                break;
            }
        }

        if( !found ) {
            array_push(deleted, STRDUP(oldname));
        }
    }
    return 1;
}

// ----------------------------------------------------------------------------

typedef struct cook_worker {
    const char **files;
    const char *rules;
    int threadid, numthreads;
    thread_ptr_t self;
    volatile int progress;
    thread_mutex_t *lock;
} cook_worker;

enum { JOBS_MAX = 256 };
static cook_worker jobs[JOBS_MAX] = {0};
static volatile bool cook_cancelable = false, cook_cancelling = false, cook_debug = false;

#ifndef COOK_ON_DEMAND
#define COOK_ON_DEMAND flag("--cook-on-demand")
#endif

static
int cook(void *userdata) {
    cook_worker *job = (cook_worker*)userdata;

    // start progress
    volatile int *progress = &job->progress;
    *progress = 0;

    // scan disk from fs_now snapshot
    array(struct fs) filtered = zipscan_filter(job->threadid, job->numthreads);
    //printf("Scanned: %d items found\n", array_count(now));

    // prepare out tempname
    char COOK_TMPFILE[64]; snprintf(COOK_TMPFILE, 64, "temp_%02d", job->threadid);

    // prepare zip
    char zipfile[64]; snprintf(zipfile, 64, ".art[%02x].zip", job->threadid);
    if( file_size(zipfile) == 0 ) unlink(zipfile);

    // populate added/deleted/changed arrays by examining current disk vs last cache
    zip *z;
    {
        z = zip_open(zipfile, "r+b");
        zipscan_diff(z, filtered);
        if( z ) zip_close(z);

        fflush(0);

        z = zip_open(zipfile, "a+b");
        if( !z ) {
            unlink(zipfile);
            z = zip_open(zipfile, "a+b"); // try again
            if(!z) PANIC("cannot open file for updating: %s", zipfile);
        }
    }

    // deleted files. --cook-additive runs are append-only, so they skip this block
    if( !flag("--cook-additive") )
    for( int i = 0, end = array_count(deleted); i < end; ++i ) {
        printf("Deleting %03d%% %s\n", (i+1) == end ? 100 : (i * 100) / end, deleted[i]);
        FILE* out = fopen(COOK_TMPFILE, "wb"); fclose(out);
        FILE* in = fopen(COOK_TMPFILE, "rb");
        char *comment = "0";
        zip_append_file/*_timeinfo*/(z, deleted[i], comment, in, 0/*, tm_now*/);
        fclose(in);
    }
    // added or changed files
    for( int i = 0, end = array_count(uncooked); i < end && !cook_cancelling; ++i ) {
        *progress = ((i+1) == end ? 90 : (i * 90) / end); // (i+i>0) * 100.f / end;

        // start cook
        const char *infile = uncooked[i]; //job->files[j];
        int inlen = file_size(infile);

        // generate a cooking script for this asset
        cook_script_t mcs = cook_script(job->rules, infile, COOK_TMPFILE);
        // puts(cs.script);

        for(int pass = 0; pass < mcs.num_passes; ++pass) {
            cook_subscript_t cs = mcs.cs[pass];

            // log to batch file for forensic purposes, if explicitly requested
            static __thread bool logging = 0; do_once logging = !!flag("--cook-debug") || cook_debug;
            if( logging ) {
                FILE *logfile = fopen(va("cook%d.cmd",job->threadid), "a+t");
                if( logfile ) { fprintf(logfile, "@rem %s\n%s\n", cs.outname, cs.script); fclose(logfile); }
                fprintf(stderr, "%s\n", cs.script);
            }

            // invoke cooking script and recap status
            const char *rc_output = app_exec(cs.script);
            int rc = atoi(rc_output);
            int outlen = file_size(cs.outfile);
            int failed = cs.script[0] ? rc || !outlen : 0;

            // print errors
            if( failed ) {
                PRINTF("Import failed: %s while executing:\n%s\nReturned:\n%s\n", cs.outname, cs.script, rc_output);
                continue;
            }

            // special char (multi-pass cook). newly generated file: refresh values
            // ensure newly created files by cook are also present on repo/disc for further cook passes
            if( pass > 0 ) { // && strchr(cs.outname, '@') ) { // pass>0 is a small optimization // special char (multi-pass cooks)
                file_delete(cs.outname);
                file_move(cs.outfile, cs.outname);
                inlen = file_size(infile = cs.outfile = cs.outname);
            }

            // process only if included. may include optional compression.
            if( cs.compress_level >= 0 ) {
                FILE *in = fopen(cs.outfile, "rb");

#if 0
                    struct stat st; stat(infile, &st);
                    struct tm *timeinfo = localtime(&st.st_mtime);
                    ASSERT(timeinfo);
#endif

                    char *comment = va("%d", inlen);
                    if( !zip_append_file/*_timeinfo*/(z, infile, comment, in, cs.compress_level/*, timeinfo*/) ) {
                        PANIC("failed to add processed file into %s: %s(%s)", zipfile, cs.outname, infile);
                    }

                fclose(in);
            }
        }
    }

    zip_close(z);

    // end progress
    if( file_size(zipfile) == 0 ) unlink(zipfile);
    *progress = 100;

    return 1;
}

static
int cook_async( void *userdata ) {
#if COOK_FROM_TERMINAL
    // nothing to do...
#else
    while(!window_handle()) sleep_ms(100); // wait for window handle to be created
#endif

    // boost cook thread #0, which happens to be the only spawn thread when num_jobs=1 (tcc case, cook-sync case).
    // also in multi-threaded scenarios, it is not bad at all to have one high priority thread...
    // in any case, game view is not going to look bad because the game will be displaying a progress bar at that time.
    cook_worker *job = (cook_worker*)userdata;
    if( job->threadid == 0 ) thread_set_high_priority();

    // tcc: only a single running thread shall pass, because of racing shared state due to missing thread_local support at compiler level
    ifdef(tcc, thread_mutex_lock( job->lock ));

    int ret = cook(userdata);

    // tcc: only a single running thread shall pass, because of racing shared state due to missing thread_local support at compiler level
    ifdef(tcc, thread_mutex_unlock( job->lock ));

    thread_exit( ret );
    return ret;
}

bool cook_start( const char *cook_ini, const char *masks, int flags ) {
    cook_ini = cook_ini ? cook_ini : COOK_INI;

    char *rules_ = file_read(cook_ini);
    if(!rules_ || rules_[0] == 0) return false;

    static char *rules; do_once rules = STRDUP(rules_);

    do_once {
    #if 0
        const char *HOME = file_pathabs(cook_ini); // ../tools/cook.ini -> c:/prj/fwk/tools/cook.ini
        if( strbeg(HOME, app_path() ) ) HOME = STRDUP( file_path( HOME += strlen(app_path()) ) ); // -> tools/ @leak
    #else
        char *HOME = STRDUP(file_pathabs(cook_ini)); // ../tools/cook.ini -> c:/prj/fwk/tools/cook.ini
        HOME[ strlen(HOME) - strlen(file_name(cook_ini)) ] = '\0'; // -> tools/ @leak
    #endif

        ART_LEN = 0; //strlen(app_path()); 
        /* = MAX_PATH;
        for each_substring(ART, ",", art_folder) {
            ART_LEN = mini(ART_LEN, strlen(art_folder));
        }*/

        if( strstr(rules, "ART=") ) {
            ART = va( "%s", strstr(rules, "ART=") + 4 );
            char *r = strchr( ART, '\r' ); if(r) *r = 0;
            char *n = strchr( ART, '\n' ); if(n) *n = 0;
            char *s = strchr( ART, ';' );  if(s) *s = 0;
            char *w = strchr( ART, ' ' );  if(w) *w = 0;
            char *out = 0; const char *sep = "";
            for each_substring(ART, ",", t) {
                char *tmp = file_pathabs(va("%s%s", HOME, t)) + ART_LEN;
                for(int i = 0; tmp[i]; ++i) if(tmp[i]=='\\') tmp[i] = '/';
                strcatf(&out, "%s%s%s", sep, tmp, strendi(tmp, "/") ? "" : "/");
                assert( out[strlen(out) - 1] == '/' );

                sep = ",";
            }
            ART = out; // @leak
        }

        if( strstr(rules, "TOOLS=") ) {
            TOOLS = va( "%s", strstr(rules, "TOOLS=") + 6 );
            char *r = strchr( TOOLS, '\r' ); if(r) *r = 0;
            char *n = strchr( TOOLS, '\n' ); if(n) *n = 0;
            char *s = strchr( TOOLS, ';' );  if(s) *s = 0;
            char *w = strchr( TOOLS, ' ' );  if(w) *w = 0;
            char *cat = va("%s%s", HOME, TOOLS), *out = 0;
            for(int i = 0; cat[i]; ++i) if(cat[i]=='\\') cat[i] = '/';
            strcatf(&out, "%s%s", cat, strend(cat, "/") ? "" : "/");
            TOOLS = out; // @leak
            assert( TOOLS[strlen(TOOLS) - 1] == '/' );

            // last chance to autodetect tools folder (from cook.ini path)
            if( !file_directory(TOOLS) ) {
                out = STRDUP(cook_ini);
                for(int i = 0; out[i]; ++i) if(out[i]=='\\') out[i] = '/';
                TOOLS = out; // @leak
            }
        }

        if( strstr(rules, "EDITOR=") ) {
            EDITOR = va( "%s", strstr(rules, "EDITOR=") + 7 );
            char *r = strchr( EDITOR, '\r' ); if(r) *r = 0;
            char *n = strchr( EDITOR, '\n' ); if(n) *n = 0;
            char *s = strchr( EDITOR, ';' );  if(s) *s = 0;
            char *w = strchr( EDITOR, ' ' );  if(w) *w = 0;
            char *cat = va("%s%s", HOME, EDITOR), *out = 0;
            for(int i = 0; cat[i]; ++i) if(cat[i]=='\\') cat[i] = '/';
            strcatf(&out, "%s%s", cat, strend(cat, "/") ? "" : "/");
            EDITOR = out; // @leak
            assert( EDITOR[strlen(EDITOR) - 1] == '/' );
        }
    }

    if( !masks ) {
        return true; // nothing to do
    }

    // estimate ART_SKIP_ROOT (C:/prj/fwk/demos/assets/file.png -> strlen(C:/prj/fwk/) -> 11)
    {
        array(char*) dirs = 0;
        for each_substring(ART, ",", art_folder) {
            array_push(dirs, file_pathabs(art_folder));
        }
        if( array_count(dirs) > 1 )  {
            for( int ok = 1, ch = dirs[0][ART_SKIP_ROOT]; ch && ok; ch = dirs[0][++ART_SKIP_ROOT] ) {
                for( int i = 1; i < array_count(dirs) && ok; ++i ) {
                    ok = dirs[i][ART_SKIP_ROOT] == ch;
                }
            }
        }
        while( ART_SKIP_ROOT > 0 && !strchr("\\/", dirs[0][ART_SKIP_ROOT-1]) ) --ART_SKIP_ROOT;
        array_free(dirs);
    }

    if( COOK_ON_DEMAND ) {
        return true; // cooking is deferred
    }

    // scan disk: all subfolders in ART (comma-separated)
    static array(char *) list = 0; // @leak
    for each_substring(ART, ",", art_folder) {
        const char **glob = file_list(art_folder, "**");
        for( unsigned i = 0; glob[i]; ++i ) {
            const char *fname = glob[i];
            if( !strmatchi(fname, masks)) continue;

            // skip special files, folders and internal files like .art.zip
            const char *dir = file_path(fname);
            if( dir[0] == '.' ) continue; // discard system dirs and hidden files
            if( strbegi(dir, TOOLS) ) continue; // discard tools folder
            if( !file_ext(fname)[0] ) continue; // discard extensionless entries
            if( !file_size(fname)) continue; // skip dirs and empty files

            // exclude vc c/c++ .obj files. they're not 3d wavefront .obj files
            if( strend(fname, ".obj") ) {
                char header[4] = {0};
                for( FILE *in = fopen(fname, "rb"); in; fclose(in), in = NULL) {
                    fread(header, 1, 2, in);
                }
                if( !memcmp(header, "\x64\x86", 2) ) continue;
                if( !memcmp(header, "\x00\x00", 2) ) continue;
            }
            // exclude vc/gcc files
            if( strend(fname, ".a") || strend(fname, ".pdb") || strend(fname, ".lib") || strend(fname, ".ilk") || strend(fname, ".exp") ) {
                continue;
            }

            // @todo: normalize path & rebase here (absolute to local)
            // [...]
            // fi.normalized = ; tolower->to_underscore([]();:+ )->remove_extra_underscores

            if (file_name(fname)[0] == '.') continue; // skip system files
            if (file_name(fname)[0] == ';') continue; // skip comment files

            array_push(list, STRDUP(fname));
        }
    }

    // inspect disk
    for( int i = 0, end = array_count(list); i < end; ++i ) {
        char *fname = list[i];

        struct fs fi = {0};
        fi.fname = fname; // STRDUP(fname);
        fi.bytes = file_size(fname);
        fi.stamp = file_stamp10(fname); // timestamp in base10(yyyymmddhhmmss)

        array_push(fs_now, fi);
    }        

    cook_debug = !!( flags & COOK_DEBUGLOG );
    cook_cancelable = !!( flags & COOK_CANCELABLE );

    // spawn all the threads
    int num_jobs = cook_jobs();
    for( int i = 0; i < num_jobs; ++i ) {
        jobs[i].self = 0;
        jobs[i].threadid = i;
        jobs[i].numthreads = flags & COOK_ASYNC ? num_jobs : 1;
        jobs[i].files = (const char **)list;
        jobs[i].rules = rules;
        jobs[i].progress = -1;
        static thread_mutex_t lock; do_once thread_mutex_init(&lock);
        jobs[i].lock = &lock;
    }
    for( int i = 0; i < num_jobs; ++i ) {
        if( flags & COOK_ASYNC ) {
            jobs[i].self = thread_init(cook_async, &jobs[i], "cook_async()", 0/*STACK_SIZE*/);
            continue;
        }

        if(!cook(&jobs[i])) return false;
    }

    return true;
}

void cook_stop() {
    // join all threads
    int num_jobs = cook_jobs();
    for( int i = 0; i < num_jobs; ++i ) {
        if(jobs[i].self) thread_join(jobs[i].self);
    }
    // remove all temporary outfiles
    const char **temps = file_list("./", "temp_*");
    for( int i = 0; temps[i]; ++i ) unlink(temps[i]);
}

int cook_progress() {
    int count = 0, sum = 0;
    for( int i = 0, end = cook_jobs(); i < end; ++i ) {
//        if( jobs[i].progress >= 0 ) {
            sum += jobs[i].progress;
            ++count;
//        }
    }
    return cook_jobs() ? sum / (count+!count) : 100;
}

void cook_cancel() {
    if( cook_cancelable ) cook_cancelling = true;
}

int cook_jobs() {
    int num_jobs = optioni("--cook-jobs", maxf(1.15,app_cores()) * 1.75), max_jobs = countof(jobs);
    ifdef(ems, num_jobs = 0);
    return clampi(num_jobs, 0, max_jobs);
}

void cook_config( const char *pathfile_to_cook_ini ) { // @todo: test run-from-"bin/" case on Linux.
    COOK_INI = pathfile_to_cook_ini;
    ASSERT( file_exist(COOK_INI) );
}
#line 0

#line 1 "fwk_data.c"

static
array(char) base64__decode(const char *in_, unsigned inlen) {
    // from libtomcrypt
    #define BASE64_ENCODE_OUT_SIZE(s)   (((s) + 2) / 3 * 4)
    #define BASE64_DECODE_OUT_SIZE(s)   (((s)) / 4 * 3)

#if 1
    unsigned long outlen = BASE64_DECODE_OUT_SIZE(inlen);
    array(char) out_ = 0; array_resize(out_, outlen);

    if( base64_decode((const unsigned char *)in_, (unsigned long)inlen, (unsigned char *)out_, &outlen) != CRYPT_OK ) {
        array_free(out_);
        return 0;
    }

    array_resize(out_, outlen);
    return out_;
#else
    unsigned outlen = BASE64_DECODE_OUT_SIZE(inlen);
    array(char) out_ = 0; array_resize(out_, outlen);

    // based on code by Jon Mayo - November 13, 2003 (PUBLIC DOMAIN)
    uint_least32_t v;
    unsigned ii, io, rem;
    char *out = (char *)out_;
    const unsigned char *in = (const unsigned char *)in_;
    const uint8_t base64dec_tab[256]= {
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255, 62,255,255,
         52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
        255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
         15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255, 63,
        255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    };

    for (io = 0, ii = 0,v = 0, rem = 0; ii < inlen; ii ++) {
        unsigned char ch;
        if (isspace(in[ii]))
            continue;
        if ((in[ii]=='=') || (!in[ii]))
            break; /* stop at = or null character*/
        ch = base64dec_tab[(unsigned char)in[ii]];
        if (ch == 255)
            break; /* stop at a parse error */
        v = (v<<6) | ch;
        rem += 6;
        if (rem >= 8) {
            rem -= 8;
            if (io >= outlen)
                return (array_free(out_), NULL); /* truncation is failure */
            out[io ++] = (v >> rem) & 255;
        }
    }
    if (rem >= 8) {
        rem -= 8;
        if (io >= outlen)
            return (array_free(out_), NULL); /* truncation is failure */
        out[io ++] = (v >> rem) & 255;
    }
    return (array_resize(out_, io), out_);
#endif
}

static array(json5) roots;
static array(char*) sources;

bool json_push(const char *source) {
    char *source_rw = STRDUP(source);
    json5 root = {0};
    char *error = json5_parse(&root, source_rw, 0);
    if( error ) {
        FREE(source_rw);
        return false;
    } else {
        array_push(sources, source_rw);
        array_push(roots, root);
        return true;
    }
}

bool json_pop() {
    if( array_count(roots) > 0 ) {
        FREE(*array_back(sources));
        array_pop(sources);

        json5_free(array_back(roots));
        array_pop(roots);
        return true;
    }
    return false;
}

json5* json_node(const char *keypath) {
    json5 *j = array_back(roots), *r = j;
    for each_substring( keypath, "/[.]", key ) {
        r = 0;
        /**/ if( j->type == JSON5_ARRAY ) r = j = &j->array[atoi(key)];
        else if( j->type == JSON5_OBJECT && isdigit(key[0]) )
        for( int i = 0, seq = atoi(key); !r && i < j->count; ++i ) {
            if( i == seq ) {
                r = j = &j->nodes[i];
                break;
            }
        }
        else if( j->type == JSON5_OBJECT )
        for( int i = 0; !r && i < j->count; ++i ) {
            if( j->nodes[i].name && !strcmp(j->nodes[i].name, key) ) {
                r = j = &j->nodes[i];
                break;
            }
        }
        if( !j ) break;
    }
    return r;
}

int (json_count)(const char *keypath) {
    json5* j = json_node(keypath);
    return j ? j->count : 0;
}

json_t *json_find(const char *type_keypath) {
    char type = type_keypath[0];
    const char *key = type_keypath+1;
    json5 *j = json_node(key);
    if( !j ) return NULL;

    static __thread int slot = 0;
    static __thread json_t buf[128] = {0};
    slot = (slot+1) % 128;

    json_t *v = &buf[slot];
    v->i = j ? j->integer : 0;
    if(type == 's' && (!v->p || j->type == JSON5_NULL)) v->s = ""; // if_null_string
    if(type == 'f' && j && j->type == JSON5_INTEGER) v->f = j->integer;
    return v;
}

json_t json_get(const char *type_keypath) {
    char type = type_keypath[0];
    const char *key = type_keypath+1;
    json5 *j = json_node(key);

    json_t v = {0};
    v.i = j ? j->integer : 0;
    if(type == 's' && (!v.p || j->type == JSON5_NULL)) v.s = ""; // if_null_string
    if(type == 'f' && j && j->type == JSON5_INTEGER) v.f = j->integer;
    return v;
}

const char *(json_key)(const char *keypath) {
    json5 *j = json_node(keypath);
    if( !j ) return "";
    return j->name;
}

// xml impl

static __thread array(char *) xml_sources;
static __thread array(struct xml *) xml_docs;

int xml_push(const char *xml_source) {
    if( xml_source ) {
        char *src = STRDUP(xml_source), *error = 0;
        for( struct xml *doc = xml_parse(src, 0, &error); doc && !error; ) {
            array_push(xml_docs, doc);
            array_push(xml_sources, src);
            return 1;
        }
        if( error ) PRINTF("%s\n", error);
        FREE(src);
    }
    return 0;
}

void xml_pop() {
    if( array_count(xml_docs) ) {
        xml_free( *array_back(xml_docs) );
        array_pop(xml_docs);
        FREE( *array_back(xml_sources) );
        array_pop(xml_sources);
    }
}

static void *xml_path(struct xml *node, char *path, int down) {
    if( !path || !path[0] ) return node;
    if( node ) {

        char type = path[0];
        if( type == '/' ) {
            int sep = strcspn(++path, "/[@$");
            if( !sep ) type = path[0];
            else
            if( 1 ) { // path[ sep ] ) {
                char tag[32]; snprintf(tag, 32, "%.*s", sep, path);
                // Find the first sibling with the given tag name (may be the same node)
                struct xml *next = down ? xml_find_down(node, tag) : xml_find(node, tag);
                return xml_path(next, &path[ sep ], 1);
            }            
        }
        if( type == '$' ) {
            return (void*)( node->down ? xml_text( node->down ) : xml_tag( node ) );
        }
        if( type == '@' ) {
            return (void*)xml_att(node, ++path);
        }
        if( type == '[' ) {
            for( int i = 0, end = atoi(++path); i < end; ++i ) { node = xml_find_next(node, xml_tag(node)); if(!node) return NULL; }
            while( isdigit(path[0]) ) ++path;
            return xml_path(node, ++path, 1);
        }
    }
    return NULL;
}

const char *(xml_string)(char *key) {
    struct xml *node = xml_path(*array_back(xml_docs), key, 0);
    if( !node ) return "(null)";
    if( strchr(key, '@') ) return (const char *)node;
    if( strchr(key, '$') ) return (const char *)node;
    return "";
}
unsigned (xml_count)(char *key) {
    struct xml *node = xml_path(*array_back(xml_docs), key, 0);
    if( !node ) return 0;
    const char *tag = xml_tag(node);
    unsigned count = 1;
    while( (node = xml_find_next(node, tag)) != 0) ++count; 
    return count;
}
array(char) (xml_blob)(char *key) { // base64 blob
    struct xml *node = xml_path(*array_back(xml_docs), key, 0);
    if( !node ) return 0;
    if( !strchr(key, '$') ) return 0;
    const char *data = (const char*)node;
    array(char) out = base64__decode(data, strlen(data)); // either array of chars (ok) or null (error)
    return out;
}

bool data_tests() {
    // data tests (json5)
    const char json5[] =
    "  /* json5 */ // comment\n"
    "  abc: 42.67, def: true, integer:0x100 \n"
    "  huge: 2.2239333e5, \n"
    "  hello: 'world /*comment in string*/ //again', \n"
    "  children : { a: 1, b: 2, c: 3 },\n"
    "  array: [+1,2,-3,4,5],    \n"
    "  invalids : [ nan, NaN, -nan, -NaN, inf, Infinity, -inf, -Infinity ],";
    if( json_push(json5) ) {
        assert( json_float("/abc") == 42.67 );
        assert( json_int("/def") == 1 );
        assert( json_int("/integer") == 0x100 );
        assert( json_float("/huge") > 2.22e5 );
        assert( strlen(json_string("/hello")) == 35 );
        assert( json_int("/children/a") == 1 );
        assert( json_int("/children.b") == 2 );
        assert( json_int("/children[c]") == 3 );
        assert( json_int("/array[%d]", 2) == -3 );
        assert( json_count("/invalids") == 8 );
        assert( isnan(json_float("/invalids[0]")) );
        assert( !json_find("/non_existing") );
        assert( PRINTF("json5 tests OK\n") );
        json_pop();
    }

    // data tests (xml)
    const char *xml = // vfs_read("test1.xml");
    "<!-- XML representation of a person record -->"
    "<person created=\"2006-11-11T19:23\" modified=\"2006-12-31T23:59\">"
    "    <firstName>Robert</firstName>"
    "    <lastName>Smith</lastName>"
    "    <address type=\"home\">"
    "        <street>12345 Sixth Ave</street>"
    "        <city>Anytown</city>"
    "        <state>CA</state>"
    "        <postalCode>98765-4321</postalCode>"
    "    </address>"
    "</person>";
    if( xml_push(xml) ) {
        assert( !strcmp("Robert", xml_string("/person/firstName/$")) );
        assert( !strcmp("Smith", xml_string("/person/lastName/$")) );
        assert( !strcmp("home", xml_string("/person/address/@type")) );
        assert( PRINTF("xml tests OK\n") );
        xml_pop();
    }

    return true;
}

// compression api

static struct zcompressor {
    // id of compressor
    unsigned enumerator;
    // name of compressor
    const char name1, *name4, *name;
    // returns worst case compression estimation for selected flags
    unsigned (*bounds)(unsigned bytes, unsigned flags);
    // returns number of bytes written. 0 if error.
    unsigned (*encode)(const void *in, unsigned inlen, void *out, unsigned outcap, unsigned flags);
    // returns number of excess bytes that will be overwritten when decoding.
    unsigned (*excess)(unsigned flags);
    // returns number of bytes written. 0 if error.
    unsigned (*decode)(const void *in, unsigned inlen, void *out, unsigned outcap);
} zlist[] = {
    { COMPRESS_RAW,     '0', "raw",  "raw",     raw_bounds, raw_encode, raw_excess, raw_decode },
    { COMPRESS_PPP,     'p', "ppp",  "ppp",     ppp_bounds, ppp_encode, ppp_excess, ppp_decode },
    { COMPRESS_ULZ,     'u', "ulz",  "ulz",     ulz_bounds, ulz_encode, ulz_excess, ulz_decode },
    { COMPRESS_LZ4,     '4', "lz4x", "lz4x",    lz4x_bounds, lz4x_encode, lz4x_excess, lz4x_decode },
    { COMPRESS_CRUSH,   'c', "crsh", "crush",   crush_bounds, crush_encode, crush_excess, crush_decode },
    { COMPRESS_DEFLATE, 'd', "defl", "deflate", deflate_bounds, deflate_encode, deflate_excess, deflate_decode },
    { COMPRESS_LZP1,    '1', "lzp1", "lzp1",    lzp1_bounds, lzp1_encode, lzp1_excess, lzp1_decode },
    { COMPRESS_LZMA,    'm', "lzma", "lzma",    lzma_bounds, lzma_encode, lzma_excess, lzma_decode },
    { COMPRESS_BALZ,    'b', "balz", "balz",    balz_bounds, balz_encode, balz_excess, balz_decode },
    { COMPRESS_LZW3,    'w', "lzw3", "lzrw3a",  lzrw3a_bounds, lzrw3a_encode, lzrw3a_excess, lzrw3a_decode },
    { COMPRESS_LZSS,    's', "lzss", "lzss",    lzss_bounds, lzss_encode, lzss_excess, lzss_decode },
    { COMPRESS_BCM,     'B', "bcm",  "bcm",     bcm_bounds, bcm_encode, bcm_excess, bcm_decode },
    { COMPRESS_ZLIB,    'z', "zlib", "zlib",    deflate_bounds, deflatez_encode, deflate_excess, deflatez_decode },
};

enum { COMPRESS_NUM = 14 };

static char *znameof(unsigned flags) {
    static __thread char buf[16];
    snprintf(buf, 16, "%4s.%c", zlist[(flags>>4)&0x0F].name4, "0123456789ABCDEF"[flags&0xF]);
    return buf;
}
unsigned zencode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].encode(in, inlen, (uint8_t*)out, outlen, flags & 0x0F);
}
unsigned zdecode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].decode((uint8_t*)in, inlen, out, outlen);
}
unsigned zbounds(unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].bounds(inlen, flags & 0x0F);
}
unsigned zexcess(unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].excess(flags & 0x0F);
}
#line 0

#line 1 "fwk_dll.c"
#if is(win32)
#   include <winsock2.h>
#   define dlopen(name,mode)    (void*)( (name) ? LoadLibraryA(name) : GetModuleHandle(NULL))
#   define dlsym(handle,symbol) GetProcAddress((HMODULE)handle, symbol )
#   define dlclose(handle)      0
#else
#   include <dlfcn.h>
#endif

void* dll(const char *filename, const char *symbol) {
/*
    char *buf, *base = file_name(filename);
    if( file_exists(buf = va("%s", base)) ||
        file_exists(buf = va("%s.dll", base)) ||
        file_exists(buf = va("%s.so", base)) ||
        file_exists(buf = va("lib%s.so", base)) ||
        file_exists(buf = va("%s.dylib", base)) ) {
        filename = buf;
    }
*/
    void *dll = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
    dll = dll ? dlsym(dll, symbol) : 0;
    return dll;
}

#if 0 // demo: cl demo.c /LD && REM dll
EXPORT int add2(int a, int b) { return a + b; }
int main() { int (*adder)() = dll("demo.dll", "add2"); printf("%d\n", adder(2,3)); }
#endif
#line 0

#line 1 "fwk_file.c"
// -----------------------------------------------------------------------------
// file

#if 0 // ifdef _WIN32
#include <winsock2.h>
#if is(tcc)
    #define CP_UTF8 65001
    int WINAPI MultiByteToWideChar();
    int WINAPI WideCharToMultiByte();
#endif
// widen() ? string1252() ? string16() ? stringw() ?
wchar_t *widen(const char *utf8) { // wide strings (win only)
    int chars = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    char *buf = va("%.*s", (int)(chars * sizeof(wchar_t)), "");
    return MultiByteToWideChar(CP_UTF8, 0, utf8, -1, (void*)buf, chars), (wchar_t *)buf;
}
#define open8(path,mode)  ifdef(win, _wopen(widen(path))               ,  open(path, mode) )
#define fopen8(path,mode) ifdef(win, _wfopen(widen(path),widen(mode))  ,  fopen(path,mode) )
#define remove8(path)     ifdef(win, _wremove(widen(path))             ,  remove(path)     )
#define rename8(path)     ifdef(win, _wrename(widen(path))             ,  rename(path)     )
#define stat8(path,st)    ifdef(win, _wstat(widen(path),st)            ,  stat(path,st)    ) // _stati64()
#define stat8_t           ifdef(win, _stat                             ,  stat_t           ) // struct _stati64
#endif

char *file_name(const char *pathfile) {
    char *s = strrchr(pathfile, '/'), *t = strrchr(pathfile, '\\');
    return va("%s", s > t ? s+1 : t ? t+1 : pathfile);
}
char *file_base(const char *pathfile) {
    char *s = file_name(pathfile);
    char *e = file_ext(pathfile);
    return s[ strlen(s) - strlen(e) ] = '\0', s;
}
char *file_pathabs( const char *pathfile ) {
    char *out = va("%*.s", DIR_MAX+1, "");
#if is(win32)
    _fullpath(out, pathfile, DIR_MAX);
#else
    realpath(pathfile, out);
#endif
    return out;
}
char *file_path(const char *pathfile) {
    return va("%.*s", (int)(strlen(pathfile)-strlen(file_name(pathfile))), pathfile);
}
char *file_load(const char *filename, int *len) { // @todo: 32 counters/thread enough?
    static __thread array(char) buffer[32] = {0}, *invalid[1];
    static __thread unsigned i = 0; i = (i+1) % 32;

    FILE *fp = filename[0] ? fopen(filename, "rb") : NULL;
    if( fp ) {
        fseek(fp, 0L, SEEK_END);
        size_t sz = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        array_resize(buffer[i], sz+1);
        sz *= fread(buffer[i],sz,1,fp) == 1;
        buffer[i][sz] = 0;
        if(len) *len = (int)sz;
        fclose(fp);
        return buffer[i]; // @fixme: return 0 on error instead?
    }
    if (len) *len = 0;
    return 0;
}
char *file_read(const char *filename) { // @todo: fix leaks
    return file_load(filename, NULL);
}
bool file_write(const char *name, const void *ptr, int len) {
    bool ok = 0;
    for( FILE *fp = name && ptr && len >= 0 ? fopen(name, "wb") : NULL; fp; fclose(fp), fp = 0) {
        ok = fwrite(ptr, len,1, fp) == 1;
    }
    return ok;
}
bool file_append(const char *name, const void *ptr, int len) {
    bool ok = 0;
    for( FILE *fp = name && ptr && len >= 0 ? fopen(name, "a+b") : NULL; fp; fclose(fp), fp = 0) {
        ok = fwrite(ptr, len,1, fp) == 1;
    }
    return ok;
}
static bool file_stat(const char *fname, struct stat *st) {
    // remove ending slashes. win32+tcc does not like them.
    int l = strlen(fname), m = l;
    while( l && (fname[l-1] == '/' || fname[l-1] == '\\') ) --l;
    fname = l == m ? fname : va("%.*s", l, fname);
    return stat(fname, st) >= 0;
}
uint64_t file_stamp(const char *fname) {
    struct stat st;
    return !file_stat(fname, &st) ? 0ULL : st.st_mtime;
}
uint64_t file_stamp10(const char *fname) {
    time_t mtime = (time_t)file_stamp(fname);
    struct tm *ti = localtime(&mtime);
    return atoi64(va("%04d%02d%02d%02d%02d%02d",ti->tm_year+1900,ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec));
}
uint64_t file_size(const char *fname) {
    struct stat st;
    return !file_stat(fname, &st) ? 0ULL : st.st_size;
}
bool file_directory( const char *pathfile ) {
    struct stat st;
    return !file_stat(pathfile, &st) ? 0 : S_IFDIR == ( st.st_mode & S_IFMT );
}
bool file_exist(const char *fname) {
    struct stat st;
    return !file_stat(fname, &st) ? false : true;
}
char *file_normalize(const char *name) {
    char *copy = va("%s", name), *s = copy, c;
#if is(win32)
    for( int i = 0; copy[i]; ++i ) { if(copy[i] == '/') copy[i] = '\\'; if(copy[i] == '\'') copy[i] = '\"'; }
#else
    for( int i = 0; copy[i]; ++i ) { if(copy[i] == '\\') copy[i] = '/'; if(copy[i] == '\"') copy[i] = '\''; }
#endif
    return copy;
}
#if 0
char *file_normalize(const char *name) {
    char *copy = va("%s", name), *s = copy, c;
    // lowercases+digits+underscores+slashes only. anything else is truncated.
    for( ; *name ; ++name ) {
        /**/ if( *name >= 'a' && *name <= 'z' ) *s++ = *name;
        else if( *name >= 'A' && *name <= 'Z' ) *s++ = *name - 'A' + 'a';
        else if( *name >= '0' && *name <= '9' ) *s++ = *name;
        else if( *name == '/' || *name == '\\') *s++ = '/';
        else if( *name <= ' ' || *name == '.' ) *s++ = '_';
    } *s++ = 0;
    // remove dupe slashes
    for( name = s = copy, c = '/'; *name ; ) {
        while( *name && *name != c ) *s++ = *name++;
        if( *name ) *s++ = c;
        while( *name && *name == c ) name++;
    } *s++ = 0;
    // remove dupe underlines
    for( name = s = copy, c = '_'; *name ; ) {
        while( *name && *name != c ) *s++ = *name++;
        if( *name ) *s++ = c;
        while( *name && *name == c ) name++;
    } *s++ = 0;
    return copy;
}
char *file_normalize_with_folder(const char *name) {
    char *s = file_normalize(name);
    char *slash = strrchr(s, '/'); if(slash) *slash = 0;
    char *penultimate = strrchr(s, '/'); if(slash) *slash = '/';
    return penultimate ? penultimate+1 : /*slash ? slash+1 :*/ s;
}
#endif
char *file_ext(const char *name) {
    char *b = file_name(name), *s = strchr(b, '.'); //strrchr(name, '.');
    return va("%s", s ? s : "" ); // , name );
}

char *file_id(const char *pathfile) {
    char *dir = file_path(pathfile); for(int i=0;dir[i];++i) dir[i]=tolower(dir[i]);
    char *base = file_name(pathfile); for(int i=0;base[i];++i) base[i]=tolower(base[i]);
#if 0 // extensionless, larry.mid and larry.txt will collide, diffuse.tga and diffuse.png will match
char *ext = strchr(base, '.'); if (ext) ext[0] = '\0'; // remove all extensions
#else // extensionless for audio/images only (materials: diffuse.tga and diffuse.png will match)
char *ext = strrchr(base, '.'); //if (ext) ext[0] = '\0'; // remove all extensions
    if(ext) if( strstr(".jpg.png.bmp.tga.hdr"".", ext) || strstr(".ogg.mp3.wav.mod.xm.flac"".", ext) || strstr(".mp4.ogv.avi.mkv.wmv.mpg.mpeg"".", ext) ) {
        ext = strchr(base, '.');
        ext[0] = '\0'; //strcpy(ext, "_xxx");
    }
#endif

// if (!dir[0]) return base;
    char *stem = va("%s/%s", dir, base); // file_name(dir);

    // /path2/path1/file2_file1 -> file1_file2/path1/path2
    int len = 0;
    int ids_count = 0;
    char ids[64][64] = { 0 };
    // split path stems
    for each_substring(stem, "/\\@", key) {
        int tokens_count = 0;
        char* tokens[64] = { 0 };
        // split tokens
        for each_substring(key, "[]()_ ", it) {
            tokens[tokens_count++] = va("%s", it);
        }
        // sort alphabetically
        if( tokens_count > 1 ) qsort(tokens, tokens_count, sizeof(char *), strcmp_qsort);
        // concat sorted token1_token2_...
        char built[256]; *built = 0;
        for( int i = 0; i < tokens_count; ++i ) {
            strlcat(built, "_", 256);
            strlcat(built, tokens[i], 256);
        }
        strncpy( ids[ ids_count ], &built[1], 64 );
        len += strlen( ids[ ids_count++ ] );
    }
    // concat in inverse order: file/path1/path2/...
    char buffer[DIR_MAX]; buffer[0] = 0;
    for( int it = ids_count; --it >= 0; ) {
        strcat(buffer, ids[it]);
        strcat(buffer, "/");
    }
    return va("%s", buffer);
}
const char** file_list(const char *cwd, const char *masks) {
    ASSERT(strend(cwd, "/"), "Error: dirs like '%s' must end with slash", cwd);

    static __thread array(char*) list = 0;
    const char *arg0 = cwd; // app_path();
    int larg0 = strlen(arg0);

    for( int i = 0; i < array_count(list); ++i ) {
        FREE(list[i]);
    }
    array_resize(list, 0);//array_free(list);

    for each_substring(masks,";",it) {
        int recurse = !!strstr(it, "**");
        #if is(win32)
        char *glob = va("dir %s/b/o:n \"%s\\%s\" 2> NUL", recurse ? "/s":"", cwd, it);
        #else // linux, osx
        char *glob = va("find %s %s -name \"%s\" | sort", cwd, !recurse ? "-maxdepth 1":"-type f", it);
        #endif
        for( FILE *in = popen(glob, "r"); in; pclose(in), in = 0) {
            char buf[1024], *line = buf;
            while( fgets(buf, sizeof(buf), in) ) {
                // clean up
                if( strstr(line, arg0) ) line = buf + larg0;
                if( !memcmp(line, "./", 2) ) line += 2;
                int len = strlen(line); while( len > 0 && line[len-1] < 32 ) line[--len] = 0;
                if( line[0] == '\0' ) continue;
                // do not insert system folders/files
                for(int i = 0; i < len; ++i ) if(line[i] == '\\') line[i] = '/';
                if( line[0] == '.' ) if( !strcmp(line,".git") || !strcmp(line,".vs") || !strcmp(line,".") || !strcmp(line,"..") ) continue;
                if( strstr(line, "/.") ) continue;
                // insert copy
                #if is(win32)
                char *copy = STRDUP(line); // full path already provided
                #else
                // while(line[0] == '/') ++line;
                char *copy = STRDUP(va("%s%s", cwd, line)); // need to prepend path
                #endif
                array_push(list, copy);
            }
        }
    }
    array_push(list, 0); // terminator
    return (const char**)list;
}

bool file_move(const char *src, const char *dst) {
    bool ok = file_exist(src) && !file_exist(dst) && 0 == rename(src, dst);
    return ok;
}
bool file_delete(const char *pathfile) {
    if( file_exist(pathfile) ) {
        for( int i = 0; i < 10; ++i ) {
            bool ok = 0 == unlink(pathfile);
            if( ok ) return true;
            sleep_ms(10);
        }
        return false;
    }
    return true;
}
bool file_copy(const char *src, const char *dst) {
    int ok = 0, BUFSIZE = 1 << 20; // 1 MiB
    static __thread char *buffer = 0; do_once buffer = REALLOC(0, BUFSIZE);
    for( FILE *in = fopen(src, "rb"); in; fclose(in), in = 0) {
        for( FILE *out = fopen(dst, "wb"); out; fclose(out), out = 0, ok = 1) {
            for( int n; !!(n = fread( buffer, 1, BUFSIZE, in )); ){
                if(fwrite( buffer, 1, n, out ) != n)
                return fclose(in), fclose(out), false;
            }
        }
    }
    return ok;
}

char* file_tempname() {
    static __thread int id;
    return va("%s/fwk-temp.%s.%p.%d", app_temp(), getenv(ifdef(win32, "username", "USER")), &id, rand());
}
FILE *file_temp(void) {
    const char *fname = file_tempname();
    FILE *fp = fopen(fname, "w+b");
    if( fp ) unlink(fname);
    return fp;
}

char *file_counter(const char *name) {
    static __thread char outfile[DIR_MAX], init = 0;
    static __thread map(char*, int) ext_counters;
    if(!init) map_init(ext_counters, less_str, hash_str), init = '\1';

    char *base = va("%s",name), *ext = file_ext(name); 
    if(ext && ext[0]) *strstr(base, ext) = '\0';

    int *counter = map_find_or_add(ext_counters, ext, 0);
    while( *counter >= 0 ) {
        *counter = *counter + 1;
        sprintf(outfile, "%s(%03d)%s", base, *counter, ext);
        if( !file_exist(outfile) ) {
            return va("%s", outfile);
        }
    }
    return 0;
}

enum { MD5_HASHLEN = 16 };
enum { SHA1_HASHLEN = 20 };
enum { CRC32_HASHLEN = 4 };

void* file_sha1(const char *file) { // 20bytes
    hash_state hs = {0};
    sha1_init(&hs);
    for( FILE *fp = fopen(file, "rb"); fp; fclose(fp), fp = 0) {
        char buf[8192];
        for( int inlen; (inlen = fread(buf, 1, sizeof(buf), fp)) > 0; ) {
            sha1_process(&hs, (const unsigned char *)buf, inlen);
        }
    }
    unsigned char *hash = va("%.*s", SHA1_HASHLEN, "");
    sha1_done(&hs, hash);
    return hash;
}

void* file_md5(const char *file) { // 16bytes
    hash_state hs = {0};
    md5_init(&hs);
    for( FILE *fp = fopen(file, "rb"); fp; fclose(fp), fp = 0) {
        char buf[8192];
        for( int inlen; (inlen = fread(buf, 1, sizeof(buf), fp)) > 0; ) {
            md5_process(&hs, (const unsigned char *)buf, inlen);
        }
    }
    unsigned char *hash = va("%.*s", MD5_HASHLEN, "");
    md5_done(&hs, hash);
    return hash;
}

void* file_crc32(const char *file) { // 4bytes
    unsigned crc = 0;
    for( FILE *fp = fopen(file, "rb"); fp; fclose(fp), fp = 0) {
        char buf[8192];
        for( int inlen; (inlen = fread(buf, 1, sizeof(buf), fp)) > 0; ) {
            crc = zip__crc32(crc, buf, inlen); // unsigned int stbiw__crc32(unsigned char *buffer, int len)
        }
    }
    unsigned char *hash = va("%.*s", (int)sizeof(crc), "");
    memcpy(hash, &crc, sizeof(crc));
    return hash;
}

#if 0
void* crc32_mem(const void *ptr, int inlen) { // 4bytes
    unsigned hash = 0;
    hash = zip__crc32(hash, ptr, inlen); // unsigned int stbiw__crc32(unsigned char *buffer, int len)
    return hash;
}
void* md5_mem(const void *ptr, int inlen) { // 16bytes
    hash_state hs = {0};
    md5_init(&hs);
    md5_process(&hs, (const unsigned char *)ptr, inlen);
    unsigned char *hash = va("%.*s", MD5_HASHLEN, "");
    md5_done(&hs, hash);
    return hash;
}
void* sha1_mem(const void *ptr, int inlen) { // 20bytes
    hash_state hs = {0};
    sha1_init(&hs);
    sha1_process(&hs, (const unsigned char *)ptr, inlen);
    unsigned char *hash = va("%.*s", SHA1_HASHLEN, "");
    sha1_done(&hs, hash);
    return hash;
}
#endif

// -----------------------------------------------------------------------------
// storage (emscripten only)

void storage_mount(const char* folder) {
    #if is(ems)
        emscripten_run_script(va("FS.mkdir('%s'); FS.mount(IDBFS, {}, '%s');", folder, folder));
    #else
        (void)folder;
    #endif
}

void storage_read() {
    #if is(ems)
    EM_ASM(
        /* sync from persisted state into memory */
        FS.syncfs(true, function (err) {
            assert(!err);
        });
    );
    #endif
}

void storage_flush() {
    #if is(ems)
    EM_ASM(
        FS.syncfs(false, function (err) {
            assert(!err);
        });
    );
    #endif
}

// -----------------------------------------------------------------------------
// compressed archives

// return list of files inside zipfile
array(char*) file_zip_list(const char *zipfile) {
    static __thread array(char*) list[16] = {0};
    static __thread int count = 0;

    count = (count+1) % 16;
    array_resize(list[count], 0);

    for( zip *z = zip_open(zipfile, "rb"); z; zip_close(z), z = 0) {
        for( unsigned i = 0; i < zip_count(z); ++i ) {
            array_push( list[count], zip_name(z, i) );
        }
    }

    return list[count];
}

// extract single file content from zipfile
array(char) file_zip_extract(const char *zipfile, const char *filename) {
    static __thread array(char) list[16] = {0};
    static __thread int count = 0;

    array(char) out = list[count = (count+1) % 16];
    array_resize(out, 0);

    for( zip *z = zip_open(zipfile, "rb"); z; zip_close(z), z = 0) {
        int index = zip_find(z, filename); // convert entry to index. returns <0 if not found.
        if( index < 0 ) return zip_close(z), out;

        unsigned outlen = zip_size(z, index);
        unsigned excess = zip_excess(z, index);
        array_resize(out, outlen + 1 + excess);
        unsigned ret = zip_extract_inplace(z, index, out, array_count(out));
        if(ret) { out[outlen] = '\0'; array_resize(out, outlen); } else { array_resize(out, 0); }
    }

    return out;
}

// append single file into zipfile. compress with DEFLATE|6. Other compressors are also supported (try LZMA|5, ULZ|9, LZ4X|3, etc.)
bool file_zip_append(const char *zipfile, const char *filename, int clevel) {
    bool ok = false;
    for( zip *z = zip_open(zipfile, "a+b"); z; zip_close(z), z = 0) {
        for( FILE *fp = fopen(filename, "rb"); fp; fclose(fp), fp = 0) {
            ok = zip_append_file(z, filename, "", fp, clevel);
        }
    }
    return ok;
}

// append mem blob into zipfile. compress with DEFLATE|6. Other compressors are also supported (try LZMA|5, ULZ|9, LZ4X|3, etc.)
// @fixme: implement zip_append_mem() and use that instead
bool file_zip_appendmem(const char *zipfile, const char *entryname, const void *ptr, unsigned len, int clevel) {
    bool ok = false;
    if( ptr )
    for( zip *z = zip_open(zipfile, "a+b"); z; zip_close(z), z = 0) {
        ok = zip_append_mem(z, entryname, "", ptr, len, clevel);
    }
    return ok;
}

// -----------------------------------------------------------------------------
// archives

enum { is_zip, is_tar, is_pak, is_dir };

typedef struct archive_dir {
    char* path;
    union {
        int type;
        int size; // for cache only
    };
    union {
        void *archive;
        void *data; // for cache only
        zip* zip_archive;
        tar* tar_archive;
        pak* pak_archive;
    };
    struct archive_dir *next;
} archive_dir;

static archive_dir *dir_mount;
static archive_dir *dir_cache; 

#ifndef MAX_CACHED_FILES    // @todo: should this be MAX_CACHED_SIZE (in MiB) instead?
#define MAX_CACHED_FILES 32 // @todo: should we cache the cooked contents instead? ie, stbi() result instead of file.png?
#endif

struct vfs_entry {
    const char *name;
    const char *id;
    unsigned size;
};
array(struct vfs_entry) vfs_hints;   // mounted raw assets
array(struct vfs_entry) vfs_entries; // mounted cooked assets

static bool vfs_mount_hints(const char *path);
static
void vfs_reload() {
    const char *app = app_name();

    array_resize(vfs_hints, 0); // @leak
    array_resize(vfs_entries, 0); // @leak

    // mount virtual filesystems later (mounting order matters: low -> to -> high priority)
#if defined(EMSCRIPTEN)
    vfs_mount("index.zip");
#else
    /* // old way
    for( int i = 0; i < JOBS_MAX; ++i) {
        if( vfs_mount(va(".art[%02x].zip", i)) ) continue;
        if( vfs_mount(va("%s[%02x].zip", app, i)) ) continue;
        if( vfs_mount(va("%s%02x.zip", app, i)) ) continue;
        // if( vfs_mount(va("%s.%02x", app, i)) ) continue;
    } */
    // faster way
    for( const char **file = file_list("./","*.zip"); *file; ++file) vfs_mount(*file);
#endif

    // vfs_resolve() will use these art_folder locations as hints when cook-on-demand is in progress.
    // cook-on-demand will not be able to resolve a virtual pathfile if there are no cooked assets on disk,
    // unless there is a record of what's actually on disk somewhere, and that's where the hints belong to.
    for each_substring(ART,",",art_folder) {
        vfs_mount_hints(art_folder);
    }
}

static
bool vfs_mount_(const char *path, array(struct vfs_entry) *entries) {
    zip *z = NULL; tar *t = NULL; pak *p = NULL; dir *d = NULL;
    int is_folder = ('/' == path[strlen(path)-1]);
    if( is_folder ) d = dir_open(path, "rb");
    if( is_folder && !d ) return 0;
    if( !is_folder ) z = zip_open(path, "rb");
    if( !is_folder && !z ) t = tar_open(path, "rb");
    if( !is_folder && !z && !t ) p = pak_open(path, "rb");
    if( !is_folder && !z && !t && !p ) return 0;

    // normalize input -> "././" to ""
    while (path[0] == '.' && path[1] == '/') path += 2;
    path = STRDUP(path);
    if( z || t || p ) {
    // save local path for archives, so we can subtract those from upcoming requests
    if(strrchr(path,'/')) strrchr(path,'/')[1] = '\0';
    } else if(d) 0[(char*)path] = 0;

    // append to mounted archives
    archive_dir *prev = dir_mount, zero = {0};
    *(dir_mount = REALLOC(0, sizeof(archive_dir))) = zero;
    dir_mount->next = prev;
    dir_mount->path = (char*)path;
    dir_mount->archive = z ? (void*)z : t ? (void*)t : p ? (void*)p : (void*)d;
    dir_mount->type = is_folder ? is_dir : z ? is_zip : t ? is_tar : p ? is_pak : -1;
    ASSERT(dir_mount->type >= 0 && dir_mount->type < 4);

    // append list of files to internal listing
    for( archive_dir *dir = dir_mount; dir ; dir = 0 ) { // for(archive_dir *dir = dir_mount; dir; dir = dir->next) {
        assert(dir->type >= 0 && dir->type < 4);
        unsigned (*fn_count[4])(void*) = {(void*)zip_count, (void*)tar_count, (void*)pak_count, (void*)dir_count};
        char*    (*fn_name[4])(void*, unsigned index) = {(void*)zip_name, (void*)tar_name, (void*)pak_name, (void*)dir_name};
        unsigned (*fn_size[4])(void*, unsigned index) = {(void*)zip_size, (void*)tar_size, (void*)pak_size, (void*)dir_size};

        for( unsigned idx = 0, end = fn_count[dir->type](dir->archive); idx < end; ++idx ) {
            assert(idx < end);
            const char *filename = STRDUP( fn_name[dir->type](dir->archive, idx) );
            const char *fileid = STRDUP( file_id(filename) );
            unsigned filesize = fn_size[dir->type](dir->archive, idx);
            // printf("%u) %s %u [%s]\n", idx, filename, filesize, fileid);
            // append to list
            array_push(*entries, (struct vfs_entry){filename, fileid, filesize});
        }
    }

    return 1;
}
static
bool vfs_mount_hints(const char *path) {
    return vfs_mount_(path, &vfs_hints);
}
bool vfs_mount(const char *path) {
    return vfs_mount_(path, &vfs_entries);
}
const char** vfs_list(const char *masks) {
    static __thread array(char*) list = 0;

    for( int i = 0; i < array_count(list); ++i ) {
        FREE(list[i]);
    }
    array_free(list);

    for each_substring(masks,";",it) {
        if( COOK_ON_DEMAND ) // edge case: any game using only vfs api + cook-on-demand flag will never find any file
        for(const char **items = file_list("./", it); *items; items++) {
            // insert copy
            char *copy = STRDUP(*items);
            array_push(list, copy);
        }

        it = va("*/%s", it);
        // int recurse = !!strstr(it, "**"); // @fixme: support non-recursive
        for( unsigned i = 0; i < array_count(vfs_entries); ++i ) {
            const char *name = vfs_entries[i].name;
            if( strmatch(name, it) ) {
                // insert copy
                char *copy = STRDUP(name);
                array_push(list, copy);
            }
        }
    }

    // sort alphabetically then remove dupes
    array_sort(list, strcmp);
    array_unique(list, strcmp_qsort);

    array_push(list, 0); // terminator
    return (const char**)list;
}

static
char *vfs_unpack(const char *pathfile, int *size) { // must free() after use
    // @todo: add cache here
    char *data = NULL;
    for(archive_dir *dir = dir_mount; dir && !data; dir = dir->next) {
        if( dir->type == is_dir ) {
#if 0 // sandboxed
            char buf[DIR_MAX];
            snprintf(buf, sizeof(buf), "%s%s", dir->path, pathfile);
            data = file_load(buf, size);
#endif
        } else {
            int      (*fn_find[3])(void *, const char *) = {(void*)zip_find, (void*)tar_find, (void*)pak_find};
            void*    (*fn_unpack[3])(void *, unsigned) = {(void*)zip_extract, (void*)tar_extract, (void*)pak_extract};
            unsigned (*fn_size[3])(void *, unsigned) = {(void*)zip_size, (void*)tar_size, (void*)pak_size};

#if 0
            const char* cleanup = pathfile + strbegi(pathfile, dir->path) * strlen(dir->path);
            while (cleanup[0] == '/') ++cleanup;
#else
            const char *cleanup = pathfile;
#endif

            int index = fn_find[dir->type](dir->archive, cleanup);
            data = fn_unpack[dir->type](dir->archive, index);
            if( size ) *size = fn_size[dir->type](dir->archive, index);
        }
        // printf("%c trying %s in %s ...\n", data ? 'Y':'N', pathfile, dir->path);
    }
    //wait_ms(1000); // <-- simulate slow hdd
    return data;
}

const char *vfs_resolve(const char *pathfile) {
    // we dont resolve absolute paths. they dont belong to the vfs
    // if( pathfile[0] == '/' || pathfile[0] == '\\' || pathfile[1] == ':' ) return pathfile;

    char* id = file_id(pathfile);
    // find best match (vfs_entries first)
    for (int i = array_count(vfs_entries); --i >= 0; ) {
        if (strbegi(vfs_entries[i].id, id) ) {
            return vfs_entries[i].name;
        }
    }
    // find best match (vfs_hints later)
    for (int i = array_count(vfs_hints); --i >= 0; ) {
        if (strbegi(vfs_hints[i].id, id) ) {
            return vfs_hints[i].name;
        }
    }
    return pathfile;
}

char* vfs_load(const char *pathfile, int *size_out) { // @todo: fix leaks, vfs_unpack()
    // @fixme: handle \\?\ absolute path (win)
    if (!pathfile[0]) return file_load(pathfile, size_out);
    while( pathfile[0] == '.' && (pathfile[1] == '/' || pathfile[1] == '\\') ) pathfile += 2;
    // if (pathfile[0] == '/' || pathfile[1] == ':') return file_load(pathfile, size_out); // @fixme: handle current cooked /home/FWK or C:/FWK path cases within zipfiles

if( size_out ) *size_out = 0;
if( strend(pathfile, "/") ) return 0; // it's a dir
static __thread map(char*,int) misses = 0, *init = 0; if(!init) init = misses, map_init(misses, less_str, hash_str);
int *found = map_find_or_add_allocated_key(misses, STRDUP(pathfile), -1); // [-1]non-init,[false]could not cook,[true]cooked
if( found && *found == 0 ) {
    return 0;
}

    //{
    // exclude garbage from material names
    // @todo: exclude double slashs in paths
    char *base = file_name(pathfile); if(strchr(base,'+')) base = strchr(base, '+')+1;
    if(base[0] == '\0') return 0; // it's a dir
    char *folder = file_path(pathfile);
    pathfile = va("%s%s", folder, base);

    // solve virtual path
    pathfile = va("%s", vfs_resolve(pathfile));
    base = file_name(pathfile);
    if(base[0] == '\0') return 0; // it's a dir
    folder = file_path(pathfile);
        // make folder variable easier to read in logs: /home/rlyeh/prj/fwk/art/demos/audio/coin.wav -> demos/audio/coin.wav 
        // static int ART_LEN = 0; do_once ART_LEN = strlen(ART);
        // if( !strncmp(folder, ART, ART_LEN) ) {
        //     folder += ART_LEN;
        // }
        char* pretty_folder = folder && strlen(folder) >= ART_LEN ? folder + ART_LEN : "";
    //}

    int size = 0;
    void *ptr = 0;

#if 0
    // clean pathfile
    while (pathfile[0] == '.' && pathfile[1] == '/') pathfile += 2;
    while (pathfile[0] == '/') ++pathfile;
#endif

    const char *lookup_id = /*file_normalize_with_folder*/(pathfile);

    // search (last item)
    static char last_item[256] = { 0 };
    static void *last_ptr = 0;
    static int   last_size = 0;
    if( !strcmpi(lookup_id, last_item)) {
        ptr = last_ptr;
        size = last_size;
    }

    // search (cache)
    if( !ptr ) {
        ptr = cache_lookup(lookup_id, &size);
    }

    if( ptr ) {
        PRINTF("Loading VFS (%s)%s (cached)\n", pretty_folder, base);
    } else {
        PRINTF("Loading VFS (%s)%s\n", pretty_folder, base);
    }

    // read cooked asset from mounted disks
    if( !ptr ) {
        ptr = vfs_unpack(pathfile, &size);

        // asset not found? maybe it has not been cooked yet at this point (see --cook-on-demand)
        if( !ptr && COOK_ON_DEMAND ) {

            static thread_mutex_t mutex, *init = 0; if(!init) thread_mutex_init(init = &mutex);
            thread_mutex_lock(&mutex);

// this block saves some boot time (editor --cook-on-demand: boot 1.50s -> 0.90s)
#if 1 // EXPERIMENTAL_DONT_COOK_NON_EXISTING_ASSETS
            static set(char*) disk = 0;
            if(!disk) { set_init_str(disk); for each_substring(ART,",",art_folder) for( const char **list = file_list(art_folder,"**"); *list; ++list) set_insert(disk, STRDUP(*list)); }
            int found = !!set_find(disk, (char*)pathfile);
            if( found )
#endif
            {
                // technically, we should only cook `base` asset at this point. however, cooks on demand can be very
                // expensive. not only because we're invoking an external tool/cook app in here, which is scanning all the
                // cook folders at every call, but also because there is this expensive vfs_reload() call at end of current scope.
                // so, in order to minimize the number of upcoming cook requests, we cook more stuff than needed at this point;
                // just in anticipation of what's likely going to happen in the next frames.
                // so, when asked to cook "models/model.fbx" we actually:
                // - do cook "models/model.* (so, "model.tga","model.png","model.jpg","model.mtl",etc. are also cooked)
                // - do cook "models/*.fbx" as well
                char *dir = file_path(pathfile + ART_SKIP_ROOT);
                char *group1 = dir[0] ? va("\"*/%s%s.*\"", dir, file_base(pathfile)) : base; // -> "*/models/model.*"
                char *group2 = dir[0] ? va("\"*/%s*%s\"", dir, file_ext(pathfile)) : ""; // -> "*/models/*.fbx"

                char *cmd = va("%scook" ifdef(osx,".osx",ifdef(linux,".linux",".exe"))" %s %s --cook-ini=%s --cook-additive --cook-jobs=1 --quiet", TOOLS, group1, group2, COOK_INI);

                // cook groups
                int rc = system(cmd);
                if(rc < 0) PANIC("cannot invoke `%scook` (return code %d)", TOOLS, rc);

                vfs_reload(); // @todo: optimize me. it is waaay inefficent to reload the whole VFS layout after cooking a single asset
            }

            thread_mutex_unlock(&mutex);

            // finally, try again
            pathfile = va("%s", vfs_resolve(pathfile));
            ptr = vfs_unpack(pathfile, &size);
        }

        if( ptr ) {
            cache_insert(lookup_id, ptr, size);
        }
    }

    if( ptr && size )
    if( ptr != last_ptr) {
        snprintf(last_item, 256, "%s", lookup_id);
        last_ptr = ptr;
        last_size = size;
    }

    // yet another last resort: redirect vfs_load() calls to file_load()
    // (for environments without tools or cooked assets)
    if(!ptr) {
        static bool have_tools; do_once have_tools = file_exist(COOK_INI);
        if( !have_tools ) {
            ptr = file_load(pathfile, size_out);
        }
    }

    if(!ptr) {
        PRINTF("Loading %s (not found)\n", pathfile);
    }

*found = ptr ? true : false;

    if( size_out ) *size_out = ptr ? size : 0;
    return ptr;
}
char* vfs_read(const char *pathfile) {
    return vfs_load(pathfile, NULL);
}
int vfs_size(const char *pathfile) {
    int sz;
    return vfs_load(pathfile, &sz), sz;
}


FILE* vfs_handle(const char *pathfile) {
    // @fixme: non-compressible assets (adpcm wavs,mp3,ogg,mp4,avi,...) are taking a suboptimal code path here.
    //         no need to unzip them. just seek within the zipfile and return the *fp at that position
    int sz;
    char *buf = vfs_load(pathfile, &sz);
    FILE *fp = fmemopen(buf ? buf : "", buf ? sz : 0, "r+b");
    ASSERT( fp, "cannot create tempfile" );
    return fp;
}
#if 0
const char *vfs_extract(const char *pathfile) { // extract a vfs file into the local (temp) filesystem
#if 0
    FILE* fp = vfs_handle(pathfile);
    return fp ? pathfile_from_handle(fp) : "";
#else
    int sz;
    char *buf = vfs_load(pathfile, &sz);
    if( !buf ) return "";

    // pool of temp files. recycles after every loop
    enum { MAX_TEMP_FILES = 16 };
    static __thread char temps[MAX_TEMP_FILES][DIR_MAX] = {0};
    static __thread int i = 0;
    if( temps[i][0] ) unlink(temps[i]);
    i = (i + 1) % MAX_TEMP_FILES;
    if(!temps[i][0] ) snprintf(temps[i], DIR_MAX, "%s", file_tempname());
    char *name = temps[i];

    FILE *tmp = fopen(name, "wb"); //unlink(name);
    ASSERT( tmp, "cannot create tempfile %s", name );
    fwrite(buf ? buf : "", 1, buf ? sz : 0, tmp);
    fclose(tmp);
    return name;
#endif
}
#endif


// -----------------------------------------------------------------------------
// cache

void* cache_lookup(const char *pathfile, int *size) { // find key->value
    if( !MAX_CACHED_FILES ) return 0;
    for(archive_dir *dir = dir_cache; dir; dir = dir->next) {
        if( !strcmp(dir->path, pathfile) ) {
            if(size) *size = dir->size;
            return dir->data;
        }
    }
    return 0;
}
void* cache_insert(const char *pathfile, void *ptr, int size) { // append key/value; return LRU or NULL
    if( !MAX_CACHED_FILES ) return 0;
    if( !ptr || !size ) return 0;

    // append to cache
    archive_dir zero = {0}, *old = dir_cache;
    *(dir_cache = REALLOC(0, sizeof(archive_dir))) = zero;
    dir_cache->next = old;
    dir_cache->path = STRDUP(pathfile);
    dir_cache->size = size;
    dir_cache->data = REALLOC(0, size+1);
    memcpy(dir_cache->data, ptr, size); size[(char*)dir_cache->data] = 0; // copy+terminator

    // keep cached files within limits
    static int added = 0;
    if( added < MAX_CACHED_FILES ) {
        ++added;
    } else {
        // remove oldest cache entry
        for( archive_dir *prev = dir_cache, *dir = prev; dir ; prev = dir, dir = dir->next ) {
            if( !dir->next ) {
                prev->next = 0; // break link
                void *data = dir->data;
                dir->path = REALLOC(dir->path, 0);
                dir->data = REALLOC(dir->data, 0);
                dir = REALLOC(dir, 0);
                return data;
            }
        }
    }
    return 0;
}

// ----------------------------------------------------------------------------
// ini


/* ini+, extended ini format
// - rlyeh, public domain
//
// # spec
//
//   ; line comment
//   [user]             ; map section name (optional)
//   name=john          ; key and value (mapped here as user.name=john)
//   +surname=doe jr.   ; sub-key and value (mapped here as user.name.surname=doe jr.)
//   age=30             ; numbers
//   color=240          ; key and value \
//   color=253          ; key and value |> array: color[0], color[1] and color[2]
//   color=255          ; key and value /
//   color=             ; remove key/value(s)
//   color=white        ; recreate key; color[1] and color[2] no longer exist
//   []                 ; unmap section
//   -note=keys may start with symbols (except plus and semicolon)
//   -note=linefeeds are either \r, \n or \r\n.
//   -note=utf8 everywhere.
*/

static
char *ini_parse( const char *s ) {
    char *map = 0;
    int mapcap = 0, maplen = 0;
    enum { DEL, REM, TAG, KEY, SUB, VAL } fsm = DEL;
    const char *cut[6] = {0}, *end[6] = {0};
    while( *s ) {
        while( *s && (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') ) ++s;
        /**/ if( *s == ';' ) cut[fsm = REM] = ++s;
        else if( *s == '[' ) cut[fsm = TAG] = ++s;
        else if( *s == '+' ) cut[fsm = SUB] = ++s;
        else if( *s == '=' ) cut[fsm = VAL] = ++s;
        else if( *s > ' ' && *s <= 'z' && *s != ']' ) cut[fsm = KEY] = cut[SUB] = end[SUB] = s;
        else { if( *s ) ++s; continue; }
        /**/ if( fsm == REM ) { while(*s && *s != '\r'&& *s != '\n') ++s; }
        else if( fsm == TAG ) { while(*s && *s != '\r'&& *s != '\n'&& *s != ']') ++s; end[TAG] = s; }
        else if( fsm == KEY ) { while(*s && *s >  ' ' && *s <= 'z' && *s != '=') ++s; end[KEY] = s; }
        else if( fsm == SUB ) { while(*s && *s >  ' ' && *s <= 'z' && *s != '=') ++s; end[SUB] = s; }
        else if( fsm == VAL ) { while(*s && *s >= ' ' && *s <= 127 && *s != ';') ++s; end[VAL] = s;
            while( end[VAL][-1] <= ' ' ) { --end[VAL]; }
            char buf[256] = {0}, *key = buf;
            if( end[TAG] - cut[TAG] ) key += sprintf(key, "%.*s.", (int)(end[TAG] - cut[TAG]), cut[TAG] );
            if( end[KEY] - cut[KEY] ) key += sprintf(key,  "%.*s", (int)(end[KEY] - cut[KEY]), cut[KEY] );
            if( end[SUB] - cut[SUB] ) key += sprintf(key, ".%.*s", (int)(end[SUB] - cut[SUB]), cut[SUB] );
            int reqlen = (key - buf) + 1 + (end[VAL] - cut[VAL]) + 1 + 1;
            if( (reqlen + maplen) >= mapcap ) map = REALLOC( map, mapcap += reqlen + 512 );
            sprintf( map + maplen, "%.*s%c%.*s%c%c", (int)(key - buf), buf, 0, (int)(end[VAL] - cut[VAL]), cut[VAL], 0, 0 );
            maplen += reqlen - 1;
        }
    }
    return map;
}

// @todo: evaluate alt api
// int count = ini_count(filename);
// char *key = ini_key(filename, id);
// char *val = ini_val(filename, id);

void ini_destroy(ini_t x) {
    for each_map(x, char*, k, char*, v) {
        FREE(k);
        FREE(v);
    }
    map_free(x);
}

ini_t ini_from_mem(const char *data) {
    if( !data || !data[0] ) return 0;

    char *kv = ini_parse(data);
    if( !kv ) return 0;

        ini_t map = 0;
        map_init(map, less_str, hash_str);
        for( char *iter = kv; iter[0]; ) {
            char *key = iter; while( *iter++ );
            char *val = iter; while( *iter++ );

            char **found = map_find(map, key);
            if( !found ) map_insert(map, STRDUP(key), STRDUP(val));
            assert( map_find(map,key) );
        }

    FREE( kv );

    return map;
}

ini_t ini(const char *filename) {
    return ini_from_mem(file_read(filename));    
}

bool ini_write(const char *filename, const char *section, const char *key, const char *value) {
    // this is a little hacky {
    char *data = file_read(filename);
    if( data && data[0] ) {
        char *begin = strrchr(data, '[');
        char *end = strrchr(data, ']');
        if( begin && end && begin < end ) {
            char *last_section = va("%.*s", (int)(end - begin - 1), begin + 1);
            if( !strcmpi(last_section, section) ) section = 0;
        }
    }
    // }

    char *s = va("%s%s=%s\r\n", section ? va("[%s]\r\n", section) : "", key, value);
    return file_append(filename, s, strlen(s));
}

#line 0

#line 1 "fwk_font.c"
// font framework. original code by Vassvik (UNLICENSED)
// - rlyeh, public domain.
//
// [x] embedded default font (bm mini).
// [x] oversampling, texture dimensions.
// [x] utf8, unicode ranges.
// [x] markup opcodes.
// [x] faces (italic, bold, regular, cjk), colors and sizes.
// [x] unicode ranges from dear-imgui (@ocornut allowed to mit-0 relicense the data tables).
// [*] alignment. kinda hacky. revisit some day.
// [ ] underlining, soft/hard shadows, outlines.
// [ ] clip/wrap/overflow regions.
// [ ] text-shaping, text-layout.
// [ ] text-wrapping.
// [ ] optimizations.
//
// ## language families that could be merged on a single texture alias
// - EU+EL+RU
// - AR+HE+RU
// - TH+VI+TW
// - ZH
// - JP
// - KR

// -----------------------------------------------------------------------------

// bm-mini.zip (public domain font)
// http://bitmapmania.m78.com
// cooz@m78.com

static const char bm_mini_ttf[] = {
/*000000*/ 0x00,0x01,0x00,0x00,0x00,0x0e,0x00,0x30,0x00,0x03,0x00,0xb0,0x4f,0x53,0x2f,0x32,
/*000010*/ 0x80,0x00,0x6d,0x88,0x00,0x00,0x4e,0x04,0x00,0x00,0x00,0x4e,0x63,0x6d,0x61,0x70,
/*000020*/ 0xf1,0x89,0xe8,0x81,0x00,0x00,0x45,0x54,0x00,0x00,0x02,0x28,0x63,0x76,0x74,0x20,
/*000030*/ 0x5a,0x9b,0xfa,0x82,0x00,0x00,0x03,0xe8,0x00,0x00,0x00,0x32,0x66,0x70,0x67,0x6d,
/*000040*/ 0x83,0x33,0xc2,0x4f,0x00,0x00,0x03,0xd4,0x00,0x00,0x00,0x14,0x67,0x6c,0x79,0x66,
/*000050*/ 0x05,0xa3,0xba,0x6b,0x00,0x00,0x04,0x64,0x00,0x00,0x3c,0xec,0x68,0x64,0x6d,0x78,
/*000060*/ 0x0d,0xad,0x0b,0x14,0x00,0x00,0x47,0x7c,0x00,0x00,0x06,0x88,0x68,0x65,0x61,0x64,
/*000070*/ 0xce,0xfe,0xc6,0xae,0x00,0x00,0x4e,0x54,0x00,0x00,0x00,0x36,0x68,0x68,0x65,0x61,
/*000080*/ 0x05,0x15,0x02,0xee,0x00,0x00,0x4e,0x8c,0x00,0x00,0x00,0x24,0x68,0x6d,0x74,0x78,
/*000090*/ 0xae,0x38,0x00,0xf4,0x00,0x00,0x42,0xe0,0x00,0x00,0x01,0x8c,0x6c,0x6f,0x63,0x61,
/*0000a0*/ 0x00,0x0b,0xe4,0xba,0x00,0x00,0x41,0x50,0x00,0x00,0x01,0x90,0x6d,0x61,0x78,0x70,
/*0000b0*/ 0x00,0xfe,0x01,0x4d,0x00,0x00,0x4e,0xb0,0x00,0x00,0x00,0x20,0x6e,0x61,0x6d,0x65,
/*0000c0*/ 0xd6,0xe2,0x1a,0x1f,0x00,0x00,0x00,0xec,0x00,0x00,0x02,0xe5,0x70,0x6f,0x73,0x74,
/*0000d0*/ 0x09,0x85,0x09,0xff,0x00,0x00,0x44,0x6c,0x00,0x00,0x00,0xe8,0x70,0x72,0x65,0x70,
/*0000e0*/ 0xc9,0x0f,0xd2,0x13,0x00,0x00,0x04,0x1c,0x00,0x00,0x00,0x48,0x00,0x00,0x00,0x15,
/*0000f0*/ 0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x76,0x00,0x3b,0x00,0x00,
/*000100*/ 0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x0e,0x00,0xb8,0x00,0x00,0x00,0x00,0x00,0x00,
/*000110*/ 0x00,0x02,0x00,0x04,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x4e,
/*000120*/ 0x01,0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x14,0x00,0xd6,0x00,0x00,
/*000130*/ 0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x48,0x01,0x83,0x00,0x00,0x00,0x00,0x00,0x00,
/*000140*/ 0x00,0x06,0x00,0x10,0x01,0xd3,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3b,
/*000150*/ 0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x07,0x00,0xb1,0x00,0x01,
/*000160*/ 0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x02,0x00,0xc6,0x00,0x01,0x00,0x00,0x00,0x00,
/*000170*/ 0x00,0x03,0x00,0x27,0x00,0xea,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x0a,
/*000180*/ 0x00,0xcc,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x24,0x01,0x5f,0x00,0x01,
/*000190*/ 0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x08,0x01,0xcb,0x00,0x03,0x00,0x01,0x04,0x09,
/*0001a0*/ 0x00,0x00,0x00,0x76,0x00,0x3b,0x00,0x03,0x00,0x01,0x04,0x09,0x00,0x01,0x00,0x0e,
/*0001b0*/ 0x00,0xb8,0x00,0x03,0x00,0x01,0x04,0x09,0x00,0x02,0x00,0x04,0x00,0xc8,0x00,0x03,
/*0001c0*/ 0x00,0x01,0x04,0x09,0x00,0x03,0x00,0x4e,0x01,0x11,0x00,0x03,0x00,0x01,0x04,0x09,
/*0001d0*/ 0x00,0x04,0x00,0x14,0x00,0xd6,0x00,0x03,0x00,0x01,0x04,0x09,0x00,0x05,0x00,0x48,
/*0001e0*/ 0x01,0x83,0x00,0x03,0x00,0x01,0x04,0x09,0x00,0x06,0x00,0x10,0x01,0xd3,0x43,0x6f,
/*0001f0*/ 0x70,0x79,0x72,0x69,0x67,0x68,0x74,0x20,0x28,0x43,0x29,0x20,0x32,0x30,0x30,0x31,
/*000200*/ 0x20,0x42,0x69,0x74,0x6d,0x61,0x70,0x4d,0x61,0x6e,0x69,0x61,0x20,0x2f,0x20,0x43,
/*000210*/ 0x4f,0x4f,0x5a,0x2e,0x20,0x41,0x6c,0x6c,0x20,0x72,0x69,0x67,0x68,0x74,0x73,0x20,
/*000220*/ 0x72,0x65,0x73,0x65,0x72,0x76,0x65,0x64,0x2e,0x00,0x43,0x00,0x6f,0x00,0x70,0x00,
/*000230*/ 0x79,0x00,0x72,0x00,0x69,0x00,0x67,0x00,0x68,0x00,0x74,0x00,0x20,0x00,0x28,0x00,
/*000240*/ 0x43,0x00,0x29,0x00,0x20,0x00,0x32,0x00,0x30,0x00,0x30,0x00,0x31,0x00,0x20,0x00,
/*000250*/ 0x42,0x00,0x69,0x00,0x74,0x00,0x6d,0x00,0x61,0x00,0x70,0x00,0x4d,0x00,0x61,0x00,
/*000260*/ 0x6e,0x00,0x69,0x00,0x61,0x00,0x20,0x00,0x2f,0x00,0x20,0x00,0x43,0x00,0x4f,0x00,
/*000270*/ 0x4f,0x00,0x5a,0x00,0x2e,0x00,0x20,0x00,0x41,0x00,0x6c,0x00,0x6c,0x00,0x20,0x00,
/*000280*/ 0x72,0x00,0x69,0x00,0x67,0x00,0x68,0x00,0x74,0x00,0x73,0x00,0x20,0x00,0x72,0x00,
/*000290*/ 0x65,0x00,0x73,0x00,0x65,0x00,0x72,0x00,0x76,0x00,0x65,0x00,0x64,0x00,0x2e,0x42,
/*0002a0*/ 0x4d,0x20,0x6d,0x69,0x6e,0x69,0x00,0x42,0x00,0x4d,0x00,0x20,0x00,0x6d,0x00,0x69,
/*0002b0*/ 0x00,0x6e,0x00,0x69,0x41,0x38,0x00,0x41,0x00,0x38,0x42,0x4d,0x20,0x6d,0x69,0x6e,
/*0002c0*/ 0x69,0x20,0x41,0x38,0x00,0x42,0x00,0x4d,0x00,0x20,0x00,0x6d,0x00,0x69,0x00,0x6e,
/*0002d0*/ 0x00,0x69,0x00,0x20,0x00,0x41,0x00,0x38,0x4d,0x61,0x63,0x72,0x6f,0x6d,0x65,0x64,
/*0002e0*/ 0x69,0x61,0x20,0x46,0x6f,0x6e,0x74,0x6f,0x67,0x72,0x61,0x70,0x68,0x65,0x72,0x20,
/*0002f0*/ 0x34,0x2e,0x31,0x4a,0x20,0x42,0x4d,0x20,0x6d,0x69,0x6e,0x69,0x20,0x41,0x38,0x00,
/*000300*/ 0x4d,0x00,0x61,0x00,0x63,0x00,0x72,0x00,0x6f,0x00,0x6d,0x00,0x65,0x00,0x64,0x00,
/*000310*/ 0x69,0x00,0x61,0x00,0x20,0x00,0x46,0x00,0x6f,0x00,0x6e,0x00,0x74,0x00,0x6f,0x00,
/*000320*/ 0x67,0x00,0x72,0x00,0x61,0x00,0x70,0x00,0x68,0x00,0x65,0x00,0x72,0x00,0x20,0x00,
/*000330*/ 0x34,0x00,0x2e,0x00,0x31,0x00,0x4a,0x00,0x20,0x00,0x42,0x00,0x4d,0x00,0x20,0x00,
/*000340*/ 0x6d,0x00,0x69,0x00,0x6e,0x00,0x69,0x00,0x20,0x00,0x41,0x00,0x38,0x4d,0x61,0x63,
/*000350*/ 0x72,0x6f,0x6d,0x65,0x64,0x69,0x61,0x20,0x46,0x6f,0x6e,0x74,0x6f,0x67,0x72,0x61,
/*000360*/ 0x70,0x68,0x65,0x72,0x20,0x34,0x2e,0x31,0x4a,0x20,0x30,0x31,0x2e,0x31,0x2e,0x32,
/*000370*/ 0x37,0x00,0x4d,0x00,0x61,0x00,0x63,0x00,0x72,0x00,0x6f,0x00,0x6d,0x00,0x65,0x00,
/*000380*/ 0x64,0x00,0x69,0x00,0x61,0x00,0x20,0x00,0x46,0x00,0x6f,0x00,0x6e,0x00,0x74,0x00,
/*000390*/ 0x6f,0x00,0x67,0x00,0x72,0x00,0x61,0x00,0x70,0x00,0x68,0x00,0x65,0x00,0x72,0x00,
/*0003a0*/ 0x20,0x00,0x34,0x00,0x2e,0x00,0x31,0x00,0x4a,0x00,0x20,0x00,0x30,0x00,0x31,0x00,
/*0003b0*/ 0x2e,0x00,0x31,0x00,0x2e,0x00,0x32,0x00,0x37,0x42,0x4d,0x6d,0x69,0x6e,0x69,0x41,
/*0003c0*/ 0x38,0x00,0x42,0x00,0x4d,0x00,0x6d,0x00,0x69,0x00,0x6e,0x00,0x69,0x00,0x41,0x00,
/*0003d0*/ 0x38,0x00,0x00,0x00,0x40,0x01,0x00,0x2c,0x76,0x45,0x20,0xb0,0x03,0x25,0x45,0x23,
/*0003e0*/ 0x61,0x68,0x18,0x23,0x68,0x60,0x44,0x2d,0xff,0x39,0x00,0x01,0x01,0x8f,0x02,0x57,
/*0003f0*/ 0x00,0x62,0x00,0xc5,0x00,0x62,0x00,0xc5,0x01,0x29,0x01,0x8d,0x01,0x8e,0x01,0x2a,
/*000400*/ 0x5a,0x67,0x12,0x06,0xd2,0xb8,0x6a,0x18,0xf8,0x2a,0x61,0xa3,0x0e,0x40,0xee,0xd2,
/*000410*/ 0x80,0x3a,0x27,0x55,0xa2,0x87,0x00,0x01,0x00,0x0d,0x00,0x00,0x40,0x11,0x0b,0x0b,
/*000420*/ 0x0a,0x0a,0x09,0x09,0x08,0x08,0x03,0x03,0x02,0x02,0x01,0x01,0x00,0x00,0x01,0x8d,
/*000430*/ 0xb8,0x01,0xff,0x85,0x45,0x68,0x44,0x45,0x68,0x44,0x45,0x68,0x44,0x45,0x68,0x44,
/*000440*/ 0x45,0x68,0x44,0x45,0x68,0x44,0x45,0x68,0x44,0x45,0x68,0x44,0xb3,0x05,0x04,0x46,
/*000450*/ 0x00,0x2b,0xb3,0x07,0x06,0x46,0x00,0x2b,0xb1,0x04,0x04,0x45,0x68,0x44,0xb1,0x06,
/*000460*/ 0x06,0x45,0x68,0x44,0x00,0x02,0x00,0x32,0x00,0x00,0x01,0x5e,0x02,0x58,0x00,0x03,
/*000470*/ 0x00,0x07,0x00,0x55,0x40,0x1f,0x01,0x08,0x08,0x40,0x09,0x02,0x07,0x04,0x04,0x01,
/*000480*/ 0x00,0x06,0x05,0x04,0x03,0x02,0x05,0x04,0x06,0x00,0x07,0x06,0x06,0x01,0x02,0x01,
/*000490*/ 0x03,0x00,0x01,0x00,0x46,0x76,0x2f,0x37,0x18,0x00,0x2f,0x3c,0x2f,0x3c,0x10,0xfd,
/*0004a0*/ 0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x00,0x31,0x30,
/*0004b0*/ 0x01,0x49,0x68,0xb9,0x00,0x00,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,
/*0004c0*/ 0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x33,0x11,0x21,0x11,0x27,0x33,0x11,
/*0004d0*/ 0x23,0x32,0x01,0x2c,0xfa,0xc8,0xc8,0x02,0x58,0xfd,0xa8,0x32,0x01,0xf4,0x00,0x02,
/*0004e0*/ 0x00,0x01,0x00,0x01,0x00,0x63,0x02,0x57,0x00,0x03,0x00,0x07,0x00,0x53,0x40,0x20,
/*0004f0*/ 0x01,0x08,0x08,0x40,0x09,0x00,0x01,0x00,0x06,0x05,0x02,0x03,0x01,0x04,0x07,0x04,
/*000500*/ 0x03,0x03,0x00,0x07,0x06,0x06,0x04,0x05,0x04,0x01,0x03,0x02,0x03,0x01,0x01,0x46,
/*000510*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,
/*000520*/ 0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x08,
/*000530*/ 0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,
/*000540*/ 0x59,0x37,0x23,0x11,0x33,0x11,0x23,0x35,0x33,0x63,0x62,0x62,0x62,0x62,0xc9,0x01,
/*000550*/ 0x8e,0xfd,0xaa,0x62,0x00,0x02,0x00,0x01,0x01,0x91,0x01,0x2b,0x02,0x57,0x00,0x03,
/*000560*/ 0x00,0x07,0x00,0x50,0x40,0x1e,0x01,0x08,0x08,0x40,0x09,0x00,0x02,0x01,0x04,0x03,
/*000570*/ 0x00,0x07,0x04,0x04,0x06,0x05,0x05,0x04,0x01,0x03,0x00,0x07,0x06,0x03,0x03,0x02,
/*000580*/ 0x03,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x2f,0x17,0x3c,0x01,
/*000590*/ 0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*0005a0*/ 0x05,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x08,
/*0005b0*/ 0xff,0xc0,0x38,0x59,0x01,0x23,0x35,0x33,0x07,0x23,0x35,0x33,0x01,0x2b,0x62,0x62,
/*0005c0*/ 0xc8,0x62,0x62,0x01,0x91,0xc6,0xc6,0xc6,0x00,0x02,0x00,0x01,0x00,0x65,0x01,0xf3,
/*0005d0*/ 0x02,0x57,0x00,0x1b,0x00,0x1f,0x00,0xb3,0x40,0x5e,0x01,0x20,0x20,0x40,0x21,0x00,
/*0005e0*/ 0x1b,0x18,0x17,0x14,0x13,0x0e,0x0d,0x0a,0x09,0x00,0x06,0x05,0x04,0x07,0x1f,0x12,
/*0005f0*/ 0x11,0x03,0x1e,0x04,0x10,0x0f,0x0c,0x0b,0x08,0x05,0x07,0x1d,0x04,0x03,0x03,0x1c,
/*000600*/ 0x04,0x1a,0x19,0x16,0x02,0x01,0x05,0x15,0x1b,0x1a,0x06,0x00,0x1f,0x0b,0x0a,0x03,
/*000610*/ 0x1c,0x06,0x09,0x08,0x05,0x04,0x01,0x05,0x00,0x0d,0x0c,0x06,0x0e,0x1e,0x19,0x18,
/*000620*/ 0x03,0x1d,0x06,0x17,0x16,0x13,0x12,0x0f,0x05,0x0e,0x07,0x06,0x03,0x03,0x02,0x15,
/*000630*/ 0x14,0x11,0x03,0x10,0x03,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,
/*000640*/ 0x2f,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,
/*000650*/ 0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,
/*000660*/ 0x17,0x3c,0x10,0xfd,0x3c,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x00,
/*000670*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x20,0x49,0x68,0x61,0xb0,0x40,0x52,
/*000680*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x20,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,
/*000690*/ 0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x35,0x33,0x15,0x33,0x35,
/*0006a0*/ 0x33,0x15,0x33,0x15,0x23,0x15,0x33,0x27,0x35,0x23,0x15,0x01,0xf3,0x65,0x61,0x67,
/*0006b0*/ 0x61,0x64,0x64,0x64,0x64,0x62,0x67,0x61,0x64,0x64,0x64,0xc8,0x62,0xc9,0x64,0x64,
/*0006c0*/ 0x64,0x65,0x61,0x67,0x61,0x64,0x64,0x64,0x65,0x61,0x67,0x03,0x62,0x62,0x00,0x01,
/*0006d0*/ 0x00,0x01,0x00,0x01,0x01,0xf3,0x01,0xf3,0x00,0x13,0x00,0x88,0x40,0x42,0x01,0x14,
/*0006e0*/ 0x14,0x40,0x15,0x00,0x13,0x0a,0x09,0x00,0x04,0x03,0x04,0x05,0x0e,0x06,0x05,0x03,
/*0006f0*/ 0x0d,0x04,0x0c,0x0b,0x08,0x03,0x07,0x10,0x0f,0x04,0x12,0x02,0x01,0x03,0x11,0x07,
/*000700*/ 0x06,0x03,0x03,0x02,0x0b,0x00,0x09,0x08,0x01,0x03,0x00,0x07,0x12,0x11,0x10,0x0d,
/*000710*/ 0x03,0x0c,0x13,0x12,0x0f,0x0e,0x0b,0x05,0x0a,0x02,0x05,0x04,0x01,0x01,0x09,0x46,
/*000720*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x2f,0x17,0x3c,0x10,0xfd,0x17,
/*000730*/ 0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,
/*000740*/ 0x3c,0x10,0xfd,0x3c,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*000750*/ 0x09,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,
/*000760*/ 0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x15,0x23,0x35,0x23,0x35,0x23,0x35,0x33,
/*000770*/ 0x35,0x33,0x15,0x33,0x35,0x33,0x15,0x33,0x01,0xf3,0x65,0x64,0x61,0x64,0x64,0x64,
/*000780*/ 0x62,0x67,0x61,0x64,0xc9,0x64,0x64,0x65,0x64,0xc5,0x64,0x64,0x64,0x65,0x00,0x03,
/*000790*/ 0x00,0x01,0x00,0x01,0x02,0xbb,0x02,0x57,0x00,0x23,0x00,0x27,0x00,0x2b,0x00,0xde,
/*0007a0*/ 0x40,0x80,0x01,0x2c,0x2c,0x40,0x2d,0x00,0x20,0x1f,0x08,0x07,0x04,0x0a,0x09,0x1e,
/*0007b0*/ 0x1d,0x04,0x18,0x17,0x0c,0x03,0x0b,0x29,0x28,0x16,0x15,0x0e,0x05,0x0d,0x04,0x0f,
/*0007c0*/ 0x2b,0x2a,0x14,0x13,0x10,0x05,0x0f,0x04,0x12,0x11,0x1a,0x06,0x05,0x03,0x19,0x04,
/*0007d0*/ 0x27,0x26,0x1c,0x04,0x03,0x05,0x1b,0x25,0x24,0x22,0x02,0x01,0x05,0x21,0x04,0x23,
/*0007e0*/ 0x00,0x27,0x24,0x05,0x04,0x01,0x05,0x00,0x06,0x02,0x1d,0x1c,0x11,0x10,0x0d,0x05,
/*0007f0*/ 0x0c,0x06,0x0e,0x21,0x0f,0x0e,0x03,0x20,0x06,0x26,0x25,0x23,0x22,0x1f,0x1e,0x0b,
/*000800*/ 0x07,0x06,0x09,0x0a,0x2a,0x29,0x17,0x16,0x13,0x05,0x12,0x06,0x14,0x2b,0x28,0x19,
/*000810*/ 0x03,0x18,0x02,0x1b,0x1a,0x15,0x03,0x14,0x03,0x09,0x08,0x03,0x03,0x02,0x01,0x01,
/*000820*/ 0x11,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,0x3f,0x17,0x3c,
/*000830*/ 0x10,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,
/*000840*/ 0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,
/*000850*/ 0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,
/*000860*/ 0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x11,0x00,0x2c,0x49,0x68,0x61,
/*000870*/ 0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x2c,0xff,0xc0,0x38,0x59,0x25,0x23,
/*000880*/ 0x15,0x23,0x35,0x23,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x23,0x15,0x23,0x35,0x23,
/*000890*/ 0x35,0x33,0x35,0x33,0x15,0x33,0x15,0x33,0x35,0x33,0x15,0x23,0x15,0x33,0x35,0x33,
/*0008a0*/ 0x15,0x33,0x07,0x35,0x23,0x15,0x01,0x35,0x23,0x15,0x02,0xbb,0x65,0x61,0x64,0x67,
/*0008b0*/ 0x61,0x64,0x67,0x61,0x64,0x64,0x62,0x64,0x67,0x61,0x64,0x67,0x61,0x64,0x64,0x62,
/*0008c0*/ 0xfe,0xd2,0x62,0x65,0x64,0x65,0x63,0xc8,0xc6,0xca,0x64,0x65,0x61,0x64,0x65,0x63,
/*0008d0*/ 0xc8,0xc6,0xca,0x64,0x65,0x61,0x62,0x62,0x01,0x2c,0x62,0x62,0x00,0x03,0x00,0x01,
/*0008e0*/ 0x00,0x01,0x01,0xf3,0x02,0x57,0x00,0x1b,0x00,0x1f,0x00,0x25,0x00,0xce,0x40,0x76,
/*0008f0*/ 0x01,0x26,0x26,0x40,0x27,0x00,0x1a,0x19,0x1b,0x04,0x03,0x03,0x00,0x04,0x05,0x10,
/*000900*/ 0x0f,0x0c,0x03,0x0b,0x04,0x09,0x25,0x24,0x1f,0x1e,0x12,0x11,0x0e,0x0d,0x0a,0x09,
/*000910*/ 0x09,0x04,0x13,0x23,0x22,0x1d,0x1c,0x18,0x17,0x14,0x07,0x13,0x04,0x15,0x21,0x20,
/*000920*/ 0x16,0x08,0x07,0x05,0x15,0x04,0x06,0x05,0x02,0x03,0x01,0x03,0x02,0x06,0x04,0x25,
/*000930*/ 0x20,0x0b,0x0a,0x07,0x05,0x06,0x06,0x04,0x1e,0x1d,0x15,0x14,0x11,0x05,0x10,0x06,
/*000940*/ 0x12,0x22,0x21,0x06,0x0c,0x24,0x23,0x19,0x18,0x0d,0x01,0x00,0x07,0x0c,0x06,0x17,
/*000950*/ 0x0f,0x0e,0x03,0x16,0x1f,0x1c,0x1b,0x03,0x1a,0x02,0x13,0x12,0x03,0x09,0x08,0x05,
/*000960*/ 0x03,0x04,0x01,0x01,0x0b,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x3c,
/*000970*/ 0x3f,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,
/*000980*/ 0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,
/*000990*/ 0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x2e,0x2e,
/*0009a0*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x0b,0x00,0x26,0x49,0x68,0x61,0xb0,0x40,
/*0009b0*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x26,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x33,
/*0009c0*/ 0x15,0x23,0x35,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x35,0x33,
/*0009d0*/ 0x15,0x33,0x15,0x23,0x15,0x33,0x35,0x33,0x25,0x35,0x23,0x15,0x13,0x35,0x23,0x35,
/*0009e0*/ 0x23,0x15,0x01,0xf3,0x64,0x64,0x62,0x67,0xc5,0x64,0x64,0x64,0x64,0x62,0x64,0x64,
/*0009f0*/ 0xcb,0x61,0xfe,0xd4,0x62,0xc6,0x64,0x62,0x01,0x2d,0xcb,0x61,0x64,0x64,0x65,0xc5,
/*000a00*/ 0x67,0x61,0x64,0x65,0x61,0x66,0x64,0x02,0x62,0x62,0xfe,0xd4,0x61,0x65,0xc6,0x00,
/*000a10*/ 0x00,0x01,0x00,0x01,0x01,0x91,0x00,0x63,0x02,0x57,0x00,0x03,0x00,0x3f,0x40,0x13,
/*000a20*/ 0x01,0x04,0x04,0x40,0x05,0x00,0x02,0x01,0x04,0x03,0x00,0x01,0x00,0x03,0x02,0x03,
/*000a30*/ 0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x01,0x2f,0x3c,0xfd,
/*000a40*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x04,0x49,0x68,0x61,0xb0,
/*000a50*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,0x59,0x13,0x23,0x35,
/*000a60*/ 0x33,0x63,0x62,0x62,0x01,0x91,0xc6,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0xc7,
/*000a70*/ 0x02,0x57,0x00,0x0b,0x00,0x67,0x40,0x2d,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x07,0x06,
/*000a80*/ 0x0b,0x04,0x03,0x03,0x00,0x04,0x05,0x0a,0x09,0x06,0x05,0x02,0x05,0x01,0x04,0x08,
/*000a90*/ 0x07,0x09,0x08,0x01,0x03,0x00,0x06,0x0a,0x03,0x02,0x06,0x04,0x0b,0x0a,0x03,0x05,
/*000aa0*/ 0x04,0x01,0x01,0x07,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,
/*000ab0*/ 0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x00,
/*000ac0*/ 0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x07,0x00,0x0c,0x49,0x68,0x61,0xb0,
/*000ad0*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x13,0x23,0x11,
/*000ae0*/ 0x33,0x15,0x23,0x35,0x23,0x11,0x33,0x35,0x33,0xc7,0x64,0x64,0x62,0x64,0x64,0x62,
/*000af0*/ 0x01,0xf5,0xfe,0x6d,0x61,0x65,0x01,0x8d,0x64,0x00,0x00,0x01,0x00,0x01,0x00,0x01,
/*000b00*/ 0x00,0xc7,0x02,0x57,0x00,0x0b,0x00,0x67,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,
/*000b10*/ 0x0b,0x0a,0x02,0x01,0x04,0x08,0x07,0x04,0x03,0x03,0x0a,0x09,0x06,0x03,0x05,0x04,
/*000b20*/ 0x0b,0x00,0x05,0x01,0x00,0x03,0x04,0x06,0x02,0x07,0x06,0x06,0x08,0x09,0x08,0x03,
/*000b30*/ 0x03,0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,
/*000b40*/ 0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,
/*000b50*/ 0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0c,0x49,0x68,
/*000b60*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x37,
/*000b70*/ 0x23,0x15,0x23,0x35,0x33,0x11,0x23,0x35,0x33,0x15,0x33,0xc7,0x65,0x61,0x64,0x64,
/*000b80*/ 0x62,0x64,0x65,0x64,0x62,0x01,0x92,0x62,0x65,0x00,0x00,0x01,0x00,0x01,0x01,0x2d,
/*000b90*/ 0x01,0x2b,0x02,0x57,0x00,0x13,0x00,0x83,0x40,0x40,0x01,0x14,0x14,0x40,0x15,0x00,
/*000ba0*/ 0x12,0x11,0x13,0x04,0x03,0x03,0x00,0x04,0x05,0x08,0x07,0x04,0x0e,0x0d,0x0a,0x03,
/*000bb0*/ 0x09,0x10,0x0f,0x0c,0x03,0x0b,0x04,0x06,0x05,0x02,0x03,0x01,0x07,0x06,0x06,0x11,
/*000bc0*/ 0x0d,0x0c,0x01,0x00,0x05,0x10,0x09,0x08,0x05,0x03,0x04,0x13,0x12,0x0f,0x03,0x0e,
/*000bd0*/ 0x03,0x0b,0x03,0x02,0x03,0x0a,0x02,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,
/*000be0*/ 0x17,0x3c,0x3f,0x17,0x3c,0x2f,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x01,0x2f,0x17,
/*000bf0*/ 0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x2e,0x2e,0x00,
/*000c00*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,
/*000c10*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x33,0x15,
/*000c20*/ 0x23,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x15,0x33,0x35,0x33,0x01,
/*000c30*/ 0x2b,0x64,0x64,0x62,0x67,0x61,0x64,0x64,0x62,0x67,0x61,0x01,0xf5,0x67,0x61,0x64,
/*000c40*/ 0x64,0x62,0x66,0x62,0x64,0x64,0x00,0x01,0x00,0x01,0x00,0x65,0x01,0x2b,0x01,0x8f,
/*000c50*/ 0x00,0x0b,0x00,0x66,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0b,0x00,0x08,0x01,
/*000c60*/ 0x06,0x05,0x08,0x03,0x0a,0x09,0x02,0x03,0x01,0x04,0x08,0x07,0x04,0x03,0x03,0x0b,
/*000c70*/ 0x07,0x06,0x03,0x0a,0x06,0x05,0x04,0x01,0x03,0x00,0x03,0x02,0x09,0x08,0x02,0x01,
/*000c80*/ 0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x2f,0x17,0x3c,0xfd,0x17,
/*000c90*/ 0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x00,0x31,
/*000ca0*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*000cb0*/ 0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,
/*000cc0*/ 0x35,0x33,0x35,0x33,0x15,0x33,0x01,0x2b,0x65,0x61,0x64,0x64,0x62,0x64,0xc9,0x64,
/*000cd0*/ 0x65,0x61,0x64,0x65,0x00,0x01,0x00,0x01,0xff,0x9d,0x00,0xc7,0x00,0x63,0x00,0x07,
/*000ce0*/ 0x00,0x53,0x40,0x1e,0x01,0x08,0x08,0x40,0x09,0x00,0x02,0x01,0x04,0x04,0x03,0x06,
/*000cf0*/ 0x05,0x04,0x07,0x00,0x05,0x04,0x06,0x02,0x07,0x06,0x03,0x02,0x01,0x00,0x01,0x01,
/*000d00*/ 0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x2f,0x3c,0x10,0xfd,0x3c,
/*000d10*/ 0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,
/*000d20*/ 0x00,0x03,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,
/*000d30*/ 0x08,0xff,0xc0,0x38,0x59,0x37,0x23,0x15,0x23,0x35,0x33,0x35,0x33,0xc7,0x65,0x61,
/*000d40*/ 0x64,0x62,0x01,0x64,0x62,0x64,0x00,0x01,0x00,0x01,0x00,0xc9,0x01,0x2b,0x01,0x2b,
/*000d50*/ 0x00,0x03,0x00,0x3d,0x40,0x11,0x01,0x04,0x04,0x40,0x05,0x00,0x03,0x02,0x01,0x00,
/*000d60*/ 0x03,0x02,0x01,0x00,0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x2f,0x3c,0x2f,0x3c,
/*000d70*/ 0x01,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x04,
/*000d80*/ 0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,
/*000d90*/ 0x59,0x25,0x21,0x35,0x21,0x01,0x2b,0xfe,0xd6,0x01,0x2a,0xc9,0x62,0x00,0x00,0x01,
/*000da0*/ 0x00,0x01,0x00,0x01,0x00,0x63,0x00,0x63,0x00,0x03,0x00,0x3f,0x40,0x13,0x01,0x04,
/*000db0*/ 0x04,0x40,0x05,0x00,0x03,0x00,0x04,0x02,0x01,0x03,0x02,0x01,0x00,0x01,0x01,0x01,
/*000dc0*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x00,
/*000dd0*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x04,0x49,0x68,0x61,0xb0,0x40,0x52,
/*000de0*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,0x59,0x37,0x23,0x35,0x33,0x63,
/*000df0*/ 0x62,0x62,0x01,0x62,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x2b,0x02,0x57,0x00,0x0b,
/*000e00*/ 0x00,0x69,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x04,0x03,0x04,0x06,0x05,0x08,
/*000e10*/ 0x07,0x04,0x02,0x01,0x0a,0x09,0x04,0x0b,0x00,0x01,0x00,0x07,0x0a,0x07,0x03,0x02,
/*000e20*/ 0x03,0x06,0x07,0x04,0x0b,0x0a,0x03,0x09,0x08,0x02,0x05,0x04,0x01,0x01,0x05,0x46,
/*000e30*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,
/*000e40*/ 0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x00,
/*000e50*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,
/*000e60*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,0x15,
/*000e70*/ 0x23,0x35,0x33,0x35,0x33,0x35,0x33,0x01,0x2b,0x65,0x64,0x61,0x65,0x63,0x62,0x01,
/*000e80*/ 0x91,0xc8,0xc8,0xc6,0xc8,0xc8,0x00,0x03,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,
/*000e90*/ 0x00,0x0b,0x00,0x11,0x00,0x17,0x00,0x95,0x40,0x4d,0x01,0x18,0x18,0x40,0x19,0x00,
/*000ea0*/ 0x15,0x14,0x05,0x05,0x11,0x10,0x05,0x00,0x17,0x16,0x0f,0x0e,0x08,0x07,0x04,0x07,
/*000eb0*/ 0x03,0x04,0x06,0x05,0x13,0x12,0x0d,0x0c,0x0a,0x02,0x01,0x07,0x09,0x04,0x0b,0x00,
/*000ec0*/ 0x17,0x12,0x05,0x04,0x01,0x05,0x00,0x06,0x02,0x0e,0x0d,0x0b,0x0a,0x07,0x05,0x06,
/*000ed0*/ 0x06,0x08,0x11,0x0c,0x06,0x13,0x14,0x13,0x10,0x03,0x0f,0x06,0x16,0x15,0x09,0x08,
/*000ee0*/ 0x03,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,
/*000ef0*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,
/*000f00*/ 0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,
/*000f10*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x18,0x49,0x68,0x61,0xb0,
/*000f20*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x18,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,
/*000f30*/ 0x23,0x35,0x23,0x11,0x33,0x35,0x33,0x15,0x33,0x07,0x35,0x23,0x15,0x33,0x35,0x13,
/*000f40*/ 0x35,0x23,0x15,0x23,0x15,0x01,0x8f,0x65,0xc5,0x64,0x64,0xc6,0x64,0x64,0xc6,0x61,
/*000f50*/ 0x65,0x62,0x64,0x65,0x64,0x65,0x01,0x8d,0x64,0x65,0x61,0x62,0xc6,0x64,0xfe,0xd4,
/*000f60*/ 0xc6,0x64,0x62,0x00,0x00,0x01,0x00,0x65,0x00,0x01,0x01,0x2b,0x02,0x57,0x00,0x07,
/*000f70*/ 0x00,0x54,0x40,0x20,0x01,0x08,0x08,0x40,0x09,0x00,0x04,0x03,0x05,0x00,0x06,0x05,
/*000f80*/ 0x02,0x03,0x01,0x04,0x07,0x00,0x03,0x02,0x06,0x05,0x04,0x07,0x06,0x03,0x01,0x00,
/*000f90*/ 0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,0x3c,0xfd,
/*000fa0*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,
/*000fb0*/ 0xb9,0x00,0x03,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*000fc0*/ 0x00,0x08,0xff,0xc0,0x38,0x59,0x25,0x23,0x11,0x23,0x35,0x33,0x35,0x33,0x01,0x2b,
/*000fd0*/ 0x62,0x64,0x64,0x62,0x01,0x01,0x91,0x61,0x64,0x00,0x00,0x01,0x00,0x01,0x00,0x01,
/*000fe0*/ 0x01,0x8f,0x02,0x57,0x00,0x19,0x00,0xa4,0x40,0x53,0x01,0x1a,0x1a,0x40,0x1b,0x00,
/*000ff0*/ 0x12,0x11,0x06,0x03,0x05,0x04,0x14,0x13,0x0a,0x03,0x09,0x04,0x03,0x04,0x16,0x15,
/*001000*/ 0x0c,0x03,0x0b,0x0e,0x0d,0x04,0x02,0x01,0x18,0x17,0x10,0x03,0x0f,0x04,0x19,0x08,
/*001010*/ 0x07,0x03,0x00,0x13,0x12,0x01,0x03,0x00,0x06,0x19,0x18,0x0d,0x03,0x02,0x03,0x0c,
/*001020*/ 0x06,0x0b,0x0a,0x05,0x03,0x04,0x07,0x06,0x06,0x08,0x15,0x14,0x11,0x03,0x10,0x06,
/*001030*/ 0x16,0x17,0x16,0x03,0x0f,0x0e,0x02,0x09,0x08,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,
/*001040*/ 0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x2f,
/*001050*/ 0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,
/*001060*/ 0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,
/*001070*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x1a,0x49,0x68,0x61,0xb0,0x40,
/*001080*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x1a,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,
/*001090*/ 0x15,0x23,0x15,0x21,0x15,0x21,0x35,0x33,0x35,0x33,0x35,0x33,0x35,0x23,0x15,0x23,
/*0010a0*/ 0x35,0x33,0x35,0x33,0x15,0x33,0x01,0x8f,0x65,0x64,0x63,0x01,0x2c,0xfe,0x72,0x65,
/*0010b0*/ 0x64,0x63,0xcb,0x61,0x64,0xc6,0x64,0x01,0x91,0x64,0x64,0x67,0x61,0xc6,0x64,0x64,
/*0010c0*/ 0x66,0x64,0x62,0x64,0x65,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,
/*0010d0*/ 0x00,0x1b,0x00,0xa4,0x40,0x57,0x01,0x1c,0x1c,0x40,0x1d,0x00,0x14,0x13,0x10,0x0f,
/*0010e0*/ 0x08,0x04,0x03,0x07,0x07,0x04,0x12,0x11,0x06,0x03,0x05,0x1b,0x18,0x17,0x03,0x00,
/*0010f0*/ 0x04,0x09,0x0c,0x0b,0x04,0x1a,0x19,0x16,0x0e,0x0d,0x0a,0x09,0x02,0x01,0x09,0x15,
/*001100*/ 0x1b,0x1a,0x07,0x00,0x09,0x08,0x06,0x02,0x07,0x06,0x06,0x05,0x01,0x00,0x03,0x04,
/*001110*/ 0x0b,0x0a,0x06,0x0c,0x19,0x18,0x11,0x03,0x10,0x06,0x17,0x16,0x13,0x0f,0x0e,0x05,
/*001120*/ 0x12,0x15,0x14,0x03,0x0d,0x0c,0x02,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,
/*001130*/ 0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,
/*001140*/ 0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,
/*001150*/ 0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,
/*001160*/ 0x49,0x68,0xb9,0x00,0x05,0x00,0x1c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*001170*/ 0x37,0xb9,0x00,0x1c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,0x35,0x33,
/*001180*/ 0x15,0x33,0x35,0x23,0x35,0x33,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x33,0x15,0x33,
/*001190*/ 0x15,0x23,0x15,0x33,0x01,0x8f,0x65,0xc5,0x64,0x62,0xca,0x64,0x64,0xcb,0x61,0x64,
/*0011a0*/ 0xc6,0x64,0x64,0x64,0x65,0x64,0x65,0x61,0x64,0xcb,0x61,0x66,0x64,0x62,0x64,0x65,
/*0011b0*/ 0x61,0x67,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0d,0x00,0x6a,
/*0011c0*/ 0x40,0x2f,0x01,0x0e,0x0e,0x40,0x0f,0x00,0x0d,0x00,0x0a,0x09,0x04,0x03,0x03,0x04,
/*0011d0*/ 0x0c,0x0b,0x02,0x03,0x01,0x08,0x07,0x04,0x06,0x05,0x0d,0x09,0x08,0x03,0x0c,0x06,
/*0011e0*/ 0x05,0x04,0x01,0x03,0x00,0x0b,0x0a,0x07,0x03,0x06,0x03,0x03,0x02,0x01,0x01,0x05,
/*0011f0*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,
/*001200*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x00,0x31,
/*001210*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*001220*/ 0x38,0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,
/*001230*/ 0x11,0x33,0x11,0x33,0x11,0x33,0x11,0x33,0x01,0x8f,0x65,0x61,0xc8,0x62,0x67,0x61,
/*001240*/ 0x64,0xc9,0xc8,0xc8,0x01,0x8e,0xfe,0xd4,0x01,0x2c,0xfe,0xd3,0x00,0x01,0x00,0x01,
/*001250*/ 0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x7b,0x40,0x38,0x01,0x10,0x10,0x40,
/*001260*/ 0x11,0x00,0x02,0x01,0x0e,0x0d,0x06,0x03,0x05,0x04,0x0f,0x0a,0x09,0x03,0x00,0x0c,
/*001270*/ 0x0b,0x04,0x08,0x07,0x04,0x03,0x03,0x0f,0x0e,0x07,0x00,0x05,0x01,0x00,0x03,0x04,
/*001280*/ 0x06,0x02,0x07,0x06,0x06,0x0d,0x0c,0x0b,0x0a,0x06,0x08,0x09,0x08,0x03,0x03,0x02,
/*001290*/ 0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,
/*0012a0*/ 0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,
/*0012b0*/ 0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,
/*0012c0*/ 0x00,0x03,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,
/*0012d0*/ 0x10,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x21,0x35,0x21,0x35,0x21,0x11,0x21,0x15,
/*0012e0*/ 0x21,0x15,0x33,0x15,0x33,0x01,0x8f,0x65,0xfe,0xd7,0x01,0x2c,0xfe,0xd4,0x01,0x8e,
/*0012f0*/ 0xfe,0xd4,0xc8,0x64,0x65,0x64,0x62,0xca,0x01,0x2a,0x62,0x67,0x64,0x00,0x00,0x03,
/*001300*/ 0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x13,0x00,0x17,0x00,0x8a,
/*001310*/ 0x40,0x49,0x01,0x18,0x18,0x40,0x19,0x00,0x17,0x16,0x13,0x12,0x08,0x07,0x04,0x07,
/*001320*/ 0x03,0x04,0x06,0x05,0x15,0x14,0x11,0x10,0x0e,0x0d,0x0a,0x02,0x01,0x09,0x09,0x04,
/*001330*/ 0x0f,0x0c,0x0b,0x03,0x00,0x17,0x14,0x05,0x04,0x01,0x05,0x00,0x06,0x02,0x12,0x11,
/*001340*/ 0x0b,0x0a,0x07,0x05,0x06,0x06,0x08,0x16,0x0f,0x0e,0x03,0x15,0x06,0x13,0x10,0x0d,
/*001350*/ 0x03,0x0c,0x09,0x08,0x03,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,
/*001360*/ 0x3f,0x3c,0x3f,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,
/*001370*/ 0x17,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,
/*001380*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x18,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*001390*/ 0x38,0x11,0x37,0xb9,0x00,0x18,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,
/*0013a0*/ 0x11,0x33,0x35,0x33,0x15,0x33,0x15,0x23,0x15,0x33,0x27,0x35,0x23,0x15,0x13,0x35,
/*0013b0*/ 0x23,0x15,0x01,0x8f,0x65,0xc5,0x64,0x64,0xc6,0x64,0x64,0x64,0x64,0xc6,0xc6,0xc6,
/*0013c0*/ 0x65,0x64,0x65,0x01,0x8d,0x64,0x65,0x61,0x67,0x67,0x62,0x62,0xfe,0xd4,0xc6,0xc6,
/*0013d0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x7d,0x40,0x38,
/*0013e0*/ 0x01,0x10,0x10,0x40,0x11,0x00,0x04,0x03,0x04,0x06,0x05,0x08,0x07,0x04,0x02,0x01,
/*0013f0*/ 0x0a,0x09,0x04,0x0f,0x00,0x0c,0x0b,0x04,0x0e,0x0d,0x0d,0x0c,0x01,0x03,0x00,0x07,
/*001400*/ 0x0e,0x07,0x03,0x02,0x03,0x06,0x07,0x04,0x0b,0x0a,0x06,0x0e,0x0f,0x0e,0x03,0x09,
/*001410*/ 0x08,0x02,0x05,0x04,0x01,0x01,0x0d,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,
/*001420*/ 0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,
/*001430*/ 0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x00,
/*001440*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x0d,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,
/*001450*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,0x15,
/*001460*/ 0x23,0x35,0x33,0x35,0x33,0x35,0x23,0x15,0x23,0x35,0x21,0x01,0x8f,0x65,0x64,0x61,
/*001470*/ 0x65,0x63,0xcb,0x61,0x01,0x8e,0x01,0x91,0xc8,0xc8,0xc6,0xc8,0x66,0x64,0xc6,0x00,
/*001480*/ 0x00,0x03,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x13,0x00,0x17,0x00,0x1b,
/*001490*/ 0x00,0x94,0x40,0x52,0x01,0x1c,0x1c,0x40,0x1d,0x00,0x1b,0x1a,0x17,0x16,0x0c,0x0b,
/*0014a0*/ 0x08,0x07,0x04,0x09,0x03,0x04,0x0a,0x09,0x06,0x03,0x05,0x19,0x18,0x15,0x14,0x12,
/*0014b0*/ 0x11,0x0e,0x02,0x01,0x09,0x0d,0x04,0x13,0x10,0x0f,0x03,0x00,0x1b,0x18,0x05,0x04,
/*0014c0*/ 0x01,0x05,0x00,0x06,0x02,0x16,0x15,0x0f,0x0e,0x0b,0x05,0x0a,0x06,0x0c,0x1a,0x19,
/*0014d0*/ 0x13,0x12,0x07,0x05,0x06,0x06,0x17,0x14,0x11,0x09,0x08,0x05,0x10,0x0d,0x0c,0x03,
/*0014e0*/ 0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,
/*0014f0*/ 0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x17,
/*001500*/ 0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,
/*001510*/ 0xb9,0x00,0x05,0x00,0x1c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*001520*/ 0x00,0x1c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,0x23,
/*001530*/ 0x35,0x33,0x35,0x33,0x15,0x33,0x15,0x23,0x15,0x33,0x27,0x35,0x23,0x15,0x13,0x35,
/*001540*/ 0x23,0x15,0x01,0x8f,0x65,0xc5,0x64,0x64,0x64,0x64,0xc6,0x64,0x64,0x64,0x64,0xc6,
/*001550*/ 0xc6,0xc6,0x65,0x64,0x65,0xc5,0x67,0x61,0x64,0x65,0x61,0x67,0x67,0x62,0x62,0xfe,
/*001560*/ 0xd4,0xc6,0xc6,0x00,0x00,0x03,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,
/*001570*/ 0x00,0x13,0x00,0x17,0x00,0x8a,0x40,0x49,0x01,0x18,0x18,0x40,0x19,0x00,0x17,0x16,
/*001580*/ 0x13,0x12,0x0c,0x0b,0x08,0x07,0x04,0x09,0x03,0x04,0x0a,0x09,0x06,0x03,0x05,0x15,
/*001590*/ 0x14,0x11,0x10,0x0e,0x02,0x01,0x07,0x0d,0x04,0x0f,0x00,0x17,0x14,0x05,0x01,0x00,
/*0015a0*/ 0x05,0x04,0x06,0x06,0x12,0x11,0x0f,0x0e,0x0b,0x05,0x0a,0x06,0x0c,0x13,0x09,0x08,
/*0015b0*/ 0x03,0x10,0x06,0x16,0x15,0x07,0x03,0x06,0x0d,0x0c,0x03,0x03,0x02,0x01,0x01,0x05,
/*0015c0*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,
/*0015d0*/ 0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,
/*0015e0*/ 0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x18,0x49,
/*0015f0*/ 0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x18,0xff,0xc0,0x38,0x59,
/*001600*/ 0x25,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x35,0x33,0x15,0x33,
/*001610*/ 0x07,0x35,0x23,0x15,0x17,0x35,0x23,0x15,0x01,0x8f,0x65,0xc5,0x64,0x64,0x64,0x64,
/*001620*/ 0xc6,0x64,0x64,0xc6,0xc6,0xc6,0x65,0x64,0x65,0x61,0x67,0xc5,0x64,0x65,0xc5,0xc6,
/*001630*/ 0xc6,0xc8,0x62,0x62,0x00,0x02,0x00,0x01,0x00,0x65,0x00,0x63,0x01,0xf3,0x00,0x03,
/*001640*/ 0x00,0x07,0x00,0x54,0x40,0x20,0x01,0x08,0x08,0x40,0x09,0x00,0x07,0x04,0x03,0x03,
/*001650*/ 0x00,0x04,0x06,0x05,0x02,0x03,0x01,0x01,0x00,0x06,0x02,0x07,0x06,0x06,0x04,0x03,
/*001660*/ 0x02,0x05,0x04,0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x2f,0x3c,0x2f,0x3c,0x10,
/*001670*/ 0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,
/*001680*/ 0x49,0x68,0xb9,0x00,0x01,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*001690*/ 0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x13,0x23,0x35,0x33,0x11,0x23,0x35,0x33,
/*0016a0*/ 0x63,0x62,0x62,0x62,0x62,0x01,0x91,0x62,0xfe,0x72,0x62,0x00,0x00,0x02,0x00,0x01,
/*0016b0*/ 0x00,0x01,0x00,0x63,0x01,0xf3,0x00,0x03,0x00,0x07,0x00,0x55,0x40,0x21,0x01,0x08,
/*0016c0*/ 0x08,0x40,0x09,0x00,0x07,0x04,0x03,0x03,0x00,0x04,0x06,0x05,0x02,0x03,0x01,0x07,
/*0016d0*/ 0x06,0x07,0x04,0x01,0x00,0x06,0x02,0x03,0x02,0x05,0x04,0x01,0x01,0x01,0x46,0x76,
/*0016e0*/ 0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,
/*0016f0*/ 0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x08,
/*001700*/ 0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,
/*001710*/ 0x59,0x13,0x23,0x35,0x33,0x11,0x23,0x35,0x33,0x63,0x62,0x62,0x62,0x62,0x01,0x91,
/*001720*/ 0x62,0xfe,0x0e,0xc6,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x2b,0x01,0xf3,0x00,0x13,
/*001730*/ 0x00,0x8a,0x40,0x42,0x01,0x14,0x14,0x40,0x15,0x00,0x10,0x0f,0x13,0x08,0x07,0x03,
/*001740*/ 0x00,0x04,0x09,0x12,0x11,0x0a,0x09,0x06,0x02,0x01,0x07,0x05,0x04,0x0b,0x0c,0x0b,
/*001750*/ 0x04,0x03,0x03,0x04,0x0e,0x0d,0x01,0x00,0x06,0x12,0x07,0x06,0x06,0x08,0x05,0x04,
/*001760*/ 0x06,0x0b,0x0a,0x0d,0x0c,0x06,0x0f,0x03,0x02,0x03,0x0e,0x13,0x12,0x11,0x10,0x02,
/*001770*/ 0x09,0x08,0x01,0x01,0x0d,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,
/*001780*/ 0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,
/*001790*/ 0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x2e,0x2e,
/*0017a0*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x0d,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,
/*0017b0*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,
/*0017c0*/ 0x15,0x33,0x15,0x33,0x15,0x23,0x35,0x23,0x35,0x23,0x35,0x33,0x35,0x33,0x35,0x33,
/*0017d0*/ 0x01,0x2b,0x65,0x63,0x64,0x64,0x62,0x64,0x64,0x65,0x63,0x62,0x01,0x91,0x64,0x67,
/*0017e0*/ 0x64,0x61,0x65,0x64,0x61,0x64,0x64,0x00,0x00,0x02,0x00,0x01,0x00,0x65,0x01,0x2b,
/*0017f0*/ 0x01,0x8f,0x00,0x03,0x00,0x07,0x00,0x54,0x40,0x1e,0x01,0x08,0x08,0x40,0x09,0x00,
/*001800*/ 0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00,0x01,0x00,0x06,0x02,0x07,0x06,0x06,0x04,
/*001810*/ 0x05,0x04,0x03,0x02,0x02,0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,
/*001820*/ 0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,
/*001830*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,
/*001840*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x01,0x21,0x35,0x21,
/*001850*/ 0x11,0x21,0x35,0x21,0x01,0x2b,0xfe,0xd6,0x01,0x2a,0xfe,0xd6,0x01,0x2a,0x01,0x2d,
/*001860*/ 0x62,0xfe,0xd6,0x62,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x2b,0x01,0xf3,0x00,0x13,
/*001870*/ 0x00,0x8e,0x40,0x44,0x01,0x14,0x14,0x40,0x15,0x00,0x10,0x0f,0x04,0x03,0x03,0x04,
/*001880*/ 0x0e,0x0d,0x06,0x03,0x05,0x0c,0x0b,0x08,0x03,0x07,0x04,0x02,0x01,0x12,0x11,0x0a,
/*001890*/ 0x03,0x09,0x04,0x13,0x00,0x13,0x12,0x06,0x00,0x09,0x01,0x00,0x03,0x08,0x06,0x02,
/*0018a0*/ 0x07,0x03,0x02,0x03,0x06,0x06,0x04,0x0b,0x0a,0x06,0x11,0x10,0x0d,0x0c,0x06,0x0e,
/*0018b0*/ 0x0f,0x0e,0x05,0x04,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,
/*0018c0*/ 0x3c,0x10,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,
/*0018d0*/ 0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,
/*0018e0*/ 0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x14,0x49,
/*0018f0*/ 0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,
/*001900*/ 0x25,0x23,0x15,0x23,0x15,0x23,0x35,0x33,0x35,0x33,0x35,0x23,0x35,0x23,0x35,0x33,
/*001910*/ 0x15,0x33,0x15,0x33,0x01,0x2b,0x65,0x64,0x61,0x65,0x63,0x64,0x64,0x62,0x64,0x64,
/*001920*/ 0xc9,0x64,0x64,0x62,0x64,0x67,0x63,0x62,0x65,0x64,0x00,0x02,0x00,0x01,0x00,0x01,
/*001930*/ 0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x13,0x00,0x86,0x40,0x40,0x01,0x14,0x14,0x40,
/*001940*/ 0x15,0x00,0x02,0x01,0x05,0x03,0x0e,0x0d,0x06,0x03,0x05,0x04,0x0f,0x00,0x08,0x07,
/*001950*/ 0x04,0x0a,0x09,0x13,0x10,0x04,0x12,0x11,0x0c,0x0b,0x04,0x05,0x03,0x05,0x01,0x00,
/*001960*/ 0x03,0x04,0x06,0x03,0x02,0x09,0x08,0x06,0x0f,0x0e,0x0b,0x07,0x06,0x05,0x0a,0x13,
/*001970*/ 0x12,0x06,0x10,0x11,0x10,0x01,0x0d,0x0c,0x03,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,
/*001980*/ 0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,
/*001990*/ 0x17,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,
/*0019a0*/ 0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,
/*0019b0*/ 0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,
/*0019c0*/ 0x01,0x23,0x15,0x23,0x35,0x33,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x33,0x15,0x33,
/*0019d0*/ 0x03,0x23,0x35,0x33,0x01,0x8f,0x65,0xc5,0xc8,0xcb,0x61,0x64,0xc6,0x64,0xc8,0x62,
/*0019e0*/ 0x62,0x01,0x2d,0x64,0x62,0xca,0x64,0x62,0x64,0x65,0xfe,0x0f,0x62,0x00,0x00,0x03,
/*0019f0*/ 0x00,0x01,0xff,0x9d,0x01,0xf3,0x01,0xf3,0x00,0x0b,0x00,0x0f,0x00,0x15,0x00,0x8a,
/*001a00*/ 0x40,0x42,0x01,0x16,0x16,0x40,0x17,0x00,0x14,0x13,0x12,0x11,0x13,0x12,0x08,0x07,
/*001a10*/ 0x04,0x05,0x03,0x04,0x06,0x05,0x11,0x10,0x0d,0x0c,0x0a,0x05,0x09,0x04,0x0b,0x00,
/*001a20*/ 0x0f,0x0e,0x04,0x15,0x02,0x01,0x03,0x14,0x0f,0x0c,0x06,0x00,0x0e,0x0d,0x06,0x15,
/*001a30*/ 0x10,0x09,0x08,0x03,0x02,0x0b,0x0a,0x07,0x03,0x06,0x02,0x05,0x04,0x01,0x03,0x00,
/*001a40*/ 0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,0x2f,
/*001a50*/ 0x3c,0x2f,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x3c,
/*001a60*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x2e,0x2e,0x31,
/*001a70*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x16,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*001a80*/ 0x38,0x11,0x37,0xb9,0x00,0x16,0xff,0xc0,0x38,0x59,0x25,0x21,0x15,0x23,0x35,0x23,
/*001a90*/ 0x11,0x33,0x35,0x21,0x15,0x33,0x03,0x35,0x23,0x15,0x37,0x35,0x21,0x11,0x33,0x11,
/*001aa0*/ 0x01,0xf3,0xfe,0xd3,0x61,0x64,0x64,0x01,0x2a,0x64,0x64,0x62,0x62,0xfe,0xd6,0x62,
/*001ab0*/ 0x01,0x64,0x65,0x01,0x8d,0x64,0x65,0xfe,0xd7,0x62,0x62,0xc8,0x62,0xfe,0x72,0x01,
/*001ac0*/ 0x2c,0x00,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x09,0x00,0x0d,
/*001ad0*/ 0x00,0x6b,0x40,0x30,0x01,0x0e,0x0e,0x40,0x0f,0x00,0x0d,0x0c,0x08,0x04,0x03,0x05,
/*001ae0*/ 0x07,0x04,0x06,0x05,0x0b,0x02,0x01,0x03,0x0a,0x04,0x09,0x00,0x0d,0x0a,0x06,0x03,
/*001af0*/ 0x02,0x0c,0x0b,0x07,0x03,0x06,0x06,0x08,0x09,0x08,0x03,0x05,0x04,0x01,0x03,0x00,
/*001b00*/ 0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x3c,0x10,0xfd,
/*001b10*/ 0x17,0x3c,0x2f,0x3c,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,
/*001b20*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0e,0x49,0x68,0x61,0xb0,
/*001b30*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x23,0x35,
/*001b40*/ 0x23,0x15,0x23,0x11,0x33,0x35,0x21,0x03,0x35,0x23,0x15,0x01,0x8f,0x62,0xca,0x62,
/*001b50*/ 0x64,0x01,0x2a,0x64,0xc6,0x01,0xc8,0xc8,0x01,0xf2,0x64,0xfe,0xd6,0xc6,0xc6,0x00,
/*001b60*/ 0x00,0x03,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0b,0x00,0x0f,0x00,0x13,
/*001b70*/ 0x00,0x82,0x40,0x41,0x01,0x14,0x14,0x40,0x15,0x00,0x13,0x12,0x0f,0x03,0x0e,0x04,
/*001b80*/ 0x04,0x03,0x11,0x10,0x0d,0x0c,0x0a,0x09,0x06,0x02,0x01,0x09,0x05,0x04,0x0b,0x08,
/*001b90*/ 0x07,0x03,0x00,0x13,0x10,0x01,0x03,0x00,0x06,0x02,0x0e,0x07,0x06,0x03,0x0d,0x06,
/*001ba0*/ 0x04,0x12,0x0b,0x0a,0x03,0x11,0x06,0x0f,0x0c,0x09,0x03,0x08,0x05,0x04,0x03,0x03,
/*001bb0*/ 0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,0x17,
/*001bc0*/ 0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x17,0x3c,
/*001bd0*/ 0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*001be0*/ 0x03,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,
/*001bf0*/ 0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x21,0x11,0x21,0x15,0x33,0x15,0x23,0x15,0x33,
/*001c00*/ 0x27,0x35,0x23,0x15,0x13,0x35,0x23,0x15,0x01,0x8f,0x65,0xfe,0xd7,0x01,0x2a,0x64,
/*001c10*/ 0x64,0x64,0x64,0xc6,0xc6,0xc6,0x65,0x64,0x02,0x56,0x65,0x61,0x67,0x67,0x62,0x62,
/*001c20*/ 0xfe,0xd4,0xc6,0xc6,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0b,
/*001c30*/ 0x00,0x64,0x40,0x2a,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x07,0x06,0x0b,0x04,0x03,0x00,
/*001c40*/ 0x0a,0x09,0x06,0x05,0x02,0x05,0x01,0x04,0x08,0x07,0x09,0x08,0x01,0x03,0x00,0x06,
/*001c50*/ 0x0a,0x03,0x02,0x06,0x04,0x0b,0x0a,0x03,0x05,0x04,0x01,0x01,0x07,0x46,0x76,0x2f,
/*001c60*/ 0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,
/*001c70*/ 0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x2e,0x2e,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,
/*001c80*/ 0xb9,0x00,0x07,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*001c90*/ 0x00,0x0c,0xff,0xc0,0x38,0x59,0x01,0x21,0x11,0x21,0x15,0x21,0x35,0x23,0x11,0x33,
/*001ca0*/ 0x35,0x21,0x01,0x8f,0xfe,0xd4,0x01,0x2c,0xfe,0xd6,0x64,0x64,0x01,0x2a,0x01,0xf5,
/*001cb0*/ 0xfe,0x6e,0x62,0x65,0x01,0x8d,0x64,0x00,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,
/*001cc0*/ 0x02,0x57,0x00,0x07,0x00,0x0b,0x00,0x65,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,
/*001cd0*/ 0x0b,0x0a,0x04,0x04,0x03,0x09,0x08,0x06,0x02,0x01,0x05,0x05,0x04,0x07,0x00,0x0b,
/*001ce0*/ 0x08,0x01,0x03,0x00,0x06,0x02,0x0a,0x07,0x06,0x03,0x09,0x06,0x04,0x05,0x04,0x03,
/*001cf0*/ 0x03,0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,
/*001d00*/ 0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,
/*001d10*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0c,0x49,0x68,0x61,0xb0,
/*001d20*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,
/*001d30*/ 0x21,0x11,0x21,0x15,0x33,0x03,0x11,0x23,0x11,0x01,0x8f,0x65,0xfe,0xd7,0x01,0x2a,
/*001d40*/ 0x64,0x64,0xc6,0x65,0x64,0x02,0x56,0x65,0xfe,0x73,0x01,0x8e,0xfe,0x72,0x00,0x01,
/*001d50*/ 0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0d,0x00,0x6d,0x40,0x2f,0x01,0x0e,
/*001d60*/ 0x0e,0x40,0x0f,0x00,0x0d,0x08,0x07,0x04,0x03,0x00,0x0c,0x0b,0x06,0x05,0x02,0x05,
/*001d70*/ 0x01,0x04,0x0a,0x09,0x0b,0x0a,0x01,0x03,0x00,0x06,0x0c,0x05,0x04,0x06,0x03,0x02,
/*001d80*/ 0x07,0x06,0x06,0x08,0x0d,0x0c,0x03,0x09,0x08,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,
/*001d90*/ 0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,
/*001da0*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,
/*001db0*/ 0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,
/*001dc0*/ 0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x01,0x21,0x15,0x33,0x15,0x23,0x15,
/*001dd0*/ 0x21,0x15,0x21,0x11,0x33,0x35,0x21,0x01,0x8f,0xfe,0xd4,0xc8,0xc8,0x01,0x2c,0xfe,
/*001de0*/ 0x72,0x64,0x01,0x2a,0x01,0xf5,0x67,0x61,0xca,0x62,0x01,0xf2,0x64,0x00,0x00,0x01,
/*001df0*/ 0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0b,0x00,0x62,0x40,0x29,0x01,0x0c,
/*001e00*/ 0x0c,0x40,0x0d,0x00,0x0b,0x04,0x03,0x00,0x0a,0x09,0x06,0x05,0x02,0x05,0x01,0x04,
/*001e10*/ 0x08,0x07,0x09,0x08,0x01,0x03,0x00,0x06,0x0a,0x05,0x04,0x06,0x03,0x02,0x0b,0x0a,
/*001e20*/ 0x03,0x07,0x06,0x01,0x01,0x07,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,
/*001e30*/ 0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,
/*001e40*/ 0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x07,0x00,0x0c,0x49,0x68,0x61,
/*001e50*/ 0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x01,0x21,
/*001e60*/ 0x15,0x33,0x15,0x23,0x11,0x23,0x11,0x33,0x35,0x21,0x01,0x8f,0xfe,0xd4,0xc8,0xc8,
/*001e70*/ 0x62,0x64,0x01,0x2a,0x01,0xf5,0x67,0x61,0xfe,0xd4,0x01,0xf2,0x64,0x00,0x00,0x01,
/*001e80*/ 0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x7c,0x40,0x39,0x01,0x10,
/*001e90*/ 0x10,0x40,0x11,0x00,0x03,0x02,0x0e,0x0d,0x05,0x00,0x0a,0x06,0x05,0x02,0x01,0x05,
/*001ea0*/ 0x09,0x04,0x04,0x03,0x0c,0x0b,0x04,0x0f,0x08,0x07,0x03,0x00,0x0b,0x0a,0x06,0x00,
/*001eb0*/ 0x09,0x05,0x04,0x03,0x08,0x06,0x06,0x0d,0x0c,0x06,0x0e,0x0f,0x0e,0x02,0x07,0x06,
/*001ec0*/ 0x03,0x01,0x00,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,
/*001ed0*/ 0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,
/*001ee0*/ 0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,
/*001ef0*/ 0x49,0x68,0xb9,0x00,0x03,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*001f00*/ 0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x11,0x33,0x35,0x21,
/*001f10*/ 0x15,0x21,0x11,0x33,0x35,0x23,0x35,0x33,0x01,0x8f,0xfe,0xd6,0x64,0x64,0x01,0x2a,
/*001f20*/ 0xfe,0xd4,0xca,0x64,0xc6,0x01,0x65,0x01,0x8d,0x64,0x62,0xfe,0x6e,0xcb,0x61,0x00,
/*001f30*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0b,0x00,0x65,0x40,0x2c,
/*001f40*/ 0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0a,0x09,0x02,0x03,0x01,0x04,0x0b,0x00,0x08,0x07,
/*001f50*/ 0x04,0x03,0x03,0x04,0x06,0x05,0x03,0x02,0x06,0x08,0x09,0x08,0x02,0x0b,0x0a,0x07,
/*001f60*/ 0x03,0x06,0x03,0x05,0x04,0x01,0x03,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,
/*001f70*/ 0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,
/*001f80*/ 0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,
/*001f90*/ 0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,
/*001fa0*/ 0xc0,0x38,0x59,0x25,0x23,0x11,0x23,0x11,0x23,0x11,0x33,0x15,0x33,0x35,0x33,0x01,
/*001fb0*/ 0x8f,0x62,0xca,0x62,0x62,0xca,0x62,0x01,0x01,0x2c,0xfe,0xd4,0x02,0x56,0xc8,0xc8,
/*001fc0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x2b,0x02,0x57,0x00,0x0b,0x00,0x6c,0x40,0x30,
/*001fd0*/ 0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0b,0x08,0x07,0x03,0x00,0x08,0x09,0x06,0x05,0x02,
/*001fe0*/ 0x03,0x01,0x08,0x03,0x04,0x03,0x04,0x0a,0x09,0x0b,0x0a,0x03,0x03,0x02,0x06,0x00,
/*001ff0*/ 0x09,0x08,0x05,0x03,0x04,0x06,0x06,0x07,0x06,0x03,0x01,0x00,0x01,0x01,0x01,0x46,
/*002000*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,
/*002010*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x00,0x31,
/*002020*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*002030*/ 0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x33,0x11,0x23,
/*002040*/ 0x35,0x21,0x15,0x23,0x11,0x33,0x01,0x2b,0xfe,0xd6,0x64,0x64,0x01,0x2a,0x64,0x64,
/*002050*/ 0x01,0x62,0x01,0x92,0x62,0x62,0xfe,0x6d,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,
/*002060*/ 0x02,0x57,0x00,0x09,0x00,0x61,0x40,0x27,0x01,0x0a,0x0a,0x40,0x0b,0x00,0x04,0x03,
/*002070*/ 0x02,0x01,0x08,0x07,0x05,0x00,0x06,0x05,0x04,0x09,0x00,0x05,0x01,0x00,0x03,0x04,
/*002080*/ 0x06,0x02,0x07,0x06,0x06,0x08,0x09,0x08,0x03,0x03,0x02,0x01,0x01,0x03,0x46,0x76,
/*002090*/ 0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,
/*0020a0*/ 0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,
/*0020b0*/ 0x68,0xb9,0x00,0x03,0x00,0x0a,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*0020c0*/ 0xb9,0x00,0x0a,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x21,0x35,0x21,0x11,0x23,0x35,
/*0020d0*/ 0x33,0x01,0x8f,0x65,0xfe,0xd7,0x01,0x2c,0x64,0xc6,0x65,0x64,0x62,0x01,0x92,0x62,
/*0020e0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x76,0x40,0x37,
/*0020f0*/ 0x01,0x10,0x10,0x40,0x11,0x00,0x0f,0x0e,0x0e,0x0d,0x0a,0x09,0x02,0x05,0x01,0x04,
/*002100*/ 0x0f,0x0c,0x0b,0x03,0x00,0x08,0x07,0x04,0x03,0x03,0x04,0x06,0x05,0x0d,0x0c,0x07,
/*002110*/ 0x06,0x03,0x02,0x06,0x08,0x09,0x08,0x02,0x0b,0x0a,0x07,0x03,0x06,0x03,0x05,0x04,
/*002120*/ 0x01,0x03,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,
/*002130*/ 0x17,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,
/*002140*/ 0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*002150*/ 0x05,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,
/*002160*/ 0xff,0xc0,0x38,0x59,0x25,0x23,0x11,0x23,0x11,0x23,0x11,0x33,0x15,0x33,0x35,0x33,
/*002170*/ 0x15,0x23,0x15,0x33,0x01,0x8f,0x62,0xca,0x62,0x62,0xca,0x62,0x64,0x64,0x01,0x01,
/*002180*/ 0x2c,0xfe,0xd4,0x02,0x56,0xc8,0xc8,0xc6,0x67,0x00,0x00,0x01,0x00,0x01,0x00,0x01,
/*002190*/ 0x01,0x8f,0x02,0x57,0x00,0x07,0x00,0x53,0x40,0x1f,0x01,0x08,0x08,0x40,0x09,0x00,
/*0021a0*/ 0x03,0x02,0x07,0x00,0x06,0x02,0x01,0x03,0x05,0x04,0x04,0x03,0x07,0x06,0x06,0x00,
/*0021b0*/ 0x05,0x04,0x03,0x01,0x00,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,
/*0021c0*/ 0x3f,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x00,0x2e,0x2e,
/*0021d0*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,
/*0021e0*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x11,
/*0021f0*/ 0x33,0x11,0x21,0x01,0x8f,0xfe,0xd6,0x64,0x62,0x01,0x2c,0x01,0x65,0x01,0xf1,0xfe,
/*002200*/ 0x0c,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0xf3,0x02,0x57,0x00,0x13,0x00,0x8a,
/*002210*/ 0x40,0x43,0x01,0x14,0x14,0x40,0x15,0x00,0x10,0x0f,0x05,0x00,0x12,0x11,0x02,0x03,
/*002220*/ 0x01,0x04,0x13,0x00,0x04,0x03,0x04,0x0e,0x0d,0x06,0x03,0x05,0x0c,0x0b,0x08,0x03,
/*002230*/ 0x07,0x04,0x0a,0x09,0x07,0x06,0x03,0x03,0x02,0x06,0x11,0x0d,0x0c,0x03,0x10,0x05,
/*002240*/ 0x04,0x06,0x0e,0x0f,0x0e,0x02,0x13,0x12,0x0b,0x03,0x0a,0x03,0x09,0x08,0x01,0x03,
/*002250*/ 0x00,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,
/*002260*/ 0x3f,0x3c,0x10,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,
/*002270*/ 0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,
/*002280*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*002290*/ 0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x25,0x23,0x11,0x23,0x15,0x23,
/*0022a0*/ 0x35,0x23,0x11,0x23,0x11,0x33,0x15,0x33,0x15,0x33,0x35,0x33,0x35,0x33,0x01,0xf3,
/*0022b0*/ 0x62,0x67,0x61,0x67,0x61,0x62,0x64,0x67,0x64,0x61,0x01,0x01,0x90,0x64,0x64,0xfe,
/*0022c0*/ 0x70,0x02,0x56,0x65,0x63,0x64,0x64,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,
/*0022d0*/ 0x02,0x57,0x00,0x0f,0x00,0x79,0x40,0x38,0x01,0x10,0x10,0x40,0x11,0x00,0x0c,0x04,
/*0022e0*/ 0x03,0x03,0x0b,0x05,0x07,0x0e,0x0d,0x02,0x03,0x01,0x04,0x0f,0x00,0x0a,0x09,0x06,
/*0022f0*/ 0x03,0x05,0x04,0x08,0x07,0x03,0x02,0x06,0x0c,0x05,0x04,0x06,0x0b,0x0a,0x0d,0x0c,
/*002300*/ 0x02,0x0f,0x0e,0x09,0x03,0x08,0x03,0x07,0x06,0x01,0x03,0x00,0x01,0x01,0x07,0x46,
/*002310*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,0x3f,0x3c,0x2f,0x3c,0xfd,
/*002320*/ 0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,
/*002330*/ 0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x07,0x00,0x10,0x49,0x68,
/*002340*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,
/*002350*/ 0x23,0x11,0x23,0x35,0x23,0x11,0x23,0x11,0x33,0x15,0x33,0x15,0x33,0x35,0x33,0x01,
/*002360*/ 0x8f,0x62,0x64,0x67,0x61,0x62,0x64,0x67,0x61,0x01,0x01,0x2d,0x63,0xfe,0x70,0x02,
/*002370*/ 0x56,0x65,0x63,0xc8,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0b,
/*002380*/ 0x00,0x0f,0x00,0x6f,0x40,0x35,0x01,0x10,0x10,0x40,0x11,0x00,0x0f,0x0e,0x08,0x07,
/*002390*/ 0x04,0x05,0x03,0x04,0x06,0x05,0x0d,0x0c,0x0a,0x02,0x01,0x05,0x09,0x04,0x0b,0x00,
/*0023a0*/ 0x0f,0x0c,0x05,0x04,0x01,0x05,0x00,0x06,0x02,0x0e,0x0d,0x0b,0x0a,0x07,0x05,0x06,
/*0023b0*/ 0x06,0x08,0x09,0x08,0x03,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,
/*0023c0*/ 0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,
/*0023d0*/ 0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,
/*0023e0*/ 0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,
/*0023f0*/ 0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,0x11,0x33,0x35,0x33,0x15,0x33,0x03,
/*002400*/ 0x11,0x23,0x11,0x01,0x8f,0x65,0xc5,0x64,0x64,0xc6,0x64,0x64,0xc6,0x65,0x64,0x65,
/*002410*/ 0x01,0x8d,0x64,0x65,0xfe,0x73,0x01,0x8e,0xfe,0x72,0x00,0x02,0x00,0x01,0x00,0x01,
/*002420*/ 0x01,0x8f,0x02,0x57,0x00,0x09,0x00,0x0d,0x00,0x6b,0x40,0x30,0x01,0x0e,0x0e,0x40,
/*002430*/ 0x0f,0x00,0x0d,0x04,0x03,0x03,0x0c,0x04,0x06,0x05,0x0b,0x0a,0x08,0x02,0x01,0x05,
/*002440*/ 0x07,0x04,0x09,0x00,0x0d,0x0a,0x01,0x03,0x00,0x06,0x03,0x02,0x0c,0x09,0x08,0x03,
/*002450*/ 0x0b,0x06,0x06,0x07,0x06,0x03,0x05,0x04,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,
/*002460*/ 0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x01,0x2f,
/*002470*/ 0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,
/*002480*/ 0x00,0x05,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,
/*002490*/ 0x0e,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,0x15,0x23,0x11,0x21,0x15,0x33,0x07,
/*0024a0*/ 0x35,0x23,0x15,0x01,0x8f,0x65,0xc7,0x62,0x01,0x2a,0x64,0x64,0xc6,0x01,0x2d,0x64,
/*0024b0*/ 0xc8,0x02,0x56,0x65,0xc5,0xc6,0xc6,0x00,0x00,0x03,0x00,0x01,0x00,0x01,0x01,0x8f,
/*0024c0*/ 0x02,0x57,0x00,0x09,0x00,0x0f,0x00,0x13,0x00,0x85,0x40,0x42,0x01,0x14,0x14,0x40,
/*0024d0*/ 0x15,0x00,0x11,0x0f,0x0e,0x03,0x10,0x05,0x00,0x13,0x12,0x0d,0x0c,0x06,0x05,0x02,
/*0024e0*/ 0x07,0x01,0x04,0x04,0x03,0x0b,0x0a,0x08,0x03,0x07,0x04,0x09,0x00,0x13,0x03,0x02,
/*0024f0*/ 0x03,0x10,0x06,0x00,0x0c,0x0b,0x09,0x08,0x05,0x05,0x04,0x06,0x06,0x0e,0x0d,0x06,
/*002500*/ 0x12,0x0f,0x0a,0x03,0x11,0x07,0x06,0x03,0x01,0x00,0x01,0x01,0x03,0x46,0x76,0x2f,
/*002510*/ 0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,
/*002520*/ 0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,
/*002530*/ 0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x14,0x49,0x68,
/*002540*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x25,
/*002550*/ 0x21,0x35,0x23,0x11,0x33,0x35,0x33,0x15,0x33,0x03,0x11,0x23,0x15,0x33,0x15,0x07,
/*002560*/ 0x35,0x23,0x15,0x01,0x8f,0xfe,0xd6,0x64,0x64,0xc6,0x64,0x64,0xc6,0x64,0x02,0x62,
/*002570*/ 0x01,0x65,0x01,0x8d,0x64,0x65,0xfe,0xd7,0x01,0x2a,0xc5,0x65,0x64,0x62,0x62,0x00,
/*002580*/ 0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0d,0x00,0x11,0x00,0x7c,
/*002590*/ 0x40,0x3c,0x01,0x12,0x12,0x40,0x13,0x00,0x11,0x04,0x03,0x03,0x10,0x04,0x06,0x05,
/*0025a0*/ 0x0f,0x0e,0x0c,0x0b,0x08,0x02,0x01,0x07,0x07,0x04,0x0d,0x0a,0x09,0x03,0x00,0x0d,
/*0025b0*/ 0x0c,0x07,0x00,0x11,0x0e,0x0b,0x03,0x0a,0x06,0x03,0x02,0x10,0x09,0x08,0x03,0x0f,
/*0025c0*/ 0x06,0x06,0x07,0x06,0x03,0x05,0x04,0x01,0x03,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,
/*0025d0*/ 0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,
/*0025e0*/ 0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,
/*0025f0*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x12,0x49,0x68,0x61,0xb0,0x40,
/*002600*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x12,0xff,0xc0,0x38,0x59,0x25,0x23,0x35,0x23,
/*002610*/ 0x15,0x23,0x11,0x21,0x15,0x33,0x15,0x23,0x15,0x33,0x27,0x35,0x23,0x15,0x01,0x8f,
/*002620*/ 0x62,0xca,0x62,0x01,0x2a,0x64,0x64,0x64,0x64,0xc6,0x01,0xc8,0xc8,0x02,0x56,0x65,
/*002630*/ 0xc5,0x67,0x67,0xc6,0xc6,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,
/*002640*/ 0x00,0x17,0x00,0xa3,0x40,0x53,0x01,0x18,0x18,0x40,0x19,0x00,0x17,0x10,0x0f,0x03,
/*002650*/ 0x00,0x04,0x05,0x16,0x06,0x05,0x02,0x01,0x05,0x15,0x04,0x07,0x14,0x08,0x07,0x03,
/*002660*/ 0x13,0x04,0x09,0x12,0x0e,0x0d,0x0a,0x09,0x05,0x11,0x04,0x0c,0x0b,0x04,0x03,0x03,
/*002670*/ 0x17,0x16,0x06,0x00,0x05,0x01,0x00,0x03,0x04,0x06,0x02,0x15,0x14,0x06,0x07,0x06,
/*002680*/ 0x09,0x08,0x06,0x13,0x12,0x0b,0x0a,0x06,0x0c,0x11,0x0d,0x0c,0x03,0x10,0x06,0x0e,
/*002690*/ 0x0f,0x0e,0x03,0x03,0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,
/*0026a0*/ 0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,
/*0026b0*/ 0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,
/*0026c0*/ 0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,
/*0026d0*/ 0x68,0xb9,0x00,0x03,0x00,0x18,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*0026e0*/ 0xb9,0x00,0x18,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x21,0x35,0x21,0x35,0x23,0x35,
/*0026f0*/ 0x23,0x35,0x23,0x35,0x33,0x35,0x21,0x15,0x21,0x15,0x33,0x15,0x33,0x15,0x33,0x01,
/*002700*/ 0x8f,0x65,0xfe,0xd7,0x01,0x2c,0x64,0x64,0x64,0x64,0x01,0x2a,0xfe,0xd4,0x64,0x64,
/*002710*/ 0x64,0x65,0x64,0x62,0x67,0x64,0x64,0x61,0x64,0x62,0x67,0x64,0x64,0x00,0x00,0x01,
/*002720*/ 0x00,0x01,0x00,0x01,0x01,0xf3,0x02,0x57,0x00,0x07,0x00,0x53,0x40,0x1f,0x01,0x08,
/*002730*/ 0x08,0x40,0x09,0x00,0x07,0x06,0x05,0x00,0x02,0x01,0x04,0x04,0x03,0x05,0x04,0x01,
/*002740*/ 0x03,0x00,0x06,0x06,0x07,0x06,0x03,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,
/*002750*/ 0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2e,
/*002760*/ 0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x08,0x49,0x68,
/*002770*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x01,
/*002780*/ 0x23,0x11,0x23,0x11,0x23,0x35,0x21,0x01,0xf3,0xc8,0x62,0xc8,0x01,0xf2,0x01,0xf5,
/*002790*/ 0xfe,0x0c,0x01,0xf4,0x62,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,
/*0027a0*/ 0x00,0x0b,0x00,0x67,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x05,0x04,0x02,0x01,
/*0027b0*/ 0x05,0x03,0x08,0x04,0x03,0x03,0x07,0x04,0x06,0x05,0x0a,0x09,0x04,0x0b,0x00,0x09,
/*0027c0*/ 0x01,0x00,0x03,0x08,0x06,0x02,0x0b,0x0a,0x07,0x03,0x06,0x03,0x03,0x02,0x01,0x01,
/*0027d0*/ 0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x10,0xfd,0x17,0x3c,
/*0027e0*/ 0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x2e,0x2e,
/*0027f0*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,
/*002800*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,
/*002810*/ 0x23,0x11,0x33,0x11,0x33,0x11,0x33,0x01,0x8f,0x65,0xc5,0x64,0x62,0xca,0x62,0x65,
/*002820*/ 0x64,0x65,0x01,0xf1,0xfe,0x0c,0x01,0xf4,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,
/*002830*/ 0x02,0x57,0x00,0x0f,0x00,0x79,0x40,0x37,0x01,0x10,0x10,0x40,0x11,0x00,0x04,0x03,
/*002840*/ 0x05,0x07,0x0a,0x09,0x06,0x03,0x05,0x04,0x08,0x07,0x0c,0x0b,0x04,0x02,0x01,0x0e,
/*002850*/ 0x0d,0x04,0x0f,0x00,0x0d,0x01,0x00,0x03,0x0c,0x06,0x02,0x0b,0x03,0x02,0x03,0x0a,
/*002860*/ 0x06,0x05,0x04,0x0f,0x0e,0x09,0x03,0x08,0x03,0x07,0x06,0x01,0x01,0x07,0x46,0x76,
/*002870*/ 0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,
/*002880*/ 0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,
/*002890*/ 0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x07,0x00,0x10,0x49,0x68,
/*0028a0*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x01,
/*0028b0*/ 0x23,0x15,0x23,0x15,0x23,0x15,0x23,0x11,0x33,0x11,0x33,0x35,0x33,0x11,0x33,0x01,
/*0028c0*/ 0x8f,0x65,0x64,0x64,0x61,0x62,0x67,0x64,0x61,0x01,0x2d,0x64,0x64,0x64,0x02,0x56,
/*0028d0*/ 0xfe,0x70,0x64,0x01,0x2c,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0xf3,0x02,0x57,
/*0028e0*/ 0x00,0x13,0x00,0x87,0x40,0x41,0x01,0x14,0x14,0x40,0x15,0x00,0x0f,0x0e,0x09,0x08,
/*0028f0*/ 0x02,0x01,0x04,0x03,0x06,0x05,0x04,0x07,0x0c,0x08,0x07,0x03,0x0b,0x04,0x0a,0x09,
/*002900*/ 0x0e,0x0d,0x04,0x10,0x04,0x03,0x03,0x0f,0x12,0x11,0x04,0x13,0x00,0x11,0x10,0x0d,
/*002910*/ 0x05,0x04,0x01,0x00,0x07,0x0c,0x06,0x02,0x13,0x12,0x0b,0x03,0x0a,0x03,0x07,0x06,
/*002920*/ 0x03,0x03,0x02,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,
/*002930*/ 0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,
/*002940*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x00,0x2e,0x2e,0x2e,0x2e,
/*002950*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,
/*002960*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,
/*002970*/ 0x23,0x15,0x23,0x35,0x23,0x11,0x33,0x11,0x33,0x11,0x33,0x11,0x33,0x11,0x33,0x01,
/*002980*/ 0xf3,0x65,0x61,0x67,0x61,0x64,0x62,0x67,0x61,0x66,0x62,0x65,0x64,0x64,0x64,0x65,
/*002990*/ 0x01,0xf1,0xfe,0x0c,0x01,0x90,0xfe,0x70,0x01,0xf4,0x00,0x01,0x00,0x01,0x00,0x01,
/*0029a0*/ 0x01,0x8f,0x02,0x57,0x00,0x13,0x00,0x87,0x40,0x44,0x01,0x14,0x14,0x40,0x15,0x00,
/*0029b0*/ 0x12,0x11,0x06,0x02,0x01,0x05,0x05,0x04,0x13,0x04,0x03,0x03,0x00,0x10,0x0f,0x0c,
/*0029c0*/ 0x0b,0x08,0x05,0x07,0x04,0x0e,0x0d,0x0a,0x03,0x09,0x0d,0x0c,0x01,0x03,0x00,0x07,
/*0029d0*/ 0x0e,0x03,0x02,0x07,0x04,0x0b,0x0a,0x07,0x03,0x06,0x07,0x10,0x11,0x10,0x02,0x13,
/*0029e0*/ 0x12,0x0f,0x03,0x0e,0x03,0x09,0x08,0x05,0x03,0x04,0x01,0x01,0x09,0x46,0x76,0x2f,
/*0029f0*/ 0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,
/*002a00*/ 0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,
/*002a10*/ 0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,0x68,
/*002a20*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x01,
/*002a30*/ 0x23,0x15,0x33,0x15,0x23,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x15,
/*002a40*/ 0x33,0x35,0x33,0x01,0x8f,0x64,0x64,0x62,0xca,0x62,0x64,0x64,0x62,0xca,0x62,0x01,
/*002a50*/ 0x91,0xcb,0xc5,0xc8,0xc8,0xc6,0xca,0xc6,0xc8,0xc8,0x00,0x01,0x00,0x01,0x00,0x01,
/*002a60*/ 0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x76,0x40,0x36,0x01,0x10,0x10,0x40,0x11,0x00,
/*002a70*/ 0x09,0x08,0x02,0x01,0x05,0x03,0x0e,0x0d,0x06,0x03,0x05,0x04,0x0f,0x00,0x0c,0x08,
/*002a80*/ 0x07,0x04,0x03,0x05,0x0b,0x04,0x0a,0x09,0x05,0x01,0x00,0x03,0x04,0x06,0x02,0x0d,
/*002a90*/ 0x0c,0x06,0x07,0x06,0x0f,0x0e,0x0b,0x03,0x0a,0x03,0x03,0x02,0x01,0x01,0x09,0x46,
/*002aa0*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,
/*002ab0*/ 0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,
/*002ac0*/ 0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x10,0x49,0x68,0x61,
/*002ad0*/ 0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x23,
/*002ae0*/ 0x15,0x23,0x35,0x33,0x35,0x23,0x35,0x23,0x11,0x33,0x11,0x33,0x11,0x33,0x01,0x8f,
/*002af0*/ 0x65,0xc5,0xc8,0xc8,0x64,0x62,0xca,0x62,0x65,0x64,0x62,0x67,0x64,0x01,0x29,0xfe,
/*002b00*/ 0xd4,0x01,0x2c,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x13,
/*002b10*/ 0x00,0x8e,0x40,0x43,0x01,0x14,0x14,0x40,0x15,0x00,0x06,0x05,0x04,0x12,0x11,0x0a,
/*002b20*/ 0x03,0x09,0x04,0x03,0x04,0x0c,0x0b,0x0e,0x0d,0x04,0x02,0x01,0x10,0x0f,0x04,0x13,
/*002b30*/ 0x08,0x07,0x03,0x00,0x01,0x00,0x07,0x12,0x0d,0x03,0x02,0x03,0x0c,0x06,0x0b,0x0a,
/*002b40*/ 0x05,0x03,0x04,0x07,0x06,0x06,0x08,0x11,0x10,0x06,0x12,0x13,0x12,0x03,0x0f,0x0e,
/*002b50*/ 0x02,0x09,0x08,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,
/*002b60*/ 0x3f,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,
/*002b70*/ 0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,
/*002b80*/ 0x17,0x3c,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x14,0x49,
/*002b90*/ 0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,
/*002ba0*/ 0x01,0x23,0x15,0x23,0x15,0x23,0x15,0x21,0x15,0x21,0x35,0x33,0x35,0x33,0x35,0x33,
/*002bb0*/ 0x35,0x21,0x35,0x21,0x01,0x8f,0x65,0x64,0x63,0x01,0x2c,0xfe,0x72,0x65,0x64,0x63,
/*002bc0*/ 0xfe,0xd4,0x01,0x8e,0x01,0x91,0x64,0x64,0x67,0x61,0xc6,0x64,0x64,0x66,0x62,0x00,
/*002bd0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x00,0xc7,0x02,0x57,0x00,0x07,0x00,0x59,0x40,0x23,
/*002be0*/ 0x01,0x08,0x08,0x40,0x09,0x00,0x07,0x04,0x03,0x03,0x00,0x05,0x01,0x06,0x05,0x04,
/*002bf0*/ 0x02,0x01,0x07,0x06,0x06,0x00,0x05,0x04,0x06,0x02,0x03,0x02,0x03,0x01,0x00,0x01,
/*002c00*/ 0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,
/*002c10*/ 0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,
/*002c20*/ 0x68,0xb9,0x00,0x01,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*002c30*/ 0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x37,0x23,0x11,0x33,0x15,0x23,0x11,0x33,0xc7,
/*002c40*/ 0xc6,0xc6,0x64,0x64,0x01,0x02,0x56,0x62,0xfe,0x6d,0x00,0x01,0x00,0x01,0x00,0x01,
/*002c50*/ 0x01,0x2b,0x02,0x57,0x00,0x0b,0x00,0x6d,0x40,0x2f,0x01,0x0c,0x0c,0x40,0x0d,0x00,
/*002c60*/ 0x0b,0x00,0x04,0x01,0x0a,0x02,0x01,0x03,0x09,0x04,0x03,0x08,0x04,0x03,0x03,0x07,
/*002c70*/ 0x04,0x06,0x05,0x0b,0x0a,0x07,0x00,0x03,0x02,0x07,0x09,0x08,0x05,0x04,0x07,0x06,
/*002c80*/ 0x07,0x06,0x03,0x01,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,
/*002c90*/ 0x3f,0x3c,0x10,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,
/*002ca0*/ 0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,
/*002cb0*/ 0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,
/*002cc0*/ 0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x35,0x23,0x35,0x23,0x35,0x33,0x15,0x33,0x15,
/*002cd0*/ 0x33,0x01,0x2b,0x62,0x64,0x64,0x62,0x64,0x64,0x01,0xc9,0xc7,0xc6,0xc9,0xc8,0x00,
/*002ce0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x00,0xc7,0x02,0x57,0x00,0x07,0x00,0x59,0x40,0x23,
/*002cf0*/ 0x01,0x08,0x08,0x40,0x09,0x00,0x06,0x05,0x02,0x03,0x01,0x05,0x00,0x04,0x03,0x04,
/*002d00*/ 0x07,0x00,0x03,0x02,0x06,0x00,0x05,0x04,0x06,0x06,0x07,0x06,0x03,0x01,0x00,0x01,
/*002d10*/ 0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x10,
/*002d20*/ 0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,
/*002d30*/ 0x68,0xb9,0x00,0x01,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*002d40*/ 0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x37,0x23,0x35,0x33,0x11,0x23,0x35,0x33,0xc7,
/*002d50*/ 0xc6,0x64,0x64,0xc6,0x01,0x62,0x01,0x92,0x62,0x00,0x00,0x01,0x00,0x01,0x01,0x91,
/*002d60*/ 0x01,0x2b,0x02,0x57,0x00,0x0b,0x00,0x69,0x40,0x2d,0x01,0x0c,0x0c,0x40,0x0d,0x00,
/*002d70*/ 0x0b,0x00,0x04,0x01,0x04,0x03,0x04,0x06,0x05,0x08,0x07,0x04,0x0a,0x02,0x01,0x03,
/*002d80*/ 0x09,0x0b,0x0a,0x06,0x00,0x07,0x06,0x03,0x03,0x02,0x06,0x08,0x05,0x04,0x01,0x03,
/*002d90*/ 0x00,0x09,0x08,0x03,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x2f,0x17,
/*002da0*/ 0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,
/*002db0*/ 0xfd,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,
/*002dc0*/ 0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,
/*002dd0*/ 0x59,0x01,0x23,0x35,0x23,0x15,0x23,0x35,0x33,0x35,0x33,0x15,0x33,0x01,0x2b,0x62,
/*002de0*/ 0x67,0x61,0x64,0x62,0x64,0x01,0x91,0x64,0x64,0x62,0x64,0x65,0x00,0x01,0x00,0x01,
/*002df0*/ 0x00,0x01,0x01,0x8f,0x00,0x63,0x00,0x03,0x00,0x3e,0x40,0x12,0x01,0x04,0x04,0x40,
/*002e00*/ 0x05,0x00,0x03,0x02,0x01,0x00,0x03,0x02,0x01,0x00,0x01,0x01,0x01,0x46,0x76,0x2f,
/*002e10*/ 0x37,0x18,0x00,0x3f,0x3c,0x2f,0x3c,0x01,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,
/*002e20*/ 0x49,0x68,0xb9,0x00,0x01,0x00,0x04,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*002e30*/ 0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x21,0x01,0x8f,0xfe,0x72,
/*002e40*/ 0x01,0x8e,0x01,0x62,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x07,
/*002e50*/ 0x00,0x0b,0x00,0x65,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0b,0x0a,0x06,0x05,
/*002e60*/ 0x02,0x05,0x01,0x04,0x04,0x03,0x09,0x08,0x04,0x07,0x00,0x0b,0x03,0x02,0x03,0x08,
/*002e70*/ 0x06,0x00,0x0a,0x09,0x05,0x03,0x04,0x06,0x06,0x07,0x06,0x02,0x01,0x00,0x01,0x01,
/*002e80*/ 0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,
/*002e90*/ 0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,
/*002ea0*/ 0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,
/*002eb0*/ 0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x35,0x33,0x35,
/*002ec0*/ 0x21,0x03,0x35,0x23,0x15,0x01,0x8f,0xfe,0xd6,0x64,0x64,0x01,0x2a,0x64,0xc6,0x01,
/*002ed0*/ 0x65,0xc5,0x64,0xfe,0xd6,0xc6,0xc6,0x00,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,
/*002ee0*/ 0x02,0x57,0x00,0x09,0x00,0x0d,0x00,0x6e,0x40,0x32,0x01,0x0e,0x0e,0x40,0x0f,0x00,
/*002ef0*/ 0x0d,0x06,0x05,0x03,0x0c,0x04,0x04,0x03,0x0b,0x0a,0x08,0x02,0x01,0x05,0x07,0x04,
/*002f00*/ 0x09,0x00,0x0d,0x0a,0x01,0x03,0x00,0x06,0x02,0x0c,0x09,0x08,0x03,0x0b,0x06,0x06,
/*002f10*/ 0x07,0x06,0x02,0x05,0x04,0x03,0x03,0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,
/*002f20*/ 0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,
/*002f30*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,
/*002f40*/ 0xb9,0x00,0x03,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*002f50*/ 0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x21,0x11,0x33,0x15,0x33,0x15,0x33,
/*002f60*/ 0x07,0x35,0x23,0x15,0x01,0x8f,0x65,0xfe,0xd7,0x62,0xc8,0x64,0x64,0xc6,0x65,0x64,
/*002f70*/ 0x02,0x56,0xc8,0x65,0xc5,0xc6,0xc6,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,
/*002f80*/ 0x01,0x8f,0x00,0x0b,0x00,0x67,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0b,0x04,
/*002f90*/ 0x03,0x00,0x0a,0x09,0x06,0x05,0x02,0x05,0x01,0x04,0x08,0x07,0x07,0x06,0x07,0x08,
/*002fa0*/ 0x09,0x08,0x01,0x03,0x00,0x06,0x0a,0x03,0x02,0x06,0x04,0x0b,0x0a,0x02,0x05,0x04,
/*002fb0*/ 0x01,0x01,0x07,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,
/*002fc0*/ 0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2e,0x2e,0x2e,
/*002fd0*/ 0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x07,0x00,0x0c,0x49,0x68,0x61,0xb0,
/*002fe0*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x01,0x21,0x15,
/*002ff0*/ 0x21,0x15,0x21,0x35,0x23,0x35,0x33,0x35,0x21,0x01,0x8f,0xfe,0xd4,0x01,0x2c,0xfe,
/*003000*/ 0xd6,0x64,0x64,0x01,0x2a,0x01,0x2d,0xca,0x62,0x65,0xc5,0x64,0x00,0x02,0x00,0x01,
/*003010*/ 0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x09,0x00,0x0d,0x00,0x6e,0x40,0x32,0x01,0x0e,
/*003020*/ 0x0e,0x40,0x0f,0x00,0x0d,0x0c,0x06,0x05,0x02,0x05,0x01,0x04,0x04,0x03,0x0b,0x08,
/*003030*/ 0x07,0x03,0x0a,0x04,0x09,0x00,0x0d,0x03,0x02,0x03,0x0a,0x06,0x00,0x0c,0x0b,0x05,
/*003040*/ 0x03,0x04,0x06,0x06,0x09,0x08,0x03,0x07,0x06,0x02,0x01,0x00,0x01,0x01,0x03,0x46,
/*003050*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,
/*003060*/ 0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,
/*003070*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*003080*/ 0x38,0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x35,0x33,
/*003090*/ 0x35,0x33,0x35,0x33,0x03,0x35,0x23,0x15,0x01,0x8f,0xfe,0xd6,0x64,0x65,0xc7,0x62,
/*0030a0*/ 0x64,0xc6,0x01,0x65,0xc5,0x64,0xc8,0xfe,0x0e,0xc6,0xc6,0x00,0x00,0x02,0x00,0x01,
/*0030b0*/ 0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x0d,0x00,0x11,0x00,0x89,0x40,0x43,0x01,0x12,
/*0030c0*/ 0x12,0x40,0x13,0x00,0x0c,0x0b,0x05,0x05,0x0d,0x04,0x03,0x03,0x00,0x05,0x01,0x0f,
/*0030d0*/ 0x0e,0x02,0x03,0x01,0x04,0x05,0x11,0x10,0x0a,0x09,0x06,0x05,0x05,0x04,0x08,0x07,
/*0030e0*/ 0x07,0x06,0x07,0x08,0x11,0x0e,0x01,0x03,0x00,0x06,0x0c,0x03,0x02,0x06,0x04,0x10,
/*0030f0*/ 0x0f,0x0d,0x0c,0x09,0x05,0x08,0x06,0x0a,0x0b,0x0a,0x02,0x05,0x04,0x01,0x01,0x07,
/*003100*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,
/*003110*/ 0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,
/*003120*/ 0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,
/*003130*/ 0x00,0x07,0x00,0x12,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,
/*003140*/ 0x12,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x33,0x15,0x21,0x35,0x23,0x35,0x33,0x35,
/*003150*/ 0x33,0x15,0x33,0x07,0x35,0x23,0x15,0x01,0x8f,0xc8,0xc8,0xfe,0xd6,0x64,0x64,0xc6,
/*003160*/ 0x64,0xc8,0x62,0xc9,0x67,0x61,0x65,0xc5,0x64,0x65,0x61,0x62,0x62,0x00,0x00,0x01,
/*003170*/ 0x00,0x01,0x00,0x01,0x01,0x2b,0x02,0x57,0x00,0x0b,0x00,0x68,0x40,0x2d,0x01,0x0c,
/*003180*/ 0x0c,0x40,0x0d,0x00,0x04,0x03,0x05,0x07,0x0b,0x00,0x05,0x09,0x0a,0x09,0x06,0x05,
/*003190*/ 0x02,0x05,0x01,0x04,0x08,0x07,0x09,0x08,0x01,0x03,0x00,0x06,0x0a,0x05,0x04,0x06,
/*0031a0*/ 0x03,0x02,0x0b,0x0a,0x03,0x07,0x06,0x01,0x01,0x07,0x46,0x76,0x2f,0x37,0x18,0x00,
/*0031b0*/ 0x3f,0x3c,0x3f,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,
/*0031c0*/ 0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*0031d0*/ 0x07,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,
/*0031e0*/ 0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x33,0x15,0x23,0x11,0x23,0x11,0x33,0x35,0x33,
/*0031f0*/ 0x01,0x2b,0xc8,0x64,0x65,0x61,0x64,0xc6,0x01,0xf5,0x67,0x61,0xfe,0xd4,0x01,0xf2,
/*003200*/ 0x64,0x00,0x00,0x02,0x00,0x01,0xff,0x39,0x01,0x8f,0x01,0x8f,0x00,0x0d,0x00,0x11,
/*003210*/ 0x00,0x7a,0x40,0x3b,0x01,0x12,0x12,0x40,0x13,0x00,0x11,0x10,0x0c,0x0b,0x08,0x07,
/*003220*/ 0x04,0x07,0x03,0x04,0x0a,0x09,0x0f,0x06,0x05,0x02,0x01,0x05,0x0e,0x04,0x0d,0x00,
/*003230*/ 0x05,0x01,0x00,0x03,0x04,0x06,0x02,0x11,0x09,0x08,0x03,0x0e,0x06,0x07,0x06,0x10,
/*003240*/ 0x0f,0x0b,0x03,0x0a,0x06,0x0c,0x0d,0x0c,0x02,0x03,0x02,0x00,0x01,0x09,0x46,0x76,
/*003250*/ 0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,
/*003260*/ 0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,
/*003270*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x12,0x49,0x68,0x61,0xb0,0x40,
/*003280*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x12,0xff,0xc0,0x38,0x59,0x05,0x23,0x15,0x23,
/*003290*/ 0x35,0x33,0x35,0x23,0x35,0x23,0x35,0x33,0x35,0x21,0x03,0x35,0x23,0x15,0x01,0x8f,
/*0032a0*/ 0x65,0xc5,0xc8,0xc8,0x64,0x64,0x01,0x2a,0x64,0xc6,0x63,0x64,0x62,0x67,0x64,0xc5,
/*0032b0*/ 0x64,0xfe,0xd6,0xc6,0xc6,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,
/*0032c0*/ 0x00,0x0b,0x00,0x65,0x40,0x2b,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0b,0x0a,0x0a,0x09,
/*0032d0*/ 0x02,0x03,0x01,0x04,0x0b,0x00,0x08,0x07,0x04,0x03,0x03,0x04,0x06,0x05,0x03,0x02,
/*0032e0*/ 0x06,0x08,0x09,0x08,0x02,0x07,0x06,0x03,0x05,0x04,0x01,0x03,0x00,0x01,0x01,0x05,
/*0032f0*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,
/*003300*/ 0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x31,0x30,
/*003310*/ 0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,
/*003320*/ 0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x11,0x23,0x11,0x23,0x11,
/*003330*/ 0x33,0x15,0x33,0x15,0x33,0x01,0x8f,0x62,0xca,0x62,0x62,0xc8,0x64,0x01,0x01,0x2c,
/*003340*/ 0xfe,0xd4,0x02,0x56,0xc8,0x65,0x00,0x02,0x00,0x01,0x00,0x01,0x00,0x63,0x02,0x57,
/*003350*/ 0x00,0x03,0x00,0x07,0x00,0x54,0x40,0x21,0x01,0x08,0x08,0x40,0x09,0x00,0x07,0x04,
/*003360*/ 0x03,0x03,0x00,0x04,0x06,0x05,0x02,0x03,0x01,0x01,0x00,0x06,0x02,0x07,0x06,0x02,
/*003370*/ 0x05,0x04,0x01,0x03,0x02,0x03,0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,
/*003380*/ 0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,
/*003390*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x08,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*0033a0*/ 0x38,0x11,0x37,0xb9,0x00,0x08,0xff,0xc0,0x38,0x59,0x13,0x23,0x35,0x33,0x11,0x23,
/*0033b0*/ 0x11,0x33,0x63,0x62,0x62,0x62,0x62,0x01,0xf5,0x62,0xfd,0xaa,0x01,0x8e,0x00,0x02,
/*0033c0*/ 0x00,0x01,0xff,0x39,0x00,0xc7,0x02,0x57,0x00,0x03,0x00,0x0b,0x00,0x68,0x40,0x2d,
/*0033d0*/ 0x01,0x0c,0x0c,0x40,0x0d,0x00,0x0a,0x09,0x02,0x03,0x01,0x04,0x0b,0x04,0x03,0x03,
/*0033e0*/ 0x00,0x06,0x05,0x04,0x08,0x07,0x01,0x00,0x06,0x02,0x09,0x05,0x04,0x03,0x08,0x06,
/*0033f0*/ 0x06,0x0b,0x0a,0x02,0x07,0x06,0x00,0x03,0x02,0x03,0x01,0x07,0x46,0x76,0x2f,0x37,
/*003400*/ 0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,
/*003410*/ 0x2f,0x3c,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,0x68,
/*003420*/ 0xb9,0x00,0x07,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*003430*/ 0x00,0x0c,0xff,0xc0,0x38,0x59,0x13,0x23,0x35,0x33,0x11,0x23,0x15,0x23,0x35,0x33,
/*003440*/ 0x11,0x33,0xc7,0x62,0x62,0x65,0x61,0x64,0x62,0x01,0xf5,0x62,0xfd,0x46,0x64,0x62,
/*003450*/ 0x01,0xf4,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x02,0x57,0x00,0x0f,0x00,0x79,
/*003460*/ 0x40,0x39,0x01,0x10,0x10,0x40,0x11,0x00,0x0e,0x0d,0x0a,0x09,0x02,0x05,0x01,0x04,
/*003470*/ 0x0f,0x0c,0x0b,0x03,0x00,0x08,0x07,0x04,0x03,0x03,0x04,0x06,0x05,0x0f,0x0e,0x07,
/*003480*/ 0x00,0x03,0x02,0x06,0x08,0x0d,0x09,0x08,0x03,0x0c,0x06,0x0a,0x0b,0x0a,0x02,0x07,
/*003490*/ 0x06,0x03,0x05,0x04,0x01,0x03,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,
/*0034a0*/ 0x3f,0x17,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,
/*0034b0*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,
/*0034c0*/ 0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,
/*0034d0*/ 0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x23,0x35,0x23,0x15,0x23,0x11,
/*0034e0*/ 0x33,0x11,0x33,0x35,0x33,0x15,0x23,0x15,0x33,0x01,0x8f,0x62,0xca,0x62,0x62,0xcb,
/*0034f0*/ 0x61,0x64,0x64,0x01,0xc8,0xc8,0x02,0x56,0xfe,0xd4,0x64,0x62,0x67,0x00,0x00,0x01,
/*003500*/ 0x00,0x01,0x00,0x01,0x00,0x63,0x02,0x57,0x00,0x03,0x00,0x40,0x40,0x14,0x01,0x04,
/*003510*/ 0x04,0x40,0x05,0x00,0x02,0x01,0x04,0x03,0x00,0x03,0x02,0x03,0x01,0x00,0x01,0x01,
/*003520*/ 0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,
/*003530*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x04,0x49,0x68,0x61,0xb0,0x40,
/*003540*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,0x59,0x37,0x23,0x11,0x33,
/*003550*/ 0x63,0x62,0x62,0x01,0x02,0x56,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0xf3,0x01,0x8f,
/*003560*/ 0x00,0x0d,0x00,0x6b,0x40,0x2f,0x01,0x0e,0x0e,0x40,0x0f,0x00,0x0d,0x0c,0x0c,0x0b,
/*003570*/ 0x02,0x03,0x01,0x04,0x0d,0x00,0x04,0x03,0x04,0x06,0x05,0x08,0x07,0x04,0x0a,0x09,
/*003580*/ 0x07,0x06,0x03,0x03,0x02,0x06,0x0a,0x0b,0x0a,0x02,0x09,0x08,0x05,0x04,0x01,0x05,
/*003590*/ 0x00,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x3c,0x10,
/*0035a0*/ 0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,
/*0035b0*/ 0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x0e,0x49,0x68,
/*0035c0*/ 0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,
/*0035d0*/ 0x23,0x11,0x23,0x11,0x23,0x11,0x23,0x11,0x23,0x11,0x21,0x15,0x33,0x01,0xf3,0x62,
/*0035e0*/ 0x67,0x61,0x67,0x61,0x01,0x8e,0x64,0x01,0x01,0x2c,0xfe,0xd4,0x01,0x2c,0xfe,0xd4,
/*0035f0*/ 0x01,0x8e,0x65,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x09,
/*003600*/ 0x00,0x5c,0x40,0x25,0x01,0x0a,0x0a,0x40,0x0b,0x00,0x09,0x08,0x08,0x07,0x02,0x03,
/*003610*/ 0x01,0x04,0x09,0x00,0x04,0x03,0x04,0x06,0x05,0x03,0x02,0x06,0x06,0x07,0x06,0x02,
/*003620*/ 0x05,0x04,0x01,0x03,0x00,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,
/*003630*/ 0x3c,0x3f,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,
/*003640*/ 0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0a,0x49,0x68,0x61,
/*003650*/ 0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0a,0xff,0xc0,0x38,0x59,0x25,0x23,
/*003660*/ 0x11,0x23,0x11,0x23,0x11,0x21,0x15,0x33,0x01,0x8f,0x62,0xca,0x62,0x01,0x2a,0x64,
/*003670*/ 0x01,0x01,0x2c,0xfe,0xd4,0x01,0x8e,0x65,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x8f,
/*003680*/ 0x01,0x8f,0x00,0x0b,0x00,0x0f,0x00,0x6f,0x40,0x35,0x01,0x10,0x10,0x40,0x11,0x00,
/*003690*/ 0x0f,0x0e,0x08,0x07,0x04,0x05,0x03,0x04,0x06,0x05,0x0d,0x0c,0x0a,0x02,0x01,0x05,
/*0036a0*/ 0x09,0x04,0x0b,0x00,0x0f,0x0c,0x05,0x04,0x01,0x05,0x00,0x06,0x02,0x0e,0x0d,0x0b,
/*0036b0*/ 0x0a,0x07,0x05,0x06,0x06,0x08,0x09,0x08,0x02,0x03,0x02,0x01,0x01,0x05,0x46,0x76,
/*0036c0*/ 0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,
/*0036d0*/ 0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,0x49,
/*0036e0*/ 0x68,0xb9,0x00,0x05,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*0036f0*/ 0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,
/*003700*/ 0x33,0x15,0x33,0x07,0x35,0x23,0x15,0x01,0x8f,0x65,0xc5,0x64,0x64,0xc6,0x64,0x64,
/*003710*/ 0xc6,0x65,0x64,0x65,0xc5,0x64,0x65,0xc5,0xc6,0xc6,0x00,0x02,0x00,0x01,0xff,0x39,
/*003720*/ 0x01,0x8f,0x01,0x8f,0x00,0x09,0x00,0x0d,0x00,0x6e,0x40,0x32,0x01,0x0e,0x0e,0x40,
/*003730*/ 0x0f,0x00,0x0d,0x04,0x03,0x03,0x0c,0x04,0x06,0x05,0x0b,0x0a,0x08,0x02,0x01,0x05,
/*003740*/ 0x07,0x04,0x09,0x00,0x0d,0x0a,0x01,0x03,0x00,0x06,0x02,0x0c,0x09,0x08,0x03,0x0b,
/*003750*/ 0x06,0x06,0x07,0x06,0x02,0x05,0x04,0x00,0x03,0x02,0x01,0x01,0x05,0x46,0x76,0x2f,
/*003760*/ 0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,
/*003770*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,
/*003780*/ 0x49,0x68,0xb9,0x00,0x05,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*003790*/ 0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x15,0x23,0x11,0x21,
/*0037a0*/ 0x15,0x33,0x07,0x35,0x23,0x15,0x01,0x8f,0x65,0xc7,0x62,0x01,0x2a,0x64,0x64,0xc6,
/*0037b0*/ 0x65,0x64,0xc8,0x02,0x56,0x65,0xc5,0xc6,0xc6,0x00,0x00,0x02,0x00,0x01,0xff,0x39,
/*0037c0*/ 0x01,0x8f,0x01,0x8f,0x00,0x09,0x00,0x0d,0x00,0x6e,0x40,0x32,0x01,0x0e,0x0e,0x40,
/*0037d0*/ 0x0f,0x00,0x0d,0x0c,0x08,0x07,0x04,0x05,0x03,0x04,0x06,0x05,0x0b,0x02,0x01,0x03,
/*0037e0*/ 0x0a,0x04,0x09,0x00,0x0d,0x05,0x04,0x03,0x0a,0x06,0x02,0x0c,0x0b,0x07,0x03,0x06,
/*0037f0*/ 0x06,0x08,0x09,0x08,0x02,0x03,0x02,0x01,0x01,0x00,0x00,0x01,0x05,0x46,0x76,0x2f,
/*003800*/ 0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,
/*003810*/ 0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x31,0x30,0x01,
/*003820*/ 0x49,0x68,0xb9,0x00,0x05,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,
/*003830*/ 0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x05,0x23,0x35,0x23,0x35,0x23,0x35,0x33,
/*003840*/ 0x35,0x21,0x03,0x35,0x23,0x15,0x01,0x8f,0x62,0xc8,0x64,0x64,0x01,0x2a,0x64,0xc6,
/*003850*/ 0xc7,0xc8,0x65,0xc5,0x64,0xfe,0xd6,0xc6,0xc6,0x00,0x00,0x01,0x00,0x01,0x00,0x01,
/*003860*/ 0x01,0x8f,0x01,0x8f,0x00,0x0b,0x00,0x65,0x40,0x2b,0x01,0x0c,0x0c,0x40,0x0d,0x00,
/*003870*/ 0x02,0x01,0x05,0x05,0x0a,0x09,0x05,0x0b,0x00,0x08,0x07,0x04,0x03,0x03,0x04,0x06,
/*003880*/ 0x05,0x03,0x02,0x06,0x09,0x01,0x00,0x03,0x08,0x0b,0x0a,0x07,0x03,0x06,0x02,0x05,
/*003890*/ 0x04,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x2f,
/*0038a0*/ 0x17,0x3c,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,
/*0038b0*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,
/*0038c0*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,
/*0038d0*/ 0x23,0x15,0x23,0x11,0x33,0x15,0x33,0x35,0x33,0x01,0x8f,0xc9,0x64,0x61,0x62,0x67,
/*0038e0*/ 0xc5,0x01,0x2d,0x64,0xc8,0x01,0x8e,0x64,0x64,0x00,0x00,0x01,0x00,0x01,0x00,0x01,
/*0038f0*/ 0x01,0x8f,0x01,0x8f,0x00,0x0f,0x00,0x85,0x40,0x3f,0x01,0x10,0x10,0x40,0x11,0x00,
/*003900*/ 0x0a,0x09,0x08,0x05,0x0f,0x0c,0x0b,0x03,0x00,0x05,0x05,0x08,0x07,0x04,0x03,0x03,
/*003910*/ 0x05,0x0d,0x0e,0x0d,0x06,0x03,0x05,0x04,0x02,0x01,0x0f,0x0e,0x06,0x00,0x05,0x01,
/*003920*/ 0x00,0x03,0x04,0x06,0x02,0x07,0x06,0x06,0x08,0x0d,0x09,0x08,0x03,0x0c,0x06,0x0a,
/*003930*/ 0x0b,0x0a,0x02,0x03,0x02,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,
/*003940*/ 0x3f,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,
/*003950*/ 0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,
/*003960*/ 0x3c,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x10,0x49,0x68,0x61,0xb0,
/*003970*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,
/*003980*/ 0x21,0x35,0x33,0x35,0x23,0x35,0x33,0x35,0x21,0x15,0x23,0x15,0x33,0x01,0x8f,0x65,
/*003990*/ 0xfe,0xd7,0xc8,0xc8,0x64,0x01,0x2a,0xc8,0xc8,0x65,0x64,0x62,0x67,0x61,0x64,0x62,
/*0039a0*/ 0x67,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x2b,0x01,0xf3,0x00,0x0b,0x00,0x64,
/*0039b0*/ 0x40,0x2a,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x03,0x02,0x0b,0x08,0x07,0x03,0x00,0x05,
/*0039c0*/ 0x01,0x0a,0x09,0x06,0x02,0x01,0x05,0x05,0x04,0x04,0x03,0x0b,0x0a,0x06,0x00,0x09,
/*0039d0*/ 0x08,0x06,0x07,0x06,0x05,0x04,0x01,0x00,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,
/*0039e0*/ 0x00,0x3f,0x3c,0x2f,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,
/*0039f0*/ 0x17,0x3c,0x10,0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,
/*003a00*/ 0x03,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x0c,
/*003a10*/ 0xff,0xc0,0x38,0x59,0x25,0x23,0x35,0x23,0x11,0x33,0x15,0x33,0x15,0x23,0x15,0x33,
/*003a20*/ 0x01,0x2b,0xc6,0x64,0x62,0xc8,0xc8,0xc8,0x01,0x65,0x01,0x8d,0x65,0x61,0xca,0x00,
/*003a30*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x09,0x00,0x5c,0x40,0x25,
/*003a40*/ 0x01,0x0a,0x0a,0x40,0x0b,0x00,0x03,0x02,0x06,0x02,0x01,0x03,0x05,0x04,0x04,0x03,
/*003a50*/ 0x08,0x07,0x04,0x09,0x00,0x07,0x06,0x06,0x00,0x09,0x08,0x05,0x03,0x04,0x02,0x01,
/*003a60*/ 0x00,0x01,0x01,0x03,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x10,
/*003a70*/ 0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x2e,0x2e,0x31,
/*003a80*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0a,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*003a90*/ 0x38,0x11,0x37,0xb9,0x00,0x0a,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x11,0x33,
/*003aa0*/ 0x11,0x33,0x11,0x33,0x01,0x8f,0xfe,0xd6,0x64,0x62,0xca,0x62,0x01,0x65,0x01,0x29,
/*003ab0*/ 0xfe,0xd4,0x01,0x2c,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x0b,
/*003ac0*/ 0x00,0x65,0x40,0x2c,0x01,0x0c,0x0c,0x40,0x0d,0x00,0x02,0x01,0x05,0x03,0x08,0x04,
/*003ad0*/ 0x03,0x03,0x07,0x04,0x06,0x05,0x0a,0x09,0x04,0x0b,0x00,0x09,0x05,0x04,0x01,0x00,
/*003ae0*/ 0x05,0x08,0x07,0x02,0x0b,0x0a,0x07,0x03,0x06,0x02,0x03,0x02,0x01,0x01,0x05,0x46,
/*003af0*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,
/*003b00*/ 0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,
/*003b10*/ 0x68,0xb9,0x00,0x05,0x00,0x0c,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*003b20*/ 0xb9,0x00,0x0c,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x15,
/*003b30*/ 0x33,0x35,0x33,0x01,0x8f,0x65,0xc5,0x64,0x62,0xca,0x62,0xc9,0xc8,0xc8,0xc6,0xc8,
/*003b40*/ 0xc8,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0xf3,0x01,0x8f,0x00,0x0d,0x00,0x6b,
/*003b50*/ 0x40,0x2f,0x01,0x0e,0x0e,0x40,0x0f,0x00,0x03,0x02,0x06,0x02,0x01,0x03,0x05,0x04,
/*003b60*/ 0x04,0x03,0x08,0x07,0x04,0x0a,0x09,0x0c,0x0b,0x04,0x0d,0x00,0x0b,0x0a,0x07,0x03,
/*003b70*/ 0x06,0x06,0x00,0x0d,0x0c,0x09,0x08,0x05,0x05,0x04,0x02,0x01,0x00,0x01,0x01,0x03,
/*003b80*/ 0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,
/*003b90*/ 0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x00,0x2e,0x2e,
/*003ba0*/ 0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x03,0x00,0x0e,0x49,0x68,0x61,0xb0,0x40,0x52,
/*003bb0*/ 0x58,0x38,0x11,0x37,0xb9,0x00,0x0e,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x23,0x11,
/*003bc0*/ 0x33,0x11,0x33,0x11,0x33,0x11,0x33,0x11,0x33,0x01,0xf3,0xfe,0x72,0x64,0x62,0x67,
/*003bd0*/ 0x61,0x67,0x61,0x01,0x65,0x01,0x29,0xfe,0xd4,0x01,0x2c,0xfe,0xd4,0x01,0x2c,0x00,
/*003be0*/ 0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x13,0x00,0x89,0x40,0x45,
/*003bf0*/ 0x01,0x14,0x14,0x40,0x15,0x00,0x0c,0x0b,0x05,0x01,0x12,0x11,0x06,0x02,0x01,0x05,
/*003c00*/ 0x05,0x04,0x13,0x04,0x03,0x03,0x00,0x10,0x0f,0x08,0x03,0x07,0x04,0x0e,0x0d,0x0a,
/*003c10*/ 0x03,0x09,0x07,0x06,0x07,0x10,0x11,0x10,0x0d,0x0c,0x01,0x05,0x00,0x06,0x0e,0x0b,
/*003c20*/ 0x0a,0x03,0x03,0x02,0x06,0x04,0x13,0x12,0x0f,0x03,0x0e,0x02,0x09,0x08,0x05,0x03,
/*003c30*/ 0x04,0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x17,0x3c,0x3f,0x17,0x3c,
/*003c40*/ 0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x01,0x2f,0x17,0x3c,0xfd,
/*003c50*/ 0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,
/*003c60*/ 0x68,0xb9,0x00,0x09,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*003c70*/ 0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x33,0x15,0x23,0x35,0x23,0x15,
/*003c80*/ 0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x15,0x33,0x35,0x33,0x01,0x8f,0x64,0x64,0x62,
/*003c90*/ 0xcb,0x61,0x64,0x64,0x62,0xcb,0x61,0x01,0x2d,0xcb,0x61,0x64,0x64,0x62,0xca,0x62,
/*003ca0*/ 0x64,0x64,0x00,0x01,0x00,0x01,0xff,0x39,0x01,0x8f,0x01,0x8f,0x00,0x0f,0x00,0x76,
/*003cb0*/ 0x40,0x36,0x01,0x10,0x10,0x40,0x11,0x00,0x09,0x08,0x02,0x01,0x05,0x03,0x0e,0x0d,
/*003cc0*/ 0x06,0x03,0x05,0x04,0x0f,0x00,0x0c,0x08,0x07,0x04,0x03,0x05,0x0b,0x04,0x0a,0x09,
/*003cd0*/ 0x05,0x01,0x00,0x03,0x04,0x06,0x02,0x0d,0x0c,0x06,0x07,0x06,0x0f,0x0e,0x0b,0x03,
/*003ce0*/ 0x0a,0x02,0x03,0x02,0x00,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,
/*003cf0*/ 0x17,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,
/*003d00*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x3c,0x00,0x2e,0x2e,0x31,0x30,0x01,0x49,0x68,
/*003d10*/ 0xb9,0x00,0x09,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*003d20*/ 0x00,0x10,0xff,0xc0,0x38,0x59,0x05,0x23,0x15,0x23,0x35,0x33,0x35,0x23,0x35,0x23,
/*003d30*/ 0x11,0x33,0x11,0x33,0x11,0x33,0x01,0x8f,0x65,0xc5,0xc8,0xc8,0x64,0x62,0xca,0x62,
/*003d40*/ 0x63,0x64,0x62,0x67,0x64,0x01,0x29,0xfe,0xd4,0x01,0x2c,0x00,0x00,0x01,0x00,0x01,
/*003d50*/ 0x00,0x01,0x01,0x8f,0x01,0x8f,0x00,0x0f,0x00,0x7c,0x40,0x38,0x01,0x10,0x10,0x40,
/*003d60*/ 0x11,0x00,0x0f,0x0a,0x09,0x08,0x07,0x02,0x01,0x00,0x04,0x03,0x04,0x0d,0x0e,0x0d,
/*003d70*/ 0x06,0x03,0x05,0x04,0x0c,0x0b,0x0d,0x05,0x04,0x03,0x0c,0x07,0x08,0x0f,0x0e,0x03,
/*003d80*/ 0x03,0x02,0x06,0x00,0x0b,0x0a,0x07,0x03,0x06,0x06,0x08,0x09,0x08,0x02,0x01,0x00,
/*003d90*/ 0x01,0x01,0x01,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x10,0xfd,0x17,
/*003da0*/ 0x3c,0x10,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,
/*003db0*/ 0xfd,0x3c,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,
/*003dc0*/ 0xb9,0x00,0x01,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,
/*003dd0*/ 0x00,0x10,0xff,0xc0,0x38,0x59,0x25,0x21,0x35,0x33,0x35,0x33,0x35,0x23,0x35,0x21,
/*003de0*/ 0x15,0x23,0x15,0x23,0x15,0x33,0x01,0x8f,0xfe,0x72,0x65,0x63,0xc8,0x01,0x8e,0x65,
/*003df0*/ 0x63,0xc8,0x01,0x62,0x64,0x66,0x62,0x62,0x64,0x67,0x00,0x01,0x00,0x01,0xff,0x9d,
/*003e00*/ 0x01,0x2b,0x02,0x57,0x00,0x13,0x00,0x89,0x40,0x42,0x01,0x14,0x14,0x40,0x15,0x00,
/*003e10*/ 0x10,0x0f,0x13,0x08,0x07,0x03,0x00,0x04,0x09,0x12,0x11,0x0a,0x09,0x06,0x02,0x01,
/*003e20*/ 0x07,0x05,0x04,0x0b,0x0c,0x0b,0x04,0x03,0x03,0x04,0x0e,0x0d,0x05,0x04,0x07,0x0b,
/*003e30*/ 0x0a,0x11,0x10,0x01,0x03,0x00,0x06,0x12,0x07,0x06,0x06,0x08,0x0d,0x0c,0x06,0x0f,
/*003e40*/ 0x03,0x02,0x03,0x0e,0x09,0x08,0x13,0x12,0x03,0x01,0x0d,0x46,0x76,0x2f,0x37,0x18,
/*003e50*/ 0x00,0x3f,0x3c,0x2f,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x10,0xfd,0x17,
/*003e60*/ 0x3c,0x2f,0x3c,0xfd,0x3c,0x01,0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x10,
/*003e70*/ 0xfd,0x17,0x3c,0x2e,0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x0d,0x00,0x14,
/*003e80*/ 0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,
/*003e90*/ 0x59,0x01,0x23,0x15,0x23,0x15,0x33,0x15,0x33,0x15,0x23,0x35,0x23,0x35,0x23,0x35,
/*003ea0*/ 0x33,0x35,0x33,0x35,0x33,0x01,0x2b,0x65,0x63,0x64,0x64,0x62,0x64,0x64,0x65,0x63,
/*003eb0*/ 0x62,0x01,0xf5,0xc8,0x67,0xc8,0x61,0x65,0xc8,0x61,0xc8,0x64,0x00,0x01,0x00,0x01,
/*003ec0*/ 0x00,0x01,0x00,0x63,0x02,0x57,0x00,0x03,0x00,0x40,0x40,0x14,0x01,0x04,0x04,0x40,
/*003ed0*/ 0x05,0x00,0x02,0x01,0x04,0x03,0x00,0x03,0x02,0x03,0x01,0x00,0x01,0x01,0x01,0x46,
/*003ee0*/ 0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x00,0x31,
/*003ef0*/ 0x30,0x01,0x49,0x68,0xb9,0x00,0x01,0x00,0x04,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,
/*003f00*/ 0x38,0x11,0x37,0xb9,0x00,0x04,0xff,0xc0,0x38,0x59,0x37,0x23,0x11,0x33,0x63,0x62,
/*003f10*/ 0x62,0x01,0x02,0x56,0x00,0x01,0x00,0x01,0xff,0x9d,0x01,0x2b,0x02,0x57,0x00,0x13,
/*003f20*/ 0x00,0x8a,0x40,0x41,0x01,0x14,0x14,0x40,0x15,0x00,0x10,0x0f,0x04,0x03,0x03,0x04,
/*003f30*/ 0x0e,0x0d,0x06,0x03,0x05,0x0c,0x0b,0x08,0x03,0x07,0x04,0x02,0x01,0x12,0x11,0x0a,
/*003f40*/ 0x03,0x09,0x04,0x13,0x00,0x0b,0x0a,0x07,0x11,0x10,0x13,0x12,0x06,0x09,0x08,0x01,
/*003f50*/ 0x03,0x00,0x07,0x06,0x06,0x04,0x0d,0x0c,0x06,0x0e,0x05,0x04,0x0f,0x0e,0x03,0x03,
/*003f60*/ 0x02,0x01,0x01,0x05,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x3c,0x2f,0x3c,
/*003f70*/ 0x10,0xfd,0x3c,0x10,0xfd,0x3c,0x2f,0x17,0x3c,0xfd,0x3c,0x2f,0x3c,0xfd,0x3c,0x01,
/*003f80*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x17,0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,
/*003f90*/ 0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x05,0x00,0x14,0x49,0x68,0x61,0xb0,0x40,
/*003fa0*/ 0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x14,0xff,0xc0,0x38,0x59,0x25,0x23,0x15,0x23,
/*003fb0*/ 0x15,0x23,0x35,0x33,0x35,0x33,0x35,0x23,0x35,0x23,0x35,0x33,0x15,0x33,0x15,0x33,
/*003fc0*/ 0x01,0x2b,0x65,0x64,0x61,0x65,0x63,0x64,0x64,0x62,0x64,0x64,0xc9,0xc8,0x64,0x62,
/*003fd0*/ 0xc8,0x67,0xc7,0x62,0x65,0xc8,0x00,0x01,0x00,0x01,0x01,0x91,0x01,0x8f,0x02,0x57,
/*003fe0*/ 0x00,0x0f,0x00,0x73,0x40,0x35,0x01,0x10,0x10,0x40,0x11,0x00,0x02,0x01,0x04,0x03,
/*003ff0*/ 0x06,0x05,0x04,0x08,0x07,0x0c,0x04,0x03,0x03,0x0b,0x04,0x0a,0x09,0x0e,0x0d,0x04,
/*004000*/ 0x0f,0x00,0x0d,0x0c,0x09,0x05,0x04,0x01,0x00,0x07,0x08,0x06,0x02,0x07,0x06,0x03,
/*004010*/ 0x03,0x02,0x0f,0x0e,0x0b,0x03,0x0a,0x03,0x01,0x07,0x46,0x76,0x2f,0x37,0x18,0x00,
/*004020*/ 0x3f,0x17,0x3c,0x2f,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,0x2f,
/*004030*/ 0x3c,0xfd,0x17,0x3c,0x2f,0x3c,0xfd,0x3c,0x10,0xfd,0x3c,0x00,0x31,0x30,0x01,0x49,
/*004040*/ 0x68,0xb9,0x00,0x07,0x00,0x10,0x49,0x68,0x61,0xb0,0x40,0x52,0x58,0x38,0x11,0x37,
/*004050*/ 0xb9,0x00,0x10,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,0x23,0x35,0x23,0x15,0x23,0x35,
/*004060*/ 0x33,0x35,0x33,0x15,0x33,0x35,0x33,0x01,0x8f,0x65,0x61,0x67,0x61,0x64,0x62,0x67,
/*004070*/ 0x61,0x01,0xf5,0x64,0x64,0x64,0x62,0x64,0x64,0x64,0x00,0x01,0x00,0x01,0x00,0x01,
/*004080*/ 0x01,0xf3,0x02,0x57,0x00,0x17,0x00,0x95,0x40,0x4a,0x01,0x18,0x18,0x40,0x19,0x00,
/*004090*/ 0x17,0x0e,0x0d,0x0a,0x09,0x04,0x03,0x00,0x06,0x02,0x01,0x03,0x05,0x04,0x07,0x12,
/*0040a0*/ 0x0c,0x0b,0x08,0x07,0x05,0x11,0x04,0x10,0x0f,0x14,0x13,0x04,0x16,0x15,0x0d,0x0c,
/*0040b0*/ 0x01,0x03,0x00,0x06,0x16,0x0b,0x0a,0x03,0x03,0x02,0x06,0x09,0x08,0x05,0x03,0x04,
/*0040c0*/ 0x15,0x14,0x11,0x03,0x10,0x03,0x17,0x16,0x13,0x12,0x0f,0x05,0x0e,0x02,0x07,0x06,
/*0040d0*/ 0x01,0x01,0x09,0x46,0x76,0x2f,0x37,0x18,0x00,0x3f,0x3c,0x3f,0x17,0x3c,0x3f,0x17,
/*0040e0*/ 0x3c,0x2f,0x17,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x01,0x2f,0x3c,0xfd,0x3c,
/*0040f0*/ 0x2f,0x3c,0xfd,0x17,0x3c,0x10,0xfd,0x17,0x3c,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,
/*004100*/ 0x2e,0x00,0x31,0x30,0x01,0x49,0x68,0xb9,0x00,0x09,0x00,0x18,0x49,0x68,0x61,0xb0,
/*004110*/ 0x40,0x52,0x58,0x38,0x11,0x37,0xb9,0x00,0x18,0xff,0xc0,0x38,0x59,0x01,0x23,0x15,
/*004120*/ 0x33,0x15,0x23,0x15,0x23,0x35,0x23,0x35,0x33,0x35,0x23,0x35,0x33,0x35,0x33,0x15,
/*004130*/ 0x33,0x35,0x33,0x15,0x33,0x01,0xf3,0xc8,0xc8,0xc9,0x61,0xc8,0xc8,0xc8,0x64,0x62,
/*004140*/ 0x67,0x61,0x64,0x01,0x2d,0x67,0x61,0x64,0x65,0x61,0x67,0x61,0xc8,0xc8,0xc8,0xc9,
/*004150*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7a,0x00,0x00,0x00,0x7a,0x00,0x00,0x00,0x7a,
/*004160*/ 0x00,0x00,0x00,0x7a,0x00,0x00,0x00,0xf0,0x00,0x00,0x01,0x64,0x00,0x00,0x02,0x6a,
/*004170*/ 0x00,0x00,0x03,0x2a,0x00,0x00,0x04,0x78,0x00,0x00,0x05,0xac,0x00,0x00,0x06,0x04,
/*004180*/ 0x00,0x00,0x06,0x96,0x00,0x00,0x07,0x26,0x00,0x00,0x07,0xe2,0x00,0x00,0x08,0x70,
/*004190*/ 0x00,0x00,0x08,0xe2,0x00,0x00,0x09,0x3a,0x00,0x00,0x09,0x90,0x00,0x00,0x0a,0x22,
/*0041a0*/ 0x00,0x00,0x0b,0x00,0x00,0x00,0x0b,0x76,0x00,0x00,0x0c,0x62,0x00,0x00,0x0d,0x4e,
/*0041b0*/ 0x00,0x00,0x0d,0xe8,0x00,0x00,0x0e,0x9a,0x00,0x00,0x0f,0x6c,0x00,0x00,0x10,0x1c,
/*0041c0*/ 0x00,0x00,0x11,0x00,0x00,0x00,0x11,0xd0,0x00,0x00,0x12,0x48,0x00,0x00,0x12,0xc0,
/*0041d0*/ 0x00,0x00,0x13,0x84,0x00,0x00,0x14,0x00,0x00,0x00,0x14,0xc6,0x00,0x00,0x15,0x8a,
/*0041e0*/ 0x00,0x00,0x16,0x5e,0x00,0x00,0x16,0xfc,0x00,0x00,0x17,0xc0,0x00,0x00,0x18,0x54,
/*0041f0*/ 0x00,0x00,0x18,0xea,0x00,0x00,0x19,0x8a,0x00,0x00,0x1a,0x1a,0x00,0x00,0x1a,0xcc,
/*004200*/ 0x00,0x00,0x1b,0x5c,0x00,0x00,0x1b,0xf4,0x00,0x00,0x1c,0x7c,0x00,0x00,0x1d,0x26,
/*004210*/ 0x00,0x00,0x1d,0x9e,0x00,0x00,0x1e,0x64,0x00,0x00,0x1f,0x10,0x00,0x00,0x1f,0xb6,
/*004220*/ 0x00,0x00,0x20,0x54,0x00,0x00,0x21,0x1c,0x00,0x00,0x21,0xd2,0x00,0x00,0x22,0xba,
/*004230*/ 0x00,0x00,0x23,0x32,0x00,0x00,0x23,0xc4,0x00,0x00,0x24,0x72,0x00,0x00,0x25,0x36,
/*004240*/ 0x00,0x00,0x25,0xf6,0x00,0x00,0x26,0xa0,0x00,0x00,0x27,0x6c,0x00,0x00,0x27,0xe6,
/*004250*/ 0x00,0x00,0x28,0x7c,0x00,0x00,0x28,0xf6,0x00,0x00,0x29,0x88,0x00,0x00,0x29,0xe0,
/*004260*/ 0x00,0x00,0x2a,0x74,0x00,0x00,0x2b,0x14,0x00,0x00,0x2b,0xa8,0x00,0x00,0x2c,0x48,
/*004270*/ 0x00,0x00,0x2d,0x0a,0x00,0x00,0x2d,0x9e,0x00,0x00,0x2e,0x52,0x00,0x00,0x2e,0xe2,
/*004280*/ 0x00,0x00,0x2f,0x5a,0x00,0x00,0x2f,0xee,0x00,0x00,0x30,0x9a,0x00,0x00,0x30,0xf2,
/*004290*/ 0x00,0x00,0x31,0x90,0x00,0x00,0x32,0x14,0x00,0x00,0x32,0xb6,0x00,0x00,0x33,0x56,
/*0042a0*/ 0x00,0x00,0x33,0xf6,0x00,0x00,0x34,0x86,0x00,0x00,0x35,0x3e,0x00,0x00,0x35,0xcc,
/*0042b0*/ 0x00,0x00,0x36,0x50,0x00,0x00,0x36,0xde,0x00,0x00,0x37,0x7c,0x00,0x00,0x38,0x3e,
/*0042c0*/ 0x00,0x00,0x38,0xe8,0x00,0x00,0x39,0x96,0x00,0x00,0x3a,0x58,0x00,0x00,0x3a,0xb0,
/*0042d0*/ 0x00,0x00,0x3b,0x72,0x00,0x00,0x3c,0x16,0x00,0x00,0x3c,0xec,0x00,0x00,0x3c,0xec,
/*0042e0*/ 0x01,0x90,0x00,0x32,0x00,0x00,0x00,0x00,0x01,0x2c,0x00,0x00,0x01,0x2c,0x00,0x00,
/*0042f0*/ 0x00,0xc8,0x00,0x01,0x01,0x90,0x00,0x01,0x02,0x58,0x00,0x01,0x02,0x58,0x00,0x01,
/*004300*/ 0x03,0x20,0x00,0x01,0x02,0x58,0x00,0x01,0x00,0xc8,0x00,0x01,0x01,0x2c,0x00,0x01,
/*004310*/ 0x01,0x2c,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0x2c,0x00,0x01,
/*004320*/ 0x01,0x90,0x00,0x01,0x00,0xc8,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004330*/ 0x01,0xf4,0x00,0x65,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004340*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004350*/ 0x01,0xf4,0x00,0x01,0x00,0xc8,0x00,0x01,0x00,0xc8,0x00,0x01,0x01,0x90,0x00,0x01,
/*004360*/ 0x01,0x90,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,0x02,0x58,0x00,0x01,
/*004370*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004380*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004390*/ 0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*0043a0*/ 0x02,0x58,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*0043b0*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x02,0x58,0x00,0x01,
/*0043c0*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x02,0x58,0x00,0x01,0x01,0xf4,0x00,0x01,
/*0043d0*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0x2c,0x00,0x01,0x01,0x90,0x00,0x01,
/*0043e0*/ 0x01,0x2c,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*0043f0*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004400*/ 0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x00,0xc8,0x00,0x01,
/*004410*/ 0x01,0x2c,0x00,0x01,0x01,0xf4,0x00,0x01,0x00,0xc8,0x00,0x01,0x02,0x58,0x00,0x01,
/*004420*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004430*/ 0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0x90,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004440*/ 0x01,0xf4,0x00,0x01,0x02,0x58,0x00,0x01,0x01,0xf4,0x00,0x01,0x01,0xf4,0x00,0x01,
/*004450*/ 0x01,0xf4,0x00,0x01,0x01,0x90,0x00,0x01,0x00,0xc8,0x00,0x01,0x01,0x90,0x00,0x01,
/*004460*/ 0x01,0xf4,0x00,0x01,0x02,0x58,0x00,0x01,0x01,0x90,0x00,0x00,0x00,0x02,0x00,0x00,
/*004470*/ 0x00,0x00,0x00,0x00,0xff,0x7b,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004480*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x00,0x00,
/*004490*/ 0x00,0x01,0x00,0x02,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x07,0x00,0x08,
/*0044a0*/ 0x00,0x09,0x00,0x0a,0x00,0x0b,0x00,0x0c,0x00,0x0d,0x00,0x0e,0x00,0x0f,0x00,0x10,
/*0044b0*/ 0x00,0x11,0x00,0x12,0x00,0x13,0x00,0x14,0x00,0x15,0x00,0x16,0x00,0x17,0x00,0x18,
/*0044c0*/ 0x00,0x19,0x00,0x1a,0x00,0x1b,0x00,0x1c,0x00,0x1d,0x00,0x1e,0x00,0x1f,0x00,0x20,
/*0044d0*/ 0x00,0x21,0x00,0x22,0x00,0x23,0x00,0x24,0x00,0x25,0x00,0x26,0x00,0x27,0x00,0x28,
/*0044e0*/ 0x00,0x29,0x00,0x2a,0x00,0x2b,0x00,0x2c,0x00,0x2d,0x00,0x2e,0x00,0x2f,0x00,0x30,
/*0044f0*/ 0x00,0x31,0x00,0x32,0x00,0x33,0x00,0x34,0x00,0x35,0x00,0x36,0x00,0x37,0x00,0x38,
/*004500*/ 0x00,0x39,0x00,0x3a,0x00,0x3b,0x00,0x3c,0x00,0x3d,0x00,0x3e,0x00,0x3f,0x00,0x40,
/*004510*/ 0x00,0x41,0x00,0x42,0x00,0x44,0x00,0x45,0x00,0x46,0x00,0x47,0x00,0x48,0x00,0x49,
/*004520*/ 0x00,0x4a,0x00,0x4b,0x00,0x4c,0x00,0x4d,0x00,0x4e,0x00,0x4f,0x00,0x50,0x00,0x51,
/*004530*/ 0x00,0x52,0x00,0x53,0x00,0x54,0x00,0x55,0x00,0x56,0x00,0x57,0x00,0x58,0x00,0x59,
/*004540*/ 0x00,0x5a,0x00,0x5b,0x00,0x5c,0x00,0x5d,0x00,0x5e,0x00,0x5f,0x00,0x60,0x00,0x61,
/*004550*/ 0x00,0x96,0x00,0xac,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x24,
/*004560*/ 0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x1c,0x00,0x03,0x00,0x01,0x00,0x00,0x01,0x24,
/*004570*/ 0x00,0x00,0x01,0x06,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,
/*004580*/ 0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004590*/ 0x00,0x00,0x00,0x01,0x00,0x00,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,
/*0045a0*/ 0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,
/*0045b0*/ 0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,
/*0045c0*/ 0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,
/*0045d0*/ 0x3d,0x3e,0x3f,0x40,0x41,0x42,0x00,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,
/*0045e0*/ 0x4c,0x4d,0x4e,0x4f,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,
/*0045f0*/ 0x5c,0x5d,0x5e,0x5f,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004600*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004610*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x00,0x00,0x00,0x00,0x61,0x00,0x00,0x00,0x00,
/*004620*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004630*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004640*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,
/*004650*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004660*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004670*/ 0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x04,0x01,0x04,0x00,0x00,0x00,0x0c,
/*004680*/ 0x00,0x08,0x00,0x02,0x00,0x04,0x00,0x5f,0x00,0x7e,0x00,0xa0,0x00,0xa5,0x20,0x10,
/*004690*/ 0xff,0xff,0x00,0x00,0x00,0x20,0x00,0x61,0x00,0xa0,0x00,0xa5,0x20,0x10,0xff,0xff,
/*0046a0*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x0c,0x00,0x8a,
/*0046b0*/ 0x00,0xc4,0x00,0xc4,0x00,0xc4,0xff,0xff,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,
/*0046c0*/ 0x00,0x07,0x00,0x08,0x00,0x09,0x00,0x0a,0x00,0x0b,0x00,0x0c,0x00,0x0d,0x00,0x0e,
/*0046d0*/ 0x00,0x0f,0x00,0x10,0x00,0x11,0x00,0x12,0x00,0x13,0x00,0x14,0x00,0x15,0x00,0x16,
/*0046e0*/ 0x00,0x17,0x00,0x18,0x00,0x19,0x00,0x1a,0x00,0x1b,0x00,0x1c,0x00,0x1d,0x00,0x1e,
/*0046f0*/ 0x00,0x1f,0x00,0x20,0x00,0x21,0x00,0x22,0x00,0x23,0x00,0x24,0x00,0x25,0x00,0x26,
/*004700*/ 0x00,0x27,0x00,0x28,0x00,0x29,0x00,0x2a,0x00,0x2b,0x00,0x2c,0x00,0x2d,0x00,0x2e,
/*004710*/ 0x00,0x2f,0x00,0x30,0x00,0x31,0x00,0x32,0x00,0x33,0x00,0x34,0x00,0x35,0x00,0x36,
/*004720*/ 0x00,0x37,0x00,0x38,0x00,0x39,0x00,0x3a,0x00,0x3b,0x00,0x3c,0x00,0x3d,0x00,0x3e,
/*004730*/ 0x00,0x3f,0x00,0x40,0x00,0x41,0x00,0x42,0x00,0x43,0x00,0x44,0x00,0x45,0x00,0x46,
/*004740*/ 0x00,0x47,0x00,0x48,0x00,0x49,0x00,0x4a,0x00,0x4b,0x00,0x4c,0x00,0x4d,0x00,0x4e,
/*004750*/ 0x00,0x4f,0x00,0x50,0x00,0x51,0x00,0x52,0x00,0x53,0x00,0x54,0x00,0x55,0x00,0x56,
/*004760*/ 0x00,0x57,0x00,0x58,0x00,0x59,0x00,0x5a,0x00,0x5b,0x00,0x5c,0x00,0x5d,0x00,0x5e,
/*004770*/ 0x00,0x5f,0x00,0x60,0x00,0x62,0x00,0x61,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x10,
/*004780*/ 0x00,0x00,0x00,0x68,0x09,0x09,0x05,0x00,0x03,0x03,0x02,0x05,0x07,0x07,0x09,0x07,
/*004790*/ 0x02,0x03,0x03,0x05,0x05,0x03,0x05,0x02,0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x06,
/*0047a0*/ 0x06,0x06,0x06,0x02,0x02,0x05,0x05,0x05,0x06,0x07,0x06,0x06,0x06,0x06,0x06,0x06,
/*0047b0*/ 0x06,0x06,0x05,0x06,0x06,0x06,0x07,0x06,0x06,0x06,0x06,0x06,0x06,0x07,0x06,0x06,
/*0047c0*/ 0x07,0x06,0x06,0x06,0x03,0x05,0x03,0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x05,0x06,
/*0047d0*/ 0x06,0x02,0x03,0x06,0x02,0x07,0x06,0x06,0x06,0x06,0x06,0x06,0x05,0x06,0x06,0x07,
/*0047e0*/ 0x06,0x06,0x06,0x05,0x02,0x05,0x06,0x07,0x05,0x00,0x00,0x00,0x0a,0x0a,0x05,0x00,
/*0047f0*/ 0x04,0x04,0x03,0x05,0x08,0x08,0x0a,0x08,0x03,0x04,0x04,0x05,0x05,0x04,0x05,0x03,
/*004800*/ 0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x03,0x03,0x05,0x05,0x05,
/*004810*/ 0x06,0x08,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x05,0x06,0x06,0x06,0x08,0x06,
/*004820*/ 0x06,0x06,0x06,0x06,0x06,0x08,0x06,0x06,0x08,0x06,0x06,0x06,0x04,0x05,0x04,0x05,
/*004830*/ 0x06,0x06,0x06,0x06,0x06,0x06,0x05,0x06,0x06,0x03,0x04,0x06,0x03,0x08,0x06,0x06,
/*004840*/ 0x06,0x06,0x06,0x06,0x05,0x06,0x06,0x08,0x06,0x06,0x06,0x05,0x03,0x05,0x06,0x08,
/*004850*/ 0x05,0x00,0x00,0x00,0x0b,0x0b,0x06,0x00,0x04,0x04,0x03,0x06,0x08,0x08,0x0b,0x08,
/*004860*/ 0x03,0x04,0x04,0x06,0x06,0x04,0x06,0x03,0x06,0x07,0x07,0x07,0x07,0x07,0x07,0x07,
/*004870*/ 0x07,0x07,0x07,0x03,0x03,0x06,0x06,0x06,0x07,0x08,0x07,0x07,0x07,0x07,0x07,0x07,
/*004880*/ 0x07,0x07,0x06,0x07,0x07,0x07,0x08,0x07,0x07,0x07,0x07,0x07,0x07,0x08,0x07,0x07,
/*004890*/ 0x08,0x07,0x07,0x07,0x04,0x06,0x04,0x06,0x07,0x07,0x07,0x07,0x07,0x07,0x06,0x07,
/*0048a0*/ 0x07,0x03,0x04,0x07,0x03,0x08,0x07,0x07,0x07,0x07,0x07,0x07,0x06,0x07,0x07,0x08,
/*0048b0*/ 0x07,0x07,0x07,0x06,0x03,0x06,0x07,0x08,0x06,0x00,0x00,0x00,0x0c,0x0c,0x06,0x00,
/*0048c0*/ 0x05,0x05,0x03,0x06,0x09,0x09,0x0c,0x09,0x03,0x05,0x05,0x06,0x06,0x05,0x06,0x03,
/*0048d0*/ 0x06,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x03,0x03,0x06,0x06,0x06,
/*0048e0*/ 0x08,0x09,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x06,0x08,0x08,0x08,0x09,0x08,
/*0048f0*/ 0x08,0x08,0x08,0x08,0x08,0x09,0x08,0x08,0x09,0x08,0x08,0x08,0x05,0x06,0x05,0x06,
/*004900*/ 0x08,0x08,0x08,0x08,0x08,0x08,0x06,0x08,0x08,0x03,0x05,0x08,0x03,0x09,0x08,0x08,
/*004910*/ 0x08,0x08,0x08,0x08,0x06,0x08,0x08,0x09,0x08,0x08,0x08,0x06,0x03,0x06,0x08,0x09,
/*004920*/ 0x06,0x00,0x00,0x00,0x0d,0x0d,0x07,0x00,0x05,0x05,0x03,0x07,0x0a,0x0a,0x0d,0x0a,
/*004930*/ 0x03,0x05,0x05,0x07,0x07,0x05,0x07,0x03,0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
/*004940*/ 0x08,0x08,0x08,0x03,0x03,0x07,0x07,0x07,0x08,0x0a,0x08,0x08,0x08,0x08,0x08,0x08,
/*004950*/ 0x08,0x08,0x07,0x08,0x08,0x08,0x0a,0x08,0x08,0x08,0x08,0x08,0x08,0x0a,0x08,0x08,
/*004960*/ 0x0a,0x08,0x08,0x08,0x05,0x07,0x05,0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x07,0x08,
/*004970*/ 0x08,0x03,0x05,0x08,0x03,0x0a,0x08,0x08,0x08,0x08,0x08,0x08,0x07,0x08,0x08,0x0a,
/*004980*/ 0x08,0x08,0x08,0x07,0x03,0x07,0x08,0x0a,0x07,0x00,0x00,0x00,0x0e,0x0e,0x07,0x00,
/*004990*/ 0x05,0x05,0x04,0x07,0x0b,0x0b,0x0e,0x0b,0x04,0x05,0x05,0x07,0x07,0x05,0x07,0x04,
/*0049a0*/ 0x07,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x04,0x04,0x07,0x07,0x07,
/*0049b0*/ 0x09,0x0b,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x07,0x09,0x09,0x09,0x0b,0x09,
/*0049c0*/ 0x09,0x09,0x09,0x09,0x09,0x0b,0x09,0x09,0x0b,0x09,0x09,0x09,0x05,0x07,0x05,0x07,
/*0049d0*/ 0x09,0x09,0x09,0x09,0x09,0x09,0x07,0x09,0x09,0x04,0x05,0x09,0x04,0x0b,0x09,0x09,
/*0049e0*/ 0x09,0x09,0x09,0x09,0x07,0x09,0x09,0x0b,0x09,0x09,0x09,0x07,0x04,0x07,0x09,0x0b,
/*0049f0*/ 0x07,0x00,0x00,0x00,0x0f,0x0f,0x08,0x00,0x06,0x06,0x04,0x08,0x0b,0x0b,0x0f,0x0b,
/*004a00*/ 0x04,0x06,0x06,0x08,0x08,0x06,0x08,0x04,0x08,0x09,0x09,0x09,0x09,0x09,0x09,0x09,
/*004a10*/ 0x09,0x09,0x09,0x04,0x04,0x08,0x08,0x08,0x09,0x0b,0x09,0x09,0x09,0x09,0x09,0x09,
/*004a20*/ 0x09,0x09,0x08,0x09,0x09,0x09,0x0b,0x09,0x09,0x09,0x09,0x09,0x09,0x0b,0x09,0x09,
/*004a30*/ 0x0b,0x09,0x09,0x09,0x06,0x08,0x06,0x08,0x09,0x09,0x09,0x09,0x09,0x09,0x08,0x09,
/*004a40*/ 0x09,0x04,0x06,0x09,0x04,0x0b,0x09,0x09,0x09,0x09,0x09,0x09,0x08,0x09,0x09,0x0b,
/*004a50*/ 0x09,0x09,0x09,0x08,0x04,0x08,0x09,0x0b,0x08,0x00,0x00,0x00,0x10,0x10,0x08,0x00,
/*004a60*/ 0x06,0x06,0x04,0x08,0x0c,0x0c,0x10,0x0c,0x04,0x06,0x06,0x08,0x08,0x06,0x08,0x04,
/*004a70*/ 0x08,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x04,0x04,0x08,0x08,0x08,
/*004a80*/ 0x0a,0x0c,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x08,0x0a,0x0a,0x0a,0x0c,0x0a,
/*004a90*/ 0x0a,0x0a,0x0a,0x0a,0x0a,0x0c,0x0a,0x0a,0x0c,0x0a,0x0a,0x0a,0x06,0x08,0x06,0x08,
/*004aa0*/ 0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x08,0x0a,0x0a,0x04,0x06,0x0a,0x04,0x0c,0x0a,0x0a,
/*004ab0*/ 0x0a,0x0a,0x0a,0x0a,0x08,0x0a,0x0a,0x0c,0x0a,0x0a,0x0a,0x08,0x04,0x08,0x0a,0x0c,
/*004ac0*/ 0x08,0x00,0x00,0x00,0x11,0x11,0x09,0x00,0x06,0x06,0x04,0x09,0x0d,0x0d,0x11,0x0d,
/*004ad0*/ 0x04,0x06,0x06,0x09,0x09,0x06,0x09,0x04,0x09,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
/*004ae0*/ 0x0b,0x0b,0x0b,0x04,0x04,0x09,0x09,0x09,0x0b,0x0d,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
/*004af0*/ 0x0b,0x0b,0x09,0x0b,0x0b,0x0b,0x0d,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0d,0x0b,0x0b,
/*004b00*/ 0x0d,0x0b,0x0b,0x0b,0x06,0x09,0x06,0x09,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x09,0x0b,
/*004b10*/ 0x0b,0x04,0x06,0x0b,0x04,0x0d,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x09,0x0b,0x0b,0x0d,
/*004b20*/ 0x0b,0x0b,0x0b,0x09,0x04,0x09,0x0b,0x0d,0x09,0x00,0x00,0x00,0x12,0x12,0x09,0x00,
/*004b30*/ 0x07,0x07,0x05,0x09,0x0e,0x0e,0x12,0x0e,0x05,0x07,0x07,0x09,0x09,0x07,0x09,0x05,
/*004b40*/ 0x09,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x05,0x05,0x09,0x09,0x09,
/*004b50*/ 0x0b,0x0e,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x09,0x0b,0x0b,0x0b,0x0e,0x0b,
/*004b60*/ 0x0b,0x0b,0x0b,0x0b,0x0b,0x0e,0x0b,0x0b,0x0e,0x0b,0x0b,0x0b,0x07,0x09,0x07,0x09,
/*004b70*/ 0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x09,0x0b,0x0b,0x05,0x07,0x0b,0x05,0x0e,0x0b,0x0b,
/*004b80*/ 0x0b,0x0b,0x0b,0x0b,0x09,0x0b,0x0b,0x0e,0x0b,0x0b,0x0b,0x09,0x05,0x09,0x0b,0x0e,
/*004b90*/ 0x09,0x00,0x00,0x00,0x13,0x13,0x0a,0x00,0x07,0x07,0x05,0x0a,0x0e,0x0e,0x13,0x0e,
/*004ba0*/ 0x05,0x07,0x07,0x0a,0x0a,0x07,0x0a,0x05,0x0a,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,
/*004bb0*/ 0x0c,0x0c,0x0c,0x05,0x05,0x0a,0x0a,0x0a,0x0c,0x0e,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,
/*004bc0*/ 0x0c,0x0c,0x0a,0x0c,0x0c,0x0c,0x0e,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0e,0x0c,0x0c,
/*004bd0*/ 0x0e,0x0c,0x0c,0x0c,0x07,0x0a,0x07,0x0a,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0a,0x0c,
/*004be0*/ 0x0c,0x05,0x07,0x0c,0x05,0x0e,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0a,0x0c,0x0c,0x0e,
/*004bf0*/ 0x0c,0x0c,0x0c,0x0a,0x05,0x0a,0x0c,0x0e,0x0a,0x00,0x00,0x00,0x14,0x14,0x0a,0x00,
/*004c00*/ 0x08,0x08,0x05,0x0a,0x0f,0x0f,0x14,0x0f,0x05,0x08,0x08,0x0a,0x0a,0x08,0x0a,0x05,
/*004c10*/ 0x0a,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x05,0x05,0x0a,0x0a,0x0a,
/*004c20*/ 0x0d,0x0f,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0a,0x0d,0x0d,0x0d,0x0f,0x0d,
/*004c30*/ 0x0d,0x0d,0x0d,0x0d,0x0d,0x0f,0x0d,0x0d,0x0f,0x0d,0x0d,0x0d,0x08,0x0a,0x08,0x0a,
/*004c40*/ 0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0a,0x0d,0x0d,0x05,0x08,0x0d,0x05,0x0f,0x0d,0x0d,
/*004c50*/ 0x0d,0x0d,0x0d,0x0d,0x0a,0x0d,0x0d,0x0f,0x0d,0x0d,0x0d,0x0a,0x05,0x0a,0x0d,0x0f,
/*004c60*/ 0x0a,0x00,0x00,0x00,0x15,0x15,0x0b,0x00,0x08,0x08,0x05,0x0b,0x10,0x10,0x15,0x10,
/*004c70*/ 0x05,0x08,0x08,0x0b,0x0b,0x08,0x0b,0x05,0x0b,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,
/*004c80*/ 0x0d,0x0d,0x0d,0x05,0x05,0x0b,0x0b,0x0b,0x0d,0x10,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,
/*004c90*/ 0x0d,0x0d,0x0b,0x0d,0x0d,0x0d,0x10,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x10,0x0d,0x0d,
/*004ca0*/ 0x10,0x0d,0x0d,0x0d,0x08,0x0b,0x08,0x0b,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0b,0x0d,
/*004cb0*/ 0x0d,0x05,0x08,0x0d,0x05,0x10,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0b,0x0d,0x0d,0x10,
/*004cc0*/ 0x0d,0x0d,0x0d,0x0b,0x05,0x0b,0x0d,0x10,0x0b,0x00,0x00,0x00,0x16,0x16,0x0b,0x00,
/*004cd0*/ 0x08,0x08,0x06,0x0b,0x11,0x11,0x16,0x11,0x06,0x08,0x08,0x0b,0x0b,0x08,0x0b,0x06,
/*004ce0*/ 0x0b,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x06,0x06,0x0b,0x0b,0x0b,
/*004cf0*/ 0x0e,0x11,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0b,0x0e,0x0e,0x0e,0x11,0x0e,
/*004d00*/ 0x0e,0x0e,0x0e,0x0e,0x0e,0x11,0x0e,0x0e,0x11,0x0e,0x0e,0x0e,0x08,0x0b,0x08,0x0b,
/*004d10*/ 0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0b,0x0e,0x0e,0x06,0x08,0x0e,0x06,0x11,0x0e,0x0e,
/*004d20*/ 0x0e,0x0e,0x0e,0x0e,0x0b,0x0e,0x0e,0x11,0x0e,0x0e,0x0e,0x0b,0x06,0x0b,0x0e,0x11,
/*004d30*/ 0x0b,0x00,0x00,0x00,0x17,0x17,0x0c,0x00,0x09,0x09,0x06,0x0c,0x11,0x11,0x17,0x11,
/*004d40*/ 0x06,0x09,0x09,0x0c,0x0c,0x09,0x0c,0x06,0x0c,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,
/*004d50*/ 0x0e,0x0e,0x0e,0x06,0x06,0x0c,0x0c,0x0c,0x0e,0x11,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,
/*004d60*/ 0x0e,0x0e,0x0c,0x0e,0x0e,0x0e,0x11,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x11,0x0e,0x0e,
/*004d70*/ 0x11,0x0e,0x0e,0x0e,0x09,0x0c,0x09,0x0c,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0c,0x0e,
/*004d80*/ 0x0e,0x06,0x09,0x0e,0x06,0x11,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0c,0x0e,0x0e,0x11,
/*004d90*/ 0x0e,0x0e,0x0e,0x0c,0x06,0x0c,0x0e,0x11,0x0c,0x00,0x00,0x00,0x18,0x18,0x0c,0x00,
/*004da0*/ 0x09,0x09,0x06,0x0c,0x12,0x12,0x18,0x12,0x06,0x09,0x09,0x0c,0x0c,0x09,0x0c,0x06,
/*004db0*/ 0x0c,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x06,0x06,0x0c,0x0c,0x0c,
/*004dc0*/ 0x0f,0x12,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0c,0x0f,0x0f,0x0f,0x12,0x0f,
/*004dd0*/ 0x0f,0x0f,0x0f,0x0f,0x0f,0x12,0x0f,0x0f,0x12,0x0f,0x0f,0x0f,0x09,0x0c,0x09,0x0c,
/*004de0*/ 0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0c,0x0f,0x0f,0x06,0x09,0x0f,0x06,0x12,0x0f,0x0f,
/*004df0*/ 0x0f,0x0f,0x0f,0x0f,0x0c,0x0f,0x0f,0x12,0x0f,0x0f,0x0f,0x0c,0x06,0x0c,0x0f,0x12,
/*004e00*/ 0x0c,0x00,0x00,0x00,0x00,0x00,0x01,0xaf,0x01,0x90,0x00,0x05,0x00,0x01,0x02,0x30,
/*004e10*/ 0x02,0x08,0x00,0x00,0x00,0x72,0x02,0x30,0x02,0x08,0x00,0x00,0x01,0x6b,0x00,0x28,
/*004e20*/ 0x00,0xcf,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*004e30*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x41,0x6c,
/*004e40*/ 0x74,0x73,0x00,0x40,0x00,0x20,0x20,0x10,0x02,0x58,0xff,0x38,0x00,0x00,0x02,0x58,
/*004e50*/ 0x00,0xc7,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0xe3,0xe5,0x12,0x31,
/*004e60*/ 0x5f,0x0f,0x3c,0xf5,0x00,0x00,0x03,0x20,0x00,0x00,0x00,0x00,0xb6,0x97,0xc2,0x82,
/*004e70*/ 0x00,0x00,0x00,0x00,0xb6,0x97,0xc2,0x82,0x00,0x00,0xff,0x39,0x02,0xbb,0x02,0x58,
/*004e80*/ 0x00,0x00,0x00,0x03,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,
/*004e90*/ 0x02,0x58,0xff,0x38,0x00,0x00,0x03,0x20,0x00,0x00,0x00,0x32,0x02,0xbb,0x00,0x01,
/*004ea0*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x63,
/*004eb0*/ 0x00,0x01,0x00,0x00,0x00,0x63,0x00,0x2c,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x02,
/*004ec0*/ 0x00,0x08,0x00,0x40,0x00,0x0a,0x00,0x00,0x00,0x84,0x00,0xde,0x00,0x01,0x00,0x01
};

static const unsigned bm_mini_ttf_length = (unsigned)sizeof(bm_mini_ttf);

// -----------------------------------------------------------------------------

// The following data tables are coming from Dear Imgui.
// Re-licensed under permission as MIT-0.
//
// @todo: 0x3100, 0x312F, FONT_TW, // Bopomofo
//        0xE000, 0xEB4C, FONT_EM, // Private use (emojis)

static const unsigned table_common[] = {
    0x0020, 0x00FF, // Basic Latin + Latin Supplement
    0xFFFD, 0xFFFD, // Invalid
    0,
};

static const unsigned table_cyrillic[] = {
    0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
    0x2DE0, 0x2DFF, // Cyrillic Extended-A
    0xA640, 0xA69F, // Cyrillic Extended-B
    0,
};

static const unsigned table_vietnamese[] = {
    0x0102, 0x0103,
    0x0110, 0x0111,
    0x0128, 0x0129,
    0x0168, 0x0169,
    0x01A0, 0x01A1,
    0x01AF, 0x01B0,
    0x1EA0, 0x1EF9,
    0,
};

static const unsigned table_thai[] = {
    0x0E00, 0x0E7F, // Thai
    0x2010, 0x205E, // General Punctuation (smaller set than full CJK 2000-206F)
    0,
};

static const unsigned table_korean[] = {
    0x1100, 0x11FF, // Hangul Jamo
    0x3131, 0x3163, // Korean alphabets (no 3130-318F?)
    0xAC00, 0xD7A3, // Korean characters (no AC00-D7AF?)
    0,
};

static const unsigned table_chinese_punctuation[] = {
    0x2000, 0x206F, // General CJK Punctuation (Thai uses 2010-205E)
    0,
};

static const unsigned table_chinese_japanese_common[] = {
    0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
    0x31F0, 0x31FF, // Katakana Phonetic Extensions
    0xFF00, 0xFFEF, // Half-width characters (Romaji)
    0,
};

static const unsigned table_chinese_full[] = {
    0x4e00, 0x9FAF, // CJK Ideograms
    0,
};

static const unsigned table_eastern_europe[] = {
    0x0100, 0x017F, // Latin_Extended-A
    0x0180, 0x024F, // Latin_Extended-B
    0x0250, 0x02AF, // International Phonetic Alphabet (IPA)
    0x02B0, 0x02FF, // Spacing modifier letters (and quotes)
    0x0300, 0x036F, // Fix IPA missing glyphs (?)
    0x16A0, 0x16F0, // Old Norse / Runic
    0x1E00, 0x1EFF, // Latin_Extended_Additional
    0x2000, 0x206F, // General punctuaction
//  0x2C60, 0x2C7F, // Latin Extended-C (Uighur New Script, the Uralic Phonetic Alphabet, Shona, and Claudian Latin)
    0xFB00, 0xFB4F, // Ligatures for the Latin, Armenian, and Hebrew scripts
    0
};

static const unsigned table_western_europe[] = {
    0x0370, 0x03FF, // Greek and Coptic
    0x10A0, 0x10FF, // Modern Georgian, Svan, and Mingrelian
    0x1F00, 0x1FFF, // fix ancient greek glyphs (?)
    0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
    0x2DE0, 0x2DFF, // Cyrillic Extended-A
    0xA640, 0xA69F, // Cyrillic Extended-B
    0xFB00, 0xFB4F, // Ligatures for the Latin, Armenian, and Hebrew scripts
    0
};

static const unsigned table_middle_east[] = {
    0x0590, 0x05FF, // Hebrew, Yiddish, Ladino, and other Jewish diaspora languages.
    0x0600, 0x06FF, // Arabic script and Arabic-Indic digits
    0xFB00, 0xFB4F, // Ligatures for the Latin, Armenian, and Hebrew scripts
    0
};

// Store 2500 regularly used characters for Simplified Chinese. Table below covers 97.97% of all characters used during the month in July, 1987.
// [ref] https://zh.wiktionary.org/wiki/%E9%99%84%E5%BD%95:%E7%8E%B0%E4%BB%A3%E6%B1%89%E8%AF%AD%E5%B8%B8%E7%94%A8%E5%AD%97%E8%A1%A8
static const unsigned short packed_table_chinese[] = { // starts with 0x4E00
    0,1,2,4,1,1,1,1,2,1,3,2,1,2,2,1,1,1,1,1,5,2,1,2,3,3,3,2,2,4,1,1,1,2,1,5,2,3,1,2,1,2,1,1,2,1,1,2,2,1,4,1,1,1,1,5,10,1,2,19,2,1,2,1,2,1,2,1,2,
    1,5,1,6,3,2,1,2,2,1,1,1,4,8,5,1,1,4,1,1,3,1,2,1,5,1,2,1,1,1,10,1,1,5,2,4,6,1,4,2,2,2,12,2,1,1,6,1,1,1,4,1,1,4,6,5,1,4,2,2,4,10,7,1,1,4,2,4,
    2,1,4,3,6,10,12,5,7,2,14,2,9,1,1,6,7,10,4,7,13,1,5,4,8,4,1,1,2,28,5,6,1,1,5,2,5,20,2,2,9,8,11,2,9,17,1,8,6,8,27,4,6,9,20,11,27,6,68,2,2,1,1,
    1,2,1,2,2,7,6,11,3,3,1,1,3,1,2,1,1,1,1,1,3,1,1,8,3,4,1,5,7,2,1,4,4,8,4,2,1,2,1,1,4,5,6,3,6,2,12,3,1,3,9,2,4,3,4,1,5,3,3,1,3,7,1,5,1,1,1,1,2,
    3,4,5,2,3,2,6,1,1,2,1,7,1,7,3,4,5,15,2,2,1,5,3,22,19,2,1,1,1,1,2,5,1,1,1,6,1,1,12,8,2,9,18,22,4,1,1,5,1,16,1,2,7,10,15,1,1,6,2,4,1,2,4,1,6,
    1,1,3,2,4,1,6,4,5,1,2,1,1,2,1,10,3,1,3,2,1,9,3,2,5,7,2,19,4,3,6,1,1,1,1,1,4,3,2,1,1,1,2,5,3,1,1,1,2,2,1,1,2,1,1,2,1,3,1,1,1,3,7,1,4,1,1,2,1,
    1,2,1,2,4,4,3,8,1,1,1,2,1,3,5,1,3,1,3,4,6,2,2,14,4,6,6,11,9,1,15,3,1,28,5,2,5,5,3,1,3,4,5,4,6,14,3,2,3,5,21,2,7,20,10,1,2,19,2,4,28,28,2,3,
    2,1,14,4,1,26,28,42,12,40,3,52,79,5,14,17,3,2,2,11,3,4,6,3,1,8,2,23,4,5,8,10,4,2,7,3,5,1,1,6,3,1,2,2,2,5,28,1,1,7,7,20,5,3,29,3,17,26,1,8,4,
    27,3,6,11,23,5,3,4,6,13,24,16,6,5,10,25,35,7,3,2,3,3,14,3,6,2,6,1,4,2,3,8,2,1,1,3,3,3,4,1,1,13,2,2,4,5,2,1,14,14,1,2,2,1,4,5,2,3,1,14,3,12,
    3,17,2,16,5,1,2,1,8,9,3,19,4,2,2,4,17,25,21,20,28,75,1,10,29,103,4,1,2,1,1,4,2,4,1,2,3,24,2,2,2,1,1,2,1,3,8,1,1,1,2,1,1,3,1,1,1,6,1,5,3,1,1,
    1,3,4,1,1,5,2,1,5,6,13,9,16,1,1,1,1,3,2,3,2,4,5,2,5,2,2,3,7,13,7,2,2,1,1,1,1,2,3,3,2,1,6,4,9,2,1,14,2,14,2,1,18,3,4,14,4,11,41,15,23,15,23,
    176,1,3,4,1,1,1,1,5,3,1,2,3,7,3,1,1,2,1,2,4,4,6,2,4,1,9,7,1,10,5,8,16,29,1,1,2,2,3,1,3,5,2,4,5,4,1,1,2,2,3,3,7,1,6,10,1,17,1,44,4,6,2,1,1,6,
    5,4,2,10,1,6,9,2,8,1,24,1,2,13,7,8,8,2,1,4,1,3,1,3,3,5,2,5,10,9,4,9,12,2,1,6,1,10,1,1,7,7,4,10,8,3,1,13,4,3,1,6,1,3,5,2,1,2,17,16,5,2,16,6,
    1,4,2,1,3,3,6,8,5,11,11,1,3,3,2,4,6,10,9,5,7,4,7,4,7,1,1,4,2,1,3,6,8,7,1,6,11,5,5,3,24,9,4,2,7,13,5,1,8,82,16,61,1,1,1,4,2,2,16,10,3,8,1,1,
    6,4,2,1,3,1,1,1,4,3,8,4,2,2,1,1,1,1,1,6,3,5,1,1,4,6,9,2,1,1,1,2,1,7,2,1,6,1,5,4,4,3,1,8,1,3,3,1,3,2,2,2,2,3,1,6,1,2,1,2,1,3,7,1,8,2,1,2,1,5,
    2,5,3,5,10,1,2,1,1,3,2,5,11,3,9,3,5,1,1,5,9,1,2,1,5,7,9,9,8,1,3,3,3,6,8,2,3,2,1,1,32,6,1,2,15,9,3,7,13,1,3,10,13,2,14,1,13,10,2,1,3,10,4,15,
    2,15,15,10,1,3,9,6,9,32,25,26,47,7,3,2,3,1,6,3,4,3,2,8,5,4,1,9,4,2,2,19,10,6,2,3,8,1,2,2,4,2,1,9,4,4,4,6,4,8,9,2,3,1,1,1,1,3,5,5,1,3,8,4,6,
    2,1,4,12,1,5,3,7,13,2,5,8,1,6,1,2,5,14,6,1,5,2,4,8,15,5,1,23,6,62,2,10,1,1,8,1,2,2,10,4,2,2,9,2,1,1,3,2,3,1,5,3,3,2,1,3,8,1,1,1,11,3,1,1,4,
    3,7,1,14,1,2,3,12,5,2,5,1,6,7,5,7,14,11,1,3,1,8,9,12,2,1,11,8,4,4,2,6,10,9,13,1,1,3,1,5,1,3,2,4,4,1,18,2,3,14,11,4,29,4,2,7,1,3,13,9,2,2,5,
    3,5,20,7,16,8,5,72,34,6,4,22,12,12,28,45,36,9,7,39,9,191,1,1,1,4,11,8,4,9,2,3,22,1,1,1,1,4,17,1,7,7,1,11,31,10,2,4,8,2,3,2,1,4,2,16,4,32,2,
    3,19,13,4,9,1,5,2,14,8,1,1,3,6,19,6,5,1,16,6,2,10,8,5,1,2,3,1,5,5,1,11,6,6,1,3,3,2,6,3,8,1,1,4,10,7,5,7,7,5,8,9,2,1,3,4,1,1,3,1,3,3,2,6,16,
    1,4,6,3,1,10,6,1,3,15,2,9,2,10,25,13,9,16,6,2,2,10,11,4,3,9,1,2,6,6,5,4,30,40,1,10,7,12,14,33,6,3,6,7,3,1,3,1,11,14,4,9,5,12,11,49,18,51,31,
    140,31,2,2,1,5,1,8,1,10,1,4,4,3,24,1,10,1,3,6,6,16,3,4,5,2,1,4,2,57,10,6,22,2,22,3,7,22,6,10,11,36,18,16,33,36,2,5,5,1,1,1,4,10,1,4,13,2,7,
    5,2,9,3,4,1,7,43,3,7,3,9,14,7,9,1,11,1,1,3,7,4,18,13,1,14,1,3,6,10,73,2,2,30,6,1,11,18,19,13,22,3,46,42,37,89,7,3,16,34,2,2,3,9,1,7,1,1,1,2,
    2,4,10,7,3,10,3,9,5,28,9,2,6,13,7,3,1,3,10,2,7,2,11,3,6,21,54,85,2,1,4,2,2,1,39,3,21,2,2,5,1,1,1,4,1,1,3,4,15,1,3,2,4,4,2,3,8,2,20,1,8,7,13,
    4,1,26,6,2,9,34,4,21,52,10,4,4,1,5,12,2,11,1,7,2,30,12,44,2,30,1,1,3,6,16,9,17,39,82,2,2,24,7,1,7,3,16,9,14,44,2,1,2,1,2,3,5,2,4,1,6,7,5,3,
    2,6,1,11,5,11,2,1,18,19,8,1,3,24,29,2,1,3,5,2,2,1,13,6,5,1,46,11,3,5,1,1,5,8,2,10,6,12,6,3,7,11,2,4,16,13,2,5,1,1,2,2,5,2,28,5,2,23,10,8,4,
    4,22,39,95,38,8,14,9,5,1,13,5,4,3,13,12,11,1,9,1,27,37,2,5,4,4,63,211,95,2,2,2,1,3,5,2,1,1,2,2,1,1,1,3,2,4,1,2,1,1,5,2,2,1,1,2,3,1,3,1,1,1,
    3,1,4,2,1,3,6,1,1,3,7,15,5,3,2,5,3,9,11,4,2,22,1,6,3,8,7,1,4,28,4,16,3,3,25,4,4,27,27,1,4,1,2,2,7,1,3,5,2,28,8,2,14,1,8,6,16,25,3,3,3,14,3,
    3,1,1,2,1,4,6,3,8,4,1,1,1,2,3,6,10,6,2,3,18,3,2,5,5,4,3,1,5,2,5,4,23,7,6,12,6,4,17,11,9,5,1,1,10,5,12,1,1,11,26,33,7,3,6,1,17,7,1,5,12,1,11,
    2,4,1,8,14,17,23,1,2,1,7,8,16,11,9,6,5,2,6,4,16,2,8,14,1,11,8,9,1,1,1,9,25,4,11,19,7,2,15,2,12,8,52,7,5,19,2,16,4,36,8,1,16,8,24,26,4,6,2,9,
    5,4,36,3,28,12,25,15,37,27,17,12,59,38,5,32,127,1,2,9,17,14,4,1,2,1,1,8,11,50,4,14,2,19,16,4,17,5,4,5,26,12,45,2,23,45,104,30,12,8,3,10,2,2,
    3,3,1,4,20,7,2,9,6,15,2,20,1,3,16,4,11,15,6,134,2,5,59,1,2,2,2,1,9,17,3,26,137,10,211,59,1,2,4,1,4,1,1,1,2,6,2,3,1,1,2,3,2,3,1,3,4,4,2,3,3,
    1,4,3,1,7,2,2,3,1,2,1,3,3,3,2,2,3,2,1,3,14,6,1,3,2,9,6,15,27,9,34,145,1,1,2,1,1,1,1,2,1,1,1,1,2,2,2,3,1,2,1,1,1,2,3,5,8,3,5,2,4,1,3,2,2,2,12,
    4,1,1,1,10,4,5,1,20,4,16,1,15,9,5,12,2,9,2,5,4,2,26,19,7,1,26,4,30,12,15,42,1,6,8,172,1,1,4,2,1,1,11,2,2,4,2,1,2,1,10,8,1,2,1,4,5,1,2,5,1,8,
    4,1,3,4,2,1,6,2,1,3,4,1,2,1,1,1,1,12,5,7,2,4,3,1,1,1,3,3,6,1,2,2,3,3,3,2,1,2,12,14,11,6,6,4,12,2,8,1,7,10,1,35,7,4,13,15,4,3,23,21,28,52,5,
    26,5,6,1,7,10,2,7,53,3,2,1,1,1,2,163,532,1,10,11,1,3,3,4,8,2,8,6,2,2,23,22,4,2,2,4,2,1,3,1,3,3,5,9,8,2,1,2,8,1,10,2,12,21,20,15,105,2,3,1,1,
    3,2,3,1,1,2,5,1,4,15,11,19,1,1,1,1,5,4,5,1,1,2,5,3,5,12,1,2,5,1,11,1,1,15,9,1,4,5,3,26,8,2,1,3,1,1,15,19,2,12,1,2,5,2,7,2,19,2,20,6,26,7,5,
    2,2,7,34,21,13,70,2,128,1,1,2,1,1,2,1,1,3,2,2,2,15,1,4,1,3,4,42,10,6,1,49,85,8,1,2,1,1,4,4,2,3,6,1,5,7,4,3,211,4,1,2,1,2,5,1,2,4,2,2,6,5,6,
    10,3,4,48,100,6,2,16,296,5,27,387,2,2,3,7,16,8,5,38,15,39,21,9,10,3,7,59,13,27,21,47,5,21,6
};

// 2999 japanese ideograms = 2136 Joyo (for common use) + 863 Jinmeiyo (for personal name) Kanji code points.
// [ref] https://github.com/ocornut/imgui/pull/3627 - Missing 1 Joyo Kanji: U+20B9F (Kun'yomi: Shikaru, On'yomi: Shitsu,shichi).
static const unsigned short packed_table_japanese[] = { // starts with 0x4E00
    0,1,2,4,1,1,1,1,2,1,3,3,2,2,1,5,3,5,7,5,6,1,2,1,7,2,6,3,1,8,1,1,4,1,1,18,2,11,2,6,2,1,2,1,5,1,2,1,3,1,2,1,2,3,3,1,1,2,3,1,1,1,12,7,9,1,4,5,1,
    1,2,1,10,1,1,9,2,2,4,5,6,9,3,1,1,1,1,9,3,18,5,2,2,2,2,1,6,3,7,1,1,1,1,2,2,4,2,1,23,2,10,4,3,5,2,4,10,2,4,13,1,6,1,9,3,1,1,6,6,7,6,3,1,2,11,3,
    2,2,3,2,15,2,2,5,4,3,6,4,1,2,5,2,12,16,6,13,9,13,2,1,1,7,16,4,7,1,19,1,5,1,2,2,7,7,8,2,6,5,4,9,18,7,4,5,9,13,11,8,15,2,1,1,1,2,1,2,2,1,2,2,8,
    2,9,3,3,1,1,4,4,1,1,1,4,9,1,4,3,5,5,2,7,5,3,4,8,2,1,13,2,3,3,1,14,1,1,4,5,1,3,6,1,5,2,1,1,3,3,3,3,1,1,2,7,6,6,7,1,4,7,6,1,1,1,1,1,12,3,3,9,5,
    2,6,1,5,6,1,2,3,18,2,4,14,4,1,3,6,1,1,6,3,5,5,3,2,2,2,2,12,3,1,4,2,3,2,3,11,1,7,4,1,2,1,3,17,1,9,1,24,1,1,4,2,2,4,1,2,7,1,1,1,3,1,2,2,4,15,1,
    1,2,1,1,2,1,5,2,5,20,2,5,9,1,10,8,7,6,1,1,1,1,1,1,6,2,1,2,8,1,1,1,1,5,1,1,3,1,1,1,1,3,1,1,12,4,1,3,1,1,1,1,1,10,3,1,7,5,13,1,2,3,4,6,1,1,30,
    2,9,9,1,15,38,11,3,1,8,24,7,1,9,8,10,2,1,9,31,2,13,6,2,9,4,49,5,2,15,2,1,10,2,1,1,1,2,2,6,15,30,35,3,14,18,8,1,16,10,28,12,19,45,38,1,3,2,3,
    13,2,1,7,3,6,5,3,4,3,1,5,7,8,1,5,3,18,5,3,6,1,21,4,24,9,24,40,3,14,3,21,3,2,1,2,4,2,3,1,15,15,6,5,1,1,3,1,5,6,1,9,7,3,3,2,1,4,3,8,21,5,16,4,
    5,2,10,11,11,3,6,3,2,9,3,6,13,1,2,1,1,1,1,11,12,6,6,1,4,2,6,5,2,1,1,3,3,6,13,3,1,1,5,1,2,3,3,14,2,1,2,2,2,5,1,9,5,1,1,6,12,3,12,3,4,13,2,14,
    2,8,1,17,5,1,16,4,2,2,21,8,9,6,23,20,12,25,19,9,38,8,3,21,40,25,33,13,4,3,1,4,1,2,4,1,2,5,26,2,1,1,2,1,3,6,2,1,1,1,1,1,1,2,3,1,1,1,9,2,3,1,1,
    1,3,6,3,2,1,1,6,6,1,8,2,2,2,1,4,1,2,3,2,7,3,2,4,1,2,1,2,2,1,1,1,1,1,3,1,2,5,4,10,9,4,9,1,1,1,1,1,1,5,3,2,1,6,4,9,6,1,10,2,31,17,8,3,7,5,40,1,
    7,7,1,6,5,2,10,7,8,4,15,39,25,6,28,47,18,10,7,1,3,1,1,2,1,1,1,3,3,3,1,1,1,3,4,2,1,4,1,3,6,10,7,8,6,2,2,1,3,3,2,5,8,7,9,12,2,15,1,1,4,1,2,1,1,
    1,3,2,1,3,3,5,6,2,3,2,10,1,4,2,8,1,1,1,11,6,1,21,4,16,3,1,3,1,4,2,3,6,5,1,3,1,1,3,3,4,6,1,1,10,4,2,7,10,4,7,4,2,9,4,3,1,1,1,4,1,8,3,4,1,3,1,
    6,1,4,2,1,4,7,2,1,8,1,4,5,1,1,2,2,4,6,2,7,1,10,1,1,3,4,11,10,8,21,4,6,1,3,5,2,1,2,28,5,5,2,3,13,1,2,3,1,4,2,1,5,20,3,8,11,1,3,3,3,1,8,10,9,2,
    10,9,2,3,1,1,2,4,1,8,3,6,1,7,8,6,11,1,4,29,8,4,3,1,2,7,13,1,4,1,6,2,6,12,12,2,20,3,2,3,6,4,8,9,2,7,34,5,1,18,6,1,1,4,4,5,7,9,1,2,2,4,3,4,1,7,
    2,2,2,6,2,3,25,5,3,6,1,4,6,7,4,2,1,4,2,13,6,4,4,3,1,5,3,4,4,3,2,1,1,4,1,2,1,1,3,1,11,1,6,3,1,7,3,6,2,8,8,6,9,3,4,11,3,2,10,12,2,5,11,1,6,4,5,
    3,1,8,5,4,6,6,3,5,1,1,3,2,1,2,2,6,17,12,1,10,1,6,12,1,6,6,19,9,6,16,1,13,4,4,15,7,17,6,11,9,15,12,6,7,2,1,2,2,15,9,3,21,4,6,49,18,7,3,2,3,1,
    6,8,2,2,6,2,9,1,3,6,4,4,1,2,16,2,5,2,1,6,2,3,5,3,1,2,5,1,2,1,9,3,1,8,6,4,8,11,3,1,1,1,1,3,1,13,8,4,1,3,2,2,1,4,1,11,1,5,2,1,5,2,5,8,6,1,1,7,
    4,3,8,3,2,7,2,1,5,1,5,2,4,7,6,2,8,5,1,11,4,5,3,6,18,1,2,13,3,3,1,21,1,1,4,1,4,1,1,1,8,1,2,2,7,1,2,4,2,2,9,2,1,1,1,4,3,6,3,12,5,1,1,1,5,6,3,2,
    4,8,2,2,4,2,7,1,8,9,5,2,3,2,1,3,2,13,7,14,6,5,1,1,2,1,4,2,23,2,1,1,6,3,1,4,1,15,3,1,7,3,9,14,1,3,1,4,1,1,5,8,1,3,8,3,8,15,11,4,14,4,4,2,5,5,
    1,7,1,6,14,7,7,8,5,15,4,8,6,5,6,2,1,13,1,20,15,11,9,2,5,6,2,11,2,6,2,5,1,5,8,4,13,19,25,4,1,1,11,1,34,2,5,9,14,6,2,2,6,1,1,14,1,3,14,13,1,6,
    12,21,14,14,6,32,17,8,32,9,28,1,2,4,11,8,3,1,14,2,5,15,1,1,1,1,3,6,4,1,3,4,11,3,1,1,11,30,1,5,1,4,1,5,8,1,1,3,2,4,3,17,35,2,6,12,17,3,1,6,2,
    1,1,12,2,7,3,3,2,1,16,2,8,3,6,5,4,7,3,3,8,1,9,8,5,1,2,1,3,2,8,1,2,9,12,1,1,2,3,8,3,24,12,4,3,7,5,8,3,3,3,3,3,3,1,23,10,3,1,2,2,6,3,1,16,1,16,
    22,3,10,4,11,6,9,7,7,3,6,2,2,2,4,10,2,1,1,2,8,7,1,6,4,1,3,3,3,5,10,12,12,2,3,12,8,15,1,1,16,6,6,1,5,9,11,4,11,4,2,6,12,1,17,5,13,1,4,9,5,1,11,
    2,1,8,1,5,7,28,8,3,5,10,2,17,3,38,22,1,2,18,12,10,4,38,18,1,4,44,19,4,1,8,4,1,12,1,4,31,12,1,14,7,75,7,5,10,6,6,13,3,2,11,11,3,2,5,28,15,6,18,
    18,5,6,4,3,16,1,7,18,7,36,3,5,3,1,7,1,9,1,10,7,2,4,2,6,2,9,7,4,3,32,12,3,7,10,2,23,16,3,1,12,3,31,4,11,1,3,8,9,5,1,30,15,6,12,3,2,2,11,19,9,
    14,2,6,2,3,19,13,17,5,3,3,25,3,14,1,1,1,36,1,3,2,19,3,13,36,9,13,31,6,4,16,34,2,5,4,2,3,3,5,1,1,1,4,3,1,17,3,2,3,5,3,1,3,2,3,5,6,3,12,11,1,3,
    1,2,26,7,12,7,2,14,3,3,7,7,11,25,25,28,16,4,36,1,2,1,6,2,1,9,3,27,17,4,3,4,13,4,1,3,2,2,1,10,4,2,4,6,3,8,2,1,18,1,1,24,2,2,4,33,2,3,63,7,1,6,
    40,7,3,4,4,2,4,15,18,1,16,1,1,11,2,41,14,1,3,18,13,3,2,4,16,2,17,7,15,24,7,18,13,44,2,2,3,6,1,1,7,5,1,7,1,4,3,3,5,10,8,2,3,1,8,1,1,27,4,2,1,
    12,1,2,1,10,6,1,6,7,5,2,3,7,11,5,11,3,6,6,2,3,15,4,9,1,1,2,1,2,11,2,8,12,8,5,4,2,3,1,5,2,2,1,14,1,12,11,4,1,11,17,17,4,3,2,5,5,7,3,1,5,9,9,8,
    2,5,6,6,13,13,2,1,2,6,1,2,2,49,4,9,1,2,10,16,7,8,4,3,2,23,4,58,3,29,1,14,19,19,11,11,2,7,5,1,3,4,6,2,18,5,12,12,17,17,3,3,2,4,1,6,2,3,4,3,1,
    1,1,1,5,1,1,9,1,3,1,3,6,1,8,1,1,2,6,4,14,3,1,4,11,4,1,3,32,1,2,4,13,4,1,2,4,2,1,3,1,11,1,4,2,1,4,4,6,3,5,1,6,5,7,6,3,23,3,5,3,5,3,3,13,3,9,10,
    1,12,10,2,3,18,13,7,160,52,4,2,2,3,2,14,5,4,12,4,6,4,1,20,4,11,6,2,12,27,1,4,1,2,2,7,4,5,2,28,3,7,25,8,3,19,3,6,10,2,2,1,10,2,5,4,1,3,4,1,5,
    3,2,6,9,3,6,2,16,3,3,16,4,5,5,3,2,1,2,16,15,8,2,6,21,2,4,1,22,5,8,1,1,21,11,2,1,11,11,19,13,12,4,2,3,2,3,6,1,8,11,1,4,2,9,5,2,1,11,2,9,1,1,2,
    14,31,9,3,4,21,14,4,8,1,7,2,2,2,5,1,4,20,3,3,4,10,1,11,9,8,2,1,4,5,14,12,14,2,17,9,6,31,4,14,1,20,13,26,5,2,7,3,6,13,2,4,2,19,6,2,2,18,9,3,5,
    12,12,14,4,6,2,3,6,9,5,22,4,5,25,6,4,8,5,2,6,27,2,35,2,16,3,7,8,8,6,6,5,9,17,2,20,6,19,2,13,3,1,1,1,4,17,12,2,14,7,1,4,18,12,38,33,2,10,1,1,
    2,13,14,17,11,50,6,33,20,26,74,16,23,45,50,13,38,33,6,6,7,4,4,2,1,3,2,5,8,7,8,9,3,11,21,9,13,1,3,10,6,7,1,2,2,18,5,5,1,9,9,2,68,9,19,13,2,5,
    1,4,4,7,4,13,3,9,10,21,17,3,26,2,1,5,2,4,5,4,1,7,4,7,3,4,2,1,6,1,1,20,4,1,9,2,2,1,3,3,2,3,2,1,1,1,20,2,3,1,6,2,3,6,2,4,8,1,3,2,10,3,5,3,4,4,
    3,4,16,1,6,1,10,2,4,2,1,1,2,10,11,2,2,3,1,24,31,4,10,10,2,5,12,16,164,15,4,16,7,9,15,19,17,1,2,1,1,5,1,1,1,1,1,3,1,4,3,1,3,1,3,1,2,1,1,3,3,7,
    2,8,1,2,2,2,1,3,4,3,7,8,12,92,2,10,3,1,3,14,5,25,16,42,4,7,7,4,2,21,5,27,26,27,21,25,30,31,2,1,5,13,3,22,5,6,6,11,9,12,1,5,9,7,5,5,22,60,3,5,
    13,1,1,8,1,1,3,3,2,1,9,3,3,18,4,1,2,3,7,6,3,1,2,3,9,1,3,1,3,2,1,3,1,1,1,2,1,11,3,1,6,9,1,3,2,3,1,2,1,5,1,1,4,3,4,1,2,2,4,4,1,7,2,1,2,2,3,5,13,
    18,3,4,14,9,9,4,16,3,7,5,8,2,6,48,28,3,1,1,4,2,14,8,2,9,2,1,15,2,4,3,2,10,16,12,8,7,1,1,3,1,1,1,2,7,4,1,6,4,38,39,16,23,7,15,15,3,2,12,7,21,
    37,27,6,5,4,8,2,10,8,8,6,5,1,2,1,3,24,1,16,17,9,23,10,17,6,1,51,55,44,13,294,9,3,6,2,4,2,2,15,1,1,1,13,21,17,68,14,8,9,4,1,4,9,3,11,7,1,1,1,
    5,6,3,2,1,1,1,2,3,8,1,2,2,4,1,5,5,2,1,4,3,7,13,4,1,4,1,3,1,1,1,5,5,10,1,6,1,5,2,1,5,2,4,1,4,5,7,3,18,2,9,11,32,4,3,3,2,4,7,11,16,9,11,8,13,38,
    32,8,4,2,1,1,2,1,2,4,4,1,1,1,4,1,21,3,11,1,16,1,1,6,1,3,2,4,9,8,57,7,44,1,3,3,13,3,10,1,1,7,5,2,7,21,47,63,3,15,4,7,1,16,1,1,2,8,2,3,42,15,4,
    1,29,7,22,10,3,78,16,12,20,18,4,67,11,5,1,3,15,6,21,31,32,27,18,13,71,35,5,142,4,10,1,2,50,19,33,16,35,37,16,19,27,7,1,133,19,1,4,8,7,20,1,4,
    4,1,10,3,1,6,1,2,51,5,40,15,24,43,22928,11,1,13,154,70,3,1,1,7,4,10,1,2,1,1,2,1,2,1,2,2,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,
    3,2,1,1,1,1,2,1,1,
};

// -----------------------------------------------------------------------------

static const char mv_vs_source[] = "//" FILELINE /*#version 330 core\n\*/"\
\n\
in vec2 vertexPosition;\n\
in vec4 instanceGlyph;\n\
\n\
uniform sampler2D sampler_font;\n\
uniform sampler2D sampler_meta;\n\
\n\
uniform float offset_firstline; // ascent - descent - linegap/2\n\
uniform float scale_factor;     // scaling factor proportional to font size\n\
uniform vec2 string_offset;     // offset of upper-left corner\n\
\n\
uniform vec2 res_meta;   // 96x2 \n\
uniform vec2 res_bitmap; // 512x256\n\
uniform vec2 resolution; // screen resolution\n\
\n\
out vec2 uv;\n\
out float color_index; // for syntax highlighting\n\
\n\
void main() { \
    // (xoff, yoff, xoff2, yoff2), from second row of texture\n\
    vec4 q2 = texture(sampler_meta, vec2((instanceGlyph.z + 0.5)/res_meta.x, 0.75))*vec4(res_bitmap, res_bitmap);\n\
\n\
    vec2 p = vertexPosition*(q2.zw - q2.xy) + q2.xy; // offset and scale it properly relative to baseline\n\
    p *= vec2(1.0, -1.0);                            // flip y, since texture is upside-down\n\
    p.y -= offset_firstline;                         // make sure the upper-left corner of the string is in the upper-left corner of the screen\n\
    p *= scale_factor;                               // scale relative to font size\n\
    p += instanceGlyph.xy + string_offset;           // move glyph into the right position\n\
    p *= 2.0/resolution;                             // to NDC\n\
    p += vec2(-1.0, 1.0);                            // move to upper-left corner instead of center\n\
\n\
    gl_Position = vec4(p, 0.0, 1.0);\n\
\n\
    // (x0, y0, x1-x0, y1-y0), from first row of texture\n\
    vec4 q = texture(sampler_meta, vec2((instanceGlyph.z + 0.5)/res_meta.x, 0.25));\n\
\n\
    // send the correct uv's in the font atlas to the fragment shader\n\
    uv = q.xy + vertexPosition*q.zw;\n\
    color_index = instanceGlyph.w;\n\
}\n";

static const char mv_fs_source[] = "//" FILELINE /*#version 330 core\n\*/"\
\n\
in vec2 uv;\n\
in float color_index;\n\
\n\
uniform sampler2D sampler_font;\n\
uniform sampler1D sampler_colors;\n\
uniform float num_colors;\n\
\n\
out vec4 outColor;\n\
\n\
void main() {\
    vec3 col = texture(sampler_colors, (color_index+0.5)/num_colors).rgb;\n\
    float s = texture(sampler_font, uv).r;\n\
    outColor = vec4(col, s);\n\
}\n";

enum { FONT_MAX_COLORS = 256};
enum { FONT_MAX_STRING_LEN = 40000 }; // more glyphs than any reasonable person would show on the screen at once. you can only fit 20736 10x10 rects in a 1920x1080 window

static unsigned font_palette[FONT_MAX_COLORS] = {
    RGB4(248, 248, 242, 255), // foreground color
    RGB4(249,  38, 114, 255), // operator
    RGB4(174, 129, 255, 255), // numeric
    RGB4(102, 217, 239, 255), // function
    RGB4(249,  38, 114, 255), // keyword
    RGB4(117, 113,  94, 255), // comment
    RGB4(102, 217, 239, 255), // type
    RGB4( 73,  72,  62, 255), // background color
    RGB4( 39,  40,  34, 255), // clear color
};

typedef struct font_t {
    bool initialized;

    //char filename[256];

    // character info
    // filled up by stb_truetype.h
    stbtt_packedchar *cdata;
    unsigned num_glyphs;
    unsigned *cp2iter;
    unsigned *iter2cp;

    // font info and data
    int height;      // bitmap height
    int width;       // bitmap width
    float font_size; // font size in pixels (matches scale[0+1] size below)
    float scale[7];  // user defined font scale (match H1..H6 tags)

    // displacement info
    float ascent;   // max distance above baseline for all glyphs
    float descent;  // max distance below baseline for all glyphs
    float linegap;  // distance betwen ascent of next line and descent of current line
    float linedist; // distance between the baseline of two lines (ascent - descent + linegap)

    // opengl stuff
    GLuint vao;
    GLuint program;

    // font bitmap texture
    // generated using stb_truetype.h
    GLuint texture_fontdata;

    // metadata texture.
    // first row contains information on which parts of the bitmap correspond to a glyph.
    // the second row contain information about the relative displacement of the glyph relative to the cursor position
    GLuint texture_offsets;

    // color texture
    // used to color each glyph individually, e.g. for syntax highlighting
    GLuint texture_colors;

    // vbos
    GLuint vbo_quad;      // vec2: simply just a regular [0,1]x[0,1] quad
    GLuint vbo_instances; // vec4: (char_pos_x, char_pos_y, char_index, color_index)
} font_t;

static font_t fonts[8] = {0};

static
void font_init() {
    do_once {
        font_face_from_mem(FONT_FACE1, bm_mini_ttf,0, 42.5f, 0);
        font_face_from_mem(FONT_FACE2, bm_mini_ttf,0, 42.5f, 0);
        font_face_from_mem(FONT_FACE3, bm_mini_ttf,0, 42.5f, 0);
        font_face_from_mem(FONT_FACE4, bm_mini_ttf,0, 42.5f, 0);
        font_face_from_mem(FONT_FACE5, bm_mini_ttf,0, 42.5f, 0);
        font_face_from_mem(FONT_FACE6, bm_mini_ttf,0, 42.5f, 0);
    }
}

// Remap color within all existing color textures
void font_color(const char *tag, uint32_t color) {
    do_once font_init();

    unsigned index = *tag - FONT_COLOR1[0];
    if( index < FONT_MAX_COLORS ) {
        font_palette[index] = color;

        for( int i = 0; i < 8; ++i ) {
            font_t *f = &fonts[i];
            if( f->initialized ) {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_1D, f->texture_colors);
                glTexSubImage1D(GL_TEXTURE_1D, 0, 0, FONT_MAX_COLORS, GL_BGRA, GL_UNSIGNED_BYTE, font_palette);
            }
        }
    }
}

void font_scales(const char *tag, float h1, float h2, float h3, float h4, float h5, float h6) {
    do_once font_init();

    unsigned index = *tag - FONT_FACE1[0];
    if( index >= 8 ) return;

    font_t *f = &fonts[index];
    if (!f->initialized) return;

    f->scale[0] = h1 / f->font_size;
    f->scale[1] = h1 / f->font_size;
    f->scale[2] = h2 / f->font_size;
    f->scale[3] = h3 / f->font_size;
    f->scale[4] = h4 / f->font_size;
    f->scale[5] = h5 / f->font_size;
    f->scale[6] = h6 / f->font_size;
}

// 1. Compile the shaders.
// 1. Call stb_truetype.h routines to read and parse a .ttf file.
// 1. Create a bitmap that is uploaded to the gpu using opengl.
// 1. Calculate and save a bunch of useful variables and put them in the global font variable.
void font_face_from_mem(const char *tag, const void *ttf_bufferv, unsigned ttf_len, float font_size, unsigned flags) {
    const unsigned char *ttf_buffer = ttf_bufferv;

    flags |= FONT_ASCII; // ensure this minimal range [0020-00FF] is always in

    unsigned index = *tag - FONT_FACE1[0];

    if( index >= 8 ) return;
    if( !ttf_buffer /*|| !ttf_len*/ ) return;
    if( font_size <= 0 || font_size > 72 ) return;

    font_t *f = &fonts[index];
    f->initialized = 1;

    // load .ttf into a bitmap using stb_truetype.h
    int dim = flags & FONT_4096 ? 4096 : flags & FONT_2048 ? 2048 : flags & FONT_1024 ? 1024 : 512;
    f->width = dim;
    f->height = dim;

    // change size [h1(largest) to h3(regular) to h6(smallest)]
    f->font_size = font_size;
    f->scale[0] = 1.0000f; // H1
    f->scale[1] = 1.0000f; // H1
    f->scale[2] = 0.7500f; // H2
    f->scale[3] = 0.6600f; // H3
    f->scale[4] = 0.5000f; // H4
    f->scale[5] = 0.3750f; // H5
    f->scale[6] = 0.2500f; // H6

    const char *vs_filename = 0, *fs_filename = 0;
    const char *vs = vs_filename ? file_read(vs_filename) : mv_vs_source;
    const char *fs = fs_filename ? file_read(fs_filename) : mv_fs_source;
    f->program = shader(vs, fs, "vertexPosition,instanceGlyph", "outColor", NULL);

    // figure out what ranges we're about to bake
    #define MERGE_TABLE(table) do { \
    for( unsigned i = 0 ; table[i]; i += 2 ) { \
        uint64_t begin = table[i+0], end = table[i+1]; \
        for( unsigned j = begin; j <= end; ++j ) { \
            array_push(sorted, j); \
        } \
    } } while(0)
    #define MERGE_PACKED_TABLE(codepoint_begin, table) do { \
    for( int i = 0, begin = codepoint_begin, end = countof(table); i < end; i++) { \
        array_push(sorted, (unsigned)(begin + table[i])); \
        begin += table[i]; \
    } } while(0)

    array(uint64_t) sorted = 0;
    if(flags & FONT_ASCII) { MERGE_TABLE(table_common); }
    if(flags & FONT_EU)    { MERGE_TABLE(table_eastern_europe); }
    if(flags & FONT_RU)    { MERGE_TABLE(table_western_europe); }
    if(flags & FONT_EL)    { MERGE_TABLE(table_western_europe); }
    if(flags & FONT_AR)    { MERGE_TABLE(table_middle_east); }
    if(flags & FONT_HE)    { MERGE_TABLE(table_middle_east); }
    if(flags & FONT_TH)    { MERGE_TABLE(table_thai); }
    if(flags & FONT_VI)    { MERGE_TABLE(table_vietnamese); }
    if(flags & FONT_KR)    { MERGE_TABLE(table_korean); }
    if(flags & FONT_JP)    { MERGE_TABLE(table_chinese_japanese_common); MERGE_PACKED_TABLE(0x4E00, packed_table_japanese); }
    if(flags & FONT_ZH)    { MERGE_TABLE(table_chinese_japanese_common); MERGE_PACKED_TABLE(0x4E00, packed_table_chinese); } // zh-simplified
    if(flags & FONT_ZH)    { MERGE_TABLE(table_chinese_punctuation); } // both zh-simplified and zh-full
//  if(flags & FONT_ZH)    { MERGE_TABLE(table_chinese_full); } // zh-full
    array_sort(sorted, sort_64);
    array_unique(sorted, sort_64); // sort + unique pass

    // pack and create bitmap
    unsigned char *bitmap = (unsigned char*)MALLOC(f->height*f->width);

        int charCount = 0xFFFF;
        f->cdata = (stbtt_packedchar*)CALLOC(1, sizeof(stbtt_packedchar) * charCount);
        f->iter2cp = (unsigned*)CALLOC( 1, sizeof(unsigned) * charCount );
        for( int i = 0; i < charCount; ++i ) f->iter2cp[i] = 0xFFFD; // default invalid glyph
        f->cp2iter = (unsigned*)CALLOC( 1, sizeof(unsigned) * charCount );
        for( int i = 0; i < charCount; ++i ) f->cp2iter[i] = 0xFFFD; // default invalid glyph

        stbtt_pack_context pc;
        if( !stbtt_PackBegin(&pc, bitmap, f->width, f->height, 0, 1, NULL) ) {
            PANIC("Failed to initialize atlas font");
        }
        stbtt_PackSetOversampling(&pc, flags & FONT_OVERSAMPLE_X ? 2 : 1, flags & FONT_OVERSAMPLE_Y ? 2 : 1);  /*useful on small fonts*/
        int count = 0;
        for( int i = 0, num = array_count(sorted); i < num; ++i ) {

            uint64_t begin = sorted[i], end = sorted[i];
            while( i < (num-1) && (sorted[i+1]-sorted[i]) == 1 ) end = sorted[++i];
            //printf("(%d,%d)", (unsigned)begin, (unsigned)end);

            if( stbtt_PackFontRange(&pc, ttf_buffer, 0, f->font_size, begin, end - begin + 1, (stbtt_packedchar*)f->cdata + begin) ) {
                for( int j = begin; j <= end; ++j ) {
                    // unicode->index runtime lookup
                    f->cp2iter[ j ] = count;
                    f->iter2cp[ count++ ] = j;
                }
            } else {
                PRINTF("!Failed to pack atlas font. Likely out of texture mem.");
            }
        }
        stbtt_PackEnd(&pc);
        f->num_glyphs = count;

    array_free(sorted);

    // calculate vertical font metrics
    stbtt_fontinfo info;
    stbtt_InitFont(&info, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));

    int a, d, l;
    float s = stbtt_ScaleForPixelHeight(&info, f->font_size);
    stbtt_GetFontVMetrics(&info, &a, &d, &l);

    f->ascent = a * s;
    f->descent = d * s;
    f->linegap = l * s;
    f->linedist = (a - d + l) * s;

    // save some gpu memory by truncating unused vertical space in atlas texture
    {
        int max_y1 = 0;
        for (int i = 0; i < f->num_glyphs; i++) {
            int cp = f->iter2cp[i];
            if( cp == 0xFFFD ) continue;
            stbtt_packedchar *cd = &f->cdata[ cp ];
            if (cd->y1 > max_y1) {
                max_y1 = cd->y1;
            }
        }
        // cut away the unused part of the bitmap
        f->height = max_y1+1;
    }

    PRINTF("Font atlas size %dx%d (GL_R, %5.2fKiB) (%u glyphs)\n", f->width, f->height, f->width * f->height / 1024.f, f->num_glyphs);

    // vao
    glGenVertexArrays(1, &f->vao);
    glBindVertexArray(f->vao);

    // quad vbo setup, used for glyph vertex positions,
    // just uv coordinates that will be stretched accordingly by the glyphs width and height
    float v[] = {0,0,  1,0,  0,1,  0,1,  1,0,  1,1};

    glGenBuffers(1, &f->vbo_quad);
    glBindBuffer(GL_ARRAY_BUFFER, f->vbo_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*)0);
    glVertexAttribDivisor(0, 0);

    // instance vbo setup: for glyph positions, glyph index and color index
    glGenBuffers(1, &f->vbo_instances);
    glBindBuffer(GL_ARRAY_BUFFER, f->vbo_instances);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*FONT_MAX_STRING_LEN, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,(void*)0);
    glVertexAttribDivisor(1, 1);
    //glEnable(GL_FRAMEBUFFER_SRGB);

    // setup and upload font bitmap texture
    glGenTextures(1, &f->texture_fontdata);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, f->texture_fontdata);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, f->width, f->height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // last chance to inspect the font atlases
    if( flag("--debug-font-atlas") )
    stbi_write_png(va("debug_font_atlas%d.png", index), f->width, f->height, 1, bitmap, 0);

    FREE(bitmap);

    // setup and upload font metadata texture
    // used for lookup in the bitmap texture
    glGenTextures(1, &f->texture_offsets);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, f->texture_offsets);

    float *texture_offsets = (float*)MALLOC(sizeof(float)*8*f->num_glyphs);

    // remap larger 0xFFFF unicodes into smaller NUM_GLYPHS glyphs
    for (int i = 0, count = 0; i < f->num_glyphs; i++) {

        unsigned cp = f->iter2cp[ i ];
        if(cp == 0xFFFD) continue;

        stbtt_packedchar *cd = &f->cdata[ cp ];
//      if(cd->x1==cd->x0) { f->iter2cp[i] = f->cp2iter[cp] = 0xFFFD; continue; }

        int k1 = 0*f->num_glyphs + count;
        int k2 = 1*f->num_glyphs + count; ++count;

        texture_offsets[4*k1+0] = cd->x0/(double)f->width;
        texture_offsets[4*k1+1] = cd->y0/(double)f->height;
        texture_offsets[4*k1+2] = (cd->x1-cd->x0)/(double)f->width;
        texture_offsets[4*k1+3] = (cd->y1-cd->y0)/(double)f->height;

        texture_offsets[4*k2+0] = cd->xoff/(double)f->width;
        texture_offsets[4*k2+1] = cd->yoff/(double)f->height;
        texture_offsets[4*k2+2] = cd->xoff2/(double)f->width;
        texture_offsets[4*k2+3] = cd->yoff2/(double)f->height;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, f->num_glyphs, 2, 0, GL_RGBA, GL_FLOAT, texture_offsets);

    FREE(texture_offsets);

    // setup color texture
    glGenTextures(1, &f->texture_colors);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, f->texture_colors);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, FONT_MAX_COLORS, 0, GL_BGRA, GL_UNSIGNED_BYTE, font_palette);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    // upload constant uniforms
    glUseProgram(f->program);
    glUniform1i(glGetUniformLocation(f->program, "sampler_font"), 0);
    glUniform1i(glGetUniformLocation(f->program, "sampler_meta"), 1);
    glUniform1i(glGetUniformLocation(f->program, "sampler_colors"), 2);

    glUniform2f(glGetUniformLocation(f->program, "res_bitmap"), f->width, f->height);
    glUniform2f(glGetUniformLocation(f->program, "res_meta"), f->num_glyphs, 2);
    glUniform1f(glGetUniformLocation(f->program, "num_colors"), FONT_MAX_COLORS);
    glUniform1f(glGetUniformLocation(f->program, "offset_firstline"), f->linedist-f->linegap);

    (void)flags;
}

void font_face(const char *tag, const char *filename_ttf, float font_size, unsigned flags) {
    do_once font_init();

    const char *buffer = //file_read(filename_ttf);
    //if(!buffer) buffer = 
        vfs_read(filename_ttf);
    font_face_from_mem(tag, buffer,0, font_size, flags);
}

static
void font_draw_cmd(font_t *f, const float *glyph_data, int glyph_idx, float factor, vec2 offset) {
    // Backup GL state
    GLint last_program, last_vertex_array;
    GLint last_texture0, last_texture1, last_texture2;
    GLint last_blend_src, last_blend_dst;
    GLint last_blend_equation_rgb, last_blend_equation_alpha;

    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture0);
    glActiveTexture(GL_TEXTURE1);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture1);
    glActiveTexture(GL_TEXTURE2);
    glGetIntegerv(GL_TEXTURE_BINDING_1D, &last_texture2);

    glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
    glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
    glGetIntegerv(GL_BLEND_EQUATION_RGB,   &last_blend_equation_rgb);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);

    GLboolean last_enable_blend      = glIsEnabled(GL_BLEND);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);

    // Setup render state: alpha-blending enabled, no depth testing and bind textures
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, f->texture_fontdata);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, f->texture_offsets);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, f->texture_colors);

    // update bindings
    glBindVertexArray(f->vao);

    // update uniforms
    glUseProgram(f->program);
    glUniform1f(glGetUniformLocation(f->program, "scale_factor"), factor);
    glUniform2fv(glGetUniformLocation(f->program, "string_offset"), 1, &offset.x);

    GLint dims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, dims);
    glUniform2f(glGetUniformLocation(f->program, "resolution"), dims[2], dims[3]);

    // actual uploading
    glBindBuffer(GL_ARRAY_BUFFER, f->vbo_instances);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4*4*glyph_idx, glyph_data);

    // actual drawing
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, glyph_idx);

    // Restore modified GL state
    glUseProgram(last_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, last_texture0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, last_texture1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, last_texture2);

    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBindVertexArray(last_vertex_array);
    glBlendFunc(last_blend_src, last_blend_dst);

    (last_enable_depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST));
    (last_enable_blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND));
}

// 1. call font_face() if it's the first time it's called.
// 1. parse the string and update the instance vbo, then upload it
// 1. draw the string
static
vec2 font_draw_ex(const char *text, vec2 offset, const char *col, void (*draw_cmd)(font_t *,const float *,int,float,vec2)) {
    do_once font_init();

    // sanity checks
    int len = strlen(text);
    if (len >= FONT_MAX_STRING_LEN) {
        return vec2(0, 0);
    }

    // pre-init
    static __thread float *text_glyph_data, **init = 0;
    if(!init) *(init = &text_glyph_data) = (float*)MALLOC(4 * FONT_MAX_STRING_LEN * sizeof(float));

    // ready
    font_t *f = &fonts[0];
    int S = 3;
    uint32_t color = 0;
    float X = 0, Y = 0, W = 0, L = f->linedist*f->scale[S], LL = L; // LL=largest linedist
    offset.y = -offset.y; // invert y polarity

    // utf8 to utf32
    array(uint32_t) unicode = string32(text);

    // parse string
    float *t = text_glyph_data;
    for( int i = 0, end = array_count(unicode); i < end; ++i ) {
        uint32_t ch = unicode[i];

        if( ch == '\n' ) {
            // change cursor, advance y, record largest x as width, increase height
            if( X > W ) W = X;
            X = 0.0;
            Y -= L;
            continue;
        }
        if( ch >= 1 && ch <= 6 ) {
            // flush previous state
            if(draw_cmd) draw_cmd(f, text_glyph_data, (t - text_glyph_data)/4, f->scale[S], offset);
            t = text_glyph_data;

            // reposition offset to align new baseline
            offset.y += (f->linedist - f->linegap) * ( f->scale[ch] - f->scale[S] );

            // change size
            S = ch;
            L = f->linedist*f->scale[S];
            if(L > LL) LL = L;
            continue;
        }
        if( ch >= 0x10 && ch <= 0x19 ) {
            color = ch - 0x10;
            continue;
        }
        if( ch >= 0x1a && ch <= 0x1f ) {
            if( fonts[ ch - 0x1a ].initialized) {
            // flush previous state
            if(draw_cmd) draw_cmd(f, text_glyph_data, (t - text_glyph_data)/4, f->scale[S], offset);
            t = text_glyph_data;

            // change face
            f = &fonts[ ch - 0x1a ];
            }
            continue;
        }

        // convert to vbo data
        int cp = ch; // f->cp2iter[ch];
        //if(cp == 0xFFFD) continue;
        *t++ = X;
        *t++ = Y;
        *t++ = f->cp2iter[cp];
        *t++ = col ? col[i] : color;

        X += f->cdata[cp].xadvance*f->scale[S];
    }

    if(draw_cmd) draw_cmd(f, text_glyph_data, (t - text_glyph_data)/4, f->scale[S], offset);

    //if(strstr(text, "fps")) printf("(%f,%f) (%f) L:%f LINEDIST:%f\n", X, Y, W, L, f->linedist);
    return abs2(vec2(W*W > X*X ? W : X, Y*Y > LL*LL ? Y : LL));
}

void *font_colorize(const char *text, const char *comma_types, const char *comma_keywords) {
    // reallocate memory
    static __thread int slot = 0;
    static __thread char *buf[16] = {0};
    static __thread array(char*) list[16] = {0};

    slot = (slot+1) % 16;
    buf[slot] = REALLOC(buf[slot], strlen(text)+1);
    memset(buf[slot], 0, strlen(text)+1);

    // ready
    char *col = buf[slot];
    char *str = STRDUP(text);

    // split inputs
    array(char*) TYPES = strsplit(comma_types, ", ");
    array(char*) KEYWORDS = strsplit(comma_keywords, ", ");

    // ignored characters
    char delims[] = " ,(){}[];\t\n";
    int num_delims = strlen(delims);

    char operators[] = "/+-*<>=&|";
    int num_operators = strlen(operators);

    struct token {
        char *start, *stop;
        enum {
            TOKEN_OTHER,TOKEN_OPERATOR,TOKEN_NUMERIC,TOKEN_FUNCTION,
            TOKEN_KEYWORD,TOKEN_COMMENT,TOKEN_VARIABLE,TOKEN_UNSET
        } type;
    } tokens[9999]; // hurr
    int num_tokens = 0; // running counter

    char *ptr = str;
    while (*ptr) {
        // skip delimiters
        int is_delim = 0;
        for (int i = 0; i < num_delims; i++) {
            if (*ptr == delims[i]) {
                is_delim = 1;
                break;
            }
        }

        if (is_delim == 1) {
            ptr++;
            continue;
        }

        // found a token!
        char *start = ptr;

        if (*ptr == '/' && *(ptr+1) == '/') {
            // found a line comment, go to end of line or end of file
            while (*ptr != '\n' && *ptr != '\0') {
                ptr++;
            }

            tokens[num_tokens].start = start;
            tokens[num_tokens].stop = ptr;
            tokens[num_tokens].type = TOKEN_COMMENT;
            num_tokens++;

            ptr++;
            continue;
        }

        if (*ptr == '/' && *(ptr+1) == '*') {
            // found a block comment, go to end of line or end of file
            while (!(*ptr == '*' && *(ptr+1) == '/') && *ptr != '\0') {
                ptr++;
            }
            ptr++;

            tokens[num_tokens].start = start;
            tokens[num_tokens].stop = ptr+1;
            tokens[num_tokens].type = TOKEN_COMMENT;
            num_tokens++;

            ptr++;
            continue;
        }

        // check if it's an operator
        int is_operator = 0;
        for (int i = 0; i < num_operators; i++) {
            if (*ptr == operators[i]) {
                is_operator = 1;
                break;
            }
        }

        if (is_operator == 1) {
            tokens[num_tokens].start = start;
            tokens[num_tokens].stop = ptr+1;
            tokens[num_tokens].type = TOKEN_OPERATOR;
            num_tokens++;
            ptr++;
            continue;
        }

        // it's either a name, type, a keyword, a function, or an names separated by an operator without spaces
        while (*ptr) {
            // check whether it's an operator stuck between two names
            int is_operator2 = 0;
            for (int i = 0; i < num_operators; i++) {
                if (*ptr == operators[i]) {
                    is_operator2 = 1;
                    break;
                }
            }

            if (is_operator2 == 1) {
                tokens[num_tokens].start = start;
                tokens[num_tokens].stop = ptr;
                tokens[num_tokens].type = TOKEN_UNSET;
                num_tokens++;
                break;
            }

            // otherwise go until we find the next delimiter
            int is_delim2 = 0;
            for (int i = 0; i < num_delims; i++) {
                if (*ptr == delims[i]) {
                    is_delim2 = 1;
                    break;
                }
            }

            if (is_delim2 == 1) {
                tokens[num_tokens].start = start;
                tokens[num_tokens].stop = ptr;
                tokens[num_tokens].type = TOKEN_UNSET;
                num_tokens++;
                ptr++;
                break;
            }

            // did not find delimiter, check next char
            ptr++;
        }
    }

    // determine the types of the unset tokens, i.e. either
    // a name, a type, a keyword, or a function
    int num_keywords = array_count(KEYWORDS);
    int num_types = array_count(TYPES);

    for (int i = 0; i < num_tokens; i++) {
        // TOKEN_OPERATOR and TOKEN_COMMENT should already be set, so skip those
        if (tokens[i].type != TOKEN_UNSET) {
            continue;
        }

        char end_char = *tokens[i].stop;

        // temporarily null terminate at end of token, restored after parsing
        *tokens[i].stop = '\0';

        // parse

        // if it's a keyword
        int is_keyword = 0;
        for (int j = 0; j < num_keywords; j++) {
            if (strcmp(tokens[i].start, KEYWORDS[j]) == 0) {
                is_keyword = 1;
                break;
            }
        }
        if (is_keyword == 1) {
            tokens[i].type = TOKEN_KEYWORD;
            *tokens[i].stop = end_char;
            continue;
        }

        // Check if it's a function
        float f;
        if (end_char == '(') {
            tokens[i].type = TOKEN_FUNCTION;
            *tokens[i].stop = end_char;
            continue;
        }

        // or if it's a numeric value. catches both integers and floats
        if (sscanf(tokens[i].start, "%f", &f) == 1) {
            tokens[i].type = TOKEN_NUMERIC;
            *tokens[i].stop = end_char;
            continue;
        }

        // if it's a variable type
        int is_type = 0;
        for (int j = 0; j < num_types; j++) {
            if (strcmp(tokens[i].start, TYPES[j]) == 0) {
                is_type = 1;
                break;
            }
        }
        if (is_type == 1) {
            tokens[i].type = TOKEN_VARIABLE;
            *tokens[i].stop = end_char;
            continue;
        }

        // otherwise it's a regular variable name
        tokens[i].type = TOKEN_OTHER;
        *tokens[i].stop = end_char;
    }

    // print all tokens and their types
    for (int i = 0; i < num_tokens; i++) {

        for (char *p = tokens[i].start; p != tokens[i].stop; p++) {
            col[(p - str)] = tokens[i].type;
        }
    }

    FREE(str);
    return col;
}

static vec2 gotoxy = {0};

// Return cursor
vec2 font_xy() {
    return gotoxy;
}

// Relocate cursor
void font_goto(float x, float y) {
    gotoxy = vec2(x, y);
}

// Print and linefeed. Text may include markup code
vec2 font_print(const char *text) {
    // @fixme: remove this hack
    if( text[0] == FONT_LEFT[0] ) {
        int l = text[1] == FONT_LEFT[1];
        int c = text[1] == FONT_CENTER[1];
        int r = text[1] == FONT_RIGHT[1];
        if( l || c || r ) {
            vec2 rect = font_rect(text + 2);
            gotoxy.x = l ? 0 : r ? (window_width() - rect.x) : window_width()/2 - rect.x/2;
            return font_print(text + 2);
        }
        int t = text[1] == FONT_TOP[1];
        int b = text[1] == FONT_BOTTOM[1];
        int m = text[1] == FONT_MIDDLE[1];
        int B = text[1] == FONT_BASELINE[1];
        if( t || b || m || B ) {
            vec2 rect = font_rect(text + 2);
            gotoxy.y = t ? 0 : b ? (window_height() - rect.y) : m ? window_height()/2-rect.y/2 : window_height()/2-rect.y/1;
            return font_print(text + 2);
        }
    }

    vec2 dims = font_draw_ex(text, gotoxy, NULL, font_draw_cmd);
    gotoxy.y += strchr(text, '\n') ? dims.y : 0;
    gotoxy.x =  strchr(text, '\n') ? 0 : gotoxy.x + dims.x;
    return dims;
}

// Print a code snippet with syntax highlighting
vec2 font_highlight(const char *text, const void *colors) {
    vec2 dims = font_draw_ex(text, gotoxy, (const char *)colors, font_draw_cmd);
    gotoxy.y += strchr(text, '\n') ? dims.y : 0;
    gotoxy.x =  strchr(text, '\n') ? 0 : gotoxy.x + dims.x;
    return dims;
}

// Calculate the size of a string, in the pixel size specified. Count stray newlines too.
vec2 font_rect(const char *str) {
    return font_draw_ex(str, gotoxy, NULL, NULL);
}
#line 0

#line 1 "fwk_input.c"
// input framework
// - rlyeh, public domain
//
// multi-touch(emscripten) port based on code by @procedural (MIT-0 licensed)

// gotta love linux
#ifdef __linux
#undef KEY_ESC
#undef KEY_TICK
#undef KEY_1
#undef KEY_2
#undef KEY_3
#undef KEY_4
#undef KEY_5
#undef KEY_6
#undef KEY_7
#undef KEY_8
#undef KEY_9
#undef KEY_0
#undef KEY_BS
#undef KEY_TAB
#undef KEY_Q
#undef KEY_W
#undef KEY_E
#undef KEY_R
#undef KEY_T
#undef KEY_Y
#undef KEY_U
#undef KEY_I
#undef KEY_O
#undef KEY_P
#undef KEY_CAPS
#undef KEY_A
#undef KEY_S
#undef KEY_D
#undef KEY_F
#undef KEY_G
#undef KEY_H
#undef KEY_J
#undef KEY_K
#undef KEY_L
#undef KEY_ENTER
#undef KEY_LSHIFT
#undef KEY_Z
#undef KEY_X
#undef KEY_C
#undef KEY_V
#undef KEY_B
#undef KEY_N
#undef KEY_M
#undef KEY_RSHIFT
#undef KEY_UP
#undef KEY_LCTRL
#undef KEY_LALT
#undef KEY_SPACE
#undef KEY_RALT
#undef KEY_RCTRL
#undef KEY_LEFT
#undef KEY_DOWN
#undef KEY_RIGHT
#undef KEY_INS
#undef KEY_HOME
#undef KEY_PGUP
#undef KEY_DEL
#undef KEY_END
#undef KEY_PGDN
#undef KEY_LMETA
#undef KEY_RMETA
#undef KEY_MENU
#undef KEY_PRINT
#undef KEY_PAUSE
#undef KEY_SCROLL
#undef KEY_NUMLOCK
#undef KEY_MINUS
#undef KEY_EQUAL
#undef KEY_LSQUARE
#undef KEY_RSQUARE
#undef KEY_SEMICOLON
#undef KEY_QUOTE
#undef KEY_HASH
#undef KEY_BAR
#undef KEY_COMMA
#undef KEY_DOT
#undef KEY_SLASH
#undef KEY_F1
#undef KEY_F2
#undef KEY_F3
#undef KEY_F4
#undef KEY_F5
#undef KEY_F6
#undef KEY_F7
#undef KEY_F8
#undef KEY_F9
#undef KEY_F10
#undef KEY_F11
#undef KEY_F12
#undef KEY_PAD1
#undef KEY_PAD2
#undef KEY_PAD3
#undef KEY_PAD4
#undef KEY_PAD5
#undef KEY_PAD6
#undef KEY_PAD7
#undef KEY_PAD8
#undef KEY_PAD9
#undef KEY_PAD0
#undef KEY_PADADD
#undef KEY_PADSUB
#undef KEY_PADMUL
#undef KEY_PADDIV
#undef KEY_PADDOT
#undef KEY_PADENTER
#undef MOUSE_L
#undef MOUSE_M
#undef MOUSE_R
#undef GAMEPAD_CONNECTED
#undef GAMEPAD_A
#undef GAMEPAD_B
#undef GAMEPAD_X
#undef GAMEPAD_Y
#undef GAMEPAD_UP
#undef GAMEPAD_DOWN
#undef GAMEPAD_LEFT
#undef GAMEPAD_RIGHT
#undef GAMEPAD_MENU
#undef GAMEPAD_START
#undef GAMEPAD_LB
#undef GAMEPAD_RB
#undef GAMEPAD_LTHUMB
#undef GAMEPAD_RTHUMB
#undef WINDOW_BLUR
#undef WINDOW_FOCUS
#undef WINDOW_CLOSE
#undef WINDOW_MINIMIZE
#undef WINDOW_MAXIMIZE
#undef WINDOW_FULLSCREEN
#undef WINDOW_WINDOWED
#undef GAMEPAD_LPAD
#undef GAMEPAD_LPAD
#undef GAMEPAD_LPADY
#undef GAMEPAD_RPAD
#undef GAMEPAD_RPAD
#undef GAMEPAD_RPADY
#undef GAMEPAD_LT
#undef GAMEPAD_RT
#undef GAMEPAD_BATTERY
#undef MOUSE
#undef MOUSE
#undef MOUSE_Y
#undef MOUSE_W
#undef TOUCH_X1
#undef TOUCH_Y1
#undef TOUCH_X2
#undef TOUCH_Y2
#undef WINDOW_RESIZE
#undef WINDOW_RESIZE
#undef WINDOW_RESIZEY
#undef WINDOW_ORIENTATION
#undef WINDOW_BATTERY
#undef GAMEPAD_GUID
#undef GAMEPAD_NAME
#endif

static int controller_id = 0;
static int controller_cycle[4] = {0};

static struct controller_t {
    const char* strings[2];
    float floats[7+3+4+4];
    char  bits[104+3+15+7];
} controller[4] = {0}, frame[4][60] = {{0},{0},{0},{0}};

static struct controller_t *input_logger(int position, int advance) {
    int *cycle = &controller_cycle[controller_id];
    position += (*cycle += advance);
    position = position >= 0 ? position % 60 : 60-1 + ((position+1) % 60);
    return &frame[controller_id][position];
}

void input_mappings(const char *filename) {
#if !is(ems) // emscripten: no glfwUpdateGamepadMappings() available
    char* mappings = vfs_read(filename);
    if( mappings ) { glfwUpdateGamepadMappings(mappings); /*REALLOC(mappings, 0);*/ }
#endif
}

void input_init() {
    do_once {
        input_mappings("gamecontrollerdb.txt");
    }
    #if 0 // deprecated
    void input_update();
    window_hook(input_update, NULL);
    #endif
}

static int any_key = 0;
int input_anykey() {
    return any_key;
}

void input_update() {
    struct controller_t *c = &controller[0]; // @fixme

    char *bits = &c->bits[0];
    float *floats = c->floats; floats -= GAMEPAD_LPADX;
    const char **strings = c->strings; strings -= GAMEPAD_GUID;
    float mouse_wheel_old = floats[MOUSE_W];

    struct controller_t clear = {0};
    *c = clear;
    for( int i = 0; i < countof(c->strings); ++i ) c->strings[i] = "";

    struct GLFWwindow *win = window_handle();
    // glfwSetInputMode(win, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    double mx, my;
    glfwGetCursorPos(win, &mx, &my);
    floats[MOUSE_X] = mx;
    floats[MOUSE_Y] = my;
    struct nk_glfw* glfw = glfwGetWindowUserPointer(win); // from nuklear, because it is overriding glfwSetScrollCallback()
    floats[MOUSE_W] = !glfw ? 0 : mouse_wheel_old + (float)glfw->scroll_bak.x + (float)glfw->scroll_bak.y;
    glfw->scroll_bak.x = glfw->scroll_bak.y = 0;

    // Dear Win32 users,
    // - Touchpad cursor freezing when any key is being pressed?
    // If using Alps/Elantech/Dell/Toshiba touchpad driver or similar, ensure to disable TouchGuard, TouchCheck, PalmTracking, etc.
    // - Touchpad button not clicking when any key is being pressed?
    // Change Touchpad settings on Windows10 from HighSentivity (default) to MostSensitivity.
    // - Apparently, a sane solution is just to never bind FIRE/JUMP actions to LMB/RMB buttons, and bind actions to keys instead.
    bits[MOUSE_L] = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    bits[MOUSE_M] = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    bits[MOUSE_R] = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

    #define k2(VK,GLFW) [KEY_##VK] = GLFW_KEY_##GLFW
    #define k(VK) k2(VK,VK)
    int table[] = {
        k2(ESC,ESCAPE),
        k2(TICK,GRAVE_ACCENT), k(1),k(2),k(3),k(4),k(5),k(6),k(7),k(8),k(9),k(0), k2(BS,BACKSPACE),
        k(TAB),                k(Q),k(W),k(E),k(R),k(T),k(Y),k(U),k(I),k(O),k(P),
        k2(CAPS,CAPS_LOCK),      k(A),k(S),k(D),k(F),k(G),k(H),k(J),k(K),k(L), k(ENTER),
        k2(LSHIFT,LEFT_SHIFT),       k(Z),k(X),k(C),k(V),k(B),k(N),k(M),  k2(RSHIFT,RIGHT_SHIFT),                      k(UP),
        k2(LCTRL,LEFT_CONTROL),k2(LALT,LEFT_ALT), k(SPACE), k2(RALT,RIGHT_ALT),k2(RCTRL,RIGHT_CONTROL), k(LEFT),k(DOWN),k(RIGHT),

        k(F1),k(F2),k(F3),k(F4),k(F5),k(F6),k(F7),k(F8),k(F9),k(F10),k(F11),k(F12), k2(PRINT,PRINT_SCREEN),k(PAUSE),
        k2(INS,INSERT),k(HOME),k2(PGUP,PAGE_UP), k2(DEL,DELETE),k(END), k2(PGDN,PAGE_DOWN),
    };
    #undef k
    #undef k2
    any_key = 0;
    for(int i = 0; i < countof(table); ++i) {
#if is(ems)
        if( table[i] ) any_key |= (bits[i] = glfwGetKey(win, table[i] ) == GLFW_PRESS);
#else
        any_key |= (bits[i] = glfwGetKeys(win)[ table[i] ]);
#endif
    }

#if is(ems)
    {
        int jid = 0; // @fixme
        EmscriptenGamepadEvent state = {0};

        if( emscripten_sample_gamepad_data() == EMSCRIPTEN_RESULT_SUCCESS ) {
            if( emscripten_get_gamepad_status(jid, &state) == EMSCRIPTEN_RESULT_SUCCESS ) {
                // hardcoded for Xbox controller
                if( state.numAxes >= 4 && state.numButtons >= 16 ) {

                    bits[GAMEPAD_CONNECTED] = 1; // !!state.connected
                    strings[GAMEPAD_GUID] = va("%s", state.id);
                    strings[GAMEPAD_NAME] = va("emscripten %s", state.mapping);
                    floats[GAMEPAD_BATTERY] = 100;

                    // e.digitalButton[i], e.analogButton[i]

                    bits[GAMEPAD_A] = state.analogButton[0]; // cross
                    bits[GAMEPAD_B] = state.analogButton[1]; // circle
                    bits[GAMEPAD_X] = state.analogButton[2]; // square
                    bits[GAMEPAD_Y] = state.analogButton[3]; // triangle

                    bits[GAMEPAD_UP] = state.analogButton[12];
                    bits[GAMEPAD_DOWN] = state.analogButton[13];
                    bits[GAMEPAD_LEFT] = state.analogButton[14];
                    bits[GAMEPAD_RIGHT] = state.analogButton[15];

                    bits[GAMEPAD_LB] = state.analogButton[4];
                    bits[GAMEPAD_RB] = state.analogButton[5];
                    bits[GAMEPAD_MENU] = state.analogButton[8];
                    bits[GAMEPAD_START] = state.analogButton[9];

                    bits[GAMEPAD_LTHUMB] = state.analogButton[10];
                    bits[GAMEPAD_RTHUMB] = state.analogButton[11];

                    floats[GAMEPAD_LT] = state.analogButton[6];
                    floats[GAMEPAD_RT] = state.analogButton[7];

                    floats[GAMEPAD_LPADX] = state.axis[0];
                    floats[GAMEPAD_LPADY] = -state.axis[1];

                    floats[GAMEPAD_RPADX] = state.axis[2];
                    floats[GAMEPAD_RPADY] = -state.axis[3];
                }
            }
        }

        if( 0 && ui_panel("emspad", 0)) {
            for(int i = 0; i <= 5; ++i )
            ui_label(va("axis #%d: %5.2f", i, (float)state.axis[i]));

            for(int i = 0; i <= 15; ++i )
            ui_label(va("button #%d: %d %5.2f", i, state.digitalButton[i], (float)state.analogButton[i]));

            ui_panel_end();
        }
    }
#else
    int jid = GLFW_JOYSTICK_1 + 0; // @fixme
    if( glfwGetGamepadName(jid) ) { // glfwJoystickPresent(jid) && glfwJoystickIsGamepad(jid) ) {
        bits[GAMEPAD_CONNECTED] = 1;
        strings[GAMEPAD_GUID] = glfwGetJoystickGUID(jid);
        strings[GAMEPAD_NAME] = glfwGetGamepadName(jid);
        floats[GAMEPAD_BATTERY] = 100; //glfwJoystickCurrentPowerLevel(jid);

        GLFWgamepadstate state;
        if (glfwGetGamepadState(jid, &state)) {
            bits[GAMEPAD_A] = state.buttons[GLFW_GAMEPAD_BUTTON_A]; // cross
            bits[GAMEPAD_B] = state.buttons[GLFW_GAMEPAD_BUTTON_B]; // circle
            bits[GAMEPAD_X] = state.buttons[GLFW_GAMEPAD_BUTTON_X]; // square
            bits[GAMEPAD_Y] = state.buttons[GLFW_GAMEPAD_BUTTON_Y]; // triangle

            bits[GAMEPAD_UP] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP];
            bits[GAMEPAD_DOWN] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN];
            bits[GAMEPAD_LEFT] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT];
            bits[GAMEPAD_RIGHT] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT];

            bits[GAMEPAD_LB] = state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER];
            bits[GAMEPAD_RB] = state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER];
            bits[GAMEPAD_MENU] = state.buttons[GLFW_GAMEPAD_BUTTON_BACK];
            bits[GAMEPAD_START] = state.buttons[GLFW_GAMEPAD_BUTTON_START]; // _GUIDE

            bits[GAMEPAD_LTHUMB] = state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB];
            bits[GAMEPAD_RTHUMB] = state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB];

            floats[GAMEPAD_LT] = input_filter_positive(state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]); // [-1..+1] -> [0..1]
            floats[GAMEPAD_RT] = input_filter_positive(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]); // [-1..+1] -> [0..1]

            floats[GAMEPAD_LPADX] = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
            floats[GAMEPAD_LPADY] = -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

            floats[GAMEPAD_RPADX] = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
            floats[GAMEPAD_RPADY] = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        }
    }
#endif

    *input_logger(0,+1) = controller[0];
}

int input_use(int id) {
    return controller_id >= 0 && controller_id <= 3 ? controller_id = id, 1 : 0;
}

float input_frame( int vk, int frame ) {
    if( controller_id > 0 ) return 0; // @fixme
    struct controller_t *c = input_logger(frame, +0);
    if(vk < GAMEPAD_LPADX) return c->bits[vk]; // if in bits...
    if(vk < GAMEPAD_GUID) return c->floats[vk - GAMEPAD_LPADX]; // if in floats...
    return 0.f; // NAN?
}
vec2 input_frame2( int vk, int frame ) {
    return vec2( input_frame(vk, frame), input_frame(vk+1, frame) );
}

const char *input_string( int vk ) {
    int frame = 0;
    if( controller_id > 0 ) return ""; // @fixme
    struct controller_t *c = input_logger(frame, +0);
    return vk >= GAMEPAD_GUID ? c->strings[vk - GAMEPAD_GUID] : ""; // if in strings...
}

// --- sugars

float input_diff( int vk ) {
    return input_frame(vk, 0) - input_frame(vk, -1);
}
vec2 input_diff2( int vk ) {
    return vec2( input_diff(vk), input_diff(vk+1) );
}
float input( int vk ) {
    return input_frame( vk, 0 );
}
vec2 input2( int vk ) {
    return vec2( input_frame(vk, 0), input_frame(vk+1, 0) );
}

// --- events

const float MS2FRAME = 0.06f; // 60 hz/1000 ms

int event( int vk ) {
    float v = input_frame(vk,0);
    return (v * v) > 0;
}
int input_chord2( int vk1, int vk2 ) {
    return event(vk1) && event(vk2);
}
int input_chord3( int vk1, int vk2, int vk3 ) {
    return event(vk1) && input_chord2(vk2, vk3);
}
int input_chord4( int vk1, int vk2, int vk3, int vk4 ) {
    return event(vk1) && input_chord3(vk2, vk3, vk4);
}
int input_down( int vk ) {
    return input_diff(vk) > 0; // input_frame(vk,-1) <= 0 && input_frame(vk,0) > 0;
}
int input_held( int vk ) {
    return input_diff(vk) == 0 && input_frame(vk,0) > 0; // input_frame(vk,-1) > 0 && input_frame(vk,0) > 0;
}
int input_up( int vk ) {
    return input_diff(vk) < 0; // input_frame(vk,-1) > 0 && input_frame(vk,0) <= 0;
}
int input_idle( int vk ) {
    return input_diff(vk) == 0 && input_frame(vk,0) <= 0; // input_frame(vk,-1) <= 0 && input_frame(vk,0) <= 0;
}
int input_repeat( int vk, int ms ) { // @fixme: broken
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) > 0 && input_frame(vk,-ms * MS2FRAME /2) > 0 && input_frame(vk,0) > 0;
}
int input_click( int vk, int ms ) { // @fixme: broken
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) <= 0 && input_frame(vk,-ms * MS2FRAME /2) > 0 && input_frame(vk,0) <= 0;
}
int input_click2( int vk, int ms ) { // @fixme: broken
    assert((unsigned)ms <= 1000);
    return input_frame(vk,-ms * MS2FRAME ) <= 0 && input_frame(vk,-ms * MS2FRAME *3/4) > 0
    && input_frame(vk,-ms * MS2FRAME *2/4) <= 0 && input_frame(vk,-ms * MS2FRAME *1/4) > 0 && input_frame(vk,0) <= 0;
}

#undef MS2FRAME

// --- filters

float input_filter_positive( float v ) { // [-1..1] -> [0..1]
    return ( v + 1 ) * 0.5f;
}
vec2  input_filter_positive2( vec2 v ) { // [-1..1] -> [0..1]
    return scale2(inc2(v,1), 0.5f);
}

vec2 input_filter_deadzone( vec2 v, float deadzone ) {
    assert(deadzone > 0);
    float mag = sqrt( v.x*v.x + v.y*v.y );
    float nx = v.x / mag, ny = v.y / mag, k = (mag - deadzone) / (1 - deadzone);
    if( k > 1 ) k = 1; // clamp
    // k = k * k; // uncomment for a smoother curve
    return mag < deadzone ? vec2(0, 0) : vec2(nx * k, ny * k);
}

vec2 input_filter_deadzone_4way( vec2 v, float deadzone ) {
    assert(deadzone > 0);
    float v0 = v.x*v.x < deadzone*deadzone ? 0 : v.x;
    float v1 = v.y*v.y < deadzone*deadzone ? 0 : v.y;
    return vec2(v0, v1);
}

// converts keyboard code to its latin char (if any)
char input_keychar(unsigned code) {
    #define k2(VK,GLFW) [KEY_##VK] = GLFW_KEY_##GLFW
    #define k(VK) k2(VK,VK)
    int table[256] = {
        k2(ESC,ESCAPE),
        k2(TICK,GRAVE_ACCENT), k(1),k(2),k(3),k(4),k(5),k(6),k(7),k(8),k(9),k(0), k2(BS,BACKSPACE),
        k(TAB),                k(Q),k(W),k(E),k(R),k(T),k(Y),k(U),k(I),k(O),k(P),
        k2(CAPS,CAPS_LOCK),      k(A),k(S),k(D),k(F),k(G),k(H),k(J),k(K),k(L), k(ENTER),
        k2(LSHIFT,LEFT_SHIFT),       k(Z),k(X),k(C),k(V),k(B),k(N),k(M),  k2(RSHIFT,RIGHT_SHIFT),                k(UP),
        k2(LCTRL,LEFT_CONTROL),k2(LALT,LEFT_ALT), k(SPACE), k2(RALT,RIGHT_ALT),k2(RCTRL,RIGHT_CONTROL), k(LEFT),k(DOWN),k(RIGHT),

        k(F1),k(F2),k(F3),k(F4),k(F5),k(F6),k(F7),k(F8),k(F9),k(F10),k(F11),k(F12), k2(PRINT,PRINT_SCREEN),k(PAUSE),
        k2(INS,INSERT),k(HOME),k2(PGUP,PAGE_UP), k2(DEL,DELETE),k(END), k2(PGDN,PAGE_DOWN),
    };
    #undef k
    #undef k2

    code = table[ code & 255 ];

    const char* name = glfwGetKeyName(code, 0);
    if( name && strlen(name) == 1 ) {
        return *name >= 'A' && *name <= 'Z' ? name[0] - 'A' + 'a' : name[0];
    }

    if( code >= GLFW_KEY_0 && code <= GLFW_KEY_9 ) return code - GLFW_KEY_0 + '0';
    if( code >= GLFW_KEY_A && code <= GLFW_KEY_Z ) return code - GLFW_KEY_A + 'a';
    switch(code) {
        default: break;
        case GLFW_KEY_APOSTROPHE:    return '\'';
        case GLFW_KEY_BACKSLASH:     return '\\';
        case GLFW_KEY_COMMA:         return ',';
        case GLFW_KEY_EQUAL:         return '=';
        case GLFW_KEY_GRAVE_ACCENT:  return '`';
        case GLFW_KEY_LEFT_BRACKET:  return '[';
        case GLFW_KEY_MINUS:         return '-';
        case GLFW_KEY_PERIOD:        return '.';
        case GLFW_KEY_RIGHT_BRACKET: return ']';
        case GLFW_KEY_SEMICOLON:     return ';';
        case GLFW_KEY_SLASH:         return '/';
        //case GLFW_KEY_WORLD_1:     return non-US #1;
        //case GLFW_KEY_WORLD_2:     return non-US #2;
    }

    return '\0';
}

// -- multi-touch input
// multi-touch(emscripten) port based on code by @procedural (MIT-0 licensed)

#if !is(ems)

void touch_init() {}
void touch_flush() {}
void input_touch_area(unsigned button, vec2 from, vec2 to)  {}
vec2 input_touch(unsigned button, float sensitivity) { return vec2(0,0); }
vec2 input_touch_delta_from_origin(unsigned button, float sensitivity) { return vec2(0,0); }
vec2 input_touch_delta(unsigned button, float sensitivity) { return vec2(0,0); }
bool input_touch_active() { return false; }

#else

static struct touch {
    bool init;
    vec2 move, cached, origin, prev;
    vec4 area;
} touch[2] = {0};

static EM_BOOL touch_move(int eventType, const EmscriptenTouchEvent *e, void *userData) {
    for( int i = 0; i < (int)e->numTouches; ++i) {
        if( !e->touches[i].isChanged ) continue;
        int j = e->touches[i].identifier;
        if( j >= countof(touch) ) continue;

        touch[j].cached = vec2(e->touches[i].clientX, e->touches[i].clientY);
        if (!touch[j].init) touch[j].init = 1, touch[j].origin = touch[j].prev = touch[j].move = touch[j].cached;
    }

    return EM_TRUE;
}

static EM_BOOL touch_end(int eventType, const EmscriptenTouchEvent *e, void *userData) {
    for( int i = 0; i < (int)e->numTouches; ++i) {
        if( !e->touches[i].isChanged ) continue;
        int j = e->touches[i].identifier;
        if( j >= countof(touch) ) continue;

        //memset(&touch[j], 0, sizeof(touch[j]));
        touch[j].init = false;
        touch[j].move = touch[j].cached = touch[j].origin = touch[j].prev = vec2(0,0);
    }

    return EM_TRUE;
}

void input_touch_area(unsigned button, vec2 from_ndc, vec2 to_ndc) {
    if( button >= countof(touch) ) return;
    touch[button].area = vec4( from_ndc.x, from_ndc.y, to_ndc.x, to_ndc.y );
}

void touch_init() {
    memset(touch, 0, sizeof(touch));

    // default areas: left screen (button #0) and right_screen (button #1)
    input_touch_area(0, vec2(0.0,0.0), vec2(0.5,1.0));
    input_touch_area(1, vec2(0.5,0.0), vec2(1.0,1.0));

    emscripten_set_touchstart_callback("#canvas", 0, EM_FALSE, &touch_move);
    emscripten_set_touchmove_callback("#canvas", 0, EM_FALSE, &touch_move);
    emscripten_set_touchend_callback("#canvas", 0, EM_FALSE, &touch_end);
}

void touch_flush() {
    for( int j = 0; j < countof(touch); ++j) {
        touch[j].prev = touch[j].move;
        touch[j].move = touch[j].cached;
    }
}

static
unsigned input_locate_button(unsigned button) {
    // locate button in user-defined areas
    vec2 c = window_canvas();
    for( int j = 0; j < countof(touch); ++j ) {
        if( touch[j].init )
        if( touch[j].origin.x >= (touch[button].area.x * c.x) )
        if( touch[j].origin.y >= (touch[button].area.y * c.y) )
        if( touch[j].origin.x <= (touch[button].area.z * c.x) )
        if( touch[j].origin.y <= (touch[button].area.w * c.y) )
            return j;
    }
    return ~0u;
}

vec2 input_touch(unsigned button, float sensitivity) {
    button = input_locate_button(button);
    if( button >= countof(touch) ) return vec2(0,0);
    return touch[button].init ? touch[button].move : vec2(0,0);
}

vec2 input_touch_delta(unsigned button, float sensitivity) {
    button = input_locate_button(button);
    if( button >= countof(touch) ) return vec2(0,0);
    return touch[button].init ? scale2( sub2(touch[button].move, touch[button].prev), sensitivity ) : vec2(0,0);
}

vec2 input_touch_delta_from_origin(unsigned button, float sensitivity) {
    button = input_locate_button(button);
    if( button >= countof(touch) ) return vec2(0,0);
    return touch[button].init ? scale2( sub2(touch[button].move, touch[button].origin), sensitivity ) : vec2(0,0);
}

bool input_touch_active() {
    for( int j = 0; j < countof(touch); ++j ) {
        if( touch[j].init ) return true;
    }
    return false;
}

#endif // !is(ems)

int ui_mouse() {
    ui_label2_float("X", input(MOUSE_X));
    ui_label2_float("Y", input(MOUSE_Y));
    ui_label2_float("Wheel", input(MOUSE_W));
    ui_separator();
    ui_label2_bool("Left", input(MOUSE_L));
    ui_label2_bool("Middle", input(MOUSE_M));
    ui_label2_bool("Right", input(MOUSE_R));
    ui_separator();
    for( int i = 0; i <= CURSOR_SW_AUTO; ++i ) if(ui_button(va("Cursor shape #%d", i))) window_cursor_shape(i);

    return 0;
}

int ui_keyboard() {
    char *keys[] = {
        "ESC",
        "TICK","1","2","3","4","5","6","7","8","9","0","BS",
        "TAB","Q","W","E","R","T","Y","U","I","O","P",
        "CAPS","A","S","D","F","G","H","J","K","L","ENTER",
        "LSHIFT","Z","X","C","V","B","N","M","RSHIFT","^",
        "LCTRL","LALT","SPACE","RALT","RCTRL","<","V",">",
    };

    vec2i rows[] = {
        vec2i(0,1),
        vec2i(1,13),
        vec2i(13,24),
        vec2i(24,35),
        vec2i(35,45),
        vec2i(45,countof(keys)),
    };

    for( int i = 0; i < countof(rows); ++i ) {
        int any = 0;
        char *row = 0;
        for( int j = rows[i].x; j < rows[i].y; ++j ) {
            strcatf(&row, input(KEY_ESC + j) ? (any|=1, "[%s]") : " %s ", keys[j]);
        }
        if(!any) ui_disable();
        ui_label(row);
        ui_enable();
        FREE(row);
    }

    return 0;
}

int ui_gamepad(int gamepad_id) {
    input_use(gamepad_id);

    bool connected = !!input(GAMEPAD_CONNECTED);

    ui_label2("Name", connected ? input_string(GAMEPAD_NAME) : "(Not connected)");

    if( !connected ) ui_disable();

    ui_separator();

    ui_label2_bool("A", input(GAMEPAD_A) );
    ui_label2_bool("B", input(GAMEPAD_B) );
    ui_label2_bool("X", input(GAMEPAD_X) );
    ui_label2_bool("Y", input(GAMEPAD_Y) );
    ui_label2_bool("Up", input(GAMEPAD_UP) );
    ui_label2_bool("Down", input(GAMEPAD_DOWN) );
    ui_label2_bool("Left", input(GAMEPAD_LEFT) );
    ui_label2_bool("Right", input(GAMEPAD_RIGHT) );
    ui_label2_bool("Menu", input(GAMEPAD_MENU) );
    ui_label2_bool("Start", input(GAMEPAD_START) );

    ui_separator();

    ui_label2_float("Left pad x", input(GAMEPAD_LPADX) );
    ui_label2_float("Left pad y", input(GAMEPAD_LPADY) );
    ui_label2_float("Left trigger", input(GAMEPAD_LT) );
    ui_label2_bool("Left bumper", input(GAMEPAD_LB) );
    ui_label2_bool("Left thumb", input(GAMEPAD_LTHUMB) );

    vec2 v = input_filter_deadzone( input2(GAMEPAD_LPADX), 0.1f );
    ui_label2_float("Filtered pad x", v.x);
    ui_label2_float("Filtered pad y", v.y);

    ui_separator();

    ui_label2_float("Right pad x", input(GAMEPAD_RPADX) );
    ui_label2_float("Right pad y", input(GAMEPAD_RPADY) );
    ui_label2_float("Right trigger", input(GAMEPAD_RT) );
    ui_label2_bool("Right bumper", input(GAMEPAD_RB) );
    ui_label2_bool("Right thumb", input(GAMEPAD_RTHUMB) );

    vec2 w = input_filter_deadzone( input2(GAMEPAD_RPADX), 0.1f );
    ui_label2_float("Filtered pad x", w.x);
    ui_label2_float("Filtered pad y", w.y);

    ui_enable();

    input_use(0);
    return 0;
}

int ui_gamepads() {
    for( int i = 0; i < 4; ++i ) ui_gamepad(i);

    return 0;
}
#line 0

#line 1 "fwk_math.c"
// -----------------------------------------------------------------------------
// math framework: rand, ease, vec2, vec3, vec4, quat, mat2, mat33, mat34, mat4
// - rlyeh, public domain
//
// Credits: @ands+@krig+@vurtun (PD), @datenwolf (WTFPL2), @evanw+@barerose (CC0), @sgorsten (Unlicense).

#include <math.h>
#include <float.h>
#include <stdint.h>
#include <stdbool.h>

vec2 atof2(const char *s) {
    vec2 v = {0};
    sscanf(s, "%f,%f", &v.x, &v.y);
    return v;
}
vec3 atof3(const char *s) {
    vec3 v = {0};
    sscanf(s, "%f,%f,%f", &v.x, &v.y, &v.z);
    return v;
}
vec4 atof4(const char *s) {
    vec4 v = {0};
    sscanf(s, "%f,%f,%f,%f", &v.x, &v.y, &v.z, &v.w);
    return v;
}

char* ftoa(float f) {
    return va("%f", f);
}
char* ftoa2(vec2 v) {
    return va("%f,%f", v.x, v.y);
}
char* ftoa3(vec3 v) {
    return va("%f,%f,%f", v.x, v.y, v.z);
}
char* ftoa4(vec4 v) {
    return va("%f,%f,%f,%f", v.x, v.y, v.z, v.w);
}

void swapf(float *a, float *b) {
    float t = *a; *a = *b; *b = *a;
}
void swapf2(vec2 *a, vec2 *b) {
    float x = a->x; a->x = b->x; b->x = a->x;
    float y = a->y; a->y = b->y; b->y = a->y;
}
void swapf3(vec3 *a, vec3 *b) {
    float x = a->x; a->x = b->x; b->x = a->x;
    float y = a->y; a->y = b->y; b->y = a->y;
    float z = a->z; a->z = b->z; b->z = a->z;
}
void swapf4(vec4 *a, vec4 *b) {
    float x = a->x; a->x = b->x; b->x = a->x;
    float y = a->y; a->y = b->y; b->y = a->y;
    float z = a->z; a->z = b->z; b->z = a->z;
    float w = a->w; a->w = b->w; b->w = a->w;
}

static uint64_t rand_xoro256(uint64_t x256_s[4]) { // xoshiro256+ 1.0 by David Blackman and Sebastiano Vigna (PD)
    const uint64_t result = x256_s[0] + x256_s[3];
    const uint64_t t = x256_s[1] << 17;

    x256_s[2] ^= x256_s[0];
    x256_s[3] ^= x256_s[1];
    x256_s[1] ^= x256_s[2];
    x256_s[0] ^= x256_s[3];

    x256_s[2] ^= t;

    x256_s[3] = (x256_s[3] << 45) | (x256_s[3] >> (64 - 45)); //x256_rotl(x256_s[3], 45);

    return result;
}
static __thread uint64_t rand_state[4] = {// = splitmix64(0),splitmix64(splitmix64(0)),... x4 times
    UINT64_C(0x9e3779b8bb0b2c64),UINT64_C(0x3c6ef372178960e7),
    UINT64_C(0xdaa66d2b71a12917),UINT64_C(0x78dde6e4d584aef9)
};
void randset(uint64_t x) {
    x = hash_64(x);
    for( int i = 0; i < 4; ++i) {
        // http://xoroshiro.di.unimi.it/splitmix64.c
        uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
        z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
        z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
        x = x ^ (z >> 31);
        rand_state[i] = x;
    }
}
uint64_t rand64(void) {
    return rand_xoro256(rand_state);
}
double randf(void) { // [0, 1) interval
    uint64_t u64 = rand64();
    // fastest way to convert in C99 a 64-bit unsigned integer to a 64-bit double
    union { uint64_t i; double d; } u; u.i = UINT64_C(0x3FF) << 52 | u64 >> 12;
    double dbl = u.d - 1.0;
    return 1 - 2.0 * ((float)(dbl / 2));
}
int randi(int mini, int maxi) { // [mini, maxi) interval ; @todo: test randi(-4,4) and #define randi(m,x) (m + randf() * (x-m))
    if( mini < maxi ) {
        uint32_t x, r, range = maxi - mini;
        do r = (x = rand64()) % range; while(range > (r-x));
        return mini + r;
    }
    return mini > maxi ? randi(maxi, mini) : mini;
}
#if 0  // @todo: deprecate me
double rng(void) { // [0..1) Lehmer RNG "minimal standard"
    static __thread unsigned int seed = 123;
    seed *= 16807;
    return seed / (double)0x100000000ULL;
}
#endif

// ----------------------------------------------------------------------------

// @todo: evaluate stb_perlin.h as well
float simplex1( float v ){ return snoise1(v); }
float simplex2( vec2 v ) { return snoise2(v.x,v.y); }
float simplex3( vec3 v ) { return snoise3(v.x,v.y,v.z); }
float simplex4( vec4 v ) { return snoise4(v.x,v.y,v.z,v.w); }

// ----------------------------------------------------------------------------

float ease_linear(float t) { return t; }

float ease_out_sine(float t) { return sinf(t*(C_PI*0.5f)); }
float ease_out_quad(float t) { return -(t*(t-2)); }
float ease_out_cubic(float t) { float f=t-1; return f*f*f+1; }
float ease_out_quart(float t) { float f=t-1; return f*f*f*(1-t)+1; }
float ease_out_quint(float t) { float f=(t-1); return f*f*f*f*f+1; }
float ease_out_expo(float t) { return (t >= 1) ? t : 1-powf(2,-10*t); }
float ease_out_circ(float t) { return sqrtf((2-t)*t); }
float ease_out_back(float t) { float f=1-t; return 1-(f*f*f-f*sinf(f*C_PI)); }
float ease_out_elastic(float t) { return sinf(-13*(C_PI*0.5f)*(t+1))*powf(2,-10*t)+1; }
float ease_out_bounce(float t) { return (t < 4.f/11) ? (121.f*t*t)/16 : (t < 8.f/11) ? (363.f/40*t*t)-(99.f/10*t)+17.f/5 : (t < 9.f/10) ? (4356.f/361*t*t)-(35442.f/1805*t)+16061.f/1805 : (54.f/5*t*t)-(513.f/25*t)+268.f/25; }

float ease_in_sine(float t) { return 1+sinf((t-1)*(C_PI*0.5f)); }
float ease_in_quad(float t) { return t*t; }
float ease_in_cubic(float t) { return t*t*t; }
float ease_in_quart(float t) { return t*t*t*t; }
float ease_in_quint(float t) { return t*t*t*t*t; }
float ease_in_expo(float t) { return (t <= 0) ? t : powf(2,10*(t-1)); }
float ease_in_circ(float t) { return 1-sqrtf(1-(t*t)); }
float ease_in_back(float t) { return t*t*t-t*sinf(t*C_PI); }
float ease_in_elastic(float t) { return sinf(13*(C_PI*0.5f)*t)*powf(2,10*(t-1)); }
float ease_in_bounce(float t) { return 1-ease_out_bounce(1-t); }

float ease_inout_sine(float t) { return 0.5f*(1-cosf(t*C_PI)); }
float ease_inout_quad(float t) { return (t < 0.5f) ? 2*t*t : (-2*t*t)+(4*t)-1; }
float ease_inout_cubic(float t) { float f; return (t < 0.5f) ? 4*t*t*t : (f=(2*t)-2,0.5f*f*f*f+1); }
float ease_inout_quart(float t) { float f; return (t < 0.5f) ? 8*t*t*t*t : (f=(t-1),-8*f*f*f*f+1); }
float ease_inout_quint(float t) { float f; return (t < 0.5f) ? 16*t*t*t*t*t : (f=((2*t)-2),0.5f*f*f*f*f*f+1); }
float ease_inout_expo(float t) { return (t <= 0 || t >= 1) ? t : t < 0.5f ? 0.5f*powf(2,(20*t)-10) : -0.5f*powf(2,(-20*t)+10)+1; }
float ease_inout_circ(float t) { return t < 0.5f ? 0.5f*(1-sqrtf(1-4*(t*t))) : 0.5f*(sqrtf(-((2*t)-3)*((2*t)-1))+1); }
float ease_inout_back(float t) { float f; return t < 0.5f ? (f=2*t,0.5f*(f*f*f-f*sinf(f*C_PI))) : (f=(1-(2*t-1)),0.5f*(1-(f*f*f-f*sinf(f*C_PI)))+0.5f); }
float ease_inout_elastic(float t) { return t < 0.5f ? 0.5f*sinf(13*(C_PI*0.5f)*(2*t))*powf(2,10*((2*t)-1)) : 0.5f*(sinf(-13*(C_PI*0.5f)*((2*t-1)+1))*powf(2,-10*(2*t-1))+2); }
float ease_inout_bounce(float t) { return t < 0.5f ? 0.5f*ease_in_bounce(t*2) : 0.5f*ease_out_bounce(t*2-1)+0.5f; }

float ease_inout_perlin(float t) { float t3=t*t*t,t4=t3*t,t5=t4*t; return 6*t5-15*t4+10*t3; }

float ease_ping_pong(float t, float(*fn1)(float), float(*fn2)(float)) { return t < 0.5 ? fn1(t*2) : fn2(1-(t-0.5)*2); }
float ease_pong_ping(float t, float(*fn1)(float), float(*fn2)(float)) { return 1 - ease_ping_pong(t,fn1,fn2); }

float ease(float t01, unsigned mode) {
    typedef float (*easing)(float);
    easing modes[] = {
        ease_linear,
        ease_out_sine,
        ease_out_quad,
        ease_out_cubic,
        ease_out_quart,
        ease_out_quint,
        ease_out_expo,
        ease_out_circ,
        ease_out_back,
        ease_out_elastic,
        ease_out_bounce,

        ease_linear,
        ease_in_sine,
        ease_in_quad,
        ease_in_cubic,
        ease_in_quart,
        ease_in_quint,
        ease_in_expo,
        ease_in_circ,
        ease_in_back,
        ease_in_elastic,
        ease_in_bounce,

        ease_linear,
        ease_inout_sine,
        ease_inout_quad,
        ease_inout_cubic,
        ease_inout_quart,
        ease_inout_quint,
        ease_inout_expo,
        ease_inout_circ,
        ease_inout_back,
        ease_inout_elastic,
        ease_inout_bounce,

        ease_inout_perlin,
    };
    return modes[clampi(mode, 0, countof(modes))](clampf(t01,0,1));
}

// ----------------------------------------------------------------------------

float deg      (float radians)      { return radians / C_PI * 180.0f; }
float rad      (float degrees)      { return degrees * C_PI / 180.0f; }

int   mini     (int    a, int    b) { return a < b ? a : b; }
int   maxi     (int    a, int    b) { return a > b ? a : b; }
int   absi     (int    a          ) { return a < 0 ? -a : a; }
int   clampi   (int v,int a,int b)  { return maxi(mini(b,v),a); }

float minf     (float  a, float  b) { return a < b ? a : b; }
float maxf     (float  a, float  b) { return a > b ? a : b; }
float absf     (float  a          ) { return a < 0.0f ? -a : a; }
float pmodf    (float  a, float  b) { return (a < 0.0f ? 1.0f : 0.0f) + (float)fmod(a, b); } // positive mod
float signf    (float  a)           { return (a < 0) ? -1.f : 1.f; }
float clampf(float v,float a,float b){return maxf(minf(b,v),a); }
float mixf(float a,float b,float t) { return a*(1-t)+b*t; }
float fractf   (float a)            { return a - (int)a; }

// ----------------------------------------------------------------------------

vec2  ptr2     (const float *a    ) { return vec2(a[0],a[1]); }
//
vec2  neg2     (vec2   a          ) { return vec2(-a.x, -a.y); }
vec2  add2     (vec2   a, vec2   b) { return vec2(a.x + b.x, a.y + b.y); }
vec2  sub2     (vec2   a, vec2   b) { return vec2(a.x - b.x, a.y - b.y); }
vec2  mul2     (vec2   a, vec2   b) { return vec2(a.x * b.x, a.y * b.y); }
vec2  div2     (vec2   a, vec2   b) { return vec2(a.x / (b.x + !b.x), a.y / (b.y + !b.y)); }
vec2  inc2     (vec2   a, float  b) { return vec2(a.x + b, a.y + b); }
vec2  dec2     (vec2   a, float  b) { return vec2(a.x - b, a.y - b); }
vec2  scale2   (vec2   a, float  b) { return vec2(a.x * b, a.y * b); }
vec2  pmod2    (vec2   a, float  b) { return vec2(pmodf(a.x, b), pmodf(a.y, b)); }
vec2  min2     (vec2   a, vec2   b) { return vec2(minf(a.x, b.x), minf(a.y, b.y)); }
vec2  max2     (vec2   a, vec2   b) { return vec2(maxf(a.x, b.x), maxf(a.y, b.y)); }
vec2  abs2     (vec2   a          ) { return vec2(absf(a.x), absf(a.y)); }
vec2  floor2   (vec2   a          ) { return vec2(floorf(a.x), floorf(a.y)); }
vec2  fract2   (vec2   a          ) { return sub2(a, floor2(a)); }
vec2  ceil2    (vec2   a          ) { return vec2(ceilf (a.x), ceilf (a.y)); }
float dot2     (vec2   a, vec2   b) { return a.x * b.x + a.y * b.y; }
vec2  refl2    (vec2   a, vec2   b) { return sub2(a, scale2(b, 2*dot2(a,b))); }
float cross2   (vec2   a, vec2   b) { return a.x * b.y - a.y * b.x; } // pseudo cross product
float len2sq   (vec2   a          ) { return a.x * a.x + a.y * a.y; }
float len2     (vec2   a          ) { return sqrtf(len2sq(a)); }
vec2  norm2    (vec2   a          ) { return len2sq(a) == 0 ? a : scale2(a, 1 / len2(a)); }
vec2  norm2sq  (vec2   a          ) { return len2sq(a) == 0 ? a : scale2(a, 1 / len2sq(a)); }
int   finite2  (vec2   a          ) { return FINITE(a.x) && FINITE(a.y); }
vec2  mix2  (vec2 a,vec2 b,float t) { return add2(scale2((a),1-(t)), scale2((b), t)); }
vec2  clamp2(vec2 v, vec2 a, vec2 b){ return vec2(maxf(minf(b.x,v.x),a.x),maxf(minf(b.y,v.y),a.y)); }
vec2 clamp2f(vec2 v,float a,float b){ return vec2(maxf(minf(b,v.x),a),maxf(minf(b,v.y),a)); }
// ----------------------------------------------------------------------------

vec3  ptr3     (const float *a    ) { return vec3(a[0],a[1],a[2]); }
vec3  vec23    (vec2   a, float z ) { return vec3(a.x,a.y,z); }
//
vec3  neg3     (vec3   a          ) { return vec3(-a.x, -a.y, -a.z); }
vec3  add3     (vec3   a, vec3   b) { return vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
vec3  sub3     (vec3   a, vec3   b) { return vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
vec3  mul3     (vec3   a, vec3   b) { return vec3(a.x * b.x, a.y * b.y, a.z * b.z); }
vec3  div3     (vec3   a, vec3   b) { return vec3(a.x / (b.x + !b.x), a.y / (b.y + !b.y), a.z / (b.z + !b.z)); }
vec3  inc3     (vec3   a, float  b) { return vec3(a.x + b, a.y + b, a.z + b); }
vec3  dec3     (vec3   a, float  b) { return vec3(a.x - b, a.y - b, a.z - b); }
vec3  scale3   (vec3   a, float  b) { return vec3(a.x * b, a.y * b, a.z * b); }
vec3  pmod3    (vec3   a, float  b) { return vec3(pmodf(a.x, b), pmodf(a.y, b), pmodf(a.z, b)); }
vec3  min3     (vec3   a, vec3   b) { return vec3(minf(a.x, b.x), minf(a.y, b.y), minf(a.z, b.z)); }
vec3  max3     (vec3   a, vec3   b) { return vec3(maxf(a.x, b.x), maxf(a.y, b.y), maxf(a.z, b.z)); }
vec3  abs3     (vec3   a          ) { return vec3(absf(a.x), absf(a.y), absf(a.z)); }
vec3  floor3   (vec3   a          ) { return vec3(floorf(a.x), floorf(a.y), floorf(a.z)); }
vec3  fract3   (vec3   a          ) { return sub3(a, floor3(a)); }
vec3  ceil3    (vec3   a          ) { return vec3(ceilf (a.x), ceilf (a.y), ceilf (a.z)); }
vec3  cross3   (vec3   a, vec3   b) { return vec3(a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y); }
float dot3     (vec3   a, vec3   b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
vec3  refl3    (vec3   a, vec3   b) { return sub3(a, scale3(b, 2*dot3(a, b))); }
float len3sq   (vec3   a          ) { return dot3(a,a); }
float len3     (vec3   a          ) { return sqrtf(len3sq(a)); }
vec3  norm3    (vec3   a          ) { return len3sq(a) == 0 ? a : scale3(a, 1 / len3(a)); }
vec3  norm3sq  (vec3   a          ) { return len3sq(a) == 0 ? a : scale3(a, 1 / len3sq(a)); }
int   finite3  (vec3   a          ) { return finite2(vec2(a.x,a.y)) && FINITE(a.z); }
vec3  mix3  (vec3 a,vec3 b,float t) { return add3(scale3((a),1-(t)), scale3((b), t)); }
vec3  clamp3(vec3 v, vec3 a, vec3 b){ return vec3(maxf(minf(b.x,v.x),a.x),maxf(minf(b.y,v.y),a.y),maxf(minf(b.z,v.z),a.z)); }
vec3 clamp3f(vec3 v,float a,float b){ return vec3(maxf(minf(b,v.x),a),maxf(minf(b,v.y),a),maxf(minf(b,v.z),a)); }
//vec3 tricross3 (vec3 a, vec3 b, vec3 c) { return cross3(a,cross3(b,c)); } // useful?
void  ortho3   (vec3 *left, vec3 *up, vec3 v) {
#if 0
    if ((v.z * v.z) > (0.7f * 0.7f)) {
        float sqrlen  = v.y*v.y + v.z*v.z;
        float invlen  = 1.f / sqrtf(sqrlen);
        *up = vec3(0, v.z*invlen, -v.y*invlen);
        *left = vec3(sqrlen*invlen, -v.x*up->z, v.x*up->y);
    } else {
        float sqrlen = v.x*v.x + v.y*v.y;
        float invlen = 1.f / sqrtf(sqrlen);
        *left = vec3(-v.y*invlen, v.x*invlen, 0);
        *up = vec3(-v.z*left->y, v.z*left->x, sqrlen*invlen);
    }
#else
    *left = (v.z*v.z) < (v.x*v.x) ? vec3(v.y,-v.x,0) : vec3(0,-v.z,v.y);
    *up = cross3(*left, v);
#endif
}

#define rotateq3(v,q) transformq(q,v)
vec3 rotatex3(vec3 dir, float degrees) { return rotateq3(dir, rotationq(degrees,1,0,0)); }
vec3 rotatey3(vec3 dir, float degrees) { return rotateq3(dir, rotationq(degrees,0,1,0)); }
vec3 rotatez3(vec3 dir, float degrees) { return rotateq3(dir, rotationq(degrees,0,0,1)); }

// ----------------------------------------------------------------------------

vec4  ptr4     (const float *a    ) { return vec4(a[0],a[1],a[2],a[3]); }
vec4  vec34    (vec3   a, float w ) { return vec4(a.x,a.y,a.z,w); }
//
vec4  neg4     (vec4   a          ) { return vec4(-a.x, -a.y, -a.z, -a.w); }
vec4  add4     (vec4   a, vec4   b) { return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
vec4  sub4     (vec4   a, vec4   b) { return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
vec4  mul4     (vec4   a, vec4   b) { return vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
vec4  div4     (vec4   a, vec4   b) { return vec4(a.x / (b.x + !b.x), a.y / (b.y + !b.y), a.z / (b.z + !b.z), a.w / (b.w + !b.w)); }
vec4  inc4     (vec4   a, float  b) { return vec4(a.x + b, a.y + b, a.z + b, a.w + b); }
vec4  dec4     (vec4   a, float  b) { return vec4(a.x - b, a.y - b, a.z - b, a.w - b); }
vec4  scale4   (vec4   a, float  b) { return vec4(a.x * b, a.y * b, a.z * b, a.w * b); }
vec4  pmod4    (vec4   a, float  b) { return vec4(pmodf(a.x, b), pmodf(a.y, b), pmodf(a.z, b), pmodf(a.w, b)); }
vec4  min4     (vec4   a, vec4   b) { return vec4(minf(a.x, b.x), minf(a.y, b.y), minf(a.z, b.z), minf(a.w, b.w)); }
vec4  max4     (vec4   a, vec4   b) { return vec4(maxf(a.x, b.x), maxf(a.y, b.y), maxf(a.z, b.z), maxf(a.w, b.w)); }
vec4  abs4     (vec4   a          ) { return vec4(absf(a.x), absf(a.y), absf(a.z), absf(a.w)); }
vec4  floor4   (vec4   a          ) { return vec4(floorf(a.x), floorf(a.y), floorf(a.z), floorf(a.w)); }
vec4  fract4   (vec4   a          ) { return sub4(a, floor4(a)); }
vec4  ceil4    (vec4   a          ) { return vec4(ceilf (a.x), ceilf (a.y), ceilf (a.z), ceilf (a.w)); }
float dot4     (vec4   a, vec4   b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
vec4  refl4    (vec4   a, vec4   b) { return sub4(a, scale4(b, 2*dot4(a, b))); }
float len4sq   (vec4   a          ) { return dot4(a,a); }
float len4     (vec4   a          ) { return sqrtf(len4sq(a)); }
vec4  norm4    (vec4   a          ) { return len4sq(a) == 0 ? a : scale4(a, 1 / len4(a)); }
vec4  norm4sq  (vec4   a          ) { return len4sq(a) == 0 ? a : scale4(a, 1 / len4sq(a)); }
int   finite4  (vec4   a          ) { return finite3(vec3(a.x,a.y,a.z)) && FINITE(a.w); }
vec4  mix4  (vec4 a,vec4 b,float t) { return add4(scale4((a),1-(t)), scale4((b), t)); }
vec4  clamp4(vec4 v, vec4 a, vec4 b){ return vec4(maxf(minf(b.x,v.x),a.x),maxf(minf(b.y,v.y),a.y),maxf(minf(b.z,v.z),a.z),maxf(minf(b.w,v.w),a.w)); }
vec4 clamp4f(vec4 v,float a,float b){ return vec4(maxf(minf(b,v.x),a),maxf(minf(b,v.y),a),maxf(minf(b,v.z),a),maxf(minf(b,v.w),a)); }
// vec4 cross4(vec4 v0, vec4 v1) { return vec34(cross3(v0.xyz, v1.xyz), (v0.w + v1.w) * 0.5f); } // may fail

// ----------------------------------------------------------------------------

quat  idq      (                  ) { return quat(1,0,0,0); } // 0,0,0,1?
quat  ptrq     (const float *a    ) { return quat(a[0],a[1],a[2],a[3]); }
quat  vec3q    (vec3   a, float w ) { return quat(a.x,a.y,a.z,w); }
quat  vec4q    (vec4   a          ) { return quat(a.x,a.y,a.z,a.w); }
//
quat  negq     (quat   a          ) { return quat(-a.x,-a.y,-a.z,-a.w); }
quat  conjq    (quat   a          ) { return quat(-a.x,-a.y,-a.z,a.w); }
quat  addq     (quat   a, quat   b) { return quat(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w); }
quat  subq     (quat   a, quat   b) { return quat(a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w); }
quat  mulq     (quat   p, quat   q) { vec3 w = scale3(p.xyz, q.w), r = add3(add3(cross3(p.xyz, q.xyz), w), scale3(q.xyz, p.w)); return quat(r.x,r.y,r.z,p.w*q.w - dot3(p.xyz, q.xyz)); }
quat  scaleq   (quat   a, float  s) { return quat(a.x*s,a.y*s,a.z*s,a.w*s); }
quat  normq    (quat   a          ) { vec4 v = norm4(a.xyzw); return quat(v.x,v.y,v.z,v.w); }
float dotq     (quat   a, quat   b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
quat  mixq(quat a, quat b, float t) { return normq(dotq(a,b) < 0 ? addq(negq(a),scaleq(addq(b,a),t)) : addq(a,scaleq(subq(b,a),t))); }
/* quat lerpq(quat a, quat b, float s) {
    return norm(quat((1-s)*a.x + s*b.x, (1-s)*a.y + s*b.y, (1-s)*a.z + s*b.z, (1-s)*a.w + s*b.w));
}*/
quat slerpq(quat a, quat b, float s) { //ok ?
    float t = acosf(dotq(a,b)), st = sinf(t), wa = sinf((1-s)*t)/st, wb = sinf(s*t)/st;
    return normq(quat(wa*a.x + wb*b.x, wa*a.y + wb*b.y, wa*a.z + wb*b.z, wa*a.w + wb*b.w));
}

quat  rotationq(float deg,float x,float y,float z){ deg=rad(deg)*0.5f; return vec3q(scale3(vec3(x,y,z),sinf(deg)),cosf(deg)); }
quat  mat44q   (mat44 M) {
    float r=0.f;
    int perm[] = { 0, 1, 2, 0, 1 }, *p = perm;
    for(int i = 0; i<3; i++) {
        float m = M[i*4+i];
        if( m < r ) continue;
        m = r;
        p = &perm[i];
    }
    r = sqrtf(1.f + M[p[0]*4+p[0]] - M[p[1]*4+p[1]] - M[p[2]*4+p[2]] );
    return r >= 1e-6 ? quat(1,0,0,0)
    : quat(r/2.f, (M[p[0]*4+p[1]] - M[p[1]*4+p[0]])/(2.f*r), (M[p[2]*4+p[0]] - M[p[0]*4+p[2]])/(2.f*r), (M[p[2]*4+p[1]] - M[p[1]*4+p[2]])/(2.f*r) );
}

vec3 rotate3q_2(vec3 v, quat q) { // rotate vec3 by quat @testme
    vec3 u = {q.x, q.y, q.z}; // extract the vector part of the quaternion
    float s = q.w;            // scalar part
    vec3 cuv = cross3(u, v);
    float duv2 = dot3(u, v) * 2;
    float ss_duu = s*s - dot3(u, u);
    return add3(add3(scale3(u,duv2), scale3(v,ss_duu)), scale3(cuv,2*s));
}

vec3 rotate3q(vec3 v, quat r) { // rotate vec3 by quat @testme
    float num12 = r.x + r.x;
    float num2 = r.y + r.y;
    float num = r.z + r.z;
    float num11 = r.w * num12;
    float num10 = r.w * num2;
    float num9 = r.w * num;
    float num8 = r.x * num12;
    float num7 = r.x * num2;
    float num6 = r.x * num;
    float num5 = r.y * num2;
    float num4 = r.y * num;
    float num3 = r.z * num;
    float num15 = ((v.x * ((1 - num5) - num3)) + (v.y * (num7 - num9))) + (v.z * (num6 + num10));
    float num14 = ((v.x * (num7 + num9)) + (v.y * ((1 - num8) - num3))) + (v.z * (num4 - num11));
    float num13 = ((v.x * (num6 - num10)) + (v.y * (num4 + num11))) + (v.z * ((1 - num8) - num5));
    return vec3(num15, num14, num13);
}

// euler <-> quat
vec3  euler    (quat q) { // bugs? returns PitchYawRoll (PYR) in degrees. ref: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    float sr_cp = 2*(q.x*q.y + q.z*q.w), cr_cp = 1-2*(q.y*q.y + q.z*q.z);
    float sy_cp = 2*(q.x*q.w + q.y*q.z), cy_cp = 1-2*(q.z*q.z + q.w*q.w), sp = 2*(q.x*q.z-q.w*q.y);
    float p = fabs(sp) >= 1 ? copysignf(C_PI / 2, sp) : asinf(sp);
    float y = atan2f(sy_cp, cy_cp);
    float r = atan2f(sr_cp, cr_cp);
    return scale3(vec3(p, y, r), TO_DEG);
}

quat  eulerq   (vec3 pyr_degrees) { // bugs?
#if 0
    quat x = vec3q(vec3(1,0,0),rad(pyr_degrees.x)); // x, not pitch
    quat y = vec3q(vec3(0,1,0),rad(pyr_degrees.y)); // y, not yaw
    quat z = vec3q(vec3(0,0,1),rad(pyr_degrees.z)); // z, not row
    return normq(mulq(mulq(x, y), z));
#else
    float p = rad(pyr_degrees.x), y = rad(pyr_degrees.y), r = rad(pyr_degrees.z);
    float ha = p * 0.5f, hb = r * 0.5f, hc = y * 0.5f;
    float cp = cosf(ha), sp = sinf(ha), cr = cosf(hb), sr = sinf(hb), cy = cosf(hc), sy = sinf(hc);
    return quat(cy*cr*cp + sy*sr*sp, cy*sr*cp - sy*cr*sp, cy*cr*sp + sy*sr*cp, sy*cr*cp - cy*sr*sp);
#endif
}

// ----------------------------------------------------------------------------

void scaling33(mat33 m, float x, float y, float z) { // !!! ok, i guess
    m[0] = x; m[1] = 0; m[2] = 0;
    m[3] = 0; m[4] = y; m[5] = 0;
    m[6] = 0; m[7] = 0; m[8] = z;
}
void scale33(mat33 m, float x, float y, float z) {
#if 0 // original !!! ok, i guess
    m[0] *= x; m[1] *= x; m[2] *= x;
    m[3] *= y; m[4] *= y; m[5] *= y;
    m[6] *= z; m[7] *= z; m[8] *= z;
#else
    m[0] *= x; m[3] *= x; m[6] *= x;
    m[1] *= y; m[4] *= y; m[7] *= y;
    m[2] *= z; m[5] *= z; m[8] *= z;
#endif
}
void id33(mat33 m) {
    scaling33(m, 1,1,1);
}
void extract33(mat33 m, const mat44 M) {  // extract rot/sca from mat44
    m[0] = M[0]; m[1] = M[1]; m[2] = M[ 2];
    m[3] = M[4]; m[4] = M[5]; m[5] = M[ 6];
    m[6] = M[8]; m[7] = M[9]; m[8] = M[10];
}
void copy33(mat33 m, const mat33 a) {
    for(int i = 0; i < 9; ++i) m[i] = a[i];
}
//
vec3 mulv33(mat33 m, vec3 v) {
    return vec3(m[0]*v.x+m[1]*v.y+m[2]*v.z,m[3]*v.x+m[4]*v.y+m[5]*v.z,m[6]*v.x+m[7]*v.y+m[8]*v.z);
}
void multiply33x2(mat33 m, const mat33 a, const mat33 b) {
    m[0] = a[0]*b[0]+a[1]*b[3]+a[2]*b[6];
    m[1] = a[0]*b[1]+a[1]*b[4]+a[2]*b[7];
    m[2] = a[0]*b[2]+a[1]*b[5]+a[2]*b[8];

    m[3] = a[3]*b[0]+a[4]*b[3]+a[5]*b[6];
    m[4] = a[3]*b[1]+a[4]*b[4]+a[5]*b[7];
    m[5] = a[3]*b[2]+a[4]*b[5]+a[5]*b[8];

    m[6] = a[6]*b[0]+a[7]*b[3]+a[8]*b[6];
    m[7] = a[6]*b[1]+a[7]*b[4]+a[8]*b[7];
    m[8] = a[6]*b[2]+a[7]*b[5]+a[8]*b[8];
}

void rotation33(mat33 m, float degrees, float x,float y,float z) {
    float radians = degrees * C_PI / 180.0f;
    float s = sinf(radians), c = cosf(radians), c1 = 1.0f - c;
    float xy = x*y, yz = y*z, zx = z*x, xs = x*s, ys = y*s, zs = z*s;
    m[0] = c1*x*x+c; m[1] = c1*xy-zs; m[2] = c1*zx+ys;
    m[3] = c1*xy+zs; m[4] = c1*y*y+c; m[5] = c1*yz-xs;
    m[6] = c1*zx-ys; m[7] = c1*yz+xs; m[8] = c1*z*z+c;
}
void rotationq33(mat33 m, quat q) {
#if  0
    float a  = q.w, b  = q.x, c  = q.y, d  = q.z;
    float a2 = a*a, b2 = b*b, c2 = c*c, d2 = d*d;
    m[ 0] = a2 + b2 - c2 - d2; m[ 1] = 2*(b*c + a*d);     m[ 2] = 2*(b*d - a*c);
    m[ 3] = 2*(b*c - a*d);     m[ 4] = a2 - b2 + c2 - d2; m[ 5] = 2*(c*d + a*b);
    m[ 6] = 2*(b*d + a*c);     m[ 7] = 2*(c*d - a*b);     m[ 8] = a2 - b2 - c2 + d2;
#else
    float x2 = q.x*q.x, y2 = q.y*q.y, z2 = q.z*q.z, w2 = q.w*q.w;
    float xz = q.x*q.z, xy = q.x*q.y, yz = q.y*q.z, wz = q.w*q.z, wy = q.w*q.y, wx = q.w*q.x;
    m[0] = 1-2*(y2+z2); m[1] =   2*(xy+wz); m[2] =   2*(xz-wy);
    m[3] =   2*(xy-wz); m[4] = 1-2*(x2+z2); m[5] =   2*(yz+wx);
    m[6] =   2*(xz+wy); m[7] =   2*(yz-wx); m[8] = 1-2*(x2+y2);
#endif
}
void rotate33(mat33 r, float degrees, float x,float y,float z) {
    if(len3sq(vec3(x,y,z)) < (1e-4 * 1e-4)) return;
    float m[9]; rotate33(m, degrees, x,y,z);
    multiply33x2(r, r, m);
}
void compose33(mat33 m, quat r, vec3 s) { // verify me
    rotationq33(m, r);
    scale33(m, s.x,s.y,s.z);
}

// ----------------------------------------------------------------------------

void id34(mat34 m) { // verify me
    m[ 0] = 1; m[ 1] = 0; m[ 2] = 0; m[ 3] = 0;
    m[ 4] = 0; m[ 5] = 1; m[ 6] = 0; m[ 7] = 0;
    m[ 8] = 0; m[ 9] = 0; m[10] = 1; m[11] = 0;
}
void copy34(mat34 m, const mat34 a) {
    for( int i = 0; i < 12; ++i ) m[i] = a[i];
}
void scale34(mat34 m, float s) {
    for( int i = 0; i < 12; ++i ) m[i] *= s;
}
void add34(mat34 m, mat34 n) {
    for( int i = 0; i < 12; ++i ) m[i] += n[i];
}
void muladd34(mat34 m, mat34 n, float s) {
    for( int i = 0; i < 12; ++i ) m[i] += n[i] * s;
}
void add34x2(mat34 m, mat34 n, mat34 o) {
    for( int i = 0; i < 12; ++i ) m[i] = n[i] + o[i];
}
void lerp34(mat34 m, mat34 n, mat34 o, float alpha) {
    for( int i = 0; i < 12; ++i ) m[i] = n[i] * (1-alpha) + o[i] * alpha;
}
void multiply34x2(mat34 m, const mat34 m0, const mat34 m1) {
    vec4 r0 = { m0[0*4+0], m0[0*4+1], m0[0*4+2], m0[0*4+3] }; // rows
    vec4 r1 = { m0[1*4+0], m0[1*4+1], m0[1*4+2], m0[1*4+3] };
    vec4 r2 = { m0[2*4+0], m0[2*4+1], m0[2*4+2], m0[2*4+3] };

    vec4 c0 = { m1[0*4+0], m1[1*4+0], m1[2*4+0], 0.0f }; // cols
    vec4 c1 = { m1[0*4+1], m1[1*4+1], m1[2*4+1], 0.0f };
    vec4 c2 = { m1[0*4+2], m1[1*4+2], m1[2*4+2], 0.0f };
    vec4 c3 = { m1[0*4+3], m1[1*4+3], m1[2*4+3], 1.0f };

    m[ 0] = dot4(r0, c0); m[ 1] = dot4(r0, c1); m[ 2] = dot4(r0, c2); m[ 3] = dot4(r0, c3);
    m[ 4] = dot4(r1, c0); m[ 5] = dot4(r1, c1); m[ 6] = dot4(r1, c2); m[ 7] = dot4(r1, c3);
    m[ 8] = dot4(r2, c0); m[ 9] = dot4(r2, c1); m[10] = dot4(r2, c2); m[11] = dot4(r2, c3);
}
void multiply34(mat34 m, const mat34 a) {
    mat34 x; copy34(x, m);
    multiply34x2(m, x, a);
}
void multiply34x3(mat34 m, const mat34 a, const mat34 b, const mat34 c) {
    mat34 x;
    multiply34x2(x, a, b);
    multiply34x2(m, x, c);
}
void compose34(mat34 m, vec3 t, quat q, vec3 s) {
    m[ 0] = s.x * (1 - 2 * q.y * q.y - 2 * q.z * q.z);
    m[ 1] = s.y * (    2 * q.x * q.y - 2 * q.w * q.z);
    m[ 2] = s.z * (    2 * q.x * q.z + 2 * q.w * q.y);
    m[ 3] = t.x;
    m[ 4] = s.x * (    2 * q.x * q.y + 2 * q.w * q.z);
    m[ 5] = s.y * (1 - 2 * q.x * q.x - 2 * q.z * q.z);
    m[ 6] = s.z * (    2 * q.y * q.z - 2 * q.w * q.x);
    m[ 7] = t.y;
    m[ 8] = s.x * (    2 * q.x * q.z - 2 * q.w * q.y);
    m[ 9] = s.y * (    2 * q.y * q.z + 2 * q.w * q.x);
    m[10] = s.z * (1 - 2 * q.x * q.x - 2 * q.y * q.y);
    m[11] = t.z;
}
void invert34(mat34 m, const mat34 o) {
    vec3 a = norm3sq(vec3(o[0*4+0], o[1*4+0], o[2*4+0]));
    vec3 b = norm3sq(vec3(o[0*4+1], o[1*4+1], o[2*4+1]));
    vec3 c = norm3sq(vec3(o[0*4+2], o[1*4+2], o[2*4+2]));
    vec3 trans = vec3(o[0*4+3], o[1*4+3], o[2*4+3]);
    vec4 A = vec34(a, -dot3(a, trans));
    vec4 B = vec34(b, -dot3(b, trans));
    vec4 C = vec34(c, -dot3(c, trans));
    m[ 0] = A.x; m[ 1] = A.y; m[ 2] = A.z; m[ 3] = A.w;
    m[ 4] = B.x; m[ 5] = B.y; m[ 6] = B.z; m[ 7] = B.w;
    m[ 8] = C.x; m[ 9] = C.y; m[10] = C.z; m[11] = C.w;
}

// ----------------------------------------------------------------------------

void scaling44(mat44 m, float x, float y, float z);
void id44(mat44 m) {
    scaling44(m, 1,1,1);
}
void identity44(mat44 m) {
    scaling44(m, 1,1,1);
}
void copy44(mat44 m, const mat44 a) {
    for( int i = 0; i < 16; ++i ) m[i] = a[i];
}
void multiply44x2(mat44 m, const mat44 a, const mat44 b) {
    for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
    m[y*4+x] = a[x] * b[y*4]+a[4+x] * b[y*4+1]+a[8+x] * b[y*4+2]+a[12+x] * b[y*4+3];
}
void multiply44x3(mat44 m, const mat44 a, const mat44 b, const mat44 c) {
    mat44 x;
    multiply44x2(x, a, b);
    multiply44x2(m, x, c);
}
void multiply44(mat44 m, const mat44 a) {
    mat44 b; copy44(b, m);
    multiply44x2(m, b, a);
}
// ---
void ortho44(mat44 m, float l, float r, float b, float t, float n, float f) {
    m[ 0] = 2/(r-l);      m[ 1] = 0;            m[ 2] = 0;            m[ 3] = 0;
    m[ 4] = 0;            m[ 5] = 2/(t-b);      m[ 6] = 0;            m[ 7] = 0;
    m[ 8] = 0;            m[ 9] = 0;            m[10] = -2/(f-n);     m[11] = 0;
    m[12] = -(r+l)/(r-l); m[13] = -(t+b)/(t-b); m[14] = -(f+n)/(f-n); m[15] = 1;
}
void frustum44(mat44 m, float l, float r, float b, float t, float n, float f) {
    m[ 0] = 2*n/(r-l);   m[ 1] = 0;           m[ 2] = 0;               m[ 3] = 0;
    m[ 4] = 0;           m[ 5] = 2*n/(t-b);   m[ 6] = 0;               m[ 7] = 0;
    m[ 8] = (r+l)/(r-l); m[ 9] = (t+b)/(t-b); m[10] = -(f+n)/(f-n);    m[11] = -1;
    m[12] = 0;           m[13] = 0;           m[14] = -2*(f*n)/(f-n);  m[15] = 0;
}
void perspective44(mat44 m, float fovy_degrees, float aspect, float nearp, float farp) {
    float y = tanf(fovy_degrees * C_PI / 360) * nearp, x = y * aspect;
    frustum44(m, -x, x, -y, y, nearp, farp);
}
void lookat44(mat44 m, vec3 eye, vec3 center, vec3 up) {
    vec3 f = norm3(sub3(center, eye));
    vec3 r = norm3(cross3(f, up));
    vec3 u = cross3(r, f);
    m[ 0] = r.x;           m[ 1] = u.x;           m[ 2] = -f.x;         m[ 3] = 0;
    m[ 4] = r.y;           m[ 5] = u.y;           m[ 6] = -f.y;         m[ 7] = 0;
    m[ 8] = r.z;           m[ 9] = u.z;           m[10] = -f.z;         m[11] = 0;
    m[12] = -dot3(r, eye); m[13] = -dot3(u, eye); m[14] = dot3(f, eye); m[15] = 1;
}
// ---
void translation44(mat44 m, float x, float y, float z) { // identity4 + translate4
    m[ 0] = 1.0f; m[ 1] = 0.0f; m[ 2] = 0.0f; m[ 3] = 0.0f;
    m[ 4] = 0.0f; m[ 5] = 1.0f; m[ 6] = 0.0f; m[ 7] = 0.0f;
    m[ 8] = 0.0f; m[ 9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
    m[12] = x;    m[13] = y;    m[14] = z;    m[15] = 1.0f;
}
void translate44(mat44 m, float x, float y, float z) { // translate in place
#if 0 // original
    vec4 t = {x, y, z, 0};
    m[12] += dot4(vec4(m[0],m[4],m[ 8],m[12]),t); // row4(M,0)
    m[13] += dot4(vec4(m[1],m[5],m[ 9],m[13]),t); // row4(M,1)
    m[14] += dot4(vec4(m[2],m[6],m[10],m[14]),t); // row4(M,2)
    m[15] += dot4(vec4(m[3],m[7],m[11],m[15]),t); // row4(M,3)
#else
    m[12] += m[ 0]*x + m[ 4]*y + m[ 8]*z;
    m[13] += m[ 1]*x + m[ 5]*y + m[ 9]*z;
    m[14] += m[ 2]*x + m[ 6]*y + m[10]*z;
    m[15] += m[ 3]*x + m[ 7]*y + m[11]*z;
#endif
}
void relocate44(mat44 m, float x, float y, float z) {
    m[12] = x; m[13] = y; m[14] = z;
}
void rotationq44(mat44 m, quat q) {
#if  0
    float a  = q.w, b  = q.x, c  = q.y, d  = q.z;
    float a2 = a*a, b2 = b*b, c2 = c*c, d2 = d*d;
    m[ 0] = a2 + b2 - c2 - d2; m[ 1] = 2*(b*c + a*d);     m[ 2] = 2*(b*d - a*c);     m[ 3] = 0;
    m[ 4] = 2*(b*c - a*d);     m[ 5] = a2 - b2 + c2 - d2; m[ 6] = 2*(c*d + a*b);     m[ 7] = 0;
    m[ 8] = 2*(b*d + a*c);     m[ 9] = 2*(c*d - a*b);     m[10] = a2 - b2 - c2 + d2; m[11] = 0;
    m[12] = 0;                 m[13] = 0;                 m[14] = 0;                 m[15] = 1;
#else
    float x2 = q.x*q.x, y2 = q.y*q.y, z2 = q.z*q.z, w2 = q.w*q.w;
    float xz = q.x*q.z, xy = q.x*q.y, yz = q.y*q.z, wz = q.w*q.z, wy = q.w*q.y, wx = q.w*q.x;
    m[ 0] = 1-2*(y2+z2); m[ 1] =   2*(xy+wz); m[ 2] =   2*(xz-wy); m[ 3] = 0;
    m[ 4] =   2*(xy-wz); m[ 5] = 1-2*(x2+z2); m[ 6] =   2*(yz+wx); m[ 7] = 0;
    m[ 8] =   2*(xz+wy); m[ 9] =   2*(yz-wx); m[10] = 1-2*(x2+y2); m[11] = 0;
    m[12] = 0;           m[13] = 0;           m[14] = 0;           m[15] = 1;
#endif
}
void rotation44(mat44 m, float degrees, float x, float y, float z) {
    //if(len3sq(vec3(x,y,z)) < (1e-4 * 1e-4)) return;

    float radians = degrees * C_PI / 180.0f;
    float c = cosf(radians), s = sinf(radians), c1 = 1.0f - c;
    m[ 0] = x*x*c1 + c;   m[ 1] = y*x*c1 + z*s; m[ 2] = x*z*c1 - y*s; m[ 3] = 0.0f;
    m[ 4] = x*y*c1 - z*s; m[ 5] = y*y*c1 + c;   m[ 6] = y*z*c1 + x*s; m[ 7] = 0.0f;
    m[ 8] = x*z*c1 + y*s; m[ 9] = y*z*c1 - x*s; m[10] = z*z*c1 + c;   m[11] = 0.0f;
    m[12] = 0.0f;         m[13] = 0.0f;         m[14] = 0.0f;         m[15] = 1.0f;
}
void rotate44(mat44 m, float degrees, float x, float y, float z) { // !!! ok, i guess
    if(len3sq(vec3(x,y,z)) < (1e-4 * 1e-4)) return;

    float radians = degrees * C_PI / 180.0f;
    float c = cosf(radians), s = -sinf(radians), c1 = 1 - c;
    float m00 = m[ 0],  m01 = m[ 1],  m02 = m[ 2], m03 = m[ 3],
          m04 = m[ 4],  m05 = m[ 5],  m06 = m[ 6], m07 = m[ 7],
          m08 = m[ 8],  m09 = m[ 9],  m10 = m[10], m11 = m[11];

    // rotation matrix
    float r00 = x*x*c1 + c,   r01 = y*x*c1 + z*s, r02 = x*z*c1 - y*s;
    float r04 = x*y*c1 - z*s, r05 = y*y*c1 + c,   r06 = y*z*c1 + x*s;
    float r08 = x*z*c1 + y*s, r09 = y*z*c1 - x*s, r10 = z*z*c1 + c;

    // multiply
    m[ 0] = r00 * m00 + r04 * m04 + r08 * m08;
    m[ 1] = r00 * m01 + r04 * m05 + r08 * m09;
    m[ 2] = r00 * m02 + r04 * m06 + r08 * m10;
    m[ 3] = r00 * m03 + r04 * m07 + r08 * m11;
    m[ 4] = r01 * m00 + r05 * m04 + r09 * m08;
    m[ 5] = r01 * m01 + r05 * m05 + r09 * m09;
    m[ 6] = r01 * m02 + r05 * m06 + r09 * m10;
    m[ 7] = r01 * m03 + r05 * m07 + r09 * m11;
    m[ 8] = r02 * m00 + r06 * m04 + r10 * m08;
    m[ 9] = r02 * m01 + r06 * m05 + r10 * m09;
    m[10] = r02 * m02 + r06 * m06 + r10 * m10;
    m[11] = r02 * m03 + r06 * m07 + r10 * m11;
}
void scaling44(mat44 m, float x, float y, float z) { // !!! ok, i guess
    m[ 0] = x; m[ 1] = 0; m[ 2] = 0; m[ 3] = 0;
    m[ 4] = 0; m[ 5] = y; m[ 6] = 0; m[ 7] = 0;
    m[ 8] = 0; m[ 9] = 0; m[10] = z; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}
void scale44(mat44 m, float x, float y, float z) {
#if 0 // original !!! ok, i guess
    m[ 0] *= x; m[ 1] *= x; m[ 2] *= x; m[ 3] *= x;
    m[ 4] *= y; m[ 5] *= y; m[ 6] *= y; m[ 7] *= y;
    m[ 8] *= z; m[ 9] *= z; m[10] *= z; m[11] *= z;
#else
    m[0] *= x; m[4] *= x; m[8] *= x;
    m[1] *= y; m[5] *= y; m[9] *= y;
    m[2] *= z; m[6] *= z; m[10] *= z;
#endif
}
// ---
void transpose44(mat44 m, const mat44 a) { // M[i][j] = A[j][i];
    m[ 0] = a[0]; m[ 1] = a[4]; m[ 2] = a[ 8]; m[ 3] = a[12];
    m[ 4] = a[1]; m[ 5] = a[5]; m[ 6] = a[ 9]; m[ 7] = a[13];
    m[ 8] = a[2]; m[ 9] = a[6]; m[10] = a[10]; m[11] = a[14];
    m[12] = a[3]; m[13] = a[7]; m[14] = a[11]; m[15] = a[15];
}

// @todo: test me
// float det33 = M[0,0]*((M[1,1]*M[2,2])-(M[2,1]*M[1,2]))-M[0,1]*(M[1,0]*M[2,2]-M[2,0]*M[1,2])+M[0,2]*(M[1,0]*M[2,1]-M[2,0]*M[1,1]);
//
// float det33 = 
//   rgt.x * fwd.y * upv.z - rgt.z * fwd.y * upv.x +
//   rgt.y * fwd.z * upv.x - rgt.y * fwd.x * upv.z + 
//   rgt.z * fwd.x * upv.y - rgt.x * fwd.z * upv.y;
//
// void transpose33(mat33 m, const mat33 a) { // M[i][j] = A[j][i];
//     m[0] = a[0]; m[1] = a[3]; m[2] = a[6];
//     m[3] = a[1]; m[4] = a[4]; m[5] = a[7];
//     m[6] = a[2]; m[7] = a[5]; m[8] = a[8];
// }

float det44(const mat44 M) { // !!! ok, i guess
    float s[6], c[6];
    s[0] = M[0*4+0]*M[1*4+1] - M[1*4+0]*M[0*4+1];
    s[1] = M[0*4+0]*M[1*4+2] - M[1*4+0]*M[0*4+2];
    s[2] = M[0*4+0]*M[1*4+3] - M[1*4+0]*M[0*4+3];
    s[3] = M[0*4+1]*M[1*4+2] - M[1*4+1]*M[0*4+2];
    s[4] = M[0*4+1]*M[1*4+3] - M[1*4+1]*M[0*4+3];
    s[5] = M[0*4+2]*M[1*4+3] - M[1*4+2]*M[0*4+3];

    c[0] = M[2*4+0]*M[3*4+1] - M[3*4+0]*M[2*4+1];
    c[1] = M[2*4+0]*M[3*4+2] - M[3*4+0]*M[2*4+2];
    c[2] = M[2*4+0]*M[3*4+3] - M[3*4+0]*M[2*4+3];
    c[3] = M[2*4+1]*M[3*4+2] - M[3*4+1]*M[2*4+2];
    c[4] = M[2*4+1]*M[3*4+3] - M[3*4+1]*M[2*4+3];
    c[5] = M[2*4+2]*M[3*4+3] - M[3*4+2]*M[2*4+3];

    return ( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );
}
bool invert44(mat44 T, const mat44 M) { // !!! ok, i guess
    float s[6], c[6];
    s[0] = M[0*4+0]*M[1*4+1] - M[1*4+0]*M[0*4+1];
    s[1] = M[0*4+0]*M[1*4+2] - M[1*4+0]*M[0*4+2];
    s[2] = M[0*4+0]*M[1*4+3] - M[1*4+0]*M[0*4+3];
    s[3] = M[0*4+1]*M[1*4+2] - M[1*4+1]*M[0*4+2];
    s[4] = M[0*4+1]*M[1*4+3] - M[1*4+1]*M[0*4+3];
    s[5] = M[0*4+2]*M[1*4+3] - M[1*4+2]*M[0*4+3];

    c[0] = M[2*4+0]*M[3*4+1] - M[3*4+0]*M[2*4+1];
    c[1] = M[2*4+0]*M[3*4+2] - M[3*4+0]*M[2*4+2];
    c[2] = M[2*4+0]*M[3*4+3] - M[3*4+0]*M[2*4+3];
    c[3] = M[2*4+1]*M[3*4+2] - M[3*4+1]*M[2*4+2];
    c[4] = M[2*4+1]*M[3*4+3] - M[3*4+1]*M[2*4+3];
    c[5] = M[2*4+2]*M[3*4+3] - M[3*4+2]*M[2*4+3];

    float det = ( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );
    if( !det ) return false;
    float idet = 1.0f / det;

    T[0*4+0] = ( M[1*4+1] * c[5] - M[1*4+2] * c[4] + M[1*4+3] * c[3]) * idet;
    T[0*4+1] = (-M[0*4+1] * c[5] + M[0*4+2] * c[4] - M[0*4+3] * c[3]) * idet;
    T[0*4+2] = ( M[3*4+1] * s[5] - M[3*4+2] * s[4] + M[3*4+3] * s[3]) * idet;
    T[0*4+3] = (-M[2*4+1] * s[5] + M[2*4+2] * s[4] - M[2*4+3] * s[3]) * idet;

    T[1*4+0] = (-M[1*4+0] * c[5] + M[1*4+2] * c[2] - M[1*4+3] * c[1]) * idet;
    T[1*4+1] = ( M[0*4+0] * c[5] - M[0*4+2] * c[2] + M[0*4+3] * c[1]) * idet;
    T[1*4+2] = (-M[3*4+0] * s[5] + M[3*4+2] * s[2] - M[3*4+3] * s[1]) * idet;
    T[1*4+3] = ( M[2*4+0] * s[5] - M[2*4+2] * s[2] + M[2*4+3] * s[1]) * idet;

    T[2*4+0] = ( M[1*4+0] * c[4] - M[1*4+1] * c[2] + M[1*4+3] * c[0]) * idet;
    T[2*4+1] = (-M[0*4+0] * c[4] + M[0*4+1] * c[2] - M[0*4+3] * c[0]) * idet;
    T[2*4+2] = ( M[3*4+0] * s[4] - M[3*4+1] * s[2] + M[3*4+3] * s[0]) * idet;
    T[2*4+3] = (-M[2*4+0] * s[4] + M[2*4+1] * s[2] - M[2*4+3] * s[0]) * idet;

    T[3*4+0] = (-M[1*4+0] * c[3] + M[1*4+1] * c[1] - M[1*4+2] * c[0]) * idet;
    T[3*4+1] = ( M[0*4+0] * c[3] - M[0*4+1] * c[1] + M[0*4+2] * c[0]) * idet;
    T[3*4+2] = (-M[3*4+0] * s[3] + M[3*4+1] * s[1] - M[3*4+2] * s[0]) * idet;
    T[3*4+3] = ( M[2*4+0] * s[3] - M[2*4+1] * s[1] + M[2*4+2] * s[0]) * idet;
    return true;
}

vec4 transform444(const mat44, const vec4);
bool unproject44(vec3 *out, vec3 xyd, vec4 viewport, mat44 mvp) {
    // xyd: usually x:mouse_x,y:window_height()-mouse_y,d:0=znear/1=zfar
    // src: https://www.khronos.org/opengl/wiki/GluProject_and_gluUnProject_code
    mat44 inv_mvp;
    if( invert44(inv_mvp, mvp) ) {
        vec4 in = vec4( (xyd.x-viewport.x)/viewport.z*2-1, (xyd.y-viewport.y)/viewport.w*2-1, 2*xyd.z-1, 1 );
        vec4 p = transform444(inv_mvp, in);
        if( p.w != 0 ) {
            p.w = 1.f/p.w;
            *out = vec3(p.x*p.w,p.y*p.w,p.z*p.w);
            return true;
        }
    }
    return false;
}

void compose44(mat44 m, vec3 t, quat q, vec3 s) {
#if 0
    // quat to rotation matrix
    m[0] = 1 - 2 * (q.y * q.y + q.z * q.z);
    m[1] = 2 * (q.x * q.y + q.z * q.w);
    m[2] = 2 * (q.x * q.z - q.y * q.w);

    m[4] = 2 * (q.x * q.y - q.z * q.w);
    m[5] = 1 - 2 * (q.x * q.x + q.z * q.z);
    m[6] = 2 * (q.y * q.z + q.x * q.w);

    m[8] = 2 * (q.x * q.z + q.y * q.w);
    m[9] = 2 * (q.y * q.z - q.x * q.w);
    m[10] = 1 - 2 * (q.x * q.x + q.y * q.y);

    // scale matrix
    m[0] *= s.x; m[4] *= s.x; m[8] *= s.x;
    m[1] *= s.y; m[5] *= s.y; m[9] *= s.y;
    m[2] *= s.z; m[6] *= s.z; m[10] *= s.z;

    // set translation
    m[12] = t.x; m[13] = t.y; m[14] = t.z;

    m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
#else
    rotationq44(m,q);
    scale44(m,s.x,s.y,s.z);
    relocate44(m,t.x,t.y,t.z);
//    relocate44(m,t.x,t.y,t.z); // ok?
//    scale44(m,s.x,s.y,s.z); // ok?

    m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
#endif
}

// ----------------------------------------------------------------------------

vec3 transform33(const mat33 m, vec3 p) {
    float x = (m[0] * p.x) + (m[4] * p.y) + (m[ 8] * p.z);
    float y = (m[1] * p.x) + (m[5] * p.y) + (m[ 9] * p.z);
    float z = (m[2] * p.x) + (m[6] * p.y) + (m[10] * p.z);
    return vec3(x,y,z);
}

vec4 transform444(const mat44 m, const vec4 p) {
    // remember w = 1 for move in space; w = 0 rotate in space;
    float x = m[0]*p.x + m[4]*p.y + m[ 8]*p.z + m[12]*p.w;
    float y = m[1]*p.x + m[5]*p.y + m[ 9]*p.z + m[13]*p.w;
    float z = m[2]*p.x + m[6]*p.y + m[10]*p.z + m[14]*p.w;
    float w = m[3]*p.x + m[7]*p.y + m[11]*p.z + m[15]*p.w;
    return vec4(x,y,z,w);
}

vec3 transform344(const mat44 m, const vec3 p) {
    vec4 v = transform444(m, vec34(p, 1));
    return scale3(v.xyz, 1.f / v.w);
}

vec3 transformq(const quat q, const vec3 v) {  // !!! ok, i guess
    // [src] https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion (laurent couvidou)
    // Extract the vector part of the quaternion
    vec3 u = vec3(q.x, q.y, q.z);
    // Extract the scalar part of the quaternion
    float s = q.w;
    // Do the math
    vec3 a = scale3(u, 2 * dot3(u,v));
    vec3 b = scale3(v, s*s - dot3(u,u));
    vec3 c = scale3(cross3(u,v), 2*s);
    return add3(a, add3(b,c));
}

#if 0
vec3 transform_axis(const coord_system, const AXIS_ENUMS);
void rebase44(mat44 m, const coord_system src_basis, const coord_system dst_basis) {
    vec3 v1 = transform_axis(src_basis, dst_basis.x);
    vec3 v2 = transform_axis(src_basis, dst_basis.y);
    vec3 v3 = transform_axis(src_basis, dst_basis.z);
    m[ 0] = v1.x; m[ 1] = v1.y; m[ 2] = v1.z; m[ 3] = 0;
    m[ 4] = v2.x; m[ 5] = v2.y; m[ 6] = v2.z; m[ 7] = 0;
    m[ 8] = v3.x; m[ 9] = v3.y; m[10] = v3.z; m[11] = 0;
    m[12] =    0; m[13] =    0; m[14] =    0; m[15] = 1;
}

vec3 transform_axis(const coord_system basis, const AXIS_ENUMS to) {
    const float dot_table[6][6] = {
        {+1,-1,0,0,0,0},{-1,+1,0,0,0,0},{0,0,+1,-1,0,0},
        {0,0,-1,+1,0,0},{0,0,0,0,+1,-1},{0,0,0,0,-1,+1},
    };
    return vec3( dot_table[basis.x][to], dot_table[basis.y][to], dot_table[basis.z][to] );
}

// A vector is the difference between two points in 3D space, possessing both direction and magnitude
vec3 transform_vector  (const mat44 m, const vec3 vector)   {
    return transform344(m, vector);
}

// A point is a specific location within a 3D space
vec3 transform_point   (const mat44 m, const vec3 p)    { // return (m * vec4{point,1).xyz()/r.w;
    float inv = 1.0f / (m[3+4*0]*p.x + m[3+4*1]*p.y + m[3+4*2]*p.z + m[3+4*3]);
    return vec3(
        (m[0+4*0]*p.x + m[0+4*1]*p.y + m[0+4*2]*p.z + m[0+4*3]) * inv,
        (m[1+4*0]*p.x + m[1+4*1]*p.y + m[1+4*2]*p.z + m[1+4*3]) * inv,
        (m[2+4*0]*p.x + m[2+4*1]*p.y + m[2+4*2]*p.z + m[2+4*3]) * inv
    );
}

// A tangent is a unit-length vector which is parallel to a piece of geometry, such as a surface or a curve
vec3 transform_tangent (const mat44 m, const vec3 tangent)  { return norm3(transform_vector(m, tangent)); }

// A normal is a unit-length bivector which is perpendicular to a piece of geometry, such as a surface or a curve
vec3 transform_normal  (const mat44 m, const vec3 normal)   {
    return transform_tangent(m, normal); // ok?

    mat44 t; transpose44(t,m); mat44 i; invert44(i,t);
    return scale3(norm3(transform_vector(i, normal)), det44(m) < 0 ? -1.f : 1.f);
}

// A quaternion can describe both a rotation and a uniform scaling in 3D space
quat transform_quat     (const mat44 m, const quat q)      {
    vec3 s = scale3(transform_vector(m, q.xyz), det44(m) < 0 ? -1.f : 1.f);
    return quat(s.x,s.y,s.z,q.w);
}

// A matrix can describe a general transformation of homogeneous coordinates in projective space
float* transform_matrix(mat44 out, const mat44 m, const mat44 matrix) {
    mat44 I; invert44(I, m);
    multiply44x3(out, I, matrix, m); // m,matrix,I instead ?
    return out;
}

// Scaling factors are not a vector, they are a compact representation of a scaling matrix
vec3 transform_scaling (const mat44 m, const vec3 scaling) {
    mat44 s; scaling44(s, scaling.x, scaling.y, scaling.z);
    mat44 out; transform_matrix(out, m, s);
    return vec3( out[0], out[5], out[10] );
}
#endif

// ----------------------------------------------------------------------------
// !!! for debugging

#include <stdio.h>
void print_( float *m, int ii, int jj ) {
    for( int j = 0; j < jj; ++j ) {
        for( int i = 0; i < ii; ++i ) printf("%8.3f", *m++);
        puts("");
    }
//    puts("---");
}
void print2( vec2 v ) { print_(&v.x,2,1); }
void print3( vec3 v ) { print_(&v.x,3,1); }
void print4( vec4 v ) { print_(&v.x,4,1); }
void printq( quat q ) { print_(&q.x,4,1); }
void print33( float *m ) { print_(m,3,3); }
void print34( float *m ) { print_(m,3,4); }
void print44( float *m ) { print_(m,4,4); }
#line 0

#line 1 "fwk_memory.c"
size_t dlmalloc_usable_size(void*); // __ANDROID_API__

#if is(bsd) || is(osx) // bsd or osx
#  include <malloc/malloc.h>
#else
#  include <malloc.h>
#endif

#ifndef SYS_REALLOC
#define SYS_REALLOC realloc
#define SYS_MSIZE /* bsd/osx, then win32, then ems/__GLIBC__, then __ANDROID_API__ */ \
    ifdef(osx, malloc_size, ifdef(bsd, malloc_size, \
        ifdef(win32, _msize, malloc_usable_size)))
#endif

// xrealloc --------------------------------------------------------------------

static __thread uint64_t xstats_current = 0, xstats_total = 0;

void* xrealloc(void* oldptr, size_t size) {
    // for stats
    size_t oldsize = xsize(oldptr);

    void *ptr = SYS_REALLOC(oldptr, size);
    if( !ptr && size ) {
        PANIC("Not memory enough (trying to allocate %u bytes)", (unsigned)size);
    }
#if ENABLE_MEMORY_POISON
    if( !oldptr && size ) {
        memset(ptr, 0xCD, size);
    }
#endif

    // for stats
    if( oldptr ) {
        xstats_current += (int64_t)size - (int64_t)oldsize;
    } else {
        xstats_current += size;
    }
    if( xstats_current > xstats_total ) {
        xstats_total = xstats_current;
    }

    return ptr;
}
size_t xsize(void* p) {
    if( p ) return SYS_MSIZE(p);
    return 0;
}
char *xstats(void) {
    return va("%03u/%03uMB", (unsigned)xstats_current / 1024 / 1024, (unsigned)xstats_total / 1024 / 1024);
}

// stack -----------------------------------------------------------------------

void* stack(int bytes) { // use negative bytes to rewind stack
    static __thread uint8_t *stack_mem = 0;
    static __thread uint64_t stack_ptr = 0;
    static __thread uint64_t stack_max = 0; // watch this var, in case you want to fine tune 4 MiB value below
    if( bytes < 0 ) {
        if( stack_ptr > stack_max ) stack_max = stack_ptr;
        return (stack_ptr = 0), NULL;
    }
    if( !stack_mem ) stack_mem = xrealloc(stack_mem, xsize(stack_mem) + 4 * 1024 * 1024);
    return &stack_mem[ (stack_ptr += bytes) - bytes ];
}

// leaks ----------------------------------------------------------------------

void* watch( void *ptr, int sz ) {
    static __thread int open = 1;
    if( ptr && open ) {
        open = 0;

        char buf[256];
        sprintf(buf, "%p.mem", ptr);
        for( FILE *fp = fopen(buf, "a+"); fp; fclose(fp), fp = 0 ) {
            fseek(fp, 0L, SEEK_END);
            const char *cs = callstack( +16 ); // +48
            fprintf(fp, "Built %s %s\n", __DATE__, __TIME__); // today() instead?
            fprintf(fp, "Memleak address: [%p], size: %d\n%s\n", ptr, sz, cs ? cs : "No callstack.");
        }

        open = 1;
    }
    return ptr;
}
void* forget( void *ptr ) {
    if( ptr ) {
        char buf[256];
        sprintf(buf, "%p.mem", ptr);
        unlink(buf);
    }
    return ptr;
}
#line 0

#line 1 "fwk_network.c"

#if is(tcc) && is(win32) // @fixme: https lib is broken with tcc. replaced with InternetReadFile() api for now

#   include <wininet.h>
#   pragma comment(lib,"wininet")

int download_file( FILE *out, const char *url ) {
    int ok = false;
    char buffer[ 4096 ];
    DWORD response_size = 0;

    if( out )
    for( HINTERNET session = InternetOpenA("fwk.download_file", PRE_CONFIG_INTERNET_ACCESS, NULL, INTERNET_INVALID_PORT_NUMBER, 0); session; InternetCloseHandle(session), session = 0 )
    for( HINTERNET request = InternetOpenUrlA(session, url, NULL, 0, INTERNET_FLAG_RELOAD, 0); request; InternetCloseHandle(request), request = 0 )
    for(; InternetReadFile(request, buffer, sizeof(buffer), &response_size) != FALSE && response_size > 0; ) {
        ok = (fwrite(buffer, response_size, 1, out) == 1);
        if(!ok) break;
    }

    return ok;
}

array(char) download( const char *url ) {
    char buffer[ 4096 ];
    DWORD response_size = 0, pos = 0;

    array(char) out = 0;
    for( HINTERNET session = InternetOpenA("fwk.download", PRE_CONFIG_INTERNET_ACCESS, NULL, INTERNET_INVALID_PORT_NUMBER, 0); session; InternetCloseHandle(session), session = 0 )
    for( HINTERNET request = InternetOpenUrlA(session, url, NULL, 0, INTERNET_FLAG_RELOAD, 0); request; InternetCloseHandle(request), request = 0 )
    for(; InternetReadFile(request, buffer, sizeof(buffer), &response_size) != FALSE && response_size > 0; ) {
        array_resize(out, pos + response_size);
        memcpy(out + (pos += response_size) - response_size, buffer, response_size);
    }

    return out;
}
#else
int download_file( FILE *out, const char *url ) {
    int ok = false;
    if( out ) for( https_t *h = https_get(url, NULL); h; https_release(h), h = NULL ) {
        while (https_process(h) == HTTPS_STATUS_PENDING) sleep_ms(1);
        //printf("fetch status%d, %d %s\n\n%.*s\n", https_process(h), h->status_code, h->content_type, (int)h->response_size, (char*)h->response_data);
        if(https_process(h) == HTTPS_STATUS_COMPLETED)
        ok = fwrite(h->response_data, h->response_size, 1, out) == 1;
    }
    return ok;
}

// @fixme: broken with tcc -m64 (our default tcc configuration)
array(char) download( const char *url ) {
    array(char) out = 0;
    for( https_t *h = https_get(url, NULL); h; https_release(h), h = NULL ) {
        while (https_process(h) == HTTPS_STATUS_PENDING) sleep_ms(1);
        //printf("fetch status:%d, %d %s\n\n%.*s\n", https_process(h), h->status_code, h->content_type, (int)h->response_size, (char*)h->response_data);
        if( https_process(h) == HTTPS_STATUS_COMPLETED ) {
            array_resize(out, h->response_size);
            memcpy(out, h->response_data, h->response_size);
        }
    }
    return out;
}
#endif

bool network_tests() {
    // network test (https)
    array(char) webfile = download("https://www.google.com/");
    printf("Network test: %d bytes downloaded from google.com\n", array_count(webfile));
    // array_push(webfile, '\0'); puts(webfile);
    return true;
}

int portname( const char *service_name, unsigned retries ) {
    // Determine port for a given service based on hash of its name.
    // If port cant be reached, client should retry with next hash.
    // Algorithm: fnv1a(name of service) -> splitmix64 num retries -> remap bucket as [min..max] ports.

    // hash64
    uint64_t hash = 14695981039346656037ULL;
    while( *service_name ) {
        hash = ( (unsigned char)*service_name++ ^ hash ) * 0x100000001b3ULL;
    }

    // splitmix64
    for( unsigned i = 0; i < retries; ++i ) {
        uint64_t h = (hash += UINT64_C(0x9E3779B97F4A7C15));
        h = (h ^ (h >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
        h = (h ^ (h >> 27)) * UINT64_C(0x94D049BB133111EB);
        h = (h ^ (h >> 31));
        hash = h;
    }

    // See dynamic ports: https://en.wikipedia.org/wiki/Ephemeral_port
    // So, excluded ranges: 32768..60999 (linux), 49152..65535 (freebsd+vista+win7), 1024..5000 (winsrv2003+bsd)
    // Output range: [5001..32724], in 4096 steps
    return ((hash & 0xFFF) * 677 / 100 + 5001);
}

static
void netdump( const void *ptr, int len ) {
    char hexbuf[256] = {0}, strbuf[256] = {0}, *data = (char*)ptr, width = 16;
    for( int jt = 0; jt < len; jt += width ) {
        char *hex = hexbuf, *str = strbuf;
        for( int it = jt, next = it + width; it < len && it < next; ++it, ++data ) {
            hex += sprintf( hex, "%02x ", (unsigned char)*data);
            str += sprintf( str, "%c", *data >= 32 && *data != '\\' ? *data : '.');
        }
        printf("%06x %-*s%s\n", jt, width*3, hexbuf, strbuf);
    }
}

// -----------------------------------------------------------------------------

#define UDP_DEBUG 0

static int udp_init() {
    do_once {
        int rc = swrapInit(); // atexit(swrapTerminate);
        if( rc ) PANIC("udp_init: swrapInit error");
    }
    return 1;
}

int udp_open(const char *address, const char *port) {
    do_once udp_init();
    int fd = swrapSocket(SWRAP_UDP, SWRAP_CONNECT, 0, address, port);
    // if( fd == -1 ) PANIC("udp_open: swrapSocket error");
    return fd;
}

int udp_bind(const char *address, const char *port) {
    do_once udp_init();
    int fd = swrapSocket(SWRAP_UDP, SWRAP_BIND, 0, address, port);
    // if( fd == -1 ) PANIC("udp_bind: swrapSocket error");
    return fd;
}

int udp_send( int fd, const void *buf, int len ) { // returns bytes sent, or -1 if error
    int rc = -1;
    if( fd >= 0 ) for( ;; ) {
        rc = swrapSend(fd, (const char *)buf, len);
#if is(win32)
        if( rc == -1 && WSAGetLastError() == WSAEINTR ) continue;
        else break;
#else
        if( rc == -1 && errno == EINTR ) continue;
        else break;
#endif
    }
#if UDP_DEBUG
    if( rc > 0 ) {
        char host[128], serv[128];
        int rc2 = swrapAddressInfo(&sa, host, 128, serv, 128 );
        if( rc2 != 0 ) PANIC("swrapAddressInfo error");
        printf("udp_send: %d bytes to %s:%s : %.*s\n", rc, host, serv, rc, buf );
        netdump(buf, rc);
    }
#endif
    return rc;
}

int udp_close( int fd ) { // @todo: expose me? needed?
#if is(win32)
    // closesocket(fd);
#else
    // close(fd);
#endif
    fd = -1; // noop
    return 0;
}

#if 0
// use socket to send data to another address
int udp_sendto( int fd, const char *ip, const char *port, const void *buf, int len ) { // return number of bytes sent
#if 0
    int rc = swrapSendTo(fd, struct swrap_addr*, (const char*)buf, len);
    if( rc == -1 ) return -1; //PANIC("udp_send: swrapSend error");
    return rc;
#else
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    // use inet_addr. tcc(win32) wont work otherwise.
    addr.sin_addr.s_addr = inet_addr(ip); // inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(atoi(port));

    int n = sendto(fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
    return n < 0 ? -1 : n;
#endif
}
#endif

int udp_peek( int fd ) { // <0 error, 0 timeout, >0 data
    int rc = swrapSelect(fd, 0.00001);
    if( rc < 0 ) return -1; // PANIC("udp_peek: swrapSelect error");
    if( rc == 0 ) return 0; // timeout
    return 1; //> 0: new data is available
}

int udp_recv( int fd, void *buf, int len ) { // <0 error, 0 orderly shutdown, >0 received bytes
    struct swrap_addr sa = {0};
    int rc = swrapReceiveFrom(fd, &sa, buf, len);
    if( rc < 0 ) return -1; // PANIC("udp_recv: swrapReceiveFrom error");
    if( rc == 0 ) return 0; // orderly shutdown

#if UDP_DEBUG
    char host[128], serv[128];
    int rc2 = swrapAddressInfo(&sa, host, 128, serv, 128 );
    if( rc2 != 0 ) PANIC("swrapAddressInfo error");
    printf("udp_recv: %d bytes from %s:%s : %.*s\n", rc, host, serv, rc, buf );
    netdump(buf, rc);
#endif

    return rc;
}

// -----------------------------------------------------------------------------

#define TCP_DEBUG 1

#if TCP_DEBUG
static set(int) tcp_set;
#endif

void tcp_init(void) {
    do_once {
        udp_init();
#if TCP_DEBUG
        set_init(tcp_set, less_int, hash_int);
#endif
    }
}
int tcp_open(const char *address, const char *port) {
    do_once tcp_init();
    int fd = swrapSocket(SWRAP_TCP, SWRAP_CONNECT, 0/*|SWRAP_NODELAY*/, address, port);
    return fd;
}
int tcp_bind(const char *interface_, const char *port, int backlog) {
    do_once tcp_init();
    int fd = swrapSocket(SWRAP_TCP, SWRAP_BIND, 0/*|SWRAP_NODELAY*//*|SWRAP_NOBLOCK*/, interface_, port);
    if( fd >= 0 ) swrapListen(fd, backlog);
    return fd;
}
int tcp_peek(int fd, int(*callback)(int)) {
    struct swrap_addr sa;
    int fd2 = swrapAccept(fd, &sa);
    if( fd2 >= 0 ) return callback(fd2);
    return -1;
}
int tcp_send(int fd, const void *buf, int len) {
    int rc = swrapSend(fd, (const char *)buf, len);
#if TCP_DEBUG
    if( set_find(tcp_set, fd) ) {
        printf("send -> %11d (status: %d) %s:%s\n", len, rc, tcp_host(fd), tcp_port(fd));
        if( rc > 0 ) netdump(buf, rc);
    }
#endif
    return rc;
}
int tcp_recv(int fd, void *buf, int len) {
    int rc = swrapReceive(fd, (char*)buf, len);
#if TCP_DEBUG
    if( rc != 0 && set_find(tcp_set, fd) ) {
        printf("recv <- %11d (status: %d) %s:%s\n", len, rc, tcp_host(fd), tcp_port(fd));
        if( rc > 0 ) netdump(buf, rc);
    }
#endif
    return rc;
}
char* tcp_host(int fd) {
    char buf[1024];
    struct swrap_addr sa;
    swrapAddress(fd, &sa);
    swrapAddressInfo(&sa, buf, 512, buf+512, 512);
    return va("%s", buf);
}
char* tcp_port(int fd) {
    char buf[1024];
    struct swrap_addr sa;
    swrapAddress(fd, &sa);
    swrapAddressInfo(&sa, buf, 512, buf+512, 512);
    return va("%s", buf+512);
}
int tcp_close(int fd) {
    swrapClose(fd);
    return 0;
}
int tcp_debug(int fd) {
#if TCP_DEBUG
    if( set_find(tcp_set, fd) ) {
        set_erase(tcp_set, fd);
        return 0;
    } else {
        set_insert(tcp_set, fd);
        return 1;
    }
#else
    return 0;
#endif
}

// -----------------------------------------------------------------------------

static void network_init() {
    do_once {
        udp_init();
        tcp_init();
    }
}
#line 0

#line 1 "fwk_netsync.c"
typedef void* (*rpc_function)();

typedef struct rpc_call {
    char *method;
    rpc_function function;
    uint64_t function_hash;
} rpc_call;

#define RPC_SIGNATURE_i_iii    UINT64_C(0x78409099752fa48a)
#define RPC_SIGNATURE_i_ii     UINT64_C(0x258290edf43985a5)
#define RPC_SIGNATURE_i_s      UINT64_C(0xf7b73162829ed667)
#define RPC_SIGNATURE_s_s      UINT64_C(0x97deedd17d9afb12)
#define RPC_SIGNATURE_s_v      UINT64_C(0x09c16a1242049b80)
#define RPC_SIGNATURE_v_s      UINT64_C(0xc1746990ab73ed24)

static
rpc_call rpc_new_call(const char *signature, rpc_function function) {
    if( signature && function ) {
        array(char*)tokens = strsplit(signature, "(,)");
        if( array_count(tokens) >= 1 ) {
            char *method = strrchr(tokens[0], ' ')+1;
            char *rettype = va("%.*s", (int)(method - tokens[0] - 1), tokens[0]);
            int num_args = array_count(tokens) - 1;
            char* hash_sig = va("%s(%s)", rettype, num_args ? (array_pop_front(tokens), strjoin(tokens, ",")) : "void");
            uint64_t hash = hash_str(hash_sig);
            method = va("%s%d", method, num_args );
#if RPC_DEBUG
            printf("%p %p %s `%s` %s(", function, (void*)hash, rettype, hash_sig, method); for(int i = 0, end = array_count(tokens); i < end; ++i) printf("%s%s", tokens[i], i == (end-1)? "":", "); puts(");");
#endif
            return (rpc_call) { strdup(method), function, hash }; // LEAK
        }
    }
    return (rpc_call) {0};
}

static map(char*, rpc_call) rpc_calls = 0;

static
void rpc_insert(const char *signature, void *function ) {
    rpc_call call = rpc_new_call(signature, function);
    if( call.method ) {
        if( !rpc_calls ) map_init(rpc_calls, less_str, hash_str);
        if( map_find(rpc_calls, call.method)) {
            map_erase(rpc_calls, call.method);
        }
        map_insert(rpc_calls, call.method, call);
    }
}

static
char *rpc_full(unsigned id, const char* method, unsigned num_args, char *args[]) {
#if RPC_DEBUG
    printf("id:%x method:%s args:", id, method );
    for( int i = 0; i < num_args; ++i ) printf("%s,", args[i]); puts("");
#endif

    method = va("%s%d", method, num_args);
    rpc_call *found = map_find(rpc_calls, (char*)method);
    if( found ) {
        switch(found->function_hash) {
            case RPC_SIGNATURE_i_iii: return va("%d %d", id, (int)(intptr_t)found->function(atoi(args[0]), atoi(args[1]), atoi(args[2])) );
            case RPC_SIGNATURE_i_ii:  return va("%d %d", id, (int)(intptr_t)found->function(atoi(args[0]), atoi(args[1])) );
            case RPC_SIGNATURE_i_s:   return va("%d %d", id, (int)(intptr_t)found->function(args[0]) );
            case RPC_SIGNATURE_s_s:   return va("%d %s", id, (char*)found->function(args[0]) );
            case RPC_SIGNATURE_s_v:   return va("%d %s", id, (char*)found->function() );
            case RPC_SIGNATURE_v_s:   return found->function(args[0]), va("%d", id);
            default: break;
        }
    }
    return va("%d -1", id);
}

static
array(char*) rpc_parse_args( const char *cmdline, bool quote_whitespaces ) { // parse cmdline arguments. must array_free() after use
    // - supports quotes: "abc" "abc def" "abc \"def\"" "abc \"def\"""ghi" etc.
    // - #comments removed
    array(char*) args = 0; // LEAK
    for( int i = 0; cmdline[i]; ) {
        char buf[256] = {0}, *ptr = buf;
        while(cmdline[i] && isspace(cmdline[i])) ++i;
        bool quoted = cmdline[i] == '\"';
        if( quoted ) {
            while(cmdline[++i]) {
                char ch = cmdline[i];
                /**/ if (ch == '\\' && cmdline[i + 1] == '\"') *ptr++ = '\"', ++i;
                else if (ch == '\"' && cmdline[i + 1] == '\"') ++i;
                else if (ch == '\"' && (!cmdline[i + 1] || isspace(cmdline[i + 1]))) {
                    ++i; break;
                }
                else *ptr++ = ch;
            }
        } else {
            while(cmdline[i] && !isspace(cmdline[i])) *ptr++ = cmdline[i++];
        }
        if (buf[0] && buf[0] != '#') { // exclude empty args + comments
            if( quote_whitespaces && quoted )
            array_push(args, va("\"%s\"",buf));
            else
            array_push(args, va("%s",buf));
        }
    }
    return args;
}

static
char* rpc(unsigned id, const char* cmdline) {
    array(char*) args = rpc_parse_args(cmdline, false);
    int num_args = array_count(args);
    char *ret = num_args ? rpc_full(id, args[0], num_args - 1, &args[1]) : rpc_full(id, "", 0, NULL);
    array_free(args);
    return ret;
}

static void enet_quit(void) {
    do_once {
        // enet_deinitialize();
    }
}
static void enet_init() {
    do_once {
        if( enet_initialize() != 0 ) {
            PANIC("cannot initialize enet");
        }
        atexit( enet_quit );
    }
}

struct peer_node_t {
    int64_t id;
    struct peer_node_t *next;
};

static ENetHost *Server;
static map(ENetPeer *, int64_t) clients;
static map(int64_t, ENetPeer *) peers;
static int64_t next_client_id = 1; // assumes ID 0 is server
static struct peer_node_t *next_free_id = NULL;
enum { MSG_INIT, MSG_BUF, MSG_RPC, MSG_RPC_RESP };

bool server_bind(int max_clients, int port) {
    map_init(clients, less_64, hash_64);
    map_init(peers, less_64, hash_64);
    assert(port == 0 || (port > 1024 && port < 65500));
    ENetAddress address = {0};
    address.host = ENET_HOST_ANY;
    address.port = port;
    Server = enet_host_create(&address, max_clients, 2 /*channels*/, 0 /*in bandwidth*/, 0 /*out bandwidth*/);
    return Server != NULL;
}

static
void server_drop_client(int64_t handle) {
    map_erase(clients, *(ENetPeer **)map_find(peers, handle));
    map_erase(peers, *(int64_t *)handle);
}

static
void server_drop_client_peer(ENetPeer *peer) {
    struct peer_node_t *node = C_CAST(struct peer_node_t *, CALLOC(sizeof(struct peer_node_t), 1));
    node->id = *(int64_t *)map_find(clients, peer);
    if (!next_free_id) {
        next_free_id = node;
    } else {
        node->next = next_free_id;
        next_free_id = node;
    }
    map_erase(peers, *(int64_t *)map_find(clients, peer));
    map_erase(clients, peer);
}

void server_broadcast_bin_flags(const void *msg, int len, uint64_t flags) {
    ENetPacket *packet = enet_packet_create(msg, len, flags&NETWORK_UNRELIABLE ? ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT : ENET_PACKET_FLAG_RELIABLE | flags&(NETWORK_UNRELIABLE|NETWORK_UNORDERED) ? ENET_PACKET_FLAG_UNSEQUENCED : 0);
    enet_host_broadcast(Server, 0, packet);
}

void server_broadcast_bin(const void *msg, int len) {
    ENetPacket *packet = enet_packet_create(msg, len, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(Server, 0, packet);
    //enet_host_flush(Server); // flush if needed
}
void server_broadcast_flags(const char *msg, uint64_t flags) {
    server_broadcast_bin_flags(msg, strlen(msg)+1, flags);
}
void server_broadcast(const char *msg) {
    server_broadcast_bin(msg, strlen(msg)+1);
}
void server_terminate() {
    enet_host_destroy(Server);
    Server = 0;
}

volatile int client_join_connected = 0;
static int client_join_threaded(void *userdata) {
    ENetHost *host = (ENetHost *)userdata;

    ENetPacket *packet = enet_packet_create("", 1, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(Server, 0, packet);

    /* Wait up to 5 seconds for the connection attempt to succeed. */
    ENetEvent event;
    client_join_connected = 0;
    client_join_connected = enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT;
    return 0;
}

int64_t client_join(const char *ip, int port) {
    assert(port > 1024 && port < 65500);
    ENetAddress address = {0};
    enet_address_set_host(&address, !strcmp(ip, "localhost") ? "127.0.0.1" : ip);
    address.port = port;

    ENetHost *host = enet_host_create(NULL, 1 /*outgoing connections*/, 2 /*channels*/, 0 /*in bandwidth*/, 0 /*out bandwidth*/);
    if(!host) return -1;
    ENetPeer *peer = enet_host_connect(host, &address, 2, 0);
    if(!peer) return -1;
    Server = host;

    ENetEvent event;
    bool client_join_connected = enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT;
    if(!client_join_connected) { enet_peer_reset(peer); return -1; }

    // wait for the response
    bool msg_received = enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE;
    if (!msg_received) { enet_peer_reset(peer); return -1; }

    char *ptr = (char *)event.packet->data;
    int64_t cid = -1;

    // decapsulate incoming packet.
    uint32_t mid = *(uint32_t*)(ptr + 0);
    ptr += 4;

    switch (mid) {
    case MSG_INIT:
        cid = *(int64_t*)ptr;
        break;
    default:
        enet_peer_reset(peer);
        enet_packet_destroy( event.packet );
        return -1;
    }

    /* Clean up the packet now that we're done using it. */
    enet_packet_destroy( event.packet );

    return cid;
}
void server_drop(int64_t handle) {
    enet_peer_disconnect_now(*(ENetPeer **)map_find(peers, handle), 0);
    server_drop_client(handle);
}

void server_send_bin(int64_t handle, const void *ptr, int len) {
    ENetPacket *packet = enet_packet_create(ptr, len, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(*(ENetPeer **)map_find(peers, handle), 0, packet);
}

void server_send(int64_t handle, const char *msg) {
    server_send_bin(handle, msg, strlen(msg)+1);
}

// ---

typedef struct netbuffer_t {
    int64_t owner;
    void *ptr;
    unsigned sz;
    uint64_t flags;
} netbuffer_t;

static array(char*) events; // @todo: make event 128 bytes max?
static array(int64_t) values; // @todo: map<key,values> instead?
static map( int64_t, array(netbuffer_t) ) buffers; // map<client,array<netbuffer>>
static double msg_send_cooldown = 0.0;
static double network_dt = 0.0;
static double last_netsync = 0.0;

void network_create(unsigned max_clients, const char *ip, const char *port_, unsigned flags) {
    if (buffers) map_clear(buffers);
    do_once {
        array_resize(values, 128);
        map_init(buffers, less_64, hash_64);

        enet_init();
    }

    ip = ip ? ip : "0.0.0.0";
    int port = atoi(port_ ? port_ : "1234");

    // network_put(NETWORK_IP, 0x7F000001); // 127.0.0.1
    network_put(NETWORK_PORT, port);
    network_put(NETWORK_LIVE, -1);
    network_put(NETWORK_COUNT, 0);
    network_put(NETWORK_CAPACITY, max_clients);
    network_put(NETWORK_BUF_CLEAR_ON_JOIN, 1);

    if( !(flags&NETWORK_CONNECT) || flags&NETWORK_BIND ) {
        // server, else client
        PRINTF("Trying to bind server, else we connect as a client...\n");
        network_put(NETWORK_RANK, 0);
        if( server_bind(max_clients, port) ) {
            network_put(NETWORK_LIVE, 1);
            PRINTF("Server bound\n");
        } else {
            network_put(NETWORK_RANK, -1); /* unassigned until we connect successfully */
            int64_t socket = client_join(ip, port);
            if( socket >= 0 ) {
                PRINTF("Client connected, id %lld\n", socket);
                network_put(NETWORK_LIVE, 1);
                network_put(NETWORK_RANK, socket);
            } else {
                PRINTF("!Client conn failed\n");
                network_put(NETWORK_LIVE, 0);

                if (!(flags&NETWORK_NOFAIL))
                    PANIC("cannot neither connect to %s:%d, nor create a server", ip, port);
            }
        }
    } else {
        // client only
        PRINTF("Connecting to server...\n");
        network_put(NETWORK_RANK, -1); /* unassigned until we connect successfully */
        int64_t socket = client_join(ip, port);
        if( socket > 0 ) {
            PRINTF("Client connected, id %lld\n", socket);
            network_put(NETWORK_LIVE, 1);
            network_put(NETWORK_RANK, socket);
        } else {
            PRINTF("!Client conn failed\n");
            network_put(NETWORK_LIVE, 0);
            if (!(flags&NETWORK_NOFAIL))
                PANIC("cannot connect to server %s:%d", ip, port);
        }
    }

    PRINTF("Network rank:%lld ip:%s port:%lld\n", network_get(NETWORK_RANK), ip, network_get(NETWORK_PORT));
}

int64_t network_put(uint64_t key, int64_t value) {
    int64_t *found = key < array_count(values) ? &values[key] : NULL;
    if(found) *found = value;
    return value;
}
int64_t network_get(uint64_t key) {
    int64_t *found = key < array_count(values) ? &values[key] : NULL;
    return found ? *found : 0;
}

void* network_buffer(void *ptr, unsigned sz, uint64_t flags, int64_t rank) {
    assert(flags);
    array(netbuffer_t) *found = map_find_or_add(buffers, rank, NULL);

    netbuffer_t nb;
    nb.owner = rank;
    nb.ptr = ptr;
    nb.sz = sz;
    nb.flags = flags;
    array_push(*found, nb);

    return ptr;
}

static
int enet_event_to_netsync(int ev) {
    switch (ev) {
        case ENET_EVENT_TYPE_CONNECT: return NETWORK_EVENT_CONNECT;
        case ENET_EVENT_TYPE_DISCONNECT: return NETWORK_EVENT_DISCONNECT;
        case ENET_EVENT_TYPE_RECEIVE: return NETWORK_EVENT_RECEIVE;
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: return NETWORK_EVENT_DISCONNECT_TIMEOUT;
    }

    /* passthrough for our own events */
    return ev;
}

char** network_sync(unsigned timeout_ms) {
    int64_t whoami = network_get(NETWORK_RANK);
    bool is_server = whoami == 0;
    bool is_client = !is_server;
    if(timeout_ms < 2) timeout_ms = 2;

    network_dt = time_ss() - last_netsync;
    last_netsync = time_ss();

    // Split buffers into clients @todo
    // clients need to do this before network polling; servers should do this after polling.
    if (msg_send_cooldown <= 0.0) {
        map_foreach(buffers, int64_t, rank, array(netbuffer_t), list) {
            for(int i = 0, end = array_count(list); i < end; ++i) {
                netbuffer_t *nb = &list[i];
                if (!is_server && !(nb->flags & NETWORK_SEND))
                    continue;
                static array(char) encapsulate;
                array_resize(encapsulate, nb->sz + 28);
                uint32_t *mid = (uint32_t*)&encapsulate[0]; *mid = MSG_BUF;
                uint64_t *st = (uint64_t*)&encapsulate[4]; *st = nb->flags;
                uint32_t *idx = (uint32_t*)&encapsulate[12]; *idx = i;
                uint32_t *len = (uint32_t*)&encapsulate[16]; *len = nb->sz;
                uint64_t *who = (uint64_t*)&encapsulate[20]; *who = nb->owner;
                // PRINTF("sending %llx %u %lld %u\n", *st, *idx, *who, *len);
                memcpy(&encapsulate[28], nb->ptr, nb->sz);
                server_broadcast_bin(&encapsulate[0], nb->sz + 28);
            }
        }
        msg_send_cooldown = (double)network_get(NETWORK_SEND_MS)/1000.0;
    } else {
        msg_send_cooldown -= network_dt;
    }

    if (is_server) {
        return server_poll(timeout_ms);
    } else {
        return client_poll(timeout_ms);
    }
}


char** server_poll(unsigned timeout_ms) {
    array_clear(events);
    if(timeout_ms < 2) timeout_ms = 2;

    // network poll
    for( ENetEvent event; Server && enet_host_service(Server, &event, timeout_ms) > 0; ) {
        char *msg = 0;
        char ip[128]; enet_peer_get_ip(event.peer, ip, 128);

        switch (event.type) {
            default: // case ENET_EVENT_TYPE_NONE:
                break;

            case ENET_EVENT_TYPE_CONNECT:;
                /* ensure we have free slot for client */
                if (map_count(clients) >= network_get(NETWORK_CAPACITY)-1) {
                    msg = va("%d Server is at maximum capacity, disconnecting the peer (::%s:%u)...", 1, ip, event.peer->address.port);
                    enet_peer_disconnect_now(event.peer, 1);
                    break;
                }

                int64_t client_id = -1;

                if (next_free_id) {
                    struct peer_node_t *node = next_free_id;
                    client_id = next_free_id->id;
                    next_free_id = next_free_id->next;
                    FREE(node);
                }
                else client_id = next_client_id++;

                // if (network_get(NETWORK_BUF_CLEAR_ON_JOIN)) {
                //     array(netbuffer_t) *list = map_find(buffers, client_id);

                //     if (list)
                //     for(int i = 0, end = array_count(list); i < end; ++i) {
                //         netbuffer_t *nb = &list[i];
                //         memset(nb->ptr, 0, nb->sz);
                //     }
                // }

                map_find_or_add(clients, event.peer, client_id);
                map_find_or_add(peers, client_id, event.peer);
                network_put(NETWORK_COUNT, network_get(NETWORK_COUNT)+1);


                // send server slot
                char init_msg[12];
                *(uint32_t*)&init_msg[0] = MSG_INIT;
                *(int64_t*)&init_msg[4] = client_id;
                server_send_bin(client_id, init_msg, 12);
                PRINTF("Client rank %lld for peer ::%s:%u\n", client_id, ip, event.peer->address.port);
                msg = va( "%d new client rank:%lld from ::%s:%u", 0, client_id, ip, event.peer->address.port );
                event.peer->data = (void*)client_id;
                break;

            case ENET_EVENT_TYPE_RECEIVE: {
                char *dbg = (char *)event.peer->data;
                char *ptr = (char *)event.packet->data;
                unsigned sz = (unsigned)event.packet->dataLength;
                unsigned id = (unsigned)event.channelID;

                // debug
                // puts(dbg);
                // hexdump(ptr, sz);

                // decapsulate incoming packet.
                uint32_t mid = *(uint32_t*)(ptr + 0);
                ptr += 4;

                switch (mid) {
                case MSG_BUF: {
                    uint64_t *flags = (uint64_t*)(ptr + 0);
                    uint32_t *idx = (uint32_t*)(ptr + 8);
                    uint32_t *len = (uint32_t*)(ptr + 12);
                    uint64_t *who = (uint64_t*)(ptr + 16);
                    ptr += 24;

                    // validate if peer owns the buffer
                    int64_t *cid = map_find(clients, event.peer);
                    uint8_t client_valid = cid ? *cid == *who : 0;

                    // apply incoming packet.
                    if( client_valid ) {
                        array(netbuffer_t) *list = map_find(buffers, *who);
                        assert( list );
                        assert( *idx < array_count(*list) );
                        netbuffer_t *nb = &(*list)[*idx];
                        assert( *len == nb->sz );
                        memcpy(nb->ptr, ptr, *len);
                    }
                } break;
                case MSG_RPC: {
                    event.type = NETWORK_EVENT_RPC; 
                    unsigned id = *(uint32_t*)ptr; ptr += 4;
                    char *cmdline = ptr;
                    char *resp = rpc(id, cmdline);
                    char *resp_msg = va("%*.s%s", 4, "", resp);
                    *(uint32_t*)&resp_msg[0] = MSG_RPC_RESP;
                    ENetPacket *packet = enet_packet_create(resp_msg, strlen(resp) + 5, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                    msg = va("%d req:%s res:%s", 0, cmdline, resp);
                } break;
                case MSG_RPC_RESP: {
                    event.type = NETWORK_EVENT_RPC_RESP; 
                    msg = va("%d %s", 0, va("%s", ptr));
                } break;
                default:
                    msg = va("%d unk msg len:%u from rank:%lld ::%s:%u", -1, sz, (uint64_t)event.peer->data, ip, event.peer->address.port); /* @TODO: hexdump? */
                    break;
                }
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy( event.packet );
            } break;

            case ENET_EVENT_TYPE_DISCONNECT:
                msg = va( "%d disconnect rank:%lld", 0, (uint64_t)event.peer->data);
                /* Reset the peer's client information. */
                FREE(event.peer->data);
                event.peer->data = NULL;
                server_drop_client_peer(event.peer);
                network_put(NETWORK_COUNT, network_get(NETWORK_COUNT)-1);
                break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                msg = va( "%d timeout rank:%lld", 0, (uint64_t)event.peer->data);
                FREE(event.peer->data);
                event.peer->data = NULL;
                server_drop_client_peer(event.peer);
                network_put(NETWORK_COUNT, network_get(NETWORK_COUNT)-1);
                break;
        }

        if(msg) array_push(events, va("%d %s", enet_event_to_netsync(event.type), msg));
    }

    array_push(events, NULL);
    return events;
}

char** client_poll(unsigned timeout_ms) {
    array_clear(events);
    int64_t whoami = network_get(NETWORK_RANK);
    if(timeout_ms < 2) timeout_ms = 2;

    // network poll
    for( ENetEvent event; Server && enet_host_service(Server, &event, timeout_ms) > 0; ) {
        char *msg = 0;
        char ip[128]; enet_peer_get_ip(event.peer, ip, 128);

        switch (event.type) {
            default: // case ENET_EVENT_TYPE_NONE:
                break;

            case ENET_EVENT_TYPE_CONNECT:
                break;

            case ENET_EVENT_TYPE_RECEIVE: {
                char *dbg = (char *)event.peer->data;
                char *ptr = (char *)event.packet->data;
                unsigned sz = (unsigned)event.packet->dataLength;
                unsigned id = (unsigned)event.channelID;

                // decapsulate incoming packet.
                uint32_t mid = *(uint32_t*)(ptr + 0);
                ptr += 4;

                switch (mid) {
                case MSG_INIT:
                    /* handled by client_join */
                    break;
                case MSG_BUF: {
                    uint64_t *flags = (uint64_t*)(ptr + 0);
                    uint32_t *idx = (uint32_t*)(ptr + 8);
                    uint32_t *len = (uint32_t*)(ptr + 12);
                    uint64_t *who = (uint64_t*)(ptr + 16);
                    ptr += 24;

                    // apply incoming packet.
                    if( *who != whoami ) {
                        array(netbuffer_t) *list = map_find(buffers, *who);
                        assert( list );
                        assert( *idx < array_count(*list) );
                        netbuffer_t *nb = &(*list)[*idx];
                        assert( *len == nb->sz );
                        memcpy(nb->ptr, ptr, *len);
                    }
                } break;
                case MSG_RPC: {
                    event.type = NETWORK_EVENT_RPC; 
                    unsigned id = *(uint32_t*)ptr; ptr += 4;
                    char *cmdline = ptr;
                    char *resp = rpc(id, cmdline);
                    char *resp_msg = va("%*.s%s", 4, "", resp);
                    *(uint32_t*)&resp_msg[0] = MSG_RPC_RESP;
                    ENetPacket *packet = enet_packet_create(resp_msg, strlen(resp) + 5, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                    msg = va("%d req:%s res:%s", 0, cmdline, resp);
                } break;
                case MSG_RPC_RESP: {
                    event.type = NETWORK_EVENT_RPC_RESP; 
                    msg = va("%d %s", 0, ptr);
                } break;
                default:
                    msg = va("%d unk msg len:%u from server", -1, sz); /* @TODO: hexdump? */
                    break;
                }
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy( event.packet );
            } break;

            case ENET_EVENT_TYPE_DISCONNECT:
                msg = va( "%d disconnect", 0 );
                /* Reset the peer's client information. */
                FREE(event.peer->data);
                event.peer->data = NULL;
                network_put(NETWORK_RANK, -1);
                network_put(NETWORK_LIVE, 0);
                break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                msg = va( "%d timeout", 0);
                FREE(event.peer->data);
                event.peer->data = NULL;
                network_put(NETWORK_RANK, -1); 
                network_put(NETWORK_LIVE, 0);
                break;
        }

        if(msg) array_push(events, va("%d %s", enet_event_to_netsync(event.type), msg));
    }

    array_push(events, NULL);
    return events;
}

int network_event(const char *msg, int *errcode, char **errstr) {
    int evid = -1;
    int err = 0;
    char errbuf[128] = {0};
    sscanf(msg, "%d %d %127[^\r\n]", &evid, &err, errbuf);
    if (errcode) *errcode = err;
    if (errstr) *errstr = va("%s", errbuf);
    return evid;
}

void network_rpc(const char *signature, void *function) {
    rpc_insert(signature, function);
}

void network_rpc_send_to(int64_t rank, unsigned id, const char *cmdline) {
    assert(network_get(NETWORK_RANK) == 0); /* must be a host */
    char *msg = va("%*.s%s", 8, "", cmdline);
    unsigned sz = strlen(cmdline) + 9;
    *(uint32_t*)&msg[0] = MSG_RPC;
    *(uint32_t*)&msg[4] = id;
    server_send_bin(rank, msg, sz);
}

void network_rpc_send(unsigned id, const char *cmdline) {
    char *msg = va("%*.s%s", 8, "", cmdline);
    unsigned sz = strlen(cmdline) + 9;
    *(uint32_t*)&msg[0] = MSG_RPC;
    *(uint32_t*)&msg[4] = id;
    server_broadcast_bin(msg, sz);
}
#line 0

#line 1 "fwk_render.c"
// -----------------------------------------------------------------------------
// opengl

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3

#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_TYPE_ERROR               0x824C
//
void glDebugCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char * message, void * userdata) {
    // whitelisted codes (also: 131169, 131204).
    if( id == 131154 ) return; // Pixel-path performance warning: Pixel transfer is synchronized with 3D rendering.
    if( id == 131185 ) return; // Buffer object 2 (bound to GL_ELEMENT_ARRAY_BUFFER_ARB, usage hint is GL_STATIC_DRAW) will use VIDEO memory as the source for buffer object operations
    if( id == 131218 ) return; // Program/shader state performance warning: Vertex shader in program 9 is being recompiled based on GL state.
    if( id == 2 ) return; // INFO: API_ID_RECOMPILE_FRAGMENT_SHADER performance warning has been generated. Fragment shader recompiled due to state change. [ID: 2]

    const char * GL_ERROR_TYPE[] = { "ERROR", "DEPRECATED BEHAVIOR", "UNDEFINED DEHAVIOUR", "PORTABILITY", "PERFORMANCE", "OTHER" };
    const char * GL_ERROR_SOURCE[] = { "API", "WINDOW SYSTEM", "SHADER COMPILER", "THIRD PARTY", "APPLICATION", "OTHER" };
    const char * GL_ERROR_SEVERITY[] = { "HIGH", "MEDIUM", "LOW", "NOTIFICATION" };

    type = type - GL_DEBUG_TYPE_ERROR;
    source = source - GL_DEBUG_SOURCE_API;
    severity = severity == GL_DEBUG_SEVERITY_NOTIFICATION ? 3 : severity - GL_DEBUG_SEVERITY_HIGH;
    if(severity >= 2) return; // do not log low_severity or notifications

    PRINTF( "!%s:%s [ID: %u]\n", type == 0 ? "ERROR":"WARNING", message, id );
//  PANIC( "!%s:%s [ID: %u]\n", type == 0 ? "ERROR":"WARNING", message, id );
}
void glDebugEnable() {
    do_once {
    typedef void (*GLDEBUGPROC)(uint32_t, uint32_t, uint32_t, uint32_t, int32_t, const char *, const void *);
    typedef void (*GLDEBUGMESSAGECALLBACKPROC)(GLDEBUGPROC, const void *);
        void *func = glfwGetProcAddress("glDebugMessageCallback");
    void (*glDebugMessageCallback)(GLDEBUGPROC, const void *) = (GLDEBUGMESSAGECALLBACKPROC)func;
        if( func ) {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallback((GLDEBUGPROC)glDebugCallback, NULL);
        }
    }
}

static
void glCopyBackbufferToTexture( texture_t *tex ) { // unused
    glActiveTexture( GL_TEXTURE0 + tex->unit );
    glBindTexture( GL_TEXTURE_2D, tex->id );
    glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 0, 0, window_width(), window_height(), 0 );
}

// ----------------------------------------------------------------------------
// shaders

void shader_print(const char *source) {
    for(int line = 0, i = 0; source[i] > 0; ) {
        printf("\t%03d: ", line+1);
        while( source[i] >= 32 || source[i] == '\t' ) fputc(source[i++], stdout);
        while( source[i] > 0 && source[i] < 32 ) line += source[i++] == '\n';
        puts("");
    }
}

// sorted by shader handle. an array of properties per shader. properties are plain strings.
static __thread map(unsigned, array(char*)) shader_reflect;

static
GLuint shader_compile( GLenum type, const char *source ) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char **)&source, NULL);
    glCompileShader(shader);

    GLint status = GL_FALSE, length;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if( status == GL_FALSE ) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
//      ASSERT(length < 2048); char buf[2048] = { 0 };
        char *buf = stack(length+1);
        glGetShaderInfoLog(shader, length, NULL, buf);

        // dump log with line numbers
        shader_print( source );
        PANIC("ERROR: shader_compile(): %s\n%s\n", type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", buf);
        return 0;
    }

    return shader;
}

unsigned shader(const char *vs, const char *fs, const char *attribs, const char *fragcolor, const char *defines){
    return shader_geom(NULL, vs, fs, attribs, fragcolor, defines);
}

unsigned shader_geom(const char *gs, const char *vs, const char *fs, const char *attribs, const char *fragcolor, const char *defines) {
    PRINTF(/*"!"*/"Compiling shader\n");

    char *glsl_defines = "";
    if( defines ) {
        for each_substring(defines, ",", def) {
            glsl_defines = va("%s#define %s\n", glsl_defines, def);
        }
    }

    const char *glsl_version = ifdef(ems, "300 es", "150");

    vs = vs[0] == '#' && vs[1] == 'v' ? vs : va("#version %s\n%s\n%s", glsl_version, glsl_defines, vs ? vs : "");
    fs = fs[0] == '#' && fs[1] == 'v' ? fs : va("#version %s\n%s\n%s", glsl_version, glsl_defines, fs ? fs : "");
    if (gs) gs = gs[0] == '#' && gs[1] == 'v' ? gs : va("#version %s\n%s\n%s", glsl_version, glsl_defines, gs ? gs : "");

#if is(ems)
    {
        char *vs_ = REALLOC( 0, strlen(vs) + 512 ); strcpy(vs_, vs);
        char *fs_ = REALLOC( 0, strlen(fs) + 512 ); strcpy(fs_, fs);
        char *gs_ = 0; if (gs) REALLOC( 0, strlen(gs) + 512 ); strcpy(gs_, gs);
        strrepl(&fs_, "#version 300 es\n", "#version 300 es\nprecision mediump float;\n");
        vs = vs_; fs = fs_; gs = gs_;
    }
#endif

    GLuint vert = shader_compile(GL_VERTEX_SHADER, vs);
    GLuint frag = shader_compile(GL_FRAGMENT_SHADER, fs);
    GLuint geom = 0; if (gs) geom = shader_compile(GL_GEOMETRY_SHADER, gs);
    GLuint program = 0;

    if( vert && frag ) {
        program = glCreateProgram();

        glAttachShader(program, vert);
        glAttachShader(program, frag);
        if (geom) glAttachShader(program, geom);

        for( int i = 0; attribs && attribs[0]; ++i ) {
            char attrib[128] = {0};
            sscanf(attribs, "%127[^,]", attrib);
            while( attribs[0] && attribs[0] != ',' ) { attribs++; }
            while( attribs[0] && attribs[0] == ',' ) { attribs++; break; }
            if(!attrib[0]) continue;
            glBindAttribLocation(program, i, attrib);
            PRINTF("Shader.attribute[%d]=%s\n", i, attrib);
        }

#if !is(ems) // @fixme
        if(fragcolor)
        glBindFragDataLocation(program, 0, fragcolor);
#endif

        glLinkProgram(program);

        GLint status = GL_FALSE, length;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
#ifdef DEBUG_SHADER
        if (status != GL_FALSE && program == DEBUG_SHADER) {
#else
        if (status == GL_FALSE) {
#endif
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            // ASSERT(length < 2048); char buf[2048] = { 0 };
            char *buf = stack(length+1);
            glGetProgramInfoLog(program, length, NULL, buf);
            puts("--- vs:");
            shader_print(vs);
            puts("--- fs:");
            shader_print(fs);
            if (geom) {
                puts("--- gs:");
                shader_print(gs);
            }
        }
        if (status == GL_FALSE) {
            PANIC("ERROR: shader(): Shader/program link: %s\n", buf);
            return 0;
        }

        glDeleteShader(vert);
        glDeleteShader(frag);
        if (geom) glDeleteShader(geom);

//#ifdef DEBUG_ANY_SHADER
//        PRINTF("Shader #%d:\n", program);
//        shader_print(vs);
//        shader_print(fs);
//#endif
    }

/*
    if( s->program ) {
        strcatf(&s->name, "// vs (%s)\n%s\n\n\n", file_vs, vs);
        strcatf(&s->name, "// fs (%s)\n%s\n\n\n", file_fs, fs);
    }
*/

    // shader compiled fine, before returning, let's parse the source and reflect the uniforms
    array(char*) props = 0;
    do_once map_init_int( shader_reflect );
    if(vs) for each_substring(vs, "\r\n", line) {
        const char *found = strstr(line, "/""//");
        if( found > line && line[0] == '/' && line[1] == '/' ) continue;
        if( found ) array_push(props, STRDUP(line));
    }
    if(fs) for each_substring(fs, "\r\n", line) {
        const char *found = strstr(line, "/""//");
        if( found > line && line[0] == '/' && line[1] == '/' ) continue;
        if( found ) array_push(props, STRDUP(line));
    }
    if(gs) for each_substring(gs, "\r\n", line) {
        const char *found = strstr(line, "/""//");
        if( found > line && line[0] == '/' && line[1] == '/' ) continue;
        if( found ) array_push(props, STRDUP(line));
    }
    if( props ) {
        map_insert(shader_reflect, program, props);
        for( int i = 0; i < array_count(props); ++i ) shader_apply_param(program, i);
    }

    return program;
}

unsigned shader_properties(unsigned shader) {
    array(char*) *found = map_find(shader_reflect, shader);
    return found ? array_count(*found) : 0;
}

char** shader_property(unsigned shader, unsigned property) {
    array(char*) *found = map_find(shader_reflect, shader);
    return found && property < array_count(*found) ? &(*found)[property] : NULL;
}

void shader_apply_param(unsigned shader, unsigned param_no) {
    unsigned num_properties = shader_properties(shader);
    if( param_no < num_properties ) {
        char *line = *shader_property(shader, param_no);

        char type[32], name[32];
        if( sscanf(line, "%*s %s %[^ =;/]", type, name) != 2 ) return;

        int is_color = !!strstri(name, "color"), top = is_color ? 1 : 10;
        vec4 minv = strstr(line, "min:") ? atof4(strstr(line, "min:") + 4) : vec4(0,0,0,0);
        vec4 setv = strstr(line, "set:") ? atof4(strstr(line, "set:") + 4) : vec4(0,0,0,0);
        vec4 maxv = strstr(line, "max:") ? atof4(strstr(line, "max:") + 4) : vec4(top,top,top,top);

        if(minv.x > maxv.x) swapf(&minv.x, &maxv.x);
        if(minv.y > maxv.y) swapf(&minv.y, &maxv.y);
        if(minv.z > maxv.z) swapf(&minv.z, &maxv.z);
        if(minv.w > maxv.w) swapf(&minv.w, &maxv.w);

        if( !strstr(line, "max:") ) {
        if(setv.x > maxv.x) maxv.x = setv.x;
        if(setv.y > maxv.y) maxv.y = setv.y;
        if(setv.z > maxv.z) maxv.z = setv.z;
        if(setv.w > maxv.w) maxv.w = setv.w;
        }

        setv = clamp4(setv, minv, maxv);

        if( strchr("ibfv", type[0]) ) {
            GLint shader_bak; glGetIntegerv(GL_CURRENT_PROGRAM, &shader_bak);
            glUseProgram(shader);
            /**/ if(type[0] == 'i') glUniform1i(glGetUniformLocation(shader, name), setv.x);
            else if(type[0] == 'b') glUniform1i(glGetUniformLocation(shader, name), !!setv.x);
            else if(type[0] == 'f') glUniform1f(glGetUniformLocation(shader, name), setv.x);
            else if(type[3] == '2') glUniform2fv(glGetUniformLocation(shader, name), 1, &setv.x);
            else if(type[3] == '3') glUniform3fv(glGetUniformLocation(shader, name), 1, &setv.x);
            else if(type[3] == '4') glUniform4fv(glGetUniformLocation(shader, name), 1, &setv.x);
            glUseProgram(shader_bak);
        }
    }
}

void shader_apply_params(unsigned shader, const char *parameter_mask) {
    unsigned num_properties = shader_properties(shader);
    for( unsigned i = 0; i < num_properties; ++i ) {
        char *line = *shader_property(shader,i);

        char name[32];
        if( sscanf(line, "%*s %*s %s", name) != 1 ) continue;
        if( !strmatch(name, parameter_mask) ) continue;
        shader_apply_param(shader, i);
    }
}

int ui_shader(unsigned shader) {
    int changed = 0;

    unsigned num_properties = shader_properties(shader);
    for( unsigned i = 0; i < num_properties; ++i ) {
        char **ptr = shader_property(shader,i);

        const char *line = *ptr; // debug: ui_label(line);
        char* tip = strstr(line, "tip:"); tip = tip && tip[4] ? tip + 4 : 0;

        char uniform[32], type[32], name[32], early_exit = '\0';
        if( sscanf(line, "%s %s %[^ =;/]", uniform, type, name) != 3 ) continue; // @todo optimize: move to shader()
        if( strcmp(uniform, "uniform") && strcmp(uniform, "}uniform") ) { if(tip) ui_label(va(ICON_MD_INFO "%s", tip)); continue; } // @todo optimize: move to shader()

        int is_color = !!strstri(name, "color"), top = is_color ? 1 : 10;
        vec4 minv = strstr(line, "min:") ? atof4(strstr(line, "min:") + 4) : vec4(0,0,0,0);
        vec4 setv = strstr(line, "set:") ? atof4(strstr(line, "set:") + 4) : vec4(0,0,0,0);
        vec4 maxv = strstr(line, "max:") ? atof4(strstr(line, "max:") + 4) : vec4(top,top,top,top);
        char *label = !tip ? va("%c%s", name[0] - 32 * !!(name[0] >= 'a'), name+1) :
            va("%c%s  " ICON_MD_INFO  "@%s", name[0] - 32 * !!(name[0] >= 'a'), name+1, tip);

        if(minv.x > maxv.x) swapf(&minv.x, &maxv.x); // @optimize: move to shader()
        if(minv.y > maxv.y) swapf(&minv.y, &maxv.y); // @optimize: move to shader()
        if(minv.z > maxv.z) swapf(&minv.z, &maxv.z); // @optimize: move to shader()
        if(minv.w > maxv.w) swapf(&minv.w, &maxv.w); // @optimize: move to shader()

        if( !strstr(line, "max:") ) {
        if(setv.x > maxv.x) maxv.x = setv.x;
        if(setv.y > maxv.y) maxv.y = setv.y;
        if(setv.z > maxv.z) maxv.z = setv.z;
        if(setv.w > maxv.w) maxv.w = setv.w;
        }

        setv = clamp4(setv, minv, maxv);

        // supports int,float,vec2/3/4,color3/4
        int touched = 0;
        if( type[0] == 'b' ) {
            bool v = !!setv.x;

            if( (touched = ui_bool(label, &v)) != 0 ) {
                setv.x = v;
            }
        }
        else if( type[0] == 'i' ) {
            int v = setv.x;

            if( (touched = ui_int(label, &v)) != 0 ) {
                setv.x = clampi(v, minv.x, maxv.x); // min..max range
            }
        }
        else if( type[0] == 'f' ) {
            setv.x = clampf(setv.x, minv.x, maxv.x);
            char *caption = va("%5.2f", setv.x);
            setv.x = (setv.x - minv.x) / (maxv.x - minv.x);

            if( (touched = ui_slider2(label, &setv.x, caption)) != 0 ) {
                setv.x = clampf(minv.x + setv.x * (maxv.x-minv.x), minv.x, maxv.x); // min..max range
            }
        }
        else if( type[0] == 'v' && type[3] == '2' ) {
            setv.xy = clamp2(setv.xy,minv.xy,maxv.xy);

            if( (touched = ui_float2(label, &setv.x)) != 0 ) {
                setv.xy = clamp2(setv.xy,minv.xy,maxv.xy);
            }
        }
        else if( type[0] == 'v' && type[3] == '3' ) {
            setv.xyz = clamp3(setv.xyz,minv.xyz,maxv.xyz);

            if( (touched = (is_color ? ui_color3f : ui_float3)(label, &setv.x)) != 0 ) {
                setv.xyz = clamp3(setv.xyz,minv.xyz,maxv.xyz);
            }
        }
        else if( type[0] == 'v' && type[3] == '4' ) {
            setv = clamp4(setv,minv,maxv);

            if( (touched = (is_color ? ui_color4f : ui_float4)(label, &setv.x)) != 0 ) {
                setv = clamp4(setv,minv,maxv);
            }
        }
        else if( tip ) ui_label( tip );

        if( touched ) {
            // upgrade value
            *ptr = FREE(*ptr);
            *ptr = stringf("%s %s %s ///set:%s min:%s max:%s tip:%s", uniform,type,name,ftoa4(setv),ftoa4(minv),ftoa4(maxv),tip?tip:"");

            // apply
            shader_apply_param(shader, i);

            changed = 1;
        }
    }

    return changed;
}

int ui_shaders() {
    if( !map_count(shader_reflect) ) return ui_label(ICON_MD_WARNING " No shaders with annotations loaded."), 0;

    int changed = 0;
    for each_map_ptr(shader_reflect, unsigned, k, array(char*), v) {
        int open = 0, clicked_or_toggled = 0;
        char *id = va("##SHD%d", *k);
        char *title = va("Shader %d", *k);
        for( int p = (open = ui_collapse(title, id)), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
            ui_label(va("Shader %d",*k));
            changed |= ui_shader(*k);
        }
    }
    return changed;
}

unsigned compute(const char *cs){
    #if is(ems)
    return 0;
    #else
    PRINTF(/*"!"*/"Compiling compute shader\n");

    cs = cs[0] == '#' && cs[1] == 'c' ? cs : va("#version 450 core\n%s", cs ? cs : "");

    GLuint comp = shader_compile(GL_COMPUTE_SHADER, cs);
    GLuint program = 0;

    if( comp ) {
        program = glCreateProgram();

        glAttachShader(program, comp);

        glLinkProgram(program);

        GLint status = GL_FALSE, length;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
#ifdef DEBUG_SHADER
        if (status != GL_FALSE && program == DEBUG_SHADER) {
#else
        if (status == GL_FALSE) {
#endif
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            char *buf = stack(length+1);
            glGetProgramInfoLog(program, length, NULL, buf);
            puts("--- cs:");
            shader_print(cs);
        }
        if (status == GL_FALSE) {
            PANIC("ERROR: shader(): Shader/program link: %s\n", buf);
            return 0;
        }

        glDeleteShader(comp);
    }
    return program;
    #endif
}

void compute_dispatch(unsigned wx, unsigned wy, unsigned wz){
    glDispatchCompute(wx, wy, wz);
}

void write_barrier(){
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void write_barrier_image(){
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void shader_destroy(unsigned program){
    if( program == ~0u ) return;
    glDeleteProgram(program);
// if(s->name) FREE(s->name), s->name = NULL;
}

unsigned ssbo_create(const void *data, int len, unsigned usage){
    static GLuint gl_usage[] = { GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY, GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY };
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, len, data, gl_usage[usage]);
    return ssbo;
}

void ssbo_destroy(unsigned ssbo){
    glDeleteBuffers(1, &ssbo);
}

void ssbo_update(int offset, int len, const void *data){
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, len, data);
}

void *ssbo_map(unsigned access){
    static GLenum gl_access[] = {GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE};
    return glMapBuffer(GL_SHADER_STORAGE_BUFFER, gl_access[access]);
}
void ssbo_unmap(){
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ssbo_bind(unsigned ssbo, unsigned unit){
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, ssbo);
}

void ssbo_unbind(){
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

static __thread unsigned last_shader = -1;
static
int shader_uniform(const char *name) {
    int ret = glGetUniformLocation(last_shader, name);
    // if( ret < 0 ) PRINTF("!cannot find uniform '%s' in shader program %d\n", name, (int)last_shader );
    return ret;
}
unsigned shader_get_active() { return last_shader; }
unsigned shader_bind(unsigned program) { unsigned ret = last_shader; return glUseProgram(last_shader = program), ret; }
void shader_int(const char *uniform, int i)     { glUniform1i(shader_uniform(uniform), i); }
void shader_float(const char *uniform, float f) { glUniform1f(shader_uniform(uniform), f); }
void shader_vec2(const char *uniform, vec2 v)   { glUniform2fv(shader_uniform(uniform), 1, &v.x); }
void shader_vec3(const char *uniform, vec3 v)   { glUniform3fv(shader_uniform(uniform), 1, &v.x); }
void shader_vec3v(const char *uniform, int count, vec3 *v) { glUniform3fv(shader_uniform(uniform), count, &v[0].x); }
void shader_vec4(const char *uniform, vec4 v)   { glUniform4fv(shader_uniform(uniform), 1, &v.x); }
void shader_mat44(const char *uniform, mat44 m) { glUniformMatrix4fv(shader_uniform(uniform), 1, GL_FALSE/*GL_TRUE*/, m); }
void shader_cubemap(const char *sampler, unsigned texture) { glUniform1i(shader_uniform(sampler), 0); glBindTexture(GL_TEXTURE_CUBE_MAP, texture); }
void shader_bool(const char *uniform, bool x) { glUniform1i(shader_uniform(uniform), x); }
void shader_uint(const char *uniform, unsigned x ) { glUniform1ui(shader_uniform(uniform), x); }
void shader_texture(const char *sampler, texture_t t) { shader_texture_unit(sampler, t.id, t.unit); }
void shader_texture_unit(const char *sampler, unsigned id, unsigned unit) {
    // @todo. if tex.h == 1 ? GL_TEXTURE_1D : GL_TEXTURE_2D
    glUniform1i(shader_uniform(sampler), unit);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}
void shader_image(texture_t t, unsigned unit, unsigned level, int layer /* -1 to disable layered access */, unsigned access){
    shader_image_unit(t.id, unit, level, layer, t.texel_type, access);
}
void shader_image_unit(unsigned texture, unsigned unit, unsigned level, int layer, unsigned texel_type, unsigned access){
    static GLenum gl_access[] = {GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE};
    glBindImageTexture(unit, texture, level, layer!=-1, layer!=-1?layer:0, gl_access[access], texel_type);
}

void shader_colormap(const char *name, colormap_t c ) {
    // assumes shader uses `struct { vec4 color; bool has_tex } name + sampler2D name_tex;`
    shader_vec4( va("%s.color", name), c.color );
    shader_bool( va("%s.has_tex", name), c.texture != NULL );
    if( c.texture ) shader_texture( va("%s_tex", name), *c.texture );
}

// -----------------------------------------------------------------------------
// colors

unsigned rgba( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
    return (unsigned)a << 24 | r << 16 | g << 8 | b;
}
unsigned bgra( uint8_t b, uint8_t g, uint8_t r, uint8_t a ) {
    return rgba(r,g,b,a);
}
float alpha( unsigned rgba ) {
    return ( rgba >> 24 ) / 255.f;
}

unsigned rgbaf(float r, float g, float b, float a) {
    return rgba(r * 255, g * 255, b * 255, a * 255);
}
unsigned bgraf(float b, float g, float r, float a) {
    return rgba(r * 255, g * 255, b * 255, a * 255);
}

// -----------------------------------------------------------------------------
// images

image_t image_create(int x, int y, int flags) {
    int n = 3; // defaults to RGB
    if(flags & IMAGE_R) n = 1;
    if(flags & IMAGE_RG) n = 2;
    if(flags & IMAGE_RGB) n = 3;
    if(flags & IMAGE_RGBA) n = 4;
    image_t img; img.x = x; img.y = y; img.n = n;
    img.pixels = REALLOC(0, x * y * n ); // @fixme: image_destroy() requires stbi allocator to match REALLOC
    return img;
}

image_t image_from_mem(const void *data, int size, int flags) {
    image_t img = {0};
    if( data && size ) {
        stbi_set_flip_vertically_on_load(flags & IMAGE_FLIP ? 1 : 0);

        int n = 0;
        if(flags & IMAGE_R) n = 1;
        if(flags & IMAGE_RG) n = 2;
        if(flags & IMAGE_RGB) n = 3;
        if(flags & IMAGE_RGBA) n = 4;
        if(flags & IMAGE_FLOAT)
        img.pixels = stbi_loadf_from_memory((const stbi_uc*)data, size, (int*)&img.x,(int*)&img.y,(int*)&img.n, n);
        else
        img.pixels = stbi_load_from_memory((const stbi_uc*)data, size, (int*)&img.x,(int*)&img.y,(int*)&img.n, n);
        if( img.pixels ) {
            PRINTF("Loaded image (%dx%d %.*s->%.*s)\n",img.w,img.h,img.n,"RGBA",n?n:img.n,"RGBA");
        } else {
            // PANIC("Error loading image (%s)\n", pathfile);
        }
        img.n = n ? n : img.n;
    }
    return img;
}

image_t image(const char *pathfile, int flags) {
    //const char *fname = vfs_remap(pathfile);
    // if( !fname[0] ) fname = vfs_remap(va("%s.png",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.jpg",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.tga",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.jpg.png",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.tga.png",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.png.jpg",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.tga.jpg",pathfile)); // needed?

    int size = 0;
    char *data = vfs_load(pathfile, &size);
    return image_from_mem(data, size, flags);
}

void image_destroy(image_t *img) {
    if(img->pixels) stbi_image_free(img->pixels);
    img->pixels = 0; // *img = (image_t){0}; // do not clear fields yet. might be useful in the future.
}

// bilinear interpolation (uv must be in image coords, range [0..w-1,0..h-1])
static
vec3 bilinear(image_t in, vec2 uv) { // image_bilinear_pixel() ?
    float w = in.x, h = in.y, u = uv.x, v = uv.y;
    float u1 = (int)u, v1 = (int)v, u2 = minf(u1+1, w-1), v2 = minf(v1+1, h-1);
    float c1 = u - u1, c2 = v - v1;
    uint8_t *p1 = &in.pixels8[ in.n * (int)(u1 + v1 * in.w) ];
    uint8_t *p2 = &in.pixels8[ in.n * (int)(u2 + v1 * in.w) ];
    uint8_t *p3 = &in.pixels8[ in.n * (int)(u1 + v2 * in.w) ];
    uint8_t *p4 = &in.pixels8[ in.n * (int)(u2 + v2 * in.w) ];
    vec3 A = vec3( p1[0], p1[1], p1[2] );
    vec3 B = vec3( p2[0], p2[1], p2[2] );
    vec3 C = vec3( p3[0], p3[1], p3[2] );
    vec3 D = vec3( p4[0], p4[1], p4[2] );
    return mix3(mix3(A, B, c1), mix3(C, D, c1), c2);
}

// -----------------------------------------------------------------------------
// textures

static
int allocate_texture_unit() {
    static int textureUnit = 0, totalTextureUnits = 0;
    do_once glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &totalTextureUnits);

    ASSERT(textureUnit < totalTextureUnits, "%d texture units exceeded", totalTextureUnits);
    return textureUnit++;
}

unsigned texture_update(texture_t *t, unsigned w, unsigned h, unsigned n, const void *pixels, int flags) {
    if( t && !t->id ) {
        glGenTextures( 1, &t->id );
        return texture_update(t, w, h, n, pixels, flags);
    }
    ASSERT( t && t->id );
    ASSERT( n <= 4 );
    GLuint pixel_types[] = { GL_RED, GL_RED, GL_RG, GL_RGB, GL_RGBA, GL_R32F, GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F };
    GLenum pixel_storage = flags & TEXTURE_FLOAT ? GL_FLOAT : GL_UNSIGNED_BYTE;
    GLuint pixel_type = pixel_types[ n ];
    GLuint texel_type = t->texel_type = pixel_types[ n + 5 * !!(flags & TEXTURE_FLOAT) ];
    GLenum wrap = GL_CLAMP_TO_EDGE;
    GLenum min_filter = GL_NEAREST, mag_filter = GL_NEAREST;
//    GLfloat color = (flags&7)/7.f, border_color[4] = { color, color, color, 1.f };

    if( flags & TEXTURE_BGR )  if( pixel_type == GL_RGB )  pixel_type = GL_BGR;
    if( flags & TEXTURE_BGR )  if( pixel_type == GL_RGBA ) pixel_type = GL_BGRA;
    if( flags & TEXTURE_SRGB ) if( texel_type == GL_RGB )  texel_type = GL_SRGB;
    if( flags & TEXTURE_SRGB ) if( texel_type == GL_RGBA ) texel_type = GL_SRGB_ALPHA; // GL_SRGB8_ALPHA8 ?

    if( flags & TEXTURE_BC1 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    if( flags & TEXTURE_BC2 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    if( flags & TEXTURE_BC3 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    if( flags & TEXTURE_DEPTH ) texel_type = pixel_type = GL_DEPTH_COMPONENT; // GL_DEPTH_COMPONENT32

    if( flags & TEXTURE_REPEAT ) wrap = GL_REPEAT;
    if( flags & TEXTURE_BORDER ) wrap = GL_CLAMP_TO_BORDER;
    if( flags & TEXTURE_LINEAR ) min_filter = GL_LINEAR, mag_filter = GL_LINEAR;
    if( flags & TEXTURE_MIPMAPS  ) min_filter = flags & TEXTURE_LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR; // : GL_LINEAR_MIPMAP_NEAREST; maybe?
    if( flags & TEXTURE_MIPMAPS  ) mag_filter = flags & TEXTURE_LINEAR ? GL_LINEAR : GL_NEAREST;

#if 0
    if( 0 ) { // flags & TEXTURE_PREMULTIPLY_ALPHA )
        uint8_t *p = pixels;
        if(n == 2) for( unsigned i = 0; i < 2*w*h; i += 2 ) {
            p[i] = (p[i] * p[i+1] + 128) >> 8;
        }
        if(n == 4) for( unsigned i = 0; i < 4*w*h; i += 4 ) {
            p[i+0] = (p[i+0] * p[i+3] + 128) >> 8;
            p[i+1] = (p[i+1] * p[i+3] + 128) >> 8;
            p[i+2] = (p[i+2] * p[i+3] + 128) >> 8;
        }
    }
#endif

GLenum texture_type = t->flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D; // @fixme: test GL_TEXTURE_2D_ARRAY

//glPixelStorei( GL_UNPACK_ALIGNMENT, n < 4 ? 1 : 4 ); // for framebuffer reading
//glActiveTexture(GL_TEXTURE0 + (flags&7));
    glBindTexture(texture_type, t->id);
    glTexImage2D(texture_type, 0, texel_type, w, h, 0, pixel_type, pixel_storage, pixels);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, mag_filter);
#if 0 // only for sampler2DShadow
    if( flags & TEXTURE_DEPTH )   glTexParameteri(texture_type, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    if( flags & TEXTURE_DEPTH )   glTexParameteri(texture_type, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
#endif
//  if( flags & TEXTURE_BORDER )  glTexParameterfv(texture_type, GL_TEXTURE_BORDER_COLOR, border_color);
    if( flags & TEXTURE_MIPMAPS ) glGenerateMipmap(texture_type);

if( flags & TEXTURE_MIPMAPS ) {
        GLfloat max_aniso = 0;
//        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_aniso);
max_aniso = 4;
//        glTexParameterf(texture_type, GL_TEXTURE_MAX_ANISOTROPY, max_aniso);
}

    // glBindTexture(texture_type, 0); // do not unbind. current code expects texture to be bound at function exit

    t->w = w;
    t->h = h;
    t->n = n;
    t->flags = flags;
    t->filename = t->filename ? t->filename : "";

    return t->id;
}

texture_t texture_create(unsigned w, unsigned h, unsigned n, const void *pixels, int flags) {
    texture_t texture = {0};
    glGenTextures( 1, &texture.id );
    texture_update( &texture, w, h, n, pixels, flags );
    texture.unit = allocate_texture_unit();
    texture.transparent = texture.n > 3; // @fixme: should be true only if any pixel.a == 0
    return texture;
}

texture_t texture_checker() {
    static texture_t texture = {0};
    if( !texture.id ) {
#if 0
        float pixels[] = { 1,0.5,0.5,1 };
        texture = texture_create(2,2,1, pixels, TEXTURE_FLOAT|TEXTURE_MIPMAPS|TEXTURE_REPEAT|TEXTURE_BORDER);
#else
        uint32_t *pixels = REALLOC(0, 256*256*4);
        for (int y = 0, i = 0; y < 256; y++) {
            for (int x = 0; x < 256; x++) {
#if 0
                extern const uint32_t secret_palette[32];
                uint32_t rgb = secret_palette[ y / 8 ] * !!((x ^ y) & 0x8);
                pixels[i++] = (rgb>>16) & 255;
                pixels[i++] = (rgb>>8) & 255;
                pixels[i++] = (rgb>>0) & 255;
                pixels[i++] = 255;
#elif 0
                extern const uint32_t secret_palette[32];
                uint32_t rgb = ((x ^ y) & 0x8) ? secret_palette[6] : secret_palette[ 8 + ((x^y) / (256/6)) ];
                pixels[i++] = (rgb>>16) & 255;
                pixels[i++] = (rgb>>8) & 255;
                pixels[i++] = (rgb>>0) & 255;
                pixels[i++] = 255;
#else
                extern const uint32_t secret_palette[32];
                uint32_t lum = (x^y) & 8 ? 128 : (x^y) & 128 ? 192 : 255;
                uint32_t rgb = rgba(lum,lum,lum,255);
                pixels[i++] = rgb;
#endif
            }
        }
        texture = texture_create(256,256,4, pixels, TEXTURE_RGBA|TEXTURE_MIPMAPS|TEXTURE_REPEAT|TEXTURE_BORDER);
        FREE(pixels);
#endif
    }
    return texture;
}

texture_t texture_from_mem(const void *ptr, int len, int flags) {
    image_t img = image_from_mem(ptr, len, flags);
    if( img.pixels ) {
        texture_t t = texture_create(img.x, img.y, img.n, img.pixels, flags);
        image_destroy(&img);
        return t;
    }
    return texture_checker();
}

texture_t texture(const char *pathfile, int flags) {
    // PRINTF("Loading file %s\n", pathfile);
    image_t img = image(pathfile, flags);
    if( img.pixels ) {
        texture_t t = texture_create(img.x, img.y, img.n, img.pixels, flags);
        t.filename = STRDUP(file_name(pathfile));
        image_destroy(&img);
        return t;
    }
    return texture_checker();
}

void texture_destroy( texture_t *t ) {
    if(t->filename && t->filename[0]) FREE(t->filename), t->filename = 0;
    if(t->fbo) fbo_destroy(t->fbo), t->fbo = 0;
    if(t->id) glDeleteTextures(1, &t->id), t->id = 0;
    *t = (texture_t){0};
}

bool texture_rec_begin(texture_t *t, unsigned tw, unsigned th) {
    for( unsigned w = tw ? tw : window_width(), h = th ? th : window_height(); w*h ; ) {
        // resize if needed
        if( t->w != w || t->h != h ) {
            // re-create texture, set texture parameters and content
            texture_update(t, w, h, 4, NULL, TEXTURE_RGBA);
            if(!t->fbo) t->fbo = fbo(t->id, 0, 0);
        }
        // bind fbo to texture
        fbo_bind(t->fbo);
        return true;
    }
    return false;

}
void texture_rec_end(texture_t *t) {
    fbo_unbind();
}

// ktx texture loader
// - rlyeh, public domain
//
// [ref] https://developer.nvidia.com/astc-texture-compression-for-game-assets
//
// # Compatibility and modes. What to choose.
// - iOS: PVRTC1_4_RGB or PVRTC1_4 (RGBA) with q:pvrtcnormal.
// - Desktop (OSX/Linux/Windows): BC1, BC1a or BC3 with q:normal.
// - Android: ETC2_RGB or ETC2_RGBA with q:etcfast. ASTC_4x4 or ASTC_8x8 with q:astcmedium, as a fallback.

#if 0
enum {
    // for glFormat
    GLFORMAT_RED                 = 0x1903,
    GLFORMAT_RG                  = 0x8227,
    GLFORMAT_RGB                 = 0x1907,
    GLFORMAT_RGBA                = 0x1908,
    //GLFORMAT_ALPHA             = 0x1906, // 8
    //GLFORMAT_LUMINANCE         = 0x1909, // 8
    //GLFORMAT_LUMINANCE_ALPHA   = 0x190A, // 88

    // for glType
    GLTYPE_UNSIGNED_BYTE         = 0x1401,

    // for glInternalFormat: RAW           // @todo: SRGB, SRGBA, SBGR, SBGRA
    UNCOMPRESSED_RGB             = 0x8051, // 888, GL_RGB8_EXT
    UNCOMPRESSED_RGB_565         = 0x8363,
    UNCOMPRESSED_RGBA            = 0x8058, // 8888, GL_RGBA8_EXT
    UNCOMPRESSED_RGBA_4444       = 0x8033,
    UNCOMPRESSED_RGBA_5551       = 0x8034,
    UNCOMPRESSED_BGR             = 0x80E0, // 888
    UNCOMPRESSED_BGRA            = 0x80E1, // 8888

    // for glInternalFormat: S3TC/DXTn/BCn // @todo: BC4,5,6,7*
    COMPRESSED_RGB_BC1           = 0x83F0, // DXT1
    COMPRESSED_RGBA_BC1          = 0x83F1, // DXT1a, BC1a
    COMPRESSED_RGBA_BC2          = 0x83F2, // DXT3
    COMPRESSED_RGBA_BC3          = 0x83F3, // DXT5
    COMPRESSED_RGBA_BC7          = 0x8E8C, // COMPRESSED_RGBA_BPTC_UNORM_ARB
    COMPRESSED_SRGB_BC1          = 0x8C4C,
    COMPRESSED_SRGBA_BC1         = 0x8C4D,
    COMPRESSED_SRGBA_BC2         = 0x8C4E,
    COMPRESSED_SRGBA_BC3         = 0x8C4F,
    // RGB_BC7f  COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB
    // RGB_BC7uf COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB
    // RGBA_BC7  COMPRESSED_RGBA_BPTC_UNORM_ARB
    // SRGBA_BC7 COMPRESSED_SRGBA_BPTC_UNORM_ARB

    // for glInternalFormat: ETC2+EAC
    COMPRESSED_R_EAC             = 0x9270, // 4bpp
    COMPRESSED_R_EAC_SIGNED      = 0x9271, // 4bpp. can preserve 0
    COMPRESSED_RG_EAC            = 0x9272, // 8bpp
    COMPRESSED_RG_EAC_SIGNED     = 0x9273, // 8bbp. can preserve 0
    COMPRESSED_RGB_ETC2          = 0x9274, // 4bpp
    COMPRESSED_RGBA_ETC2         = 0x9276, // 4bpp A1
    COMPRESSED_RGBA_ETC2_EAC     = 0x9278, // 8bpp
    COMPRESSED_SRGB_ETC2         = 0x9275, // 4bpp
    COMPRESSED_SRGBA_ETC2        = 0x9277, // 4bpp A1
    COMPRESSED_SRGBA_ETC2_EAC    = 0x9279, // 8bpp

    // for glInternalFormat: PVR
    COMPRESSED_RGB_PVR1_2        = 0x8C01,
    COMPRESSED_RGB_PVR1_4        = 0x8C00,
    COMPRESSED_RGBA_PVR1_2       = 0x8C03,
    COMPRESSED_RGBA_PVR1_4       = 0x8C02,

    COMPRESSED_SRGB_PVR1_2       = 0x8A54, // _EXT
    COMPRESSED_SRGB_PVR1_4       = 0x8A55, // _EXT
    COMPRESSED_SRGBA_PVR1_2      = 0x8A56, // _EXT
    COMPRESSED_SRGBA_PVR1_4      = 0x8A57, // _EXT

    COMPRESSED_RGBA_PVR2_2       = 0x9137,
    COMPRESSED_RGBA_PVR2_4       = 0x9138,
    COMPRESSED_SRGBA_PVR2_2      = 0x93F0,
    COMPRESSED_SRGBA_PVR2_4      = 0x93F1,

    // for glInternalFormat: ASTC
    COMPRESSED_RGBA_ASTC4x4      = 0x93B0, // 8.00bpp
    COMPRESSED_RGBA_ASTC5x4      = 0x93B1, // 6.40bpp
    COMPRESSED_RGBA_ASTC5x5      = 0x93B2, // 5.12bpp
    COMPRESSED_RGBA_ASTC6x5      = 0x93B3, // 4.27bpp
    COMPRESSED_RGBA_ASTC6x6      = 0x93B4, // 3.56bpp
    COMPRESSED_RGBA_ASTC8x5      = 0x93B5, // 3.20bpp
    COMPRESSED_RGBA_ASTC8x6      = 0x93B6, // 2.67bpp
    COMPRESSED_RGBA_ASTC8x8      = 0x93B7, // 2.56bpp
    COMPRESSED_RGBA_ASTC10x5     = 0x93B8, // 2.13bpp
    COMPRESSED_RGBA_ASTC10x6     = 0x93B9, // 2.00bpp
    COMPRESSED_RGBA_ASTC10x8     = 0x93BA, // 1.60bpp
    COMPRESSED_RGBA_ASTC10x10    = 0x93BB, // 1.28bpp
    COMPRESSED_RGBA_ASTC12x10    = 0x93BC, // 1.07bpp
    COMPRESSED_RGBA_ASTC12x12    = 0x93BD, // 0.89bpp

    COMPRESSED_SRGBA_ASTC4x4     = 0x93D0, // 8.00bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC5x4     = 0x93D1, // 6.40bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC5x5     = 0x93D2, // 5.12bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC6x5     = 0x93D3, // 4.27bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC6x6     = 0x93D4, // 3.56bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC8x5     = 0x93D5, // 3.20bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC8x6     = 0x93D6, // 2.67bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC8x8     = 0x93D7, // 2.56bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x5    = 0x93D8, // 2.13bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x6    = 0x93D9, // 2.00bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x8    = 0x93DA, // 1.60bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x10   = 0x93DB, // 1.28bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC12x10   = 0x93DC, // 1.07bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC12x12   = 0x93DD, // 0.89bpp SRGB8 A8

    // others:
    // COMPRESSED_RED_RGTC1
    // COMPRESSED_SIGNED_RED_RGTC1
    // COMPRESSED_RG_RGTC2
    // COMPRESSED_SIGNED_RG_RGTC2
};
#endif

#pragma pack(push, 1) // not really needed. the struct is exactly 64 bytes, and all members are 32-bit unsigned
typedef struct ktx_header {
    unsigned identifier[3];         // "«KTX 11»\r\n\x1A\n"
    unsigned endianness;            // 0x04030201 if match

    unsigned glType;                // 0 if compressed; otherwise: UNSIGNED_BYTE, UNSIGNED_SHORT_5_6_5, etc.
    unsigned glTypeSize;            // 1 if compressed; otherwise, size in bytes of glType for endianness conversion. not needed.
    unsigned glFormat;              // STENCIL_INDEX, DEPTH_COMPONENT, DEPTH_STENCIL, RED, GREEN, BLUE, RG, RGB, RGBA, BGR, BGRA, RED_INTEGER, GREEN_INTEGER, BLUE_INTEGER, RG_INTEGER, RGB_INTEGER, RGBA_INTEGER, BGR_INTEGER, BGRA_INTEGER,
    unsigned glInternalFormat;      // COMPRESSED_RED, COMPRESSED_RG, COMPRESSED_RGB, COMPRESSED_RGBA, COMPRESSED_SRGB, COMPRESSED_SRGB_ALPHA, COMPRESSED_RED_RGTC1, COMPRESSED_SIGNED_RED_RGTC1, COMPRESSED_RG_RGTC2, COMPRESSED_SIGNED_RG_RGTC2, COMPRESSED_RGBA_BPTC_UNORM, COMPRESSED_SRGB_ALPHA_BPTC_UNORM, COMPRESSED_RGB_BPTC_SIGNED_FLOAT, COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, COMPRESSED_RGB8_ETC2, COMPRESSED_SRGB8_ETC2, COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, COMPRESSED_RGBA8_ETC2_EAC, COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, COMPRESSED_R11_EAC, COMPRESSED_SIGNED_R11_EAC, COMPRESSED_RG11_EAC, COMPRESSED_SIGNED_RG11_EAC,
    unsigned glBaseInternalFormat;  // DEPTH_COMPONENT, DEPTH_STENCIL, RED, RG, RGB, RGBA, STENCIL_INDEX,

    unsigned width;
    unsigned height;
    unsigned depth;

    unsigned num_surfaces; // >1 for material
    unsigned num_faces;    // =6 for cubemaps (+X,-X,+Y,-Y,+Z,-Z order), 1 otherwise
    unsigned num_mipmaps;  // >1 for mipmaps

    unsigned metadata_size; // length of following header
    // struct ktx_metadata {
    //     unsigned key_and_value_size;
    //     char     key_and_value[key_and_value_size];
    //     char     value_padding[3 - ((key_and_value_size + 3) % 4)];
    // };
    // struct ktx_texture_data {
    //     unsigned size;
    //     char     data[0];
    // } tx;
} ktx_header;
#pragma pack(pop)

typedef struct ktx_texture {
    unsigned width;
    unsigned height;
    unsigned depth;
    unsigned size;
    const char* data;
} ktx_texture;

typedef struct ktx {
    ktx_header hdr;
    const char *error;
} ktx;

static __thread array(ktx_texture) ktx_textures;

static
ktx ktx_load(const void *data, unsigned int len) {
    ktx ctx = {0};

    // check ktx signature
    bool is_ktx = (len > sizeof(ktx_header)) && !memcmp(data, "\xABKTX 11\xBB\r\n\x1A\n", 12);
    if( !is_ktx ) {
        return ctx.error = "ERROR_BAD_KTX_FILE", ctx;
    }

    // copy texture header
    ktx_header *hdr = &ctx.hdr;
    *hdr = *((const ktx_header *)data);

    // sanity checks
    STATIC_ASSERT(sizeof(ktx_header) == (16*4));
    for( int i = 0; i < sizeof(ktx_header)/4; ++i) {
        i[(unsigned*)hdr] = lil32(i[(unsigned*)hdr]);
    }
    if( hdr->endianness != 0x04030201 ) {
        return ctx.error = "ERROR_BAD_ENDIANNESS", ctx;
    }
    if( (hdr->num_faces != 1) && (hdr->num_faces != 6) ) {
        return ctx.error = "ERROR_BAD_NUMBER_OF_FACES", ctx;
    }

    // normalize glInternalFormat if uncompressed.
    if( hdr->glType != 0 ) {
        hdr->glInternalFormat = hdr->glBaseInternalFormat;
    }

    // normalize [1..N] range
    hdr->num_mipmaps += !hdr->num_mipmaps;
    hdr->num_surfaces += !hdr->num_surfaces;
    hdr->num_faces += !hdr->num_faces;

    // basically,
    // for each level in num_mipmaps { UInt32 imageSize;
    //     for each surface in num_surfaces {
    //         for each face in num_faces {
    //             for each slice in depth {
    //                 for each row in height {
    //                     for each pixel in width {
    //                         byte data[size_based_on_pixelformat]
    //         byte facePadding[0-3] }}}
    //     }
    // Byte mipPadding[0-3] }

    array_resize(ktx_textures, hdr->num_mipmaps * hdr->num_surfaces * hdr->num_faces);

    const char *bitmap = ((const char*)data) + sizeof(ktx_header) + hdr->metadata_size;

    for( unsigned m = 0; m < hdr->num_mipmaps; ++m ) {
        for( unsigned s = 0; s < hdr->num_surfaces; ++s ) {
            for( unsigned f = 0; f < hdr->num_faces; ++f ) {
                ktx_texture *t = &ktx_textures[f+s*hdr->num_faces+m*hdr->num_faces*hdr->num_surfaces];

                // set dimensions [1..N]
                t->width  = (hdr->width  >> m) + !(hdr->width  >> m);
                t->height = (hdr->height >> m) + !(hdr->height >> m);
                t->depth  = (hdr->depth  >> m) + !(hdr->depth  >> m);

                // seek to mip
                const char *ptr = bitmap;
                for( int i = 0; i <= m; i++ ) {
                    // if cubemap, *ptr holds unpadded size of single face,
                    // else, *ptr holds size of all surfaces+faces+slices for whole mipmap.
                    unsigned size = lil32(*(unsigned*)ptr);
                    unsigned padding = 3 - ((size + 3) % 4);

                    // seek to data
                    t->data = ptr + 4 + (size * f);

                    // seek to next mipmap
                    ptr = ptr + 4 + (size * hdr->num_faces) + padding;

                    // adjust size
                    t->size = (uintptr_t)(ptr - t->data); // -padding; needed?
                }

                // ensure we're in bounds
                ASSERT(t->data < ((char*)data + len), "%p < %p", t->data, ((char*)data + len));
                ASSERT(((char*)t->data+t->size) <= ((char*)data + len), "%p < %p", (char*)t->data + t->size, ((char*)data + len));
            }
        }
    }

    return ctx;
}

// ---

texture_t texture_compressed_from_mem(const void *data, int len, unsigned flags) {
    ktx ctx = ktx_load(data, len);
    if( ctx.error ) {
        // puts(ctx.error);
        // return texture_checker();
        return texture_from_mem(data, len, flags);
    }

    ktx_header hdr = ctx.hdr;

    // flags
    int target = hdr.num_faces == 6 ? GL_TEXTURE_CUBE_MAP : hdr.depth > 0 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
    int dimensions = target == GL_TEXTURE_3D ? 3 : target == GL_TEXTURE_2D || target == GL_TEXTURE_CUBE_MAP ? 2 : 1;

    // create texture
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(target, id);

    // filtering
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, hdr.num_mipmaps > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    // wrapping
    if( dimensions > 0 ) glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    if( dimensions > 1 ) glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if( dimensions > 2 ) glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_REPEAT);

    if( target == GL_TEXTURE_CUBE_MAP ) target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;

    // GLenum internalFormat = flags & TEXTURE_SRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8; // @fixme

    int bytes = 0;
    enum { border = 0 };
    for( int m = 0; m < hdr.num_mipmaps; ++m ) {
        for( int s = 0; s < hdr.num_surfaces; ++s ) {
            for( int f = 0; f < hdr.num_faces; ++f ) {
                int d3 = target == GL_TEXTURE_3D, compr = hdr.glType == 0, mode = d3+compr*2;
                ktx_texture *t = &ktx_textures[f+s*hdr.num_faces+m*hdr.num_faces*hdr.num_surfaces];
                /**/ if(mode==0) glTexImage2D(target+f,m,hdr.glInternalFormat,t->width,t->height,                   border,hdr.glFormat,hdr.glType,t->data);
                else if(mode==1) glTexImage3D(target  ,m,hdr.glInternalFormat,t->width,t->height,t->depth,          border,hdr.glFormat,hdr.glType,t->data);
                else if(mode==2) glCompressedTexImage2D(target+f,m,hdr.glInternalFormat,t->width,t->height,         border,t->size,t->data);
                else if(mode==3) glCompressedTexImage3D(target  ,m,hdr.glInternalFormat,t->width,t->height,t->depth,border,t->size,t->data);
                bytes += t->size;
            }
        }
    }

//    if( !hdr.num_mipmaps )
//    if( flags & TEXTURE_MIPMAPS ) glGenerateMipmap(target);

    texture_t t = {0};
    t.id = id;
    t.w = ktx_textures[0].width;
    t.h = ktx_textures[0].height;
    t.d = ktx_textures[0].depth;
    // @todo: reconstruct flags

    PRINTF("dims:%dx%dx%d,size:%.2fMiB,mips:%d,layers:%d,faces:%d\n", t.w, t.h, t.d, bytes / 1024.0 / 1024.0, hdr.num_mipmaps, hdr.num_surfaces, hdr.num_faces);
    return t;
}

texture_t texture_compressed(const char *pathfile, unsigned flags) {
    //const char *fname = vfs_remap(pathfile);

    int size = 0;
    char *data = vfs_load(pathfile, &size);
    return texture_compressed_from_mem(data, size, flags);
}

// -----------------------------------------------------------------------------
// shadowmaps

shadowmap_t shadowmap(int texture_width) { // = 1024
    shadowmap_t s = {0};
    s.texture_width = texture_width;

    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s.saved_fb);

    glGenFramebuffers(1, &s.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, s.fbo);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &s.texture);
    glBindTexture(GL_TEXTURE_2D, s.texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texture_width, texture_width, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, s.texture, 0);

#if is(ems)
    GLenum nones[] = { GL_NONE };
    glDrawBuffers(1, nones);
    glReadBuffer(GL_NONE);
#else
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, s.saved_fb);
    return s;
}

void shadowmap_destroy(shadowmap_t *s) {
    if (s->texture) {
        glDeleteTextures(1, &s->texture);
    }
    if (s->fbo) {
        glDeleteFramebuffers(1, &s->fbo);
    }
    shadowmap_t z = {0};
    *s = z;
}

void shadowmap_set_shadowmatrix(shadowmap_t *s, vec3 aLightPos, vec3 aLightAt, vec3 aLightUp, const mat44 projection) {
    copy44(s->proj, projection);
    s->light_position = vec4(aLightPos.x, aLightPos.y, aLightPos.z, 1);
    lookat44(s->mv, aLightPos, aLightAt, aLightUp);

    mat44 bias = {
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0 };

    // s->shadowmatrix = bias;
    // s->shadowmatrix *= s->proj;
    // s->shadowmatrix *= s->mv;
//  multiply44x3(s->shadowmatrix, s->mv, s->proj, bias);
    multiply44x3(s->shadowmatrix, bias, s->proj, s->mv);

    // mvp = projection * s->mv;
//  multiply44x2(s->mvp, s->mv, projection);
    multiply44x2(s->mvp, projection, s->mv);
}

void shadowmap_begin(shadowmap_t *s) {
    glGetIntegerv(GL_VIEWPORT, &s->saved_viewport[0]);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s->saved_fb);

    glBindFramebuffer(GL_FRAMEBUFFER, s->fbo);
    glViewport(0, 0, s->texture_width, s->texture_width);

    glClearDepth(1);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void shadowmap_end(shadowmap_t *s) {
    glViewport(s->saved_viewport[0], s->saved_viewport[1], s->saved_viewport[2], s->saved_viewport[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, s->saved_fb);
}

// shadowmap utils

void shadowmatrix_proj(mat44 shm_proj, float aLightFov, float znear, float zfar) {
    perspective44(shm_proj, aLightFov, 1.0f, znear, zfar);
}

void shadowmatrix_ortho(mat44 shm_proj, float left, float right, float bottom, float top, float znear, float zfar) {
    ortho44(shm_proj, left, right, bottom, top, znear, zfar);
}

// -----------------------------------------------------------------------------
// fullscreen quads

// usage: bind empty vao & commit call for 6 (quad) or 3 vertices (tri).
// ie, glBindVertexArray(empty_vao); glDrawArrays(GL_TRIANGLES, 0, 3);

void fullscreen_quad_rgb( texture_t texture, float gamma ) {
    static int program = -1, vao = -1, u_inv_gamma = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B_flipped.glsl");
        const char* fs = vfs_read("shaders/fs_2_4_texel_inv_gamma.glsl");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        u_inv_gamma = glGetUniformLocation(program, "u_inv_gamma");
        glGenVertexArrays( 1, (GLuint*)&vao );
    }

    GLenum texture_type = texture.flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_inv_gamma, 1.0f / (gamma + !gamma) );

    glBindVertexArray( vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( texture_type, texture.id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( texture_type, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

void fullscreen_quad_rgb_flipped( texture_t texture, float gamma ) {
    static int program = -1, vao = -1, u_inv_gamma = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B.glsl");
        const char* fs = vfs_read("shaders/fs_2_4_texel_inv_gamma.glsl");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        u_inv_gamma = glGetUniformLocation(program, "u_inv_gamma");
        glGenVertexArrays( 1, (GLuint*)&vao );
    }

    GLenum texture_type = texture.flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_inv_gamma, 1.0f / (gamma + !gamma) );

    glBindVertexArray( vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( texture_type, texture.id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( texture_type, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

void fullscreen_quad_ycbcr( texture_t textureYCbCr[3], float gamma ) {
    static int program = -1, vao = -1, u_gamma = -1, uy = -1, ucb = -1, ucr = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B_flipped.glsl");
        const char* fs = vfs_read("shaders/fs_2_4_texel_ycbr_gamma_saturation.glsl");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        u_gamma = glGetUniformLocation(program, "u_gamma");

        uy = glGetUniformLocation(program, "u_texture_y");
        ucb = glGetUniformLocation(program, "u_texture_cb");
        ucr = glGetUniformLocation(program, "u_texture_cr");

        glGenVertexArrays( 1, (GLuint*)&vao );
    }

//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_gamma, gamma );

    glBindVertexArray( vao );

    glUniform1i(uy, 0);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[0].id );

    glUniform1i(ucb, 1);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[1].id );

    glUniform1i(ucr, 2);
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[2].id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( GL_TEXTURE_2D, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

void fullscreen_quad_ycbcr_flipped( texture_t textureYCbCr[3], float gamma ) {
    static int program = -1, vao = -1, u_gamma = -1, uy = -1, ucb = -1, ucr = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B.glsl");
        const char* fs = vfs_read("shaders/fs_2_4_texel_ycbr_gamma_saturation.glsl");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        u_gamma = glGetUniformLocation(program, "u_gamma");

        uy = glGetUniformLocation(program, "u_texture_y");
        ucb = glGetUniformLocation(program, "u_texture_cb");
        ucr = glGetUniformLocation(program, "u_texture_cr");

        glGenVertexArrays( 1, (GLuint*)&vao );
    }

//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_gamma, gamma );

    glBindVertexArray( vao );

    glUniform1i(uy, 0);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[0].id );

    glUniform1i(ucb, 1);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[1].id );

    glUniform1i(ucr, 2);
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[2].id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( GL_TEXTURE_2D, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

// ----------------------------------------------------------------------------
// sprites

typedef struct sprite_t {
    float px, py, pz;         // origin x, y, depth
    float ox, oy, cos, sin;   // offset x, offset y, cos/sin of rotation degree
    float sx, sy;             // scale x,y
    uint32_t rgba;            // vertex color
    float cellw, cellh;       // dimensions of any cell in spritesheet

    union {
    struct {
        int frame, ncx, ncy;      // frame in a (num cellx, num celly) spritesheet
    };
    struct {
        float x, y, w, h;         // normalized[0..1] within texture bounds
    };
    };
} sprite_t;

// sprite batching
typedef struct batch_t { array(sprite_t) sprites; mesh_t mesh; int dirty; } batch_t;
typedef map(int, batch_t) batch_group_t; // mapkey is anything that forces a flush. texture_id for now, might be texture_id+program_id soon

// sprite stream
typedef struct sprite_vertex { vec3 pos; vec2 uv; uint32_t rgba; } sprite_vertex;
typedef struct sprite_index  { GLuint triangle[3]; } sprite_index;

#define sprite_vertex(...) C_CAST(sprite_vertex, __VA_ARGS__)
#define sprite_index(...)  C_CAST(sprite_index, __VA_ARGS__)

// sprite impl
static int sprite_count = 0;
static int sprite_program = -1;
static array(sprite_index)  sprite_indices = 0;
static array(sprite_vertex) sprite_vertices = 0;
static batch_group_t sprite_additive_group = {0}; // (w/2,h/2) centered
static batch_group_t sprite_translucent_group = {0}; // (w/2,h/2) centered
static batch_group_t sprite_00_translucent_group = {0}; // (0,0) centered

void sprite( texture_t texture, float position[3], float rotation, uint32_t color ) {
    float offset[2] = {0,0}, scale[2] = {1,1}, spritesheet[3] = {0,0,0};
    sprite_sheet( texture, spritesheet, position, rotation, offset, scale, 0, color, false );
}

// rect(x,y,w,h) is [0..1] normalized, z-index, pos(x,y,scale), rotation (degrees), color (rgba)
void sprite_rect( texture_t t, vec4 rect, float zindex, vec3 pos, float tilt_deg, unsigned tint_rgba) {
    // @todo: no need to queue if alpha or scale are zero
    sprite_t s = {0};

    s.x = rect.x, s.y = rect.y, s.w = rect.z, s.h = rect.w;
    s.cellw = s.w * t.w, s.cellh = s.h * t.h;

    s.px = pos.x, s.py = pos.y, s.pz = zindex;
    s.sx = s.sy = pos.z;
    s.rgba = tint_rgba;
    s.ox = 0/*ox*/ * s.sx;
    s.oy = 0/*oy*/ * s.sy;
    if( tilt_deg ) {
        tilt_deg = (tilt_deg + 0) * ((float)C_PI / 180);
        s.cos = cosf(tilt_deg);
        s.sin = sinf(tilt_deg);
    } else {
        s.cos = 1;
        s.sin = 0;
    }

    batch_group_t *batches = &sprite_00_translucent_group;
    batch_t *found = map_find_or_add(*batches, t.id, (batch_t){0});

    array_push(found->sprites, s);
}

void sprite_sheet( texture_t texture, float spritesheet[3], float position[3], float rotation, float offset[2], float scale[2], int is_additive, uint32_t rgba, int resolution_independant) {
    const float px = position[0], py = position[1], pz = position[2];
    const float ox = offset[0], oy = offset[1], sx = scale[0], sy = scale[1];
    const float frame = spritesheet[0], xcells = spritesheet[1], ycells = spritesheet[2];

    if (frame < 0) return;
    if (frame > 0 && frame >= (xcells * ycells)) return;

    // no need to queue if alpha or scale are zero
    if( sx && sy && alpha(rgba) ) {
        vec3 bak = camera_get_active()->position;
        if( resolution_independant ) { // @todo: optimize me
        sprite_flush();
        camera_get_active()->position = vec3(window_width()/2,window_height()/2,1);
        }

        sprite_t s;
        s.px = px;
        s.py = py;
        s.pz = pz;
        s.frame = frame;
        s.ncx = xcells ? xcells : 1;
        s.ncy = ycells ? ycells : 1;
        s.sx = sx;
        s.sy = sy;
        s.ox = ox * sx;
        s.oy = oy * sy;
        s.cellw = (texture.x * sx / s.ncx);
        s.cellh = (texture.y * sy / s.ncy);
        s.rgba = rgba;
        s.cos = 1;
        s.sin = 0;
        if(rotation) {
            rotation = (rotation + 0) * ((float)C_PI / 180);
            s.cos = cosf(rotation);
            s.sin = sinf(rotation);
        }

        batch_group_t *batches = is_additive ? &sprite_additive_group : &sprite_translucent_group;
#if 0
        batch_t *found = map_find(*batches, texture.id);
        if( !found ) found = map_insert(*batches, texture.id, (batch_t){0});
#else
        batch_t *found = map_find_or_add(*batches, texture.id, (batch_t){0});
#endif

        array_push(found->sprites, s);

        if( resolution_independant ) { // @todo: optimize me
        sprite_flush();
        camera_get_active()->position = bak;
        }
    }
}

static void sprite_rebuild_meshes() {
    sprite_count = 0;

    batch_group_t* list[] = { &sprite_additive_group, &sprite_translucent_group };
    for( int l = 0; l < countof(list); ++l) {
        for each_map_ptr(*list[l], int,_, batch_t,bt) {

            bt->dirty = array_count(bt->sprites) ? 1 : 0;
            if( !bt->dirty ) continue;

            int index = 0;
            array_clear(sprite_indices);
            array_clear(sprite_vertices);

            array_foreach_ptr(bt->sprites, sprite_t,it ) {
                float x0 = it->ox - it->cellw/2, x3 = x0 + it->cellw;
                float y0 = it->oy - it->cellh/2, y3 = y0;
                float x1 = x0,                   x2 = x3;
                float y1 = y0 + it->cellh,       y2 = y1;

                // @todo: move this affine transform into glsl shader
                vec3 v0 = { it->px + ( x0 * it->cos - y0 * it->sin ), it->py + ( x0 * it->sin + y0 * it->cos ), it->pz };
                vec3 v1 = { it->px + ( x1 * it->cos - y1 * it->sin ), it->py + ( x1 * it->sin + y1 * it->cos ), it->pz };
                vec3 v2 = { it->px + ( x2 * it->cos - y2 * it->sin ), it->py + ( x2 * it->sin + y2 * it->cos ), it->pz };
                vec3 v3 = { it->px + ( x3 * it->cos - y3 * it->sin ), it->py + ( x3 * it->sin + y3 * it->cos ), it->pz };

                float cx = (1.0f / it->ncx) - 1e-9f;
                float cy = (1.0f / it->ncy) - 1e-9f;
                int idx = (int)it->frame;
                int px = idx % it->ncx;
                int py = idx / it->ncx;

                float ux = px * cx, uy = py * cy;
                float vx = ux + cx, vy = uy + cy;

                vec2 uv0 = vec2(ux, uy);
                vec2 uv1 = vec2(ux, vy);
                vec2 uv2 = vec2(vx, vy);
                vec2 uv3 = vec2(vx, uy);

                array_push( sprite_vertices, sprite_vertex(v0, uv0, it->rgba) ); // Vertex 0 (A)
                array_push( sprite_vertices, sprite_vertex(v1, uv1, it->rgba) ); // Vertex 1 (B)
                array_push( sprite_vertices, sprite_vertex(v2, uv2, it->rgba) ); // Vertex 2 (C)
                array_push( sprite_vertices, sprite_vertex(v3, uv3, it->rgba) ); // Vertex 3 (D)

                //      A--B                  A               A-B
                // quad |  | becomes triangle |\  and triangle \|
                //      D--C                  D-C               C
                GLuint A = (index+0), B = (index+1), C = (index+2), D = (index+3); index += 4;

                array_push( sprite_indices, sprite_index(C, D, A) ); // Triangle 1
                array_push( sprite_indices, sprite_index(C, A, B) ); // Triangle 2
            }

            mesh_update(&bt->mesh, "p3 t2 c4B", 0,array_count(sprite_vertices),sprite_vertices, 3*array_count(sprite_indices),sprite_indices, MESH_STATIC);

            // clear elements from queue
            sprite_count += array_count(bt->sprites);
            array_clear(bt->sprites);
        }
    }

    batch_group_t* list2[] = { &sprite_00_translucent_group };
    for( int l = 0; l < countof(list2); ++l) {
        for each_map_ptr(*list2[l], int,_, batch_t,bt) {

            bt->dirty = array_count(bt->sprites) ? 1 : 0;
            if( !bt->dirty ) continue;

            int index = 0;
            array_clear(sprite_indices);
            array_clear(sprite_vertices);

            array_foreach_ptr(bt->sprites, sprite_t,it ) {
                float x0 = it->ox - it->cellw/2, x3 = x0 + it->cellw;
                float y0 = it->oy - it->cellh/2, y3 = y0;
                float x1 = x0,                   x2 = x3;
                float y1 = y0 + it->cellh,       y2 = y1;

                // @todo: move this affine transform into glsl shader
                vec3 v0 = { it->px + ( x0 * it->cos - y0 * it->sin ), it->py + ( x0 * it->sin + y0 * it->cos ), it->pz };
                vec3 v1 = { it->px + ( x1 * it->cos - y1 * it->sin ), it->py + ( x1 * it->sin + y1 * it->cos ), it->pz };
                vec3 v2 = { it->px + ( x2 * it->cos - y2 * it->sin ), it->py + ( x2 * it->sin + y2 * it->cos ), it->pz };
                vec3 v3 = { it->px + ( x3 * it->cos - y3 * it->sin ), it->py + ( x3 * it->sin + y3 * it->cos ), it->pz };

                float ux = it->x, vx = ux + it->w;
                float uy = it->y, vy = uy + it->h;

                vec2 uv0 = vec2(ux, uy);
                vec2 uv1 = vec2(ux, vy);
                vec2 uv2 = vec2(vx, vy);
                vec2 uv3 = vec2(vx, uy);

                array_push( sprite_vertices, sprite_vertex(v0, uv0, it->rgba) ); // Vertex 0 (A)
                array_push( sprite_vertices, sprite_vertex(v1, uv1, it->rgba) ); // Vertex 1 (B)
                array_push( sprite_vertices, sprite_vertex(v2, uv2, it->rgba) ); // Vertex 2 (C)
                array_push( sprite_vertices, sprite_vertex(v3, uv3, it->rgba) ); // Vertex 3 (D)

                //      A--B                  A               A-B
                // quad |  | becomes triangle |\  and triangle \|
                //      D--C                  D-C               C
                GLuint A = (index+0), B = (index+1), C = (index+2), D = (index+3); index += 4;

                array_push( sprite_indices, sprite_index(C, D, A) ); // Triangle 1
                array_push( sprite_indices, sprite_index(C, A, B) ); // Triangle 2
            }

            mesh_update(&bt->mesh, "p3 t2 c4B", 0,array_count(sprite_vertices),sprite_vertices, 3*array_count(sprite_indices),sprite_indices, MESH_STATIC);

            // clear elements from queue
            sprite_count += array_count(bt->sprites);
            array_clear(bt->sprites);
        }
    }
}

static void sprite_render_meshes() {
    if( map_count(sprite_additive_group) <= 0 )
    if( map_count(sprite_translucent_group) <= 0 )
    if( map_count(sprite_00_translucent_group) <= 0 )
        return;

    if( sprite_program < 0 ) {
        sprite_program = shader( vfs_read("shaders/vs_324_24_sprite.glsl"), vfs_read("shaders/fs_24_4_sprite.glsl"),
            "att_Position,att_TexCoord,att_Color",
            "fragColor", NULL
        );
    }

    // use the shader and  bind the texture @ unit 0
    shader_bind(sprite_program);
    glActiveTexture(GL_TEXTURE0);

    // setup rendering state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL); // try to help with zfighting

    // update camera
    // camera_fps(camera_get_active(), 0,0);
    vec3 pos = camera_get_active()->position;
    float zoom = absf(pos.z); if(zoom < 0.1f) zoom = 0.1f; zoom = 1.f / (zoom + !zoom);
    float width  = window_width();
    float height = window_height();

    // set mvp in the uniform. (0,0) is center of screen.
    mat44 mvp2d;
    float zdepth_max = window_height(); // 1;
    float l = pos.x - width  * zoom / 2;
    float r = pos.x + width  * zoom / 2;
    float b = pos.y + height * zoom / 2;
    float t = pos.y - height * zoom / 2;
    ortho44(mvp2d, l,r,b,t, -zdepth_max, +zdepth_max);

    shader_mat44("u_mvp", mvp2d);

    // set (unit 0) in the uniform texture sampler, and render batch
    // for all additive then translucent groups

    if( map_count(sprite_additive_group) > 0 ) {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE );
        for each_map_ptr(sprite_additive_group, int,texture_id, batch_t,bt) {
            if( bt->dirty ) {
                shader_texture_unit("u_texture", *texture_id, 0);
                mesh_render(&bt->mesh);
            }
        }
//        map_clear(sprite_additive_group);
    }

    if( map_count(sprite_translucent_group) > 0 ) {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        for each_map_ptr(sprite_translucent_group, int,texture_id, batch_t,bt) {
            if( bt->dirty ) {
                shader_texture_unit("u_texture", *texture_id, 0);
                mesh_render(&bt->mesh);
            }
        }
//        map_clear(sprite_translucent_group);
    }

    if( map_count(sprite_00_translucent_group) > 0 ) {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        for each_map_ptr(sprite_00_translucent_group, int,texture_id, batch_t,bt) {
            if( bt->dirty ) {
                shader_texture_unit("u_texture", *texture_id, 0);
                mesh_render(&bt->mesh);
            }
        }
//        map_clear(sprite_00_translucent_group);
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glUseProgram(0);
}

static void sprite_init() {
    do_once {
    map_init(sprite_00_translucent_group, less_int, hash_int);
    map_init(sprite_translucent_group, less_int, hash_int);
    map_init(sprite_additive_group, less_int, hash_int);
    }
}

void sprite_flush() {
    profile("Sprite.rebuild_time") {
    sprite_rebuild_meshes();
    }
    profile("Sprite.render_time") {
    sprite_render_meshes();
    }
}

// -----------------------------------------------------------------------------
// tilemaps

tilemap_t tilemap(const char *map, int blank_chr, int linefeed_chr) {
    tilemap_t t = {0};
    t.tint = ~0u; // WHITE
    t.blank_chr = blank_chr;
    for( ; *map ; ++map ) {
        if( map[0] == linefeed_chr ) ++t.rows;
        else {
            array_push(t.map, map[0]);
            ++t.cols;
        }
    }
    return t;
}

void tilemap_render_ext( tilemap_t m, tileset_t t, float zindex, float xy_zoom[3], float tilt, unsigned tint, bool is_additive ) {
    vec3 old_pos = camera_get_active()->position;
    sprite_flush();
    camera_get_active()->position = vec3(window_width()/2,window_height()/2,1);

    float scale[2] = {xy_zoom[2], xy_zoom[2]};
    xy_zoom[2] = zindex;

    float offset[2] = {0,0};
    float spritesheet[3] = {0,t.cols,t.rows}; // selected tile index and spritesheet dimensions (cols,rows)

    for( unsigned y = 0, c = 0; y < m.rows; ++y ) {
        for( unsigned x = 0; x < m.cols; ++x, ++c ) {
            if( m.map[c] != m.blank_chr ) {
                spritesheet[0] = m.map[c];
                sprite_sheet(t.tex, spritesheet, xy_zoom, tilt, offset, scale, is_additive, tint, false);
            }
            offset[0] += t.tile_w;
        }
        offset[0] = 0, offset[1] += t.tile_h;
    }

    sprite_flush();
    camera_get_active()->position = old_pos;
}

void tilemap_render( tilemap_t map, tileset_t set ) {
    map.position.x += set.tile_w;
    map.position.y += set.tile_h;
    tilemap_render_ext( map, set, map.zindex, &map.position.x, map.tilt, map.tint, map.is_additive );
}

tileset_t tileset(texture_t tex, unsigned tile_w, unsigned tile_h, unsigned cols, unsigned rows) {
    tileset_t t = {0};
    t.tex = tex;
    t.cols = cols, t.rows = rows;
    t.tile_w = tile_w, t.tile_h = tile_h;
    return t;
}

int tileset_ui( tileset_t t ) {
    ui_subimage(va("Selection #%d (%d,%d)", t.selected, t.selected % t.cols, t.selected / t.cols), t.tex.id, t.tex.w, t.tex.h, (t.selected % t.cols) * t.tile_w, (t.selected / t.cols) * t.tile_h, t.tile_w, t.tile_h);
    int choice;
    if( (choice = ui_image(0, t.tex.id, t.tex.w,t.tex.h)) ) { 
        int px = ((choice / 100) / 100.f) * t.tex.w / t.tile_w;
        int py = ((choice % 100) / 100.f) * t.tex.h / t.tile_h;
        t.selected = px + py * t.cols;
    }
    // if( (choice = ui_buttons(3, "load", "save", "clear")) ) {}
    return t.selected;
}

// -----------------------------------------------------------------------------
// tiled

tiled_t tiled(const char *file_tmx) {
    tiled_t zero = {0}, ti = zero;

    // read file and parse json
    if( !xml_push(file_tmx) ) return zero;

    // sanity checks
    bool supported = !strcmp(xml_string("/map/@orientation"), "orthogonal") && !strcmp(xml_string("/map/@renderorder"), "right-down");
    if( !supported ) return xml_pop(), zero;

    // tileset
    const char *file_tsx = xml_string("/map/tileset/@source");
    if( !xml_push(vfs_read(file_tsx)) ) return zero;
        const char *set_src = xml_string("/tileset/image/@source");
        int set_w = xml_int("/tileset/@tilewidth");
        int set_h = xml_int("/tileset/@tileheight");
        int set_c = xml_int("/tileset/@columns");
        int set_r = xml_int("/tileset/@tilecount") / set_c;
        tileset_t set = tileset(texture(set_src,0), set_w, set_h, set_c, set_r );
    xml_pop();

    // actual parsing
    ti.w = xml_int("/map/@width");
    ti.h = xml_int("/map/@height");
    ti.tilew = xml_int("/map/@tilewidth");
    ti.tileh = xml_int("/map/@tileheight");
    ti.first_gid = xml_int("/map/tileset/@firstgid");
    ti.map_name = STRDUP( xml_string("/map/tileset/@source") ); // @leak

    for(int l = 0, layers = xml_count("/map/layer"); l < layers; ++l ) {
        if( strcmp(xml_string("/map/layer[%d]/data/@encoding",l), "base64") || strcmp(xml_string("/map/layer[%d]/data/@compression",l), "zlib") ) {
            PRINTF("Warning: layer encoding not supported: '%s' -> layer '%s'\n", file_tmx, *array_back(ti.names));
            continue;
        }

        int cols = xml_int("/map/layer[%d]/@width",l);
        int rows = xml_int("/map/layer[%d]/@height",l);

        tilemap_t tm = tilemap("", ' ', '\n');
        tm.blank_chr = ~0u; //ti.first_gid - 1;
        tm.cols = cols;
        tm.rows = rows;
        array_resize(tm.map, tm.cols * tm.rows);
        memset(tm.map, 0xFF, tm.cols * tm.rows * sizeof(int));

        for( int c = 0, chunks = xml_count("/map/layer[%d]/data/chunk", l); c <= chunks; ++c ) {
            int cw, ch;
            int cx, cy;
            array(char) b64 = 0;

            if( !chunks ) { // non-infinite mode
                b64 = xml_blob("/map/layer[%d]/data/$",l);
                cw = tm.cols, ch = tm.rows;
                cx = 0, cy = 0;
            } else { // infinite mode
                b64 = xml_blob("/map/layer[%d]/data/chunk[%d]/$",l,c);
                cw = xml_int("/map/layer[%d]/data/chunk[%d]/@width",l,c), ch = xml_int("/map/layer[%d]/data/chunk[%d]/@height",l,c); // 20x20
                cx = xml_int("/map/layer[%d]/data/chunk[%d]/@x",l,c), cy = xml_int("/map/layer[%d]/data/chunk[%d]/@y",l,c); // (-16,-32)
                cx = abs(cx), cy = abs(cy);
            }

            int outlen = cw * ch * 4;
            static __thread int *out = 0; out = (int *)REALLOC( 0, outlen + zexcess(COMPRESS_ZLIB) ); // @leak
            if( zdecode( out, outlen, b64, array_count(b64), COMPRESS_ZLIB ) > 0 ) {
                for( int y = 0, p = 0; y < ch; ++y ) {
                    for( int x = 0; x < cw; ++x, ++p ) {
                        if( out[p] >= ti.first_gid ) {
                            int offset = (x + cx) + (y + cy) * tm.cols;
                            if( offset >= 0 && offset < (cw * ch) )
                            tm.map[ offset ] = out[ p ] - ti.first_gid;
                        }
                    }
                }
            }
            else {
                PRINTF("Warning: bad zlib stream: '%s' -> layer #%d -> chunk #%d\n", file_tmx, l, c);
            }

            array_free(b64);
        }

        array_push(ti.layers, tm);
        array_push(ti.names, STRDUP(xml_string("/map/layer[%d]/@name",l)));
        array_push(ti.visible, true);
        array_push(ti.sets, set);
    }

    xml_pop();
    return ti;
}

void tiled_render(tiled_t tmx, vec3 pos) {
    for( unsigned i = 0, end = array_count(tmx.layers); i < end; ++i ) {
        tmx.layers[i].position = pos; // add3(camera_get_active()->position, pos);
        if( tmx.parallax ) tmx.layers[i].position.x /= (3+i), tmx.layers[i].position.y /= (5+i);
        if( tmx.visible[i] ) tilemap_render(tmx.layers[i], tmx.sets[i]);
    }
}

void tiled_ui(tiled_t *t) {
    ui_label2("Loaded map", t->map_name ? t->map_name : "(none)");
    ui_label2("Map dimensions", va("%dx%d", t->w, t->h));
    ui_label2("Tile dimensions", va("%dx%d", t->tilew, t->tileh));
    ui_separator();
    ui_bool("Parallax", &t->parallax);
    ui_separator();
    ui_label2("Layers", va("%d", array_count(t->layers)));
    for( int i = 0; i < array_count(t->layers); ++i ) {
        if( ui_label2_toolbar(va("- %s (%dx%d)", t->names[i], t->layers[i].cols, t->layers[i].rows ), t->visible[i] ? "\xee\xa3\xb4" : "\xee\xa3\xb5") > 0 ) { // ICON_MD_VISIBILITY / ICON_MD_VISIBILITY_OFF
            t->visible[i] ^= true;
        }
    }
    ui_separator();
    if( ui_collapse(va("Sets: %d", array_count(t->layers)), va("%p",t))) {
        for( int i = 0; i < array_count(t->layers); ++i ) {
            if( ui_collapse(va("%d", i+1), va("%p%d",t,i)) ) {
                t->sets[i].selected = tileset_ui( t->sets[i] );
                ui_collapse_end();
            }
        }
        ui_collapse_end();
    }
}

// -----------------------------------------------------------------------------
// spine json loader (wip)
// - rlyeh, public domain
//
// [ref] http://es.esotericsoftware.com/spine-json-format
//
// notable misses:
// - mesh deforms
// - cubic beziers
// - shears
// - bounding boxes

enum { SPINE_MAX_BONES = 64 }; // max bones

typedef struct spine_bone_t {
    char *name, *parent;
    struct spine_bone_t *parent_bone;

    float z; // draw order usually matches bone-id. ie, zindex == bone_id .. root(0) < chest (mid) < finger(top)

    float len;
    float x, y, deg;        // base
    float x2, y2, deg2;     // accum / temporaries during bone transform time
    float x3, y3, deg3;     // values from timeline

    unsigned rect_id;
    unsigned atlas_id;
} spine_bone_t;

typedef struct spine_slot_t {
    char *name, *bone, *attach;
} spine_slot_t;

typedef struct spine_rect_t {
    char *name;
    float x,y,w,h,sx,sy,deg;
} spine_rect_t;

typedef struct spine_skin_t {
    char *name;
    array(spine_rect_t) rects;
} spine_skin_t;

typedef struct spine_animkey_t { // offline; only during loading
    float time, curve[4];        // time is mandatory, curve is optional
    union {
        char *name;              // type: attachment (mode-1)
        struct { float deg; };   // type: rotate (mode-2)
        struct { float x,y; };   // type: translate (mode-3)
    };
} spine_animkey_t;

#if 0
typedef struct spine_pose_t { // runtime; only during playing
    unsigned frame;
    array(vec4) xform; // entry per bone. translation(x,y),rotation(z),attachment-id(w)
} spine_pose_t;
#endif

typedef struct spine_anim_t {
    char *name;
    union {
#if 0
        struct {
            unsigned frames;
            array(spine_pose_t) poses;
        };
#endif
        struct {
            array(spine_animkey_t) attach_keys[SPINE_MAX_BONES];
            array(spine_animkey_t) rotate_keys[SPINE_MAX_BONES];
            array(spine_animkey_t) translate_keys[SPINE_MAX_BONES];
        };
    };
} spine_anim_t;

typedef struct spine_atlas_t {
    char *name;
    float x,y,w,h,deg;
} spine_atlas_t;

typedef struct spine_t {
    char *name;
    texture_t texture;
    unsigned skin;
    array(spine_bone_t) bones;
    array(spine_slot_t) slots;
    array(spine_skin_t) skins;
    array(spine_anim_t) anims;
    array(spine_atlas_t) atlas;
    // anim controller
    unsigned inuse;
    float time, maxtime;
    unsigned debug_atlas_id;
} spine_t;

// ---

static
void spine_convert_animkeys_to_animpose(spine_anim_t *input) {
    spine_anim_t copy = *input; // @todo
    // @leak: attach/rot/tra keys
}

static
int find_bone_id(spine_t *s, const char *bone_name) {
    for( unsigned i = 0, end = array_count(s->bones); i < end; ++i )
        if( !strcmp(s->bones[i].name, bone_name)) return i;
    return -1;
}
static
spine_bone_t *find_bone(spine_t *s, const char *bone_name) {
    int bone_id = find_bone_id(s, bone_name);
    return bone_id >= 0 ? &s->bones[bone_id] : NULL;
}

void spine_skin(spine_t *p, unsigned skin) {
    if( !p->texture.id ) return;
    if( skin >= array_count(p->skins) ) return;

    p->skin = skin;

    char *skin_name = va("%s/", p->skins[skin].name);
    int header = strlen(skin_name);

    for( int i = 0; i < array_count(p->atlas); ++i) {
        if(!strbeg(p->atlas[i].name, skin_name)) continue;

        int bone_id = find_bone_id(p, p->atlas[i].name+header );
        if( bone_id < 0 ) continue;

        p->bones[bone_id].atlas_id = i;
    }

    for( int i = 0; i < array_count(p->skins[p->skin].rects); ++i) {
        int bone_id = find_bone_id(p, p->skins[p->skin].rects[i].name );
        if( bone_id < 0 ) continue;

        p->bones[bone_id].rect_id = i;
    }
}

static
bool spine_(spine_t *t, const char *file_json, const char *file_atlas, unsigned flags) {
    char *atlas = vfs_read(file_atlas);
    if(!atlas || !atlas[0]) return false;

    memset(t, 0, sizeof(spine_t));

    // goblins.png
    //   size: 1024, 128
    //   filter: Linear, Linear
    //   pma: true
    // dagger
    //   bounds: 2, 18, 26, 108
    // goblin/eyes-closed
    //   bounds: 2, 4, 34, 12
    spine_atlas_t *sa = 0;
    const char *last_id = 0;
    const char *texture_name = 0;
    const char *texture_filter = 0;
    const char *texture_format = 0;
    const char *texture_repeat = 0;
    float texture_width = 0, texture_height = 0, temp;
    for each_substring(atlas, "\r\n", it) {
        it += strspn(it, " \t\f\v");
        /**/ if( strbeg(it, "pma:" ) || strbeg(it, "index:") ) {} // ignored
        else if( strbeg(it, "size:" ) ) sscanf(it+5, "%f,%f", &texture_width, &texture_height);
        else if( strbeg(it, "rotate:" ) ) { float tmp; tmp=sa->w,sa->w=sa->h,sa->h=tmp; sa->deg = 90; } // assert(val==90)
        else if( strbeg(it, "repeat:" ) ) texture_repeat = it+7; // temp string
        else if( strbeg(it, "filter:" ) ) texture_filter = it+7; // temp string
        else if( strbeg(it, "format:" ) ) texture_format = it+7; // temp string
        else if( strbeg(it, "bounds:" ) ) {
            sscanf(it+7, "%f,%f,%f,%f", &sa->x, &sa->y, &sa->w, &sa->h);
        }
        else if( !texture_name ) texture_name = va("%s", it);
        else {
            array_push(t->atlas, ((spine_atlas_t){0}) );
            sa = &t->atlas[array_count(t->atlas) - 1];
            sa->name = STRDUP(it);
        }
    }
    for( int i = 0; i < array_count(t->atlas); ++i ) {
        sa = &t->atlas[i];
        sa->x /= texture_width, sa->y /= texture_height;
        sa->w /= texture_width, sa->h /= texture_height;
    }

    if(!texture_name) return false;

    t->texture = texture(texture_name, TEXTURE_LINEAR);

    json_push(vfs_read(file_json)); // @fixme: json_push_from_file() ?

    array_resize(t->bones, json_count("/bones"));
    array_reserve(t->slots, json_count("/slots"));
    array_resize(t->skins, json_count("/skins"));
    array_resize(t->anims, json_count("/animations"));

    for( int i = 0, end = json_count("/bones"); i < end; ++i ) {
        spine_bone_t v = {0};
        v.name = STRDUP(json_string("/bones[%d]/name", i));
        v.parent = STRDUP(json_string("/bones[%d]/parent", i));
        v.x = json_float("/bones[%d]/x", i);
        v.y = json_float("/bones[%d]/y", i);
        v.z = i;
        v.len = json_float("/bones[%d]/length", i);
        v.deg = json_float("/bones[%d]/rotation", i);
        t->bones[i] = v;

        for( int j = i-1; j > 0; --j ) {
            if( strcmp(t->bones[j].name,v.parent) ) continue;
            t->bones[i].parent_bone = &t->bones[j];
            break;
        }
    }

    for( int i = 0, end = json_count("/slots"); i < end; ++i ) {
        spine_slot_t v = {0};
        v.name = STRDUP(json_string("/slots[%d]/name", i));
        v.bone = STRDUP(json_string("/slots[%d]/bone", i));
        v.attach = STRDUP(json_string("/slots[%d]/attachment", i));

        array_push(t->slots, v);

        // slots define draw-order. so, update draw-order/zindex in bone
        spine_bone_t *b = find_bone(t, v.name);
        if( b ) b->z = i;
    }

    for( int i = 0, end = json_count("/skins"); i < end; ++i ) {
        spine_skin_t v = {0};
        v.name = STRDUP(json_string("/skins[%d]/name", i));

        for( int j = 0, jend = json_count("/skins[%d]/attachments",i); j < jend; ++j ) // /skins/default/
        for( int k = 0, kend = json_count("/skins[%d]/attachments[%d]",i,j); k < kend; ++k ) { // /skins/default/left hand item/
            spine_rect_t r = {0};
            r.name = STRDUP(json_key("/skins[%d]/attachments[%d][%d]",i,j,k)); // stringf("%s-%s-%s", json_key("/skins[%d]",i), json_key("/skins[%d][%d]",i,j), json_key("/skins[%d][%d][%d]",i,j,k));
            r.x = json_float("/skins[%d]/attachments[%d][%d]/x",i,j,k);
            r.y = json_float("/skins[%d]/attachments[%d][%d]/y",i,j,k);
            r.sx= json_float("/skins[%d]/attachments[%d][%d]/scaleX",i,j,k); r.sx += !r.sx;
            r.sy= json_float("/skins[%d]/attachments[%d][%d]/scaleY",i,j,k); r.sy += !r.sy;
            r.w = json_float("/skins[%d]/attachments[%d][%d]/width",i,j,k);
            r.h = json_float("/skins[%d]/attachments[%d][%d]/height",i,j,k);
            r.deg = json_float("/skins[%d]/attachments[%d][%d]/rotation",i,j,k);
            array_push(v.rects, r);
        }

        t->skins[i] = v;
    }

#if 1
    // simplify:
    // merge /skins/default into existing /skins/*, then delete /skins/default
    if( array_count(t->skins) > 1 ) {
        for( int i = 1; i < array_count(t->skins); ++i ) {
            for( int j = 0; j < array_count(t->skins[0].rects); ++j ) {
                array_push(t->skins[i].rects, t->skins[0].rects[j]);
            }
        }
        // @leak @fixme: free(t->skins[0])
        for( int i = 0; i < array_count(t->skins)-1; ++i ) {
            t->skins[i] = t->skins[i+1];
        }
        array_pop(t->skins);
    }
#endif

    for( int i = 0, end = json_count("/animations"); i < end; ++i ) {
        int id;
        const char *name;

        spine_anim_t v = {0};
        v.name = STRDUP(json_key("/animations[%d]", i));

        // slots / attachments

        for( int j = 0, jend = json_count("/animations[%d]/slots",i); j < jend; ++j )
        for( int k = 0, kend = json_count("/animations[%d]/slots[%d]",i,j); k < kend; ++k ) // ids
        {
            int bone_id = find_bone_id(t, json_key("/animations[%d]/bones[%d]",i,j));
            if( bone_id < 0 ) continue;

            for( int l = 0, lend = json_count("/animations[%d]/slots[%d][%d]",i,j,k); l < lend; ++l ) { // channels (rot,tra,attach)
                spine_animkey_t key = {0};

                key.name = STRDUP(json_string("/animations[%d]/slots[%d][%d][%d]/name",i,j,k,l));
                key.time = json_float("/animations[%d]/slots[%d][%d][%d]/time",i,j,k,l);
                if( json_count("/animations[%d]/slots[%d][%d][%d]/curve",i,j,k,l) == 4 ) {
                key.curve[0] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[0]",i,j,k,l);
                key.curve[1] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[1]",i,j,k,l);
                key.curve[2] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[2]",i,j,k,l);
                key.curve[3] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[3]",i,j,k,l);
                }

                // @todo: convert name to id
                // for(id = 0; t->bones[id].name && strcmp(t->bones[id].name,key.name); ++id)
                // printf("%s vs %s\n", key.name, t->bones[id].name);

                array_push(v.attach_keys[bone_id], key);
            }
        }

        // bones

        for( int j = 0, jend = json_count("/animations[%d]/bones",i); j < jend; ++j ) // slots or bones
        for( int k = 0, kend = json_count("/animations[%d]/bones[%d]",i,j); k < kend; ++k ) { // bone ids
            int bone_id = find_bone_id(t, json_key("/animations[%d]/bones[%d]",i,j));
            if( bone_id < 0 ) continue;

            // parse bones
            for( int l = 0, lend = json_count("/animations[%d]/bones[%d][%d]",i,j,k); l < lend; ++l ) { // channels (rot,tra,attach)
                const char *channel = json_key("/animations[%d]/bones[%d][%d]",i,j,k);
                int track = !strcmp(channel, "rotate") ? 1 : !strcmp(channel, "translate") ? 2 : 0;
                if( !track ) continue;

                spine_animkey_t key = {0};

                key.time = json_float("/animations[%d]/bones[%d][%d][%d]/time",i,j,k,l);
                if( json_count("/animations[%d]/bones[%d][%d][%d]/curve",i,j,k,l) == 4 ) {
                key.curve[0] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[0]",i,j,k,l);
                key.curve[1] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[1]",i,j,k,l);
                key.curve[2] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[2]",i,j,k,l);
                key.curve[3] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[3]",i,j,k,l);
                }

                if( track == 1 )
                key.deg = json_float("/animations[%d]/bones[%d][%d][%d]/value",i,j,k,l), // "/angle"
                array_push(v.rotate_keys[bone_id], key);
                else
                key.x = json_float("/animations[%d]/bones[%d][%d][%d]/x",i,j,k,l),
                key.y = json_float("/animations[%d]/bones[%d][%d][%d]/y",i,j,k,l),
                array_push(v.translate_keys[bone_id], key);
            }
        }

        t->anims[i] = v;
    }

    json_pop();

    spine_skin(t, 0);

    return true;
}

spine_t* spine(const char *file_json, const char *file_atlas, unsigned flags) {
    spine_t *t = MALLOC(sizeof(spine_t));
    if( !spine_(t, file_json, file_atlas, flags) ) return FREE(t), NULL;
    return t;
}

void spine_render(spine_t *p, vec3 offset, unsigned flags) {
    if( !p->texture.id ) return;
    if( !flags ) return;

    ddraw_push_2d();
        // if( flags & 2 ) ddraw_line(vec3(0,0,0), vec3(window_width(),window_height(),0));
        // if( flags & 2 ) ddraw_line(vec3(window_width(),0,0), vec3(0,window_height(),0));

        // int already_computed[SPINE_MAX_BONES] = {0}; // @fixme: optimize: update longest chains first, then remnant branches

        for( int i = 1; i < array_count(p->bones); ++i ) {
            spine_bone_t *self = &p->bones[i];
            if( !self->rect_id ) continue;

            int num_bones = 0;
            static array(spine_bone_t*) chain = 0; array_resize(chain, 0);
            for( spine_bone_t *next = self; next ; next = next->parent_bone, ++num_bones ) {
                array_push(chain, next);
            }

            vec3 target = {0}, prev = {0};
            for( int j = 0, end = array_count(chain); j < end; ++j ) { // traverse from root(skipped) -> `i` bone direction
                int j_opposite = end - 1 - j;

                spine_bone_t *b = chain[j_opposite]; // bone
                spine_bone_t *pb = b->parent_bone; // parent bone

                float pb_x2 = 0, pb_y2 = 0, pb_deg2 = 0;
                if( pb ) pb_x2 = pb->x2, pb_y2 = pb->y2, pb_deg2 = pb->deg2;

                const float deg2rad = C_PI / 180;
                b->x2 =      b->x3 + pb_x2   + b->x * cos( -pb_deg2 * deg2rad ) - b->y * sin( -pb_deg2 * deg2rad );
                b->y2 =     -b->y3 + pb_y2   - b->y * cos(  pb_deg2 * deg2rad ) + b->x * sin(  pb_deg2 * deg2rad );
                b->deg2 = -b->deg3 + pb_deg2 - b->deg;

                prev = target;
                target = vec3(b->x2,b->y2,b->deg2);
            }

            target.z = 0;
            target = add3(target, offset);
            prev.z = 0;
            prev = add3(prev, offset);

            if( flags & 2 ) {
                ddraw_point( target );
                ddraw_text( target, -0.25f, self->name );
                ddraw_bone( prev, target ); // from parent to bone
            }
            if( flags & 1 ) {
                spine_atlas_t *a = &p->atlas[self->atlas_id];
                spine_rect_t *r = &p->skins[p->skin].rects[self->rect_id];

                vec4 rect = ptr4(&a->x);
                float zindex = self->z;
                float offsx = 0;
                float offsy = 0;
                float tilt = self->deg2 + (a->deg - r->deg);
                unsigned tint = self->atlas_id == p->debug_atlas_id ? 0xFF<<24 | 0xFF : ~0u;

                if( 1 ) {
                    vec3 dir = vec3(r->x,r->y,0);
                    dir = rotatez3(dir, self->deg2);
                    offsx = dir.x * r->sx;
                    offsy = dir.y * r->sy;
                }

                sprite_rect(p->texture, rect, zindex, add3(vec3(target.x,target.y,1),vec3(offsx,offsy,0)), tilt, tint);
            }
         }

    ddraw_pop_2d();
    ddraw_flush();
}

static
void spine_animate_(spine_t *p, float *time, float *maxtime, float delta) {
    if( !p->texture.id ) return;

    if( delta > 1/120.f ) delta = 1/120.f;
    if( *time >= *maxtime ) *time = 0; else *time += delta;

    // reset root // needed?
    p->bones[0].x2 = 0;
    p->bones[0].y2 = 0;
    p->bones[0].deg2 = 0;
    p->bones[0].x3 = 0;
    p->bones[0].y3 = 0;
    p->bones[0].deg3 = 0;

    for( int i = 0, end = array_count(p->bones); i < end; ++i) {
        // @todo: attach channel
        // @todo: per channel: if curve == linear || curve == stepped || array_count(curve) == 4 {...}
        for each_array_ptr(p->anims[p->inuse].rotate_keys[i], spine_animkey_t, r) {
            double r0 = r->time;
            *maxtime = maxf( *maxtime, r0 );
            if( absf(*time - r0) < delta ) {
                p->bones[i].deg3 = r->deg;
            }
        }
        for each_array_ptr(p->anims[p->inuse].translate_keys[i], spine_animkey_t, r) {
            double r0 = r->time;
            *maxtime = maxf( *maxtime, r0 );
            if( absf(*time - r0) < delta ) {
                p->bones[i].x3 = r->x;
                p->bones[i].y3 = r->y;
            }
        }
    }
}

void spine_animate(spine_t *p, float delta) {
    spine_animate_(p, &p->time, &p->maxtime, delta);
}

void spine_ui(spine_t *p) {
    if( ui_collapse(va("Anims: %d", array_count(p->anims)), va("%p-a", p))) {
        for each_array_ptr(p->anims, spine_anim_t, q) {
            if(ui_slider2("", &p->time, va("%.2f/%.0f %.2f%%", p->time, p->maxtime, p->time * 100.f))) {
                spine_animate(p, 0);
            }

            int choice = ui_label2_toolbar(q->name, ICON_MD_PAUSE_CIRCLE " " ICON_MD_PLAY_CIRCLE);
            if( choice == 1 ) window_pause( 0 ); // play
            if( choice == 2 ) window_pause( 1 ); // pause

            for( int i = 0; i < SPINE_MAX_BONES; ++i ) {
                ui_separator();
                ui_label(va("Bone %d: Attachment keys", i));
                for each_array_ptr(q->attach_keys[i], spine_animkey_t, r) {
                    ui_label(va("%.2f [%.2f %.2f %.2f %.2f] %s", r->time, r->curve[0], r->curve[1], r->curve[2], r->curve[3], r->name));
                }
                ui_label(va("Bone %d: Rotate keys", i));
                for each_array_ptr(q->rotate_keys[i], spine_animkey_t, r) {
                    ui_label(va("%.2f [%.2f %.2f %.2f %.2f] %.2f deg", r->time, r->curve[0], r->curve[1], r->curve[2], r->curve[3], r->deg));
                }
                ui_label(va("Bone %d: Translate keys", i));
                for each_array_ptr(q->translate_keys[i], spine_animkey_t, r) {
                    ui_label(va("%.2f [%.2f %.2f %.2f %.2f] (%.2f,%.2f)", r->time, r->curve[0], r->curve[1], r->curve[2], r->curve[3], r->x, r->y));
                }
            }
        }
        ui_collapse_end();
    }
    if( ui_collapse(va("Bones: %d", array_count(p->bones)), va("%p-b", p))) {
        for each_array_ptr(p->bones, spine_bone_t, q)
        if( ui_collapse(q->name, va("%p-b2", q)) ) {
            ui_label2("Parent:", q->parent);
            ui_label2("X:", va("%.2f", q->x));
            ui_label2("Y:", va("%.2f", q->y));
            ui_label2("Length:", va("%.2f", q->len));
            ui_label2("Rotation:", va("%.2f", q->deg));
            ui_collapse_end();
        }
        ui_collapse_end();
    }
    if( ui_collapse(va("Slots: %d", array_count(p->slots)), va("%p-s", p))) {
        for each_array_ptr(p->slots, spine_slot_t, q)
        if( ui_collapse(q->name, va("%p-s2", q)) ) {
            ui_label2("Bone:", q->bone);
            ui_label2("Attachment:", q->attach);
            ui_collapse_end();
        }
        ui_collapse_end();
    }
    if( ui_collapse(va("Skins: %d", array_count(p->skins)), va("%p-k", p))) {
        for each_array_ptr(p->skins, spine_skin_t, q)
        if( ui_collapse(q->name, va("%p-k2", q)) ) {
            for each_array_ptr(q->rects, spine_rect_t, r)
            if( ui_collapse(r->name, va("%p-k3", r)) ) {
                ui_label2("X:", va("%.2f", r->x));
                ui_label2("Y:", va("%.2f", r->y));
                ui_label2("Scale X:", va("%.2f", r->sx));
                ui_label2("Scale Y:", va("%.2f", r->sy));
                ui_label2("Width:", va("%.2f", r->w));
                ui_label2("Height:", va("%.2f", r->h));
                ui_label2("Rotation:", va("%.2f", r->deg));
                ui_collapse_end();

                spine_bone_t *b = find_bone(p, r->name);
                if( b ) {
                    p->debug_atlas_id = b->atlas_id;

                    static float tilt = 0;
                    if( input(KEY_LCTRL) ) tilt += 60*1/60.f; else tilt = 0;
                    spine_atlas_t *r = p->atlas + b->atlas_id;
                    sprite_flush();
                    camera_get_active()->position = vec3(0,0,2);
                            vec4 rect = ptr4(&r->x); float zindex = 0; vec3 xy_zoom = vec3(0,0,0); unsigned tint = ~0u;
                            sprite_rect(p->texture,
                                // rect: vec4(r->x*1.0/p->texture.w,r->y*1.0/p->texture.h,(r->x+r->w)*1.0/p->texture.w,(r->y+r->h)*1.0/p->texture.h),
                                ptr4(&r->x), // atlas
                                0, vec3(0,0,0), r->deg + tilt, tint);
                            sprite_flush();
                    camera_get_active()->position = vec3(+window_width()/3,window_height()/2.25,2);
                }
            }
            ui_collapse_end();
        }
        ui_collapse_end();
    }

    if( ui_int("Use skin", &p->skin) ) {
    p->skin = clampf(p->skin, 0, array_count(p->skins) - 1);
    spine_skin(p, p->skin);
    }

    if( p->texture.id ) ui_texture(0, p->texture);
}

// -----------------------------------------------------------------------------
// cubemaps

// project cubemap coords into sphere normals
static
vec3 cubemap2polar(int face, int x, int y, int texture_width) {
    float u = (x / (texture_width - 1.f)) * 2 - 1;
    float v = (y / (texture_width - 1.f)) * 2 - 1;
    /**/ if( face == 0 ) return vec3( u, -1, -v);
    else if( face == 1 ) return vec3(-v, -u,  1);
    else if( face == 2 ) return vec3(-1, -u, -v);
    else if( face == 3 ) return vec3(-u,  1, -v);
    else if( face == 4 ) return vec3( v, -u, -1);
    else                 return vec3( 1,  u, -v);
}
// project normal in a sphere as 2d texcoord
static
vec2 polar2uv(vec3 n) {
    n = norm3(n);
    float theta = atan2(n.y, n.x);
    float phi = atan2(n.z, hypot(n.x, n.y));
    float u = (theta + C_PI) / C_PI;
    float v = (C_PI/2 - phi) / C_PI;
    return vec2(u, v);
}

// equirectangular panorama (2:1) to cubemap - in RGB, out RGB
static
void panorama2cubemap_(image_t out[6], const image_t in, int width){
    int face;
    #pragma omp parallel for
    for( face = 0; face < 6; ++face ) {
        out[face] = image_create(width, width, IMAGE_RGB);
        for (int j=0; j < width; ++j) {
            uint32_t *line = &out[ face ].pixels32[ 0 + j * width ];
            for (int i=0; i < width; ++i) {
                vec3 polar = cubemap2polar(face, i, j, width);
                vec2 uv = polar2uv(polar);
                uv = scale2(uv, in.h-1); // source coords (assumes 2:1, 2*h == w)
                vec3 rgb = bilinear(in, uv);
                union color {
                    struct { uint8_t r,g,b,a; };
                    uint32_t rgba;
                } c = { rgb.x, rgb.y, rgb.z, 255 };
                line[i] = c.rgba;
            }
        }
    }
}
// equirectangular panorama (2:1) to cubemap - in RGB, out RGBA
void panorama2cubemap(image_t out[6], const image_t in, int width) {
    int face;
    #pragma omp parallel for
    for( face = 0; face < 6; ++face ) {
        out[face] = image_create(width, width, IMAGE_RGBA);
        for (int j=0; j < width; ++j) {
            uint32_t *line = &out[ face ].pixels32[ 0 + j * width ];
            for (int i=0; i < width; ++i) {
                vec3 polar = cubemap2polar(face, i, j, width);
                vec2 uv = polar2uv(polar);
                uv = scale2(uv, in.h-1); // source coords (assumes 2:1, 2*h == w)
                vec3 rgb = bilinear(in, uv);
                union color {
                    struct { uint8_t r,g,b,a; };
                    uint32_t rgba;
                } c = { rgb.x, rgb.y, rgb.z, 255 };
                line[i] = c.rgba;
            }
        }
    }
}


cubemap_t cubemap6( const image_t images[6], int flags ) {
    cubemap_t c = {0}, z = {0};

    glGenTextures(1, &c.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, c.id);

    int samples = 0;
    for (int i = 0; i < 6; i++) {
        image_t img = images[i]; //image(textures[i], IMAGE_RGB);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, img.w, img.h, 0, img.n == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);

        // calculate SH coefficients (@ands)
        const vec3 skyDir[] = {{ 1, 0, 0},{-1, 0, 0},{ 0, 1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1}};
        const vec3 skyX[]   = {{ 0, 0,-1},{ 0, 0, 1},{ 1, 0, 0},{ 1, 0, 0},{ 1, 0, 0},{-1, 0, 0}};
        const vec3 skyY[]   = {{ 0, 1, 0},{ 0, 1, 0},{ 0, 0,-1},{ 0, 0, 1},{ 0, 1, 0},{ 0, 1, 0}};
        int step = 16;
        for (int y = 0; y < img.h; y += step) {
            unsigned char *p = (unsigned char*)img.pixels + y * img.w * img.n;
            for (int x = 0; x < img.w; x += step) {
                vec3 n = add3(
                    add3(
                        scale3(skyX[i],  2.0f * (x / (img.w - 1.0f)) - 1.0f),
                        scale3(skyY[i], -2.0f * (y / (img.h - 1.0f)) + 1.0f)),
                    skyDir[i]); // texelDirection;
                float l = len3(n);
                vec3 light = scale3(vec3(p[0], p[1], p[2]), 1 / (255.0f * l * l * l)); // texelSolidAngle * texel_radiance;
                n = norm3(n);
                c.sh[0] = add3(c.sh[0], scale3(light,  0.282095f));
                c.sh[1] = add3(c.sh[1], scale3(light, -0.488603f * n.y * 2.0 / 3.0));
                c.sh[2] = add3(c.sh[2], scale3(light,  0.488603f * n.z * 2.0 / 3.0));
                c.sh[3] = add3(c.sh[3], scale3(light, -0.488603f * n.x * 2.0 / 3.0));
                c.sh[4] = add3(c.sh[4], scale3(light,  1.092548f * n.x * n.y / 4.0));
                c.sh[5] = add3(c.sh[5], scale3(light, -1.092548f * n.y * n.z / 4.0));
                c.sh[6] = add3(c.sh[6], scale3(light,  0.315392f * (3.0f * n.z * n.z - 1.0f) / 4.0));
                c.sh[7] = add3(c.sh[7], scale3(light, -1.092548f * n.x * n.z / 4.0));
                c.sh[8] = add3(c.sh[8], scale3(light,  0.546274f * (n.x * n.x - n.y * n.y) / 4.0));
                p += img.n * step;
                samples++;
            }
        }
    }

    for (int s = 0; s < 9; s++) {
        c.sh[s] = scale3(c.sh[s], 32.f / samples);
    }

    // if( glGenerateMipmap )
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, /* glGenerateMipmap ?*/ GL_LINEAR_MIPMAP_LINEAR /*: GL_LINEAR*/);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return c;
}

cubemap_t cubemap( const image_t in, int flags ) {
    ASSERT( in.n == 4 );
    image_t out[6];
    panorama2cubemap(out, in, in.h);
    image_t swap[6] = { out[0],out[3],out[1],out[4],out[2],out[5] };
    cubemap_t c = cubemap6(swap, flags);
    int i;
    #pragma omp parallel for
    for( i = 0; i < 6; ++i) image_destroy(&out[i]);
    return c;
}

void cubemap_destroy(cubemap_t *c) {
    glDeleteTextures(1, &c->id);
    c->id = 0; // do not destroy SH coefficients still. they might be useful in the future.
}

static cubemap_t *last_cubemap;

cubemap_t* cubemap_get_active() {
    return last_cubemap;
}

// -----------------------------------------------------------------------------
// skyboxes

skybox_t skybox(const char *asset, int flags) {
    skybox_t sky = {0};

    // sky mesh
    vec3 vertices[] = {{+1,-1,+1},{+1,+1,+1},{+1,+1,-1},{-1,+1,-1},{+1,-1,-1},{-1,-1,-1},{-1,-1,+1},{-1,+1,+1}};
    unsigned indices[] = { 0, 1, 2, 3, 4, 5, 6, 3, 7, 1, 6, 0, 4, 2 };
    mesh_update(&sky.geometry, "p3", 0,countof(vertices),vertices, countof(indices),indices, MESH_TRIANGLE_STRIP);

    // sky program
    sky.flags = flags ? flags : !!asset; // either cubemap or rayleigh
    sky.program = shader(vfs_read("shaders/vs_3_3_skybox.glsl"),
        sky.flags ? vfs_read("fs_3_4_skybox.glsl") : vfs_read("shaders/fs_3_4_skybox_rayleigh.glsl"),
        "att_position", "fragcolor", NULL);

    // sky cubemap & SH
    if( asset ) {
        int is_panorama = vfs_size( asset );
        if( is_panorama ) {
            stbi_hdr_to_ldr_gamma(1.2f);
            image_t panorama = image( asset, IMAGE_RGBA );
            sky.cubemap = cubemap( panorama, 0 ); // RGBA required
            image_destroy(&panorama);
        } else { // is folder
            image_t images[6] = {0};
            images[0] = image( va("%s/posx", asset), IMAGE_RGB ); // cubepx
            images[1] = image( va("%s/negx", asset), IMAGE_RGB ); // cubenx
            images[2] = image( va("%s/posy", asset), IMAGE_RGB ); // cubepy
            images[3] = image( va("%s/negy", asset), IMAGE_RGB ); // cubeny
            images[4] = image( va("%s/posz", asset), IMAGE_RGB ); // cubepz
            images[5] = image( va("%s/negz", asset), IMAGE_RGB ); // cubenz
            sky.cubemap = cubemap6( images, 0 );
            for( int i = 0; i < countof(images); ++i ) image_destroy(&images[i]);
        }
    } else {
        // set up mie defaults
        shader_bind(sky.program);
        shader_vec3("uSunPos", vec3( 0, 0.1, -1 ));
        shader_vec3("uRayOrigin", vec3(0.0, 6372000.0, 0.0));
        shader_float("uSunIntensity", 22.0);
        shader_float("uPlanetRadius", 6371000.0);
        shader_float("uAtmosphereRadius", 6471000.0);
        shader_vec3("uRayleighScattering", vec3(5.5e-6, 13.0e-6, 22.4e-6));
        shader_float("uMieScattering", 21e-6);
        shader_float("uRayleighScaleHeight", 8000.0);
        shader_float("uMieScaleHeight", 1200.0);
        shader_float("uMiePreferredDirection", 0.758);
        skybox_mie_calc_sh(&sky, 1.2);
    }

    return sky;
}

void skybox_mie_calc_sh(skybox_t *sky, float sky_intensity) {
    unsigned WIDTH = 1024, HEIGHT = 1024;
    int last_fb;
    int vp[4];
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &last_fb);
    glGetIntegerv(GL_VIEWPORT, vp);

    if (!sky_intensity) {
        sky_intensity = 1.0f;
    }

    if (!sky->pixels)
        sky->pixels = MALLOC(WIDTH*HEIGHT*12);

    if (!sky->framebuffers[0]) {
        for(int i = 0; i < 6; ++i) {
            glGenFramebuffers(1, &sky->framebuffers[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, sky->framebuffers[i]);
            
            glGenTextures(1, &sky->textures[i]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sky->textures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sky->textures[i], 0);
        }
    }

    static vec3 directions[6] = {{ 1, 0, 0},{-1, 0, 0},{ 0, 1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1}};

    int samples = 0;
    for(int i = 0; i < 6; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, sky->framebuffers[i]);
        glViewport(0, 0, WIDTH, HEIGHT);
        glUseProgram(sky->program);

        mat44 proj; perspective44(proj, 90.0f, WIDTH / (float)HEIGHT, 0.1f, 500.f);
        mat44 view; lookat44(view, vec3(0,0,0), directions[i], vec3(0,-1,0));

        skybox_render(sky, proj, view);

        glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_FLOAT, sky->pixels);

        // calculate SH coefficients (@ands)
        // copied from cubemap6 method
        const vec3 skyDir[] = {{ 1, 0, 0},{-1, 0, 0},{ 0, 1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1}};
        const vec3 skyX[]   = {{ 0, 0,-1},{ 0, 0, 1},{ 1, 0, 0},{ 1, 0, 0},{ 1, 0, 0},{-1, 0, 0}};
        const vec3 skyY[]   = {{ 0, 1, 0},{ 0, 1, 0},{ 0, 0,-1},{ 0, 0, 1},{ 0, 1, 0},{ 0, 1, 0}};
        int step = 16;
        for (int y = 0; y < HEIGHT; y += step) {
            float *p = (float*)(sky->pixels + y * WIDTH * 3);
            for (int x = 0; x < WIDTH; x += step) {
                vec3 n = add3(
                    add3(
                        scale3(skyX[i],  2.0f * (x / (WIDTH - 1.0f)) - 1.0f),
                        scale3(skyY[i], -2.0f * (y / (HEIGHT - 1.0f)) + 1.0f)),
                    skyDir[i]); // texelDirection;
                float l = len3(n);
                vec3 light = scale3(vec3(p[0], p[1], p[2]), (1 / (l * l * l)) * sky_intensity); // texelSolidAngle * texel_radiance;
                n = norm3(n);
                sky->cubemap.sh[0] = add3(sky->cubemap.sh[0], scale3(light,  0.282095f));
                sky->cubemap.sh[1] = add3(sky->cubemap.sh[1], scale3(light, -0.488603f * n.y * 2.0 / 3.0));
                sky->cubemap.sh[2] = add3(sky->cubemap.sh[2], scale3(light,  0.488603f * n.z * 2.0 / 3.0));
                sky->cubemap.sh[3] = add3(sky->cubemap.sh[3], scale3(light, -0.488603f * n.x * 2.0 / 3.0));
                sky->cubemap.sh[4] = add3(sky->cubemap.sh[4], scale3(light,  1.092548f * n.x * n.y / 4.0));
                sky->cubemap.sh[5] = add3(sky->cubemap.sh[5], scale3(light, -1.092548f * n.y * n.z / 4.0));
                sky->cubemap.sh[6] = add3(sky->cubemap.sh[6], scale3(light,  0.315392f * (3.0f * n.z * n.z - 1.0f) / 4.0));
                sky->cubemap.sh[7] = add3(sky->cubemap.sh[7], scale3(light, -1.092548f * n.x * n.z / 4.0));
                sky->cubemap.sh[8] = add3(sky->cubemap.sh[8], scale3(light,  0.546274f * (n.x * n.x - n.y * n.y) / 4.0));
                p += 3 * step;
                samples++;
            }
        }
    }

    for (int s = 0; s < 9; s++) {
        sky->cubemap.sh[s] = scale3(sky->cubemap.sh[s], 32.f / samples);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, last_fb);
    glViewport(vp[0], vp[1], vp[2], vp[3]);
}

void skybox_sh_reset(skybox_t *sky) {
    for (int s = 0; s < 9; s++) {
        sky->cubemap.sh[s] = vec3(0,0,0);
    }    
}

void skybox_sh_add_light(skybox_t *sky, vec3 light, vec3 dir, float strength) {
    // Normalize the direction
    vec3 norm_dir = norm3(dir);

    // Scale the light color and intensity
    vec3 scaled_light = scale3(light, strength);

    // Add light to the SH coefficients
    sky->cubemap.sh[0] = add3(sky->cubemap.sh[0], scale3(scaled_light,  0.282095f));
    sky->cubemap.sh[1] = add3(sky->cubemap.sh[1], scale3(scaled_light, -0.488603f * norm_dir.y));
    sky->cubemap.sh[2] = add3(sky->cubemap.sh[2], scale3(scaled_light,  0.488603f * norm_dir.z));
    sky->cubemap.sh[3] = add3(sky->cubemap.sh[3], scale3(scaled_light, -0.488603f * norm_dir.x));
}

API vec4 window_getcolor_(); // internal use, not public

int skybox_push_state(skybox_t *sky, mat44 proj, mat44 view) {
    last_cubemap = &sky->cubemap;

    //glClear(GL_DEPTH_BUFFER_BIT);
    //glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // we have to reset clear color here, because of wrong alpha compositing issues on native transparent windows otherwise
    vec4 bgcolor = window_getcolor_(); glClearColor(bgcolor.r, bgcolor.g, bgcolor.b, 1); // @transparent

    mat44 mvp; multiply44x2(mvp, proj, view);

    //glDepthMask(GL_FALSE);
    shader_bind(sky->program);
    shader_mat44("u_mvp", mvp);
    if( sky->flags ) {
        shader_cubemap("u_cubemap", sky->cubemap.id);
    }
    return 0; // @fixme: return sortable hash here?
}
int skybox_pop_state() {
    //vec4 bgcolor = window_getcolor_(); glClearColor(bgcolor.r, bgcolor.g, bgcolor.b, window_has_transparent() ? 0 : bgcolor.a); // @transparent
    //glDepthMask(GL_TRUE);
    //glClear(GL_DEPTH_BUFFER_BIT);
    return 0;
}
int skybox_render(skybox_t *sky, mat44 proj, mat44 view) {
    skybox_push_state(sky, proj, view);
    glEnable(GL_DEPTH_TEST);
    mesh_render(&sky->geometry);
    skybox_pop_state();
    return 0;
}
void skybox_destroy(skybox_t *sky) {
    glDeleteProgram(sky->program);
    cubemap_destroy(&sky->cubemap);
    mesh_destroy(&sky->geometry);

    if (sky->pixels) {
        FREE(sky->pixels);
        glDeleteFramebuffers(6, sky->framebuffers);
        glDeleteTextures(6, sky->textures);
    }
}

// -----------------------------------------------------------------------------
// meshes

mesh_t mesh() {
    mesh_t z = {0};
    return z;
}

aabb mesh_bounds(mesh_t *m) {
    aabb b = {{1e9,1e9,1e9},{-1e9,-1e9,-1e9}};
    for( int i = 0; i < array_count(m->in_vertex3); ++i ) {
        if( m->in_vertex3[i].x < b.min.x ) b.min.x = m->in_vertex3[i].x;
        if( m->in_vertex3[i].x > b.max.x ) b.max.x = m->in_vertex3[i].x;

        if( m->in_vertex3[i].y < b.min.y ) b.min.y = m->in_vertex3[i].y;
        if( m->in_vertex3[i].y > b.max.y ) b.max.y = m->in_vertex3[i].y;

        if( m->in_vertex3[i].z < b.min.z ) b.min.z = m->in_vertex3[i].z;
        if( m->in_vertex3[i].z > b.max.z ) b.max.z = m->in_vertex3[i].z;
    }
    return b;
}

void mesh_update(mesh_t *m, const char *format, int vertex_stride,int vertex_count,const void *vertex_data, int index_count,const void *index_data, int flags) {
    m->flags = flags;

    // setup
    unsigned sizeof_index = sizeof(GLuint);
    unsigned sizeof_vertex = 0;
    m->index_count = index_count;
    m->vertex_count = vertex_count;

    // iterate vertex attributes { position, normal + uv + tangent + bitangent + ... }
    struct vertex_descriptor {
        int vertex_type, num_attribute, num_components, alt_normalized;
        int stride, offset;
    } descriptor[16] = {0}, *dc = &descriptor[0];

    do switch( *format ) {
        break; case '*': dc->alt_normalized = 1;
        break; case '0': dc->num_components = 0;
        break; case '1': dc->num_components = 1;
        break; case '2': dc->num_components = 2;
        break; case '3': dc->num_components = 3;
        break; case '4': dc->num_components = 4;
        break; case 'F': dc->vertex_type = GL_FLOAT;
        break; case 'U': case 'I': dc->vertex_type = GL_UNSIGNED_INT;
        break; case 'B': if(format[-1] >= '0' && format[-1] <= '9') dc->vertex_type = GL_UNSIGNED_BYTE; //else bitangent.
        break; case ' ': while (format[1] == ' ') format++; case '\0':
            if (!dc->vertex_type) dc->vertex_type = GL_FLOAT;
            dc->offset = sizeof_vertex;
            sizeof_vertex += (dc->stride = dc->num_components * (dc->vertex_type == GL_UNSIGNED_BYTE ? 1 : 4));
            ++dc;
        break; default: if( !strchr("pntbcwai", *format) ) PANIC("unsupported vertex type '%c'", *format);
    } while (*format++);

    if(vertex_stride > 0) sizeof_vertex = vertex_stride;

    // layout
    if(!m->vao) glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);

    // index data
    if( index_data && index_count ) {
        m->index_count = index_count;

        if(!m->ibo) glGenBuffers(1, &m->ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->index_count * sizeof_index, index_data, flags & MESH_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
    }

    // vertex data
    if( vertex_data && vertex_count ) {
        m->vertex_count = vertex_count;

        if(!m->vbo) glGenBuffers(1, &m->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
        glBufferData(GL_ARRAY_BUFFER, m->vertex_count * sizeof_vertex, vertex_data, flags & MESH_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
    }

    for( int i = 0; i < 8; ++i ) {
//        glDisableVertexAttribArray(i);
    }

    // vertex setup: iterate descriptors
    for( int i = 0; i < countof(descriptor); ++i ) {
        if( descriptor[i].num_components ) {
            glDisableVertexAttribArray(i);
            glVertexAttribPointer(i,
                descriptor[i].num_components, descriptor[i].vertex_type, (descriptor[i].vertex_type == GL_UNSIGNED_BYTE ? GL_TRUE : GL_FALSE) ^ (descriptor[i].alt_normalized ? GL_TRUE : GL_FALSE),
                sizeof_vertex, (GLchar*)NULL + descriptor[i].offset);
            glEnableVertexAttribArray(i);
        } else {
            glDisableVertexAttribArray(i);
        }
    }

    glBindVertexArray(0);
}

void mesh_render(mesh_t *sm) {
    if( sm->vao ) {
        glBindVertexArray(sm->vao);
        if( sm->ibo ) { // with indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm->ibo); // <-- why intel?
            glDrawElements(sm->flags & MESH_TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, sm->index_count, GL_UNSIGNED_INT, (char*)0);
            profile_incstat("Render.num_drawcalls", +1);
            profile_incstat("Render.num_triangles", sm->index_count/3);
        } else { // with vertices only
            glDrawArrays(sm->flags & MESH_TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, sm->vertex_count /* / 3 */);
            profile_incstat("Render.num_drawcalls", +1);
            profile_incstat("Render.num_triangles", sm->vertex_count/3);
        }
    }
}

void mesh_render_prim(mesh_t *sm, unsigned prim) {
    if( sm->vao ) {
        glBindVertexArray(sm->vao);
        if( sm->ibo ) { // with indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm->ibo); // <-- why intel?
            glDrawElements(prim, sm->index_count, GL_UNSIGNED_INT, (char*)0);
            profile_incstat("Render.num_drawcalls", +1);
            profile_incstat("Render.num_triangles", sm->index_count/3);
        } else { // with vertices only
            glDrawArrays(prim, 0, sm->vertex_count /* / 3 */);
            profile_incstat("Render.num_drawcalls", +1);
            profile_incstat("Render.num_triangles", sm->vertex_count/3);
        }
    }
}

void mesh_destroy(mesh_t *m) {
    // @todo
    (void)m;
}

// -----------------------------------------------------------------------------
// screenshots

void* screenshot( int n ) { // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA
    // sync, 10 ms -- pixel perfect

    int w = window_width(), h = window_height();
    int mode = n == 3 ? GL_RGB : n == -3 ? GL_BGR : n == 4 ? GL_RGBA : GL_BGRA;
    static __thread uint8_t *pixels = 0;
    pixels = (uint8_t*)REALLOC(pixels, w * h * 4 ); // @leak per thread

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0); // disable any pbo, in case somebody did for us
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, pixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    return pixels;
}

void* screenshot_async( int n ) { // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA
#if is(ems)
    return screenshot(n); // no glMapBuffer() on emscripten
#else
    // async, 0 ms -- @fixme: MSAA can cause some artifacts with PBOs: either use glDisable(GL_MULTISAMPLE) when recording or do not create window with WINDOW_MSAAx options at all.

    int w = window_width(), h = window_height();
    int mode = n == 3 ? GL_RGB : n == -3 ? GL_BGR : n == 4 ? GL_RGBA : GL_BGRA;
    static __thread uint8_t *pixels = 0;
    pixels = (uint8_t*)REALLOC(pixels, w * h * 4 ); // @leak per thread

    enum { NUM_PBOS = 16 };
    static __thread GLuint pbo[NUM_PBOS] = {0}, lastw = 0, lasth = 0, bound = 0;

    if( lastw != w || lasth != h ) {
        lastw = w, lasth = h;
        bound = 0;

        for( int i = 0; i < NUM_PBOS; ++i ) {
        if(!pbo[i]) glGenBuffers(1, &pbo[i]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, w * h * 4, NULL, GL_STREAM_READ); // GL_STATIC_READ);
        //glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, (GLvoid*)((GLchar*)NULL+0));
        }
    }

    // read from oldest bound pbo
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[bound]);
    void *ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    memcpy(pixels, ptr, w * h * abs(n));
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

    // trigger next read
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, (GLvoid*)((GLchar*)NULL+0));
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    bound = (bound + 1) % NUM_PBOS;
    return pixels;
#endif
}

// -----------------------------------------------------------------------------
// viewports

void viewport_clear(bool color, bool depth) {
    glClearDepthf(1);
    glClearStencil(0);
    glClear((color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0));
}

void viewport_clip(vec2 from, vec2 to) {
    float x = from.x, y = from.y, w = to.x-from.x, h = to.y-from.y;

    y = window_height()-y-h;
    glViewport(x, y, w, h);
    glScissor(x, y, w, h);
}

// -----------------------------------------------------------------------------
// fbos

unsigned fbo(unsigned color_texture_id, unsigned depth_texture_id, int flags) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if( color_texture_id ) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture_id, 0);
    if( depth_texture_id ) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture_id, 0);
#if 0 // this is working; it's just not enabled for now
    else {
        // create a non-sampleable renderbuffer object for depth and stencil attachments
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, color.width, color.height); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    }
#endif

#if is(ems)
    GLenum nones[] = { GL_NONE };
    if(flags) glDrawBuffers(1, nones);
    if(flags) glReadBuffer(GL_NONE);
#else
    if(flags) glDrawBuffer(GL_NONE);
    if(flags) glReadBuffer(GL_NONE);
#endif

#if 1
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( GL_FRAMEBUFFER_COMPLETE != result ) {
        PANIC("ERROR: Framebuffer not complete.");
    }
#else
    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        case GL_FRAMEBUFFER_COMPLETE: break;
        case GL_FRAMEBUFFER_UNDEFINED: PANIC("GL_FRAMEBUFFER_UNDEFINED");
        case GL_FRAMEBUFFER_UNSUPPORTED: PANIC("GL_FRAMEBUFFER_UNSUPPORTED");
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: PANIC("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: PANIC("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: PANIC("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
//      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: PANIC("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
//      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT");
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
        default: PANIC("ERROR: Framebuffer not complete. glCheckFramebufferStatus returned %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
#endif

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
    return fbo;
}
void fbo_bind(unsigned id) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}
void fbo_unbind() {
    fbo_bind(0);
}
void fbo_destroy(unsigned id) {
    // glDeleteRenderbuffers(1, &renderbuffer);
    glDeleteFramebuffers(1, &id);
}

// -----------------------------------------------------------------------------
// post-fxs swapchain

typedef struct passfx passfx;
typedef struct postfx postfx;

void postfx_create(postfx *fx, int flags);
void postfx_destroy(postfx *fx);

bool postfx_load(postfx *fx, const char *name, const char *fragment);
bool postfx_begin(postfx *fx, int width, int height);
bool postfx_end(postfx *fx);

bool postfx_enabled(postfx *fx, int pass_number);
bool postfx_enable(postfx *fx, int pass_number, bool enabled);
// bool postfx_toggle(postfx *fx, int pass_number);
void postfx_clear(postfx *fx);

char* postfx_name(postfx *fx, int slot);

int   ui_postfx(postfx *fx, int slot);

struct passfx {
    mesh_t m;
    char *name;
    unsigned program;
    int uniforms[16];
};

struct postfx {
    // renderbuffers: color & depth textures
    unsigned fb[2];
    texture_t diffuse[2], depth[2];
    // shader passes
    passfx pass[64];
    uint64_t mask;
    // global enable flag
    bool enabled;
    //
    int num_loaded;
};

enum {
    u_color,
    u_depth,
    u_time,
    u_frame,
    u_width, u_height,
    u_mousex, u_mousey,
    u_channelres0x, u_channelres0y,
    u_channelres1x, u_channelres1y,
};

void postfx_create(postfx *fx, int flags) {
    postfx z = {0};
    *fx = z;
    fx->enabled = 1;
    (void)flags;
}

void postfx_destroy( postfx *fx ) {
    for( int i = 0; i < 64; ++i ) {
        FREE(fx->pass[i].name);
    }
    texture_destroy(&fx->diffuse[0]);
    texture_destroy(&fx->diffuse[1]);
    texture_destroy(&fx->depth[0]);
    texture_destroy(&fx->depth[1]);
    fbo_destroy(fx->fb[0]);
    fbo_destroy(fx->fb[1]);
    postfx z = {0};
    *fx = z;
}

char* postfx_name(postfx *fx, int slot) {
    return slot < 0 ? "" : fx->pass[ slot & 63 ].name;
}
int postfx_find(postfx *fx, const char *name) {
    name = file_name(name);
    for( int i = 0; i < 64; ++i) if(!strcmpi(fx->pass[i].name, name)) return i;
    return -1;
}

int postfx_load_from_mem( postfx *fx, const char *name, const char *fs ) {
    PRINTF("%s\n", name);
    if(!fs || !fs[0]) return -1; // PANIC("!invalid fragment shader");

    int slot = fx->num_loaded++;

    passfx *p = &fx->pass[ slot & 63 ];
    p->name = STRDUP(name);

    const char *vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B.glsl");

    // patch fragment
    char *fs2 = (char*)CALLOC(1, 128*1024);
    strcat(fs2, vfs_read("shaders/fs_2_4_preamble.glsl"));

    if( strstr(fs, "mainImage") ) {
        strcat(fs2, vfs_read("shaders/fs_main_shadertoy.glsl") );
    }

    strcat(fs2, fs);

    p->program = shader(vs, fs2, "vtexcoord", "fragColor" , NULL);

    FREE(fs2);

    glUseProgram(p->program); // needed?

    for( int i = 0; i < countof(p->uniforms); ++i ) p->uniforms[i] = -1;

    if( p->uniforms[u_time] == -1 )   p->uniforms[u_time] = glGetUniformLocation(p->program, "iTime");

    if( p->uniforms[u_frame] == -1 )   p->uniforms[u_frame] = glGetUniformLocation(p->program, "iFrame");

    if( p->uniforms[u_width] == -1 )  p->uniforms[u_width] = glGetUniformLocation(p->program, "iWidth");
    if( p->uniforms[u_height] == -1 ) p->uniforms[u_height] = glGetUniformLocation(p->program, "iHeight");

    if( p->uniforms[u_mousex] == -1 ) p->uniforms[u_mousex] = glGetUniformLocation(p->program, "iMousex");
    if( p->uniforms[u_mousey] == -1 ) p->uniforms[u_mousey] = glGetUniformLocation(p->program, "iMousey");

    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tex");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tex0");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tColor");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tDiffuse");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "iChannel0");

    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "tex1");
    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "tDepth");
    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "iChannel1");

    if( p->uniforms[u_channelres0x] == -1 ) p->uniforms[u_channelres0x] = glGetUniformLocation(p->program, "iChannelRes0x");
    if( p->uniforms[u_channelres0y] == -1 ) p->uniforms[u_channelres0y] = glGetUniformLocation(p->program, "iChannelRes0y");

    if( p->uniforms[u_channelres1x] == -1 ) p->uniforms[u_channelres1x] = glGetUniformLocation(p->program, "iChannelRes1x");
    if( p->uniforms[u_channelres1y] == -1 ) p->uniforms[u_channelres1y] = glGetUniformLocation(p->program, "iChannelRes1y");

    // set quad
    glGenVertexArrays(1, &p->m.vao);
    return slot;
}

bool postfx_enable(postfx *fx, int pass, bool enabled) {
    if( pass < 0 ) return false;
    fx->mask = enabled ? fx->mask | (1ull << pass) : fx->mask & ~(1ull << pass);
    fx->enabled = !!popcnt64(fx->mask);
    return fx->enabled;
}

bool postfx_enabled(postfx *fx, int pass) {
    if( pass < 0 ) return false;
    return (!!(fx->mask & (1ull << pass)));
}

bool postfx_toggle(postfx *fx, int pass) {
    if( pass < 0 ) return false;
    return postfx_enable(fx, pass, 1 ^ postfx_enabled(fx, pass));
}

void postfx_clear(postfx *fx) {
    fx->mask = fx->enabled = 0;
}

int ui_postfx(postfx *fx, int pass) {
    int on = ui_enabled();
    ( postfx_enabled(fx,pass) ? ui_enable : ui_disable )();
    int rc = ui_shader(fx->pass[pass].program);
    ( on ? ui_enable : ui_disable )();
    return rc;
}

static __thread array(handle) last_fb;

bool postfx_begin(postfx *fx, int width, int height) {
    // reset clear color: needed in case transparent window is being used (alpha != 0)
    glClearColor(0,0,0,0); // @transparent

    width += !width;
    height += !height;

    int fb;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fb);
    array_push(last_fb, fb);

    // resize if needed
    if( fx->diffuse[0].w != width || fx->diffuse[0].h != height ) {
        texture_destroy(&fx->diffuse[0]);
        texture_destroy(&fx->diffuse[1]);
        texture_destroy(&fx->depth[0]);
        texture_destroy(&fx->depth[1]);
        fbo_destroy(fx->fb[0]);
        fbo_destroy(fx->fb[1]);

        // create texture, set texture parameters and content
        fx->diffuse[0] = texture_create(width, height, 4, NULL, TEXTURE_RGBA);
        fx->depth[0] = texture_create(width, height, 1,  NULL, TEXTURE_DEPTH|TEXTURE_FLOAT);
        fx->fb[0] = fbo(fx->diffuse[0].id, fx->depth[0].id, 0);

        // create texture, set texture parameters and content
        fx->diffuse[1] = texture_create(width, height, 4, NULL, TEXTURE_RGBA);
        fx->depth[1] = texture_create(width, height, 1, NULL, TEXTURE_DEPTH|TEXTURE_FLOAT);
        fx->fb[1] = fbo(fx->diffuse[1].id, fx->depth[1].id, 0);
    }

    uint64_t num_active_passes = popcnt64(fx->mask);
    bool active = fx->enabled && num_active_passes;
    if( !active ) {
        array_pop(last_fb);
        fbo_bind(fb);
        return false;
    }

    fbo_bind(fx->fb[1]);

    viewport_clear(true, true);
    viewport_clip(vec2(0,0), vec2(width, height));

    fbo_bind(fx->fb[0]);

    viewport_clear(true, true);
    viewport_clip(vec2(0,0), vec2(width, height));

    return true;
}

bool postfx_end(postfx *fx) {
    uint64_t num_active_passes = popcnt64(fx->mask);
    bool active = fx->enabled && num_active_passes;
    if( !active ) {
        return false;
    }

    handle fb = *array_back(last_fb);
    array_pop(last_fb);
    fbo_bind(fb);

    // disable depth test in 2d rendering
    bool is_depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    int frame = 0;
    float t = time_ms() / 1000.f;
    float w = fx->diffuse[0].w;
    float h = fx->diffuse[0].h;
    float mx = input(MOUSE_X);
    float my = input(MOUSE_Y);

    for(int i = 0, e = countof(fx->pass); i < e; ++i) {
        if( fx->mask & (1ull << i) ) {
            passfx *pass = &fx->pass[i];

            if( !pass->program ) { --num_active_passes; continue; }
            glUseProgram(pass->program);

            // bind texture to texture unit 0
            // shader_texture_unit(fx->diffuse[frame], 0);
 glActiveTexture(GL_TEXTURE0 + 0);            glBindTexture(GL_TEXTURE_2D, fx->diffuse[frame].id);
            glUniform1i(pass->uniforms[u_color], 0);

            glUniform1f(pass->uniforms[u_channelres0x], fx->diffuse[frame].w);
            glUniform1f(pass->uniforms[u_channelres0y], fx->diffuse[frame].h);

            // bind depth to texture unit 1
            // shader_texture_unit(fx->depth[frame], 1);
 glActiveTexture(GL_TEXTURE0 + 1);            glBindTexture(GL_TEXTURE_2D, fx->depth[frame].id);
            glUniform1i(pass->uniforms[u_depth], 1);

            // bind uniforms
            static unsigned f = 0; ++f;
            glUniform1f(pass->uniforms[u_time], t);
            glUniform1f(pass->uniforms[u_frame], f-1);
            glUniform1f(pass->uniforms[u_width], w);
            glUniform1f(pass->uniforms[u_height], h);

            glUniform1f(pass->uniforms[u_mousex], mx);
            glUniform1f(pass->uniforms[u_mousey], my);

            // bind the vao
            int bound = --num_active_passes;
            if( bound ) fbo_bind(fx->fb[frame ^= 1]);

                // fullscreen quad
                glBindVertexArray(pass->m.vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                profile_incstat("Render.num_drawcalls", +1);
                profile_incstat("Render.num_triangles", +2);
                glBindVertexArray(0);

            if( bound ) fbo_bind(fb);
            else glUseProgram(0);
        }
    }

    if(is_depth_test_enabled);
    glEnable(GL_DEPTH_TEST);

    // restore clear color: needed in case transparent window is being used (alpha != 0)
    glClearColor(0,0,0,1); // @transparent

    return true;
}

static postfx fx;
int fx_load_from_mem(const char *nameid, const char *content) {
    do_once postfx_create(&fx, 0);
    return postfx_load_from_mem(&fx, nameid, content);
}
int fx_load(const char *filemask) {
    static set(char*) added = 0; do_once set_init_str(added);
    for(const char **list = vfs_list(filemask); *list; list++) {
        if( set_find(added, (char*)*list) ) continue;
        char *name = STRDUP(*list); // @leak
        set_insert(added, name);
        (void)postfx_load_from_mem(&fx, file_name(name), vfs_read(name));
    }
    return 1;
}
void fx_begin() {
    postfx_begin(&fx, window_width(), window_height());
}
void fx_begin_res(int w, int h) {
    postfx_begin(&fx, w, h);
}
void fx_end() {
    postfx_end(&fx);
}
int fx_enabled(int pass) {
    return postfx_enabled(&fx, pass);
}
void fx_enable(int pass, int enabled) {
    postfx_enable(&fx, pass, enabled);
}
void fx_enable_all(int enabled) {
    for( int i = 0; i < fx.num_loaded; ++i ) fx_enable(i, enabled);
}
char *fx_name(int pass) {
    return postfx_name(&fx, pass);
}
int fx_find(const char *name) {
    return postfx_find(&fx, name);
}
int ui_fx(int pass) {
    return ui_postfx(&fx, pass);
}
int ui_fxs() {
    if(!fx.num_loaded) return ui_label(ICON_MD_WARNING " No Post FXs with annotations loaded."), 0;

    int changed = 0;
    for( int i = 0; i < 64; ++i ) {
        char *name = fx_name(i); if( !name ) break;
        bool b = fx_enabled(i);
        if( ui_bool(name, &b) ) fx_enable(i, fx_enabled(i) ^ 1);
        ui_fx(i);
        ui_separator();
    }
    return changed;
}

// -----------------------------------------------------------------------------
// brdf

static texture_t brdf = {0};

static void brdf_load() {
    const char *filename;
    filename = "Skyboxes/brdf_lut1k_256x256_32F.ktx";
    filename = "Skyboxes/brdf_lut2k_512x512_32F.ktx";

    brdf = texture_compressed( filename,
        TEXTURE_CLAMP | TEXTURE_NEAREST | TEXTURE_RG | TEXTURE_FLOAT | TEXTURE_SRGB
    );
    ASSERT(brdf.id != texture_checker().id, "!Couldn't load BRDF lookup table '%s'!", filename );
}

texture_t brdf_lut() {
    do_once brdf_load();
    return brdf;
}

// -----------------------------------------------------------------------------
// materials

bool colormap( colormap_t *cm, const char *material_file, bool load_as_srgb ) {
    if( !material_file ) return false;

    if( cm->texture ) {
        texture_destroy(cm->texture);
        FREE(cm->texture), cm->texture = NULL;
    }

    int srgb = load_as_srgb ? TEXTURE_SRGB : 0;
    int hdr = strendi(material_file, ".hdr") ? TEXTURE_FLOAT | TEXTURE_RGBA : 0;
    texture_t t = texture_compressed(material_file, TEXTURE_LINEAR | TEXTURE_MIPMAPS | TEXTURE_REPEAT | hdr | srgb);

    if( t.id == texture_checker().id ) {
        cm->texture = NULL;
        return false;
    }
    cm->texture = CALLOC(1, sizeof(texture_t));
    *cm->texture = t;
    return true;
}

bool pbr_material(pbr_material_t *pbr, const char *material) {
    if( !material || !pbr ) return false;

    //pbr_material_destroy(pbr);
    *pbr = (pbr_material_t){0};

    pbr->name = STRDUP(material);

    pbr->specular_shininess = 1.0f;
    /*
    if( const float *f = aiGetMaterialFloat(scn_material[i], aiMaterialTypeString(MATERIAL_SHININESS)) ) {
        pbr->specular_shininess = *f;
    }
    */

    pbr->diffuse.color = vec4(0.5,0.5,0.5,0.5);
    pbr->normals.color = vec4(0,0,0,0);
    pbr->specular.color = vec4(0,0,0,0);
    pbr->albedo.color = vec4(0.5,0.5,0.5,1.0);
    pbr->roughness.color = vec4(1,1,1,1);
    pbr->metallic.color = vec4(0,0,0,0);
    pbr->ao.color = vec4(1,1,1,1);
    pbr->ambient.color = vec4(0,0,0,1);
    pbr->emissive.color = vec4(0,0,0,0);

    array(char*) tokens = strsplit(material, "+");
    for( int j = 0, end = array_count(tokens); j < end; ++j ) {
        char *t = tokens[j];
        if( strstri(t, "_D.") || strstri(t, "Diffuse") || strstri(t, "BaseColor") )    colormap(&pbr->diffuse, t, 1);
        if( strstri(t, "_N.") || strstri(t, "Normal") )     colormap(&pbr->normals, t, 0);
        if( strstri(t, "_S.") || strstri(t, "Specular") )   colormap(&pbr->specular, t, 0);
        if( strstri(t, "_A.") || strstri(t, "Albedo") )     colormap(&pbr->albedo, t, 1); // 0?
        if( strstri(t, "_MR.")|| strstri(t, "Roughness") )  colormap(&pbr->roughness, t, 0);
        else
        if( strstri(t, "_M.") || strstri(t, "Metallic") )   colormap(&pbr->metallic, t, 0);
      //if( strstri(t, "_S.") || strstri(t, "Shininess") )  colormap(&pbr->roughness, t, 0);
      //if( strstri(t, "_A.") || strstri(t, "Ambient") )    colormap(&pbr->ambient, t, 0);
        if( strstri(t, "_E.") || strstri(t, "Emissive") )   colormap(&pbr->emissive, t, 1);
        if( strstri(t, "_AO.") || strstri(t, "AO") || strstri(t, "Occlusion") ) colormap(&pbr->ao, t, 0);
    }

    return true;
}

void pbr_material_destroy(pbr_material_t *m) {
    if( m->name )              FREE(m->name), m->name = NULL;
    if( m->diffuse.texture)    texture_destroy( m->diffuse.texture );
    if( m->normals.texture)    texture_destroy( m->normals.texture );
    if( m->specular.texture)   texture_destroy( m->specular.texture );
    if( m->albedo.texture)     texture_destroy( m->albedo.texture );
    if( m->roughness.texture)  texture_destroy( m->roughness.texture );
    if( m->metallic.texture)   texture_destroy( m->metallic.texture );
    if( m->ao.texture )        texture_destroy( m->ao.texture );
    if( m->ambient.texture )   texture_destroy( m->ambient.texture );
    *m = (pbr_material_t){0};
}

// ----------------------------------------------------------------------------
// shadertoys
//
// @todo: multipass
// - https://www.shadertoy.com/view/Mst3Wr - la calanque
// - https://www.shadertoy.com/view/XsyGWV - sirenian dawn
// - https://www.shadertoy.com/view/Xst3zX - wordtoy
// - https://www.shadertoy.com/view/MddGzf - bricks game
// - https://www.shadertoy.com/view/Ms33WB - post process - ssao
// - https://www.shadertoy.com/view/Xds3zN

enum shadertoy_uniforms {
    iFrame,
    iTime,
    iDate,
    iGlobalTime,
    iGlobalFrame,
    iGlobalDelta,
    iChannel0,
    iChannel1,
    iChannel2,
    iChannel3,
    iResolution,
    iMouse,
    iOffset,
    iSampleRate,
    iChannelResolution,
    iChannelTime,
    // iCameraScreen
    // iCameraPosition
    // iCameraActive
};

shadertoy_t shadertoy( const char *shaderfile, unsigned flags ) {
    shadertoy_t s = {0};
    s.flags = flags;

    char *file = vfs_read(shaderfile);
    if( !file ) return s;

    glGenVertexArrays(1, &s.vao);

    char *fs = stringf("%s%s", vfs_read("header_shadertoy.glsl"), file);
    s.program = shader((flags&SHADERTOY_FLIP_Y) ? vfs_read("shaders/vs_shadertoy_flip.glsl") : vfs_read("shaders/vs_shadertoy.glsl"), fs, "", "fragColor", NULL);
    FREE(fs);

    if( strstr(file, "noise3.jpg"))
    s.texture_channels[0] = texture("shadertoys/tex12.png", 0).id;
    else
    s.texture_channels[0] = texture("shadertoys/tex04.jpg", 0).id;

    s.uniforms[iFrame] = glGetUniformLocation(s.program, "iFrame");
    s.uniforms[iTime] = glGetUniformLocation(s.program, "iTime");
    s.uniforms[iDate] = glGetUniformLocation(s.program, "iDate");
    s.uniforms[iGlobalTime] = glGetUniformLocation(s.program, "iGlobalTime");
    s.uniforms[iGlobalDelta] = glGetUniformLocation(s.program, "iGlobalDelta");
    s.uniforms[iGlobalFrame] = glGetUniformLocation(s.program, "iGlobalFrame");
    s.uniforms[iResolution] = glGetUniformLocation(s.program, "iResolution");

    s.uniforms[iChannel0] = glGetUniformLocation(s.program, "iChannel0");
    s.uniforms[iChannel1] = glGetUniformLocation(s.program, "iChannel1");
    s.uniforms[iChannel2] = glGetUniformLocation(s.program, "iChannel2");
    s.uniforms[iChannel3] = glGetUniformLocation(s.program, "iChannel3");

    s.uniforms[iMouse] = glGetUniformLocation(s.program, "iMouse");
    s.uniforms[iOffset] = glGetUniformLocation(s.program, "iOffset");
    s.uniforms[iSampleRate] = glGetUniformLocation(s.program, "iSampleRate");
    s.uniforms[iChannelResolution] = glGetUniformLocation(s.program, "iChannelResolution");
    s.uniforms[iChannelTime] = glGetUniformLocation(s.program, "iChannelTime");

    return s;
}

shadertoy_t* shadertoy_render(shadertoy_t *s, float delta) {
    if( s->program && s->vao ) {
        if( s->dims.x && !(s->flags&SHADERTOY_IGNORE_FBO) && !texture_rec_begin(&s->tx, s->dims.x, s->dims.y) ) {
            return s;
        }

        float mx = input(MOUSE_X), my = input(MOUSE_Y);
        if(input(MOUSE_L)) s->clickx = mx, s->clicky = my;

        time_t tmsec = time(0);
        struct tm *tm = localtime(&tmsec);
        s->t += delta * 1000;

        glUseProgram(s->program);
        glUniform1f(s->uniforms[iGlobalTime], s->t / 1000.f );
        glUniform1f(s->uniforms[iGlobalFrame], s->frame++);
        glUniform1f(s->uniforms[iGlobalDelta], delta / 1000.f );
        glUniform2f(s->uniforms[iResolution], s->dims.x ? s->dims.x : window_width(), s->dims.y ? s->dims.y : window_height());
        if (!(s->flags&SHADERTOY_IGNORE_MOUSE)) glUniform4f(s->uniforms[iMouse], mx, my, s->clickx, s->clicky );

        glUniform1i(s->uniforms[iFrame], (int)window_frame());
        glUniform1f(s->uniforms[iTime], time_ss());
        glUniform4f(s->uniforms[iDate], tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_sec + tm->tm_min * 60 + tm->tm_hour * 3600);

        int unit = 0;
        for( int i = 0; i < 4; i++ ) {
            if( s->texture_channels[i] ) {
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, s->texture_channels[i]);
                glUniform1i(s->uniforms[iChannel0+i], unit);
                unit++;
            }
        }

        glBindVertexArray(s->vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        if(s->dims.x && !(s->flags&SHADERTOY_IGNORE_FBO)) texture_rec_end(&s->tx); // texture_rec
    }
    return s;
}

// -----------------------------------------------------------------------------
// skeletal meshes (iqm)

#define IQM_MAGIC "INTERQUAKEMODEL"
#define IQM_VERSION 2

struct iqmheader {
    char magic[16];
    unsigned version;
    unsigned filesize;
    unsigned flags;
    unsigned num_text, ofs_text;
    unsigned num_meshes, ofs_meshes;
    unsigned num_vertexarrays, num_vertexes, ofs_vertexarrays;
    unsigned num_triangles, ofs_triangles, ofs_adjacency;
    unsigned num_joints, ofs_joints;
    unsigned num_poses, ofs_poses;
    unsigned num_anims, ofs_anims;
    unsigned num_frames, num_framechannels, ofs_frames, ofs_bounds;
    unsigned num_comment, ofs_comment;
    unsigned num_extensions, ofs_extensions;
};

struct iqmmesh {
    unsigned name;
    unsigned material;
    unsigned first_vertex, num_vertexes;
    unsigned first_triangle, num_triangles;
};

enum {
    IQM_POSITION,
    IQM_TEXCOORD,
    IQM_NORMAL,
    IQM_TANGENT,
    IQM_BLENDINDEXES,
    IQM_BLENDWEIGHTS,
    IQM_COLOR,
    IQM_CUSTOM = 0x10
};

enum {
    IQM_BYTE,
    IQM_UBYTE,
    IQM_SHORT,
    IQM_USHORT,
    IQM_INT,
    IQM_UINT,
    IQM_HALF,
    IQM_FLOAT,
    IQM_DOUBLE,
};

struct iqmtriangle {
    unsigned vertex[3];
};

struct iqmadjacency {
    unsigned triangle[3];
};

struct iqmjoint {
    unsigned name;
    int parent;
    float translate[3], rotate[4], scale[3];
};

struct iqmpose {
    int parent;
    unsigned mask;
    float channeloffset[10];
    float channelscale[10];
};

struct iqmanim {
    unsigned name;
    unsigned first_frame, num_frames;
    float framerate;
    unsigned flags;
};

enum {
    IQM_LOOP = 1<<0
};

struct iqmvertexarray {
    unsigned type;
    unsigned flags;
    unsigned format;
    unsigned size;
    unsigned offset;
};

struct iqmbounds {
    union {
        struct { float bbmin[3], bbmax[3]; };
        struct { vec3 min3, max3; };
        aabb box;
    };
    float xyradius, radius;
};

// -----------------------------------------------------------------------------

typedef struct iqm_vertex {
    GLfloat position[3];
    GLfloat texcoord[2];
    GLfloat normal[3];
    GLfloat tangent[4];
    GLubyte blendindexes[4];
    GLubyte blendweights[4];
    GLfloat blendvertexindex;
    GLubyte color[4];
} iqm_vertex;

typedef struct iqm_t {
    int nummeshes, numtris, numverts, numjoints, numframes, numanims;
    GLuint program;
    GLuint vao, ibo, vbo;
    GLuint *textures;
    uint8_t *buf, *meshdata, *animdata;
    struct iqmmesh *meshes;
    struct iqmjoint *joints;
    struct iqmpose *poses;
    struct iqmanim *anims;
    struct iqmbounds *bounds;
    mat34 *baseframe, *inversebaseframe, *outframe, *frames;
    GLint bonematsoffset;
    vec4 *colormaps;
} iqm_t;

#define program (q->program)
#define meshdata (q->meshdata)
#define animdata (q->animdata)
#define nummeshes (q->nummeshes)
#define numtris (q->numtris)
#define numverts (q->numverts)
#define numjoints (q->numjoints)
#define numframes (q->numframes)
#define numanims (q->numanims)
#define meshes (q->meshes)
#define textures (q->textures)
#define joints (q->joints)
#define poses (q->poses)
#define anims (q->anims)
#define baseframe (q->baseframe)
#define inversebaseframe (q->inversebaseframe)
#define outframe (q->outframe)
#define frames (q->frames)
#define vao (q->vao)
#define ibo (q->ibo)
#define vbo (q->vbo)
#define bonematsoffset (q->bonematsoffset)
#define buf (q->buf)
#define bounds (q->bounds)
#define colormaps (q->colormaps)

void model_set_texture(model_t m, texture_t t) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    for( int i = 0; i < nummeshes; ++i) { // assume 1 texture per mesh
        textures[i] = t.id;
    }
}

static
void model_set_uniforms(model_t m, int shader, mat44 mv, mat44 proj, mat44 view, mat44 model) {  // @todo: cache uniform locs
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glUseProgram(shader);
    int loc;
    //if( (loc = glGetUniformLocation(shader, "M")) >= 0 ) glUniformMatrix4fv( loc, 1, GL_FALSE/*GL_TRUE*/, m); // RIM
    if( (loc = glGetUniformLocation(shader, "MV")) >= 0 ) {
        glUniformMatrix4fv( loc, 1, GL_FALSE, mv);
    }
    else
    if( (loc = glGetUniformLocation(shader, "u_mv")) >= 0 ) {
        glUniformMatrix4fv( loc, 1, GL_FALSE, mv);
    }
    if( (loc = glGetUniformLocation(shader, "MVP")) >= 0 ) {
        mat44 mvp; multiply44x2(mvp, proj, mv); // multiply44x3(mvp, proj, view, model);
        glUniformMatrix4fv( loc, 1, GL_FALSE, mvp);
    }
    else
    if( (loc = glGetUniformLocation(shader, "u_mvp")) >= 0 ) {
        mat44 mvp; multiply44x2(mvp, proj, mv); // multiply44x3(mvp, proj, view, model);
        glUniformMatrix4fv( loc, 1, GL_FALSE, mvp);
    }
    if( (loc = glGetUniformLocation(shader, "VP")) >= 0 ) {
        mat44 vp; multiply44x2(vp, proj, view);
        glUniformMatrix4fv( loc, 1, GL_FALSE, vp);
    }
    else
    if( (loc = glGetUniformLocation(shader, "u_vp")) >= 0 ) {
        mat44 vp; multiply44x2(vp, proj, view);
        glUniformMatrix4fv( loc, 1, GL_FALSE, vp);
    }
    if( (loc = glGetUniformLocation(shader, "u_cam_pos")) >= 0 ) {
        vec3 pos = vec3(view[12], view[13], view[14]);
        glUniform3fv( loc, 1, &pos.x );
    }
    else
    if( (loc = glGetUniformLocation(shader, "cam_pos")) >= 0 ) {
        vec3 pos = vec3(view[12], view[13], view[14]);
        glUniform3fv( loc, 1, &pos.x );
    }
    if( (loc = glGetUniformLocation(shader, "u_cam_dir")) >= 0 ) {
        vec3 dir = norm3(vec3(view[2], view[6], view[10]));
        glUniform3fv( loc, 1, &dir.x );
    }
    else
    if( (loc = glGetUniformLocation(shader, "cam_dir")) >= 0 ) {
        vec3 dir = norm3(vec3(view[2], view[6], view[10]));
        glUniform3fv( loc, 1, &dir.x );
    }
#if 0
    // @todo: mat44 projview (useful?)
#endif
    if ((loc = glGetUniformLocation(shader, "M")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, model);
    }
    else
    if ((loc = glGetUniformLocation(shader, "model")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, model);
    }
    if ((loc = glGetUniformLocation(shader, "V")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, view);
    }
    else
    if ((loc = glGetUniformLocation(shader, "view")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, view);
    }
    if ((loc = glGetUniformLocation(shader, "P")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
    }
    else
    if ((loc = glGetUniformLocation(shader, "proj")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
    }
    if( (loc = glGetUniformLocation(shader, "SKINNED")) >= 0 ) glUniform1i( loc, numanims ? GL_TRUE : GL_FALSE);
    if( numanims )
    if( (loc = glGetUniformLocation(shader, "vsBoneMatrix")) >= 0 ) glUniformMatrix3x4fv( loc, numjoints, GL_FALSE, outframe[0]);

    if ((loc = glGetUniformLocation(shader, "u_matcaps")) >= 0) {
        glUniform1i(loc, m.flags & MODEL_MATCAPS ? GL_TRUE:GL_FALSE);
    }
}
static
void model_set_state(model_t m) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glBindVertexArray( vao );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, position) );
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, texcoord) );
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, normal) );
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, tangent) );

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    // vertex color
    glVertexAttribPointer(11, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,color) );
    glEnableVertexAttribArray(11);

    // animation
    if(numframes > 0) {
        glVertexAttribPointer( 8, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,blendindexes) );
        glVertexAttribPointer( 9, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,blendweights) );
        glVertexAttribPointer(10, 1, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, blendvertexindex) );
        glEnableVertexAttribArray(8);
        glEnableVertexAttribArray(9);
        glEnableVertexAttribArray(10);
    }

    // mat4 attribute; for instanced rendering
    if( 1 ) {
        unsigned vec4_size = sizeof(vec4);
        unsigned mat4_size = sizeof(vec4) * 4;

        // vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, m.vao_instanced);
        glBufferData(GL_ARRAY_BUFFER, m.num_instances * mat4_size, m.instanced_matrices, GL_STATIC_DRAW);

        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)(((char*)NULL)+(0 * vec4_size)));
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)(((char*)NULL)+(1 * vec4_size)));
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)(((char*)NULL)+(2 * vec4_size)));
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)(((char*)NULL)+(3 * vec4_size)));

        glEnableVertexAttribArray(4);
        glEnableVertexAttribArray(5);
        glEnableVertexAttribArray(6);
        glEnableVertexAttribArray(7);

        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
    }

    // 7 bitangent? into texcoord.z?

    glBindVertexArray( 0 );
}

static
bool model_load_meshes(iqm_t *q, const struct iqmheader *hdr, model_t *m) {
    if(meshdata) return false;

    lil32p(&buf[hdr->ofs_vertexarrays], hdr->num_vertexarrays*sizeof(struct iqmvertexarray)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_triangles], hdr->num_triangles*sizeof(struct iqmtriangle)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_meshes], hdr->num_meshes*sizeof(struct iqmmesh)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_joints], hdr->num_joints*sizeof(struct iqmjoint)/sizeof(uint32_t));

    meshdata = buf;
    nummeshes = hdr->num_meshes;
    numtris = hdr->num_triangles;
    numverts = hdr->num_vertexes;
    numjoints = hdr->num_joints;
    outframe = CALLOC(hdr->num_joints, sizeof(mat34));

    float *inposition = NULL, *innormal = NULL, *intangent = NULL, *intexcoord = NULL, *invertexindex = NULL;
    uint8_t *inblendindex8 = NULL, *inblendweight8 = NULL;
    int *inblendindexi = NULL; float *inblendweightf = NULL;
    uint8_t *invertexcolor8 = NULL;
    struct iqmvertexarray *vas = (struct iqmvertexarray *)&buf[hdr->ofs_vertexarrays];
    for(int i = 0; i < (int)hdr->num_vertexarrays; i++) {
        struct iqmvertexarray *va = &vas[i];
        switch(va->type) {
        default: continue; // return PANIC("unknown iqm vertex type (%d)", va->type), false;
        break; case IQM_POSITION: ASSERT(va->format == IQM_FLOAT && va->size == 3); inposition = (float *)&buf[va->offset]; lil32pf(inposition, 3*hdr->num_vertexes);
        break; case IQM_NORMAL: ASSERT(va->format == IQM_FLOAT && va->size == 3); innormal = (float *)&buf[va->offset]; lil32pf(innormal, 3*hdr->num_vertexes);
        break; case IQM_TANGENT: ASSERT(va->format == IQM_FLOAT && va->size == 4); intangent = (float *)&buf[va->offset]; lil32pf(intangent, 4*hdr->num_vertexes);
        break; case IQM_TEXCOORD: ASSERT(va->format == IQM_FLOAT && va->size == 2); intexcoord = (float *)&buf[va->offset]; lil32pf(intexcoord, 2*hdr->num_vertexes);
        break; case IQM_COLOR: ASSERT(va->size == 4); ASSERT(va->format == IQM_UBYTE); invertexcolor8 = (uint8_t *)&buf[va->offset];
        break; case IQM_BLENDINDEXES: ASSERT(va->size == 4); ASSERT(va->format == IQM_UBYTE || va->format == IQM_INT);
        if(va->format == IQM_UBYTE) inblendindex8 = (uint8_t *)&buf[va->offset];
        else inblendindexi = (int *)&buf[va->offset];
        break; case IQM_BLENDWEIGHTS: ASSERT(va->size == 4); ASSERT(va->format == IQM_UBYTE || va->format == IQM_FLOAT);
        if(va->format == IQM_UBYTE) inblendweight8 = (uint8_t *)&buf[va->offset];
        else inblendweightf = (float *)&buf[va->offset];
        invertexindex = (inblendweight8 ? (float*)(inblendweight8 + 4) : inblendweightf + 4 );
        }
    }

    if (hdr->ofs_bounds) lil32p(buf + hdr->ofs_bounds, hdr->num_frames * sizeof(struct iqmbounds));
    if (hdr->ofs_bounds) bounds = (struct iqmbounds *) &buf[hdr->ofs_bounds];

    meshes = (struct iqmmesh *)&buf[hdr->ofs_meshes];
    joints = (struct iqmjoint *)&buf[hdr->ofs_joints];

    baseframe = CALLOC(hdr->num_joints, sizeof(mat34));
    inversebaseframe = CALLOC(hdr->num_joints, sizeof(mat34));
    for(int i = 0; i < (int)hdr->num_joints; i++) {
        struct iqmjoint *j = &joints[i];
        compose34(baseframe[i], ptr3(j->translate), normq(ptrq(j->rotate)), ptr3(j->scale));
        invert34(inversebaseframe[i], baseframe[i]);
        if(j->parent >= 0) {
            multiply34x2(baseframe[i], baseframe[j->parent], baseframe[i]);
            multiply34(inversebaseframe[i], inversebaseframe[j->parent]);
        }
    }

    struct iqmtriangle *tris = (struct iqmtriangle *)&buf[hdr->ofs_triangles];

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if(!ibo) glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, hdr->num_triangles*sizeof(struct iqmtriangle), tris, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    iqm_vertex *verts = CALLOC(hdr->num_vertexes, sizeof(iqm_vertex));
    for(int i = 0; i < (int)hdr->num_vertexes; i++) {
        iqm_vertex *v = &verts[i];
        if(inposition) memcpy(v->position, &inposition[i*3], sizeof(v->position));
        if(innormal) memcpy(v->normal, &innormal[i*3], sizeof(v->normal));
        if(intangent) memcpy(v->tangent, &intangent[i*4], sizeof(v->tangent));
        if(intexcoord) memcpy(v->texcoord, &intexcoord[i*2], sizeof(v->texcoord));
        if(inblendindex8) memcpy(v->blendindexes, &inblendindex8[i*4], sizeof(v->blendindexes));
        if(inblendweight8) memcpy(v->blendweights, &inblendweight8[i*4], sizeof(v->blendweights));
        if(inblendindexi) {
            uint8_t conv[4] = { inblendindexi[i*4], inblendindexi[i*4+1], inblendindexi[i*4+2], inblendindexi[i*4+3] };
            memcpy(v->blendindexes, conv, sizeof(v->blendindexes));
        }
        if(inblendweightf) {
            uint8_t conv[4] = { inblendweightf[i*4] * 255, inblendweightf[i*4+1] * 255, inblendweightf[i*4+2] * 255, inblendweightf[i*4+3] * 255 };
            memcpy(v->blendweights, conv, sizeof(v->blendweights));
        }
        if(invertexindex) {
            float conv = i;
            memcpy(&v->blendvertexindex, &conv, 4);
        }
        if(invertexcolor8) memcpy(v->color, &invertexcolor8[i*4], sizeof(v->color));
    }

    if(!vbo) glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, hdr->num_vertexes*sizeof(iqm_vertex), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

m->stride = sizeof(iqm_vertex);
#if 0
m->stride = 0;
if(inposition) m->stride += sizeof(verts[0].position);
if(innormal) m->stride += sizeof(verts[0].normal);
if(intangent) m->stride += sizeof(verts[0].tangent);
if(intexcoord) m->stride += sizeof(verts[0].texcoord);
if(inblendindex8) m->stride += sizeof(verts[0].blendindexes); // no index8? bug?
if(inblendweight8) m->stride += sizeof(verts[0].blendweights); // no weight8? bug?
if(inblendindexi) m->stride += sizeof(verts[0].blendindexes);
if(inblendweightf) m->stride += sizeof(verts[0].blendweights);
if(invertexcolor8) m->stride += sizeof(verts[0].color);
#endif
//for( int i = 0; i < 16; ++i ) printf("%.9g%s", ((float*)verts)[i], (i % 3) == 2 ? "\n" : ",");
//m->verts = verts; //FREE(verts);
m->verts = 0; FREE(verts);

    textures = CALLOC(hdr->num_meshes * 8, sizeof(GLuint));
    colormaps = CALLOC(hdr->num_meshes * 8, sizeof(vec4));
    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        int invalid = texture_checker().id;
        textures[i] = invalid;
    }

    const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        struct iqmmesh *m = &meshes[i];
        PRINTF("loaded mesh: %s\n", &str[m->name]);
    }

    return true;
}

static
bool model_load_anims(iqm_t *q, const struct iqmheader *hdr) {
    if((int)hdr->num_poses != numjoints) return false;

    if(animdata) {
        if(animdata != meshdata) FREE(animdata);
        FREE(frames);
        animdata = NULL;
        anims = NULL;
        frames = 0;
        numframes = 0;
        numanims = 0;
    }

    lil32p(&buf[hdr->ofs_poses], hdr->num_poses*sizeof(struct iqmpose)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_anims], hdr->num_anims*sizeof(struct iqmanim)/sizeof(uint32_t));
    lil16p((uint16_t *)&buf[hdr->ofs_frames], hdr->num_frames*hdr->num_framechannels);

    animdata = buf;
    numanims = hdr->num_anims;
    numframes = hdr->num_frames;

    anims = (struct iqmanim *)&buf[hdr->ofs_anims];
    poses = (struct iqmpose *)&buf[hdr->ofs_poses];
    frames = CALLOC(hdr->num_frames * hdr->num_poses, sizeof(mat34));
    uint16_t *framedata = (uint16_t *)&buf[hdr->ofs_frames];

    for(int i = 0; i < (int)hdr->num_frames; i++) {
        for(int j = 0; j < (int)hdr->num_poses; j++) {
            struct iqmpose *p = &poses[j];
            quat rotate;
            vec3 translate, scale;
            translate.x = p->channeloffset[0]; if(p->mask&0x01) translate.x += *framedata++ * p->channelscale[0];
            translate.y = p->channeloffset[1]; if(p->mask&0x02) translate.y += *framedata++ * p->channelscale[1];
            translate.z = p->channeloffset[2]; if(p->mask&0x04) translate.z += *framedata++ * p->channelscale[2];

            rotate.x = p->channeloffset[3]; if(p->mask&0x08) rotate.x += *framedata++ * p->channelscale[3];
            rotate.y = p->channeloffset[4]; if(p->mask&0x10) rotate.y += *framedata++ * p->channelscale[4];
            rotate.z = p->channeloffset[5]; if(p->mask&0x20) rotate.z += *framedata++ * p->channelscale[5];
            rotate.w = p->channeloffset[6]; if(p->mask&0x40) rotate.w += *framedata++ * p->channelscale[6];

            scale.x = p->channeloffset[7]; if(p->mask&0x80)  scale.x += *framedata++ * p->channelscale[7];
            scale.y = p->channeloffset[8]; if(p->mask&0x100) scale.y += *framedata++ * p->channelscale[8];
            scale.z = p->channeloffset[9]; if(p->mask&0x200) scale.z += *framedata++ * p->channelscale[9];

            // Concatenate each pose with the inverse base pose to avoid doing this at animation time.
            // If the joint has a parent, then it needs to be pre-concatenated with its parent's base pose.
            // Thus it all negates at animation time like so:
            //   (parentPose * parentInverseBasePose) * (parentBasePose * childPose * childInverseBasePose) =>
            //   parentPose * (parentInverseBasePose * parentBasePose) * childPose * childInverseBasePose =>
            //   parentPose * childPose * childInverseBasePose

            mat34 m; compose34(m, translate, normq(rotate), scale);
            if(p->parent >= 0) multiply34x3(frames[i*hdr->num_poses + j], baseframe[p->parent], m, inversebaseframe[j]);
            else multiply34x2(frames[i*hdr->num_poses + j], m, inversebaseframe[j]);
        }
    }

    const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
    for(int i = 0; i < (int)hdr->num_anims; i++) {
        struct iqmanim *a = &anims[i];
        PRINTF("loaded anim[%d]: %s\n", i, &str[a->name]);
    }

    return true;
}

// prevents crash on osx when strcpy'ing non __restrict arguments
static char* strcpy_safe(char *d, const char *s) {
    sprintf(d, "%s", s);
    return d;
}

static
bool model_load_textures(iqm_t *q, const struct iqmheader *hdr, model_t *model) {
    textures = textures ? textures : CALLOC(hdr->num_meshes * 8, sizeof(GLuint)); // up to 8 textures per mesh
    colormaps = colormaps ? colormaps : CALLOC(hdr->num_meshes * 8, sizeof(vec4)); // up to 8 colormaps per mesh

    GLuint *out = textures;

    const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        struct iqmmesh *m = &meshes[i];

        int flags = TEXTURE_MIPMAPS|TEXTURE_REPEAT; // LINEAR, NEAREST
        int invalid = texture_checker().id;

#if 1
        char *material_embedded_texture = strstr(&str[m->material], "+b64:");
        if( material_embedded_texture ) {
            *material_embedded_texture = '\0';
            material_embedded_texture += 5;
            array(char) embedded_texture = base64__decode(material_embedded_texture, strlen(material_embedded_texture));
            //printf("%s %d\n", material_embedded_texture, array_count(embedded_texture));
            //hexdump(embedded_texture, array_count(embedded_texture));
            *out = texture_compressed_from_mem( embedded_texture, array_count(embedded_texture), 0 ).id;
            array_free(embedded_texture);
        }

        char* material_color_hex = strstr(&str[m->material], "+$");
        vec4 material_color = vec4(1,1,1,1);
        if( material_color_hex ) {
            *material_color_hex = '\0';
            material_color_hex += 2;
            material_color.r = ((material_color_hex[0] >= 'a') ? material_color_hex[0] - 'a' + 10 : material_color_hex[0] - '0') / 15.f;
            material_color.g = ((material_color_hex[1] >= 'a') ? material_color_hex[1] - 'a' + 10 : material_color_hex[1] - '0') / 15.f;
            material_color.b = ((material_color_hex[2] >= 'a') ? material_color_hex[2] - 'a' + 10 : material_color_hex[2] - '0') / 15.f;
            #if 0 // not enabled because of some .obj files like suzanne, with color_hex=9990 found
            if(material_color_hex[3])
            material_color.a = ((material_color_hex[3] >= 'a') ? material_color_hex[3] - 'a' + 10 : material_color_hex[3] - '0') / 15.f;
            else
            #endif
            material_color.a = 1;
        }

        if( !material_embedded_texture ) {
            char* material_name;
            // remove any material+name from materials (.fbx)
            // try left token first
            if( 1 ) {
                material_name = va("%s", &str[m->material]);
                char* plus = strrchr(material_name, '+');
                if (plus) { strcpy_safe(plus, file_ext(material_name)); }
                *out = texture_compressed(material_name, flags).id;
            }
            // else try right token
            if (*out == invalid) {
                material_name = file_normalize( va("%s", &str[m->material]) );
                char* plus = strrchr(material_name, '+'), *slash = strrchr(material_name, '/');
                if (plus) {
                    strcpy_safe(slash ? slash + 1 : material_name, plus + 1);
                    *out = texture_compressed(material_name, flags).id;
                }
            }
            // else last resort
            if (*out == invalid) {
                *out = texture_compressed(material_name, flags).id; // needed?
            }
        }

        if( *out != invalid) {
            PRINTF("loaded material[%d]: %s\n", i, &str[m->material]);
        } else {
            PRINTF("warn: material[%d] not found: %s\n", i, &str[m->material]);
            PRINTF("warn: using placeholder material[%d]=texture_checker\n", i);
            *out = texture_checker().id; // placeholder
        }

        {
            model->num_textures++;
            array_push(model->texture_names, STRDUP(&str[m->material]));

            material_t mt = {0};
            mt.name = STRDUP(&str[m->material]);
            mt.layer[mt.count].color = material_color_hex ? material_color : vec4(1,1,1,1);
            mt.layer[mt.count++].texture = *out++;
            array_push(model->materials, mt);
        }

#else
        material_t mt = {0};
        mt.name = STRDUP(&str[m->material]);

        array(char*) tokens = strsplit(&str[m->material], "+");
        for each_array(tokens, char*, it) {
            *out = texture(it, flags).id;

            if( *out == invalid ) {
                PRINTF("warn: material[%d] not found: %s\n", i, it);
            } else {
                PRINTF("loaded material[%d]: %s\n", i, it);

                mt.layer[mt.count++].texture = *out;

                ++out;
            }
        }

        // if no materials were loaded, try to signal a checkered placeholder
        if( out == textures ) {
            PRINTF("warn: using placeholder material[%d]=texture_checker\n", i);
            *out++ = invalid;
        }

        int count = (int)(intptr_t)(out - textures);
        model->num_textures += count;
        array_push(model->texture_names, STRDUP(&str[m->material]));

        array_push(model->materials, mt);
#endif

    }

    if( array_count(model->materials) == 0 ) {
        material_t mt = {0};
        mt.name = "placeholder";
        mt.count = 1;
        mt.layer[0].color = vec4(1,1,1,1);
        mt.layer[0].texture = texture_checker().id;

        array_push(model->materials, mt);
    }

    return true;
}

model_t model_from_mem(const void *mem, int len, int flags) {
    model_t m = {0};

    const char *ptr = (const char *)mem;
    // can't cache shader programs since we enable features via flags here
    // static int shaderprog = -1;
    // if( shaderprog < 0 ) {
        const char *symbols[] = { "{{include-shadowmap}}", vfs_read("shaders/fs_0_0_shadowmap_lit.glsl") }; // #define RIM
        int shaderprog = shader(strlerp(1,symbols,vfs_read("shaders/vs_323444143_16_332_model.glsl")), strlerp(1,symbols,vfs_read("shaders/fs_32_4_model.glsl")), //fs,
            "att_position,att_texcoord,att_normal,att_tangent,att_instanced_matrix,,,,att_indexes,att_weights,att_vertexindex,att_color,att_bitangent","fragColor",
            va("SHADING_PHONG,%s", (flags&MODEL_RIMLIGHT)?"RIM":""));
    // }

    iqm_t *q = CALLOC(1, sizeof(iqm_t));
    program = shaderprog;

    int error = 1;
    if( ptr && len ) {
        struct iqmheader hdr; memcpy(&hdr, ptr, sizeof(hdr)); ptr += sizeof(hdr);
        if( !memcmp(hdr.magic, IQM_MAGIC, sizeof(hdr.magic))) {
            lil32p(&hdr.version, (sizeof(hdr) - sizeof(hdr.magic))/sizeof(uint32_t));
            if(hdr.version == IQM_VERSION) {
                buf = CALLOC(hdr.filesize, sizeof(uint8_t));
                memcpy(buf + sizeof(hdr), ptr, hdr.filesize - sizeof(hdr));
                error = 0;
                if( hdr.num_meshes > 0 && !(flags & MODEL_NO_MESHES) )     error |= !model_load_meshes(q, &hdr, &m);
                if( hdr.num_meshes > 0 && !(flags & MODEL_NO_TEXTURES) )   error |= !model_load_textures(q, &hdr, &m);
                if( hdr.num_anims  > 0 && !(flags & MODEL_NO_ANIMATIONS) ) error |= !model_load_anims(q, &hdr);
                if( buf != meshdata && buf != animdata ) FREE(buf);
            }
        }
    }

    if( error ) {
        PRINTF("Error: cannot load %s", "model");
        FREE(q), q = 0;
    } else {
        #undef vao
        #undef ibo
        #undef vbo
        m.vao = q->vao;
        m.ibo = q->ibo;
        m.vbo = q->vbo;
        m.num_verts = numverts;
        #define vao (q->vao)
        #define ibo (q->ibo)
        #define vbo (q->vbo)

        // m.boxes = bounds; // <@todo
        m.num_meshes = nummeshes;
        m.num_triangles = numtris;
        m.num_joints = numjoints;
        //m.num_poses = numposes;
        m.num_anims = numanims;
        m.num_frames = numframes;
        m.iqm = q;
        m.curframe = model_animate(m, 0);
        #undef program
        m.program = (q->program);
        #define program (q->program)

        //m.num_textures = nummeshes; // assume 1 texture only per mesh
        #undef textures
        m.textures = (q->textures);
        #define textures (q->textures)

        m.flags = flags;

        id44(m.pivot);

        m.num_instances = 0;
        m.instanced_matrices = m.pivot;

        glGenBuffers(1, &m.vao_instanced);
        model_set_state(m);
    }
    return m;
}
model_t model(const char *filename, int flags) {
    int len;  // vfs_pushd(filedir(filename))
    char *ptr = vfs_load(filename, &len); // + vfs_popd
    return model_from_mem( ptr, len, flags );
}

bool model_get_bone_pose(model_t m, unsigned joint, mat34 *out) {
    if(!m.iqm) return false;
    iqm_t *q = m.iqm;

    if(joint >= numjoints) return false;

    multiply34x2(*out, outframe[joint], baseframe[joint]);
    return true;
}

anim_t clip(float minframe, float maxframe, float blendtime, unsigned flags) {
    return ((anim_t){minframe, maxframe, blendtime, flags, 1e6});
}
anim_t loop(float minframe, float maxframe, float blendtime, unsigned flags) {
    return clip(minframe, maxframe, blendtime, flags | ANIM_LOOP);
}

static
void anim_tick(anim_t *p, bool is_primary, float delta) { // delta can be negative (reverses anim)
    if( !is_primary ) p->active = 0;
    if( is_primary && !p->active ) {
        p->active = 1;
        p->timer = 0;
        p->alpha = 0;
        if( p->flags & ANIM_DONT_RESET_AFTER_USE ) {} else p->curframe = 1e6;
    }

    p->alpha = 1 - ease(p->timer / p->blendtime, p->easing);
    p->timer += window_delta();

    p->curframe += delta;
    if(p->curframe < p->from || p->curframe > p->to ) p->curframe = delta >= 0 ? p->from : p->to;
    p->pose = pose(delta >= 0, p->curframe, p->from, p->to, p->flags & ANIM_LOOP, NULL);
}

float model_animate_blends(model_t m, anim_t *primary, anim_t *secondary, float delta) {
    if(!m.iqm) return -1;
    iqm_t *q = m.iqm;

    anim_tick(primary, 1, delta);
    anim_tick(secondary, 0, delta);

    float alpha = primary->alpha;
//  if( alpha <= 0 ) return model_animate(m, primary.pose.x);
//  if( alpha >= 1 ) return model_animate(m, secondary.pose.x);

    unsigned frame1 = primary->pose.x;
    unsigned frame2 = primary->pose.y;
    float    alphaA = primary->pose.z;

    unsigned frame3 = secondary->pose.x;
    unsigned frame4 = secondary->pose.y;
    float    alphaB = secondary->pose.z;

    mat34 *mat1 = &frames[frame1 * numjoints];
    mat34 *mat2 = &frames[frame2 * numjoints];
    mat34 *mat3 = &frames[frame3 * numjoints];
    mat34 *mat4 = &frames[frame4 * numjoints];

    for(int i = 0; i < numjoints; i++) {
        mat34 matA, matB, matF;
        lerp34(matA, mat1[i], mat2[i], alphaA);
        lerp34(matB, mat3[i], mat4[i], alphaB);
        lerp34(matF, matA, matB, alpha );
        if(joints[i].parent >= 0) multiply34x2(outframe[i], outframe[joints[i].parent], matF);
        else copy34(outframe[i], matF);
    }

    return frame1 + alpha;
}

vec3 pose(bool forward_time, float curframe, int minframe, int maxframe, bool loop, float *retframe) {
    float offset = curframe - (int)curframe;
#if 1
    int frame1 = (int)curframe;
    int frame2 = frame1 + (forward_time ? 1 : -1);
#else
    float frame1 = curframe;
    float frame2 = curframe + (forward_time ? 1 : -1);
#endif

    if( loop ) {
        int distance = maxframe - minframe;
        frame1 = fmod(frame1 - minframe, distance) + minframe; // frame1 >= maxframe ? minframe : frame1 < minframe ? maxframe - clampf(minframe - frame1, 0, distance) : frame1;
        frame2 = fmod(frame2 - minframe, distance) + minframe; // frame2 >= maxframe ? minframe : frame2 < minframe ? maxframe - clampf(minframe - frame2, 0, distance) : frame2;
        if(retframe) *retframe = fmod(frame1 + offset - minframe, distance) + minframe;
    } else {
        frame1 = clampf(frame1, minframe, maxframe);
        frame2 = clampf(frame2, minframe, maxframe);
        if(retframe) *retframe = clampf(frame1 + offset, minframe, maxframe);
    }

    return vec3(frame1 + (offset > 0 && offset < 1 ? offset : 0),frame2,offset);
}

float model_animate_clip(model_t m, float curframe, int minframe, int maxframe, bool loop) {
    if(!m.iqm) return -1;
    iqm_t *q = m.iqm;

    float retframe = -1;
    if( numframes > 0 ) {
        vec3 p = pose(curframe >= m.curframe, curframe, minframe, maxframe, loop, &retframe);
        int frame1 = p.x;
        int frame2 = p.y;
        float offset = p.z;

        mat34 *mat1 = &frames[frame1 * numjoints];
        mat34 *mat2 = &frames[frame2 * numjoints];

        // @todo: add animation blending and inter-frame blending here
        for(int i = 0; i < numjoints; i++) {
            mat34 mat; lerp34(mat, mat1[i], mat2[i], offset);
            if(joints[i].parent >= 0) multiply34x2(outframe[i], outframe[joints[i].parent], mat);
            else copy34(outframe[i], mat);
        }
    }

    return retframe;
}

void model_render_skeleton(model_t m, mat44 M) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    if(!numjoints) return;

    ddraw_ontop_push(true);
    ddraw_color_push(RED);

    for( int joint = 0; joint < numjoints; joint++ ) {
        if( joints[joint].parent < 0) continue;

        // bone space...
        mat34 f;
        model_get_bone_pose(m, joint, &f);
        vec3 pos = vec3(f[3],f[7],f[11]);

        model_get_bone_pose(m, joints[joint].parent, &f);
        vec3 src = vec3(f[3],f[7],f[11]);

        // ...to model space
        src = transform344(M, src);
        pos = transform344(M, pos);

        // red line
        ddraw_color(RED);
//      ddraw_line(src, pos);
        ddraw_bone(src, pos);

        // green dot
        ddraw_color(GREEN);
        ddraw_point(pos);

        // yellow text
        ddraw_color(YELLOW);
        ddraw_text(pos, 0.005, va("%d", joint));
    }

    ddraw_color_pop();
    ddraw_ontop_pop();
}

float model_animate(model_t m, float curframe) {
    if(!m.iqm) return -1;
    iqm_t *q = m.iqm;
    return model_animate_clip(m, curframe, 0, numframes-1, true);
}

static
void model_draw_call(model_t m) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glBindVertexArray( vao );

    struct iqmtriangle *tris = NULL;
    for(int i = 0; i < nummeshes; i++) {
        struct iqmmesh *im = &meshes[i];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[i] );
        glUniform1i(glGetUniformLocation(program, "fsDiffTex"), 0 /*<-- unit!*/ );

        int loc;
        if ((loc = glGetUniformLocation(program, "u_textured")) >= 0) {
            bool textured = !!textures[i] && textures[i] != texture_checker().id; // m.materials[i].layer[0].texture != texture_checker().id;
            glUniform1i(loc, textured ? GL_TRUE : GL_FALSE);
            if ((loc = glGetUniformLocation(program, "u_diffuse")) >= 0) {
                glUniform4f(loc, m.materials[i].layer[0].color.r, m.materials[i].layer[0].color.g, m.materials[i].layer[0].color.b, m.materials[i].layer[0].color.a);
            }
        }

        glDrawElementsInstanced(GL_TRIANGLES, 3*im->num_triangles, GL_UNSIGNED_INT, &tris[im->first_triangle], m.num_instances);
        profile_incstat("Render.num_drawcalls", +1);
        profile_incstat("Render.num_triangles", +im->num_triangles);
    }

    glBindVertexArray( 0 );
}

void model_render_instanced(model_t m, mat44 proj, mat44 view, mat44* models, int shader, unsigned count) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    // @fixme: instanced billboards
    mat44 mv; multiply44x2(mv, view, models[0]);
    if( m.billboard ) {
        float d = sqrt(mv[4*0+0] * mv[4*0+0] + mv[4*1+1] * mv[4*1+1] + mv[4*2+2] * mv[4*2+2]);
        if(m.billboard & 4) mv[4*0+0] = d, mv[4*0+1] =  0, mv[4*0+2] = 0;
        if(m.billboard & 2) mv[4*1+0] = 0, mv[4*1+1] = -d, mv[4*1+2] = 0;
        if(m.billboard & 1) mv[4*2+0] = 0, mv[4*2+1] =  0, mv[4*2+2] = d;
    }

    if( count != m.num_instances ) {
        m.num_instances = count;
        m.instanced_matrices = (float*)models;
        model_set_state(m);
    }

    model_set_uniforms(m, shader > 0 ? shader : program, mv, proj, view, models[0]);
    model_draw_call(m);
}

void model_render(model_t m, mat44 proj, mat44 view, mat44 model, int shader) {
    model_render_instanced(m, proj, view, (mat44*)model, shader, 1);
}

// static
aabb aabb_transform( aabb A, mat44 M ) {
    // Based on "Transforming Axis-Aligned Bounding Boxes" by Jim Arvo, 1990
    aabb B = { {M[12],M[13],M[14]}, {M[12],M[13],M[14]} }; // extract translation from mat44
    for( int i = 0; i < 3; i++ )
    for( int j = 0; j < 3; j++ ) {
        float a = M[i*4+j] * j[&A.min.x]; // use mat33 from mat44
        float b = M[i*4+j] * j[&A.max.x]; // use mat33 from mat44
        if( a < b ) {
            i[&B.min.x] += a;
            i[&B.max.x] += b;
        } else {
            i[&B.min.x] += b;
            i[&B.max.x] += a;
        }
    }
    return B;
}

aabb model_aabb(model_t m, mat44 transform) {
    iqm_t *q = m.iqm;
    if( q && bounds ) {
    int f = ( (int)m.curframe ) % (numframes + !numframes);
    vec3 bbmin = ptr3(bounds[f].bbmin);
    vec3 bbmax = ptr3(bounds[f].bbmax);
    return aabb_transform(aabb(bbmin,bbmax), transform);
    }
    return aabb(vec3(0,0,0),vec3(0,0,0));
}

void model_destroy(model_t m) {
    FREE(m.verts);
    for( int i = 0, end = array_count(m.texture_names); i < end; ++i ) {
        FREE(m.texture_names[i]);
    }
    array_free(m.texture_names);

    iqm_t *q = m.iqm;
//    if(m.mesh) mesh_destroy(m.mesh);
    FREE(outframe);
    FREE(colormaps);
    FREE(textures);
    FREE(baseframe);
    FREE(inversebaseframe);
    if(animdata != meshdata) FREE(animdata);
    //FREE(meshdata);
    FREE(frames);
    FREE(buf);
    FREE(q);
}

#undef program
#undef meshdata
#undef animdata
#undef nummeshes
#undef numtris
#undef numverts
#undef numjoints
#undef numframes
#undef numanims
#undef meshes
#undef textures
#undef joints
#undef poses
#undef anims
#undef baseframe
#undef inversebaseframe
#undef outframe
#undef frames
#undef vao
#undef ibo
#undef vbo
#undef bonematsoffset
#undef buf
#undef bounds
#undef colormaps

anims_t animations(const char *pathfile, int flags) {
    anims_t a = {0};
    char *anim_file = vfs_read(strendi(pathfile,".txt") ? pathfile : va("%s@animlist.txt", pathfile));
    if( anim_file ) {
        // deserialize anim
        a.speed = 1.0;
        for each_substring(anim_file, "\r\n", anim) {
            int from, to;
            char anim_name[128] = {0};
            if( sscanf(anim, "%*s %d-%d %127[^\r\n]", &from, &to, anim_name) != 3) continue;
            array_push(a.anims, !!strstri(anim_name, "loop") || !strcmpi(anim_name, "idle") ? loop(from, to, 0, 0) : clip(from, to, 0, 0)); // [from,to,flags]
            array_back(a.anims)->name = strswap(strswap(strswap(STRDUP(anim_name), "Loop", ""), "loop", ""), "()", ""); // @leak
        }
    } else {
        // placeholder
        array_push(a.anims, clip(0,1,0,0));
        array_back(a.anims)->name = STRDUP("Error"); // @leak
    }
    return a;
}
#line 0

#line 1 "fwk_renderdd.c"
static const char *dd_vs = "//" FILELINE "\n"
    "in vec3 att_position;\n"
    "uniform mat4 u_MVP;\n"
    "uniform vec3 u_color;\n"
    "out vec3 out_color;\n"
    "void main() {\n"
    "    gl_Position = u_MVP * vec4(att_position, 1.0);\n"
    "    gl_PointSize = 4.0; /* for GL_POINTS draw commands */\n"
    "    out_color = u_color;\n"
    "}";

static const char *dd_fs = "//" FILELINE "\n"
    // "precision mediump float;\n"
    "in vec3 out_color;\n"
    "out vec4 fragcolor;\n"
    "void main() {\n"
    "   fragcolor = vec4(out_color, 1.0);\n"
    "}";

#define X(x) RGBX(x,255)
const uint32_t secret_palette[32] = { // pico8 secret palette (CC0, public domain)
X(0x000000),X(0x1D2B53),X(0x7E2553),X(0x008751),X(0xAB5236),X(0x5F574F),X(0xC2C3C7),X(0xFFF1E8), /*00.07*/
X(0xFF004D),X(0xFFA300),X(0xFFEC27),X(0x00E436),X(0x29ADFF),X(0x83769C),X(0xFF77A8),X(0xFFCCAA), /*08.15*/
X(0x291814),X(0x111D35),X(0x422136),X(0x125359),X(0x742F29),X(0x49333B),X(0xA28879),X(0xF3EF7D), /*16.23*/
X(0xBE1250),X(0xFF6C24),X(0xA8E72E),X(0x00B543),X(0x065AB5),X(0x754665),X(0xFF6E59),X(0xFF9D81), /*24.31*/
};
#undef X

typedef struct text2d_cmd {
    const char *str;
    uint32_t col;
    vec3 pos;
    float sca;
} text2d_cmd;

static uint32_t                  dd_color = ~0u;
static GLuint                    dd_program = -1;
static int                       dd_u_color = -1;
static map(unsigned,array(vec3)) dd_lists[2][3] = {0}; // [0/1 ontop][0/1/2 thin lines/thick lines/points]
static bool                      dd_use_line = 0;
static bool                      dd_ontop = 0;
static array(text2d_cmd)         dd_text2d;
static array(vec4)               dd_matrix2d;

void ddraw_push_2d() {
    float width = window_width();
    float height = window_height();
    float zdepth_max = window_height();
    array_push(dd_matrix2d, vec4(width,height,zdepth_max,0));

    ddraw_flush();
}

void ddraw_pop_2d() {
    vec4 dim = *array_back(dd_matrix2d);
    array_pop(dd_matrix2d);

    mat44 id, proj;
    id44(id);
    ortho44(proj, 0,dim.x,dim.y,0, -dim.z, +dim.z);
    ddraw_flush_projview(proj, id);
}

void ddraw_flush() {
    ddraw_flush_projview(camera_get_active()->proj, camera_get_active()->view);
}

void ddraw_flush_projview(mat44 proj, mat44 view) {
    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    mat44 mvp;
    multiply44x2(mvp, proj, view); // MVP where M=id

    glUseProgram(dd_program);
    glUniformMatrix4fv(glGetUniformLocation(dd_program, "u_MVP"), 1, GL_FALSE, mvp);

    static GLuint vao, vbo;
    if(!vao) glGenVertexArrays(1, &vao);    glBindVertexArray(vao);
    if(!vbo) glGenBuffers(1, &vbo);         glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_PROGRAM_POINT_SIZE); // for GL_POINTS
    glEnable(GL_LINE_SMOOTH); // for GL_LINES (thin)

    for( int i = 0; i < 3; ++i ) { // [0] thin, [1] thick, [2] points
        GLenum mode = i < 2 ? GL_LINES : GL_POINTS;
        glLineWidth(i == 1 ? 1 : 0.3); // 0.625);
        for each_map(dd_lists[dd_ontop][i], unsigned, rgb, array(vec3), list) {
            int count = array_count(list);
            if(!count) continue;
                // color
                vec3 rgbf = {((rgb>>16)&255)/255.f,((rgb>>8)&255)/255.f,((rgb>>0)&255)/255.f};
                glUniform3fv(dd_u_color, GL_TRUE, &rgbf.x);
                // config vertex data
                glBufferData(GL_ARRAY_BUFFER, count * 3 * 4, list, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
                // feed vertex data
                glDrawArrays(mode, 0, count);
                profile_incstat("Render.num_drawcalls", +1);
                profile_incstat(i < 2 ? "Render.num_lines" : "Render.num_points", count);
            array_clear(list);
        }
    }

    if(array_count(dd_text2d)) { // text 2d
        // queue
        for(int i = 0; i < array_count(dd_text2d); ++i) {
            ddraw_color(dd_text2d[i].col);
            ddraw_text(dd_text2d[i].pos, dd_text2d[i].sca, dd_text2d[i].str); 
        }

        // flush
        float mvp[16]; float zdepth_max = 1;
        ortho44(mvp, -window_width()/2, window_width()/2, -window_height()/2, window_height()/2, -1, 1);
        translate44(mvp, -window_width()/2, window_height()/2, 0);
        glUniformMatrix4fv(glGetUniformLocation(dd_program, "u_MVP"), 1, GL_FALSE, mvp);
        for( int i = 0; i < 3; ++i ) { // [0] thin, [1] thick, [2] points
            GLenum mode = i < 2 ? GL_LINES : GL_POINTS;
            glLineWidth(i == 1 ? 1 : 0.3); // 0.625);
            for each_map(dd_lists[dd_ontop][i], unsigned, rgb, array(vec3), list) {
                int count = array_count(list);
                if(!count) continue;
                    // color
                    vec3 rgbf = {((rgb>>16)&255)/255.f,((rgb>>8)&255)/255.f,((rgb>>0)&255)/255.f};
                    glUniform3fv(dd_u_color, GL_TRUE, &rgbf.x);
                    // config vertex data
                    glBufferData(GL_ARRAY_BUFFER, count * 3 * 4, list, GL_STATIC_DRAW);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
                    // feed vertex data
                    glDrawArrays(mode, 0, count);
                    profile_incstat("Render.num_drawcalls", +1);
                    profile_incstat(i < 2 ? "Render.num_lines" : "Render.num_points", count);
                array_clear(list);
            }
        }

        // clear
        array_resize(dd_text2d, 0);
    }
    
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_PROGRAM_POINT_SIZE);

    glBindVertexArray(0);

    ddraw_color(WHITE); // reset color for next drawcall
}

static array(bool) dd_ontops;
void ddraw_ontop(int enabled) {
    dd_ontop = !!enabled;
}
void ddraw_ontop_push(int enabled) {
    array_push(dd_ontops, dd_ontop);
    dd_ontop = !!enabled;
}
void ddraw_ontop_pop() {
    bool *pop = array_pop(dd_ontops);
    if(pop) dd_ontop = *pop;
}

static array(uint32_t) dd_colors;
void ddraw_color(unsigned rgb) {
    dd_color = rgb;
}
void ddraw_color_push(unsigned rgb) {
    array_push(dd_colors, dd_color);
    dd_color = rgb;
}
void ddraw_color_pop() {
    unsigned *pop = array_pop(dd_colors);
    if(pop) dd_color = *pop;
}

void ddraw_point(vec3 from) {
    array(vec3) *found = map_find_or_add(dd_lists[dd_ontop][2], dd_color, 0);
    array_push(*found, from);
}
void ddraw_line_thin(vec3 from, vec3 to) { // thin lines
    array(vec3) *found = map_find_or_add(dd_lists[dd_ontop][0], dd_color, 0);
    array_push(*found, from);
    array_push(*found, to);
}
void ddraw_line(vec3 from, vec3 to) { // thick lines
    array(vec3) *found = map_find_or_add(dd_lists[dd_ontop][1], dd_color, 0);
    array_push(*found, from);
    array_push(*found, to);
}
void ddraw_line_dashed(vec3 from, vec3 to) { // thick lines
    vec3 dist = sub3(to, from); vec3 unit = norm3(dist);
    for( float len = 0, mag = len3(dist) / 2; len < mag; ++len ) {
        to = add3(from, unit);
        ddraw_line(from, to);
        from = add3(to, unit);
    }
}
void ddraw_triangle(vec3 pa, vec3 pb, vec3 pc) {
    ddraw_line(pa, pb);
    ddraw_line(pa, pc);
    ddraw_line(pb, pc);
}
void ddraw_axis(float units) {
    ddraw_color(RED);   ddraw_line(vec3(0,0,0), vec3(units,0,0)); ddraw_line_dashed(vec3(0,0,0), vec3(-units,0,0));
    ddraw_color(GREEN); ddraw_line(vec3(0,0,0), vec3(0,units,0)); ddraw_line_dashed(vec3(0,0,0), vec3(0,-units,0));
    ddraw_color(BLUE);  ddraw_line(vec3(0,0,0), vec3(0,0,units)); ddraw_line_dashed(vec3(0,0,0), vec3(0,0,-units));
}
void ddraw_ground_(float scale) { // 10x10
    ddraw_color( WHITE ); // outer
    for( float i = -scale, c = 0; c <= 20; c += 20, i += c * (scale/10) ) {
        ddraw_line(vec3(-scale,0,i), vec3(+scale,0,i)); // horiz
        ddraw_line(vec3(i,0,-scale), vec3(i,0,+scale)); // vert
    }
    ddraw_color( RGB3(149,149,149) ); // inner, light grey
    for( float i = -scale + scale/10, c = 1; c < 20; ++c, i += (scale/10) ) {
        ddraw_line_thin(vec3(-scale,0,i), vec3(+scale,0,i)); // horiz
        ddraw_line_thin(vec3(i,0,-scale), vec3(i,0,+scale)); // vert
    }
}
void ddraw_ground(float scale) {
    if( scale ) {
        ddraw_ground_(scale);
    } else {
        ddraw_ground_(100);
        ddraw_ground_(10);
        ddraw_ground_(1);
        ddraw_ground_(0.1);
        ddraw_ground_(0.01);
    }
}
void ddraw_grid(float scale) {
    ddraw_ground(scale);
    ddraw_axis(scale ? scale : 100);
}

void ddraw_text2d(vec2 pos, const char *text) {
    struct text2d_cmd t = {0};
    t.sca = 0.5f; // 0.5 is like vertical 12units each
    t.pos = vec3(pos.x, 0 - pos.y - 12, 0);
    t.str = text;
    t.col = YELLOW;
    array_push(dd_text2d, t);
}

void (ddraw_text)(vec3 pos, float scale, const char *text) {
    // [ref] http://paulbourke.net/dataformats/hershey/ (PD)
    // [ref] https://sol.gfxile.net/hershey/fontprev.html (WTFPL2)
    static const char *hershey[] = { /* simplex font */
    "AQ","IKFVFH@@FCEBFAGBFC","FQEVEO@@MVMO","LVLZE:@@RZK:@@EMSM@@DGRG","[UIZI=@@MZ"
    "M=@@RSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","`YVVDA@@IVKTKRJPHOFODQDSEUGVIVK"
    "UNTQTTUVV@@RHPGOEOCQASAUBVDVFTHRH","c[XMXNWOVOUNTLRGPDNBLAHAFBECDEDGEIFJMNNOOQ"
    "OSNULVJUISIQJNLKQDSBUAWAXBXC","HKFTEUFVGUGSFQEP","KOLZJXHUFQELEHFCH?J<L:","KOD"
    "ZFXHUJQKLKHJCH?F<D:","IQIVIJ@@DSNM@@NSDM","F[NSNA@@EJWJ","IKGBFAEBFCGBG@F>E=",\
    "C[EJWJ","FKFCEBFAGBFC","CWUZC:","RUJVGUERDMDJEEGBJALAOBQERJRMQROULVJV","EUGRIS"
    "LVLA","OUEQERFTGUIVMVOUPTQRQPPNNKDARA","PUFVQVKNNNPMQLRIRGQDOBLAIAFBECDE","GUN"
    "VDHSH@@NVNA","RUPVFVEMFNIOLOONQLRIRGQDOBLAIAFBECDE","XUQSPUMVKVHUFREMEHFDHBKAL"
    "AOBQDRGRHQKOMLNKNHMFKEH","FURVHA@@DVRV","^UIVFUESEQFOHNLMOLQJRHREQCPBMAIAFBECD"
    "EDHEJGLJMNNPOQQQSPUMVIV","XUQOPLNJKIJIGJELDODPESGUJVKVNUPSQOQJPENBKAIAFBED","L"
    "KFOENFMGNFO@@FCEBFAGBFC","OKFOENFMGNFO@@GBFAEBFCGBG@F>E=","DYUSEJUA","F[EMWM@@"
    "EGWG","DYESUJEA","USDQDRETFUHVLVNUOTPRPPONNMJKJH@@JCIBJAKBJC","x\\SNRPPQMQKPJO"
    "ILIIJGLFOFQGRI@@MQKOJLJIKGLF@@SQRIRGTFVFXHYKYMXPWRUTSUPVMVJUHTFREPDMDJEGFEHCJB"
    "MAPASBUCVD@@TQSISGTF","ISJVBA@@JVRA@@EHOH","XVEVEA@@EVNVQURTSRSPRNQMNL@@ELNLQK"
    "RJSHSERCQBNAEA","SVSQRSPUNVJVHUFSEQDNDIEFFDHBJANAPBRDSF","PVEVEA@@EVLVOUQSRQSN"
    "SIRFQDOBLAEA","LTEVEA@@EVRV@@ELML@@EARA","ISEVEA@@EVRV@@ELML","WVSQRSPUNVJVHUF"
    "SEQDNDIEFFDHBJANAPBRDSFSI@@NISI","IWEVEA@@SVSA@@ELSL","CIEVEA","KQMVMFLCKBIAGA"
    "EBDCCFCH","IVEVEA@@SVEH@@JMSA","FREVEA@@EAQA","LYEVEA@@EVMA@@UVMA@@UVUA","IWEV"
    "EA@@EVSA@@SVSA","VWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV","NVEVEA@@EVNVQU"
    "RTSRSORMQLNKEK","YWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV@@MES?","QVEVEA@@"
    "EVNVQURTSRSPRNQMNLEL@@LLSA","UURSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","FQIV"
    "IA@@BVPV","KWEVEGFDHBKAMAPBRDSGSV","FSBVJA@@RVJA","LYCVHA@@MVHA@@MVRA@@WVRA",""
    "FUDVRA@@RVDA","GSBVJLJA@@RVJL","IURVDA@@DVRV@@DARA","LOEZE:@@FZF:@@EZLZ@@E:L:",
    "COAVO>","LOJZJ:@@KZK:@@DZKZ@@D:K:","KQGPISKP@@DMIRNM@@IRIA","CQA?Q?","HKGVFUES"
    "EQFPGQFR","RTPOPA@@PLNNLOIOGNELDIDGEDGBIALANBPD","RTEVEA@@ELGNIOLONNPLQIQGPDNB"
    "LAIAGBED","OSPLNNLOIOGNELDIDGEDGBIALANBPD","RTPVPA@@PLNNLOIOGNELDIDGEDGBIALANB"
    "PD","RSDIPIPKOMNNLOIOGNELDIDGEDGBIALANBPD","IMKVIVGUFRFA@@COJO","WTPOP?O<N;L:I"
    ":G;@@PLNNLOIOGNELDIDGEDGBIALANBPD","KTEVEA@@EKHNJOMOONPKPA","IIDVEUFVEWDV@@EOE"
    "A","LKFVGUHVGWFV@@GOG>F;D:B:","IREVEA@@OOEE@@IIPA","CIEVEA","S_EOEA@@EKHNJOMOO"
    "NPKPA@@PKSNUOXOZN[K[A","KTEOEA@@EKHNJOMOONPKPA","RTIOGNELDIDGEDGBIALANBPDQGQIP"
    "LNNLOIO","RTEOE:@@ELGNIOLONNPLQIQGPDNBLAIAGBED","RTPOP:@@PLNNLOIOGNELDIDGEDGBI"
    "ALANBPD","INEOEA@@EIFLHNJOMO","RROLNNKOHOENDLEJGILHNGOEODNBKAHAEBDD","IMFVFEGB"
    "IAKA@@COJO","KTEOEEFBHAKAMBPE@@POPA","FQCOIA@@OOIA","LWDOHA@@LOHA@@LOPA@@TOPA",
    "FRDOOA@@OODA","JQCOIA@@OOIAG=E;C:B:","IROODA@@DOOO@@DAOA","hOJZHYGXFVFTGRHQIOI"
    "MGK@@HYGWGUHSIRJPJNILEJIHJFJDIBHAG?G=H;@@GIIGIEHCGBF@F>G<H;J:","CIEZE:","hOFZH"
    "YIXJVJTIRHQGOGMIK@@HYIWIUHSGRFPFNGLKJGHFFFDGBHAI?I=H;@@IIGGGEHCIBJ@J>I<H;F:",""
    "XYDGDIELGMIMKLOIQHSHUIVK@@DIEKGLILKKOHQGSGUHVKVM" };
    vec3 src = pos, old = {0}; float abs_scale = absf(scale);
    for( signed char c; (c = *text++, c > 0 && c < 127); ) {
        if( c == '\n' || c == '\r' ) {
            pos.x = src.x, pos.y -= scale * ((signed char)hershey['W'-32][1] - 65) * 1.25f; // spacing @1
        } else {
            const char *glyph = (const char*)hershey[c - 32];
            if( c > 32 ) for( int pen = 0, i = 0; i < (glyph[0] - 65); i++ ) { // verts @0
                int x = glyph[2 + i*2 + 0] - 65, y = glyph[2 + i*2 + 1] - 65;
                if( x == -1 && y == -1 ) pen = 0; else {
                    vec3 next = add3(pos, vec3(abs_scale*x, scale*y, 0));
                    if( !pen ) pen = 1; else ddraw_line(old, next);
                    old = next;
                }
            }
            pos.x += abs_scale * (glyph[1] - 65); // spacing @1
        }
    }
}

void ddraw_prism(vec3 center, float radius, float height, vec3 normal, int segments) {
    vec3 left = {0}, up = {0};
    ortho3(&left, &up, normal);

    vec3 point, lastPoint;
    up = scale3(up, radius);
    left = scale3(left, radius);
    lastPoint = add3(center, up);
    vec3 pivot = add3(center, scale3(normal, height));

    for (int i = 1; i <= segments; ++i) {
        const float radians = (C_PI * 2) * i / segments;

        vec3 vs = scale3(left, sinf(radians));
        vec3 vc = scale3(up,   cosf(radians));

        point = add3(center, vs);
        point = add3(point,  vc);

        ddraw_line(lastPoint, point);
        if( height > 0 ) ddraw_line(point, pivot);
        else if(height < 0) {
            ddraw_line(point, add3(point,scale3(normal, -height)));
        }
        lastPoint = point;
    }

    if(height < 0) ddraw_prism(add3(center, scale3(normal, -height)), radius, 0, normal, segments);
}

void ddraw_cube(vec3 center, float radius) { // draw_prism(center, 1, -1, vec3(0,1,0), 4);
    float half = radius * 0.5f;
    vec3 l = vec3(center.x-half,center.y+half,center.z-half); // left-top-far
    vec3 r = vec3(center.x+half,center.y-half,center.z+half); // right-bottom-near

    ddraw_line(l, vec3(r.x,l.y,l.z));
    ddraw_line(vec3(r.x,l.y,l.z), vec3(r.x,l.y,r.z));
    ddraw_line(vec3(r.x,l.y,r.z), vec3(l.x,l.y,r.z));
    ddraw_line(vec3(l.x,l.y,r.z), l);
    ddraw_line(l, vec3(l.x,r.y,l.z));

    ddraw_line(r, vec3(l.x,r.y,r.z));
    ddraw_line(vec3(l.x,r.y,r.z), vec3(l.x,r.y,l.z));
    ddraw_line(vec3(l.x,r.y,l.z), vec3(r.x,r.y,l.z));
    ddraw_line(vec3(r.x,r.y,l.z), r);
    ddraw_line(r, vec3(r.x,l.y,r.z));

    ddraw_line(vec3(l.x,l.y,r.z), vec3(l.x,r.y,r.z));
    ddraw_line(vec3(r.x,l.y,l.z), vec3(r.x,r.y,l.z));
}

#if 0 // @fixme: broken
void ddraw_cube44(vec3 radius, mat44 M) {
    float m33[9]; extract33(m33, M); // = { M[0,1,2], M[4,5,6], M[8,9,10] }    
    ddraw_cube33( vec3(M[12], M[13], M[14]), radius, m33 );
}
#endif

void ddraw_cube33(vec3 center, vec3 radius, mat33 M) {
    vec3 half = scale3(radius, 0.5f);
    vec3 l = vec3(-half.x,+half.y,-half.z); // left-top-far
    vec3 r = vec3(+half.x,-half.y,+half.z); // right-bottom-near

    vec3 points[8] = {
        vec3(l.x, r.y, r.z),
        vec3(l.x, r.y, l.z),
        vec3(r.x, r.y, l.z),
        vec3(r.x, r.y, r.z),
        vec3(l.x, l.y, r.z),
        vec3(l.x, l.y, l.z),
        vec3(r.x, l.y, l.z),
        vec3(r.x, l.y, r.z),
    };

    for( int i = 0; i < 8; ++i ) {
        points[i] = add3(center, transform33(M, points[i]));
    }

    ddraw_bounds(points);
}

void ddraw_normal(vec3 pos, vec3 n) {
    ddraw_color(YELLOW);
    ddraw_line(pos, add3(pos, norm3(n)));
}

void ddraw_circle(vec3 pos, vec3 n, float r) { ddraw_prism(pos, r, 0, n, 32); }
void ddraw_ring(vec3 pos, vec3 n, float r) { ddraw_circle(pos,n,r);ddraw_circle(pos,n,r*0.90); }
void ddraw_hexagon(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 6); }
void ddraw_pentagon(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 5); }
void ddraw_square(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 4); }
//void ddraw_triangle(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 3); }
void ddraw_sphere(vec3 center, float radius) {
    float lod = 8, yp = -radius, rp = 0, y, r, x, z;
    for( int j = 1; j <= lod / 2; ++j, yp = y, rp = r ) {
        y = j * 2.f / (lod / 2) - 1;
        r = cosf(y * 3.14159f / 2) * radius;
        y = sinf(y * 3.14159f / 2) * radius;

        float xp = 1, zp = 0;
        for( int i = 1; i <= lod; ++i, xp = x, zp = z ) {
            x = 3.14159f * 2 * i / lod;
            z = sinf(x);
            x = cosf(x);

            vec3 a1 = add3(center, vec3(xp * rp, yp, zp * rp));
            vec3 b1 = add3(center, vec3(xp * r,  y,  zp * r));
            vec3 c1 = add3(center, vec3(x  * r,  y,  z  * r));

            ddraw_line(a1,b1);
            ddraw_line(b1,c1);
            ddraw_line(c1,a1);

            vec3 a2 = add3(center, vec3(xp * rp, yp, zp * rp));
            vec3 b2 = add3(center, vec3(x  * r,  y,  z  * r));
            vec3 c2 = add3(center, vec3(x  * rp, yp, z  * rp));

            ddraw_line(a2,b2);
            ddraw_line(b2,c2);
            ddraw_line(c2,a2);
        }
    }
}
void ddraw_box(vec3 c, vec3 extents) {
    vec3 points[8], whd = scale3(extents, 0.5f);
    #define DD_BOX_V(v, op1, op2, op3) (v).x = c.x op1 whd.x; (v).y = c.y op2 whd.y; (v).z = c.z op3 whd.z
    DD_BOX_V(points[0], -, +, +);
    DD_BOX_V(points[1], -, +, -);
    DD_BOX_V(points[2], +, +, -);
    DD_BOX_V(points[3], +, +, +);
    DD_BOX_V(points[4], -, -, +);
    DD_BOX_V(points[5], -, -, -);
    DD_BOX_V(points[6], +, -, -);
    DD_BOX_V(points[7], +, -, +);
    #undef DD_BOX_V
    ddraw_bounds(points);
}
void ddraw_capsule(vec3 from, vec3 to, float r) {
    /* calculate axis */
    vec3 up, right, forward;
    forward = sub3(to, from);
    forward = norm3(forward);
    ortho3(&right, &up, forward);

    /* calculate first two cone verts (buttom + top) */
    vec3 lastf, lastt;
    lastf = scale3(up,r);
    lastt = add3(to,lastf);
    lastf = add3(from,lastf);

    /* step along circle outline and draw lines */
    enum { step_size = 20 };
    for (int i = step_size; i <= 360; i += step_size) {
        /* calculate current rotation */
        vec3 ax = scale3(right, sinf(i*TO_RAD));
        vec3 ay = scale3(up, cosf(i*TO_RAD));

        /* calculate current vertices on cone */
        vec3 tmp = add3(ax, ay);
        vec3 pf = scale3(tmp, r);
        vec3 pt = scale3(tmp, r);

        pf = add3(pf, from);
        pt = add3(pt, to);

        /* draw cone vertices */
        ddraw_line(lastf, pf);
        ddraw_line(lastt, pt);
        ddraw_line(pf, pt);

        lastf = pf;
        lastt = pt;

        /* calculate first top sphere vert */
        vec3 prevt = scale3(tmp, r);
        vec3 prevf = add3(prevt, from);
        prevt = add3(prevt, to);

        /* sphere (two half spheres )*/
        for (int j = 1; j < 180/step_size; j++) {
            /* angles */
            float ta = j*step_size;
            float fa = 360-(j*step_size);

            /* top half-sphere */
            ax = scale3(forward, sinf(ta*TO_RAD));
            ay = scale3(tmp, cosf(ta*TO_RAD));

            vec3 t = add3(ax, ay);
            pf = scale3(t, r);
            pf = add3(pf, to);
            ddraw_line(pf, prevt);
            prevt = pf;

            /* bottom half-sphere */
            ax = scale3(forward, sinf(fa*TO_RAD));
            ay = scale3(tmp, cosf(fa*TO_RAD));

            t = add3(ax, ay);
            pf = scale3(t, r);
            pf = add3(pf, from);
            ddraw_line(pf, prevf);
            prevf = pf;
        }
    }
}
void ddraw_pyramid(vec3 center, float height, int segments) {
    ddraw_prism(center, 1, height, vec3(0,1,0), segments);
}
void ddraw_cylinder(vec3 center, float height, int segments) {
    ddraw_prism(center, 1, -height, vec3(0,1,0), segments);    
}
void ddraw_diamond(vec3 from, vec3 to, float size) {
    poly p = diamond(from, to, size);
    vec3 *dmd = p.verts;

    vec3 *a = dmd + 0;
    vec3 *b = dmd + 1;
    vec3 *c = dmd + 2;
    vec3 *d = dmd + 3;
    vec3 *t = dmd + 4;
    vec3 *f = dmd + 5;

    /* draw vertices */
    ddraw_line(*a, *b);
    ddraw_line(*b, *c);
    ddraw_line(*c, *d);
    ddraw_line(*d, *a);

    /* draw roof */
    ddraw_line(*a, *t);
    ddraw_line(*b, *t);
    ddraw_line(*c, *t);
    ddraw_line(*d, *t);

    /* draw floor */
    ddraw_line(*a, *f);
    ddraw_line(*b, *f);
    ddraw_line(*c, *f);
    ddraw_line(*d, *f);

    poly_free(&p);
}
void ddraw_cone(vec3 center, vec3 top, float radius) {
    vec3 diff3 = sub3(top, center);
    ddraw_prism(center, radius ? radius : 1, len3(diff3), norm3(diff3), 24);
}
void ddraw_cone_lowres(vec3 center, vec3 top, float radius) {
    vec3 diff3 = sub3(top, center);
    ddraw_prism(center, radius ? radius : 1, len3(diff3), norm3(diff3), 3);
}
void ddraw_bone(vec3 center, vec3 end) {
    vec3 diff3 = sub3(end, center);
    float len = len3(diff3), len10 = len / 10;
    ddraw_prism(center, len10, 0, vec3(1,0,0), 24);
    ddraw_prism(center, len10, 0, vec3(0,1,0), 24);
    ddraw_prism(center, len10, 0, vec3(0,0,1), 24);
    ddraw_line(end, add3(center, vec3(0,+len10,0)));
    ddraw_line(end, add3(center, vec3(0,-len10,0)));
}
void ddraw_bounds(const vec3 points[8]) {
    for( int i = 0; i < 4; ++i ) {
        ddraw_line(points[i], points[(i + 1) & 3]);
        ddraw_line(points[i], points[4 + i]);
        ddraw_line(points[4 + i], points[4 + ((i + 1) & 3)]);
    }
}
void ddraw_bounds_corners(const vec3 points[8]) {
    for( int i = 0; i < 4; ++i ) {
        #define ddraw_unit(a,b) ddraw_line(a,add3(a,norm3(sub3(b,a)))), ddraw_line(b,add3(b,norm3(sub3(a,b))))
        ddraw_unit(points[i], points[(i + 1) & 3]);
        ddraw_unit(points[i], points[4 + i]);
        ddraw_unit(points[4 + i], points[4 + ((i + 1) & 3)]);
        #undef ddraw_unit
    }
}
void ddraw_aabb(vec3 minbb, vec3 maxbb) {
    vec3 points[8], bb[2] = { minbb, maxbb };
    for (int i = 0; i < 8; ++i) {
        points[i].x = bb[(i ^ (i >> 1)) & 1].x;
        points[i].y = bb[     (i >> 1)  & 1].y;
        points[i].z = bb[     (i >> 2)  & 1].z;
    }
    ddraw_bounds/*_corners*/(points);
}
void ddraw_frustum(float projview[16]) {
    mat44 clipmatrix = {0}; // clip matrix
    invert44(clipmatrix, projview);

    // Start with the standard clip volume, then bring it back to world space.
    const vec3 planes[8] = {
        {-1,-1,-1}, {+1,-1,-1}, {+1,+1,-1}, {-1,+1,-1}, // near plane
        {-1,-1,+1}, {+1,-1,+1}, {+1,+1,+1}, {-1,+1,+1}, // far plane
    };

    vec3 points[8];
    float wCoords[8];

    // Transform the planes by the inverse clip matrix:
    for( int i = 0; i < 8; ++i ) {
        // wCoords[i] = matTransformPointXYZW2(&points[i], planes[i], clipmatrix);
        vec3 *out = &points[i], in = planes[i]; const float *m = clipmatrix;
        out->x = (m[0] * in.x) + (m[4] * in.y) + (m[ 8] * in.z) + m[12]; // in.w (vec4) assumed to be 1
        out->y = (m[1] * in.x) + (m[5] * in.y) + (m[ 9] * in.z) + m[13];
        out->z = (m[2] * in.x) + (m[6] * in.y) + (m[10] * in.z) + m[14];
        wCoords[i] =  (m[3] * in.x) + (m[7] * in.y) + (m[11] * in.z) + m[15]; // rw

        // bail if any W ended up as zero.
        const float epsilon = 1e-9f;
        if (absf(wCoords[i]) < epsilon) {
            return;
        }
    }

    // Divide by the W component of each:
    for( int i = 0; i < 8; ++i ) {
        points[i].x /= wCoords[i];
        points[i].y /= wCoords[i];
        points[i].z /= wCoords[i];
    }

    // Connect the dots:
    ddraw_bounds(points);
}

void ddraw_camera(camera_t *cam) {
    vec3 center = cam->position;
    vec3 rightdir = cross3(cam->lookdir,cam->updir);
    float proj[16]; // reproject perspective matrix with a smaller view distance (100 units)
    perspective44(proj, cam->fov, window_width() / ((float)window_height()+!window_height()), 0.01f, 100.f);

    ddraw_color_push(YELLOW);
    // frustum
    mat44 projview; multiply44x2(projview, /*cam->*/proj, cam->view);
    ddraw_frustum(projview);
    // top circles
    ddraw_circle(add3(center,add3(cam->lookdir,cam->updir)), rightdir, 2);
    ddraw_circle(add3(center,add3(neg3(cam->lookdir),cam->updir)), rightdir, 2);
    // orientation
    ddraw_color(RED);
    ddraw_arrow(cam->position, add3(cam->position,cam->lookdir));
    ddraw_color(GREEN);
    ddraw_arrow(cam->position, add3(cam->position,cam->updir));
    ddraw_color(BLUE);
    ddraw_arrow(cam->position, add3(cam->position,rightdir));
    ddraw_color_pop();
}

void ddraw_arrow(vec3 begin, vec3 end) {
    vec3 diff = sub3(end, begin);
    float len = len3(diff), stick_len = len * 2 / 3;

    ddraw_line(begin, end);
    ddraw_cone_lowres(add3(begin, scale3(norm3(diff), stick_len)), end, len / 6);
}

void ddraw_plane(vec3 p, vec3 n, float scale) {
    // if n is too similar to up vector, use right. else use up vector
    vec3 v1 = cross3(n, dot3(n, vec3(0,1,0)) > 0.8f ? vec3(1,0,0) : vec3(0,1,0));
    vec3 v2 = cross3(n, v1);

    // draw axis
    ddraw_line(p, add3(p,n));
    ddraw_line(p, add3(p,v1));
    ddraw_line(p, add3(p,v2));

    // get plane coords
    v1 = scale3(v1, scale);
    v2 = scale3(v2, scale);
    vec3 p1 = add3(add3(p, v1), v2);
    vec3 p2 = add3(sub3(p, v1), v2);
    vec3 p3 = sub3(sub3(p, v1), v2);
    vec3 p4 = sub3(add3(p, v1), v2);

    // draw plane
    ddraw_line(p1, p2);
    ddraw_line(p2, p3);
    ddraw_line(p3, p4);
    ddraw_line(p4, p1);
}

void ddraw_boid(vec3 position, vec3 dir) {
    dir = norm3(dir);

    // if n is too similar to up vector, use right. else use up vector
    vec3 v1 = cross3(dir, dot3(dir, vec3(0,1,0)) > 0.8f ? vec3(1,0,0) : vec3(0,1,0));
    vec3 v2 = cross3(dir, v1);
    v1 = cross3(dir, v2);

    uint32_t bak = dd_color;
    ddraw_color( position.y < 0 ? ORANGE : CYAN );

        vec3 front = add3(position, scale3(dir, 1));
        vec3 back = add3(position, scale3(dir, -0.25f));
        vec3 right = add3(back, scale3(v1, 0.5f));
        vec3 left = add3(back, scale3(v1, -0.5f));
        ddraw_line( front, left );
        ddraw_line( left, position );
        ddraw_line( position, right );
        ddraw_line( right, front );

    dd_color = bak;
}

void ddraw_circle__with_orientation(vec3 center, vec3 dir, float radius) {
    // we'll skip 3 segments out of 32. 1.5 per half circle.
    int segments = 32, skip = 3, drawn_segments = segments-skip;

//  dir = norm3(dir);
    vec3 right = cross3(dir, vec3(0,1,0));
    vec3 up = cross3(dir, right);
    right = cross3(dir, up);

    vec3 point, lastPoint;
    dir = scale3(dir, radius);
    right = scale3(right, radius);
    //lastPoint = add3(center, dir);

    {
        const float radians = (C_PI * 2) * (0+skip/2.f) / segments;
        vec3 vs = scale3(right, sinf(radians));
        vec3 vc = scale3(dir,   cosf(radians));
        lastPoint = add3(center, vs);
        lastPoint = add3(lastPoint, vc);
    }

    //ddraw_color(GREEN);
    ddraw_line(lastPoint, add3(center, scale3(dir, radius * (radius <= 1 ? 1.25 : 0.65) )));
    //ddraw_color(YELLOW);

    for (int i = 0; i <= drawn_segments; ++i) {
        const float radians = (C_PI * 2) * (i+skip/2.f) / segments;

        vec3 vs = scale3(right, sinf(radians));
        vec3 vc = scale3(dir,   cosf(radians));

        point = add3(center, vs);
        point = add3(point,  vc);

        ddraw_line(lastPoint, point);
        lastPoint = point;
    }

    //ddraw_color(RED);
    ddraw_line(lastPoint, add3(center, scale3(dir, radius * (radius <= 1 ? 1.25 : 0.65) )));
}

void ddraw_position_dir( vec3 position, vec3 direction, float radius ) {
    // idea from http://www.cs.caltech.edu/~keenan/m3drv.pdf and flotilla game UI
    uint32_t bak = dd_color;

    vec3 ground = vec3(position.x, 0, position.z);
    ddraw_color( position.y < 0 ? PINK/*ORANGE*/ : CYAN );
    ddraw_point( ground );
    ddraw_point( position );
    (position.y < 0 ? ddraw_line_dashed : ddraw_line)( ground, position );

    vec3 n = norm3(direction), up = vec3(0,1,0);
    for( int i = 0; i < 10 && i <= fabs(position.y); ++i ) {
        if( i < 2 && len3(direction) )
        ddraw_circle__with_orientation(ground, n, radius);
        else
        ddraw_circle(ground, up, radius);
        radius *= 0.9f;
    }

    dd_color = bak;
}

void ddraw_position( vec3 position, float radius ) {
    ddraw_position_dir(position, vec3(0,0,0), radius);
}

void ddraw_init() {
    do_once {
    for( int i = 0; i < 2; ++i )
    for( int j = 0; j < 3; ++j ) map_init(dd_lists[i][j], less_int, hash_int);
    dd_program = shader(dd_vs,dd_fs,"att_position","fragcolor", NULL);
    dd_u_color = glGetUniformLocation(dd_program, "u_color");
    ddraw_flush(); // alloc vao & vbo, also resets color
    }
}

void ddraw_demo() {
    ddraw_color_push(YELLOW);

    // freeze current frame for (frustum) camera forensics
    static mat44 projview_copy;
    do_once {
        multiply44x2(projview_copy, camera_get_active()->proj, camera_get_active()->view);
    }
    ddraw_frustum(projview_copy);

    //ddraw_grid();

    vec3 origin = {0,0,0};
    ddraw_color(ORANGE);
    ddraw_arrow(origin, vec3(-1,1,1));
    ddraw_color(YELLOW);
    ddraw_text(vec3(-1,1,1), 0.008f, va("hello 1%s2!", "world"));

    const char abc[] = " !\"#$%&'()*+,-./\n"
        "0123456789:;<=>?@\n"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`\n"
        "abcdefghijklmnopqrstuvwxyz{|}~";
    ddraw_text(vec3(2,2,2), 0.008f, abc);


    for( int i = -5; i <= 5; ++i ) {
        ddraw_pyramid(vec3(i*2,0,3),  0, i+5+2); ddraw_text(vec3(i*2,0,3), 0.008f, va("%d/1", i));
        ddraw_pyramid(vec3(i*2,0,6), -2, i+5+2); ddraw_text(vec3(i*2,0,6), 0.008f, va("%d/2", i));
        ddraw_pyramid(vec3(i*2,0,9), +2, i+5+2); ddraw_text(vec3(i*2,0,9), 0.008f, va("%d/3", i));
    }

#if 1 // @fixme: add positions to these
    // ddraw_triangle(origin, 1);
    ddraw_square(origin, 1);
    ddraw_pentagon(origin, 1);
    ddraw_hexagon(origin, 1);
    ddraw_cube(origin, 1);
    ddraw_pyramid(origin, 2, 3);
    ddraw_pyramid(origin, 2, 16);
    ddraw_cone(origin, add3(origin, vec3(0,1,0)), 0.5f);
    ddraw_arrow(origin, vec3(0,1,0));
    ddraw_bone(vec3(0,0,0), vec3(3,3,3));
    ddraw_aabb(vec3(0,0,0), vec3(1,1,1));
#endif

    ddraw_plane(vec3(0,10,0), vec3(0,1,0), 10);
    //ddraw_boid(vec3(15,0,15), vec3(-15,0,-15) );
    ddraw_position(vec3(10,10,10), 1);
    ddraw_position(vec3(-10,-10,10), 1);

    ddraw_point(vec3(-2,0,-2));
    ddraw_color(PURPLE);
    ddraw_sphere(vec3(-3,0,-3),1);

    ddraw_color_pop();
}
#line 0

#line 1 "fwk_scene.c"
//
// @todo: remove explicit GL code from here

static camera_t *last_camera;

camera_t camera() {
    camera_t *old = last_camera;

    static camera_t cam = {0};
    do_once {
        cam.speed = 0.50f;
        cam.position = vec3(10,10,10);
        cam.updir = vec3(0,1,0);
        cam.fov = 45;

        cam.damping = false;
        cam.move_friction = 0.09f;
        cam.move_damping = 0.96f;
        cam.look_friction = 0.30f;
        cam.look_damping = 0.96f;
        cam.last_look = vec2(0,0);
        cam.last_move = vec3(0,0,0);

        // update proj & view
        camera_lookat(&cam,vec3(-5,0,-5));

        // @todo: remove this hack that is used to consolidate dampings
        if( 1 ) {
            vec3 zero = {0};
            for( int i = 0; i < 1000; ++i ) {
                camera_moveby(&cam, zero);
                camera_fps(&cam,0,0);
            }
        }
    }

    last_camera = old;
    *camera_get_active() = cam;
    return cam;
}

camera_t *camera_get_active() {
    static camera_t defaults = {0};
    if( !last_camera ) {
        identity44(defaults.view);
        identity44(defaults.proj);
        last_camera = &defaults;
    }
    return last_camera;
}

void camera_moveby(camera_t *cam, vec3 inc) {
    // calculate camera damping
    if( cam->damping ) {
        float fr = cam->move_friction; fr *= fr; fr *= fr; fr *= fr;
        float sm = clampf(cam->move_damping, 0, 0.999f); sm *= sm; sm *= sm;

        cam->last_move = scale3(cam->last_move, 1 - fr);
        inc.x = cam->last_move.x = inc.x * (1 - sm) + cam->last_move.x * sm;
        inc.y = cam->last_move.y = inc.y * (1 - sm) + cam->last_move.y * sm;
        inc.z = cam->last_move.z = inc.z * (1 - sm) + cam->last_move.z * sm;
    }

    vec3 dir = norm3(cross3(cam->lookdir, cam->updir));
    cam->position = add3(cam->position, scale3(dir, inc.x)); // right
    cam->position = add3(cam->position, scale3(cam->updir, inc.y)); // up
    cam->position = add3(cam->position, scale3(cam->lookdir, inc.z)); // front

    camera_fps(cam, 0, 0);
}

void camera_teleport(camera_t *cam, vec3 pos) {
    bool damping = cam->damping;
    cam->damping = 0;
    cam->last_move = vec3(0,0,0);
    cam->position = pos;
    camera_fps(cam, 0, 0);
    cam->damping = damping;
}

void camera_lookat(camera_t *cam, vec3 target) {
    // invert expression that cam->lookdir = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
    // look.y = sin p > y = asin(p)
    // look.x = cos y * cos p; -> cos p = look.x / cos y \ look.x / cos y = look.z / sin y
    // look.z = sin y * cos p; -> cos p = look.z / sin y /
    // so, sin y / cos y = look x / look z > tan y = look x / look z > y = atan(look x / look z)

    vec3 look = norm3(sub3(target, cam->position));
    const float rad2deg = 1 / 0.0174532f;
    float pitch = asin(look.y) * rad2deg;
    float yaw = atan2(look.z, look.x) * rad2deg; // coords swapped. it was (look.x, look.z) before. @todo: testme

    camera_fps(cam, yaw-cam->yaw, pitch-cam->pitch);
}

void camera_enable(camera_t *cam) {
    // camera_t *other = camera_get_active(); // init default camera in case there is none
    last_camera = cam;
    // trigger a dummy update -> update matrices
    camera_fps(cam, 0, 0);
}

void camera_fov(camera_t *cam, float fov) {
    last_camera = cam;

#if 0 // isometric/dimetric
    #define orthogonal(proj, fov, aspect, znear, zfar) \
    ortho44((proj), -(fov) * (aspect), (fov) * (aspect), -(fov), (fov), (znear), (zfar))

    float DIMETRIC = 30.000f;
    float ISOMETRIC = 35.264f;
    float aspect = window_width() / ((float)window_height()+!!window_height());
    orthogonal(cam->proj, 45, aspect, -1000, 1000); // why -1000?
    // cam->yaw = 45;
    cam->pitch = -ISOMETRIC;
#endif
    cam->fov = fov;
    perspective44(cam->proj, cam->fov, window_width() / ((float)window_height()+!window_height()), 0.01f, 1000.f);
}

void camera_fps(camera_t *cam, float yaw, float pitch) {
    last_camera = cam;

    // camera damping
    if( cam->damping ) {
        float fr = cam->look_friction; fr *= fr; fr *= fr; fr *= fr;
        float sm = clampf(cam->look_damping, 0, 0.999f); sm *= sm; sm *= sm;

        cam->last_look = scale2(cam->last_look, 1 - fr);
        yaw = cam->last_look.y = yaw * (1 - sm) + cam->last_look.y * sm;
        pitch = cam->last_look.x = pitch * (1 - sm) + cam->last_look.x * sm;
    }

    cam->yaw += yaw;
    cam->yaw = fmod(cam->yaw, 360);
    cam->pitch += pitch;
    cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;

    const float deg2rad = 0.0174532f, y = cam->yaw * deg2rad, p = cam->pitch * deg2rad;
    cam->lookdir = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
    lookat44(cam->view, cam->position, add3(cam->position, cam->lookdir), cam->updir); // eye,center,up

    camera_fov(cam, cam->fov);
}

void camera_orbit( camera_t *cam, float yaw, float pitch, float inc_distance ) {
    last_camera = cam;

    vec2 inc_mouse = vec2(yaw, pitch);

    // @todo: worth moving all these members into camera_t ?
    static vec2 _mouse = {0,0};
    static vec2 _polarity = { +1,-1 };
    static vec2 _sensitivity = { 2,2 };
    static float _friction = 0.75; //99;
    static float _distance; do_once _distance = len3(cam->position);

    // update dummy state
    camera_fps(cam, 0,0);

    // add smooth input
    _mouse = mix2(_mouse, add2(_mouse, mul2(mul2(inc_mouse,_sensitivity),_polarity)), _friction);
    _distance = mixf(_distance, _distance+inc_distance, _friction);

    // look: update angles
    vec2 offset = sub2( _mouse, ptr2(&cam->last_move.x) );
    if( 1 ) { // if _enabled
        cam->yaw += offset.x;
        cam->pitch += offset.y;
        // look: limit pitch angle [-89..89]
        cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;
    }

    // compute view matrix
    float x = rad(cam->yaw), y = rad(cam->pitch), cx = cosf(x), cy = cosf(y), sx = sinf(x), sy = sinf(y);
    lookat44(cam->view, vec3( cx*cy*_distance, sy*_distance, sx*cy*_distance ), vec3(0,0,0), vec3(0,1,0) );

    // save for next call
    cam->last_move.x = _mouse.x;
    cam->last_move.y = _mouse.y;
}

int ui_camera( camera_t *cam ) {
    int changed = 0;
    changed |= ui_float("Speed", &cam->speed);
    ui_separator();
    changed |= ui_bool("Damping", &cam->damping);
    if( !cam->damping ) ui_disable();
    changed |= ui_slider2("Move friction", &cam->move_friction, va("%5.2f", cam->move_friction));
    changed |= ui_slider2("Move damping", &cam->move_damping, va("%5.2f", cam->move_damping));
    changed |= ui_slider2("View driction", &cam->look_friction, va("%5.2f", cam->look_friction));
    changed |= ui_slider2("View damping", &cam->look_damping, va("%5.2f", cam->look_damping));
    if( !cam->damping ) ui_enable();
    ui_separator();
    changed |= ui_float3("Position", &cam->position.x);
    changed |= ui_float3("LookDir", &cam->lookdir.x);
    changed |= ui_float3("UpDir", &cam->updir.x);
    changed |= ui_mat44("View matrix", cam->view);
    ui_separator();
    changed |= ui_float("FOV (degrees)", &cam->fov);
    changed |= ui_mat44("Projection matrix", cam->proj);
    return changed;
}

// -----------------------------------------------------------------------------

static
void object_update(object_t *obj) {
    quat p = eulerq(vec3(obj->pivot.x,obj->pivot.y,obj->pivot.z));
    quat e = eulerq(vec3(obj->euler.x,obj->euler.y,obj->euler.z));
    compose44(obj->transform, obj->pos, mulq(e, p), obj->sca);
}

object_t object() {
    object_t obj = {0};
    identity44(obj.transform);
    //obj.rot = idq();
    obj.sca = vec3(1,1,1);
    //obj.bounds = aabb(vec3(0,0,0),vec3(1,1,1)); // defaults to small 1-unit cube
object_rotate(&obj, vec3(0,0,0));
    //array_init(obj.textures);
    return obj;
}

void object_pivot(object_t *obj, vec3 euler) {
    obj->pivot = euler;
    object_update(obj);
}

void object_rotate(object_t *obj, vec3 euler) {
    quat p = eulerq(vec3(obj->pivot.x,obj->pivot.y,obj->pivot.z));
    quat e = eulerq(vec3(euler.x,euler.y,euler.z));
    obj->rot = mulq(p,e);
    obj->euler = euler;
    object_update(obj);
}

void object_teleport(object_t *obj, vec3 pos) {
    obj->pos = pos;
    object_update(obj);
}

void object_move(object_t *obj, vec3 inc) {
    obj->pos = add3(obj->pos, inc);
    object_update(obj);
}

void object_scale(object_t *obj, vec3 sca) {
    obj->sca = vec3(sca.x, sca.y, sca.z);
    object_update(obj);
}

vec3 object_position(object_t *obj) {
    return vec3(obj->transform[12], obj->transform[13], obj->transform[14]);
}

void object_model(object_t *obj, model_t model) {
    obj->model = model;
}

void object_push_diffuse(object_t *obj, texture_t tex) {
    array_push(obj->textures, tex.id);
}

void object_pop_diffuse(object_t *obj) {
    array_pop(obj->textures);
}

void object_diffuse(object_t *obj, texture_t tex) {
    array_clear(obj->textures);
    object_push_diffuse(obj, tex);
}

void object_billboard(object_t *obj, unsigned mode) {
    obj->billboard = mode;
}

// -----------------------------------------------------------------------------

light_t light() {
    light_t l = {0};
    l.diffuse = vec3(1,1,1);
    l.dir = vec3(1,-1,-1);
    l.falloff.constant = 1.0f;
    l.falloff.linear = 0.09f;
    l.falloff.quadratic = 0.0032f;
    l.specularPower = 32.f;
    l.innerCone = 0.9f; // 25 deg
    l.outerCone = 0.85f; // 31 deg

    return l;
}

void light_type(light_t* l, char type) {
    l->cached = 0;
    l->type = type;
}

void light_diffuse(light_t* l, vec3 color) {
    l->cached = 0;
    l->diffuse = color;
}

void light_specular(light_t* l, vec3 color) {
    l->cached = 0;
    l->specular = color;
}

void light_ambient(light_t* l, vec3 color) {
    l->cached = 0;
    l->ambient = color;
}

void light_teleport(light_t* l, vec3 pos) {
    l->cached = 0;
    l->pos = pos;
}

void light_dir(light_t* l, vec3 dir) {
    l->cached = 0;
    l->dir = dir;
}

void light_power(light_t* l, float power) {
    l->cached = 0;
    l->specularPower = power;
}

void light_falloff(light_t* l, float constant, float linear, float quadratic) {
    l->cached = 0;
    l->falloff.constant = constant;
    l->falloff.linear = linear;
    l->falloff.quadratic = quadratic;
}

void light_cone(light_t* l, float innerCone, float outerCone) {
    l->cached = 0;
    l->innerCone = acos(innerCone);
    l->outerCone = acos(outerCone);
}

void light_update(unsigned num_lights, light_t *lv) {
    shader_int("u_num_lights", num_lights);

    for (unsigned i=0; i < num_lights; ++i) {
        lv[i].cached = 1;
        shader_int(va("u_lights[%d].type", i), lv[i].type);
        shader_vec3(va("u_lights[%d].pos", i), lv[i].pos);
        shader_vec3(va("u_lights[%d].dir", i), lv[i].dir);
        shader_vec3(va("u_lights[%d].diffuse", i), lv[i].diffuse);
        shader_vec3(va("u_lights[%d].specular", i), lv[i].specular);
        shader_vec3(va("u_lights[%d].ambient", i), lv[i].ambient);
        shader_float(va("u_lights[%d].power", i), lv[i].specularPower);
        shader_float(va("u_lights[%d].constant", i), lv[i].falloff.constant);
        shader_float(va("u_lights[%d].linear", i), lv[i].falloff.linear);
        shader_float(va("u_lights[%d].quadratic", i), lv[i].falloff.quadratic);
        shader_float(va("u_lights[%d].innerCone", i), lv[i].innerCone);
        shader_float(va("u_lights[%d].outerCone", i), lv[i].outerCone);
    }
}

// -----------------------------------------------------------------------------

array(scene_t*) scenes;
scene_t* last_scene;

static void scene_init() {
#ifndef __EMSCRIPTEN__ // @fixme ems -> shaders
    do_once scene_push();      // create an empty scene by default
#endif
}

scene_t* scene_get_active() {
    return last_scene;
}

scene_t* scene_push() {
    scene_t *s = REALLOC(0, sizeof(scene_t)), clear = {0}; *s = clear;
    s->skybox = skybox(NULL, 0);
    array_push(scenes, s);
    last_scene = s;
    return s;
}

void scene_pop() {
    // @fixme: fix leaks, scene_cleanup();
    scene_t clear = {0};
    *last_scene = clear;
    array_pop(scenes);
    last_scene = *array_back(scenes);
}

int scene_merge(const char *source) {
    int count = 0;
    if( json_push(source) ) {
        for(int i = 0, e = json_count("/") - 1; i <= e ; ++i) {
            const char *skybox_folder = json_string("/[%d]/skybox",i);
            if( skybox_folder[0] ) {
                PRINTF("Loading skybox folder: %s\n", skybox_folder);
                last_scene->skybox = skybox( skybox_folder, 0 );
                continue;
            }
            const char *mesh_file = json_string("/[%d]/mesh",i);
            const char *texture_file = json_string("/[%d]/texture",i);
            const char *animation_file = json_string("/[%d]/animation",i);
            vec3 position = vec3(json_float("/[%d]/position[0]",i),json_float("/[%d]/position[1]",i),json_float("/[%d]/position[2]",i));
            vec3 rotation = vec3(json_float("/[%d]/rotation[0]",i),json_float("/[%d]/rotation[1]",i),json_float("/[%d]/rotation[2]",i));
            vec3 scale = scale3(vec3(1,1,1), json_float("/[%d]/scale",i));
            bool opt_swap_zy = json_int("/[%d]/swapzy",i);
            bool opt_flip_uv = json_int("/[%d]/flipuv",i);
            PRINTF("Scene %d/%d Loading: %s\n", i, e, mesh_file);
            PRINTF("Scene %d/%d Texture: %s\n", i, e, texture_file);
            PRINTF("Scene %d/%d Animation: %s\n", i, e, animation_file);
            PRINTF("Scene %d/%d Position: (%f,%f,%f)\n", i, e, position.x, position.y, position.z);
            PRINTF("Scene %d/%d Rotation: (%f,%f,%f)\n", i, e, rotation.x, rotation.y, rotation.z);
            PRINTF("Scene %d/%d Scale: (%f,%f,%f)\n", i, e, scale.x, scale.y, scale.z);
            PRINTF("Scene %d/%d Swap_ZY: %d\n", i, e, opt_swap_zy );
            PRINTF("Scene %d/%d Flip_UV: %d\n", i, e, opt_flip_uv );
            model_t m = model_from_mem(vfs_read(mesh_file), vfs_size(mesh_file), 0/*opt_swap_zy*/);
            //char *a = archive_read(animation_file);
            object_t *o = scene_spawn();
            object_model(o, m);
            if( texture_file[0] ) object_diffuse(o, texture_from_mem(vfs_read(texture_file), vfs_size(texture_file), opt_flip_uv ? IMAGE_FLIP : 0) );
            object_scale(o, scale);
            object_teleport(o, position);
            object_pivot(o, rotation); // object_rotate(o, rotation);
            //object_name(x), scene_find(name)
// o->bounds = aabb(mul3(m.bounds.min,o->sca),mul3(m.bounds.max,o->sca));
// PRINTF("aabb={%f,%f,%f},{%f,%f,%f}\n", o->bounds.min.x, o->bounds.min.y, o->bounds.min.z, o->bounds.max.x, o->bounds.max.y, o->bounds.max.z);

/*
            if(opt_swap_zy) {
                // swap zy bounds
                vec3 min = o->bounds.min, max = o->bounds.max;
                o->bounds = aabb( vec3(min.x,min.z,min.y), vec3(max.x,max.z,max.y) );
            }
*/

            count++;
        }
        json_pop();
    }
    // PRINTF("scene loading took %5.2fs\n", secs);
    PRINTF("%d objects merged into scene\n", count);
    return count;
}

object_t* scene_spawn() {
    object_t obj = object();
    array_push(last_scene->objs, obj);

    return array_back(last_scene->objs);
}

unsigned scene_count() {
    return array_count(last_scene->objs);
}

object_t* scene_index(unsigned obj_index) {
    unsigned obj_count = scene_count();
    ASSERT(obj_index < obj_count, "Object index %d exceeds number (%d) of spawned objects", obj_index, obj_count);
    return &last_scene->objs[obj_index];
}

light_t* scene_spawn_light() {
    light_t l = light();
    array_push(last_scene->lights, l);

    return array_back(last_scene->lights);
}

unsigned scene_count_light() {
    return array_count(last_scene->lights);
}

light_t* scene_index_light(unsigned light_index) {
    unsigned light_count = scene_count_light();
    ASSERT(light_index < light_count, "Light index %d exceeds number (%d) of spawned lights", light_index, light_count);
    return &last_scene->lights[light_index];
}


void scene_render(int flags) {
    camera_t *cam = camera_get_active();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glActiveTexture(GL_TEXTURE0);

    if(flags & SCENE_BACKGROUND) {
        if(last_scene->skybox.program) {
        skybox_push_state(&last_scene->skybox, cam->proj, cam->view);

        glDisable(GL_DEPTH_TEST);
    //  glDepthFunc(GL_LESS);
    //    glActiveTexture(GL_TEXTURE0);
    //    (flags & SCENE_CULLFACE ? glEnable : glDisable)(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
    //    glPolygonMode( GL_FRONT_AND_BACK, flags & SCENE_WIREFRAME ? GL_LINE : GL_FILL );

        mesh_render(&last_scene->skybox.geometry);
        skybox_pop_state();
        }

        ddraw_flush();
    }

    glDepthFunc(GL_LESS);
    glActiveTexture(GL_TEXTURE0);

    // @fixme: CW ok for one-sided rendering. CCW ok for FXs. we need both
    (flags & SCENE_CULLFACE ? glEnable : glDisable)(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
    glPolygonMode( GL_FRONT_AND_BACK, flags & SCENE_WIREFRAME ? GL_LINE : GL_FILL );
    // @todo alpha mode
    // @todo texture mode

    if( flags & SCENE_FOREGROUND ) {
        bool do_relighting = 0;
        for (unsigned j = 0; j < array_count(last_scene->lights); ++j) {
            if (!last_scene->lights[j].cached) {
                do_relighting = 1;
                break;
            }
        }

        for(unsigned j = 0, obj_count = scene_count(); j < obj_count; ++j ) {
            object_t *obj = scene_index(j);
            model_t *model = &obj->model;
            mat44 *views = (mat44*)(&cam->view);

            // @todo: avoid heap allocs here?
            static array(handle) old_textures = 0;

            int do_retexturing = model->iqm && array_count(obj->textures) > 0;
            if( do_retexturing ) {
                for(int i = 0; i < model->iqm->nummeshes; ++i) {
                    array_push(old_textures, model->iqm->textures[i]);
                    model->iqm->textures[i] = *array_back(obj->textures);
                }
            }

            if ( do_relighting || !obj->light_cached ) {
                obj->light_cached = 1;
                shader_bind(model->program);
                light_update(array_count(last_scene->lights), last_scene->lights);
            }

            if ( flags&SCENE_UPDATE_SH_COEF ) {
                shader_bind(model->program);
                shader_vec3v("u_coefficients_sh", 9, last_scene->skybox.cubemap.sh);
            }

            model->billboard = obj->billboard;
            model_render(*model, cam->proj, cam->view, obj->transform, 0);

            if( do_retexturing ) {
                for(int i = 0; i < model->iqm->nummeshes; ++i) {
                    model->iqm->textures[i] = old_textures[i];
                }
                array_resize(old_textures, 0);
            }
        }
        glBindVertexArray(0);
    }

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}
#line 0

#line 1 "fwk_script.c"
typedef lua_State lua;

// the Lua interpreter
static lua *L;

#if is(linux)
void luaopen_libfwk(lua_State *L) {}
#endif

static void* script__realloc(void *userdef, void *ptr, size_t osize, size_t nsize) {
    (void)userdef;
    return ptr = REALLOC( ptr, /* (osize+1) * */ nsize );
}
static int script__traceback(lua_State *L) {
    if (!lua_isstring(L, 1)) { // try metamethod if non-string error object
        if (lua_isnoneornil(L, 1) ||
            !luaL_callmeta(L, 1, "__tostring") ||
            !lua_isstring(L, -1))
            return 1;  // return non-string error object
        lua_remove(L, 1);  // replace object with result of __tostring metamethod
    }
    luaL_traceback(L, L, lua_tostring(L, 1), 1);
    return 1;
}
static void script__error(lua_State *L, int status) {
    if (status != 0) {
        const char *errormsg = lua_tostring(L, -1);
        PRINTF( "!-- %s\n", errormsg);
        lua_pop(L, 1); // remove error message
    }
}
static int script__call(lua_State *L, int narg, int clear) {
#if ENABLE_FASTCALL_LUA
    lua_call(L, 0, 0);
    return 0;
#else
    int base = lua_gettop(L) - narg;  // function index
    lua_pushcfunction(L, script__traceback);  // push traceback function
    lua_insert(L, base);  // put it under chunk and args
    int status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
    script__error(L, status);
    lua_remove(L, base);  // remove traceback function
    if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0); // force gc in case of errors
    return status;
#endif
}

void script_bind_function(const char *c_name, void *c_function) {
    lua_pushcfunction( L, c_function );
    lua_setglobal( L, c_name );
}

void script_call(const char *lua_function) {
    lua_getglobal( L, lua_function );
    script__call( L, 0, 1 );
}

void script_bind_class(const char *classname, int num, const char **methods, void **functions) {
    lua_newtable( L );

    for( int i = 0; i < num; ++i) {
        lua_pushcfunction( L, functions[i] );
        lua_setfield( L, 1, methods[i] );
    }

    lua_setglobal( L, classname );
}

void script_run(const char *script) {
    int ret = luaL_dostring(L, script);
    if( ret != LUA_OK ) {
      PRINTF("!Script failed to run: %s\n", lua_tostring(L, -1));
      lua_pop(L, 1); // pop error message
    }
}

void script_runfile(const char *pathfile) {
    PRINTF( "Loading script '%s'\n", pathfile );
    int loadResult = luaL_loadfile( L, pathfile );

    /**/ if( loadResult == LUA_OK ) {
        script__call( L, 0, 1 );
    }
    else if( loadResult == LUA_ERRSYNTAX ) {
        PRINTF("!Script failed to load (LUA_ERRSYNTAX, '%s'): %s\n", lua_tostring( L, 1 ), pathfile );
//      lua_pop(L, 1); // pop error message
    }
    else if( loadResult == LUA_ERRMEM ) {
        PRINTF("!Script failed to load (LUA_ERRMEM): %s\n", pathfile);
    }
    else {
        PRINTF("!Script failed to load: %s\n", pathfile );
    }
}

// syntax sugars
/* usage:
int window_create_lua(lua *L) {
    window_create(arg_float(1), arg_int(2));
    return_void(0);
}
int window_swap_lua(lua *L) {
    int r = window_swap();
    return_int(r);
}
*/

#define arg_int(nth) lua_tointeger(L, nth)
#define arg_bool(nth) lua_toboolean(L, nth)
#define arg__Bool(nth) lua_toboolean(L, nth)
#define arg_float(nth) (float)lua_tonumber(L, nth)
#define arg_double(nth) lua_tonumber(L, nth)
#define arg_string(nth) lua_tolstring(L, nth, 0)
#define return_void(x) return ((x), 0)
#define return_bool(x) return (lua_pushboolean(L, x), 1)
#define return__Bool(x) return (lua_pushboolean(L, x), 1)
#define return_int(x) return (lua_pushinteger(L, x), 1)
#define return_float(x) return (lua_pushnumber(L, x), 1)
#define return_double(x) return (lua_pushnumber(L, x), 1)
#define return_string(x) return (lua_pushstring(L, x), 1)

#define WRAP_ALL(...)                EXPAND(WRAP_ALL, __VA_ARGS__)
#define WRAP_ALL2(rc, func)          int func##_lua(lua*L) { return_##rc(func()); }
#define WRAP_ALL3(rc, func, a1)      int func##_lua(lua*L) { return_##rc(func(arg_##a1(1))); }
#define WRAP_ALL4(rc, func, a1,a2)   int func##_lua(lua*L) { return_##rc(func(arg_##a1(1),arg_##a2(2))); }

#define BIND_ALL(...)                EXPAND(BIND_ALL, __VA_ARGS__);
#define BIND_ALL2(rc,func)           script_bind_function(#func, func##_lua)
#define BIND_ALL3(rc,func,a1)        script_bind_function(#func, func##_lua)
#define BIND_ALL4(rc,func,a1,a2)     script_bind_function(#func, func##_lua)

#define XMACRO(X) /* @fixme: add all remaining FWK functions */ \
    X(bool, window_create, float, int ) \
    X(bool, window_swap ) \
    X(void, ddraw_grid, float ) \
    X(bool, ui_panel, string, int ) \
    X(bool, ui_notify, string, string ) \
    X(void, ui_panel_end )

XMACRO(WRAP_ALL)

void script_quit(void) {
    if( L ) {
        lua_close(L);
        L = 0;
    }
}
void script_init() {
    if( !L ) {
        // fwk_init();

        // initialize Lua
        L = lua_newstate(script__realloc, 0); // L = luaL_newstate();

        // load various Lua libraries
        luaL_openlibs(L);
        luaopen_base(L);
        luaopen_table(L);
        luaopen_io(L);
        luaopen_string(L);
        luaopen_math(L);

        // @fixme: workaround that prevents script binding on lua 5.4.3 on top of luajit 2.1.0-beta3 on linux. lua_setglobal() crashing when accessing null L->l_G
        if(L->l_G) {
        XMACRO(BIND_ALL);
        }

        atexit(script_quit);
    }
}

bool script_tests() {
    // script test (lua)
    script_run( "-- Bye.lua\nio.write(\"script test: Bye world!, from \", _VERSION, \"\\n\")" );    
    return true;
}

#undef XMACRO
#line 0

#line 1 "fwk_system.c"
#if (is(tcc) && is(linux)) || (is(gcc) && !is(mingw)) // || is(clang)
int __argc; char **__argv;
#if !is(ems)
__attribute__((constructor)) void init_argcv(int argc, char **argv) { __argc = argc; __argv = argv; }
#endif
#endif

const char *app_path() { // @fixme: should return absolute path always. see tcc -g -run
    static char buffer[1024] = {0};
    if( buffer[0] ) return buffer;
#if is(win32)
    unsigned length = GetModuleFileNameA(NULL, buffer, sizeof(buffer)); // @todo: use GetModuleFileNameW+wchar_t && convert to utf8 instead
    char *a = strrchr(buffer, '/');  if(!a) a = buffer + strlen(buffer);
    char *b = strrchr(buffer, '\\'); if(!b) b = buffer + strlen(buffer);
    char slash = (a < b ? *a : b < a ? *b : '/');
    snprintf(buffer, 1024, "%.*s%c", length - (int)(a < b ? b - a : a - b), buffer, slash);
    if( strendi(buffer, "tools\\tcc\\") ) { // fix tcc -g -run case. @fixme: use TOOLS instead
        strcat(buffer, "..\\..\\");
    }
#else // #elif is(linux)
    char path[21] = {0};
    sprintf(path, "/proc/%d/exe", getpid());
    readlink(path, buffer, sizeof(buffer));
#endif
    return buffer;
}

const char *app_temp() {
    static char buffer[256] = {0};
    if( !buffer[0] ) {
        snprintf(buffer, 256, "%s", ifdef(win32, getenv("TEMP"), P_tmpdir));
        for( int i = 0; buffer[i]; ++i ) if( buffer[i] == '\\' ) buffer[i] = '/';
        if(buffer[strlen(buffer)-1] != '/') strcat(buffer, "/");
    }
    return buffer;
}

/*
    bool exporting_dll = !strcmp(STRINGIZE(API), STRINGIZE(EXPORT));
    bool importing_dll = !strcmp(STRINGIZE(API), STRINGIZE(IMPORT));
    else static_build
*/

#ifndef APP_NAME
#define APP_NAME ifdef(ems, "", (__argv ? __argv[0] : ""))
#endif

const char *app_name() {
    static char buffer[256] = {0};
    if( !buffer[0] ) {
        char s[256];
        strncpy(s, APP_NAME, 256);
        char *a = strrchr(s, '/');
        char *b = strrchr(s, '\\');
        strncpy(buffer, a > b ? a+1 : b > a ? b+1 : s, 256);
        if(strendi(buffer, ".exe")) buffer[strlen(buffer) - 4] = 0;
    }
    return buffer;
}

const char *app_cmdline() {
    static char *cmdline = 0;
    if( !cmdline ) {
        if( argc() <= 1 ) strcatf(&cmdline, "%s", " ");
        for( int i = 1; i < argc(); ++i ) strcatf(&cmdline, " %s", argv(i));
    }
    return cmdline+1;
}

const char *app_cache() {
    static char buffer[256] = {0};
    if( !buffer[0] ) {

        #if is(osx)
        snprintf(buffer, 256, "~/Library/Caches/%s/", app_name()); // getenv("user.home")
        #elif is(win32) // APPDATA for roaming?
        snprintf(buffer, 256, "%s\\%s\\", getenv("LOCALAPPDATA"), app_name()); // getenv("LOCALAPPDATA")
        #else // linux
        snprintf(buffer, 256, "~/.cache/%s/", app_name()); // getenv("user.home")
        #endif

        mkdir(buffer, 0777);

        for( int i = 0; buffer[i]; ++i ) if( buffer[i] == '\\' ) buffer[i] = '/';
    }

    return buffer;
}

const char * app_exec( const char *cmd ) {
    static __thread char output[4096+16] = {0};

    if( !cmd[0] ) return "0               ";
    cmd = file_normalize(cmd);

    int rc = -1;
    char *buf = output + 16; buf[0] = 0; // memset(buf, 0, 4096);
    for( FILE *fp = popen( cmd, "r" ); fp; rc = pclose(fp), fp = 0) {
        while( fgets(buf, 4096 - 1, fp) ) {
        }
    }
    if( rc != 0 ) {
        char *r = strrchr(buf, '\r'); if(r) *r = 0;
        char *n = strrchr(buf, '\n'); if(n) *n = 0;
    }
    return snprintf(output, 16, "%-15d", rc), buf[-1] = ' ', output;
}

int app_spawn( const char *cmd ) {
    if( !cmd[0] ) return -1;
    cmd = file_normalize(cmd);

    return system(cmd);
}

#if is(osx)
#include <execinfo.h> // backtrace, backtrace_symbols
#include <dlfcn.h>    // dladdr, Dl_info
#elif is(gcc) && !is(ems) && !is(mingw) // maybe is(linux) is enough?
#include <execinfo.h>  // backtrace, backtrace_symbols
#elif is(win32) // && !defined __TINYC__
#include <winsock2.h>  // windows.h alternative
#include <dbghelp.h>
#pragma comment(lib, "DbgHelp")
#pragma comment(lib, "Kernel32")
static int backtrace( void **addr, int maxtraces ) {
    static bool init = 0;
    do_once SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES);
    do_once init = SymInitialize(GetCurrentProcess(), NULL, TRUE);
    if(!init) return 0; // error: cannot initialize DbgHelp.lib

    //typedef USHORT (WINAPI *pFN)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG); // _MSC_VER
    typedef USHORT (WINAPI *pFN)(); // TINYC
    static pFN rtlCaptureStackBackTrace = 0;
    if( !rtlCaptureStackBackTrace ) {
        rtlCaptureStackBackTrace = (pFN)GetProcAddress(LoadLibraryA("kernel32.dll"), "RtlCaptureStackBackTrace");
    }
    if( !rtlCaptureStackBackTrace ) {
        return 0;
    }
    return rtlCaptureStackBackTrace(1, maxtraces, (PVOID *)addr, (DWORD *) 0);
}
static char **backtrace_symbols(void *const *list,int size) {
    HANDLE process = GetCurrentProcess();

    struct symbol_t {
        SYMBOL_INFO info;
        TCHAR symbolname[256], terminator;
    } si = { {0} };
    si.info.SizeOfStruct = sizeof(SYMBOL_INFO);
    si.info.MaxNameLen = sizeof(si.symbolname) / sizeof(TCHAR); // number of chars, not bytes

    IMAGEHLP_LINE l64 = { 0 };
    l64.SizeOfStruct = sizeof(IMAGEHLP_LINE);

    static __thread char **symbols = 0; //[32][64] = {0};
    if( !symbols ) {
        symbols = SYS_REALLOC(0, 128 * sizeof(char*));
        for( int i = 0; i < 128; ++i) symbols[i] = SYS_REALLOC(0, 128 * sizeof(char));
    }

    if(size > 128) size = 128;
    for( int i = 0; i < size; ++i ) {

        char *ptr = symbols[i];
        *ptr = '\0';

        if (SymFromAddr(process, (DWORD64)(uintptr_t)list[i], 0, &si.info)) {
            //char undecorated[1024];
            //UnDecorateSymbolName(si.info.Name, undecorated, sizeof(undecorated)-1, UNDNAME_COMPLETE);
            char* undecorated = (char*)si.info.Name;
            ptr += snprintf(ptr, 128, "%s", undecorated);
        } else {
            ptr += snprintf(ptr, 128, "%s", "(?""?)");
        }

        DWORD dw = 0;
        if (SymGetLineFromAddr(process, (DWORD64)(uintptr_t)list[i], &dw, &l64)) {
            ptr += snprintf(ptr, 128 - (ptr - symbols[i]), " (%s:%u)", l64.FileName, (unsigned)l64.LineNumber);
        }
    }

    return symbols;
}
#else
static int backtrace(void **heap, int num) { return 0; }
static char **backtrace_symbols(void *const *sym,int num) { return 0; }
#endif

char *callstack( int traces ) {
    static __thread char *output = 0;
    if(!output ) output = SYS_REALLOC( 0, 128 * (64+2) );
    if( output ) output[0] = '\0';
    char *ptr = output;

    enum { skip = 1 }; /* exclude 1 trace from stack (this function) */
    enum { maxtraces = 128 };

    int inc = 1;
    if( traces < 0 ) traces = -traces, inc = -1;
    if( traces == 0 ) return "";
    if( traces > maxtraces ) traces = maxtraces;

    void* stacks[maxtraces/* + 1*/]; // = { 0 };
    traces = backtrace( stacks, traces );
    char **symbols = backtrace_symbols( stacks, traces ); // @todo: optimization: map(void*,char*) cache; and retrieve only symbols not in cache

    char demangled[1024] = "??";
    int L = 0, B = inc>0 ? skip - 1 : traces, E = inc>0 ? traces : skip - 1;
    for( int i = B; ( i += inc ) != E; ) {
#if is(linux)
        #if ENABLE_LINUX_CALLSTACKS
        // @fixme: following snippet works if compiled with '-g', albeit terribly slow
        // should concat addresses into a multi-address line

        char *binary = symbols[i];
        char *address = strchr( symbols[i], '(' ) + 1; 
        *strrchr( address, ')') = '\0'; *(address - 1) = '\0';

        for( FILE *fp = popen(va("addr2line -e %s %s", binary, address), "r" ); fp ; pclose(fp), fp = 0 ) { //addr2line -e binary -f -C address
            fgets(demangled, sizeof(demangled), fp);
            int len = strlen(demangled); while( len > 0 && demangled[len-1] < 32 ) demangled[--len] = 0;
        }
        symbols[i] = demangled;
        #else
        // make it shorter. ie, `0x00558997ccc87e ./a.out(+0x20187e) [0x00558997ccc87e]`
        strchr(symbols[i], ')')[1] = '\0';
        #endif
#elif is(osx)
        /*struct*/ Dl_info info;
        if( dladdr(stacks[i], &info) && info.dli_sname ) {
            const char *dmgbuf = info.dli_sname[0] != '_' ? NULL :
                 ifdef(cpp, __cxa_demangle(info.dli_sname, NULL, 0, NULL), info.dli_sname);
            strcpy( demangled, dmgbuf ? dmgbuf : info.dli_sname );
            symbols[i] = demangled;
            if( dmgbuf ) free( (void*)dmgbuf );
        }
#endif
        ptr += sprintf(ptr, "%03d: %#016llx %s\n", ++L, (unsigned long long)(uintptr_t)stacks[i], symbols[i]); // format gymnastics because %p is not standard when printing pointers
    }

#if is(linux) || is(osx)
     if(symbols) free(symbols);
#endif

     return output ? output : "";
}

int callstackf( FILE *fp, int traces ) {
    char *buf = callstack(traces);
    fputs(buf, fp);
    return 0;
}

// signals --------------------------------------------------------------------

const char *signal_name(int signal) {
    if(signal == SIGABRT) return "SIGABRT - \"abort\", abnormal termination";
    if(signal == SIGFPE) return "SIGFPE - floating point exception";
    if(signal == SIGILL) return "SIGILL - \"illegal\", invalid instruction";
    if(signal == SIGSEGV) return "SIGSEGV - \"segmentation violation\", invalid memory access";
    if(signal == SIGINT) return "SIGINT - \"interrupt\", interactive attention request sent to the program";
    if(signal == SIGTERM) return "SIGTERM - \"terminate\", termination request sent to the program";
    ifndef(win32, if(signal == SIGBUS) return "SIGBUS");
    ifdef(linux, if(signal == SIGSTKFLT) return "SIGSTKFLT");
    ifndef(win32, if(signal == SIGQUIT) return "SIGQUIT");
    return "??";
}
void signal_handler_ignore(int signal_) {
    signal(signal_, signal_handler_ignore);
}
void signal_handler_quit(int signal) {
    // fprintf(stderr, "Ok: caught signal %s (%d)\n", signal_name(signal), signal);
    exit(0);
}
void signal_handler_abort(int signal) {
    fprintf(stderr, "Error: unexpected signal %s (%d)\n%s\n", signal_name(signal), signal, callstack(16));
    exit(-1);
}
void signal_handler_debug(int signal) {
    breakpoint("Error: unexpected signal");
    fprintf(stderr, "Error: unexpected signal %s (%d)\n%s\n", signal_name(signal), signal, callstack(16));
    exit(-1);
}
void signal_hooks(void) {
    // expected signals
    signal(SIGINT, signal_handler_quit);
    signal(SIGTERM, signal_handler_quit);
    ifndef(win32, signal(SIGQUIT, signal_handler_quit));
    // unexpected signals
    signal(SIGABRT, signal_handler_abort);
    signal(SIGFPE, signal_handler_abort);
    signal(SIGILL, signal_handler_abort);
    signal(SIGSEGV, signal_handler_abort);
    ifndef(win32, signal(SIGBUS, signal_handler_abort));
    ifdef(linux, signal(SIGSTKFLT, signal_handler_abort));
}

#ifdef SIGNAL_DEMO
void crash() {
    char *ptr = 0;
    *ptr = 1;
}
void hang() {
    for(;;);
}
int main(int argc, char **argv) {
    signal_hooks();
    crash(); // hang();
}
#define main main__
#endif

// endian ----------------------------------------------------------------------

#if is(cl)
#include <stdlib.h>
#define swap16 _byteswap_ushort
#define swap32 _byteswap_ulong
#define swap64 _byteswap_uint64
#elif is(gcc)
#define swap16 __builtin_bswap16
#define swap32 __builtin_bswap32
#define swap64 __builtin_bswap64
#else
uint16_t swap16( uint16_t x ) { return (x << 8) | (x >> 8); }
uint32_t swap32( uint32_t x ) { x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); return (x << 16) | (x >> 16); }
uint64_t swap64( uint64_t x ) { x = ((x <<  8) & 0xff00ff00ff00ff00ULL) | ((x >>  8) & 0x00ff00ff00ff00ffULL); x = ((x << 16) & 0xffff0000ffff0000ULL) | ((x >> 16) & 0x0000ffff0000ffffULL); return (x << 32) | (x >> 32); }
#endif

float    swap32f(float n)  { union { float  t; uint32_t i; } conv; conv.t = n; conv.i = swap32(conv.i); return conv.t; }
double   swap64f(double n) { union { double t; uint64_t i; } conv; conv.t = n; conv.i = swap64(conv.i); return conv.t; }

#define is_big()    ((*(uint16_t *)"\0\1") == 1)
#define is_little() ((*(uint16_t *)"\0\1") != 1)

uint16_t  lil16(uint16_t n) { return is_big()    ? swap16(n) : n; }
uint32_t  lil32(uint32_t n) { return is_big()    ? swap32(n) : n; }
uint64_t  lil64(uint64_t n) { return is_big()    ? swap64(n) : n; }
uint16_t  big16(uint16_t n) { return is_little() ? swap16(n) : n; }
uint32_t  big32(uint32_t n) { return is_little() ? swap32(n) : n; }
uint64_t  big64(uint64_t n) { return is_little() ? swap64(n) : n; }

float     lil32f(float n)  { return is_big()    ? swap32f(n) : n; }
double    lil64f(double n) { return is_big()    ? swap64f(n) : n; }
float     big32f(float n)  { return is_little() ? swap32f(n) : n; }
double    big64f(double n) { return is_little() ? swap64f(n) : n; }

uint16_t* lil16p(void *p, int sz)  { if(is_big()   ) { uint16_t *n = (uint16_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap16(n[i]); } return p; }
uint16_t* big16p(void *p, int sz)  { if(is_little()) { uint16_t *n = (uint16_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap16(n[i]); } return p; }
uint32_t* lil32p(void *p, int sz)  { if(is_big()   ) { uint32_t *n = (uint32_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap32(n[i]); } return p; }
uint32_t* big32p(void *p, int sz)  { if(is_little()) { uint32_t *n = (uint32_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap32(n[i]); } return p; }
uint64_t* lil64p(void *p, int sz)  { if(is_big()   ) { uint64_t *n = (uint64_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap64(n[i]); } return p; }
uint64_t* big64p(void *p, int sz)  { if(is_little()) { uint64_t *n = (uint64_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap64(n[i]); } return p; }

float   * lil32pf(void *p, int sz) { if(is_big()   ) { float    *n = (float    *)p; for(int i = 0; i < sz; ++i) n[i] = swap32f(n[i]); } return p; }
float   * big32pf(void *p, int sz) { if(is_little()) { float    *n = (float    *)p; for(int i = 0; i < sz; ++i) n[i] = swap32f(n[i]); } return p; }
double  * lil64pf(void *p, int sz) { if(is_big()   ) { double   *n = (double   *)p; for(int i = 0; i < sz; ++i) n[i] = swap64f(n[i]); } return p; }
double  * big64pf(void *p, int sz) { if(is_little()) { double   *n = (double   *)p; for(int i = 0; i < sz; ++i) n[i] = swap64f(n[i]); } return p; }

// cpu -------------------------------------------------------------------------

#if is(linux)
#include <sched.h>
#endif

int app_cores() {
#if is(win32)
    DWORD_PTR pm, sm;
    if( GetProcessAffinityMask(GetCurrentProcess(), &pm, &sm) ) if( pm ) {
        int count = 0;
        while( pm ) {
            ++count;
            pm &= pm - 1;
        }
        return count;
    }
    { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
#else // unix
    int count = sysconf(_SC_NPROCESSORS_ONLN);
    return count > 0 ? count : 1;
#endif
#if 0
#elif is(linux)
    cpu_set_t prevmask, testmask;
    CPU_ZERO(&prevmask);
    CPU_ZERO(&testmask);
    sched_getaffinity(0, sizeof(prevmask), &prevmask);     //Get current mask
    sched_setaffinity(0, sizeof(testmask), &testmask);     //Set zero mask
    sched_getaffinity(0, sizeof(testmask), &testmask);     //Get mask for all CPUs
    sched_setaffinity(0, sizeof(prevmask), &prevmask);     //Reset current mask
    int num = CPU_COUNT(&testmask);
    return (num > 1 ? num : 1);
#elif is(cpp)
    return (int)std::thread::hardware_concurrency();
#elif defined(_OPENMP)
    // omp
    int cores = 0;
    #pragma omp parallel
    {
        #pragma omp atomic
        ++cores;
    }
    return cores;
#endif
}

// -----------------------------------------------------------------------------
// Battery API. Based on code by Rabia Alhaffar (UNLICENSE)
// - rlyeh, public domain.

#if is(win32)
#include <winsock2.h>

int app_battery() {
    SYSTEM_POWER_STATUS ibstatus;

    if (GetSystemPowerStatus(&ibstatus) == FALSE) {
        return 0;
    }

    int level = (ibstatus.BatteryLifePercent != 255) ? ibstatus.BatteryLifePercent : 0;
    int charging = (ibstatus.BatteryFlag & 8) > 0;
    return charging ? +level : -level;
}

#elif defined __linux__ // is(linux)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int app_battery() {
    static int battery_status_handle;
    static int battery_capacity_handle;

    do_once {
        battery_status_handle = open("/sys/class/power_supply/BAT0/status", O_RDONLY);
        battery_capacity_handle = open("/sys/class/power_supply/BAT0/capacity", O_RDONLY);
    }

    if (battery_status_handle == -1 || battery_capacity_handle == -1) {
        return 0;
    }

    char buffer[512];

    // level
    lseek(battery_capacity_handle, 0, SEEK_SET);
    int readlen = read(battery_capacity_handle, buffer, 511); buffer[readlen < 0 ? 0 : readlen] = '\0';
    int level = atoi(buffer);

    // charging
    lseek(battery_status_handle, 0, SEEK_SET);
    readlen = read(battery_status_handle, buffer, 511); buffer[readlen < 0 ? 0 : readlen] = '\0';
    int charging = strstr(buffer, "Discharging") ? 0 : 1;
    return charging ? +level : -level;
}

#elif is(osx)
#import <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#import <IOKit/ps/IOPowerSources.h>
#import <IOKit/ps/IOPSKeys.h>

int app_battery() {
    static CFDictionaryRef psrc;

    do_once {
        CFTypeRef blob = IOPSCopyPowerSourcesInfo();
        CFArrayRef sources = IOPSCopyPowerSourcesList(blob);
        int sourcesCount = CFArrayGetCount(sources);

        if (sourcesCount > 0) {
            psrc = IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, 0));
        }
    }

    if(psrc == NULL) return 0;

    int cur_cap = 0;
    CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(psrc, CFSTR(kIOPSCurrentCapacityKey)), kCFNumberSInt32Type, &cur_cap);

    int max_cap = 0;
    CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(psrc, CFSTR(kIOPSMaxCapacityKey)), kCFNumberSInt32Type, &max_cap);

    int level = (int)(cur_cap * 100.f / max_cap);
    int charging = CFDictionaryGetValue(psrc, CFSTR(kIOPSIsChargingKey)) == kCFBooleanTrue;
    return charging ? +level : -level;
}

#else

int app_battery() {
    return 0;
}

#endif

// ----------------------------------------------------------------------------
// time

#if 0
uint64_t time_gpu() {
    GLint64 t = 123456789;
    glGetInteger64v(GL_TIMESTAMP, &t);
    return (uint64_t)t;
}
#endif
uint64_t date() {
    time_t epoch = time(0);
    struct tm *ti = localtime(&epoch);
    return atoi64(va("%04d%02d%02d%02d%02d%02d",ti->tm_year+1900,ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec));
}
char *date_string() {
    time_t epoch = time(0);
    struct tm *ti = localtime(&epoch);
    return va("%04d-%02d-%02d %02d:%02d:%02d",ti->tm_year+1900,ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec);
}
uint64_t date_epoch() {
    time_t epoch = time(0);
    return epoch;
}
#if 0
double time_ss() {
    return glfwGetTime();
}
double time_ms() {
    return glfwGetTime() * 1000.0;
}
uint64_t time_us() {
    return (uint64_t)(glfwGetTime() * 1000000.0); // @fixme: use a high resolution timer instead, or time_gpu below
}
uint64_t sleep_us(uint64_t us) { // @fixme: use a high resolution sleeper instead
    return sleep_ms( us / 1000.0 );
}
double sleep_ms(double ms) {
    double now = time_ms();
    if( ms <= 0 ) {
#if is(win32)
        Sleep(0); // yield
#else
        usleep(0);
#endif
    } else {
#if is(win32)
        Sleep(ms);
#else
        usleep(ms * 1000);
#endif
    }
    return time_ms() - now;
}
double sleep_ss(double ss) {
    return sleep_ms( ss * 1000 ) / 1000.0;
}
#endif

// high-perf functions

#define TIMER_E3 1000ULL
#define TIMER_E6 1000000ULL
#define TIMER_E9 1000000000ULL

#ifdef CLOCK_MONOTONIC_RAW
#define TIME_MONOTONIC CLOCK_MONOTONIC_RAW
#elif defined CLOCK_MONOTONIC
#define TIME_MONOTONIC CLOCK_MONOTONIC
#else
// #define TIME_MONOTONIC CLOCK_REALTIME // untested
#endif

static uint64_t nanotimer(uint64_t *out_freq) {
    if( out_freq ) {
#if is(win32)
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        *out_freq = li.QuadPart;
//#elif is(ANDROID)
//      *out_freq = CLOCKS_PER_SEC;
#elif defined TIME_MONOTONIC
        *out_freq = TIMER_E9;
#else
        *out_freq = TIMER_E6;
#endif
    }
#if is(win32)
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (uint64_t)li.QuadPart;
//#elif is(ANDROID)
//    return (uint64_t)clock();
#elif defined TIME_MONOTONIC
    struct timespec ts;
    clock_gettime(TIME_MONOTONIC, &ts);
    return (TIMER_E9 * (uint64_t)ts.tv_sec) + ts.tv_nsec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (TIMER_E6 * (uint64_t)tv.tv_sec) + tv.tv_usec;
#endif
}

uint64_t time_ns() {
    static uint64_t epoch = 0;
    static uint64_t freq = 0;
    if( !freq ) {
        epoch = nanotimer(&freq);
    }

    uint64_t a = nanotimer(NULL) - epoch;
    uint64_t b = TIMER_E9;
    uint64_t c = freq;

    // Computes (a*b)/c without overflow, as long as both (a*b) and the overall result fit into 64-bits.
    // [ref] https://github.com/rust-lang/rust/blob/3809bbf47c8557bd149b3e52ceb47434ca8378d5/src/libstd/sys_common/mod.rs#L124
    uint64_t q = a / c;
    uint64_t r = a % c;
    return q * b + r * b / c;
}
uint64_t time_us() {
    return time_ns() / TIMER_E3;
}
uint64_t time_ms() {
    return time_ns() / TIMER_E6;
}
double time_ss() {
    return time_ns() / 1e9; // TIMER_E9;
}
double time_mm() {
    return time_ss() / 60;
}
double time_hh() {
    return time_mm() / 60;
}

void sleep_ns( double ns ) {
#if is(win32)
    if( ns >= 100 ) {
        LARGE_INTEGER li;      // Windows sleep in 100ns units
        HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
        li.QuadPart = (LONGLONG)(__int64)(-ns/100); // Negative for relative time
        SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
#else
    if( ns > 0 ) {
        struct timespec wait = {0};
        wait.tv_sec = ns / 1e9;
        wait.tv_nsec = ns - wait.tv_sec * 1e9;
        nanosleep(&wait, NULL);
#endif
    } else {
#if is(win32)
        Sleep(0); // yield, Sleep(0), SwitchToThread
#else
        usleep(0);
#endif
    }
}
void sleep_us( double us ) {
    sleep_ns(us * 1e3);
}
void sleep_ms( double ms ) {
    sleep_ns(ms * 1e6);
}
void sleep_ss( double ss ) {
    sleep_ns(ss * 1e9);
}

// ----------------------------------------------------------------------------
// timer

struct timer_internal_t {
    unsigned ms;
    unsigned (*callback)(unsigned interval, void *arg);
    void *arg;
    thread_ptr_t thd;
};

static int timer_func(void *arg) {
    struct timer_internal_t *p = (struct timer_internal_t*)arg;

    sleep_ms( p->ms );

    for( ;; ) {
        unsigned then = time_ms();

            p->ms = p->callback(p->ms, p->arg);
            if( !p->ms ) break;

        unsigned now = time_ms();
        unsigned lapse = now - then;
        int diff = p->ms - lapse;
        sleep_ms( diff <= 0 ? 0 : diff );
    }

    thread_exit(0);
    return 0;
}

static __thread array(struct timer_internal_t *) timers;

unsigned timer(unsigned ms, unsigned (*callback)(unsigned ms, void *arg), void *arg) {
    struct timer_internal_t *p = MALLOC( sizeof(struct timer_internal_t) );
    p->ms = ms;
    p->callback = callback;
    p->arg = arg;
    p->thd = thread_init( timer_func, p, "", 0 );

    array_push(timers, p);
    return array_count(timers);
}
void timer_destroy(unsigned i) {
    if( i-- ) {
        thread_join(timers[i]->thd);
        thread_term(timers[i]->thd);
        FREE(timers[i]);
        timers[i] = 0;
    }
}


// ----------------------------------------------------------------------------
// argc/v

static void argc_init() {
#if is(tcc) && is(linux)
    do_once {
        char buffer[128], arg0[128] = {0};
        for( FILE *fp = fopen("/proc/self/status", "rb"); fp; fclose(fp), fp = 0) {
            while( fgets(buffer, 128, fp) ) {
                if( strbeg(buffer, "Name:") ) {
                    sscanf(buffer + 5, "%s", arg0 );
                    break;
                }
            }
        }
        extern char **environ;
        __argv = environ - 2; // last argv, as stack is [argc][argv0][argv1][...][NULL][envp]
        while( !strend(*__argv,arg0) ) --__argv;
        __argc = *(int*)(__argv-1);
    }
#endif
}

int argc() {
    do_once argc_init();
    return __argc;
}
char* argv(int arg) {
    do_once argc_init();
    static __thread char empty[1];
    return (unsigned)arg < __argc ? __argv[arg] : (empty[0] = '\0', empty);
}

// ----------------------------------------------------------------------------
// options

int flag(const char *commalist) {
    while( commalist[0] ) {
        const char *begin = commalist;
        while(*commalist != ',' && *commalist != '\0') ++commalist;
        const char *end = commalist;

        char token[128];
        snprintf(token,   128, "%.*s",  (int)(end - begin), begin);

        for( int i = 1; i < argc(); ++i ) {
            char *arg = argv(i);

            if( !strcmpi( arg, token ) ) {  // --arg
                return 1;
            }
        }

        commalist = end + !!end[0];
    }
    return 0;
}

const char *option(const char *commalist, const char *defaults) {
    while( commalist[0] ) {
        const char *begin = commalist;
        while(*commalist != ',' && *commalist != '\0') ++commalist;
        const char *end = commalist;

        char token[128], tokeneq[128];
        snprintf(token,   128, "%.*s",  (int)(end - begin), begin);
        snprintf(tokeneq, 128, "%.*s=", (int)(end - begin), begin);

        for( int i = 1; i < argc(); ++i ) {
            char *arg = argv(i);

            if( strbegi( arg, tokeneq ) ) { // --arg=value
                return argv(i) + strlen(tokeneq);
            }
            if( !strcmpi( arg, token ) ) {  // --arg value
                if( (i+1) < argc() ) {
                    return argv(i+1);
                }
            }
        }

        commalist = end + !!end[0];
    }
    return defaults;
}

int optioni(const char *commalist, int defaults) {
    const char *rc = option(commalist, 0);
    return rc ? atoi(rc) : defaults;
}
float optionf(const char *commalist, float defaults) {
    const char *rc = option(commalist, 0);
    return rc ? atof(rc) : defaults;
}

// ----------------------------------------------------------------------------
// tty

void tty_color(unsigned color) {
    #if is(win32)
    do_once {
        DWORD mode = 0; SetConsoleMode(GetStdHandle(-11), (GetConsoleMode(GetStdHandle(-11), &mode), mode|4));
    }
    #endif
    if( color ) {
        // if( color == RED ) breakpoint("break on RED"); // debug
        unsigned r = (color >> 16) & 255;
        unsigned g = (color >>  8) & 255;
        unsigned b = (color >>  0) & 255;
        // 24-bit console ESC[ … 38;2;<r>;<g>;<b> … m Select RGB foreground color
        // 256-color console ESC[38;5;<fgcode>m
        // 0x00-0x07:  standard colors (as in ESC [ 30..37 m)
        // 0x08-0x0F:  high intensity colors (as in ESC [ 90..97 m)
        // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
        // 0xE8-0xFF:  grayscale from black to white in 24 steps
        r /= 51, g /= 51, b /= 51; // [0..5]
        printf("\033[38;5;%dm", r*36+g*6+b+16); // "\033[0;3%sm", color_code);
    } else {
        printf("%s", "\x1B[39;49m"); // reset
    }
}
void tty_puts(unsigned color, const char *text) {
    tty_color(color); puts(text);
}
void tty_init() {
    tty_color(0);
}
int tty_cols() {
#if is(win32)
    CONSOLE_SCREEN_BUFFER_INFO c;
    if( GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &c) ) {
        int w = c.srWindow.Right-c.srWindow.Left-c.dwCursorPosition.X;
        return w > 2 ? w - 1 : w; // w-1 to allow window resizing to a larger dimension (already printed text would break otherwise)
    }
#endif
#ifdef TIOCGWINSZ
    struct winsize ws;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    return ws.ws_col - 1;
#endif
#ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    return ts.ts_cols - 1;
#endif
    return 80;
}
void tty_detach() {
    ifdef(win32, FreeConsole());
}
void tty_attach() {
#if is(win32)
    // in order to have a Windows gui application with console:
    // - use WinMain() then AllocConsole(), but that may require supporintg different entry points for different platforms.
    // - /link /SUBSYSTEM:CONSOLE and then call FreeConsole() if no console is needed, but feels naive to flash the terminal for a second.
    // - /link /SUBSYSTEM:WINDOWS /entry:mainCRTStartup, then AllocConsole() as follows. Quoting @pmttavara: 
    //   "following calls are the closest i'm aware you can get to /SUBSYSTEM:CONSOLE in a gui program 
    //   while cleanly handling existing consoles (cmd.exe), pipes (ninja) and no console (VS/RemedyBG; double-clicking the game)"
    do_once {
        if( !AttachConsole(ATTACH_PARENT_PROCESS) && GetLastError() != ERROR_ACCESS_DENIED ) ASSERT( AllocConsole() );
        printf("\n"); // print >= 1 byte to distinguish empty stdout from a redirected stdout (fgetpos() position <= 0)
        fpos_t pos = 0;
        if( fgetpos(stdout, &pos) != 0 || pos <= 0 ) {
            ASSERT(freopen("CONIN$" , "r", stdin ));
            ASSERT(freopen("CONOUT$", "w", stderr));
            ASSERT(freopen("CONOUT$", "w", stdout));
        }
    }
#endif
}

// -----------------------------------------------------------------------------
// debugger

#include <stdio.h>
void hexdumpf( FILE *fp, const void *ptr, unsigned len, int width ) {
    unsigned char *data = (unsigned char*)ptr;
    for( unsigned jt = 0; jt <= len; jt += width ) {
        fprintf( fp, "; %05d%s", jt, jt == len ? "\n" : " " );
        for( unsigned it = jt, next = it + width; it < len && it < next; ++it ) {
            fprintf( fp, "%02x %s", (unsigned char)data[it], &" \n\0...\n"[ (1+it) < len ? 2 * !!((1+it) % width) : 3 ] );
        }
        fprintf( fp, "; %05d%s", jt, jt == len ? "\n" : " " );
        for( unsigned it = jt, next = it + width; it < len && it < next; ++it ) {
            fprintf( fp, " %c %s", (signed char)data[it] >= 32 ? (signed char)data[it] : (signed char)'.', &" \n\0..."[ (1+it) < len ? 2 * !!((1+it) % width) : 3 ] );
        }
    }
    fprintf(fp, " %d bytes\n", len);
}
void hexdump( const void *ptr, unsigned len ) {
    hexdumpf( stdout, ptr, len, 16 );
}

#if 0 // is(cl) only
static void debugbreak(void) {
    do { \
        __try { DebugBreak(); } \
        __except (GetExceptionCode() == EXCEPTION_BREAKPOINT ? \
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {} \
    } while(0);
}
#endif

#if is(win32)
static void debugbreak(void) { if(IsDebuggerPresent()) DebugBreak(); }
#else // is(unix)
static int is_debugger_present = -1;
static void _sigtrap_handler(int signum) {
    is_debugger_present = 0;
    signal(SIGTRAP, SIG_DFL);
}
static void debugbreak(void) { // break if debugger present
    // __builtin_trap(); //
    //raise(SIGABRT); // SIGTRAP);
    //__asm__ volatile("int $0x03");
    if( is_debugger_present < 0 ) {
        is_debugger_present = 1;
        signal(SIGTRAP, _sigtrap_handler);
        raise(SIGTRAP);
    }
}
#endif

void alert(const char *message) { // @todo: move to app_, besides die()
#if is(win32)
    MessageBoxA(0, message, 0,0);
#elif is(ems)
    emscripten_run_script(va("alert('%s')", message));
#elif is(linux)
    for(FILE *fp = fopen("/tmp/fwk.warning","wb");fp;fp=0)
    fputs(message,fp), fclose(fp), system("xmessage -center -file /tmp/fwk.warning");
#elif is(osx)
    system(va("osascript -e 'display alert \"Alert\" message \"%s\"'", message));
#endif
}

void breakpoint(const char *reason) {
    window_visible(false);
    if( reason ) {
        const char *fulltext = reason[0] == '!' ? va("%s\n%s", reason+1, callstack(+48)) : reason;
        PRINTF("%s", fulltext);

        (alert)(fulltext);
    }
    debugbreak();
    window_visible(true);
}

bool has_debugger() {
#if is(win32)
    return IsDebuggerPresent(); // SetLastError(123); OutputDebugStringA("\1"); enabled = GetLastError() != 123;
#else
    return false;
#endif
}

void die(const char *message) {
   fprintf(stderr, "%s\n", message);
   fflush(stderr);
   alert(message);
   exit(-1);
}

// ----------------------------------------------------------------------------
// logger

unsigned determine_color_from_text(const char *text) {
    /**/ if( strstri(text, "fail") || strstri(text, "error") ) return RED;
    else if( strstri(text, "warn") || strstri(text, "not found") ) return YELLOW;
    return 0;
}

//static int __thread _thread_id;
//#define PRINTF(...)      (printf("%03d %07.3fs|%-16s|", (((unsigned)(uintptr_t)&_thread_id)>>8) % 1000, time_ss(), __FUNCTION__), printf(__VA_ARGS__), printf("%s", 1[#__VA_ARGS__] == '!' ? callstack(+48) : "")) // verbose logger

int (PRINTF)(const char *text, const char *stack, const char *file, int line, const char *function) {
    double secs = time_ss();
    uint32_t color = /*errno ? RED :*/ determine_color_from_text(text); // errno = 0;
    #if is(cl)
    char *slash = strrchr(file, '\\'); if(slash) file = slash + 1;
    #endif
    char *location = va("|%s|%s:%d", /*errno?strerror(errno):*/function, file, line);
    int cols = tty_cols() + 1 - (int)strlen(location);

static thread_mutex_t lock, *init = 0; if(!init) thread_mutex_init(init = &lock);
thread_mutex_lock( &lock );

    tty_color(color);
    printf("\r%*.s%s", cols, "", location);
    printf("\r%07.3fs|%s%s", secs, text, stack);
    tty_color(0);

thread_mutex_unlock( &lock );
    return 1;
}

// ----------------------------------------------------------------------------
// panic

static void *panic_oom_reserve; // for out-of-memory recovery
int (PANIC)(const char *error, const char *file, int line) {
    panic_oom_reserve = SYS_REALLOC(panic_oom_reserve, 0);

    tty_color(RED);

    error += error[0] == '!';
    fprintf(stderr, "Error: %s (%s:%d) (errno:%s)\n", error, file, line, strerror(errno));
    fprintf(stderr, "%s", callstack(+16)); // no \n
    fflush(0); // fflush(stderr);

    tty_color(0);

    breakpoint(error);
    exit(-line);
    return 1;
}

// ----------------------------------------------------------------------------
// threads

struct thread_wrapper {
    int (*func)(void *user_data);
    void *user_data;
};

static
int thread_proc( void* user_data ) {
    struct thread_wrapper *w = (struct thread_wrapper*)user_data;
    int return_code = w->func( w->user_data );
    thread_exit( return_code );
    FREE(w);
    return 0;
}

void* thread( int (*thread_func)(void* user_data), void* user_data ) {
    struct thread_wrapper *w = MALLOC(sizeof(struct thread_wrapper));
    w->func = thread_func;
    w->user_data = user_data;

    int thread_stack_size = 0;
    const char *thread_name = "";
    thread_ptr_t thd = thread_init( thread_proc, w, thread_name, thread_stack_size );
    return thd;
}
void thread_destroy( void *thd ) {
    int rc = thread_join(thd);
    thread_term(thd);
}

#line 0

#line 1 "fwk_ui.c"

#ifndef UI_ICONS_SMALL
//#define UI_ICONS_SMALL 1
#endif

#define UI_FONT_ENUM(carlito,b612) b612 // carlito

#define UI_FONT_ICONS    "MaterialIconsSharp-Regular.otf" // "MaterialIconsOutlined-Regular.otf" "MaterialIcons-Regular.ttf" //
#define UI_FONT_REGULAR  UI_FONT_ENUM("Carlito",     "B612")     "-Regular.ttf"
#define UI_FONT_HEADING  UI_FONT_ENUM("Carlito",     "B612")     "-BoldItalic.ttf"
#define UI_FONT_TERMINAL UI_FONT_ENUM("Inconsolata", "B612Mono") "-Regular.ttf"

#if UI_LESSER_SPACING
    enum { UI_SEPARATOR_HEIGHT = 5, UI_MENUBAR_ICON_HEIGHT = 20, UI_ROW_HEIGHT = 22, UI_MENUROW_HEIGHT = 32 };
#else
    enum { UI_SEPARATOR_HEIGHT = 10, UI_MENUBAR_ICON_HEIGHT = 25, UI_ROW_HEIGHT = 32, UI_MENUROW_HEIGHT = 32 };
#endif

#if UI_FONT_LARGE
    #define UI_FONT_REGULAR_SIZE    UI_FONT_ENUM(18,17)
    #define UI_FONT_HEADING_SIZE    UI_FONT_ENUM(20,19)
    #define UI_FONT_TERMINAL_SIZE   UI_FONT_ENUM(14,14)
#elif UI_FONT_SMALL
    #define UI_FONT_REGULAR_SIZE    UI_FONT_ENUM(13,14)
    #define UI_FONT_HEADING_SIZE    UI_FONT_ENUM(14.5,15)
    #define UI_FONT_TERMINAL_SIZE   UI_FONT_ENUM(14,14)
#else
    #define UI_FONT_REGULAR_SIZE    UI_FONT_ENUM(14.5,16)
    #define UI_FONT_HEADING_SIZE    UI_FONT_ENUM(16,17.5)
    #define UI_FONT_TERMINAL_SIZE   UI_FONT_ENUM(14,14)
#endif

#if UI_ICONS_SMALL
    #define UI_ICON_FONTSIZE        UI_FONT_ENUM(16.5f,16.5f)
    #define UI_ICON_SPACING_X       UI_FONT_ENUM(-2,-2)
    #define UI_ICON_SPACING_Y       UI_FONT_ENUM(4.5f,3.5f)
#else
    #define UI_ICON_FONTSIZE        UI_FONT_ENUM(20,20)
    #define UI_ICON_SPACING_X       UI_FONT_ENUM(0,0)
    #define UI_ICON_SPACING_Y       UI_FONT_ENUM(6.5f,5.0f)
#endif

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

static struct nk_context *ui_ctx;
static struct nk_glfw nk_glfw = {0};

void* ui_handle() {
    return ui_ctx;
}

static void nk_config_custom_fonts() {
    #define UI_ICON_MIN ICON_MIN_MD
    #define UI_ICON_MED ICON_MAX_16_MD
    #define UI_ICON_MAX ICON_MAX_MD

    #define ICON_BARS        ICON_MD_MENU
    #define ICON_FILE        ICON_MD_INSERT_DRIVE_FILE
    #define ICON_TRASH       ICON_MD_DELETE

    struct nk_font *font = NULL;
    struct nk_font_atlas *atlas = NULL;
    nk_glfw3_font_stash_begin(&nk_glfw, &atlas); // nk_sdl_font_stash_begin(&atlas);

        // Default font(#1)...

        for( char *data = vfs_read(UI_FONT_REGULAR); data; data = 0 ) {
            float font_size = UI_FONT_REGULAR_SIZE;
                struct nk_font_config cfg = nk_font_config(font_size);
                cfg.oversample_v = 2;
                cfg.pixel_snap = 0;
            // win32: struct nk_font *arial = nk_font_atlas_add_from_file(atlas, va("%s/fonts/arial.ttf",getenv("windir")), font_size, &cfg); font = arial ? arial : font;
            // struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/DroidSans.ttf", font_size, &cfg); font = droid ? droid : font;
            struct nk_font *regular = nk_font_atlas_add_from_memory(atlas, data, vfs_size(UI_FONT_REGULAR), font_size, &cfg); font = regular ? regular : font;
        }

        // ...with icons embedded on it.

        for( char *data = vfs_read(UI_FONT_ICONS); data; data = 0 ) {
            static const nk_rune icon_range[] = {UI_ICON_MIN, UI_ICON_MED /*MAX*/, 0};

            struct nk_font_config cfg = nk_font_config(UI_ICON_FONTSIZE);
            cfg.range = icon_range; // nk_font_default_glyph_ranges();
            cfg.merge_mode = 1;

            cfg.spacing.x += UI_ICON_SPACING_X;
            cfg.spacing.y += UI_ICON_SPACING_Y;
         // cfg.font->ascent += ICON_ASCENT;
         // cfg.font->height += ICON_HEIGHT;

            cfg.oversample_h = 1;
            cfg.oversample_v = 1;
            cfg.pixel_snap = 1;

            struct nk_font *icons = nk_font_atlas_add_from_memory(atlas, data, vfs_size(UI_FONT_ICONS), UI_ICON_FONTSIZE, &cfg);
        }

        // Monospaced font. Used in terminals or consoles.

        for( char *data = vfs_read(UI_FONT_TERMINAL); data; data = 0 ) {
            const float font_size = UI_FONT_REGULAR_SIZE; 
            static const nk_rune icon_range[] = {32, 127, 0};

            struct nk_font_config cfg = nk_font_config(font_size);
            cfg.range = icon_range;

            cfg.oversample_h = 1;
            cfg.oversample_v = 1;
            cfg.pixel_snap = 1;

            // struct nk_font *proggy = nk_font_atlas_add_default(atlas, font_size, &cfg);
            struct nk_font *bold = nk_font_atlas_add_from_memory(atlas, data, vfs_size(UI_FONT_TERMINAL), font_size, &cfg);
        }

        // Extra optional fonts from here...

        for( char *data = vfs_read(UI_FONT_HEADING); data; data = 0 ) {
            struct nk_font *bold = nk_font_atlas_add_from_memory(atlas, data, vfs_size(UI_FONT_HEADING), UI_FONT_HEADING_SIZE, 0); // font = bold ? bold : font;
        }

    nk_glfw3_font_stash_end(&nk_glfw); // nk_sdl_font_stash_end();
//  ASSERT(font);
    if(font) nk_style_set_font(ui_ctx, &font->handle);

    // Load Cursor: if you uncomment cursor loading please hide the cursor
    // nk_style_load_all_cursors(ctx, atlas->cursors); glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

static void nk_config_custom_theme() {
    #ifdef UI_HUE
    float default_hue = UI_HUE;
    #else
    // 0.09 orange, 0.14 yellow, 0.40 green, 0.45 turquoise, 0.50 cyan, 0.52 default, 0.55 blue, 0.80 purple, 0.92 cherry, 0.96 red
    float hues[] = { 0.40,0.45,0.50,0.52,0.55,0.80,0.92 };
    float default_hue = hues[ (int)((((date() / 10000) % 100) / 24.f) * countof(hues)) ]; // YYYYMMDDhhmmss -> hh as 0..1
default_hue = 0.52;
    #endif
    float hue = clampf( optionf("--ui-hue", default_hue /*= 0.52*/), 0, 1 );
    struct nk_color main_hue   = nk_hsv_f(hue+0.025, 0.80, 0.400); // washed
    struct nk_color hover_hue  = nk_hsv_f(hue+0.025, 1.00, 0.600); // vivid
    struct nk_color active_hue = nk_hsv_f(hue-0.010, 1.00, 0.600); // bright; same /S/V than vivid, but H/ slighty biased towards a different luma on spectrum
    struct nk_color main       = nk_hsv_f(    0.600, 0.00, 0.125); // washed b/w
    struct nk_color hover      = nk_hsv_f(    0.900, 0.00, 0.000); // vivid  b/w
    struct nk_color active     = nk_hsv_f(    0.600, 0.00, 0.150); // bright b/w
    struct nk_color table[NK_COLOR_COUNT] = {0};
    table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(42, 42, 42, 245);
    table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 245);
    table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
    table[NK_COLOR_BUTTON] = main;
    table[NK_COLOR_BUTTON_HOVER] = hover;
    table[NK_COLOR_BUTTON_ACTIVE] = active;
    // ok
    table[NK_COLOR_TOGGLE] = nk_rgba(45*1.2, 53*1.2, 56*1.2, 255); // table[NK_COLOR_WINDOW]; // nk_rgba(45/1.2, 53/1.2, 56/1.2, 255);
    table[NK_COLOR_TOGGLE_HOVER] = active;
    table[NK_COLOR_TOGGLE_CURSOR] = main; // vivid_blue;
    table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = main_hue;
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = hover_hue;
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = active_hue;
    table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SLIDER_CURSOR] = main_hue;
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = hover_hue;
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = active_hue;
    table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);

    // table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);

    // table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
table[NK_COLOR_CHART_COLOR] = main_hue;
table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = hover_hue; // nk_rgba(255, 0, 0, 255);
    // table[NK_COLOR_TAB_HEADER] = main;
    // table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
    // table[NK_COLOR_SELECT_ACTIVE] = main;

    // @transparent
    #if !is(ems)
    if( glfwGetWindowAttrib(window_handle(), GLFW_TRANSPARENT_FRAMEBUFFER) == GLFW_TRUE ) {
        table[NK_COLOR_WINDOW].a =
        table[NK_COLOR_HEADER].a = 255;
    }
    #endif
    // @transparent

    nk_style_default(ui_ctx);
    nk_style_from_table(ui_ctx, table);

    struct nk_style *s = &ui_ctx->style;
    s->window.spacing = nk_vec2(4,0);
    s->window.combo_border = 0.f;
    s->window.scrollbar_size = nk_vec2(5,5);
    s->property.rounding = 0;
    s->combo.border = 0;
    s->button.border = 1;
    s->edit.border = 0;

    if( UI_ROW_HEIGHT < 32 ) { // UI_LESSER_SPACING
    s->window.header.label_padding.y /= 2; // 2
    s->window.header.padding.y /= 2; // /= 4 -> 1
    }
}

static float ui_alpha = 1;
static array(float) ui_alphas;
static void ui_alpha_push(float alpha) {
    array_push(ui_alphas, ui_alpha);
    ui_alpha = alpha;

    struct nk_color c;
    struct nk_style *s = &ui_ctx->style;
    c = s->window.background;                  c.a = alpha * 255; nk_style_push_color(ui_ctx, &s->window.background, c);
    c = s->text.color;                         c.a = alpha * 255; nk_style_push_color(ui_ctx, &s->text.color, c);
    c = s->window.fixed_background.data.color; c.a = alpha * 255; nk_style_push_style_item(ui_ctx, &s->window.fixed_background, nk_style_item_color(c));
}
static void ui_alpha_pop() {
    if( array_count(ui_alphas) ) {
        nk_style_pop_style_item(ui_ctx);
        nk_style_pop_color(ui_ctx);
        nk_style_pop_color(ui_ctx);

        ui_alpha = *array_back(ui_alphas);
        array_pop(ui_alphas);
    }
}

// -----------------------------------------------------------------------------
// ui menu

typedef struct ui_item_t {
    char *buf;
    int bufcap;
    int type; // 0xED17 'edit' for a writeable inputbox buffer, else read-only label
} ui_item_t;

static array(ui_item_t) ui_items; // queued menu names. to be evaluated during next frame
static vec2 ui_results = {0}; // clicked menu items from last frame

int ui_item() {
    return ui_items ? (ui_results.x == array_count(ui_items) ? ui_results.y : 0) : 0;
}

int ui_menu(const char *items) { // semicolon- or comma-separated items
    array_push(ui_items, ((ui_item_t){STRDUP(items),0,0}));
    return ui_item();
}
int ui_menu_editbox(char *buf, int bufcap) {
    array_push(ui_items, ((ui_item_t){buf,bufcap,0xED17}));
    return ui_item();
}

int ui_has_menubar() {
    return !!ui_items; // array_count(ui_items) > 0;
}

static
void ui_separator_line() {
    struct nk_rect space; nk_layout_peek(&space, ui_ctx); // bounds.w *= 0.95f;
    struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
    nk_stroke_line(canvas, space.x+0,space.y+0,space.x+space.w,space.y+0, 3.0, nk_rgb(128,128,128));
}

NK_API nk_bool
nk_menu_begin_text_styled(struct nk_context *ctx, const char *title, int len,
    nk_flags align, struct nk_vec2 size, struct nk_style_button *style_button) //< @r-lyeh: added style_button param
{
    struct nk_window *win;
    const struct nk_input *in;
    struct nk_rect header;
    int is_clicked = nk_false;
    nk_flags state;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return 0;

    win = ctx->current;
    state = nk_widget(&header, ctx);
    if (!state) return 0;
    in = (state == NK_WIDGET_ROM || win->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    if (nk_do_button_text(&ctx->last_widget_state, &win->buffer, header,
        title, len, align, NK_BUTTON_DEFAULT, style_button, in, ctx->style.font))
        is_clicked = nk_true;
    return nk_menu_begin(ctx, win, title, is_clicked, header, size);
}

static
vec2 ui_toolbar_(array(ui_item_t) ui_items, vec2 ui_results) {
    // adjust size for all the upcoming UI elements
    // old method: nk_layout_row_dynamic(ui_ctx, UI_MENUBAR_ICON_HEIGHT/*h*/, array_count(ui_items));
    {
        const struct nk_style *style = &ui_ctx->style;
        const struct nk_user_font *f = style->font;

        nk_layout_row_template_begin(ui_ctx, UI_MENUBAR_ICON_HEIGHT/*h*/);
        for(int i = 0; i < array_count(ui_items); ++i) {
            char first_token[512];
            sscanf(ui_items[i].buf, "%[^,;|]", first_token); // @fixme: vsnscanf

            char *tooltip = strchr(first_token, '@');
            int len = tooltip ? (int)(tooltip - first_token /*- 1*/) : strlen(first_token);

            float pixels_width = f->width(f->userdata, f->height, first_token, len);
            pixels_width += style->window.header.label_padding.x * 2 + style->window.header.padding.x * 2;
            if( pixels_width < 5 ) pixels_width = 5;
            nk_layout_row_template_push_static(ui_ctx, pixels_width);
        }
        nk_layout_row_template_end(ui_ctx);
    }

    // display the UI elements
    bool has_popups = ui_popups();
    for( int i = 0, end = array_count(ui_items); i < end; ++i ) {
        array(char*) ids = strsplit(ui_items[i].buf, ",;|");

        // transparent style
        static struct nk_style_button transparent_style;
        do_once transparent_style = ui_ctx->style.button;
        do_once transparent_style.normal.data.color = nk_rgba(0,0,0,0);
        do_once transparent_style.border_color = nk_rgba(0,0,0,0);
        do_once transparent_style.active = transparent_style.normal;
        do_once transparent_style.hover = transparent_style.normal;
        do_once transparent_style.hover.data.color = nk_rgba(0,0,0,127);
        transparent_style.text_alignment = NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE; // array_count(ids) > 1 ? NK_TEXT_ALIGN_LEFT : NK_TEXT_ALIGN_CENTERED;

        char *tooltip = strchr(ids[0], '@');
        int len = tooltip ? (int)(tooltip -  ids[0]) : strlen(ids[0]);

        // single button
        if( array_count(ids) == 1 ) {
            // tooltip
            if( tooltip && !has_popups ) {
                struct nk_rect bounds = nk_widget_bounds(ui_ctx);
                if (nk_input_is_mouse_hovering_rect(&ui_ctx->input, bounds) && nk_window_has_focus(ui_ctx)) {
                    nk_tooltip(ui_ctx, tooltip+1);
                }
            }
            // input...
            if( ui_items[i].type == 0xED17 ) {
                int active = nk_edit_string_zero_terminated(ui_ctx, NK_EDIT_AUTO_SELECT|NK_EDIT_CLIPBOARD|NK_EDIT_FIELD/*NK_EDIT_BOX*/|NK_EDIT_SIG_ENTER, ui_items[i].buf, ui_items[i].bufcap, nk_filter_default);
                if( !!(active & NK_EDIT_COMMITED) ) ui_results = vec2(i+1, 0+1), nk_edit_unfocus(ui_ctx);
            }
            else
            // ... else text
            if( nk_button_text_styled(ui_ctx, &transparent_style, ids[0], len) ) {
                ui_results = vec2(i+1, 0+1);
            }
        }
        else {
            struct nk_vec2 dims = {120, array_count(ids) * UI_MENUROW_HEIGHT};
            const struct nk_style *style = &ui_ctx->style;
            const struct nk_user_font *f = style->font;
            static array(float) lens = 0; array_resize(lens, array_count(ids));
            lens[0] = len;
            for( int j = 1; j < array_count(ids); ++j ) {
                lens[j] = strlen(ids[j]);
                float width_px = f->width(f->userdata, f->height, ids[j], lens[j]);
                dims.x = maxf(dims.x, width_px);
            }
            dims.x += 2 * style->window.header.label_padding.x;

            // dropdown menu
            if( nk_menu_begin_text_styled(ui_ctx, ids[0], lens[0], NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE, dims, &transparent_style) ) {
                nk_layout_row_dynamic(ui_ctx, 0, 1);

                for( int j = 1; j < array_count(ids); ++j ) {
                    char *item = ids[j];
                    if( *item == '-' ) {
                        while(*item == '-') ++item, --lens[j];
                        //nk_menu_item_label(ui_ctx, "---", NK_TEXT_LEFT);
                        ui_separator_line();
                    }

                    if( nk_menu_item_text(ui_ctx, item, lens[j], NK_TEXT_LEFT) ) {
                        ui_results = vec2(i+1, j+1-1);
                    }
                }                    

                nk_menu_end(ui_ctx);
            }            
        }
    }

    return ui_results;
}

int ui_toolbar(const char *icons) { // usage: int clicked_icon = ui_toolbar( ICON_1 ";" ICON_2 ";" ICON_3 ";" ICON_4 );
    vec2 results = {0};
    array(char*) items = strsplit(icons, ",;|");
        static array(ui_item_t) temp = 0;
        array_resize(temp, array_count(items));
        for( int i = 0; i < array_count(items); ++i ) temp[i].buf = items[i], temp[i].bufcap = 0, temp[i].type = 0;
    return ui_toolbar_(temp, results).x;
}


// UI Windows handlers. These are not OS Windows but UI Windows instead. For OS Windows check window_*() API.

#ifndef WINDOWS_INI
#define WINDOWS_INI editor_path("windows.ini")
#endif

static map(char*,unsigned) ui_windows = 0;

static void ui_init() {
    do_once {
        nk_config_custom_fonts();
        nk_config_custom_theme();

        map_init(ui_windows, less_str, hash_str);
    }
}

static int ui_window_register(const char *panel_or_window_title) {
    unsigned *state = map_find_or_add_allocated_key(ui_windows, STRDUP(panel_or_window_title), 0);

    // check for visibility flag on first call
    int visible = 0;
    if( *state == 0 ) {
        static ini_t i = 0;
        do_once i = ini(WINDOWS_INI); // @leak
        char **found = i ? map_find(i, va("%s.visible", panel_or_window_title)) : NULL;
        if( found ) visible = (*found)[0] == '1';
    }

    *state |= 2;
    return visible;
}
int ui_visible(const char *panel_or_window_title) {
    return *map_find_or_add_allocated_key(ui_windows, STRDUP(panel_or_window_title), 0) & 1;
}
int ui_show(const char *panel_or_window_title, int enabled) {
    unsigned *found = map_find_or_add_allocated_key(ui_windows, STRDUP(panel_or_window_title), 0);
    if( enabled ) {
        *found |= 1;
        nk_window_collapse(ui_ctx, panel_or_window_title, NK_MAXIMIZED); // in case windows was previously collapsed
    } else {
        *found &= ~1;
    }
    return !!enabled;
}
int ui_dims(const char *panel_or_window_title, float width, float height) {
    nk_window_set_size(ui_ctx, panel_or_window_title, (struct nk_vec2){width, height});
    return 0;
}
vec2 ui_get_dims() {
    return (vec2){nk_window_get_width(ui_ctx), nk_window_get_height(ui_ctx)};
}
static char *ui_build_window_list() {
    char *build_windows_menu = 0;
    strcatf(&build_windows_menu, "%s;", ICON_MD_VIEW_QUILT); // "Windows");
    for each_map_ptr_sorted(ui_windows, char*, k, unsigned, v) {
        strcatf(&build_windows_menu, "%s %s;", ui_visible(*k) ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK, *k); // ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF, *k); // ICON_MD_TOGGLE_ON : ICON_MD_TOGGLE_OFF, *k);
    }
    strcatf(&build_windows_menu, "-%s;%s", ICON_MD_RECYCLING " Reset layout", ICON_MD_SAVE_AS " Save layout");
    return build_windows_menu; // @leak if discarded
}
static int ui_layout_all_reset(const char *mask);
static int ui_layout_all_save_disk(const char *mask);
static int ui_layout_all_load_disk(const char *mask);


static
void ui_menu_render() {
    // clean up from past frame
    ui_results = vec2(0,0);
    if( !ui_items ) return;
    if( !array_count(ui_items) ) return;

// artificially inject Windows menu on the first icon
bool show_window_menu = !!array_count(ui_items);
if( show_window_menu ) {
    array_push_front(ui_items, ((ui_item_t){ui_build_window_list(), 0, 0}));
}

    // process menus
    if( nk_begin(ui_ctx, "Menu", nk_rect(0, 0, window_width(), UI_MENUROW_HEIGHT), NK_WINDOW_NO_SCROLLBAR/*|NK_WINDOW_BACKGROUND*/)) {
        if( ui_ctx->current ) {
            nk_menubar_begin(ui_ctx);

            ui_results = ui_toolbar_(ui_items, ui_results);

            //nk_layout_row_end(ui_ctx);
            nk_menubar_end(ui_ctx);
        }
    }
    nk_end(ui_ctx);

if( show_window_menu ) {
    // if clicked on first menu (Windows)
    if( ui_results.x == 1 ) {
        array(char*) split = strsplit(ui_items[0].buf,";"); // *array_back(ui_items), ";");
        const char *title = split[(int)ui_results.y]; title += title[0] == '-'; title += 3 * (title[0] == '\xee'); title += title[0] == ' '; /*skip separator+emoji+space*/
        // toggle window unless clicked on lasts items {"reset layout", "save layout"}
        bool clicked_reset_layout = ui_results.y == array_count(split) - 2;
        bool clicked_save_layout = ui_results.y == array_count(split) - 1;
        /**/ if( clicked_reset_layout ) ui_layout_all_reset("*");
        else if( clicked_save_layout ) file_delete(WINDOWS_INI), ui_layout_all_save_disk("*");
        else ui_show(title, ui_visible(title) ^ true);
        // reset value so developers don't catch this click
        ui_results = vec2(0,0);
    }
    // restore state prior to previously injected Windows menu
    else
    ui_results.x = ui_results.x > 0 ? ui_results.x - 1 : 0;
}

    // clean up for next frame
    for( int i = 0; i < array_count(ui_items); ++i ) {
        if(ui_items[i].type != 0xED17)
            FREE(ui_items[i].buf);
    }
    array_resize(ui_items, 0);
}

// -----------------------------------------------------------------------------

static int ui_dirty = 1;
static int ui_has_active_popups = 0;
static float ui_hue = 0; // hue
static int ui_is_hover = 0;
static int ui_is_active = 0;
static uint64_t ui_active_mask = 0;

int ui_popups() {
    return ui_has_active_popups;
}
int ui_hover() {
    return ui_is_hover;
}
int ui_active() {
    return ui_is_active; //window_has_cursor() && nk_window_is_any_hovered(ui_ctx) && nk_item_is_any_active(ui_ctx);
}

static
int ui_set_enable_(int enabled) {
    static struct nk_style off, on;
    do_once {
        off = on = ui_ctx->style;
        float alpha = 0.5f;

        off.text.color.a *= alpha;

        off.button.normal.data.color.a *= alpha;
        off.button.hover.data.color.a *= alpha;
        off.button.active.data.color.a *= alpha;
        off.button.border_color.a *= alpha;
        off.button.text_background.a *= alpha;
        off.button.text_normal.a *= alpha;
        off.button.text_hover.a *= alpha;
        off.button.text_active.a *= alpha;

        off.contextual_button.normal.data.color.a *= alpha;
        off.contextual_button.hover.data.color.a *= alpha;
        off.contextual_button.active.data.color.a *= alpha;
        off.contextual_button.border_color.a *= alpha;
        off.contextual_button.text_background.a *= alpha;
        off.contextual_button.text_normal.a *= alpha;
        off.contextual_button.text_hover.a *= alpha;
        off.contextual_button.text_active.a *= alpha;

        off.menu_button.normal.data.color.a *= alpha;
        off.menu_button.hover.data.color.a *= alpha;
        off.menu_button.active.data.color.a *= alpha;
        off.menu_button.border_color.a *= alpha;
        off.menu_button.text_background.a *= alpha;
        off.menu_button.text_normal.a *= alpha;
        off.menu_button.text_hover.a *= alpha;
        off.menu_button.text_active.a *= alpha;

        off.option.normal.data.color.a *= alpha;
        off.option.hover.data.color.a *= alpha;
        off.option.active.data.color.a *= alpha;
        off.option.border_color.a *= alpha;
        off.option.cursor_normal.data.color.a *= alpha;
        off.option.cursor_hover.data.color.a *= alpha;
        off.option.text_normal.a *= alpha;
        off.option.text_hover.a *= alpha;
        off.option.text_active.a *= alpha;
        off.option.text_background.a *= alpha;

        off.checkbox.normal.data.color.a *= alpha;
        off.checkbox.hover.data.color.a *= alpha;
        off.checkbox.active.data.color.a *= alpha;
        off.checkbox.border_color.a *= alpha;
        off.checkbox.cursor_normal.data.color.a *= alpha;
        off.checkbox.cursor_hover.data.color.a *= alpha;
        off.checkbox.text_normal.a *= alpha;
        off.checkbox.text_hover.a *= alpha;
        off.checkbox.text_active.a *= alpha;
        off.checkbox.text_background.a *= alpha;

        off.selectable.normal.data.color.a *= alpha;
        off.selectable.hover.data.color.a *= alpha;
        off.selectable.pressed.data.color.a *= alpha;
        off.selectable.normal_active.data.color.a *= alpha;
        off.selectable.hover_active.data.color.a *= alpha;
        off.selectable.pressed_active.data.color.a *= alpha;
        off.selectable.text_normal.a *= alpha;
        off.selectable.text_hover.a *= alpha;
        off.selectable.text_pressed.a *= alpha;
        off.selectable.text_normal_active.a *= alpha;
        off.selectable.text_hover_active.a *= alpha;
        off.selectable.text_pressed_active.a *= alpha;
        off.selectable.text_background.a *= alpha;

        off.slider.normal.data.color.a *= alpha;
        off.slider.hover.data.color.a *= alpha;
        off.slider.active.data.color.a *= alpha;
        off.slider.border_color.a *= alpha;
        off.slider.bar_normal.a *= alpha;
        off.slider.bar_hover.a *= alpha;
        off.slider.bar_active.a *= alpha;
        off.slider.bar_filled.a *= alpha;
        off.slider.cursor_normal.data.color.a *= alpha;
        off.slider.cursor_hover.data.color.a *= alpha;
        off.slider.cursor_active.data.color.a *= alpha;
        off.slider.dec_button.normal.data.color.a *= alpha;
        off.slider.dec_button.hover.data.color.a *= alpha;
        off.slider.dec_button.active.data.color.a *= alpha;
        off.slider.dec_button.border_color.a *= alpha;
        off.slider.dec_button.text_background.a *= alpha;
        off.slider.dec_button.text_normal.a *= alpha;
        off.slider.dec_button.text_hover.a *= alpha;
        off.slider.dec_button.text_active.a *= alpha;
        off.slider.inc_button.normal.data.color.a *= alpha;
        off.slider.inc_button.hover.data.color.a *= alpha;
        off.slider.inc_button.active.data.color.a *= alpha;
        off.slider.inc_button.border_color.a *= alpha;
        off.slider.inc_button.text_background.a *= alpha;
        off.slider.inc_button.text_normal.a *= alpha;
        off.slider.inc_button.text_hover.a *= alpha;
        off.slider.inc_button.text_active.a *= alpha;

        off.progress.normal.data.color.a *= alpha;
        off.progress.hover.data.color.a *= alpha;
        off.progress.active.data.color.a *= alpha;
        off.progress.border_color.a *= alpha;
        off.progress.cursor_normal.data.color.a *= alpha;
        off.progress.cursor_hover.data.color.a *= alpha;
        off.progress.cursor_active.data.color.a *= alpha;
        off.progress.cursor_border_color.a *= alpha;

        off.property.normal.data.color.a *= alpha;
        off.property.hover.data.color.a *= alpha;
        off.property.active.data.color.a *= alpha;
        off.property.border_color.a *= alpha;
        off.property.label_normal.a *= alpha;
        off.property.label_hover.a *= alpha;
        off.property.label_active.a *= alpha;
        off.property.edit.normal.data.color.a *= alpha;
        off.property.edit.hover.data.color.a *= alpha;
        off.property.edit.active.data.color.a *= alpha;
        off.property.edit.border_color.a *= alpha;
        off.property.edit.cursor_normal.a *= alpha;
        off.property.edit.cursor_hover.a *= alpha;
        off.property.edit.cursor_text_normal.a *= alpha;
        off.property.edit.cursor_text_hover.a *= alpha;
        off.property.edit.text_normal.a *= alpha;
        off.property.edit.text_hover.a *= alpha;
        off.property.edit.text_active.a *= alpha;
        off.property.edit.selected_normal.a *= alpha;
        off.property.edit.selected_hover.a *= alpha;
        off.property.edit.selected_text_normal.a *= alpha;
        off.property.edit.selected_text_hover.a *= alpha;
        off.property.dec_button.normal.data.color.a *= alpha;
        off.property.dec_button.hover.data.color.a *= alpha;
        off.property.dec_button.active.data.color.a *= alpha;
        off.property.dec_button.border_color.a *= alpha;
        off.property.dec_button.text_background.a *= alpha;
        off.property.dec_button.text_normal.a *= alpha;
        off.property.dec_button.text_hover.a *= alpha;
        off.property.dec_button.text_active.a *= alpha;
        off.property.inc_button.normal.data.color.a *= alpha;
        off.property.inc_button.hover.data.color.a *= alpha;
        off.property.inc_button.active.data.color.a *= alpha;
        off.property.inc_button.border_color.a *= alpha;
        off.property.inc_button.text_background.a *= alpha;
        off.property.inc_button.text_normal.a *= alpha;
        off.property.inc_button.text_hover.a *= alpha;
        off.property.inc_button.text_active.a *= alpha;

        off.edit.normal.data.color.a *= alpha;
        off.edit.hover.data.color.a *= alpha;
        off.edit.active.data.color.a *= alpha;
        off.edit.border_color.a *= alpha;
        off.edit.cursor_normal.a *= alpha;
        off.edit.cursor_hover.a *= alpha;
        off.edit.cursor_text_normal.a *= alpha;
        off.edit.cursor_text_hover.a *= alpha;
        off.edit.text_normal.a *= alpha;
        off.edit.text_hover.a *= alpha;
        off.edit.text_active.a *= alpha;
        off.edit.selected_normal.a *= alpha;
        off.edit.selected_hover.a *= alpha;
        off.edit.selected_text_normal.a *= alpha;
        off.edit.selected_text_hover.a *= alpha;

        off.chart.background.data.color.a *= alpha;
        off.chart.border_color.a *= alpha;
        off.chart.selected_color.a *= alpha;
        off.chart.color.a *= alpha;

        off.scrollh.normal.data.color.a *= alpha;
        off.scrollh.hover.data.color.a *= alpha;
        off.scrollh.active.data.color.a *= alpha;
        off.scrollh.border_color.a *= alpha;
        off.scrollh.cursor_normal.data.color.a *= alpha;
        off.scrollh.cursor_hover.data.color.a *= alpha;
        off.scrollh.cursor_active.data.color.a *= alpha;
        off.scrollh.cursor_border_color.a *= alpha;

        off.scrollv.normal.data.color.a *= alpha;
        off.scrollv.hover.data.color.a *= alpha;
        off.scrollv.active.data.color.a *= alpha;
        off.scrollv.border_color.a *= alpha;
        off.scrollv.cursor_normal.data.color.a *= alpha;
        off.scrollv.cursor_hover.data.color.a *= alpha;
        off.scrollv.cursor_active.data.color.a *= alpha;
        off.scrollv.cursor_border_color.a *= alpha;

        off.tab.background.data.color.a *= alpha;
        off.tab.border_color.a *= alpha;
        off.tab.text.a *= alpha;

        off.combo.normal.data.color.a *= alpha;
        off.combo.hover.data.color.a *= alpha;
        off.combo.active.data.color.a *= alpha;
        off.combo.border_color.a *= alpha;
        off.combo.label_normal.a *= alpha;
        off.combo.label_hover.a *= alpha;
        off.combo.label_active.a *= alpha;
        off.combo.symbol_normal.a *= alpha;
        off.combo.symbol_hover.a *= alpha;
        off.combo.symbol_active.a *= alpha;
        off.combo.button.normal.data.color.a *= alpha;
        off.combo.button.hover.data.color.a *= alpha;
        off.combo.button.active.data.color.a *= alpha;
        off.combo.button.border_color.a *= alpha;
        off.combo.button.text_background.a *= alpha;
        off.combo.button.text_normal.a *= alpha;
        off.combo.button.text_hover.a *= alpha;
        off.combo.button.text_active.a *= alpha;

        off.window.fixed_background.data.color.a *= alpha;
        off.window.background.a *= alpha;
        off.window.border_color.a *= alpha;
        off.window.popup_border_color.a *= alpha;
        off.window.combo_border_color.a *= alpha;
        off.window.contextual_border_color.a *= alpha;
        off.window.menu_border_color.a *= alpha;
        off.window.group_border_color.a *= alpha;
        off.window.tooltip_border_color.a *= alpha;
        off.window.scaler.data.color.a *= alpha;
        off.window.header.normal.data.color.a *= alpha;
        off.window.header.hover.data.color.a *= alpha;
        off.window.header.active.data.color.a *= alpha;
    }
    static struct nk_input input;
    if (!enabled) {
        ui_alpha_push(0.5);
        ui_ctx->style = off; // .button = off.button; 
        input = ui_ctx->input;
        memset(&ui_ctx->input, 0, sizeof(ui_ctx->input));
    } else {
        ui_alpha_pop();
        ui_ctx->style = on; // .button = on.button;
        ui_ctx->input = input;
    }
    return enabled;
}

static int ui_is_enabled = 1;
int ui_enable() {
    return ui_is_enabled == 1 ? 0 : ui_set_enable_(ui_is_enabled = 1);
}
int ui_disable() {
    return ui_is_enabled == 0 ? 0 : ui_set_enable_(ui_is_enabled = 0);
}
int ui_enabled() {
    return ui_is_enabled;
}

static
void ui_destroy(void) {
    if(ui_ctx) {
        nk_glfw3_shutdown(&nk_glfw); // nk_sdl_shutdown();
        ui_ctx = 0;
    }
}
static
void ui_create() {
    do_once atexit(ui_destroy);

    if( ui_dirty ) {
        nk_glfw3_new_frame(&nk_glfw); //g->nk_glfw);
        ui_dirty = 0;

        ui_enable();
    }
}

enum {
    UI_NOTIFICATION_1 = 32, // sets panel as 1-story notification. used by ui_notify()
    UI_NOTIFICATION_2 = 64, // sets panel as 2-story notification. used by ui_notify()
};

struct ui_notify {
    char *title;
    char *body; // char *icon;
    float timeout;
    float alpha;
    int   used;
};

static array(struct ui_notify) ui_notifications; // format=("%d*%s\n%s", timeout, title, body)

static
void ui_notify_render() {
    // draw queued notifications
    if( array_count(ui_notifications) ) {
        struct ui_notify *n = array_back(ui_notifications);

        static double timeout = 0;
        timeout += 1/60.f; // window_delta(); // @fixme: use editor_time() instead

        ui_alpha_push( timeout >= n->timeout ? 1 - clampf(timeout - n->timeout,0,1) : 1 );

            if( timeout < (n->timeout + 1) ) { // N secs display + 1s fadeout
                if(n->used++ < 3) nk_window_set_focus(ui_ctx, "!notify");

                if( ui_panel( "!notify", n->title && n->body ? UI_NOTIFICATION_2 : UI_NOTIFICATION_1 ) ) {
                    if(n->title) ui_label(n->title);
                    if(n->body)  ui_label(n->body);

                    ui_panel_end();
                }
            }

            if( timeout >= (n->timeout + 2) ) { // 1s fadeout + 1s idle
                timeout = 0;

                if(n->title) FREE(n->title);
                if(n->body)  FREE(n->body);
                array_pop(ui_notifications);
            }

        ui_alpha_pop();
    }
}

static
void ui_hue_cycle( unsigned num_cycles ) {
    // cycle color (phi ratio)
    for( unsigned i = 0; i < num_cycles; ++i ) {
        //ui_hue = (ui_hue+0.61803f)*1.61803f; while(ui_hue > 1) ui_hue -= 1;
        ui_hue *= 1.61803f / 1.85f; while(ui_hue > 1) ui_hue -= 1;
    }
}

static
void ui_render() {

    // draw queued menus
    ui_notify_render();
    ui_menu_render();

    /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state after
     * rendering the UI. */
    //nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

    GLfloat bkColor[4]; glGetFloatv(GL_COLOR_CLEAR_VALUE, bkColor); // @transparent
    glClearColor(0,0,0,1); // @transparent
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,!bkColor[3] ? GL_TRUE : GL_FALSE);  // @transparent
    nk_glfw3_render(&nk_glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);  // @transparent

#if is(ems)
    glFinish();
#endif

    ui_dirty = 1;
    ui_hue = 0;

    ui_is_hover = nk_window_is_any_hovered(ui_ctx) && window_has_cursor();

    if(input_down(MOUSE_L))
        ui_is_active = (ui_is_hover && nk_item_is_any_active(ui_ctx));
    if(input_up(MOUSE_L))
        ui_is_active = 0;
}


// -----------------------------------------------------------------------------
// save/restore all window layouts on every framebuffer resize

#define UI_SNAP_PX      1 /*treshold of pixels when snapping panels/windows to the application borders [1..N]*/
#define UI_ANIM_ALPHA 0.9 /*animation alpha used when restoring panels/windows state from application resizing events: [0..1]*/
//#define UI_MENUBAR_Y     32 // menubar and row

typedef struct ui_layout {
    const char *title;

    bool is_panel;

    vec2   desktop;
    vec2     p0,p1;
    float    l0,l1;

    float alpha;
    float anim_timer;

} ui_layout;

static array(ui_layout) ui_layouts[2] = {0};

static
int ui_layout_find(const char *title, bool is_panel) {
    int i = 0;
    for each_array_ptr(ui_layouts[is_panel], ui_layout, s) {
        if( !strcmp(title, s->title) ) return i;
        ++i;
    }
    ui_layout s = {0};
    s.is_panel = is_panel;
    s.title = STRDUP(title);
    array_push(ui_layouts[is_panel], s);
    return array_count(ui_layouts[is_panel]) - 1;
}

static
void ui_layout_save_mem(int idx, vec2 desktop, float workarea_h, struct nk_rect *xywh_, bool is_panel) {
    struct nk_rect xywh = *xywh_; //< workaround for a (tcc-0.9.27+lubuntu16) bug, where xywh_ is never populated (ie, empty always) when passed by-copy

    ui_layout *s = &ui_layouts[is_panel][idx];

    struct nk_window *win = nk_window_find(ui_ctx, s->title);
    // if(win && win->flags & NK_WINDOW_FULLSCREEN) return;    // skip if maximized

    s->desktop = desktop;

float excess = 0;
if( win && (win->flags & NK_WINDOW_MINIMIZED)) {
    excess = xywh.h - UI_MENUROW_HEIGHT;
    xywh.h = UI_MENUROW_HEIGHT;
}

    // sanity checks
    if(xywh.x<0)                               xywh.x = 0;
    if(xywh.w>desktop.w-UI_SNAP_PX)            xywh.w = desktop.w-UI_SNAP_PX-1;

    if(xywh.y<workarea_h)                      xywh.y = workarea_h;
    if(xywh.h>desktop.h-workarea_h-UI_SNAP_PX) xywh.h = desktop.h-workarea_h-UI_SNAP_PX-1;

    if((xywh.x+xywh.w)>desktop.w)              xywh.x-= xywh.x+xywh.w-desktop.w;
    if((xywh.y+xywh.h)>desktop.h)              xywh.y-= xywh.y+xywh.h-desktop.h;

if( win && (win->flags & NK_WINDOW_MINIMIZED)) {
    xywh.h += excess;
}

    // build reconstruction vectors from bottom-right corner
    s->p0 = vec2(xywh.x/s->desktop.x,xywh.y/s->desktop.y);
    s->p1 = vec2(xywh.w/s->desktop.x,xywh.h/s->desktop.y);
    s->p0 = sub2(s->p0, vec2(1,1)); s->l0 = len2(s->p0);
    s->p1 = sub2(s->p1, vec2(1,1)); s->l1 = len2(s->p1);
}

static
struct nk_rect ui_layout_load_mem(int idx, vec2 desktop, bool is_panel) {
    ui_layout *s = &ui_layouts[is_panel][idx];

    // extract reconstruction coords from bottom-right corner
    vec2 p0 = mul2(add2(vec2(1,1), scale2(norm2(s->p0), s->l0)), desktop);
    vec2 p1 = mul2(add2(vec2(1,1), scale2(norm2(s->p1), s->l1)), desktop);

    return nk_rect( p0.x, p0.y, p1.x, p1.y );
}

static
int ui_layout_all_reset(const char *mask) {
    ui_layout z = {0};

    vec2 desktop = vec2(window_width(), window_height());
    float workarea_h = ui_has_menubar()*UI_MENUROW_HEIGHT; // @fixme workarea -> reserved_area

    for( int is_panel = 0; is_panel < 2; ++is_panel ) {
        for( int j = 0; j < array_count(ui_layouts[is_panel]); ++j ) {
            if( ui_layouts[is_panel][j].title ) {

                if( nk_window_is_hidden(ui_ctx, ui_layouts[is_panel][j].title) ) continue;

                struct nk_rect xywh = { 0, workarea_h + j * UI_MENUROW_HEIGHT, desktop.w / 3.333, UI_MENUROW_HEIGHT };
                if( is_panel ) {
                    xywh.x = 0;
                    xywh.y = workarea_h + j * UI_MENUROW_HEIGHT;
                    xywh.w = desktop.w / 4;
                    xywh.h = desktop.h / 3;
                } else {
                    xywh.x = desktop.w / 3.00 + j * UI_MENUROW_HEIGHT;
                    xywh.y = workarea_h + j * UI_MENUROW_HEIGHT;
                    xywh.w = desktop.w / 4;
                    xywh.h = desktop.h / 3;
                }
                nk_window_set_focus(ui_ctx, ui_layouts[is_panel][j].title);
                nk_window_collapse(ui_ctx, ui_layouts[is_panel][j].title, is_panel ? 0 : 1);
                struct nk_window* win = is_panel ? 0 : nk_window_find(ui_ctx, ui_layouts[is_panel][j].title );
                if(win) win->flags &= ~NK_WINDOW_FULLSCREEN;
                if(win) win->flags &= ~NK_WINDOW_MINIMIZED;
                ui_layout_save_mem(j, desktop, workarea_h, &xywh, is_panel);
                ui_layouts[is_panel][j].anim_timer = 1.0;
            }
        }
    }

    return 1;
}

static
int ui_layout_all_save_disk(const char *mask) {
    float w = window_width(), h = window_height();
    for each_map_ptr_sorted(ui_windows, char*, k, unsigned, v) {
        struct nk_window *win = nk_window_find(ui_ctx, *k);
        if( win && strmatchi(*k, mask) ) {
            ini_write(WINDOWS_INI, *k, "x", va("%f", win->bounds.x / w ));
            ini_write(WINDOWS_INI, *k, "y", va("%f", win->bounds.y / h ));
            ini_write(WINDOWS_INI, *k, "w", va("%f", win->bounds.w / w ));
            ini_write(WINDOWS_INI, *k, "h", va("%f", win->bounds.h / h ));
            ini_write(WINDOWS_INI, *k, "visible", ui_visible(*k) ? "1":"0");
        }
    }
    return 1;
}

static
const char *ui_layout_load_disk(const char *title, const char *mask, ini_t i, struct nk_rect *r) {
    if(!i) return 0;

    const char *dot = strrchr(title, '.');
    if( dot ) title = va("%.*s", (int)(dot - title), title);
    if( !strmatchi(title, mask) ) return 0;

    char **x = map_find(i, va("%s.x", title));
    char **y = map_find(i, va("%s.y", title));
    char **w = map_find(i, va("%s.w", title));
    char **h = map_find(i, va("%s.h", title));
    if( x && y && w && h ) {
        float ww = window_width(), wh = window_height();
        r->x = atof(*x) * ww;
        r->y = atof(*y) * wh;
        r->w = atof(*w) * ww;
        r->h = atof(*h) * wh;

        char **on = map_find(i, va("%s.visible", title));

        return title;
    }
    return 0;
}

static
int ui_layout_all_load_disk(const char *mask) {
    ini_t i = ini(WINDOWS_INI); // @leak
    if( !i ) return 0;
    for each_map(i, char*, k, char*, v) {
        struct nk_rect out = {0};
        const char *title = ui_layout_load_disk(k, mask, i, &out);
        if( title ) {
            struct nk_window *win = nk_window_find(ui_ctx, title);
            if( win ) {            
                win->bounds.x = out.x;
                win->bounds.y = out.y;
                win->bounds.w = out.w;
                win->bounds.h = out.h;
            }
        }
    }
    return 1;
}


// -----------------------------------------------------------------------------
// shared code for both panels and windows. really messy.

static
int ui_begin_panel_or_window_(const char *title, int flags, bool is_window) {

struct nk_window *win = nk_window_find(ui_ctx, title);

int is_panel = !is_window;
bool starts_minimized = is_panel ? !(flags & PANEL_OPEN) : 0;
bool is_closable = is_window;
bool is_scalable = true;
bool is_movable = true;
bool is_auto_minimizes = starts_minimized; // false;
bool is_pinned = win && (win->flags & NK_WINDOW_PINNED);

if( is_pinned ) {
//    is_closable = false;
    is_auto_minimizes = false;
    is_scalable = false;
//    is_movable = false;
}

    ui_create();

    uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
    for(int i = 0; title[i]; ++i) hash = (hash ^ title[i]) * mult;
    ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

    int idx = ui_layout_find(title, is_panel);
    ui_layout *s = &ui_layouts[is_panel][idx];

vec2 desktop = vec2(window_width(), window_height());
float workarea_h = ui_has_menubar()*UI_MENUROW_HEIGHT;

    int row = idx + !!ui_has_menubar(); // add 1 to skip menu
    vec2 offset = vec2(0, UI_ROW_HEIGHT*row);
    float w = desktop.w / 3.33, h = (flags & UI_NOTIFICATION_2 ? UI_MENUROW_HEIGHT*2 : (flags & UI_NOTIFICATION_1 ? UI_MENUROW_HEIGHT : desktop.h - offset.y * 2 - 1)); // h = desktop.h * 0.66; //
    struct nk_rect start_coords = {offset.x, offset.y, offset.x+w, offset.y+h};

if(is_window) {
    w = desktop.w / 1.5;
    h = desktop.h / 1.5;
    start_coords.x = (desktop.w-w)/2;
    start_coords.y = (desktop.h-h)/2 + workarea_h/2;
    start_coords.w = w;
    start_coords.h = h;
}

    static vec2 edge = {0}; static int edge_type = 0; // [off],L,R,U,D
    do_once edge = vec2(desktop.w * 0.33, desktop.h * 0.66);

// do not snap windows and/or save windows when using may be interacting with UI
int is_active = 0;
int mouse_pressed = !!input(MOUSE_L) && ui_ctx->active == win;
if( win ) {
    // update global window activity bitmask
    is_active = ui_ctx->active == win;
    ui_active_mask = is_active ? ui_active_mask | (1ull << idx) : ui_active_mask & ~(1ull << idx);
}

//  struct nk_style *s = &ui_ctx->style;
//  nk_style_push_color(ui_ctx, &s->window.header.normal.data.color, nk_hsv_f(ui_hue,0.6,0.8));

// adjust inactive edges automatically
if( win ) {
    bool group1_any             = !is_active; // && !input(MOUSE_L);
    bool group2_not_resizing    =  is_active && !win->is_window_resizing;
    bool group2_interacting     =  is_active && input(MOUSE_L);

#if 0
    if( group1_any ) {
        // cancel self-adjust if this window is not overlapping the active one that is being resized at the moment
        struct nk_window *parent = ui_ctx->active;

        struct nk_rect a = win->bounds, b = parent->bounds;
        bool overlap = a.x <= (b.x+b.w) && b.x <= (a.x+a.w) && a.y <= (b.y+b.h) && b.y <= (a.y+a.h);

        group1_any = overlap;
    }
#else
    if( group1_any )
        group1_any = !(win->flags & NK_WINDOW_PINNED);
#endif

    if( group1_any ) {
        float mouse_x = clampf(input(MOUSE_X), 0, desktop.w);
        float mouse_y = clampf(input(MOUSE_Y), 0, desktop.h);
        float distance_x = absf(mouse_x - win->bounds.x) / desktop.w;
        float distance_y = absf(mouse_y - win->bounds.y) / desktop.h;
        float alpha_x = sqrt(sqrt(distance_x)); // amplify signals a little bit: 0.1->0.56,0.5->0.84,0.98->0.99,etc
        float alpha_y = sqrt(sqrt(distance_y));

        /**/ if( (edge_type & 1) && win->bounds.x <= UI_SNAP_PX ) {
            win->bounds.w = win->bounds.w * alpha_y + edge.w * (1-alpha_y);
        }
        else if( (edge_type & 2) && (win->bounds.x + win->bounds.w) >= (desktop.w-UI_SNAP_PX) ) {
            win->bounds.w = win->bounds.w * alpha_y + edge.w * (1-alpha_y);
            win->bounds.x = desktop.w - win->bounds.w;
        }
        if( (edge_type & 8) && (win->bounds.y + (win->flags & NK_WINDOW_MINIMIZED ? UI_ROW_HEIGHT : win->bounds.h)) >= (desktop.h-UI_SNAP_PX) ) {
            win->bounds.h = win->bounds.h * alpha_x + edge.h * (1-alpha_x);
            win->bounds.y = desktop.h - (win->flags & NK_WINDOW_MINIMIZED ? UI_ROW_HEIGHT : win->bounds.h);
        }
    }

    // skip any saving if window is animating (moving) and/or maximized
    bool anim_in_progress = s->anim_timer > 1e-3;
    s->anim_timer *= anim_in_progress * UI_ANIM_ALPHA;

    if( group1_any || !group2_interacting || anim_in_progress ) {
        struct nk_rect target = ui_layout_load_mem(idx, desktop, is_panel);
        float alpha = len2sq(sub2(s->desktop, desktop)) ? 0 : UI_ANIM_ALPHA; // smooth unless we're restoring a desktop change 
#if 1
        if( is_window && win->flags & NK_WINDOW_FULLSCREEN ) {
            target.x = 1;
            target.w = desktop.w - 1;
            target.y = workarea_h + 1;
            target.h = desktop.h - workarea_h - 2;
        }
        if( is_window && win->is_window_restoring > 1e-2) {
            win->is_window_restoring = win->is_window_restoring * alpha + 0 * (1 - alpha);
            target.w = desktop.w / 2;
            target.h = (desktop.h - workarea_h) / 2;
            target.x = (desktop.w - target.w) / 2;
            target.y = ((desktop.h - workarea_h) - target.h) / 2;
        }
#endif
        win->bounds = nk_rect( 
            win->bounds.x * alpha + target.x * (1 - alpha),
            win->bounds.y * alpha + target.y * (1 - alpha),
            win->bounds.w * alpha + target.w * (1 - alpha),
            win->bounds.h * alpha + target.h * (1 - alpha)
        );
    }
    if(!anim_in_progress)
    ui_layout_save_mem(idx, desktop, workarea_h, &win->bounds, is_panel);
} else { // if(!win)
    ui_layout_save_mem(idx, desktop, workarea_h, &start_coords, is_panel);
}


    int window_flags = NK_WINDOW_PINNABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_NO_SCROLLBAR_X | (is_window ? NK_WINDOW_MAXIMIZABLE : 0);
    if( starts_minimized ) window_flags |= (win ? 0 : NK_WINDOW_MINIMIZED);
    if( is_auto_minimizes ) window_flags |= is_active ? 0 : !!starts_minimized * NK_WINDOW_MINIMIZED;
    if( is_movable )  window_flags |= NK_WINDOW_MOVABLE;
    if( is_closable ) window_flags |= NK_WINDOW_CLOSABLE;
    if( is_scalable ) {
        window_flags |= NK_WINDOW_SCALABLE;
        if(win) window_flags |= input(MOUSE_X) < (win->bounds.x + win->bounds.w/2) ? NK_WINDOW_SCALE_LEFT : 0;
        if(win) window_flags |= input(MOUSE_Y) < (win->bounds.y + win->bounds.h/2) ? NK_WINDOW_SCALE_TOP : 0;
    }

//    if( is_pinned )
        window_flags |= NK_WINDOW_BORDER;

if( is_panel && win && !is_active ) {
    if( !is_pinned && is_auto_minimizes ) {
        window_flags |= NK_WINDOW_MINIMIZED;
    }
}

// if( is_modal ) window_flags &= ~(NK_WINDOW_MINIMIZED | NK_WINDOW_MINIMIZABLE);
if( is_panel && win ) {
//    if( win->bounds.x > 0 && (win->bounds.x+win->bounds.w) < desktop.w-1 ) window_flags &= ~NK_WINDOW_MINIMIZED;
}

if(!win) { // if newly created window (!win)
    // first time, try to restore from WINDOWS_INI file
    static ini_t i; do_once i = ini(WINDOWS_INI); // @leak
    ui_layout_load_disk(title, "*", i, &start_coords);
    ui_layout_save_mem(idx, desktop, workarea_h, &start_coords, is_panel);
}

bool is_notify = flags & (UI_NOTIFICATION_1 | UI_NOTIFICATION_2);
if( is_notify ) {
    window_flags = NK_WINDOW_MOVABLE | NK_WINDOW_NOT_INTERACTIVE | NK_WINDOW_NO_SCROLLBAR;
    start_coords = nk_rect(desktop.w / 2 - w / 2, -h, w, h);
}

    if( nk_begin(ui_ctx, title, start_coords, window_flags) ) {

// set width for all inactive panels
struct nk_rect bounds = nk_window_get_bounds(ui_ctx); 
if( mouse_pressed && win && win->is_window_resizing ) {
    edge = vec2(bounds.w, bounds.h);

    // push direction
    int top  = !!(win->is_window_resizing & NK_WINDOW_SCALE_TOP);
    int left = !!(win->is_window_resizing & NK_WINDOW_SCALE_LEFT), right = !left;

    edge_type = 0;
    /**/ if( right && (win->bounds.x <= UI_SNAP_PX) ) edge_type |= 1;
    else if(  left && (win->bounds.x + win->bounds.w) >= (desktop.w-UI_SNAP_PX) ) edge_type |= 2;
    /**/ if(   top && (win->bounds.y + win->bounds.h) >= (desktop.h-UI_SNAP_PX) ) edge_type |= 8;

    // @fixme
    // - if window is in a corner (sharing 2 edges), do not allow for multi edges. either vert or horiz depending on the clicked scaler
    // - or maybe, only propagate edge changes to the other windows that overlapping our window.
}

        return 1;
    } else {

if(is_panel) {
   ui_panel_end(); 
} else ui_window_end();

        return 0;
    }
}

static const char *ui_last_title = 0;
static int *ui_last_enabled = 0;
static int ui_has_window = 0;
static int ui_window_has_menubar = 0;
int ui_window(const char *title, int *enabled) {
    if( window_width() <= 0 ) return 0;
    if( window_height() <= 0 ) return 0;
    if( !ui_ctx || !ui_ctx->style.font ) return 0;

    bool forced_creation = enabled && *enabled; // ( enabled ? *enabled : !ui_has_menubar() );
    forced_creation |= ui_window_register(title);
    if(!ui_visible(title)) {
        if( !forced_creation ) return 0;
        ui_show(title, forced_creation);
    }

    ui_last_enabled = enabled;
    ui_last_title = title;
    ui_has_window = 1;
    return ui_begin_panel_or_window_(title, /*flags*/0, true);
}
int ui_window_end() {
    if(ui_window_has_menubar) nk_menubar_end(ui_ctx), ui_window_has_menubar = 0;
    nk_end(ui_ctx), ui_has_window = 0;

    int closed = 0;
    if( nk_window_is_hidden(ui_ctx, ui_last_title) ) {
        nk_window_close(ui_ctx, ui_last_title);
        ui_show(ui_last_title, false);
        if( ui_last_enabled ) *ui_last_enabled = 0; // clear developers' flag
        closed = 1;
    }

    // @transparent
    #if !is(ems)
    static bool has_transparent_attrib = 0; do_once has_transparent_attrib = glfwGetWindowAttrib(window_handle(), GLFW_TRANSPARENT_FRAMEBUFFER) == GLFW_TRUE;
    if( closed && has_transparent_attrib && !ui_has_menubar() ) {
        bool any_open = 0;
        for each_map_ptr(ui_windows, char*, k, unsigned, v) any_open |= *v & 1;
        if( !any_open ) glfwSetWindowShouldClose(window_handle(), GLFW_TRUE);
    }
    #endif
    // @transparent

    return 0;
}

int ui_panel(const char *title, int flags) {
    if( window_width() <= 0 ) return 0;
    if( window_height() <= 0 ) return 0;
    if( !ui_ctx || !ui_ctx->style.font ) return 0;

    if( ui_has_window ) {
        // transparent style
        static struct nk_style_button transparent_style;
        do_once transparent_style = ui_ctx->style.button;
        do_once transparent_style.normal.data.color = nk_rgba(0,0,0,0);
        do_once transparent_style.border_color = nk_rgba(0,0,0,0);
        do_once transparent_style.active = transparent_style.normal;
        do_once transparent_style.hover = transparent_style.normal;
        do_once transparent_style.hover.data.color = nk_rgba(0,0,0,127);
        transparent_style.text_alignment = NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE;

        if(!ui_window_has_menubar) nk_menubar_begin(ui_ctx);
        if(!ui_window_has_menubar) nk_layout_row_begin(ui_ctx, NK_STATIC, UI_MENUBAR_ICON_HEIGHT, 4);
        if(!ui_window_has_menubar) nk_layout_row_push(ui_ctx, 70);
        ui_window_has_menubar = 1;

        return nk_menu_begin_text_styled(ui_ctx, title, strlen(title), NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE, nk_vec2(220, 200), &transparent_style);    
    }

    return ui_begin_panel_or_window_(title, flags, false);
}
int ui_panel_end() {
    if( ui_has_window ) {
        nk_menu_end(ui_ctx);
        return 0;
    }
    nk_end(ui_ctx);
//  nk_style_pop_color(ui_ctx);
    return 0;
}

static unsigned ui_collapse_state = 0;
int ui_collapse(const char *label, const char *id) { // mask: 0(closed),1(open),2(created)

    uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
    for(int i = 0; id[i]; ++i) hash = (hash ^ id[i]) * mult;
    ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

    ui_collapse_state = nk_tree_base_(ui_ctx, NK_TREE_NODE, 0, label, NK_MINIMIZED, id, strlen(id), 0);
    return ui_collapse_state & 1; // |1 open, |2 clicked, |4 toggled
}
int ui_collapse_clicked() {
    return ui_collapse_state >> 1; // |1 clicked, |2 toggled
}
int ui_collapse_end() {
    return nk_tree_pop(ui_ctx), 1;
}


int ui_contextual() {
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);
    bounds.y -= 25;
    return ui_popups() ? 0 : nk_contextual_begin(ui_ctx, 0, nk_vec2(150, 300), bounds);
}
int ui_contextual_end() {
    nk_contextual_end(ui_ctx);
    return 1;
}
int ui_submenu(const char *options) {
    int choice = 0;
    if( ui_contextual() ) {
        array(char*) tokens = strsplit(options, ";");
        for( int i = 0; i < array_count(tokens) ; ++i ) {
            if( ui_button_transparent(tokens[i]) ) choice = i + 1;
        }
        ui_contextual_end();
    }
    return choice;
}

// -----------------------------------------------------------------------------
// code for all the widgets

static
int nk_button_transparent(struct nk_context *ctx, const char *text) {
    static struct nk_style_button transparent_style;
    do_once transparent_style = ctx->style.button;
    do_once transparent_style.text_alignment = NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE;
    do_once transparent_style.normal.data.color = nk_rgba(0,0,0,0);
    do_once transparent_style.border_color = nk_rgba(0,0,0,0);
    do_once transparent_style.active = transparent_style.normal;
    do_once transparent_style.hover = transparent_style.normal;
    do_once transparent_style.hover.data.color = nk_rgba(0,0,0,127);
    transparent_style.text_background.a = 255 * ui_alpha;
    transparent_style.text_normal.a = 255 * ui_alpha;
    transparent_style.text_hover.a = 255 * ui_alpha;
    transparent_style.text_active.a = 255 * ui_alpha;
    return nk_button_label_styled(ctx, &transparent_style, text);
}

// internal vars for our editor. @todo: maybe expose these to the end-user as well?
bool ui_label_icon_highlight;
vec2 ui_label_icon_clicked_L; // left
vec2 ui_label_icon_clicked_R; // right

static
int ui_label_(const char *label, int alignment) {
    // beware: assuming label can start with any ICON_MD_ glyph, which I consider them to be a 3-bytes utf8 sequence.
    // done for optimization reasons because this codepath is called a lot!
    const char *icon = label ? label : ""; while( icon[0] == '!' || icon[0] == '*' ) ++icon;
    int has_icon = (unsigned)icon[0] > 127, icon_len = 3, icon_width_px = 1*24;

    struct nk_rect bounds = nk_widget_bounds(ui_ctx);
    const struct nk_input *input = &ui_ctx->input;
    int is_hovering = nk_input_is_mouse_hovering_rect(input, bounds) && !ui_has_active_popups;
    if( is_hovering ) {
        struct nk_rect winbounds = nk_window_get_bounds(ui_ctx);
        is_hovering &= nk_input_is_mouse_hovering_rect(input, winbounds);

        struct nk_window *win = ui_ctx->current;
        bool has_contextual = !win->name; // contextual windows are annonymous

        is_hovering &= has_contextual || nk_window_has_focus(ui_ctx);
    }

    int skip_color_tab = label && label[0] == '!';
    if( skip_color_tab) label++;

    int spacing = 8; // between left colorbar and content
    struct nk_window *win = ui_ctx->current;
    struct nk_panel *layout = win->layout;
    layout->at_x += spacing;
    layout->bounds.w -= spacing;
    if( !skip_color_tab ) {
        float w = is_hovering ? 4 : 2; // spacing*3/4 : spacing/2-1;
        bounds.w = w;
        bounds.h -= 1;
        struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
        nk_fill_rect(canvas, bounds, 0, nk_hsva_f(ui_hue, 0.75f, 0.8f, ui_alpha) );
    }

    if(!label || !label[0]) {
        nk_label(ui_ctx, "", alignment);
        layout->at_x -= spacing;
        layout->bounds.w += spacing;
        return 0;
    }

        const char *split = strchr(label, '@');
            char buffer[128]; if( split ) label = (snprintf(buffer, 128, "%.*s", (int)(split-label), label), buffer);

struct nk_style *style = &ui_ctx->style;
bool bold = label[0] == '*'; label += bold;
struct nk_font *font = bold && nk_glfw.atlas.fonts->next ? nk_glfw.atlas.fonts->next->next /*3rd font*/ : NULL; // list

if( !has_icon ) {
    // set bold style and color if needed
    if( font && nk_style_push_font(ui_ctx, &font->handle) ) {} else font = 0;
    if( font )  nk_style_push_color(ui_ctx, &style->text.color, nk_rgba(255, 255, 255, 255 * ui_alpha));
    nk_label(ui_ctx, label, alignment);
} else {
    char *icon_glyph = va("%.*s", icon_len, icon);

// @todo: implement nk_push_layout() 
//  nk_rect bounds = {..}; nk_panel_alloc_space(bounds, ctx);
    struct nk_window *win = ui_ctx->current;
    struct nk_panel *layout = win->layout, copy = *layout;
    struct nk_rect before; nk_layout_peek(&before, ui_ctx);
    nk_label_colored(ui_ctx, icon_glyph, alignment, nk_rgba(255, 255, 255, (64 + 192 * ui_label_icon_highlight) * ui_alpha) );
    struct nk_rect after; nk_layout_peek(&after, ui_ctx);
    *layout = copy;
    layout->at_x += icon_width_px; layout->bounds.w -= icon_width_px; // nk_layout_space_push(ui_ctx, nk_rect(0,0,icon_width_px,0));

    // set bold style and color if needed
    if( font && nk_style_push_font(ui_ctx, &font->handle) ) {} else font = 0;
    if( font )  nk_style_push_color(ui_ctx, &style->text.color, nk_rgba(255, 255, 255, 255 * ui_alpha));
    nk_label(ui_ctx, icon+icon_len, alignment);

    layout->at_x -= icon_width_px; layout->bounds.w += icon_width_px;
}

if( font )  nk_style_pop_color(ui_ctx);
if( font )  nk_style_pop_font(ui_ctx);

            if (split && is_hovering && !ui_has_active_popups && nk_window_has_focus(ui_ctx)) {
                nk_tooltip(ui_ctx, split + 1);
            }

    layout->at_x -= spacing;
    layout->bounds.w += spacing;

    // old way
    // ui_labeicon_l_icked_L.x = is_hovering ? nk_input_has_mouse_click_down_in_rect(input, NK_BUTTON_LEFT, layout->bounds, nk_true) : 0;
    // new way
    // this is an ugly hack to detect which icon (within a label) we're clicking on. 
    // @todo: figure out a better way to detect this... would it be better to have a ui_label_toolbar(lbl,bar) helper function instead?
    ui_label_icon_clicked_L.x = is_hovering ? ( (int)((input->mouse.pos.x - bounds.x) - (alignment == NK_TEXT_RIGHT ? bounds.w : 0) ) * nk_input_is_mouse_released(input, NK_BUTTON_LEFT)) : 0;

    return ui_label_icon_clicked_L.x;
}

int ui_label(const char *text) {
    int align = text[0] == '>' ? (text++, NK_TEXT_RIGHT) : text[0] == '=' ? (text++, NK_TEXT_CENTERED) : text[0] == '<' ? (text++, NK_TEXT_LEFT) : NK_TEXT_LEFT;
    nk_layout_row_dynamic(ui_ctx, 0, 1);
    return ui_label_(text, align);
}
int ui_label2(const char *label, const char *text_) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);

    int align1 = label[0] == '>' ? (label++, NK_TEXT_RIGHT) : label[0] == '=' ? (label++, NK_TEXT_CENTERED) : label[0] == '<' ? (label++, NK_TEXT_LEFT) : NK_TEXT_LEFT;
    int align2 = text_[0] == '>' ? (text_++, NK_TEXT_RIGHT) : text_[0] == '=' ? (text_++, NK_TEXT_CENTERED) : text_[0] == '<' ? (text_++, NK_TEXT_LEFT) : NK_TEXT_LEFT;
    ui_label_(label, align1);

const struct nk_input *input = &ui_ctx->input;
struct nk_rect bounds = nk_widget_bounds(ui_ctx);
int is_hovering = nk_input_is_mouse_hovering_rect(input, bounds) && !ui_has_active_popups;
if( is_hovering ) {
    struct nk_rect winbounds = nk_window_get_bounds(ui_ctx);
    is_hovering &= nk_input_is_mouse_hovering_rect(input, winbounds);
    is_hovering &= nk_window_has_focus(ui_ctx);
}

    nk_label(ui_ctx, text_, align2);

// this is an ugly hack to detect which icon (within a label) we're clicking on. 
// @todo: figure out a better way to detect this... would it be better to have a ui_label_toolbar(lbl,bar) helper function instead?
ui_label_icon_clicked_R.x = is_hovering ? ( (int)((input->mouse.pos.x - bounds.x) - (align2 == NK_TEXT_RIGHT ? bounds.w : 0) ) * nk_input_is_mouse_released(input, NK_BUTTON_LEFT)) : 0;

    return ui_label_icon_clicked_R.x;
}
int ui_label2_bool(const char *text, bool value) {
    bool b = !!value;
    return ui_bool(text, &b), 0;
}
int ui_label2_float(const char *text, float value) {
    float f = (float)value;
    return ui_float(text, &f), 0;
}
int ui_label2_wrap(const char *label, const char *str) { // @fixme: does not work (remove dynamic layout?)
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);
    nk_text_wrap(ui_ctx, str, strlen(str));
    return 0;
}
int ui_label2_toolbar(const char *label, const char *icons) {
    int mouse_click = ui_label2(label, va(">%s", icons));
    int choice = !mouse_click ? 0 : 1 + -mouse_click / (UI_ICON_FONTSIZE + UI_ICON_SPACING_X); // divided by px per ICON_MD_ glyph approximately
    int glyphs = strlen(icons) / 3;
    return choice > glyphs ? 0 : choice;
}

int ui_notify(const char *title, const char *body) {
    struct ui_notify n = {0};
    n.title = title && title[0] ? stringf("*%s", title) : 0;
    n.body = body && body[0] ? STRDUP(body) : 0;
    n.timeout = 2; // 4s = 2s timeout (+ 1s fade + 1s idle)
    n.alpha = 1;
    array_push_front(ui_notifications, n);
    return 1;
}

int ui_button_transparent(const char *text) {
    nk_layout_row_dynamic(ui_ctx, 0, 1);
    int align = text[0] == '>' ? (text++, NK_TEXT_RIGHT) : text[0] == '=' ? (text++, NK_TEXT_CENTERED) : text[0] == '<' ? (text++, NK_TEXT_LEFT) : NK_TEXT_CENTERED;
    return !!nk_contextual_item_label(ui_ctx, text, align);
}

#ifndef UI_BUTTON_MONOCHROME
#define UI_BUTTON_MONOCHROME 0
#endif

static
int ui_button_(const char *text) {
    if( 1 ) {
#if UI_BUTTON_MONOCHROME
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_normal, nk_rgba(0,0,0,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_hover,  nk_rgba(0,0,0,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_active, nk_rgba(0,0,0,ui_alpha));

        nk_style_push_color(ui_ctx, &ui_ctx->style.button.normal.data.color, nk_hsva_f(ui_hue,0.0,0.8*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.hover.data.color,  nk_hsva_f(ui_hue,0.0,1.0*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.active.data.color, nk_hsva_f(ui_hue,0.0,0.4*ui_alpha));
#elif 0 // old
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_normal, nk_rgba(0,0,0,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_hover,  nk_rgba(0,0,0,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_active, nk_rgba(0,0,0,ui_alpha));

        nk_style_push_color(ui_ctx, &ui_ctx->style.button.normal.data.color, nk_hsva_f(ui_hue,0.75,0.8*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.hover.data.color,  nk_hsva_f(ui_hue,1.00,1.0*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.active.data.color, nk_hsva_f(ui_hue,0.60,0.4*ui_alpha));
#else // new
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_normal, nk_rgba_f(0.00,0.00,0.00,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_hover,  nk_rgba_f(0.11,0.11,0.11,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_active, nk_rgba_f(0.00,0.00,0.00,ui_alpha));

        nk_style_push_color(ui_ctx, &ui_ctx->style.button.normal.data.color, nk_hsva_f(ui_hue,0.80,0.6,0.90*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.hover.data.color,  nk_hsva_f(ui_hue,0.85,0.9,0.90*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.active.data.color, nk_hsva_f(ui_hue,0.80,0.6,0.90*ui_alpha));
#endif
    }

    struct nk_rect bounds = nk_widget_bounds(ui_ctx);

    const char *split = strchr(text, '@'), *tooltip = split + 1;
    int ret = nk_button_text(ui_ctx, text, split ? (int)(split - text) : strlen(text) );

    const struct nk_input *in = &ui_ctx->input;
    if (split && nk_input_is_mouse_hovering_rect(in, bounds) && !ui_has_active_popups && nk_window_has_focus(ui_ctx)) {
        nk_tooltip(ui_ctx, tooltip);
    }

    if( 1 ) {
        nk_style_pop_color(ui_ctx);
        nk_style_pop_color(ui_ctx);
        nk_style_pop_color(ui_ctx);

        nk_style_pop_color(ui_ctx);
        nk_style_pop_color(ui_ctx);
        nk_style_pop_color(ui_ctx);
    }

    return ret;
}

int ui_buttons(int buttons, ...) {
    nk_layout_row_dynamic(ui_ctx, 0, buttons);

    float ui_hue_old = ui_hue;

        int indent = 8;
        struct nk_window *win = ui_ctx->current;
        struct nk_panel *layout = win->layout;
        struct nk_panel copy = *layout;
        ui_label_("", NK_TEXT_LEFT);
        *layout = copy;
        layout->at_x += indent;
        layout->bounds.w -= indent;

            int rc = 0;
            va_list list;
            va_start(list, buttons);
            for( int i = 0; i < buttons; ++i ) {
                if( ui_button_( va_arg(list, const char*) ) ) rc = i+1;
                ui_hue_cycle( 3 );
            }
            va_end(list);

        layout->at_x -= indent;
        layout->bounds.w += indent;

    ui_hue = ui_hue_old;
    return rc;
}

int ui_button(const char *s) {
    return ui_buttons(1, s);
}

int ui_toggle(const char *label, bool *value) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    // nk_label(ui_ctx, label, alignment);
    int rc = nk_button_transparent(ui_ctx, *value ? ICON_MD_TOGGLE_ON : ICON_MD_TOGGLE_OFF);
    return rc ? (*value ^= 1), rc : rc;
}

int ui_color4f(const char *label, float *color4) {
    float c[4] = { color4[0]*255, color4[1]*255, color4[2]*255, color4[3]*255 };
    int ret = ui_color4(label, c);
    for( int i = 0; i < 4; ++i ) color4[i] = c[i] / 255.0f;
    return ret;
}

static enum color_mode {COL_RGB, COL_HSV} ui_color_mode = COL_RGB;

int ui_color4(const char *label, float *color4) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_colorf after = { color4[0]*ui_alpha/255, color4[1]*ui_alpha/255, color4[2]*ui_alpha/255, color4[3]/255 }, before = after;
    if (nk_combo_begin_color(ui_ctx, nk_rgb_cf(after), nk_vec2(200,400))) {
        nk_layout_row_dynamic(ui_ctx, 120, 1);
        after = nk_color_picker(ui_ctx, after, NK_RGBA);

        nk_layout_row_dynamic(ui_ctx, 0, 2);
        ui_color_mode = nk_option_label(ui_ctx, "RGB", ui_color_mode == COL_RGB) ? COL_RGB : ui_color_mode;
        ui_color_mode = nk_option_label(ui_ctx, "HSV", ui_color_mode == COL_HSV) ? COL_HSV : ui_color_mode;

        nk_layout_row_dynamic(ui_ctx, 0, 1);
        if (ui_color_mode == COL_RGB) {
            after.r = nk_propertyf(ui_ctx, "#R:", 0, after.r, 1.0f, 0.01f,0.005f);
            after.g = nk_propertyf(ui_ctx, "#G:", 0, after.g, 1.0f, 0.01f,0.005f);
            after.b = nk_propertyf(ui_ctx, "#B:", 0, after.b, 1.0f, 0.01f,0.005f);
            after.a = nk_propertyf(ui_ctx, "#A:", 0, after.a, 1.0f, 0.01f,0.005f);
        } else {
            float hsva[4];
            nk_colorf_hsva_fv(hsva, after);
            hsva[0] = nk_propertyf(ui_ctx, "#H:", 0, hsva[0], 1.0f, 0.01f,0.05f);
            hsva[1] = nk_propertyf(ui_ctx, "#S:", 0, hsva[1], 1.0f, 0.01f,0.05f);
            hsva[2] = nk_propertyf(ui_ctx, "#V:", 0, hsva[2], 1.0f, 0.01f,0.05f);
            hsva[3] = nk_propertyf(ui_ctx, "#A:", 0, hsva[3], 1.0f, 0.01f,0.05f);
            after = nk_hsva_colorfv(hsva);
        }

        color4[0] = after.r * 255;
        color4[1] = after.g * 255;
        color4[2] = after.b * 255;
        color4[3] = after.a * 255;

        nk_combo_end(ui_ctx);
    }
    return !!memcmp(&before.r, &after.r, sizeof(struct nk_colorf));
}

int ui_color3f(const char *label, float *color3) {
    float c[3] = { color3[0]*255, color3[1]*255, color3[2]*255 };
    int ret = ui_color3(label, c);
    for( int i = 0; i < 3; ++i ) color3[i] = c[i] / 255.0f;
    return ret;
}

int ui_color3(const char *label, float *color3) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_colorf after = { color3[0]*ui_alpha/255, color3[1]*ui_alpha/255, color3[2]*ui_alpha/255, 1 }, before = after;
    if (nk_combo_begin_color(ui_ctx, nk_rgb_cf(after), nk_vec2(200,400))) {
        nk_layout_row_dynamic(ui_ctx, 120, 1);
        after = nk_color_picker(ui_ctx, after, NK_RGB);

        nk_layout_row_dynamic(ui_ctx, 0, 2);
        ui_color_mode = nk_option_label(ui_ctx, "RGB", ui_color_mode == COL_RGB) ? COL_RGB : ui_color_mode;
        ui_color_mode = nk_option_label(ui_ctx, "HSV", ui_color_mode == COL_HSV) ? COL_HSV : ui_color_mode;

        nk_layout_row_dynamic(ui_ctx, 0, 1);
        if (ui_color_mode == COL_RGB) {
            after.r = nk_propertyf(ui_ctx, "#R:", 0, after.r, 1.0f, 0.01f,0.005f);
            after.g = nk_propertyf(ui_ctx, "#G:", 0, after.g, 1.0f, 0.01f,0.005f);
            after.b = nk_propertyf(ui_ctx, "#B:", 0, after.b, 1.0f, 0.01f,0.005f);
        } else {
            float hsva[4];
            nk_colorf_hsva_fv(hsva, after);
            hsva[0] = nk_propertyf(ui_ctx, "#H:", 0, hsva[0], 1.0f, 0.01f,0.05f);
            hsva[1] = nk_propertyf(ui_ctx, "#S:", 0, hsva[1], 1.0f, 0.01f,0.05f);
            hsva[2] = nk_propertyf(ui_ctx, "#V:", 0, hsva[2], 1.0f, 0.01f,0.05f);
            after = nk_hsva_colorfv(hsva);
        }

        color3[0] = after.r * 255;
        color3[1] = after.g * 255;
        color3[2] = after.b * 255;

        nk_combo_end(ui_ctx);
    }
    return !!memcmp(&before.r, &after.r, sizeof(struct nk_colorf));
}

int ui_list(const char *label, const char **items, int num_items, int *selector) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int val = nk_combo(ui_ctx, items, num_items, *selector, UI_ROW_HEIGHT, nk_vec2(200,200));
    int chg = val != *selector;
    *selector = val;
    return chg;
}

int ui_slider(const char *label, float *slider) {
    // return ui_slider2(label, slider, va("%.2f ", *slider));
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    nk_size val = *slider * 1000;
    int chg = nk_progress(ui_ctx, &val, 1000, NK_MODIFIABLE);
    *slider = val / 1000.f;
    return chg;
}
int ui_slider2(const char *label, float *slider, const char *caption) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_window *win = ui_ctx->current;
    const struct nk_style *style = &ui_ctx->style;
    struct nk_rect bounds; nk_layout_peek(&bounds, ui_ctx); bounds.w -= 10; // bounds.w *= 0.95f;
    struct nk_vec2 item_padding = style->text.padding;
    struct nk_text text;
    text.padding.x = item_padding.x;
    text.padding.y = item_padding.y;
    text.background = style->window.background;
    text.text = nk_rgba_f(1,1,1,ui_alpha);

        nk_size val = *slider * 1000;
        int chg = nk_progress(ui_ctx, &val, 1000, NK_MODIFIABLE);
        *slider = val / 1000.f;

    chg |= input(MOUSE_L) && nk_input_is_mouse_hovering_rect(&ui_ctx->input, bounds); // , true);

    nk_widget_text(&win->buffer, bounds, caption, strlen(caption), &text, NK_TEXT_RIGHT, style->font);
    return chg;
}

int ui_bool(const char *label, bool *enabled ) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int val = *enabled;
#if 0
    int chg = !!nk_checkbox_label(ui_ctx, "", &val);
#else
    int chg = !!nk_button_transparent(ui_ctx, val ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK );
#endif
    *enabled ^= chg;
    return chg;
}

int ui_int(const char *label, int *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int prev = *v;
    *v = nk_propertyi(ui_ctx, "#", INT_MIN, *v, INT_MAX, 1,1);
    return prev != *v;
}

int ui_unsigned(const char *label, unsigned *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    unsigned prev = *v;
    *v = (unsigned)nk_propertyd(ui_ctx, "#", 0, *v, UINT_MAX, 1,1);
    return prev != *v;
}

int ui_short(const char *label, short *v) {
    int i = *v, ret = ui_int( label, &i );
    return *v = (short)i, ret;
}

int ui_float(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    float prev = v[0]; v[0] = nk_propertyf(ui_ctx, "#", -FLT_MAX, v[0], FLT_MAX, 0.01f,0.005f);
    return prev != v[0];
}

int ui_double(const char *label, double *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    double prev = v[0]; v[0] = nk_propertyd(ui_ctx, "#", -DBL_MAX, v[0], DBL_MAX, 0.01f,0.005f);
    return prev != v[0];
}

int ui_clampf(const char *label, float *v, float minf, float maxf) {
    if( minf > maxf ) return ui_clampf(label, v, maxf, minf);

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    float prev = v[0]; v[0] = nk_propertyf(ui_ctx, "#", minf, v[0], maxf, 0.1f,0.05f);
    return prev != v[0];
}

int ui_float2(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = va("%.2f, %.2f", v[0], v[1]);
    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 0, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, 1,0.5f);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, 1,0.5f);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1];
    }
    return 0;
}

int ui_float3(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = va("%.2f, %.2f, %.2f", v[0], v[1], v[2]);
    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 0, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, 1,0.5f);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, 1,0.5f);
        float prev2 = v[2]; nk_property_float(ui_ctx, "#Z:", -FLT_MAX, &v[2], FLT_MAX, 1,0.5f);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1] || prev2 != v[2];
    }
    return 0;
}

int ui_float4(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = va("%.2f, %.2f, %.2f, %.2f", v[0], v[1], v[2], v[3]);
    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 0, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, 1,0.5f);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, 1,0.5f);
        float prev2 = v[2]; nk_property_float(ui_ctx, "#Z:", -FLT_MAX, &v[2], FLT_MAX, 1,0.5f);
        float prev3 = v[3]; nk_property_float(ui_ctx, "#W:", -FLT_MAX, &v[3], FLT_MAX, 1,0.5f);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1] || prev2 != v[2] || prev3 != v[3];
    }
    return 0;
}

int ui_mat33(const char *label, float M[9]) {
    int changed = 0;
    changed |= ui_label(label);
    changed |= ui_float3(NULL, M);
    changed |= ui_float3(NULL, M+3);
    changed |= ui_float3(NULL, M+6);
    return changed;
}
int ui_mat34(const char *label, float M[12]) {
    int changed = 0;
    changed |= ui_label(label);
    changed |= ui_float4(NULL, M);
    changed |= ui_float4(NULL, M+4);
    changed |= ui_float4(NULL, M+8);
    return changed;
}
int ui_mat44(const char *label, float M[16]) {
    int changed = 0;
    changed |= ui_label(label);
    changed |= ui_float4(NULL, M);
    changed |= ui_float4(NULL, M+4);
    changed |= ui_float4(NULL, M+8);
    changed |= ui_float4(NULL, M+12);
    return changed;
}

int ui_buffer(const char *label, char *buffer, int buflen) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int active = nk_edit_string_zero_terminated(ui_ctx, NK_EDIT_AUTO_SELECT|NK_EDIT_CLIPBOARD|NK_EDIT_FIELD/*NK_EDIT_BOX*/|NK_EDIT_SIG_ENTER, buffer, buflen, nk_filter_default);
    return !!(active & NK_EDIT_COMMITED) ? nk_edit_unfocus(ui_ctx), 1 : 0;
}

int ui_string(const char *label, char **str) {
    char *bak = va("%s%c", *str ? *str : "", '\0');
    int rc = ui_buffer(label, bak, strlen(bak)+2);
    if( *str ) 0[*str] = '\0';
    strcatf(str, "%s", bak);
    return rc;
}

int ui_separator() {
    nk_layout_row_dynamic(ui_ctx, UI_SEPARATOR_HEIGHT, 1);

    ui_hue_cycle( 1 );

    struct nk_command_buffer *canvas;
    struct nk_input *input = &ui_ctx->input;
    canvas = nk_window_get_canvas(ui_ctx);
    struct nk_rect space;
    enum nk_widget_layout_states state;
    state = nk_widget(&space, ui_ctx);
    if (state) nk_fill_rect(canvas, space, 0, ui_ctx->style.window.header.normal.data.color );

    return 1;
}

int ui_subimage(const char *label, handle id, unsigned iw, unsigned ih, unsigned sx, unsigned sy, unsigned sw, unsigned sh) {
    nk_layout_row_dynamic(ui_ctx, sh < 30 || id == texture_checker().id ? 0 : sh, 1 + (label && label[0]));
    if( label && label[0] ) ui_label_(label, NK_TEXT_LEFT);

    struct nk_rect bounds; nk_layout_peek(&bounds, ui_ctx); bounds.w -= 10; // bounds.w *= 0.95f;

    int ret = nk_button_image_styled(ui_ctx, &ui_ctx->style.button, nk_subimage_id((int)id, iw, ih, nk_rect(sx,sy,sw,sh)));
    if( !ret ) {
        ret |= input(MOUSE_L) && nk_input_is_mouse_hovering_rect(&ui_ctx->input, bounds); // , true);
    }
    if( ret ) {
        int px = 100 * (ui_ctx->input.mouse.pos.x - bounds.x ) / (float)bounds.w;
        int py = 100 * (ui_ctx->input.mouse.pos.y - bounds.y ) / (float)bounds.h;
        return px * 100 + py; // printf("%d %d xy:%d\n", px, py, (px * 100) + py);
    }
    return ret; // !!ret;
}

int ui_image(const char *label, handle id, unsigned w, unsigned h) {
    return ui_subimage(label, id, w,h, 0,0,w,h);
}

int ui_texture(const char *label, texture_t t) {
    return ui_subimage(label, t.id, t.w,t.h, 0,0,t.w,t.h);
}

int ui_subtexture(const char *label, texture_t t, unsigned x, unsigned y, unsigned w, unsigned h) {
    return ui_subimage(label, t.id, t.w,t.h, x,y,w,h);
}

int ui_colormap( const char *map_name, colormap_t *cm ) {
    int ret = 0;
    if( !cm->texture ) {
        const char *title = va("%s (no image)", map_name);
        if( ui_image( title, texture_checker().id, 0,0 ) ) {
            ret = 2;
        }
    } else {
        unsigned w = cm->texture->w, h = cm->texture->h;
        ui_label(va("%s (%s)", map_name, cm->texture->filename) ); // @fixme: labelf would crash?

        const char *fmt[] = { "", "R", "RG", "RGB", "RGBA" };
        const char *title = va("%s %dx%d %s", map_name, w, h, fmt[cm->texture->n]);
        if( ui_image( title, cm->texture->id, 128, 128 ) ) {
            ret = 2;
        }
    }

    if( ui_color4f( va("%s Color", map_name), (float *) &cm->color ) ) {
        ret = 1;
    }
    return ret;
}

int ui_radio(const char *label, const char **items, int num_items, int *selector) {
    int ret = 0;
    if( label && label[0] ) ui_label(label);
    for( int i = 0; i < num_items; i++ ) {
        bool enabled = *selector == i;
        if( ui_bool( va("%s%s", label && label[0] ? "  ":"", items[i]), &enabled ) ) {
            *selector = i;
            ret |= 1;
        }
    }
    return ret;
}

int ui_section(const char *section) {
    //ui_separator();
    return ui_label(va("*%s", section));
}

int ui_dialog(const char *title, const char *text, int choices, bool *show) { // @fixme: return
    ui_has_active_popups = 0;
    if(*show) {
        static struct nk_rect s = {0, 0, 300, 190};
        if (nk_popup_begin(ui_ctx, NK_POPUP_STATIC, title, NK_WINDOW_BORDER|NK_WINDOW_CLOSABLE, s)) {
            nk_layout_row_dynamic(ui_ctx, 20, 1);
            for( char label[1024]; *text && sscanf(text, "%[^\r\n]", label); ) {
                nk_label(ui_ctx, label, NK_TEXT_LEFT);
                text += strlen(label); while(*text && *text < 32) text++;
            }

            if( choices ) {
                if( ui_buttons(choices > 1 ? 2 : 1, "OK", "Cancel") ) {
                    *show = 0;
                    nk_popup_close(ui_ctx);
                }
            }

            nk_popup_end(ui_ctx);
        } else {
            *show = nk_false;
        }
        ui_has_active_popups = *show;
    }
    return *show;
}

#define ui_bitmask_template(X) \
int ui_bitmask##X(const char *label, uint##X##_t *enabled) { \
    /* @fixme: better way to retrieve widget width? nk_layout_row_dynamic() seems excessive */ \
    nk_layout_row_dynamic(ui_ctx, 1, 1); \
    struct nk_rect bounds = nk_widget_bounds(ui_ctx); \
\
    /* actual widget: label + X checkboxes */ \
    nk_layout_row_begin(ui_ctx, NK_STATIC, 0, 1+X); \
\
        int offset = bounds.w > (15*X) ? bounds.w - (15*X) : 0; /* bits widget below needs at least 118px wide */ \
        nk_layout_row_push(ui_ctx, offset); \
        ui_label_(label, NK_TEXT_LEFT); \
\
        uint8_t copy = *enabled; \
        for( int i = 0; i < X; ++i ) { \
            int b = (X-1-i); \
            nk_layout_row_push(ui_ctx, 10); \
            /* bit */ \
            int val = (*enabled >> b) & 1; \
            int chg = nk_checkbox_label(ui_ctx, "", &val); \
            *enabled = (*enabled & ~(1 << b)) | ((!!val) << b); \
            /* tooltip */ \
            struct nk_rect bb = { offset + 10 + i * 14, bounds.y, 14, 30 }; /* 10:padding,14:width,30:height */ \
            if (nk_input_is_mouse_hovering_rect(&ui_ctx->input, bb) && !ui_has_active_popups && nk_window_has_focus(ui_ctx)) { \
                nk_tooltipf(ui_ctx, "Bit %d", b); \
            } \
        } \
\
    nk_layout_row_end(ui_ctx); \
    return copy ^ *enabled; \
}

ui_bitmask_template(8);
ui_bitmask_template(16);
//ui_bitmask_template(32);

int ui_console() { // @fixme: buggy
    static char *cmd = 0;
    static int enter = 0;

    struct nk_font *font = nk_glfw.atlas.fonts->next /*2nd font*/;
    if( font && nk_style_push_font(ui_ctx, &font->handle) ) {} else font = 0;

        struct nk_rect bounds = {0,0,400,300}; // @fixme: how to retrieve inlined region below? (inlined)
        if( 1 ) /*if( windowed || (!windowed && *inlined) )*/ bounds = nk_window_get_content_region(ui_ctx);
        else { struct nk_rect b; nk_layout_peek(&b, ui_ctx); bounds.w = b.w; }

    nk_layout_row_static(ui_ctx, bounds.h - UI_ROW_HEIGHT, bounds.w, 1);

    static array(char*) lines = 0;
    for( int i = 0; i < array_count(lines); ++i )
    if(lines[i]) nk_label_wrap(ui_ctx, lines[i]); // "This is a very long line to hopefully get this text to be wrapped into multiple lines to show line wrapping");

    if( enter ) {
        array_push(lines, 0);

        for( FILE *fp = popen(cmd, "r"); fp; pclose(fp), fp = 0) {
            int ch;
            do {
                ch = fgetc(fp);
                if( strchr("\r\n\t\b", ch) ) {
                    array_push(lines,0);
                    continue;
                }
                if( ch >= ' ' ) strcatf(array_back(lines), "%c", ch);
            } while(ch > 0);
        }

        cmd[0] = 0;
    }

    enter = ui_string("", &cmd);

    if( font )  nk_style_pop_font(ui_ctx);

    return enter;
}

int ui_browse(const char **output, bool *inlined) {
    static struct browser_media media = {0};
    static struct browser browsers[2] = {0}; // 2 instances max: 0=inlined, 1=windowed
    static char *results[2] = {0}; // 2 instances max: 0=inlined, 1=windowed
    do_once {
        const int W = 96, H = 96; // 2048x481 px, 21x5 cells
        texture_t i = texture("icons/suru.png", TEXTURE_RGBA|TEXTURE_MIPMAPS);
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H, 16, 3), BROWSER_FOLDER); // default group
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H,  2, 4), BROWSER_HOME); 
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H, 17, 3), BROWSER_COMPUTER);
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H,  1, 4), BROWSER_PROJECT);
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H,  0, 4), BROWSER_DESKTOP);

        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  8, 0), "");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 10, 2), ".txt.md.doc.license" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  8, 1), ".ini.cfg" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  8, 3), ".xlsx" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  9, 0), ".c" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  4, 1), ".h.hpp.hh.hxx" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  4, 2), ".fs.vs.gs.fx.glsl.shader" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 12, 0), ".cpp.cc.cxx" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 15, 0), ".json" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 15, 2), ".bat.sh" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  6, 1), ".htm.html" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 20, 1), ".xml" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 12, 1), ".js" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  0, 3), ".ts" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  6, 2), ".py" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 16, 1), ".lua" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 10, 0), ".css.doc" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  6, 0), ".wav.flac.ogg.mp1.mp3.mod.xm.s3m.it.sfxr.mid.fur" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  1, 3), ".ttf.ttc.otf" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  7, 1), ".jpg.jpeg.png.bmp.psd.pic.pnm.ico.ktx.pvr.dds.astc.basis.hdr.tga.gif" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  4, 3), ".mp4.mpg.ogv.mkv.wmv.avi" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  2, 1), ".iqm.iqe.gltf.gltf2.glb.fbx.obj.dae.blend.md3.md5.ms3d.smd.x.3ds.bvh.dxf.lwo" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  0, 1), ".exe" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  7, 0), ".bin.dSYM.pdb.o.lib.dll" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 15, 3), ".zip.rar.7z.pak" ".");

        for( int j = 0; j < countof(browsers); ++j ) browser_init(&browsers[j]);
        browsers[0].listing = 1; // inlined version got listing by default, as there is not much space for layouting
    }
    // at_exit: browser_free(&browser);

    int clicked = 0;
    bool windowed = !inlined;
    if( windowed || (!windowed && *inlined) ) {

        struct browser *browser = browsers + windowed; // select instance
        char **result = results + windowed; // select instance

        struct nk_rect bounds = {0,0,400,300}; // @fixme: how to retrieve inlined region below? (inlined)
        if( windowed || (!windowed && *inlined) ) bounds = nk_window_get_content_region(ui_ctx);
        else { struct nk_rect b; nk_layout_peek(&b, ui_ctx); bounds.w = b.w; }

        clicked = browser_run(ui_ctx, browser, windowed, bounds);
        if( clicked ) {
            strcatf(result, "%d", 0);
            (*result)[0] = '\0';
            strcatf(result, "%s", browser->file);
            if( inlined ) *inlined = 0;

            const char *target = ifdef(win32, "/", "\\"), *replace = ifdef(win32, "\\", "/");
            strswap(*result, target, replace);

            if( output ) *output = *result;
        }
    }

    return clicked;
}

/*
//  demo:
    static const char *file;
    if( ui_panel("inlined", 0)) {
        static bool show_browser = 0;
        if( ui_button("my button") ) { show_browser = true; }
        if( ui_browse(&file, &show_browser) ) puts(file);
        ui_panel_end();
    }
    if( ui_window("windowed", 0) ) {
        if( ui_browse(&file, NULL) ) puts(file);
        ui_window_end();
    }
*/

// ----------------------------------------------------------------------------

int ui_demo(int do_windows) {
    static int integer = 42;
    static bool toggle = true;
    static bool boolean = true;
    static float floating = 3.14159;
    static float float2[2] = {1,2};
    static float float3[3] = {1,2,3};
    static float float4[4] = {1,2,3,4};
    static float rgb[3] = {0.84,0.67,0.17};
    static float rgba[4] = {0.67,0.90,0.12,1};
    static float slider = 0.5f;
    static float slider2 = 0.5f;
    static char string[64] = "hello world 123";
    static int item = 0; const char *list[] = {"one","two","three"};
    static bool show_dialog = false;
    static bool show_browser = false;
    static const char* browsed_file = "";
    static uint8_t bitmask = 0x55;
    static int hits = 0;
    static int window1 = 0, window2 = 0, window3 = 0;
    static int disable_all = 0;

    if( ui_panel("UI", 0) ) {
        int choice = ui_toolbar("Browser;Toast@Notify;Toggle on/off");
            if(choice == 1) show_browser = true;
            if(choice == 2) ui_notify(va("My random toast (%d)", rand()), va("This is notification #%d", ++hits));
            if(choice == 3) disable_all ^= 1;

        if( disable_all ) ui_disable();

        if( ui_browse(&browsed_file, &show_browser) ) puts(browsed_file);

        if( ui_section("Labels")) {}
        if( ui_label("my label")) {}
        if( ui_label("my label with tooltip@built on " __DATE__ " " __TIME__)) {}
        if( ui_label2_toolbar("my toolbar", ICON_MD_STAR ICON_MD_STAR_OUTLINE ICON_MD_BOOKMARK ICON_MD_BOOKMARK_BORDER) ) {}
        //if( ui_label2_wrap("my long label", "and some long long long long text wrapped")) {}

        if( ui_section("Types")) {}
        if( ui_bool("my bool", &boolean) ) puts("bool changed");
        if( ui_int("my int", &integer) ) puts("int changed");
        if( ui_float("my float", &floating) ) puts("float changed");
        if( ui_buffer("my string", string, 64) ) puts("string changed");

        if( ui_section("Vectors") ) {}
        if( ui_float2("my float2", float2) ) puts("float2 changed");
        if( ui_float3("my float3", float3) ) puts("float3 changed");
        if( ui_float4("my float4", float4) ) puts("float4 changed");

        if( ui_section("Lists")) {}
        if( ui_list("my list", list, 3, &item ) ) puts("list changed");

        if( ui_section("Colors")) {}
        if( ui_color3f("my color3", rgb) ) puts("color3 changed");
        if( ui_color4f("my color4@this is a tooltip", rgba) ) puts("color4 changed");

        if( ui_section("Sliders")) {}
        if( ui_slider("my slider", &slider)) puts("slider changed");
        if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");

        if( do_windows ) {
        if( ui_section("Windows")) {}
        int show = ui_buttons(3, "Container", "SubPanel", "SubRender");
        if( show == 1 ) window1 = 1;
        if( show == 2 ) window2 = 1;
        if( show == 3 ) window3 = 1;
        }

        if( ui_section("Others")) {}
        if( ui_bitmask8("my bitmask", &bitmask) ) printf("bitmask changed %x\n", bitmask);
        if( ui_toggle("my toggle", &toggle) ) printf("toggle %s\n", toggle ? "on":"off");
        if( ui_image("my image", texture_checker().id, 0, 0) ) { puts("image clicked"); }

        if( ui_separator() ) {}
        if( ui_button("my button") ) { puts("button clicked"); show_dialog = true; }
        if( ui_buttons(2, "yes", "no") ) { puts("button clicked"); }
        if( ui_buttons(3, "yes", "no", "maybe") ) { puts("button clicked"); }
        if( ui_dialog("my dialog", __FILE__ "\n" __DATE__ "\n" "Public Domain.", 2/*two buttons*/, &show_dialog) ) {}

        if( disable_all ) ui_enable(); // restore enabled state

        ui_panel_end();
    }

    if( !do_windows ) return 0;

    // window api showcasing
    if( ui_window("Container demo", &window1) ) {
        ui_label("label #1");
        if( ui_bool("my bool", &boolean) ) puts("bool changed");
        if( ui_int("my int", &integer) ) puts("int changed");
        if( ui_float("my float", &floating) ) puts("float changed");
        if( ui_buffer("my string", string, 64) ) puts("string changed");
        ui_window_end();
    }

    if( ui_window("SubPanel demo", &window2) ) {
        if( ui_panel("panel #2", 0) ) {
            ui_label("label #2");
            if( ui_bool("my bool", &boolean) ) puts("bool changed");
            if( ui_int("my int", &integer) ) puts("int changed");
            if( ui_float("my float", &floating) ) puts("float changed");
            if( ui_buffer("my string", string, 64) ) puts("string changed");
            ui_panel_end();
        }
        ui_window_end();
    }

    if( ui_window("SubRender demo", &window3) ) {
        if( ui_panel("panel #3A", 0) ) {
            if( ui_bool("my bool", &boolean) ) puts("bool changed");
            if( ui_int("my int", &integer) ) puts("int changed");
            if( ui_float("my float", &floating) ) puts("float changed");
            if( ui_buffer("my string", string, 64) ) puts("string changed");
            if( ui_separator() ) {}
            if( ui_slider("my slider", &slider)) puts("slider changed");
            if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");
            ui_panel_end();
        }
        if( ui_panel("panel #3B", 0) ) {
            if( ui_bool("my bool", &boolean) ) puts("bool changed");
            if( ui_int("my int", &integer) ) puts("int changed");
            if( ui_float("my float", &floating) ) puts("float changed");
            if( ui_buffer("my string", string, 64) ) puts("string changed");
            if( ui_separator() ) {}
            if( ui_slider("my slider", &slider)) puts("slider changed");
            if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");
            ui_panel_end();
        }

        const char *title = "SubRender demo";
        struct nk_window *win = nk_window_find(ui_ctx, title);
        if( win ) {
            enum { menubar_height = 65 }; // title bar (~32) + menu bounds (~25)
            struct nk_rect bounds = win->bounds; bounds.y += menubar_height; bounds.h -= menubar_height; 
#if 1
            ddraw_flush();

            // @fixme: this is breaking rendering when post-fxs are in use. edit: cannot reproduce
            static texture_t tx = {0};
            if( texture_rec_begin(&tx, bounds.w, bounds.h )) {
                glClearColor(0.15,0.15,0.15,1);
                glClear(GL_COLOR_BUFFER_BIT);
                ddraw_grid(10);
                ddraw_flush();
                texture_rec_end(&tx);
            }
            struct nk_image image = nk_image_id((int)tx.id);
            nk_draw_image_flipped(nk_window_get_canvas(ui_ctx), bounds, &image, nk_white);
#else
            static video_t *v = NULL;
            do_once v = video( "bjork-all-is-full-of-love.mp4", VIDEO_RGB );

            texture_t *textures = video_decode( v );

            struct nk_image image = nk_image_id((int)textures[0].id);
            nk_draw_image(nk_window_get_canvas(ui_ctx), bounds, &image, nk_white);
#endif
        }

        ui_window_end();
    }
    return 0;
}
#line 0

#line 1 "fwk_profile.c"
#if ENABLE_PROFILER
profiler_t profiler;
int profiler_enabled = 1;

void (profiler_init)() { map_init(profiler, less_str, hash_str); profiler_enabled &= !!profiler; }
int  (profiler_enable)(bool on) { return profiler_enabled = on; }
void (ui_profiler)() {
    // @todo: ui_plot()

    double fps = window_fps();
    profile_setstat("Render.num_fps", fps);

    // draw fps-meter: 300 samples, [0..70] range each, 70px height plot.
    nk_layout_row_dynamic(ui_ctx, 70, 1);

    enum { COUNT = 300 };

    static float values[COUNT] = {0}; static int offset = 0;
    values[offset=(offset+1)%COUNT] = fps;

    int index = -1;
    if( nk_chart_begin(ui_ctx, NK_CHART_LINES, COUNT, 0.f, 70.f) ) {
        for( int i = 0; i < COUNT; ++i ) {
            nk_flags res = nk_chart_push(ui_ctx, (float)values[i]);
            if( res & NK_CHART_HOVERING ) index = i;
            if( res & NK_CHART_CLICKED ) index = i;
        }
        nk_chart_end(ui_ctx);
    }

    //  hightlight 60fps, 36fps and 12fps
    struct nk_rect space; nk_layout_peek(&space, ui_ctx);
    struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
    nk_stroke_line(canvas, space.x+0,space.y-60,space.x+space.w,space.y-60, 1.0, nk_rgba(0,255,0,128));
    nk_stroke_line(canvas, space.x+0,space.y-36,space.x+space.w,space.y-36, 1.0, nk_rgba(255,255,0,128));
    nk_stroke_line(canvas, space.x+0,space.y-12,space.x+space.w,space.y-12, 1.0, nk_rgba(255,0,0,128));

    if( index >= 0 ) {
        nk_tooltipf(ui_ctx, "%.2f fps", (float)values[index]);
    }

    for each_map_ptr_sorted(profiler, const char *, key, struct profile_t, val ) {
        if( isnan(val->stat) ) {
            float v = val->avg/1000.0;
            ui_slider2(*key, &v, va("%.2f ms", val->avg/1000.0));
        } else {
            float v = val->stat;
            ui_slider2(*key, &v, va("%.2f", val->stat));
            val->stat = 0;
        }
    }
}
#endif
#line 0

#line 1 "fwk_video.c"
// tip: convert video to x265/mp4. note: higher crf to increase compression (default crf is 28)
// ffmpeg -i {{infile}} -c:v libx265 -crf 24 -c:a copy {{outfile}}

struct video_t {
    // mpeg player
    plm_t *plm;
    double previous_time;
    bool paused;
    bool has_ycbcr;
    bool has_audio;
    // yCbCr
    union {
        struct {
            texture_t textureY;
            texture_t textureCb;
            texture_t textureCr;
        };
        texture_t textures[3];
    };
    // rgb
    void *surface;
    texture_t texture;
};

static void mpeg_update_texture(GLuint unit, GLuint texture, plm_plane_t *plane) {
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RED, plane->width, plane->height, 0,
        GL_RED, GL_UNSIGNED_BYTE, plane->data
    );
}
static void mpeg_video_callback( plm_t* plm, plm_frame_t* frame, void* user ) {
    video_t *v = (video_t*)user;
    if(v->paused) return;

    if(v->has_ycbcr) {
        mpeg_update_texture(GL_TEXTURE0, v->textureY.id, &frame->y);
        mpeg_update_texture(GL_TEXTURE1, v->textureCb.id, &frame->cb);
        mpeg_update_texture(GL_TEXTURE2, v->textureCr.id, &frame->cr);
    } else {
        plm_frame_to_rgb( frame, v->surface, v->texture.w * 3 );
        texture_update( &v->texture, v->texture.w, v->texture.h, v->texture.n, v->surface, v->texture.flags );
    }
    (void)plm;
}
static void mpeg_audio_callback(plm_t *plm, plm_samples_t *samples, void *user) {
    video_t *v = (video_t*)user;
    audio_queue(v->paused ? NULL : samples->interleaved, samples->count, AUDIO_FLOAT | AUDIO_2CH | AUDIO_44KHZ );
    (void)plm;
}

video_t* video( const char *filename, int flags ) {
    plm_t* plm = plm_create_with_file( vfs_handle(filename), 1 );
    if ( !plm ) {
        PANIC( "!Cannot open '%s' file for reading\n", filename );
        return 0;
    }

    int w = plm_get_width( plm );
    int h = plm_get_height( plm );
    float fps = plm_get_framerate( plm );
    float rate = plm_get_samplerate( plm );

    video_t *v = MALLOC(sizeof(video_t)), zero = {0};
    *v = zero;

    v->has_ycbcr = flags & VIDEO_RGB ? 0 : 1;

    if( v->has_ycbcr ) {
        v->textureY = texture_create( w, h, 1, NULL, TEXTURE_R );
        v->textureCb = texture_create( w, h, 1, NULL, TEXTURE_R );
        v->textureCr = texture_create( w, h, 1, NULL, TEXTURE_R );
    } else {
        int w16 = (w+15) & ~15;
        int h16 = (h+15) & ~15;
        v->texture = texture_create( w16, h16, 3, NULL, TEXTURE_SRGB );
        v->surface = REALLOC( v->surface,  w16 * h16 * 3 );
    }

    v->plm = plm;

    v->has_audio = flags & VIDEO_NO_AUDIO ? 0 : 1;

    plm_set_loop(plm, flags & VIDEO_LOOP);
    plm_set_video_decode_callback(plm, mpeg_video_callback, v);
    if( v->has_audio ) {
        plm_set_audio_enabled(plm, true);
        plm_set_audio_stream(plm, 0);
        plm_set_audio_decode_callback(plm, mpeg_audio_callback, v);
    }

    PRINTF( "Video texture: %s (%dx%dx%d %.0ffps %.1fKHz)\n", file_name(filename), w, h, v->has_ycbcr ? 3:1, fps, rate / 1000 );
    return v;
}

texture_t* video_decode( video_t *v ) { // decodes next frame, returns associated texture(s)
    double current_time = time_ss();
    double elapsed_time = current_time - v->previous_time;
    if (elapsed_time > 1.0 / 30.0) {
        elapsed_time = 1.0 / 30.0;
    }
    v->previous_time = current_time;

    if(!v->paused)
    plm_decode(v->plm, elapsed_time);

    return v->has_ycbcr ? &v->textureY : &v->texture;
}

void video_destroy(video_t *v) {
    plm_destroy( v->plm );

    if( v->has_ycbcr ) {
        texture_destroy(&v->textureY);
        texture_destroy(&v->textureCr);
        texture_destroy(&v->textureCb);
    } else {
        texture_destroy(&v->texture);
        v->surface = REALLOC(v->surface, 0);
    }

    video_t zero = {0};
    *v = zero;
    FREE(v);
}

int video_has_finished(video_t *v) {
    return !!plm_has_ended(v->plm);
}
double video_duration(video_t *v) {
    return plm_get_duration(v->plm);
}
int video_seek(video_t *v, double seek_to) {
    plm_seek(v->plm, clampf(seek_to, 0, video_duration(v)), FALSE);
    if( v->has_audio ) audio_queue_clear();
    return 1;
}
double video_position(video_t *v) {
    return plm_get_time(v->plm);
}
void video_pause(video_t *v, bool paused) {
    v->paused = paused;
}
bool video_is_paused(video_t *v) {
    return v->paused;
}
bool video_is_rgb(video_t *v) {
    return !v->has_ycbcr;
}
texture_t* video_textures( video_t *v ) {
    return v->has_ycbcr ? &v->textureY : &v->texture;
}

// -----------------------------------------------------------------------------
// ffmpeg video recording
// [src] http://blog.mmacklin.com/2013/06/11/real-time-video-capture-with-ffmpeg/

static FILE* rec_ffmpeg;
static FILE* rec_mpeg1;

void record_stop(void) {
    if(rec_ffmpeg) ifdef(win32, _pclose, pclose)(rec_ffmpeg);
    rec_ffmpeg = 0;

    if(rec_mpeg1) fclose(rec_mpeg1);
    rec_mpeg1 = 0;
}

bool record_active() {
    return rec_ffmpeg || rec_mpeg1;
}

bool record_start(const char *outfile_mp4) {
    do_once atexit(record_stop);

    record_stop();

    // first choice: external ffmpeg encoder
    if( !rec_ffmpeg ) {
        extern const char *TOOLS;

        char *tools_native_path = strswap( va("%s/", TOOLS), ifdef(win32, "/", "\\"), ifdef(win32, "\\", "/") );

        char *cmd = va("%sffmpeg%s "
                    "-hide_banner -loglevel error " // less verbose
                    "-r %d -f rawvideo -pix_fmt bgr24 -s %dx%d " // raw BGR WxH-60Hz frames
                    // "-framerate 30 " // interpolating new video output frames from the source frames
                    "-i - "              // read frames from stdin
                    //"-draw_mouse 1 "
                    "-threads 0 "
                    //"-vsync vfr "
                    "-preset ultrafast " // collection of options that will provide a certain encoding speed [fast,ultrafast]
                    // "-tune zerolatency " // change settings based upon the specifics of your input
                    //"-crf 21 "           // range of the CRF scale [0(lossless)..23(default)..51(worst quality)]
                    "-pix_fmt yuv420p "  // compatible with Windows Media Player and Quicktime
                    "-vf vflip "         // flip Y
//                  "-vf \"pad=ceil(iw/2)*2:ceil(ih/2)*2\" "
                    "-y \"%s\"", tools_native_path, ifdef(win32, ".exe", ifdef(osx, ".osx",".linux")),
                    (int)window_fps(), window_width(), window_height(), outfile_mp4);    // overwrite output file

        // -rtbufsize 100M (https://trac.ffmpeg.org/wiki/DirectShow#BufferingLatency) Prevent some frames in the buffer from being dropped.
        // -probesize 10M (https://www.ffmpeg.org/ffmpeg-formats.html#Format-Options) Set probing size in bytes, i.e. the size of the data to analyze to get stream information. A higher value will enable detecting more information in case it is dispersed into the stream, but will increase latency. Must be an integer not lesser than 32. It is 5000000 by default.
        // -c:v libx264 (https://www.ffmpeg.org/ffmpeg.html#Main-options) Select an encoder (when used before an output file) or a decoder (when used before an input file) for one or more streams. codec is the name of a decoder/encoder or a special value copy (output only) to indicate that the stream is not to be re-encoded.

        // open pipe to ffmpeg's stdin in binary write mode
        rec_ffmpeg = ifdef(win32, _popen(cmd, "wb"), popen(cmd, "w"));
    }

    // fallback: built-in mpeg1 encoder
    if( !rec_ffmpeg ) {
        rec_mpeg1 = fopen(outfile_mp4, "wb"); // "a+b"
    }

    return record_active();
}

void record_frame() {
    if( record_active() ) {
        void* pixels = screenshot_async(-3); // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA. ps: BGR is fastest on my intel discrete gpu

        if( rec_ffmpeg ) {
            fwrite(pixels, 3 * window_width() * window_height(), 1, rec_ffmpeg);
        }
        if( rec_mpeg1 ) {
            jo_write_mpeg(rec_mpeg1, pixels, window_width(), window_height(), 24);  // 24fps
        }
    }
}
#line 0

#line 1 "fwk_window.c"
//-----------------------------------------------------------------------------
// fps locking

static volatile float framerate = 0;
static volatile unsigned fps_active, timer_counter, loop_counter;
static
int fps__timing_thread(void *arg) {
    int64_t ns_excess = 0;
    while( fps_active ) {
        if( framerate <= 0 ) {
            loop_counter = timer_counter = 0;
            sleep_ms(250);
        } else {
            timer_counter++;
            int64_t tt = (int64_t)(1e9/(float)framerate) - ns_excess;
            uint64_t took = -time_ns();
        #if is(win32)
            timeBeginPeriod(1); Sleep( tt > 0 ? tt/1e6 : 0 );
        #else
            sleep_ns( (float)tt );
        #endif
            took += time_ns();
            ns_excess = took - tt;
            if( ns_excess < 0 ) ns_excess = 0;
            //puts( strf("%lld", ns_excess) );
        }
    }
    fps_active = 1;

    (void)arg;
    return thread_exit(0), 0;
}
static
void fps_locker( int on ) {
    if( on ) {
        // private threaded timer
        fps_active = 1, timer_counter = loop_counter = 0;
        thread_init( fps__timing_thread, 0, "fps__timing_thread()", 0 );
    } else {
        fps_active = 0;
    }
}
// function that locks render to desired `framerate` framerate (in FPS).
// - assumes fps_locker() was enabled beforehand.
// - returns true if must render, else 0.
static
int fps_wait() {
    if( framerate <= 0 ) return 1;
    if( !fps_active ) return 1;

    // if we throttled too much, cpu idle wait
    while( fps_active && (loop_counter > timer_counter) ) {
        //thread_yield();
        sleep_ns(100);
    }

    // max auto frameskip is 10: ie, even if speed is low paint at least one frame every 10
    enum { maxframeskip = 10 };
    if( timer_counter > loop_counter + maxframeskip ) {
        loop_counter = timer_counter;
    }
    loop_counter++;

    // only draw if we are fast enough, otherwise skip the frame
    return loop_counter >= timer_counter;
}
static
void window_vsync(float hz) {
    if( hz <= 0 ) return;
    do_once fps_locker(1);
    framerate = hz;
    fps_wait();
}

//-----------------------------------------------------------------------------

#if 0 // deprecated
static void (*hooks[64])() = {0};
static void *userdatas[64] = {0};

bool window_hook(void (*func)(), void* user) {
    window_unhook( func );
    for( int i = 0; i < 64; ++i ) {
        if( !hooks[i] ) {
            hooks[i] = func;
            userdatas[i] = user;
            return true;
        }
    }
    return false;
}
void window_unhook(void (*func)()) {
    for( int i = 0; i < 64; ++i ) {
        if(hooks[i] == func) {
            hooks[i] = 0;
            userdatas[i] = 0;
        }
    }
}
#endif

static GLFWwindow *window;
static int w, h, xpos, ypos, paused;
static int fullscreen, xprev, yprev, wprev, hprev;
static uint64_t frame_count;
static double t, dt, fps, hz = 0.00;
static char title[128] = {0};
static char screenshot_file[DIR_MAX];
static int locked_aspect_ratio = 0;
static vec4 winbgcolor = {0,0,0,1};

vec4 window_getcolor_() { return winbgcolor; } // internal

// -----------------------------------------------------------------------------
// glfw

struct app {
    GLFWwindow *window;
    int width, height, keep_running;
    unsigned flags;

    struct nk_context *ctx;
    struct nk_glfw *nk_glfw;
} appHandle = {0}, *g;

static void glfw_error_callback(int error, const char *description) {
    if( is(osx) && error == 65544 ) return; // whitelisted
    PANIC("%s (error %x)", description, error);
}

void glfw_quit(void) {
    do_once {
        glfwTerminate();
    }
}

void glfw_init() {
    do_once {
        g = &appHandle;

        glfwSetErrorCallback(glfw_error_callback);
        int ok = !!glfwInit();
        assert(ok); // if(!ok) PANIC("cannot initialize glfw");

        atexit(glfw_quit); //glfwTerminate);
    }
}

void window_drop_callback(GLFWwindow* window, int count, const char** paths) {
    // @fixme: win: convert from utf8 to window16 before processing

    char pathdir[DIR_MAX]; snprintf(pathdir, DIR_MAX, "%s/import/%llu_%s/", ART, (unsigned long long)date(), ifdef(linux, getlogin(), getenv("USERNAME")));
    mkdir( pathdir, 0777 );

    int errors = 0;
    for( int i = 0; i < count; ++i ) {
        const char *src = paths[i];
        const char *dst = va("%s%s", pathdir, file_name(src));
        errors += file_copy(src, dst) ? 0 : 1;
    }

    if( errors ) PANIC("%d errors found during file dropping", errors);
    else  window_reload();

    (void)window;
}

void window_hints(unsigned flags) {
    #ifdef __APPLE__
    //glfwInitHint( GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE );
    //glfwWindowHint( GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE );
    //glfwWindowHint( GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_FALSE );
    //glfwWindowHint( GLFW_COCOA_MENUBAR, GLFW_FALSE );
    #endif

    #ifdef __APPLE__
    /* We need to explicitly ask for a 3.2 context on OS X */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // osx
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); // osx, 2:#version150,3:330
    #else
    // Compute shaders need 4.5 otherwise. But...
    // According to the GLFW docs, the context version hint acts as a minimum version.
    // i.e, the context you actually get may be a higher or highest version (which is usually the case)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    #endif
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //osx
    #endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //osx+ems
    glfwWindowHint(GLFW_STENCIL_BITS, 8); //osx
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    //glfwWindowHint( GLFW_RED_BITS, 8 );
    //glfwWindowHint( GLFW_GREEN_BITS, 8 );
    //glfwWindowHint( GLFW_BLUE_BITS, 8 );
    //glfwWindowHint( GLFW_ALPHA_BITS, 8 );
    //glfwWindowHint( GLFW_DEPTH_BITS, 24 );

    //glfwWindowHint(GLFW_AUX_BUFFERS, Nth);
    //glfwWindowHint(GLFW_STEREO, GL_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

    // Prevent fullscreen window minimize on focus loss
    glfwWindowHint( GLFW_AUTO_ICONIFY, GL_FALSE );

    // Fix SRGB on intels
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // makes it non-resizable
    if(flags & WINDOW_MSAA2) glfwWindowHint(GLFW_SAMPLES, 2); // x2 AA
    if(flags & WINDOW_MSAA4) glfwWindowHint(GLFW_SAMPLES, 4); // x4 AA
    if(flags & WINDOW_MSAA8) glfwWindowHint(GLFW_SAMPLES, 8); // x8 AA

    g->flags = flags;
}

struct nk_glfw *window_handle_nkglfw() {
    return g->nk_glfw;
}

void glNewFrame() {
    // @transparent debug
    // if( input_down(KEY_F1) ) window_transparent(window_has_transparent()^1);
    // if( input_down(KEY_F2) ) window_maximize(window_has_maximize()^1);
    // @transparent debug

#if 0 // is(ems)
    int canvasWidth, canvasHeight;
    emscripten_get_canvas_element_size("#canvas", &canvasWidth, &canvasHeight);
    w = canvasWidth;
    h = canvasHeight;
    //printf("%dx%d\n", w, h);
#else
    //glfwGetWindowSize(window, &w, &h);
    glfwGetFramebufferSize(window, &w, &h);
    //printf("%dx%d\n", w, h);
#endif

    g->width = w;
    g->height = h;

    // blending defaults
    glEnable(GL_BLEND);

    // culling defaults
//  glEnable(GL_CULL_FACE);
//  glCullFace(GL_BACK);
//  glFrontFace(GL_CCW);

    // depth-testing defaults
    glEnable(GL_DEPTH_TEST);
//  glDepthFunc(GL_LESS);

    // depth-writing defaults
//  glDepthMask(GL_TRUE);

    // seamless cubemaps
//  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glViewport(0, 0, window_width(), window_height());

    // GLfloat bgColor[4]; glGetFloatv(GL_COLOR_CLEAR_VALUE, bgColor);
    glClearColor(winbgcolor.r, winbgcolor.g, winbgcolor.b, window_has_transparent() ? 0 : winbgcolor.a); // @transparent
    //glClearColor(0.15,0.15,0.15,1);
    //glClearColor( clearColor.r, clearColor.g, clearColor.b, clearColor.a );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
}

bool window_create_from_handle(void *handle, float scale, unsigned flags) {
    glfw_init();
    fwk_init();
    if(!t) t = glfwGetTime();

    #if is(ems)
    scale = 100.f;
    #endif

    if( flag("--fullscreen") ) scale = 100;
    scale = (scale < 1 ? scale * 100 : scale);

    bool FLAGS_FULLSCREEN = scale > 100;
    bool FLAGS_FULLSCREEN_DESKTOP = scale == 100;
    bool FLAGS_WINDOWED = scale < 100;
    bool FLAGS_TRANSPARENT = flag("--transparent") || (flags & WINDOW_TRANSPARENT);
    if( FLAGS_TRANSPARENT ) FLAGS_FULLSCREEN = 0, FLAGS_FULLSCREEN_DESKTOP = 0, FLAGS_WINDOWED = 1;
    scale = (scale > 100 ? 100 : scale) / 100.f;
    int winWidth = window_canvas().w * scale;
    int winHeight = window_canvas().h * scale;

    window_hints(flags);

    GLFWmonitor* monitor = NULL;
    #ifndef __EMSCRIPTEN__
    if( FLAGS_FULLSCREEN || FLAGS_FULLSCREEN_DESKTOP ) {
        monitor = glfwGetPrimaryMonitor();
    }
    if( FLAGS_FULLSCREEN_DESKTOP ) {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        winWidth = mode->width;
        winHeight = mode->height;
    }
    if( FLAGS_WINDOWED ) {
        #if !is(ems)
        if( FLAGS_TRANSPARENT ) { // @transparent
            //glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE); // see through. requires undecorated
            //glfwWindowHint(GLFW_FLOATING, GLFW_TRUE); // always on top
            glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        }
        #endif
        // windowed
        float ratio = (float)winWidth / (winHeight + !winHeight);
        if( flags & WINDOW_SQUARE )    winWidth = winHeight = winWidth > winHeight ? winHeight : winWidth;
        //if( flags & WINDOW_LANDSCAPE ) if( winWidth < winHeight ) winHeight = winWidth * ratio;
        if( flags & WINDOW_PORTRAIT )  if( winWidth > winHeight ) winWidth = winHeight * (1.f / ratio);
    }
    #endif

    window = handle ? handle : glfwCreateWindow(winWidth, winHeight, "", monitor, NULL);
    if( !window ) return PANIC("GLFW Window creation failed"), false;

    glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    if( flags & WINDOW_FIXED ) { // disable resizing
        glfwSetWindowSizeLimits(window, w, h, w, h);
    }
    if( flags & (WINDOW_SQUARE | WINDOW_PORTRAIT | WINDOW_LANDSCAPE | WINDOW_ASPECT) ) { // keep aspect ratio
        window_aspect_lock(w, h);
    }

    #ifndef __EMSCRIPTEN__
    if( FLAGS_WINDOWED ) {
        // center window
        monitor = monitor ? monitor : glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        int area_width = mode->width, area_height = mode->height;
        glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &area_width, &area_height);
        glfwSetWindowPos(window, xpos = xpos + (area_width - winWidth) / 2, ypos = ypos + (area_height - winHeight) / 2);
        //printf("%dx%d @(%d,%d) [res:%dx%d]\n", winWidth, winHeight, xpos,ypos, area_width, area_height );

        wprev = w, hprev = h;
        xprev = xpos, yprev = ypos;
    }
    #endif

    glfwMakeContextCurrent(window);

    #if is(ems)
    if( FLAGS_FULLSCREEN ) window_fullscreen(1);
    #else
    int gl_version = gladLoadGL(glfwGetProcAddress);
    #endif

    glDebugEnable();
    
    // setup nuklear ui
    ui_ctx = nk_glfw3_init(&nk_glfw, window, NK_GLFW3_INSTALL_CALLBACKS);
   
    //glEnable(GL_TEXTURE_2D);

    // 0:disable vsync, 1:enable vsync, <0:adaptive (allow vsync when framerate is higher than syncrate and disable vsync when framerate drops below syncrate)
    flags |= optioni("--vsync", 1) || flag("--vsync") ? WINDOW_VSYNC : WINDOW_VSYNC_DISABLED;
    flags |= optioni("--vsync-adaptive", 0) || flag("--vsync-adaptive") ? WINDOW_VSYNC_ADAPTIVE : 0;
    int has_adaptive_vsync = glfwExtensionSupported("WGL_EXT_swap_control_tear") || glfwExtensionSupported("GLX_EXT_swap_control_tear") || glfwExtensionSupported("EXT_swap_control_tear");
    int wants_adaptive_vsync = (flags & WINDOW_VSYNC_ADAPTIVE);
    int interval = has_adaptive_vsync && wants_adaptive_vsync ? -1 : (flags & WINDOW_VSYNC_DISABLED ? 0 : 1);
    glfwSwapInterval(interval);

    const GLFWvidmode *mode = glfwGetVideoMode(monitor ? monitor : glfwGetPrimaryMonitor());
    PRINTF("Monitor: %s (%dHz, vsync=%d)\n", glfwGetMonitorName(monitor ? monitor : glfwGetPrimaryMonitor()), mode->refreshRate, interval);
    PRINTF("GPU device: %s\n", glGetString(GL_RENDERER));
    PRINTF("GPU driver: %s\n", glGetString(GL_VERSION));

    #if !is(ems)
    PRINTF("GPU OpenGL: %d.%d\n", GLAD_VERSION_MAJOR(gl_version), GLAD_VERSION_MINOR(gl_version));

    if( FLAGS_TRANSPARENT ) { // @transparent
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        if( scale >= 1 ) glfwMaximizeWindow(window);
    }
    #endif

    g->ctx = ui_ctx;
    g->nk_glfw = &nk_glfw;
    g->window = window;
    g->width = window_width();
    g->height = window_height();

    // window_cursor(flags & WINDOW_NO_MOUSE ? false : true);
    glfwSetDropCallback(window, window_drop_callback);

    // camera inits for fwk_pre_init() -> ddraw_flush() -> get_active_camera()
    // static camera_t cam = {0}; id44(cam.view); id44(cam.proj); extern camera_t *last_camera; last_camera = &cam;
    fwk_pre_init();

        // display a progress bar meanwhile cook is working in the background
        // Sleep(500);
        if( !COOK_ON_DEMAND )
        if( file_exist(COOK_INI) && cook_jobs() )
        while( cook_progress() < 100 ) {
            for( int frames = 0; frames < 2/*10*/ && window_swap(); frames += cook_progress() >= 100 ) {
                window_title(va("%s %.2d%%", cook_cancelling ? "Aborting" : "Cooking assets", cook_progress()));
                if( input(KEY_ESC) ) cook_cancel();

                glNewFrame();

                static float previous[JOBS_MAX] = {0};

                #define ddraw_progress_bar(JOB_ID, JOB_MAX, PERCENT) do { \
                   /* NDC coordinates (2d): bottom-left(-1,-1), center(0,0), top-right(+1,+1) */ \
                   float progress = (PERCENT+1) / 100.f; if(progress > 1) progress = 1; \
                   float speed = progress < 1 ? 0.2f : 0.5f; \
                   float smooth = previous[JOB_ID] = progress * speed + previous[JOB_ID] * (1-speed); \
                   \
                   float pixel = 2.f / window_height(), dist = smooth*2-1, y = pixel*3*JOB_ID; \
                   if(JOB_ID==0)ddraw_line(vec3(-1,y-pixel*2,0), vec3(1,   y-pixel*2,0)); /* full line */ \
                   ddraw_line(vec3(-1,y-pixel  ,0), vec3(dist,y-pixel  ,0)); /* progress line */ \
                   ddraw_line(vec3(-1,y+0      ,0), vec3(dist,y+0      ,0)); /* progress line */ \
                   ddraw_line(vec3(-1,y+pixel  ,0), vec3(dist,y+pixel  ,0)); /* progress line */ \
                   if(JOB_ID==JOB_MAX-1)ddraw_line(vec3(-1,y+pixel*2,0), vec3(1,   y+pixel*2,0)); /* full line */ \
                } while(0)

                if( FLAGS_TRANSPARENT ) {} else // @transparent
                for(int i = 0; i < cook_jobs(); ++i) ddraw_progress_bar(i, cook_jobs(), jobs[i].progress);
                // ddraw_progress_bar(0, 1, cook_progress());

                ddraw_flush();

                do_once window_visible(1);

                // render progress bar at 30Hz + give the cook threads more time to actually cook the assets.
                // no big deal since progress bar is usually quiet when cooking assets most of the time.
                // also, make the delay even larger when window is minimized or hidden.
                // shaved cook times: 88s -> 57s (tcc), 50s -> 43s (vc)
                sleep_ms( window_has_visible() && window_has_focus() ? 8 : 16 );
            }
            // set black screen
            glNewFrame();
            window_swap();
            window_title("");
        }

    if(cook_cancelling) cook_stop(), exit(-1);

    fwk_post_init(mode->refreshRate);
    return true;
}

bool window_create(float scale, unsigned flags) {
    return window_create_from_handle(NULL, scale, flags);
}

static double boot_time = 0;

char* window_stats() {
    static double num_frames = 0, begin = FLT_MAX, prev_frame = 0;

    double now = time_ss();
    if( boot_time < 0 ) boot_time = now;

    if( begin > now ) {
        begin = now;
        num_frames = 0;
    }
    if( (now - begin) >= 0.25f ) {
        fps = num_frames * (1.f / (now - begin));
    }
    if( (now - begin) > 1 ) {
        begin = now + ((now - begin) - 1);
        num_frames = 0;
    }

    const char *cmdline = app_cmdline();

    // @todo: print %used/%avail kib mem, %used/%avail objs as well
    static char buf[256];
    snprintf(buf, 256, "%s | boot %.2fs | %5.2ffps (%.2fms)%s%s", title, !boot_time ? now : boot_time, fps, (now - prev_frame) * 1000.f, cmdline[0] ? " | ":"", cmdline[0] ? cmdline:"");

    prev_frame = now;
    ++num_frames;

    return buf + 3 * (buf[0] == ' ');
}

int window_frame_begin() {
    glfwPollEvents();

    // we cannot simply terminate threads on some OSes. also, aborted cook jobs could leave temporary files on disc.
    // so let's try to be polite: we will be disabling any window closing briefly until all cook is either done or canceled.
    static bool has_cook; do_once has_cook = !COOK_ON_DEMAND && file_exist(COOK_INI) && cook_jobs();
    if( has_cook ) {
        has_cook = cook_progress() < 100;
        if( glfwWindowShouldClose(g->window) ) cook_cancel();
        glfwSetWindowShouldClose(g->window, GLFW_FALSE);
    }

    if( glfwWindowShouldClose(g->window) ) {
        return 0;
    }

    glNewFrame();

    ui_create();

    bool may_render_stats = 1;

    int has_menu = ui_has_menubar();
    if( !has_menu ) {
        static int cook_on_demand; do_once cook_on_demand = COOK_ON_DEMAND;
        if( !cook_on_demand ) {
            // render profiler, unless we are in the cook progress screen
            static unsigned frames = 0; if(frames <= 0) frames += cook_progress() >= 100;
            may_render_stats = (frames > 0);
        }
    }

    // @transparent
    static bool has_transparent_attrib = 0; ifndef(ems, do_once has_transparent_attrib = glfwGetWindowAttrib(window_handle(), GLFW_TRANSPARENT_FRAMEBUFFER) == GLFW_TRUE);
    if( has_transparent_attrib ) may_render_stats = 0;
    // @transparent

    // generate Debug panel contents
    if( may_render_stats ) {
        if( has_menu ? ui_window("Debug " ICON_MD_SETTINGS, 0) : ui_panel("Debug " ICON_MD_SETTINGS, 0) ) {

#if 1
            static char *filter = 0;
            static int do_filter = 0;
            if( input_down(KEY_F) ) if( input(KEY_LCTRL) || input(KEY_RCTRL) ) do_filter ^= 1;
            int choice = ui_toolbar(ICON_MD_SEARCH ";");
            if( choice == 1 ) do_filter = 1;
            if( do_filter ) {
                ui_string(ICON_MD_CLOSE " Filter " ICON_MD_SEARCH, &filter);
                if( ui_label_icon_clicked_L.x > 0 && ui_label_icon_clicked_L.x <= 24 ) { // if clicked on CANCEL icon (1st icon)
                    do_filter = 0;
                }
            } else {
                if( filter ) filter[0] = '\0';
            }
            char *filter_mask = filter && filter[0] ? va("*%s*", filter) : "*";
#endif

            int open = 0, clicked_or_toggled = 0;

            #define ui_collapse_filtered(lbl,id) (strmatchi(lbl,filter_mask) && ui_collapse(lbl,id))

            for( int p = (open = ui_collapse_filtered(ICON_MD_FOLDER_SPECIAL " Art", "Debug.Art")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                bool inlined = true;
                const char *file = 0;
                if( ui_browse(&file, &inlined) ) {
                    const char *sep = ifdef(win32, "\"", "'");
                    app_exec(va("%s %s%s%s", ifdef(win32, "start \"\"", ifdef(osx, "open", "xdg-open")), sep, file, sep));
                }
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_ROCKET_LAUNCH " AI", "Debug.AI")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                // @todo
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_VOLUME_UP " Audio", "Debug.Audio")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                // @todo
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_VIDEOCAM " Camera", "Debug.Camera")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                ui_camera( camera_get_active() );
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_BUILD " Cook", "Debug.Cook")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                // @todo
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_SIGNAL_CELLULAR_ALT " Network", "Debug.Network")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                // @todo
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_CONTENT_PASTE " Scripts", "Debug.Scripts")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                // @todo
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_MOVIE " FXs", "Debug.FXs")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                ui_fxs();
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_SPEED " Profiler", "Debug.Profiler")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                ui_profiler();
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_STAR_HALF " Shaders", "Debug.Shaders")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                ui_shaders();
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_KEYBOARD " Keyboard", "Debug.Keyboard")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                ui_keyboard();
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_MOUSE " Mouse", "Debug.Mouse")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                ui_mouse();
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_GAMEPAD " Gamepads", "Debug.Gamepads")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                for( int q = 0; q < 4; ++q ) {
                    for( int r = (open = ui_collapse(va("Gamepad #%d",q+1), va("Debug.Gamepads%d",q))), dummy = (clicked_or_toggled = ui_collapse_clicked()); r; ui_collapse_end(), r = 0) {
                        ui_gamepad(q);
                    }
                }
            }
            for( int p = (open = ui_collapse_filtered(ICON_MD_VIEW_QUILT " UI", "Debug.UI")), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
                int choice = ui_toolbar(ICON_MD_RECYCLING " Reset layout;" ICON_MD_SAVE_AS " Save layout");
                if( choice == 1 ) ui_layout_all_reset("*");
                if( choice == 2 ) file_delete(WINDOWS_INI), ui_layout_all_save_disk("*");

                for each_map_ptr_sorted(ui_windows, char*, k, unsigned, v) {
                    bool visible = ui_visible(*k);
                    if( ui_bool( *k, &visible ) ) {
                        ui_show( *k, ui_visible(*k) ^ true );
                    }
                }
            }

            (has_menu ? ui_window_end : ui_panel_end)();
        }
    }
 
#if 0 // deprecated
    // run user-defined hooks
    for(int i = 0; i < 64; ++i) {
        if( hooks[i] ) hooks[i]( userdatas[i] );
    }
#endif

    double now = paused ? t : glfwGetTime();
    dt = now - t;
    t = now;

    char *st = window_stats();
    static double timer = 0;
    timer += window_delta();
    if( timer >= 0.25 ) {
        glfwSetWindowTitle(window, st);
        timer = 0;
    }

    void input_update();
    input_update();

    return 1;
}

void window_frame_end() {
    // flush batching systems that need to be rendered before frame swapping. order matters.
    {
        font_goto(0,0);        
        touch_flush();
        sprite_flush();

        // flush all debugdraw calls before swap
        dd_ontop = 0;
        ddraw_flush();
        glClear(GL_DEPTH_BUFFER_BIT);
        dd_ontop = 1;
        ddraw_flush();

        ui_render();
    }

#if !is(ems)
    // save screenshot if queued
    if( screenshot_file[0] ) {
        int n = 3;
        void *rgb = screenshot(n);
        stbi_flip_vertically_on_write(true);
        if(!stbi_write_png(screenshot_file, w, h, n, rgb, n * w) ) {
            PANIC("!could not write screenshot file `%s`\n", screenshot_file);
        }
        screenshot_file[0] = 0;
    }
    if( record_active() ) {
        void record_frame();
        record_frame();
    }
#endif
}

void window_frame_swap() {
    // glFinish();
#if !is(ems)
    window_vsync(hz);
#endif
    glfwSwapBuffers(window);
    // emscripten_webgl_commit_frame();

    static int delay = 0; do_once delay = optioni("--delay", 0);
    if( delay && !COOK_ON_DEMAND && cook_progress() >= 100 ) sleep_ms( delay );
}

static
void window_shutdown() {
    do_once {
        #if ENABLE_SELFIES

        snprintf(screenshot_file, DIR_MAX, "%s.png", app_name());

        int n = 3;
        void *rgb = screenshot(n);
        stbi_flip_vertically_on_write(true);
        if(!stbi_write_png(screenshot_file, w, h, n, rgb, n * w) ) {
            PANIC("!could not write screenshot file `%s`\n", screenshot_file);
        }
        screenshot_file[0] = 0;

        #endif

        window_loop_exit(); // finish emscripten loop automatically        
    }
}

int window_swap() {

    // end frame
    if( frame_count > 0 ) {
        window_frame_end();
        window_frame_swap();
    }

    ++frame_count;

    // begin frame
    int ready = window_frame_begin();
    if( !ready ) {
        window_shutdown();
        return 0;
    }
    return 1;
}

static
void (*window_render_callback)(void* loopArg);

static vec2 last_canvas_size;

static
void window_resize() {
#if is(ems)
    EM_ASM(canvas.canResize = 0);
    if (g->flags&WINDOW_FIXED) return;
    EM_ASM(canvas.canResize = 1);
    vec2 size = window_canvas();
    do_once last_canvas_size = size;
    if (size.x != last_canvas_size.x || size.y != last_canvas_size.y) {
        w = size.x;
        h = size.y;
        g->width  = w;
        g->height = h;
        glfwSetWindowSize(g->window, w, h);
        // emscripten_set_canvas_size(w, h);
    }
#endif /* __EMSCRIPTEN__ */
}

static
void window_loop_wrapper( void *loopArg ) {
    if( window_frame_begin() ) {
        window_resize();
        window_render_callback(loopArg);
        window_frame_end();
        window_frame_swap();
    } else {
        do_once window_shutdown();
    }
}

void window_loop(void (*user_function)(void* loopArg), void* loopArg ) {
#if is(ems)
    window_render_callback = user_function;
    emscripten_set_main_loop_arg(window_loop_wrapper, loopArg, 0, 1);
#else
    g->keep_running = true;
    while (g->keep_running)
        user_function(loopArg);
#endif /* __EMSCRIPTEN__ */
}

void window_loop_exit() {
#if is(ems)
    emscripten_cancel_main_loop();
#else
    g->keep_running = false;
#endif /* __EMSCRIPTEN__ */
}

vec2 window_canvas() {
#if is(ems)
    int width = EM_ASM_INT_V(return canvas.width || window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth);
    int height = EM_ASM_INT_V(return canvas.height || window.innerHeight || document.documentElement.clientHeight|| document.body.clientHeight);
    return vec2(width, height);
#else
    glfw_init();
    const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
    assert( mode );
    return vec2(mode->width, mode->height);
#endif /* __EMSCRIPTEN__ */
}

int window_width() {
    return w;
}
int window_height() {
    return h;
}
double window_time() {
    return t;
}
double window_delta() {
    return dt;
}

double window_fps() {
    return fps;
}
void window_fps_lock(float fps) {
    hz = fps;
}
void window_fps_unlock() {
    hz = 0;
}
double window_fps_target() {
    return hz;
}

uint64_t window_frame() {
    return frame_count;
}
void window_title(const char *title_) {
    snprintf(title, 128, "%s", title_);
    if( !title[0] ) glfwSetWindowTitle(window, title);
}
void window_color(unsigned color) {
    unsigned b = (color >>  0) & 255;
    unsigned g = (color >>  8) & 255;
    unsigned r = (color >> 16) & 255;
    unsigned a = (color >> 24) & 255;
    winbgcolor = vec4(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
}
void window_icon(const char *file_icon) {
    unsigned len = file_size(file_icon); // len = len ? len : vfs_size(file_icon); // @fixme: reenable this to allow icons to be put in cooked .zipfiles
    if( len ) {
        void *data = file_read(file_icon); // data = data ? data : vfs_read(file_icon); // @fixme: reenable this to allow icons to be put in cooked .zipfiles
        if( data ) {
            image_t img = image_from_mem(data, len, IMAGE_RGBA);
            if( img.w && img.h && img.pixels ) {
                GLFWimage images[1];
                images[0].width = img.w;
                images[0].height = img.h;
                images[0].pixels = img.pixels;
                glfwSetWindowIcon(window, 1, images);
                return;
            }
        }
    }
#if is(win32)
    HANDLE hIcon = LoadImageA(0, file_icon, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    if( hIcon ) {
        HWND hWnd = glfwGetWin32Window(window);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
        SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
        return;
    }
#endif
}
void* window_handle() {
    return window;
}

void window_reload() {
    // @todo: save_on_exit();
    fflush(0);
    chdir(app_path());
    execv(__argv[0], __argv);
    exit(0);
}

int window_record(const char *outfile_mp4) {
    record_start(outfile_mp4);
    // @todo: if( flags & RECORD_MOUSE )
    if( record_active() ) window_cursor_shape(CURSOR_SW_AUTO); else window_cursor_shape(CURSOR_HW_ARROW);
    return record_active();
}


// -----------------------------------------------------------------------------
// fullscreen

static
GLFWmonitor *window_find_monitor(int wx, int wy) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();

    // find best monitor given current window coordinates. @todo: select by ocuppied window area inside each monitor instead.
    int num_monitors = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&num_monitors);
#if is(ems)
    return *monitors;
#else
    for( int i = 0; i < num_monitors; ++i) {
        int mx = 0, my = 0, mw = 0, mh = 0;
        glfwGetMonitorWorkarea(monitors[i], &mx, &my, &mw, &mh);
        monitor = wx >= mx && wx <= (mx+mw) && wy >= my && wy <= (my+mh) ? monitors[i] : monitor;
    }
    return monitor;
#endif
}

#if 0 // to deprecate

void window_fullscreen(int enabled) {
    fullscreen = !!enabled;
#ifndef __EMSCRIPTEN__
    if( fullscreen ) {
        int wx = 0, wy = 0; glfwGetWindowPos(window, &wx, &wy);
        GLFWmonitor *monitor = window_find_monitor(wx, wy);

        wprev = w, hprev = h, xprev = wx, yprev = wy; // save window context for further restoring

        int width, height;
        glfwGetMonitorWorkarea(monitor, NULL, NULL, &width, &height);
        glfwSetWindowMonitor(window, monitor, 0, 0, width, height, GLFW_DONT_CARE);
    } else {
        glfwSetWindowMonitor(window, NULL, xpos, ypos, wprev, hprev, GLFW_DONT_CARE);
        glfwSetWindowPos(window, xprev, yprev);
    }
#endif
}
int window_has_fullscreen() {
    return fullscreen;
}

#else

int window_has_fullscreen() {
#if is(ems)
    EmscriptenFullscreenChangeEvent fsce;
    emscripten_get_fullscreen_status(&fsce);
    return !!fsce.isFullscreen;
#else
    return !!glfwGetWindowMonitor(g->window);
#endif /* __EMSCRIPTEN__ */
}

void window_fullscreen(int enabled) {
    if( window_has_fullscreen() == !!enabled ) return;

#if is(ems)

#if 0 // deprecated: crash
    if( enabled ) {
        emscripten_exit_soft_fullscreen();

        /* Workaround https://github.com/kripken/emscripten/issues/5124#issuecomment-292849872 */
        EM_ASM(JSEvents.inEventHandler = true);
        EM_ASM(JSEvents.currentEventHandler = {allowsDeferredCalls:true});

        EmscriptenFullscreenStrategy strategy = {0};
        strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH; // _ASPECT
        strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF; // _NONE _HIDEF
        strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT; // _NEAREST

        emscripten_request_fullscreen_strategy(NULL, EM_FALSE/*EM_TRUE*/, &strategy);
        //emscripten_enter_soft_fullscreen(NULL, &strategy);
    } else {
        emscripten_exit_fullscreen();
    }
#else
    if( enabled )
    EM_ASM(Module.requestFullscreen(1, 1)); 
    else
    EM_ASM(Module.exitFullscreen()); 
#endif

#else

#if 0
    if( enabled ) {
        /*glfwGetWindowPos(g->window, &g->window_xpos, &g->window_ypos);*/
        glfwGetWindowSize(g->window, &g->width, &g->height);
        glfwSetWindowMonitor(g->window, glfwGetPrimaryMonitor(), 0, 0, g->width, g->height, GLFW_DONT_CARE);
    } else {            
        glfwSetWindowMonitor(g->window, NULL, 0, 0, g->width, g->height, GLFW_DONT_CARE);
    }
#else
    if( enabled ) {
        int wx = 0, wy = 0; glfwGetWindowPos(window, &wx, &wy);
        GLFWmonitor *monitor = window_find_monitor(wx, wy);

        wprev = w, hprev = h, xprev = wx, yprev = wy; // save window context for further restoring

        int width, height;
        glfwGetMonitorWorkarea(monitor, NULL, NULL, &width, &height);
        glfwSetWindowMonitor(window, monitor, 0, 0, width, height, GLFW_DONT_CARE);
    } else {
        glfwSetWindowMonitor(window, NULL, xpos, ypos, wprev, hprev, GLFW_DONT_CARE);
        glfwSetWindowPos(window, xprev, yprev);
    }
#endif

#endif
}

#endif

void window_pause(int enabled) {
    paused = enabled;
}
int window_has_pause() {
    return paused;
}
void window_focus() {
    glfwFocusWindow(window);
}
int window_has_focus() {
    return !!glfwGetWindowAttrib(window, GLFW_FOCUSED);
}
void window_cursor(int visible) {
    glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}
int window_has_cursor() {
    return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}
void window_cursor_shape(unsigned mode) {
    static GLFWcursor* cursors[7] = { 0 };
    static unsigned enums[7] = {
        0,
        GLFW_ARROW_CURSOR,
        GLFW_IBEAM_CURSOR,
        GLFW_CROSSHAIR_CURSOR,
        GLFW_HAND_CURSOR,
        GLFW_HRESIZE_CURSOR,
        GLFW_VRESIZE_CURSOR,
    };
    do_once {
        static unsigned pixels[16 * 16] = { 0x01000000 }; // ABGR(le) glfw3 note: A(0x00) means 0xFF for some reason
        static GLFWimage image = { 16, 16, (void*)pixels };
        static GLFWcursor* empty;
        for( int x = 0; x < 16 * 16; ++x ) pixels[x] = pixels[0];
        empty = glfwCreateCursor(&image, 0, 0);

        for(int i = 0; i < countof(enums); ++i) cursors[i] = i ? glfwCreateStandardCursor( enums[i] ) : empty;
    }
    if( mode == CURSOR_SW_AUTO ) { // UI (nuklear) driven cursor
        nk_style_show_cursor(ui_handle());
        glfwSetCursor(window, cursors[0] );
    } else {
        nk_style_hide_cursor(ui_handle());
        glfwSetCursor(window, mode < countof(enums) ? cursors[mode] : NULL);
    }
}
void window_visible(int visible) {
    if(!window) return;
    //if(window) (visible ? glfwRestoreWindow : glfwIconifyWindow)(window);
    (visible ? glfwShowWindow : glfwHideWindow)(window);
    // call glfwpollevents in linux to flush visiblity changes that would happen in next frame otherwise
    #if is(linux) || is(osx)
    glfwPollEvents();
    #endif
}
int window_has_visible() {
    return glfwGetWindowAttrib(window, GLFW_VISIBLE);
}

void window_screenshot(const char* outfile_png) {
    snprintf(screenshot_file, DIR_MAX, "%s", outfile_png ? outfile_png : "");
}

double window_aspect() {
    return (double)w / (h + !h);
}
void window_aspect_lock(unsigned numer, unsigned denom) {
    if(!window) return;
    if( numer * denom ) {
        glfwSetWindowAspectRatio(window, numer, denom);
    } else {
        glfwSetWindowAspectRatio(window, GLFW_DONT_CARE, GLFW_DONT_CARE);
    }
}
void window_aspect_unlock() {
    if(!window) return;
    window_aspect_lock(0, 0);
}

void window_transparent(int enabled) {
    #if !is(ems)
    if( !window_has_fullscreen() ) {
        if( enabled ) {
            glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
            //glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH , GLFW_TRUE);
            //glfwMaximizeWindow(window);
        } else {
            //glfwRestoreWindow(window);
            //glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH , GLFW_FALSE);
            glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        }
    }
    #endif
}
int window_has_transparent() {
    return ifdef(ems, 0, glfwGetWindowAttrib(window, GLFW_DECORATED) != GLFW_TRUE);
}

void window_maximize(int enabled) {
    ifdef(ems, return);
    if( !window_has_fullscreen() ) {
        if( enabled ) {
            glfwMaximizeWindow(window);
        } else {
            glfwRestoreWindow(window);
        }
    }
}
int window_has_maximize() {
    return ifdef(ems, 0, glfwGetWindowAttrib(window, GLFW_MAXIMIZED) == GLFW_TRUE);
}


#line 0

#line 1 "fwk_obj.c"
void *obj_initialize( void **ptr, char *type ) {
    unsigned payload = ((unsigned char)type[0]) << 8; type[0] = '\1'; // @fixme: actually use this '\1' custom tag
#ifndef NDEBUG
    strcatf(&type, "%s", callstack(+16)); // debug: for debugging purposes only
#endif
    ptr[0] = OBJBOX((void *)type, payload);

    return &ptr[1];
}

void obj_free( void *obj ) {
    void *ptr = *((void**)obj - 1);
    FREE( OBJUNBOX(ptr) );
    obj = (void**)obj - 1;
    FREE( obj );
}

void obj_del( void *obj ) {
    unsigned type = obj_typeid(obj);
    (type[dtor])(obj);
    obj_free( obj );
}

unsigned obj_instances( const void *obj ) {
    return OBJPAYLOAD3(obj) + 1;
}

// ---

const char *obj_output( const void *obj ) {
    void* ptr = *((void**)obj - 1);
    char *str = OBJUNBOX(ptr);

    while( *str != '\n' ) ++str;
    return (const char *)str + 1;
}
void (obj_printf)( void *obj, const char *text ) {
    void* ptr = *((void**)obj - 1);
    char* str = OBJUNBOX(ptr);
    unsigned payload = OBJPAYLOAD16(ptr);

    strcatf(&str, "%s", text);

    *((void**)obj - 1) = OBJBOX(str, payload);
}

// ---

const char *obj_typeof( const void *obj ) {
    void* ptr = *((void**)obj - 1);
    ptr = OBJUNBOX(ptr);

    char name[256]; sscanf((const char*)ptr + 1, "%[^\n]", name); // @fixme: overflow
    return va("%s", name);
}

unsigned obj_typeid(const void *obj) {
    void* ptr = *((void**)obj - 1);
    unsigned payload = OBJPAYLOAD16(ptr);
    return payload >> 8;
}

bool obj_typeeq( const void *obj1, const void *obj2 ) {
    if( obj_typeid(obj1) != obj_typeid(obj2) ) return false;
    return !strcmp( obj_typeof(obj1), obj_typeof(obj2) );
}

unsigned obj_typeid_from_name(const char *name) {
    name += strbegi(name, "struct ") ? 8 : strbegi(name, "union ") ? 7 : 0;
    unsigned typeid = hash_str(name) & 255; // @fixme: increase bits / decrease colliders (256 types only!)
    ASSERT( typeid, "Name of given class has an empty (zeroed) hash. Limitation by design" );

    static map(unsigned, char *) registered; // @fixme: add mutex
    do_once map_init(registered, less_int, hash_int);
    do_once map_insert(registered, 1, "(typeless)");
    char **found = map_find(registered, typeid);
    if(!found) map_insert(registered, typeid, STRDUP(name));
    else ASSERT( !strcmp(name, *found), "Uh-oh, types collided. Please rename one of these two classes '%s'/'%s'", name, *found);

    return typeid;
}

// ---

unsigned obj_sizeof(const void *obj) {
    void *ptr = (void**)obj - 1;
    return ALLOCSIZE(ptr) - sizeof(void*);
}

void obj_zero(void *obj) {
    // clear console log
    void* ptr = *((void**)obj - 1);
    char* str = OBJUNBOX(ptr);
    if( str[0] ) {
        unsigned payload = OBJPAYLOAD16(ptr);

        unsigned namelen = strlen(obj_typeof(obj));
        str = REALLOC(str, 1+namelen+2); // preserve \1+name+\n+\0
        str[1+namelen+2-1] = '\0';

        *((void**)obj - 1) = OBJBOX(str, payload);
    }

    // reset data
    dtor(obj);
    memset(obj, 0, obj_sizeof(obj));
    ctor(obj);
}

void obj_hexdumpf(FILE *out, const void *obj) {
    hexdumpf(out, obj, obj_sizeof(obj), 16);

    const char *output = obj_output(obj);
    fprintf( out, "; ptr=[%p] sizeof=%d typeof=%s typeid=%#x refs=%d\n%s%s\n",
        obj, obj_sizeof(obj), obj_typeof(obj), obj_typeid(obj), (int)(OBJPAYLOAD16(obj) & 0xFF),
        output[0] ? output : "(no output)", output[0] ? "---" : "");
}

void obj_hexdump(const void *obj) {
    obj_hexdumpf( stdout, obj );
}

// object: load/save

unsigned obj_load_buffer(void *obj, const void *src, unsigned srclen) {
    unsigned objlen = obj_sizeof(obj);
    if( srclen > objlen ) return 0; // @fixme: do something clever
    memcpy(obj, src, srclen); // @fixme: do something clever
    return objlen;
}
unsigned obj_load(void *obj, const array(char) buffer) {
    unsigned bytes = buffer ? obj_load_buffer(obj, buffer, array_count((char*)buffer)) : 0;
    return bytes;
}
unsigned obj_load_file(void *obj, FILE *fp) {
    unsigned len = obj_sizeof(obj);
    char *buffer = va("%*.s", len, "");
    unsigned read = fread(buffer, 1, len, fp);
    if( read != (1*len) ) {
        return 0;
    }
    unsigned bytes = obj_load_buffer(obj, buffer, len);
    return bytes;
}

unsigned obj_save_buffer(void *dst, unsigned cap, const void *obj) {
    unsigned len = obj_sizeof(obj);
    if( len > cap ) return 0;
    memcpy(dst, obj, len); // @fixme: do something clever
    return len;
}
array(char) obj_save(const void *obj) { // empty if error. must array_free() after use
    array(char) data = 0;
    unsigned len = obj_sizeof(obj);
    array_resize(data, len);
    unsigned bytes = obj_save_buffer(data, len, obj);
    array_resize(data, bytes);
    return data;
}
unsigned obj_save_file(FILE *fp, const void *obj) {
    unsigned len = obj_sizeof(obj);
    char *buffer = va("%*.s", len, "");
    unsigned bytes = obj_save_buffer(buffer, len, obj);
    if( bytes > 0 ) {
        unsigned written = fwrite(buffer, 1, len, fp);
        if( written == (1*len) ) {
            return written;
        }
    }
    return 0; // error
}

// ---

static int __thread global_ref_count; // @fixme: make it atomic

static void objref_check_atexit(void) {
    if(global_ref_count > 0) fprintf(stderr, "Warn! Possible memory_leaks: %d refs not destroyed\n", global_ref_count);
    if(global_ref_count < 0) fprintf(stderr, "Warn! Possible double free: %d refs double destroyed\n", -global_ref_count);
}

void* obj_ref(void *obj) {
    do_once atexit(objref_check_atexit);

    if( obj ) {
        void *ptr = *((void**)obj - 1);
        unsigned payload = OBJPAYLOAD16(ptr);
        unsigned ref_count = payload & 255;
        ASSERT(ref_count < 255, "Object cannot hold more than 256 refs. Limitation by design.");

        *((void**)obj - 1) = OBJBOX(OBJUNBOX(ptr), payload + 1);
        global_ref_count++;
    }

    return obj;
}

void* obj_unref(void *obj) {
    if( obj ) {
        void *ptr = *((void**)obj - 1);
        unsigned payload = OBJPAYLOAD16(ptr);
        unsigned ref_count = payload & 255;

        *((void**)obj - 1) = OBJBOX(OBJUNBOX(ptr), payload - 1);
        global_ref_count--;

        if( ref_count <= 1 ) {
            obj_del(obj);
            return 0;
        }
    }

    return obj;
}

// ---

void dummy1() {}
#define dummy8   dummy1,dummy1,dummy1,dummy1,dummy1,dummy1,dummy1,dummy1
#define dummy64  dummy8,dummy8,dummy8,dummy8,dummy8,dummy8,dummy8,dummy8
#define dummy256 dummy64,dummy64,dummy64,dummy64

void (*dtor[256])() = { dummy256 };
void (*ctor[256])() = { dummy256 };

// ---

static set(uintptr_t) vtables; // @fixme: add mutex

void (obj_override)(const char *objclass, void (**vtable)(), void(*fn)()) {
    do_once set_init(vtables, less_64, hash_64);
    set_find_or_add(vtables, (uintptr_t)vtable);

    vtable[ obj_typeid_from_name(objclass) ] = fn;
}

void (obj_extend)(const char *dstclass, const char *srcclass) { // wip, @testme
    unsigned dst_id = obj_typeid_from_name(dstclass);
    unsigned src_id = obj_typeid_from_name(srcclass);

    // iterate src vtables, and assign them to dst
    if(dst_id != src_id)
    for each_set(vtables, void **, src_table) {
        if( src_table[src_id] ) {
            src_table[dst_id] = (void(*)()) (src_table[ src_id ]);
        }
    }
}

// ---

void *obj_clone(const void *obj) { // @fixme: clone object console as well?
    unsigned sz = obj_sizeof(obj);
    const char *nm = obj_typeof(obj);
    unsigned id = obj_typeid(obj);

    void *obj2 = obj_initialize((void**)MALLOC(sizeof(void*)+sz), stringf("%c%s\n" "cloned" "\n", id, nm)); // STRDUP( OBJUNBOX(*((void**)obj - 1)) ));
    memcpy(obj2, obj, sz);
    return obj2;
}

void *obj_copy(void **dst, const void *src) {
    if(!*dst) return *dst = obj_clone(src);

    if( obj_typeeq(*dst, src) ) {
        return memcpy(*dst, src, obj_sizeof(src));
    }

    return NULL;
}

void *obj_mutate(void **dst_, const void *src) {
    // mutate a class. ie, convert a given object class into a different one,
    // while preserving the original metas and references as much as possible.
    //
    // @fixme: systems might be tracking objects in the future. the fact that we
    // can reallocate a pointer (and hence, change its address) seems way too dangerous,
    // as the tracking systems could crash when referencing a mutated object.
    // solutions: do not reallocate if sizeof(new_class) > sizeof(old_class) maybe? good enough?
    // also, optimization hint: no need to reallocate if both sizes matches, just copy contents.

    if(!*dst_) return *dst_ = obj_clone(src);

    void *dst = *dst_;
    dtor(dst);

        unsigned src_sz = obj_sizeof(src);
        unsigned src_id = obj_typeid(src);

        void *dst_ptr = *((void**)dst - 1);
        unsigned payload = (OBJPAYLOAD16(dst_ptr) & 255) | src_id << 8;
        FREE( OBJUNBOX(dst_ptr) );
        *((void**)dst - 1) = OBJBOX( STRDUP( OBJUNBOX(*((void**)src - 1)) ), payload);

        void *base = (void*)((void**)dst - 1);
        base = REALLOC(base, src_sz + sizeof(void*));
        *dst_ = (char*)base + sizeof(void*);
        dst = (char*)base + sizeof(void*);
        memcpy(dst, src, src_sz);

    ctor(dst);
    return dst;
}


#ifdef OBJ_DEMO

typedef struct MyObject {
    char* id;
    int x,y;
    float rotation;
    struct MyObject *next;
} MyObject;

void tests1() {
    // Construct two objects
    MyObject *root = obj_new(MyObject, 0);
    MyObject *obj = obj_new(MyObject, "An identifier!", 0x11, 0x22, 3.1415f, root );

    // Log some lines
    obj_printf(root, "this is a logline #1\n");
    obj_printf(root, "this is a logline #2\n");
    obj_printf(root, "this is a logline #3\n");

    obj_printf(obj, "yet another logline #1\n");
    obj_printf(obj, "yet another logline #2\n");

    // Dump contents of our objects

    obj_hexdump(root);
    obj_hexdump(obj);

    // Save to mem

    array(char) buffer = obj_save(obj);
    printf("%d bytes\n", (int)array_count(buffer));

    // Clear

    obj_zero( obj );
    obj_hexdump( obj );

    // Reload

    obj_load( obj, buffer );
    obj_hexdump( obj );

    // Copy tests

    {
        MyObject *clone = obj_clone(obj);
        obj_hexdump(clone);
        obj_del(clone);
    }

    {
        MyObject *copy = 0;
        obj_copy(&copy, obj);
        obj_hexdump(copy);
        obj_del(copy);
    }

    {
        MyObject *copy = obj_new(MyObject, "A different identifier!", 0x33, 0x44, 0.0f, root );
        obj_copy(&copy, obj);
        obj_hexdump(copy);
        obj_del(copy);
    }

    {
        void *copy = obj_malloc(100, "an untyped class" );
        obj_mutate(&copy, obj);
        obj_hexdump(copy);
        obj_copy(&copy, obj);
        obj_hexdump(copy);
        obj_del(copy);
    }

    // Benchmarking call overhead.
    // We're here using dtor as a method to test. Since there is actually no
    // destructor associated to this class, it will be safe to call it extensively (no double frees).
    //
    // results:
    // 427 million calls/s @ old i5-4300/1.90Ghz laptop. compiled with "cl /Ox /Os /MT /DNDEBUG /GL /GF /arch:AVX2"

    #ifndef N
    #define N (INT32_MAX-1)
    #endif

    double t = (puts("benchmarking..."), -clock() / (double)CLOCKS_PER_SEC);
    for( int i = 0; i < N; ++i ) {
        dtor(root);
    }
    t += clock() / (double)CLOCKS_PER_SEC;
    printf("Benchmark: %5.2f objcalls/s %5.2fM objcalls/s\n", N/(t), (N/1000)/(t*1000)); // ((N+N)*5) / (t) );

}

// --------------

#define dump(obj) dump[obj_typeid(obj)](obj)
#define area(obj) area[obj_typeid(obj)](obj)

extern void  (*dump[256])();
extern float (*area[256])();

void  (*dump[256])() = {0};
float (*area[256])() = {0};

// --------------

typedef struct box {
    float w;
} box;

void  box_ctor(box *b) { printf("box already constructed: box-w:%f\n", b->w); }
void  box_dtor(box *b) { puts("deleting box..."); }
void  box_dump(box *b) { printf("box-w:%f\n", b->w); }
float box_area(box *b) { return b->w * b->w; }

#define REGISTER_BOX \
    obj_override(box, ctor); \
    obj_override(box, dump); \
    obj_override(box, area); \
    obj_override(box, dtor);

typedef struct rect {
    float w, h;
} rect;

void  rect_dump(rect *r) { printf("rect-w:%f rect-h:%f\n", r->w, r->h); }
float rect_area(rect *r) { return r->w * r->h; }
void  rect_dtor(rect *r) { puts("deleting rect..."); }

#define REGISTER_RECT \
    obj_override(rect, dump); \
    obj_override(rect, area); \
    obj_override(rect, dtor);

void tests2() {
    REGISTER_BOX
    REGISTER_RECT

    box *b = obj_new(box, 100);
    rect *r = obj_new(rect, 100, 200);

    dump(b);
    dump(r);

    printf("%f\n", area(b));
    printf("%f\n", area(r));

    obj_del(b);
    obj_ref(r); obj_unref(r); //obj_del(r);

    int *untyped = obj_malloc( sizeof(int) );
    int *my_number = obj_malloc( sizeof(int), "a comment about my_number" );
    char *my_text = obj_malloc( 32, "some debug info here" );

    *untyped = 100;
    *my_number = 123;
    sprintf( my_text, "hello world" );

    struct my_bitmap { int w, h, bpp; const char *pixels; };
    struct my_bitmap *my_bitmap = obj_new(struct my_bitmap, 2,2,8, "\1\2\3\4");

    printf( "%p(%s,%u)\n", my_bitmap, obj_typeof(my_bitmap), obj_typeid(my_bitmap) );
    printf( "%d(%s,%d)\n", *untyped, obj_typeof(untyped), obj_typeid(untyped) );
    printf( "%d(%s,%d)\n", *my_number, obj_typeof(my_number), obj_typeid(my_number) );
    printf( "%s(%s,%d)\n", my_text, obj_typeof(my_text), obj_typeid(my_text) );

    obj_printf(my_text, "hello world #1\n");
    obj_printf(my_text, "hello world #2\n");
    puts(obj_output(my_text));

    printf( "%s(%s,%d)\n", my_text, obj_typeof(my_text), obj_typeid(my_text) );

    printf( "equal?:%d\n", obj_typeeq(my_number, untyped) );
    printf( "equal?:%d\n", obj_typeeq(my_number, my_number) );
    printf( "equal?:%d\n", obj_typeeq(my_number, my_text) );
    printf( "equal?:%d\n", obj_typeeq(my_number, my_bitmap) );

    obj_free( untyped );
    obj_free( my_text );
    obj_free( my_bitmap );
    obj_del( my_number ); // should not crash, even if allocated with obj_malloc()
}

int main() {
    tests1();
    tests2();
    puts("ok");
}

/*
    MEMBER( MyObject, char*, id );
    MEMBER( MyObject, int, x );
    MEMBER( MyObject, int, y );
    MEMBER( MyObject, float, rotation, "(degrees)" );
    MEMBER( MyObject, MyObject*, next, "(linked list)" );
*/

#define main main__
#endif
#line 0

#line 1 "fwk_ai.c"
// AI framework
// - rlyeh, public domain.
//
// [src] original A-star code by @mmozeiko (PD) - https://gist.github.com/mmozeiko/68f0a8459ef2f98bcd879158011cc275
// [src] original swarm/boids code by @Cultrarius (UNLICENSE) - https://github.com/Cultrarius/Swarmz

// boids/swarm -----------------------------------------------------------------

vec3 rnd3() { // random uniform
    float theta = randf() * C_PI * 2;
    float r = sqrt(randf());
    float z = sqrt(1.0f - r * r) * (randf() > 0.5f ? -1.0f : 1.0f);
    return vec3(r * cos(theta), r * sin(theta), z);
}
int less3(vec3 *lhs, vec3 *rhs) {
    if(lhs->x != rhs->x) return lhs->x - rhs->x;
    if(lhs->y != rhs->y) return lhs->y - rhs->y;
    if(lhs->z != rhs->z) return lhs->z - rhs->z;
    return 0;
}
uint64_t hash3(vec3 *v) {
    uint64_t h1 = hash_flt(v->x);
    uint64_t h2 = hash_flt(v->y);
    uint64_t h3 = hash_flt(v->z);
    return (h1 * 31 + h2) * 31 + h3;
}
vec3 clamplen3(vec3 v, float length) {
    return len3(v) <= length ? v : scale3(norm3(v), length);
}

float transform_distance(float distance, SWARM_DISTANCE type) {
    float quad;
    /**/ if (type == SWARM_DISTANCE_LINEAR)            return distance;
    else if (type == SWARM_DISTANCE_QUADRATIC)         return distance * distance;
    else if (type == SWARM_DISTANCE_INVERSE_LINEAR)    return distance == 0 ? 0 : 1 / distance;
    else if (type == SWARM_DISTANCE_INVERSE_QUADRATIC) return (quad = distance * distance), (quad == 0 ? 0 : 1 / quad);
    return distance; // throw exception instead?
}

typedef struct nearby_boid_t {
    boid_t *boid;
    vec3 direction;
    float distance;
} nearby_boid_t;

static
vec3 get_voxel_for_boid(float perception_radius, const boid_t *b) { // quantize position
    float r = absf(perception_radius);
    return vec3( (int)(b->position.x / r), (int)(b->position.y / r), (int)(b->position.z / r) );
}

static
void check_voxel_for_boids(float perception_radius, float blindspot_angledeg_compare_value, array(boid_t*) voxel_cached, array(nearby_boid_t) *result, const vec3 voxelPos, const boid_t *b) {
    for each_array_ptr(voxel_cached, const boid_t*, test) {
        vec3 p1 = b->position;
        vec3 p2 = (*test)->position;
        vec3 vec = sub3(p2, p1);
        float distance = len3(vec);

        float compare_value = 0;
        float l1 = len3(vec);
        float l2 = len3(b->velocity);
        if (l1 != 0 && l2 != 0) {
            compare_value = dot3(neg3(b->velocity), vec) / (l1 * l2);
        }

        if ((&b) != test && distance <= perception_radius && (blindspot_angledeg_compare_value > compare_value || len3(b->velocity) == 0)) {
            nearby_boid_t nb;
            nb.boid = (boid_t*)*test;
            nb.distance = distance;
            nb.direction = vec;
            array_push(*result, nb);
        }
    }
}

static
array(nearby_boid_t) get_nearby_boids(const swarm_t *self, const boid_t *b) {
    array(nearby_boid_t) result = 0;
    array_reserve(result, array_count(self->boids));

    vec3 voxelPos = get_voxel_for_boid(self->perception_radius, b);
    voxelPos.x -= 1;
    voxelPos.y -= 1;
    voxelPos.z -= 1;
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                array(boid_t*) *found = map_find(self->voxel_cache_, &voxelPos);
                if( found ) check_voxel_for_boids(self->perception_radius, self->blindspot_angledeg_compare_value_, *found, &result, voxelPos, b);
                voxelPos.z++;
            }
            voxelPos.z -= 3;
            voxelPos.y++;
        }
        voxelPos.y -= 3;
        voxelPos.x++;
    }
    return result;
}

static
void update_boid(swarm_t *self, boid_t *b) {
    vec3 separation_sum = {0};
    vec3 heading_sum = {0};
    vec3 position_sum = {0};
    vec3 po = b->position;

    array(nearby_boid_t) nearby = get_nearby_boids(self, b); // @leak

    for each_array_ptr(nearby, nearby_boid_t, closeboid_t) {
        if (closeboid_t->distance == 0) {
            separation_sum = add3(separation_sum, scale3(rnd3(), 1000)); //addscale3
        }
        else {
            float separation_factor = transform_distance(closeboid_t->distance, self->separation_type);
            separation_sum = add3(separation_sum, scale3(neg3(closeboid_t->direction), separation_factor)); // addscale3
        }
        heading_sum = add3(heading_sum, closeboid_t->boid->velocity); // inc3
        position_sum = add3(position_sum, closeboid_t->boid->position); // inc3
    }

    vec3 steering_target = b->position;
    float target_distance = -1;
    for( int i = 0, end = array_count(self->steering_targets); i < end; ++i ) {
        vec3 *target = &self->steering_targets[i];

        float distance = transform_distance(len3(sub3(*target,b->position)), self->steering_target_type);
        if (target_distance < 0 || distance < target_distance) {
            steering_target = *target;
            target_distance = distance;
        }
    }

    int nearby_size = array_count(nearby);

    // Separation: steer to avoid crowding local flockmates
    vec3 separation = nearby_size > 0 ? scale3(separation_sum, 1.f / nearby_size) : separation_sum;

    // Alignment: steer towards the average heading of local flockmates
    vec3 alignment = nearby_size > 0 ? scale3(heading_sum, 1.f / nearby_size) : heading_sum;

    // Cohesion: steer to move toward the average position of local flockmates
    vec3 avgposition = nearby_size > 0 ? scale3(position_sum, 1.f / nearby_size) : b->position;
    vec3 cohesion = sub3(avgposition, b->position);

    // Steering: steer towards the nearest target location (like a moth to the light)
    vec3 steering = scale3(norm3(sub3(steering_target, b->position)), target_distance);

    // calculate boid acceleration
    vec3 acceleration;
    acceleration = scale3(separation, self->separation_weight);
    acceleration = add3(acceleration, scale3(alignment, self->alignment_weight));
    acceleration = add3(acceleration, scale3(cohesion, self->cohesion_weight));
    acceleration = add3(acceleration, scale3(steering, self->steering_weight));
    b->acceleration = clamplen3(acceleration, self->max_acceleration);
}

swarm_t swarm() {
    swarm_t self = {0};

    self.boids = NULL;

    self.perception_radius = 3; // 30

    self.separation_weight = 0.1; // 1
    self.separation_type = SWARM_DISTANCE_INVERSE_QUADRATIC;

    self.alignment_weight = 0.1; // 1
    self.cohesion_weight = 0.1; // 1

    self.steering_weight = 0.1; // 0.1
    // array_push(self.steering_targets, vec3(0,0,0));
    self.steering_target_type = SWARM_DISTANCE_LINEAR;

    self.blindspot_angledeg = 2; // 20
    self.max_acceleration = 1; // 10;
    self.max_velocity = 2; // 20;

    self.blindspot_angledeg_compare_value_ = 0; // = cos(M_PI * 2 * blindspot_angledeg / 360)
    map_init(self.voxel_cache_, less3, hash3);
    return self;
}

void swarm_update_acceleration_only(swarm_t *self) {
    self->perception_radius += !self->perception_radius; // 0->1

    // build voxel cache
    map_clear(self->voxel_cache_);
    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];

        vec3 *key = MALLOC(sizeof(vec3));
        *key = get_voxel_for_boid(self->perception_radius, b);
        array(boid_t*) *found = map_find_or_add_allocated_key( self->voxel_cache_, key, 0 );
        array_push(*found, b);
    }

    // update all boids
    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];
        update_boid(self, b);
    }
}

void swarm_update_acceleration_and_velocity_only(swarm_t *self, float delta) {
    self->blindspot_angledeg_compare_value_ = cosf(C_PI * 2 * self->blindspot_angledeg / 360.0f);
    swarm_update_acceleration_only(self);

    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];
        b->velocity = clamplen3(add3(b->velocity, scale3(b->acceleration, delta)), self->max_velocity);
    }
}

void swarm_update(swarm_t *self, float delta) {
    swarm_update_acceleration_and_velocity_only(self, delta);

    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];
        b->prev_position = b->position;
        b->position = add3(b->position, scale3(b->velocity, delta));
    }
}

int ui_swarm(swarm_t *self) {
    const char *distances[] = {
    "Linear",
    "Inverse Linear",
    "Quadratic",
    "Inverse Quadratic"
    };

    int rc = 0;
    rc |= ui_float( "Perception Radius", &self->perception_radius);
    ui_separator();

    rc |= ui_float( "Separation Weight", &self->separation_weight);
    rc |= ui_radio( "Separation Type", distances, countof(distances), (int*)&self->separation_type);
    ui_separator();

    rc |= ui_float( "Alignment Weight", &self->alignment_weight);
    rc |= ui_float( "Cohesion Weight", &self->cohesion_weight);
    ui_separator();

    rc |= ui_float( "Steering Weight", &self->steering_weight);
    //array(vec3) steering_targets;
    rc |= ui_radio( "Steering Target Type", distances, countof(distances), (int*)&self->steering_target_type);
    ui_separator();

    rc |= ui_float( "Blindspot Angle", &self->blindspot_angledeg);
    rc |= ui_float( "Max Acceleration", &self->max_acceleration);
    rc |= ui_float( "Max Velocity", &self->max_velocity);

    return rc;
}

// pathfinding -----------------------------------------------------------------

int pathfind_astar(int width, int height, const unsigned* map, vec2i src, vec2i dst, vec2i* path, size_t maxpath) {
#define ALLOW_DIAGONAL_MOVEMENT 1

#if ALLOW_DIAGONAL_MOVEMENT
    #define ASTAR_DIR_COUNT 8
#else
    #define ASTAR_DIR_COUNT 4
#endif

    static const vec2i dir[ASTAR_DIR_COUNT] =
    {
        {  1,  0 },
        {  0,  1 },
        { -1,  0 },
        {  0, -1 },
#if ALLOW_DIAGONAL_MOVEMENT
        {  1,  1 },
        {  1, -1 },
        { -1,  1 },
        { -1, -1 },
#endif
    };

    #define ASTAR_POS_TYPE vec2i
    #define ASTAR_POS_START src
    #define ASTAR_POS_FINISH dst
    #define ASTAR_POS_INDEX(p) ((p).y * width + (p).x)
    #define ASTAR_MAX_INDEX (width * height)
    #define ASTAR_INDEX_POS(p, i) \
        do {                      \
            (p).x = (i) % width;  \
            (p).y = (i) / width;  \
        } while (0)
    #define ASTAR_POS_EQUAL(a, b) ((a).x == (b).x && (a).y == (b).y)
    #define ASTAR_MAP_IS_FREE(p) ((p).y >= 0 && (p).y < height && (p).x >= 0 && (p).x < width && (char)map[(p).y * width + (p).x] == 0)
    #define ASTAR_NEXT_POS(p, i) \
        do {                     \
            (p).x += dir[i].x;   \
            (p).y += dir[i].y;   \
        } while (0)
    #define ASTAR_PREV_POS(p, i) \
        do {                     \
            (p).x -= dir[i].x;   \
            (p).y -= dir[i].y;   \
        } while (0)
    #define ASTAR_GET_COST(a, b) (abs((a).x - (b).x) + abs((a).y - (b).y))

#if ALLOW_DIAGONAL_MOVEMENT
    #define ASTAR_EXTRA_COST(i) (i < 4 ? 5 : 7) // 7/5 is approx sqrt(2)
    #define ASTAR_COST_MUL 5
#endif

    size_t path_count = 0;
    #define ASTAR_PATH(p) if (path_count < maxpath) path[path_count++] = p

    // tempwork memory, not thread-safe.
    #define ASTAR_TEMP_SIZE (ASTAR_MAX_INDEX * (sizeof(unsigned)*2) + sizeof(unsigned)*4) // (16<<20)
    #define ASTAR_TEMP temp
    static array(char) ASTAR_TEMP; do_once array_resize(ASTAR_TEMP, ASTAR_TEMP_SIZE);

    // #if 1  "astar.h"
    {
        // generic A* pathfinding

        //
        // INTERFACE
        //

        // mandatory macros

        #ifndef ASTAR_POS_TYPE
        #error ASTAR_POS_TYPE should specify position type
        #endif

        #ifndef ASTAR_POS_START
        #error ASTAR_POS_START should specify start position
        #endif

        #ifndef ASTAR_POS_FINISH
        #error ASTAR_POS_FINISH should specify finish position
        #endif

        #ifndef ASTAR_POS_INDEX
        #error ASTAR_POS_INDEX(p) should specify macro to map position to index
        #endif

        #ifndef ASTAR_MAX_INDEX 
        #error ASTAR_MAX_INDEX should specify max count of indices the position can map to
        #endif

        #ifndef ASTAR_INDEX_POS
        #error ASTAR_INDEX_POS(i) should specify macro to map index to position
        #endif

        #ifndef ASTAR_POS_EQUAL
        #error ASTAR_POS_EQUAL(a, b) should specify macro to compare if two positions are the same
        #endif

        #ifndef ASTAR_MAP_IS_FREE
        #error ASTAR_MAP_IS_FREE(p) should specify macro to check if map at position p is free
        #endif

        #ifndef ASTAR_NEXT_POS
        #error ASTAR_NEXT_POS(p, i) should specify macro to get next position in specific direction
        #endif

        #ifndef ASTAR_PREV_POS
        #error ASTAR_PREV_POS(p, i) should specify macro to get previous position from specific direction
        #endif

        #ifndef ASTAR_DIR_COUNT
        #error ASTAR_DIR_COUNT should specify possible direction count
        #endif

        #ifndef ASTAR_GET_COST
        #error ASTAR_GET_COST(a, b) should specify macro to get get cost between two positions
        #endif

        #ifndef ASTAR_PATH
        #error ASTAR_PATH(p) should specify macro that will be invoked on each position for path (in reverse order), including start/finish positions
        #endif

        #if !defined(ASTAR_TEMP) || !defined(ASTAR_TEMP_SIZE)
        #error ASTAR_TEMP and ASTAR_TEMP_SIZE should specify variable & size for temporary memory (should be at least ASTAR_MAX_INDEX * 4 + extra)
        #endif

        // optional macros

        // adds extra cost for specific direction (useful for increasing cost for diagonal movements)
        #ifndef ASTAR_EXTRA_COST
        #define ASTAR_EXTRA_COST(i) 1
        #endif

        // multiplier for adding cost values (current_cost + mul * new_cost) - useful when using extra cost for diagonal movements
        #ifndef ASTAR_COST_MUL
        #define ASTAR_COST_MUL 1
        #endif

        //
        // IMPLEMENTATION
        //

        #if ASTAR_DIR_COUNT <= 4
        #define ASTAR_DIR_BITS 2
        #elif ASTAR_DIR_COUNT <= 8
        #define ASTAR_DIR_BITS 3
        #elif ASTAR_DIR_COUNT <= 16
        #define ASTAR_DIR_BITS 4
        #elif ASTAR_DIR_COUNT <= 32
        #define ASTAR_DIR_BITS 5
        #elif ASTAR_DIR_COUNT <= 64
        #define ASTAR_DIR_BITS 6
        #else
        #error Too many elements for ASTAR_DIR_COUNT, 64 is max
        #endif

        #define ASTAR_COST_BITS (32 - 1 - ASTAR_DIR_BITS)

        #define ASTAR_HEAP_SWAP(a, b) \
        do {                          \
            heapnode t = heap[a];     \
            heap[a] = heap[b];        \
            heap[b] = t;              \
        } while (0)                   \

        #define ASTAR_HEAP_PUSH(idx, c)                   \
        do {                                              \
            heap[heap_count].index = idx;                 \
            heap[heap_count].cost = c;                    \
            int i = heap_count++;                         \
            int p = (i - 1) / 2;                          \
            while (i != 0 && heap[p].cost > heap[i].cost) \
            {                                             \
                ASTAR_HEAP_SWAP(i, p);                    \
                i = p;                                    \
                p = (i - 1) / 2;                          \
            }                                             \
        } while (0) 

        #define ASTAR_HEAP_POP()                                          \
        do {                                                              \
            heap[0] = heap[--heap_count];                                 \
            int i = 0;                                                    \
            for (;;)                                                      \
            {                                                             \
                int l = 2 * i + 1;                                        \
                int r = 2 * i + 2;                                        \
                int s = i;                                                \
                if (l < heap_count && heap[l].cost < heap[i].cost) s = l; \
                if (r < heap_count && heap[r].cost < heap[s].cost) s = r; \
                if (s == i) break;                                        \
                ASTAR_HEAP_SWAP(i, s);                                    \
                i = s;                                                    \
          }                                                               \
        } while (0)

        typedef union {
            struct {
                unsigned int cost    : ASTAR_COST_BITS;
                unsigned int dir     : ASTAR_DIR_BITS;
                unsigned int visited : 1;
            };
            unsigned int all;
        } node;

        typedef struct {
            unsigned int index;
            unsigned int cost;
        } heapnode;

        if (ASTAR_TEMP_SIZE >= sizeof(node) * ASTAR_MAX_INDEX + sizeof(heapnode))
        {
            node* nodes = (node*)ASTAR_TEMP;
            for (unsigned int i = 0; i < ASTAR_MAX_INDEX; i++)
            {
                nodes[i].all = 0;
            }

            heapnode* heap = (heapnode*)((char*)ASTAR_TEMP + sizeof(node) * ASTAR_MAX_INDEX);
            unsigned int heap_max = (ASTAR_TEMP_SIZE - sizeof(node) * ASTAR_MAX_INDEX) / sizeof(heapnode);
            int heap_count = 0;

            ASTAR_POS_TYPE p = ASTAR_POS_START;
            unsigned int nindex = ASTAR_POS_INDEX(p);
            node* n = nodes + nindex;
            n->cost = 0;
            n->visited = 1;
            ASTAR_HEAP_PUSH(nindex, ASTAR_GET_COST(p, ASTAR_POS_FINISH));

            int found = 0;
            while (heap_count != 0)
            {
                nindex = heap[0].index;
                n = nodes + nindex;
                ASTAR_HEAP_POP();

                ASTAR_INDEX_POS(p, nindex);
                if (ASTAR_POS_EQUAL(p, ASTAR_POS_FINISH))
                {
                    found = 1;
                    break;
                }
                n->visited = 1;

                for (unsigned int i = 0; i < ASTAR_DIR_COUNT; i++)
                {
                    ASTAR_POS_TYPE next = p;
                    ASTAR_NEXT_POS(next, i);
                    if (ASTAR_MAP_IS_FREE(next))
                    {
                        unsigned int nnext_index = ASTAR_POS_INDEX(next);
                        node* nnext = nodes + nnext_index;
                        unsigned int cost = n->cost + ASTAR_EXTRA_COST(i);
                        if (nnext->visited == 0 || cost < nnext->cost)
                        {
                            nnext->cost = cost;
                            nnext->dir = i;
                            nnext->visited = 1;
                            if (heap_count == heap_max)
                            {
                                // out of memory
                                goto bail;
                            }
                            unsigned int new_cost = cost + ASTAR_COST_MUL * ASTAR_GET_COST(next, ASTAR_POS_FINISH);
                            ASTAR_HEAP_PUSH(nnext_index, new_cost);
                        }
                    }
                }
            }
        bail:

            if (found)
            {
                ASTAR_PATH(p);
                while (!ASTAR_POS_EQUAL(p, ASTAR_POS_START))
                {
                    ASTAR_PREV_POS(p, n->dir);
                    n = nodes + ASTAR_POS_INDEX(p);
                    ASTAR_PATH(p);
                }
            }
        }
        else
        {
            // not enough temp memory
        }

        #undef ASTAR_POS_TYPE
        #undef ASTAR_POS_START
        #undef ASTAR_POS_FINISH
        #undef ASTAR_POS_INDEX
        #undef ASTAR_MAX_INDEX
        #undef ASTAR_INDEX_POS
        #undef ASTAR_POS_EQUAL
        #undef ASTAR_MAP_IS_FREE
        #undef ASTAR_NEXT_POS
        #undef ASTAR_PREV_POS
        #undef ASTAR_DIR_COUNT
        #undef ASTAR_GET_COST
        #undef ASTAR_EXTRA_COST
        #undef ASTAR_COST_MUL
        #undef ASTAR_PATH
        #undef ASTAR_TEMP
        #undef ASTAR_TEMP_SIZE
        #undef ASTAR_COST_BITS
        #undef ASTAR_DIR_BITS
        #undef ASTAR_HEAP_SWAP
        #undef ASTAR_HEAP_PUSH
        #undef ASTAR_HEAP_POP
    }
    // #endif "astar.h"

    return path_count;
}
#line 0

#line 1 "fwk_bt.c"
map(char*, bt_func) binds;

void bt_addfun(const char *name, int(*func)()){
    do_once map_init_str(binds);
    map_find_or_add_allocated_key(binds, STRDUP(name), func);
}
bt_func bt_findfun(const char *name) {
    bt_func *found = map_find(binds, (char*)name);
    return found ? *found : 0;
}
char * bt_funcname(bt_func fn) {
    for each_map(binds,char*,k,bt_func,f) {
        if( f == fn ) return k;
    }
    return 0;
}

bt_t bt(const char *ini_file, unsigned flags) {
    bt_t z = {0}, root = z;
    array(char*) m = strsplit(vfs_read(ini_file), "\r\n");

    bt_t *self = &root;
    self->type = cc8(root);
    //self->parent = self;

    for( int i = 0; i < array_count(m); ++i ) {
        // parse ini
        int level = strspn(m[i], " \t");
        char *k = m[i] + level;
        if( k[0] == '[' ) {
            if( strcmp(k+1, "bt]") ) return z; // we only support [bt]
            continue;
        }
        int sep = strcspn(k, " =:");
        char *v = k + sep; if(sep) *v++ = '\0'; else v = k + strlen(k); // v = (char*)"";
        array(char*) args = *v ? strsplit(v, " ") : NULL;

        // insert node in tree
        bt_t *out = self;
        while( level-- ) {
            out = array_back(out->children);
        }
        array_push(out->children, ((bt_t){0}));
        //array_back(out->children)->parent = out;
        out = array_back(out->children);

        // config node
        out->type = *(uint64_t*)va("%-8s", k);
        if( array_count(args) ) out->argf = atof(args[0]);
        if( !strcmp(k, "run") ) out->action = bt_findfun(v);
    }

    return root;
}

int bt_run(bt_t *b) {
    int rc = 0;

    /**/ if( b->type == cc8(     run) ) { return b->action ? b->action() : 0; }
    else if( b->type == cc8(     not) ) { return !bt_run(b->children + 0); }
    else if( b->type == cc8(   sleep) ) { return sleep_ss(b->argf), bt_run(b->children + 0); }
    else if( b->type == cc8(   defer) ) { rc = bt_run(b->children + 0); return sleep_ss(b->argf), rc; }
    else if( b->type == cc8(    loop) ) { int rc; for(int i = 0; i < b->argf; ++i) rc = bt_run(b->children + 0); return rc; }
    else if( b->type == cc8(    once) ) { return b->argf ? 0 : (b->argf = 1), bt_run(b->children + 0); }
    else if( b->type == cc8(   count) ) { return b->argf <= 0 ? 0 : --b->argf, bt_run(b->children + 0); }
    else if( b->type == cc8(    pass) ) { return bt_run(b->children + 0), 1; }
    else if( b->type == cc8(    fail) ) { return bt_run(b->children + 0), 0; }
    else if( b->type == cc8(  result) ) { return bt_run(b->children + 0), !!b->argf; }
    else if( b->type == cc8(     all) ) { for( int i = 0; i < array_count(b->children); ++i ) if(!bt_run(b->children+i)) return 0; return 1; }
    else if( b->type == cc8(     any) ) { for( int i = 0; i < array_count(b->children); ++i ) if( bt_run(b->children+i)) return 1; return 0; }
    else if( b->type == cc8(    root) ) { for( int i = 0; i < array_count(b->children); ++i ) rc|=bt_run(b->children+i); return rc; }
    else if( b->type == cc8(rootfail) ) { rc = 1; for( int i = 0; i < array_count(b->children); ++i ) rc&=~bt_run(b->children+i); return rc; }

    return 0;
}

void ui_bt(bt_t *b) {
    if( b ) {
        char *info = bt_funcname(b->action);
        if(!info) info = va("%d", array_count(b->children));

        if( ui_collapse(va("%s (%s)", cc8str(b->type), info), va("bt%p",b)) ) {
            for( int i = 0; i < array_count(b->children); ++i) {
                ui_bt(b->children + i);
            }
            ui_collapse_end();
        }
    }
}
#line 0

#line 1 "fwk_editor.c"
// editing:
// nope > functions: add/rem property

char *editor_path(const char *path) {
    return va("%s/%s", EDITOR, path);
}

vec3 editor_pick(float mouse_x, float mouse_y) {
    // unproject 2d coord as 3d coord
    camera_t *camera = camera_get_active();
    vec3 out, xyd = vec3(mouse_x,window_height()-mouse_y,1); // usually x:mouse_x,y:window_height()-mouse_y,d:0=znear/1=zfar
    mat44 mvp, model; identity44(model); multiply44x3(mvp, camera->proj, camera->view, model);
    bool ok = unproject44(&out, xyd, vec4(0,0,window_width(),window_height()), mvp);
    return out;
}

int editor_ui_bits8(const char *label, uint8_t *enabled) { // @to deprecate
    int clicked = 0;
    uint8_t copy = *enabled;

    // @fixme: better way to retrieve widget width? nk_layout_row_dynamic() seems excessive
    nk_layout_row_dynamic(ui_ctx, 1, 1);
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);

    // actual widget: label + 8 checkboxes
    enum { HEIGHT = 18, BITS = 8, SPAN = 118 }; // bits widget below needs at least 118px wide
    nk_layout_row_begin(ui_ctx, NK_STATIC, HEIGHT, 1+BITS);

        int offset = bounds.w > SPAN ? bounds.w - SPAN : 0;
        nk_layout_row_push(ui_ctx, offset);
        if( ui_label_(label, NK_TEXT_LEFT) ) clicked = 1<<31;

        for( int i = 0; i < BITS; ++i ) {
            nk_layout_row_push(ui_ctx, 10);
            // bit
            int val = (*enabled >> i) & 1;
            int chg = nk_checkbox_label(ui_ctx, "", &val);
            *enabled = (*enabled & ~(1 << i)) | ((!!val) << i);
            // tooltip
            struct nk_rect bb = { offset + 10 + i * 14, bounds.y, 14, HEIGHT }; // 10:padding,14:width
            if (nk_input_is_mouse_hovering_rect(&ui_ctx->input, bb) && !ui_popups()) {
                const char *tips[BITS] = {"Init","Tick","Draw","Quit","","","",""};
                if(tips[i][0]) nk_tooltipf(ui_ctx, "%s", tips[i]);
            }
        }

    nk_layout_row_end(ui_ctx);
    return clicked | (copy ^ *enabled);
}

static int gizmo__mode;
static int gizmo__active;
static int gizmo__hover;
bool gizmo_active() {
    return gizmo__active;
}
bool gizmo_hover() {
    return gizmo__hover;
}
int gizmo(vec3 *pos, vec3 *rot, vec3 *sca) {
#if 0
    ddraw_flush();
    mat44 copy; copy44(copy, camera_get_active()->view);
    if( 1 ) {
        float *mv = camera_get_active()->view;
        float d = sqrt(mv[4*0+0] * mv[4*0+0] + mv[4*1+1] * mv[4*1+1] + mv[4*2+2] * mv[4*2+2]);
        if(4) mv[4*0+0] = d, mv[4*0+1] = 0, mv[4*0+2] = 0;
        if(2) mv[4*1+0] = 0, mv[4*1+1] = d, mv[4*1+2] = 0;
        if(1) mv[4*2+0] = 0, mv[4*2+1] = 0, mv[4*2+2] = d;
    }
#endif

    ddraw_color_push(dd_color);
    ddraw_ontop_push(1);

    int enabled = !ui_active() && !ui_hover();
    vec3 mouse = enabled ? vec3(input(MOUSE_X),input(MOUSE_Y),input_down(MOUSE_L)) : vec3(0,0,0); // x,y,l
    vec3 from = camera_get_active()->position;
    vec3 to = editor_pick(mouse.x, mouse.y);
    ray r = ray(from, to);

    static vec3 src3, hit3, off3; static vec2 src2;
    #define on_gizmo_dragged(X,Y,Z,COLOR,DRAWCMD, ...) do { \
        vec3 dir = vec3(X,Y,Z); \
        line axis = {add3(*pos, scale3(dir,100)), add3(*pos, scale3(dir,-100))}; \
        plane ground = { vec3(0,0,0), vec3(Y?1:0,Y?0:1,0) }; \
        vec3 unit = vec3(X+(1.0-X)*0.3,Y+(1.0-Y)*0.3,Z+(1.0-Z)*0.3); \
        aabb arrow = { sub3(*pos,unit), add3(*pos,unit) }; \
        hit *hit_arrow = ray_hit_aabb(r, arrow), *hit_ground = ray_hit_plane(r, ground); \
        ddraw_color( hit_arrow || gizmo__active == (X*4+Y*2+Z) ? gizmo__hover = 1, YELLOW : COLOR ); \
        DRAWCMD; \
        if( !gizmo__active && hit_arrow && mouse.z ) src2 = vec2(mouse.x,mouse.y), src3 = *pos, hit3 = hit_ground->p, off3 = mul3(sub3(src3,hit3),vec3(X,Y,Z)), gizmo__active = X*4+Y*2+Z; \
        if( (gizmo__active && gizmo__active==(X*4+Y*2+Z)) || (!gizmo__active && hit_arrow) ) { ddraw_color( COLOR ); ( 1 ? ddraw_line : ddraw_line_dashed)(axis.a, axis.b); } \
        if( gizmo__active == (X*4+Y*2+Z) && hit_ground ) {{ __VA_ARGS__ }; modified = 1; gizmo__active *= !!input(MOUSE_L); } \
    } while(0)
    #define gizmo_translate(X,Y,Z,COLOR) \
        on_gizmo_dragged(X,Y,Z,COLOR, ddraw_arrow(*pos,add3(*pos,vec3(X,Y,Z))), { \
            *pos = add3(line_closest_point(axis, hit_ground->p), off3); \
        } )
    #define gizmo_scale(X,Y,Z,COLOR) \
        on_gizmo_dragged(X,Y,Z,COLOR, (ddraw_line(*pos,add3(*pos,vec3(X,Y,Z))),ddraw_sphere(add3(*pos,vec3(X-0.1*X,Y-0.1*Y,Z-0.1*Z)),0.1)), { /*ddraw_aabb(arrow.min,arrow.max)*/ \
            int component = (X*1+Y*2+Z*3)-1; \
            float mag = len2(sub2(vec2(mouse.x, mouse.y), src2)); \
            float magx = (mouse.x - src2.x) * (mouse.x - src2.x); \
            float magy = (mouse.y - src2.y) * (mouse.y - src2.y); \
            float sgn = (magx > magy ? mouse.x > src2.x : mouse.y > src2.y) ? 1 : -1; \
            sca->v3[component] -= sgn * mag * 0.01; \
            src2 = vec2(mouse.x, mouse.y); \
        } )
    #define gizmo_rotate(X,Y,Z,COLOR) do { \
            vec3 dir = vec3(X,Y,Z); \
            line axis = {add3(*pos, scale3(dir,100)), add3(*pos, scale3(dir,-100))}; \
            plane ground = { vec3(0,0,0), vec3(0,1,0) }; \
                vec3 unit = vec3(X+(1.0-X)*0.3,Y+(1.0-Y)*0.3,Z+(1.0-Z)*0.3); \
                aabb arrow = { sub3(*pos,unit), add3(*pos,unit) }; \
                hit *hit_arrow = ray_hit_aabb(r, arrow), *hit_ground = ray_hit_plane(r, ground); \
                int hover = (hit_arrow ? (X*4+Y*2+Z) : 0); \
            if( gizmo__active == (X*4+Y*2+Z) ) { ddraw_color(gizmo__active ? gizmo__hover = 1, YELLOW : WHITE); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            else if( !gizmo__active && hover == (X*4+Y*2+Z) ) { gizmo__hover = 1; ddraw_color(COLOR); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            else if( !gizmo__active ) { ddraw_color(WHITE); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            if( !gizmo__active && hit_arrow && mouse.z ) src2 = vec2(mouse.x,mouse.y), gizmo__active = hover; \
            if( (!gizmo__active && hover == (X*4+Y*2+Z)) || gizmo__active == (X*4+Y*2+Z) ) { gizmo__hover = 1; ddraw_color( COLOR ); ( 1 ? ddraw_line_thin : ddraw_line_dashed)(axis.a, axis.b); } \
            if( gizmo__active && gizmo__active == (X*4+Y*2+Z) && hit_ground && enabled ) { \
                int component = (Y*1+X*2+Z*3)-1; /*pitch,yaw,roll*/ \
                float mag = len2(sub2(vec2(mouse.x, mouse.y), src2)); \
                float magx = (mouse.x - src2.x) * (mouse.x - src2.x); \
                float magy = (mouse.y - src2.y) * (mouse.y - src2.y); \
                float sgn = (magx > magy ? mouse.x > src2.x : mouse.y > src2.y) ? 1 : -1; \
                rot->v3[component] += sgn * mag; \
                /*rot->v3[component] = clampf(rot->v3[component], -360, +360);*/ \
                src2 = vec2(mouse.x, mouse.y); \
                \
            } \
            gizmo__active *= enabled && !!input(MOUSE_L); \
        } while(0)

    gizmo__hover = 0;

    int modified = 0;
    if(enabled && input_down(KEY_SPACE)) gizmo__active = 0, gizmo__mode = (gizmo__mode + 1) % 3;
    if(gizmo__mode == 0) gizmo_translate(1,0,0, RED);
    if(gizmo__mode == 0) gizmo_translate(0,1,0, GREEN);
    if(gizmo__mode == 0) gizmo_translate(0,0,1, BLUE);
    if(gizmo__mode == 1) gizmo_scale(1,0,0, RED);
    if(gizmo__mode == 1) gizmo_scale(0,1,0, GREEN);
    if(gizmo__mode == 1) gizmo_scale(0,0,1, BLUE);
    if(gizmo__mode == 2) gizmo_rotate(1,0,0, RED);
    if(gizmo__mode == 2) gizmo_rotate(0,1,0, GREEN);
    if(gizmo__mode == 2) gizmo_rotate(0,0,1, BLUE);

#if 0
    ddraw_flush();
    copy44(camera_get_active()->view, copy);
#endif

    ddraw_ontop_pop();
    ddraw_color_pop();

    return modified;
}

char* dialog_load() {
    const char *windowTitle = NULL;
    const char *defaultPathFile = NULL;
    const char *filterHints = NULL; // "image files"
    const char *filters[] = { "*.*" };
    int allowMultipleSelections = 0;

    tinyfd_assumeGraphicDisplay = 1;
    return tinyfd_openFileDialog( windowTitle, defaultPathFile, countof(filters), filters, filterHints, allowMultipleSelections );
}
char* dialog_save() {
    const char *windowTitle = NULL;
    const char *defaultPathFile = NULL;
    const char *filterHints = NULL; // "image files"
    const char *filters[] = { "*.*" };

    tinyfd_assumeGraphicDisplay = 1;
    return tinyfd_saveFileDialog( windowTitle, defaultPathFile, countof(filters), filters, filterHints );
}

// -- localization kit

static const char *kit_lang = "enUS", *kit_langs = 
    "enUS,enGB,"
    "frFR,"
    "esES,esAR,esMX,"
    "deDE,deCH,deAT,"
    "itIT,itCH,"
    "ptBR,ptPT,"
    "zhCN,zhSG,zhTW,zhHK,zhMO,"
    "ruRU,elGR,trTR,daDK,noNB,svSE,nlNL,plPL,fiFI,jaJP,"
    "koKR,csCZ,huHU,roRO,thTH,bgBG,heIL"
;

static map(char*,char*) kit_ids;
static map(char*,char*) kit_vars;

#ifndef KIT_FMT_ID2 
#define KIT_FMT_ID2 "%s.%s"
#endif

void kit_init() {
    do_once map_init(kit_ids, less_str, hash_str);
    do_once map_init(kit_vars, less_str, hash_str);
}

void kit_insert( const char *id, const char *translation) {
    char *id2 = va(KIT_FMT_ID2, kit_lang, id);

    char **found = map_find_or_add_allocated_key(kit_ids, STRDUP(id2), NULL);
    if(*found) FREE(*found);
    *found = STRDUP(translation);
}

bool kit_merge( const char *filename ) {
    // @todo: xlsx2ini
    return false;
}

void kit_clear() {
    map_clear(kit_ids);
}

bool kit_load( const char *filename ) {
    return kit_clear(), kit_merge( filename );
}

void kit_set( const char *key, const char *value ) {
    value = value && value[0] ? value : "";

    char **found = map_find_or_add_allocated_key(kit_vars, STRDUP(key), NULL );
    if(*found) FREE(*found);
    *found = STRDUP(value);
}

void kit_reset() {
    map_clear(kit_vars);
}

char *kit_translate2( const char *id, const char *lang ) {
    char *id2 = va(KIT_FMT_ID2, lang, id);

    char **found = map_find(kit_ids, id2);

    // return original [[ID]] if no translation is found
    if( !found ) return va("[[%s]]", id);

    // return translation if no {{moustaches}} are found
    if( !strstr(*found, "{{") ) return *found;

    // else replace all found {{moustaches}} with context vars
    {
        // make room
        static __thread char *results[16] = {0};
        static __thread unsigned counter = 0; counter = (counter+1) % 16;

        char *buffer = results[ counter ];
        if( buffer ) FREE(buffer), buffer = 0;

        // iterate moustaches
        const char *begin, *end, *text = *found;
        while( NULL != (begin = strstr(text, "{{")) ) {
            end = strstr(begin+2, "}}");
            if( end ) {
                char *var = va("%.*s", (int)(end - (begin+2)), begin+2);
                char **found_var = map_find(kit_vars, var);

                if( found_var && 0[*found_var] ) {
                    strcatf(&buffer, "%.*s%s", (int)(begin - text), text, *found_var);
                } else {
                    strcatf(&buffer, "%.*s{{%s}}", (int)(begin - text), text, var);
                }

                text = end+2;
            } else {
                strcatf(&buffer, "%.*s", (int)(begin - text), text);

                text = begin+2;
            }
        }

        strcatf(&buffer, "%s", text);
        return buffer;
    }
}

char *kit_translate( const char *id ) {
    return kit_translate2( id, kit_lang );
}

void kit_locale( const char *lang ) {
    kit_lang = STRDUP(lang); // @leak
}

void kit_dump_state( FILE *fp ) {
    for each_map(kit_ids, char *, k, char *, v) {
        fprintf(fp, "[ID ] %s=%s\n", k, v);
    }
    for each_map(kit_vars, char *, k, char *, v) {
        fprintf(fp, "[VAR] %s=%s\n", k, v);
    }
}

/*
int main() {
    kit_init();

    kit_locale("enUS");
    kit_insert("HELLO_PLAYERS", "Hi {{PLAYER1}} and {{PLAYER2}}!");
    kit_insert("GREET_PLAYERS", "Nice to meet you.");

    kit_locale("esES");
    kit_insert("HELLO_PLAYERS", "Hola {{PLAYER1}} y {{PLAYER2}}!");
    kit_insert("GREET_PLAYERS", "Un placer conoceros.");

    kit_locale("enUS");
    printf("%s %s\n", kit_translate("HELLO_PLAYERS"), kit_translate("GREET_PLAYERS")); // Hi {{PLAYER1}} and {{PLAYER2}}! Nice to meet you.

    kit_locale("esES");
    kit_set("PLAYER1", "John");
    kit_set("PLAYER2", "Karl");
    printf("%s %s\n", kit_translate("HELLO_PLAYERS"), kit_translate("GREET_PLAYERS")); // Hola John y Karl! Un placer conoceros.

    assert( 0 == strcmp(kit_translate("NON_EXISTING"), "[[NON_EXISTING]]")); // [[NON_EXISTING]]
    assert(~puts("Ok"));
}
*/
#line 0

// editor is last in place, so it can use all internals from above headers

#line 1 "fwk_main.c"
// ----------------------------------------------------------------------------

static void fwk_pre_init() {
    const char *appname = app_name();
    window_icon(va("%s.png", appname));
    ifdef(win32,window_icon(va("%s.ico", appname)));

    glfwPollEvents();

    int i;
    #pragma omp parallel for
    for( i = 0; i <= 3; ++i) {
        /**/ if( i == 0 ) ddraw_init();// init this on thread#0 since it will be compiling shaders, and shaders need to be compiled from the very same thread than glfwMakeContextCurrent() was set up
        else if( i == 1 ) sprite_init();
        else if( i == 2 ) profiler_init();
        else if( i == 3 ) storage_mount("save/"), storage_read(), touch_init(); // for ems
    }

    // window_swap();
}
static void fwk_post_init(float refresh_rate) {
    // cook cleanup
    cook_stop();

    vfs_reload();

    // init more subsystems; beware of VFS mounting, as some of these may need cooked assets at this point
    int i;
#if 1 // #ifdef PARALLEL_INIT
    #pragma omp parallel for
#endif
    for( i = 0; i <= 3; ++i) {
        /**/ if( i == 0 ) ui_init(), scene_init(); // init these on thread #0, since both will be compiling shaders, and shaders need to be compiled from the very same thread than glfwMakeContextCurrent() was set up
        else if( i == 1 ) audio_init(0); // initialize audio after cooking // reasoning for this: do not launch audio threads while cooks are in progress, so there is more cpu for cooking actually
        else if( i == 2 ) script_init(), kit_init(), midi_init();
        else if( i == 3 ) input_init(), network_init();
    }

    // @todo
    // window_icon(vfs_file(va("%s.png", app)));
    // window_icon(vfs_file(va("%s.ico", app)));

    // display window
    glfwShowWindow(window);
    glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    randset(time_ns());
    boot_time = -time_ss(); // measure boot time, this is continued in window_stats()

    // clean any errno setup by cooking stage
    errno = 0;

    hz = refresh_rate;
    // t = glfwGetTime();
}

// ----------------------------------------------------------------------------

static
void fwk_quit(void) {
    storage_flush();
    midi_quit();
}

void fwk_init() {
    do_once {
        // install signal handlers
        ifdef(debug, signal_hooks());

        // init panic handler
        panic_oom_reserve = SYS_REALLOC(panic_oom_reserve, 1<<20); // 1MiB

        // init glfw
        glfw_init();

        // enable ansi console
        tty_init();

        // chdir to root (if invoked as tcc -g -run)
        chdir(app_path());

        // skip tcc argvs (if invoked as tcc file.c fwk.c -g -run) (win)
        if( __argc > 1 ) if( strstr(__argv[0], "/tcc") || strstr(__argv[0], "\\tcc") ) {
            __argc = 0;
        }

        // create or update cook.zip file
        if( /* !COOK_ON_DEMAND && */ file_exist(COOK_INI) && cook_jobs() ) {
            cook_start(COOK_INI, "**", 0|COOK_ASYNC|COOK_CANCELABLE );
        }

        atexit(fwk_quit);
    }
}

// Enable more performant GPUs on laptops. Does this work into a dll?
// int NvOptimusEnablement = 1;
// int AmdPowerXpressRequestHighPerformance = 1;

#if is(linux) && is(tcc) // fixes `tcc: error: undefined symbol '__dso_handle'`
int __dso_handle; // compiled with: `tcc demo.c fwk.c -D__STDC_NO_VLA__ -lX11`
#endif

#if is(win32) && is(tcc) // fixes `tcc: error: undefined symbol '_InterlockedExchangeAdd'` when compiling with `-m64` flag
__CRT_INLINE LONG _InterlockedExchangeAdd(LONG volatile *add, LONG val) {
    LONG old;
    do old = *add; while( InterlockedCompareExchange(add, old + val, old) != old );
    return old;
}
__CRT_INLINE LONGLONG _InterlockedExchangeAdd64(LONGLONG volatile *add, LONGLONG val) { // 64bit version, for completeness
    LONGLONG old;
    do old = *add; while( InterlockedCompareExchange64(add, old + val, old) != old );
    return old;
}
#endif

#ifdef ZIG_CC
static int IN6_IS_ADDR_V4MAPPED(const struct in6_addr *a) { return ((a->s6_words[0]==0) && (a->s6_words[1]==0) && (a->s6_words[2]==0) && (a->s6_words[3]==0) && (a->s6_words[4]==0) && (a->s6_words[5]==0xffff)); }
const struct in6_addr in6addr_any; // = IN6ADDR_ANY_INIT;
//static const struct in6_addr in6addr_loopback = IN6ADDR_LOOPBACK_INIT;
#endif

#line 0
