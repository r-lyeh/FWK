// -----------------------------------------------------------------------------
// in-game editor
// - rlyeh, public domain.

#define EDITOR_VERSION "2023.10"

// ----------------------------------------------------------------------------
// editor bindings

typedef struct editor_bind_t {
    const char *command;
    const char *bindings;
    void (*fn)();
} editor_bind_t;

API void editor_addbind(editor_bind_t bind);

#define EDITOR_BIND(CMD,KEYS,...) \
    void macro(editor_bind_##CMD##_fn_)() { __VA_ARGS__ }; AUTORUN { array_push(editor_binds, ((editor_bind_t){#CMD,KEYS,macro(editor_bind_##CMD##_fn_)}) ); }

// ----------------------------------------------------------------------------
// editor properties

#define EDITOR_PROPERTYDEF(T,property_name) \
    typedef map(void*,T) editor_##property_name##_map_t; \
API editor_##property_name##_map_t *editor_##property_name##_map(); \
API T editor_##property_name(const void *obj); \
API void editor_set##property_name(const void *obj, T value); \
API void editor_alt##property_name(const void *obj); \
API void editor_no##property_name(void *obj);

EDITOR_PROPERTYDEF(int,  open);     ///- whether object is tree opened in tree editor
EDITOR_PROPERTYDEF(int,  selected); ///- whether object is displaying a contextual popup or not
EDITOR_PROPERTYDEF(int,  changed);  ///- whether object is displaying a contextual popup or not
EDITOR_PROPERTYDEF(int,  popup);    ///- whether object is displaying a contextual popup or not
EDITOR_PROPERTYDEF(int,  bookmarked); ///-
EDITOR_PROPERTYDEF(int,  visible); ///-
EDITOR_PROPERTYDEF(int,  script); ///-
EDITOR_PROPERTYDEF(int,  event); ///-
EDITOR_PROPERTYDEF(char*,iconinstance); ///-
EDITOR_PROPERTYDEF(char*,iconclass); ///-
EDITOR_PROPERTYDEF(int,  treeoffsety); ///-

API void editor_destroy_properties(void *o);
API void editor_load_on_boot(void);
API void editor_save_on_quit(void);

// ----------------------------------------------------------------------------
// editor ui

enum EDITOR_MODE {
    EDITOR_PANEL,
    EDITOR_WINDOW,
    EDITOR_WINDOW_NK,
    EDITOR_WINDOW_NK_SMALL,
};

API int editor_begin(const char *title, int mode);
API int editor_end(int mode);

// ----------------------------------------------------------------------------------------
// editor selection

API int editor_filter();
API void editor_select(const char *mask);
API void editor_unselect(); // same than editor_select("!**");

API void editor_select_aabb(aabb box);
API void editor_selectgroup(obj *first, obj *last);
API void* editor_first_selected();
API void* editor_last_selected();

// ----------------------------------------------------------------------------------------
// editor instancing

API void editor_addtoworld(obj *o);
API void editor_watch(const void *o);
API void* editor_spawn(const char *ini); // deprecate?
API void editor_spawn1();

API void editor_destroy_selected();
API void editor_inspect(obj *o);

// ----------------------------------------------------------------------------------------
// editor utils

//API void  editor();
//API bool  editor_active();
API vec3   editor_pick(float mouse_x, float mouse_y);
API char*  editor_path(const char *path);

API void editor_setmouse(int x, int y);
API vec2 editor_glyph(int x, int y, const char *style, unsigned codepoint);
API vec2 editor_glyphs(int x, int y, const char *style, const char *utf8);
API void editor_gizmos(int dim);

// ----------------------------------------------------------------------------------------
// editor loop

API int         editor_send(const char *cmd); // returns job-id
API const char* editor_recv(int jobid, double timeout_ss);

API void editor_pump();
API void editor_frame( void (*game)(unsigned, float, double) );

// ----------------------------------------------------------------------------------------
// engine section: @todo: expand me

API float* engine_getf(const char *key);
API int*   engine_geti(const char *key);
API char** engine_gets(const char *key);
API int    engine_send(const char *cmd, const char *optional_value);

API int    ui_engine();
