// -----------------------------------------------------------------------------
// factory of handle ids, based on code by randy gaul (PD/Zlib licensed)
// - rlyeh, public domain
//
// [src] http://www.randygaul.net/wp-content/uploads/2021/04/handle_table.cpp
// [ref] http://bitsquid.blogspot.com.es/2011/09/managing-decoupling-part-4-id-lookup.html
// [ref] http://glampert.com/2016/05-04/dissecting-idhashindex/
// [ref] https://github.com/nlguillemot/dof/blob/master/viewer/packed_freelist.h
// [ref] https://gist.github.com/pervognsen/ffd89e45b5750e9ce4c6c8589fc7f253

// you cannot change this one: the number of ID_DATA_BITS you can store in a handle depends on ID_COUNT_BITS
#define ID_DATA_BITS (64-ID_COUNT_BITS)

typedef union id64 {
    uint64_t h;
    struct {
#if (ID_INDEX_BITS+ID_COUNT_BITS) != 64
        uint64_t padding : 64-ID_INDEX_BITS-ID_COUNT_BITS;
#endif
        uint64_t index : ID_INDEX_BITS;
        uint64_t count : ID_COUNT_BITS;
    };
} id64;

typedef struct id_factory id_factory;
id_factory id_factory_create(uint64_t capacity /*= 256*/);
id64        id_factory_insert(id_factory *f, uint64_t data);
uint64_t     id_factory_getvalue(id_factory *f, id64 handle);
void         id_factory_setvalue(id_factory *f, id64 handle, uint64_t data);
void        id_factory_erase(id_factory *f, id64 handle);
bool        id_factory_isvalid(id_factory *f, id64 handle);
void       id_factory_destroy(id_factory *f);

// ---

typedef struct id_factory {
    uint64_t freelist;
    uint64_t capacity;
    uint64_t canary;
    union entry* entries;
} id_factory;

typedef union entry {
    struct {
        uint64_t data  : ID_DATA_BITS;
        uint64_t count : ID_COUNT_BITS;
    };
    uint64_t h;
} entry;

id_factory id_factory_create(uint64_t capacity) {
    if(!capacity) capacity = 1ULL << ID_INDEX_BITS;

    id_factory f = {0};
    f.entries = CALLOC(1, sizeof(entry) * capacity);
    f.capacity = capacity;

    for (int i = 0; i < capacity - 1; ++i) {
        f.entries[i].data = i + 1;
        f.entries[i].count = 0;
    }
    f.entries[capacity - 1].h = 0;
    f.entries[capacity - 1].data = ~0;
    f.entries[capacity - 1].count = ~0;
    f.canary = f.entries[capacity - 1].data;

    return f;
}

void id_factory_destroy(id_factory *f) {
    FREE(f->entries);
}

id64 id_factory_insert(id_factory *f, uint64_t data) {
    // pop element off the free list
    assert(f->freelist != f->canary && "max alive capacity reached");
    uint64_t index = f->freelist;
    f->freelist = f->entries[f->freelist].data;

    // create new id64
    f->entries[index].data = data;
    id64 handle = {0};
    handle.index = index;
    handle.count = f->entries[index].count;
    return handle;
}

void id_factory_erase(id_factory *f, id64 handle) {
    // push id64 onto the freelist
    uint64_t index = handle.index;
    f->entries[index].data = f->freelist;
    f->freelist = index;

    // increment the count. this signifies a change in lifetime (this particular id64 is now dead)
    // the next time this particular index is used in alloc, a new `count` will be used to uniquely identify
    f->entries[index].count++;
}

uint64_t id_factory_getvalue(id_factory *f, id64 handle) {
    uint64_t index = handle.index;
    uint64_t count = handle.count;
    assert(f->entries[index].count == count);
    return f->entries[index].data;
}

void id_factory_setvalue(id_factory *f, id64 handle, uint64_t data) {
    uint64_t index = handle.index;
    uint64_t count = handle.count;
    assert(f->entries[index].count == count);
    f->entries[index].data = data;
}

bool id_factory_isvalid(id_factory *f, id64 handle) {
    uint64_t index = handle.index;
    uint64_t count = handle.count;
    if (index >= f->capacity) return false;
    return f->entries[index].count == count;
}

#if 0
// monitor history of a single entity by running `id_factory | find " 123."`
AUTORUN {
    trap_install();

    id_factory f = id_factory_create(optioni("--NUM",256));

    array(id64) ids = 0;
    for( int i = 0 ; ; ++i, i &= ((1ULL << ID_INDEX_BITS) - 1) ) { // infinite wrap
        printf("count_ids(%d) ", array_count(ids));
        bool insert = randf() < 0.49; // biased against deletion
        if( insert ) {
            id64 h = id_factory_insert(&f, i);
            array_push(ids, h);
            printf("add %llu.%llu\n", h.index, h.count);
        } else {
            int count = array_count(ids);
            if( count ) {
                int chosen = randi(0,count);
                printf("del %d.\n", chosen);
                id64 h = ids[chosen];
                id_factory_erase(&f, h);
                array_erase(ids, chosen);
            }
        }
    }
}
#endif

