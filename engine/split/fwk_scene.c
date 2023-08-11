//
// @todo: remove explicit GL code from here

static camera_t *last_camera;

camera_t camera() {
    camera_t *old = last_camera;

    camera_t cam = {0};
    cam.speed = 1;
    cam.position = vec3(10,10,10);
    cam.last_look = cam.last_move = vec3(0,0,0);
    cam.up = vec3(0,1,0);
    cam.fov = 45;

    // @todo: remove this hack
    static int smoothing = -1; if( smoothing < 0 ) smoothing = flag("--camera-smooth");
    if( smoothing ) {
        for( int i = 0; i < 1000; ++i ) {
            camera_move(&cam,0,0,0);
            camera_fps(&cam,0,0);
        }
    }

    // update proj & view
    camera_lookat(&cam,vec3(-5,0,-5));

    last_camera = old;
    *camera_get_active() = cam;
    return cam;
}

camera_t *camera_get_active() {
    static camera_t defaults = {0};
    if( !last_camera ) {
        identity44(defaults.view);
        identity44(defaults.proj);
        last_camera = &defaults;
    }
    return last_camera;
}

void camera_move(camera_t *cam, float incx, float incy, float incz) {
    // enable camera smoothing
    static int smoothing = -1; if( smoothing < 0 ) smoothing = flag("--camera-smooth");
    if( smoothing ) {
        float move_friction = 0.99f;
        cam->last_move = scale3(cam->last_move, move_friction);
        float move_filtering = 0.975f;
        incx = cam->last_move.x = incx * (1 - move_filtering) + cam->last_move.x * move_filtering;
        incy = cam->last_move.y = incy * (1 - move_filtering) + cam->last_move.y * move_filtering;
        incz = cam->last_move.z = incz * (1 - move_filtering) + cam->last_move.z * move_filtering;
    }

    vec3 dir = norm3(cross3(cam->look, cam->up));
    cam->position = add3(cam->position, scale3(dir, incx)); // right
    cam->position = add3(cam->position, scale3(cam->up, incy)); // up
    cam->position = add3(cam->position, scale3(cam->look, incz)); // front

    camera_fps(cam, 0, 0);
}

void camera_teleport(camera_t *cam, vec3 pos) {
    cam->position = pos;
    camera_fps(cam, 0, 0);
}

void camera_lookat(camera_t *cam, vec3 target) {
    // invert expression that cam->look = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
    // look.y = sin p > y = asin(p)
    // look.x = cos y * cos p; -> cos p = look.x / cos y \ look.x / cos y = look.z / sin y
    // look.z = sin y * cos p; -> cos p = look.z / sin y /
    // so, sin y / cos y = look x / look z > tan y = look x / look z > y = atan(look x / look z)

    vec3 look = norm3(sub3(target, cam->position));
    const float rad2deg = 1 / 0.0174532f;
    float pitch = asin(look.y) * rad2deg;
    float yaw = atan2(look.z, look.x) * rad2deg; // coords swapped. it was (look.x, look.z) before. @todo: testme

    camera_fps(cam, yaw-cam->yaw, pitch-cam->pitch);
}

void camera_enable(camera_t *cam) {
    // camera_t *other = camera_get_active(); // init default camera in case there is none
    last_camera = cam;
    // trigger a dummy update -> update matrices
    camera_fps(cam, 0, 0);
}

void camera_fov(camera_t *cam, float fov) {
    cam->fov = fov;
}

void camera_fps(camera_t *cam, float yaw, float pitch) {
    last_camera = cam;

    // enable camera smoothing
    static int smoothing = -1; if( smoothing < 0 ) smoothing = flag("--camera-smooth");
    if( smoothing ) {
        float look_friction = 0.999f;
        cam->last_look.x *= look_friction;
        cam->last_look.y *= look_friction;
        float look_filtering = 0.05f;
        yaw = cam->last_look.y = yaw * look_filtering + cam->last_look.y * (1 - look_filtering);
        pitch = cam->last_look.x = pitch * look_filtering + cam->last_look.x * (1 - look_filtering);
    }

    cam->yaw += yaw;
    cam->yaw = fmod(cam->yaw, 360);
    cam->pitch += pitch;
    cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;

    const float deg2rad = 0.0174532f, y = cam->yaw * deg2rad, p = cam->pitch * deg2rad;
    cam->look = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));

    lookat44(cam->view, cam->position, add3(cam->position, cam->look), cam->up); // eye,center,up
    perspective44(cam->proj, cam->fov, window_width() / ((float)window_height()+!window_height()), 0.01f, 1000.f);

