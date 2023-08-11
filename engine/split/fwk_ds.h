// data structures and utils: array, set, map, hash, sort.
// - rlyeh, public domain

// -----------------------------------------------------------------------------
// sort

API int sort_64(const void *a, const void *b);

// -----------------------------------------------------------------------------
// less

API int less_64(uint64_t a, uint64_t b);
API int less_int(int a, int b);
API int less_ptr(void *a, void *b);
API int less_str(char *a, char *b);

// -----------------------------------------------------------------------------
// un/hash

API uint32_t unhash_32(uint32_t x);
API uint32_t hash_32(uint32_t x);
API uint64_t hash_64(uint64_t x);
API uint64_t hash_flt(double x);
API uint64_t hash_str(const char* str);
API uint64_t hash_int(int key);
API uint64_t hash_ptr(const void* ptr);

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
#else // new: with reserve support (bugs?)
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
#define each_array(t,val_t,v) \
    ( int __it = 0, __end = array_count(t); __it < __end; ++__it ) \
        for( val_t v = __it[t], *on__ = &v; on__; on__ = 0 )

#define array_foreach_ptr(t,val_t,v) for each_array_ptr(t,val_t,v)
#define each_array_ptr(t,val_t,v) \
    ( int __it = 0, __end = array_count(t); __it < __end; ++__it ) \
        for( val_t *v = (val_t*)&__it[t]; v; v = 0 )

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

#define array_copy(t, src) do { /*todo: review old vrealloc call!*/ \
    array_free(t); \
    (t) = vrealloc( (t), array_count(src) * sizeof(0[t])); \
    memcpy( (t), src, array_count(src) * sizeof(0[t])); \
} while(0)

#define array_erase(t, i) do { /*may alter ordering*/ \
    memcpy( &(t)[i], &(t)[array_count(t) - 1], sizeof(0[t])); \
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
API int   (map_count)(map *m);
API void  (map_gc)(map *m); // only if using MAP_DONT_ERASE
API bool  (map_sort)(map* m);
API void  (map_clear)(map* m);

// -----------------------------------------------------------------------------
// four-cc, eight-cc

API unsigned cc4(const char *id);
API uint64_t cc8(const char *id);
API char *cc4str(unsigned cc);
API char *cc8str(uint64_t cc);

// fast path
#define cc4(abcd)      ( *(unsigned*) #abcd     "    "     ) // lil32() ?
#define cc8(abcdefgh)  ( *(uint64_t*) #abcdefgh "        " ) // lil64() ?
