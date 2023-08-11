// -----------------------------------------------------------------------------
// scene framework
// - rlyeh, public domain

// camera

typedef struct camera_t {
    mat44 view, proj;
    vec3 position, up, look; // position, updir, lookdir
    float yaw, pitch, speed; // mirror_x, mirror_y;
    vec3 last_look, last_move; // used for friction and smoothing
    float fov; // deg(45)
} camera_t;

API camera_t camera();
API void camera_teleport(camera_t *cam, vec3 pos);
API void camera_move(camera_t *cam, float incx, float incy, float incz);
API void camera_fov(camera_t *cam, float fov);
API void camera_fps(camera_t *cam, float yaw, float pitch);
API void camera_orbit(camera_t *cam, float yaw, float pitch, float inc_distance);
API void camera_lookat(camera_t *cam, vec3 target);
API void camera_enable(camera_t *cam);
API camera_t *camera_get_active();

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

// scene

enum SCENE_FLAGS {
    SCENE_WIREFRAME = 1,
    SCENE_CULLFACE = 2,
    SCENE_BACKGROUND = 4,
    SCENE_FOREGROUND = 8,
};

typedef struct scene_t {
    handle program;

    array(object_t) objs;

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