#if 0 // isometric/dimetric
    #define orthogonal(proj, fov, aspect, znear, zfar) \
    ortho44((proj), -(fov) * (aspect), (fov) * (aspect), -(fov), (fov), (znear), (zfar))

    float DIMETRIC = 30.000f;
    float ISOMETRIC = 35.264f;
    float aspect = window_width() / ((float)window_height()+!!window_height());
    orthogonal(cam->proj, 45, aspect, -1000, 1000); // why -1000?
    // cam->yaw = 45;
    cam->pitch = -ISOMETRIC;
#endif
}

void camera_orbit( camera_t *cam, float yaw, float pitch, float inc_distance ) {
    last_camera = cam;

    vec2 inc_mouse = vec2(yaw, pitch);

    // @todo: worth moving all these members into camera_t ?
    static vec2 _mouse = {0,0};
    static vec2 _polarity = { +1,-1 };
    static vec2 _sensitivity = { 2,2 };
    static float _friction = 0.75; //99;
    static float _distance; do_once _distance = len3(cam->position);

    // update dummy state
    camera_fps(cam, 0,0);

    // add smooth input
    _mouse = mix2(_mouse, add2(_mouse, mul2(mul2(inc_mouse,_sensitivity),_polarity)), _friction);
    _distance = mixf(_distance, _distance+inc_distance, _friction);

    // look: update angles
    vec2 offset = sub2( _mouse, ptr2(&cam->last_move.x) );
    if( 1 ) { // if _enabled
        cam->yaw += offset.x;
        cam->pitch += offset.y;
        // look: limit pitch angle [-89..89]
        cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;
    }

    // compute view matrix
    float x = rad(cam->yaw), y = rad(cam->pitch), cx = cosf(x), cy = cosf(y), sx = sinf(x), sy = sinf(y);
    lookat44(cam->view, vec3( cx*cy*_distance, sy*_distance, sx*cy*_distance ), vec3(0,0,0), vec3(0,1,0) );

    // save for next call
    cam->last_move.x = _mouse.x;
    cam->last_move.y = _mouse.y;
}

// -----------------------------------------------------------------------------

static
void object_update(object_t *obj) {
    quat p = eulerq(vec3(obj->pivot.x,obj->pivot.y,obj->pivot.z));
    quat e = eulerq(vec3(obj->euler.x,obj->euler.y,obj->euler.z));
    compose44(obj->transform, obj->pos, mulq(e, p), obj->sca);
}

object_t object() {
    object_t obj = {0};
    identity44(obj.transform);
    //obj.rot = idq();
    obj.sca = vec3(1,1,1);
    //obj.bounds = aabb(vec3(0,0,0),vec3(1,1,1)); // defaults to small 1-unit cube
object_rotate(&obj, vec3(0,0,0));
    //array_init(obj.textures);
    return obj;
}

void object_pivot(object_t *obj, vec3 euler) {
    obj->pivot = euler;
    object_update(obj);
}

void object_rotate(object_t *obj, vec3 euler) {
    quat p = eulerq(vec3(obj->pivot.x,obj->pivot.y,obj->pivot.z));
    quat e = eulerq(vec3(euler.x,euler.y,euler.z));
    obj->rot = mulq(p,e);
    obj->euler = euler;
    object_update(obj);
}

void object_teleport(object_t *obj, vec3 pos) {
    obj->pos = pos;
    object_update(obj);
}

void object_move(object_t *obj, vec3 inc) {
    obj->pos = add3(obj->pos, inc);
    object_update(obj);
}

