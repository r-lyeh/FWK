// -----------------------------------------------------------------------------
// naive rendering framework
// - rlyeh, public domain
//
// IQM skeletal meshes by @lsalzman (public domain) - https://bit.ly/2OQh0Me
// SH code by @ands (public domain) - https://github.com/ands/spherical_harmonics_playground
// SHM code by @jarikomppa (unlicensed) - https://github.com/jarikomppa/shadertoolkit

typedef unsigned handle; // GLuint

// -----------------------------------------------------------------------------
// renderstate
typedef struct renderstate_t {
    // Clear color
    float clear_color[4];

    // Color mask
    bool color_mask[4];

    // Clear depth
    double clear_depth;

    // Depth test
    bool depth_test_enabled;
    bool depth_write_enabled;
    unsigned depth_func;
    
    // Polygon offset
    bool polygon_offset_enabled;
    float polygon_offset;
    float polygon_offset_factor;

    // Blending
    bool blend_enabled;
    unsigned blend_func;
    unsigned blend_src;
    unsigned blend_dst;

    // Culling
    bool cull_face_enabled;
    unsigned cull_face_mode;

    // Stencil test
    bool stencil_test_enabled;
    unsigned stencil_func;
    unsigned stencil_op_fail, stencil_op_zfail, stencil_op_zpass;
    int stencil_ref;
    unsigned stencil_read_mask;
    unsigned stencil_write_mask;

    // Face culling direction
    unsigned front_face; // GL_CW or GL_CCW

    // Line width
    bool line_smooth_enabled;
    float line_width;

    // Point size
    bool point_size_enabled;
    float point_size;

    // Polygon mode
    unsigned polygon_mode_face;
    unsigned polygon_mode_draw;

    // Scissor test
    bool scissor_test_enabled;
} renderstate_t;

API renderstate_t renderstate();
API bool            renderstate_compare(const renderstate_t *stateA, const renderstate_t *stateB);
API void            renderstate_apply(const renderstate_t *state);

// -----------------------------------------------------------------------------
// colors

