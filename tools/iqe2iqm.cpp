/*
Copyright (c) 2010-2019 Lee Salzman
https://github.com/lsalzman/iqm/

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


// includes minor patches by rlyeh (public domain).
// - Make static meshes generate bounding boxes as well
// - Removed fbx,smd,obj,md5mesh,md5anim support
// - Compiling in VC with 'cl /MT' crashed often. Using '/MD' seemed to be safer
// - So, I increased the vector growsize & removed all deallocations to "fix" random crashes
// - Not quite sure still, so I also removed custom allocators
// - Also, switched string hashing to fnv1a, hoping to distribute hashmap buckets more uniformly
// - Note: crashes behaves worse on x64. x86 seems to be more predictable.

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <assert.h>
#include <vector>
#include <stdint.h>
#define _CRT_SECURE_NO_WARNINGS 1
#define ASSERT(x) do{ if(!(x)) exit(-fprintf(stderr, "assertion failed: `" #x "` %s:%d\n%s\n", __FILE__, __LINE__, callstack(+16))); } while(0) // assert(x)
//#define ASSERT(x) do{}while(0) // hack
#define delete_array (0),(void) // hack to minimize crashes
//#define delete_array delete[]

#ifdef _WIN32 // && !defined __TINYC__
#define SYS_MEM_REALLOC realloc
#define __thread __declspec(thread)
#define concat(a,b)      conc4t(a,b)
#define conc4t(a,b)      a##b ///-
#define macro(name)      concat(name, __LINE__)
#define do_once static int macro(init) = 1; for(;macro(init);macro(init) = 0)
#include <winsock2.h>  // windows.h alternative
#include <dbghelp.h>
#pragma comment(lib, "DbgHelp")
#pragma comment(lib, "Kernel32")
static int backtrace( void **addr, int maxtraces ) {
    static bool init = 0;
    do_once SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES);
    do_once init = SymInitialize(GetCurrentProcess(), NULL, TRUE);
    if(!init) return 0; // error: cannot initialize DbgHelp.lib

    typedef USHORT (WINAPI *pFN)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG); // _MSC_VER
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
        symbols = (char**)SYS_MEM_REALLOC(0, 128 * sizeof(char*));
        for( int i = 0; i < 128; ++i) symbols[i] = (char*)SYS_MEM_REALLOC(0, 128 * sizeof(char));
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
char *callstack( int traces ) {
    static __thread char *output = 0;
    if(!output ) output = (char*)SYS_MEM_REALLOC( 0, 128 * (64+2) );
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
        ptr += sprintf(ptr, "%03d: %#016llx %s\n", ++L, (unsigned long long)(uintptr_t)stacks[i], symbols[i]); // format gymnastics because %p is not standard when printing pointers
    }

     return output ? output : "";
}
#endif


#ifndef __IQM_H__
#define __IQM_H__

#define IQM_MAGIC "INTERQUAKEMODEL"
#define IQM_VERSION 2

struct iqmheader
{
    char magic[16];
    unsigned int version;
    unsigned int filesize;
    unsigned int flags;
    unsigned int num_text, ofs_text;
    unsigned int num_meshes, ofs_meshes;
    unsigned int num_vertexarrays, num_vertexes, ofs_vertexarrays;
    unsigned int num_triangles, ofs_triangles, ofs_adjacency;
    unsigned int num_joints, ofs_joints;
    unsigned int num_poses, ofs_poses;
    unsigned int num_anims, ofs_anims;
    unsigned int num_frames, num_framechannels, ofs_frames, ofs_bounds;
    unsigned int num_comment, ofs_comment;
    unsigned int num_extensions, ofs_extensions;
};

struct iqmmesh
{
    unsigned int name;
    unsigned int material;
    unsigned int first_vertex, num_vertexes;
    unsigned int first_triangle, num_triangles;
};

enum
{
    IQM_POSITION     = 0,
    IQM_TEXCOORD     = 1,
    IQM_NORMAL       = 2,
    IQM_TANGENT      = 3,
    IQM_BLENDINDEXES = 4,
    IQM_BLENDWEIGHTS = 5,
    IQM_COLOR        = 6,
    IQM_CUSTOM       = 0x10
};

enum
{
    IQM_BYTE   = 0,
    IQM_UBYTE  = 1,
    IQM_SHORT  = 2,
    IQM_USHORT = 3,
    IQM_INT    = 4,
    IQM_UINT   = 5,
    IQM_HALF   = 6,
    IQM_FLOAT  = 7,
    IQM_DOUBLE = 8
};

struct iqmtriangle
{
    unsigned int vertex[3];
};

struct iqmadjacency
{
    unsigned int triangle[3];
};

struct iqmjointv1
{
    unsigned int name;
    int parent;
    float translate[3], rotate[3], scale[3];
};

struct iqmjoint
{
    unsigned int name;
    int parent;
    float translate[3], rotate[4], scale[3];
};

struct iqmposev1
{
    int parent;
    unsigned int mask;
    float channeloffset[9];
    float channelscale[9];
};

struct iqmpose
{
    int parent;
    unsigned int mask;
    float channeloffset[10];
    float channelscale[10];
};

struct iqmanim
{
    unsigned int name;
    unsigned int first_frame, num_frames;
    float framerate;
    unsigned int flags;
};

enum
{
    IQM_LOOP = 1<<0
};

struct iqmvertexarray
{
    unsigned int type;
    unsigned int flags;
    unsigned int format;
    unsigned int size;
    unsigned int offset;
};

struct iqmbounds
{
    float bbmin[3], bbmax[3];
    float xyradius, radius;
};

struct iqmextension
{
    unsigned int name;
    unsigned int num_data, ofs_data;
    unsigned int ofs_extensions; // pointer to next extension
};

#endif




#ifdef NULL
#undef NULL
#endif
#define NULL 0

#ifdef _WIN32
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
#define strcasecmp _stricmp
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef signed long long int llong;
typedef unsigned long long int ullong;

#if 0
inline void *operator new(size_t size)
{
    void *p = malloc(size);
    if(!p) abort();
    return p;
}
inline void *operator new[](size_t size)
{
    void *p = malloc(size);
    if(!p) abort();
    return p;
}
inline void operator delete(void *p) { if(p) free(p); }
inline void operator delete[](void *p) { if(p) free(p); }

inline void *operator new(size_t, void *p) { return p; }
inline void *operator new[](size_t, void *p) { return p; }
inline void operator delete(void *, void *) {}
inline void operator delete[](void *, void *) {}
#endif

#ifdef swap
#undef swap
#endif
template<class T>
static inline void swap(T &a, T &b)
{
    T t = a;
    a = b;
    b = t;
}
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
template<class T>
static inline T max(T a, T b)
{
    return a > b ? a : b;
}
template<class T>
static inline T min(T a, T b)
{
    return a < b ? a : b;
}

#define clamp(a,b,c) (max(b, min(a, c)))

#define loop(v,m) for(int v = 0; v<int(m); v++)
#define loopi(m) loop(i,m)
#define loopj(m) loop(j,m)
#define loopk(m) loop(k,m)
#define loopl(m) loop(l,m)

#ifdef WIN32
#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.14159265

#ifndef __GNUC__
#pragma warning (3: 4189)       // local variable is initialized but not referenced
#pragma warning (disable: 4244) // conversion from 'int' to 'float', possible loss of data
#pragma warning (disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning (disable: 4355) // 'this' : used in base member initializer list
#pragma warning (disable: 4996) // 'strncpy' was declared deprecated
#endif

#define strcasecmp _stricmp
#define PATHDIV '\\'
#else
#define __cdecl
#define _vsnprintf vsnprintf
#define PATHDIV '/'
#endif

// easy safe strings

#define MAXSTRLEN 260
typedef char string[MAXSTRLEN];

inline void vformatstring(char *d, const char *fmt, va_list v, int len = MAXSTRLEN) { _vsnprintf(d, len, fmt, v); d[len-1] = 0; }
inline char *copystring(char *d, const char *s, size_t len = MAXSTRLEN)
{
    size_t slen = min(strlen(s)+1, len);
    memcpy(d, s, slen);
    d[slen-1] = 0;
    return d;
}
inline char *concatstring(char *d, const char *s) { size_t len = strlen(d); return copystring(d+len, s, MAXSTRLEN-len); }

template<size_t N> inline void formatstring(char (&d)[N], const char *fmt, ...)
{
    va_list v;
    va_start(v, fmt);
    vformatstring(d, fmt, v, int(N));
    va_end(v);
}

#define defformatstring(d,...) string d; formatstring(d, __VA_ARGS__)
#define defvformatstring(d,last,fmt) string d; { va_list ap; va_start(ap, last); vformatstring(d, fmt, ap); va_end(ap); }

inline char *newstring(size_t l)                { return new char[l+1]; }
inline char *newstring(const char *s, size_t l) { return copystring(newstring(l), s, l+1); }
inline char *newstring(const char *s)           { size_t l = strlen(s); char *d = newstring(l); memcpy(d, s, l+1); return d; }

#define loopv(v)    for(int i = 0; i<(v).length(); i++)
#define loopvj(v)   for(int j = 0; j<(v).length(); j++)
#define loopvk(v)   for(int k = 0; k<(v).length(); k++)
#define loopvrev(v) for(int i = (v).length()-1; i>=0; i--)

template <class T> struct vector : public std::vector<T>
{
    static const int MINSIZE = 8;

mutable //< @r-lyeh
    T *buf;
mutable //< @r-lyeh
    int alen, ulen;

    vector() : buf(NULL), alen(0), ulen(0)
    {
    }

    vector(const vector &v) : buf(NULL), alen(0), ulen(0)
    {
        *this = v;
    }

    ~vector() { setsize(0); if(buf) delete_array (uchar *)buf; }

    vector<T> &operator=(const vector<T> &v)
    {
        setsize(0);
        if(v.length() > alen) growbuf(v.length());
        loopv(v) add(v[i]);

        if(ulen==alen) growbuf(ulen+1); //< @r-lyeh

        return *this;
    }

    T &add(const T &x)
    {
        if(ulen==alen) growbuf(ulen+1);
        new (&buf[ulen]) T(x);
        return buf[ulen++];
    }

    T &add()
    {
        if(ulen==alen) growbuf(ulen+1);
        new (&buf[ulen]) T;
        return buf[ulen++];
    }

    T &dup()
    {
        if(ulen==alen) growbuf(ulen+1);
        new (&buf[ulen]) T(buf[ulen-1]);
        return buf[ulen++];
    }

    bool inrange(uint i) const { return i<uint(ulen); }
    bool inrange(int i) const { return i>=0 && i<ulen; }

    T &pop() { return buf[--ulen]; }
    T &last() { return buf[ulen-1]; }
    void drop() { ulen--; buf[ulen].~T(); }

    bool empty() const { return ulen==0; }
    int capacity() const { return alen; }
    int length() const { return ulen; }
    T &operator[](int i) {
        if(i<ulen) reserve(i+1); //< @r-lyeh
        //ASSERT(i>=0 && i<ulen);
        return buf[i]; }
    const T &operator[](int i) const {
        if(i<ulen) reserve(i+1); //< @r-lyeh
        //ASSERT(i >= 0 && i<ulen);
        return buf[i]; }

    void setsize(int i) { ASSERT(i <= ulen); ulen = i; }

    void swap(vector<T> &v)
    {
        ::swap(buf, v.buf);
        ::swap(ulen, v.ulen);
        ::swap(alen, v.alen);
    }

    T *getbuf() { return buf; }
    const T *getbuf() const { return buf; }
    bool inbuf(const T *e) const { return e >= buf && e < &buf[ulen]; }

    void growbuf(int sz)
const //< @r-lyeh
    {
        int olen = alen;
        if(!alen) alen = max(MINSIZE, sz);
        else while(alen < sz) alen *= 2;
        if(alen <= olen) return;
        uchar *newbuf = new uchar[(alen*10)*sizeof(T)]; //< @rlyeh +1
        if(olen > 0)
        {
            memcpy(newbuf, buf, olen*sizeof(T));
            delete_array (uchar *)buf;
        }
        buf = (T *)newbuf;
    }

    T *reserve(int sz)
const //< @r-lyeh
    {
        if(ulen+sz > alen) growbuf(ulen+sz);
        return &buf[ulen];
    }

    void advance(int sz)
    {
        ulen += sz;
    }

    void put(const T *v, int n)
    {
        memcpy(reserve(n), v, n*sizeof(T));
        advance(n);
    }
};

static inline uint htcombine(uint h1, uint h2)
{
    return h1 ^ (h2 + 0x9E3779B9 + (h1 << 6) + (h1 >> 2));
}

static inline uint hthash(const char *key)
{
#if 0
    uint h = 5381;
    for(int i = 0, k; (k = key[i]); i++) h = ((h<<5)+h)^k;    // bernstein k=33 xor
    return h;
#else //< @r-lyeh: different hashmap distribution for buckets
    uint64_t hash = 14695981039346656037ULL;
    while( *key ) {
        hash = ( (unsigned char)*key++ ^ hash ) * 0x100000001b3ULL;
    }
    return (uint)hash;
#endif
}

static inline bool htcmp(const char *x, const char *y)
{
    return !strcmp(x, y);
}

// https://burtleburtle.net/bob/hash/integer.html
static inline uint hthash(uint k)
{
    k = (k ^ 61) ^ (k >> 16);
    k = k + (k << 3);
    k = k ^ (k >> 4);
    k = k * 0x27d4eb2d;
    k = k ^ (k >> 15);
    return k;
}

static inline uint hthash(ullong k)
{
    k = (~k) + (k << 21);
    k = k ^ (k >> 24);
    k = (k + (k << 3)) + (k << 8);
    k = k ^ (k >> 14);
    k = (k + (k << 2)) + (k << 4);
    k = k ^ (k << 28);
    k = k + (k << 31);
    return k;
}

static inline uint hthash(int k)
{
    return hthash(uint(k));
}

static inline bool htcmp(int x, int y)
{
    return x==y;
}

static inline bool htcmp(double x, double y)
{
    return x == y;
}

static inline uint hthash(double k)
{
    union { double d; ullong u; } conv;
    conv.d = k;
    return hthash(conv.u);
}

template <class K, class T> struct hashtable
{
    typedef K key;
    typedef const K const_key;
    typedef T value;
    typedef const T const_value;

    enum { CHUNKSIZE = 64, MAXLOADFACTOR = 75, RESIZERATIO = 4, MAXSIZE = 128<<20 };

    struct chain      { T data; K key; chain *next; };
    struct chainchunk { chain chains[CHUNKSIZE]; chainchunk *next; };

    int size;
    int numelems;
    chain **table;

    chainchunk *chunks;
    chain *unused;

    hashtable(int size = 1<<10)
      : size(size)
    {
        numelems = 0;
        chunks = NULL;
        unused = NULL;
        table = new chain*[size];
        loopi(size) table[i] = NULL;
    }

    ~hashtable()
    {
        if(table) delete_array table;
        deletechunks();
    }

    chain *insert(const K &key, uint h)
    {
        if(size*RESIZERATIO < MAXSIZE && float(++numelems) / size * 100.0 > MAXLOADFACTOR) { rehash(); h = hthash(key)&(size-1); }
        return insert(unused, chunks, table, key, h);
    }

    static chain *insert(chain *&unused, chainchunk *&chunks, chain **&table, const K& key, uint h)
    {
        if(!unused)
        {
            chainchunk *chunk = new chainchunk;
            chunk->next = chunks;
            chunks = chunk;
            loopi(CHUNKSIZE-1) chunk->chains[i].next = &chunk->chains[i+1];
            chunk->chains[CHUNKSIZE-1].next = unused;
            unused = chunk->chains;
        }
        chain *c = unused;
        unused = unused->next;
        c->key = key;
        c->next = table[h];
        table[h] = c;
        return c;
    }

    #define HTFIND(success, fail) \
        uint h = hthash(key)&(size-1); \
        for(chain *c = table[h]; c; c = c->next) \
        { \
            if(htcmp(key, c->key)) return (success); \
        } \
        return (fail);

    template<class L>
    T *access(const L &key)
    {
        HTFIND(&c->data, NULL);
    }

    template<class L>
    T &access(const L &key, const T &data)
    {
        HTFIND(c->data, insert(key, h)->data = data);
    }

    template<class L>
    const T &find(const L &key, const T &notfound)
    {
        HTFIND(c->data, notfound);
    }

    template<class L>
    T &operator[](const L &key)
    {
        HTFIND(c->data, insert(key, h)->data);
    }

    #undef HTFIND

    template<class L>
    bool remove(const L &key)
    {
        uint h = hthash(key)&(size-1);
        for(chain **p = &table[h], *c = table[h]; c; p = &c->next, c = c->next)
        {
            if(htcmp(key, c->key))
            {
                *p = c->next;
                c->data.~T();
                c->key.~K();
                new (&c->data) T;
                new (&c->key) K;
                c->next = unused;
                unused = c;
                numelems--;
                return true;
            }
        }
        return false;
    }

    void deletechunks()
    {
        for(chainchunk *nextchunk; chunks; chunks = nextchunk)
        {
            nextchunk = chunks->next;
            delete chunks;
        }
    }

    void clear()
    {
        if(!numelems) return;
        loopi(size) table[i] = NULL;
        numelems = 0;
        unused = NULL;
        deletechunks();
    }

    void rehash()
    {
        int newsize = size*RESIZERATIO;
        chain **newtable = new chain* [newsize];
        loopi(newsize) newtable[i] = NULL;

        chainchunk *newchunks = NULL;
        chain *newunused = NULL;
        loopi(size) for (chain *c = table[i]; c;)
        {
            const K& k = c->key;
            uint h = hthash(k)&(newsize-1);
            insert(newunused, newchunks, newtable, k, h)->data = c->data;
            c = c->next;
        }

        delete[] table;
        deletechunks();

        size = newsize;
        table = newtable;
        chunks = newchunks;
        unused = newunused;
    }
};

#define enumerate(ht,k,e,t,f,b) loopi((ht).size)  for(hashtable<k,t>::chain *enumc = (ht).table[i]; enumc;) { hashtable<k,t>::const_key &e = enumc->key; t &f = enumc->data; enumc = enumc->next; b; }

template<class T>
struct unionfind
{
    struct ufval
    {
        int rank, next;
        T val;

        ufval(const T &val) : rank(0), next(-1), val(val) {}
    };

    vector<ufval> ufvals;

    void clear()
    {
        ufvals.setsize(0);
    }

    const T &find(int k, const T &noval, const T &initval)
    {
        if(k>=ufvals.length()) return initval;
        while(ufvals[k].next>=0) k = ufvals[k].next;
        if(ufvals[k].val == noval) ufvals[k].val = initval;
        return ufvals[k].val;
    }

    int compressfind(int k)
    {
        if(ufvals[k].next<0) return k;
        return ufvals[k].next = compressfind(ufvals[k].next);
    }

    void unite (int x, int y, const T &noval)
    {
        while(ufvals.length() <= max(x, y)) ufvals.add(ufval(noval));
        x = compressfind(x);
        y = compressfind(y);
        if(x==y) return;
        ufval &xval = ufvals[x], &yval = ufvals[y];
        if(xval.rank < yval.rank) xval.next = y;
        else
        {
            yval.next = x;
            if(xval.rank==yval.rank) yval.rank++;
        }
    }
};

template<class T>
struct listnode
{
    T *prev, *next;
};

template<class T>
struct list
{
    typedef listnode<T> node;

    int size;
    listnode<T> nodes;

    list() { clear(); }

    bool empty() const { return nodes.prev == nodes.next; }
    bool notempty() const { return nodes.prev != nodes.next; }

    T *first() const { return nodes.next; }
    T *last() const { return nodes.prev; }
    T *end() const { return (T *)&nodes; }

    void clear()
    {
        size = 0;
        nodes.prev = nodes.next = (T *)&nodes;
    }

    T *remove(T *node)
    {
        size--;
        node->prev->next = node->next;
        node->next->prev = node->prev;
        return node;
    }

    T *insertafter(T *node, T *pos)
    {
        size++;
        node->next = pos->next;
        node->next->prev = node;
        node->prev = pos;
        pos->next = node;
        return node;
    }

    T *insertbefore(T *node, T *pos)
    {
        size++;
        node->prev = pos->prev;
        node->prev->next = node;
        node->next = pos;
        pos->prev = node;
        return node;
    }

    T *insertfirst(T *node) { return insertafter(node, end()); }
    T *insertlast(T *node) { return insertbefore(node, end()); }

    T *removefirst() { return remove(first()); }
    T *removelast() { return remove(last()); }
};

static inline bool islittleendian() { union { int i; uchar b[sizeof(int)]; } conv; conv.i = 1; return conv.b[0] != 0; }
inline ushort endianswap16(ushort n) { return (n<<8) | (n>>8); }
inline uint endianswap32(uint n) { return (n<<24) | (n>>24) | ((n>>8)&0xFF00) | ((n<<8)&0xFF0000); }
inline ullong endianswap64(ullong n) { return endianswap32(uint(n >> 32)) | ((ullong)endianswap32(uint(n)) << 32); }
template<class T> inline T endianswap(T n) { union { T t; uint i; } conv; conv.t = n; conv.i = endianswap32(conv.i); return conv.t; }
template<> inline uchar endianswap<uchar>(uchar n) { return n; }
template<> inline char endianswap<char>(char n) { return n; }
template<> inline ushort endianswap<ushort>(ushort n) { return endianswap16(n); }
template<> inline short endianswap<short>(short n) { return endianswap16(n); }
template<> inline uint endianswap<uint>(uint n) { return endianswap32(n); }
template<> inline int endianswap<int>(int n) { return endianswap32(n); }
template<> inline ullong endianswap<ullong>(ullong n) { return endianswap64(n); }
template<> inline llong endianswap<llong>(llong n) { return endianswap64(n); }
template<> inline double endianswap<double>(double n) { union { double t; uint i; } conv; conv.t = n; conv.i = endianswap64(conv.i); return conv.t; }
template<class T> inline void endianswap(T *buf, int len) { for(T *end = &buf[len]; buf < end; buf++) *buf = endianswap(*buf); }
template<class T> inline T endiansame(T n) { return n; }
template<class T> inline void endiansame(T *buf, int len) {}
template<class T> inline T lilswap(T n) { return islittleendian() ? n : endianswap(n); }
template<class T> inline void lilswap(T *buf, int len) { if(!islittleendian()) endianswap(buf, len); }
template<class T> inline T bigswap(T n) { return islittleendian() ? endianswap(n) : n; }
template<class T> inline void bigswap(T *buf, int len) { if(islittleendian()) endianswap(buf, len); }

/* workaround for some C platforms that have these two functions as macros - not used anywhere */
#ifdef getchar
#undef getchar
#endif
#ifdef putchar
#undef putchar
#endif