void object_scale(object_t *obj, vec3 sca) {
    obj->sca = vec3(sca.x, sca.y, sca.z);
    object_update(obj);
}

vec3 object_position(object_t *obj) {
    return vec3(obj->transform[12], obj->transform[13], obj->transform[14]);
}

void object_model(object_t *obj, model_t model) {
    obj->model = model;
}

void object_push_diffuse(object_t *obj, texture_t tex) {
    array_push(obj->textures, tex.id);
}

void object_pop_diffuse(object_t *obj) {
    array_pop(obj->textures);
}

void object_diffuse(object_t *obj, texture_t tex) {
    array_clear(obj->textures);
    object_push_diffuse(obj, tex);
}

void object_billboard(object_t *obj, unsigned mode) {
    obj->billboard = mode;
}

// -----------------------------------------------------------------------------

array(scene_t*) scenes;
scene_t* last_scene;

static void scene_init() {
#ifndef __EMSCRIPTEN__ // @fixme ems -> shaders
    do_once scene_push();      // create an empty scene by default
#endif
}

scene_t* scene_get_active() {
    return last_scene;
}

scene_t* scene_push() {
    scene_t *s = REALLOC(0, sizeof(scene_t)), clear = {0}; *s = clear;
    const char *symbols[] = { "{{include-shadowmap}}", fs_0_0_shadowmap_lit };
    s->program = shader(strlerp(1, symbols, vs_332_32), strlerp(1, symbols, fs_32_4_model), "att_position,att_normal,att_texcoord,att_color", "fragcolor");
    s->skybox = skybox(NULL, 0);
    array_push(scenes, s);
    last_scene = s;
    return s;
}

void scene_pop() {
    // @fixme: fix leaks, scene_cleanup();
    scene_t clear = {0};
    *last_scene = clear;
    array_pop(scenes);
    last_scene = *array_back(scenes);
}

int scene_merge(const char *source) {
    int count = 0;
    if( json_push(source) ) {
        for(int i = 0, e = json_count("/") - 1; i <= e ; ++i) {
            const char *skybox_folder = json_string("/[%d]/skybox",i);
            if( skybox_folder[0] ) {
                PRINTF("Loading skybox folder: %s\n", skybox_folder);
                last_scene->skybox = skybox( skybox_folder, 0 );
                continue;
            }
            const char *mesh_file = json_string("/[%d]/mesh",i);
            const char *texture_file = json_string("/[%d]/texture",i);
            const char *animation_file = json_string("/[%d]/animation",i);
            vec3 position = vec3(json_float("/[%d]/position[0]",i),json_float("/[%d]/position[1]",i),json_float("/[%d]/position[2]",i));
            vec3 rotation = vec3(json_float("/[%d]/rotation[0]",i),json_float("/[%d]/rotation[1]",i),json_float("/[%d]/rotation[2]",i));
            vec3 scale = scale3(vec3(1,1,1), json_float("/[%d]/scale",i));
            bool opt_swap_zy = json_int("/[%d]/swapzy",i);
            bool opt_flip_uv = json_int("/[%d]/flipuv",i);
            PRINTF("Scene %d/%d Loading: %s\n", i, e, mesh_file);
            PRINTF("Scene %d/%d Texture: %s\n", i, e, texture_file);
            PRINTF("Scene %d/%d Animation: %s\n", i, e, animation_file);
            PRINTF("Scene %d/%d Position: (%f,%f,%f)\n", i, e, position.x, position.y, position.z);
            PRINTF("Scene %d/%d Rotation: (%f,%f,%f)\n", i, e, rotation.x, rotation.y, rotation.z);
            PRINTF("Scene %d/%d Scale: (%f,%f,%f)\n", i, e, scale.x, scale.y, scale.z);
            PRINTF("Scene %d/%d Swap_ZY: %d\n", i, e, opt_swap_zy );
            PRINTF("Scene %d/%d Flip_UV: %d\n", i, e, opt_flip_uv );
            model_t m = model_from_mem(vfs_read(mesh_file), vfs_size(mesh_file), 0/*opt_swap_zy*/);
            //char *a = archive_read(animation_file);
            object_t *o = scene_spawn();
            object_model(o, m);
            if( texture_file[0] ) object_diffuse(o, texture_from_mem(vfs_read(texture_file), vfs_size(texture_file), opt_flip_uv ? IMAGE_FLIP : 0) );
            object_scale(o, scale);
            object_teleport(o, position);
            object_pivot(o, rotation); // object_rotate(o, rotation);
            //object_name(x), scene_find(name)
// o->bounds = aabb(mul3(m.bounds.min,o->sca),mul3(m.bounds.max,o->sca));
// PRINTF("aabb={%f,%f,%f},{%f,%f,%f}\n", o->bounds.min.x, o->bounds.min.y, o->bounds.min.z, o->bounds.max.x, o->bounds.max.y, o->bounds.max.z);

/*
            if(opt_swap_zy) {
                // swap zy bounds
                vec3 min = o->bounds.min, max = o->bounds.max;
                o->bounds = aabb( vec3(min.x,min.z,min.y), vec3(max.x,max.z,max.y) );
            }
*/

            count++;
        }
        json_pop();
    }
    // PRINTF("scene loading took %5.2fs\n", secs);
    PRINTF("%d objects merged into scene\n", count);
    return count;
}

