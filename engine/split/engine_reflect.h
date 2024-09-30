// C reflection: enums, functions, structs, members and anotations.
// - rlyeh, public domain
//
// @todo: nested structs? pointers in members?
// @todo: declare TYPEDEF(vec3, float[3]), TYPEDEF(mat4, vec4[4]/*float[16]*/)

#ifndef OBJTYPE
#define OBJTYPE(T) 0
#endif

typedef struct reflect_t {
    unsigned id, objtype;
    union {
    unsigned sz;
    unsigned member_offset;
    unsigned enum_value;
    };
    const char *name;
    const char *info;
    void *addr;
    unsigned parent;
    const char *type;
    unsigned bytes;
} reflect_t;

// inscribe api

#define ENUM(V, ...) \
    enum_inscribe(#V,V, "E" __VA_ARGS__ " ("FILELINE")")

#define FUNCTION(F, ...) \
    function_inscribe(#F,(void*)F, "F" __VA_ARGS__ " ("FILELINE")")

#define STRUCT(T, type, member, ...) \
    struct_inscribe(#T,sizeof(T),OBJTYPE(T),"S" " ("FILELINE")"), \
    type_inscribe(#type,sizeof(((T){0}).member),"T" __VA_ARGS__ " ("FILELINE")"), \
    member_inscribe(#T, #member,(uintptr_t)&((T*)0)->member, "M" __VA_ARGS__ " ("FILELINE")", #type, sizeof(((T){0}).member) )

// find api

API unsigned           enum_find(const char *E);
API void *             function_find(const char *F);

API reflect_t          member_find(const char *T, const char *M); /// find specific member
API void *             member_findptr(void *obj, const char *T, const char *M); // @deprecate
API array(reflect_t)*  members_find(const char *T);

// iterate members in a struct

#define each_member(T,R) \
    (array(reflect_t) *found_ = members_find(T); found_; found_ = 0) \
        for(int it_ = 0, end_ = array_count(*found_); it_ != end_; ++it_ ) \
            for(reflect_t *R = &(*found_)[it_]; R; R = 0 )

// private api, still exposed

API void               type_inscribe(const char *TY,unsigned TYsz,const char *infos);
API void               enum_inscribe(const char *E,unsigned Eval,const char *infos);
API void               struct_inscribe(const char *T,unsigned Tsz,unsigned OBJTYPEid, const char *infos);
API void               member_inscribe(const char *T, const char *M,unsigned Msz, const char *infos, const char *type, unsigned bytes);
API void               function_inscribe(const char *F,void *func,const char *infos);
API const char*        symbol_naked(const char *s);

API int                ui_reflect(const char *mask); // *, model* or NULL