// ----------------------------------------------------------------------
// public api

static id_factory fid; // @fixme: threadsafe

uintptr_t id_make(void *ptr) {
    do_once fid = id_factory_create(0), id_factory_insert(&fid, 0); // init and reserve id(0)
    id64 newid = id_factory_insert(&fid, (uint64_t)(uintptr_t)ptr ); // 48-bit effective addr
    return newid.h;
}

void *id_handle(uintptr_t id) {
    return (void *)(uintptr_t)id_factory_getvalue(&fid, ((id64){ (uint64_t)id }) );
}

void id_dispose(uintptr_t id) {
    id_factory_erase(&fid, ((id64){ (uint64_t)id }) );
}

bool id_valid(uintptr_t id) {
    return id_factory_isvalid(&fid, ((id64){ (uint64_t)id }) );
}

// ----------------------------------------------------------------------
// C objects framework
// - rlyeh, public domain.

// --- implement new vtables

obj_vtable(ctor, void,  {} );
obj_vtable(dtor, void,  {} );

obj_vtable_null(save, char* );
obj_vtable_null(load, bool  );
obj_vtable_null(test, int   );

obj_vtable_null(init, int   );
obj_vtable_null(quit, int   );
obj_vtable_null(tick, int   );
obj_vtable_null(draw, int   );

obj_vtable_null(lerp, int   );
obj_vtable_null(edit, int   ); // OSC cmds: argc,argv "undo","redo","cut","copy","paste","edit","view","menu"
obj_vtable_null(menu, int   );
obj_vtable_null(aabb, int   );
obj_vtable_null(icon, char* );

// ----------------------------------------------------------------------------

const char *OBJTYPES[256] = { 0 }; // = { REPEAT256("") };

// ----------------------------------------------------------------------------
// heap/stack ctor/dtor

void *obj_malloc(unsigned sz) {
    //sz = sizeof(obj) + sz + sizeof(array(obj*))); // useful?
    obj *ptr = CALLOC(1, sz);
    OBJ_CTOR_HDR(ptr,1,sz,OBJTYPE_obj);
    return ptr;
}
void *obj_free(void *o) {
    if( !((obj*)o)->objrefs ) {
        obj_detach(o);
        obj_dtor(o);
        //obj_zero(o);
        if( ((obj*)o)->objheap ) {
            FREE(o);
        }
        return 0;
    }
    return o; // cannot destroy: object is still referenced
}

// ----------------------------------------------------------------------------
// core

uintptr_t obj_header(const void *o) {
    return ((obj*)o)->objheader;
}
uintptr_t obj_id(const void *o) {
    return ((obj*)o)->objid;
}
unsigned obj_typeid(const void *o) {
    return ((obj*)o)->objtype;
}
const char *obj_type(const void *o) {
    return OBJTYPES[ (((obj*)o)->objtype) ];
}
//const char *obj_name(const void *o) {
//    return quark(((obj*)o)->objnameid);
//}
int obj_sizeof(const void *o) {
    return (int)( ((const obj*)o)->objsizew << OBJ_MIN_PRAGMAPACK_BITS );
}
int obj_size(const void *o) { // size of all members together in struct. may include padding bytes.
    static int obj_sizes[256] = {0};
    unsigned objtypeid = ((obj*)o)->objtype;
    if( objtypeid > 1 && !obj_sizes[objtypeid] ) { // check reflection for a more accurate objsize (without padding bits)
        reflect_init();
        array(reflect_t) *found = map_find(members, intern(obj_type(o)));
        if(!found)
            obj_sizes[objtypeid] = obj_sizeof(o) - sizeof(obj); // @fixme: -= sizeof(entity);
        else
        for each_array_ptr(*found, reflect_t, it)
            obj_sizes[objtypeid] += it->bytes;
    }
    return obj_sizes[objtypeid];
}
char *obj_data(void *o) { // pointer to the first member in struct
    return (char*)o + sizeof(obj);
}
const char *obj_datac(const void *o) { // const pointer to the first struct member
    return (const char*)o + sizeof(obj);
}
void* obj_payload(const void *o) { // pointer right after last member in struct
    return (char*)o + (((obj*)o)->objsizew<<OBJ_MIN_PRAGMAPACK_BITS);
}
void *obj_zero(void *o) { // reset all object members
    return memset(obj_data(o), 0, obj_size(o)), o;
}

