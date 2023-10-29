// ----------------------------------------------------------------------------
// game ui

typedef struct guiskin_t {
    void (*drawrect)(void* userdata, const char *skin, vec4 rect);
    void (*getskinsize)(void* userdata, const char *skin, vec2 *size);
    void (*free)(void* userdata);
    void *userdata;
} guiskin_t;

API void    gui_pushskin(guiskin_t skin);
API void*       gui_userdata();
API vec2        gui_getskinsize(const char *skin);
// --
API void        gui_panel(int id, vec4 rect, const char *skin);
API bool        gui_button(int id, vec4 rect, const char *skin);
API void    gui_popskin();

// helpers
#define gui_panel(...) gui_panel(__LINE__, __VA_ARGS__)
#define gui_button(...) gui_button(__LINE__, __VA_ARGS__)

// default renderers

/// skinned
typedef struct skinned_t {
    atlas_t atlas;
    float scale;
} skinned_t;

API guiskin_t gui_skinned(const char *inifile, float scale);
