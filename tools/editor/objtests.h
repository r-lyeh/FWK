AUTOTEST {
    test_obj_core();
}

// ----------------------------------------------------------------------------

typedef struct orc { OBJ
    char name[8];
} orc;

//#pragma pack(push, 1)
typedef struct boy { OBJ
    char name[8];
    float hp;
} boy;
//#pragma pack(pop)

OBJTYPEDEF(orc,100)
int   orc_ctor(orc* self) { strcpy(self->name, "orc!"); printf("i'm orc %p\n", self); return 0; }
int   orc_tick(orc* self) { printf("%p orc tick\n", self); return 0; }
char* orc_save(orc *self) { return obj_saveini(self); } // PACKMSG("ss", "orc_v1", self->name); }

OBJTYPEDEF(boy,101)
int   boy_ctor(boy* self) { strcpy(self->name, "boy!"); printf("i'm boy %p\n", self); self->hp = 0x80; return 0; }
int   boy_tick(boy* self) { printf("%p boy tick, hp:%f\n", self, self->hp); return 0; }
char* boy_save(boy *self) { return obj_saveini(self); } // PACKMSG("ssf", "boy_v1", self->name, self->hp); }

AUTOTEST {
    obj_extend(orc, ctor);
    obj_extend(orc, tick);
    obj_extend(orc, save);

    obj_extend(boy, ctor);
    obj_extend(boy, tick);
    obj_extend(boy, save);
}

AUTOTEST {
    // instance gameobjs

    boy *obj1 = obj_new(boy, "boy", 123);
    orc *obj2 = obj_new(orc, "orc");
    orc *obj3 = obj_new(orc, "orc");

    printf("%p\n", obj_datac(obj1));
    printf("%d vs %d vs %d\n", (int)sizeof(boy), obj_sizeof(obj1), obj_size(obj1));

    // generics
    obj_tick(obj1);
    obj_tick(obj2);
    obj_tick(obj3);

    obj_hexdump(obj1);

    obj_free(obj1); // will free

    obj_ref(obj2);
    obj_unref(obj2); // will free

    obj_ref(obj3);
    obj_free(obj3); // will do nothing
    obj_unref(obj3); // will free

    // make a dangling reference. this will be printed at end of program.
    static int dangling;
    obj1 = obj_new(boy, "boy", 123);
    obj_ref(obj1);
}

// --- scene

AUTOTEST {
    test_obj_scene();
}

// --- comps

struct WorldTravellerComponent {
    unsigned world_source;
    unsigned world_target;
};

struct TransformComponent {
    vec3 position;
    quat rotation;
    vec3 scale;
};

struct VelocityComponent {
    float speed;
};

struct LookComponent {
    float sensitivity; // _and_polarity;
    vec2 rotation;
};

struct PhysicsComponent {
    float gravity;
    vec3 velocity;
    vec3 acceleration;
    vec3 rotationVelocity;
    vec3 rotationAcceleration;
};

struct RenderComponent {
    aabb box;
    mesh_t mesh;
    texture_t texture;
};

AUTOTEST {
    test_obj_ecs();
}

// generics

// --- example: new class

// declare new object

TYPEDEF_STRUCT(box,102,
    int x,y,w,h;
);

// /* same as: */
// typedef struct box { OBJ
//     int x,y,w,h;
// } box;
// OBJTYPEDEF(box,102)

// implement a few built-in interfaces

#define box(...)         obj(box, __VA_ARGS__)
char*   box_save(const box *b) { return obj_saveini(b); } // PACKMSG("siiii", "box_v1", b->x, b->y, b->w, b->h); }
bool    box_load(box *b, const char* s) { return !!obj_loadini(b, s); } // char *header; return UNPACKMSG(s, "siiii", &header, &b->x, &b->y, &b->w, &b->h) && !strcmp(header, "box_v1"); }
int     box_test(box *b) { return b->w > 0 && b->h > 0; }
void    box_dtor(box *b) { puts("bye box!"); }

// create a new obj interface (not only for box! valid for every other obj that would extend on this)

obj_vtable(area, float, { return 0; });

#define obj_area(o,...) obj_method(area, o, ##__VA_ARGS__)

// implement area interface for box object

float   box_area(box *b) { return b->w * b->h; }