static
void test_obj_core() {
    obj *r = obj_new_ext(obj, "root");
    obj *s = obj_new_ext(obj, "root");

    test(r);
    test( 0 == strcmp(obj_type(r), "obj") );
    test( 0 == strcmp(obj_name(r), "root") );
    test( OBJTYPE_obj == obj_typeid(r) );

    test(s);
    test( 0 == strcmp(obj_type(s), "obj") );
    test( 0 == strcmp(obj_name(s), "root") );
    test( OBJTYPE_obj == obj_typeid(s) );

    test( obj_id(r) != 0 );
    test( obj_id(s) != 0 );
    test( obj_id(r) != obj_id(s) );

    obj t = obj(obj); obj_setname(&t, "root");
    obj u = obj(obj); obj_setname(&u, "root");

    test(&t);
    test( 0 == strcmp(obj_type(&t), "obj") );
    test( 0 == strcmp(obj_name(&t), "root") );
    test( OBJTYPE_obj == obj_typeid(&t) );

    test(&u);
    test( 0 == strcmp(obj_type(&u), "obj") );
    test( 0 == strcmp(obj_name(&u), "root") );
    test( OBJTYPE_obj == obj_typeid(&u) );

    test( obj_id(&t) == 0 );
    test( obj_id(&u) == 0 );
    test( obj_id(&t) == obj_id(&u) );
}

// ----------------------------------------------------------------------------
// refcounting

// static int __thread global_ref_count; // @fixme: make it atomic
// static void objref_check_atexit(void) {
//     if(global_ref_count) tty_color(YELLOW), fprintf(stderr, "Warn! obj_refs not zero (%d)\n", global_ref_count), tty_color(0);
// }
// AUTORUN { (atexit)(objref_check_atexit); }

void *obj_ref(void *oo) {
    obj* o = (obj*)oo;
    int num = o->objrefs;
    ++o->objrefs;
    assert( num < o->objrefs && "Object referenced too many times");
    //++global_ref_count;
    return o;
}
void *obj_unref(void *oo) {
    obj* o = (obj*)oo;
    if( o->objrefs ) --o->objrefs;
    if( o->objrefs ) return o;
    obj_free(o);
    //--global_ref_count;
    return 0;
}

// ----------------------------------------------------------------------------
// scene tree

array(obj*)* obj_children(const void *o) {
    array(obj*) *c = &((obj*)o)->objchildren;
    if(!(*c)) array_push((*c), NULL); // default parenting: none. @todo: optimize & move this at construction time
    return c;
}
obj* obj_parent(const void *o) {
    array(obj*) *c = obj_children(o);
    return 0[*c]; // (*c) ? 0[*c] : NULL;
}
obj* obj_root(const void *o) {
    while( obj_parent(o) ) o = obj_parent(o);
    return (obj*)o;
}
array(obj*)* obj_siblings(const void *o) {
    return obj_children(obj_parent(o));
}

static
obj* obj_reparent(obj *o, const void *p) {
    array(obj*) *c = obj_children(o);
    0[*c] = (void*)p;
    return o;
}

obj* obj_detach(void *c) {
    obj *p = obj_parent(c);
    if( p ) {
        uintptr_t id = obj_id(c);

        array(obj*) *oo = obj_children(p);
        for( int i = 1, end = array_count(*oo); i < end; ++i) {
            obj *v = (*oo)[i];
            {
                if( obj_id(v) == id ) {
                    obj_reparent(c, 0);
                    array_erase_slow(*oo, i);
                    return c;
                }
            }
        }
    }
    return 0;
}
obj* obj_attach(void *o, void *c) {
    // reattach
    obj_detach(c);
    obj_reparent(c, o);
    // insert into children
    array(obj*) *p = obj_children(o);
    array_push(*p, c);
    return o;
}

int obj_dumptree(const void *o) {
    static int tabs = 0;
    printf("%*s" "+- %s\n", tabs++, "", obj_name(o));
    for each_objchild(o, obj*, v) {
        obj_dumptree(v);
    }
    --tabs;
    return 0;
}

