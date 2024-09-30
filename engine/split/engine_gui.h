// ----------------------------------------------------------------------------
// game ui

typedef struct guiskin_t {
    void (*drawrect)(void* userdata, const char *skin, const char *fallback, vec4 rect);
    void (*getskinsize)(void* userdata, const char *skin, const char *fallback, vec2 *size);
    void (*getskincolor)(void* userdata, const char *skin, const char *fallback, unsigned *color);
    void (*getscissorrect)(void* userdata, const char *skin, const char *fallback, vec4 rect, vec4 *dims);
    bool (*ismouseinrect)(void* userdata, const char *skin, const char *fallback, vec4 rect);
    void (*free)(void* userdata);
    void *userdata;
} guiskin_t;

API void    gui_pushskin(guiskin_t skin);
API void*       gui_userdata();
API vec2        gui_getskinsize(const char *skin, const char *fallback);
API unsigned    gui_getskincolor(const char *skin, const char *fallback);
API bool        gui_ismouseinrect(const char *skin, const char *fallback, vec4 rect);
API vec4        gui_getscissorrect(const char *skin, const char *fallback, vec4 rect);
// --
API void        gui_panel_id(int id, vec4 rect, const char *skin);
API void            gui_rect_id(int id, vec4 rect, const char *skin);
API void            gui_label_id(int id, const char *skin, const char *text, vec4 rect);
API bool            gui_button_id(int id, vec4 rect, const char *skin);
API bool            gui_button_label_id(int id, const char *text, vec4 rect, const char *skin);
API bool            gui_slider_id(int id, vec4 rect, const char *skin, float min, float max, float step, float *value);
API bool            gui_slider_label_id(int id, const char *text, vec4 rect, const char *skin, float min, float max, float step, float *value);
API void        gui_panel_end();
API void    gui_popskin();

// helpers
#define gui_panel(...) gui_panel_id(__LINE__, __VA_ARGS__)
#define gui_rect(...) gui_rect_id(__LINE__, __VA_ARGS__)
#define gui_label(...) gui_label_id(__LINE__, __VA_ARGS__)
#define gui_button(...) gui_button_id(__LINE__, __VA_ARGS__)
#define gui_button_label(...) gui_button_label_id(__LINE__, __VA_ARGS__)
#define gui_slider(...) gui_slider_id(__LINE__, __VA_ARGS__)
#define gui_slider_label(...) gui_slider_label_id(__LINE__, __VA_ARGS__)

// default renderers

typedef struct skinned_t {
    atlas_t atlas;
    float scale;
} skinned_t;

// The skinning engine depends on an Aseprite asset with slices set up.
//   While you can specify your own skins for various GUI widgets, some
//   skin variants are hardcoded and expected to be present in your asset:
//
//   gui_panel():
//    - "panel" (overridable)
//   gui_button():
//    - "button" (base overridable)
//      - "_hover" (ex. "scarybtn_hover")
//      - "_press"
//   gui_rect():
//    - no defaults, always pass your own skin/slice name
//   gui_slider():
//    - "slider" (overridable)
//    - "slider_cursor" (partially overridable, ex. "bigslider_cursor")
//      - "_hover" (ex. "slider_cursor_hover")
//      - "_press"
//
API guiskin_t gui_skinned(const char *asefile, float scale);