struct stream
{
    virtual ~stream() {}
    virtual void close() = 0;
    virtual bool end() = 0;
    virtual long tell() { return -1; }
    virtual bool seek(long offset, int whence = SEEK_SET) { return false; }
    virtual long size();
    virtual int read(void *buf, int len) { return 0; }
    virtual int write(const void *buf, int len) { return 0; }
    virtual int getchar() { uchar c; return read(&c, 1) == 1 ? c : -1; }
    virtual bool putchar(int n) { uchar c = n; return write(&c, 1) == 1; }
    virtual bool getline(char *str, int len);
    virtual bool putstring(const char *str) { int len = strlen(str); return write(str, len) == len; }
    virtual bool putline(const char *str) { return putstring(str) && putchar('\n'); }
    virtual int printf(const char *fmt, ...) { return -1; }

    template<class T> bool put(T n) { return write(&n, sizeof(n)) == sizeof(n); }
    template<class T> bool putlil(T n) { return put<T>(lilswap(n)); }
    template<class T> bool putbig(T n) { return put<T>(bigswap(n)); }

    template<class T> T get() { T n; return read(&n, sizeof(n)) == sizeof(n) ? n : 0; }
    template<class T> T getlil() { return lilswap(get<T>()); }
    template<class T> T getbig() { return bigswap(get<T>()); }
};

long stream::size()
{
    long pos = tell(), endpos;
    if(pos < 0 || !seek(0, SEEK_END)) return -1;
    endpos = tell();
    return pos == endpos || seek(pos, SEEK_SET) ? endpos : -1;
}

bool stream::getline(char *str, int len)
{
    loopi(len-1)
    {
        if(read(&str[i], 1) != 1) { str[i] = '\0'; return i > 0; }
        else if(str[i] == '\n') { str[i+1] = '\0'; return true; }
    }
    if(len > 0) str[len-1] = '\0';
    return true;
}

struct filestream : stream
{
    FILE *file;

    filestream() : file(NULL) {}
    ~filestream() { close(); }

    bool open(const char *name, const char *mode)
    {
        if(file) return false;
        file = fopen(name, mode);
        return file!=NULL;
    }

    void close()
    {
        if(file) { fclose(file); file = NULL; }
    }

    bool end() { return feof(file)!=0; }
    long tell() { return ftell(file); }
    bool seek(long offset, int whence) { return fseek(file, offset, whence) >= 0; }
    int read(void *buf, int len) { return fread(buf, 1, len, file); }
    int write(const void *buf, int len) { return fwrite(buf, 1, len, file); }
    int getchar() { return fgetc(file); }
    bool putchar(int c) { return fputc(c, file)!=EOF; }
    bool getline(char *str, int len) { return fgets(str, len, file)!=NULL; }
    bool putstring(const char *str) { return fputs(str, file)!=EOF; }

    int printf(const char *fmt, ...)
    {
        va_list v;
        va_start(v, fmt);
        int result = vfprintf(file, fmt, v);
        va_end(v);
        return result;
    }
};

char *path(char *s)
{
    for(char *curpart = s;;)
    {
        char *endpart = strchr(curpart, '&');
        if(endpart) *endpart = '\0';
        if(curpart[0]=='<')
        {
            char *file = strrchr(curpart, '>');
            if(!file) return s;
            curpart = file+1;
        }
        for(char *t = curpart; (t = strpbrk(t, "/\\")); *t++ = PATHDIV);
        for(char *prevdir = NULL, *curdir = s;;)
        {
            prevdir = curdir[0]==PATHDIV ? curdir+1 : curdir;
            curdir = strchr(prevdir, PATHDIV);
            if(!curdir) break;
            if(prevdir+1==curdir && prevdir[0]=='.')
            {
                memmove(prevdir, curdir+1, strlen(curdir+1)+1);
                curdir = prevdir;
            }
            else if(curdir[1]=='.' && curdir[2]=='.' && curdir[3]==PATHDIV)
            {
                if(prevdir+2==curdir && prevdir[0]=='.' && prevdir[1]=='.') continue;
                memmove(prevdir, curdir+4, strlen(curdir+4)+1);
                curdir = prevdir;
            }
        }
        if(endpart)
        {
            *endpart = '&';
            curpart = endpart+1;
        }
        else break;
    }
    return s;
}

char *path(const char *s, bool copy)
{
    static string tmp;
    copystring(tmp, s);
    path(tmp);
    return tmp;
}

const char *parentdir(const char *directory)
{
    const char *p = directory + strlen(directory);
    while(p > directory && *p != '/' && *p != '\\') p--;
    static string parent;
    size_t len = p-directory+1;
    copystring(parent, directory, len);
    return parent;
}

stream *openfile(const char *filename, const char *mode)
{
    filestream *file = new filestream;
    if(!file->open(path(filename, true), mode)) { delete file; return NULL; }
    return file;
}

struct Vec4;

struct Vec3
{
    union
    {
        struct { double x, y, z; };
        double v[3];
    };

