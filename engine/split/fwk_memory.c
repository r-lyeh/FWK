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

static __thread uint64_t xstats_current = 0, xstats_total = 0, xstats_allocs = 0;

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
        xstats_allocs -= !size;
    } else {
        xstats_current += size;
        xstats_allocs += !!size;
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
    uint64_t xtra = 0; // xstats_allocs * 65536; // assumes 64K pagesize for every alloc
    return va("%03u/%03uMB", (unsigned)((xstats_current+xtra) / 1024 / 1024), (unsigned)((xstats_total+xtra) / 1024 / 1024));
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
