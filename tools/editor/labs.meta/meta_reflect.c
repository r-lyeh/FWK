// missing //M(eta), DNA,
// meta_parse(file), meta_load, meta_save

#define REFLECT_C
//#define REFLECT_DEMO

// C reflection: enums, functions, structs, nested structs, members and pointers.
// - rlyeh, public domain

#ifndef REFLECT_H
#define REFLECT_H

// # reflection api
//
// - reflected symbol struct
//
// - define reflected symbol (3 quick macros).
// - define reflected symbol (complete function).
//
// - size of reflected symbol.
// - find reflected function (by name).
// - find reflected enum (by name).
// - find reflected field in struct (by name).
//
// - iterate all reflected fields in struct.
// - iterate all reflected symbols in registry.
//
// - @todo: reflect* reflect_find()
// - @todo: code annotations? "display-name", "min", "max", "range", "default"
// - @todo: declare TYPEDEF(vec3, float[3]), TYPEDEF(mat4, vec4[4]/*float[16]*/)

typedef struct reflect {
    union {
    void *any;
    int offs;
    };
    const char *type, *name, *base, *info;
    unsigned size:23, is_pod:1, is_ptr:6, internal_type:2;
} reflect;

#define ENUM(type, name, ...)             reflect_add(0, (void*)name, #type, #name, "", "" #__VA_ARGS__ "\0", sizeof(enum type))
#define FUNCTION(type, name, ...)         reflect_add(1, &name, #type, #name, "", "" #__VA_ARGS__ "\0", sizeof(void *))
#define STRUCT(struct, type, name, ...)   reflect_add(2, &(((struct *)0)->name), #type, #name, #struct, "" #__VA_ARGS__ "\0", (int)sizeof(type) )

void  reflect_add( int internal_type, void *any, const char *type, const char *name, const char *base, const char *info, int size );

int   reflect_sizeof( const char *type );
void* reflect_function( const char *name );
int   reflect_enum( const char *name );
void* reflect_field( const char *type, void *obj, const char *name, const char **opt_type );

bool  reflect_has_fields( const char *base, void *obj );
void  reflect_iterate_fields( const char *type, void *obj, void (*callback)( const reflect *r, void *value, void *userdata ), void *userdata );
void  reflect_iterate_registry( void (*callback)( const reflect *r, void *userdata ), void *userdata );

#endif

// ----------------------------------------------------------------------------

#ifdef REFLECT_C
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef REFLECT_REGISTRY_LIMIT
#define REFLECT_REGISTRY_LIMIT 0 // 0 for unlimited registry size (using heap), or N for fixed max entries (using stack)
#endif

#if REFLECT_REGISTRY_LIMIT > 0
static reflect *registry[REFLECT_REGISTRY_LIMIT] = {0}, registry_[REFLECT_REGISTRY_LIMIT] = {0};
#else
static reflect **registry = 0;
#endif

enum {
    REFLECT_TYPE_ENUM,
    REFLECT_TYPE_FUNCTION,
    REFLECT_TYPE_FIELD,
};

static int reflect_counter = 0;

void reflect_add( int internal_type, void *any, const char *type, const char *name, const char *base, const char *info, int size ) {
    ++reflect_counter;

#if REFLECT_REGISTRY_LIMIT > 0
    registry[reflect_counter-1] = &registry_[reflect_counter-1];
#else
    registry = (reflect **)realloc( registry, reflect_counter * sizeof(reflect *));
    registry[reflect_counter-1] = (reflect *)malloc( sizeof(reflect) );
#endif

    reflect s = { any, type, name, base, info, size, 1, 0, internal_type };

    reflect *r = registry[reflect_counter-1];
    *r = s;

    // evaluate is_ptr
    for( int i = strlen(r->type); !r->is_ptr && --i; ) {
        r->is_ptr = r->type[i] == '*' ? i : 0;
    }
    for( int i = r->is_ptr; i >= 0; --i ) {
        r->is_ptr = r->type[i] == ' ' ? i : r->is_ptr;
    }

    // @fixme: try to avoid dynamic allocs
    // future me: removing this will break some strcmps(type) below
    if( r->is_ptr ) {
        char buf[128]; sprintf(buf, "%.*s", r->is_ptr, r->type);
        r->type = (const char *)strdup(buf);
    }

    // evaluate is_pod. kind of bubble sort.
    for( int i = 0; i < reflect_counter; ++i ) {
        for( int j = 0; j < reflect_counter; ++j ) {
            if( !strcmp(registry[i]->base, registry[j]->type) ) {
                registry[j]->is_pod = 0;
            }
        }
    }
}

int reflect_sizeof( const char *base ) {
    int size = 0;
    if( base ) for( int i = 0; i < reflect_counter; ++i ) {
        if( !strcmp(registry[i]->base, base) ) size += registry[i]->size;
    }
    return size;
}