AUTOTEST {
    // reflect
    STRUCT(box,int,x);
    STRUCT(box,int,y);
    STRUCT(box,int,w);
    STRUCT(box,int,h);

    // extend
    obj_extend(box,save);
    obj_extend(box,load);
    obj_extend(box,test);
    obj_extend(box,dtor);
    obj_extend(box,area);

    // -- example

    box b = box(0,0,2,3);
    box *c = obj_new(box, 1,1,3,4);

    test( obj_test(&b) );
    test( obj_test(c) );

    test( obj_area(&b) == 6 );
    test( obj_area(c) == 12 );

    // serialization tests
    test_obj_serialization(&b, c);

    test_obj_similarity(&b, c);
    char *sav = obj_save(c);
    obj_load(&b, sav);
    // obj_load(&b, obj_save(c)); // @fixme: this expression in mingw
    test_obj_equality(&b, c);
}

// --- reflection

typedef struct MyVec3 { OBJ
    float x,y,z;
} MyVec3;

OBJTYPEDEF(MyVec3,77)

typedef struct MyTransform {
    MyVec3 location;          ///R @todo: ideally, we would want self-reflection to inscribe these members
    MyVec3 rotation;          ///R @todo: ideally, we would want self-reflection to inscribe these members
    float  scale;             ///R @todo: ideally, we would want self-reflection to inscribe these members
} MyTransform;

/*
// @todo: which is technically like doing
AUTORUN {
    STRUCT(MyTransform, MyVec3, location);
    STRUCT(MyTransform, MyVec3, rotation);
    STRUCT(MyTransform, float, scale);
}
*/

OBJTYPEDEF(MyTransform,78);

typedef struct MyObject { OBJ
    char *id;
    int spawnx,spawny;
    float time;
    struct MyObject *next;
    MyTransform tr;
} MyObject;

OBJTYPEDEF(MyObject,79);


AUTOTEST {
    STRUCT( MyVec3, float, x, "Right" );
    STRUCT( MyVec3, float, y, "Forward" );
    STRUCT( MyVec3, float, z, "Up" );

    STRUCT( MyTransform, MyVec3, location, "World location (absolute)" );
    STRUCT( MyTransform, MyVec3, rotation, "Local rotation (in degrees)" );
    STRUCT( MyTransform, float, scale, "Local scale (in centimeters)" );

    STRUCT( MyObject, int, spawnx, "Actor Spawn X" );
    STRUCT( MyObject, int, spawny, "Actor Spawn Y" );
    STRUCT( MyObject, string, id, "Actor name" );
    STRUCT( MyObject, MyTransform, tr, "Actor transform" );
    STRUCT( MyObject, float, time, "Actor time" );
    STRUCT( MyObject, MyObject *, next, "Next actor in seq" );

    // reflect_print("MyVec3");
    // reflect_print("MyTransform");
    // reflect_print("MyObject");

    // construct a type from a reflected struct
    MyVec3 *o = obj_new(MyVec3, 1,2,-3);
    test( 12 == obj_size(o) );
    MyVec3 *o2 = obj_make("[MyVec3]\nfloat.y=2\nfloat.x=1\nfloat.z=-3");
    test( !obj_comp(o,o2) ) || obj_hexdump(o), obj_hexdump(o2);

    for each_objmember(o,type,name,ptr) {
        /**/ if( !strcmp(type, "float") )  printf("%s %s = %f\n", type, name, *(float*)ptr );
        else if( !strcmp(type, "double") ) printf("%s %s = %f\n", type, name, *(double*)ptr );
    }

    test_obj_console(o);
    test_obj_console(o2);

    // Setup objects
    MyObject *root = obj_make("[MyObject]");
    obj_hexdump(root);

    MyObject *oo = obj_make("[MyObject]\nid=\"An identifier!\"\nx=123\ny=256\nrotation=90\nnext=root\n");
    obj_hexdump(oo);

    // Dump contents of our objects

    obj_print(oo);
    puts("---");

    obj_hexdump(oo);
    puts("---");

    // Save to mem

    char *sav = obj_savebin(oo);
    test( sav && strlen(sav) > 0 );

    // Clear

    obj_zero(oo);
    obj_hexdump(oo);
    puts("---");

    // Reload

    obj_loadbin(oo, sav);
    obj_hexdump(oo);
}