static
void test_obj_scene() {
    obj *r = obj_new_ext(obj, "root");           // root
    obj *c1 = obj_new_ext(obj, "child1");        // child1
    obj *c2 = obj_new_ext(obj, "child2");        // child2
    obj *gc1 = obj_new_ext(obj, "grandchild1");  // grandchild1
    obj *gc2 = obj_new_ext(obj, "grandchild2");  // grandchild2
    obj *gc3 = obj_new_ext(obj, "grandchild3");  // grandchild3

    test( !obj_parent(r) );
    test( !obj_parent(c1) );
    test( !obj_parent(c2) );
    test( !obj_parent(gc1) );
    test( !obj_parent(gc2) );
    test( !obj_parent(gc3) );
    test( obj_root(r) == r );
    test( obj_root(c1) == c1 );
    test( obj_root(c2) == c2 );
    test( obj_root(gc1) == gc1 );
    test( obj_root(gc2) == gc2 );
    test( obj_root(gc3) == gc3 );

                               // r
    obj_attach(r, c1);         // +- c1
    obj_attach(c1, gc1);       //  +- gc1
    obj_attach(r, c2);         // +- c2
    obj_attach(c2, gc2);       //  +- gc2
    obj_attach(c2, gc3);       //  +- gc3

    obj_dumptree(r);
    // puts("---");

    test( obj_parent(r) == 0 );
    test( obj_parent(c1) == r );
    test( obj_parent(c2) == r );
    test( obj_parent(gc1) == c1 );
    test( obj_parent(gc2) == c2 );
    test( obj_parent(gc3) == c2 );

    test( obj_root(r) == r );
    test( obj_root(c1) == r );
    test( obj_root(c2) == r );
    test( obj_root(gc1) == r );
    test( obj_root(gc2) == r );
    test( obj_root(gc3) == r );

    for each_objchild(r, obj*, o) test( o == c1 || o == c2 );
    for each_objchild(c1, obj*, o) test( o == gc1 );
    for each_objchild(c2, obj*, o) test( o == gc2 || o == gc3 );

    obj_detach(c1);
    test( !obj_parent(c1) );
    for each_objchild(r, obj*, o) test( o != c1 );
    for each_objchild(c1, obj*, o) test( o == gc1 );

    obj_detach(c2);
    test( !obj_parent(c2) );
    for each_objchild(r, obj*, o) test( o != c2 );
    for each_objchild(c2, obj*, o) test( o == gc2 || o == gc3 );
}

// ----------------------------------------------------------------------------
// metadata

static map(int,int) oms;
static thread_mutex_t *oms_lock;
void *obj_setmeta(void *o, const char *key, const char *value) {
    void *ret = 0;
    do_threadlock(oms_lock) {
        if(!oms) map_init_int(oms);
        int *q = map_find_or_add(oms, intern(va("%p-%s",(void*)obj_id((obj*)o),key)), 0);
        if(!*q && !value[0]) {} else *q = intern(value);
        quark(*q), ret = o;
    }
    return ret;
}
const char* obj_meta(const void *o, const char *key) {
    const char *ret = 0;
    do_threadlock(oms_lock) {
        if(!oms) map_init_int(oms);
        int *q = map_find_or_add(oms, intern(va("%p-%s",(void*)obj_id((obj*)o),key)), 0);
        ret = quark(*q);
    }
    return ret;
}

void *obj_setname(void *o, const char *name) {
    ifdef(debug,((obj*)o)->objname = name);
    return obj_setmeta(o, "name", name);
}
const char *obj_name(const void *o) {
    const char *objname = obj_meta(o, "name");
    return objname[0] ? objname : "obj";
}


static
void test_obj_metadatas( void *o1 ) {
    obj *o = (obj *)o1;
    test( !strcmp("", obj_meta(o, "has_passed_test")) );
    test( obj_setmeta(o, "has_passed_test", "yes") );
    test( !strcmp("yes", obj_meta(o, "has_passed_test")) );
}

// ----------------------------------------------------------------------------
// stl

void* obj_swap(void *dst, void *src) { // @testme
    int len = obj_size(dst);
    char *buffer = ALLOCA(len);
    memcpy(buffer,        obj_datac(dst), len);
    memcpy(obj_data(dst), obj_datac(src), len);
    memcpy(obj_data(src), buffer,         len);
    return dst;
}

void* obj_copy_fast(void *dst, const void *src) {
    // note: prefer obj_copy() as it should handle pointers and guids as well
    return memcpy(obj_data(dst), obj_datac(src), obj_size(dst));
}
void* obj_copy(void *dst, const void *src) { // @testme
    // @todo: use obj_copy_fast() silently if the object does not contain any pointers/guids
    return obj_loadini(dst, obj_saveini(src));
    // return obj_load(dst, obj_save(src));
    // return obj_loadbin(dst, obj_savebin(src));
    // return obj_loadini(dst, obj_saveini(src));
    // return obj_loadjson(dst, obj_savejson(src));
    // return obj_loadmpack(dst, obj_savempack(src));
}

int obj_comp_fast(const void *a, const void *b) {
    // note: prefer obj_comp() as it should handle pointers and guids as well
    return memcmp(obj_datac(a), obj_datac(b), obj_size(a));
}
int obj_comp(const void *a, const void *b) {
    // @todo: use obj_comp_fast() silently if the object does not contain any pointers/guids
    return strcmp(obj_saveini(a),obj_saveini(b));
}
int obj_lesser(const void *a, const void *b) {
    return obj_comp(a,b) < 0;
}
int obj_greater(const void *a, const void *b) {
    return obj_comp(a,b) > 0;
}
int obj_equal(const void *a, const void *b) {
    return obj_comp(a,b) == 0;
}

