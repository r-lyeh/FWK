// -----------------------------------------------------------------------------
// C object framework (constructors/destructors, methods, rtti, refcounting)
// - rlyeh, public domain.
//
// ## object api (low level)
//
// - [ ] make object from reflected type (factory)
// - [x] make object (if debug, include callstack as well)
// - [x] ctor method (optional, ref to constructor)
// - [x] dtor method (optional, ref to deleter)
// - [x] zero mem object
// - [x] object logger
// - [ ] iterate members in a struct
//
// - [x] clone/copy/mutate classes
// - [x] load/save objects from/to memory/disk
// - [ ] diff/patch objects
// - [ ] experimental: support for AoSoA layout (using objcnt, 3bits)
//
// ## object decomposition
//
//                             <---------|--------->
//            OBJ-SHADOW (64-bits)       |   OBJ CONTENT (N bytes)
// +-----+-----+-------------+-----------+-----+-----+-----+-----+--
// |TYPE |REFS.|   OBJ NAME  |  obj cnt  | ... | ... | ... | ... | .
// +-----+-----+-------------+-----------+-----+-----+-----+-----+--
// \-16-bits--/\---45-bits--/\--3-bits--/\-------N-bytes-----------
//
// OBJ TYPE+NAME format:
// - [type] custom tags at 0x0
// - [1..N] name
// - [\n]   blank separator
// - [comments, logger, infos, etc] << obj_printf();
//
// ## object limitations
// - 256 classes max
// - 256 references max
// - 8-byte overhead per object
// - 2 total allocs per object (could be flattened into 1 with some more work)
//
// @todo: obj_extend( "class_src", "class_dst" ); call[super(obj)]()
// @todo: preferred load/save format: [ver:1,user:2,type:1] ([eof|size:7/15/23/31][blob:N])+ [crc:1/2/3/4]
// @todo: more serious loading/saving spec

// object api (heap+rtti)

API void*       obj_malloc( int sz, ... );
API void*       obj_calloc( int sz, ... );
API void        obj_free( void *obj );

API bool        obj_typeeq( const void *obj1, const void *obj2 );
API const char* obj_typeof( const void *obj );
API unsigned    obj_typeid( const void *obj );
API unsigned    obj_typeid_from_name( const char *name );

// object api (ctor/dtor, refcounting, oop)

API void        obj_new( const char *type, ... );
API void        obj_del( void *obj );

API void*       obj_ref( void *obj );
API void*       obj_unref( void *obj );

API void        obj_extend( const char *dstclass, const char *srcclass );
API void        obj_override( const char *objclass, void (**vtable)(), void(*fn)() );

// object: serialize

API unsigned    obj_load(void *obj, const array(char) buffer);
API unsigned    obj_load_file(void *obj, FILE *fp);
API unsigned    obj_load_inplace(void *obj, const void *src, unsigned srclen);

API array(char) obj_save(const void *obj); // empty if error. must array_free() after use
API unsigned    obj_save_file(FILE *fp, const void *obj);
API unsigned    obj_save_inplace(void *dst, unsigned cap, const void *obj);

// object: utils

API unsigned    obj_instances( const void *obj );

API void        obj_zero( void *obj );
API unsigned    obj_sizeof( const void *obj );

API void        obj_hexdump( const void *obj );
API void        obj_hexdumpf( FILE *out, const void *obj );

API void        obj_printf( void *obj, const char *text );
API const char* obj_output( const void *obj );

API void *      obj_clone(const void *obj);
API void *      obj_copy(void **dst, const void *src);
API void *      obj_mutate(void **dst, const void *src);

// object: method dispatch tables

#define ctor(obj) obj_method0(obj, ctor) // ctor[obj_typeid(obj)](obj)
#define dtor(obj) obj_method0(obj, dtor) // dtor[obj_typeid(obj)](obj)

API extern void (*ctor[256])(); ///-
API extern void (*dtor[256])(); ///-

// object: syntax sugars

#define     obj_malloc(sz, ...) obj_initialize((void**)MALLOC(  sizeof(void*)+sz), stringf("\1untyped\n%s\n", "" #__VA_ARGS__))
#define     obj_calloc(sz, ...) obj_initialize((void**)CALLOC(1,sizeof(void*)+sz), stringf("\1untyped\n%s\n", "" #__VA_ARGS__))

#define     obj_new0(type) obj_new(type, 0)
#define     obj_new(type, ...) ( \
                obj_tmpalloc = obj_initialize((void**)CALLOC(1, sizeof(void*)+sizeof(type)), stringf("%c" #type "\n", (char)obj_typeid_from_name(#type))), \
                (*(type*)obj_tmpalloc = (type){ __VA_ARGS__ }), \
                ctor(obj_tmpalloc), \
                (type*)obj_tmpalloc )

#define     obj_override(class, method)    obj_override(#class, (void(**)())method, (void(*)())class##_##method)
#define     obj_method0(obj, method)       method[obj_typeid(obj)]((obj))
#define     obj_method(obj, method, ...)   method[obj_typeid(obj)]((obj), __VA_ARGS__)

#define     obj_printf(obj, ...)           obj_printf(obj, va(__VA_ARGS__))

#define     obj_extend(dstclass, srcclass) obj_extend(#dstclass, #srcclass)

// object: implementation details

// https://stackoverflow.com/questions/16198700/using-the-extra-16-bits-in-64-bit-pointers (note: using 19-bits here)
#define OBJBOX(ptr, payload16) (void*)(((long long unsigned)(payload16) << 48) | (long long unsigned)(ptr))
#define OBJUNBOX(ptr)          (void*)((long long unsigned)(ptr) & 0x0000FFFFFFFFFFFFull)
#define OBJPAYLOAD16(ptr)      (((long long unsigned)(ptr)) >> 48)
#define OBJPAYLOAD3(ptr)       (((long long unsigned)(ptr)) & 7)

API void* obj_initialize( void **ptr, char *type_and_info );
static __thread void *obj_tmpalloc;