void *reflect_field( const char *base, void *obj, const char *name, const char **opt_type ) {
    if( base ) for( int i = 0; i < reflect_counter; ++i ) {
        if( registry[i]->internal_type == REFLECT_TYPE_FIELD && strcmp(registry[i]->base, base) ) continue;
        if(!strcmp(registry[i]->name, name) ) return (opt_type ? *opt_type = registry[i]->type : NULL), (char*)obj + (long long int)registry[i]->any;
    }
    return 0;
}

int reflect_enum( const char *name ) {
    if( name ) for( int i = 0; i < reflect_counter; ++i ) {
        if( registry[i]->internal_type == REFLECT_TYPE_ENUM && !strcmp( registry[i]->name, name ) ) {
            return (int)(long long int)registry[i]->any;
        }
    }
    return 0;
}

// do not use void(void) signature here
static void* reflect_dummy_call() {
    return 0;
}

void* reflect_function( const char *name ) {
    if( name ) for( int i = 0; i < reflect_counter; ++i ) {
        if( registry[i]->internal_type == REFLECT_TYPE_FUNCTION && !strcmp( registry[i]->name, name ) ) {
            return registry[i]->any;
        }
    }
    return &reflect_dummy_call; // return NULL instead?
}

void reflect_iterate_registry( void (*callback)( const reflect *r, void *userdata ), void *userdata ) {
    for( int i = 0; i < reflect_counter; ++i ) {
        callback( registry[i], userdata );
    }
}

#ifdef _MSC_VER
__declspec(thread)
#else
__thread
#endif
struct reflect_context {
    const char *name;
    const char *info;
} reflect_ctx = { "", "" };

void reflect_iterate_fields( const char *base, void *obj, void (*callback)( const reflect *r, void *value, void *userdata ), void *userdata ) {
    if( base ) for( int i = 0, nb = strlen(base); i < reflect_counter; ++i ) {
        if( registry[i]->internal_type == REFLECT_TYPE_FIELD && !strncmp(registry[i]->base, base, nb) ) {
            for( ; i < reflect_counter && !strncmp(registry[i]->base, base, nb); ++i ) {
                void *any = ((char*)obj + (long long int)registry[i]->any);

                if( any && registry[i]->is_ptr ) {
                    any = (void*)*((long long int *)any);
                }

                if( any ) {
                    struct reflect_context copy = reflect_ctx;
                    char buf1[128]; sprintf(buf1, "%s.%s", reflect_ctx.name, registry[i]->name);
                    char buf2[128] = {0}; if( registry[i]->info[0] ) sprintf(buf2, "%s%s%.*s", reflect_ctx.info, reflect_ctx.info[0] ? " > " : "", (int)(strlen(registry[i]->info) - 2), 1+registry[i]->info);
                    reflect_ctx.name = buf1 + (buf1[0] == '.');
                    reflect_ctx.info = buf2;

                    if( registry[i]->is_pod ) {
                        reflect m = *registry[i];
                        m.name = reflect_ctx.name;
                        m.info = reflect_ctx.info;
                        callback( &m, any, userdata );
                    } else {
                        char buf3[128], *rebase = (char*)registry[i]->type;
                        if(registry[i]->is_ptr) sprintf( rebase = buf3, "%.*s", registry[i]->is_ptr, registry[i]->type );
                        reflect_iterate_fields( rebase, any, callback, userdata );
                    }

                    reflect_ctx = copy;
                }
            }
            return;
        }
    }
}

bool reflect_has_fields( const char *base, void *obj ) {
    if( base ) for( int i = 0, nb = strlen(base); i < reflect_counter; ++i ) {
        if( registry[i]->internal_type == REFLECT_TYPE_FIELD && !strncmp(registry[i]->base, base, nb) ) {
            for( ; i < reflect_counter && !strncmp(registry[i]->base, base, nb); ++i ) {
                return true;
            }
        }
    }
    return false;
}

void reflect_dump_registry(FILE *fp) {
    fprintf( fp, "%s {\n", __FUNCTION__ );
    fprintf( fp, "\tsizeof(reflect)=%d\n", (int)sizeof(reflect) );

    for( int i = 0; i < reflect_counter; ++i ) {
        reflect *r = registry[i];

        /**/ if( r->internal_type == REFLECT_TYPE_FUNCTION )
        fprintf(fp, "\tfunction %s = %s; // %d@[%p] %s\n", r->name, r->type, r->size, r->any, r->info);
        else if( r->internal_type == REFLECT_TYPE_ENUM )
        fprintf(fp, "\tenum %s%s%s = %d; // %d %s\n", r->type[0] ? r->type : "", r->type[0] ? "." : "", r->name, (int)(long long int)r->any, r->size, r->info );
        else if( r->internal_type == REFLECT_TYPE_FIELD )
        fprintf(fp, "\t%s%s%s %s.%s; // %d@%d %s\n", r->is_pod ? "field " : "struct ", r->type, r->is_ptr ? "*" : "", r->base, r->name, r->size, (int)(long long int)r->any, r->info );
    }

    fprintf( fp, "} %s\n", __FUNCTION__ );
}