uint64_t obj_hash(const void *o) {
    return hash_bin(obj_datac(o), obj_size(o));
}

static
void test_obj_similarity(void *o1, void *o2) {
    obj *b = (obj*)o1;
    obj *c = (obj*)o2;
    test( 0 == strcmp(obj_name(b),obj_name(c)) );
    test( 0 == strcmp(obj_type(b),obj_type(c)) );
}
static
void test_obj_equality(void *o1, void *o2) {
    obj *b = (obj*)o1;
    obj *c = (obj*)o2;
    test_obj_similarity(b, c);
    test( obj_size(b) == obj_size(c) );
    test( obj_hash(b) == obj_hash(c) );
    test( 0 == obj_comp(b,c) );
    test( obj_equal(b,c) );
    test( !obj_lesser(b,c) );
    test( !obj_greater(b,c) );
}
static
void test_obj_exact(void *o1, void *o2) {
    obj *b = (obj*)o1;
    obj *c = (obj*)o2;
    test_obj_equality(b, c);
    test( obj_header(b) == obj_header(c) );
    test( 0 == memcmp(b, c, obj_sizeof(b)) );
}

// ----------------------------------------------------------------------------
// debug

bool obj_hexdump(const void *oo) {
    const obj *o = (const obj *)oo;
    int header = 1 * sizeof(obj);
    printf("; name[%s] type[%s] id[%d..%d] unused[%08x] sizeof[%02d] %p\n",
        obj_name(o), obj_type(o),
        (int)o->objid>>16, (int)o->objid&0xffff, (int)o->objunused,
        obj_sizeof(o), (void*)o->objheader);
    return hexdump(obj_datac(o) - header, obj_size(o) + header), 1;
}
int obj_print(const void *o) {
    char *sav = obj_saveini(o); // obj_savejson(o)
    return puts(sav);
}
static char *obj_tempname = 0;
static FILE *obj_filelog = 0;
int (obj_printf)(const void *o, const char *text) {
    if( !obj_tempname ) {
        obj_tempname = stringf("%s.log", app_name());
        unlink(obj_tempname);
        obj_filelog = fopen(obj_tempname, "w+b");
        if( obj_filelog ) fseek(obj_filelog, 0L, SEEK_SET);
    }
    int rc = 0;
    for( char *end; (end = strchr(text, '\n')) != NULL; ) {
        rc |= fprintf(obj_filelog, "[%p] %.*s\n", o, (int)(end - text), text );
        text = end + 1;
    }
    if( text[0] ) rc |= fprintf(obj_filelog, "[%p] %s\n", o, text);
    return rc;
}
int obj_console(const void *o) { // obj_output() ?
    if( obj_filelog ) fflush(obj_filelog);
    return obj_tempname && !system(va(ifdef(win32,"type \"%s\" | find \"[%p]\"", "cat %s | grep \"[%p]\""), obj_tempname, o));
}

static
void test_obj_console(void *o1) {
    obj *o = (obj *)o1;

    obj_printf(o, "this is [%s], line 1\n", obj_name(o));
    obj_printf(NULL, "this line does not belong to any object\n");
    obj_printf(o, "this is [%s], line 2\n", obj_name(o));
    obj_console(o);
}

// ----------------------------------------------------------------------------
// serialization

