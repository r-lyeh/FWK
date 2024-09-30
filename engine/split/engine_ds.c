
// -----------------------------------------------------------------------------
// sort/less

int less_64_ptr(const void *a, const void *b) {
    return 0[(uint64_t*)a] - 0[(uint64_t*)b];
}
int less_int_ptr(const void *a, const void *b) {
    return 0[(int*)a] - 0[(int*)b];
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
