// C reflection: enums, functions, structs, members and anotations.
// - rlyeh, public domain
//
// @todo: nested structs? pointers in members?
// @todo: declare TYPEDEF(vec3, float[3]), TYPEDEF(mat4, vec4[4]/*float[16]*/)

static map(unsigned, reflect_t) reflects;
static map(unsigned, array(reflect_t)) members;

void reflect_init() {
    if(!reflects) map_init_int(reflects);
    if(!members)  map_init_int(members);
}
AUTORUN {
    reflect_init();
}

static
const char* symbol(const char *s) {
    if( strbeg(s, "const ") ) s += 6;
    if( strbeg(s, "union ") ) s += 6;
    if( strbeg(s, "struct ") ) s += 7;
    if(!strstr(s, " *") ) return s;
    char *copy = va("%s", s);
    do strswap(copy," *","*"); while( strstr(copy, " *") ); // char  * -> char*
    return (const char *)copy;
}

void type_inscribe(const char *TY,unsigned TYsz,const char *infos) {
    reflect_init();
    unsigned TYid = intern(TY = symbol(TY));
    map_find_or_add(reflects, TYid, ((reflect_t){TYid, 0, TYsz, TY, infos}));
}
void enum_inscribe(const char *E,unsigned Eval,const char *infos) {
    reflect_init();
    unsigned Eid = intern(E = symbol(E));
    map_find_or_add(reflects, Eid, ((reflect_t){Eid,0, Eval, E,infos}));
}
unsigned enum_find(const char *E) {
    reflect_init();
    E = symbol(E);
    return map_find(reflects, intern(E))->sz;
}
void function_inscribe(const char *F,void *func,const char *infos) {
    reflect_init();
    unsigned Fid = intern(F = symbol(F));
    map_find_or_add(reflects, Fid, ((reflect_t){Fid,0, 0, F,infos, func}));
    reflect_t *found = map_find(reflects,Fid);
}
void *function_find(const char *F) {
    reflect_init();
    F = symbol(F);
    return map_find(reflects, intern(F))->addr;
}
void struct_inscribe(const char *T,unsigned Tsz,unsigned OBJTYPEid, const char *infos) {
    reflect_init();
    unsigned Tid = intern(T = symbol(T));
    map_find_or_add(reflects, Tid, ((reflect_t){Tid, OBJTYPEid, Tsz, T, infos}));
}
void member_inscribe(const char *T, const char *M,unsigned Msz, const char *infos, const char *type, unsigned bytes) {
    reflect_init();
    unsigned Tid = intern(T = symbol(T));
    unsigned Mid = intern(M = symbol(M));
    type = symbol(type);
    map_find_or_add(reflects, (Mid<<16)|Tid, ((reflect_t){Mid, 0, Msz, M, infos, NULL, Tid, type }));
    // add member separately as well
    if(!members) map_init_int(members);
    array(reflect_t) *found = map_find_or_add(members, Tid, 0);
    array_push(*found, ((reflect_t){Mid, 0, Msz, M, infos, NULL, Tid, type, bytes }));
}
reflect_t member_find(const char *T, const char *M) {
    reflect_init();
    T = symbol(T);
    M = symbol(M);
    return *map_find(reflects, (intern(M)<<16)|intern(T));
}
void *member_findptr(void *obj, const char *T, const char *M) {
    reflect_init();
    T = symbol(T);
    M = symbol(M);
    return (char*)obj + member_find(T,M).sz;
}
array(reflect_t)* members_find(const char *T) {
    reflect_init();
    T = symbol(T);
    return map_find(members, intern(T));
}

static
void ui_reflect_(const reflect_t *R, const char *filter, int mask) {
    // debug:
    // ui_label(va("name:%s info:'%s' id:%u objtype:%u sz:%u addr:%p parent:%u type:%s\n",
    //     R->name ? R->name : "", R->info ? R->info : "", R->id, R->objtype, R->sz, R->addr, R->parent, R->type ? R->type : ""));

    if( mask == *R->info ) {
        static __thread char *buf = 0;
        if( buf ) *buf = '\0';

        struct nk_context *ui_ctx = (struct nk_context *)ui_handle();
        /*for ui_push_hspace(16)*/ {
            array(reflect_t) *T = map_find(members, intern(R->name));
            /**/ if( T )         {ui_label(strcatf(&buf,"S struct %s@%s", R->name, R->info+1)); for each_array_ptr(*T, reflect_t, it) if(strmatchi(it->name,filter)) ui_reflect_(it,filter,'M'); }
            else if( R->addr )    ui_label(strcatf(&buf,"F func %s()@%s", R->name, R->info+1));
            else if( !R->parent ) ui_label(strcatf(&buf,"E enum %s = %d@%s", R->name, R->sz, R->info+1));
            else                  ui_label(strcatf(&buf,"M %s %s@%s", R->type, R->name, R->info+1));
        }
    }
}

API void *ui_handle();
int ui_reflect(const char *filter) {
    if( !filter ) filter = "*";

    int enabled = ui_enabled();
    ui_disable();

        // ENUMS, then FUNCTIONS, then STRUCTS
        unsigned masks[] = { 'E', 'F', 'S' };
        for( int i = 0; i < countof(masks); ++i )
        for each_map_ptr(reflects, unsigned, k, reflect_t, R) {
            if( strmatchi(R->name, filter)) {
                ui_reflect_(R, filter, masks[i]);
            }
        }

    if( enabled ) ui_enable();
    return 0;
}

// -- tests

// type0 is reserved (no type)
// type1 reserved for objs
// type2 reserved for entities
// @todo: type3 and 4 likely reserved for components and systems??
// enum { OBJTYPE_vec3 = 0x03 };

AUTOTEST {
    // register structs, enums and functions. with and without comments+tags

    STRUCT( vec3, float, x );
    STRUCT( vec3, float, y );
    STRUCT( vec3, float, z, "Up" );

    ENUM( IMAGE_RGB );
    ENUM( TEXTURE_RGB, "3-channel Red+Green+Blue texture flag" );
    ENUM( TEXTURE_RGBA, "4-channel Red+Green+Blue+Alpha texture flag" );

    FUNCTION( puts );
    FUNCTION( printf, "function that prints formatted text to stdout" );

    // verify some reflected infos

    test( function_find("puts") == puts );
    test( function_find("printf") == printf );

    test( enum_find("TEXTURE_RGB") == TEXTURE_RGB );
    test( enum_find("TEXTURE_RGBA") == TEXTURE_RGBA );

    // iterate reflected struct
    for each_member("vec3", R) {
        //printf("+%s vec3.%s (+%x) // %s\n", R->type, R->name, R->member_offset, R->info);
    }

    //reflect_print("puts");
    //reflect_print("TEXTURE_RGBA");
    //reflect_print("vec3");

    //reflect_dump("*");
}
