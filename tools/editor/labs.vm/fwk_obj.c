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
