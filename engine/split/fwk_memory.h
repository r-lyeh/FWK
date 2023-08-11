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

static FORCE_INLINE void *(REALLOC_)(void *p, size_t n) { return n ? WATCH(xrealloc(p,n),n) : xrealloc(FORGET(p),0); } ///-
static FORCE_INLINE void *(CALLOC_)(size_t m, size_t n) { return n *= m, memset(REALLOC(0,n),0,n); } ///-
static FORCE_INLINE char *(STRDUP_)(const char *s) { size_t n = strlen(s)+1; return ((char*)memcpy(REALLOC(0,n), s, n)); } ///-