const char *p2s(const char *type, void *p) {
    // @todo: p2s(int interned_type, void *p)
    /**/ if( !strcmp(type, "int") ) return itoa1(*(int*)p);
    else if( !strcmp(type, "unsigned") ) return itoa1(*(unsigned*)p);
    else if( !strcmp(type, "float") ) return ftoa1(*(float*)p);
    else if( !strcmp(type, "double") ) return ftoa1(*(double*)p);
    else if( !strcmp(type, "uintptr_t") ) return va("%p", (void*)*(uintptr_t*)p);
    else if( !strcmp(type, "vec2i") ) return itoa2(*(vec2i*)p);
    else if( !strcmp(type, "vec3i") ) return itoa3(*(vec3i*)p);
    else if( !strcmp(type, "vec2") ) return ftoa2(*(vec2*)p);
    else if( !strcmp(type, "vec3") ) return ftoa3(*(vec3*)p);
    else if( !strcmp(type, "vec4") ) return ftoa4(*(vec4*)p);
    else if( !strcmp(type, "rgb") ) return rgbatoa(*(unsigned*)p);
    else if( !strcmp(type, "rgba") ) return rgbatoa(*(unsigned*)p);
    else if( !strcmp(type, "char*") || !strcmp(type, "string") ) return va("%s", *(char**)p);
    // @todo: if strchr('*') assume obj, if reflected save guid: obj_id();
    return tty_color(YELLOW), printf("p2s: cannot serialize `%s` type\n", type), tty_color(0), "";
}
bool s2p(void *P, const char *type, const char *str) {
    int i; unsigned u; float f; double g; char *s = 0; uintptr_t p;
    vec2 v2; vec3 v3; vec4 v4; vec2i v2i; vec3i v3i;
    /**/ if( !strcmp(type, "int") )       return !!memcpy(P, (i = atoi1(str), &i), sizeof(i));
    else if( !strcmp(type, "unsigned") )  return !!memcpy(P, (u = atoi1(str), &u), sizeof(u));
    else if( !strcmp(type, "vec2i") )     return !!memcpy(P, (v2i = atoi2(str), &v2i), sizeof(v2i));
    else if( !strcmp(type, "vec3i") )     return !!memcpy(P, (v3i = atoi3(str), &v3i), sizeof(v3i));
    else if( !strcmp(type, "float") )     return !!memcpy(P, (f = atof1(str), &f), sizeof(f));
    else if( !strcmp(type, "double") )    return !!memcpy(P, (g = atof1(str), &g), sizeof(g));
    else if( !strcmp(type, "vec2") )      return !!memcpy(P, (v2 = atof2(str), &v2), sizeof(v2));
    else if( !strcmp(type, "vec3") )      return !!memcpy(P, (v3 = atof3(str), &v3), sizeof(v3));
    else if( !strcmp(type, "vec4") )      return !!memcpy(P, (v4 = atof4(str), &v4), sizeof(v4));
    else if( !strcmp(type, "rgb") )       return !!memcpy(P, (u = atorgba(str), &u), sizeof(u));
    else if( !strcmp(type, "rgba") )      return !!memcpy(P, (u = atorgba(str), &u), sizeof(u));
    else if( !strcmp(type, "uintptr_t") ) return !!memcpy(P, (p = strtol(str, NULL, 16), &p), sizeof(p));
    else if( !strcmp(type, "char*") || !strcmp(type, "string") ) {
        char substring[128] = {0};
        sscanf(str, "%[^\r\n]", substring);

        strcatf(&s, "%s", substring);

        *(uintptr_t*)(P) = (uintptr_t)s;
        return 1;
    }
    // @todo: if strchr('*') assume obj, if reflected load guid: obj_id();
    return tty_color(YELLOW), printf("s2p: cannot deserialize `%s` type\n", type), tty_color(0), 0;
}

char *obj_saveini(const void *o) { // @testme
    char *out = 0;
    const char *T = obj_type(o);
    strcatf(&out, "[%s] ; v100\n", T);
    for each_member(T,R) {
        const char *sav = p2s(R->type,(char*)(o)+R->sz);
        if(!sav) return FREE(out), NULL;
        strcatf(&out,"%s.%s=%s\n", R->type,R->name,sav );
    }
    char *cpy = va("%s", out);
    FREE(out);
    return cpy;
}
obj *obj_mergeini(void *o, const char *ini) { // @testme
    const char *sqr = strchr(ini, '[');
    if( !sqr ) return 0;
    ini = sqr+1;

    char T[64] = {0};
    if( sscanf(ini, "%63[^]]", T) != 1 ) return 0; // @todo: parse version as well
    ini += strlen(T);

    for each_member(T,R) {
        char *lookup = va("\n%s.%s=", R->type,R->name), *found = 0;

        // type needed? /*
        if(!found) { found = strstr(ini, lookup); if (found) found += strlen(lookup); }
        if(!found) { *lookup = '\r'; }
        if(!found) { found = strstr(ini, lookup); if (found) found += strlen(lookup); }
        // */

        if(!found) lookup = va("\n%s=", R->name);

        if(!found) { found = strstr(ini, lookup); if (found) found += strlen(lookup); }
        if(!found) { *lookup = '\r'; }
        if(!found) { found = strstr(ini, lookup); if (found) found += strlen(lookup); }

        if( found) {
            if(!s2p((char*)(o)+R->sz, R->type, found))
                return 0;
        }
    }
    return o;
}
obj *obj_loadini(void *o, const char *ini) { // @testme
    return obj_mergeini(obj_zero(o), ini);
}

