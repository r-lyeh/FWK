// -----------------------------------------------------------------------------
// scene framework
// - rlyeh, public domain

// camera

typedef struct camera_t {
    mat44 view, proj;
    vec3 position, updir, lookdir, rightdir;
    float yaw, pitch, roll; // mirror of (x,y) lookdir in deg;
    float speed, accel, fov; // fov in deg(45)
    float near_clip, far_clip;
    float frustum_fov_multiplier;

    float move_friction, move_damping;
    float look_friction, look_damping;
    vec3 last_look; vec3 last_move; // used for friction and damping
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
API void camera_fps2(camera_t *cam, float yaw, float pitch, float roll);
API void camera_orbit(camera_t *cam, float yaw, float pitch, float inc_distance);
API void camera_lookat(camera_t *cam, vec3 target);
API void camera_enable(camera_t *cam);
API void camera_freefly(camera_t *cam);
API frustum camera_frustum_build(camera_t *cam);
API camera_t *camera_get_active();

API int  ui_camera(camera_t *cam);
API void ddraw_camera(camera_t *cam);

// object

typedef struct object_t {
    uint64_t renderbucket;
    mat44 transform;
    quat rot;
    vec3 sca, pos, euler, pivot;
    array(texture_t) textures;
    model_t model;
    anim_t anim;
    float anim_speed;
    aabb bounds;
    unsigned billboard; // [0..7] x(4),y(2),z(1) masks
    bool disable_frustum_check;
    bool cast_shadows;
    bool fullbright;
    bool batchable;

    // internal states
    array(handle) old_texture_ids;
    array(texture_t) old_textures;
    float distance;
    bool skip_draw;
    bool light_cached; //< used by scene to update light data
    bool was_batched;
    array(mat44) instances;
    array(unsigned) pair_instance;
    uint32_t checksum;
} object_t;

API object_t object();
API bool object_compare(object_t *obj1, object_t *obj2);
API void object_rotate(object_t *obj, vec3 euler);
API void object_pivot(object_t *obj, vec3 euler);
API void object_teleport(object_t *obj, vec3 pos);
API void object_move(object_t *obj, vec3 inc);
API vec3 object_position(object_t *obj);
API void object_scale(object_t *obj, vec3 sca);
API void object_batchable(object_t *obj, bool batchable);
//
API void object_model(object_t *obj, model_t model);
API void object_anim(object_t *obj, anim_t anim, float speed);
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
    SCENE_UPDATE_SH_COEF = 16,
    SCENE_CAST_SHADOWS = 32,
    // SCENE_DISABLE_BATCHING = 64,
};

typedef struct scene_t {
    array(object_t) objs;
    array(light_t) lights;

    // special objects below:
    skybox_t skybox;
    int u_coefficients_sh;
    shadowmap_t shadowmap;
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
API void      scene_merge_lights(const char *source);
API unsigned  scene_count_light();
API light_t*  scene_index_light(unsigned index);
API void      scene_skybox(skybox_t sky);
