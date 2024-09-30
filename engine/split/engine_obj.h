// -----------------------------------------------------------------------------
// factory of handle ids

// convert between hard refs (pointers) and weak refs (ids)
API uintptr_t id_make(void *ptr);
API void *     id_handle(uintptr_t id);
API void       id_dispose(uintptr_t id);
API bool        id_valid(uintptr_t id);

// configuration:
// ideally, these two should be 32 each. they were changed to fit our OBJHEADER bits
#ifndef ID_INDEX_BITS
#define ID_INDEX_BITS 16
#endif
#ifndef ID_COUNT_BITS
#define ID_COUNT_BITS  3
#endif

// C objects framework
// - rlyeh, public domain.
//
// ## object limitations
// - 8-byte overhead per object
// - XX-byte overhead per object-entity
// - 32 components max per object-entity
// - 256 classes max per game
// - 256 references max per object
// - 1024K bytes max per object
// - 8 generations + 64K IDs per running instance (19-bit IDs)
// - support for pragma pack(1) structs not enabled by default.

/* /!\ if you plan to use pragma pack(1) on any struct, you need #define OBJ_MIN_PRAGMAPACK_BITS 0 at the expense of max class size /!\ */
#ifndef   OBJ_MIN_PRAGMAPACK_BITS
//#define OBJ_MIN_PRAGMAPACK_BITS 3 // allows pragma packs >= 8. objsizew becomes 8<<3, so 2048 bytes max per class (default)
#define   OBJ_MIN_PRAGMAPACK_BITS 2 // allows pragma packs >= 4. objsizew becomes 8<<2, so 1024 bytes max per class
//#define OBJ_MIN_PRAGMAPACK_BITS 1 // allows pragma packs >= 2. objsizew becomes 8<<1, so  512 bytes max per class
//#define OBJ_MIN_PRAGMAPACK_BITS 0 // allows pragma packs >= 1. objsizew becomes 8<<0, so  256 bytes max per class
#endif

#define OBJHEADER \
    struct { \
        ifdef(debug, const char *objname;) \
        union { \
            uintptr_t objheader; \
            struct {  \
            uintptr_t objtype:8; \
            uintptr_t objsizew:8; \
            uintptr_t objrefs:8; \
            uintptr_t objheap:1; \
            uintptr_t objcomps:1; /* << can be removed? check payload ptr instead? */ \
            uintptr_t objunused:64-8-8-8-1-1-ID_INDEX_BITS-ID_COUNT_BITS; /*19*/ \
            uintptr_t objid:ID_INDEX_BITS+ID_COUNT_BITS; /*16+3*/ \
            }; \
        }; \
        array(struct obj*) objchildren; \
    };

#ifndef OBJ
#define OBJ \
    OBJHEADER
#endif

// ----------------------------------------------------------------------------
// syntax sugars

#ifdef OBJTYPE
#undef OBJTYPE
#endif

#define OBJTYPE(T) \
    OBJTYPE_##T

#define OBJTYPEDEF(NAME,N) \
     enum { OBJTYPE(NAME) = N }; \
     STATIC_ASSERT( N <= 255 ); \
     STATIC_ASSERT( sizeof(NAME) == ((sizeof(NAME)>>OBJ_MIN_PRAGMAPACK_BITS)<<OBJ_MIN_PRAGMAPACK_BITS) ); // (sizeof(NAME) & ((1<<OBJ_MIN_PRAGMAPACK_BITS)-1)) == 0 );

// ----------------------------------------------------------------------------
// objects

#define TYPEDEF_STRUCT(NAME,N,...) \
    typedef struct NAME { OBJ \
        __VA_ARGS__ \
        char payload[0]; \
    } NAME; OBJTYPEDEF(NAME,N);

// TYPEDEF_STRUCT(obj,0);
    typedef struct obj { OBJ } obj;

// ----------------------------------------------------------------------------
// entities

