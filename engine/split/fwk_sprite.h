// -----------------------------------------------------------------------------
// sprites

typedef enum SPRITE_FLAGS {
    SPRITE_PROJECTED = 1,
    SPRITE_ADDITIVE = 2,
    SPRITE_CENTERED = 4,
    SPRITE_RESOLUTION_INDEPENDANT = 128,
} SPRITE_FLAGS;

// texture id, position(x,y,depth sort), tint color, rotation angle
API void sprite( texture_t texture, float position[3], float rotation /*0*/, unsigned color /*~0u*/, unsigned flags);

// texture id, rect(x,y,w,h) is [0..1] normalized, then: pos(xyz,z-index), (scale.xy,offset.xy), rotation (degrees), color (rgba)
API void sprite_rect( texture_t t, vec4 rect, vec4 pos, vec4 scaleoff, float tilt_deg, unsigned tint_rgba, unsigned flags);

// texture id, sheet(frameNumber,X,Y) (frame in a X*Y spritesheet), position(x,y,depth sort), rotation angle, offset(x,y), scale(x,y), is_additive, tint color
API void sprite_sheet( texture_t texture, float sheet[3], float position[3], float rotation, float offset[2], float scale[2], unsigned rgba, unsigned flags);

API void sprite_flush();

// -----------------------------------------------------------------------------
// tilemaps

typedef struct tileset_t {
    texture_t tex;            // spritesheet
    unsigned tile_w, tile_h;  // dimensions per tile in pixels
    unsigned cols, rows;      // tileset num_cols, num_rows
    unsigned selected;        // active tile (while editing)
} tileset_t;

API tileset_t tileset(texture_t tex, unsigned tile_w, unsigned tile_h, unsigned cols, unsigned rows);

API int       ui_tileset( tileset_t t );

typedef struct tilemap_t {
    int blank_chr;                // transparent tile
    unsigned cols, rows;          // map dimensions (in tiles)
    array(int) map;

    vec3 position;                // x,y,scale
    float zindex;
    float tilt;
    unsigned tint;
    bool is_additive;
} tilemap_t;

API tilemap_t tilemap(const char *map, int blank_chr, int linefeed_chr);
API void      tilemap_render( tilemap_t m, tileset_t style );
API void      tilemap_render_ext( tilemap_t m, tileset_t style, float zindex, float xy_zoom[3], float tilt, unsigned tint, bool is_additive );

// -----------------------------------------------------------------------------
// tiled maps

typedef struct tiled_t {
    char *map_name;
    unsigned first_gid, tilew, tileh, w, h;

    bool parallax;
    vec3 position;
    array(bool) visible;
    array(tilemap_t) layers;
    array(tileset_t) sets;
    array(char*) names;
} tiled_t;

API tiled_t tiled(const char *file_tmx);
API void    tiled_render(tiled_t tmx, vec3 pos);

API void    ui_tiled(tiled_t *t);

// -----------------------------------------------------------------------------
// spines

typedef struct spine_t spine_t;

API spine_t*spine(const char *file_json, const char *file_atlas, unsigned flags);
API void    spine_skin(spine_t *p, unsigned skin);
API void    spine_render(spine_t *p, vec3 offset, unsigned flags);
API void    spine_animate(spine_t *p, float delta);

API void    ui_spine(spine_t *p);

// ----------------------------------------------------------------------------
// atlas api
typedef struct atlas_frame_t {
    unsigned delay;
    vec4 sheet;
    vec2 anchor; // @todo
    array(vec3i) indices;
    array(vec2) coords;
    array(vec2) uvs;
} atlas_frame_t;

typedef struct atlas_anim_t {
    unsigned name;
    array(unsigned) frames;
} atlas_anim_t;

typedef struct atlas_slice_frame_t {
    vec4 bounds;
    bool has_9slice;
    vec4 core;
    vec2 pivot;
    unsigned color;
    char *text;
} atlas_slice_frame_t;

typedef struct atlas_slice_t {
    unsigned name;
    array(unsigned) frames;
} atlas_slice_t;

typedef struct atlas_t {
    texture_t tex;

    array(atlas_frame_t) frames;
    array(atlas_anim_t)  anims;
    array(atlas_slice_t) slices;
    array(atlas_slice_frame_t) slice_frames;

    quarks_db db;
} atlas_t;

API atlas_t atlas_create(const char *inifile, unsigned flags);
API int         ui_atlas(atlas_t *a);
API int         ui_atlas_frame(atlas_frame_t *f);
API void    atlas_destroy(atlas_t *a);

// ----------------------------------------------------------------------------
// sprite v2 api

typedef struct sprite_t { OBJ
    vec4 gamepad; // up,down,left,right
    vec2 fire;    // a,b

    vec4 pos;
    vec2 sca;
    float tilt;
    unsigned tint;
    unsigned frame;
    unsigned timer, timer_ms;
    unsigned flip_, flipped;
    unsigned play;
    bool paused;
    // array(unsigned) play_queue; or unsigned play_next;
    struct atlas_t *a; // shared
    //atlas_t own; // owned
} sprite_t;

OBJTYPEDEF(sprite_t,10);
API void     sprite_ctor(sprite_t *s);
API void     sprite_dtor(sprite_t *s);
API void     sprite_tick(sprite_t *s);
API void     sprite_draw(sprite_t *s);
API void     sprite_edit(sprite_t *s);

API sprite_t*sprite_new(const char *ase, int bindings[6]);
API void     sprite_del(sprite_t *s);
API void     sprite_setanim(sprite_t *s, unsigned name);