#endif

// ----------------------------------------------------------------------------

#ifdef META_DEMO_IMMEDIATE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void echo( const reflect *r, void *value, void *userdata ) {
    FILE *fp = (FILE*)userdata;
    if( fp ) {
    /**/ if( !strcmp(r->type, "int") )    fprintf(fp, "%6s %-32s = %d;\t// %s\n", r->type, r->name, *(int*)value, r->info);
    else if( !strcmp(r->type, "string") ) fprintf(fp, "%6s %-32s = %s;\t// %s\n", r->type, r->name, *(char**)value, r->info);
    else if( !strcmp(r->type, "float") )  fprintf(fp, "%6s %-32s = %f;\t// %s\n", r->type, r->name, *(float*)value, r->info);
    else if( !strcmp(r->type, "double") ) fprintf(fp, "%6s %-32s = %f;\t// %s\n", r->type, r->name, *(double*)value, r->info);
    }
}

enum MY_ENUM {
    TEXTURE = 101,
    IMAGE = 102,
};

typedef char* string;

typedef struct MyVec3 {
    double x,y,z;
} MyVec3;

typedef struct MyTransform {
    MyVec3 location;
    MyVec3 rotation;
    float  scale;
} MyTransform;

typedef struct MyObject {
    struct MyObject *parent;
    string id;
    int hash;
    MyTransform transform;
} MyObject;

int MyAddFunction( int a,int b ) {
    return a + b;
}


int main( int argc, char **argv ) {

    // # enums
    // - register
    // - reflect

    ENUM( MY_ENUM, TEXTURE );
    ENUM( MY_ENUM, IMAGE );

    assert( 101 == reflect_enum("TEXTURE") );
    assert( 102 == reflect_enum("IMAGE") );

    // # functions
    // - register
    // - find
    // - call
    // - try call (undefined functions are also safe to call)

    FUNCTION( int(int a, int b), MyAddFunction, "Function adding two numbers" );
    FUNCTION( int(const char *fmt, ...), printf, "Print text to console" );

    int (*add_hook)() = reflect_function("MyAddFunction");
    int (*print_hook)(const char *, ...) = reflect_function("printf");

    assert( 123 == add_hook(100,23) );
    print_hook("hello from reflected function %d\n", 123);

    print_hook = reflect_function("undefined_symbol_here$(·!!");
    print_hook("this call should never print\n");
    puts("---");

    // # structs
    // - register simple
    // - register nested
    // - register nested with pointers
    // - print struct sizes
    // - iterate simple
    // - iterate nested
    // - iterate nested with pointers

    STRUCT( MyVec3, double, x, "Right" );
    STRUCT( MyVec3, double, y, "Forward" );
    STRUCT( MyVec3, double, z, "Up" );

    STRUCT( MyTransform, MyVec3, location, "World location (absolute)" );
    STRUCT( MyTransform, MyVec3, rotation, "Local rotation (in degrees)" );
    STRUCT( MyTransform, float, scale, "Local scale (in centimeters)" );

    STRUCT( MyObject, int, hash, "Actor hash" );
    STRUCT( MyObject, string, id, "Actor name" );
    STRUCT( MyObject, MyTransform, transform, "Actor transform" );
    STRUCT( MyObject, MyObject *, parent, "Actor parent" );

    printf("reflect_sizeof(MyVec3)=%d\n", reflect_sizeof("MyVec3"));
    printf("reflect_sizeof(MyTransform)=%d\n", reflect_sizeof("MyTransform"));
    printf("reflect_sizeof(MyObject)=%d\n", reflect_sizeof("MyObject"));
    puts("---");

    MyVec3 vec = {1.1,2.2,3.3};
    reflect_iterate_fields( "MyVec3", &vec, echo, stdout );
    puts("---");

    MyTransform tf = { {1.1,2.2,3.3}, {45,90,180}, 10 };
    reflect_iterate_fields( "MyTransform", &tf, echo, stdout );
    puts("---");

    MyObject root = { NULL, "Scene root", 0, { {0,0,0}, {0,0,0}, 1 } };
    MyObject obj = { &root, "Name identifier", 123, { {1.1,2.2,3.3}, {45,90,180}, 10 } };
    reflect_iterate_fields( "MyObject", &obj, echo, stdout );
    puts("---");

    // # dump internals

    reflect_dump_registry(stdout);
    puts("---");

    // # benchmark

    #ifndef N
    #define N (argc > 1 ? atoi(argv[1]) : 500 * 1000)
    #endif

    clock_t t0 = clock();
    for( int i = 0; i < N; ++i) {
        reflect_iterate_fields( "MyObject", &obj, echo, NULL );
    }
    clock_t t1 = clock();

    double t = (t1 - t0) / (double)CLOCKS_PER_SEC;
    printf("Benchmark: processed %.f members in %5.2fs = %5.2f members/sec\n", N*9.0, t, (N*9.0)/t ); // 9 total members in MyObject
}

#endif