#define OBJCOMPONENTS_MAX 32
#define OBJCOMPONENTS_ALL_ENABLED 0xAAAAAAAAAAAAAAAAULL
#define OBJCOMPONENTS_ALL_FLAGGED 0x5555555555555555ULL
#define COMPONENTS_ONLY(x) ((x) & ~OBJCOMPONENTS_ALL_FLAGGED)

#ifndef ENTITY
#define ENTITY \
    struct { OBJHEADER union { struct { uintptr_t objenabled:OBJCOMPONENTS_MAX, objflagged:OBJCOMPONENTS_MAX; }; uintptr_t cflags; }; void *c[OBJCOMPONENTS_MAX]; };
#endif

#define TYPEDEF_ENTITY(NAME,N,...) \
    typedef struct NAME { ENTITY \
        __VA_ARGS__ \
        char payload[0]; \
    } NAME; OBJTYPEDEF(NAME,N);

// OBJTYPEDEF(entity,1)
    typedef struct entity { ENTITY } entity;

#define entity_new(TYPE, ...)             OBJ_CTOR(TYPE, #TYPE, 1, 0, __VA_ARGS__)
#define entity_new_ext(TYPE, NAME, ...)   OBJ_CTOR(TYPE,  NAME, 1, 0, __VA_ARGS__)

// ----------------------------------------------------------------------------
// heap/stack ctor/dtor

static __thread obj *objtmp;
#define OBJ_CTOR_HDR(PTR,HEAP,SIZEOF_OBJ,OBJ_TYPE) ( \
        (PTR)->objheader = HEAP ? id_make(PTR) : 0, /*should assign to .objid instead. however, id_make() returns shifted bits already*/ \
        (PTR)->objtype = (OBJ_TYPE), \
        (PTR)->objheap = (HEAP), \
        (PTR)->objsizew = (SIZEOF_OBJ>>OBJ_MIN_PRAGMAPACK_BITS))
#define OBJ_CTOR_PTR(PTR,HEAP,SIZEOF_OBJ,OBJ_TYPE) ( \
        OBJ_CTOR_HDR(PTR,HEAP,SIZEOF_OBJ,OBJ_TYPE), \
        obj_ctor(PTR))
#define OBJ_CTOR(TYPE, NAME, HEAP, PAYLOAD_SIZE, ...) (TYPE*)( \
        objtmp = (HEAP ? MALLOC(sizeof(TYPE)+(PAYLOAD_SIZE)) : ALLOCA(sizeof(TYPE)+(PAYLOAD_SIZE))), \
        *(TYPE*)objtmp = ((TYPE){ {0,}, __VA_ARGS__}), \
        ((PAYLOAD_SIZE) ? memset((char*)objtmp + sizeof(TYPE), 0, (PAYLOAD_SIZE)) : objtmp), \
        ( OBJTYPES[ OBJTYPE(TYPE) ] = #TYPE ), \
        OBJ_CTOR_PTR(objtmp, HEAP,sizeof(TYPE),OBJTYPE(TYPE)), \
        ifdef(debug, (obj_printf)(objtmp, va("%s", callstack(+16))), 0), \
        obj_setname(objtmp, NAME))

#define obj(TYPE, ...)                *OBJ_CTOR(TYPE, #TYPE, 0, 0, __VA_ARGS__)
#define obj_new(TYPE, ...)             OBJ_CTOR(TYPE, #TYPE, 1, 0, __VA_ARGS__)
#define obj_new_ext(TYPE, NAME, ...)   OBJ_CTOR(TYPE,  NAME, 1, 0, __VA_ARGS__)

void*   obj_malloc(unsigned sz);
void*   obj_free(void *o);

// ----------------------------------------------------------------------------
// obj generics. can be extended.

#define obj_ctor(o,...) obj_method(ctor, o, ##__VA_ARGS__)
#define obj_dtor(o,...) obj_method(dtor, o, ##__VA_ARGS__)