object_t* scene_spawn() {
    object_t obj = object();
    array_push(last_scene->objs, obj);

    return array_back(last_scene->objs);
}

unsigned scene_count() {
    return array_count(last_scene->objs);
}

object_t* scene_index(unsigned obj_index) {
    unsigned obj_count = scene_count();
    ASSERT(obj_index < obj_count, "Object index %d exceeds number (%d) of spawned objects", obj_index, obj_count);
    return &last_scene->objs[obj_index];
}

void scene_render(int flags) {
    camera_t *cam = camera_get_active();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(last_scene->program);

    if(flags & SCENE_BACKGROUND) {
        if(last_scene->skybox.program) {
        skybox_push_state(&last_scene->skybox, cam->proj, cam->view);

        glDisable(GL_DEPTH_TEST);
    //  glDepthFunc(GL_LESS);
    //    glActiveTexture(GL_TEXTURE0);
    //    (flags & SCENE_CULLFACE ? glEnable : glDisable)(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
    //    glPolygonMode( GL_FRONT_AND_BACK, flags & SCENE_WIREFRAME ? GL_LINE : GL_FILL );

        mesh_render(&last_scene->skybox.geometry);
        skybox_pop_state();
        }

        ddraw_flush();
    }

    glDepthFunc(GL_LESS);
    glActiveTexture(GL_TEXTURE0);
//  glUseProgram(last_scene->program);

    // @fixme: CW ok for one-sided rendering. CCW ok for FXs. we need both
    (flags & SCENE_CULLFACE ? glEnable : glDisable)(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
    glPolygonMode( GL_FRONT_AND_BACK, flags & SCENE_WIREFRAME ? GL_LINE : GL_FILL );
    // @todo alpha mode
    // @todo texture mode

    if( flags & SCENE_FOREGROUND ) {
        for(unsigned j = 0, obj_count = scene_count(); j < obj_count; ++j ) {
            object_t *obj = scene_index(j);
            model_t *model = &obj->model;
            mat44 *views = (mat44*)(&cam->view);

            // @todo: avoid heap allocs here?
            static array(handle) old_textures = 0;

            int do_retexturing = model->iqm && array_count(obj->textures) > 0;
            if( do_retexturing ) {
                for(int i = 0; i < model->iqm->nummeshes; ++i) {
                    array_push(old_textures, model->iqm->textures[i]);
                    model->iqm->textures[i] = *array_back(obj->textures);
                }
            }

            model->billboard = obj->billboard;
            model_render(*model, cam->proj, cam->view, obj->transform, 0);

            if( do_retexturing ) {
                for(int i = 0; i < model->iqm->nummeshes; ++i) {
                    model->iqm->textures[i] = old_textures[i];
                }
                array_resize(old_textures, 0);
            }
        }
        glBindVertexArray(0);
    }

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}
