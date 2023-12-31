#ifndef LUAFFI_H  
#define LUAFFI_H 
 
/* vim: ts=4 sw=4 sts=4 et tw=78
 *
 * Copyright (c) 2011 James R. McKaskill
 *
 * This software is licensed under the stock MIT license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * ----------------------------------------------------------------------------
 */

//#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
//# include <lua.h>
//# include <lauxlib.h>
}
# define EXTERN_C extern "C"
#else
//# include <lua.h>
//# include <lauxlib.h>
# define EXTERN_C extern
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#endif

#if __STDC_VERSION__+0 >= 199901L && !defined(__TINYC__) //< @r-lyeh: handle tcc case
#include <complex.h>
#define HAVE_COMPLEX
#define HAVE_LONG_DOUBLE
#endif

#ifndef NDEBUG
#define DASM_CHECKS
#endif

struct jit;
#define Dst_DECL	struct jit* Dst
#define Dst_REF		(Dst->ctx)
#define DASM_EXTERN(a,b,c,d) get_extern(a,b,c,d)

//#include "dynasm/dasm_proto.h"

#if defined LUA_FFI_BUILD_AS_DLL
# define LUAFFI_EXPORT __declspec(dllexport)
#elif defined __GNUC__
# define LUAFFI_EXPORT __attribute__((visibility("default")))
#else
# define LUAFFI_EXPORT
#endif

EXTERN_C LUAFFI_EXPORT int luaopen_ffi(lua_State* L);

static int lua_absindex2(lua_State* L, int idx) {
    return (LUA_REGISTRYINDEX <= idx && idx < 0)
         ? lua_gettop(L) + idx + 1
         : idx;
}
/* use our own version of lua_absindex such that lua_absindex(L, 0) == 0 */
#define lua_absindex(L, idx) lua_absindex2(L, idx)

#if LUA_VERSION_NUM == 501
static void lua_callk(lua_State *L, int nargs, int nresults, int ctx, lua_CFunction k)
{
    lua_call(L, nargs, nresults);
}
/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
static void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  for (; l && l->name; l++) {  /* fill the table with given functions */
    int i;
    for (i = 0; i < nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -nup);
    lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_pop(L, nup);  /* remove upvalues */
}
#define lua_setuservalue lua_setfenv
#define lua_getuservalue lua_getfenv
#define lua_rawlen lua_objlen
static char* luaL_prepbuffsize(luaL_Buffer* B, size_t sz) {
    if (sz > LUAL_BUFFERSIZE) {
        luaL_error(B->L, "string too long");
    }
    return luaL_prepbuffer(B);
}
#endif

/* architectures */
#if defined _WIN32 && defined UNDER_CE
# define OS_CE
#elif defined _WIN32
# define OS_WIN
#elif defined __APPLE__ && defined __MACH__
# define OS_OSX
#elif defined __linux__
# define OS_LINUX
#elif defined __FreeBSD__ || defined __OpenBSD__ || defined __NetBSD__
# define OS_BSD
#elif defined unix || defined __unix__ || defined __unix || defined _POSIX_VERSION || defined _XOPEN_VERSION
# define OS_POSIX
#endif

/* architecture */
#if defined __i386__ || defined _M_IX86
# define ARCH_X86
#elif defined __amd64__ || defined _M_X64
# define ARCH_X64
#elif defined __arm__ || defined __ARM__ || defined ARM || defined __ARM || defined __arm
# define ARCH_ARM
#elif defined OS_LINUX && defined __TINYC__ //< @r-lyeh: tcc+linux
# define ARCH_X64 //< @r-lyeh: tcc+linux
#else
# error
#endif


#ifdef _WIN32

#   ifdef UNDER_CE
        static void* DoLoadLibraryA(const char* name) {
          wchar_t buf[MAX_PATH];
          int sz = MultiByteToWideChar(CP_UTF8, 0, name, -1, buf, 512);
          if (sz > 0) {
            buf[sz] = 0;
            return LoadLibraryW(buf);
          } else {
            return NULL;
          }
        }
#       define LoadLibraryA DoLoadLibraryA
#   else
#       define GetProcAddressA GetProcAddress
#   endif

#   define LIB_FORMAT_1 "%s.dll"
#   define AllocPage(size) VirtualAlloc(NULL, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE)
#   define FreePage(data, size) VirtualFree(data, 0, MEM_RELEASE)
#   define EnableExecute(data, size) do {DWORD old; VirtualProtect(data, size, PAGE_EXECUTE, &old); FlushInstructionCache(GetCurrentProcess(), data, size);} while (0)
#   define EnableWrite(data, size) do {DWORD old; VirtualProtect(data, size, PAGE_READWRITE, &old);} while (0)

#else
#   define LIB_FORMAT_1 "%s.so"
#   define LIB_FORMAT_2 "lib%s.so"
#   define LoadLibraryA(name) dlopen(name, RTLD_LAZY | RTLD_GLOBAL)
#   define GetProcAddressA(lib, name) dlsym(lib, name)
#   define AllocPage(size) mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0)
#   define FreePage(data, size) munmap(data, size)
#   define EnableExecute(data, size) mprotect(data, size, PROT_READ|PROT_EXEC)
#   define EnableWrite(data, size) mprotect(data, size, PROT_READ|PROT_WRITE)
#endif

#if defined ARCH_X86 || defined ARCH_X64
#define ALLOW_MISALIGNED_ACCESS
#endif

struct token;

struct parser {
    int line;
    const char* next;
    const char* prev;
    unsigned align_mask;
};

struct page {
    size_t size;
    size_t off;
    size_t freed;
};

struct jit {
    lua_State* L;
    int32_t last_errno;
    struct dasm_State* ctx;
    size_t pagenum;
    struct page** pages;
    size_t align_page_size;
    void** globals;
    int function_extern;
    void* lua_dll;
    void* kernel32_dll;
};

#define ALIGN_DOWN(PTR, MASK) \
  (((uintptr_t) (PTR)) & (~ ((uintptr_t) (MASK)) ))
#define LUAFFI_ALIGN_UP(PTR, MASK) \
  (( ((uintptr_t) (PTR)) + ((uintptr_t) (MASK)) ) & (~ ((uintptr_t) (MASK)) ))

/* struct cdata/struct ctype */

#define PTR_ALIGN_MASK (sizeof(void*) - 1)
#define FUNCTION_ALIGN_MASK (sizeof(void (*)()) - 1)
#define DEFAULT_ALIGN_MASK 7

#ifdef OS_OSX
/* TODO: figure out why the alignof trick doesn't work on OS X */
#define ALIGNED_DEFAULT 7
#elif defined __GNUC__
#define ALIGNED_DEFAULT (__alignof__(void* __attribute__((aligned))) - 1)
#else
#define ALIGNED_DEFAULT PTR_ALIGN_MASK
#endif

extern int jit_key;
extern int ctype_mt_key;
extern int cdata_mt_key;
extern int cmodule_mt_key;
extern int callback_mt_key;
extern int constants_key;
extern int types_key;
extern int gc_key;
extern int callbacks_key;
extern int functions_key;
extern int abi_key;
extern int next_unnamed_key;
extern int niluv_key;
extern int asmname_key;

int equals_upval(lua_State* L, int idx, int* key);
void push_upval(lua_State* L, int* key);
void set_upval(lua_State* L, int* key);
struct jit* get_jit(lua_State* L);

/* both ctype and cdata are stored as userdatas
 *
 * usr value is a table shared between the related subtypes which has:
 * name -> member ctype (for structs and unions)
 * +ves -> member ctype - in memory order (for structs)
 * +ves -> argument ctype (for function prototypes)
 * 0 -> return ctype (for function prototypes)
 * light userdata -> misc
 */

enum {
    C_CALL,
    STD_CALL,
    FAST_CALL,
};

enum {
    INVALID_TYPE,
    VOID_TYPE,
    FLOAT_TYPE,
    DOUBLE_TYPE,
    LONG_DOUBLE_TYPE,
    COMPLEX_FLOAT_TYPE,
    COMPLEX_DOUBLE_TYPE,
    COMPLEX_LONG_DOUBLE_TYPE,
    BOOL_TYPE,
    INT8_TYPE,
    INT16_TYPE,
    INT32_TYPE,
    INT64_TYPE,
    INTPTR_TYPE,
    ENUM_TYPE,
    UNION_TYPE,
    STRUCT_TYPE,
    FUNCTION_TYPE,
    FUNCTION_PTR_TYPE,
};

#define IS_CHAR_UNSIGNED (((char) -1) > 0)
#define IS_COMPLEX(type) ((type) == COMPLEX_FLOAT_TYPE || (type) == COMPLEX_DOUBLE_TYPE)

#define POINTER_BITS 2
#define POINTER_MAX ((1 << POINTER_BITS) - 1)

#define ALIGNOF(S) ((int) ((char*) &S.v - (char*) &S - 1))

/* Note: if adding a new member that is associated with a struct/union
 * definition then it needs to be copied over in ctype.c:set_defined for when
 * we create types based off of the declaration alone.
 *
 * Since this is used as a header for every ctype and cdata, and we create a
 * ton of them on the stack, we try and minimise its size.
 */
struct ctype {
    size_t base_size; /* size of the base type in bytes */

    union {
        /* valid if is_bitfield */
        struct {
            /* size of bitfield in bits */
            unsigned bit_size : 7;
            /* offset within the current byte between 0-63 */
            unsigned bit_offset : 6;
        };
        /* Valid if is_array */
        size_t array_size;
        /* Valid for is_variable_struct or is_variable_array. If
         * variable_size_known (only used for is_variable_struct) then this is
         * the total increment otherwise this is the per element increment.
         */
        size_t variable_increment;
    };
    size_t offset;
    unsigned align_mask : 4; /* as (align bytes - 1) eg 7 gives 8 byte alignment */
    unsigned pointers : POINTER_BITS; /* number of dereferences to get to the base type including +1 for arrays */
    unsigned const_mask : POINTER_MAX + 1; /* const pointer mask, LSB is current pointer, +1 for the whether the base type is const */
    unsigned type : 5; /* value given by type enum above */
    unsigned is_reference : 1;
    unsigned is_array : 1;
    unsigned is_defined : 1;
    unsigned is_null : 1;
    unsigned has_member_name : 1;
    unsigned calling_convention : 2;
    unsigned has_var_arg : 1;
    unsigned is_variable_array : 1; /* set for variable array types where we don't know the variable size yet */
    unsigned is_variable_struct : 1;
    unsigned variable_size_known : 1; /* used for variable structs after we know the variable size */
    unsigned is_bitfield : 1;
    unsigned has_bitfield : 1;
    unsigned is_jitted : 1;
    unsigned is_packed : 1;
    unsigned is_unsigned : 1;
};

#ifdef _MSC_VER
__declspec(align(16))
#endif
struct cdata {
    const struct ctype type
#ifdef __GNUC__
      __attribute__ ((aligned(16)))
#endif
      ;
};

typedef void (*cfunction)(void);

#ifdef HAVE_COMPLEX
typedef double complex complex_double;
typedef float complex complex_float;
#else
typedef struct {
    double real, imag;
} complex_double;

typedef struct {
    float real, imag;
} complex_float;

static double creal(complex_double c) {
    return c.real;
}
static float crealf(complex_float c) {
    return c.real;
}

static double cimag(complex_double c) {
    return c.imag;
}
static float cimagf(complex_float c) {
    return c.imag;
}
#endif

#define CALLBACK_FUNC_USR_IDX 1

void set_defined(lua_State* L, int ct_usr, struct ctype* ct);
struct ctype* push_ctype(lua_State* L, int ct_usr, const struct ctype* ct);
void* push_cdata(lua_State* L, int ct_usr, const struct ctype* ct); /* called from asm */
void push_callback(lua_State* L, cfunction f);
void check_ctype(lua_State* L, int idx, struct ctype* ct);
void* to_cdata(lua_State* L, int idx, struct ctype* ct);
void* check_cdata(lua_State* L, int idx, struct ctype* ct);
size_t ctype_size(lua_State* L, const struct ctype* ct);

int parse_type(lua_State* L, struct parser* P, struct ctype* type);
void parse_argument(lua_State* L, struct parser* P, int ct_usr, struct ctype* type, struct token* name, struct parser* asmname);
void push_type_name(lua_State* L, int usr, const struct ctype* ct);

int push_user_mt(lua_State* L, int ct_usr, const struct ctype* ct);

int ffi_cdef(lua_State* L);

void push_func_ref(lua_State* L, cfunction func);
void free_code(struct jit* jit, lua_State* L, cfunction func);
int x86_return_size(lua_State* L, int usr, const struct ctype* ct);
void compile_function(lua_State* L, cfunction f, int ct_usr, const struct ctype* ct);
cfunction compile_callback(lua_State* L, int fidx, int ct_usr, const struct ctype* ct);
void compile_globals(struct jit* jit, lua_State* L);
int get_extern(struct jit* jit, uint8_t* addr, int idx, int type);

/* WARNING: assembly needs to be updated for prototype changes of these functions */
int check_bool(lua_State* L, int idx);
double check_double(lua_State* L, int idx);
double check_complex_imag(lua_State* L, int idx);
float check_float(lua_State* L, int idx);
uint64_t check_uint64(lua_State* L, int idx);
int64_t check_int64(lua_State* L, int idx);
int32_t check_int32(lua_State* L, int idx);
uint32_t check_uint32(lua_State* L, int idx);
uintptr_t check_uintptr(lua_State* L, int idx);
int32_t check_enum(lua_State* L, int idx, int to_usr, const struct ctype* tt);
/* these two will always push a value so that we can create structs/functions on the fly */
void* check_typed_pointer(lua_State* L, int idx, int to_usr, const struct ctype* tt);
cfunction check_typed_cfunction(lua_State* L, int idx, int to_usr, const struct ctype* tt);
complex_double check_complex_double(lua_State* L, int idx);
complex_float check_complex_float(lua_State* L, int idx);

void unpack_varargs_stack(lua_State* L, int first, int last, char* to);
void unpack_varargs_reg(lua_State* L, int first, int last, char* to);

void unpack_varargs_stack_skip(lua_State* L, int first, int last, int ints_to_skip, int floats_to_skip, char* to);
void unpack_varargs_float(lua_State* L, int first, int last, int max, char* to);
void unpack_varargs_int(lua_State* L, int first, int last, int max, char* to);



 
/*
** DynASM encoding engine prototypes.
** Copyright (C) 2005-2011 Mike Pall. All rights reserved.
** Released under the MIT/X license. See dynasm.lua for full copyright notice.
*/

#ifndef _DASM_PROTO_H
#define _DASM_PROTO_H

#include <stddef.h>
#include <stdarg.h>

#define DASM_IDENT	"DynASM 1.3.0"
#define DASM_VERSION	10300	/* 1.3.0 */

#ifndef Dst_DECL
#define Dst_DECL	dasm_State **Dst
#endif

#ifndef Dst_REF
#define Dst_REF		(*Dst)
#endif

#ifndef DASM_FDEF
#define DASM_FDEF	extern
#endif

#ifndef DASM_M_GROW
#define DASM_M_GROW(ctx, t, p, sz, need) \
  do { \
    size_t _sz = (sz), _need = (need); \
    if (_sz < _need) { \
      if (_sz < 16) _sz = 16; \
      while (_sz < _need) _sz += _sz; \
      (p) = (t *)realloc((p), _sz); \
      if ((p) == NULL) exit(1); \
      (sz) = _sz; \
    } \
  } while(0)
#endif

#ifndef DASM_M_FREE
#define DASM_M_FREE(ctx, p, sz)	free(p)
#endif

/* Internal DynASM encoder state. */
typedef struct dasm_State dasm_State;


/* Initialize and free DynASM state. */
DASM_FDEF void dasm_init(Dst_DECL, int maxsection);
DASM_FDEF void dasm_free(Dst_DECL);

/* Setup global array. Must be called before dasm_setup(). */
DASM_FDEF void dasm_setupglobal(Dst_DECL, void **gl, unsigned int maxgl);

/* Grow PC label array. Can be called after dasm_setup(), too. */
DASM_FDEF void dasm_growpc(Dst_DECL, unsigned int maxpc);

/* Setup encoder. */
DASM_FDEF void dasm_setup(Dst_DECL, const void *actionlist);

/* Feed encoder with actions. Calls are generated by pre-processor. */
DASM_FDEF void dasm_put(Dst_DECL, int start, ...);

/* Link sections and return the resulting size. */
DASM_FDEF int dasm_link(Dst_DECL, size_t *szp);

/* Encode sections into buffer. */
DASM_FDEF int dasm_encode(Dst_DECL, void *buffer);

/* Get PC label offset. */
DASM_FDEF int dasm_getpclabel(Dst_DECL, unsigned int pc);

#ifdef DASM_CHECKS
/* Optional sanity checker to call between isolated encoding steps. */
DASM_FDEF int dasm_checkstep(Dst_DECL, int secmatch);
#else
#define dasm_checkstep(a, b)	0
#endif


#endif /* _DASM_PROTO_H */
 
#endif 
 
#ifdef LUAFFI_C  
 
static cfunction compile(Dst_DECL, lua_State* L, cfunction func, int ref); 
 
#if defined __arm__ || defined __arm || defined __ARM__ || defined __ARM || defined ARM || defined _ARM_ || defined ARMV4I || defined _M_ARM 
/*
** DynASM ARM encoding engine.
** Copyright (C) 2005-2011 Mike Pall. All rights reserved.
** Released under the MIT/X license. See dynasm.lua for full copyright notice.
*/

#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define DASM_ARCH		"arm"

#ifndef DASM_EXTERN
#define DASM_EXTERN(a,b,c,d)	0
#endif

/* Action definitions. */
enum {
  DASM_STOP, DASM_SECTION, DASM_ESC, DASM_REL_EXT,
  /* The following actions need a buffer position. */
  DASM_ALIGN, DASM_REL_LG, DASM_LABEL_LG,
  /* The following actions also have an argument. */
  DASM_REL_PC, DASM_LABEL_PC,
  DASM_LONG, DASM_IMM, DASM_IMM12, DASM_IMM16, DASM_IMML8, DASM_IMML12,
  DASM__MAX
};

/* Maximum number of section buffer positions for a single dasm_put() call. */
#define DASM_MAXSECPOS		25

/* DynASM encoder status codes. Action list offset or number are or'ed in. */
#define DASM_S_OK		0x00000000
#define DASM_S_NOMEM		0x01000000
#define DASM_S_PHASE		0x02000000
#define DASM_S_MATCH_SEC	0x03000000
#define DASM_S_RANGE_I		0x11000000
#define DASM_S_RANGE_SEC	0x12000000
#define DASM_S_RANGE_LG		0x13000000
#define DASM_S_RANGE_PC		0x14000000
#define DASM_S_RANGE_REL	0x15000000
#define DASM_S_UNDEF_LG		0x21000000
#define DASM_S_UNDEF_PC		0x22000000

/* Macros to convert positions (8 bit section + 24 bit index). */
#define DASM_POS2IDX(pos)	((pos)&0x00ffffff)
#define DASM_POS2BIAS(pos)	((pos)&0xff000000)
#define DASM_SEC2POS(sec)	((sec)<<24)
#define DASM_POS2SEC(pos)	((pos)>>24)
#define DASM_POS2PTR(D, pos)	(D->sections[DASM_POS2SEC(pos)].rbuf + (pos))

/* Action list type. */
typedef const unsigned int *dasm_ActList;

/* Per-section structure. */
typedef struct dasm_Section {
  int *rbuf;		/* Biased buffer pointer (negative section bias). */
  int *buf;		/* True buffer pointer. */
  size_t bsize;		/* Buffer size in bytes. */
  int pos;		/* Biased buffer position. */
  int epos;		/* End of biased buffer position - max single put. */
  int ofs;		/* Byte offset into section. */
} dasm_Section;

/* Core structure holding the DynASM encoding state. */
struct dasm_State {
  size_t psize;			/* Allocated size of this structure. */
  dasm_ActList actionlist;	/* Current actionlist pointer. */
  int *lglabels;		/* Local/global chain/pos ptrs. */
  size_t lgsize;
  int *pclabels;		/* PC label chains/pos ptrs. */
  size_t pcsize;
  void **globals;		/* Array of globals (bias -10). */
  dasm_Section *section;	/* Pointer to active section. */
  size_t codesize;		/* Total size of all code sections. */
  int maxsection;		/* 0 <= sectionidx < maxsection. */
  int status;			/* Status code. */
  dasm_Section sections[1];	/* All sections. Alloc-extended. */
};

/* The size of the core structure depends on the max. number of sections. */
#define DASM_PSZ(ms)	(sizeof(dasm_State)+(ms-1)*sizeof(dasm_Section))


/* Initialize DynASM state. */
void dasm_init(Dst_DECL, int maxsection)
{
  dasm_State *D;
  size_t psz = 0;
  int i;
  Dst_REF = NULL;
  DASM_M_GROW(Dst, struct dasm_State, Dst_REF, psz, DASM_PSZ(maxsection));
  D = Dst_REF;
  D->psize = psz;
  D->lglabels = NULL;
  D->lgsize = 0;
  D->pclabels = NULL;
  D->pcsize = 0;
  D->globals = NULL;
  D->maxsection = maxsection;
  for (i = 0; i < maxsection; i++) {
    D->sections[i].buf = NULL;  /* Need this for pass3. */
    D->sections[i].rbuf = D->sections[i].buf - DASM_SEC2POS(i);
    D->sections[i].bsize = 0;
    D->sections[i].epos = 0;  /* Wrong, but is recalculated after resize. */
  }
}

/* Free DynASM state. */
void dasm_free(Dst_DECL)
{
  dasm_State *D = Dst_REF;
  int i;
  for (i = 0; i < D->maxsection; i++)
    if (D->sections[i].buf)
      DASM_M_FREE(Dst, D->sections[i].buf, D->sections[i].bsize);
  if (D->pclabels) DASM_M_FREE(Dst, D->pclabels, D->pcsize);
  if (D->lglabels) DASM_M_FREE(Dst, D->lglabels, D->lgsize);
  DASM_M_FREE(Dst, D, D->psize);
}

/* Setup global label array. Must be called before dasm_setup(). */
void dasm_setupglobal(Dst_DECL, void **gl, unsigned int maxgl)
{
  dasm_State *D = Dst_REF;
  D->globals = gl - 10;  /* Negative bias to compensate for locals. */
  DASM_M_GROW(Dst, int, D->lglabels, D->lgsize, (10+maxgl)*sizeof(int));
}

/* Grow PC label array. Can be called after dasm_setup(), too. */
void dasm_growpc(Dst_DECL, unsigned int maxpc)
{
  dasm_State *D = Dst_REF;
  size_t osz = D->pcsize;
  DASM_M_GROW(Dst, int, D->pclabels, D->pcsize, maxpc*sizeof(int));
  memset((void *)(((unsigned char *)D->pclabels)+osz), 0, D->pcsize-osz);
}

/* Setup encoder. */
void dasm_setup(Dst_DECL, const void *actionlist)
{
  dasm_State *D = Dst_REF;
  int i;
  D->actionlist = (dasm_ActList)actionlist;
  D->status = DASM_S_OK;
  D->section = &D->sections[0];
  memset((void *)D->lglabels, 0, D->lgsize);
  if (D->pclabels) memset((void *)D->pclabels, 0, D->pcsize);
  for (i = 0; i < D->maxsection; i++) {
    D->sections[i].pos = DASM_SEC2POS(i);
    D->sections[i].ofs = 0;
  }
}


#ifdef DASM_CHECKS
#define CK(x, st) \
  do { if (!(x)) { \
    D->status = DASM_S_##st|(p-D->actionlist-1); return; } } while (0)
#define CKPL(kind, st) \
  do { if ((size_t)((char *)pl-(char *)D->kind##labels) >= D->kind##size) { \
    D->status = DASM_S_RANGE_##st|(p-D->actionlist-1); return; } } while (0)
#else
#define CK(x, st)	((void)0)
#define CKPL(kind, st)	((void)0)
#endif

static int dasm_imm12(unsigned int n)
{
  int i;
  for (i = 0; i < 16; i++, n = (n << 2) | (n >> 30))
    if (n <= 255) return (int)(n + (i << 8));
  return -1;
}

/* Pass 1: Store actions and args, link branches/labels, estimate offsets. */
void dasm_put(Dst_DECL, int start, ...)
{
  va_list ap;
  dasm_State *D = Dst_REF;
  dasm_ActList p = D->actionlist + start;
  dasm_Section *sec = D->section;
  int pos = sec->pos, ofs = sec->ofs;
  int *b;

  if (pos >= sec->epos) {
    DASM_M_GROW(Dst, int, sec->buf, sec->bsize,
      sec->bsize + 2*DASM_MAXSECPOS*sizeof(int));
    sec->rbuf = sec->buf - DASM_POS2BIAS(pos);
    sec->epos = (int)sec->bsize/sizeof(int) - DASM_MAXSECPOS+DASM_POS2BIAS(pos);
  }

  b = sec->rbuf;
  b[pos++] = start;

  va_start(ap, start);
  while (1) {
    unsigned int ins = *p++;
    unsigned int action = (ins >> 16);
    if (action >= DASM__MAX) {
      ofs += 4;
    } else {
      int *pl, n = action >= DASM_REL_PC ? va_arg(ap, int) : 0;
      switch (action) {
      case DASM_STOP: goto stop;
      case DASM_SECTION:
	n = (ins & 255); CK(n < D->maxsection, RANGE_SEC);
	D->section = &D->sections[n]; goto stop;
      case DASM_ESC: p++; ofs += 4; break;
      case DASM_REL_EXT: break;
      case DASM_ALIGN: ofs += (ins & 255); b[pos++] = ofs; break;
      case DASM_REL_LG:
	n = (ins & 2047) - 10; pl = D->lglabels + n;
	if (n >= 0) { CKPL(lg, LG); goto putrel; }  /* Bkwd rel or global. */
	pl += 10; n = *pl;
	if (n < 0) n = 0;  /* Start new chain for fwd rel if label exists. */
	goto linkrel;
      case DASM_REL_PC:
	pl = D->pclabels + n; CKPL(pc, PC);
      putrel:
	n = *pl;
	if (n < 0) {  /* Label exists. Get label pos and store it. */
	  b[pos] = -n;
	} else {
      linkrel:
	  b[pos] = n;  /* Else link to rel chain, anchored at label. */
	  *pl = pos;
	}
	pos++;
	break;
      case DASM_LABEL_LG:
	pl = D->lglabels + (ins & 2047) - 10; CKPL(lg, LG); goto putlabel;
      case DASM_LABEL_PC:
	pl = D->pclabels + n; CKPL(pc, PC);
      putlabel:
	n = *pl;  /* n > 0: Collapse rel chain and replace with label pos. */
	while (n > 0) { int *pb = DASM_POS2PTR(D, n); n = *pb; *pb = pos;
	}
	*pl = -pos;  /* Label exists now. */
	b[pos++] = ofs;  /* Store pass1 offset estimate. */
	break;
      case DASM_LONG:
        ofs += 4;
        b[pos++] = n;
        break;
      case DASM_IMM:
      case DASM_IMM16:
#ifdef DASM_CHECKS
	CK((n & ((1<<((ins>>10)&31))-1)) == 0, RANGE_I);
	if ((ins & 0x8000))
	  CK(((n + (1<<(((ins>>5)&31)-1)))>>((ins>>5)&31)) == 0, RANGE_I);
	else
	  CK((n>>((ins>>5)&31)) == 0, RANGE_I);
#endif
	b[pos++] = n;
	break;
      case DASM_IMML8:
      case DASM_IMML12:
	CK(n >= 0 ? ((n>>((ins>>5)&31)) == 0) :
		    (((-n)>>((ins>>5)&31)) == 0), RANGE_I);
	b[pos++] = n;
	break;
      case DASM_IMM12:
	CK(dasm_imm12((unsigned int)n) != -1, RANGE_I);
	b[pos++] = n;
	break;
      }
    }
  }
stop:
  va_end(ap);
  sec->pos = pos;
  sec->ofs = ofs;
}
#undef CK

/* Pass 2: Link sections, shrink aligns, fix label offsets. */
int dasm_link(Dst_DECL, size_t *szp)
{
  dasm_State *D = Dst_REF;
  int secnum;
  int ofs = 0;

#ifdef DASM_CHECKS
  *szp = 0;
  if (D->status != DASM_S_OK) return D->status;
  {
    int pc;
    for (pc = 0; pc*sizeof(int) < D->pcsize; pc++)
      if (D->pclabels[pc] > 0) return DASM_S_UNDEF_PC|pc;
  }
#endif

  { /* Handle globals not defined in this translation unit. */
    int idx;
    for (idx = 20; idx*sizeof(int) < D->lgsize; idx++) {
      int n = D->lglabels[idx];
      /* Undefined label: Collapse rel chain and replace with marker (< 0). */
      while (n > 0) { int *pb = DASM_POS2PTR(D, n); n = *pb; *pb = -idx; }
    }
  }

  /* Combine all code sections. No support for data sections (yet). */
  for (secnum = 0; secnum < D->maxsection; secnum++) {
    dasm_Section *sec = D->sections + secnum;
    int *b = sec->rbuf;
    int pos = DASM_SEC2POS(secnum);
    int lastpos = sec->pos;

    while (pos != lastpos) {
      dasm_ActList p = D->actionlist + b[pos++];
      while (1) {
	unsigned int ins = *p++;
	unsigned int action = (ins >> 16);
	switch (action) {
	case DASM_STOP: case DASM_SECTION: goto stop;
	case DASM_ESC: p++; break;
	case DASM_REL_EXT: break;
	case DASM_ALIGN: ofs -= (b[pos++] + ofs) & (ins & 255); break;
	case DASM_REL_LG: case DASM_REL_PC: pos++; break;
	case DASM_LABEL_LG: case DASM_LABEL_PC: b[pos++] += ofs; break;
        case DASM_LONG: case DASM_IMM: case DASM_IMM12: case DASM_IMM16:
	case DASM_IMML8: case DASM_IMML12: pos++; break;
	}
      }
      stop: (void)0;
    }
    ofs += sec->ofs;  /* Next section starts right after current section. */
  }

  D->codesize = ofs;  /* Total size of all code sections */
  *szp = ofs;
  return DASM_S_OK;
}

#ifdef DASM_CHECKS
#define CK(x, st) \
  do { if (!(x)) return DASM_S_##st|(p-D->actionlist-1); } while (0)
#else
#define CK(x, st)	((void)0)
#endif

/* Pass 3: Encode sections. */
int dasm_encode(Dst_DECL, void *buffer)
{
  dasm_State *D = Dst_REF;
  char *base = (char *)buffer;
  unsigned int *cp = (unsigned int *)buffer;
  int secnum;

  /* Encode all code sections. No support for data sections (yet). */
  for (secnum = 0; secnum < D->maxsection; secnum++) {
    dasm_Section *sec = D->sections + secnum;
    int *b = sec->buf;
    int *endb = sec->rbuf + sec->pos;

    while (b != endb) {
      dasm_ActList p = D->actionlist + *b++;
      while (1) {
	unsigned int ins = *p++;
	unsigned int action = (ins >> 16);
	int n = (action >= DASM_ALIGN && action < DASM__MAX) ? *b++ : 0;
	switch (action) {
	case DASM_STOP: case DASM_SECTION: goto stop;
	case DASM_ESC: *cp++ = *p++; break;
	case DASM_REL_EXT:
	  n = DASM_EXTERN(Dst, (unsigned char *)cp, (ins&2047), !(ins&2048));
	  goto patchrel;
	case DASM_ALIGN:
	  ins &= 255; while ((((char *)cp - base) & ins)) *cp++ = 0xe1a00000;
	  break;
	case DASM_REL_LG:
	  CK(n >= 0, UNDEF_LG);
	case DASM_REL_PC:
	  CK(n >= 0, UNDEF_PC);
	  n = *DASM_POS2PTR(D, n) - (int)((char *)cp - base) - 4;
	patchrel:
	  if ((ins & 0x800) == 0) {
	    CK((n & 3) == 0 && ((n+0x02000000) >> 26) == 0, RANGE_REL);
	    cp[-1] |= ((n >> 2) & 0x00ffffff);
	  } else if ((ins & 0x1000)) {
	    CK((n & 3) == 0 && -256 <= n && n <= 256, RANGE_REL);
	    goto patchimml8;
	  } else {
	    CK((n & 3) == 0 && -4096 <= n && n <= 4096, RANGE_REL);
	    goto patchimml12;
	  }
	  break;
	case DASM_LABEL_LG:
	  ins &= 2047; if (ins >= 20) D->globals[ins-10] = (void *)(base + n);
	  break;
	case DASM_LABEL_PC: break;
        case DASM_LONG:
          *cp++ = n;
          break;
	case DASM_IMM:
	  cp[-1] |= ((n>>((ins>>10)&31)) & ((1<<((ins>>5)&31))-1)) << (ins&31);
	  break;
	case DASM_IMM12:
	  cp[-1] |= dasm_imm12((unsigned int)n);
	  break;
	case DASM_IMM16:
	  cp[-1] |= ((n & 0xf000) << 4) | (n & 0x0fff);
	  break;
	case DASM_IMML8: patchimml8:
	  cp[-1] |= n >= 0 ? (0x00800000 | (n & 0x0f) | ((n & 0xf0) << 4)) :
			     ((-n & 0x0f) | ((-n & 0xf0) << 4));
	  break;
	case DASM_IMML12: patchimml12:
	  cp[-1] |= n >= 0 ? (0x00800000 | n) : (-n);
	  break;
	default: *cp++ = ins; break;
	}
      }
      stop: (void)0;
    }
  }

  if (base + D->codesize != (char *)cp)  /* Check for phase errors. */
    return DASM_S_PHASE;
  return DASM_S_OK;
}
#undef CK

/* Get PC label offset. */
int dasm_getpclabel(Dst_DECL, unsigned int pc)
{
  dasm_State *D = Dst_REF;
  if (pc*sizeof(int) < D->pcsize) {
    int pos = D->pclabels[pc];
    if (pos < 0) return *DASM_POS2PTR(D, -pos);
    if (pos > 0) return -1;  /* Undefined. */
  }
  return -2;  /* Unused or out of range. */
}

#ifdef DASM_CHECKS
/* Optional sanity checker to call between isolated encoding steps. */
int dasm_checkstep(Dst_DECL, int secmatch)
{
  dasm_State *D = Dst_REF;
  if (D->status == DASM_S_OK) {
    int i;
    for (i = 1; i <= 9; i++) {
      if (D->lglabels[i] > 0) { D->status = DASM_S_UNDEF_LG|i; break; }
      D->lglabels[i] = 0;
    }
  }
  if (D->status == DASM_S_OK && secmatch >= 0 &&
      D->section != &D->sections[secmatch])
    D->status = DASM_S_MATCH_SEC|(D->section-D->sections);
  return D->status;
}
#endif

#else 
/*
** DynASM x86 encoding engine.
** Copyright (C) 2005-2011 Mike Pall. All rights reserved.
** Released under the MIT/X license. See dynasm.lua for full copyright notice.
*/

#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define DASM_ARCH		"x86"

#ifndef DASM_EXTERN
#define DASM_EXTERN(a,b,c,d)	0
#endif

/* Action definitions. DASM_STOP must be 255. */
enum {
  DASM_DISP = 233,
  DASM_IMM_S, DASM_IMM_B, DASM_IMM_W, DASM_IMM_D, DASM_IMM_WB, DASM_IMM_DB,
  DASM_VREG, DASM_SPACE, DASM_SETLABEL, DASM_REL_A, DASM_REL_LG, DASM_REL_PC,
  DASM_IMM_LG, DASM_IMM_PC, DASM_LABEL_LG, DASM_LABEL_PC, DASM_ALIGN,
  DASM_EXTERN, DASM_ESC, DASM_MARK, DASM_SECTION, DASM_STOP
};

/* Maximum number of section buffer positions for a single dasm_put() call. */
#define DASM_MAXSECPOS		25

/* DynASM encoder status codes. Action list offset or number are or'ed in. */
#define DASM_S_OK		0x00000000
#define DASM_S_NOMEM		0x01000000
#define DASM_S_PHASE		0x02000000
#define DASM_S_MATCH_SEC	0x03000000
#define DASM_S_RANGE_I		0x11000000
#define DASM_S_RANGE_SEC	0x12000000
#define DASM_S_RANGE_LG		0x13000000
#define DASM_S_RANGE_PC		0x14000000
#define DASM_S_RANGE_VREG	0x15000000
#define DASM_S_UNDEF_L		0x21000000
#define DASM_S_UNDEF_PC		0x22000000

/* Macros to convert positions (8 bit section + 24 bit index). */
#define DASM_POS2IDX(pos)	((pos)&0x00ffffff)
#define DASM_POS2BIAS(pos)	((pos)&0xff000000)
#define DASM_SEC2POS(sec)	((sec)<<24)
#define DASM_POS2SEC(pos)	((pos)>>24)
#define DASM_POS2PTR(D, pos)	(D->sections[DASM_POS2SEC(pos)].rbuf + (pos))

/* Action list type. */
typedef const unsigned char *dasm_ActList;

/* Per-section structure. */
typedef struct dasm_Section {
  int *rbuf;		/* Biased buffer pointer (negative section bias). */
  int *buf;		/* True buffer pointer. */
  size_t bsize;		/* Buffer size in bytes. */
  int pos;		/* Biased buffer position. */
  int epos;		/* End of biased buffer position - max single put. */
  int ofs;		/* Byte offset into section. */
} dasm_Section;

/* Core structure holding the DynASM encoding state. */
struct dasm_State {
  size_t psize;			/* Allocated size of this structure. */
  dasm_ActList actionlist;	/* Current actionlist pointer. */
  int *lglabels;		/* Local/global chain/pos ptrs. */
  size_t lgsize;
  int *pclabels;		/* PC label chains/pos ptrs. */
  size_t pcsize;
  void **globals;		/* Array of globals (bias -10). */
  dasm_Section *section;	/* Pointer to active section. */
  size_t codesize;		/* Total size of all code sections. */
  int maxsection;		/* 0 <= sectionidx < maxsection. */
  int status;			/* Status code. */
  dasm_Section sections[1];	/* All sections. Alloc-extended. */
};

/* The size of the core structure depends on the max. number of sections. */
#define DASM_PSZ(ms)	(sizeof(dasm_State)+(ms-1)*sizeof(dasm_Section))


/* Initialize DynASM state. */
void dasm_init(Dst_DECL, int maxsection)
{
  dasm_State *D;
  size_t psz = 0;
  int i;
  Dst_REF = NULL;
  DASM_M_GROW(Dst, struct dasm_State, Dst_REF, psz, DASM_PSZ(maxsection));
  D = Dst_REF;
  D->psize = psz;
  D->lglabels = NULL;
  D->lgsize = 0;
  D->pclabels = NULL;
  D->pcsize = 0;
  D->globals = NULL;
  D->maxsection = maxsection;
  for (i = 0; i < maxsection; i++) {
    D->sections[i].buf = NULL;  /* Need this for pass3. */
    D->sections[i].rbuf = D->sections[i].buf - DASM_SEC2POS(i);
    D->sections[i].bsize = 0;
    D->sections[i].epos = 0;  /* Wrong, but is recalculated after resize. */
  }
}

/* Free DynASM state. */
void dasm_free(Dst_DECL)
{
  dasm_State *D = Dst_REF;
  int i;
  for (i = 0; i < D->maxsection; i++)
    if (D->sections[i].buf)
      DASM_M_FREE(Dst, D->sections[i].buf, D->sections[i].bsize);
  if (D->pclabels) DASM_M_FREE(Dst, D->pclabels, D->pcsize);
  if (D->lglabels) DASM_M_FREE(Dst, D->lglabels, D->lgsize);
  DASM_M_FREE(Dst, D, D->psize);
}

/* Setup global label array. Must be called before dasm_setup(). */
void dasm_setupglobal(Dst_DECL, void **gl, unsigned int maxgl)
{
  dasm_State *D = Dst_REF;
  D->globals = gl - 10;  /* Negative bias to compensate for locals. */
  DASM_M_GROW(Dst, int, D->lglabels, D->lgsize, (10+maxgl)*sizeof(int));
}

/* Grow PC label array. Can be called after dasm_setup(), too. */
void dasm_growpc(Dst_DECL, unsigned int maxpc)
{
  dasm_State *D = Dst_REF;
  size_t osz = D->pcsize;
  DASM_M_GROW(Dst, int, D->pclabels, D->pcsize, maxpc*sizeof(int));
  memset((void *)(((unsigned char *)D->pclabels)+osz), 0, D->pcsize-osz);
}

/* Setup encoder. */
void dasm_setup(Dst_DECL, const void *actionlist)
{
  dasm_State *D = Dst_REF;
  int i;
  D->actionlist = (dasm_ActList)actionlist;
  D->status = DASM_S_OK;
  D->section = &D->sections[0];
  memset((void *)D->lglabels, 0, D->lgsize);
  if (D->pclabels) memset((void *)D->pclabels, 0, D->pcsize);
  for (i = 0; i < D->maxsection; i++) {
    D->sections[i].pos = DASM_SEC2POS(i);
    D->sections[i].ofs = 0;
  }
}


#ifdef DASM_CHECKS
#define CK(x, st) \
  do { if (!(x)) { \
    D->status = DASM_S_##st|(int)(p-D->actionlist-1); return; } } while (0)
#define CKPL(kind, st) \
  do { if ((size_t)((char *)pl-(char *)D->kind##labels) >= D->kind##size) { \
    D->status=DASM_S_RANGE_##st|(int)(p-D->actionlist-1); return; } } while (0)
#else
#define CK(x, st)	((void)0)
#define CKPL(kind, st)	((void)0)
#endif

/* Pass 1: Store actions and args, link branches/labels, estimate offsets. */
void dasm_put(Dst_DECL, int start, ...)
{
  va_list ap;
  dasm_State *D = Dst_REF;
  dasm_ActList p = D->actionlist + start;
  dasm_Section *sec = D->section;
  int pos = sec->pos, ofs = sec->ofs, mrm = 4;
  int *b;

  if (pos >= sec->epos) {
    DASM_M_GROW(Dst, int, sec->buf, sec->bsize,
      sec->bsize + 2*DASM_MAXSECPOS*sizeof(int));
    sec->rbuf = sec->buf - DASM_POS2BIAS(pos);
    sec->epos = (int)sec->bsize/sizeof(int) - DASM_MAXSECPOS+DASM_POS2BIAS(pos);
  }

  b = sec->rbuf;
  b[pos++] = start;

  va_start(ap, start);
  while (1) {
    int action = *p++;
    if (action < DASM_DISP) {
      ofs++;
    } else if (action <= DASM_REL_A) {
      int n = va_arg(ap, int);
      b[pos++] = n;
      switch (action) {
      case DASM_DISP:
	if (n == 0) { if ((mrm&7) == 4) mrm = p[-2]; if ((mrm&7) != 5) break; }
      case DASM_IMM_DB: if (((n+128)&-256) == 0) goto ob;
      case DASM_REL_A: /* Assumes ptrdiff_t is int. !x64 */
      case DASM_IMM_D: ofs += 4; break;
      case DASM_IMM_S: CK(((n+128)&-256) == 0, RANGE_I); goto ob;
      case DASM_IMM_B: CK((n&-256) == 0, RANGE_I); ob: ofs++; break;
      case DASM_IMM_WB: if (((n+128)&-256) == 0) goto ob;
      case DASM_IMM_W: CK((n&-65536) == 0, RANGE_I); ofs += 2; break;
      case DASM_SPACE: p++; ofs += n; break;
      case DASM_SETLABEL: b[pos-2] = -0x40000000; break;  /* Neg. label ofs. */
      case DASM_VREG: CK((n&-8) == 0 && (n != 4 || (*p&1) == 0), RANGE_VREG);
	if (*p++ == 1 && *p == DASM_DISP) mrm = n; continue;
      }
      mrm = 4;
    } else {
      int *pl, n;
      switch (action) {
      case DASM_REL_LG:
      case DASM_IMM_LG:
	n = *p++; pl = D->lglabels + n;
	if (n <= 246) { CKPL(lg, LG); goto putrel; }  /* Bkwd rel or global. */
	pl -= 246; n = *pl;
	if (n < 0) n = 0;  /* Start new chain for fwd rel if label exists. */
	goto linkrel;
      case DASM_REL_PC:
      case DASM_IMM_PC: pl = D->pclabels + va_arg(ap, int); CKPL(pc, PC);
      putrel:
	n = *pl;
	if (n < 0) {  /* Label exists. Get label pos and store it. */
	  b[pos] = -n;
	} else {
      linkrel:
	  b[pos] = n;  /* Else link to rel chain, anchored at label. */
	  *pl = pos;
	}
	pos++;
	ofs += 4;  /* Maximum offset needed. */
	if (action == DASM_REL_LG || action == DASM_REL_PC)
	  b[pos++] = ofs;  /* Store pass1 offset estimate. */
	break;
      case DASM_LABEL_LG: pl = D->lglabels + *p++; CKPL(lg, LG); goto putlabel;
      case DASM_LABEL_PC: pl = D->pclabels + va_arg(ap, int); CKPL(pc, PC);
      putlabel:
	n = *pl;  /* n > 0: Collapse rel chain and replace with label pos. */
	while (n > 0) { int *pb = DASM_POS2PTR(D, n); n = *pb; *pb = pos; }
	*pl = -pos;  /* Label exists now. */
	b[pos++] = ofs;  /* Store pass1 offset estimate. */
	break;
      case DASM_ALIGN:
	ofs += *p++;  /* Maximum alignment needed (arg is 2**n-1). */
	b[pos++] = ofs;  /* Store pass1 offset estimate. */
	break;
      case DASM_EXTERN: p += 2; ofs += 4; break;
      case DASM_ESC: p++; ofs++; break;
      case DASM_MARK: mrm = p[-2]; break;
      case DASM_SECTION:
	n = *p; CK(n < D->maxsection, RANGE_SEC); D->section = &D->sections[n];
      case DASM_STOP: goto stop;
      }
    }
  }
stop:
  va_end(ap);
  sec->pos = pos;
  sec->ofs = ofs;
}
#undef CK

/* Pass 2: Link sections, shrink branches/aligns, fix label offsets. */
int dasm_link(Dst_DECL, size_t *szp)
{
  dasm_State *D = Dst_REF;
  int secnum;
  int ofs = 0;

#ifdef DASM_CHECKS
  *szp = 0;
  if (D->status != DASM_S_OK) return D->status;
  {
    int pc;
    for (pc = 0; pc*sizeof(int) < D->pcsize; pc++)
      if (D->pclabels[pc] > 0) return DASM_S_UNDEF_PC|pc;
  }
#endif

  { /* Handle globals not defined in this translation unit. */
    int idx;
    for (idx = 10; idx*sizeof(int) < D->lgsize; idx++) {
      int n = D->lglabels[idx];
      /* Undefined label: Collapse rel chain and replace with marker (< 0). */
      while (n > 0) { int *pb = DASM_POS2PTR(D, n); n = *pb; *pb = -idx; }
    }
  }

  /* Combine all code sections. No support for data sections (yet). */
  for (secnum = 0; secnum < D->maxsection; secnum++) {
    dasm_Section *sec = D->sections + secnum;
    int *b = sec->rbuf;
    int pos = DASM_SEC2POS(secnum);
    int lastpos = sec->pos;

    while (pos != lastpos) {
      dasm_ActList p = D->actionlist + b[pos++];
      while (1) {
	int op, action = *p++;
	switch (action) {
	case DASM_REL_LG: p++; op = p[-3]; goto rel_pc;
	case DASM_REL_PC: op = p[-2]; rel_pc: {
	  int shrink = op == 0xe9 ? 3 : ((op&0xf0) == 0x80 ? 4 : 0);
	  if (shrink) {  /* Shrinkable branch opcode? */
	    int lofs, lpos = b[pos];
	    if (lpos < 0) goto noshrink;  /* Ext global? */
	    lofs = *DASM_POS2PTR(D, lpos);
	    if (lpos > pos) {  /* Fwd label: add cumulative section offsets. */
	      int i;
	      for (i = secnum; i < DASM_POS2SEC(lpos); i++)
		lofs += D->sections[i].ofs;
	    } else {
	      lofs -= ofs;  /* Bkwd label: unfix offset. */
	    }
	    lofs -= b[pos+1];  /* Short branch ok? */
	    if (lofs >= -128-shrink && lofs <= 127) ofs -= shrink;  /* Yes. */
	    else { noshrink: shrink = 0; }  /* No, cannot shrink op. */
	  }
	  b[pos+1] = shrink;
	  pos += 2;
	  break;
	}
	case DASM_SPACE: case DASM_IMM_LG: case DASM_VREG: p++;
	case DASM_DISP: case DASM_IMM_S: case DASM_IMM_B: case DASM_IMM_W:
	case DASM_IMM_D: case DASM_IMM_WB: case DASM_IMM_DB:
	case DASM_SETLABEL: case DASM_REL_A: case DASM_IMM_PC: pos++; break;
	case DASM_LABEL_LG: p++;
	case DASM_LABEL_PC: b[pos++] += ofs; break; /* Fix label offset. */
	case DASM_ALIGN: ofs -= (b[pos++]+ofs)&*p++; break; /* Adjust ofs. */
	case DASM_EXTERN: p += 2; break;
	case DASM_ESC: p++; break;
	case DASM_MARK: break;
	case DASM_SECTION: case DASM_STOP: goto stop;
	}
      }
      stop: (void)0;
    }
    ofs += sec->ofs;  /* Next section starts right after current section. */
  }

  D->codesize = ofs;  /* Total size of all code sections */
  *szp = ofs;
  return DASM_S_OK;
}

#define dasmb(x)	*cp++ = (unsigned char)(x)
#ifndef DASM_ALIGNED_WRITES
#define dasmw(x) \
  do { *((unsigned short *)cp) = (unsigned short)(x); cp+=2; } while (0)
#define dasmd(x) \
  do { *((unsigned int *)cp) = (unsigned int)(x); cp+=4; } while (0)
#else
#define dasmw(x)	do { dasmb(x); dasmb((x)>>8); } while (0)
#define dasmd(x)	do { dasmw(x); dasmw((x)>>16); } while (0)
#endif

/* Pass 3: Encode sections. */
int dasm_encode(Dst_DECL, void *buffer)
{
  dasm_State *D = Dst_REF;
  unsigned char *base = (unsigned char *)buffer;
  unsigned char *cp = base;
  int secnum;

  /* Encode all code sections. No support for data sections (yet). */
  for (secnum = 0; secnum < D->maxsection; secnum++) {
    dasm_Section *sec = D->sections + secnum;
    int *b = sec->buf;
    int *endb = sec->rbuf + sec->pos;

    while (b != endb) {
      dasm_ActList p = D->actionlist + *b++;
      unsigned char *mark = NULL;
      while (1) {
	int action = *p++;
	int n = (action >= DASM_DISP && action <= DASM_ALIGN) ? *b++ : 0;
	switch (action) {
	case DASM_DISP: if (!mark) mark = cp; {
	  unsigned char *mm = mark;
	  if (*p != DASM_IMM_DB && *p != DASM_IMM_WB) mark = NULL;
	  if (n == 0) { int mrm = mm[-1]&7; if (mrm == 4) mrm = mm[0]&7;
	    if (mrm != 5) { mm[-1] -= 0x80; break; } }
	  if (((n+128) & -256) != 0) goto wd; else mm[-1] -= 0x40;
	}
	case DASM_IMM_S: case DASM_IMM_B: wb: dasmb(n); break;
	case DASM_IMM_DB: if (((n+128)&-256) == 0) {
	    db: if (!mark) mark = cp; mark[-2] += 2; mark = NULL; goto wb;
	  } else mark = NULL;
	case DASM_IMM_D: wd: dasmd(n); break;
	case DASM_IMM_WB: if (((n+128)&-256) == 0) goto db; else mark = NULL;
	case DASM_IMM_W: dasmw(n); break;
	case DASM_VREG: { int t = *p++; if (t >= 2) n<<=3; cp[-1] |= n; break; }
	case DASM_REL_LG: p++; if (n >= 0) goto rel_pc;
	  b++; n = (int)(ptrdiff_t)D->globals[-n];
	case DASM_REL_A: rel_a: n -= (int)(ptrdiff_t)(cp+4); goto wd; /* !x64 */
	case DASM_REL_PC: rel_pc: {
	  int shrink = *b++;
	  int *pb = DASM_POS2PTR(D, n); if (*pb < 0) { n = pb[1]; goto rel_a; }
	  n = *pb - ((int)(cp-base) + 4-shrink);
	  if (shrink == 0) goto wd;
	  if (shrink == 4) { cp--; cp[-1] = *cp-0x10; } else cp[-1] = 0xeb;
	  goto wb;
	}
	case DASM_IMM_LG:
	  p++; if (n < 0) { n = (int)(ptrdiff_t)D->globals[-n]; goto wd; }
	case DASM_IMM_PC: {
	  int *pb = DASM_POS2PTR(D, n);
	  n = *pb < 0 ? pb[1] : (*pb + (int)(ptrdiff_t)base);
	  goto wd;
	}
	case DASM_LABEL_LG: {
	  int idx = *p++;
	  if (idx >= 10)
	    D->globals[idx] = (void *)(base + (*p == DASM_SETLABEL ? *b : n));
	  break;
	}
	case DASM_LABEL_PC: case DASM_SETLABEL: break;
	case DASM_SPACE: { int fill = *p++; while (n--) *cp++ = fill; break; }
	case DASM_ALIGN:
	  n = *p++;
	  while (((cp-base) & n)) *cp++ = 0x90; /* nop */
	  break;
	case DASM_EXTERN: n = DASM_EXTERN(Dst, cp, p[1], *p); p += 2; goto wd;
	case DASM_MARK: mark = cp; break;
	case DASM_ESC: action = *p++;
	default: *cp++ = action; break;
	case DASM_SECTION: case DASM_STOP: goto stop;
	}
      }
      stop: (void)0;
    }
  }

  if (base + D->codesize != cp)  /* Check for phase errors. */
    return DASM_S_PHASE;
  return DASM_S_OK;
}

/* Get PC label offset. */
int dasm_getpclabel(Dst_DECL, unsigned int pc)
{
  dasm_State *D = Dst_REF;
  if (pc*sizeof(int) < D->pcsize) {
    int pos = D->pclabels[pc];
    if (pos < 0) return *DASM_POS2PTR(D, -pos);
    if (pos > 0) return -1;  /* Undefined. */
  }
  return -2;  /* Unused or out of range. */
}

#ifdef DASM_CHECKS
/* Optional sanity checker to call between isolated encoding steps. */
int dasm_checkstep(Dst_DECL, int secmatch)
{
  dasm_State *D = Dst_REF;
  if (D->status == DASM_S_OK) {
    int i;
    for (i = 1; i <= 9; i++) {
      if (D->lglabels[i] > 0) { D->status = DASM_S_UNDEF_L|i; break; }
      D->lglabels[i] = 0;
    }
  }
  if (D->status == DASM_S_OK && secmatch >= 0 &&
      D->section != &D->sections[secmatch])
    D->status = DASM_S_MATCH_SEC|(int)(D->section-D->sections);
  return D->status;
}
#endif

#endif 
 
#if defined __arm__ || defined __arm || defined __ARM__ || defined __ARM || defined ARM || defined _ARM_ || defined ARMV4I || defined _M_ARM 
/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM arm version 1.3.0
** DO NOT EDIT! The original file is in "call_arm.dasc".
*/

#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif

/* vim: ts=4 sw=4 sts=4 et tw=78
 * Copyright (c) 2011 James R. McKaskill. See license in ffi.h
 */
static const unsigned int build_actionlist[571] = {
0xe1a0c00d,
0xe92d000f,
0xe92d50f0,
0xe24c6010,
0xe59f5008,
0xe59f2008,
0xe59f1008,
0xea000000,
0x00050001,
0x00090000,
0x00090000,
0x00090000,
0x0006000b,
0xe1a00005,
0xeb000000,
0x00030000,
0x00000000,
0xe3a02000,
0x000b0000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030000,
0x00000000,
0xe3a02000,
0x000b0000,
0xe3e01000,
0x000b0000,
0xe1a00005,
0xeb000000,
0x00030000,
0xe59f2000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030001,
0xe4962004,
0xe5802000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x00030002,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030003,
0xe59f2000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030001,
0xe4962004,
0xe4963004,
0xe5802000,
0xe5803004,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x00030002,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030003,
0xe59f2000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030001,
0xe4962004,
0xe5802000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x00030002,
0x00000000,
0xe4961004,
0xe1a00005,
0xeb000000,
0x00030004,
0x00000000,
0xe4961004,
0xe1a01c01,
0xe1a01c41,
0xe1a00005,
0xeb000000,
0x00030005,
0x00000000,
0xe4961004,
0xe20110ff,
0xe1a00005,
0xeb000000,
0x00030006,
0x00000000,
0xe4961004,
0xe1a01801,
0xe1a01841,
0xe1a00005,
0xeb000000,
0x00030005,
0x00000000,
0xe4961004,
0xe1a01801,
0xe1a01821,
0xe1a00005,
0xeb000000,
0x00030006,
0x00000000,
0xe4961004,
0xe1a00005,
0xeb000000,
0x00030005,
0x00000000,
0xe4961004,
0xe1a00005,
0xeb000000,
0x00030006,
0x00000000,
0xe4961004,
0xe1a00005,
0xeb000000,
0x00030007,
0x00000000,
0xe8b60006,
0xe1a00005,
0xeb000000,
0x00030008,
0x00000000,
0xe3a03000,
0xe3a02000,
0x000b0000,
0xe3a01000,
0x000b0000,
0xe1a00005,
0xeb000000,
0x00030009,
0x00000000,
0xe3a02000,
0x000b0000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x00030000,
0xe59f3000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3a02000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x0003000a,
0xe1a06000,
0xe3e01003,
0xe1a00005,
0xeb000000,
0x0003000b,
0xe1a00006,
0x00000000,
0xe3a02000,
0x000b0000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x00030000,
0xe59f3000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e02000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x0003000c,
0xe1a06000,
0xe3e01003,
0xe1a00005,
0xeb000000,
0x0003000b,
0xe1a00006,
0x00000000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x0003000b,
0x00000000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x0003000d,
0x00000000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x0003000e,
0x00000000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x0003000f,
0x00000000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030010,
0x00000000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030011,
0x00000000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030012,
0x00000000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030013,
0x00000000,
0xe1a06000,
0xe3e01002,
0xe1a00005,
0xeb000000,
0x0003000b,
0xe1a00006,
0x00000000,
0xe1a06000,
0xe1a07001,
0xe3e01002,
0xe1a00005,
0xeb000000,
0x0003000b,
0xe1a00006,
0xe1a01007,
0x00000000,
0xe89da0f0,
0x00000000,
0xe1a0c00d,
0xe92d0001,
0xe92d58f0,
0xe24cb004,
0xe1a05000,
0xe1a00005,
0xeb000000,
0x00030014,
0xe1a04000,
0xe3540000,
0x000b0000,
0x00000000,
0xaa000000,
0x00050001,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe1a00005,
0xeb000000,
0x00030015,
0x0006000b,
0x00000000,
0x0a000000,
0x00050001,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe1a00005,
0xeb000000,
0x00030015,
0x0006000b,
0x00000000,
0xe04dd184,
0xe24dd010,
0xe1a0600d,
0x00000000,
0xe59f3004,
0xe59f2004,
0xea000000,
0x00050001,
0x00090000,
0x00090000,
0x0006000b,
0xe3a01000,
0x000b0000,
0xe1a00005,
0x00000000,
0xeb000000,
0x0003000a,
0x00000000,
0xeb000000,
0x00030016,
0x00000000,
0xeb000000,
0x0003000c,
0x00000000,
0xe4860004,
0x00000000,
0xe3a01000,
0x000b0000,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000d,
0xe1a00c00,
0xe1a00c40,
0xe4860004,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000d,
0xe1a00800,
0xe1a00840,
0xe4860004,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000d,
0xe4860004,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000e,
0xe20000ff,
0xe4860004,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000e,
0xe1a00800,
0xe1a00820,
0xe4860004,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000e,
0xe4860004,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000f,
0xe4860004,
0xe4861004,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030010,
0xe4860004,
0xe4861004,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030013,
0xe4860004,
0xe4861004,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030011,
0xe4860004,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030012,
0xe4860004,
0x00000000,
0xe1a03006,
0xe1a02004,
0xe3a01000,
0x000b0000,
0xe1a00005,
0xeb000000,
0x00030017,
0x00000000,
0xe59f0000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5900000,
0xeb000000,
0x00030018,
0x00000000,
0xe8bd000f,
0xeb000000,
0x00030019,
0x00000000,
0xe1a06000,
0xeb000000,
0x0003001a,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe59f2004,
0xe59f1004,
0xea000000,
0x00050001,
0x00090000,
0x00090000,
0x0006000b,
0xe1a00005,
0xeb000000,
0x00030001,
0xe5806000,
0xe3a00001,
0xe91ba870,
0x00000000,
0xe1a06000,
0xe1a07001,
0xeb000000,
0x0003001a,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a00005,
0xeb000000,
0x00030003,
0xe59f2000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030001,
0xe5806000,
0xe5807004,
0xe3a00001,
0xe91ba870,
0x00000000,
0xe1a06000,
0xeb000000,
0x0003001a,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a00005,
0xeb000000,
0x00030003,
0xe59f2000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030001,
0xe5806000,
0xe3a00001,
0xe91ba870,
0x00000000,
0xeb000000,
0x0003001a,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe3a00000,
0xe91ba870,
0x00000000,
0xe1a06000,
0xeb000000,
0x0003001a,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a01006,
0xe1a00005,
0xeb000000,
0x00030004,
0xe3a00001,
0xe91ba870,
0x00000000,
0xe1a06000,
0xeb000000,
0x0003001a,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a01006,
0xe1a00005,
0xeb000000,
0x00030005,
0xe3a00001,
0xe91ba870,
0x00000000,
0xe1a06000,
0xeb000000,
0x0003001a,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a01006,
0xe1a00005,
0xeb000000,
0x00030006,
0xe3a00001,
0xe91ba870,
0x00000000,
0xe1a06000,
0xeb000000,
0x0003001a,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a01006,
0xe1a00005,
0xeb000000,
0x00030007,
0xe3a00001,
0xe91ba870,
0x00000000,
0xe1a06000,
0xe1a07001,
0xeb000000,
0x0003001a,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a02007,
0xe1a01006,
0xe1a00005,
0xeb000000,
0x00030008,
0xe3a00001,
0xe91ba870,
0x00000000
};

static const char *const globnames[] = {
  (const char *)0
};
static const char *const extnames[] = {
  "lua_rawgeti",
  "push_cdata",
  "lua_remove",
  "lua_pushnil",
  "lua_pushboolean",
  "push_int",
  "push_uint",
  "push_float",
  "lua_pushnumber",
  "lua_callk",
  "to_typed_pointer",
  "lua_settop",
  "to_enum",
  "to_int32",
  "to_uint32",
  "to_int64",
  "to_uint64",
  "to_uintptr",
  "to_float",
  "to_double",
  "lua_gettop",
  "luaL_error",
  "to_typed_function",
  "unpack_varargs_stack",
  "SetLastError",
  "FUNCTION",
  "GetLastError",
  (const char *)0
};

#define JUMP_SIZE 8
#define MIN_BRANCH ((INT32_MIN) >> 8)
#define MAX_BRANCH ((INT32_MAX) >> 8)
#define BRANCH_OFF 4

static void compile_extern_jump(struct jit* jit, lua_State* L, function_t func, uint8_t* code)
{
    /* The jump code is the function pointer followed by a stub to call the
     * function pointer. The stub exists so we can jump to functions with an
     * offset greater than 32MB.
     *
     * Note we have to manually set this up since there are commands buffered
     * in the jit state.
     */
    *(function_t*) code = func;
    /* ldr pc, [pc - 12] */
    *(uint32_t*) &code[4] = 0xE51FF00CU;
}




void compile_globals(struct jit* jit, lua_State* L)
{
    (void) jit;
}

function_t push_callback(struct jit* jit, lua_State* L, int fidx, int ct_usr, const struct ctype* ct)
{
    struct jit* Dst = jit;
    int i, nargs, num_upvals, ref;
    const struct ctype* mt;

    int top = lua_gettop(L);

    ct_usr = lua_absindex(L, ct_usr);
    fidx = lua_absindex(L, fidx);
    nargs = (int) lua_rawlen(L, ct_usr);

    dasm_setup(Dst, build_actionlist);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
    num_upvals = 0;

    if (ct->has_var_arg) {
        luaL_error(L, "can't create callbacks with varargs");
    }

    /* prolog and get the upval table */
    dasm_put(Dst, 0, (uintptr_t)(L), (uintptr_t)(ref), (uintptr_t)(LUA_REGISTRYINDEX));

    /* get the lua function */
    lua_pushvalue(L, fidx);
    lua_rawseti(L, -2, ++num_upvals);
    dasm_put(Dst, 17, num_upvals);

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);

        if (mt->pointers) {
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */

            dasm_put(Dst, 24, num_upvals-1, i, (uintptr_t)(mt));

        } else {
            switch (mt->type) {
            case INT64_TYPE:
            case UINT64_TYPE:
                lua_rawseti(L, -2, ++num_upvals); /* mt */
                dasm_put(Dst, 47, (uintptr_t)(mt));
                break;

            case UINTPTR_TYPE:
                lua_rawseti(L, -2, ++num_upvals); /* mt */
                dasm_put(Dst, 68, (uintptr_t)(mt));
                break;

            case BOOL_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 87);
                break;

            case INT8_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 92);
                break;

            case UINT8_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 99);
                break;

            case INT16_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 105);
                break;

            case UINT16_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 112);
                break;

            case ENUM_TYPE:
            case INT32_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 119);
                break;

            case UINT32_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 124);
                break;

            case FLOAT_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 129);
                break;

            case DOUBLE_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 134);
                break;

            default:
                luaL_error(L, "NYI: callback arg type");
            }
        }
    }

    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);

    dasm_put(Dst, 139, ((mt->pointers || mt->type != VOID_TYPE) ? 1 : 0), nargs);

    if (mt->pointers) {
        lua_getuservalue(L, -1);
        lua_rawseti(L, -3, ++num_upvals); /* usr value */
        lua_rawseti(L, -2, ++num_upvals); /* mt */

        dasm_put(Dst, 148, num_upvals-1, (uintptr_t)(mt));
    } else {
        switch (mt->type) {
        case ENUM_TYPE:
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */

            dasm_put(Dst, 171, num_upvals-1, (uintptr_t)(mt));
            break;

        case VOID_TYPE:
            dasm_put(Dst, 194);
            lua_pop(L, 1);
            break;

        case BOOL_TYPE:
        case INT8_TYPE:
        case INT16_TYPE:
        case INT32_TYPE:
            dasm_put(Dst, 199);
            goto single;

        case UINT8_TYPE:
        case UINT16_TYPE:
        case UINT32_TYPE:
            dasm_put(Dst, 204);
            goto single;

        case INT64_TYPE:
            dasm_put(Dst, 209);
            goto dual;

        case UINT64_TYPE:
            dasm_put(Dst, 214);
            goto dual;

        case UINTPTR_TYPE:
            dasm_put(Dst, 219);
            goto single;

        case FLOAT_TYPE:
            dasm_put(Dst, 224);
            goto single;

        case DOUBLE_TYPE:
            dasm_put(Dst, 229);
            goto dual;

        single:
            dasm_put(Dst, 234);
            lua_pop(L, 1);
            break;

        dual:
            dasm_put(Dst, 241);
            lua_pop(L, 1);
            break;

        default:
            luaL_error(L, "NYI: callback return type");
        }
    }

    dasm_put(Dst, 250);

    lua_pop(L, 1); /* upval table - already in registry */
    assert(lua_gettop(L) == top);

    {
        void* p;
        struct ctype ft;
        function_t func;

        func = compile(jit, L, NULL, ref);

        ft = *ct;
        ft.is_jitted = 1;
        p = push_cdata(L, ct_usr, &ft);
        *(function_t*) p = func;

        assert(lua_gettop(L) == top + 1);

        return func;
    }
}

void push_function(struct jit* jit, lua_State* L, function_t func, int ct_usr, const struct ctype* ct)
{
    struct jit* Dst = jit;
    int i, nargs, num_upvals;
    const struct ctype* mt;
    void* p;

    int top = lua_gettop(L);

    ct_usr = lua_absindex(L, ct_usr);
    nargs = (int) lua_rawlen(L, ct_usr);

    p = push_cdata(L, ct_usr, ct);
    *(function_t*) p = func;
    num_upvals = 1;

    dasm_setup(Dst, build_actionlist);

    dasm_put(Dst, 252, nargs);
    if (ct->has_var_arg) {
        dasm_put(Dst, 264, (uintptr_t)("too few arguments"));
    } else {
        dasm_put(Dst, 276, (uintptr_t)("incorrect number of arguments"));
    }

    /* reserve enough stack space for all of the arguments (8 bytes per
     * argument for double and maintains alignment). Add an extra 16 bytes so
     * that the pop {r0, r1, r2, r3} doesn't clean out our stack frame */
    dasm_put(Dst, 288);

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);

        if (mt->pointers || mt->type == FUNCTION_PTR_TYPE || mt->type == ENUM_TYPE) {
            lua_getuservalue(L, -1);
            num_upvals += 2;

            dasm_put(Dst, 292, (uintptr_t)(mt), (uintptr_t)(lua_upvalueindex(num_upvals)), i);

            if (mt->pointers) {
                dasm_put(Dst, 303);
            } else if (mt->type == FUNCTION_PTR_TYPE) {
                dasm_put(Dst, 306);
            } else if (mt->type == ENUM_TYPE) {
                dasm_put(Dst, 309);
            }

            dasm_put(Dst, 312);

        } else {
            lua_pop(L, 1);
            dasm_put(Dst, 314, i);

            switch (mt->type) {
            case INT8_TYPE:
                dasm_put(Dst, 317);
                break;

            case INT16_TYPE:
                dasm_put(Dst, 324);
                break;

            case INT32_TYPE:
                dasm_put(Dst, 331);
                break;

            case UINT8_TYPE:
                dasm_put(Dst, 336);
                break;

            case UINT16_TYPE:
                dasm_put(Dst, 342);
                break;

            case UINT32_TYPE:
                dasm_put(Dst, 349);
                break;

            case INT64_TYPE:
                dasm_put(Dst, 354);
                break;

            case UINT64_TYPE:
                dasm_put(Dst, 360);
                break;

            case DOUBLE_TYPE:
                dasm_put(Dst, 366);
                break;

            case UINTPTR_TYPE:
                dasm_put(Dst, 372);
                break;

            case FLOAT_TYPE:
                dasm_put(Dst, 377);
                break;

            default:
                luaL_error(L, "NYI: call arg type");
            }
        }
    }

    if (ct->has_var_arg) {
        dasm_put(Dst, 382, nargs+1);
    }

    dasm_put(Dst, 390, (uintptr_t)(&jit->last_errno));

    dasm_put(Dst, 399);



    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);

    if (mt->pointers) {
        lua_getuservalue(L, -1);
        num_upvals += 2;
        dasm_put(Dst, 403, (uintptr_t)(&jit->last_errno), (uintptr_t)(mt), (uintptr_t)(lua_upvalueindex(num_upvals)));

    } else {
        switch (mt->type) {
        case INT64_TYPE:
        case UINT64_TYPE:
            num_upvals++;
            dasm_put(Dst, 426, (uintptr_t)(&jit->last_errno), (uintptr_t)(mt));
            break;

        case UINTPTR_TYPE:
            num_upvals++;
            dasm_put(Dst, 453, (uintptr_t)(&jit->last_errno), (uintptr_t)(mt));
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 478, (uintptr_t)(&jit->last_errno));
            break;

        case BOOL_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 489, (uintptr_t)(&jit->last_errno));
            break;

        case INT8_TYPE:
        case INT16_TYPE:
        case INT32_TYPE:
        case ENUM_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 505, (uintptr_t)(&jit->last_errno));
            break;

        case UINT8_TYPE:
        case UINT16_TYPE:
        case UINT32_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 521, (uintptr_t)(&jit->last_errno));
            break;

        case FLOAT_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 537, (uintptr_t)(&jit->last_errno));
            break;

        case DOUBLE_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 553, (uintptr_t)(&jit->last_errno));
            break;

        default:
            luaL_error(L, "NYI: call return type");
        }
    }

    assert(lua_gettop(L) == top + num_upvals);
    lua_pushcclosure(L, (lua_CFunction) compile(jit, L, func, LUA_NOREF), num_upvals);
}

#elif defined _WIN64 
/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM x64 version 1.3.0
** DO NOT EDIT! The original file is in "call_x86.dasc".
*/

#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif

/* vim: ts=4 sw=4 sts=4 et tw=78
 * Copyright (c) 2011 James R. McKaskill. See license in ffi.h
 */

static const unsigned char build_actionlist[1965] = {
  248,10,184,1,0.0,0.0,0.0,76,139,109,252,240,76,139,101,252,248,72,137,252,
  236,93,195,255,248,11,232,251,1,0,72,185,237,237,137,1,184,0,0.0,0.0,0.0,
  76,139,109,252,240,76,139,101,252,248,72,137,252,236,93,195,255,248,12,102.0,
  15.0,214,68,36,32,232,251,1,0,72,185,237,237,137,1,252,243.0,15.0,126,76,
  36,32,76,137,225,232,251,1,1,252,233,244,10,255,248,13,15.0,182,192,137,68,
  36,32,232,251,1,0,72,185,237,237,137,1,139,68,36,32,72,137,194,76,137,225,
  232,251,1,2,252,233,244,10,255,248,14,137,68,36,32,232,251,1,0,72,185,237,
  237,137,1,139,68,36,32,72,137,194,76,137,225,232,251,1,3,252,233,244,10,255,
  248,15,137,68,36,32,232,251,1,0,72,185,237,237,137,1,139,68,36,32,72,137,
  194,76,137,225,232,251,1,4,252,233,244,10,255,248,16,102,184,0,0.0,72,186,
  237,237,76,137,225,232,251,1,5,255,248,17,102,184,0,0.0,72,186,237,237,76,
  137,225,232,251,1,5,255,248,18,72,137,77,16,72,137,85,24,76,137,69,32,76,
  137,77,40,102.0,15.0,214,69,252,240,102.0,15.0,214,77,232,102.0,15.0,214,
  85,224,102.0,15.0,214,93,216,195,255,72,139,141,233,255,72,137,132,253,36,
  233,255,221.0,133,233,255,217.0,133,233,255,252,243.0,15.0,126,133,233,255,
  252,243.0,15.0,90,133,233,255,221.0,156,253,36,233,255,217.0,156,253,36,233,
  255,102.0,15.0,214,132,253,36,233,255,252,242.0,15.0,90,192,102.0,15.0,214,
  132,253,36,233,255,252,242.0,15.0,90,192,102.0,15.0,126,132,253,36,233,255,
  85,72,137,229,65,84,72,129.0,252,236,239,232,244,18,255,73,188,237,237,255,
  73,199.0,192,237,72,199.0,194,237,76,137,225,232,251,1,6,255,73,199.0,192,
  237,72,199.0,194,252,255,252,255.0,252,255.0,252,255.0,76,137,225,232,251,
  1,6,255,73,199.0,192,237,72,199.0,194,237,76,137,225,232,251,1,6,73,184,237,
  237,72,199.0,194,252,255,252,255.0,252,255.0,252,255.0,76,137,225,232,251,
  1,7,255,72,137,8,72,199.0,194,252,254,252,255.0,252,255.0,252,255.0,76,137,
  225,232,251,1,8,255,73,184,237,237,72,199.0,194,0,0.0,0.0,0.0,76,137,225,
  232,251,1,7,255,72,137,8,255,102.0,15.0,214,0,255,217.0,24,255,217.0,88,4,
  255,102.0,15.0,214,64,8,255,252,243.0,15.0,126,200,76,137,225,232,251,1,1,
  255,15.0,182,201,72,137,202,76,137,225,232,251,1,2,255,15.0,182,201,255,15.0,
  190,201,255,72,137,202,76,137,225,232,251,1,3,255,15.0,183,201,255,15.0,191,
  201,255,72,137,202,76,137,225,232,251,1,4,255,73,185,237,237,73,199.0,192,
  237,72,199.0,194,237,76,137,225,232,251,1,9,255,73,199.0,192,237,72,199.0,
  194,252,254,252,255.0,252,255.0,252,255.0,76,137,225,232,251,1,6,73,185,237,
  237,73,199.0,192,252,255,252,255.0,252,255.0,252,255.0,72,199.0,194,252,254,
  252,255.0,252,255.0,252,255.0,76,137,225,232,251,1,10,72,137,68,36,32,72,
  199.0,194,252,252,252,255.0,252,255.0,252,255.0,76,137,225,232,251,1,11,72,
  139,68,36,32,255,73,199.0,192,237,72,199.0,194,252,254,252,255.0,252,255.0,
  252,255.0,76,137,225,232,251,1,6,73,185,237,237,73,199.0,192,252,255,252,
  255.0,252,255.0,252,255.0,72,199.0,194,252,254,252,255.0,252,255.0,252,255.0,
  76,137,225,232,251,1,12,137,68,36,32,72,199.0,194,252,252,252,255.0,252,255.0,
  252,255.0,76,137,225,232,251,1,11,139,68,36,32,255,72,199.0,194,252,254,252,
  255.0,252,255.0,252,255.0,76,137,225,232,251,1,11,255,72,199.0,194,252,255,
  252,255.0,252,255.0,252,255.0,76,137,225,232,251,1,13,255,72,199.0,194,252,
  255,252,255.0,252,255.0,252,255.0,76,137,225,232,251,1,14,255,137,68,36,32,
  72,199.0,194,252,253,252,255.0,252,255.0,252,255.0,76,137,225,232,251,1,11,
  139,68,36,32,255,72,199.0,194,252,255,252,255.0,252,255.0,252,255.0,76,137,
  225,232,251,1,15,255,72,199.0,194,252,255,252,255.0,252,255.0,252,255.0,76,
  137,225,232,251,1,16,255,72,137,68,36,32,72,199.0,194,252,253,252,255.0,252,
  255.0,252,255.0,76,137,225,232,251,1,11,72,139,68,36,32,255,72,199.0,194,
  252,255,252,255.0,252,255.0,252,255.0,76,137,225,232,251,1,17,72,137,68,36,
  32,72,199.0,194,252,253,252,255.0,252,255.0,252,255.0,76,137,225,232,251,
  1,11,72,139,68,36,32,255,72,199.0,194,252,255,252,255.0,252,255.0,252,255.0,
  76,137,225,232,251,1,18,102.0,15.0,214,68,36,32,72,199.0,194,252,253,252,
  255.0,252,255.0,252,255.0,76,137,225,232,251,1,11,255,252,242.0,15.0,90,68,
  36,32,255,252,243.0,15.0,126,68,36,32,255,72,199.0,194,252,255,252,255.0,
  252,255.0,252,255.0,76,137,225,232,251,1,19,102.0,15.0,214,68,36,32,72,199.0,
  194,252,253,252,255.0,252,255.0,252,255.0,76,137,225,232,251,1,11,252,243.0,
  15.0,126,68,36,32,255,72,199.0,194,252,255,252,255.0,252,255.0,252,255.0,
  76,137,225,232,251,1,20,102.0,15.0,214,68,36,32,102.0,15.0,214,76,36,40,72,
  199.0,194,252,253,252,255.0,252,255.0,252,255.0,76,137,225,232,251,1,11,252,
  243.0,15.0,126,68,36,32,252,243.0,15.0,126,76,36,40,255,72,139,141,233,73,
  199.0,192,252,255,252,255.0,252,255.0,252,255.0,76,137,226,72,137,201,232,
  251,1,20,72,131.0,252,236,4,72,199.0,194,252,253,252,255.0,252,255.0,252,
  255.0,76,137,225,232,251,1,11,255,76,139,101,252,248,72,137,252,236,93,194,
  236,255,85,72,137,229,65,84,65,85,73,137,204,72,131.0,252,236,32,76,137,225,
  232,251,1,21,73,137,197,72,129.0,252,248,239,15.0,140,244,16,255,15.0,143,
  244,17,255,72,193.0,224,4,72,41,196,72,129.0,252,236,239,255,73,184,237,237,
  72,199.0,194,0,0.0,0.0,0.0,76,137,225,232,251,1,7,72,131.0,252,236,16,255,
  73,185,237,237,73,199.0,192,237,72,199.0,194,237,76,137,225,232,251,1,10,
  255,73,185,237,237,73,199.0,192,237,72,199.0,194,237,76,137,225,232,251,1,
  22,255,73,185,237,237,73,199.0,192,237,72,199.0,194,237,76,137,225,232,251,
  1,12,255,72,199.0,194,237,76,137,225,232,251,1,14,255,15.0,182,192,255,15.0,
  190,192,255,15.0,183,192,255,15.0,191,192,255,72,199.0,194,237,76,137,225,
  232,251,1,14,131.0,252,248,0,15.0,149.0,208,15.0,182,192,255,72,199.0,194,
  237,76,137,225,232,251,1,13,255,72,199.0,194,237,76,137,225,232,251,1,17,
  255,72,199.0,194,237,76,137,225,232,251,1,15,255,72,199.0,194,237,76,137,
  225,232,251,1,16,255,72,199.0,194,237,76,137,225,232,251,1,18,255,72,199.0,
  194,237,76,137,225,232,251,1,20,255,252,243.0,15.0,126,193,255,72,141,132,
  253,36,233,72,131.0,252,236,4,73,199.0,192,237,76,137,226,72,137,193,232,
  251,1,20,255,72,199.0,194,237,76,137,225,232,251,1,19,255,72,199.0,194,237,
  76,137,225,232,251,1,19,137,4,36,217.0,4,36,255,137,20,36,217.0,4,36,255,
  73,129.0,252,253,239,15.0,142,244,247,72,137,224,72,129.0,192,239,73,137,
  193,77,137,232,72,199.0,194,237,76,137,225,232,251,1,23,72,137,224,72,129.0,
  192,239,73,137,193,73,199.0,192,237,72,199.0,194,237,76,137,225,232,251,1,
  24,252,233,244,248,248,1,72,137,224,72,129.0,192,239,73,137,193,77,137,232,
  72,199.0,194,237,76,137,225,232,251,1,24,248,2,255,72,137,224,72,129.0,192,
  239,73,137,193,77,137,232,72,199.0,194,237,76,137,225,232,251,1,23,255,72,
  185,237,237,139,1,72,137,193,232,251,1,25,255,72,131.0,196,32,255,252,243.0,
  15.0,126,156,253,36,233,255,76,139,140,253,36,233,255,252,243.0,15.0,126,
  148,253,36,233,255,76,139,132,253,36,233,255,252,243.0,15.0,126,140,253,36,
  233,255,72,139,148,253,36,233,255,252,243.0,15.0,126,4,36,255,72,139,12,36,
  255,232,251,1,26,72,131.0,252,236,48,255,72,137,68,36,32,232,251,1,0,72,185,
  237,237,137,1,73,184,237,237,72,199.0,194,237,76,137,225,232,251,1,7,72,139,
  76,36,32,72,137,8,252,233,244,10,255,72,137,68,36,32,232,251,1,0,72,185,237,
  237,137,1,73,184,237,237,72,199.0,194,0,0.0,0.0,0.0,76,137,225,232,251,1,
  7,72,139,76,36,32,72,137,8,252,233,244,10,255,102.0,15.0,214,68,36,32,232,
  251,1,0,72,185,237,237,137,1,73,184,237,237,72,199.0,194,0,0.0,0.0,0.0,76,
  137,225,232,251,1,7,72,139,76,36,32,72,137,8,252,233,244,10,255,102.0,15.0,
  214,76,36,40,102.0,15.0,214,68,36,32,232,251,1,0,72,185,237,237,137,1,73,
  184,237,237,72,199.0,194,0,0.0,0.0,0.0,76,137,225,232,251,1,7,72,139,76,36,
  40,72,137,72,8,72,139,76,36,32,72,137,8,252,233,244,10,255,252,233,244,11,
  255,252,233,244,13,255,252,233,244,14,255,252,233,244,15,255,252,243.0,15.0,
  90,192,252,233,244,12,255
};

static const char *const globnames[] = {
  "lua_return_arg",
  "lua_return_void",
  "lua_return_double",
  "lua_return_bool",
  "lua_return_int",
  "lua_return_uint",
  "too_few_arguments",
  "too_many_arguments",
  "save_registers",
  (const char *)0
};
static const char *const extnames[] = {
  "GetLastError",
  "lua_pushnumber",
  "lua_pushboolean",
  "push_int",
  "push_uint",
  "luaL_error",
  "lua_rawgeti",
  "push_cdata",
  "lua_remove",
  "lua_callk",
  "check_typed_pointer",
  "lua_settop",
  "check_enum",
  "check_uint32",
  "check_int32",
  "check_uint64",
  "check_int64",
  "check_uintptr",
  "check_double",
  "check_complex_float",
  "check_complex_double",
  "lua_gettop",
  "check_typed_cfunction",
  "unpack_varargs_stack",
  "unpack_varargs_reg",
  "SetLastError",
  "FUNCTION",
  (const char *)0
};



#if defined _WIN64 || defined __amd64__
#define JUMP_SIZE 14
#else
#define JUMP_SIZE 4
#endif

#define MIN_BRANCH INT32_MIN
#define MAX_BRANCH INT32_MAX
#define BRANCH_OFF 4

static void compile_extern_jump(struct jit* jit, lua_State* L, cfunction func, uint8_t* code)
{
    /* The jump code is the function pointer followed by a stub to call the
     * function pointer. The stub exists in 64 bit so we can jump to functions
     * with an offset greater than 2 GB.
     *
     * Note we have to manually set this up since there are commands buffered
     * in the jit state and dynasm doesn't support rip relative addressing.
     *
     * eg on 64 bit:
     * 0-8: function ptr
     * 8-14: jmp aword [rip-14]
     *
     * for 32 bit we only set the function ptr as it can always fit in a 32
     * bit displacement
     */
#if defined _WIN64 || defined __amd64__
    *(cfunction*) code = func;
    code[8] = 0xFF; /* FF /4 operand for jmp */
    code[9] = 0x25; /* RIP displacement */
    *(int32_t*) &code[10] = -14;
#else
    *(cfunction*) code = func;
#endif
}

void compile_globals(struct jit* jit, lua_State* L)
{
    struct jit* Dst = jit;
    int* perr = &jit->last_errno;
    dasm_setup(Dst, build_actionlist);

    /* Note: since the return code uses EBP to reset the stack pointer, we
     * don't have to track the amount of stack space used. It also means we
     * can handle stdcall and cdecl with the same code.
     */

    /* Note the various call_* functions want 32 bytes of 16 byte aligned
     * stack
     */




    /* the general idea for the return functions is:
     * 1) Save return value on stack
     * 2) Call get_errno (this trashes the registers hence #1)
     * 3) Unpack return value from stack
     * 4) Call lua push function
     * 5) Set eax to number of returned args (0 or 1)
     * 6) Call return which pops our stack frame
     */

    dasm_put(Dst, 0);

    dasm_put(Dst, 24, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 58, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 95, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 133, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 168, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 203, (unsigned int)((uintptr_t)(&"too few arguments")), (unsigned int)(((uintptr_t)(&"too few arguments"))>>32));

    dasm_put(Dst, 221, (unsigned int)((uintptr_t)(&"too many arguments")), (unsigned int)(((uintptr_t)(&"too many arguments"))>>32));

    dasm_put(Dst, 239);

    compile(Dst, L, NULL, LUA_NOREF);
}

int x86_return_size(lua_State* L, int usr, const struct ctype* ct)
{
    int ret = 0;
    const struct ctype* mt;

    if (ct->calling_convention != C_CALL) {
        size_t i;
        size_t argn = lua_rawlen(L, usr);
        for (i = 1; i <= argn; i++) {
            lua_rawgeti(L, usr, (int) i);
            mt = (const struct ctype*) lua_touserdata(L, -1);

            if (mt->pointers) {
                ret += sizeof(void*);
            } else {
                switch (mt->type) {
                case DOUBLE_TYPE:
                case COMPLEX_FLOAT_TYPE:
                case INT64_TYPE:
                    ret += 8;
                    break;
                case COMPLEX_DOUBLE_TYPE:
                    ret += 16;
                    break;
                case INTPTR_TYPE:
                    ret += sizeof(intptr_t);
                    break;
                case FUNCTION_PTR_TYPE:
                    ret += sizeof(cfunction);
                    break;
                case BOOL_TYPE:
                case FLOAT_TYPE:
                case INT8_TYPE:
                case INT16_TYPE:
                case INT32_TYPE:
                case ENUM_TYPE:
                    ret += 4;
                    break;
                default:
                    return luaL_error(L, "NYI - argument type");
                }
            }

            lua_pop(L, 1);
        }
    }

#if !defined _WIN64 && !defined __amd64__
    lua_rawgeti(L, usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
    if (!mt->pointers && mt->type == COMPLEX_DOUBLE_TYPE) {
        ret += sizeof(void*);
    }
    lua_pop(L, 1);
#endif

    return ret;
}

#ifdef _WIN64
#define MAX_REGISTERS(ct) 4 /* rcx, rdx, r8, r9 */

#elif defined __amd64__
#define MAX_INT_REGISTERS(ct) 6 /* rdi, rsi, rdx, rcx, r8, r9 */
#define MAX_FLOAT_REGISTERS(ct) 8 /* xmm0-7 */

#else
#define MAX_INT_REGISTERS(ct) ((ct)->calling_convention == FAST_CALL ? 2 /* ecx, edx */ : 0)
#define MAX_FLOAT_REGISTERS(ct) 0
#endif

struct reg_alloc {
#ifdef _WIN64
    int regs;
    int is_float[4];
    int is_int[4];
#else
    int floats;
    int ints;
#endif
    int off;
};

#ifdef _WIN64
#define REGISTER_STACK_SPACE(ct) (4*8)
#elif defined __amd64__
#define REGISTER_STACK_SPACE(ct) (14*8)
#else
#define REGISTER_STACK_SPACE(ct) LUAFFI_ALIGN_UP(((ct)->calling_convention == FAST_CALL ? 2*4 : 0), 15)
#endif

/* Fastcall:
 * Uses ecx, edx as first two int registers
 * Everything else on stack (include 64bit ints)
 * No overflow stack space
 * Pops the stack before returning
 * Returns int in eax, float in ST0
 * We use the same register allocation logic as posix x64 with 2 int regs and 0 float regs
 */

static void LUAFFI_get_int(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_int64)
{
    /* grab the register from the shadow space */
#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        dasm_put(Dst, 280, 16 + 8*reg->regs);
        reg->regs++;
    }
#elif __amd64__
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 280, - 80 - 8*reg->ints);
        reg->ints++;
    }
#else
    if (!is_int64 && reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 281, - 8 - 4*reg->ints);
        reg->ints++;
    }
#endif
    else if (is_int64) {
        dasm_put(Dst, 280, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 281, reg->off);
        reg->off += 4;
    }
}

static void add_int(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_int64)
{
#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        dasm_put(Dst, 285, 32 + 8*(reg->regs));
        reg->is_int[reg->regs++] = 1;
    }
#elif __amd64__
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 285, 32 + 8*reg->ints);
        reg->ints++;
    }
#else
    if (!is_int64 && reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 285, 32 + 4*reg->ints);
        reg->ints++;
    }
#endif

    else if (is_int64) {
        dasm_put(Dst, 285, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 286, reg->off);
        reg->off += 4;
    }
}

static void get_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
#if !defined _WIN64 && !defined __amd64__
    assert(MAX_FLOAT_REGISTERS(ct) == 0);
    if (is_double) {
        dasm_put(Dst, 292, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 296, reg->off);
        reg->off += 4;
    }
#else
    int off;

#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        off = -16 - 8*reg->regs;
        reg->regs++;
    }
#else
    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        off = -16 - 8*reg->floats;
        reg->floats++;
    }
#endif
    else {
        off = reg->off;
        reg->off += is_double ? 8 : 4;
    }

    if (is_double) {
        dasm_put(Dst, 300, off);
    } else {
        dasm_put(Dst, 307, off);
    }
#endif
}

static void add_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
#if !defined _WIN64 && !defined __amd64__
    assert(MAX_FLOAT_REGISTERS(ct) == 0);
    if (is_double) {
        dasm_put(Dst, 314, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 320, reg->off);
        reg->off += 4;
    }
#else

#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        if (is_double) {
            dasm_put(Dst, 326, 32 + 8*(reg->regs));
        } else {
            dasm_put(Dst, 334, 32 + 8*(reg->regs));
        }
        reg->is_float[reg->regs++] = 1;
    }
#else
    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        if (is_double) {
            dasm_put(Dst, 326, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        } else {
            dasm_put(Dst, 334, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        }
        reg->floats++;
    }
#endif

    else if (is_double) {
        dasm_put(Dst, 326, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 347, reg->off);
        reg->off += 4;
    }
#endif
}

#if defined _WIN64 || defined __amd64__
#define add_pointer(jit, ct, reg) add_int(jit, ct, reg, 1)
#define get_pointer(jit, ct, reg) LUAFFI_get_int(jit, ct, reg, 1)
#else
#define add_pointer(jit, ct, reg) add_int(jit, ct, reg, 0)
#define get_pointer(jit, ct, reg) LUAFFI_get_int(jit, ct, reg, 0)
#endif

cfunction compile_callback(lua_State* L, int fidx, int ct_usr, const struct ctype* ct)
{
    int i, nargs;
    cfunction* pf;
    struct ctype ct2 = *ct;
    const struct ctype* mt;
    struct reg_alloc reg;
    int num_upvals = 0;
    int top = lua_gettop(L);
    struct jit* Dst = get_jit(L);
    int ref;
    int hidden_arg_off = 0;

    ct_usr = lua_absindex(L, ct_usr);
    fidx = lua_absindex(L, fidx);

    assert(lua_isnil(L, fidx) || lua_isfunction(L, fidx));

    memset(&reg, 0, sizeof(reg));
#ifdef _WIN64
    reg.off = 16 + REGISTER_STACK_SPACE(ct); /* stack registers are above the shadow space */
#elif __amd64__
    reg.off = 16;
#else
    reg.off = 8;
#endif

    dasm_setup(Dst, build_actionlist);

    // add a table to store ctype and function upvalues
    // callback_set assumes the first value is the lua function
    nargs = (int) lua_rawlen(L, ct_usr);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    if (ct->has_var_arg) {
        luaL_error(L, "can't create callbacks with varargs");
    }

    // setup a stack frame to hold args for the call into lua_call

    dasm_put(Dst, 360, 8 + 16 + 32 + REGISTER_STACK_SPACE(ct));
    if (ct->calling_convention == FAST_CALL) {
    }

    // hardcode the lua_State* value into the assembly
    dasm_put(Dst, 375, (unsigned int)((uintptr_t)(L)), (unsigned int)(((uintptr_t)(L))>>32));

    /* get the upval table */
    dasm_put(Dst, 380, ref, LUA_REGISTRYINDEX);

    /* get the lua function */
    lua_pushvalue(L, fidx);
    lua_rawseti(L, -2, ++num_upvals);
    assert(num_upvals == CALLBACK_FUNC_USR_IDX);
    dasm_put(Dst, 396, num_upvals);

#if !defined _WIN64 && !defined __amd64__
    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
    if (!mt->pointers && mt->type == COMPLEX_DOUBLE_TYPE) {
        hidden_arg_off = reg.off;
        reg.off += sizeof(void*);
    }
    lua_pop(L, 1);
#else
    (void) hidden_arg_off;
#endif

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);

        if (mt->pointers) {
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */
            /* on the lua stack in the callback:
             * upval tbl, lua func, i-1 args
             */
            dasm_put(Dst, 419, num_upvals-1, -i-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
            get_pointer(Dst, ct, &reg);
            dasm_put(Dst, 457);
        } else {
            switch (mt->type) {
            case INT64_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                dasm_put(Dst, 479, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                LUAFFI_get_int(Dst, ct, &reg, 1);
                dasm_put(Dst, 498);
                break;

            case INTPTR_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                dasm_put(Dst, 479, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_pointer(Dst, ct, &reg);
                dasm_put(Dst, 498);
                break;

            case COMPLEX_FLOAT_TYPE:
                lua_pop(L, 1);
#if defined _WIN64 || defined __amd64__
                /* complex floats are two floats packed into a double */
                dasm_put(Dst, 479, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 502);
#else
                /* complex floats are real followed by imag on the stack */
                dasm_put(Dst, 479, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 507);
                get_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 510);
#endif
                break;

            case COMPLEX_DOUBLE_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 479, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                /* real */
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 502);
                /* imag */
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 514);
                break;

            case FLOAT_TYPE:
            case DOUBLE_TYPE:
                lua_pop(L, 1);
                get_float(Dst, ct, &reg, mt->type == DOUBLE_TYPE);
                dasm_put(Dst, 520);
                break;

            case BOOL_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                dasm_put(Dst, 533);
                break;

            case INT8_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 547);
                } else {
                    dasm_put(Dst, 551);
                }
                dasm_put(Dst, 555);
                break;

            case INT16_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 566);
                } else {
                    dasm_put(Dst, 570);
                }
                dasm_put(Dst, 555);
                break;

            case ENUM_TYPE:
            case INT32_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 574);
                } else {
                    dasm_put(Dst, 555);
                }
                break;

            default:
                luaL_error(L, "NYI: callback arg type");
            }
        }
    }

    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);

    dasm_put(Dst, 585, (unsigned int)((uintptr_t)(0)), (unsigned int)(((uintptr_t)(0))>>32), (mt->pointers || mt->type != VOID_TYPE) ? 1 : 0, nargs);

    // Unpack the return argument if not "void", also clean-up the lua stack
    // to remove the return argument and bind table. Use lua_settop rather
    // than lua_pop as lua_pop is implemented as a macro.
    if (mt->pointers) {
        lua_getuservalue(L, -1);
        lua_rawseti(L, -3, ++num_upvals); /* usr value */
        lua_rawseti(L, -2, ++num_upvals); /* mt */
        dasm_put(Dst, 605, num_upvals-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));

    } else {
        switch (mt->type) {
        case ENUM_TYPE:
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */
            dasm_put(Dst, 689, num_upvals-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 771);
            break;

        case BOOL_TYPE:
        case INT8_TYPE:
        case INT16_TYPE:
        case INT32_TYPE:
            lua_pop(L, 1);
            if (mt->is_unsigned) {
                dasm_put(Dst, 790);
            } else {
                dasm_put(Dst, 809);
            }
            dasm_put(Dst, 828);
            break;

        case INT64_TYPE:
            lua_pop(L, 1);

            if (mt->is_unsigned) {
                dasm_put(Dst, 855);
            } else {
                dasm_put(Dst, 874);
            }

            dasm_put(Dst, 893);
            break;

        case INTPTR_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 922);
            break;

        case FLOAT_TYPE:
        case DOUBLE_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 969);
            if (mt->type == FLOAT_TYPE) {
                dasm_put(Dst, 1012);
            } else {
                dasm_put(Dst, 1020);
            }
            break;

        case COMPLEX_FLOAT_TYPE:
            lua_pop(L, 1);
#if !defined HAVE_COMPLEX
            luaL_error(L, "ffi lib compiled without complex number support");
#endif
            /* on 64 bit complex floats are two floats packed into a double,
             * on 32 bit returned complex floats use eax and edx */
            dasm_put(Dst, 1028);
            break;

        case COMPLEX_DOUBLE_TYPE:
            lua_pop(L, 1);
#if !defined HAVE_COMPLEX
            luaL_error(L, "ffi lib compiled without complex number support");
#endif
            /* on 64 bit, returned complex doubles use xmm0, xmm1, on 32 bit
             * there is a hidden first parameter that points to 16 bytes where
             * the returned arg is stored which is popped by the called
             * function */
#if defined _WIN64 || defined __amd64__
            dasm_put(Dst, 1078);
#else
            dasm_put(Dst, 1141, hidden_arg_off);
#endif
            break;

        default:
            luaL_error(L, "NYI: callback return type");
        }
    }

    dasm_put(Dst, 1190, x86_return_size(L, ct_usr, ct));

    lua_pop(L, 1); /* upval table - already in registry */
    assert(lua_gettop(L) == top);

    ct2.is_jitted = 1;
    pf = (cfunction*) push_cdata(L, ct_usr, &ct2);
    *pf = compile(Dst, L, NULL, ref);

    assert(lua_gettop(L) == top + 1);

    return *pf;
}

void compile_function(lua_State* L, cfunction func, int ct_usr, const struct ctype* ct)
{
    size_t i, nargs;
    int num_upvals;
    const struct ctype* mbr_ct;
    struct jit* Dst = get_jit(L);
    struct reg_alloc reg;
    void* p;
    int top = lua_gettop(L);
    int* perr = &Dst->last_errno;

    ct_usr = lua_absindex(L, ct_usr);

    memset(&reg, 0, sizeof(reg));
    reg.off = 32 + REGISTER_STACK_SPACE(ct);

    dasm_setup(Dst, build_actionlist);

    p = push_cdata(L, ct_usr, ct);
    *(cfunction*) p = func;
    num_upvals = 1;

    nargs = lua_rawlen(L, ct_usr);

    if (ct->calling_convention != C_CALL && ct->has_var_arg) {
        luaL_error(L, "vararg is only allowed with the c calling convention");
    }

    dasm_put(Dst, 1203, nargs);

    if (!ct->has_var_arg) {
        dasm_put(Dst, 1239);
    }

    /* no need to zero extend eax returned by lua_gettop to rax as x86-64
     * preguarentees that the upper 32 bits will be zero */
    dasm_put(Dst, 1244, 32 + REGISTER_STACK_SPACE(ct));

#if !defined _WIN64 && !defined __amd64__
    /* Returned complex doubles require a hidden first parameter where the
     * data is stored, which is popped by the calling code. */
    lua_rawgeti(L, ct_usr, 0);
    mbr_ct = (const struct ctype*) lua_touserdata(L, -1);
    if (!mbr_ct->pointers && mbr_ct->type == COMPLEX_DOUBLE_TYPE) {
        /* we can allocate more space for arguments as long as no add_*
         * function has been called yet, mbr_ct will be added as an upvalue in
         * the return processing later */
        dasm_put(Dst, 1257, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32));
        add_pointer(Dst, ct, &reg);
    }
    lua_pop(L, 1);
#endif

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, (int) i);
        mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

        if (mbr_ct->pointers) {
            lua_getuservalue(L, -1);
            num_upvals += 2;
            dasm_put(Dst, 1281, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
            add_pointer(Dst, ct, &reg);
        } else {
            switch (mbr_ct->type) {
            case FUNCTION_PTR_TYPE:
                lua_getuservalue(L, -1);
                num_upvals += 2;
                dasm_put(Dst, 1301, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
                add_pointer(Dst, ct, &reg);
                break;

            case ENUM_TYPE:
                lua_getuservalue(L, -1);
                num_upvals += 2;
                dasm_put(Dst, 1321, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
                add_int(Dst, ct, &reg, 0);
                break;

            case INT8_TYPE:
                dasm_put(Dst, 1341, i);
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1353);
                } else {
                    dasm_put(Dst, 1357);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT16_TYPE:
                dasm_put(Dst, 1341, i);
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1361);
                } else {
                    dasm_put(Dst, 1365);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case BOOL_TYPE:
                dasm_put(Dst, 1369, i);
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT32_TYPE:
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1391, i);
                } else {
                    dasm_put(Dst, 1341, i);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INTPTR_TYPE:
                dasm_put(Dst, 1403, i);
                add_pointer(Dst, ct, &reg);
                lua_pop(L, 1);
                break;

            case INT64_TYPE:
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1415, i);
                } else {
                    dasm_put(Dst, 1427, i);
                }
                add_int(Dst, ct, &reg, 1);
                lua_pop(L, 1);
                break;

            case DOUBLE_TYPE:
                dasm_put(Dst, 1439, i);
                add_float(Dst, ct, &reg, 1);
                lua_pop(L, 1);
                break;

            case COMPLEX_DOUBLE_TYPE:
                /* on 64 bit, returned complex doubles use xmm0, xmm1, on 32 bit
                 * there is a hidden first parameter that points to 16 bytes where
                 * the returned arg is stored (this is popped by the called
                 * function) */
#if defined _WIN64 || defined __amd64__
                dasm_put(Dst, 1451, i);
                add_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 1463);
                add_float(Dst, ct, &reg, 1);
#else
                dasm_put(Dst, 1469, reg.off, i);
                reg.off += 16;
#endif
                lua_pop(L, 1);
                break;

            case FLOAT_TYPE:
                dasm_put(Dst, 1439, i);
                add_float(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case COMPLEX_FLOAT_TYPE:
#if defined _WIN64 || defined __amd64__
                dasm_put(Dst, 1495, i);
                /* complex floats are two floats packed into a double */
                add_float(Dst, ct, &reg, 1);
#else
                /* returned complex floats use eax and edx */
                dasm_put(Dst, 1507, i);
                add_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 1525);
                add_float(Dst, ct, &reg, 0);
#endif
                lua_pop(L, 1);
                break;

            default:
                luaL_error(L, "NYI: call arg type");
            }
        }
    }

    if (ct->has_var_arg) {
#ifdef _WIN64
        if (reg.regs < MAX_REGISTERS(ct)) {
            assert(reg.regs == nargs);
            dasm_put(Dst, 1532, MAX_REGISTERS(ct), 32 + 8*MAX_REGISTERS(ct), MAX_REGISTERS(ct)+1, 32 + 8*(reg.regs), MAX_REGISTERS(ct), nargs+1, 32 + 8*(reg.regs), nargs+1);
        } else {
            dasm_put(Dst, 1623, reg.off, nargs+1);
        }

        for (i = nargs; i < MAX_REGISTERS(ct); i++) {
            reg.is_int[i] = reg.is_float[i] = 1;
        }
        reg.regs = MAX_REGISTERS(ct);
#elif defined __amd64__
        if (reg.floats < MAX_FLOAT_REGISTERS(ct)) {
        }

        if (reg.ints < MAX_INT_REGISTERS(ct)) {
        }


        reg.floats = MAX_FLOAT_REGISTERS(ct);
        reg.ints = MAX_INT_REGISTERS(ct);
#else
#endif
    }

    dasm_put(Dst, 1648, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    /* remove the stack space to call local functions */
    dasm_put(Dst, 1662);

#ifdef _WIN64
    switch (reg.regs) {
    case 4:
        if (reg.is_float[3]) {
            dasm_put(Dst, 1667, 8*3);
        }
        if (reg.is_int[3]) {
            dasm_put(Dst, 1676, 8*3);
        }
    case 3:
        if (reg.is_float[2]) {
            dasm_put(Dst, 1683, 8*2);
        }
        if (reg.is_int[2]) {
            dasm_put(Dst, 1692, 8*2);
        }
    case 2:
        if (reg.is_float[1]) {
            dasm_put(Dst, 1699, 8*1);
        }
        if (reg.is_int[1]) {
            dasm_put(Dst, 1708, 8*1);
        }
    case 1:
        if (reg.is_float[0]) {
            dasm_put(Dst, 1715);
        }
        if (reg.is_int[0]) {
            dasm_put(Dst, 1722);
        }
    case 0:
        break;
    }

    /* don't remove the space for the registers as we need 32 bytes of register overflow space */
    assert(REGISTER_STACK_SPACE(ct) == 32);

#elif defined __amd64__
    switch (reg.floats) {
    case 8:
    case 7:
    case 6:
    case 5:
    case 4:
    case 3:
    case 2:
    case 1:
    case 0:
        break;
    }

    switch (reg.ints) {
    case 6:
    case 5:
    case 4:
    case 3:
    case 2:
    case 1:
    case 0:
        break;
    }

#else
    if (ct->calling_convention == FAST_CALL) {
        switch (reg.ints) {
        case 2:
        case 1:
        case 0:
            break;
        }

    }
#endif

#ifdef __amd64__
    if (ct->has_var_arg) {
        /* al stores an upper limit on the number of float register, note that
         * its allowed to be more than the actual number of float registers used as
         * long as its 0-8 */
    }
#endif

    dasm_put(Dst, 1727);

    /* note on windows X86 the stack may be only aligned to 4 (stdcall will
     * have popped a multiple of 4 bytes), but we don't need 16 byte alignment on
     * that platform
     */

    lua_rawgeti(L, ct_usr, 0);
    mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

    if (mbr_ct->pointers || mbr_ct->type == INTPTR_TYPE) {
        lua_getuservalue(L, -1);
        num_upvals += 2;
        dasm_put(Dst, 1737, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));

    } else {
        switch (mbr_ct->type) {
        case FUNCTION_PTR_TYPE:
            lua_getuservalue(L, -1);
            num_upvals += 2;
            dasm_put(Dst, 1737, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));
            break;

        case INT64_TYPE:
            num_upvals++;
            dasm_put(Dst, 1780, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32));
            break;

        case COMPLEX_FLOAT_TYPE:
            num_upvals++;
            dasm_put(Dst, 1826, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32));
            break;

        case COMPLEX_DOUBLE_TYPE:
            num_upvals++;
            dasm_put(Dst, 1873, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32));
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1935);
            break;

        case BOOL_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1940);
            break;

        case INT8_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 1353);
            } else {
                dasm_put(Dst, 1357);
            }
            dasm_put(Dst, 1945);
            break;

        case INT16_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 1361);
            } else {
                dasm_put(Dst, 1365);
            }
            dasm_put(Dst, 1945);
            break;

        case INT32_TYPE:
        case ENUM_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 1950);
            } else {
                dasm_put(Dst, 1945);
            }
            break;

        case FLOAT_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1955);
            break;

        case DOUBLE_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1960);
            break;

        default:
            luaL_error(L, "NYI: call return type");
        }
    }

    assert(lua_gettop(L) == top + num_upvals);
    {
        cfunction f = compile(Dst, L, func, LUA_NOREF);
        /* add a callback as an upval so that the jitted code gets cleaned up when
         * the function gets gc'd */
        push_callback(L, f);
        lua_pushcclosure(L, (lua_CFunction) f, num_upvals+1);
    }
}

#elif defined __amd64__ 
/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM x64 version 1.3.0
** DO NOT EDIT! The original file is in "call_x86.dasc".
*/

#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif

/* vim: ts=4 sw=4 sts=4 et tw=78
 * Copyright (c) 2011 James R. McKaskill. See license in ffi.h
 */

static const unsigned char build_actionlist[2022] = {
  248,10,184,1,0.0,0.0,0.0,76,139,109,252,240,76,139,101,252,248,72,137,252,
  236,93,195,255,248,11,232,251,1,0,72,185,237,237,137,1,184,0,0.0,0.0,0.0,
  76,139,109,252,240,76,139,101,252,248,72,137,252,236,93,195,255,248,12,102.0,
  15.0,214,68,36,32,232,251,1,0,72,185,237,237,137,1,252,243.0,15.0,126,68,
  36,32,76,137,231,232,251,1,1,252,233,244,10,255,248,13,15.0,182,192,137,68,
  36,32,232,251,1,0,72,185,237,237,137,1,139,68,36,32,72,137,198,76,137,231,
  232,251,1,2,252,233,244,10,255,248,14,137,68,36,32,232,251,1,0,72,185,237,
  237,137,1,139,68,36,32,72,137,198,76,137,231,232,251,1,3,252,233,244,10,255,
  248,15,137,68,36,32,232,251,1,0,72,185,237,237,137,1,139,68,36,32,72,137,
  198,76,137,231,232,251,1,4,252,233,244,10,255,248,16,102,184,0,0.0,72,190,
  237,237,76,137,231,232,251,1,5,255,248,17,102,184,0,0.0,72,190,237,237,76,
  137,231,232,251,1,5,255,248,18,102.0,15.0,214,69,252,240,102.0,15.0,214,77,
  232,102.0,15.0,214,85,224,102.0,15.0,214,93,216,102.0,15.0,214,101,208,102.0,
  15.0,214,109,200,102.0,15.0,214,117,192,102.0,15.0,214,125,184,72,137,125,
  176,72,137,117,168,72,137,85,160,72,137,77,152,76,137,69,144,76,137,77,136,
  195,255,72,139,141,233,255,72,137,132,253,36,233,255,221.0,133,233,255,217.0,
  133,233,255,252,243.0,15.0,126,133,233,255,252,243.0,15.0,90,133,233,255,
  221.0,156,253,36,233,255,217.0,156,253,36,233,255,102.0,15.0,214,132,253,
  36,233,255,252,242.0,15.0,90,192,102.0,15.0,214,132,253,36,233,255,252,242.0,
  15.0,90,192,102.0,15.0,126,132,253,36,233,255,85,72,137,229,65,84,72,129.0,
  252,236,239,232,244,18,255,73,188,237,237,255,72,199.0,194,237,72,199.0,198,
  237,76,137,231,232,251,1,6,255,72,199.0,194,237,72,199.0,198,252,255,252,
  255.0,252,255.0,252,255.0,76,137,231,232,251,1,6,255,72,199.0,194,237,72,
  199.0,198,237,76,137,231,232,251,1,6,72,186,237,237,72,199.0,198,252,255,
  252,255.0,252,255.0,252,255.0,76,137,231,232,251,1,7,255,72,137,8,72,199.0,
  198,252,254,252,255.0,252,255.0,252,255.0,76,137,231,232,251,1,8,255,72,186,
  237,237,72,199.0,198,0,0.0,0.0,0.0,76,137,231,232,251,1,7,255,72,137,8,255,
  102.0,15.0,214,0,255,217.0,24,255,217.0,88,4,255,102.0,15.0,214,64,8,255,
  76,137,231,232,251,1,1,255,15.0,182,201,72,137,206,76,137,231,232,251,1,2,
  255,15.0,182,201,255,15.0,190,201,255,72,137,206,76,137,231,232,251,1,3,255,
  15.0,183,201,255,15.0,191,201,255,72,137,206,76,137,231,232,251,1,4,255,72,
  185,237,237,72,199.0,194,237,72,199.0,198,237,76,137,231,232,251,1,9,255,
  72,199.0,194,237,72,199.0,198,252,254,252,255.0,252,255.0,252,255.0,76,137,
  231,232,251,1,6,72,185,237,237,72,199.0,194,252,255,252,255.0,252,255.0,252,
  255.0,72,199.0,198,252,254,252,255.0,252,255.0,252,255.0,76,137,231,232,251,
  1,10,72,137,68,36,32,72,199.0,198,252,252,252,255.0,252,255.0,252,255.0,76,
  137,231,232,251,1,11,72,139,68,36,32,255,72,199.0,194,237,72,199.0,198,252,
  254,252,255.0,252,255.0,252,255.0,76,137,231,232,251,1,6,72,185,237,237,72,
  199.0,194,252,255,252,255.0,252,255.0,252,255.0,72,199.0,198,252,254,252,
  255.0,252,255.0,252,255.0,76,137,231,232,251,1,12,137,68,36,32,72,199.0,198,
  252,252,252,255.0,252,255.0,252,255.0,76,137,231,232,251,1,11,139,68,36,32,
  255,72,199.0,198,252,254,252,255.0,252,255.0,252,255.0,76,137,231,232,251,
  1,11,255,72,199.0,198,252,255,252,255.0,252,255.0,252,255.0,76,137,231,232,
  251,1,13,255,72,199.0,198,252,255,252,255.0,252,255.0,252,255.0,76,137,231,
  232,251,1,14,255,137,68,36,32,72,199.0,198,252,253,252,255.0,252,255.0,252,
  255.0,76,137,231,232,251,1,11,139,68,36,32,255,72,199.0,198,252,255,252,255.0,
  252,255.0,252,255.0,76,137,231,232,251,1,15,255,72,199.0,198,252,255,252,
  255.0,252,255.0,252,255.0,76,137,231,232,251,1,16,255,72,137,68,36,32,72,
  199.0,198,252,253,252,255.0,252,255.0,252,255.0,76,137,231,232,251,1,11,72,
  139,68,36,32,255,72,199.0,198,252,255,252,255.0,252,255.0,252,255.0,76,137,
  231,232,251,1,17,72,137,68,36,32,72,199.0,198,252,253,252,255.0,252,255.0,
  252,255.0,76,137,231,232,251,1,11,72,139,68,36,32,255,72,199.0,198,252,255,
  252,255.0,252,255.0,252,255.0,76,137,231,232,251,1,18,102.0,15.0,214,68,36,
  32,72,199.0,198,252,253,252,255.0,252,255.0,252,255.0,76,137,231,232,251,
  1,11,255,252,242.0,15.0,90,68,36,32,255,252,243.0,15.0,126,68,36,32,255,72,
  199.0,198,252,255,252,255.0,252,255.0,252,255.0,76,137,231,232,251,1,19,102.0,
  15.0,214,68,36,32,72,199.0,198,252,253,252,255.0,252,255.0,252,255.0,76,137,
  231,232,251,1,11,252,243.0,15.0,126,68,36,32,255,72,199.0,198,252,255,252,
  255.0,252,255.0,252,255.0,76,137,231,232,251,1,20,102.0,15.0,214,68,36,32,
  102.0,15.0,214,76,36,40,72,199.0,198,252,253,252,255.0,252,255.0,252,255.0,
  76,137,231,232,251,1,11,252,243.0,15.0,126,68,36,32,252,243.0,15.0,126,76,
  36,40,255,72,139,141,233,72,199.0,194,252,255,252,255.0,252,255.0,252,255.0,
  76,137,230,72,137,207,232,251,1,20,72,131.0,252,236,4,72,199.0,198,252,253,
  252,255.0,252,255.0,252,255.0,76,137,231,232,251,1,11,255,76,139,101,252,
  248,72,137,252,236,93,194,236,255,85,72,137,229,65,84,65,85,73,137,252,252,
  76,137,231,232,251,1,21,73,137,197,72,129.0,252,248,239,15.0,140,244,16,255,
  15.0,143,244,17,255,72,193.0,224,4,72,41,196,72,129.0,252,236,239,255,72,
  186,237,237,72,199.0,198,0,0.0,0.0,0.0,76,137,231,232,251,1,7,72,131.0,252,
  236,16,255,72,185,237,237,72,199.0,194,237,72,199.0,198,237,76,137,231,232,
  251,1,10,255,72,185,237,237,72,199.0,194,237,72,199.0,198,237,76,137,231,
  232,251,1,22,255,72,185,237,237,72,199.0,194,237,72,199.0,198,237,76,137,
  231,232,251,1,12,255,72,199.0,198,237,76,137,231,232,251,1,14,255,15.0,182,
  192,255,15.0,190,192,255,15.0,183,192,255,15.0,191,192,255,72,199.0,198,237,
  76,137,231,232,251,1,14,131.0,252,248,0,15.0,149.0,208,15.0,182,192,255,72,
  199.0,198,237,76,137,231,232,251,1,13,255,72,199.0,198,237,76,137,231,232,
  251,1,17,255,72,199.0,198,237,76,137,231,232,251,1,15,255,72,199.0,198,237,
  76,137,231,232,251,1,16,255,72,199.0,198,237,76,137,231,232,251,1,18,255,
  72,199.0,198,237,76,137,231,232,251,1,20,255,252,243.0,15.0,126,193,255,72,
  141,132,253,36,233,72,131.0,252,236,4,72,199.0,194,237,76,137,230,72,137,
  199,232,251,1,20,255,72,199.0,198,237,76,137,231,232,251,1,19,255,72,199.0,
  198,237,76,137,231,232,251,1,19,137,4,36,217.0,4,36,255,137,20,36,217.0,4,
  36,255,72,137,224,72,129.0,192,239,73,137,192,72,199.0,193,237,76,137,252,
  234,72,199.0,198,237,76,137,231,232,251,1,23,255,72,137,224,72,129.0,192,
  239,73,137,192,72,199.0,193,237,76,137,252,234,72,199.0,198,237,76,137,231,
  232,251,1,24,255,72,137,224,72,129.0,192,239,73,137,193,73,199.0,192,237,
  72,199.0,193,237,76,137,252,234,72,199.0,198,237,76,137,231,232,251,1,25,
  255,72,185,237,237,139,1,72,137,199,232,251,1,26,255,72,131.0,196,32,255,
  252,243.0,15.0,126,188,253,36,233,255,252,243.0,15.0,126,180,253,36,233,255,
  252,243.0,15.0,126,172,253,36,233,255,252,243.0,15.0,126,164,253,36,233,255,
  252,243.0,15.0,126,156,253,36,233,255,252,243.0,15.0,126,148,253,36,233,255,
  252,243.0,15.0,126,140,253,36,233,255,252,243.0,15.0,126,132,253,36,233,255,
  76,139,140,253,36,233,255,76,139,132,253,36,233,255,72,139,140,253,36,233,
  255,72,139,148,253,36,233,255,72,139,180,253,36,233,255,72,139,60,36,255,
  72,129.0,196,239,255,176,8,255,232,251,1,27,72,131.0,252,236,48,255,72,137,
  68,36,32,232,251,1,0,72,185,237,237,137,1,72,186,237,237,72,199.0,198,237,
  76,137,231,232,251,1,7,72,139,76,36,32,72,137,8,252,233,244,10,255,72,137,
  68,36,32,232,251,1,0,72,185,237,237,137,1,72,186,237,237,72,199.0,198,0,0.0,
  0.0,0.0,76,137,231,232,251,1,7,72,139,76,36,32,72,137,8,252,233,244,10,255,
  102.0,15.0,214,68,36,32,232,251,1,0,72,185,237,237,137,1,72,186,237,237,72,
  199.0,198,0,0.0,0.0,0.0,76,137,231,232,251,1,7,72,139,76,36,32,72,137,8,252,
  233,244,10,255,102.0,15.0,214,76,36,40,102.0,15.0,214,68,36,32,232,251,1,
  0,72,185,237,237,137,1,72,186,237,237,72,199.0,198,0,0.0,0.0,0.0,76,137,231,
  232,251,1,7,72,139,76,36,40,72,137,72,8,72,139,76,36,32,72,137,8,252,233,
  244,10,255,252,233,244,11,255,252,233,244,13,255,252,233,244,14,255,252,233,
  244,15,255,252,243.0,15.0,90,192,252,233,244,12,255
};

static const char *const globnames[] = {
  "lua_return_arg",
  "lua_return_void",
  "lua_return_double",
  "lua_return_bool",
  "lua_return_int",
  "lua_return_uint",
  "too_few_arguments",
  "too_many_arguments",
  "save_registers",
  (const char *)0
};
static const char *const extnames[] = {
  "GetLastError",
  "lua_pushnumber",
  "lua_pushboolean",
  "push_int",
  "push_uint",
  "luaL_error",
  "lua_rawgeti",
  "push_cdata",
  "lua_remove",
  "lua_callk",
  "check_typed_pointer",
  "lua_settop",
  "check_enum",
  "check_uint32",
  "check_int32",
  "check_uint64",
  "check_int64",
  "check_uintptr",
  "check_double",
  "check_complex_float",
  "check_complex_double",
  "lua_gettop",
  "check_typed_cfunction",
  "unpack_varargs_float",
  "unpack_varargs_int",
  "unpack_varargs_stack_skip",
  "SetLastError",
  "FUNCTION",
  (const char *)0
};



#if defined _WIN64 || defined __amd64__
#define JUMP_SIZE 14
#else
#define JUMP_SIZE 4
#endif

#define MIN_BRANCH INT32_MIN
#define MAX_BRANCH INT32_MAX
#define BRANCH_OFF 4

static void compile_extern_jump(struct jit* jit, lua_State* L, cfunction func, uint8_t* code)
{
    /* The jump code is the function pointer followed by a stub to call the
     * function pointer. The stub exists in 64 bit so we can jump to functions
     * with an offset greater than 2 GB.
     *
     * Note we have to manually set this up since there are commands buffered
     * in the jit state and dynasm doesn't support rip relative addressing.
     *
     * eg on 64 bit:
     * 0-8: function ptr
     * 8-14: jmp aword [rip-14]
     *
     * for 32 bit we only set the function ptr as it can always fit in a 32
     * bit displacement
     */
#if defined _WIN64 || defined __amd64__
    *(cfunction*) code = func;
    code[8] = 0xFF; /* FF /4 operand for jmp */
    code[9] = 0x25; /* RIP displacement */
    *(int32_t*) &code[10] = -14;
#else
    *(cfunction*) code = func;
#endif
}

void compile_globals(struct jit* jit, lua_State* L)
{
    struct jit* Dst = jit;
    int* perr = &jit->last_errno;
    dasm_setup(Dst, build_actionlist);

    /* Note: since the return code uses EBP to reset the stack pointer, we
     * don't have to track the amount of stack space used. It also means we
     * can handle stdcall and cdecl with the same code.
     */

    /* Note the various call_* functions want 32 bytes of 16 byte aligned
     * stack
     */




    /* the general idea for the return functions is:
     * 1) Save return value on stack
     * 2) Call get_errno (this trashes the registers hence #1)
     * 3) Unpack return value from stack
     * 4) Call lua push function
     * 5) Set eax to number of returned args (0 or 1)
     * 6) Call return which pops our stack frame
     */

    dasm_put(Dst, 0);

    dasm_put(Dst, 24, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 58, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 95, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 133, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 168, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 203, (unsigned int)((uintptr_t)(&"too few arguments")), (unsigned int)(((uintptr_t)(&"too few arguments"))>>32));

    dasm_put(Dst, 221, (unsigned int)((uintptr_t)(&"too many arguments")), (unsigned int)(((uintptr_t)(&"too many arguments"))>>32));

    dasm_put(Dst, 239);

    compile(Dst, L, NULL, LUA_NOREF);
}

int x86_return_size(lua_State* L, int usr, const struct ctype* ct)
{
    int ret = 0;
    const struct ctype* mt;

    if (ct->calling_convention != C_CALL) {
        size_t i;
        size_t argn = lua_rawlen(L, usr);
        for (i = 1; i <= argn; i++) {
            lua_rawgeti(L, usr, (int) i);
            mt = (const struct ctype*) lua_touserdata(L, -1);

            if (mt->pointers) {
                ret += sizeof(void*);
            } else {
                switch (mt->type) {
                case DOUBLE_TYPE:
                case COMPLEX_FLOAT_TYPE:
                case INT64_TYPE:
                    ret += 8;
                    break;
                case COMPLEX_DOUBLE_TYPE:
                    ret += 16;
                    break;
                case INTPTR_TYPE:
                    ret += sizeof(intptr_t);
                    break;
                case FUNCTION_PTR_TYPE:
                    ret += sizeof(cfunction);
                    break;
                case BOOL_TYPE:
                case FLOAT_TYPE:
                case INT8_TYPE:
                case INT16_TYPE:
                case INT32_TYPE:
                case ENUM_TYPE:
                    ret += 4;
                    break;
                default:
                    return luaL_error(L, "NYI - argument type");
                }
            }

            lua_pop(L, 1);
        }
    }

#if !defined _WIN64 && !defined __amd64__
    lua_rawgeti(L, usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
    if (!mt->pointers && mt->type == COMPLEX_DOUBLE_TYPE) {
        ret += sizeof(void*);
    }
    lua_pop(L, 1);
#endif

    return ret;
}

#ifdef _WIN64
#define MAX_REGISTERS(ct) 4 /* rcx, rdx, r8, r9 */

#elif defined __amd64__
#define MAX_INT_REGISTERS(ct) 6 /* rdi, rsi, rdx, rcx, r8, r9 */
#define MAX_FLOAT_REGISTERS(ct) 8 /* xmm0-7 */

#else
#define MAX_INT_REGISTERS(ct) ((ct)->calling_convention == FAST_CALL ? 2 /* ecx, edx */ : 0)
#define MAX_FLOAT_REGISTERS(ct) 0
#endif

struct reg_alloc {
#ifdef _WIN64
    int regs;
    int is_float[4];
    int is_int[4];
#else
    int floats;
    int ints;
#endif
    int off;
};

#ifdef _WIN64
#define REGISTER_STACK_SPACE(ct) (4*8)
#elif defined __amd64__
#define REGISTER_STACK_SPACE(ct) (14*8)
#else
#define REGISTER_STACK_SPACE(ct) LUAFFI_ALIGN_UP(((ct)->calling_convention == FAST_CALL ? 2*4 : 0), 15)
#endif

/* Fastcall:
 * Uses ecx, edx as first two int registers
 * Everything else on stack (include 64bit ints)
 * No overflow stack space
 * Pops the stack before returning
 * Returns int in eax, float in ST0
 * We use the same register allocation logic as posix x64 with 2 int regs and 0 float regs
 */

static void LUAFFI_get_int(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_int64)
{
    /* grab the register from the shadow space */
#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        dasm_put(Dst, 308, 16 + 8*reg->regs);
        reg->regs++;
    }
#elif __amd64__
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 308, - 80 - 8*reg->ints);
        reg->ints++;
    }
#else
    if (!is_int64 && reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 309, - 8 - 4*reg->ints);
        reg->ints++;
    }
#endif
    else if (is_int64) {
        dasm_put(Dst, 308, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 309, reg->off);
        reg->off += 4;
    }
}

static void add_int(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_int64)
{
#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        dasm_put(Dst, 313, 32 + 8*(reg->regs));
        reg->is_int[reg->regs++] = 1;
    }
#elif __amd64__
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 313, 32 + 8*reg->ints);
        reg->ints++;
    }
#else
    if (!is_int64 && reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 313, 32 + 4*reg->ints);
        reg->ints++;
    }
#endif

    else if (is_int64) {
        dasm_put(Dst, 313, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 314, reg->off);
        reg->off += 4;
    }
}

static void get_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
#if !defined _WIN64 && !defined __amd64__
    assert(MAX_FLOAT_REGISTERS(ct) == 0);
    if (is_double) {
        dasm_put(Dst, 320, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 324, reg->off);
        reg->off += 4;
    }
#else
    int off;

#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        off = -16 - 8*reg->regs;
        reg->regs++;
    }
#else
    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        off = -16 - 8*reg->floats;
        reg->floats++;
    }
#endif
    else {
        off = reg->off;
        reg->off += is_double ? 8 : 4;
    }

    if (is_double) {
        dasm_put(Dst, 328, off);
    } else {
        dasm_put(Dst, 335, off);
    }
#endif
}

static void add_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
#if !defined _WIN64 && !defined __amd64__
    assert(MAX_FLOAT_REGISTERS(ct) == 0);
    if (is_double) {
        dasm_put(Dst, 342, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 348, reg->off);
        reg->off += 4;
    }
#else

#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        if (is_double) {
            dasm_put(Dst, 354, 32 + 8*(reg->regs));
        } else {
            dasm_put(Dst, 362, 32 + 8*(reg->regs));
        }
        reg->is_float[reg->regs++] = 1;
    }
#else
    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        if (is_double) {
            dasm_put(Dst, 354, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        } else {
            dasm_put(Dst, 362, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        }
        reg->floats++;
    }
#endif

    else if (is_double) {
        dasm_put(Dst, 354, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 375, reg->off);
        reg->off += 4;
    }
#endif
}

#if defined _WIN64 || defined __amd64__
#define add_pointer(jit, ct, reg) add_int(jit, ct, reg, 1)
#define get_pointer(jit, ct, reg) LUAFFI_get_int(jit, ct, reg, 1)
#else
#define add_pointer(jit, ct, reg) add_int(jit, ct, reg, 0)
#define get_pointer(jit, ct, reg) LUAFFI_get_int(jit, ct, reg, 0)
#endif

cfunction compile_callback(lua_State* L, int fidx, int ct_usr, const struct ctype* ct)
{
    int i, nargs;
    cfunction* pf;
    struct ctype ct2 = *ct;
    const struct ctype* mt;
    struct reg_alloc reg;
    int num_upvals = 0;
    int top = lua_gettop(L);
    struct jit* Dst = get_jit(L);
    int ref;
    int hidden_arg_off = 0;

    ct_usr = lua_absindex(L, ct_usr);
    fidx = lua_absindex(L, fidx);

    assert(lua_isnil(L, fidx) || lua_isfunction(L, fidx));

    memset(&reg, 0, sizeof(reg));
#ifdef _WIN64
    reg.off = 16 + REGISTER_STACK_SPACE(ct); /* stack registers are above the shadow space */
#elif __amd64__
    reg.off = 16;
#else
    reg.off = 8;
#endif

    dasm_setup(Dst, build_actionlist);

    // add a table to store ctype and function upvalues
    // callback_set assumes the first value is the lua function
    nargs = (int) lua_rawlen(L, ct_usr);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    if (ct->has_var_arg) {
        luaL_error(L, "can't create callbacks with varargs");
    }

    // setup a stack frame to hold args for the call into lua_call

    dasm_put(Dst, 388, 8 + 16 + 32 + REGISTER_STACK_SPACE(ct));
    if (ct->calling_convention == FAST_CALL) {
    }

    // hardcode the lua_State* value into the assembly
    dasm_put(Dst, 403, (unsigned int)((uintptr_t)(L)), (unsigned int)(((uintptr_t)(L))>>32));

    /* get the upval table */
    dasm_put(Dst, 408, ref, LUA_REGISTRYINDEX);

    /* get the lua function */
    lua_pushvalue(L, fidx);
    lua_rawseti(L, -2, ++num_upvals);
    assert(num_upvals == CALLBACK_FUNC_USR_IDX);
    dasm_put(Dst, 424, num_upvals);

#if !defined _WIN64 && !defined __amd64__
    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
    if (!mt->pointers && mt->type == COMPLEX_DOUBLE_TYPE) {
        hidden_arg_off = reg.off;
        reg.off += sizeof(void*);
    }
    lua_pop(L, 1);
#else
    (void) hidden_arg_off;
#endif

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);

        if (mt->pointers) {
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */
            /* on the lua stack in the callback:
             * upval tbl, lua func, i-1 args
             */
            dasm_put(Dst, 447, num_upvals-1, -i-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
            get_pointer(Dst, ct, &reg);
            dasm_put(Dst, 485);
        } else {
            switch (mt->type) {
            case INT64_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                dasm_put(Dst, 507, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                LUAFFI_get_int(Dst, ct, &reg, 1);
                dasm_put(Dst, 526);
                break;

            case INTPTR_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                dasm_put(Dst, 507, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_pointer(Dst, ct, &reg);
                dasm_put(Dst, 526);
                break;

            case COMPLEX_FLOAT_TYPE:
                lua_pop(L, 1);
#if defined _WIN64 || defined __amd64__
                /* complex floats are two floats packed into a double */
                dasm_put(Dst, 507, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 530);
#else
                /* complex floats are real followed by imag on the stack */
                dasm_put(Dst, 507, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 535);
                get_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 538);
#endif
                break;

            case COMPLEX_DOUBLE_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 507, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                /* real */
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 530);
                /* imag */
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 542);
                break;

            case FLOAT_TYPE:
            case DOUBLE_TYPE:
                lua_pop(L, 1);
                get_float(Dst, ct, &reg, mt->type == DOUBLE_TYPE);
                dasm_put(Dst, 548);
                break;

            case BOOL_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                dasm_put(Dst, 556);
                break;

            case INT8_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 570);
                } else {
                    dasm_put(Dst, 574);
                }
                dasm_put(Dst, 578);
                break;

            case INT16_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 589);
                } else {
                    dasm_put(Dst, 593);
                }
                dasm_put(Dst, 578);
                break;

            case ENUM_TYPE:
            case INT32_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 597);
                } else {
                    dasm_put(Dst, 578);
                }
                break;

            default:
                luaL_error(L, "NYI: callback arg type");
            }
        }
    }

    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);

    dasm_put(Dst, 608, (unsigned int)((uintptr_t)(0)), (unsigned int)(((uintptr_t)(0))>>32), (mt->pointers || mt->type != VOID_TYPE) ? 1 : 0, nargs);

    // Unpack the return argument if not "void", also clean-up the lua stack
    // to remove the return argument and bind table. Use lua_settop rather
    // than lua_pop as lua_pop is implemented as a macro.
    if (mt->pointers) {
        lua_getuservalue(L, -1);
        lua_rawseti(L, -3, ++num_upvals); /* usr value */
        lua_rawseti(L, -2, ++num_upvals); /* mt */
        dasm_put(Dst, 628, num_upvals-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));

    } else {
        switch (mt->type) {
        case ENUM_TYPE:
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */
            dasm_put(Dst, 712, num_upvals-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 794);
            break;

        case BOOL_TYPE:
        case INT8_TYPE:
        case INT16_TYPE:
        case INT32_TYPE:
            lua_pop(L, 1);
            if (mt->is_unsigned) {
                dasm_put(Dst, 813);
            } else {
                dasm_put(Dst, 832);
            }
            dasm_put(Dst, 851);
            break;

        case INT64_TYPE:
            lua_pop(L, 1);

            if (mt->is_unsigned) {
                dasm_put(Dst, 878);
            } else {
                dasm_put(Dst, 897);
            }

            dasm_put(Dst, 916);
            break;

        case INTPTR_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 945);
            break;

        case FLOAT_TYPE:
        case DOUBLE_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 992);
            if (mt->type == FLOAT_TYPE) {
                dasm_put(Dst, 1035);
            } else {
                dasm_put(Dst, 1043);
            }
            break;

        case COMPLEX_FLOAT_TYPE:
            lua_pop(L, 1);
#if !defined HAVE_COMPLEX
            luaL_error(L, "ffi lib compiled without complex number support");
#endif
            /* on 64 bit complex floats are two floats packed into a double,
             * on 32 bit returned complex floats use eax and edx */
            dasm_put(Dst, 1051);
            break;

        case COMPLEX_DOUBLE_TYPE:
            lua_pop(L, 1);
#if !defined HAVE_COMPLEX
            luaL_error(L, "ffi lib compiled without complex number support");
#endif
            /* on 64 bit, returned complex doubles use xmm0, xmm1, on 32 bit
             * there is a hidden first parameter that points to 16 bytes where
             * the returned arg is stored which is popped by the called
             * function */
#if defined _WIN64 || defined __amd64__
            dasm_put(Dst, 1101);
#else
            dasm_put(Dst, 1164, hidden_arg_off);
#endif
            break;

        default:
            luaL_error(L, "NYI: callback return type");
        }
    }

    dasm_put(Dst, 1213, x86_return_size(L, ct_usr, ct));

    lua_pop(L, 1); /* upval table - already in registry */
    assert(lua_gettop(L) == top);

    ct2.is_jitted = 1;
    pf = (cfunction*) push_cdata(L, ct_usr, &ct2);
    *pf = compile(Dst, L, NULL, ref);

    assert(lua_gettop(L) == top + 1);

    return *pf;
}

void compile_function(lua_State* L, cfunction func, int ct_usr, const struct ctype* ct)
{
    size_t i, nargs;
    int num_upvals;
    const struct ctype* mbr_ct;
    struct jit* Dst = get_jit(L);
    struct reg_alloc reg;
    void* p;
    int top = lua_gettop(L);
    int* perr = &Dst->last_errno;

    ct_usr = lua_absindex(L, ct_usr);

    memset(&reg, 0, sizeof(reg));
    reg.off = 32 + REGISTER_STACK_SPACE(ct);

    dasm_setup(Dst, build_actionlist);

    p = push_cdata(L, ct_usr, ct);
    *(cfunction*) p = func;
    num_upvals = 1;

    nargs = lua_rawlen(L, ct_usr);

    if (ct->calling_convention != C_CALL && ct->has_var_arg) {
        luaL_error(L, "vararg is only allowed with the c calling convention");
    }

    dasm_put(Dst, 1226, nargs);

    if (!ct->has_var_arg) {
        dasm_put(Dst, 1258);
    }

    /* no need to zero extend eax returned by lua_gettop to rax as x86-64
     * preguarentees that the upper 32 bits will be zero */
    dasm_put(Dst, 1263, 32 + REGISTER_STACK_SPACE(ct));

#if !defined _WIN64 && !defined __amd64__
    /* Returned complex doubles require a hidden first parameter where the
     * data is stored, which is popped by the calling code. */
    lua_rawgeti(L, ct_usr, 0);
    mbr_ct = (const struct ctype*) lua_touserdata(L, -1);
    if (!mbr_ct->pointers && mbr_ct->type == COMPLEX_DOUBLE_TYPE) {
        /* we can allocate more space for arguments as long as no add_*
         * function has been called yet, mbr_ct will be added as an upvalue in
         * the return processing later */
        dasm_put(Dst, 1276, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32));
        add_pointer(Dst, ct, &reg);
    }
    lua_pop(L, 1);
#endif

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, (int) i);
        mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

        if (mbr_ct->pointers) {
            lua_getuservalue(L, -1);
            num_upvals += 2;
            dasm_put(Dst, 1300, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
            add_pointer(Dst, ct, &reg);
        } else {
            switch (mbr_ct->type) {
            case FUNCTION_PTR_TYPE:
                lua_getuservalue(L, -1);
                num_upvals += 2;
                dasm_put(Dst, 1320, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
                add_pointer(Dst, ct, &reg);
                break;

            case ENUM_TYPE:
                lua_getuservalue(L, -1);
                num_upvals += 2;
                dasm_put(Dst, 1340, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
                add_int(Dst, ct, &reg, 0);
                break;

            case INT8_TYPE:
                dasm_put(Dst, 1360, i);
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1372);
                } else {
                    dasm_put(Dst, 1376);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT16_TYPE:
                dasm_put(Dst, 1360, i);
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1380);
                } else {
                    dasm_put(Dst, 1384);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case BOOL_TYPE:
                dasm_put(Dst, 1388, i);
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT32_TYPE:
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1410, i);
                } else {
                    dasm_put(Dst, 1360, i);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INTPTR_TYPE:
                dasm_put(Dst, 1422, i);
                add_pointer(Dst, ct, &reg);
                lua_pop(L, 1);
                break;

            case INT64_TYPE:
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1434, i);
                } else {
                    dasm_put(Dst, 1446, i);
                }
                add_int(Dst, ct, &reg, 1);
                lua_pop(L, 1);
                break;

            case DOUBLE_TYPE:
                dasm_put(Dst, 1458, i);
                add_float(Dst, ct, &reg, 1);
                lua_pop(L, 1);
                break;

            case COMPLEX_DOUBLE_TYPE:
                /* on 64 bit, returned complex doubles use xmm0, xmm1, on 32 bit
                 * there is a hidden first parameter that points to 16 bytes where
                 * the returned arg is stored (this is popped by the called
                 * function) */
#if defined _WIN64 || defined __amd64__
                dasm_put(Dst, 1470, i);
                add_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 1482);
                add_float(Dst, ct, &reg, 1);
#else
                dasm_put(Dst, 1488, reg.off, i);
                reg.off += 16;
#endif
                lua_pop(L, 1);
                break;

            case FLOAT_TYPE:
                dasm_put(Dst, 1458, i);
                add_float(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case COMPLEX_FLOAT_TYPE:
#if defined _WIN64 || defined __amd64__
                dasm_put(Dst, 1514, i);
                /* complex floats are two floats packed into a double */
                add_float(Dst, ct, &reg, 1);
#else
                /* returned complex floats use eax and edx */
                dasm_put(Dst, 1526, i);
                add_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 1544);
                add_float(Dst, ct, &reg, 0);
#endif
                lua_pop(L, 1);
                break;

            default:
                luaL_error(L, "NYI: call arg type");
            }
        }
    }

    if (ct->has_var_arg) {
#ifdef _WIN64
        if (reg.regs < MAX_REGISTERS(ct)) {
            assert(reg.regs == nargs);
        } else {
        }

        for (i = nargs; i < MAX_REGISTERS(ct); i++) {
            reg.is_int[i] = reg.is_float[i] = 1;
        }
        reg.regs = MAX_REGISTERS(ct);
#elif defined __amd64__
        if (reg.floats < MAX_FLOAT_REGISTERS(ct)) {
            dasm_put(Dst, 1551, 32 + 8*(MAX_INT_REGISTERS(ct) + reg.floats), MAX_FLOAT_REGISTERS(ct) - reg.floats, nargs+1);
        }

        if (reg.ints < MAX_INT_REGISTERS(ct)) {
            dasm_put(Dst, 1581, 32 + 8*(reg.ints), MAX_INT_REGISTERS(ct) - reg.ints, nargs+1);
        }

        dasm_put(Dst, 1611, reg.off, MAX_FLOAT_REGISTERS(ct) - reg.floats, MAX_INT_REGISTERS(ct) - reg.ints, nargs+1);

        reg.floats = MAX_FLOAT_REGISTERS(ct);
        reg.ints = MAX_INT_REGISTERS(ct);
#else
#endif
    }

    dasm_put(Dst, 1645, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    /* remove the stack space to call local functions */
    dasm_put(Dst, 1659);

#ifdef _WIN64
    switch (reg.regs) {
    case 4:
        if (reg.is_float[3]) {
        }
        if (reg.is_int[3]) {
        }
    case 3:
        if (reg.is_float[2]) {
        }
        if (reg.is_int[2]) {
        }
    case 2:
        if (reg.is_float[1]) {
        }
        if (reg.is_int[1]) {
        }
    case 1:
        if (reg.is_float[0]) {
        }
        if (reg.is_int[0]) {
        }
    case 0:
        break;
    }

    /* don't remove the space for the registers as we need 32 bytes of register overflow space */
    assert(REGISTER_STACK_SPACE(ct) == 32);

#elif defined __amd64__
    switch (reg.floats) {
    case 8:
        dasm_put(Dst, 1664, 8*(MAX_INT_REGISTERS(ct)+7));
    case 7:
        dasm_put(Dst, 1673, 8*(MAX_INT_REGISTERS(ct)+6));
    case 6:
        dasm_put(Dst, 1682, 8*(MAX_INT_REGISTERS(ct)+5));
    case 5:
        dasm_put(Dst, 1691, 8*(MAX_INT_REGISTERS(ct)+4));
    case 4:
        dasm_put(Dst, 1700, 8*(MAX_INT_REGISTERS(ct)+3));
    case 3:
        dasm_put(Dst, 1709, 8*(MAX_INT_REGISTERS(ct)+2));
    case 2:
        dasm_put(Dst, 1718, 8*(MAX_INT_REGISTERS(ct)+1));
    case 1:
        dasm_put(Dst, 1727, 8*(MAX_INT_REGISTERS(ct)));
    case 0:
        break;
    }

    switch (reg.ints) {
    case 6:
        dasm_put(Dst, 1736, 8*5);
    case 5:
        dasm_put(Dst, 1743, 8*4);
    case 4:
        dasm_put(Dst, 1750, 8*3);
    case 3:
        dasm_put(Dst, 1757, 8*2);
    case 2:
        dasm_put(Dst, 1764, 8*1);
    case 1:
        dasm_put(Dst, 1771);
    case 0:
        break;
    }

    dasm_put(Dst, 1776, REGISTER_STACK_SPACE(ct));
#else
    if (ct->calling_convention == FAST_CALL) {
        switch (reg.ints) {
        case 2:
        case 1:
        case 0:
            break;
        }

    }
#endif

#ifdef __amd64__
    if (ct->has_var_arg) {
        /* al stores an upper limit on the number of float register, note that
         * its allowed to be more than the actual number of float registers used as
         * long as its 0-8 */
        dasm_put(Dst, 1781);
    }
#endif

    dasm_put(Dst, 1784);

    /* note on windows X86 the stack may be only aligned to 4 (stdcall will
     * have popped a multiple of 4 bytes), but we don't need 16 byte alignment on
     * that platform
     */

    lua_rawgeti(L, ct_usr, 0);
    mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

    if (mbr_ct->pointers || mbr_ct->type == INTPTR_TYPE) {
        lua_getuservalue(L, -1);
        num_upvals += 2;
        dasm_put(Dst, 1794, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));

    } else {
        switch (mbr_ct->type) {
        case FUNCTION_PTR_TYPE:
            lua_getuservalue(L, -1);
            num_upvals += 2;
            dasm_put(Dst, 1794, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));
            break;

        case INT64_TYPE:
            num_upvals++;
            dasm_put(Dst, 1837, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32));
            break;

        case COMPLEX_FLOAT_TYPE:
            num_upvals++;
            dasm_put(Dst, 1883, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32));
            break;

        case COMPLEX_DOUBLE_TYPE:
            num_upvals++;
            dasm_put(Dst, 1930, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32));
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1992);
            break;

        case BOOL_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1997);
            break;

        case INT8_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 1372);
            } else {
                dasm_put(Dst, 1376);
            }
            dasm_put(Dst, 2002);
            break;

        case INT16_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 1380);
            } else {
                dasm_put(Dst, 1384);
            }
            dasm_put(Dst, 2002);
            break;

        case INT32_TYPE:
        case ENUM_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 2007);
            } else {
                dasm_put(Dst, 2002);
            }
            break;

        case FLOAT_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 2012);
            break;

        case DOUBLE_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 2017);
            break;

        default:
            luaL_error(L, "NYI: call return type");
        }
    }

    assert(lua_gettop(L) == top + num_upvals);
    {
        cfunction f = compile(Dst, L, func, LUA_NOREF);
        /* add a callback as an upval so that the jitted code gets cleaned up when
         * the function gets gc'd */
        push_callback(L, f);
        lua_pushcclosure(L, (lua_CFunction) f, num_upvals+1);
    }
}

#else 
/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM x86 version 1.3.0
** DO NOT EDIT! The original file is in "call_x86.dasc".
*/

#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif

/* vim: ts=4 sw=4 sts=4 et tw=78
 * Copyright (c) 2011 James R. McKaskill. See license in ffi.h
 */

static const unsigned char build_actionlist[1803] = {
  248,10,184,1,0.0,0.0,0.0,139,117,252,248,139,125,252,252,137,252,236,93,195,
  255,248,11,232,251,1,0,185,237,137,1,184,0,0.0,0.0,0.0,139,117,252,248,139,
  125,252,252,137,252,236,93,195,255,248,12,221.0,92,36,4,232,251,1,0,185,237,
  137,1,137,60,36,232,251,1,1,252,233,244,10,255,248,13,15.0,182,192,137,68,
  36,32,232,251,1,0,185,237,137,1,139,68,36,32,137,68,36,4,137,60,36,232,251,
  1,2,252,233,244,10,255,248,14,137,68,36,32,232,251,1,0,185,237,137,1,139,
  68,36,32,137,68,36,4,137,60,36,232,251,1,3,252,233,244,10,255,248,15,137,
  68,36,32,232,251,1,0,185,237,137,1,139,68,36,32,137,68,36,4,137,60,36,232,
  251,1,4,252,233,244,10,255,248,16,102,184,0,0.0,199.0,68,36,4,237,137,60,
  36,232,251,1,5,255,248,17,102,184,0,0.0,199.0,68,36,4,237,137,60,36,232,251,
  1,5,255,248,18,137,77,252,248,137,85,252,244,195,255,139,141,233,255,139,
  141,233,139,149,233,255,137,132,253,36,233,255,137,132,253,36,233,137,148,
  253,36,233,255,221.0,133,233,255,217.0,133,233,255,252,243.0,15.0,126,133,
  233,255,252,243.0,15.0,90,133,233,255,221.0,156,253,36,233,255,217.0,156,
  253,36,233,255,102.0,15.0,214,132,253,36,233,255,252,242.0,15.0,90,192,102.0,
  15.0,214,132,253,36,233,255,252,242.0,15.0,90,192,102.0,15.0,126,132,253,
  36,233,255,85,137,229,87,129.0,252,236,239,255,232,244,18,255,191,237,255,
  199.0,68,36,8,237,199.0,68,36,4,237,137,60,36,232,251,1,6,255,199.0,68,36,
  8,237,199.0,68,36,4,252,255,252,255.0,252,255.0,252,255.0,137,60,36,232,251,
  1,6,255,199.0,68,36,8,237,199.0,68,36,4,237,137,60,36,232,251,1,6,199.0,68,
  36,8,237,199.0,68,36,4,252,255,252,255.0,252,255.0,252,255.0,137,60,36,232,
  251,1,7,255,137,8,199.0,68,36,4,252,254,252,255.0,252,255.0,252,255.0,137,
  60,36,232,251,1,8,255,199.0,68,36,8,237,199.0,68,36,4,0,0.0,0.0,0.0,137,60,
  36,232,251,1,7,255,137,8,137,80,4,255,137,8,255,102.0,15.0,214,0,255,217.0,
  24,255,217.0,88,4,255,221.0,24,255,221.0,88,8,255,221.0,92,36,4,137,60,36,
  232,251,1,1,255,15.0,182,201,137,76,36,4,137,60,36,232,251,1,2,255,15.0,182,
  201,255,15.0,190,201,255,137,76,36,4,137,60,36,232,251,1,3,255,15.0,183,201,
  255,15.0,191,201,255,137,76,36,4,137,60,36,232,251,1,4,255,199.0,68,36,12,
  0,0.0,0.0,0.0,199.0,68,36,8,237,199.0,68,36,4,237,137,60,36,232,251,1,9,255,
  199.0,68,36,8,237,199.0,68,36,4,252,254,252,255.0,252,255.0,252,255.0,137,
  60,36,232,251,1,6,199.0,68,36,12,237,199.0,68,36,8,252,255,252,255.0,252,
  255.0,252,255.0,199.0,68,36,4,252,254,252,255.0,252,255.0,252,255.0,137,60,
  36,232,251,1,10,137,68,36,32,199.0,68,36,4,252,252,252,255.0,252,255.0,252,
  255.0,137,60,36,232,251,1,11,139,68,36,32,255,199.0,68,36,8,237,199.0,68,
  36,4,252,254,252,255.0,252,255.0,252,255.0,137,60,36,232,251,1,6,199.0,68,
  36,12,237,199.0,68,36,8,252,255,252,255.0,252,255.0,252,255.0,199.0,68,36,
  4,252,254,252,255.0,252,255.0,252,255.0,137,60,36,232,251,1,12,137,68,36,
  32,199.0,68,36,4,252,252,252,255.0,252,255.0,252,255.0,137,60,36,232,251,
  1,11,139,68,36,32,255,199.0,68,36,4,252,254,252,255.0,252,255.0,252,255.0,
  137,60,36,232,251,1,11,255,199.0,68,36,4,252,255,252,255.0,252,255.0,252,
  255.0,137,60,36,232,251,1,13,255,199.0,68,36,4,252,255,252,255.0,252,255.0,
  252,255.0,137,60,36,232,251,1,14,255,137,68,36,32,199.0,68,36,4,252,253,252,
  255.0,252,255.0,252,255.0,137,60,36,232,251,1,11,139,68,36,32,255,199.0,68,
  36,4,252,255,252,255.0,252,255.0,252,255.0,137,60,36,232,251,1,15,255,199.0,
  68,36,4,252,255,252,255.0,252,255.0,252,255.0,137,60,36,232,251,1,16,255,
  137,68,36,32,137,84,36,36,199.0,68,36,4,252,253,252,255.0,252,255.0,252,255.0,
  137,60,36,232,251,1,11,139,68,36,32,139,84,36,36,255,199.0,68,36,4,252,255,
  252,255.0,252,255.0,252,255.0,137,60,36,232,251,1,17,137,68,36,32,199.0,68,
  36,4,252,253,252,255.0,252,255.0,252,255.0,137,60,36,232,251,1,11,139,68,
  36,32,255,199.0,68,36,4,252,255,252,255.0,252,255.0,252,255.0,137,60,36,232,
  251,1,18,255,221.0,92,36,32,199.0,68,36,4,252,253,252,255.0,252,255.0,252,
  255.0,137,60,36,232,251,1,11,221.0,68,36,32,255,199.0,68,36,4,252,255,252,
  255.0,252,255.0,252,255.0,137,60,36,232,251,1,19,137,68,36,32,137,84,36,36,
  199.0,68,36,4,252,253,252,255.0,252,255.0,252,255.0,137,60,36,232,251,1,11,
  139,68,36,32,139,84,36,36,255,199.0,68,36,4,252,255,252,255.0,252,255.0,252,
  255.0,137,60,36,232,251,1,20,102.0,15.0,214,68,36,32,102.0,15.0,214,76,36,
  40,199.0,68,36,4,252,253,252,255.0,252,255.0,252,255.0,137,60,36,232,251,
  1,11,252,243.0,15.0,126,68,36,32,252,243.0,15.0,126,76,36,40,255,139,141,
  233,199.0,68,36,8,252,255,252,255.0,252,255.0,252,255.0,137,124,36,4,137,
  12,36,232,251,1,20,131.0,252,236,4,199.0,68,36,4,252,253,252,255.0,252,255.0,
  252,255.0,137,60,36,232,251,1,11,255,139,125,252,252,137,252,236,93,194,236,
  255,85,137,229,87,86,139,189,233,131.0,252,236,16,137,60,36,232,251,1,21,
  137,198,129.0,252,248,239,15.0,140,244,16,255,15.0,143,244,17,255,193.0,224,
  4,41,196,129.0,252,236,239,255,199.0,68,36,8,237,199.0,68,36,4,0,0.0,0.0,
  0.0,137,60,36,232,251,1,7,131.0,252,236,16,255,199.0,68,36,12,237,199.0,68,
  36,8,237,199.0,68,36,4,237,137,60,36,232,251,1,10,255,199.0,68,36,12,237,
  199.0,68,36,8,237,199.0,68,36,4,237,137,60,36,232,251,1,22,255,199.0,68,36,
  12,237,199.0,68,36,8,237,199.0,68,36,4,237,137,60,36,232,251,1,12,255,199.0,
  68,36,4,237,137,60,36,232,251,1,14,255,15.0,182,192,255,15.0,190,192,255,
  15.0,183,192,255,15.0,191,192,255,199.0,68,36,4,237,137,60,36,232,251,1,14,
  131.0,252,248,0,15.0,149.0,208,15.0,182,192,255,199.0,68,36,4,237,137,60,
  36,232,251,1,13,255,199.0,68,36,4,237,137,60,36,232,251,1,17,255,199.0,68,
  36,4,237,137,60,36,232,251,1,15,255,199.0,68,36,4,237,137,60,36,232,251,1,
  16,255,199.0,68,36,4,237,137,60,36,232,251,1,18,255,199.0,68,36,4,237,137,
  60,36,232,251,1,20,255,252,243.0,15.0,126,193,255,141,132,253,36,233,131.0,
  252,236,4,199.0,68,36,8,237,137,124,36,4,137,4,36,232,251,1,20,255,199.0,
  68,36,4,237,137,60,36,232,251,1,19,255,199.0,68,36,4,237,137,60,36,232,251,
  1,19,137,4,36,217.0,4,36,255,137,20,36,217.0,4,36,255,137,224,129.0,192,239,
  137,68,36,12,137,116,36,8,199.0,68,36,4,237,137,60,36,232,251,1,23,255,185,
  237,139,1,137,4,36,232,251,1,24,255,131.0,196,28,255,139,148,253,36,233,255,
  139,12,36,255,129.0,196,239,255,232,251,1,25,131.0,252,236,48,255,137,68,
  36,32,232,251,1,0,185,237,137,1,199.0,68,36,8,237,199.0,68,36,4,237,137,60,
  36,232,251,1,7,139,76,36,32,137,8,252,233,244,10,255,137,84,36,36,137,68,
  36,32,232,251,1,0,185,237,137,1,199.0,68,36,8,237,199.0,68,36,4,0,0.0,0.0,
  0.0,137,60,36,232,251,1,7,139,76,36,36,139,84,36,32,137,72,4,137,16,252,233,
  244,10,255,137,68,36,32,137,84,36,36,232,251,1,0,185,237,137,1,199.0,68,36,
  8,237,199.0,68,36,4,0,0.0,0.0,0.0,137,60,36,232,251,1,7,139,76,36,32,137,
  8,139,76,36,36,137,72,4,252,233,244,10,255,131.0,252,236,4,232,251,1,0,185,
  237,137,1,252,233,244,10,255,252,233,244,11,255,252,233,244,13,255,252,233,
  244,14,255,252,233,244,15,255,252,233,244,12,255
};

static const char *const globnames[] = {
  "lua_return_arg",
  "lua_return_void",
  "lua_return_double",
  "lua_return_bool",
  "lua_return_int",
  "lua_return_uint",
  "too_few_arguments",
  "too_many_arguments",
  "save_registers",
  (const char *)0
};
static const char *const extnames[] = {
  "GetLastError",
  "lua_pushnumber",
  "lua_pushboolean",
  "push_int",
  "push_uint",
  "luaL_error",
  "lua_rawgeti",
  "push_cdata",
  "lua_remove",
  "lua_callk",
  "check_typed_pointer",
  "lua_settop",
  "check_enum",
  "check_uint32",
  "check_int32",
  "check_uint64",
  "check_int64",
  "check_uintptr",
  "check_double",
  "check_complex_float",
  "check_complex_double",
  "lua_gettop",
  "check_typed_cfunction",
  "unpack_varargs_stack",
  "SetLastError",
  "FUNCTION",
  (const char *)0
};



#if defined _WIN64 || defined __amd64__
#define JUMP_SIZE 14
#else
#define JUMP_SIZE 4
#endif

#define MIN_BRANCH INT32_MIN
#define MAX_BRANCH INT32_MAX
#define BRANCH_OFF 4

static void compile_extern_jump(struct jit* jit, lua_State* L, cfunction func, uint8_t* code)
{
    /* The jump code is the function pointer followed by a stub to call the
     * function pointer. The stub exists in 64 bit so we can jump to functions
     * with an offset greater than 2 GB.
     *
     * Note we have to manually set this up since there are commands buffered
     * in the jit state and dynasm doesn't support rip relative addressing.
     *
     * eg on 64 bit:
     * 0-8: function ptr
     * 8-14: jmp aword [rip-14]
     *
     * for 32 bit we only set the function ptr as it can always fit in a 32
     * bit displacement
     */
#if defined _WIN64 || defined __amd64__
    *(cfunction*) code = func;
    code[8] = 0xFF; /* FF /4 operand for jmp */
    code[9] = 0x25; /* RIP displacement */
    *(int32_t*) &code[10] = -14;
#else
    *(cfunction*) code = func;
#endif
}

void compile_globals(struct jit* jit, lua_State* L)
{
    struct jit* Dst = jit;
    int* perr = &jit->last_errno;
    dasm_setup(Dst, build_actionlist);

    /* Note: since the return code uses EBP to reset the stack pointer, we
     * don't have to track the amount of stack space used. It also means we
     * can handle stdcall and cdecl with the same code.
     */

    /* Note the various call_* functions want 32 bytes of 16 byte aligned
     * stack
     */




    /* the general idea for the return functions is:
     * 1) Save return value on stack
     * 2) Call get_errno (this trashes the registers hence #1)
     * 3) Unpack return value from stack
     * 4) Call lua push function
     * 5) Set eax to number of returned args (0 or 1)
     * 6) Call return which pops our stack frame
     */

    dasm_put(Dst, 0);

    dasm_put(Dst, 21, perr);

    dasm_put(Dst, 50, perr);

    dasm_put(Dst, 76, perr);

    dasm_put(Dst, 113, perr);

    dasm_put(Dst, 147, perr);

    dasm_put(Dst, 181, (ptrdiff_t)("too few arguments"));

    dasm_put(Dst, 200, (ptrdiff_t)("too many arguments"));

    dasm_put(Dst, 219);

    compile(Dst, L, NULL, LUA_NOREF);
}

int x86_return_size(lua_State* L, int usr, const struct ctype* ct)
{
    int ret = 0;
    const struct ctype* mt;

    if (ct->calling_convention != C_CALL) {
        size_t i;
        size_t argn = lua_rawlen(L, usr);
        for (i = 1; i <= argn; i++) {
            lua_rawgeti(L, usr, (int) i);
            mt = (const struct ctype*) lua_touserdata(L, -1);

            if (mt->pointers) {
                ret += sizeof(void*);
            } else {
                switch (mt->type) {
                case DOUBLE_TYPE:
                case COMPLEX_FLOAT_TYPE:
                case INT64_TYPE:
                    ret += 8;
                    break;
                case COMPLEX_DOUBLE_TYPE:
                    ret += 16;
                    break;
                case INTPTR_TYPE:
                    ret += sizeof(intptr_t);
                    break;
                case FUNCTION_PTR_TYPE:
                    ret += sizeof(cfunction);
                    break;
                case BOOL_TYPE:
                case FLOAT_TYPE:
                case INT8_TYPE:
                case INT16_TYPE:
                case INT32_TYPE:
                case ENUM_TYPE:
                    ret += 4;
                    break;
                default:
                    return luaL_error(L, "NYI - argument type");
                }
            }

            lua_pop(L, 1);
        }
    }

#if !defined _WIN64 && !defined __amd64__
    lua_rawgeti(L, usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
    if (!mt->pointers && mt->type == COMPLEX_DOUBLE_TYPE) {
        ret += sizeof(void*);
    }
    lua_pop(L, 1);
#endif

    return ret;
}

#ifdef _WIN64
#define MAX_REGISTERS(ct) 4 /* rcx, rdx, r8, r9 */

#elif defined __amd64__
#define MAX_INT_REGISTERS(ct) 6 /* rdi, rsi, rdx, rcx, r8, r9 */
#define MAX_FLOAT_REGISTERS(ct) 8 /* xmm0-7 */

#else
#define MAX_INT_REGISTERS(ct) ((ct)->calling_convention == FAST_CALL ? 2 /* ecx, edx */ : 0)
#define MAX_FLOAT_REGISTERS(ct) 0
#endif

struct reg_alloc {
#ifdef _WIN64
    int regs;
    int is_float[4];
    int is_int[4];
#else
    int floats;
    int ints;
#endif
    int off;
};

#ifdef _WIN64
#define REGISTER_STACK_SPACE(ct) (4*8)
#elif defined __amd64__
#define REGISTER_STACK_SPACE(ct) (14*8)
#else
#define REGISTER_STACK_SPACE(ct) LUAFFI_ALIGN_UP(((ct)->calling_convention == FAST_CALL ? 2*4 : 0), 15)
#endif

/* Fastcall:
 * Uses ecx, edx as first two int registers
 * Everything else on stack (include 64bit ints)
 * No overflow stack space
 * Pops the stack before returning
 * Returns int in eax, float in ST0
 * We use the same register allocation logic as posix x64 with 2 int regs and 0 float regs
 */

static void LUAFFI_get_int(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_int64)
{
    /* grab the register from the shadow space */
#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        dasm_put(Dst, 231, 16 + 8*reg->regs);
        reg->regs++;
    }
#elif __amd64__
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 231, - 80 - 8*reg->ints);
        reg->ints++;
    }
#else
    if (!is_int64 && reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 231, - 8 - 4*reg->ints);
        reg->ints++;
    }
#endif
    else if (is_int64) {
        dasm_put(Dst, 235, reg->off, reg->off + 4);
        reg->off += 8;
    } else {
        dasm_put(Dst, 231, reg->off);
        reg->off += 4;
    }
}

static void add_int(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_int64)
{
#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        dasm_put(Dst, 242, 32 + 8*(reg->regs));
        reg->is_int[reg->regs++] = 1;
    }
#elif __amd64__
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 242, 32 + 8*reg->ints);
        reg->ints++;
    }
#else
    if (!is_int64 && reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 242, 32 + 4*reg->ints);
        reg->ints++;
    }
#endif

    else if (is_int64) {
        dasm_put(Dst, 248, reg->off, reg->off + 4);
        reg->off += 8;
    } else {
        dasm_put(Dst, 242, reg->off);
        reg->off += 4;
    }
}

static void get_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
#if !defined _WIN64 && !defined __amd64__
    assert(MAX_FLOAT_REGISTERS(ct) == 0);
    if (is_double) {
        dasm_put(Dst, 259, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 263, reg->off);
        reg->off += 4;
    }
#else
    int off;

#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        off = -16 - 8*reg->regs;
        reg->regs++;
    }
#else
    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        off = -16 - 8*reg->floats;
        reg->floats++;
    }
#endif
    else {
        off = reg->off;
        reg->off += is_double ? 8 : 4;
    }

    if (is_double) {
        dasm_put(Dst, 267, off);
    } else {
        dasm_put(Dst, 274, off);
    }
#endif
}

static void add_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
#if !defined _WIN64 && !defined __amd64__
    assert(MAX_FLOAT_REGISTERS(ct) == 0);
    if (is_double) {
        dasm_put(Dst, 281, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 287, reg->off);
        reg->off += 4;
    }
#else

#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        if (is_double) {
            dasm_put(Dst, 293, 32 + 8*(reg->regs));
        } else {
            dasm_put(Dst, 301, 32 + 8*(reg->regs));
        }
        reg->is_float[reg->regs++] = 1;
    }
#else
    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        if (is_double) {
            dasm_put(Dst, 293, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        } else {
            dasm_put(Dst, 301, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        }
        reg->floats++;
    }
#endif

    else if (is_double) {
        dasm_put(Dst, 293, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 314, reg->off);
        reg->off += 4;
    }
#endif
}

#if defined _WIN64 || defined __amd64__
#define add_pointer(jit, ct, reg) add_int(jit, ct, reg, 1)
#define get_pointer(jit, ct, reg) LUAFFI_get_int(jit, ct, reg, 1)
#else
#define add_pointer(jit, ct, reg) add_int(jit, ct, reg, 0)
#define get_pointer(jit, ct, reg) LUAFFI_get_int(jit, ct, reg, 0)
#endif

cfunction compile_callback(lua_State* L, int fidx, int ct_usr, const struct ctype* ct)
{
    int i, nargs;
    cfunction* pf;
    struct ctype ct2 = *ct;
    const struct ctype* mt;
    struct reg_alloc reg;
    int num_upvals = 0;
    int top = lua_gettop(L);
    struct jit* Dst = get_jit(L);
    int ref;
    int hidden_arg_off = 0;

    ct_usr = lua_absindex(L, ct_usr);
    fidx = lua_absindex(L, fidx);

    assert(lua_isnil(L, fidx) || lua_isfunction(L, fidx));

    memset(&reg, 0, sizeof(reg));
#ifdef _WIN64
    reg.off = 16 + REGISTER_STACK_SPACE(ct); /* stack registers are above the shadow space */
#elif __amd64__
    reg.off = 16;
#else
    reg.off = 8;
#endif

    dasm_setup(Dst, build_actionlist);

    // add a table to store ctype and function upvalues
    // callback_set assumes the first value is the lua function
    nargs = (int) lua_rawlen(L, ct_usr);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    if (ct->has_var_arg) {
        luaL_error(L, "can't create callbacks with varargs");
    }

    // setup a stack frame to hold args for the call into lua_call

    dasm_put(Dst, 327, 4 + 16 + 32 + REGISTER_STACK_SPACE(ct));
    if (ct->calling_convention == FAST_CALL) {
        dasm_put(Dst, 336);
    }

    // hardcode the lua_State* value into the assembly
    dasm_put(Dst, 340, L);

    /* get the upval table */
    dasm_put(Dst, 343, ref, LUA_REGISTRYINDEX);

    /* get the lua function */
    lua_pushvalue(L, fidx);
    lua_rawseti(L, -2, ++num_upvals);
    assert(num_upvals == CALLBACK_FUNC_USR_IDX);
    dasm_put(Dst, 361, num_upvals);

#if !defined _WIN64 && !defined __amd64__
    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
    if (!mt->pointers && mt->type == COMPLEX_DOUBLE_TYPE) {
        hidden_arg_off = reg.off;
        reg.off += sizeof(void*);
    }
    lua_pop(L, 1);
#else
    (void) hidden_arg_off;
#endif

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);

        if (mt->pointers) {
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */
            /* on the lua stack in the callback:
             * upval tbl, lua func, i-1 args
             */
            dasm_put(Dst, 386, num_upvals-1, -i-1, mt);
            get_pointer(Dst, ct, &reg);
            dasm_put(Dst, 428);
        } else {
            switch (mt->type) {
            case INT64_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                dasm_put(Dst, 450, mt);
                LUAFFI_get_int(Dst, ct, &reg, 1);
                dasm_put(Dst, 471);
                break;

            case INTPTR_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                dasm_put(Dst, 450, mt);
                get_pointer(Dst, ct, &reg);
                dasm_put(Dst, 477);
                break;

            case COMPLEX_FLOAT_TYPE:
                lua_pop(L, 1);
#if defined _WIN64 || defined __amd64__
                /* complex floats are two floats packed into a double */
                dasm_put(Dst, 450, mt);
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 480);
#else
                /* complex floats are real followed by imag on the stack */
                dasm_put(Dst, 450, mt);
                get_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 485);
                get_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 488);
#endif
                break;

            case COMPLEX_DOUBLE_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 450, mt);
                /* real */
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 492);
                /* imag */
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 495);
                break;

            case FLOAT_TYPE:
            case DOUBLE_TYPE:
                lua_pop(L, 1);
                get_float(Dst, ct, &reg, mt->type == DOUBLE_TYPE);
                dasm_put(Dst, 499);
                break;

            case BOOL_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                dasm_put(Dst, 511);
                break;

            case INT8_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 526);
                } else {
                    dasm_put(Dst, 530);
                }
                dasm_put(Dst, 534);
                break;

            case INT16_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 546);
                } else {
                    dasm_put(Dst, 550);
                }
                dasm_put(Dst, 534);
                break;

            case ENUM_TYPE:
            case INT32_TYPE:
                lua_pop(L, 1);
                LUAFFI_get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 554);
                } else {
                    dasm_put(Dst, 534);
                }
                break;

            default:
                luaL_error(L, "NYI: callback arg type");
            }
        }
    }

    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);

    dasm_put(Dst, 566, (mt->pointers || mt->type != VOID_TYPE) ? 1 : 0, nargs);

    // Unpack the return argument if not "void", also clean-up the lua stack
    // to remove the return argument and bind table. Use lua_settop rather
    // than lua_pop as lua_pop is implemented as a macro.
    if (mt->pointers) {
        lua_getuservalue(L, -1);
        lua_rawseti(L, -3, ++num_upvals); /* usr value */
        lua_rawseti(L, -2, ++num_upvals); /* mt */
        dasm_put(Dst, 592, num_upvals-1, mt);

    } else {
        switch (mt->type) {
        case ENUM_TYPE:
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */
            dasm_put(Dst, 680, num_upvals-1, mt);
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 768);
            break;

        case BOOL_TYPE:
        case INT8_TYPE:
        case INT16_TYPE:
        case INT32_TYPE:
            lua_pop(L, 1);
            if (mt->is_unsigned) {
                dasm_put(Dst, 788);
            } else {
                dasm_put(Dst, 808);
            }
            dasm_put(Dst, 828);
            break;

        case INT64_TYPE:
            lua_pop(L, 1);

            if (mt->is_unsigned) {
                dasm_put(Dst, 856);
            } else {
                dasm_put(Dst, 876);
            }

            dasm_put(Dst, 896);
            break;

        case INTPTR_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 932);
            break;

        case FLOAT_TYPE:
        case DOUBLE_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 979);
            if (mt->type == FLOAT_TYPE) {
            } else {
            }
            dasm_put(Dst, 999);
            break;

        case COMPLEX_FLOAT_TYPE:
            lua_pop(L, 1);
#if !defined HAVE_COMPLEX
            luaL_error(L, "ffi lib compiled without complex number support");
#endif
            /* on 64 bit complex floats are two floats packed into a double,
             * on 32 bit returned complex floats use eax and edx */
            dasm_put(Dst, 1027);
            break;

        case COMPLEX_DOUBLE_TYPE:
            lua_pop(L, 1);
#if !defined HAVE_COMPLEX
            luaL_error(L, "ffi lib compiled without complex number support");
#endif
            /* on 64 bit, returned complex doubles use xmm0, xmm1, on 32 bit
             * there is a hidden first parameter that points to 16 bytes where
             * the returned arg is stored which is popped by the called
             * function */
#if defined _WIN64 || defined __amd64__
            dasm_put(Dst, 1082);
#else
            dasm_put(Dst, 1147, hidden_arg_off);
#endif
            break;

        default:
            luaL_error(L, "NYI: callback return type");
        }
    }

    dasm_put(Dst, 1197, x86_return_size(L, ct_usr, ct));

    lua_pop(L, 1); /* upval table - already in registry */
    assert(lua_gettop(L) == top);

    ct2.is_jitted = 1;
    pf = (cfunction*) push_cdata(L, ct_usr, &ct2);
    *pf = compile(Dst, L, NULL, ref);

    assert(lua_gettop(L) == top + 1);

    return *pf;
}

void compile_function(lua_State* L, cfunction func, int ct_usr, const struct ctype* ct)
{
    size_t i, nargs;
    int num_upvals;
    const struct ctype* mbr_ct;
    struct jit* Dst = get_jit(L);
    struct reg_alloc reg;
    void* p;
    int top = lua_gettop(L);
    int* perr = &Dst->last_errno;

    ct_usr = lua_absindex(L, ct_usr);

    memset(&reg, 0, sizeof(reg));
    reg.off = 32 + REGISTER_STACK_SPACE(ct);

    dasm_setup(Dst, build_actionlist);

    p = push_cdata(L, ct_usr, ct);
    *(cfunction*) p = func;
    num_upvals = 1;

    nargs = lua_rawlen(L, ct_usr);

    if (ct->calling_convention != C_CALL && ct->has_var_arg) {
        luaL_error(L, "vararg is only allowed with the c calling convention");
    }

    dasm_put(Dst, 1208, 8, nargs);

    if (!ct->has_var_arg) {
        dasm_put(Dst, 1238);
    }

    /* no need to zero extend eax returned by lua_gettop to rax as x86-64
     * preguarentees that the upper 32 bits will be zero */
    dasm_put(Dst, 1243, 32 + REGISTER_STACK_SPACE(ct));

#if !defined _WIN64 && !defined __amd64__
    /* Returned complex doubles require a hidden first parameter where the
     * data is stored, which is popped by the calling code. */
    lua_rawgeti(L, ct_usr, 0);
    mbr_ct = (const struct ctype*) lua_touserdata(L, -1);
    if (!mbr_ct->pointers && mbr_ct->type == COMPLEX_DOUBLE_TYPE) {
        /* we can allocate more space for arguments as long as no add_*
         * function has been called yet, mbr_ct will be added as an upvalue in
         * the return processing later */
        dasm_put(Dst, 1253, mbr_ct);
        add_pointer(Dst, ct, &reg);
    }
    lua_pop(L, 1);
#endif

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, (int) i);
        mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

        if (mbr_ct->pointers) {
            lua_getuservalue(L, -1);
            num_upvals += 2;
            dasm_put(Dst, 1278, mbr_ct, lua_upvalueindex(num_upvals), i);
            add_pointer(Dst, ct, &reg);
        } else {
            switch (mbr_ct->type) {
            case FUNCTION_PTR_TYPE:
                lua_getuservalue(L, -1);
                num_upvals += 2;
                dasm_put(Dst, 1301, mbr_ct, lua_upvalueindex(num_upvals), i);
                add_pointer(Dst, ct, &reg);
                break;

            case ENUM_TYPE:
                lua_getuservalue(L, -1);
                num_upvals += 2;
                dasm_put(Dst, 1324, mbr_ct, lua_upvalueindex(num_upvals), i);
                add_int(Dst, ct, &reg, 0);
                break;

            case INT8_TYPE:
                dasm_put(Dst, 1347, i);
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1360);
                } else {
                    dasm_put(Dst, 1364);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT16_TYPE:
                dasm_put(Dst, 1347, i);
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1368);
                } else {
                    dasm_put(Dst, 1372);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case BOOL_TYPE:
                dasm_put(Dst, 1376, i);
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT32_TYPE:
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1399, i);
                } else {
                    dasm_put(Dst, 1347, i);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INTPTR_TYPE:
                dasm_put(Dst, 1412, i);
                add_pointer(Dst, ct, &reg);
                lua_pop(L, 1);
                break;

            case INT64_TYPE:
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1425, i);
                } else {
                    dasm_put(Dst, 1438, i);
                }
                add_int(Dst, ct, &reg, 1);
                lua_pop(L, 1);
                break;

            case DOUBLE_TYPE:
                dasm_put(Dst, 1451, i);
                add_float(Dst, ct, &reg, 1);
                lua_pop(L, 1);
                break;

            case COMPLEX_DOUBLE_TYPE:
                /* on 64 bit, returned complex doubles use xmm0, xmm1, on 32 bit
                 * there is a hidden first parameter that points to 16 bytes where
                 * the returned arg is stored (this is popped by the called
                 * function) */
#if defined _WIN64 || defined __amd64__
                dasm_put(Dst, 1464, i);
                add_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 1477);
                add_float(Dst, ct, &reg, 1);
#else
                dasm_put(Dst, 1483, reg.off, i);
                reg.off += 16;
#endif
                lua_pop(L, 1);
                break;

            case FLOAT_TYPE:
                dasm_put(Dst, 1451, i);
                add_float(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case COMPLEX_FLOAT_TYPE:
#if defined _WIN64 || defined __amd64__
                dasm_put(Dst, 1509, i);
                /* complex floats are two floats packed into a double */
                add_float(Dst, ct, &reg, 1);
#else
                /* returned complex floats use eax and edx */
                dasm_put(Dst, 1522, i);
                add_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 1541);
                add_float(Dst, ct, &reg, 0);
#endif
                lua_pop(L, 1);
                break;

            default:
                luaL_error(L, "NYI: call arg type");
            }
        }
    }

    if (ct->has_var_arg) {
#ifdef _WIN64
        if (reg.regs < MAX_REGISTERS(ct)) {
            assert(reg.regs == nargs);
        } else {
        }

        for (i = nargs; i < MAX_REGISTERS(ct); i++) {
            reg.is_int[i] = reg.is_float[i] = 1;
        }
        reg.regs = MAX_REGISTERS(ct);
#elif defined __amd64__
        if (reg.floats < MAX_FLOAT_REGISTERS(ct)) {
        }

        if (reg.ints < MAX_INT_REGISTERS(ct)) {
        }


        reg.floats = MAX_FLOAT_REGISTERS(ct);
        reg.ints = MAX_INT_REGISTERS(ct);
#else
        dasm_put(Dst, 1548, reg.off, nargs+1);
#endif
    }

    dasm_put(Dst, 1574, perr);

    /* remove the stack space to call local functions */
    dasm_put(Dst, 1586);

#ifdef _WIN64
    switch (reg.regs) {
    case 4:
        if (reg.is_float[3]) {
        }
        if (reg.is_int[3]) {
        }
    case 3:
        if (reg.is_float[2]) {
        }
        if (reg.is_int[2]) {
        }
    case 2:
        if (reg.is_float[1]) {
        }
        if (reg.is_int[1]) {
        }
    case 1:
        if (reg.is_float[0]) {
        }
        if (reg.is_int[0]) {
        }
    case 0:
        break;
    }

    /* don't remove the space for the registers as we need 32 bytes of register overflow space */
    assert(REGISTER_STACK_SPACE(ct) == 32);

#elif defined __amd64__
    switch (reg.floats) {
    case 8:
    case 7:
    case 6:
    case 5:
    case 4:
    case 3:
    case 2:
    case 1:
    case 0:
        break;
    }

    switch (reg.ints) {
    case 6:
    case 5:
    case 4:
    case 3:
    case 2:
    case 1:
    case 0:
        break;
    }

#else
    if (ct->calling_convention == FAST_CALL) {
        switch (reg.ints) {
        case 2:
            dasm_put(Dst, 1590, 4);
        case 1:
            dasm_put(Dst, 1596);
        case 0:
            break;
        }

        dasm_put(Dst, 1600, REGISTER_STACK_SPACE(ct));
    }
#endif

#ifdef __amd64__
    if (ct->has_var_arg) {
        /* al stores an upper limit on the number of float register, note that
         * its allowed to be more than the actual number of float registers used as
         * long as its 0-8 */
    }
#endif

    dasm_put(Dst, 1604);

    /* note on windows X86 the stack may be only aligned to 4 (stdcall will
     * have popped a multiple of 4 bytes), but we don't need 16 byte alignment on
     * that platform
     */

    lua_rawgeti(L, ct_usr, 0);
    mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

    if (mbr_ct->pointers || mbr_ct->type == INTPTR_TYPE) {
        lua_getuservalue(L, -1);
        num_upvals += 2;
        dasm_put(Dst, 1613, perr, mbr_ct, lua_upvalueindex(num_upvals));

    } else {
        switch (mbr_ct->type) {
        case FUNCTION_PTR_TYPE:
            lua_getuservalue(L, -1);
            num_upvals += 2;
            dasm_put(Dst, 1613, perr, mbr_ct, lua_upvalueindex(num_upvals));
            break;

        case INT64_TYPE:
            num_upvals++;
            dasm_put(Dst, 1653, perr, mbr_ct);
            break;

        case COMPLEX_FLOAT_TYPE:
            num_upvals++;
            dasm_put(Dst, 1707, perr, mbr_ct);
            break;

        case COMPLEX_DOUBLE_TYPE:
            num_upvals++;
            dasm_put(Dst, 1761, perr);
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1778);
            break;

        case BOOL_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1783);
            break;

        case INT8_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 1360);
            } else {
                dasm_put(Dst, 1364);
            }
            dasm_put(Dst, 1788);
            break;

        case INT16_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 1368);
            } else {
                dasm_put(Dst, 1372);
            }
            dasm_put(Dst, 1788);
            break;

        case INT32_TYPE:
        case ENUM_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 1793);
            } else {
                dasm_put(Dst, 1788);
            }
            break;

        case FLOAT_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1798);
            break;

        case DOUBLE_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1798);
            break;

        default:
            luaL_error(L, "NYI: call return type");
        }
    }

    assert(lua_gettop(L) == top + num_upvals);
    {
        cfunction f = compile(Dst, L, func, LUA_NOREF);
        /* add a callback as an upval so that the jitted code gets cleaned up when
         * the function gets gc'd */
        push_callback(L, f);
        lua_pushcclosure(L, (lua_CFunction) f, num_upvals+1);
    }
}

#endif 
 
/* vim: ts=4 sw=4 sts=4 et tw=78
 * Copyright (c) 2011 James R. McKaskill. See license in ffi.h
 */
//#include "ffi.h"

static cfunction compile(Dst_DECL, lua_State* L, cfunction func, int ref);

static void* reserve_code(struct jit* jit, lua_State* L, size_t sz);
static void commit_code(struct jit* jit, void* p, size_t sz);

static void push_int(lua_State* L, int val)
{ lua_pushnumber(L, val); }

static void push_uint(lua_State* L, unsigned int val)
{ lua_pushnumber(L, val); }

static void push_float(lua_State* L, float val)
{ lua_pushnumber(L, val); }

#ifndef _WIN32
static int GetLastError(void)
{ return errno; }
static void SetLastError(int err)
{ errno = err; }
#endif

#ifdef NDEBUG
#define shred(a,b,c)
#else
#define shred(p,s,e) memset((uint8_t*)(p)+(s),0xCC,(e)-(s))
#endif


#ifdef _WIN64
//#include "dynasm/dasm_x86.h"
//#include "call_x64win.h"
#elif defined __amd64__
//#include "dynasm/dasm_x86.h"
//#include "call_x64.h"
#elif defined __arm__ || defined __arm || defined __ARM__ || defined __ARM || defined ARM || defined _ARM_ || defined ARMV4I || defined _M_ARM
//#include "dynasm/dasm_arm.h"
//#include "call_arm.h"
#else
//#include "dynasm/dasm_x86.h"
//#include "call_x86.h"
#endif

struct jit_head {
    size_t size;
    int ref;
    uint8_t jump[JUMP_SIZE];
};

#define LINKTABLE_MAX_SIZE (sizeof(extnames) / sizeof(extnames[0]) * (JUMP_SIZE))

static cfunction compile(struct jit* jit, lua_State* L, cfunction func, int ref)
{
    struct jit_head* code;
    size_t codesz;
    int err;

    dasm_checkstep(jit, -1);
    if ((err = dasm_link(jit, &codesz)) != 0) {
        char buf[32];
        sprintf(buf, "%x", err);
        luaL_error(L, "dasm_link error %s", buf);
    }

    codesz += sizeof(struct jit_head);
    code = (struct jit_head*) reserve_code(jit, L, codesz);
    code->ref = ref;
    code->size = codesz;
    compile_extern_jump(jit, L, func, code->jump);

    if ((err = dasm_encode(jit, code+1)) != 0) {
        char buf[32];
        sprintf(buf, "%x", err);
        commit_code(jit, code, 0);
        luaL_error(L, "dasm_encode error %s", buf);
    }

    commit_code(jit, code, codesz);
    return (cfunction) (code+1);
}

typedef uint8_t jump_t[JUMP_SIZE];

int get_extern(struct jit* jit, uint8_t* addr, int idx, int type)
{
    struct page* page = jit->pages[jit->pagenum-1];
    jump_t* jumps = (jump_t*) (page+1);
    struct jit_head* h = (struct jit_head*) ((uint8_t*) page + page->off);
    uint8_t* jmp;
    ptrdiff_t off;

    if (idx == jit->function_extern) {
       jmp = h->jump;
    } else {
       jmp = jumps[idx];
    }

    /* compensate for room taken up for the offset so that we can work rip
     * relative */
    addr += BRANCH_OFF;

    /* see if we can fit the offset in the branch displacement, if not use the
     * jump instruction */
    off = *(uint8_t**) jmp - addr;

    if (MIN_BRANCH <= off && off <= MAX_BRANCH) {
        return (int32_t) off;
    } else {
        return (int32_t)(jmp + sizeof(uint8_t*) - addr);
    }
}

#if LUA_VERSION_NUM >= 503
LUA_API void lua_remove_compat (lua_State *L, int idx) {
    lua_remove(L, idx);
}
#endif

static void* reserve_code(struct jit* jit, lua_State* L, size_t sz)
{
    struct page* page;
    size_t off = (jit->pagenum > 0) ? jit->pages[jit->pagenum-1]->off : 0;
    size_t size = (jit->pagenum > 0) ? jit->pages[jit->pagenum-1]->size : 0;

    if (off + sz >= size) {
        int i;
        uint8_t* pdata;
        cfunction func;

        /* need to create a new page */
        jit->pages = (struct page**) realloc(jit->pages, (++jit->pagenum) * sizeof(jit->pages[0]));

        size = LUAFFI_ALIGN_UP(sz + LINKTABLE_MAX_SIZE + sizeof(struct page), jit->align_page_size);

        page = (struct page*) AllocPage(size);
        jit->pages[jit->pagenum-1] = page;
        pdata = (uint8_t*) page;
        page->size = size;
        page->off = sizeof(struct page);

        lua_newtable(L);

#define ADDFUNC(DLL, NAME) \
        lua_pushliteral(L, #NAME); \
        func = DLL ? (cfunction) GetProcAddressA(DLL, #NAME) : NULL; \
        func = func ? func : (cfunction) &NAME; \
        lua_pushcfunction(L, (lua_CFunction) func); \
        lua_rawset(L, -3)

        ADDFUNC(NULL, check_double);
        ADDFUNC(NULL, check_float);
        ADDFUNC(NULL, check_uint64);
        ADDFUNC(NULL, check_int64);
        ADDFUNC(NULL, check_int32);
        ADDFUNC(NULL, check_uint32);
        ADDFUNC(NULL, check_uintptr);
        ADDFUNC(NULL, check_enum);
        ADDFUNC(NULL, check_typed_pointer);
        ADDFUNC(NULL, check_typed_cfunction);
        ADDFUNC(NULL, check_complex_double);
        ADDFUNC(NULL, check_complex_float);
        ADDFUNC(NULL, unpack_varargs_stack);
        ADDFUNC(NULL, unpack_varargs_stack_skip);
        ADDFUNC(NULL, unpack_varargs_reg);
        ADDFUNC(NULL, unpack_varargs_float);
        ADDFUNC(NULL, unpack_varargs_int);
        ADDFUNC(NULL, push_cdata);
        ADDFUNC(NULL, push_int);
        ADDFUNC(NULL, push_uint);
        ADDFUNC(NULL, push_float);
        ADDFUNC(jit->kernel32_dll, SetLastError);
        ADDFUNC(jit->kernel32_dll, GetLastError);
        ADDFUNC(jit->lua_dll, luaL_error);
        ADDFUNC(jit->lua_dll, lua_pushnumber);
        ADDFUNC(jit->lua_dll, lua_pushboolean);
        ADDFUNC(jit->lua_dll, lua_gettop);
        ADDFUNC(jit->lua_dll, lua_rawgeti);
        ADDFUNC(jit->lua_dll, lua_pushnil);
        ADDFUNC(jit->lua_dll, lua_callk);
        ADDFUNC(jit->lua_dll, lua_settop);
#if LUA_VERSION_NUM >= 503
    lua_pushliteral(L, "lua_remove");
    lua_pushcfunction(L, (lua_CFunction) lua_remove_compat);
    lua_rawset(L, -3);
#else
         ADDFUNC(jit->lua_dll, lua_remove);
#endif
#undef ADDFUNC

        for (i = 0; extnames[i] != NULL; i++) {

            if (strcmp(extnames[i], "FUNCTION") == 0) {
                shred(pdata + page->off, 0, JUMP_SIZE);
                jit->function_extern = i;

            } else {
                lua_getfield(L, -1, extnames[i]);
                func = (cfunction) lua_tocfunction(L, -1);

                if (func == NULL) {
                    luaL_error(L, "internal error: missing link for %s", extnames[i]);
                }

                compile_extern_jump(jit, L, func, pdata + page->off);
                lua_pop(L, 1);
            }

            page->off += JUMP_SIZE;
        }

        page->freed = page->off;
        lua_pop(L, 1);

    } else {
        page = jit->pages[jit->pagenum-1];
        EnableWrite(page, page->size);
    }

    return (uint8_t*) page + page->off;
}

static void commit_code(struct jit* jit, void* code, size_t sz)
{
    struct page* page = jit->pages[jit->pagenum-1];
    page->off += sz;
    EnableExecute(page, page->size);
    {
#if 0
        FILE* out = fopen("\\Hard Disk\\out.bin", "wb");
        fwrite(page, page->off, 1, out);
        fclose(out);
#endif
    }
}

/* push_func_ref pushes a copy of the upval table embedded in the compiled
 * function func.
 */
void push_func_ref(lua_State* L, cfunction func)
{
    struct jit_head* h = ((struct jit_head*) func) - 1;
    lua_rawgeti(L, LUA_REGISTRYINDEX, h->ref);
}

void free_code(struct jit* jit, lua_State* L, cfunction func)
{
    size_t i;
    struct jit_head* h = ((struct jit_head*) func) - 1;
    for (i = 0; i < jit->pagenum; i++) {
        struct page* p = jit->pages[i];

        if ((uint8_t*) h < (uint8_t*) p || (uint8_t*) p + p->size <= (uint8_t*) h) {
            continue;
        }

        luaL_unref(L, LUA_REGISTRYINDEX, h->ref);

        EnableWrite(p, p->size);
        p->freed += h->size;

        shred(h, 0, h->size);

        if (p->freed < p->off) {
            EnableExecute(p, p->size);
            return;
        }

        FreePage(p, p->size);
        memmove(&jit->pages[i], &jit->pages[i+1], (jit->pagenum - (i+1)) * sizeof(jit->pages[0]));
        jit->pagenum--;
        return;
    }

    assert(!"couldn't find func in the jit pages");
}

/* vim: ts=4 sw=4 sts=4 et tw=78
 * Copyright (c) 2011 James R. McKaskill. See license in ffi.h
 */
//#include "ffi.h"

static int to_define_key;

static void update_on_definition(lua_State* L, int ct_usr, int ct_idx)
{
    ct_usr = lua_absindex(L, ct_usr);
    ct_idx = lua_absindex(L, ct_idx);

    lua_pushlightuserdata(L, &to_define_key);
    lua_rawget(L, ct_usr);

    if (lua_isnil(L, -1)) {
        lua_pop(L, 1); /* pop the nil */

        /* {} */
        lua_newtable(L);

        /* {__mode='k'} */
        lua_newtable(L);
        lua_pushliteral(L, "k");
        lua_setfield(L, -2, "__mode");

        /* setmetatable({}, {__mode='k'}) */
        lua_setmetatable(L, -2);

        /* usr[TO_UPDATE_KEY] = setmetatable({}, {__mode='k'}) */
        lua_pushlightuserdata(L, &to_define_key);
        lua_pushvalue(L, -2);
        lua_rawset(L, ct_usr);

        /* leave the table on the stack */
    }

    /* to_update[ctype or cdata] = true */
    lua_pushvalue(L, ct_idx);
    lua_pushboolean(L, 1);
    lua_rawset(L, -3);

    /* pop the to_update table */
    lua_pop(L, 1);
}

void set_defined(lua_State* L, int ct_usr, struct ctype* ct)
{
    ct_usr = lua_absindex(L, ct_usr);

    ct->is_defined = 1;

    /* update ctypes and cdatas that were created before the definition came in */
    lua_pushlightuserdata(L, &to_define_key);
    lua_rawget(L, ct_usr);

    if (!lua_isnil(L, -1)) {
        lua_pushnil(L);

        while (lua_next(L, -2)) {
            struct ctype* upd = (struct ctype*) lua_touserdata(L, -2);
            upd->base_size = ct->base_size;
            upd->align_mask = ct->align_mask;
            upd->is_defined = 1;
            upd->is_variable_struct = ct->is_variable_struct;
            upd->variable_increment = ct->variable_increment;
            assert(!upd->variable_size_known);
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
        /* usr[TO_UPDATE_KEY] = nil */
        lua_pushlightuserdata(L, &to_define_key);
        lua_pushnil(L);
        lua_rawset(L, ct_usr);
    } else {
        lua_pop(L, 1);
    }
}

struct ctype* push_ctype(lua_State* L, int ct_usr, const struct ctype* ct)
{
    struct ctype* ret;
    ct_usr = lua_absindex(L, ct_usr);

    ret = (struct ctype*) lua_newuserdata(L, sizeof(struct ctype));
    *ret = *ct;

    push_upval(L, &ctype_mt_key);
    lua_setmetatable(L, -2);

#if LUA_VERSION_NUM == 501
    if (!ct_usr || lua_isnil(L, ct_usr)) {
        push_upval(L, &niluv_key);
        lua_setfenv(L, -2);
    }
#endif

    if (ct_usr && !lua_isnil(L, ct_usr)) {
        lua_pushvalue(L, ct_usr);
        lua_setuservalue(L, -2);
    }

    if (!ct->is_defined && ct_usr && !lua_isnil(L, ct_usr)) {
        update_on_definition(L, ct_usr, -1);
    }

    return ret;
}

size_t ctype_size(lua_State* L, const struct ctype* ct)
{
    if (ct->pointers - ct->is_array) {
        return sizeof(void*) * (ct->is_array ? ct->array_size : 1);

    } else if (!ct->is_defined || ct->type == VOID_TYPE) {
        return luaL_error(L, "can't calculate size of an undefined type");

    } else if (ct->variable_size_known) {
        assert(ct->is_variable_struct && !ct->is_array);
        return ct->base_size + ct->variable_increment;

    } else if (ct->is_variable_array || ct->is_variable_struct) {
        return luaL_error(L, "internal error: calc size of variable type with unknown size");

    } else {
        return ct->base_size * (ct->is_array ? ct->array_size : 1);
    }
}

void* push_cdata(lua_State* L, int ct_usr, const struct ctype* ct)
{
    struct cdata* cd;
    size_t sz = ct->is_reference ? sizeof(void*) : ctype_size(L, ct);
    ct_usr = lua_absindex(L, ct_usr);

    /* This is to stop valgrind from complaining. Bitfields are accessed in 8
     * byte chunks so that the code doesn't have to deal with different access
     * patterns, but this means that occasionally it will read past the end of
     * the struct. As its not setting the bits past the end (only reading and
     * then writing the bits back) and the read is aligned its a non-issue,
     * but valgrind complains nonetheless.
     */
    if (ct->has_bitfield) {
        sz = LUAFFI_ALIGN_UP(sz, 7);
    }

    cd = (struct cdata*) lua_newuserdata(L, sizeof(struct cdata) + sz);
    *(struct ctype*) &cd->type = *ct;
    memset(cd+1, 0, sz);

    /* TODO: handle cases where lua_newuserdata returns a pointer that is not
     * aligned */
#if 0
    assert((uintptr_t) (cd + 1) % 8 == 0);
#endif

#if LUA_VERSION_NUM == 501
    if (!ct_usr || lua_isnil(L, ct_usr)) {
        push_upval(L, &niluv_key);
        lua_setfenv(L, -2);
    }
#endif

    if (ct_usr && !lua_isnil(L, ct_usr)) {
        lua_pushvalue(L, ct_usr);
        lua_setuservalue(L, -2);
    }

    push_upval(L, &cdata_mt_key);
    lua_setmetatable(L, -2);

    if (!ct->is_defined && ct_usr && !lua_isnil(L, ct_usr)) {
        update_on_definition(L, ct_usr, -1);
    }

    return cd+1;
}

void push_callback(lua_State* L, cfunction f)
{
    cfunction* pf = (cfunction*) lua_newuserdata(L, sizeof(cfunction));
    *pf = f;

    push_upval(L, &callback_mt_key);
    lua_setmetatable(L, -2);
}

/* returns the value as a ctype, pushes the user value onto the stack */
void check_ctype(lua_State* L, int idx, struct ctype* ct)
{
    if (lua_isstring(L, idx)) {
        struct parser P;
        P.line = 1;
        P.prev = P.next = lua_tostring(L, idx);
        P.align_mask = DEFAULT_ALIGN_MASK;
        parse_type(L, &P, ct);
        parse_argument(L, &P, -1, ct, NULL, NULL);
        lua_remove(L, -2); /* remove the user value from parse_type */

    } else if (lua_getmetatable(L, idx)) {
        if (!equals_upval(L, -1, &ctype_mt_key)
                && !equals_upval(L, -1, &cdata_mt_key)) {
            goto err;
        }

        lua_pop(L, 1); /* pop the metatable */
        *ct = *(struct ctype*) lua_touserdata(L, idx);
        lua_getuservalue(L, idx);

    } else {
        goto err;
    }

    return;

err:
    luaL_error(L, "expected cdata, ctype or string for arg #%d", idx);
}

/* to_cdata returns the struct cdata* and pushes the user value onto the
 * stack. If the index is not a ctype then ct is not touched, a nil is pushed,
 * NULL is returned, and ct->type is set to INVALID_TYPE.  Also dereferences
 * references */
void* to_cdata(lua_State* L, int idx, struct ctype* ct)
{
    struct cdata* cd;

    ct->type = INVALID_TYPE;
    if (!lua_isuserdata(L, idx) || !lua_getmetatable(L, idx)) {
        lua_pushnil(L);
        return NULL;
    }

    if (!equals_upval(L, -1, &cdata_mt_key)) {
        lua_pop(L, 1); /* mt */
        lua_pushnil(L);
        return NULL;
    }

    lua_pop(L, 1); /* mt */
    cd = (struct cdata*) lua_touserdata(L, idx);
    *ct = cd->type;
    lua_getuservalue(L, idx);

    if (ct->is_reference) {
        ct->is_reference = 0;
        return *(void**) (cd+1);

    } else if (ct->pointers && !ct->is_array) {
        return *(void**) (cd+1);

    } else {
        return cd + 1;
    }
}

/* check_cdata returns the struct cdata* and pushes the user value onto the
 * stack. Also dereferences references. */
void* check_cdata(lua_State* L, int idx, struct ctype* ct)
{
    void* p = to_cdata(L, idx, ct);
    if (ct->type == INVALID_TYPE) {
        luaL_error(L, "expected cdata for arg #%d", idx);
    }
    return p;
}

/* vim: ts=4 sw=4 sts=4 et tw=78
 * Copyright (c) 2011 James R. McKaskill. See license in ffi.h
 */
//#include "ffi.h"

#define IS_CONST(tok) (IS_LITERAL(tok, "const") || IS_LITERAL(tok, "__const") || IS_LITERAL(tok, "__const__"))
#define IS_VOLATILE(tok) (IS_LITERAL(tok, "volatile") || IS_LITERAL(tok, "__volatile") || IS_LITERAL(tok, "__volatile__"))
#define IS_RESTRICT(tok) (IS_LITERAL(tok, "restrict") || IS_LITERAL(tok, "__restrict") || IS_LITERAL(tok, "__restrict__"))

enum etoken {
    TOK_NIL,
    TOK_NUMBER,
    TOK_STRING,
    TOK_TOKEN,

    /* the order of these values must match the token strings in lex.c */

    TOK_3_BEGIN,
    TOK_VA_ARG,

    TOK_2_BEGIN,
    TOK_LEFT_SHIFT, TOK_RIGHT_SHIFT, TOK_LOGICAL_AND, TOK_LOGICAL_OR, TOK_LESS_EQUAL,
    TOK_GREATER_EQUAL, TOK_EQUAL, TOK_NOT_EQUAL,

    TOK_1_BEGIN,
    TOK_OPEN_CURLY, TOK_CLOSE_CURLY, TOK_SEMICOLON, TOK_COMMA, TOK_COLON,
    TOK_ASSIGN, TOK_OPEN_PAREN, TOK_CLOSE_PAREN, TOK_OPEN_SQUARE, TOK_CLOSE_SQUARE,
    TOK_DOT, TOK_AMPERSAND, TOK_LOGICAL_NOT, TOK_BITWISE_NOT, TOK_MINUS,
    TOK_PLUS, TOK_STAR, TOK_DIVIDE, TOK_MODULUS, TOK_LESS,
    TOK_GREATER, TOK_BITWISE_XOR, TOK_BITWISE_OR, TOK_QUESTION, TOK_POUND,

    TOK_REFERENCE = TOK_AMPERSAND,
    TOK_MULTIPLY = TOK_STAR,
    TOK_BITWISE_AND = TOK_AMPERSAND,
};

struct token {
    enum etoken type;
    int64_t integer;
    const char* str;
    size_t size;
};

#define IS_LITERAL(TOK, STR) \
  (((TOK).size == sizeof(STR) - 1) && 0 == memcmp((TOK).str, STR, sizeof(STR) - 1))

/* the order of tokens _must_ match the order of the enum etoken enum */

static char tok3[][4] = {
    "...", /* unused ">>=", "<<=", */
};

static char tok2[][3] = {
    "<<", ">>", "&&", "||", "<=",
    ">=", "==", "!=",
    /* unused "+=", "-=", "*=", "/=", "%=", "&=", "^=", "|=", "++", "--", "->", "::", */
};

static char tok1[] = {
    '{', '}', ';', ',', ':',
    '=', '(', ')', '[', ']',
    '.', '&', '!', '~', '-',
    '+', '*', '/', '%', '<',
    '>', '^', '|', '?', '#'
};

static int next_token(lua_State* L, struct parser* P, struct token* tok)
{
    size_t i;
    const char* s = P->next;

    /* UTF8 BOM */
    if (s[0] == '\xEF' && s[1] == '\xBB' && s[2] == '\xBF') {
        s += 3;
    }

    /* consume whitespace and comments */
    for (;;) {
        /* consume whitespace */
        while(*s == '\t' || *s == '\n' || *s == ' ' || *s == '\v' || *s == '\r') {
            if (*s == '\n') {
                P->line++;
            }
            s++;
        }

        /* consume comments */
        if (*s == '/' && *(s+1) == '/') {

            s = strchr(s, '\n');
            if (!s) {
                luaL_error(L, "non-terminated comment");
            }

        } else if (*s == '/' && *(s+1) == '*') {
            s += 2;

            for (;;) {
                if (s[0] == '\0') {
                    luaL_error(L, "non-terminated comment");
                } else if (s[0] == '*' && s[1] == '/') {
                    s += 2;
                    break;
                } else if (s[0] == '\n') {
                    P->line++;
                }
                s++;
            }

        } else if (*s == '\0') {
            tok->type = TOK_NIL;
            return 0;

        } else {
            break;
        }
    }

    P->prev = s;

    for (i = 0; i < sizeof(tok3) / sizeof(tok3[0]); i++) {
        if (s[0] == tok3[i][0] && s[1] == tok3[i][1] && s[2] == tok3[i][2]) {
            tok->type = (enum etoken) (TOK_3_BEGIN + 1 + i);
            P->next = s + 3;
            goto end;
        }
    }

    for (i = 0; i < sizeof(tok2) / sizeof(tok2[0]); i++) {
        if (s[0] == tok2[i][0] && s[1] == tok2[i][1]) {
            tok->type = (enum etoken) (TOK_2_BEGIN + 1 + i);
            P->next = s + 2;
            goto end;
        }
    }

    for (i = 0; i < sizeof(tok1) / sizeof(tok1[0]); i++) {
        if (s[0] == tok1[i]) {
            tok->type = (enum etoken) (TOK_1_BEGIN + 1 + i);
            P->next = s + 1;
            goto end;
        }
    }

    if (*s == '.' || *s == '-' || ('0' <= *s && *s <= '9')) {
        /* number */
        tok->type = TOK_NUMBER;

        /* split out the negative case so we get the full range of bits for
         * unsigned (eg to support 0xFFFFFFFF where sizeof(long) == 4)
         */
        if (*s == '-') {
            tok->integer = strtol(s, (char**) &s, 0);
        } else {
            tok->integer = strtoul(s, (char**) &s, 0);
        }

        while (*s == 'u' || *s == 'U' || *s == 'l' || *s == 'L') {
            s++;
        }

        P->next = s;
        goto end;

    } else if (*s == '\'' || *s == '\"') {
        /* "..." or '...' */
        char quote = *s;
        s++; /* jump over " */

        tok->type = TOK_STRING;
        tok->str = s;

        while (*s != quote) {

            if (*s == '\0' || (*s == '\\' && *(s+1) == '\0')) {
                return luaL_error(L, "string not finished");
            }

            if (*s == '\\') {
                s++;
            }

            s++;
        }

        tok->size = s - tok->str;
        s++; /* jump over " */
        P->next = s;
        goto end;

    } else if (('a' <= *s && *s <= 'z') || ('A' <= *s && *s <= 'Z') || *s == '_') {
        /* tokens */
        tok->type = TOK_TOKEN;
        tok->str = s;

        while (('a' <= *s && *s <= 'z') || ('A' <= *s && *s <= 'Z') || *s == '_' || ('0' <= *s && *s <= '9')) {
            s++;
        }

        tok->size = s - tok->str;
        P->next = s;
        goto end;

    } else {
        return luaL_error(L, "invalid character %d", P->line);
    }

end:
    /*fprintf(stderr, "token %d %d %.*s %.10s\n", tok->type, (int) tok->size, (tok->type == TOK_TOKEN || tok->type == TOK_STRING) ? (int) tok->size : 0, tok->str, P->next);*/
    return 1;
}

static void require_token(lua_State* L, struct parser* P, struct token* tok)
{
    if (!next_token(L, P, tok)) {
        luaL_error(L, "unexpected end");
    }
}

static void check_token(lua_State* L, struct parser* P, int type, const char* str, const char* err, ...)
{
    struct token tok;
    if (!next_token(L, P, &tok) || tok.type != type || (tok.type == TOK_TOKEN && (tok.size != strlen(str) || memcmp(tok.str, str, tok.size) != 0))) {
        va_list ap;
        va_start(ap, err);
        lua_pushvfstring(L, err, ap);
        lua_error(L);
    }
}

static void put_back(struct parser* P)
{ P->next = P->prev; }


int64_t calculate_constant(lua_State* L, struct parser* P);

static int g_name_key;
static int g_front_name_key;
static int g_back_name_key;

#ifndef max
#define max(a,b) ((a) < (b) ? (b) : (a))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

enum test {TEST};

/* Parses an enum definition from after the open curly through to the close
 * curly. Expects the user table to be on the top of the stack
 */
static int parse_enum(lua_State* L, struct parser* P, struct ctype* type)
{
    struct token tok;
    int value = -1;
    int ct_usr = lua_gettop(L);

    for (;;) {
        require_token(L, P, &tok);

        assert(lua_gettop(L) == ct_usr);

        if (tok.type == TOK_CLOSE_CURLY) {
            break;
        } else if (tok.type != TOK_TOKEN) {
            return luaL_error(L, "unexpected token in enum at line %d", P->line);
        }

        lua_pushlstring(L, tok.str, tok.size);

        require_token(L, P, &tok);

        if (tok.type == TOK_COMMA || tok.type == TOK_CLOSE_CURLY) {
            /* we have an auto calculated enum value */
            value++;
        } else if (tok.type == TOK_ASSIGN) {
            /* we have an explicit enum value */
            value = (int) calculate_constant(L, P);
            require_token(L, P, &tok);
        } else {
            return luaL_error(L, "unexpected token in enum at line %d", P->line);
        }

        assert(lua_gettop(L) == ct_usr + 1);

        /* add the enum value to the constants table */
        push_upval(L, &constants_key);
        lua_pushvalue(L, -2);
        lua_pushnumber(L, value);
        lua_rawset(L, -3);
        lua_pop(L, 1);

        assert(lua_gettop(L) == ct_usr + 1);

        /* add the enum value to the enum usr value table */
        lua_pushnumber(L, value);
        lua_rawset(L, ct_usr);

        if (tok.type == TOK_CLOSE_CURLY) {
            break;
        } else if (tok.type != TOK_COMMA) {
            return luaL_error(L, "unexpected token in enum at line %d", P->line);
        }
    }

    type->base_size = sizeof(enum test);
    type->align_mask = sizeof(enum test) - 1;

    assert(lua_gettop(L) == ct_usr);
    return 0;
}

static void calculate_member_position(lua_State* L, struct parser* P, struct ctype* ct, struct ctype* mt, int* pbit_offset, int* pbitfield_type)
{
    int bit_offset = *pbit_offset;

    if (ct->type == UNION_TYPE) {
        size_t msize;

        if (mt->is_variable_struct || mt->is_variable_array) {
            luaL_error(L, "NYI: variable sized members in unions");
            return;

        } else if (mt->is_bitfield) {
            msize = (mt->align_mask + 1);
#ifdef _WIN32
            /* MSVC has a bug where it doesn't update the alignment of
             * a union for bitfield members. */
            mt->align_mask = 0;
#endif

        } else if (mt->is_array) {
            msize = mt->array_size * (mt->pointers > 1 ? sizeof(void*) : mt->base_size);

        } else {
            msize = mt->pointers ? sizeof(void*) : mt->base_size;
        }

        ct->base_size = max(ct->base_size, msize);

    } else if (mt->is_bitfield) {
        if (mt->has_member_name && mt->bit_size == 0) {
            luaL_error(L, "zero length bitfields must be unnamed on line %d", P->line);
        }

#if defined _WIN32
        /* MSVC uses a seperate storage unit for each size. This is aligned
         * before the first bitfield. :0 finishes up the storage unit using
         * the greater alignment of the storage unit or the type used with the
         * :0. This is equivalent to the :0 always creating a new storage
         * unit, but not necesserily using it yet.
         */

        if (*pbitfield_type == -1 && mt->bit_size == 0) {
            /* :0 not after a bitfield are ignored */
            return;
        }

        {
            int different_storage = mt->align_mask != *pbitfield_type;
            int no_room_left = bit_offset + mt->bit_size > (mt->align_mask + 1) * CHAR_BIT;

            if (different_storage || no_room_left || !mt->bit_size) {
                ct->base_size += (bit_offset + CHAR_BIT - 1) / CHAR_BIT;
                bit_offset = 0;
                if (*pbitfield_type >= 0) {
                    ct->base_size = LUAFFI_ALIGN_UP(ct->base_size, *pbitfield_type);
                }
                ct->base_size = LUAFFI_ALIGN_UP(ct->base_size, mt->align_mask);
            }
        }

        mt->bit_offset = bit_offset;
        mt->offset = ct->base_size;

        *pbitfield_type = mt->align_mask;
        bit_offset += mt->bit_size;

#elif defined OS_OSX
        /* OSX doesn't use containers and bitfields are not aligned. So
         * bitfields never add any padding, except for :0 which still forces
         * an alignment based off the type used with the :0 */
        if (mt->bit_size) {
            mt->offset = ct->base_size;
            mt->bit_offset = bit_offset;
            bit_offset += mt->bit_size;
            ct->base_size += bit_offset / CHAR_BIT;
            bit_offset = bit_offset % CHAR_BIT;
        } else {
            ct->base_size += (bit_offset + CHAR_BIT - 1) / CHAR_BIT;
            ct->base_size = LUAFFI_ALIGN_UP(ct->base_size, mt->align_mask);
            bit_offset = 0;
        }

        if (!mt->has_member_name) {
            /* unnamed bitfields don't update the struct alignment */
            mt->align_mask = 0;
        }

#elif defined __GNUC__ || defined __TINYC__ //< @r-lyeh: tcc case
        /* GCC tries to pack bitfields in as close as much as possible, but
         * still making sure that they don't cross alignment boundaries.
         * :0 forces an alignment based off the type used with the :0
         */

        int bits_used = (ct->base_size - ALIGN_DOWN(ct->base_size, mt->align_mask)) * CHAR_BIT + bit_offset;
        int need_to_realign = bits_used + mt->bit_size > mt->base_size * CHAR_BIT;

        if (!mt->is_packed && (!mt->bit_size || need_to_realign)) {
            ct->base_size += (bit_offset + CHAR_BIT - 1) / CHAR_BIT;
            ct->base_size = LUAFFI_ALIGN_UP(ct->base_size, mt->align_mask);
            bit_offset = 0;
        }

        mt->bit_offset = bit_offset;
        mt->offset = ct->base_size;

        bit_offset += mt->bit_size;
        ct->base_size += bit_offset / CHAR_BIT;
        bit_offset = bit_offset % CHAR_BIT;

        /* unnamed bitfields don't update the struct alignment */
        if (!mt->has_member_name) {
            mt->align_mask = 0;
        }
#else
#error
#endif

    } else {
        /* finish up the current bitfield storage unit */
        ct->base_size += (bit_offset + CHAR_BIT - 1) / CHAR_BIT;
        bit_offset = 0;

        if (*pbitfield_type >= 0) {
            ct->base_size = LUAFFI_ALIGN_UP(ct->base_size, *pbitfield_type);
        }

        *pbitfield_type = -1;

        ct->base_size = LUAFFI_ALIGN_UP(ct->base_size, mt->align_mask);
        mt->offset = ct->base_size;

        if (mt->is_variable_array) {
            ct->is_variable_struct = 1;
            ct->variable_increment = mt->pointers > 1 ? sizeof(void*) : mt->base_size;

        } else if (mt->is_variable_struct) {
            assert(!mt->variable_size_known && !mt->is_array && !mt->pointers);
            ct->base_size += mt->base_size;
            ct->is_variable_struct = 1;
            ct->variable_increment = mt->variable_increment;

        } else if (mt->is_array) {
            ct->base_size += mt->array_size * (mt->pointers > 1 ? sizeof(void*) : mt->base_size);

        } else {
            ct->base_size += mt->pointers ? sizeof(void*) : mt->base_size;
        }
    }

    /* increase the outer struct/union alignment if needed */
    if (mt->align_mask > (int) ct->align_mask) {
        ct->align_mask = mt->align_mask;
    }

    if (mt->has_bitfield || mt->is_bitfield) {
        ct->has_bitfield = 1;
    }

    *pbit_offset = bit_offset;
}

static int copy_submembers(lua_State* L, int to_usr, int from_usr, const struct ctype* ft, int* midx)
{
    struct ctype ct;
    int i, sublen;

    from_usr = lua_absindex(L, from_usr);
    to_usr = lua_absindex(L, to_usr);

    /* integer keys */
    sublen = (int) lua_rawlen(L, from_usr);
    for (i = 1; i <= sublen; i++) {
        lua_rawgeti(L, from_usr, i);

        ct = *(const struct ctype*) lua_touserdata(L, -1);
        ct.offset += ft->offset;
        lua_getuservalue(L, -1);

        push_ctype(L, -1, &ct);
        lua_rawseti(L, to_usr, (*midx)++);

        lua_pop(L, 2); /* ctype, user value */
    }

    /* string keys */
    lua_pushnil(L);
    while (lua_next(L, from_usr)) {
        if (lua_type(L, -2) == LUA_TSTRING) {
            struct ctype ct = *(const struct ctype*) lua_touserdata(L, -1);
            ct.offset += ft->offset;
            lua_getuservalue(L, -1);

            /* uservalue[sub_mname] = new_sub_mtype */
            lua_pushvalue(L, -3);
            push_ctype(L, -2, &ct);
            lua_rawset(L, to_usr);

            lua_pop(L, 1); /* remove submember user value */
        }
        lua_pop(L, 1);
    }

    return 0;
}

static int add_member(lua_State* L, int ct_usr, int mname, int mbr_usr, const struct ctype* mt, int* midx)
{
    ct_usr = lua_absindex(L, ct_usr);
    mname = lua_absindex(L, mname);

    push_ctype(L, mbr_usr, mt);

    /* usrvalue[mbr index] = pushed mtype */
    lua_pushvalue(L, -1);
    lua_rawseti(L, ct_usr, (*midx)++);

    /* set usrvalue[mname] = pushed mtype */
    lua_pushvalue(L, mname);
    lua_pushvalue(L, -2);
    lua_rawset(L, ct_usr);

    /* set usrvalue[mtype] = mname */
    lua_pushvalue(L, -1);
    lua_pushvalue(L, mname);
    lua_rawset(L, ct_usr);

    lua_pop(L, 1);

    return 0;
}

/* Parses a struct from after the open curly through to the close curly.
 */
static int parse_struct(lua_State* L, struct parser* P, int tmp_usr, const struct ctype* ct)
{
    struct token tok;
    int midx = 1;
    int top = lua_gettop(L);

    tmp_usr = lua_absindex(L, tmp_usr);

    /* parse members */
    for (;;) {
        struct ctype mbase;

        assert(lua_gettop(L) == top);

        /* see if we're at the end of the struct */
        require_token(L, P, &tok);
        if (tok.type == TOK_CLOSE_CURLY) {
            break;
        } else if (ct->is_variable_struct) {
            return luaL_error(L, "can't have members after a variable sized member on line %d", P->line);
        } else {
            put_back(P);
        }

        /* members are of the form
         * <base type> <arg>, <arg>, <arg>;
         * eg struct foo bar, *bar2[2];
         * mbase is 'struct foo'
         * mtype is '' then '*[2]'
         * mname is 'bar' then 'bar2'
         */

        parse_type(L, P, &mbase);

        for (;;) {
            struct token mname;
            struct ctype mt = mbase;

            memset(&mname, 0, sizeof(mname));

            if (ct->is_variable_struct) {
                return luaL_error(L, "can't have members after a variable sized member on line %d", P->line);
            }

            assert(lua_gettop(L) == top + 1);
            parse_argument(L, P, -1, &mt, &mname, NULL);
            assert(lua_gettop(L) == top + 2);

            if (!mt.is_defined && (mt.pointers - mt.is_array) == 0) {
                return luaL_error(L, "member type is undefined on line %d", P->line);
            }

            if (mt.type == VOID_TYPE && (mt.pointers - mt.is_array) == 0) {
                return luaL_error(L, "member type can not be void on line %d", P->line);
            }

            mt.has_member_name = (mname.size > 0);
            lua_pushlstring(L, mname.str, mname.size);

            add_member(L, tmp_usr, -1, -2, &mt, &midx);

            /* pop the usr value from push_argument and the member name */
            lua_pop(L, 2);
            assert(lua_gettop(L) == top + 1);

            require_token(L, P, &tok);
            if (tok.type == TOK_SEMICOLON) {
                break;
            } else if (tok.type != TOK_COMMA) {
                luaL_error(L, "unexpected token in struct definition on line %d", P->line);
            }
        }

        /* pop the usr value from push_type */
        lua_pop(L, 1);
    }

    assert(lua_gettop(L) == top);
    return 0;
}

static int calculate_struct_offsets(lua_State* L, struct parser* P, int ct_usr, struct ctype* ct, int tmp_usr)
{
    int i;
    int midx = 1;
    int sz = (int) lua_rawlen(L, tmp_usr);
    int bit_offset = 0;
    int bitfield_type = -1;

    ct_usr = lua_absindex(L, ct_usr);
    tmp_usr = lua_absindex(L, tmp_usr);

    for (i = 1; i <= sz; i++) {
        struct ctype mt;

        /* get the member type */
        lua_rawgeti(L, tmp_usr, i);
        mt = *(const struct ctype*) lua_touserdata(L, -1);

        /* get the member user table */
        lua_getuservalue(L, -1);

        /* get the member name */
        lua_pushvalue(L, -2);
        lua_rawget(L, tmp_usr);

        calculate_member_position(L, P, ct, &mt, &bit_offset, &bitfield_type);

        if (mt.has_member_name) {
            assert(!lua_isnil(L, -1));
            add_member(L, ct_usr, -1, -2, &mt, &midx);

        } else if (mt.type == STRUCT_TYPE || mt.type == UNION_TYPE) {
            /* With an unnamed member we copy all of the submembers into our
             * usr value adjusting the offset as necessary. Note ctypes are
             * immutable so need to push a new ctype to update the offset.
             */
            copy_submembers(L, ct_usr, -2, &mt, &midx);

        } else {
            /* We ignore unnamed members that aren't structs or unions. These
             * are there just to change the padding */
        }

        lua_pop(L, 3);
    }

    /* finish up the current bitfield storage unit */
    ct->base_size += (bit_offset + CHAR_BIT - 1) / CHAR_BIT;

    /* only void is allowed 0 size */
    if (ct->base_size == 0) {
        ct->base_size = 1;
    }

    ct->base_size = LUAFFI_ALIGN_UP(ct->base_size, ct->align_mask);
    return 0;
}

/* copy over attributes that could be specified before the typedef eg
 * __attribute__(packed) const type_t */
static void instantiate_typedef(struct parser* P, struct ctype* tt, const struct ctype* ft)
{
    struct ctype pt = *tt;
    *tt = *ft;

    tt->const_mask |= pt.const_mask;
    tt->is_packed = pt.is_packed;

    if (tt->is_packed) {
        tt->align_mask = 0;
    } else {
        /* Instantiate the typedef in the current packing. This may be
         * further updated if a pointer is added or another alignment
         * attribute is applied. If pt.align_mask is already non-zero than an
         * increased alignment via __declspec(aligned(#)) has been set. */
        tt->align_mask = max(min(P->align_mask, tt->align_mask), pt.align_mask);
    }
}

/* this parses a struct or union starting with the optional
 * name before the opening brace
 * leaves the type usr value on the stack
 */
static int parse_record(lua_State* L, struct parser* P, struct ctype* ct)
{
    struct token tok;
    int top = lua_gettop(L);

    require_token(L, P, &tok);

    /* name is optional */
    if (tok.type == TOK_TOKEN) {
        /* declaration */
        lua_pushlstring(L, tok.str, tok.size);

        assert(lua_gettop(L) == top+1);

        /* lookup the name to see if we've seen this type before */
        push_upval(L, &types_key);
        lua_pushvalue(L, -2);
        lua_rawget(L, top+2);

        assert(lua_gettop(L) == top+3);

        if (lua_isnil(L, -1)) {
            lua_pop(L, 1); /* pop the nil usr value */
            lua_newtable(L); /* the new usr table */

            /* stack layout is:
             * top+1: record name
             * top+2: types table
             * top+3: new usr table
             */

            lua_pushlightuserdata(L, &g_name_key);
            lua_pushvalue(L, top+1);
            lua_rawset(L, top+3); /* usr[name_key] = name */

            lua_pushvalue(L, top+1);
            push_ctype(L, top+3, ct);
            lua_rawset(L, top+2); /* types[name] = new_ctype */

        } else {
            /* get the exsting declared type */
            const struct ctype* prevt = (const struct ctype*) lua_touserdata(L, top+3);

            if (prevt->type != ct->type) {
                lua_getuservalue(L, top+3);
                push_type_name(L, -1, ct);
                push_type_name(L, top+3, prevt);
                luaL_error(L, "type '%s' previously declared as '%s'", lua_tostring(L, -2), lua_tostring(L, -1));
            }

            instantiate_typedef(P, ct, prevt);

            /* replace the ctype with its usr value */
            lua_getuservalue(L, -1);
            lua_replace(L, -2);
        }

        /* remove the extra name and types table */
        lua_replace(L, -3);
        lua_pop(L, 1);

        assert(lua_gettop(L) == top + 1 && lua_istable(L, -1));

        /* if a name is given then we may be at the end of the string
         * eg for ffi.new('struct foo')
         */
        if (!next_token(L, P, &tok)) {
            return 0;
        }

    } else {
        /* create a new unnamed record */
        int num;

        /* get the next unnamed number */
        push_upval(L, &next_unnamed_key);
        num = lua_tointeger(L, -1);
        lua_pop(L, 1);

        /* increment the unnamed upval */
        lua_pushinteger(L, num + 1);
        set_upval(L, &next_unnamed_key);

        lua_newtable(L); /* the new usr table - leave on stack */

        /* usr[name_key] = num */
        lua_pushlightuserdata(L, &g_name_key);
        lua_pushfstring(L, "%d", num);
        lua_rawset(L, -3);
    }

    if (tok.type != TOK_OPEN_CURLY) {
        /* this may just be a declaration or use of the type as an argument or
         * member */
        put_back(P);
        return 0;
    }

    if (ct->is_defined) {
        return luaL_error(L, "redefinition in line %d", P->line);
    }

    assert(lua_gettop(L) == top + 1 && lua_istable(L, -1));

    if (ct->type == ENUM_TYPE) {
        parse_enum(L, P, ct);
    } else {
        /* we do a two stage parse, where we parse the content first and build up
         * the temp user table. We then iterate over that to calculate the offsets
         * and fill out ct_usr. This is so we can handle out of order members
         * (eg vtable) and attributes specified at the end of the struct.
         */
        lua_newtable(L);
        parse_struct(L, P, -1, ct);
        calculate_struct_offsets(L, P, -2, ct, -1);
        assert(lua_gettop(L) == top + 2 && lua_istable(L, -1));
        lua_pop(L, 1);
    }

    assert(lua_gettop(L) == top + 1 && lua_istable(L, -1));
    set_defined(L, -1, ct);
    assert(lua_gettop(L) == top + 1);
    return 0;
}

/* parses single or multi work built in types, and pushes it onto the stack */
static int parse_type_name(lua_State* L, struct parser* P)
{
    struct token tok;
    int flags = 0;

    enum {
        UNSIGNED = 0x01,
        SIGNED = 0x02,
        LONG = 0x04,
        SHORT = 0x08,
        INT = 0x10,
        CHAR = 0x20,
        LONG_LONG = 0x40,
        INT8 = 0x80,
        INT16 = 0x100,
        INT32 = 0x200,
        INT64 = 0x400,
        DOUBLE = 0x800,
        FLOAT = 0x1000,
        COMPLEX = 0x2000,
    };

    require_token(L, P, &tok);

    /* we have to manually decode the builtin types since they can take up
     * more then one token
     */
    for (;;) {
        if (tok.type != TOK_TOKEN) {
            break;
        } else if (IS_LITERAL(tok, "unsigned")) {
            flags |= UNSIGNED;
        } else if (IS_LITERAL(tok, "signed")) {
            flags |= SIGNED;
        } else if (IS_LITERAL(tok, "short")) {
            flags |= SHORT;
        } else if (IS_LITERAL(tok, "char")) {
            flags |= CHAR;
        } else if (IS_LITERAL(tok, "long")) {
            flags |= (flags & LONG) ? LONG_LONG : LONG;
        } else if (IS_LITERAL(tok, "int")) {
            flags |= INT;
        } else if (IS_LITERAL(tok, "__int8")) {
            flags |= INT8;
        } else if (IS_LITERAL(tok, "__int16")) {
            flags |= INT16;
        } else if (IS_LITERAL(tok, "__int32")) {
            flags |= INT32;
        } else if (IS_LITERAL(tok, "__int64")) {
            flags |= INT64;
        } else if (IS_LITERAL(tok, "double")) {
            flags |= DOUBLE;
        } else if (IS_LITERAL(tok, "float")) {
            flags |= FLOAT;
        } else if (IS_LITERAL(tok, "complex") || IS_LITERAL(tok, "_Complex")) {
            flags |= COMPLEX;
        } else if (IS_LITERAL(tok, "register")) {
            /* ignore */
        } else {
            break;
        }

        if (!next_token(L, P, &tok)) {
            break;
        }
    }

    if (flags) {
        put_back(P);
    }

    if (flags & CHAR) {
        if (flags & SIGNED) {
            lua_pushliteral(L, "int8_t");
        } else if (flags & UNSIGNED) {
            lua_pushliteral(L, "uint8_t");
        } else {
            lua_pushstring(L, (((char) -1) > 0) ? "uint8_t" : "int8_t");
        }

    } else if (flags & INT8) {
        lua_pushstring(L, (flags & UNSIGNED) ? "uint8_t" : "int8_t");
    } else if (flags & INT16) {
        lua_pushstring(L, (flags & UNSIGNED) ? "uint16_t" : "int16_t");
    } else if (flags & INT32) {
        lua_pushstring(L, (flags & UNSIGNED) ? "uint32_t" : "int32_t");
    } else if (flags & (INT64 | LONG_LONG)) {
        lua_pushstring(L, (flags & UNSIGNED) ? "uint64_t" : "int64_t");

    } else if (flags & COMPLEX) {
        if (flags & LONG) {
            lua_pushliteral(L, "complex long double");
        } else if (flags & FLOAT) {
            lua_pushliteral(L, "complex float");
        } else {
            lua_pushliteral(L, "complex double");
        }

    } else if (flags & DOUBLE) {
        if (flags & LONG) {
            lua_pushliteral(L, "long double");
        } else {
            lua_pushliteral(L, "double");
        }

    } else if (flags & FLOAT) {
        lua_pushliteral(L, "float");

    } else if (flags & SHORT) {
#define SHORT_TYPE(u) (sizeof(short) == sizeof(int64_t) ? u "int64_t" : sizeof(short) == sizeof(int32_t) ? u "int32_t" : u "int16_t")
        if (flags & UNSIGNED) {
            lua_pushstring(L, SHORT_TYPE("u"));
        } else {
            lua_pushstring(L, SHORT_TYPE(""));
        }
#undef SHORT_TYPE

    } else if (flags & LONG) {
#define LONG_TYPE(u) (sizeof(long) == sizeof(int64_t) ? u "int64_t" : u "int32_t")
        if (flags & UNSIGNED) {
            lua_pushstring(L, LONG_TYPE("u"));
        } else {
            lua_pushstring(L, LONG_TYPE(""));
        }
#undef LONG_TYPE

    } else if (flags) {
#define INT_TYPE(u) (sizeof(int) == sizeof(int64_t) ? u "int64_t" : sizeof(int) == sizeof(int32_t) ? u "int32_t" : u "int16_t")
        if (flags & UNSIGNED) {
            lua_pushstring(L, INT_TYPE("u"));
        } else {
            lua_pushstring(L, INT_TYPE(""));
        }
#undef INT_TYPE

    } else {
        lua_pushlstring(L, tok.str, tok.size);
    }

    return 0;
}

/* parse_attribute parses a token to see if it is an attribute. It may then
 * parse some following tokens to decode the attribute setting the appropriate
 * fields in ct. It will return 1 if the token was used (and possibly some
 * more following it) or 0 if not. If the token was used, the next token must
 * be retrieved using next_token/require_token.
 */
static int parse_attribute(lua_State* L, struct parser* P, struct token* tok, struct ctype* ct, struct parser* asmname)
{
    if (tok->type != TOK_TOKEN) {
        return 0;

    } else if (asmname && (IS_LITERAL(*tok, "__asm__") || IS_LITERAL(*tok, "__asm"))) {
        check_token(L, P, TOK_OPEN_PAREN, NULL, "unexpected token after __asm__ on line %d", P->line);
        *asmname = *P;

        require_token(L, P, tok);
        while (tok->type == TOK_STRING) {
            require_token(L, P, tok);
        }

        if (tok->type != TOK_CLOSE_PAREN) {
            luaL_error(L, "unexpected token after __asm__ on line %d", P->line);
        }
        return 1;

    } else if (IS_LITERAL(*tok, "__attribute__") || IS_LITERAL(*tok, "__declspec")) {
        int parens = 1;
        check_token(L, P, TOK_OPEN_PAREN, NULL, "expected parenthesis after __attribute__ or __declspec on line %d", P->line);

        for (;;) {
            require_token(L, P, tok);
            if (tok->type == TOK_OPEN_PAREN) {
                parens++;
            } else if (tok->type == TOK_CLOSE_PAREN) {
                if (--parens == 0) {
                    break;
                }

            } else if (tok->type != TOK_TOKEN) {
                /* ignore unknown symbols within parentheses */

            } else if (IS_LITERAL(*tok, "align") || IS_LITERAL(*tok, "aligned") || IS_LITERAL(*tok, "__aligned__")) {
                unsigned align = 0;
                require_token(L, P, tok);

                switch (tok->type) {
                case TOK_CLOSE_PAREN:
                    align = ALIGNED_DEFAULT;
                    put_back(P);
                    break;

                case TOK_OPEN_PAREN:
                    require_token(L, P, tok);

                    if (tok->type != TOK_NUMBER) {
                        luaL_error(L, "expected align(#) on line %d", P->line);
                    }

                    switch (tok->integer) {
                    case 1: align = 0; break;
                    case 2: align = 1; break;
                    case 4: align = 3; break;
                    case 8: align = 7; break;
                    case 16: align = 15; break;
                    default:
                        luaL_error(L, "unsupported align size on line %d", P->line);
                    }

                    check_token(L, P, TOK_CLOSE_PAREN, NULL, "expected align(#) on line %d", P->line);
                    break;

                default:
                    luaL_error(L, "expected align(#) on line %d", P->line);
                }

                /* __attribute__(aligned(#)) is only supposed to increase alignment */
                ct->align_mask = max(align, ct->align_mask);

            } else if (IS_LITERAL(*tok, "packed") || IS_LITERAL(*tok, "__packed__")) {
                ct->align_mask = 0;
                ct->is_packed = 1;

            } else if (IS_LITERAL(*tok, "mode") || IS_LITERAL(*tok, "__mode__")) {

                check_token(L, P, TOK_OPEN_PAREN, NULL, "expected mode(MODE) on line %d", P->line);

                require_token(L, P, tok);
                if (tok->type != TOK_TOKEN) {
                    luaL_error(L, "expected mode(MODE) on line %d", P->line);
                }

                if (ct->type == FLOAT_TYPE || ct->type == DOUBLE_TYPE) {
                    struct {char ch; float v;} af;
                    struct {char ch; double v;} ad;

                    if (IS_LITERAL(*tok, "SF") || IS_LITERAL(*tok, "__SF__")) {
                        ct->type = FLOAT_TYPE;
                        ct->base_size = sizeof(float);
                        ct->align_mask = ALIGNOF(af);

                    } else if (IS_LITERAL(*tok, "DF") || IS_LITERAL(*tok, "__DF__")) {
                        ct->type = DOUBLE_TYPE;
                        ct->base_size = sizeof(double);
                        ct->align_mask = ALIGNOF(ad);

                    } else {
                        luaL_error(L, "unexpected mode on line %d", P->line);
                    }

                } else {
                    struct {char ch; uint16_t v;} a16;
                    struct {char ch; uint32_t v;} a32;
                    struct {char ch; uint64_t v;} a64;

                    if (IS_LITERAL(*tok, "QI") || IS_LITERAL(*tok, "__QI__")
                            || IS_LITERAL(*tok, "byte") || IS_LITERAL(*tok, "__byte__")
                            ) {
                        ct->type = INT8_TYPE;
                        ct->base_size = sizeof(uint8_t);
                        ct->align_mask = 0;

                    } else if (IS_LITERAL(*tok, "HI") || IS_LITERAL(*tok, "__HI__")) {
                        ct->type = INT16_TYPE;
                        ct->base_size = sizeof(uint16_t);
                        ct->align_mask = ALIGNOF(a16);

                    } else if (IS_LITERAL(*tok, "SI") || IS_LITERAL(*tok, "__SI__")
#if defined ARCH_X86 || defined ARCH_ARM
                            || IS_LITERAL(*tok, "word") || IS_LITERAL(*tok, "__word__")
                            || IS_LITERAL(*tok, "pointer") || IS_LITERAL(*tok, "__pointer__")
#endif
                            ) {
                        ct->type = INT32_TYPE;
                        ct->base_size = sizeof(uint32_t);
                        ct->align_mask = ALIGNOF(a32);

                    } else if (IS_LITERAL(*tok, "DI") || IS_LITERAL(*tok, "__DI__")
#if defined ARCH_X64
                            || IS_LITERAL(*tok, "word") || IS_LITERAL(*tok, "__word__")
                            || IS_LITERAL(*tok, "pointer") || IS_LITERAL(*tok, "__pointer__")
#endif
                            ) {
                        ct->type = INT64_TYPE;
                        ct->base_size = sizeof(uint64_t);
                        ct->align_mask = ALIGNOF(a64);

                    } else {
                        luaL_error(L, "unexpected mode on line %d", P->line);
                    }
                }

                check_token(L, P, TOK_CLOSE_PAREN, NULL, "expected mode(MODE) on line %d", P->line);

            } else if (IS_LITERAL(*tok, "cdecl") || IS_LITERAL(*tok, "__cdecl__")) {
                ct->calling_convention = C_CALL;

            } else if (IS_LITERAL(*tok, "fastcall") || IS_LITERAL(*tok, "__fastcall__")) {
                ct->calling_convention = FAST_CALL;

            } else if (IS_LITERAL(*tok, "stdcall") || IS_LITERAL(*tok, "__stdcall__")) {
                ct->calling_convention = STD_CALL;
            }
            /* ignore unknown tokens within parentheses */
        }
        return 1;

    } else if (IS_LITERAL(*tok, "__cdecl")) {
        ct->calling_convention = C_CALL;
        return 1;

    } else if (IS_LITERAL(*tok, "__fastcall")) {
        ct->calling_convention = FAST_CALL;
        return 1;

    } else if (IS_LITERAL(*tok, "__stdcall")) {
        ct->calling_convention = STD_CALL;
        return 1;

    } else if (IS_LITERAL(*tok, "__extension__") || IS_LITERAL(*tok, "extern")) {
        /* ignore */
        return 1;

    } else {
        return 0;
    }
}

/* parses out the base type of a type expression in a function declaration,
 * struct definition, typedef etc
 *
 * leaves the usr value of the type on the stack
 */
int parse_type(lua_State* L, struct parser* P, struct ctype* ct)
{
    struct token tok;
    int top = lua_gettop(L);

    memset(ct, 0, sizeof(*ct));

    require_token(L, P, &tok);

    /* get function attributes before the return type */
    while (parse_attribute(L, P, &tok, ct, NULL)) {
        require_token(L, P, &tok);
    }

    /* get const/volatile before the base type */
    for (;;) {
        if (tok.type != TOK_TOKEN) {
            return luaL_error(L, "unexpected value before type name on line %d", P->line);

        } else if (IS_CONST(tok)) {
            ct->const_mask = 1;
            require_token(L, P, &tok);

        } else if (IS_VOLATILE(tok) || IS_RESTRICT(tok)) {
            /* ignored for now */
            require_token(L, P, &tok);

        } else {
            break;
        }
    }

    /* get base type */
    if (tok.type != TOK_TOKEN) {
        return luaL_error(L, "unexpected value before type name on line %d", P->line);

    } else if (IS_LITERAL(tok, "struct")) {
        ct->type = STRUCT_TYPE;
        parse_record(L, P, ct);

    } else if (IS_LITERAL(tok, "union")) {
        ct->type = UNION_TYPE;
        parse_record(L, P, ct);

    } else if (IS_LITERAL(tok, "enum")) {
        ct->type = ENUM_TYPE;
        parse_record(L, P, ct);

    } else {
        put_back(P);

        /* lookup type */
        push_upval(L, &types_key);
        parse_type_name(L, P);
        lua_rawget(L, -2);
        lua_remove(L, -2);

        if (lua_isnil(L, -1)) {
            lua_pushlstring(L, tok.str, tok.size);
            return luaL_error(L, "unknown type %s on line %d", lua_tostring(L, -1), P->line);
        }

        instantiate_typedef(P, ct, (const struct ctype*) lua_touserdata(L, -1));

        /* we only want the usr tbl from the ctype in the types tbl */
        lua_getuservalue(L, -1);
        lua_replace(L, -2);
    }

    while (next_token(L, P, &tok)) {
        if (tok.type != TOK_TOKEN) {
            put_back(P);
            break;

        } else if (IS_CONST(tok) || IS_VOLATILE(tok)) {
            /* ignore for now */

        } else {
            put_back(P);
            break;
        }
    }

    assert(lua_gettop(L) == top + 1 && (lua_istable(L, -1) || lua_isnil(L, -1)));
    return 0;
}

enum name_type {
    BOTH,
    FRONT,
    BACK,
};

static void append_type_name(luaL_Buffer* B, int usr, const struct ctype* ct, enum name_type type)
{
    size_t i;
    lua_State* L = B->L;

    usr = lua_absindex(L, usr);

    if (type == FRONT || type == BOTH) {
        if (ct->type != FUNCTION_PTR_TYPE && (ct->const_mask & (1 << ct->pointers))) {
            luaL_addstring(B, "const ");
        }

        if (ct->is_unsigned) {
            luaL_addstring(B, "unsigned ");
        }

        switch (ct->type) {
        case ENUM_TYPE:
            luaL_addstring(B, "enum ");
            goto get_name;

        case STRUCT_TYPE:
            luaL_addstring(B, "struct ");
            goto get_name;

        case UNION_TYPE:
            luaL_addstring(B, "union ");
            goto get_name;

        get_name:
            lua_pushlightuserdata(L, &g_name_key);
            lua_rawget(L, usr);
            luaL_addvalue(B);
            break;

        case FUNCTION_TYPE:
        case FUNCTION_PTR_TYPE:
            lua_pushlightuserdata(L, &g_front_name_key);
            lua_rawget(L, usr);
            luaL_addvalue(B);
            break;

        case VOID_TYPE:
            luaL_addstring(B, "void");
            break;
        case BOOL_TYPE:
            luaL_addstring(B, "bool");
            break;
        case DOUBLE_TYPE:
            luaL_addstring(B, "double");
            break;
        case LONG_DOUBLE_TYPE:
            luaL_addstring(B, "long double");
            break;
        case FLOAT_TYPE:
            luaL_addstring(B, "float");
            break;
        case COMPLEX_LONG_DOUBLE_TYPE:
            luaL_addstring(B, "long complex double");
            break;
        case COMPLEX_DOUBLE_TYPE:
            luaL_addstring(B, "complex double");
            break;
        case COMPLEX_FLOAT_TYPE:
            luaL_addstring(B, "complex float");
            break;
        case INT8_TYPE:
            luaL_addstring(B, "char");
            break;
        case INT16_TYPE:
            luaL_addstring(B, "short");
            break;
        case INT32_TYPE:
            luaL_addstring(B, "int");
            break;
        case INT64_TYPE:
            luaL_addstring(B, "long long");
            break;

        case INTPTR_TYPE:
            if (sizeof(intptr_t) == sizeof(int32_t)) {
                luaL_addstring(B, "long");
            } else if (sizeof(intptr_t) == sizeof(int64_t)) {
                luaL_addstring(B, "long long");
            } else {
                luaL_error(L, "internal error - bad type");
            }
            break;

        default:
            luaL_error(L, "internal error - bad type %d", ct->type);
        }

        for (i = 0; i < ct->pointers - ct->is_array; i++) {
            luaL_addchar(B, '*');
            if (ct->const_mask & (1 << (ct->pointers - i - 1))) {
                luaL_addstring(B, " const");
            }
        }
    }

    if (type == BOTH || type == BACK) {
        if (ct->is_reference) {
            luaL_addstring(B, "(&)");
        }

        if (ct->is_variable_array && !ct->variable_size_known) {
            luaL_addstring(B, "[?]");
        } else if (ct->is_array) {
            lua_pushfstring(L, "[%d]", (int) ct->array_size);
            luaL_addvalue(B);
        }

        if (ct->type == FUNCTION_PTR_TYPE || ct->type == FUNCTION_TYPE) {
            lua_pushlightuserdata(L, &g_back_name_key);
            lua_rawget(L, usr);
            luaL_addvalue(B);
        }

        if (ct->is_bitfield) {
            lua_pushfstring(L, " : %d", (int) ct->bit_size);
            luaL_addvalue(B);
        }
    }
}

void push_type_name(lua_State* L, int usr, const struct ctype* ct)
{
    luaL_Buffer B;
    usr = lua_absindex(L, usr);
    luaL_buffinit(L, &B);
    append_type_name(&B, usr, ct, BOTH);
    luaL_pushresult(&B);
}

static void push_function_type_strings(lua_State* L, int usr, const struct ctype* ct)
{
    size_t i, args;
    luaL_Buffer B;
    int top = lua_gettop(L);
    const struct ctype* ret_ct;

    int arg_ct = top+3;
    int arg_usr = top+4;
    int ret_usr = top+6;

    usr = lua_absindex(L, usr);

    /* return type */
    lua_settop(L, top+4); /* room for two returns and two temp positions */
    lua_rawgeti(L, usr, 0);
    lua_getuservalue(L, -1);
    ret_ct = (const struct ctype*) lua_touserdata(L, -2);

    luaL_buffinit(L, &B);
    append_type_name(&B, ret_usr, ret_ct, FRONT);

    if (ret_ct->type != FUNCTION_TYPE && ret_ct->type != FUNCTION_PTR_TYPE) {
        luaL_addchar(&B, ' ');
    }

    switch (ct->calling_convention) {
    case STD_CALL:
        luaL_addstring(&B, "(__stdcall *");
        break;
    case FAST_CALL:
        luaL_addstring(&B, "(__fastcall *");
        break;
    case C_CALL:
        luaL_addstring(&B, "(*");
        break;
    default:
        luaL_error(L, "internal error - unknown calling convention");
    }

    luaL_pushresult(&B);
    lua_replace(L, top+1);

    luaL_buffinit(L, &B);
    luaL_addstring(&B, ")(");

    /* arguments */
    args = lua_rawlen(L, usr);
    for (i = 1; i <= args; i++) {
        if (i > 1) {
            luaL_addstring(&B, ", ");
        }

        /* note push the arg and user value below the indexes used by the buffer
         * and use indexes relative to top to avoid problems due to the buffer
         * system pushing a variable number of arguments onto the stack */
        lua_rawgeti(L, usr, (int) i);
        lua_replace(L, arg_ct);
        lua_getuservalue(L, arg_ct);
        lua_replace(L, arg_usr);
        append_type_name(&B, arg_usr, (const struct ctype*) lua_touserdata(L, arg_ct), BOTH);
    }

    luaL_addstring(&B, ")");
    append_type_name(&B, ret_usr, ret_ct, BACK);
    luaL_pushresult(&B);
    lua_replace(L, top+2);

    lua_settop(L, top+2);
    assert(lua_isstring(L, top+1) && lua_isstring(L, top+2));
}

/* parses from after the opening paranthesis to after the closing parenthesis */
static void parse_function_arguments(lua_State* L, struct parser* P, int ct_usr, struct ctype* ct)
{
    struct token tok;
    int args = 0;
    int top = lua_gettop(L);

    ct_usr = lua_absindex(L, ct_usr);

    for (;;) {
        require_token(L, P, &tok);

        if (tok.type == TOK_CLOSE_PAREN) {
            break;
        }

        if (args) {
            if (tok.type != TOK_COMMA) {
                luaL_error(L, "unexpected token in function argument %d on line %d", args, P->line);
            }

            require_token(L, P, &tok);
        }

        if (tok.type == TOK_VA_ARG) {
            ct->has_var_arg = true;
            check_token(L, P, TOK_CLOSE_PAREN, "", "unexpected token after ... in function on line %d", P->line);
            break;

        } else if (tok.type == TOK_TOKEN) {
            struct ctype at;

            put_back(P);
            parse_type(L, P, &at);
            parse_argument(L, P, -1, &at, NULL, NULL);

            assert(lua_gettop(L) == top + 2);

            /* array arguments are just treated as their base pointer type */
            at.is_array = 0;

            /* check for the c style int func(void) and error on other uses of arguments of type void */
            if (at.type == VOID_TYPE && at.pointers == 0) {
                if (args) {
                    luaL_error(L, "can't have argument of type void on line %d", P->line);
                }

                check_token(L, P, TOK_CLOSE_PAREN, "", "unexpected void in function on line %d", P->line);
                lua_pop(L, 2);
                break;
            }

            push_ctype(L, -1, &at);
            lua_rawseti(L, ct_usr, ++args);

            lua_pop(L, 2); /* parse_type and parse_argument at_usr */

        } else {
            luaL_error(L, "unexpected token in function argument %d on line %d", args+1, P->line);
        }
    }

    assert(lua_gettop(L) == top);
}

static int max_bitfield_size(int type)
{
    switch (type) {
    case BOOL_TYPE:
        return 1;
    case INT8_TYPE:
        return 8;
    case INT16_TYPE:
        return 16;
    case INT32_TYPE:
    case ENUM_TYPE:
        return 32;
    case INT64_TYPE:
        return 64;
    default:
        return -1;
    }
}

static struct ctype* parse_argument2(lua_State* L, struct parser* P, int ct_usr, struct ctype* ct, struct token* name, struct parser* asmname);

/* parses from after the first ( in a function declaration or function pointer
 * can be one of:
 * void foo(...) before ...
 * void (foo)(...) before foo
 * void (* <>)(...) before <> which is the inner type
 */
static struct ctype* parse_function(lua_State* L, struct parser* P, int ct_usr, struct ctype* ct, struct token* name, struct parser* asmname)
{
    /* We have a function pointer or a function. The usr table will
     * get replaced by the canonical one (if there is one) in
     * find_canonical_usr after all the arguments and returns have
     * been parsed. */
    struct token tok;
    int top = lua_gettop(L);
    struct ctype* ret;

    lua_newtable(L);
    ret = push_ctype(L, ct_usr, ct);
    lua_rawseti(L, -2, 0);
    ct_usr = lua_gettop(L);

    memset(ct, 0, sizeof(*ct));
    ct->base_size = sizeof(void (*)());
    ct->align_mask = min(FUNCTION_ALIGN_MASK, P->align_mask);
    ct->type = FUNCTION_TYPE;
    ct->is_defined = 1;

    if (name->type == TOK_NIL) {
        for (;;) {
            require_token(L, P, &tok);

            if (tok.type == TOK_STAR) {

                if (ct->type == FUNCTION_TYPE) {
                    ct->type = FUNCTION_PTR_TYPE;
                } else if (ct->pointers == POINTER_MAX) {
                    luaL_error(L, "maximum number of pointer derefs reached - use a struct to break up the pointers on line %d", P->line);
                } else {
                    ct->pointers++;
                    ct->const_mask <<= 1;
                }

            } else if (parse_attribute(L, P, &tok, ct, asmname)) {
                /* parse_attribute sets the appropriate fields */

            } else {
                /* call parse_argument to handle the inner contents
                 * e.g. the <> in "void (* <>) (...)". Note that the
                 * inner contents can itself be a function, a function
                 * ptr, array, etc (e.g. "void (*signal(int sig, void
                 * (*func)(int)))(int)" ).
                 */
                put_back(P);
                ct = parse_argument2(L, P, ct_usr, ct, name, asmname);
                break;
            }
        }

        check_token(L, P, TOK_CLOSE_PAREN, NULL, "unexpected token in function on line %d", P->line);
        check_token(L, P, TOK_OPEN_PAREN, NULL, "unexpected token in function on line %d", P->line);
    }

    parse_function_arguments(L, P, ct_usr, ct);

    /* if we have an inner function then set the outer function ptr as its
     * return type and return the inner function
     * e.g. for void (* <signal(int, void (*)(int))> )(int) inner is
     * surrounded by <>, return type is void (*)(int)
     */
    if (lua_gettop(L) == ct_usr+1) {
        lua_replace(L, ct_usr);
    }

    assert(lua_gettop(L) == top + 1 && lua_istable(L, -1));
    return ret;
}

static struct ctype* parse_argument2(lua_State* L, struct parser* P, int ct_usr, struct ctype* ct, struct token* name, struct parser* asmname)
{
    struct token tok;
    int top = lua_gettop(L);
    int ft_usr = 0;

    luaL_checkstack(L, 10, "function too complex");
    ct_usr = lua_absindex(L, ct_usr);

    for (;;) {
        if (!next_token(L, P, &tok)) {
            /* we've reached the end of the string */
            break;

        } else if (tok.type == TOK_STAR) {
            if (ct->pointers == POINTER_MAX) {
                luaL_error(L, "maximum number of pointer derefs reached - use a struct to break up the pointers on line %d", P->line);
            }

            ct->pointers++;
            ct->const_mask <<= 1;

            /* __declspec(align(#)) may come before the type in a member */
            if (!ct->is_packed) {
                ct->align_mask = max(min(PTR_ALIGN_MASK, P->align_mask), ct->align_mask);
            }

        } else if (tok.type == TOK_REFERENCE) {
            luaL_error(L, "NYI: c++ reference types");

        } else if (parse_attribute(L, P, &tok, ct, asmname)) {
            /* parse attribute has filled out appropriate fields in type */

        } else if (tok.type == TOK_OPEN_PAREN) {
            ct = parse_function(L, P, ct_usr, ct, name, asmname);
            ft_usr = lua_gettop(L);

        } else if (tok.type == TOK_OPEN_SQUARE) {
            /* array */
            if (ct->pointers == POINTER_MAX) {
                luaL_error(L, "maximum number of pointer derefs reached - use a struct to break up the pointers");
            }
            ct->is_array = 1;
            ct->pointers++;
            ct->const_mask <<= 1;
            require_token(L, P, &tok);

            if (ct->pointers == 1 && !ct->is_defined) {
                luaL_error(L, "array of undefined type on line %d", P->line);
            }

            if (ct->is_variable_struct || ct->is_variable_array) {
                luaL_error(L, "can't have an array of a variably sized type on line %d", P->line);
            }

            if (tok.type == TOK_QUESTION) {
                ct->is_variable_array = 1;
                ct->variable_increment = (ct->pointers > 1) ? sizeof(void*) : ct->base_size;
                check_token(L, P, TOK_CLOSE_SQUARE, "", "invalid character in array on line %d", P->line);

            } else if (tok.type == TOK_CLOSE_SQUARE) {
                ct->array_size = 0;

            } else if (tok.type == TOK_TOKEN && IS_RESTRICT(tok)) {
                /* odd gcc extension foo[__restrict] for arguments */
                ct->array_size = 0;
                check_token(L, P, TOK_CLOSE_SQUARE, "", "invalid character in array on line %d", P->line);

            } else {
                int64_t asize;
                put_back(P);
                asize = calculate_constant(L, P);
                if (asize < 0) {
                    luaL_error(L, "array size can not be negative on line %d", P->line);
                }
                ct->array_size = (size_t) asize;
                check_token(L, P, TOK_CLOSE_SQUARE, "", "invalid character in array on line %d", P->line);
            }

        } else if (tok.type == TOK_COLON) {
            int64_t bsize = calculate_constant(L, P);

            if (ct->pointers || bsize < 0 || bsize > max_bitfield_size(ct->type)) {
                luaL_error(L, "invalid bitfield on line %d", P->line);
            }

            ct->is_bitfield = 1;
            ct->bit_size = (unsigned) bsize;

        } else if (tok.type != TOK_TOKEN) {
            /* we've reached the end of the declaration */
            put_back(P);
            break;

        } else if (IS_CONST(tok)) {
            ct->const_mask |= 1;

        } else if (IS_VOLATILE(tok) || IS_RESTRICT(tok)) {
            /* ignored for now */

        } else {
            *name = tok;
        }
    }

    assert((ft_usr == 0 && lua_gettop(L) == top) || (lua_gettop(L) == top + 1 && ft_usr == top + 1 && (lua_istable(L, -1) || lua_isnil(L, -1))));
    return ct;
}

static void find_canonical_usr(lua_State* L, int ct_usr, const struct ctype *ct)
{
    struct ctype rt;
    int top = lua_gettop(L);
    int types;

    if (ct->type != FUNCTION_PTR_TYPE && ct->type != FUNCTION_TYPE) {
        return;
    }

    luaL_checkstack(L, 10, "function too complex");
    ct_usr = lua_absindex(L, ct_usr);

    /* check to see if we already have the canonical usr table */
    lua_pushlightuserdata(L, &g_name_key);
    lua_rawget(L, ct_usr);
    if (!lua_isnil(L, -1)) {
        lua_pop(L, 1);
        assert(top == lua_gettop(L));
        return;
    }
    lua_pop(L, 1);

    assert(top == lua_gettop(L));

    /* first canonize the return type */
    lua_rawgeti(L, ct_usr, 0);
    rt = *(struct ctype*) lua_touserdata(L, -1);
    lua_getuservalue(L, -1);
    find_canonical_usr(L, -1, &rt);
    push_ctype(L, -1, &rt);
    lua_rawseti(L, ct_usr, 0);
    lua_pop(L, 2); /* return ctype and usr */

    assert(top == lua_gettop(L));

    /* look up the type string in the types table */
    push_upval(L, &types_key);
    types = lua_gettop(L);

    push_function_type_strings(L, ct_usr, ct);
    lua_pushvalue(L, -2);
    lua_pushvalue(L, -2);
    lua_concat(L, 2);

    lua_pushvalue(L, -1);
    lua_rawget(L, types);

    assert(lua_gettop(L) == types + 4 && types == top + 1);
    /* stack: types, front, back, both, looked up value */

    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);

        lua_pushlightuserdata(L, &g_front_name_key);
        lua_pushvalue(L, -4);
        lua_rawset(L, ct_usr);

        lua_pushlightuserdata(L, &g_back_name_key);
        lua_pushvalue(L, -3);
        lua_rawset(L, ct_usr);

        lua_pushlightuserdata(L, &g_name_key);
        lua_pushvalue(L, -2);
        lua_rawset(L, ct_usr);

        lua_pushvalue(L, -1);
        push_ctype(L, ct_usr, ct);
        lua_rawset(L, types);
    } else {
        lua_getuservalue(L, -1);
        lua_replace(L, ct_usr);
        lua_pop(L, 1);
    }

    lua_pop(L, 4);
    assert(top == lua_gettop(L) && types == top + 1);
}


/* parses after the main base type of a typedef, function argument or
 * struct/union member
 * eg for const void* bar[3] the base type is void with the subtype so far of
 * const, this parses the "* bar[3]" and updates the type argument
 *
 * ct_usr and type must be as filled out by parse_type
 *
 * pushes the updated user value on the top of the stack
 */
void parse_argument(lua_State* L, struct parser* P, int ct_usr, struct ctype* ct, struct token* pname, struct parser* asmname)
{
    struct token tok, name;
    int top = lua_gettop(L);

    memset(&name, 0, sizeof(name));
    parse_argument2(L, P, ct_usr, ct, &name, asmname);

    for (;;) {
        if (!next_token(L, P, &tok)) {
            break;
        } else if (parse_attribute(L, P, &tok, ct, asmname)) {
            /* parse_attribute sets the appropriate fields */
        } else {
            put_back(P);
            break;
        }
    }

    if (lua_gettop(L) == top) {
        lua_pushvalue(L, ct_usr);
    }

    find_canonical_usr(L, -1, ct);

    if (pname) {
        *pname = name;
    }
}

static void parse_typedef(lua_State* L, struct parser* P)
{
    struct token tok;
    struct ctype base_type;
    int top = lua_gettop(L);

    parse_type(L, P, &base_type);

    for (;;) {
        struct ctype arg_type = base_type;
        struct token name;

        memset(&name, 0, sizeof(name));

        assert(lua_gettop(L) == top + 1);
        parse_argument(L, P, -1, &arg_type, &name, NULL);
        assert(lua_gettop(L) == top + 2);

        if (!name.size) {
            luaL_error(L, "Can't have a typedef without a name on line %d", P->line);
        } else if (arg_type.is_variable_array) {
            luaL_error(L, "Can't typedef a variable length array on line %d", P->line);
        }

        push_upval(L, &types_key);
        lua_pushlstring(L, name.str, name.size);
        push_ctype(L, -3, &arg_type);
        lua_rawset(L, -3);
        lua_pop(L, 2); /* types and parse_argument usr tbl */

        require_token(L, P, &tok);

        if (tok.type == TOK_SEMICOLON) {
            break;
        } else if (tok.type != TOK_COMMA) {
            luaL_error(L, "Unexpected character in typedef on line %d", P->line);
        }
    }

    lua_pop(L, 1); /* parse_type usr tbl */
    assert(lua_gettop(L) == top);
}

static bool is_hex(char ch)
{ return ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F'); }

static bool is_digit(char ch)
{ return '0' <= ch && ch <= '9'; }

static int from_hex(char ch)
{
    if (ch >= 'a') {
        return ch - 'a' + 10;
    } else if (ch >= 'A') {
        return ch - 'A' + 10;
    } else {
        return ch - '0';
    }
}

static void push_strings(lua_State* L, struct parser* P)
{
    luaL_Buffer B;
    luaL_buffinit(L, &B);

    for (;;) {
        const char *p, *e;
        char *t, *s;
        struct token tok;

        require_token(L, P, &tok);
        if (tok.type != TOK_STRING) {
            break;
        }

        p = tok.str;
        e = p + tok.size;

        t = luaL_prepbuffsize(&B, tok.size);
        s = t;

        while (p < e) {
            if (*p == '\\') {
                if (++p == e) {
                    luaL_error(L, "parse error in string");
                }
                switch (*p) {
                case '\\': *(t++) = '\\'; p++; break;
                case '\"': *(t++) = '\"'; p++; break;
                case '\'': *(t++) = '\''; p++; break;
                case 'n': *(t++) = '\n'; p++; break;
                case 'r': *(t++) = '\r'; p++; break;
                case 'b': *(t++) = '\b'; p++; break;
                case 't': *(t++) = '\t'; p++; break;
                case 'f': *(t++) = '\f'; p++; break;
                case 'a': *(t++) = '\a'; p++; break;
                case 'v': *(t++) = '\v'; p++; break;
                case 'e': *(t++) = 0x1B; p++; break;
                case 'x':
                    {
                        uint8_t u;
                        p++;
                        if (p + 2 > e || !is_hex(p[0]) || !is_hex(p[1])) {
                            luaL_error(L, "parse error in string");
                        }
                        u = (from_hex(p[0]) << 4) | from_hex(p[1]);
                        *(t++) = *(char*) &u;
                        p += 2;
                        break;
                    }
                default:
                    {
                        uint8_t u;
                        const char* e2 = min(p + 3, e);
                        if (!is_digit(*p)) {
                            luaL_error(L, "parse error in string");
                        }
                        u = *p - '0';
                        p++;
                        while (is_digit(*p) && p < e2) {
                            u = 10*u + *p-'0';
                            p++;
                        }
                        *(t++) = *(char*) &u;
                        break;
                    }
                }
            } else {
                *(t++) = *(p++);
            }
        }

        luaL_addsize(&B, t-s);
    }

    luaL_pushresult(&B);
}

#define END 0
#define PRAGMA_POP 1

static int parse_root(lua_State* L, struct parser* P)
{
    int top = lua_gettop(L);
    struct token tok;

    while (next_token(L, P, &tok)) {
        /* we can have:
         * struct definition
         * enum definition
         * union definition
         * struct/enum/union declaration
         * typedef
         * function declaration
         * pragma pack
         */

        assert(lua_gettop(L) == top);

        if (tok.type == TOK_SEMICOLON) {
            /* empty semicolon in root continue on */

        } else if (tok.type == TOK_POUND) {

            check_token(L, P, TOK_TOKEN, "pragma", "unexpected pre processor directive on line %d", P->line);
            check_token(L, P, TOK_TOKEN, "pack", "unexpected pre processor directive on line %d", P->line);
            check_token(L, P, TOK_OPEN_PAREN, "", "invalid pack directive on line %d", P->line);

            require_token(L, P, &tok);

            if (tok.type == TOK_NUMBER) {
                if (tok.integer != 1 && tok.integer != 2 && tok.integer != 4 && tok.integer != 8 && tok.integer != 16) {
                    luaL_error(L, "pack directive with invalid pack size on line %d", P->line);
                }

                P->align_mask = (unsigned) (tok.integer - 1);
                check_token(L, P, TOK_CLOSE_PAREN, "", "invalid pack directive on line %d", P->line);

            } else if (tok.type == TOK_TOKEN && IS_LITERAL(tok, "push")) {
                int line = P->line;
                unsigned previous_alignment = P->align_mask;

                check_token(L, P, TOK_CLOSE_PAREN, "", "invalid pack directive on line %d", P->line);

                if (parse_root(L, P) != PRAGMA_POP) {
                    luaL_error(L, "reached end of string without a pragma pop to match the push on line %d", line);
                }

                P->align_mask = previous_alignment;

            } else if (tok.type == TOK_TOKEN && IS_LITERAL(tok, "pop")) {
                check_token(L, P, TOK_CLOSE_PAREN, "", "invalid pack directive on line %d", P->line);
                return PRAGMA_POP;

            } else {
                luaL_error(L, "invalid pack directive on line %d", P->line);
            }


        } else if (tok.type != TOK_TOKEN) {
            return luaL_error(L, "unexpected character on line %d", P->line);

        } else if (IS_LITERAL(tok, "__extension__")) {
            /* ignore */
            continue;

        } else if (IS_LITERAL(tok, "extern")) {
            /* ignore extern as data and functions can only be extern */
            continue;

        } else if (IS_LITERAL(tok, "typedef")) {
            parse_typedef(L, P);

        } else if (IS_LITERAL(tok, "static")) {
            struct ctype at;

            int64_t val;
            require_token(L, P, &tok);
            if (!IS_CONST(tok)) {
                luaL_error(L, "expected 'static const int' on line %d", P->line);
            }

            parse_type(L, P, &at);

            require_token(L, P, &tok);
            if (tok.type != TOK_TOKEN) {
                luaL_error(L, "expected constant name after 'static const int' on line %d", P->line);
            }

            check_token(L, P, TOK_ASSIGN, "", "expected = after 'static const int <name>' on line %d", P->line);

            val = calculate_constant(L, P);

            check_token(L, P, TOK_SEMICOLON, "", "expected ; after 'static const int' definition on line %d", P->line);

            push_upval(L, &constants_key);
            lua_pushlstring(L, tok.str, tok.size);

            switch (at.type) {
                case INT8_TYPE:
                case INT16_TYPE:
                case INT32_TYPE:
                    if (at.is_unsigned)
                        lua_pushnumber(L, (unsigned int) val);
                    else
                        lua_pushnumber(L, (int) val);
                    break;

                default:
                    luaL_error(L, "expected a valid 8-, 16-, or 32-bit signed or unsigned integer type after 'static const' on line %d", P->line);
            }

            lua_rawset(L, -3);
            lua_pop(L, 2); /*constants and type*/

        } else {
            /* type declaration, type definition, or function declaration */
            struct ctype type;
            struct token name;
            struct parser asmname;

            memset(&name, 0, sizeof(name));
            memset(&asmname, 0, sizeof(asmname));

            put_back(P);
            parse_type(L, P, &type);

            for (;;) {
                parse_argument(L, P, -1, &type, &name, &asmname);

                if (name.size) {
                    /* global/function declaration */

                    /* set asmname_tbl[name] = asmname */
                    if (asmname.next) {
                        push_upval(L, &asmname_key);
                        lua_pushlstring(L, name.str, name.size);
                        push_strings(L, &asmname);
                        lua_rawset(L, -3);
                        lua_pop(L, 1); /* asmname upval */
                    }

                    push_upval(L, &functions_key);
                    lua_pushlstring(L, name.str, name.size);
                    push_ctype(L, -3, &type);
                    lua_rawset(L, -3);
                    lua_pop(L, 1); /* functions upval */
                } else {
                    /* type declaration/definition - already been processed */
                }

                lua_pop(L, 1);

                require_token(L, P, &tok);

                if (tok.type == TOK_SEMICOLON) {
                    break;
                } else if (tok.type != TOK_COMMA) {
                    luaL_error(L, "missing semicolon on line %d", P->line);
                }
            }

            lua_pop(L, 1);
        }
    }

    return END;
}

int ffi_cdef(lua_State* L)
{
    struct parser P;

    P.line = 1;
    P.prev = P.next = luaL_checkstring(L, 1);
    P.align_mask = DEFAULT_ALIGN_MASK;

    if (parse_root(L, &P) == PRAGMA_POP) {
        luaL_error(L, "pragma pop without an associated push on line %d", P.line);
    }

    return 0;
}

/* calculate_constant handles operator precedence by having a number of
 * recursive commands each of which computes the result at that level of
 * precedence and above. calculate_constant1 is the highest precedence
 */

static int try_cast(lua_State* L)
{
    struct parser* P = (struct parser*) lua_touserdata(L, 1);
    struct ctype ct;
    struct token name, tok;
    memset(&name, 0, sizeof(name));

    parse_type(L, P, &ct);
    parse_argument(L, P, -1, &ct, &name, NULL);

    require_token(L, P, &tok);
    if (tok.type != TOK_CLOSE_PAREN || name.size) {
        return luaL_error(L, "invalid cast");
    }

    if (ct.pointers || ct.type != INT32_TYPE) {
        return luaL_error(L, "unsupported cast on line %d", P->line);
    }

    return 0;
}

static int64_t calculate_constant2(lua_State* L, struct parser* P, struct token* tok);

/* () */
static int64_t calculate_constant1(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t ret;

    if (tok->type == TOK_NUMBER) {
        ret = tok->integer;
        next_token(L, P, tok);
        return ret;

    } else if (tok->type == TOK_TOKEN) {
        /* look up name in constants table */
        push_upval(L, &constants_key);
        lua_pushlstring(L, tok->str, tok->size);
        lua_rawget(L, -2);
        lua_remove(L, -2); /* constants table */

        if (!lua_isnumber(L, -1)) {
            lua_pushlstring(L, tok->str, tok->size);
            luaL_error(L, "use of undefined constant %s on line %d", lua_tostring(L, -1), P->line);
        }

        ret = (int64_t) lua_tonumber(L, -1);
        lua_pop(L, 1);
        next_token(L, P, tok);
        return ret;

    } else if (tok->type == TOK_OPEN_PAREN) {
        struct parser before_cast = *P;
        int top = lua_gettop(L);

        /* see if this is a numeric cast, which we ignore */
        lua_pushcfunction(L, &try_cast);
        lua_pushlightuserdata(L, P);
        if (!lua_pcall(L, 1, 0, 0)) {
            next_token(L, P, tok);
            return calculate_constant2(L, P, tok);
        }
        lua_settop(L, top);

        *P = before_cast;
        ret = calculate_constant(L, P);

        require_token(L, P, tok);
        if (tok->type != TOK_CLOSE_PAREN) {
            luaL_error(L, "error whilst parsing constant at line %d", P->line);
        }

        next_token(L, P, tok);
        return ret;

    } else {
        return luaL_error(L, "unexpected token whilst parsing constant at line %d", P->line);
    }
}

/* ! and ~, unary + and -, and sizeof */
static int64_t calculate_constant2(lua_State* L, struct parser* P, struct token* tok)
{
    if (tok->type == TOK_LOGICAL_NOT) {
        require_token(L, P, tok);
        return !calculate_constant2(L, P, tok);

    } else if (tok->type == TOK_BITWISE_NOT) {
        require_token(L, P, tok);
        return ~calculate_constant2(L, P, tok);

    } else if (tok->type == TOK_PLUS) {
        require_token(L, P, tok);
        return calculate_constant2(L, P, tok);

    } else if (tok->type == TOK_MINUS) {
        require_token(L, P, tok);
        return -calculate_constant2(L, P, tok);

    } else if (tok->type == TOK_TOKEN &&
            (IS_LITERAL(*tok, "sizeof")
             || IS_LITERAL(*tok, "alignof")
             || IS_LITERAL(*tok, "__alignof__")
             || IS_LITERAL(*tok, "__alignof"))) {

        bool issize = IS_LITERAL(*tok, "sizeof");
        struct ctype type;

        require_token(L, P, tok);
        if (tok->type != TOK_OPEN_PAREN) {
            luaL_error(L, "invalid sizeof at line %d", P->line);
        }

        parse_type(L, P, &type);
        parse_argument(L, P, -1, &type, NULL, NULL);
        lua_pop(L, 2);

        require_token(L, P, tok);
        if (tok->type != TOK_CLOSE_PAREN) {
            luaL_error(L, "invalid sizeof at line %d", P->line);
        }

        next_token(L, P, tok);

        return issize ? ctype_size(L, &type) : type.align_mask + 1;

    } else {
        return calculate_constant1(L, P, tok);
    }
}

/* binary * / and % (left associative) */
static int64_t calculate_constant3(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t left = calculate_constant2(L, P, tok);

    for (;;) {
        if (tok->type == TOK_MULTIPLY) {
            require_token(L, P, tok);
            left *= calculate_constant2(L, P, tok);

        } else if (tok->type == TOK_DIVIDE) {
            require_token(L, P, tok);
            left /= calculate_constant2(L, P, tok);

        } else if (tok->type == TOK_MODULUS) {
            require_token(L, P, tok);
            left %= calculate_constant2(L, P, tok);

        } else {
            return left;
        }
    }
}

/* binary + and - (left associative) */
static int64_t calculate_constant4(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t left = calculate_constant3(L, P, tok);

    for (;;) {
        if (tok->type == TOK_PLUS) {
            require_token(L, P, tok);
            left += calculate_constant3(L, P, tok);

        } else if (tok->type == TOK_MINUS) {
            require_token(L, P, tok);
            left -= calculate_constant3(L, P, tok);

        } else {
            return left;
        }
    }
}

/* binary << and >> (left associative) */
static int64_t calculate_constant5(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t left = calculate_constant4(L, P, tok);

    for (;;) {
        if (tok->type == TOK_LEFT_SHIFT) {
            require_token(L, P, tok);
            left <<= calculate_constant4(L, P, tok);

        } else if (tok->type == TOK_RIGHT_SHIFT) {
            require_token(L, P, tok);
            left >>= calculate_constant4(L, P, tok);

        } else {
            return left;
        }
    }
}

/* binary <, <=, >, and >= (left associative) */
static int64_t calculate_constant6(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t left = calculate_constant5(L, P, tok);

    for (;;) {
        if (tok->type == TOK_LESS) {
            require_token(L, P, tok);
            left = (left < calculate_constant5(L, P, tok));

        } else if (tok->type == TOK_LESS_EQUAL) {
            require_token(L, P, tok);
            left = (left <= calculate_constant5(L, P, tok));

        } else if (tok->type == TOK_GREATER) {
            require_token(L, P, tok);
            left = (left > calculate_constant5(L, P, tok));

        } else if (tok->type == TOK_GREATER_EQUAL) {
            require_token(L, P, tok);
            left = (left >= calculate_constant5(L, P, tok));

        } else {
            return left;
        }
    }
}

/* binary ==, != (left associative) */
static int64_t calculate_constant7(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t left = calculate_constant6(L, P, tok);

    for (;;) {
        if (tok->type == TOK_EQUAL) {
            require_token(L, P, tok);
            left = (left == calculate_constant6(L, P, tok));

        } else if (tok->type == TOK_NOT_EQUAL) {
            require_token(L, P, tok);
            left = (left != calculate_constant6(L, P, tok));

        } else {
            return left;
        }
    }
}

/* binary & (left associative) */
static int64_t calculate_constant8(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t left = calculate_constant7(L, P, tok);

    for (;;) {
        if (tok->type == TOK_BITWISE_AND) {
            require_token(L, P, tok);
            left = (left & calculate_constant7(L, P, tok));

        } else {
            return left;
        }
    }
}

/* binary ^ (left associative) */
static int64_t calculate_constant9(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t left = calculate_constant8(L, P, tok);

    for (;;) {
        if (tok->type == TOK_BITWISE_XOR) {
            require_token(L, P, tok);
            left = (left ^ calculate_constant8(L, P, tok));

        } else {
            return left;
        }
    }
}

/* binary | (left associative) */
static int64_t calculate_constant10(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t left = calculate_constant9(L, P, tok);

    for (;;) {
        if (tok->type == TOK_BITWISE_OR) {
            require_token(L, P, tok);
            left = (left | calculate_constant9(L, P, tok));

        } else {
            return left;
        }
    }
}

/* binary && (left associative) */
static int64_t calculate_constant11(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t left = calculate_constant10(L, P, tok);

    for (;;) {
        if (tok->type == TOK_LOGICAL_AND) {
            require_token(L, P, tok);
            left = (left && calculate_constant10(L, P, tok));

        } else {
            return left;
        }
    }
}

/* binary || (left associative) */
static int64_t calculate_constant12(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t left = calculate_constant11(L, P, tok);

    for (;;) {
        if (tok->type == TOK_LOGICAL_OR) {
            require_token(L, P, tok);
            left = (left || calculate_constant11(L, P, tok));

        } else {
            return left;
        }
    }
}

/* ternary ?: (right associative) */
static int64_t calculate_constant13(lua_State* L, struct parser* P, struct token* tok)
{
    int64_t left = calculate_constant12(L, P, tok);

    if (tok->type == TOK_QUESTION) {
        int64_t middle, right;
        require_token(L, P, tok);
        middle = calculate_constant13(L, P, tok);
        if (tok->type != TOK_COLON) {
            luaL_error(L, "invalid ternery (? :) in constant on line %d", P->line);
        }
        require_token(L, P, tok);
        right = calculate_constant13(L, P, tok);
        return left ? middle : right;

    } else {
        return left;
    }
}

int64_t calculate_constant(lua_State* L, struct parser* P)
{
    struct token tok;
    int64_t ret;
    require_token(L, P, &tok);
    ret = calculate_constant13(L, P, &tok);

    if (tok.type != TOK_NIL) {
        put_back(P);
    }

    return ret;
}




/* vim: ts=4 sw=4 sts=4 et tw=78
 * Copyright (c) 2011 James R. McKaskill. See license in ffi.h
 */
//#include "ffi.h"
#include <math.h>
#include <inttypes.h>

/* Set to 1 to get extra debugging on print */
#define DEBUG_TOSTRING 0

int jit_key;
int ctype_mt_key;
int cdata_mt_key;
int callback_mt_key;
int cmodule_mt_key;
int constants_key;
int types_key;
int gc_key;
int callbacks_key;
int functions_key;
int abi_key;
int next_unnamed_key;
int niluv_key;
int asmname_key;

void push_upval(lua_State* L, int* key)
{
    lua_pushlightuserdata(L, key);
    lua_rawget(L, LUA_REGISTRYINDEX);
}

void set_upval(lua_State* L, int* key)
{
    lua_pushlightuserdata(L, key);
    lua_insert(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

int equals_upval(lua_State* L, int idx, int* key)
{
    int ret;
    lua_pushvalue(L, idx);
    push_upval(L, key);
    ret = lua_rawequal(L, -2, -1);
    lua_pop(L, 2);
    return ret;
}

struct jit* get_jit(lua_State* L)
{
    struct jit* jit;
    push_upval(L, &jit_key);
    jit = (struct jit*) lua_touserdata(L, -1);
    jit->L = L;
    lua_pop(L, 1); /* still in registry */
    return jit;
}

static int type_error(lua_State* L, int idx, const char* to_type, int to_usr, const struct ctype* to_ct)
{
    luaL_Buffer B;
    struct ctype ft;

    assert(to_type || (to_usr && to_ct));
    if (to_usr) {
        to_usr = lua_absindex(L, to_usr);
    }

    idx = lua_absindex(L, idx);

    luaL_buffinit(L, &B);
    to_cdata(L, idx, &ft);

    if (ft.type != INVALID_TYPE) {
        push_type_name(L, -1, &ft);
        lua_pushfstring(L, "unable to convert argument %d from cdata<%s> to cdata<", idx, lua_tostring(L, -1));
        lua_remove(L, -2);
        luaL_addvalue(&B);
    } else {
        lua_pushfstring(L, "unable to convert argument %d from lua<%s> to cdata<", idx, luaL_typename(L, idx));
        luaL_addvalue(&B);
    }

    if (to_ct) {
        push_type_name(L, to_usr, to_ct);
        luaL_addvalue(&B);
    } else {
        luaL_addstring(&B, to_type);
    }

    luaL_addchar(&B, '>');

    luaL_pushresult(&B);
    return lua_error(L);
}

static int64_t check_intptr(lua_State* L, int idx, void* p, struct ctype* ct)
{
    if (ct->type == INVALID_TYPE) {
        int64_t ret;
        memset(ct, 0, sizeof(*ct));
        ct->base_size = 8;
        ct->type = INT64_TYPE;
        ct->is_defined = 1;
        ret = luaL_checknumber(L, idx);
        return ret;

    } else if (ct->pointers) {
        return (intptr_t) p;
    }

    switch (ct->type) {
    case INTPTR_TYPE:
    case FUNCTION_PTR_TYPE:
        return *(intptr_t*) p;

    case INT64_TYPE:
        return *(int64_t*) p;

    case INT32_TYPE:
        return ct->is_unsigned ? (int64_t) *(uint32_t*) p : (int64_t) *(int32_t*) p;

    case INT16_TYPE:
        return ct->is_unsigned ? (int64_t) *(uint16_t*) p : (int64_t) *(int16_t*) p;

    case INT8_TYPE:
        return ct->is_unsigned ? (int64_t) *(uint8_t*) p : (int64_t) *(int8_t*) p;

    default:
        type_error(L, idx, "intptr_t", 0, NULL);
        return 0;
    }
}

#define TO_NUMBER(TYPE, ALLOW_POINTERS)                                     \
    TYPE real = 0, imag = 0;                                                \
    void* p;                                                                \
    struct ctype ct;                                                        \
                                                                            \
    switch (lua_type(L, idx)) {                                             \
    case LUA_TBOOLEAN:                                                      \
        real = (TYPE) lua_toboolean(L, idx);                                \
        break;                                                              \
                                                                            \
    case LUA_TNUMBER:                                                       \
        real = (TYPE) lua_tonumber(L, idx);                                 \
        break;                                                              \
                                                                            \
    case LUA_TSTRING:                                                       \
        if (!ALLOW_POINTERS) {                                              \
            type_error(L, idx, #TYPE, 0, NULL);                             \
        }                                                                   \
        real = (TYPE) (intptr_t) lua_tostring(L, idx);                      \
        break;                                                              \
                                                                            \
    case LUA_TLIGHTUSERDATA:                                                \
        if (!ALLOW_POINTERS) {                                              \
            type_error(L, idx, #TYPE, 0, NULL);                             \
        }                                                                   \
        real = (TYPE) (intptr_t) lua_topointer(L, idx);                     \
        break;                                                              \
                                                                            \
    case LUA_TUSERDATA:                                                     \
        p = to_cdata(L, idx, &ct);                                          \
                                                                            \
        if (ct.type == INVALID_TYPE) {                                      \
            if (!ALLOW_POINTERS) {                                          \
                type_error(L, idx, #TYPE, 0, NULL);                         \
            }                                                               \
            real = (TYPE) (intptr_t) p;                                     \
        } else if (ct.pointers || ct.type == STRUCT_TYPE || ct.type == UNION_TYPE) {\
            if (!ALLOW_POINTERS) {                                          \
                type_error(L, idx, #TYPE, 0, NULL);                         \
            }                                                               \
            real = (TYPE) (intptr_t) p;                                     \
        } else if (ct.type == COMPLEX_DOUBLE_TYPE) {                        \
            real = (TYPE) creal(*(complex_double*) p);                      \
            imag = (TYPE) cimag(*(complex_double*) p);                      \
        } else if (ct.type == COMPLEX_FLOAT_TYPE) {                         \
            real = (TYPE) crealf(*(complex_float*) p);                      \
            imag = (TYPE) cimagf(*(complex_float*) p);                      \
        } else if (ct.type == DOUBLE_TYPE) {                                \
            real = (TYPE) *(double*) p;                                     \
        } else if (ct.type == FLOAT_TYPE) {                                 \
            real = (TYPE) *(float*) p;                                      \
        } else {                                                            \
            real = check_intptr(L, idx, p, &ct);                            \
        }                                                                   \
        lua_pop(L, 1);                                                      \
        break;                                                              \
                                                                            \
    case LUA_TNIL:                                                          \
        real = (TYPE) 0;                                                    \
        break;                                                              \
                                                                            \
    default:                                                                \
        type_error(L, idx, #TYPE, 0, NULL);                                 \
    }                                                                       \

static int64_t cast_int64(lua_State* L, int idx, int is_cast)
{ TO_NUMBER(int64_t, is_cast); (void) imag; return real; }

static uint64_t cast_uint64(lua_State* L, int idx, int is_cast)
{ TO_NUMBER(uint64_t, is_cast); (void) imag; return real; }

int32_t check_int32(lua_State* L, int idx)
{ return (int32_t) cast_int64(L, idx, 0); }

uint32_t check_uint32(lua_State* L, int idx)
{ return (uint32_t) cast_uint64(L, idx, 0); }

int64_t check_int64(lua_State* L, int idx)
{ return cast_int64(L, idx, 0); }

uint64_t check_uint64(lua_State* L, int idx)
{ return cast_uint64(L, idx, 0); }

static void do_check_double(lua_State* L, int idx, double* preal, double* pimag)
{
    TO_NUMBER(double, 0);
    if (preal) *preal = real;
    if (pimag) *pimag = imag;
}

double check_double(lua_State* L, int idx)
{ double ret; do_check_double(L, idx, &ret, NULL); return ret; }

float check_float(lua_State* L, int idx)
{ double ret; do_check_double(L, idx, &ret, NULL); return ret; }

uintptr_t check_uintptr(lua_State* L, int idx)
{ TO_NUMBER(uintptr_t, 1); (void) imag; return real; }

#ifdef HAVE_COMPLEX
complex_double check_complex_double(lua_State* L, int idx)
{ double real, imag; do_check_double(L, idx, &real, &imag); return real + imag * 1i; }

complex_float check_complex_float(lua_State* L, int idx)
{ double real, imag; do_check_double(L, idx, &real, &imag); return real + imag * 1i; }

#else
complex_double check_complex_double(lua_State* L, int idx)
{
    complex_double c;
    do_check_double(L, idx, &c.real, &c.imag);
    return c;
}

complex_float check_complex_float(lua_State* L, int idx)
{
    complex_double d;
    complex_float f;
    do_check_double(L, idx, &d.real, &d.imag);
    f.real = d.real;
    f.imag = d.imag;
    return f;
}
#endif

static size_t unpack_vararg(lua_State* L, int i, char* to)
{
    void* p;
    struct ctype ct;

    switch (lua_type(L, i)) {
    case LUA_TBOOLEAN:
        *(int*) to = lua_toboolean(L, i);
        return sizeof(int);

    case LUA_TNUMBER:
        *(double*) to = lua_tonumber(L, i);
        return sizeof(double);

    case LUA_TSTRING:
        *(const char**) to = lua_tostring(L, i);
        return sizeof(const char*);

    case LUA_TLIGHTUSERDATA:
        *(void**) to = lua_touserdata(L, i);
        return sizeof(void*);

    case LUA_TUSERDATA:
        p = to_cdata(L, i, &ct);

        if (ct.type == INVALID_TYPE) {
            *(void**) to = p;
            return sizeof(void*);
        }

        lua_pop(L, 1);

        if (ct.pointers || ct.type == INTPTR_TYPE) {
            *(void**) to = p;
            return sizeof(void*);

        } else if (ct.type == INT32_TYPE) {
            *(int32_t*) to = *(int32_t*) p;
            return sizeof(int32_t);

        } else if (ct.type == INT64_TYPE) {
            *(int64_t*) to = *(int64_t*) p;
            return sizeof(int64_t);
        }
        goto err;

    case LUA_TNIL:
        *(void**) to = NULL;
        return sizeof(void*);

    default:
        goto err;
    }

err:
    return type_error(L, i, "vararg", 0, NULL);
}

void unpack_varargs_stack(lua_State* L, int first, int last, char* to)
{
    int i;

    for (i = first; i <= last; i++) {
        to += unpack_vararg(L, i, to);
    }
}

void unpack_varargs_stack_skip(lua_State* L, int first, int last, int ints_to_skip, int floats_to_skip, char* to)
{
    int i;

    for (i = first; i <= last; i++) {
        int type = lua_type(L, i);

        if (type == LUA_TNUMBER && --floats_to_skip >= 0) {
            continue;
        } else if (type != LUA_TNUMBER && --ints_to_skip >= 0) {
            continue;
        }

        to += unpack_vararg(L, i, to);
    }
}

void unpack_varargs_float(lua_State* L, int first, int last, int max, char* to)
{
    int i;

    for (i = first; i <= last && max > 0; i++) {
        if (lua_type(L, i) == LUA_TNUMBER) {
            unpack_vararg(L, i, to);
            to += sizeof(double);
            max--;
        }
    }
}

void unpack_varargs_int(lua_State* L, int first, int last, int max, char* to)
{
    int i;

    for (i = first; i <= last && max > 0; i++) {
        if (lua_type(L, i) != LUA_TNUMBER) {
            unpack_vararg(L, i, to);
            to += sizeof(void*);
            max--;
        }
    }
}

void unpack_varargs_reg(lua_State* L, int first, int last, char* to)
{
    int i;

    for (i = first; i <= last; i++) {
        unpack_vararg(L, i, to);
        to += sizeof(double);
    }
}

/* to_enum tries to convert a value at idx to the enum type indicated by to_ct
 * and uv to_usr. For strings this means it will do a string lookup for the
 * enum type. It leaves the stack unchanged. Will throw an error if the type
 * at idx can't be conerted.
 */
int32_t check_enum(lua_State* L, int idx, int to_usr, const struct ctype* to_ct)
{
    int32_t ret;

    switch (lua_type(L, idx)) {
    case LUA_TSTRING:
        /* lookup string in to_usr to find value */
        to_usr = lua_absindex(L, to_usr);
        lua_pushvalue(L, idx);
        lua_rawget(L, to_usr);

        if (lua_isnil(L, -1)) {
            goto err;
        }

        ret = (int32_t) lua_tointeger(L, -1);
        lua_pop(L, 1);
        return ret;

    case LUA_TUSERDATA:
        return check_int32(L, idx);

    case LUA_TNIL:
        return (int32_t) 0;

    case LUA_TNUMBER:
        return (int32_t) lua_tointeger(L, idx);

    default:
        goto err;
    }

err:
    return type_error(L, idx, NULL, to_usr, to_ct);
}

/* to_pointer tries converts a value at idx to a pointer. It fills out ct and
 * pushes the uv of the found type. It will throw a lua error if it can not
 * convert the value to a pointer. */
static void* check_pointer(lua_State* L, int idx, struct ctype* ct)
{
    void* p;
    memset(ct, 0, sizeof(*ct));
    ct->pointers = 1;
    idx = lua_absindex(L, idx);

    switch (lua_type(L, idx)) {
    case LUA_TNIL:
        ct->type = VOID_TYPE;
        ct->is_null = 1;
        lua_pushnil(L);
        return NULL;

    case LUA_TNUMBER:
        ct->type = INTPTR_TYPE;
        ct->is_unsigned = 1;
        ct->pointers = 0;
        lua_pushnil(L);
        return (void*) (uintptr_t) lua_tonumber(L, idx);

    case LUA_TLIGHTUSERDATA:
        ct->type = VOID_TYPE;
        lua_pushnil(L);
        return lua_touserdata(L, idx);

    case LUA_TSTRING:
        ct->type = INT8_TYPE;
        ct->is_unsigned = IS_CHAR_UNSIGNED;
        ct->is_array = 1;
        ct->base_size = 1;
        ct->const_mask = 2;
        lua_pushnil(L);
        return (void*) lua_tolstring(L, idx, &ct->array_size);

    case LUA_TUSERDATA:
        p = to_cdata(L, idx, ct);

        if (ct->type == INVALID_TYPE) {
            /* some other type of user data */
            ct->type = VOID_TYPE;
            return lua_touserdata(L, idx);
        } else if (ct->type == STRUCT_TYPE || ct->type == UNION_TYPE) {
            return p;
        } else {
            return (void*) (intptr_t) check_intptr(L, idx, p, ct);
        }
        break;
    }

    type_error(L, idx, "pointer", 0, NULL);
    return NULL;
}

static int is_void_ptr(const struct ctype* ct)
{
    return ct->type == VOID_TYPE
        && ct->pointers == 1;
}

static int is_same_type(lua_State* L, int usr1, int usr2, const struct ctype* t1, const struct ctype* t2)
{
    if (t1->type != t2->type) {
        return 0;
    }

#if LUA_VERSION_NUM == 501
    if (lua_isnil(L, usr1) != lua_isnil(L, usr2)) {
        int ret;
        usr1 = lua_absindex(L, usr1);
        usr2 = lua_absindex(L, usr2);
        push_upval(L, &niluv_key);

        ret = lua_rawequal(L, usr1, -1)
            || lua_rawequal(L, usr2, -1);

        lua_pop(L, 1);

        if (ret) {
            return 1;
        }
    }
#endif

    return lua_rawequal(L, usr1, usr2);
}

static void set_struct(lua_State* L, int idx, void* to, int to_usr, const struct ctype* tt, int check_pointers);

/* to_typed_pointer converts a value at idx to a type tt with target uv to_usr
 * checking all types. May push a temporary value so that it can create
 * structs on the fly. */
void* check_typed_pointer(lua_State* L, int idx, int to_usr, const struct ctype* tt)
{
    struct ctype ft;
    void* p;

    to_usr = lua_absindex(L, to_usr);
    idx = lua_absindex(L, idx);

    if (tt->pointers == 1 && (tt->type == STRUCT_TYPE || tt->type == UNION_TYPE) && lua_type(L, idx) == LUA_TTABLE) {
        /* need to construct a struct of the target type */
        struct ctype ct = *tt;
        ct.pointers = ct.is_array = 0;
        p = push_cdata(L, to_usr, &ct);
        set_struct(L, idx, p, to_usr, &ct, 1);
        return p;
    }

    p = check_pointer(L, idx, &ft);

    if (tt->pointers == 1 && ft.pointers == 0 && (ft.type == STRUCT_TYPE || ft.type == UNION_TYPE)) {
        /* auto dereference structs */
        ft.pointers = 1;
        ft.const_mask <<= 1;
    }

    if (is_void_ptr(tt)) {
        /* any pointer can convert to void* */
        goto suc;

    } else if (ft.is_null) {
        /* NULL can convert to any pointer */
        goto suc;

    } else if (!is_same_type(L, to_usr, -1, tt, &ft)) {
        /* the base type is different */
        goto err;

    } else if (tt->pointers != ft.pointers) {
        goto err;

    } else if (ft.const_mask & ~tt->const_mask) {
        /* for every const in from it must be in to, there are further rules
         * for const casting (see the c++ spec), but they are hard to test
         * quickly */
        goto err;
    }

suc:
    return p;

err:
    type_error(L, idx, NULL, to_usr, tt);
    return NULL;
}

/* to_cfunction converts a value at idx with usr table at to_usr and type tt
 * into a function. Leaves the stack unchanged. */
static cfunction check_cfunction(lua_State* L, int idx, int to_usr, const struct ctype* tt, int check_pointers)
{
    void* p;
    struct ctype ft;
    cfunction f;
    int top = lua_gettop(L);

    idx = lua_absindex(L, idx);
    to_usr = lua_absindex(L, to_usr);

    switch (lua_type(L, idx)) {
    case LUA_TFUNCTION:
        /* Function cdatas are pinned and must be manually cleaned up by
         * calling func:free(). */
        push_upval(L, &callbacks_key);
        f = compile_callback(L, idx, to_usr, tt);
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
        lua_pop(L, 1); /* callbacks tbl */
        return f;

    case LUA_TNIL:
        return NULL;

    case LUA_TLIGHTUSERDATA:
        if (check_pointers) {
            goto err;
        } else {
            return (cfunction) lua_touserdata(L, idx);
        }

    case LUA_TUSERDATA:
        p = to_cdata(L, idx, &ft);
        assert(lua_gettop(L) == top + 1);

        if (ft.type == INVALID_TYPE) {
            if (check_pointers) {
                goto err;
            } else {
                lua_pop(L, 1);
                return (cfunction) lua_touserdata(L, idx);
            }

        } else if (ft.is_null) {
            lua_pop(L, 1);
            return NULL;

        } else if (!check_pointers && (ft.pointers || ft.type == INTPTR_TYPE)) {
            lua_pop(L, 1);
            return (cfunction) *(void**) p;

        } else if (ft.type != FUNCTION_PTR_TYPE) {
            goto err;

        } else if (!check_pointers) {
            lua_pop(L, 1);
            return *(cfunction*) p;

        } else if (ft.calling_convention != tt->calling_convention) {
            goto err;

        } else if (!is_same_type(L, -1, to_usr, &ft, tt)) {
            goto err;

        } else {
            lua_pop(L, 1);
            return *(cfunction*) p;
        }

    default:
        goto err;
    }

err:
    type_error(L, idx, NULL, to_usr, tt);
    return NULL;
}

/* to_type_cfunction converts a value at idx with uv at to_usr and type tt to
 * a cfunction. Leaves the stack unchanged. */
cfunction check_typed_cfunction(lua_State* L, int idx, int to_usr, const struct ctype* tt)
{ return check_cfunction(L, idx, to_usr, tt, 1); }

static void set_value(lua_State* L, int idx, void* to, int to_usr, const struct ctype* tt, int check_pointers);

static void set_array(lua_State* L, int idx, void* to, int to_usr, const struct ctype* tt, int check_pointers)
{
    size_t i, sz, esz;
    struct ctype et;

    idx = lua_absindex(L, idx);
    to_usr = lua_absindex(L, to_usr);

    switch (lua_type(L, idx)) {
    case LUA_TSTRING:
        if (tt->pointers == 1 && tt->type == INT8_TYPE) {
            const char* str = lua_tolstring(L, idx, &sz);

            if (!tt->is_variable_array && sz >= tt->array_size) {
                memcpy(to, str, tt->array_size);
            } else {
                /* include nul terminator */
                memcpy(to, str, sz+1);
            }
        } else {
            goto err;
        }
        break;

    case LUA_TTABLE:
        et = *tt;
        et.pointers--;
        et.const_mask >>= 1;
        et.is_array = 0;
        esz = et.pointers ? sizeof(void*) : et.base_size;

        lua_rawgeti(L, idx, 2);

        if (tt->is_variable_array) {
            /* we have no idea how big the array is, so set values based off
             * how many items were given to us */
            lua_pop(L, 1);
            for (i = 0; i < lua_rawlen(L, idx); i++) {
                lua_rawgeti(L, idx, (int) i + 1);
                set_value(L, -1, (char*) to + esz * i, to_usr, &et, check_pointers);
                lua_pop(L, 1);
            }

        } else if (lua_isnil(L, -1)) {
            /* there is no second element, so we set the whole array to the
             * first element (or nil - ie 0) if there is no first element) */
            lua_pop(L, 1);
            lua_rawgeti(L, idx, 1);

            if (lua_isnil(L, -1)) {
                memset(to, 0, ctype_size(L, tt));
            } else {
                /* if its still variable we have no idea how many values to set */
                for (i = 0; i < tt->array_size; i++) {
                    set_value(L, -1, (char*) to + esz * i, to_usr, &et, check_pointers);
                }
            }

            lua_pop(L, 1);

        } else {
            /* there is a second element, so we set each element using the
             * equiv index in the table initializer */
            lua_pop(L, 1);
            for (i = 0; i < tt->array_size; i++) {
                lua_rawgeti(L, idx, (int) (i+1));

                if (lua_isnil(L, -1)) {
                    /* we've hit the end of the values provided in the
                     * initializer, so memset the rest to zero */
                    lua_pop(L, 1);
                    memset((char*) to + esz * i, 0, (tt->array_size - i) * esz);
                    break;

                } else {
                    set_value(L, -1, (char*) to + esz * i, to_usr, &et, check_pointers);
                    lua_pop(L, 1);
                }
            }
        }
        break;

    default:
        goto err;
    }

    return;

err:
    type_error(L, idx, NULL, to_usr, tt);
}

/* pops the member key from the stack, leaves the member user value on the
 * stack. Returns the member offset. Returns -ve if the member can not be
 * found. */
static ptrdiff_t get_member(lua_State* L, int usr, const struct ctype* ct, struct ctype* mt)
{
    ptrdiff_t off;
    lua_rawget(L, usr);

    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return -1;
    }

    *mt = *(const struct ctype*) lua_touserdata(L, -1);
    lua_getuservalue(L, -1);
    lua_replace(L, -2);

    if (mt->is_variable_array && ct->variable_size_known) {
        /* eg char mbr[?] */
        size_t sz = (mt->pointers > 1) ? sizeof(void*) : mt->base_size;
        assert(ct->is_variable_struct && mt->is_array);
        mt->array_size = ct->variable_increment / sz;
        mt->is_variable_array = 0;

    } else if (mt->is_variable_struct && ct->variable_size_known) {
        /* eg struct {char a; char b[?]} mbr; */
        assert(ct->is_variable_struct);
        mt->variable_size_known = 1;
        mt->variable_increment = ct->variable_increment;
    }

    off = mt->offset;
    mt->offset = 0;
    return off;
}

static void set_struct(lua_State* L, int idx, void* to, int to_usr, const struct ctype* tt, int check_pointers)
{
    int have_first = 0;
    int have_other = 0;
    struct ctype mt;
    void* p;

    to_usr = lua_absindex(L, to_usr);
    idx = lua_absindex(L, idx);

    switch (lua_type(L, idx)) {
    case LUA_TTABLE:
        /* match up to the members based off the table initializers key - this
         * will match both numbered and named members in the user table
         * we need a special case for when no entries in the initializer -
         * zero initialize the c struct, and only one entry in the initializer
         * - set all members to this value */
        memset(to, 0, ctype_size(L, tt));
        lua_pushnil(L);
        while (lua_next(L, idx)) {
            ptrdiff_t off;

            if (!have_first && lua_tonumber(L, -2) == 1 && lua_tonumber(L, -1) != 0) {
                have_first = 1;
            } else if (!have_other && (lua_type(L, -2) != LUA_TNUMBER || lua_tonumber(L, -2) != 1)) {
                have_other = 1;
            }

            lua_pushvalue(L, -2);
            off = get_member(L, to_usr, tt, &mt);
            assert(off >= 0);
            set_value(L, -2, (char*) to + off, -1, &mt, check_pointers);

            /* initializer value, mt usr */
            lua_pop(L, 2);
        }

        /* if we only had a single non zero value then initialize all members to that value */
        if (!have_other && have_first && tt->type != UNION_TYPE) {
            size_t i, sz;
            ptrdiff_t off;
            lua_rawgeti(L, idx, 1);
            sz = lua_rawlen(L, to_usr);

            for (i = 2; i < sz; i++) {
                lua_pushnumber(L, i);
                off = get_member(L, to_usr, tt, &mt);
                assert(off >= 0);
                set_value(L, -2, (char*) to + off, -1, &mt, check_pointers);
                lua_pop(L, 1); /* mt usr */
            }

            lua_pop(L, 1); /* initializer table */
        }
        break;

    case LUA_TUSERDATA:
        if (check_pointers) {
            p = check_typed_pointer(L, idx, to_usr, tt);
        } else {
            struct ctype ct;
            p = check_pointer(L, idx, &ct);
        }
        memcpy(to, p, tt->base_size);
        lua_pop(L, 1);
        break;

    default:
        goto err;
    }

    return;

err:
    type_error(L, idx, NULL, to_usr, tt);
}

static void set_value(lua_State* L, int idx, void* to, int to_usr, const struct ctype* tt, int check_pointers)
{
    int top = lua_gettop(L);

    if (tt->is_array) {
        set_array(L, idx, to, to_usr, tt, check_pointers);

    } else if (tt->pointers) {
        union {
            uint8_t c[sizeof(void*)];
            void* p;
        } u;

        if (lua_istable(L, idx)) {
            luaL_error(L, "Can't set a pointer member to a struct that's about to be freed");
        }

        if (check_pointers) {
            u.p = check_typed_pointer(L, idx, to_usr, tt);
        } else {
            struct ctype ct;
            u.p = check_pointer(L, idx, &ct);
        }

#ifndef ALLOW_MISALIGNED_ACCESS
        if ((uintptr_t) to & PTR_ALIGN_MASK) {
            memcpy(to, u.c, sizeof(void*));
        } else
#endif
        {
            *(void**) to = u.p;
        }

        lua_pop(L, 1);

    } else if (tt->is_bitfield) {

        uint64_t hi_mask = UINT64_C(0) - (UINT64_C(1) << (tt->bit_offset + tt->bit_size));
        uint64_t low_mask = (UINT64_C(1) << tt->bit_offset) - UINT64_C(1);
        uint64_t val = check_uint64(L, idx);
        val &= (UINT64_C(1) << tt->bit_size) - 1;
        val <<= tt->bit_offset;
        *(uint64_t*) to = val | (*(uint64_t*) to & (hi_mask | low_mask));

    } else if (tt->type == STRUCT_TYPE || tt->type == UNION_TYPE) {
        set_struct(L, idx, to, to_usr, tt, check_pointers);

    } else {

#ifndef ALLOW_MISALIGNED_ACCESS
        union {
            uint8_t c[8];
            _Bool b;
            uint64_t u64;
            float f;
            double d;
            cfunction func;
        } misalign;

        void* origto = to;

        if ((uintptr_t) origto & (tt->base_size - 1)) {
            to = misalign.c;
        }
#endif

        switch (tt->type) {
        case BOOL_TYPE:
            *(_Bool*) to = (cast_int64(L, idx, !check_pointers) != 0);
            break;
        case INT8_TYPE:
            if (tt->is_unsigned) {
                *(uint8_t*) to = (uint8_t) cast_uint64(L, idx, !check_pointers);
            } else {
                *(int8_t*) to = (int8_t) cast_int64(L, idx, !check_pointers);
            }
            break;
        case INT16_TYPE:
            if (tt->is_unsigned) {
                *(uint16_t*) to = (uint16_t) cast_uint64(L, idx, !check_pointers);
            } else {
                *(int16_t*) to = (int16_t) cast_int64(L, idx, !check_pointers);
            }
            break;
        case INT32_TYPE:
            if (tt->is_unsigned) {
                *(uint32_t*) to = (uint32_t) cast_uint64(L, idx, !check_pointers);
            } else {
                *(int32_t*) to = (int32_t) cast_int64(L, idx, !check_pointers);
            }
            break;
        case INT64_TYPE:
            if (tt->is_unsigned) {
                *(uint64_t*) to = cast_uint64(L, idx, !check_pointers);
            } else {
                *(int64_t*) to = cast_int64(L, idx, !check_pointers);
            }
            break;
        case FLOAT_TYPE:
            *(float*) to = (float) check_double(L, idx);
            break;
        case DOUBLE_TYPE:
            *(double*) to = check_double(L, idx);
            break;
        case COMPLEX_FLOAT_TYPE:
            *(complex_float*) to = check_complex_float(L, idx);
            break;
        case COMPLEX_DOUBLE_TYPE:
            *(complex_double*) to = check_complex_double(L, idx);
            break;
        case INTPTR_TYPE:
            *(uintptr_t*) to = check_uintptr(L, idx);
            break;
        case ENUM_TYPE:
            *(int32_t*) to = check_enum(L, idx, to_usr, tt);
            break;
        case FUNCTION_PTR_TYPE:
            *(cfunction*) to = check_cfunction(L, idx, to_usr, tt, check_pointers);
            break;
        default:
            goto err;
        }

#ifndef ALLOW_MISALIGNED_ACCESS
        if ((uintptr_t) origto & (tt->base_size - 1)) {
            memcpy(origto, misalign.c, tt->base_size);
        }
#endif
    }

    assert(lua_gettop(L) == top);
    return;
err:
    type_error(L, idx, NULL, to_usr, tt);
}

static int ffi_typeof(lua_State* L)
{
    struct ctype ct;
    check_ctype(L, 1, &ct);
    push_ctype(L, -1, &ct);
    return 1;
}

static void setmintop(lua_State* L, int idx)
{
    if (lua_gettop(L) < idx) {
        lua_settop(L, idx);
    }
}

/* warning: in the case that it finds an array size, it removes that index */
static void get_variable_array_size(lua_State* L, int idx, struct ctype* ct)
{
    /* we only care about the variable buisness for the variable array
     * directly ie ffi.new('char[?]') or the struct that contains the variable
     * array ffi.new('struct {char v[?]}'). A pointer to the struct doesn't
     * care about the variable size (it treats it as a zero sized array). */

    if (ct->is_variable_array) {
        assert(ct->is_array);
        ct->array_size = (size_t) luaL_checknumber(L, idx);
        ct->is_variable_array = 0;
        lua_remove(L, idx);

    } else if (ct->is_variable_struct && !ct->variable_size_known) {
        assert(ct->type == STRUCT_TYPE && !ct->is_array);
        ct->variable_increment *= (size_t) luaL_checknumber(L, idx);
        ct->variable_size_known = 1;
        lua_remove(L, idx);
    }
}

static int try_set_value(lua_State* L)
{
    void* p = lua_touserdata(L, 2);
    struct ctype* ct = (struct ctype*) lua_touserdata(L, 4);
    int check_ptrs = lua_toboolean(L, 5);
    set_value(L, 1, p, 3, ct, check_ptrs);
    return 0;
}

static int do_new(lua_State* L, int is_cast)
{
    int cargs, i;
    void* p;
    struct ctype ct;
    int check_ptrs = !is_cast;

    check_ctype(L, 1, &ct);

    /* don't push a callback when we have a c function, as cb:set needs a
     * compiled callback from a lua function to work */
    if (!ct.pointers && ct.type == FUNCTION_PTR_TYPE && (lua_isnil(L, 2) || lua_isfunction(L, 2))) {
        /* Function cdatas are pinned and must be manually cleaned up by
         * calling func:free(). */
        compile_callback(L, 2, -1, &ct);
        push_upval(L, &callbacks_key);
        lua_pushvalue(L, -2);
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
        lua_pop(L, 1); /* callbacks tbl */
        return 1;
    }

    /* this removes the vararg argument if its needed, and errors if its invalid */
    if (!is_cast) {
        get_variable_array_size(L, 2, &ct);
    }

    p = push_cdata(L, -1, &ct);

    /* if the user mt has a __gc function then call ffi.gc on this value */
    if (push_user_mt(L, -2, &ct)) {
        push_upval(L, &gc_key);
        lua_pushvalue(L, -3);

        /* user_mt.__gc */
        lua_pushliteral(L, "__gc");
        lua_rawget(L, -4);

        lua_rawset(L, -3); /* gc_upval[cdata] = user_mt.__gc */
        lua_pop(L, 2); /* user_mt and gc_upval */
    }

    /* stack is:
     * ctype arg
     * ctor args ... 0+
     * ctype usr
     * cdata
     */

    cargs = lua_gettop(L) - 3;

    if (cargs == 0) {
        return 1;
    }

    if (cargs == 1) {
        /* try packed form first
         * packed: ffi.new('int[3]', {1})
         * unpacked: ffi.new('int[3]', 1)
         */
        lua_pushcfunction(L, &try_set_value);
        lua_pushvalue(L, 2); /* ctor arg */
        lua_pushlightuserdata(L, p);
        lua_pushvalue(L, -5); /* ctype usr */
        lua_pushlightuserdata(L, &ct);
        lua_pushboolean(L, check_ptrs);

        if (!lua_pcall(L, 5, 0, 0)) {
            return 1;
        }

        /* remove any errors */
        lua_settop(L, 4);
    }

    /* if we have more than 2 ctor arguments then they must be unpacked, e.g.
     * ffi.new('int[3]', 1, 2, 3) */
    lua_createtable(L, cargs, 0);
    lua_replace(L, 1);
    for (i = 1; i <= cargs; i++) {
        lua_pushvalue(L, i + 1);
        lua_rawseti(L, 1, i);
    }
    assert(lua_gettop(L) == cargs + 3);
    set_value(L, 1, p, -2, &ct, check_ptrs);

    return 1;
}

static int ffi_new(lua_State* L)
{ return do_new(L, 0); }

static int ffi_cast(lua_State* L)
{ return do_new(L, 1); }

static int ctype_new(lua_State* L)
{ return do_new(L, 0); }

static int ctype_call(lua_State* L)
{
    struct ctype ct;
    int top = lua_gettop(L);

    check_ctype(L, 1, &ct);

    if (push_user_mt(L, -1, &ct)) {
        lua_pushstring(L, "__new");
        lua_rawget(L, -2);
        if (!lua_isnil(L, -1)) {
            lua_insert(L, 1); // function at bottom of stack under args
            lua_pop(L, 2);
            lua_call(L, top, 1);
            return 1;
        }
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    assert(lua_gettop(L) == top);
    return do_new(L, 0);
}

static int ffi_sizeof(lua_State* L)
{
    struct ctype ct;
    check_ctype(L, 1, &ct);
    get_variable_array_size(L, 2, &ct);
    lua_pushnumber(L, ctype_size(L, &ct));
    return 1;
}

static int ffi_alignof(lua_State* L)
{
    struct ctype ct, mt;
    lua_settop(L, 2);
    check_ctype(L, 1, &ct);

    /* if no member is specified then we return the alignment of the type */
    if (lua_isnil(L, 2)) {
        lua_pushnumber(L, ct.align_mask + 1);
        return 1;
    }

    /* get the alignment of the member */
    lua_pushvalue(L, 2);
    if (get_member(L, -2, &ct, &mt) < 0) {
        push_type_name(L, 3, &ct);
        return luaL_error(L, "type %s has no member %s", lua_tostring(L, -1), lua_tostring(L, 2));
    }

    lua_pushnumber(L, mt.align_mask + 1);
    return 1;
}

static int ffi_offsetof(lua_State* L)
{
    ptrdiff_t off;
    struct ctype ct, mt;
    lua_settop(L, 2);
    check_ctype(L, 1, &ct);

    lua_pushvalue(L, 2);
    off = get_member(L, -2, &ct, &mt); /* this replaces the member key at -1 with the mbr usr value */
    if (off < 0) {
        push_type_name(L, 3, &ct);
        return luaL_error(L, "type %s has no member %s", lua_tostring(L, -1), lua_tostring(L, 2));
    }

    lua_pushnumber(L, off);

    if (!mt.is_bitfield) {
        return 1;
    }

    lua_pushnumber(L, mt.bit_offset);
    lua_pushnumber(L, mt.bit_size);
    return 3;
}

static int ffi_istype(lua_State* L)
{
    struct ctype tt, ft;
    check_ctype(L, 1, &tt);
    to_cdata(L, 2, &ft);

    if (ft.type == INVALID_TYPE) {
        goto fail;
    }

    if (!is_same_type(L, 3, 4, &tt, &ft)) {
        goto fail;
    }

    if (tt.pointers != ft.pointers) {
        goto fail;
    }

    if (tt.is_array != ft.is_array) {
        goto fail;
    }

    if (tt.is_array && tt.array_size != ft.array_size) {
        goto fail;
    }

    if (tt.calling_convention != ft.calling_convention) {
        goto fail;
    }

    lua_pushboolean(L, 1);
    return 1;

fail:
    lua_pushboolean(L, 0);
    return 1;
}

static int cdata_gc(lua_State* L)
{
    struct ctype ct;
    check_cdata(L, 1, &ct);
    lua_settop(L, 1);

    /* call the gc func if there is any registered */
    lua_pushvalue(L, 1);
    lua_rawget(L, lua_upvalueindex(2));
    if (!lua_isnil(L, -1)) {
        lua_pushvalue(L, 1);
        lua_pcall(L, 1, 0, 0);
    }

    /* unset the closure */
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    lua_rawset(L, lua_upvalueindex(1));

    return 0;
}

static int callback_free(lua_State* L)
{
    cfunction* p = (cfunction*) lua_touserdata(L, 1);
    free_code(get_jit(L), L, *p);
    return 0;
}

static int cdata_free(lua_State* L)
{
    struct ctype ct;
    cfunction* p = (cfunction*) check_cdata(L, 1, &ct);
    lua_settop(L, 1);

    /* unset the closure */
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    lua_rawset(L, lua_upvalueindex(1));

    if (ct.is_jitted) {
        free_code(get_jit(L), L, *p);
        *p = NULL;
    }

    return 0;
}

static int cdata_set(lua_State* L)
{
    struct ctype ct;
    cfunction* p = (cfunction*) check_cdata(L, 1, &ct);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    if (!ct.is_jitted) {
        luaL_error(L, "can't set the function for a non-lua callback");
    }

    if (*p == NULL) {
        luaL_error(L, "can't set the function for a free'd callback");
    }

    push_func_ref(L, *p);
    lua_pushvalue(L, 2);
    lua_rawseti(L, -2, CALLBACK_FUNC_USR_IDX);

    /* remove the closure for this callback as it embeds the function pointer
     * value */
    lua_pushvalue(L, 1);
    lua_pushboolean(L, 1);
    lua_rawset(L, lua_upvalueindex(1));

    return 0;
}

static int cdata_call(lua_State* L)
{
    struct ctype ct;
    int top = lua_gettop(L);
    cfunction* p = (cfunction*) check_cdata(L, 1, &ct);

    if (push_user_mt(L, -1, &ct)) {
        lua_pushliteral(L, "__call");
        lua_rawget(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_insert(L, 1);
            lua_pop(L, 2); /* ct_usr, user_mt */
            lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);
            return lua_gettop(L);
        }
    }
    if (ct.pointers || ct.type != FUNCTION_PTR_TYPE) {
        return luaL_error(L, "only function callbacks are callable");
    }

    lua_pushvalue(L, 1);
    lua_rawget(L, lua_upvalueindex(1));

    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        compile_function(L, *p, -1, &ct);

        assert(lua_gettop(L) == top + 2); /* uv, closure */

        /* closures[func] = closure */
        lua_pushvalue(L, 1);
        lua_pushvalue(L, -2);
        lua_rawset(L, lua_upvalueindex(1));

        lua_replace(L, 1);
    } else {
        lua_replace(L, 1);
    }

    lua_pop(L, 1); /* uv */
    assert(lua_gettop(L) == top);

    lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);
    return lua_gettop(L);
}

static int user_mt_key;

static int ffi_metatype(lua_State* L)
{
    struct ctype ct;
    lua_settop(L, 2);

    check_ctype(L, 1, &ct);
    if (lua_type(L, 2) != LUA_TTABLE && lua_type(L, 2) != LUA_TNIL) {
        return luaL_argerror(L, 2, "metatable must be a table or nil");
    }

    lua_pushlightuserdata(L, &user_mt_key);
    lua_pushvalue(L, 2);
    lua_rawset(L, 3); /* user[user_mt_key] = mt */

    /* return the passed in ctype */
    push_ctype(L, 3, &ct);
    return 1;
}

/* push_user_mt returns 1 if the type has a user metatable and pushes it onto
 * the stack, otherwise it returns 0 and pushes nothing */
int push_user_mt(lua_State* L, int ct_usr, const struct ctype* ct)
{
    if (ct->type != STRUCT_TYPE && ct->type != UNION_TYPE) {
        return 0;
    }

    ct_usr = lua_absindex(L, ct_usr);
    lua_pushlightuserdata(L, &user_mt_key);
    lua_rawget(L, ct_usr);

    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return 0;
    }
    return 1;
}

static int ffi_gc(lua_State* L)
{
    struct ctype ct;
    lua_settop(L, 2);
    check_cdata(L, 1, &ct);

    push_upval(L, &gc_key);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_rawset(L, -3);

    /* return the cdata back */
    lua_settop(L, 1);
    return 1;
}

/* lookup_cdata_index returns the offset of the found type and user value on
 * the stack if valid. Otherwise returns -ve and doesn't touch the stack.
 */
static ptrdiff_t lookup_cdata_index(lua_State* L, int idx, int ct_usr, struct ctype* ct)
{
    struct ctype mt;
    ptrdiff_t off;

    ct_usr = lua_absindex(L, ct_usr);

    switch (lua_type(L, idx)) {
    case LUA_TNUMBER:
        /* possibilities are array, pointer */

        if (!ct->pointers || is_void_ptr(ct)) {
            return -1;
        }

        ct->is_array = 0;
        ct->pointers--;
        ct->const_mask >>= 1;

        lua_pushvalue(L, ct_usr);

        return (ct->pointers ? sizeof(void*) : ct->base_size) * lua_tonumber(L, 2);

    case LUA_TSTRING:
        /* possibilities are struct/union, pointer to struct/union */

        if ((ct->type != STRUCT_TYPE && ct->type != UNION_TYPE) || ct->is_array || ct->pointers > 1) {
            return -1;
        }

        lua_pushvalue(L, idx);
        off = get_member(L, ct_usr, ct, &mt);
        if (off < 0) {
            return -1;
        }

        *ct = mt;
        return off;

    default:
        return -1;
    }
}

static int cdata_newindex(lua_State* L)
{
    struct ctype tt;
    char* to;
    ptrdiff_t off;

    lua_settop(L, 3);

    to = (char*) check_cdata(L, 1, &tt);
    off = lookup_cdata_index(L, 2, -1, &tt);

    if (off < 0) {
        if (!push_user_mt(L, -1, &tt)) {
            goto err;
        }

        lua_pushliteral(L, "__newindex");
        lua_rawget(L, -2);

        if (lua_isnil(L, -1)) {
            goto err;
        }

        lua_insert(L, 1);
        lua_settop(L, 4);
        lua_call(L, 3, LUA_MULTRET);
        return lua_gettop(L);
    }

    if (tt.const_mask & 1) {
        return luaL_error(L, "can't set const data");
    }

    set_value(L, 3, to + off, -1, &tt, 1);
    return 0;

err:
    push_type_name(L, 4, &tt);
    return luaL_error(L, "type %s has no member %s", lua_tostring(L, -1), lua_tostring(L, 2));
}

static int cdata_index(lua_State* L)
{
    void* to;
    struct ctype ct;
    char* data;
    ptrdiff_t off;

    lua_settop(L, 2);
    data = (char*) check_cdata(L, 1, &ct);
    assert(lua_gettop(L) == 3);

    if (!ct.pointers) {
        switch (ct.type) {
        case FUNCTION_PTR_TYPE:
            /* Callbacks use the same metatable as standard cdata values, but have set
             * and free members. So instead of mt.__index = mt, we do the equiv here. */
            lua_getmetatable(L, 1);
            lua_pushvalue(L, 2);
            lua_rawget(L, -2);
            return 1;

            /* This provides the .re and .im virtual members */
        case COMPLEX_DOUBLE_TYPE:
        case COMPLEX_FLOAT_TYPE:
            if (!lua_isstring(L, 2)) {
                luaL_error(L, "invalid member for complex number");

            } else if (strcmp(lua_tostring(L, 2), "re") == 0) {
                lua_pushnumber(L, ct.type == COMPLEX_DOUBLE_TYPE ? creal(*(complex_double*) data) : crealf(*(complex_float*) data));

            } else if (strcmp(lua_tostring(L, 2), "im") == 0) {
                lua_pushnumber(L, ct.type == COMPLEX_DOUBLE_TYPE ? cimag(*(complex_double*) data) : cimagf(*(complex_float*) data));

            } else {
                luaL_error(L, "invalid member for complex number");
            }
            return 1;
        }
    }

    off = lookup_cdata_index(L, 2, -1, &ct);

    if (off < 0) {
        assert(lua_gettop(L) == 3);
        if (!push_user_mt(L, -1, &ct)) {
            goto err;
        }

        lua_pushliteral(L, "__index");
        lua_rawget(L, -2);

        if (lua_isnil(L, -1)) {
            goto err;
        }

        if (lua_istable(L, -1)) {
            lua_pushvalue(L, 2);
            lua_gettable(L, -2);
            return 1;
        }

        lua_insert(L, 1);
        lua_settop(L, 3);
        lua_call(L, 2, LUA_MULTRET);
        return lua_gettop(L);

err:
        push_type_name(L, 3, &ct);
        return luaL_error(L, "type %s has no member %s", lua_tostring(L, -1), lua_tostring(L, 2));
    }

    assert(lua_gettop(L) == 4); /* ct, key, ct_usr, mbr_usr */
    data += off;

    if (ct.is_array) {
        /* push a reference to the array */
        ct.is_reference = 1;
        to = push_cdata(L, -1, &ct);
        *(void**) to = data;
        return 1;

    } else if (ct.is_bitfield) {

        if (ct.type == INT64_TYPE) {
            struct ctype rt;
            uint64_t val = *(uint64_t*) data;
            val >>= ct.bit_offset;
            val &= (UINT64_C(1) << ct.bit_size) - 1;

            memset(&rt, 0, sizeof(rt));
            rt.base_size = 8;
            rt.type = INT64_TYPE;
            rt.is_unsigned = 1;
            rt.is_defined = 1;

            to = push_cdata(L, 0, &rt);
            *(uint64_t*) to = val;

            return 1;

        } else if (ct.type == BOOL_TYPE) {
            uint64_t val = *(uint64_t*) data;
            lua_pushboolean(L, (int) (val & (UINT64_C(1) << ct.bit_offset)));
            return 1;

        } else {
            uint64_t val = *(uint64_t*) data;
            val >>= ct.bit_offset;
            val &= (UINT64_C(1) << ct.bit_size) - 1;
            lua_pushnumber(L, val);
            return 1;
        }

    } else if (ct.pointers) {
#ifndef ALLOW_MISALIGNED_ACCESS
        union {
            uint8_t c[8];
            void* p;
        } misalignbuf;

        if ((uintptr_t) data & PTR_ALIGN_MASK) {
            memcpy(misalignbuf.c, data, sizeof(void*));
            data = misalignbuf.c;
        }
#endif
        to = push_cdata(L, -1, &ct);
        *(void**) to = *(void**) data;
        return 1;

    } else if (ct.type == STRUCT_TYPE || ct.type == UNION_TYPE) {
        /* push a reference to the member */
        ct.is_reference = 1;
        to = push_cdata(L, -1, &ct);
        *(void**) to = data;
        return 1;

    } else if (ct.type == FUNCTION_PTR_TYPE) {
        cfunction* pf = (cfunction*) push_cdata(L, -1, &ct);
        *pf = *(cfunction*) data;
        return 1;

    } else {
#ifndef ALLOW_MISALIGNED_ACCESS
        union {
            uint8_t c[8];
            double d;
            float f;
            uint64_t u64;
        } misalignbuf;

        assert(ct.base_size <= 8);

        if ((uintptr_t) data & (ct.base_size - 1)) {
            memcpy(misalignbuf.c, data, ct.base_size);
            data = misalignbuf.c;
        }
#endif

        switch (ct.type) {
        case BOOL_TYPE:
            lua_pushboolean(L, *(_Bool*) data);
            break;
        case INT8_TYPE:
            lua_pushnumber(L, ct.is_unsigned ? (lua_Number) *(uint8_t*) data : (lua_Number) *(int8_t*) data);
            break;
        case INT16_TYPE:
            lua_pushnumber(L, ct.is_unsigned ? (lua_Number) *(uint16_t*) data : (lua_Number) *(int16_t*) data);
            break;
        case ENUM_TYPE:
        case INT32_TYPE:
            lua_pushnumber(L, ct.is_unsigned ? (lua_Number) *(uint32_t*) data : (lua_Number) *(int32_t*) data);
            break;
        case INT64_TYPE:
            to = push_cdata(L, -1, &ct);
            *(int64_t*) to = *(int64_t*) data;
            break;
        case INTPTR_TYPE:
            to = push_cdata(L, -1, &ct);
            *(intptr_t*) to = *(intptr_t*) data;
            break;
        case FLOAT_TYPE:
            lua_pushnumber(L, *(float*) data);
            break;
        case DOUBLE_TYPE:
            lua_pushnumber(L, *(double*) data);
            break;
        default:
            luaL_error(L, "internal error: invalid member type");
        }

        return 1;
    }
}

static complex_double check_complex(lua_State* L, int idx, void* p, struct ctype* ct)
{
    if (ct->type == INVALID_TYPE) {
        double d = luaL_checknumber(L, idx);
#ifdef HAVE_COMPLEX
        return d;
#else
        complex_double c;
        c.real = d;
        c.imag = 0;
        return c;
#endif
    } else if (ct->type == COMPLEX_DOUBLE_TYPE) {
        return *(complex_double*) p;
    } else if (ct->type == COMPLEX_FLOAT_TYPE) {
        complex_float* f = (complex_float*) p;
#ifdef HAVE_COMPLEX
        return *f;
#else
        complex_double d;
        d.real = f->real;
        d.imag = f->imag;
        return d;
#endif
    } else {
        complex_double dummy;
        type_error(L, idx, "complex", 0, NULL);
        memset(&dummy, 0, sizeof(dummy));
        return dummy;
    }
}

static int rank(const struct ctype* ct)
{
    if (ct->pointers) {
        return 5;
    }

    switch (ct->type) {
    case COMPLEX_DOUBLE_TYPE:
        return 7;
    case COMPLEX_FLOAT_TYPE:
        return 6;
    case INTPTR_TYPE:
        return sizeof(intptr_t) >= sizeof(int64_t) ? 4 : 1;
    case INT64_TYPE:
        return ct->is_unsigned ? 3 : 2;
    case INT32_TYPE:
    case INT16_TYPE:
    case INT8_TYPE:
        return 2;
    default:
        return 0;
    }
}

static void push_complex(lua_State* L, complex_double res, int ct_usr, const struct ctype* ct)
{
    if (ct->type == COMPLEX_DOUBLE_TYPE) {
        complex_double* p = (complex_double*) push_cdata(L, ct_usr, ct);
        *p = res;
    } else {
        complex_float* p = (complex_float*) push_cdata(L, ct_usr, ct);
#ifdef HAVE_COMPLEX
        *p = (complex float) res;
#else
        p->real = (float) res.real;
        p->imag = (float) res.imag;
#endif
    }
}

static void push_number(lua_State* L, int64_t val, int ct_usr, const struct ctype* ct)
{
    if ((ct->pointers || ct->type == INTPTR_TYPE) && sizeof(intptr_t) != sizeof(int64_t)) {
        intptr_t* p = (intptr_t*) push_cdata(L, ct_usr, ct);
        *p = val;
    } else {
        int64_t* p = (int64_t*) push_cdata(L, ct_usr, ct);
        *p = val;
    }
}

static int call_user_op(lua_State* L, const char* opfield, int idx, int ct_usr, const struct ctype* ct)
{
    idx = lua_absindex(L, idx);

    if (push_user_mt(L, ct_usr, ct)) {
        lua_pushstring(L, opfield);
        lua_rawget(L, -2);
        if (!lua_isnil(L, -1)) {
            int top = lua_gettop(L);
            lua_pushvalue(L, idx);
            lua_call(L, 1, LUA_MULTRET);
            return lua_gettop(L) - top + 1;
        }
      lua_pop(L, 2);
    }
    return -1;
}

static int cdata_unm(lua_State* L)
{
    struct ctype ct;
    void* p;
    int64_t val;
    int ret;

    lua_settop(L, 1);
    p = to_cdata(L, 1, &ct);

    ret = call_user_op(L, "__unm", 1, 2, &ct);
    if (ret >= 0) {
        return ret;
    }

    val = check_intptr(L, 1, p, &ct);

    if (ct.pointers) {
        luaL_error(L, "can't negate a pointer value");
    } else {
        memset(&ct, 0, sizeof(ct));
        ct.type = INT64_TYPE;
        ct.base_size = 8;
        ct.is_defined = 1;
        push_number(L, -val, 0, &ct);
    }

    return 1;
}

/* returns -ve if no binop was called otherwise returns the number of return
 * arguments */
static int call_user_binop(lua_State* L, const char* opfield, int lidx, int lusr, const struct ctype* lt, int ridx, int rusr, const struct ctype* rt)
{
    lidx = lua_absindex(L, lidx);
    ridx = lua_absindex(L, ridx);

    if (push_user_mt(L, lusr, lt)) {
        lua_pushstring(L, opfield);
        lua_rawget(L, -2);

        if (!lua_isnil(L, -1)) {
            int top = lua_gettop(L);
            lua_pushvalue(L, lidx);
            lua_pushvalue(L, ridx);
            lua_call(L, 2, LUA_MULTRET);
            return lua_gettop(L) - top + 1;
        }

        lua_pop(L, 2); /* user_mt and user_mt.op */
    }

    if (push_user_mt(L, rusr, rt)) {
        lua_pushstring(L, opfield);
        lua_rawget(L, -2);

        if (!lua_isnil(L, -1)) {
            int top = lua_gettop(L);
            lua_pushvalue(L, lidx);
            lua_pushvalue(L, ridx);
            lua_call(L, 2, LUA_MULTRET);
            return lua_gettop(L) - top + 1;
        }

        lua_pop(L, 2); /* user_mt and user_mt.op */
    }

    return -1;
}

static int cdata_concat(lua_State* L)
{
    struct ctype lt, rt;
    int ret;

    lua_settop(L, 2);
    to_cdata(L, 1, &lt);
    to_cdata(L, 2, &rt);

    ret = call_user_binop(L, "__concat", 1, 3, &lt, 2, 4, &rt);
    if (ret >= 0) {
        return ret;
    }

    return luaL_error(L, "NYI");
}

static int cdata_len(lua_State* L)
{
    struct ctype ct;
    int ret;

    lua_settop(L, 1);
    to_cdata(L, 1, &ct);

    ret = call_user_op(L, "__len", 1, 2, &ct);
    if (ret >= 0) {
        return ret;
    }

    push_type_name(L, 2, &ct);
    return luaL_error(L, "type %s does not implement the __len metamethod", lua_tostring(L, -1));
}

static int cdata_pairs(lua_State* L)
{
    struct ctype ct;
    int ret;

    lua_settop(L, 1);
    to_cdata(L, 1, &ct);

    ret = call_user_op(L, "__pairs", 1, 2, &ct);
    if (ret >= 0) {
        return ret;
    }

    push_type_name(L, 2, &ct);
    return luaL_error(L, "type %s does not implement the __pairs metamethod", lua_tostring(L, -1));
}

static int cdata_ipairs(lua_State* L)
{
    struct ctype ct;
    int ret;

    lua_settop(L, 1);
    to_cdata(L, 1, &ct);

    ret = call_user_op(L, "__ipairs", 1, 2, &ct);
    if (ret >= 0) {
        return ret;
    }

    push_type_name(L, 2, &ct);
    return luaL_error(L, "type %s does not implement the __ipairs metamethod", lua_tostring(L, -1));
}

static int cdata_add(lua_State* L)
{
    struct ctype lt, rt, ct;
    void *lp, *rp;
    int ct_usr;
    int ret;

    lua_settop(L, 2);

    lp = to_cdata(L, 1, &lt);
    rp = to_cdata(L, 2, &rt);
    assert(lua_gettop(L) == 4);

    ret = call_user_binop(L, "__add", 1, 3, &lt, 2, 4, &rt);
    if (ret >= 0) {
        return ret;
    }
    assert(lua_gettop(L) == 4);

    ct_usr = rank(&lt) > rank(&rt) ? 3 : 4;
    ct = rank(&lt) > rank(&rt) ? lt : rt;

    if (IS_COMPLEX(ct.type)) {
        complex_double left, right, res;

        left = check_complex(L, 1, lp, &lt);
        right = check_complex(L, 2, rp, &rt);
        assert(lua_gettop(L) == 4);

#ifdef HAVE_COMPLEX
        res = left + right;
#else
        res.real = left.real + right.real;
        res.imag = left.imag + right.imag;
#endif

        push_complex(L, res, ct_usr, &ct);
        return 1;

    } else {
        int64_t left = check_intptr(L, 1, lp, &lt);
        int64_t right = check_intptr(L, 2, rp, &rt);
        assert(lua_gettop(L) == 4);

        /* note due to 2s complement it doesn't matter if we do the addition as int or uint,
         * but the result needs to be uint64_t if either of the sources are */

        if (lt.pointers && rt.pointers) {
            luaL_error(L, "can't add two pointers");

        } else if (lt.pointers) {
            int64_t res = left + (lt.pointers > 1 ? sizeof(void*) : lt.base_size) * right;
            lt.is_array = 0;
            push_number(L, res, 3, &lt);

        } else if (rt.pointers) {
            int64_t res = right + (rt.pointers > 1 ? sizeof(void*) : rt.base_size) * left;
            rt.is_array = 0;
            push_number(L, res, 4, &rt);

        } else {
            push_number(L, left + right, ct_usr, &ct);
        }

        return 1;
    }
}

static int cdata_sub(lua_State* L)
{
    struct ctype lt, rt, ct;
    void *lp, *rp;
    int ct_usr;
    int ret;

    lua_settop(L, 2);

    lp = to_cdata(L, 1, &lt);
    rp = to_cdata(L, 2, &rt);

    ret = call_user_binop(L, "__sub", 1, 3, &lt, 2, 4, &rt);
    if (ret >= 0) {
        return ret;
    }

    ct_usr = rank(&lt) > rank(&rt) ? 3 : 4;
    ct = rank(&lt) > rank(&rt) ? lt : rt;

    if (IS_COMPLEX(ct.type)) {
        complex_double left, right, res;

        left = check_complex(L, 1, lp, &lt);
        right = check_complex(L, 2, rp, &rt);

#ifdef HAVE_COMPLEX
        res = left - right;
#else
        res.real = left.real - right.real;
        res.imag = left.imag - right.imag;
#endif

        push_complex(L, res, ct_usr, &ct);
        return 1;

    } else {
        int64_t left = check_intptr(L, 1, lp, &lt);
        int64_t right = check_intptr(L, 2, rp, &rt);

        if (rt.pointers) {
            luaL_error(L, "NYI: can't subtract a pointer value");

        } else if (lt.pointers) {
            int64_t res = left - (lt.pointers > 1 ? sizeof(void*) : lt.base_size) * right;
            lt.is_array = 0;
            push_number(L, res, 3, &lt);

        } else {
            int64_t res = left - right;
            push_number(L, res, ct_usr, &ct);
        }

        return 1;
    }
}

/* TODO fix for unsigned */
#define NUMBER_ONLY_BINOP(OPSTR, DO_NORMAL, DO_COMPLEX)                     \
    struct ctype lt, rt, ct;                                                \
    void *lp, *rp;                                                          \
    int ct_usr;                                                             \
    int ret;                                                                \
                                                                            \
    lua_settop(L, 2);                                                       \
                                                                            \
    lp = to_cdata(L, 1, &lt);                                               \
    rp = to_cdata(L, 2, &rt);                                               \
                                                                            \
    ret = call_user_binop(L, OPSTR, 1, 3, &lt, 2, 4, &rt);                  \
    if (ret >= 0) {                                                         \
        return ret;                                                         \
    }                                                                       \
                                                                            \
    ct_usr = rank(&lt) > rank(&rt) ? 3 : 4;                                 \
    ct = rank(&lt) > rank(&rt) ? lt : rt;                                   \
                                                                            \
    if (IS_COMPLEX(ct.type)) {                                              \
        complex_double res;                                                 \
        complex_double left = check_complex(L, 1, lp, &lt);                 \
        complex_double right = check_complex(L, 2, rp, &rt);                \
                                                                            \
        DO_COMPLEX(left, right, res);                                       \
        push_complex(L, res, ct_usr, &ct);                                  \
                                                                            \
    } else if (lt.pointers || rt.pointers) {                                \
        luaL_error(L, "can't operate on a pointer value");                  \
                                                                            \
    } else {                                                                \
        int64_t res;                                                        \
        int64_t left = check_intptr(L, 1, lp, &lt);                         \
        int64_t right = check_intptr(L, 2, rp, &rt);                        \
                                                                            \
        DO_NORMAL(left, right, res);                                        \
        push_number(L, res, ct_usr, &ct);                                   \
    }                                                                       \
                                                                            \
    return 1

#define MUL(l,r,s) s = l * r
#define DIV(l,r,s) s = l / r
#define MOD(l,r,s) s = l % r
#define POW(l,r,s) s = pow(l, r)

#ifdef HAVE_COMPLEX
#define MULC(l,r,s) s = l * r
#define DIVC(l,r,s) s = l / r
#define MODC(l,r,s) (void) l, (void) r, memset(&s, 0, sizeof(s)), luaL_error(L, "NYI: complex mod")
#define POWC(l,r,s) s = cpow(l, r)
#else
#define MULC(l,r,s) s.real = l.real * r.real - l.imag * r.imag, s.imag = l.real * r.imag + l.imag * r.real
#define DIVC(l,r,s) s.real = (l.real * r.real + l.imag * r.imag) / (r.real * r.real + r.imag * r.imag), \
                    s.imag = (l.imag * r.real - l.real * r.imag) / (r.real * r.real + r.imag * r.imag)
#define MODC(l,r,s) (void) l, (void) r, memset(&s, 0, sizeof(s)), luaL_error(L, "NYI: complex mod")
#define POWC(l,r,s) (void) l, (void) r, memset(&s, 0, sizeof(s)), luaL_error(L, "NYI: complex pow")
#endif

static int cdata_mul(lua_State* L)
{ NUMBER_ONLY_BINOP("__mul", MUL, MULC); }

static int cdata_div(lua_State* L)
{ NUMBER_ONLY_BINOP("__div", DIV, DIVC); }

static int cdata_mod(lua_State* L)
{ NUMBER_ONLY_BINOP("__mod", MOD, MODC); }

static int cdata_pow(lua_State* L)
{ NUMBER_ONLY_BINOP("__pow", POW, POWC); }

#define COMPARE_BINOP(OPSTR, OP, OPC)                                       \
    struct ctype lt, rt;                                                    \
    void *lp, *rp;                                                          \
    int ret, res;                                                           \
                                                                            \
    lua_settop(L, 2);                                                       \
                                                                            \
    lp = to_cdata(L, 1, &lt);                                               \
    rp = to_cdata(L, 2, &rt);                                               \
                                                                            \
    ret = call_user_binop(L, OPSTR, 1, 3, &lt, 2, 4, &rt);                  \
    if (ret >= 0) {                                                         \
        return ret;                                                         \
    }                                                                       \
                                                                            \
    if (IS_COMPLEX(lt.type) || IS_COMPLEX(rt.type)) {                       \
        complex_double left = check_complex(L, 1, lp, &lt);                 \
        complex_double right = check_complex(L, 2, rp, &rt);                \
                                                                            \
        res = OPC(left, right);                                             \
                                                                            \
        lua_pushboolean(L, res);                                            \
                                                                            \
    } else {                                                                \
        int64_t left = check_intptr(L, 1, lp, &lt);                         \
        int64_t right = check_intptr(L, 2, rp, &rt);                        \
                                                                            \
        if (lt.pointers && rt.pointers) {                                   \
            if (is_void_ptr(&lt) || is_void_ptr(&rt) || is_same_type(L, 3, 4, &lt, &rt)) { \
                res = OP((uint64_t) left, (uint64_t) right);                \
            } else {                                                        \
                goto err;                                                   \
            }                                                               \
                                                                            \
        } else if (lt.is_null && rt.type == FUNCTION_PTR_TYPE) {            \
            res = OP((uint64_t) left, (uint64_t) right);                    \
                                                                            \
        } else if (rt.is_null && lt.type == FUNCTION_PTR_TYPE) {            \
            res = OP((uint64_t) left, (uint64_t) right);                    \
                                                                            \
        } else if (lt.pointers && rt.type == INTPTR_TYPE && rt.is_unsigned) {\
            res = OP((uint64_t) left, (uint64_t) right);                    \
                                                                            \
        } else if (rt.pointers && lt.type == INTPTR_TYPE && lt.is_unsigned) {\
            res = OP((uint64_t) left, (uint64_t) right);                    \
                                                                            \
        } else if (rt.pointers || lt.pointers) {                            \
            goto err;                                                       \
                                                                            \
        } else if (lt.is_unsigned && rt.is_unsigned) {                      \
            res = OP((uint64_t) left, (uint64_t) right);                    \
                                                                            \
        } else if (lt.is_unsigned) {                                        \
            res = OP((int64_t) (uint64_t) left, right);                     \
                                                                            \
        } else if (rt.is_unsigned) {                                        \
            res = OP(left, (int64_t) (uint64_t) right);                     \
                                                                            \
        } else {                                                            \
            res = OP(left, right);                                          \
        }                                                                   \
                                                                            \
        lua_pushboolean(L, res);                                            \
    }                                                                       \
    return 1

#define EQ(l, r) (l) == (r)
#define LT(l, r) (l) < (r)
#define LE(l, r) (l) <= (r)

#ifdef HAVE_COMPLEX
#define EQC(l, r) (l) == (r)
#else
#define EQC(l, r) (l).real == (r).real && (l).imag == (r).imag
#endif

#define LEC(l, r) EQC(l, r), luaL_error(L, "complex numbers are non-orderable")
#define LTC(l, r) EQC(l, r), luaL_error(L, "complex numbers are non-orderable")

static int cdata_eq(lua_State* L)
{
    COMPARE_BINOP("__eq", EQ, EQC);
err:
    lua_pushboolean(L, 0);
    return 1;
}

static int cdata_lt(lua_State* L)
{
    COMPARE_BINOP("__lt", LT, LTC);
err:
    lua_getuservalue(L, 1);
    lua_getuservalue(L, 2);
    push_type_name(L, -2, &lt);
    push_type_name(L, -2, &lt);
    return luaL_error(L, "trying to compare incompatible types %s and %s", lua_tostring(L, -2), lua_tostring(L, -1));
}

static int cdata_le(lua_State* L)
{
    COMPARE_BINOP("__le", LE, LEC);
err:
    lua_getuservalue(L, 1);
    lua_getuservalue(L, 2);
    push_type_name(L, -2, &lt);
    push_type_name(L, -2, &lt);
    return luaL_error(L, "trying to compare incompatible types %s and %s", lua_tostring(L, -2), lua_tostring(L, -1));
}

static const char* etype_tostring(int type)
{
    switch (type) {
    case VOID_TYPE: return "void";
    case DOUBLE_TYPE: return "double";
    case FLOAT_TYPE: return "float";
    case COMPLEX_DOUBLE_TYPE: return "complex double";
    case COMPLEX_FLOAT_TYPE: return "complex float";
    case BOOL_TYPE: return "bool";
    case INT8_TYPE: return "int8";
    case INT16_TYPE: return "int16";
    case INT32_TYPE: return "int32";
    case INT64_TYPE: return "int64";
    case INTPTR_TYPE: return "intptr";
    case ENUM_TYPE: return "enum";
    case UNION_TYPE: return "union";
    case STRUCT_TYPE: return "struct";
    case FUNCTION_PTR_TYPE: return "function ptr";
    case FUNCTION_TYPE: return "function";
    default: return "invalid";
    }
}

static void print_type(lua_State* L, const struct ctype* ct)
{
    lua_pushfstring(L, " sz %d %d %d align %d ptr %d %d %d type %s%s %d %d %d name %d call %d %d var %d %d %d bit %d %d %d %d jit %d",
            /* sz */
            ct->base_size,
            ct->array_size,
            ct->offset,
            /* align */
            ct->align_mask,
            /* ptr */
            ct->is_array,
            ct->pointers,
            ct->const_mask,
            /* type */
            ct->is_unsigned ? "u" : "",
            etype_tostring(ct->type),
            ct->is_reference,
            ct->is_defined,
            ct->is_null,
            /* name */
            ct->has_member_name,
            /* call */
            ct->calling_convention,
            ct->has_var_arg,
            /* var */
            ct->is_variable_array,
            ct->is_variable_struct,
            ct->variable_size_known,
            /* bit */
            ct->is_bitfield,
            ct->has_bitfield,
            ct->bit_offset,
            ct->bit_size,
            /* jit */
            ct->is_jitted);
}

static int ctype_tostring(lua_State* L)
{
    struct ctype ct;
    assert(lua_type(L, 1) == LUA_TUSERDATA);
    lua_settop(L, 1);
    check_ctype(L, 1, &ct);
    assert(lua_gettop(L) == 2);
    push_type_name(L, -1, &ct);
    lua_pushfstring(L, "ctype<%s> %p", lua_tostring(L, -1), lua_topointer(L, 1));

    if (DEBUG_TOSTRING) {
        print_type(L, &ct);
        lua_concat(L, 2);
    }

    return 1;
}

static int cdata_tostring(lua_State* L)
{
    struct ctype ct;
    char buf[64];
    void* p;
    int ret;

    lua_settop(L, 1);
    p = to_cdata(L, 1, &ct);

    ret = call_user_op(L, "__tostring", 1, 2, &ct);
    if (ret >= 0) {
        return ret;
    }

    if (ct.pointers > 0 || ct.type == STRUCT_TYPE || ct.type == UNION_TYPE) {
        push_type_name(L, -1, &ct);
        lua_pushfstring(L, "cdata<%s>: %p", lua_tostring(L, -1), p);

        if (DEBUG_TOSTRING) {
            print_type(L, &ct);
            lua_concat(L, 2);
        }

        return 1;
    }

    switch (ct.type) {
    case COMPLEX_DOUBLE_TYPE:
        {
            complex_double c = *(complex_double*) p;
            if (cimag(c) != 0) {
                lua_pushfstring(L, "%f+%fi", creal(c), cimag(c));
            } else {
                lua_pushfstring(L, "%f", creal(c));
            }
        }
        return 1;

    case COMPLEX_FLOAT_TYPE:
        {
            complex_float c = *(complex_float*) p;
            if (cimagf(c) != 0) {
                lua_pushfstring(L, "%f+%fi", crealf(c), cimagf(c));
            } else {
                lua_pushfstring(L, "%f", crealf(c));
            }
        }
        return 1;

    case FUNCTION_PTR_TYPE:
        p = *(void**) p;
        push_type_name(L, -1, &ct);
        lua_pushfstring(L, "cdata<%s>: %p", lua_tostring(L, -1), *(void**) p);
        return 1;

    case INTPTR_TYPE:
        lua_pushfstring(L, "%p", *(uintptr_t*) p);
        return 1;

    case INT64_TYPE:
        sprintf(buf, ct.is_unsigned ? "%"PRIu64 : "%"PRId64, *(uint64_t*) p);
        lua_pushstring(L, buf);
        return 1;

    default:
        sprintf(buf, ct.is_unsigned ? "%"PRId64 : "%"PRId64, (int64_t) check_intptr(L, 1, p, &ct));
        lua_pushstring(L, buf);
        return 1;
    }
}

static int ffi_errno(lua_State* L)
{
    struct jit* jit = get_jit(L);

    if (!lua_isnoneornil(L, 1)) {
        lua_pushnumber(L, jit->last_errno);
        jit->last_errno = luaL_checknumber(L, 1);
    } else {
        lua_pushnumber(L, jit->last_errno);
    }

    return 1;
}

static int ffi_number(lua_State* L)
{
    struct ctype ct;
    void* data = to_cdata(L, 1, &ct);

    if (ct.type != INVALID_TYPE) {
        lua_pushnumber(L, check_intptr(L, 1, data, &ct));
        return 1;
    } else {
        /* call the old _G.tonumber, we use an upvalue as _G.tonumber is set
         * to this function */
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_insert(L, 1);
        lua_call(L, lua_gettop(L)-1, LUA_MULTRET);
        return lua_gettop(L);
    }
}

static int ffi_string(lua_State* L)
{
    struct ctype ct;
    char* data;
    lua_settop(L, 2);

    data = (char*) check_cdata(L, 1, &ct);

    if (is_void_ptr(&ct)) {
        lua_pushlstring(L, data, (size_t) luaL_checknumber(L, 2));
        return 1;

    } else if (ct.type == INT8_TYPE && ct.pointers == 1) {
        size_t sz;

        if (!lua_isnil(L, 2)) {
            sz = (size_t) luaL_checknumber(L, 2);

        } else if (ct.is_array && !ct.is_variable_array) {
            char* nul = memchr(data, '\0', ct.array_size);
            sz = nul ? nul - data : ct.array_size;

        } else {
            sz = strlen(data);
        }

        lua_pushlstring(L, data, sz);
        return 1;
    }

    return luaL_error(L, "unable to convert cdata to string");
}

static int ffi_copy(lua_State* L)
{
    struct ctype ft, tt;
    char *to, *from;

    setmintop(L, 3);
    to = (char*) check_pointer(L, 1, &tt);
    from = (char*) check_pointer(L, 2, &ft);

    if (!lua_isnoneornil(L, 3)) {
        memcpy(to, from, (size_t) luaL_checknumber(L, 3));

    } else if (ft.type == INT8_TYPE && ft.pointers == 1) {
        size_t sz = ft.is_array ? ft.array_size : strlen(from);
        memcpy(to, from, sz);
        to[sz] = '\0';
    }

    return 0;
}

static int ffi_fill(lua_State* L)
{
    struct ctype ct;
    void* to;
    size_t sz;
    int val = 0;

    setmintop(L, 3);
    to = check_pointer(L, 1, &ct);
    sz = (size_t) luaL_checknumber(L, 2);

    if (!lua_isnoneornil(L, 3)) {
        val = (int) luaL_checkinteger(L, 3);
    }

    memset(to, val, sz);
    return 0;
}

static int ffi_abi(lua_State* L)
{
    luaL_checkstring(L, 1);
    push_upval(L, &abi_key);
    lua_pushvalue(L, 1);
    lua_rawget(L, -2);
    lua_pushboolean(L, lua_toboolean(L, -1));
    return 1;
}

static int ffi_load(lua_State* L)
{
    const char* libname = luaL_checkstring(L, 1);
    void** lib = (void**) lua_newuserdata(L, sizeof(void*));

    *lib = LoadLibraryA(libname);

#ifdef LIB_FORMAT_1
    if (!*lib) {
        libname = lua_pushfstring(L, LIB_FORMAT_1, lua_tostring(L, 1));
        *lib = LoadLibraryA(libname);
        lua_pop(L, 1);
    }
#endif

#ifdef LIB_FORMAT_2
    if (!*lib) {
        libname = lua_pushfstring(L, LIB_FORMAT_2, lua_tostring(L, 1));
        *lib = LoadLibraryA(libname);
        lua_pop(L, 1);
    }
#endif

    if (!*lib) {
        return luaL_error(L, "could not load library %s", lua_tostring(L, 1));
    }

    lua_newtable(L);
    lua_setuservalue(L, -2);

    push_upval(L, &cmodule_mt_key);
    lua_setmetatable(L, -2);
    return 1;
}

static void* find_symbol(lua_State* L, int modidx, const char* asmname)
{
    size_t i;
    void** libs;
    size_t num;
    void* sym = NULL;

    libs = (void**) lua_touserdata(L, modidx);
    num = lua_rawlen(L, modidx) / sizeof(void*);

    for (i = 0; i < num && sym == NULL; i++) {
        if (libs[i]) {
            sym = GetProcAddressA(libs[i], asmname);
        }
    }

    return sym;
}

/* pushes the user table */
static void* lookup_global(lua_State* L, int modidx, int nameidx, const char** pname, struct ctype* ct)
{
    int top = lua_gettop(L);
    void* sym;

    modidx = lua_absindex(L, modidx);
    nameidx = lua_absindex(L, nameidx);

    *pname = luaL_checkstring(L, nameidx);

    /* get the ctype */
    push_upval(L, &functions_key);
    lua_pushvalue(L, nameidx);
    lua_rawget(L, -2);
    if (lua_isnil(L, -1)) {
        luaL_error(L, "missing declaration for function/global %s", *pname);
        return NULL;
    }

    /* leave just the ct_usr on the stack */
    *ct = *(const struct ctype*) lua_touserdata(L, -1);
    lua_getuservalue(L, -1);
    lua_replace(L, top + 1);
    lua_pop(L, 1);

    assert(lua_gettop(L) == top + 1);

    /* get the assembly name */
    push_upval(L, &asmname_key);
    lua_pushvalue(L, nameidx);
    lua_rawget(L, -2);
    if (lua_isstring(L, -1)) {
        *pname = lua_tostring(L, -1);
    }
    lua_pop(L, 2);

    sym = find_symbol(L, modidx, *pname);

    assert(lua_gettop(L) == top + 1);
    return sym;
}

static int cmodule_index(lua_State* L)
{
    const char* asmname;
    struct ctype ct;
    void *sym;

    lua_settop(L, 2);

    /* see if we have already loaded the function */
    lua_getuservalue(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) {
        return 1;
    }
    lua_pop(L, 2);

    /* check the constants table */
    push_upval(L, &constants_key);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) {
        return 1;
    }
    lua_pop(L, 2);

    /* lookup_global pushes the ct_usr */
    sym = lookup_global(L, 1, 2, &asmname, &ct);

#if defined _WIN32 && !defined _WIN64 && (defined __i386__ || defined _M_IX86)
    if (!sym && ct.type == FUNCTION_TYPE) {
        ct.calling_convention = STD_CALL;
        lua_pushfstring(L, "_%s@%d", asmname, x86_return_size(L, -1, &ct));
        sym = find_symbol(L, 1, lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    if (!sym && ct.type == FUNCTION_TYPE) {
        ct.calling_convention = FAST_CALL;
        lua_pushfstring(L, "@%s@%d", asmname, x86_return_size(L, -1, &ct));
        sym = find_symbol(L, 1, lua_tostring(L, -1));
        lua_pop(L, 1);
    }
#endif

    if (!sym) {
        return luaL_error(L, "failed to find function/global %s", asmname);
    }

    assert(lua_gettop(L) == 3); /* module, name, ct_usr */

    if (ct.type == FUNCTION_TYPE) {
        compile_function(L, (cfunction) sym, -1, &ct);
        assert(lua_gettop(L) == 4); /* module, name, ct_usr, function */

        /* set module usr value[luaname] = function to cache for next time */
        lua_getuservalue(L, 1);
        lua_pushvalue(L, 2);
        lua_pushvalue(L, -3);
        lua_rawset(L, -3);
        lua_pop(L, 1); /* module uv */
        return 1;
    }

    /* extern const char* foo; and extern const char foo[]; */
    if (ct.pointers == 1 && ct.type == INT8_TYPE) {
        char* str = (char*) sym;
        if (!ct.is_array) {
            str = *(char**) sym;
        }
        lua_pushstring(L, str);
        return 1;
    }

    /* extern struct foo foo[], extern void* foo[]; and extern struct foo foo; */
    if (ct.is_array || (!ct.pointers && (ct.type == UNION_TYPE || ct.type == STRUCT_TYPE))) {
        void* p;
        ct.is_reference = 1;
        p = push_cdata(L, -1, &ct);
        *(void**) p = sym;
        return 1;
    }

    /* extern void* foo; and extern void (*foo)(); */
    if (ct.pointers || ct.type == FUNCTION_PTR_TYPE) {
        void* p = push_cdata(L, -1, &ct);
        *(void**) p = *(void**) sym;
        return 1;
    }

    switch (ct.type) {
    case COMPLEX_DOUBLE_TYPE:
    case COMPLEX_FLOAT_TYPE:
    case INTPTR_TYPE:
    case INT64_TYPE:
        {
            /* TODO: complex float/double need to be references if .re and
             * .imag are setable */
            void* p = push_cdata(L, -1, &ct);
            memcpy(p, sym, ct.base_size);
            return 1;
        }

    case DOUBLE_TYPE:
        lua_pushnumber(L, *(double*) sym);
        return 1;

    case FLOAT_TYPE:
        lua_pushnumber(L, *(float*) sym);
        return 1;

    case BOOL_TYPE:
        lua_pushboolean(L, *(bool*) sym);
        return 1;

    case INT8_TYPE:
        lua_pushnumber(L, ct.is_unsigned ? (lua_Number) *(uint8_t*) sym : (lua_Number) *(int8_t*) sym);
        return 1;

    case INT16_TYPE:
        lua_pushnumber(L, ct.is_unsigned ? (lua_Number) *(uint16_t*) sym : (lua_Number) *(int16_t*) sym);
        return 1;

    case INT32_TYPE:
    case ENUM_TYPE:
        lua_pushnumber(L, ct.is_unsigned ? (lua_Number) *(uint32_t*) sym : (lua_Number) *(int32_t*) sym);
        return 1;
    }

    return luaL_error(L, "NYI - global value type");
}

static int cmodule_newindex(lua_State* L)
{
    const char* name;
    void* sym;
    struct ctype ct;

    lua_settop(L, 3);

    /* pushes the ct_usr */
    sym = lookup_global(L, 1, 2, &name, &ct);
    assert(lua_gettop(L) == 4); /* module, name, value, ct_usr */

    if (sym == NULL) {
        return luaL_error(L, "failed to find global %s", name);
    }

    if (ct.type == FUNCTION_TYPE || ct.is_array || (ct.const_mask & 1)) {
        return luaL_error(L, "can not set global %s", name);
    }

    set_value(L, 3, sym, -1, &ct, 1);
    return 0;
}

static int jit_gc(lua_State* L)
{
    size_t i;
    struct jit* jit = get_jit(L);
    dasm_free(jit);
    for (i = 0; i < jit->pagenum; i++) {
        FreePage(jit->pages[i], jit->pages[i]->size);
    }
    free(jit->globals);
    return 0;
}

static int ffi_debug(lua_State* L)
{
    lua_newtable(L);
    push_upval(L, &ctype_mt_key);
    lua_setfield(L, -2, "ctype_mt");
    push_upval(L, &cdata_mt_key);
    lua_setfield(L, -2, "cdata_mt");
    push_upval(L, &cmodule_mt_key);
    lua_setfield(L, -2, "cmodule_mt");
    push_upval(L, &constants_key);
    lua_setfield(L, -2, "constants");
    push_upval(L, &types_key);
    lua_setfield(L, -2, "types");
    push_upval(L, &jit_key);
    lua_setfield(L, -2, "jit");
    push_upval(L, &gc_key);
    lua_setfield(L, -2, "gc");
    push_upval(L, &callbacks_key);
    lua_setfield(L, -2, "callbacks");
    push_upval(L, &functions_key);
    lua_setfield(L, -2, "functions");
    push_upval(L, &abi_key);
    lua_setfield(L, -2, "abi");
    push_upval(L, &next_unnamed_key);
    lua_setfield(L, -2, "next_unnamed");
    return 1;
}

static int do64(lua_State* L, int is_unsigned)
{
    lua_Number low, high;
    struct ctype ct;
    int64_t val;

    lua_settop(L, 2);

    if (!lua_isnil(L, 2)) {
        high = luaL_checknumber(L, 1);
        low = luaL_checknumber(L, 2);
    } else {
        high = 0;
        low = luaL_checknumber(L, 1);
    }

    val = ((int64_t) (uint32_t) high << 32) | (int64_t) (uint32_t) low;

    if (!is_unsigned && (high < 0 || low < 0)) {
        val = -val;
    }

    memset(&ct, 0, sizeof(ct));
    ct.type = INT64_TYPE;
    ct.is_unsigned = is_unsigned;
    ct.is_defined = 1;
    ct.base_size = sizeof(int64_t);
    push_number(L, (int64_t) val, 0, &ct);

    return 1;
}

static int ffi_i64(lua_State* L)
{ return do64(L, 0); }

static int ffi_u64(lua_State* L)
{ return do64(L, 1); }

static const luaL_Reg cdata_mt[] = {
    {"__gc", &cdata_gc},
    {"__call", &cdata_call},
    {"free", &cdata_free},
    {"set", &cdata_set},
    {"__index", &cdata_index},
    {"__newindex", &cdata_newindex},
    {"__add", &cdata_add},
    {"__sub", &cdata_sub},
    {"__mul", &cdata_mul},
    {"__div", &cdata_div},
    {"__mod", &cdata_mod},
    {"__pow", &cdata_pow},
    {"__unm", &cdata_unm},
    {"__eq", &cdata_eq},
    {"__lt", &cdata_lt},
    {"__le", &cdata_le},
    {"__tostring", &cdata_tostring},
    {"__concat", &cdata_concat},
    {"__len", &cdata_len},
    {"__pairs", &cdata_pairs},
    {"__ipairs", &cdata_ipairs},
    {NULL, NULL}
};

static const luaL_Reg callback_mt[] = {
    {"__gc", &callback_free},
    {NULL, NULL}
};

static const luaL_Reg ctype_mt[] = {
    {"__call", &ctype_call},
    {"__new", &ctype_new},
    {"__tostring", &ctype_tostring},
    {NULL, NULL}
};

static const luaL_Reg cmodule_mt[] = {
    {"__index", &cmodule_index},
    {"__newindex", &cmodule_newindex},
    {NULL, NULL}
};

static const luaL_Reg jit_mt[] = {
    {"__gc", &jit_gc},
    {NULL, NULL}
};

static const luaL_Reg ffi_reg[] = {
    {"cdef", &ffi_cdef},
    {"load", &ffi_load},
    {"new", &ffi_new},
    {"typeof", &ffi_typeof},
    {"cast", &ffi_cast},
    {"metatype", &ffi_metatype},
    {"gc", &ffi_gc},
    {"sizeof", &ffi_sizeof},
    {"alignof", &ffi_alignof},
    {"offsetof", &ffi_offsetof},
    {"istype", &ffi_istype},
    {"errno", &ffi_errno},
    {"string", &ffi_string},
    {"copy", &ffi_copy},
    {"fill", &ffi_fill},
    {"abi", &ffi_abi},
    {"debug", &ffi_debug},
    {"i64", &ffi_i64},
    {"u64", &ffi_u64},
    {NULL, NULL}
};

/* leaves the usr table on the stack */
static void push_builtin(lua_State* L, struct ctype* ct, const char* name, int type, int size, int align, int is_unsigned)
{
    memset(ct, 0, sizeof(*ct));
    ct->type = type;
    ct->base_size = size;
    ct->align_mask = align;
    ct->is_defined = 1;
    ct->is_unsigned = is_unsigned;

    push_upval(L, &types_key);
    push_ctype(L, 0, ct);
    lua_setfield(L, -2, name);
    lua_pop(L, 1); /* types */
}

static void push_builtin_undef(lua_State* L, struct ctype* ct, const char* name, int type)
{
    memset(ct, 0, sizeof(*ct));
    ct->type = type;

    push_upval(L, &types_key);
    push_ctype(L, 0, ct);
    lua_setfield(L, -2, name);
    lua_pop(L, 1); /* types */
}

static void add_typedef(lua_State* L, const char* from, const char* to)
{
    struct ctype ct;
    struct parser P;
    P.line = 1;
    P.align_mask = DEFAULT_ALIGN_MASK;
    P.next = P.prev = from;

    push_upval(L, &types_key);
    parse_type(L, &P, &ct);
    parse_argument(L, &P, -1, &ct, NULL, NULL);
    push_ctype(L, -1, &ct);

    /* stack is at +4: types, type usr, arg usr, ctype */

    lua_setfield(L, -4, to);
    lua_pop(L, 3); /* types, type usr, arg usr */
}

static int setup_upvals(lua_State* L)
{
    struct jit* jit = get_jit(L);

    /* jit setup */
    {
        dasm_init(jit, 64);
#ifdef _WIN32
        {
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            jit->align_page_size = si.dwAllocationGranularity - 1;
        }
#else
        jit->align_page_size = sysconf(_SC_PAGE_SIZE) - 1;
#endif
        jit->globals = (void**) malloc(64 * sizeof(void*));
        dasm_setupglobal(jit, jit->globals, 64);
        compile_globals(jit, L);
    }

    /* ffi.C */
    {
#ifdef _WIN32
        size_t sz = sizeof(HMODULE) * 6;
        HMODULE* libs = lua_newuserdata(L, sz);
        memset(libs, 0, sz);

        /* exe */
        GetModuleHandle(NULL);
        /* lua dll */
#ifdef LUA_DLL_NAME
#define STR2(tok) #tok
#define STR(tok) STR2(tok)
        libs[1] = LoadLibraryA(STR(LUA_DLL_NAME));
#undef STR
#undef STR2
#endif

        /* crt */
#ifdef UNDER_CE
        libs[2] = LoadLibraryA("coredll.dll");
#else
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (char*) &_fmode, &libs[2]);
        libs[3] = LoadLibraryA("kernel32.dll");
        libs[4] = LoadLibraryA("user32.dll");
        libs[5] = LoadLibraryA("gdi32.dll");
#endif

        jit->lua_dll = libs[1];
        jit->kernel32_dll = libs[3];

#else /* !_WIN32 */
        size_t sz = sizeof(void*) * 5;
        void** libs = lua_newuserdata(L, sz);
        memset(libs, 0, sz);

        libs[0] = LoadLibraryA(NULL); /* exe */
        libs[1] = LoadLibraryA("libc.so");
#ifdef __GNUC__
        libs[2] = LoadLibraryA("libgcc.so");
#endif
        libs[3] = LoadLibraryA("libm.so");
        libs[4] = LoadLibraryA("libdl.so");
#endif

        lua_newtable(L);
        lua_setuservalue(L, -2);

        push_upval(L, &cmodule_mt_key);
        lua_setmetatable(L, -2);

        lua_setfield(L, 1, "C");
    }

    /* setup builtin types */
    {
        complex_double* pc;
        struct {char ch; uint16_t v;} a16;
        struct {char ch; uint32_t v;} a32;
        struct {char ch; uint64_t v;} a64;
        struct {char ch; float v;} af;
        struct {char ch; double v;} ad;
#ifdef HAVE_LONG_DOUBLE
        struct {char ch; long double v;} ald;
#endif
        struct {char ch; uintptr_t v;} aptr;
        struct ctype ct;
        struct {char ch; complex_float v;} cf;
        struct {char ch; complex_double v;} cd;
#if defined HAVE_LONG_DOUBLE && defined HAVE_COMPLEX
        struct {char ch; complex long double v;} cld;
#endif

        push_builtin(L, &ct, "void", VOID_TYPE, 0, 0, 0);
        push_builtin(L, &ct, "bool", BOOL_TYPE, sizeof(_Bool), sizeof(_Bool) -1, 1);
        push_builtin(L, &ct, "uint8_t", INT8_TYPE, sizeof(uint8_t), 0, 1);
        push_builtin(L, &ct, "int8_t", INT8_TYPE, sizeof(int8_t), 0, 0);
        push_builtin(L, &ct, "uint16_t", INT16_TYPE, sizeof(uint16_t), ALIGNOF(a16), 1);
        push_builtin(L, &ct, "int16_t", INT16_TYPE, sizeof(int16_t), ALIGNOF(a16), 0);
        push_builtin(L, &ct, "uint32_t", INT32_TYPE, sizeof(uint32_t), ALIGNOF(a32), 1);
        push_builtin(L, &ct, "int32_t", INT32_TYPE, sizeof(int32_t), ALIGNOF(a32), 0);
        push_builtin(L, &ct, "uint64_t", INT64_TYPE, sizeof(uint64_t), ALIGNOF(a64), 1);
        push_builtin(L, &ct, "int64_t", INT64_TYPE, sizeof(int64_t), ALIGNOF(a64), 0);
        push_builtin(L, &ct, "float", FLOAT_TYPE, sizeof(float), ALIGNOF(af), 0);
        push_builtin(L, &ct, "double", DOUBLE_TYPE, sizeof(double), ALIGNOF(ad), 0);
#ifdef HAVE_LONG_DOUBLE
        push_builtin(L, &ct, "long double", LONG_DOUBLE_TYPE, sizeof(long double), ALIGNOF(ald), 0);
#else
        push_builtin_undef(L, &ct, "long double", LONG_DOUBLE_TYPE);
#endif
        push_builtin(L, &ct, "uintptr_t", INTPTR_TYPE, sizeof(uintptr_t), ALIGNOF(aptr), 1);
        push_builtin(L, &ct, "intptr_t", INTPTR_TYPE, sizeof(uintptr_t), ALIGNOF(aptr), 0);
        push_builtin(L, &ct, "complex float", COMPLEX_FLOAT_TYPE, sizeof(complex_float), ALIGNOF(cf), 0);
        push_builtin(L, &ct, "complex double", COMPLEX_DOUBLE_TYPE, sizeof(complex_double), ALIGNOF(cd), 0);
#if defined HAVE_LONG_DOUBLE && defined HAVE_COMPLEX
        push_builtin(L, &ct, "complex long double", COMPLEX_LONG_DOUBLE_TYPE, sizeof(complex long double), ALIGNOF(cld), 0);
#else
        push_builtin_undef(L, &ct, "complex long double", COMPLEX_LONG_DOUBLE_TYPE);
#endif

        /* add NULL and i constants */
        push_upval(L, &constants_key);

        memset(&ct, 0, sizeof(ct));
        ct.type = VOID_TYPE;
        ct.is_defined = 1;
        ct.pointers = 1;
        ct.is_null = 1;

        push_cdata(L, 0, &ct);
        lua_setfield(L, -2, "NULL");

        memset(&ct, 0, sizeof(ct));
        ct.type = COMPLEX_DOUBLE_TYPE;
        ct.is_defined = 1;
        ct.base_size = sizeof(complex_double);
        pc = (complex_double*) push_cdata(L, 0, &ct);
#ifdef HAVE_COMPLEX
        *pc = 1i;
#else
        pc->real = 0;
        pc->imag = 1;
#endif
        lua_setfield(L, -2, "i");

        lua_pop(L, 1); /* constants */
    }

    assert(lua_gettop(L) == 1);

    /* setup builtin typedefs */
    {
        add_typedef(L, "bool", "_Bool");

        if (sizeof(uint32_t) == sizeof(size_t)) {
            add_typedef(L, "uint32_t", "size_t");
            add_typedef(L, "int32_t", "ssize_t");
        } else if (sizeof(uint64_t) == sizeof(size_t)) {
            add_typedef(L, "uint64_t", "size_t");
            add_typedef(L, "int64_t", "ssize_t");
        }

        if (sizeof(int32_t) == sizeof(intptr_t)) {
            add_typedef(L, "int32_t", "intptr_t");
            add_typedef(L, "int32_t", "ptrdiff_t");
        } else if (sizeof(int64_t) == sizeof(intptr_t)) {
            add_typedef(L, "int64_t", "intptr_t");
            add_typedef(L, "int64_t", "ptrdiff_t");
        }

        if (sizeof(uint8_t) == sizeof(wchar_t)) {
            add_typedef(L, "uint8_t", "wchar_t");
        } else if (sizeof(uint16_t) == sizeof(wchar_t)) {
            add_typedef(L, "uint16_t", "wchar_t");
        } else if (sizeof(uint32_t) == sizeof(wchar_t)) {
            add_typedef(L, "uint32_t", "wchar_t");
        }

        if (sizeof(va_list) == sizeof(char*)) {
            add_typedef(L, "char*", "va_list");
        } else {
            struct {char ch; va_list v;} av;
            lua_pushfstring(L, "struct {char data[%d] __attribute__((align(%d)));}", (int) sizeof(va_list), (int) ALIGNOF(av) + 1);
            add_typedef(L, lua_tostring(L, -1), "va_list");
            lua_pop(L, 1);
        }

        add_typedef(L, "va_list", "__builtin_va_list");
        add_typedef(L, "va_list", "__gnuc_va_list");
    }

    assert(lua_gettop(L) == 1);

    /* setup ABI params table */
    push_upval(L, &abi_key);

#if defined ARCH_X86 || defined ARCH_ARM
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "32bit");
#elif defined ARCH_X64
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "64bit");
#else
#error
#endif

#if defined ARCH_X86 || defined ARCH_X64 || defined ARCH_ARM
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "le");
#else
#error
#endif

#if defined ARCH_X86 || defined ARCH_X64
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "fpu");
#elif defined ARCH_ARM
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "softfp");
#else
#error
#endif
    lua_pop(L, 1); /* abi tbl */


    /* GC table - shouldn't pin cdata values */
    push_upval(L, &gc_key);
    lua_newtable(L);
    lua_pushliteral(L, "k");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L, -2);
    lua_pop(L, 1); /* gc table */


    /* ffi.os */
#if defined OS_CE
    lua_pushliteral(L, "WindowsCE");
#elif defined OS_WIN
    lua_pushliteral(L, "Windows");
#elif defined OS_OSX
    lua_pushliteral(L, "OSX");
#elif defined OS_LINUX
    lua_pushliteral(L, "Linux");
#elif defined OS_BSD
    lua_pushliteral(L, "BSD");
#elif defined OS_POSIX
    lua_pushliteral(L, "POSIX");
#else
    lua_pushliteral(L, "Other");
#endif
    lua_setfield(L, 1, "os");


    /* ffi.arch */
#if defined ARCH_X86
    lua_pushliteral(L, "x86");
#elif defined ARCH_X64
    lua_pushliteral(L, "x64");
#elif defined ARCH_ARM
    lua_pushliteral(L, "arm");
#else
# error
#endif
    lua_setfield(L, 1, "arch");

    assert(lua_gettop(L) == 1);

    return 0;
}

static void setup_mt(lua_State* L, const luaL_Reg* mt, int upvals)
{
    lua_pushboolean(L, 1);
    lua_setfield(L, -upvals-2, "__metatable");
    luaL_setfuncs(L, mt, upvals);
}

int luaopen_ffi(lua_State* L)
{
    lua_settop(L, 0);

    lua_newtable(L);
    set_upval(L, &niluv_key);

    lua_newtable(L);
    setup_mt(L, ctype_mt, 0);
    set_upval(L, &ctype_mt_key);

    lua_newtable(L);
    set_upval(L, &callbacks_key);

    lua_newtable(L);
    set_upval(L, &gc_key);

    lua_newtable(L);
    push_upval(L, &callbacks_key);
    push_upval(L, &gc_key);
    setup_mt(L, cdata_mt, 2);
    set_upval(L, &cdata_mt_key);

    lua_newtable(L);
    setup_mt(L, callback_mt, 0);
    set_upval(L, &callback_mt_key);

    lua_newtable(L);
    setup_mt(L, cmodule_mt, 0);
    set_upval(L, &cmodule_mt_key);

    memset(lua_newuserdata(L, sizeof(struct jit)), 0, sizeof(struct jit));
    lua_newtable(L);
    setup_mt(L, jit_mt, 0);
    lua_setmetatable(L, -2);
    set_upval(L, &jit_key);

    lua_newtable(L);
    set_upval(L, &constants_key);

    lua_newtable(L);
    set_upval(L, &types_key);

    lua_newtable(L);
    set_upval(L, &functions_key);

    lua_newtable(L);
    set_upval(L, &asmname_key);

    lua_newtable(L);
    set_upval(L, &abi_key);

    lua_pushinteger(L, 1);
    set_upval(L, &next_unnamed_key);

    assert(lua_gettop(L) == 0);

    /* ffi table */
    lua_newtable(L);
    luaL_setfuncs(L, ffi_reg, 0);

    /* setup_upvals(ffi tbl) */
    lua_pushcfunction(L, &setup_upvals);
    lua_pushvalue(L, 1);
    lua_call(L, 1, 0);

    assert(lua_gettop(L) == 1);

    lua_getglobal(L, "tonumber");
    lua_pushcclosure(L, &ffi_number, 1);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "tonumber");
    lua_setfield(L, -2, "number"); /* ffi.number */

    return 1;
}
 
#endif 
 