// --- Benchmarks for call overhead.

AUTOTEST {
    // Here, we're using a blank ctor call as a method to test/stress call overhead.
    //
    // results (old i5-4300/1.90Ghz laptop):
    // v1: 427 million calls/s. compiled with "cl /Ox     /Os /MT /DNDEBUG /GL /GF /arch:AVX2"
    // v2: 450 million calls/s. compiled with "cl /Ox /O2     /MT /DNDEBUG     /GF /arch:AVX2"

    double t;
    enum { N = 100000000 };

    t = -time_ss();
        MyVec3 o = obj(MyVec3, 1,2,3); obj_setname(&o, "MyVec3");
        for( unsigned i = 0; i < N; ++i ) {
            obj_ctor(&o);
        }
    t += time_ss();
    printf("Benchmark: %5.2f objcalls/s %5.2fM objcalls/s\n", N/(t), (N/1000)/(t*1000)); // ((N+N)*5) / (t) );

    t = -time_ss();
        MyVec3 *op = obj_new(MyVec3, 1,2,3);
        for( unsigned i = 0; i < N; ++i ) {
            obj_ctor(op);
        }
    t += time_ss();
    printf("Benchmark: %5.2f objcalls/s %5.2fM objcalls/s\n", N/(t), (N/1000)/(t*1000)); // ((N+N)*5) / (t) );
}

// --- metas

AUTOTEST {
    box b = box(1,2,3,4);

    test( !strcmp("box", obj_type(&b)) );
    test( !strcmp("box", obj_name(&b)) );

    b = box(1,2,3,4);
    obj_setname(&b, "MyBox1");

    test( !strcmp("box", obj_type(&b)) );
    test( !strcmp("MyBox1", obj_name(&b)) );

    test_obj_metadatas(&b);

    obj_free(&b);
}



TYPEDEF_STRUCT(test_vec3_serial, __COUNTER__,
    vec3 position;
    vec3 accel;
);
AUTOTEST {
    STRUCT(test_vec3_serial, vec3, position);
    STRUCT(test_vec3_serial, vec3, accel);
    test_vec3_serial v = obj(test_vec3_serial, {1,2,3},{4,5,6}), z = obj(test_vec3_serial);
    test(obj_comp(&v,&z) != 0) || obj_print(&v) & obj_print(&z);
    obj_loadini(&z, obj_saveini(&v));
    test(obj_comp(&v,&z) == 0) || obj_print(&z);
}

TYPEDEF_STRUCT(MyObject2, __COUNTER__,
    const char* id;
    int x,y;
    float rotation;
    struct MyObject2 *next;
);
AUTORUN {
    STRUCT(MyObject2, const char *, id);
    STRUCT(MyObject2, int, x);
    STRUCT(MyObject2, int, y);
    STRUCT(MyObject2, float, rotation);
    STRUCT(MyObject2, struct MyObject2*, next);

    // Construct two objects
    MyObject2 *root = obj_new(MyObject2);
    MyObject2 *o = obj_new(MyObject2, "An identifier!", 0x11, 0x22, 3.1415f, root );
    // Copy tests
    {
        printf("%d vs %d vs %d\n", (int)sizeof(MyObject2), obj_size(o), (int)sizeof(obj) + obj_size(o) + (int)sizeof(array(void*)));
        MyObject2 *clone = obj_clone(o);
        test(obj_comp(clone,o) == 0) || obj_print(o) & obj_print(clone) & obj_hexdump(o) & obj_hexdump(clone);
        test(obj_free(clone) == 0);
    }

    {
        MyObject2 *copy = obj_new(MyObject2);
        test(obj_copy(copy, o));
        test(obj_comp(copy,o)==0) || obj_print(copy);
        test(obj_free(copy) == 0);
    }

    {
        MyObject2 *copy = obj_new(MyObject2, "A different identifier!", 0x33, 0x44, 0.0f, root );
        test(obj_copy(copy, o));
        test(obj_comp(copy,o)==0) || obj_print(copy);
        test(obj_free(copy) == 0);
    }

    {
        void *copy = obj_malloc(100); // untyped class
        obj_mutate(copy, o);
        obj_print(copy);
        obj_copy(copy, o);
        obj_print(copy);
        obj_free(copy);
    }
}