API unsigned rgba( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
API unsigned bgra( uint8_t b, uint8_t g, uint8_t r, uint8_t a );
API unsigned rgbaf( float r, float g, float b, float a );
API unsigned bgraf( float b, float g, float r, float a );
API unsigned alpha( unsigned rgba );

#define RGBX(rgb,x)   ( ((rgb)&0xFFFFFF) | (((unsigned)(x))<<24) )
#define RGB3(r,g,b)   ( (255<<24) | ((b)<<16) | ((g)<<8) | (r) )
#define RGB4(r,g,b,a) ( ((a)<<24) | ((b)<<16) | ((g)<<8) | (r) )

#define BLACK   RGBX(0x000000,255)
#define WHITE   RGBX(0xE8F1FF,255)

#define RED     RGB3(   255, 0,48 )
#define GREEN   RGB3(  144,255,48 )
#define CYAN    RGB3(   0,192,255 )
#define ORANGE  RGB3(  255,144,48 )
#define PURPLE  RGB3(  102,77,102 ) // 178,128,255 )
#define YELLOW  RGB3(   255,224,0 )
#define GRAY    RGB3(    32,32,32 ) // dark gray
#define SILVER  RGB3( 149,149,149 ) // dark white, gray-ish
#define PINK    RGB3(  255,48,144 )
#define AQUA    RGB3(  48,255,144 )

#define BLUE    RGBX(0xB55A06,255)

API unsigned atorgba(const char *s);
API char *   rgbatoa(unsigned rgba);

// -----------------------------------------------------------------------------
// images

/// flags when constructing the image_t type. see: image, image_from_mem
/// IMAGE_R: 1-channel image (R)
/// IMAGE_RG: 2-channel image (R,G)
/// IMAGE_RGB: 3-channel image (R,G,B)
/// IMAGE_RGBA: 4-channel image (R,G,B,A)
/// IMAGE_FLIP: Flip image vertically
/// IMAGE_FLOAT: Float pixel components
enum IMAGE_FLAGS {
    IMAGE_R     = 0x01000,
    IMAGE_RG    = 0x02000,
    IMAGE_RGB   = 0x04000,
    IMAGE_RGBA  = 0x08000,
    IMAGE_FLIP  = 0x10000,
    IMAGE_FLOAT = 0x20000,
};

/// type that holds linear uncompressed bitmap of any given dimensions.
/// w,h: image dimensions in pixels. `x,y` alias.
/// comps: number of components per pixel. `n` alias.
/// pixels: untyped pointer to linear bitmap data. typed pointers use `pixels8/16/32/f` aliases.
/// see: texture_t
typedef struct image_t {
    union { unsigned x, w; };
    union { unsigned y, h; };
    union { unsigned n, comps; };
    union { void *pixels; uint8_t *pixels8; uint16_t *pixels16; uint32_t *pixels32; float *pixelsf; };
} image_t;

API image_t image(const char *pathfile, int flags);
API image_t image_from_mem(const void *ptr, int len, int flags);
API void    image_destroy(image_t *img);

// -----------------------------------------------------------------------------
// textures

enum TEXTURE_FLAGS {
    // UNIT[0..7]

    TEXTURE_BC1 = 8,  // DXT1, RGB with 8:1 compression ratio (+ optional 1bpp for alpha)
    TEXTURE_BC2 = 16, // DXT3, RGBA with 4:1 compression ratio (BC1 for RGB + 4bpp for alpha)
    TEXTURE_BC3 = 32, // DXT5, RGBA with 4:1 compression ratio (BC1 for RGB + BC4 for A)
//  TEXTURE_BC4,     // Alpha

    TEXTURE_NEAREST = 0,
    TEXTURE_LINEAR = 64,
    TEXTURE_MIPMAPS = 128,
    TEXTURE_ANISOTROPY = 1 << 30,

    TEXTURE_CLAMP = 0,
    TEXTURE_BORDER = 0x100,
    TEXTURE_REPEAT = 0x200,

    TEXTURE_BYTE = 0,
    TEXTURE_FLOAT = IMAGE_FLOAT,

    TEXTURE_COLOR = 0,
    TEXTURE_DEPTH = 0x800,

    TEXTURE_R = IMAGE_R,
    TEXTURE_RG = IMAGE_RG,
    TEXTURE_RGB = IMAGE_RGB,
    TEXTURE_RGBA = IMAGE_RGBA,
    TEXTURE_FLIP = IMAGE_FLIP,

    // @fixme
    TEXTURE_SRGB = 1 << 24,
    TEXTURE_BGR = 1 << 25,
    TEXTURE_BGRA = TEXTURE_BGR,
    TEXTURE_ARRAY = 1 << 26,
};

typedef struct texture_t {
    union { unsigned x, w; };
    union { unsigned y, h; };
    union { unsigned z, d; };
    union { unsigned n, bpp; };
    handle id;
    unsigned texel_type;
    unsigned flags;
    char* filename;
    bool transparent;
    unsigned fbo; // for texture recording
    union { unsigned userdata, delay; };
} texture_t;

API texture_t texture_compressed(const char *filename, unsigned flags);
API texture_t texture_compressed_from_mem(const void *data, int len, unsigned flags);

API texture_t texture(const char* filename, int flags);
API texture_t texture_from_mem(const void* ptr, int len, int flags);
API texture_t texture_create(unsigned w, unsigned h, unsigned n, const void *pixels, int flags);
API texture_t texture_checker();
API void      texture_destroy(texture_t *t);
API int       texture_unit(); // returns rolling counter up to GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
// textureLod(filename, dir, lod);
// void texture_add_loader( int(*loader)(const char *filename, int *w, int *h, int *bpp, int reqbpp, int flags) );
API unsigned  texture_update(texture_t *t, unsigned w, unsigned h, unsigned n, const void *pixels, int flags);

API bool      texture_rec_begin(texture_t *t, unsigned w, unsigned h); // texture_rec
API void      texture_rec_end(texture_t *t); // texture_rec

// -----------------------------------------------------------------------------
// brdf

API texture_t brdf_lut();

// -----------------------------------------------------------------------------
// colormap

typedef struct colormap_t {
    vec4 color;
    texture_t *texture;
} colormap_t;

API bool colormap( colormap_t *cm, const char *texture_name, bool load_as_srgb );

// -----------------------------------------------------------------------------
// fullscreen quads

API void fullscreen_quad_rgb( texture_t texture_rgb );
API void fullscreen_quad_rgb_flipped( texture_t texture );
API void fullscreen_quad_ycbcr( texture_t texture_YCbCr[3] );
API void fullscreen_quad_ycbcr_flipped( texture_t texture_YCbCr[3] );

// -----------------------------------------------------------------------------
// cubemaps

typedef struct cubemap_t {
    unsigned id;    // texture id
    vec3 sh[9];     // precomputed spherical harmonics coefficients
} cubemap_t;

API cubemap_t  cubemap( const image_t image, int flags ); // 1 equirectangular panorama
API cubemap_t  cubemap6( const image_t images[6], int flags ); // 6 cubemap faces
API void       cubemap_destroy(cubemap_t *c);
API cubemap_t* cubemap_get_active();

// -----------------------------------------------------------------------------
// fbos

API unsigned fbo( unsigned texture_color, unsigned texture_depth, int wr_flags );
API void     fbo_bind(unsigned id);
API void     fbo_unbind();
API void     fbo_destroy(unsigned id);

// -----------------------------------------------------------------------------
// shadowmaps

// #ifndef VSMCUBE
// #define VSMCUBE 0
// #endif
// #ifndef VSMBLUR
// #define VSMBLUR 1
// #endif

typedef struct shadowmap_t {
    mat44  shadowmatrix;
    mat44  mvp;
    mat44  mv;
    mat44  proj;
    vec4   light_position;
    int    saved_fb;
    int    saved_viewport[4];
    handle fbo, texture;
    int texture_width;
} shadowmap_t;

API shadowmap_t shadowmap(int texture_width); // = 1024
API void shadowmap_destroy(shadowmap_t *s);

API void shadowmap_set_shadowmatrix(shadowmap_t *s, vec3 aLightPos, vec3 aLightAt, vec3 aLightUp, const mat44 projection);
API void shadowmap_begin(shadowmap_t *s);
API void shadowmap_end(shadowmap_t *s);

// shadowmap utils

API void shadowmatrix_proj(mat44 shm_proj, float aLightFov, float znear, float zfar);
API void shadowmatrix_ortho(mat44 shm_proj, float left, float right, float bottom, float top, float znear, float zfar);

// -----------------------------------------------------------------------------
// shaders

API unsigned shader(const char *vs, const char *fs, const char *attribs, const char *fragcolor, const char *defines);
API unsigned shader_geom(const char *gs, const char *vs, const char *fs, const char *attribs, const char *fragcolor, const char *defines);
API unsigned shader_bind(unsigned program);
API      int shader_uniform(const char *name);
API     void shader_bool(const char *uniform, bool i );
API     void shader_int(const char *uniform, int i);
API     void shader_uint(const char *uniform, unsigned i );
API     void shader_float(const char *uniform, float f);
API     void shader_vec2(const char *uniform, vec2 v);
API     void shader_vec3(const char *uniform, vec3 v);
API     void shader_vec3v(const char *uniform, int count, vec3 *v);
API     void shader_vec4(const char *uniform, vec4 v);
API     void shader_mat44(const char *uniform, mat44 m);
API     void shader_texture(const char *sampler, texture_t texture);
API     void shader_texture_unit(const char *sampler, unsigned texture, unsigned unit);
API     void shader_colormap(const char *name, colormap_t cm);
API unsigned shader_get_active();
API void     shader_destroy(unsigned shader);

// reflection. [0..N] are shader properties

API unsigned     shader_properties(unsigned shader);
API char**       shader_property(unsigned shader, unsigned property_no);

API void         shader_apply_param(unsigned shader, unsigned param_no);
API void         shader_apply_params(unsigned shader, const char *parameter_mask);

API int          ui_shader(unsigned shader);
API int          ui_shaders();

// compute shaders
enum BUFFER_MODE {
    BUFFER_READ,
    BUFFER_WRITE,
    BUFFER_READ_WRITE
};

/// Loads the compute shader and compiles a GL program.
/// return: GL program, 0 if failed.
/// cs: shader source code
API unsigned compute(const char *cs);

/// Runs the compute program with provided global workgroup size on x y z grid.
/// wx: global workgroup size x
/// wy: global workgroup size y
/// wz: global workgroup size z
API void compute_dispatch(unsigned wx, unsigned wy, unsigned wz);

/// Binds a texture to the program
/// !!! Set `layer` to -1 to disable layered access.
/// t: texture to bind
/// unit: texture unit bind index
/// level: texture level access (MIP0, MIP1, ...)
/// layer: bind layer
/// access: texture access policy
/// see: BUFFER_MODE
API void shader_image(texture_t t, unsigned unit, unsigned level, int layer, unsigned access);

/// Binds a texture to the program
/// !!! Set `layer` to -1 to disable layered access.
/// texture: GL texture handle
/// unit: texture unit bind index
/// level: texture level access (MIP0, MIP1, ...)
/// layer: bind layer
/// texel_type: image texel format (RGBA8, RGBA32F, ...)
/// access: texture access policy
/// see: BUFFER_MODE
API void shader_image_unit(unsigned texture, unsigned unit, unsigned level, int layer, unsigned texel_type, unsigned access);

// gpu memory barriers

/// Blocks main thread until all memory operations are done by the GPU.
API void write_barrier();

/// Blocks main thread until all image operations are done by the GPU.
API void write_barrier_image();

// ssbo
/// `STATIC`, `DYNAMIC` AND `STREAM` specify the frequency at which we intend to access the data.
/// `DRAW` favors CPU->GPU operations.
/// `READ` favors GPU->CPU operations.
/// `COPY` favors CPU->GPU->CPU operations.
enum SSBO_USAGE {
    STATIC_DRAW,
    STATIC_READ,
    STATIC_COPY,

    DYNAMIC_DRAW,
    DYNAMIC_READ,
    DYNAMIC_COPY,

    STREAM_DRAW,
    STREAM_READ,
    STREAM_COPY
};

enum SSBO_ACCESS {
    SSBO_READ = BUFFER_READ,
    SSBO_WRITE = BUFFER_WRITE,
    SSBO_READ_WRITE = BUFFER_READ_WRITE
};

/// Create Shader Storage Buffer Object
/// !!! `data` can be NULL
/// data: optional pointer to data to upload
/// len: buffer size, must not be 0
/// usage: buffer usage policy
/// see: SSBO_USAGE
API unsigned ssbo_create(const void *data, int len, unsigned usage);

/// Destroys an SSBO resource
API void ssbo_destroy(unsigned ssbo);

/// Updates an existing SSBO
/// !!! `len` can not exceed the original buffer size specified in `ssbo_create` !
/// offset: offset to buffer memory
/// len: amount of data to write
/// data: pointer to data we aim to write, can not be NULL
API void ssbo_update(int offset, int len, const void *data);

/// Bind an SSBO resource to the provided bind unit index
/// ssbo: resource object
/// unit: bind unit index
API void ssbo_bind(unsigned ssbo, unsigned unit);

/// Map an SSBO resource to the system memory
/// !!! Make sure to `ssbo_unmap` the buffer once done working with it.
/// access: buffer access policy
/// return: pointer to physical memory of the buffer
/// see: SSBO_ACCESS
API void *ssbo_map(unsigned access);

/// Unmaps an SSBO resource
/// !!! Pointer provided by `ssbo_map` becomes invalid.
API void ssbo_unmap();

/// Unbinds an SSBO resource
API void ssbo_unbind();

// -----------------------------------------------------------------------------
// meshes (@fixme: deprecate?)

enum MESH_FLAGS {
    MESH_STATIC = 0, // STATIC, DYNAMIC, STREAM // zero|single|many updates per frame
    MESH_STREAM = 1,
    MESH_TRIANGLE_STRIP = 2,
};

typedef struct mesh_t {
    handle vao, vbo, ibo;
    unsigned vertex_count;
    unsigned index_count;
    unsigned flags;

    array(int) lod_collapse_map; // to which neighbor each vertex collapses. ie, [10] -> 7 (used by LODs) @leak

    // @leaks: following members are totally unused. convenient for end-users to keep their custom datas somewhere while processing.
    union {
    array(unsigned) in_index;
    array(vec3i)    in_index3;
    };
    union {
    array(unsigned) out_index;
    array(vec3i)    out_index3;
    };
    union {
    array(float) in_vertex;
    array(vec3) in_vertex3;
    };
    union {
    array(float) out_vertex;
    array(vec3) out_vertex3;
    };
} mesh_t;

API mesh_t mesh();
API   void mesh_update(mesh_t *m, const char *format, int vertex_stride,int vertex_count,const void *interleaved_vertex_data, int index_count,const void *index_data, int flags);
API   void mesh_render(mesh_t *m);
API   void mesh_render_prim(mesh_t *sm, unsigned prim);
API   void mesh_destroy(mesh_t *m);
API   aabb mesh_bounds(mesh_t *m);

// -----------------------------------------------------------------------------
// skyboxes

enum SKYBOX_FLAGS {
	SKYBOX_RAYLEIGH,
	SKYBOX_CUBEMAP,
	SKYBOX_PBR,
};

typedef struct skybox_t {
    handle program;
    mesh_t geometry;
    cubemap_t cubemap;
    int flags;

    // mie
    int framebuffers[6];
    int textures[6];
    float *pixels;

    // pbr
    texture_t sky, refl, env;
} skybox_t;

API skybox_t skybox(const char *panorama_or_cubemap_folder, int flags);
API skybox_t skybox_pbr(const char *sky_map, const char *refl_map, const char *env_map);
API int      skybox_render(skybox_t *sky, mat44 proj, mat44 view);
API void     skybox_destroy(skybox_t *sky);
API void     skybox_mie_calc_sh(skybox_t *sky, float sky_intensity);
API void     skybox_sh_reset(skybox_t *sky);
API void     skybox_sh_add_light(skybox_t *sky, vec3 light, vec3 dir, float strength);

API int      skybox_push_state(skybox_t *sky, mat44 proj, mat44 view); // @to deprecate
API int      skybox_pop_state(); // @to deprecate

// -----------------------------------------------------------------------------
// materials

enum MATERIAL_ENUMS {
	MATERIAL_CHANNEL_DIFFUSE,
	MATERIAL_CHANNEL_NORMALS,
	MATERIAL_CHANNEL_SPECULAR,
	MATERIAL_CHANNEL_ALBEDO,
	MATERIAL_CHANNEL_ROUGHNESS,
	MATERIAL_CHANNEL_METALLIC,
	MATERIAL_CHANNEL_AO,
	MATERIAL_CHANNEL_AMBIENT,
	MATERIAL_CHANNEL_EMISSIVE,
    
    MAX_CHANNELS_PER_MATERIAL
};

typedef struct material_layer_t {
    char   texname[32];
    float  value;
    colormap_t map;
} material_layer_t;

typedef struct material_t {
    char *name;
    material_layer_t layer[MAX_CHANNELS_PER_MATERIAL];
} material_t;

// -----------------------------------------------------------------------------
// shadertoys

enum {
    SHADERTOY_FLIP_Y = 2,
    SHADERTOY_IGNORE_FBO = 4,
    SHADERTOY_IGNORE_MOUSE = 8,
};

typedef struct shadertoy_t {
    handle vao, program;
    int uniforms[32];
    int texture_channels[4];
    int frame;
    uint64_t t;
    texture_t tx;
    vec2i dims;
    int flags;
    vec4 mouse;
} shadertoy_t;

API shadertoy_t  shadertoy( const char *shaderfile, unsigned flags );
API shadertoy_t* shadertoy_render( shadertoy_t *s, float delta );

// -----------------------------------------------------------------------------
// anims

enum ANIM_FLAGS {
    ANIM_LOOP = 1,
    ANIM_DONT_RESET_AFTER_USE = 2,
};

typedef struct anim_t {
    int        from;
    int        to;
    float      blendtime;
    unsigned   flags;
    float      curframe;

    unsigned   easing;
    float      alpha; // refreshed at every tick
    float      timer; // private
    bool       active;

    vec3       pose; // private
    char*      name; // debug
} anim_t;

API anim_t clip(float minframe, float maxframe, float blendtime, unsigned flags);
API anim_t loop(float minframe, float maxframe, float blendtime, unsigned flags);
API array(anim_t) animlist(const char *filename);

// -----------------------------------------------------------------------------
// models

enum MODEL_FLAGS {
    MODEL_NO_ANIMATIONS = 1,
    MODEL_NO_MESHES = 2,
    MODEL_NO_TEXTURES = 4,
    MODEL_NO_FILTERING = 8,
    MODEL_MATCAPS = 16,
    MODEL_RIMLIGHT = 32,
    MODEL_PBR = 64,
};

enum SHADING_MODE {
    SHADING_NONE,
    SHADING_PHONG,
    SHADING_PBR,
};

enum RENDER_PASS {
    RENDER_PASS_NORMAL,
    RENDER_PASS_SHADOW,
    RENDER_PASS_LIGHTMAP,
    
    NUM_RENDER_PASSES
};

enum MODEL_UNIFORMS {
    MODEL_UNIFORM_MV,
    MODEL_UNIFORM_MVP,
    MODEL_UNIFORM_VP,
    MODEL_UNIFORM_CAM_POS,
    MODEL_UNIFORM_CAM_DIR,
    MODEL_UNIFORM_BILLBOARD,
    MODEL_UNIFORM_TEXLIT,
    MODEL_UNIFORM_MODEL,
    MODEL_UNIFORM_VIEW,
    MODEL_UNIFORM_INV_VIEW,
    MODEL_UNIFORM_PROJ,
    MODEL_UNIFORM_SKINNED,
    MODEL_UNIFORM_VS_BONE_MATRIX,
    MODEL_UNIFORM_U_MATCAPS,
    MODEL_UNIFORM_RESOLUTION,
    MODEL_UNIFORM_HAS_TEX_SKYSPHERE,
    MODEL_UNIFORM_HAS_TEX_SKYENV,
    MODEL_UNIFORM_TEX_SKYSPHERE,
    MODEL_UNIFORM_SKYSPHERE_MIP_COUNT,
    MODEL_UNIFORM_TEX_SKYENV,
    MODEL_UNIFORM_TEX_BRDF_LUT,
    MODEL_UNIFORM_FRAME_COUNT,

    NUM_MODEL_UNIFORMS
};


typedef struct model_t {
    struct iqm_t *iqm; // private

    int shading; // based on SHADING_MODE
    unsigned num_textures;
    handle *textures;
    char **texture_names;
    array(material_t) materials;
    int uniforms[NUM_MODEL_UNIFORMS];
    
    texture_t sky_refl, sky_env;

    texture_t lightmap;
    float *lmdata;

    unsigned num_meshes;
    unsigned num_triangles;
    unsigned num_joints; // num_poses;
    unsigned num_anims;
    unsigned num_frames;
    handle program;
    float curframe;
    mat44 pivot;

    int stride; // usually 68 bytes for a p3 u2 u2 n3 t4 i4B w4B c4B vertex stream
    void *verts;
    int num_verts;
    void *tris;
    int num_tris;
    handle vao, ibo, vbo, vao_instanced;

    unsigned flags;
    unsigned billboard;

    float *instanced_matrices;
    unsigned num_instances;

    int stored_flags;
    renderstate_t rs[NUM_RENDER_PASSES];
} model_t;

enum BILLBOARD_MODE {
    BILLBOARD_X = 0x1,
    BILLBOARD_Y = 0x2,
    BILLBOARD_Z = 0x4,

    BILLBOARD_CYLINDRICAL = BILLBOARD_X|BILLBOARD_Z,
    BILLBOARD_SPHERICAL = BILLBOARD_X|BILLBOARD_Y|BILLBOARD_Z
};

API model_t  model(const char *filename, int flags);
API model_t  model_from_mem(const void *mem, int sz, int flags);
API float    model_animate(model_t, float curframe);
API float    model_animate_clip(model_t, float curframe, int minframe, int maxframe, bool loop);
API float    model_animate_blends(model_t m, anim_t *primary, anim_t *secondary, float delta);
API aabb     model_aabb(model_t, mat44 transform);
API void     model_shading(model_t*, int shading);
API void     model_skybox(model_t*, skybox_t sky, bool load_sh);
API void     model_render(model_t, mat44 proj, mat44 view, mat44 model, int shader);
API void     model_render_skeleton(model_t, mat44 model);
API void     model_render_instanced(model_t, mat44 proj, mat44 view, mat44 *models, int shader, unsigned count);
API void     model_set_texture(model_t, texture_t t);
API bool     model_get_bone_pose(model_t m, unsigned joint, mat34 *out);
API void     model_destroy(model_t);

API unsigned model_getpass();
API unsigned model_setpass(unsigned pass);

API vec3     pose(bool forward, float curframe, int minframe, int maxframe, bool loop, float *opt_retframe);

// -----------------------------------------------------------------------------
// model animations

typedef struct anims_t {
    int   inuse; // animation number in use
    float speed; // x1.00
    array(anim_t) anims; // [begin,end,flags] frames of every animation in set
} anims_t;

API anims_t animations(const char *pathfile, int flags);

// -----------------------------------------------------------------------------
// lightmapping utils
// @fixme: support xatlas uv packing

typedef struct lightmap_t {
    struct lm_context *ctx; // private
    bool ready;
    int w, h;
    int atlas_w, atlas_h; //@fixme: implement
    texture_t atlas; //@fixme: implement this
    array(model_t*) models;
    unsigned shader;
} lightmap_t;

API lightmap_t lightmap(int hmsize /*64*/, float near, float far, vec3 color /*1,1,1 for AO*/, int passes /*2*/, float threshold /*0.01f*/, float distmod /*0.0f*/);
API void       lightmap_setup(lightmap_t *lm, int w, int h);
API void          lightmap_bake(lightmap_t *lm, int bounces, void (*drawscene)(lightmap_t *lm, model_t *m, float *view, float *proj, void *userdata), void (*progressupdate)(float progress), void *userdata);
API void       lightmap_destroy(lightmap_t *lm);

// -----------------------------------------------------------------------------
// post-fxs

API void     viewport_color(unsigned color);
API void     viewport_clear(bool color, bool depth);
API void     viewport_clip(vec2 from, vec2 to);

API int      fx_load(const char *file);
API int      fx_load_from_mem(const char *nameid, const char *content);
API void     fx_begin();
API void     fx_begin_res(int w, int h);
API void     fx_end();
API void     fx_enable(int pass, int enabled);
API int      fx_enabled(int pass);
API void     fx_enable_all(int enabled);
API char *   fx_name(int pass);
API int      fx_find(const char *name);
API void     fx_setparam(int pass, const char *name, float value);
API void     fx_order(int pass, unsigned priority);
API unsigned fx_program(int pass);

API int      ui_fx(int pass);
API int      ui_fxs();

// -----------------------------------------------------------------------------
// utils

API void*    screenshot(int components); // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA
API void*    screenshot_async(int components); // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA
