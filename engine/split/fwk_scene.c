//
// @todo: remove explicit GL code from here

static camera_t *last_camera;

camera_t camera() {
    camera_t *old = last_camera;

    static camera_t cam = {0};
    do_once {
        cam.speed = 0.50f;
        cam.position = vec3(10,10,10);
        cam.updir = vec3(0,1,0);
        cam.fov = 45;
        cam.orthographic = false;
        cam.distance = 3; // len3(cam.position);

        cam.damping = false;
        cam.move_friction = 0.09f;
        cam.move_damping = 0.96f;
        cam.look_friction = 0.30f;
        cam.look_damping = 0.96f;
        cam.last_look = vec2(0,0);
        cam.last_move = vec3(0,0,0);

        // update proj & view
        camera_lookat(&cam,vec3(-5,0,-5));

        // @todo: remove this hack that is used to consolidate dampings
        if( 1 ) {
            vec3 zero = {0};
            for( int i = 0; i < 1000; ++i ) {
                camera_moveby(&cam, zero);
                camera_fps(&cam,0,0);
            }
        }
    }

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

void camera_moveby(camera_t *cam, vec3 inc) {
    // calculate camera damping
    if( cam->damping ) {
        float fr = cam->move_friction; fr *= fr; fr *= fr; fr *= fr;
        float sm = clampf(cam->move_damping, 0, 0.999f); sm *= sm; sm *= sm;

        cam->last_move = scale3(cam->last_move, 1 - fr);
        inc.x = cam->last_move.x = inc.x * (1 - sm) + cam->last_move.x * sm;
        inc.y = cam->last_move.y = inc.y * (1 - sm) + cam->last_move.y * sm;
        inc.z = cam->last_move.z = inc.z * (1 - sm) + cam->last_move.z * sm;
    }

    vec3 dir = norm3(cross3(cam->lookdir, cam->updir));
    cam->position = add3(cam->position, scale3(dir, inc.x)); // right
    cam->position = add3(cam->position, scale3(cam->updir, inc.y)); // up
    cam->position = add3(cam->position, scale3(cam->lookdir, inc.z)); // front

    camera_fps(cam, 0, 0);
}

void camera_teleport(camera_t *cam, vec3 pos) {
    bool damping = cam->damping;
    cam->damping = 0;
    cam->last_move = vec3(0,0,0);
    cam->position = pos;
    camera_fps(cam, 0, 0);
    cam->damping = damping;
}

void camera_lookat(camera_t *cam, vec3 target) {
    // invert expression that cam->lookdir = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
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
    last_camera = cam;

    float aspect = window_width() / ((float)window_height()+!window_height());

    cam->fov = fov;

    if( cam->orthographic ) {
        ortho44(cam->proj, -cam->fov * aspect, cam->fov * aspect, -cam->fov, cam->fov, 0.01f, 2000);
        // [ref] https://commons.wikimedia.org/wiki/File:Isometric_dimetric_camera_views.png
        // float pitch = cam->dimetric ? 30.000f : 35.264f; // dimetric or isometric
        // cam->pitch = -pitch; // quickly reorient towards origin
    } else {
        perspective44(cam->proj, cam->fov, aspect, 0.01f, 2000.f);
    }
}

void camera_fps(camera_t *cam, float yaw, float pitch) {
    last_camera = cam;

    // camera damping
    if( cam->damping ) {
        float fr = cam->look_friction; fr *= fr; fr *= fr; fr *= fr;
        float sm = clampf(cam->look_damping, 0, 0.999f); sm *= sm; sm *= sm;

        cam->last_look = scale2(cam->last_look, 1 - fr);
        yaw = cam->last_look.y = yaw * (1 - sm) + cam->last_look.y * sm;
        pitch = cam->last_look.x = pitch * (1 - sm) + cam->last_look.x * sm;
    }

    cam->yaw += yaw;
    cam->yaw = fmod(cam->yaw, 360);
    cam->pitch += pitch;
    cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;

    const float deg2rad = 0.0174532f, y = cam->yaw * deg2rad, p = cam->pitch * deg2rad;
    cam->lookdir = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
    lookat44(cam->view, cam->position, add3(cam->position, cam->lookdir), cam->updir); // eye,center,up

    camera_fov(cam, cam->fov);
}

void camera_orbit( camera_t *cam, float yaw, float pitch, float inc_distance ) {
    last_camera = cam;

    // update dummy state
    camera_fps(cam, 0,0);

    // @todo: add damping
    vec3 _mouse = vec3(yaw, pitch, inc_distance);
    cam->yaw += _mouse.x;
    cam->pitch += _mouse.y;
    cam->distance += _mouse.z;

    // look: limit pitch angle [-89..89]
    cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;

    // compute view matrix
    float x = rad(cam->yaw), y = rad(-cam->pitch), cx = cosf(x), cy = cosf(y), sx = sinf(x), sy = sinf(y);
    lookat44(cam->view, vec3( cx*cy*cam->distance, sy*cam->distance, sx*cy*cam->distance ), vec3(0,0,0), vec3(0,1,0) );

    // save for next call
    cam->last_move.x = _mouse.x;
    cam->last_move.y = _mouse.y;
}

int ui_camera( camera_t *cam ) {
    int changed = 0;
    changed |= ui_bool("Orthographic", &cam->orthographic);
    changed |= ui_bool("Damping", &cam->damping);
    if( !cam->damping ) ui_disable();
    changed |= ui_slider2("Move friction", &cam->move_friction, va("%5.3f", cam->move_friction));
    changed |= ui_slider2("Move damping", &cam->move_damping, va("%5.3f", cam->move_damping));
    changed |= ui_slider2("View friction", &cam->look_friction, va("%5.3f", cam->look_friction));
    changed |= ui_slider2("View damping", &cam->look_damping, va("%5.3f", cam->look_damping));
    if( !cam->damping ) ui_enable();
    ui_separator();
    changed |= ui_float("Speed", &cam->speed);
    changed |= ui_float3("Position", cam->position.v3);
    changed |= ui_float3("LookDir", cam->lookdir.v3);
    changed |= ui_float3("UpDir", cam->updir.v3);
    ui_disable();
    changed |= ui_mat44("View matrix", cam->view);
    ui_enable();
    ui_separator();
    changed |= ui_float("FOV (degrees)", &cam->fov);
    changed |= ui_float("Orbit distance", &cam->distance);
    ui_disable();
    changed |= ui_mat44("Projection matrix", cam->proj);
    ui_enable();
    return changed;
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

light_t light() {
    light_t l = {0};
    l.diffuse = vec3(1,1,1);
    l.dir = vec3(1,-1,-1);
    l.falloff.constant = 1.0f;
    l.falloff.linear = 0.09f;
    l.falloff.quadratic = 0.0032f;
    l.specularPower = 32.f;
    l.innerCone = 0.9f; // 25 deg
    l.outerCone = 0.85f; // 31 deg

    return l;
}

void light_type(light_t* l, char type) {
    l->cached = 0;
    l->type = type;
}

void light_diffuse(light_t* l, vec3 color) {
    l->cached = 0;
    l->diffuse = color;
}

void light_specular(light_t* l, vec3 color) {
    l->cached = 0;
    l->specular = color;
}

void light_ambient(light_t* l, vec3 color) {
    l->cached = 0;
    l->ambient = color;
}

void light_teleport(light_t* l, vec3 pos) {
    l->cached = 0;
    l->pos = pos;
}

void light_dir(light_t* l, vec3 dir) {
    l->cached = 0;
    l->dir = dir;
}

void light_power(light_t* l, float power) {
    l->cached = 0;
    l->specularPower = power;
}

void light_falloff(light_t* l, float constant, float linear, float quadratic) {
    l->cached = 0;
    l->falloff.constant = constant;
    l->falloff.linear = linear;
    l->falloff.quadratic = quadratic;
}

void light_cone(light_t* l, float innerCone, float outerCone) {
    l->cached = 0;
    l->innerCone = acos(innerCone);
    l->outerCone = acos(outerCone);
}

void light_update(unsigned num_lights, light_t *lv) {
    shader_int("u_num_lights", num_lights);

    for (unsigned i=0; i < num_lights; ++i) {
        lv[i].cached = 1;
        shader_int(va("u_lights[%d].type", i), lv[i].type);
        shader_vec3(va("u_lights[%d].pos", i), lv[i].pos);
        shader_vec3(va("u_lights[%d].dir", i), lv[i].dir);
        shader_vec3(va("u_lights[%d].diffuse", i), lv[i].diffuse);
        shader_vec3(va("u_lights[%d].specular", i), lv[i].specular);
        shader_vec3(va("u_lights[%d].ambient", i), lv[i].ambient);
        shader_float(va("u_lights[%d].power", i), lv[i].specularPower);
        shader_float(va("u_lights[%d].constant", i), lv[i].falloff.constant);
        shader_float(va("u_lights[%d].linear", i), lv[i].falloff.linear);
        shader_float(va("u_lights[%d].quadratic", i), lv[i].falloff.quadratic);
        shader_float(va("u_lights[%d].innerCone", i), lv[i].innerCone);
        shader_float(va("u_lights[%d].outerCone", i), lv[i].outerCone);
    }
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

light_t* scene_spawn_light() {
    light_t l = light();
    array_push(last_scene->lights, l);

    return array_back(last_scene->lights);
}

unsigned scene_count_light() {
    return array_count(last_scene->lights);
}

light_t* scene_index_light(unsigned light_index) {
    unsigned light_count = scene_count_light();
    ASSERT(light_index < light_count, "Light index %d exceeds number (%d) of spawned lights", light_index, light_count);
    return &last_scene->lights[light_index];
}


void scene_render(int flags) {
    camera_t *cam = camera_get_active();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glActiveTexture(GL_TEXTURE0);

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

    // @fixme: CW ok for one-sided rendering. CCW ok for FXs. we need both
    (flags & SCENE_CULLFACE ? glEnable : glDisable)(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
    glPolygonMode( GL_FRONT_AND_BACK, flags & SCENE_WIREFRAME ? GL_LINE : GL_FILL );
    // @todo alpha mode
    // @todo texture mode

    if( flags & SCENE_FOREGROUND ) {
        bool do_relighting = 0;
        for (unsigned j = 0; j < array_count(last_scene->lights); ++j) {
            if (!last_scene->lights[j].cached) {
                do_relighting = 1;
                break;
            }
        }

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

            if ( do_relighting || !obj->light_cached ) {
                obj->light_cached = 1;
                shader_bind(model->program);
                light_update(array_count(last_scene->lights), last_scene->lights);
            }

            if ( flags&SCENE_UPDATE_SH_COEF ) {
                shader_bind(model->program);
                shader_vec3v("u_coefficients_sh", 9, last_scene->skybox.cubemap.sh);
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
