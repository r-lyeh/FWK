// -----------------------------------------------------------------------------
// scene framework
// - rlyeh, public domain

// camera

typedef struct camera_t {
    mat44 view, proj;
    vec3 position, updir, lookdir;
    float yaw, pitch; // mirror of (x,y) lookdir in deg;
    float speed, fov; // fov in deg(45)

    float move_friction, move_damping;
    float look_friction, look_damping;
    vec2 last_look; vec3 last_move; // used for friction and damping
    bool damping;

    bool orthographic; // 0 perspective, 1 orthographic; when ortho: dimetric[if pitch == -30º], isometric[if pitch == 35.264º]
    float distance;    // distance to pivot, when orbiting
    // vec2 polarity = { +1,-1 }; // @todo
    // vec2 sensitivity = { 2,2 }; // @todo
} camera_t;

API camera_t camera();
API void camera_teleport(camera_t *cam, vec3 pos);
API void camera_moveby(camera_t *cam, vec3 inc);
API void camera_fov(camera_t *cam, float fov);
API void camera_fps(camera_t *cam, float yaw, float pitch);
API void camera_orbit(camera_t *cam, float yaw, float pitch, float inc_distance);
API void camera_lookat(camera_t *cam, vec3 target);
API void camera_enable(camera_t *cam);
API camera_t *camera_get_active();

API int  ui_camera(camera_t *cam);
API void ddraw_camera(camera_t *cam);

// object

typedef struct object_t {
    uint64_t renderbucket;
    mat44 transform;
    quat rot;
    vec3 sca, pos, euler, pivot;
    array(handle) textures;
    model_t model;
    aabb bounds;
    unsigned billboard; // [0..7] x(4),y(2),z(1) masks
    bool light_cached; //< used by scene to update light data
} object_t;

API object_t object();
API void object_rotate(object_t *obj, vec3 euler);
API void object_pivot(object_t *obj, vec3 euler);
API void object_teleport(object_t *obj, vec3 pos);
API void object_move(object_t *obj, vec3 inc);
API vec3 object_position(object_t *obj);
API void object_scale(object_t *obj, vec3 sca);
//
API void object_model(object_t *obj, model_t model);
API void object_diffuse(object_t *obj, texture_t tex);
API void object_diffuse_push(object_t *obj, texture_t tex);
API void object_diffuse_pop(object_t *obj);
API void object_billboard(object_t *obj, unsigned mode);

// object_pose(transform); // @todo


// light
enum LIGHT_TYPE {
    LIGHT_DIRECTIONAL,
    LIGHT_POINT,
    LIGHT_SPOT,
};

enum LIGHT_FLAGS {
    LIGHT_CAST_SHADOWS = 1,
};

typedef struct light_t {
    char type;
    vec3 diffuse, specular, ambient;
    vec3 pos, dir;
    struct {
        float constant, linear, quadratic;
    } falloff;
    float specularPower;
    float innerCone, outerCone;
    //@todo: cookie, flare

    // internals
    bool cached; //< used by scene to invalidate cached light data
} light_t;

API light_t light();
// API void    light_flags(int flags);
API void    light_type(light_t* l, char type);
API void    light_diffuse(light_t* l, vec3 color);
API void    light_specular(light_t* l, vec3 color);
API void    light_ambient(light_t* l, vec3 color);
API void    light_teleport(light_t* l, vec3 pos);
API void    light_dir(light_t* l, vec3 dir);
API void    light_power(light_t* l, float power);
API void    light_falloff(light_t* l, float constant, float linear, float quadratic);
API void    light_cone(light_t* l, float innerCone, float outerCone);
API void    light_update(unsigned num_lights, light_t *lv);

// scene

enum SCENE_FLAGS {
    SCENE_WIREFRAME = 1,
    SCENE_CULLFACE = 2,
    SCENE_BACKGROUND = 4,
    SCENE_FOREGROUND = 8,
    SCENE_UPDATE_SH_COEF = 16,
};

typedef struct scene_t {
    array(object_t) objs;
    array(light_t) lights;

    // special objects below:
    skybox_t skybox;
    int u_coefficients_sh;
} scene_t;

API scene_t*  scene_push();
API void      scene_pop();
API scene_t*  scene_get_active();

API int       scene_merge(const char *source);
API void      scene_render(int flags);

API object_t* scene_spawn();
API unsigned  scene_count();
API object_t* scene_index(unsigned index);

API light_t*  scene_spawn_light();
API unsigned  scene_count_light();
API light_t*  scene_index_light(unsigned index);