char *obj_savejson(const void *o) {
    char *j = 0;
    const char *T = obj_type(o);
    for each_member(T,R) {
        const char *sav = p2s(R->type,(char*)(o)+R->sz);
        if(!sav) return FREE(j), NULL;
        char is_string = !strcmp(R->type,"char*") || !strcmp(R->type,"string");
        strcatf(&j," %s: %s%s%s,\n", R->name,is_string?"\"":"",sav,is_string?"\"":"" );
    }
    char *out = va("%s: { // v100\n%s}\n", T,j);
    FREE(j);
#if is(debug)
    json5 root = { 0 };
    char *error = json5_parse(&root, va("%s", out), 0);
    assert( !error );
    json5_free(&root);
#endif
    return out;
}
obj *obj_mergejson(void *o, const char *json) {
    // @fixme: va() call below could be optimized out since we could figure it out if json was internally provided (via va or strdup), or user-provided
    json5 root = { 0 };
    char *error = json5_parse(&root, va("%s", json), 0); // @todo: parse version comment
    if( !error && root.type == JSON5_OBJECT && root.count == 1 ) {
        json5 *n = &root.nodes[0];
        char *T = n->name;
        for each_member(T,R) {
            for( int i = 0; i < n->count; ++i ) {
                if( !strcmp(R->name, n->nodes[i].name) ) {
                    void *p = (char*)o + R->sz;
                    /**/ if( n->nodes[i].type == JSON5_UNDEFINED ) {}
                    else if( n->nodes[i].type == JSON5_NULL ) {
                        *(uintptr_t*)(p) = (uintptr_t)0;
                    }
                    else if( n->nodes[i].type == JSON5_BOOL ) {
                        *(bool*)p = n->nodes[i].boolean;
                    }
                    else if( n->nodes[i].type == JSON5_INTEGER ) {
                        if( strstr(R->type, "64" ) )
                        *(int64_t*)p = n->nodes[i].integer;
                        else
                        *(int*)p = n->nodes[i].integer;
                    }
                    else if( n->nodes[i].type == JSON5_STRING ) {
                        char *s = 0;
                        strcatf(&s, "%s", n->nodes[i].string);
                        *(uintptr_t*)(p) = (uintptr_t)s;
                    }
                    else if( n->nodes[i].type == JSON5_REAL ) {
                        if( R->type[0] == 'f' )
                        *(float*)(p) = n->nodes[i].real;
                        else
                        *(double*)(p) = n->nodes[i].real;
                    }
                    else if( n->nodes[i].type == JSON5_OBJECT ) {}
                    else if( n->nodes[i].type == JSON5_ARRAY ) {}
                    break;
                }
            }
        }
    }
    json5_free(&root);
    return error ? 0 : o;
}
obj *obj_loadjson(void *o, const char *json) { // @testme
    return obj_mergejson(obj_zero(o), json);
}

char *obj_savebin(const void *o) { // PACKMSG("ss", "entity_v1", quark(self->objnameid)); // = PACKMSG("p", obj_data(&b), (uint64_t)obj_size(&b));
    int len = cobs_bounds(obj_size(o));
    char *sav = va("%*.s", len, "");
    len = cobs_encode(obj_datac(o), obj_size(o), sav, len);
    sav[len] = '\0';
    return sav;
}
obj *obj_mergebin(void *o, const char *sav) { // UNPACKMSG(sav, "p", obj_data(c), (uint64_t)obj_size(c));
    int outlen = cobs_decode(sav, strlen(sav), obj_data(o), obj_size(o));
    return outlen != obj_size(o) ? NULL : o;
}
obj *obj_loadbin(void *o, const char *sav) {
    return obj_mergebin(obj_zero(o), sav);
}

char *obj_savempack(const void *o) { // @todo
    return "";
}
obj *obj_mergempack(void *o, const char *sav) { // @todo
    return 0;
}
obj *obj_loadmpack(void *o, const char *sav) { // @todo
    return obj_mergempack(obj_zero(o), sav);
}

static __thread map(void*,array(char*)) obj_stack;
int obj_push(const void *o) {
    if(!obj_stack) map_init_ptr(obj_stack);
    array(char*) *found = map_find_or_add(obj_stack,(void*)o,0);

    char *bin = STRDUP(obj_saveini(o)); // @todo: savebin
    array_push(*found, bin);
    return 1;
}
int obj_pop(void *o) {
    if(!obj_stack) map_init_ptr(obj_stack);
    array(char*) *found = map_find_or_add(obj_stack,(void*)o,0);

    char **bin = array_back(*found);
    if( bin ) {
        int rc = !!obj_loadini(o, *bin); // @todo: loadbin
        if( array_count(*found) > 1 ) {
            FREE(*bin);
            array_pop(*found);
        }
        return rc;
    }
    return 0;
}

static
void test_obj_serialization(void *o1, void *o2) {
    obj* b = (obj*)o1;
    obj* c = (obj*)o2;

    char *json = obj_savejson(b); // puts(json);
    test( json[0] );
    char *ini = obj_saveini(b); // puts(ini);
    test( ini[0] );
    char *bin = obj_savebin(b); // puts(bin);
    test( bin[0] );

    obj_push(c);

        test( obj_copy(c,b) );
        test( obj_comp(b,c) == 0 ) || obj_hexdump(b) & obj_hexdump(c);

        test( obj_zero(c) );
        test( obj_comp(c,b) != 0 ) || obj_hexdump(c);
        test( obj_loadbin(c, bin) );
        test( obj_comp(c,b) == 0 ) || obj_hexdump(c) & obj_hexdump(b);

        test( obj_zero(c) );
        test( obj_comp(c,b) != 0 ) || obj_hexdump(c);
        test( obj_loadini(c, ini) );
        test( obj_comp(c,b) == 0 ) || obj_hexdump(c) & obj_hexdump(b);

        test( obj_zero(c) );
        test( obj_comp(c,b) != 0 ) || obj_hexdump(c);
        test( obj_loadjson(c, json) );
        test( obj_comp(c,b) == 0 ) || obj_hexdump(c) & obj_hexdump(b);

    obj_pop(c);
    obj_hexdump(c);
}