#define obj_save(o,...) obj_method(save, o, ##__VA_ARGS__)
#define obj_load(o,...) obj_method(load, o, ##__VA_ARGS__)

#define obj_test(o,...) obj_method(test, o, ##__VA_ARGS__)

#define obj_init(o,...) obj_method(init, o, ##__VA_ARGS__)
#define obj_quit(o,...) obj_method(quit, o, ##__VA_ARGS__)
#define obj_tick(o,...) obj_method(tick, o, ##__VA_ARGS__)
#define obj_draw(o,...) obj_method(draw, o, ##__VA_ARGS__)

#define obj_lerp(o,...) obj_method(lerp, o, ##__VA_ARGS__)
#define obj_edit(o,...) obj_method(edit, o, ##__VA_ARGS__)
#define obj_menu(o,...) obj_method(menu, o, ##__VA_ARGS__)
#define obj_aabb(o,...) obj_method(aabb, o, ##__VA_ARGS__)
#define obj_icon(o,...) obj_method(icon, o, ##__VA_ARGS__)

// --- syntax sugars

#define obj_extend(T,method)       (obj_##method[OBJTYPE(T)] = (void*)T##_##method)
#define obj_extend_t(T,method)     (obj_##method[OBJTYPE(T##_t)] = (void*)T##_##method)
#define obj_method(method,o,...)   (obj_##method[((struct obj*)(o))->objtype](o,##__VA_ARGS__)) // (obj_##method[((struct obj*)(o))->objtype]((o), ##__VA_ARGS__))
#define obj_hasmethod(o,method)    (obj_typeid(o)[obj_##method])

#define obj_vtable(method,RC,...)   RC macro(obj_##method)(){ __VA_ARGS__ }; RC (*obj_##method[256])() = { REPEAT256(macro(obj_##method)) };
#define obj_vtable_null(method,RC)  RC (*obj_##method[256])() = { 0 }; // null virtual table. will crash unless obj_extend'ed

#define REPEAT16(f)  f,f,f,f,f,f,f,f,f,f,f,f,f,f,f,f ///-
#define REPEAT64(f)  REPEAT16(f),REPEAT16(f),REPEAT16(f),REPEAT16(f) ///-
#define REPEAT256(f) REPEAT64(f),REPEAT64(f),REPEAT64(f),REPEAT64(f) ///-

#undef  EXTEND
#define EXTEND(...) EXPAND(EXTEND, __VA_ARGS__)
#define EXTEND2(o,F1) obj_extend(o,F1) ///-
#define EXTEND3(o,F1,F2) obj_extend(o,F1), obj_extend(o,F2) ///-
#define EXTEND4(o,F1,F2,F3) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3) ///-
#define EXTEND5(o,F1,F2,F3,F4) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4) ///-
#define EXTEND6(o,F1,F2,F3,F4,F5) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4), obj_extend(o,F5) ///-
#define EXTEND7(o,F1,F2,F3,F4,F5,F6) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4), obj_extend(o,F5), obj_extend(o,F6) ///-
#define EXTEND8(o,F1,F2,F3,F4,F5,F6,F7) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4), obj_extend(o,F5), obj_extend(o,F6), obj_extend(o,F7) ///-
#define EXTEND9(o,F1,F2,F3,F4,F5,F6,F7,F8) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4), obj_extend(o,F5), obj_extend(o,F6), obj_extend(o,F7), obj_extend(o,F8) ///-
#define EXTEND10(o,F1,F2,F3,F4,F5,F6,F7,F8,F9) obj_extend(o,F1), obj_extend(o,F2), obj_extend(o,F3), obj_extend(o,F4), obj_extend(o,F5), obj_extend(o,F6), obj_extend(o,F7), obj_extend(o,F8), obj_extend(o,F9) ///-

#define EXTEND_T(...) EXPAND(EXTEND_T, __VA_ARGS__)
#define EXTEND_T2(o,F1) obj_extend_t(o,F1) ///-
#define EXTEND_T3(o,F1,F2) obj_extend_t(o,F1), obj_extend_t(o,F2) ///-
#define EXTEND_T4(o,F1,F2,F3) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3) ///-
#define EXTEND_T5(o,F1,F2,F3,F4) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4) ///-
#define EXTEND_T6(o,F1,F2,F3,F4,F5) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4), obj_extend_t(o,F5) ///-
#define EXTEND_T7(o,F1,F2,F3,F4,F5,F6) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4), obj_extend_t(o,F5), obj_extend_t(o,F6) ///-
#define EXTEND_T8(o,F1,F2,F3,F4,F5,F6,F7) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4), obj_extend_t(o,F5), obj_extend_t(o,F6), obj_extend_t(o,F7) ///-
#define EXTEND_T9(o,F1,F2,F3,F4,F5,F6,F7,F8) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4), obj_extend_t(o,F5), obj_extend_t(o,F6), obj_extend_t(o,F7), obj_extend_t(o,F8) ///-
#define EXTEND_T10(o,F1,F2,F3,F4,F5,F6,F7,F8,F9) obj_extend_t(o,F1), obj_extend_t(o,F2), obj_extend_t(o,F3), obj_extend_t(o,F4), obj_extend_t(o,F5), obj_extend_t(o,F6), obj_extend_t(o,F7), obj_extend_t(o,F8), obj_extend_t(o,F9) ///-

// --- declare vtables

API extern void  (*obj_ctor[256])(); ///-
API extern void  (*obj_dtor[256])(); ///-

API extern char* (*obj_save[256])(); ///-
API extern bool  (*obj_load[256])(); ///-
API extern int   (*obj_test[256])(); ///-

API extern int   (*obj_init[256])(); ///-
API extern int   (*obj_quit[256])(); ///-
API extern int   (*obj_tick[256])(); ///-
API extern int   (*obj_draw[256])(); ///-
API extern int   (*obj_lerp[256])(); ///-

API extern int   (*obj_aabb[256])(); ///-
API extern int   (*obj_edit[256])(); ///-
API extern int   (*obj_menu[256])(); ///-
API extern char* (*obj_icon[256])(); ///-

API extern const char*OBJTYPES[256]; ///-

// ----------------------------------------------------------------------------
// core

API uintptr_t   obj_header(const void *o);
API uintptr_t   obj_id(const void *o);

API const char* obj_type(const void *o);
API unsigned    obj_typeid(const void *o);

API int         obj_sizeof(const void *o);
API int         obj_size(const void *o); // size of all members together in struct. may include padding bytes.

API char*       obj_data(void *o); // pointer to the first member in struct
API const char* obj_datac(const void *o); // const pointer to the first struct member

API void*       obj_payload(const void *o); // pointer right after last member in struct
API void*       obj_zero(void *o); // reset all object members

// ----------------------------------------------------------------------------
// refcounting

API void*       obj_ref(void *oo);
API void*       obj_unref(void *oo);

// ----------------------------------------------------------------------------
// scene tree

#define each_objchild(p,T,o) /*non-recursive*/ \
    (array(struct obj*)* children = obj_children(p); children; children = 0) \
        for(int _i = 1, _end = array_count(*children); _i < _end; ++_i) \
            for(T o = (T)((*children)[_i]); o && (obj_parent(o) == p); o = 0)

API obj*        obj_detach(void *c);
API obj*        obj_attach(void *o, void *c);

API obj*        obj_root(const void *o);
API obj*        obj_parent(const void *o);
API array(obj*)*obj_children(const void *o); // child[0]: parent, child[1]: 1st child, child[2]: 2nd child...
API array(obj*)*obj_siblings(const void *o); // child[0]: grandpa, child[1]: sibling1, child[2]: sibling2...