    Vec3() {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
    explicit Vec3(const double *v) : x(v[0]), y(v[1]), z(v[2]) {}
    explicit Vec3(const Vec4 &v);

    double &operator[](int i) { return v[i]; }
    double operator[](int i) const { return v[i]; }

    bool operator==(const Vec3 &o) const { return !memcmp(&x, &o.x, sizeof(double)*3); } //< @r-lyeh: optimize. return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const Vec3 &o) const { return  memcmp(&x, &o.x, sizeof(double)*3); } //< @r-lyeh: optimize. return x != o.x || y != o.y || z != o.z; }
    bool operator<(const Vec3 &o) const { return x < o.x || y < o.y || z < o.z; }
    bool operator>(const Vec3 &o) const { return x > o.x || y > o.y || z > o.z; }

    Vec3 operator+(const Vec3 &o) const { return Vec3(x+o.x, y+o.y, z+o.z); }
    Vec3 operator-(const Vec3 &o) const { return Vec3(x-o.x, y-o.y, z-o.z); }
    Vec3 operator+(double k) const { return Vec3(x+k, y+k, z+k); }
    Vec3 operator-(double k) const { return Vec3(x-k, y-k, z-k); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    Vec3 operator*(const Vec3 &o) const { return Vec3(x*o.x, y*o.y, z*o.z); }
    Vec3 operator/(const Vec3 &o) const { return Vec3(x/o.x, y/o.y, z/o.z); }
    Vec3 operator*(double k) const { return Vec3(x*k, y*k, z*k); }
    Vec3 operator/(double k) const { return Vec3(x/k, y/k, z/k); }

    Vec3 &operator+=(const Vec3 &o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3 &operator-=(const Vec3 &o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3 &operator+=(double k) { x += k; y += k; z += k; return *this; }
    Vec3 &operator-=(double k) { x -= k; y -= k; z -= k; return *this; }
    Vec3 &operator*=(const Vec3 &o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
    Vec3 &operator/=(const Vec3 &o) { x /= o.x; y /= o.y; z /= o.z; return *this; }
    Vec3 &operator*=(double k) { x *= k; y *= k; z *= k; return *this; }
    Vec3 &operator/=(double k) { x /= k; y /= k; z /= k; return *this; }

    double dot(const Vec3 &o) const { return x*o.x + y*o.y + z*o.z; }
    double magnitude() const { return sqrt(dot(*this)); }
    double squaredlen() const { return dot(*this); }
    double dist(const Vec3 &o) const { return (*this - o).magnitude(); }
    Vec3 normalize() const { return *this * (1.0 / magnitude()); }
    Vec3 cross(const Vec3 &o) const { return Vec3(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
    Vec3 reflect(const Vec3 &n) const { return *this - n*2.0*dot(n); }
    Vec3 project(const Vec3 &n) const { return *this - n*dot(n); }

    Vec3 zxy() const { return Vec3(z, x, y); }
    Vec3 zyx() const { return Vec3(z, y, x); }
    Vec3 yxz() const { return Vec3(y, x, z); }
    Vec3 yzx() const { return Vec3(y, z, x); }
    Vec3 xzy() const { return Vec3(x, z, y); }
};

static inline bool htcmp(const Vec3 &x, const Vec3 &y)
{
    return x == y;
}

static inline uint hthash(const Vec3 &k)
{
    uint h = hthash(k.x);
    h = htcombine(h, k.y);
    h = htcombine(h, k.z);
    return h;
}

struct Vec4
{
    union
    {
        struct { double x, y, z, w; };
        double v[4];
    };

    Vec4() {}
    Vec4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}
    explicit Vec4(const Vec3 &p, double w = 0) : x(p.x), y(p.y), z(p.z), w(w) {}
    explicit Vec4(const double *v) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

    double &operator[](int i)       { return v[i]; }
    double  operator[](int i) const { return v[i]; }

    bool operator==(const Vec4 &o) const { return !memcmp(&x, &o.x, sizeof(double)*4); } //< @r-lyeh: optimize. return x == o.x && y == o.y && z == o.z && w == o.w; }
    bool operator!=(const Vec4 &o) const { return  memcmp(&x, &o.x, sizeof(double)*4); } //< @r-lyeh: optimize. return x != o.x || y != o.y || z != o.z || w != o.w; }
    bool operator<(const Vec4 &o) const { return x < o.x || y < o.y || z < o.z || w < o.w; }
    bool operator>(const Vec4 &o) const { return x > o.x || y > o.y || z > o.z || w > o.w; }

    Vec4 operator+(const Vec4 &o) const { return Vec4(x+o.x, y+o.y, z+o.z, w+o.w); }
    Vec4 operator-(const Vec4 &o) const { return Vec4(x-o.x, y-o.y, z-o.z, w-o.w); }
    Vec4 operator+(double k) const { return Vec4(x+k, y+k, z+k, w+k); }
    Vec4 operator-(double k) const { return Vec4(x-k, y-k, z-k, w-k); }
    Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }
    Vec4 operator*(double k) const { return Vec4(x*k, y*k, z*k, w*k); }
    Vec4 operator/(double k) const { return Vec4(x/k, y/k, z/k, w/k); }
    Vec4 addw(double f) const { return Vec4(x, y, z, w + f); }

    Vec4 &operator+=(const Vec4 &o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
    Vec4 &operator+=(const Vec3 &o) { x += o.x; y += o.y; z += o.z; return * this; }
    Vec4 &operator-=(const Vec4 &o) { x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this; }
    Vec4 &operator-=(const Vec3 &o) { x -= o.x; y -= o.y; z -= o.z; return * this; }
    Vec4 &operator+=(double k) { x += k; y += k; z += k; w += k; return *this; }
    Vec4 &operator-=(double k) { x -= k; y -= k; z -= k; w -= k; return *this; }
    Vec4 &operator*=(double k) { x *= k; y *= k; z *= k; w *= k; return *this; }
    Vec4 &operator/=(double k) { x /= k; y /= k; z /= k; w /= k; return *this; }

    double dot3(const Vec4 &o) const { return x*o.x + y*o.y + z*o.z; }
    double dot3(const Vec3 &o) const { return x*o.x + y*o.y + z*o.z; }
    double dot(const Vec4 &o) const { return dot3(o) + w*o.w; }
    double dot(const Vec3 &o) const  { return x*o.x + y*o.y + z*o.z + w; }
    double magnitude() const  { return sqrt(dot(*this)); }
    double magnitude3() const { return sqrt(dot3(*this)); }
    Vec4 normalize() const { return *this * (1.0 / magnitude()); }
    Vec3 cross3(const Vec4 &o) const { return Vec3(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
    Vec3 cross3(const Vec3 &o) const { return Vec3(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }

    void setxyz(const Vec3 &o) { x = o.x; y = o.y; z = o.z; }
};

inline Vec3::Vec3(const Vec4 &v) : x(v.x), y(v.y), z(v.z) {}

static inline bool htcmp(const Vec4 &x, const Vec4 &y)
{
    return x == y;
}

static inline uint hthash(const Vec4 &k)
{
    uint h = hthash(k.x);
    h = htcombine(h, k.y);
    h = htcombine(h, k.z);
    h = htcombine(h, k.w);
    return h;
}

struct Matrix3x3;
struct Matrix3x4;

struct Quat : Vec4
{
    Quat() {}
    Quat(double x, double y, double z, double w) : Vec4(x, y, z, w) {}
    Quat(double angle, const Vec3 &axis)
    {
        double s = sin(0.5*angle);
        x = s*axis.x;
        y = s*axis.y;
        z = s*axis.z;
        w = cos(0.5*angle);
    }
    explicit Quat(const Vec3 &v) : Vec4(v.x, v.y, v.z, -sqrt(max(1.0 - v.squaredlen(), 0.0))) {}
    explicit Quat(const Matrix3x3 &m) { convertmatrix(m); }
    explicit Quat(const Matrix3x4 &m) { convertmatrix(m); }

    void restorew()
    {
        w = -sqrt(max(1.0 - dot3(*this), 0.0));
    }

    Quat operator*(const Quat &o) const
    {
        return Quat(w*o.x + x*o.w + y*o.z - z*o.y,
                    w*o.y - x*o.z + y*o.w + z*o.x,
                    w*o.z + x*o.y - y*o.x + z*o.w,
                    w*o.w - x*o.x - y*o.y - z*o.z);
    }
    Quat &operator*=(const Quat &o) { return (*this = *this * o); }

    Quat operator+(const Vec4 &o) const { return Quat(x+o.x, y+o.y, z+o.z, w+o.w); }
    Quat &operator+=(const Vec4 &o) { return (*this = *this + o); }
    Quat operator-(const Vec4 &o) const { return Quat(x-o.x, y-o.y, z-o.z, w-o.w); }
    Quat &operator-=(const Vec4 &o) { return (*this = *this - o); }

    Quat operator-() const { return Quat(-x, -y, -z, w); }

    void flip() { x = -x; y = -y; z = -z; w = -w; }

    Vec3 transform(const Vec3 &p) const
    {
        return p + cross3(cross3(p) + p*w)*2.0;
    }

    template<class M>
    void convertmatrix(const M &m)
    {
        double trace = m.a.x + m.b.y + m.c.z;
        if(trace>0)
        {
            double r = sqrt(1 + trace), inv = 0.5/r;
            w = 0.5*r;
            x = (m.c.y - m.b.z)*inv;
            y = (m.a.z - m.c.x)*inv;
            z = (m.b.x - m.a.y)*inv;
        }
        else if(m.a.x > m.b.y && m.a.x > m.c.z)
        {
            double r = sqrt(1 + m.a.x - m.b.y - m.c.z), inv = 0.5/r;
            x = 0.5*r;
            y = (m.b.x + m.a.y)*inv;
            z = (m.a.z + m.c.x)*inv;
            w = (m.c.y - m.b.z)*inv;
        }
        else if(m.b.y > m.c.z)
        {
            double r = sqrt(1 + m.b.y - m.a.x - m.c.z), inv = 0.5/r;
            x = (m.b.x + m.a.y)*inv;
            y = 0.5*r;
            z = (m.c.y + m.b.z)*inv;
            w = (m.a.z - m.c.x)*inv;
        }
        else
        {
            double r = sqrt(1 + m.c.z - m.a.x - m.b.y), inv = 0.5/r;
            x = (m.a.z + m.c.x)*inv;
            y = (m.c.y + m.b.z)*inv;
            z = 0.5*r;
            w = (m.b.x - m.a.y)*inv;
        }
    }

    static Quat fromangles(const Vec3 &rot)
    {
        double cx = cos(rot.x/2), sx = sin(rot.x/2),
               cy = cos(rot.y/2), sy = sin(rot.y/2),
               cz = cos(rot.z/2), sz = sin(rot.z/2);
        Quat q(sx*cy*cz - cx*sy*sz,
               cx*sy*cz + sx*cy*sz,
               cx*cy*sz - sx*sy*cz,
               cx*cy*cz + sx*sy*sz);
        if(q.w > 0) q.flip();
        return q;
    }

    static Quat fromdegrees(const Vec3 &rot) { return fromangles(rot * (M_PI / 180)); }
};

struct Matrix3x3
{
    Vec3 a, b, c;

    Matrix3x3() {}
    Matrix3x3(const Vec3 &a, const Vec3 &b, const Vec3 &c) : a(a), b(b), c(c) {}
    explicit Matrix3x3(const Quat &q) { convertquat(q); }
    explicit Matrix3x3(const Quat &q, const Vec3 &scale)
    {
        convertquat(q);
        a *= scale;
        b *= scale;
        c *= scale;
    }

    void convertquat(const Quat &q)
    {
        double x = q.x, y = q.y, z = q.z, w = q.w,
               tx = 2*x, ty = 2*y, tz = 2*z,
               txx = tx*x, tyy = ty*y, tzz = tz*z,
               txy = tx*y, txz = tx*z, tyz = ty*z,
               twx = w*tx, twy = w*ty, twz = w*tz;
        a = Vec3(1 - (tyy + tzz), txy - twz, txz + twy);
        b = Vec3(txy + twz, 1 - (txx + tzz), tyz - twx);
        c = Vec3(txz - twy, tyz + twx, 1 - (txx + tyy));
    }

    Matrix3x3 operator*(const Matrix3x3 &o) const
    {
        return Matrix3x3(
            o.a*a.x + o.b*a.y + o.c*a.z,
            o.a*b.x + o.b*b.y + o.c*b.z,
            o.a*c.x + o.b*c.y + o.c*c.z);
    }
    Matrix3x3 &operator*=(const Matrix3x3 &o) { return (*this = *this * o); }

    void transpose(const Matrix3x3 &o)
    {
        a = Vec3(o.a.x, o.b.x, o.c.x);
        b = Vec3(o.a.y, o.b.y, o.c.y);
        c = Vec3(o.a.z, o.b.z, o.c.z);
    }
    void transpose() { transpose(Matrix3x3(*this)); }

    Vec3 transform(const Vec3 &o) const { return Vec3(a.dot(o), b.dot(o), c.dot(o)); }

    float determinant()
    {
        return
            a.x * b.y * c.z +
            a.y * b.z * c.x +
            a.z * b.x * c.y -
            a.z * b.y * c.x -
            a.y * b.x * c.z -
            a.x * b.z * c.y;
    }
};

struct Matrix3x4
{
    Vec4 a, b, c;

    Matrix3x4() {}
    Matrix3x4(const Vec4 &a, const Vec4 &b, const Vec4 &c) : a(a), b(b), c(c) {}
    explicit Matrix3x4(const Matrix3x3 &rot, const Vec3 &trans)
        : a(Vec4(rot.a, trans.x)), b(Vec4(rot.b, trans.y)), c(Vec4(rot.c, trans.z))
    {
    }
    explicit Matrix3x4(const Quat &rot, const Vec3 &trans)
    {
        *this = Matrix3x4(Matrix3x3(rot), trans);
    }
    explicit Matrix3x4(const Quat &rot, const Vec3 &trans, const Vec3 &scale)
    {
        *this = Matrix3x4(Matrix3x3(rot, scale), trans);
    }

    Matrix3x4 operator*(float k) const { return Matrix3x4(*this) *= k; }
    Matrix3x4 &operator*=(float k)
    {
        a *= k;
        b *= k;
        c *= k;
        return *this;
    }

    Matrix3x4 operator+(const Matrix3x4 &o) const { return Matrix3x4(*this) += o; }
    Matrix3x4 &operator+=(const Matrix3x4 &o)
    {
        a += o.a;
        b += o.b;
        c += o.c;
        return *this;
    }

    void invert(const Matrix3x4 &o)
    {
        Matrix3x3 invrot(Vec3(o.a.x, o.b.x, o.c.x), Vec3(o.a.y, o.b.y, o.c.y), Vec3(o.a.z, o.b.z, o.c.z));
        invrot.a /= invrot.a.squaredlen();
        invrot.b /= invrot.b.squaredlen();
        invrot.c /= invrot.c.squaredlen();
        Vec3 trans(o.a.w, o.b.w, o.c.w);
        a = Vec4(invrot.a, -invrot.a.dot(trans));
        b = Vec4(invrot.b, -invrot.b.dot(trans));
        c = Vec4(invrot.c, -invrot.c.dot(trans));
    }
    void invert() { invert(Matrix3x4(*this)); }

    Matrix3x4 operator*(const Matrix3x4 &o) const
    {
        return Matrix3x4(
            (o.a*a.x + o.b*a.y + o.c*a.z).addw(a.w),
            (o.a*b.x + o.b*b.y + o.c*b.z).addw(b.w),
            (o.a*c.x + o.b*c.y + o.c*c.z).addw(c.w));
    }
    Matrix3x4 &operator*=(const Matrix3x4 &o) { return (*this = *this * o); }

    Vec3 transform(const Vec3 &o) const { return Vec3(a.dot(o), b.dot(o), c.dot(o)); }
};

void conoutf(const char *s, ...)
{
    defvformatstring(msg,s,s);
    printf("%s\n", msg);
}

void fatal(const char *s, ...)    // failure exit
{
    defvformatstring(msg,s,s);
    fprintf(stderr, "%s\n", msg);

    exit(EXIT_FAILURE);
}






struct triangle { uint vert[3]; triangle() {} triangle(uint v0, uint v1, uint v2) { vert[0] = v0; vert[1] = v1; vert[2] = v2; } };
vector<triangle> triangles, neighbors;

struct mesh { uint name, material; uint firstvert, numverts; uint firsttri, numtris; mesh() : name(0), material(0), firstvert(0), numverts(0), firsttri(0), numtris(0) {} };
vector<mesh> meshes;

struct anim { uint name; uint firstframe, numframes; float fps; uint flags; anim() : name(0), firstframe(0), numframes(0), fps(0), flags(0) {} };
vector<anim> anims;

struct joint { uint name; int parent; float pos[3], orient[4], scale[3]; joint() : name(0), parent(-1) { memset(pos, 0, sizeof(pos)); memset(orient, 0, sizeof(orient)); memset(scale, 0, sizeof(scale)); } };
vector<joint> joints;

struct pose { int parent; uint flags; float offset[10], scale[10]; pose() : parent(-1), flags(0) { memset(offset, 0, sizeof(offset)); memset(scale, 0, sizeof(scale)); } };
vector<pose> poses;

struct framebounds { Vec3 bbmin, bbmax; double xyradius, radius; framebounds() : bbmin(0, 0, 0), bbmax(0, 0, 0), xyradius(0), radius(0) {} };
vector<framebounds> bounds;

struct transform
{
    Vec3 pos;
    Quat orient;
    Vec3 scale;

    transform() {}
    transform(const Vec3 &pos, const Quat &orient, const Vec3 &scale = Vec3(1, 1, 1)) : pos(pos), orient(orient), scale(scale) {}
};
vector<transform> frames;

vector<char> stringdata, commentdata;

struct sharedstring
{
    uint offset;
    sharedstring() {}
    sharedstring(const char *s) : offset(stringdata.length()) { stringdata.put(s, strlen(s)+1); }
};

static inline bool htcmp(const char *x, const sharedstring &s)
{
    return htcmp(x, &stringdata[s.offset]);
}

static inline uint hthash(const sharedstring &s)
{
    return hthash(&stringdata[s.offset]);
}

hashtable<sharedstring, uint> stringoffsets;

uint sharestring(const char *s)
{
    if(stringdata.empty()) stringoffsets.access("", 0);
    return stringoffsets.access(s ? s : "", stringdata.length());
}

struct blendcombo
{
    int sorted;
    double weights[4];
    uchar bones[4];

    blendcombo() : sorted(0) {}

    void reset() { sorted = 0; }

    void addweight(double weight, int bone)
    {
        if(weight <= 1e-3) return;
        loopk(sorted) if(weight > weights[k])
        {
            for(int l = min(sorted-1, 2); l >= k; l--)
            {
                weights[l+1] = weights[l];
                bones[l+1] = bones[l];
            }
            weights[k] = weight;
            bones[k] = bone;
            if(sorted<4) sorted++;
            return;
        }
        if(sorted>=4) return;
        weights[sorted] = weight;
        bones[sorted] = bone;
        sorted++;
    }

    void finalize()
    {
        loopj(4-sorted) { weights[sorted+j] = 0; bones[sorted+j] = 0; }
        if(sorted <= 0) return;
        double total = 0;
        loopj(sorted) total += weights[j];
        total = 1.0/total;
        loopj(sorted) weights[j] *= total;
    }

    void serialize(uchar *vweights) const
    {
        int total = 0;
        loopk(4) total += (vweights[k] = uchar(0.5 + weights[k]*255));
        if(sorted <= 0) return;
        while(total > 255)
        {
            loopk(4) if(vweights[k] > 0 && total > 255) { vweights[k]--; total--; }
        }
        while(total < 255)
        {
            loopk(4) if(vweights[k] < 255 && total < 255) { vweights[k]++; total++; }
        }
    }

    bool operator==(const blendcombo &c) { loopi(4) if(weights[i] != c.weights[i] || bones[i] != c.bones[i]) return false; return true; }
    bool operator!=(const blendcombo &c) { loopi(4) if(weights[i] != c.weights[i] || bones[i] != c.bones[i]) return true; return false; }
};

static bool parseindex(char *&c, int &val)
{
    while(isspace(*c)) c++;
    char *end = NULL;
    int rval = strtol(c, &end, 10);
    if(c == end) return false;
    val = rval;
    c = end;
    return true;
}

static double parseattrib(char *&c, double ival = 0)
{
    while(isspace(*c)) c++;
    char *end = NULL;
    double val = strtod(c, &end);
    if(c == end) val = ival;
    else c = end;
    return val;
}

static bool maybeparseattrib(char *&c, double &result)
{
    while(isspace(*c)) c++;
    char *end = NULL;
    double val = strtod(c, &end);
    if(c == end) return false;
    c = end;
    result = val;
    return true;
}

#if 0
static bool parsename(char *&c, char *buf, int bufsize = sizeof(string))
{
    while(isspace(*c)) c++;
    char *end;
    if(*c == '"')
    {
        c++;
        end = c;
        while(*end && *end != '"') end++;
        copystring(buf, c, min(int(end-c+1), bufsize));
        if(*end == '"') end++;
    }
    else
    {
        end = c;
        while(*end && !isspace(*end)) end++;
        copystring(buf, c, min(int(end-c+1), bufsize));
    }
    if(c == end) return false;
    c = end;
    return true;
}
#endif

static char *trimname(char *&c)
{
    while(isspace(*c)) c++;
    char *start, *end;
    if(*c == '"')
    {
        c++;
        start = end = c;
        while(*end && *end != '"') end++;
        if(*end) { *end = '\0'; end++; }
    }
    else
    {
        start = end = c;
        while(*end && !isspace(*end)) end++;
        if(*end) { *end = '\0'; end++; }
    }
    c = end;
    return start;
}

static Vec4 parseattribs4(char *&c, const Vec4 &ival = Vec4(0, 0, 0, 0))
{
    Vec4 val;
    loopk(4) val[k] = parseattrib(c, ival[k]);
    return val;
}

static Vec3 parseattribs3(char *&c, const Vec3 &ival = Vec3(0, 0, 0))
{
    Vec3 val;
    loopk(3) val[k] = parseattrib(c, ival[k]);
    return val;
}

static blendcombo parseblends(char *&c)
{
    blendcombo b;
    int index;
    while(parseindex(c, index))
    {
        double weight = parseattrib(c, 0);
        b.addweight(weight, index);
    }
    b.finalize();
    return b;
}

struct ejoint
{
    const char *name;
    int parent;

    ejoint() : name(NULL), parent(-1) {}
};

struct eanim
{
    const char *name;
    int startframe, endframe;
    double fps;
    uint flags;

    eanim() : name(NULL), startframe(0), endframe(INT_MAX), fps(0), flags(0) {}
};

struct emesh
{
    const char *name, *material;
    int firsttri;
    bool used;

    emesh() : name(NULL), material(NULL), firsttri(0), used(false) {}
    emesh(const char *name, const char *material, int firsttri = 0) : name(name), material(material), firsttri(firsttri), used(false) {}
};

struct evarray
{
    string name;
    int type, format, size;

    evarray() : type(IQM_POSITION), format(IQM_FLOAT), size(3) { name[0] = '\0'; }
    evarray(int type, int format, int size, const char *initname = "") : type(type), format(format), size(size) { copystring(name, initname); }
};

struct esmoothgroup
{
    enum
    {
        F_USED     = 1<<0,
        F_UVSMOOTH = 1<<1
    };

    int key;
    float angle;
    int flags;

    esmoothgroup() : key(-1), angle(-1), flags(0) {}
};

struct etriangle
{
    int smoothgroup;
    uint vert[3], weld[3];

    etriangle()
        : smoothgroup(-1)
    {
    }
    etriangle(int v0, int v1, int v2, int smoothgroup = -1)
        : smoothgroup(smoothgroup)
    {
        vert[0] = v0;
        vert[1] = v1;
        vert[2] = v2;
    }
};

vector<Vec4> mpositions, epositions, etexcoords, etangents, ecolors, ecustom[10];
vector<Vec3> enormals, ebitangents;
vector<blendcombo> mblends, eblends;
vector<etriangle> etriangles;
vector<esmoothgroup> esmoothgroups;
vector<int> esmoothindexes;
vector<uchar> esmoothedges;
vector<ejoint> ejoints;
vector<transform> eposes;
vector<Matrix3x4> mjoints;
vector<int> eframes;
vector<eanim> eanims;
vector<emesh> emeshes;
vector<evarray> evarrays;
hashtable<const char *, char *> enames;

const char *getnamekey(const char *name)
{
    char **exists = enames.access(name);
    if(exists) return *exists;
    char *key = newstring(name);
    enames[key] = key;
    return key;
}

struct weldinfo
{
    int tri, vert;
    weldinfo *next;
};

void weldvert(const vector<Vec3> &norms, const Vec4 &pos, weldinfo *welds, int &numwelds, unionfind<int> &welder)
{
    welder.clear();
    int windex = 0;
    for(weldinfo *w = welds; w; w = w->next, windex++)
    {
        etriangle &wt = etriangles[w->tri];
        esmoothgroup &wg = esmoothgroups[wt.smoothgroup];
        int vindex = windex + 1;
        for(weldinfo *v = w->next; v; v = v->next, vindex++)
        {
            etriangle &vt = etriangles[v->tri];
            esmoothgroup &vg = esmoothgroups[vt.smoothgroup];
            if(wg.key != vg.key) continue;
            if(norms[w->tri].dot(norms[v->tri]) < max(wg.angle, vg.angle)) continue;
            if(((wg.flags | vg.flags) & esmoothgroup::F_UVSMOOTH) &&
               etexcoords[wt.vert[w->vert]] != etexcoords[vt.vert[v->vert]])
                continue;
            if(esmoothindexes.length() > max(w->vert, v->vert) && esmoothindexes[w->vert] != esmoothindexes[v->vert])
                continue;
            if(esmoothedges.length())
            {
                int w0 = w->vert, w1 = (w->vert+1)%3, w2 = (w->vert+2)%3;
                const Vec4 &wp1 = epositions[wt.vert[w1]],
                           &wp2 = epositions[wt.vert[w2]];
                int v0 = v->vert, v1 = (v->vert+1)%3, v2 = (v->vert+2)%3;
                const Vec4 &vp1 = epositions[vt.vert[v1]],
                           &vp2 = epositions[vt.vert[v2]];
                int wf = esmoothedges[w->tri], vf = esmoothedges[v->tri];
                if((wp1 != vp1 || !(((wf>>w0)|(vf>>v0))&1)) &&
                   (wp1 != vp2 || !(((wf>>w0)|(vf>>v2))&1)) &&
                   (wp2 != vp1 || !(((wf>>w2)|(vf>>v0))&1)) &&
                   (wp2 != vp2 || !(((wf>>w2)|(vf>>v2))&1)))
                    continue;
            }
            welder.unite(windex, vindex, -1);
        }
    }
    windex = 0;
    for(weldinfo *w = welds; w; w = w->next, windex++)
    {
        etriangle &wt = etriangles[w->tri];
        wt.weld[w->vert] = welder.find(windex, -1, numwelds);
        if(wt.weld[w->vert] == uint(numwelds)) numwelds++;
    }
}

void smoothverts(bool areaweight = true)
{
    if(etriangles.empty()) return;

    if(enormals.length())
    {
        loopv(etriangles)
        {
            etriangle &t = etriangles[i];
            loopk(3) t.weld[k] = t.vert[k];
        }
        return;
    }

    if(etexcoords.empty()) loopv(esmoothgroups) esmoothgroups[i].flags &= ~esmoothgroup::F_UVSMOOTH;
    if(esmoothedges.length()) while(esmoothedges.length() < etriangles.length()) esmoothedges.add(7);

    vector<Vec3> tarea, tnorms;
    loopv(etriangles)
    {
        etriangle &t = etriangles[i];
        Vec3 v0(epositions[t.vert[0]]),
             v1(epositions[t.vert[1]]),
             v2(epositions[t.vert[2]]);
        tnorms.add(tarea.add((v2 - v0).cross(v1 - v0)).normalize());
    }

    int nextalloc = 0;
    vector<weldinfo *> allocs;
    hashtable<Vec4, weldinfo *> welds(1<<12);

    loopv(etriangles)
    {
        etriangle &t = etriangles[i];
        loopk(3)
        {
            weldinfo **next = &welds.access(epositions[t.vert[k]], NULL);
            if(! (nextalloc % 1024)) allocs.add(new weldinfo[1024]);
            weldinfo &w = allocs[nextalloc/1024][nextalloc%1024];
            nextalloc++;
            w.tri = i;
            w.vert = k;
            w.next = *next;
            *next = &w;
        }
    }

    int numwelds = 0;
    unionfind<int> welder;
    enumerate(welds, Vec4, vpos, weldinfo *, vwelds, weldvert(tnorms, vpos, vwelds, numwelds, welder));

    loopv(allocs) delete_array allocs[i];

    loopi(numwelds) enormals.add(Vec3(0, 0, 0));
    loopv(etriangles)
    {
        etriangle &t = etriangles[i];
        loopk(3) enormals[t.weld[k]]+= areaweight ? tarea[i] : tnorms[i];
    }
    loopv(enormals) if(enormals[i] != Vec3(0, 0, 0)) enormals[i] = enormals[i].normalize();
}

struct sharedvert
{
    int index, weld;

    sharedvert() {}
    sharedvert(int index, int weld) : index(index), weld(weld) {}
};

static inline bool htcmp(const sharedvert &v, const sharedvert &s)
{
    if(epositions[v.index] != epositions[s.index]) return false;
    if(etexcoords.length() && etexcoords[v.index] != etexcoords[s.index]) return false;
    if(enormals.length() && enormals[v.weld] != enormals[s.weld]) return false;
    if(eblends.length() && eblends[v.index] != eblends[s.index]) return false;
    if(ecolors.length() && ecolors[v.index] != ecolors[s.index]) return false;
    loopi(10) if(ecustom[i].length() && ecustom[i][v.index] != ecustom[i][s.index]) return false;
    return true;
}

static inline uint hthash(const sharedvert &v)
{
    return hthash(epositions[v.index]);
}

const struct vertexarraytype
{
    const char *name;
    int code;
} vatypes[] =
{
    { "position", IQM_POSITION },
    { "texcoord", IQM_TEXCOORD },
    { "normal", IQM_NORMAL  },
    { "tangent", IQM_TANGENT },
    { "blendindexes", IQM_BLENDINDEXES },
    { "blendweights", IQM_BLENDWEIGHTS },
    { "color", IQM_COLOR },
    { "custom0", IQM_CUSTOM + 0 },
    { "custom1", IQM_CUSTOM + 1 },
    { "custom2", IQM_CUSTOM + 2 },
    { "custom3", IQM_CUSTOM + 3 },
    { "custom4", IQM_CUSTOM + 4 },
    { "custom5", IQM_CUSTOM + 5 },
    { "custom6", IQM_CUSTOM + 6 },
    { "custom7", IQM_CUSTOM + 7 },
    { "custom8", IQM_CUSTOM + 8 },
    { "custom9", IQM_CUSTOM + 9 }
};

int findvertexarraytype(const char *name)
{
    loopi(sizeof(vatypes)/sizeof(vatypes[0]))
    {
        if(!strcasecmp(vatypes[i].name, name))
            return vatypes[i].code;
    }
    return -1;
}

const struct vertexarrayformat
{
    const char *name;
    int code;
    int size;
} vaformats[] =
{
    { "byte", IQM_BYTE, 1 },
    { "ubyte", IQM_UBYTE, 1 },
    { "short", IQM_SHORT, 2 },
    { "ushort", IQM_USHORT, 2 },
    { "int", IQM_INT, 4 },
    { "uint", IQM_UINT, 4 },
    { "half", IQM_HALF, 2 },
    { "float", IQM_FLOAT, 4 },
    { "double", IQM_DOUBLE, 8 }
};

int findvertexarrayformat(const char *name)
{
    loopi(sizeof(vaformats)/sizeof(vaformats[0]))
    {
        if(!strcasecmp(vaformats[i].name, name))
            return vaformats[i].code;
    }
    return -1;
}

struct vertexarray
{
    uint type, flags, format, size, offset;

    vertexarray(uint type, uint format, uint size, uint offset) : type(type), flags(0), format(format), size(size), offset(offset) {}

    int formatsize() const
    {
        return vaformats[format].size;
    }

    int bytesize() const
    {
        return size * vaformats[format].size;
    }
};

vector<sharedvert> vmap;
vector<vertexarray> varrays;
vector<uchar> vdata;

struct halfdata
{
    ushort val;

    halfdata(double d)
    {
        union
        {
            ullong i;
            double d;
        } conv;
        conv.d = d;
        ushort signbit = ushort((conv.i>>63)&1);
        ushort mantissa = ushort((conv.i>>(52-10))&0x3FF);
        int exponent = int((conv.i>>52)&0x7FF) - 1023 + 15;
        if(exponent <= 0)
        {
            mantissa |= 0x400;
            mantissa >>= min(1-exponent, 10+1);
            exponent = 0;
        }
        else if(exponent >= 0x1F)
        {
            mantissa = 0;
            exponent = 0x1F;
        }
        val = (signbit<<15) | (ushort(exponent)<<10) | mantissa;
    }
};

template<> inline halfdata endianswap<halfdata>(halfdata n) { n.val = endianswap16(n.val); return n; }

template<int TYPE> static inline int remapindex(int i, const sharedvert &v) { return v.index; }
template<> inline int remapindex<IQM_NORMAL>(int i, const sharedvert &v) { return v.weld; }
template<> inline int remapindex<IQM_TANGENT>(int i, const sharedvert &v) { return i; }

template<class T, class U>
static inline void putattrib(T &out, const U &val) { out = T(val); }

template<class T, class U>
static inline void uroundattrib(T &out, const U &val, double scale) { out = T(clamp(0.5 + val*scale, 0.0, scale)); }
template<class T, class U>
static inline void sroundattrib(T &out, const U &val, double scale, double low, double high) { double n = val*scale*0.5; out = T(clamp(n < 0 ? ceil(n - 1) : floor(n), low, high)); }

template<class T, class U>
static inline void scaleattrib(T &out, const U &val) { putattrib(out, val); }
template<class U>
static inline void scaleattrib(char &out, const U &val) { sroundattrib(out, val, 255.0, -128.0, 127.0); }
template<class U>
static inline void scaleattrib(short &out, const U &val) { sroundattrib(out, val, 65535.0, -32768.0, 32767.0); }
template<class U>
static inline void scaleattrib(int &out, const U &val) { sroundattrib(out, val, 4294967295.0, -2147483648.0, 2147483647.0); }
template<class U>
static inline void scaleattrib(uchar &out, const U &val) { uroundattrib(out, val, 255.0); }
template<class U>
static inline void scaleattrib(ushort &out, const U &val) { uroundattrib(out, val, 65535.0); }
template<class U>
static inline void scaleattrib(uint &out, const U &val) { uroundattrib(out, val, 4294967295.0); }

template<int T>
static inline bool normalizedattrib() { return true; }

template<int TYPE, int FMT, class T, class U>
static inline void serializeattrib(const vertexarray &va, T *data, const U &attrib)
{
    if(normalizedattrib<TYPE>()) switch(va.size)
    {
    case 4: scaleattrib(data[3], attrib.w);
    case 3: scaleattrib(data[2], attrib.z);
    case 2: scaleattrib(data[1], attrib.y);
    case 1: scaleattrib(data[0], attrib.x);
    }
    else switch(va.size)
    {
    case 4: putattrib(data[3], attrib.w);
    case 3: putattrib(data[2], attrib.z);
    case 2: putattrib(data[1], attrib.y);
    case 1: putattrib(data[0], attrib.x);
    }
    lilswap(data, va.size);
}

template<int TYPE, int FMT, class T>
static inline void serializeattrib(const vertexarray &va, T *data, const Vec3 &attrib)
{
    if(normalizedattrib<TYPE>()) switch(va.size)
    {
    case 3: scaleattrib(data[2], attrib.z);
    case 2: scaleattrib(data[1], attrib.y);
    case 1: scaleattrib(data[0], attrib.x);
    }
    else switch(va.size)
    {
    case 3: putattrib(data[2], attrib.z);
    case 2: putattrib(data[1], attrib.y);
    case 1: putattrib(data[0], attrib.x);
    }
    lilswap(data, va.size);
}

template<int TYPE, int FMT, class T>
static inline void serializeattrib(const vertexarray &va, T *data, const blendcombo &blend)
{
    if(TYPE == IQM_BLENDINDEXES)
    {
        switch(va.size)
        {
        case 4: putattrib(data[3], blend.bones[3]);
        case 3: putattrib(data[2], blend.bones[2]);
        case 2: putattrib(data[1], blend.bones[1]);
        case 1: putattrib(data[0], blend.bones[0]);
        }
    }
    else if(FMT == IQM_UBYTE)
    {
        uchar weights[4];
        blend.serialize(weights);
        switch(va.size)
        {
        case 4: putattrib(data[3], weights[3]);
        case 3: putattrib(data[2], weights[2]);
        case 2: putattrib(data[1], weights[1]);
        case 1: putattrib(data[0], weights[0]);
        }
    }
    else
    {
        switch(va.size)
        {
        case 4: scaleattrib(data[3], blend.weights[3]);
        case 3: scaleattrib(data[2], blend.weights[2]);
        case 2: scaleattrib(data[1], blend.weights[1]);
        case 1: scaleattrib(data[0], blend.weights[0]);
        }
    }
    lilswap(data, va.size);
}

template<int TYPE, class T>
void setupvertexarray(const vector<T> &attribs, int type, int fmt, int size)
{
    vertexarray &va = varrays.add(vertexarray(type, fmt, size, vdata.length()));
    const char *name = "";
    loopv(evarrays) if(evarrays[i].type == (int)va.type)
    {
        evarray &info = evarrays[i];
        va.format = info.format;
        va.size = clamp(info.size, 1, 4);
        name = info.name;
        break;
    }
    uint align = max(va.formatsize(), 4);
    if(va.offset%align) { uint pad = align - va.offset%align; va.offset += pad; loopi(pad) vdata.add(0); }
    if(va.type >= IQM_CUSTOM)
    {
        if(!name[0])
        {
            defformatstring(customname, "custom%d", va.type-IQM_CUSTOM);
            va.type = IQM_CUSTOM + sharestring(customname);
        }
        else va.type = IQM_CUSTOM + sharestring(name);
    }
    int totalsize = va.bytesize() * vmap.length();
    uchar *data = vdata.reserve(totalsize);
    vdata.advance(totalsize);
    loopv(vmap)
    {
        const T &attrib = attribs[remapindex<TYPE>(i, vmap[i])];
        switch(va.format)
        {
        case IQM_BYTE: serializeattrib<TYPE, IQM_BYTE>(va, (char *)data, attrib); break;
        case IQM_UBYTE: serializeattrib<TYPE, IQM_UBYTE>(va, (uchar *)data, attrib); break;
        case IQM_SHORT: serializeattrib<TYPE, IQM_SHORT>(va, (short *)data, attrib); break;
        case IQM_USHORT: serializeattrib<TYPE, IQM_USHORT>(va, (ushort *)data, attrib); break;
        case IQM_INT: serializeattrib<TYPE, IQM_INT>(va, (int *)data, attrib); break;
        case IQM_UINT: serializeattrib<TYPE, IQM_UINT>(va, (uint *)data, attrib); break;
        case IQM_HALF: serializeattrib<TYPE, IQM_HALF>(va, (halfdata *)data, attrib); break;
        case IQM_FLOAT: serializeattrib<TYPE, IQM_FLOAT>(va, (float *)data, attrib); break;
        case IQM_DOUBLE: serializeattrib<TYPE, IQM_DOUBLE>(va, (double *)data, attrib); break;
        }
        data += va.bytesize();
    }
}

// linear speed vertex cache optimization from Tom Forsyth

#define MAXVCACHE 64 // 32 //< @r-lyeh: optimize

struct triangleinfo
{
    bool used;
    float score;
    uint vert[3];

    triangleinfo() {}
    triangleinfo(uint v0, uint v1, uint v2)
    {
        vert[0] = v0;
        vert[1] = v1;
        vert[2] = v2;
    }
};

struct vertexcache : listnode<vertexcache>
{
    int index, rank;
    float score;
    int numuses;
    triangleinfo **uses;

    vertexcache() : index(-1), rank(-1), score(-1.0f), numuses(0), uses(NULL) {}

    void calcscore()
    {
        if(numuses > 0)
        {
            score = 2.0f * powf(numuses, -0.5f);
            if(rank >= 3) score += powf(1.0f - (rank - 3)/float(MAXVCACHE - 3), 1.5f);
            else if(rank >= 0) score += 0.75f;
        }
        else score = -1.0f;
    }

    void removeuse(triangleinfo *t)
    {
        loopi(numuses) if(uses[i] == t)
        {
            uses[i] = uses[--numuses];
            return;
        }
    }
};

void maketriangles(vector<triangleinfo> &tris, const vector<sharedvert> &mmap)
{
    triangleinfo **uses = new triangleinfo *[3*tris.length()];
    vertexcache *verts = new vertexcache[mmap.length()];
    list<vertexcache> vcache;

    loopv(tris)
    {
        triangleinfo &t = tris[i];
        t.used = t.vert[0] == t.vert[1] || t.vert[1] == t.vert[2] || t.vert[2] == t.vert[0];
        if(t.used) continue;
        loopk(3) verts[t.vert[k]].numuses++;
    }
    triangleinfo **curuse = uses;
    loopvrev(tris)
    {
        triangleinfo &t = tris[i];
        if(t.used) continue;
        loopk(3)
        {
            vertexcache &v = verts[t.vert[k]];
            if(!v.uses) { curuse += v.numuses; v.uses = curuse; }
            *--v.uses = &t;
        }
    }
    loopv(mmap) verts[i].calcscore();
    triangleinfo *besttri = NULL;
    float bestscore = -1e16f;
    loopv(tris)
    {
        triangleinfo &t = tris[i];
        if(t.used) continue;
        t.score = verts[t.vert[0]].score + verts[t.vert[1]].score + verts[t.vert[2]].score;
        if(t.score > bestscore) { besttri = &t; bestscore = t.score; }
    }

    //int reloads = 0, n = 0;
    while(besttri)
    {
        besttri->used = true;
        triangle &t = triangles.add();
        loopk(3)
        {
            vertexcache &v = verts[besttri->vert[k]];
            if(v.index < 0) { v.index = vmap.length(); vmap.add(mmap[besttri->vert[k]]); }
            t.vert[k] = v.index;
            v.removeuse(besttri);
            if(v.rank >= 0) vcache.remove(&v)->rank = -1;
            //else reloads++;
            if(v.numuses <= 0) continue;
            vcache.insertfirst(&v);
            v.rank = 0;
        }
        int rank = 0;
        for(vertexcache *v = vcache.first(); v != vcache.end(); v = v->next)
        {
            v->rank = rank++;
            v->calcscore();
        }
        besttri = NULL;
        bestscore = -1e16f;
        for(vertexcache *v = vcache.first(); v != vcache.end(); v = v->next)
        {
            loopi(v->numuses)
            {
                triangleinfo &t = *v->uses[i];
                t.score = verts[t.vert[0]].score + verts[t.vert[1]].score + verts[t.vert[2]].score;
                if(t.score > bestscore) { besttri = &t; bestscore = t.score; }
            }
        }
        while(vcache.size > MAXVCACHE) vcache.removelast()->rank = -1;
        if(!besttri) loopv(tris)
        {
            triangleinfo &t = tris[i];
            if(!t.used && t.score > bestscore) { besttri = &t; bestscore = t.score; }
        }
    }
    //printf("reloads: %d, worst: %d, best: %d\n", reloads, tris.length()*3, mmap.length());

    delete_array uses;
    delete_array verts;
}

void calctangents(bool areaweight = true)
{
    Vec3 *tangent = new Vec3[2*vmap.length()], *bitangent = tangent+vmap.length();
    memset((void*)tangent, 0, 2*vmap.length()*sizeof(Vec3));
    loopv(triangles)
    {
        const triangle &t = triangles[i];
        sharedvert &i0 = vmap[t.vert[0]],
                   &i1 = vmap[t.vert[1]],
                   &i2 = vmap[t.vert[2]];

        Vec3 v0(epositions[i0.index]), e1 = Vec3(epositions[i1.index]) - v0, e2 = Vec3(epositions[i2.index]) - v0;

        double u1 = etexcoords[i1.index].x - etexcoords[i0.index].x, v1 = etexcoords[i1.index].y - etexcoords[i0.index].y,
               u2 = etexcoords[i2.index].x - etexcoords[i0.index].x, v2 = etexcoords[i2.index].y - etexcoords[i0.index].y;
        Vec3 u = e2*v1 - e1*v2,
             v = e2*u1 - e1*u2;

        if(e2.cross(e1).dot(v.cross(u)) < 0)
        {
            u = -u;
            v = -v;
        }

        if(!areaweight)
        {
            u = u.normalize();
            v = v.normalize();
        }

        loopj(3)
        {
            tangent[t.vert[j]] += u;
            bitangent[t.vert[j]] += v;
        }
    }
    loopv(vmap)
    {
        const Vec3 &n = enormals[vmap[i].weld],
                   &t = tangent[i],
                   &bt = bitangent[i];
        etangents.add(Vec4((t - n*n.dot(t)).normalize(), n.cross(t).dot(bt) < 0 ? -1 : 1));
    }
    delete_array tangent;
}

struct neighborkey
{
    uint e0, e1;

    neighborkey() {}
    neighborkey(uint i0, uint i1)
    {
        if(epositions[i0] < epositions[i1]) { e0 = i0; e1 = i1; }
        else { e0 = i1; e1 = i0; }
    }

    uint hash() const { return hthash(epositions[e0]) + hthash(epositions[e1]); }
    bool operator==(const neighborkey &n) const
    {
        return epositions[e0] == epositions[n.e0] && epositions[e1] == epositions[n.e1] &&
               (eblends.empty() || (eblends[e0] == eblends[n.e0] && eblends[e1] == eblends[n.e1]));
    }
};

static inline uint hthash(const neighborkey &n) { return n.hash(); }
static inline bool htcmp(const neighborkey &x, const neighborkey &y) { return x == y; }

struct neighborval
{
    uint tris[2];

    neighborval() {}
    neighborval(uint i) { tris[0] = i; tris[1] = 0xFFFFFFFFU; }

    void add(uint i)
    {
        if(tris[1] != 0xFFFFFFFFU) tris[0] = tris[1] = 0xFFFFFFFFU;
        else if(tris[0] != 0xFFFFFFFFU) tris[1] = i;
    }

    int opposite(uint i) const
    {
        return tris[0] == i ? tris[1] : tris[0];
    }
};

void makeneighbors()
{
    hashtable<neighborkey, neighborval> nhash;

    loopv(triangles)
    {
        triangle &t = triangles[i];
        for(int j = 0, p = 2; j < 3; p = j, j++)
        {
            neighborkey key(t.vert[p], t.vert[j]);
            neighborval *val = nhash.access(key);
            if(val) val->add(i);
            else nhash[key] = neighborval(i);
        }
    }

    loopv(triangles)
    {
        triangle &t = triangles[i];
        triangle &n = neighbors.add();
        for(int j = 0, p = 2; j < 3; p = j, j++)
            n.vert[p] = nhash[neighborkey(t.vert[p], t.vert[j])].opposite(i);
    }
}

Quat erotate(0, 0, 0, 1);
double escale = 1;
Vec3 emeshtrans(0, 0, 0);

void makemeshes()
{
    meshes.setsize(0);
    triangles.setsize(0);
    neighbors.setsize(0);
    vmap.setsize(0);
    varrays.setsize(0);
    vdata.setsize(0);

    hashtable<sharedvert, uint> mshare(max(1<<12, etriangles.length() * 3));
    vector<sharedvert> mmap;
    vector<triangleinfo> tinfo;

    loopv(emeshes)
    {
        emesh &em1 = emeshes[i];
        if(em1.used) continue;
        for(int j = i; j < emeshes.length(); j++)
        {
            emesh &em = emeshes[j];
            if(em.name != em1.name || em.material != em1.material) continue;
            int lasttri = emeshes.inrange(i+1) ? emeshes[i+1].firsttri : etriangles.length();
            for(int k = em.firsttri; k < lasttri; k++)
            {
                etriangle &et = etriangles[k];
                triangleinfo &t = tinfo.add();
                loopl(3)
                {
                    sharedvert v(et.vert[l], et.weld[l]);
                    t.vert[l] = mshare.access(v, mmap.length());
                    if(!mmap.inrange(t.vert[l])) mmap.add(v);
                }
            }
            em.used = true;
        }
        if(tinfo.empty()) continue;

        mesh &m = meshes.add();
        m.name = sharestring(em1.name);
        m.material = sharestring(em1.material);
        m.firsttri = triangles.length();
        m.firstvert = vmap.length();
        maketriangles(tinfo, mmap);
        m.numtris = triangles.length() - m.firsttri;
        m.numverts = vmap.length() - m.firstvert;

        mshare.clear();
        mmap.setsize(0);
        tinfo.setsize(0);
    }

    if(triangles.length()) makeneighbors();

    if(escale != 1) loopv(epositions) epositions[i] *= escale;
    if(erotate != Quat(0, 0, 0, 1))
    {
        loopv(epositions) epositions[i].setxyz(erotate.transform(Vec3(epositions[i])));
        loopv(enormals) enormals[i] = erotate.transform(enormals[i]);
        loopv(etangents) etangents[i].setxyz(erotate.transform(Vec3(etangents[i])));
        loopv(ebitangents) ebitangents[i] = erotate.transform(ebitangents[i]);
    }
    if(emeshtrans != Vec3(0, 0, 0)) loopv(epositions) epositions[i] += emeshtrans;
    if(epositions.length()) setupvertexarray<IQM_POSITION>(epositions, IQM_POSITION, IQM_FLOAT, 3);
    if(etexcoords.length()) setupvertexarray<IQM_TEXCOORD>(etexcoords, IQM_TEXCOORD, IQM_FLOAT, 2);
    if(enormals.length()) setupvertexarray<IQM_NORMAL>(enormals, IQM_NORMAL, IQM_FLOAT, 3);
    if(etangents.length())
    {
        if(ebitangents.length() && enormals.length())
        {
            loopv(etangents) if(ebitangents.inrange(i) && enormals.inrange(i))
                etangents[i].w = enormals[i].cross(Vec3(etangents[i])).dot(ebitangents[i]) < 0 ? -1 : 1;
        }
        setupvertexarray<IQM_TANGENT>(etangents, IQM_TANGENT, IQM_FLOAT, 4);
    }
    else if(enormals.length() && etexcoords.length())
    {
        calctangents();
        setupvertexarray<IQM_TANGENT>(etangents, IQM_TANGENT, IQM_FLOAT, 4);
    }
    if(eblends.length())
    {
        setupvertexarray<IQM_BLENDINDEXES>(eblends, IQM_BLENDINDEXES, IQM_UBYTE, 4);
        setupvertexarray<IQM_BLENDWEIGHTS>(eblends, IQM_BLENDWEIGHTS, IQM_UBYTE, 4);
    }
    if(ecolors.length()) setupvertexarray<IQM_COLOR>(ecolors, IQM_COLOR, IQM_UBYTE, 4);
    loopi(10) if(ecustom[i].length()) setupvertexarray<IQM_CUSTOM>(ecustom[i], IQM_CUSTOM + i, IQM_FLOAT, 4);

    if(epositions.length())
    {
        mpositions.setsize(0);
        mpositions.swap(epositions);
    }
    if(eblends.length())
    {
        mblends.setsize(0);
        mblends.swap(eblends);
    }
}

void makebounds(framebounds &bb, Matrix3x4 *buf, Matrix3x4 *invbase, transform *frame)
{
if(frame) //< @r-lyeh: make static meshes generate bounding boxes as well
    loopv(ejoints)
    {
        ejoint &j = ejoints[i];
        if(j.parent >= 0) buf[i] = buf[j.parent] * Matrix3x4(frame[i].orient, frame[i].pos, frame[i].scale);
        else buf[i] = Matrix3x4(frame[i].orient, frame[i].pos, frame[i].scale);
    }
if (invbase) //< @r-lyeh: make static meshes generate bounding boxes as well
    loopv(ejoints) buf[i] *= invbase[i];
    loopv(mpositions)
    {
        const blendcombo &c = mblends[i];
        Matrix3x4 m(Vec4(0, 0, 0, 0), Vec4(0, 0, 0, 0), Vec4(0, 0, 0, 0));
if(!frame) m = Matrix3x4(Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0), Vec4(0, 0, 1, 0)); else //< @r-lyeh: make static meshes generate bounding boxes as well
        loopk(4) if(c.weights[k] > 0)
            m += buf[c.bones[k]] * c.weights[k];
        Vec3 p = m.transform(Vec3(mpositions[i]));

        if(!i) bb.bbmin = bb.bbmax = p;
        else
        {
            bb.bbmin.x = min(bb.bbmin.x, p.x);
            bb.bbmin.y = min(bb.bbmin.y, p.y);
            bb.bbmin.z = min(bb.bbmin.z, p.z);
            bb.bbmax.x = max(bb.bbmax.x, p.x);
            bb.bbmax.y = max(bb.bbmax.y, p.y);
            bb.bbmax.z = max(bb.bbmax.z, p.z);
        }
        double xyradius = p.x*p.x + p.y*p.y;
        bb.xyradius = max(bb.xyradius, xyradius);
        bb.radius = max(bb.radius, xyradius + p.z*p.z);
    }
    if(bb.xyradius > 0) bb.xyradius = sqrt(bb.xyradius);
    if(bb.radius > 0) bb.radius = sqrt(bb.radius);
}

void makerelativebasepose()
{
    int numbasejoints = min(ejoints.length(), eframes.length() ? eframes[0] : eposes.length());
    for(int i = numbasejoints-1; i >= 0; i--)
    {
        ejoint &ej = ejoints[i];
        if(ej.parent < 0) continue;
        transform &parent = eposes[ej.parent], &child = eposes[i];
        child.pos = (-parent.orient).transform(child.pos - parent.pos);
        child.orient = (-parent.orient)*child.orient;
        if(child.orient.w > 0) child.orient.flip();
    }
}

bool forcejoints = false;

void makeanims()
{
    if(escale != 1) loopv(eposes) eposes[i].pos *= escale;
    if(erotate != Quat(0, 0, 0, 1)) loopv(ejoints)
    {
        ejoint &ej = ejoints[i];
        if(ej.parent < 0) for(int j = i; j < eposes.length(); j += ejoints.length())
        {
            transform &p = eposes[j];
            p.orient = erotate * p.orient;
            p.pos = erotate.transform(p.pos);
        }
    }
    int numbasejoints = eframes.length() ? eframes[0] : eposes.length();
    if((forcejoints || meshes.length()) && joints.empty())
    {
        mjoints.setsize(0);
        loopv(ejoints)
        {
            ejoint &ej = ejoints[i];
            joint &j = joints.add();
            j.name = sharestring(ej.name);
            j.parent = ej.parent;
            if(i < numbasejoints)
            {
                mjoints.add().invert(Matrix3x4(eposes[i].orient, eposes[i].pos, eposes[i].scale));
                loopk(3) j.pos[k] = eposes[i].pos[k];
                loopk(4) j.orient[k] = eposes[i].orient[k];
                loopk(3) j.scale[k] = eposes[i].scale[k];
            }
            else mjoints.add().invert(Matrix3x4(Quat(0, 0, 0, 1), Vec3(0, 0, 0), Vec3(1, 1, 1)));
            if(ej.parent >= 0) mjoints[i] *= mjoints[ej.parent];
        }
    }
    if(eanims.empty()) { makebounds(bounds.add(), new Matrix3x4, NULL, NULL); return; } //return; //< @r-lyeh: make static meshes generate bounding boxes as well
    if(poses.empty()) loopv(ejoints)
    {
        ejoint &ej = ejoints[i];
        pose &p = poses.add();
        p.parent = ej.parent;
    }
    if(poses.empty()) return;
    int totalframes = frames.length()/poses.length();
    Matrix3x4 *mbuf = mpositions.length() && mblends.length() && mjoints.length() ? new Matrix3x4[poses.length()] : NULL;
    loopv(eanims)
    {
        eanim &ea = eanims[i];
        anim &a = anims.add();
        a.name = sharestring(ea.name);
        a.firstframe = totalframes;
        a.numframes = 0;
        a.fps = ea.fps;
        a.flags = ea.flags;
        for(int j = ea.startframe, end = eanims.inrange(i+1) ? eanims[i+1].startframe : eframes.length(); j < end && j <= ea.endframe; j++)
        {
            int offset = eframes[j], range = (eframes.inrange(j+1) ? eframes[j+1] : eposes.length()) - offset;
            if(range <= 0) continue;
            loopk(min(range, poses.length())) frames.add(eposes[offset + k]);
            loopk(max(poses.length() - range, 0)) frames.add(transform(Vec3(0, 0, 0), Quat(0, 0, 0, 1), Vec3(1, 1, 1)));
            if(mbuf) makebounds(bounds.add(), mbuf, mjoints.getbuf(), &frames[frames.length() - poses.length()]);
            a.numframes++;
        }
        totalframes += a.numframes;
    }
    if(mbuf) delete_array mbuf;
}

bool resetimporter(bool reuse = false)
{
    if(reuse)
    {
        ejoints.setsize(0);
        evarrays.setsize(0);

        return false;
    }

    epositions.setsize(0);
    etexcoords.setsize(0);
    etangents.setsize(0);
    ebitangents.setsize(0);
    ecolors.setsize(0);
    loopi(10) ecustom[i].setsize(0);
    eblends.setsize(0);
    etriangles.setsize(0);
    esmoothindexes.setsize(0);
    esmoothedges.setsize(0);
    esmoothgroups.setsize(0);
    esmoothgroups.add();
    ejoints.setsize(0);
    eposes.setsize(0);
    eframes.setsize(0);
    eanims.setsize(0);
    emeshes.setsize(0);
    evarrays.setsize(0);
    erotate = Quat(0, 0, 0, 1);

    return true;
}

struct filespec
{
    const char *file;
    const char *name;
    double fps;
    uint flags;
    int startframe;
    int endframe;

    filespec() { reset(); }

    void reset()
    {
        file = NULL;
        name = NULL;
        fps = 0;
        flags = 0;
        startframe = 0;
        endframe = -1;
    }
};

bool parseiqe(stream *f)
{
    enum { sizeof_tex = (8*1024) * (8*1024) * (4) }; // max cap: 8K texture, RGBA8888,
    const unsigned sizeof_buf = ceil(sizeof_tex / 3) * 4; // then, max capacity encoded as base64
    static char *buf = new char [sizeof_buf];

    const char *curmesh = getnamekey(""), *curmaterial = getnamekey("");
    bool needmesh = true;
    int fmoffset = 0;
    if(!f->getline(buf, sizeof_buf)) return false;
    if(!strchr(buf, '#') || strstr(buf, "# Inter-Quake Export") != strchr(buf, '#')) return false;
    while(f->getline(buf, sizeof_buf))
    {
        char *c = buf;
        while(isspace(*c)) ++c;
        if(isalpha(c[0]) && isalnum(c[1]) && (!c[2] || isspace(c[2]))) switch(*c++)
        {
        case 'v':
            switch(*c++)
            {
            case 'p': epositions.add(parseattribs4(c, Vec4(0, 0, 0, 1))); continue;
            case 't': etexcoords.add(parseattribs4(c)); continue;
            case 'n': enormals.add(parseattribs3(c)); continue;
            case 'x':
            {
                Vec4 tangent(parseattribs3(c), 0);
                Vec3 bitangent(0, 0, 0);
                bitangent.x = parseattrib(c);
                if(maybeparseattrib(c, bitangent.y))
                {
                    bitangent.z = parseattrib(c);
                    ebitangents.add(bitangent);
                }
                else tangent.w = bitangent.x;
                etangents.add(tangent);
                continue;
            }
            case 'b': eblends.add(parseblends(c)); continue;
            case 'c': ecolors.add(parseattribs4(c, Vec4(0, 0, 0, 1))); continue;
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            {
                int n = c[-1] - '0';
                ecustom[n].add(parseattribs4(c));
                continue;
            }
            case 's':
                parseindex(c, esmoothindexes.add());
                continue;
            }
            break;
        case 'p':
        {
            transform t;
            switch(*c++)
            {
            case 'q':
                {
                    t.pos = parseattribs3(c);
                    loopk(3) t.orient[k] = parseattrib(c);
                    t.orient.restorew();
                    double w = parseattrib(c, t.orient.w);
                    if(w != t.orient.w)
                    {
                        t.orient.w = w;
                        t.orient.normalize();
//        double x2 = f.orient.x*f.orient.x, y2 = f.orient.y*f.orient.y, z2 = f.orient.z*f.orient.z, w2 = f.orient.w*f.orient.w, s2 = x2 + y2 + z2 + w2;
//        f.orient.x = keepsign(f.orient.x, sqrt(max(1.0 - (w2 + y2 + z2) / s2, 0.0)));
//        f.orient.y = keepsign(f.orient.y, sqrt(max(1.0 - (w2 + x2 + z2) / s2, 0.0)));
//        f.orient.z = keepsign(f.orient.z, sqrt(max(1.0 - (w2 + x2 + y2) / s2, 0.0)));
//        f.orient.w = keepsign(f.orient.w, sqrt(max(1.0 - (x2 + y2 + z2) / s2, 0.0)));
                    }
                    if(t.orient.w > 0) t.orient.flip();
                    t.scale = parseattribs3(c, Vec3(1, 1, 1));
                    eposes.add(t);
                    continue;
                }
            case 'm':
                {
                    t.pos = parseattribs3(c);
                    Matrix3x3 m;
                    m.a = parseattribs3(c);
                    m.b = parseattribs3(c);
                    m.c = parseattribs3(c);
                    Vec3 mscale(Vec3(m.a.x, m.b.x, m.c.x).magnitude(), Vec3(m.a.y, m.b.y, m.c.y).magnitude(), Vec3(m.a.z, m.b.z, m.c.z).magnitude());
                    // check determinant for sign of scaling
                    if(m.determinant() < 0) mscale = -mscale;
                    m.a /= mscale;
                    m.b /= mscale;
                    m.c /= mscale;
                    t.orient = Quat(m);
                    if(t.orient.w > 0) t.orient.flip();
                    t.scale = parseattribs3(c, Vec3(1, 1, 1)) * mscale;
                    eposes.add(t);
                    continue;
                }
            case 'a':
                {
                    t.pos = parseattribs3(c);
                    Vec3 rot = parseattribs3(c);
                    t.orient = Quat::fromangles(rot);
                    t.scale = parseattribs3(c, Vec3(1, 1, 1));
                    eposes.add(t);
                    continue;
                }
            }
            break;
        }
        case 'f':
            switch(*c++)
            {
            case 'a':
                {
                    int i1 = 0, i2 = 0, i3 = 0;
                    if(!parseindex(c, i1) || !parseindex(c, i2)) continue;
                    if(needmesh)
                    {
                        emeshes.add(emesh(curmesh, curmaterial, etriangles.length()));
                        needmesh = false;
                    }
                    if(i1 < 0) i1 = max(epositions.length() + i1, 0);
                    if(i2 < 0) i2 = max(epositions.length() + i2, 0);
                    while(parseindex(c, i3))
                    {
                        if(i3 < 0) i3 = max(epositions.length() + i3, 0);
                        esmoothgroups.last().flags |= esmoothgroup::F_USED;
                        etriangles.add(etriangle(i1, i2, i3, esmoothgroups.length()-1));
                        i2 = i3;
                    }
                    continue;
                }
            case 'm':
                {
                    int i1 = 0, i2 = 0, i3 = 0;
                    if(!parseindex(c, i1) || !parseindex(c, i2)) continue;
                    if(needmesh)
                    {
                        emeshes.add(emesh(curmesh, curmaterial, etriangles.length()));
                        needmesh = false;
                    }
                    i1 = i1 < 0 ? max(epositions.length() + i1, 0) : (fmoffset + i1);
                    i2 = i2 < 0 ? max(epositions.length() + i2, 0) : (fmoffset + i2);
                    while(parseindex(c, i3))
                    {
                        i3 = i3 < 0 ? max(epositions.length() + i3, 0) : (fmoffset + i3);
                        esmoothgroups.last().flags |= esmoothgroup::F_USED;
                        etriangles.add(etriangle(i1, i2, i3, esmoothgroups.length()-1));
                        i2 = i3;
                    }
                    continue;
                }
            case 's':
                {
                    int i1 = 0, i2 = 0, i3 = 0;
                    uchar flags = 0;
                    if(!parseindex(c, i1) || !parseindex(c, i2) || !parseindex(c, i3)) continue;
                    flags |= clamp(i1, 0, 1);
                    flags |= clamp(i2, 0, 1)<<1;
                    flags |= clamp(i3, 0, 1)<<2;
                    esmoothgroups.last().flags |= esmoothgroup::F_USED;
                    while(parseindex(c, i3))
                    {
                        esmoothedges.add(flags | 4);
                        flags = 1 | ((flags & 4) >> 1) | (clamp(i3, 0, 1)<<2);
                    }
                    esmoothedges.add(flags);
                    continue;
                }
            }
            break;
        }
        char *args = c;
        while(*args && !isspace(*args)) args++;
        if(!strncmp(c, "smoothgroup", max(int(args-c), 11)))
        {
            if(esmoothgroups.last().flags & esmoothgroup::F_USED) esmoothgroups.dup();
            parseindex(args, esmoothgroups.last().key);
        }
        else if(!strncmp(c, "smoothangle", max(int(args-c), 11)))
        {
            if(esmoothgroups.last().flags & esmoothgroup::F_USED) esmoothgroups.dup();
            double angle = parseattrib(args, 0);
            esmoothgroups.last().angle = fabs(cos(clamp(angle, -180.0, 180.0) * M_PI/180));
        }
        else if(!strncmp(c, "smoothuv", max(int(args-c), 8)))
        {
            if(esmoothgroups.last().flags & esmoothgroup::F_USED) esmoothgroups.dup();
            int val = 1;
            if(parseindex(args, val) && val <= 0) esmoothgroups.last().flags &= ~esmoothgroup::F_UVSMOOTH;
            else esmoothgroups.last().flags |= esmoothgroup::F_UVSMOOTH;
        }
        else if(!strncmp(c, "mesh", max(int(args-c), 4)))
        {
            curmesh = getnamekey(trimname(args));
            if(emeshes.empty() || emeshes.last().name != curmesh) needmesh = true;
            fmoffset = epositions.length();

#if 0
            emesh &m = emeshes.add();
            m.firsttri = etriangles.length();
            fmoffset = epositions.length();
            parsename(args, m.name);
#endif
        }
        else if(!strncmp(c, "material", max(int(args-c), 8)))
        {
            curmaterial = getnamekey(trimname(args));
            if(emeshes.empty() || emeshes.last().material != curmaterial) needmesh = true;
//            if(emeshes.length()) parsename(c, emeshes.last().material);
        }
        else if(!strncmp(c, "joint", max(int(args-c), 5)))
        {
            ejoint &j = ejoints.add();
            j.name = getnamekey(trimname(args));
            parseindex(args, j.parent);
        }
        else if(!strncmp(c, "vertexarray", max(int(args-c), 11)))
        {
            evarray &va = evarrays.add();
            va.type = findvertexarraytype(trimname(args));
            va.format = findvertexarrayformat(trimname(args));
            va.size = strtol(args, &args, 10);
            copystring(va.name, trimname(args));
        }
        else if(!strncmp(c, "animation", max(int(args-c), 9)))
        {
            eanim &a = eanims.add();
            a.name = getnamekey(trimname(args));
            a.startframe = eframes.length();
            if(!eframes.length() || eframes.last() != eposes.length()) eframes.add(eposes.length());
        }
        else if(!strncmp(c, "frame", max(int(args-c), 5)))
        {
            if(eanims.length() && eframes.length() && eframes.last() != eposes.length()) eframes.add(eposes.length());
        }
        else if(!strncmp(c, "framerate", max(int(args-c), 9)))
        {
            if(eanims.length())
            {
                double fps = parseattrib(args);
                eanims.last().fps = max(fps, 0.0);
            }
        }
        else if(!strncmp(c, "loop", max(int(args-c), 4)))
        {
            if(eanims.length()) eanims.last().flags |= IQM_LOOP;
        }
        else if(!strncmp(c, "comment", max(int(args-c), 7)))
        {
            if(commentdata.length()) break;
            for(;;)
            {
                int len = f->read(commentdata.reserve(1024), 1024);
                commentdata.advance(len);
                if(len < 1024) { commentdata.add('\0'); break; }
            }
        }
    }

    return true;
}

bool loadiqe(const char *filename, const filespec &spec)
{
    int numfiles = 0;
    while(filename)
    {
        const char *endfile = strchr(filename, ',');
        const char *file = endfile ? newstring(filename, endfile-filename) : filename;
        stream *f = openfile(file, "r");
        if(f)
        {
            resetimporter(numfiles > 0);
            if(parseiqe(f)) numfiles++;
            delete f;
        }

        if(!endfile) break;

        delete_array file;
        filename = endfile+1;
    }

    if(!numfiles) return false;

    if(eanims.length() == 1)
    {
        eanim &a = eanims.last();
        if(spec.name) a.name = spec.name;
        if(spec.fps > 0) a.fps = spec.fps;
        a.flags |= spec.flags;
        if(spec.endframe >= 0) a.endframe = a.startframe + spec.endframe;
        else if(spec.endframe < -1) a.endframe = a.startframe + max(eframes.length() - a.startframe + spec.endframe + 1, 0);
        a.startframe += spec.startframe;
    }

    if(emeshes.length())
    {
        smoothverts();
        makemeshes();
    }
    makeanims();

    return true;
}

struct objvert { int attrib[3]; objvert() { attrib[0] = attrib[1] = attrib[2] = -1; } };
static inline uint hthash(const objvert &k) { return k.attrib[0] ^ k.attrib[1] ^ k.attrib[2]; };
static inline bool htcmp(const objvert &x, const objvert &y) { return x.attrib[0] == y.attrib[0] && x.attrib[1] == y.attrib[1] && x.attrib[2] == y.attrib[2]; }

void parseobjvert(char *s, vector<Vec3> &out)
{
    Vec3 &v = out.add(Vec3(0, 0, 0));
    while(isalpha(*s)) s++;
    loopi(3)
    {
        v[i] = strtod(s, &s);
        while(isspace(*s)) s++;
        if(!*s) break;
    }
}

bool parseobj(stream *f)
{
    vector<Vec3> attrib[3];
    char buf[512];
    hashtable<objvert, int> verthash;
    string meshname = "", matname = "";
    int curmesh = -1, smooth = 0;

    while(f->getline(buf, sizeof(buf)))
    {
        char *c = buf;
        while(isspace(*c)) c++;
        switch(*c)
        {
            case '#': continue;
            case 'v':
                if(isspace(c[1])) parseobjvert(c, attrib[0]);
                else if(c[1]=='t') parseobjvert(c, attrib[1]);
                else if(c[1]=='n') parseobjvert(c, attrib[2]);
                break;
            case 'g':
            {
                while(isalpha(*c)) c++;
                while(isspace(*c)) c++;
                char *name = c;
                size_t namelen = strlen(name);
                while(namelen > 0 && isspace(name[namelen-1])) namelen--;
                copystring(meshname, name, min(namelen+1, sizeof(meshname)));
                curmesh = -1;
                break;
            }
            case 'u':
            {
                if(strncmp(c, "usemtl", 6)) continue;
                while(isalpha(*c)) c++;
                while(isspace(*c)) c++;
                char *name = c;
                size_t namelen = strlen(name);
                while(namelen > 0 && isspace(name[namelen-1])) namelen--;
                copystring(matname, name, min(namelen+1, sizeof(matname)));
                curmesh = -1;
                break;
            }
            case 's':
            {
                if(!isspace(c[1])) continue;
                while(isalpha(*c)) c++;
                while(isspace(*c)) c++;
                int key = strtol(c, &c, 10);
                smooth = -1;
                loopv(esmoothgroups) if(esmoothgroups[i].key == key) { smooth = i; break; }
                if(smooth < 0)
                {
                    smooth = esmoothgroups.length();
                    esmoothgroups.add().key = key;
                }
                break;
            }
            case 'f':
            {
                if(curmesh < 0)
                {
                    emesh m;
                    m.name = getnamekey(meshname);
                    m.material = getnamekey(matname);
                    m.firsttri = etriangles.length();
                    curmesh = emeshes.length();
                    emeshes.add(m);
                    verthash.clear();
                }
                int v0 = -1, v1 = -1;
                while(isalpha(*c)) c++;
                for(;;)
                {
                    while(isspace(*c)) c++;
                    if(!*c) break;
                    objvert vkey;
                    loopi(3)
                    {
                        vkey.attrib[i] = strtol(c, &c, 10);
                        if(vkey.attrib[i] < 0) vkey.attrib[i] = attrib[i].length() + vkey.attrib[i];
                        else vkey.attrib[i]--;
                        if(!attrib[i].inrange(vkey.attrib[i])) vkey.attrib[i] = -1;
                        if(*c!='/') break;
                        c++;
                    }
                    int *index = verthash.access(vkey);
                    if(!index)
                    {
                        index = &verthash[vkey];
                        *index = epositions.length();
                        epositions.add(Vec4(vkey.attrib[0] < 0 ? Vec3(0, 0, 0) : attrib[0][vkey.attrib[0]].zxy(), 1));
                        if(vkey.attrib[2] >= 0) enormals.add(attrib[2][vkey.attrib[2]].zxy());
                        etexcoords.add(vkey.attrib[1] < 0 ? Vec4(0, 0, 0, 0) : Vec4(attrib[1][vkey.attrib[1]].x, 1-attrib[1][vkey.attrib[1]].y, 0, 0));
                    }
                    if(v0 < 0) v0 = *index;
                    else if(v1 < 0) v1 = *index;
                    else
                    {
                        etriangles.add(etriangle(*index, v1, v0, smooth));
                        v1 = *index;
                    }
                }
                break;
            }
        }
    }

    return true;
}





int framesize = 0;
vector<ushort> animdata;

#define QUANTIZE(offset, base, scale) ushort(0.5f + (float(offset) - base) / scale)

void calcanimdata()
{
    if(frames.length()) loopv(poses)
    {
        pose &j = poses[i];
        loopk(10) { j.offset[k] = 1e16f; j.scale[k] = -1e16f; }
    }
    loopv(frames)
    {
        pose &j = poses[i%poses.length()];
        transform &f = frames[i];
        loopk(3)
        {
            j.offset[k] = min(j.offset[k], float(f.pos[k]));
            j.scale[k] = max(j.scale[k], float(f.pos[k]));
        }
        loopk(4)
        {
            j.offset[3+k] = min(j.offset[3+k], float(f.orient[k]));
            j.scale[3+k] = max(j.scale[3+k], float(f.orient[k]));
        }
        loopk(3)
        {
            j.offset[7+k] = min(j.offset[7+k], float(f.scale[k]));
            j.scale[7+k] = max(j.scale[7+k], float(f.scale[k]));
        }
    }
    loopv(poses)
    {
        pose &j = poses[i];
        loopk(10)
        {
            j.scale[k] -= j.offset[k];
            if(j.scale[k] >= 1e-10f) { framesize++; j.scale[k] /= 0xFFFF; j.flags |= 1<<k; }
            else j.scale[k] = 0.0f;
        }
    }
#if 0
    int runlength = 0, blocksize = 0, blocks = 0;
    #define FLUSHVAL(val) \
        if(!blocksize || (animdata.last() == val ? runlength >= 0xFF : runlength || blocksize > 0xFF)) \
        { \
            animdata.add(0); \
            animdata.add(val); \
            blocksize = 1; \
            runlength = 0; \
            blocks++; \
        } \
        else if(animdata.last() == val) \
        { \
            animdata[animdata.length()-blocksize-1] += 0x10; \
            runlength++; \
        } \
        else \
        { \
            animdata[animdata.length()-blocksize-1]++; \
            animdata.add(val); \
            blocksize++; \
        }
    loopv(joints)
    {
        joint &j = joints[i];
        loopk(3) if(j.flags & (0x01<<k))
        {
            for(int l = i; l < frames.length(); l += poses.length())
            {
                transform &f = frames[l];
                ushort val = QUANTIZE(f.pos[k], j.offset[k], j.scale[k]);
                FLUSHVAL(val);
            }
        }
        loopk(4) if(j.flags & (0x08<<k))
        {
            for(int l = i; l < frames.length(); l += poses.length())
            {
                transform &f = frames[l];
                ushort val = QUANTIZE(f.orient[k], j.offset[3+k], j.scale[3+k]);
                FLUSHVAL(val);
            }
        }
        loopk(3) if(j.flags & (0x80<<k))
        {
            for(int l = i; l < frames.length(); l += poses.length())
            {
                transform &f = frames[l];
                ushort val = QUANTIZE(f.scale[k], j.offset[7+k], j.scale[7+k]);
                FLUSHVAL(val);
            }
        }
    }
    printf("%d frames of size %d/%d compressed from %d/%d to %d in %d blocks", frames.length()/poses.length(), framesize, poses.length()*9, framesize*frames.length()/poses.length(), poses.length()*9*frames.length()/poses.length(), animdata.length(), blocks);
#else
    loopv(frames)
    {
        pose &j = poses[i%poses.length()];
        transform &f = frames[i];
        loopk(3) if(j.flags & (0x01<<k)) animdata.add(QUANTIZE(f.pos[k], j.offset[k], j.scale[k]));
        loopk(4) if(j.flags & (0x08<<k)) animdata.add(QUANTIZE(f.orient[k], j.offset[3+k], j.scale[3+k]));
        loopk(3) if(j.flags & (0x80<<k)) animdata.add(QUANTIZE(f.scale[k], j.offset[7+k], j.scale[7+k]));
    }
#endif
    while(vdata.length()%4) vdata.add(0);
    while(stringdata.length()%4) stringdata.add('\0');
    while(commentdata.length()%4) commentdata.add('\0');
    while(animdata.length()%2) animdata.add(0);
}

bool writeiqm(const char *filename)
{
    stream *f = openfile(filename, "wb");
    if(!f) return false;

    iqmheader hdr;
    memset(&hdr, 0, sizeof(hdr));
    copystring(hdr.magic, IQM_MAGIC, sizeof(hdr.magic));
    hdr.filesize = sizeof(hdr);
    hdr.version = IQM_VERSION;
    if(stringdata.length()) { hdr.ofs_text = hdr.filesize; } hdr.num_text = stringdata.length(); hdr.filesize += hdr.num_text;
    hdr.num_meshes = meshes.length(); if(meshes.length()) hdr.ofs_meshes = hdr.filesize; hdr.filesize += meshes.length() * sizeof(mesh);
    uint voffset = hdr.filesize + varrays.length() * sizeof(vertexarray);
    hdr.num_vertexarrays = varrays.length(); if(varrays.length()) hdr.ofs_vertexarrays = hdr.filesize; hdr.filesize += varrays.length() * sizeof(vertexarray);
    uint valign = (8 - (hdr.filesize%8))%8;
    voffset += valign;
    hdr.filesize += valign + vdata.length();
    hdr.num_vertexes = vmap.length();
    hdr.num_triangles = triangles.length(); if(triangles.length()) hdr.ofs_triangles = hdr.filesize; hdr.filesize += triangles.length() * sizeof(triangle);
    if(neighbors.length()) { hdr.ofs_adjacency = hdr.filesize; } hdr.filesize += neighbors.length() * sizeof(triangle);
    hdr.num_joints = joints.length(); if(joints.length()) hdr.ofs_joints = hdr.filesize; hdr.filesize += joints.length() * sizeof(joint);
    hdr.num_poses = poses.length(); if(poses.length()) hdr.ofs_poses = hdr.filesize; hdr.filesize += poses.length() * sizeof(pose);
    hdr.num_anims = anims.length(); if(anims.length()) hdr.ofs_anims = hdr.filesize; hdr.filesize += anims.length() * sizeof(anim);
    hdr.num_frames = poses.length() ? frames.length()/poses.length() : 0; hdr.num_framechannels = framesize;
    if(animdata.length()) { hdr.ofs_frames = hdr.filesize; } hdr.filesize += animdata.length() * sizeof(ushort);
    if(bounds.length()) { hdr.ofs_bounds = hdr.filesize; } hdr.filesize += bounds.length() * sizeof(float[8]);
    if(commentdata.length()) { hdr.ofs_comment = hdr.filesize; } hdr.num_comment = commentdata.length(); hdr.filesize += hdr.num_comment;

    lilswap(&hdr.version, (sizeof(hdr) - sizeof(hdr.magic))/sizeof(uint));
    f->write(&hdr, sizeof(hdr));

    if(stringdata.length()) f->write(stringdata.getbuf(), stringdata.length());

    loopv(meshes)
    {
        mesh &m = meshes[i];
        f->putlil(m.name);
        f->putlil(m.material);
        f->putlil(m.firstvert);
        f->putlil(m.numverts);
        f->putlil(m.firsttri);
        f->putlil(m.numtris);
    }

    loopv(varrays)
    {
        vertexarray &v = varrays[i];
        f->putlil(v.type);
        f->putlil(v.flags);
        f->putlil(v.format);
        f->putlil(v.size);
        f->putlil(voffset + v.offset);
    }

    loopi(valign) f->putchar(0);
    f->write(vdata.getbuf(), vdata.length());

    loopv(triangles)
    {
        triangle &t = triangles[i];
        loopk(3) f->putlil(t.vert[k]);
    }

    loopv(neighbors)
    {
        triangle &t = neighbors[i];
        loopk(3) f->putlil(t.vert[k]);
    }

    loopv(joints)
    {
        joint &j = joints[i];
        f->putlil(j.name);
        f->putlil(j.parent);
        loopk(3) f->putlil(float(j.pos[k]));
        loopk(4) f->putlil(float(j.orient[k]));
        loopk(3) f->putlil(float(j.scale[k]));
    }

    loopv(poses)
    {
        pose &p = poses[i];
        f->putlil(p.parent);
        f->putlil(p.flags);
        loopk(10) f->putlil(p.offset[k]);
        loopk(10) f->putlil(p.scale[k]);
    }

    loopv(anims)
    {
        anim &a = anims[i];
        f->putlil(a.name);
        f->putlil(a.firstframe);
        f->putlil(a.numframes);
        f->putlil(a.fps);
        f->putlil(a.flags);
    }

    loopv(animdata) f->putlil(animdata[i]);

    loopv(bounds)
    {
        framebounds &b = bounds[i];
        loopk(3) f->putlil(float(b.bbmin[k]));
        loopk(3) f->putlil(float(b.bbmax[k]));
        f->putlil(float(b.xyradius));
        f->putlil(float(b.radius));
    }

    if(commentdata.length()) f->write(commentdata.getbuf(), commentdata.length());

    delete f;
    return true;
}


void help(bool exitstatus = EXIT_SUCCESS)
{
    fprintf(exitstatus != EXIT_SUCCESS ? stderr : stdout,
"Usage:\n"
"\n"
"./iqm [options] output.iqm mesh.iqe anim1.iqe ... animN.iqe\n"
"\n"
"It is possible to combine multiple mesh files of the exact same vertex layout\n"
"and skeleton by supplying them as \"mesh1.iqe,mesh2.iqe,mesh3.iqe\", that is,\n"
"a comma-separated list of the mesh files (with no spaces) in place of the usual\n"
"mesh filename.\n"
"\n"
"Options can be any of the following command-line switches:\n"
"\n"
"    -s N\n"
"    --scale N\n"
"      Sets the output scale to N (float).\n"
"\n"
"    --meshtrans Z\n"
"    --meshtrans X,Y,Z\n"
"      Translates a mesh by X,Y,Z (floats). This does not affect the skeleton.\n"
"\n"
"    -j\n"
"    --forcejoints\n"
"      Forces the exporting of joint information in animation files without\n"
"      meshes.\n"
"\n"
"Each animation file can be preceded by any combination of the following command-\n"
"line switches:\n"
"\n"
"    --name A\n"
"      Sets the name of the animation to A.\n"
"    --fps N\n"
"      Sets the FPS of the animation to N (float).\n"
"    --loop\n"
"      Sets the loop flag for the animation.\n"
"    --start N\n"
"      Sets the first frame of the animation to N (integer).\n"
"    --end N\n"
"      Sets the last frame of the animation to N (integer).\n"
"\n"
"You can supply either a mesh file, animation files, or both.\n"
"Note that if an input mesh file is supplied, it must come before the animation\n"
"files in the file list.\n"
"The output IQM file will contain the supplied mesh and any supplied animations.\n"
"If no mesh is provided,the IQM file will simply contain the supplied animations.\n"
    );
    exit(exitstatus);
}

int main(int argc, char **argv)
{
    if(argc <= 1) help(EXIT_FAILURE);

    vector<filespec> infiles;
    filespec inspec;
    const char *outfile = NULL;
    for(int i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            if(argv[i][1] == '-')
            {
                if(!strcasecmp(&argv[i][2], "fps")) { if(i + 1 < argc) inspec.fps = atof(argv[++i]); }
                else if(!strcasecmp(&argv[i][2], "name")) { if(i + 1 < argc) inspec.name = argv[++i]; }
                else if(!strcasecmp(&argv[i][2], "loop")) { inspec.flags |= IQM_LOOP; }
                else if(!strcasecmp(&argv[i][2], "start")) { if(i + 1 < argc) inspec.startframe = max(atoi(argv[++i]), 0); }
                else if(!strcasecmp(&argv[i][2], "end")) { if(i + 1 < argc) inspec.endframe = atoi(argv[++i]); }
                else if(!strcasecmp(&argv[i][2], "scale")) { if(i + 1 < argc) escale = clamp(atof(argv[++i]), 1e-8, 1e8); }
                else if(!strcasecmp(&argv[i][2], "help")) help();
                else if(!strcasecmp(&argv[i][2], "forcejoints")) forcejoints = true;
                else if(!strcasecmp(&argv[i][2], "meshtrans"))
                {
                    if(i + 1 < argc) switch(sscanf(argv[++i], "%lf , %lf , %lf", &emeshtrans.x, &emeshtrans.y, &emeshtrans.z))
                    {
                        case 1: emeshtrans = Vec3(0, 0, emeshtrans.x); break;
                    }
                }
            }
            else switch(argv[i][1])
            {
            case 'h':
                help();
                break;
            case 's':
                if(i + 1 < argc) escale = clamp(atof(argv[++i]), 1e-8, 1e8);
                break;
            case 'j':
                forcejoints = true;
                break;
            }
        }
        else if(!outfile) outfile = argv[i];
        else
        {
            infiles.add(inspec).file = argv[i];
            inspec.reset();
        }
    }

    if(!outfile) fatal("no output file specified");
    else if(infiles.empty()) fatal("no input files specified");

    if(escale != 1) printf("scale: %f\n", escale);
    if(emeshtrans != Vec3(0, 0, 0)) printf("mesh translate: %f, %f, %f\n", emeshtrans.x, emeshtrans.y, emeshtrans.z);

    loopv(infiles)
    {
        const filespec &inspec = infiles[i];
        const char *infile = inspec.file, *type = strrchr(infile, '.');
        if(!type) fatal("no file type: %s", infile);
        if(!strcasecmp(type, ".iqe"))
        {
            if(loadiqe(infile, inspec)) conoutf("imported: %s", infile);
            else fatal("failed reading: %s", infile);
        }
        else fatal("unknown file type: %s", type);
    }

    calcanimdata();

    conoutf("");

    if(writeiqm(outfile)) conoutf("exported: %s", outfile);
    else fatal("failed writing: %s", outfile);

    return EXIT_SUCCESS;
}

// cl iqe2iqm.cpp /DDEBUG /MT /Zi /fsanitize=address
// cl iqe2iqm.cpp /O2 /Oy /MT /DNDEBUG