// ----------------------------------------------------------------------------
// components

bool obj_addcomponent(entity *e, unsigned c, void *ptr) {
    e->cflags |= (3ULL << c);
    e->c[c & (OBJCOMPONENTS_MAX-1)] = ptr;
    return 1;
}
bool obj_hascomponent(entity *e, unsigned c) {
    return !!(e->cflags & (3ULL << c));
}
void* obj_getcomponent(entity *e, unsigned c) {
    return e->c[c & (OBJCOMPONENTS_MAX-1)];
}
bool obj_delcomponent(entity *e, unsigned c) {
    e->cflags &= ~(3ULL << c);
    e->c[c & (OBJCOMPONENTS_MAX-1)] = NULL;
    return 1;
}
bool obj_usecomponent(entity *e, unsigned c) {
    e->cflags |= (1ULL << c);
    return 1;
}
bool obj_offcomponent(entity *e, unsigned c) {
    e->cflags &= ~(1ULL << c);
    return 0;
}

char *entity_save(entity *self) {
    char *sav = obj_saveini(self);
    return sav;
}

static
void entity_register() {
    do_once {
        STRUCT(entity, uintptr_t, cflags);
        obj_extend(entity, save);
    }
}

AUTORUN{
    entity_register();
}

static
void test_obj_ecs() {
    entity_register(); // why is this required here? autorun init fiasco?

    entity *e = entity_new(entity);
    puts(obj_save(e));

    for( int i = 0; i < 32; ++i) test(0 == obj_hascomponent(e, i));
    for( int i = 0; i < 32; ++i) test(1 == obj_addcomponent(e, i, NULL));
    for( int i = 0; i < 32; ++i) test(1 == obj_hascomponent(e, i));
    for( int i = 0; i < 32; ++i) test(1 == obj_delcomponent(e, i));
    for( int i = 0; i < 32; ++i) test(0 == obj_hascomponent(e, i));
}

// ----------------------------------------------------------------------------
// reflection

void* obj_mutate(void *dst, const void *src) {
    ((obj*)dst)->objheader = ((const obj *)src)->objheader;

#if 0
    // mutate a class. ie, convert a given object class into a different one,
    // while preserving the original metas, components and references as much as possible.
    // @todo iterate per field

    dtor(dst);

        unsigned src_sz = obj_sizeof(src);
        unsigned src_id = obj_id(src);

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
#endif
    return dst;
}

void *obj_clone(const void *src) {
    int sz = sizeof(obj) + obj_size(src) + sizeof(array(obj*));
    enum { N = 8 }; sz = ((sz + (N - 1)) & -N);  // Round up to N-byte boundary
    obj *ptr = obj_malloc( sz );
    obj_mutate(ptr, src); // ptr->objheader = ((const obj *)src)->objheader;
    obj_loadini(ptr, obj_saveini(src));
    return ptr;
}

void* obj_merge(void *dst, const void *src) { // @testme
    char *bin = obj_savebin(src);
    return obj_mergebin(dst, bin);
}

void *obj_make(const char *str) {
    const char *T;
    const char *I = strchr(str, '['); // is_ini
    const char *J = strchr(str, '{'); // is_json
    if( !I && !J ) return 0;
    else if( I && !J ) T = I;
    else if( !I && J ) T = J;
    else T = I < J ? I : J;

    char name[64] = {0};
    if( sscanf(T+1, T == I ? "%63[^]]" : "%63[^:=]", name) != 1 ) return 0;

    int has_components = 0; // @todo: support entities too

    unsigned Tid = intern(name);
    reflect_init();
    reflect_t *found = map_find(reflects, Tid);
    if(!found) return obj_new(obj);

    obj *ptr = CALLOC(1, found->sz + (has_components+1) * sizeof(array(obj*)));
    void *ret = (T == I ? obj_mergeini : obj_mergejson)(ptr, str);
    OBJTYPES[ found->objtype ] = found->name;
    OBJ_CTOR_PTR(ptr,1,/*found->id,*/found->sz,found->objtype);
    obj_setname(ptr, name); // found->id);

    return ptr; // returns partial construction as well. @todo: just return `ret` for a more strict built/failed policy
}