API int         obj_dumptree(const void *o);

// ----------------------------------------------------------------------------
// metadata

API void*       obj_setmeta(void *o, const char *key, const char *value);
API const char* obj_meta(const void *o, const char *key);

API void*       obj_setname(void *o, const char *name);
API const char* obj_name(const void *o);

// ----------------------------------------------------------------------------
// stl

API void*       obj_swap(void *dst, void *src);
API void*       obj_copy_fast(void *dst, const void *src);
API void*       obj_copy(void *dst, const void *src);

API int         obj_comp_fast(const void *a, const void *b);
API int         obj_comp(const void *a, const void *b);
API int         obj_lesser(const void *a, const void *b);
API int         obj_greater(const void *a, const void *b);
API int         obj_equal(const void *a, const void *b);

API uint64_t    obj_hash(const void *o);

// ----------------------------------------------------------------------------
// debug

API bool        obj_hexdump(const void *oo);
API int         obj_print(const void *o);

API int         obj_printf(const void *o, const char *text);
API int         obj_console(const void *o); // obj_output() ?

#define         obj_printf(o, ...) obj_printf(o, va(__VA_ARGS__))

// ----------------------------------------------------------------------------
// serialization

API char*       obj_saveini(const void *o);
API obj*        obj_mergeini(void *o, const char *ini);
API obj*        obj_loadini(void *o, const char *ini);

API char*       obj_savejson(const void *o);
API obj*        obj_mergejson(void *o, const char *json);
API obj*        obj_loadjson(void *o, const char *json);

API char*       obj_savebin(const void *o);
API obj*        obj_mergebin(void *o, const char *sav);
API obj*        obj_loadbin(void *o, const char *sav);

API char*       obj_savempack(const void *o); // @todo
API obj*        obj_mergempack(void *o, const char *sav); // @todo
API obj*        obj_loadmpack(void *o, const char *sav); // @todo

API int         obj_push(const void *o);
API int         obj_pop(void *o);

// ----------------------------------------------------------------------------
// components

API bool        obj_addcomponent(entity *e, unsigned c, void *ptr);
API bool        obj_hascomponent(entity *e, unsigned c);
API void*       obj_getcomponent(entity *e, unsigned c);
API bool        obj_delcomponent(entity *e, unsigned c);
API bool        obj_usecomponent(entity *e, unsigned c);
API bool        obj_offcomponent(entity *e, unsigned c);

API char*       entity_save(entity *self);

// ----------------------------------------------------------------------------
// reflection

#define each_objmember(oo,TYPE,NAME,PTR) \
    (array(reflect_t) *found_ = members_find(obj_type(oo)); found_; found_ = 0) \
        for(int it_ = 0, end_ = array_count(*found_); it_ != end_; ++it_ ) \
            for(reflect_t *R = &(*found_)[it_]; R; R = 0 ) \
                for(const char *NAME = R->name, *TYPE = R->type; NAME || TYPE; ) \
                    for(void *PTR = ((char*)oo) + R->sz ; NAME || TYPE ; NAME = TYPE = 0 )

API void*       obj_clone(const void *src);
API void*       obj_merge(void *dst, const void *src); // @testme
API void*       obj_mutate(void *dst, const void *src);
API void*       obj_make(const char *str);

// built-ins

typedef enum OBJTYPE_BUILTINS {
    OBJTYPE_obj    =  0,
    OBJTYPE_entity =  1,
    OBJTYPE_vec2   =  2,
    OBJTYPE_vec3   =  3,
    OBJTYPE_vec4   =  4,
    OBJTYPE_quat   =  5,
    OBJTYPE_mat33  =  6,
    OBJTYPE_mat34  =  7,
    OBJTYPE_mat44  =  8,
    OBJTYPE_vec2i  =  9,
    OBJTYPE_vec3i  = 10,
} OBJTYPE_BUILTINS;
