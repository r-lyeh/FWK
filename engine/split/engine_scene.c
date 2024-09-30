//
// @todo: remove explicit GL code from here

static camera_t *last_camera;

camera_t camera() {
    camera_t *old = last_camera;

    static camera_t cam = {0};
    do_once {
        cam.speed = 0.50f;
        cam.accel = 0.5f;
        cam.position = vec3(10,10,10);
        cam.updir = vec3(0,1,0);
        cam.rightdir = vec3(1,0,0);
        cam.fov = 45;
        cam.aspect = 0.0f;
        cam.frustum_fov_multiplier = 1.0f;
        cam.orthographic = false;
        cam.distance = 3; // len3(cam.position);
        cam.near_clip = 0.1f;
        cam.far_clip = 1000.f;

        cam.damping = false;
        cam.move_friction = 0.09f;
        cam.move_damping = 0.96f;
        cam.look_friction = 0.30f;
        cam.look_damping = 0.96f;
        cam.last_look = vec3(0,0,0);
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

    if (!last_camera)
        *camera_get_active() = cam;
    last_camera = old;
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

void camera_freefly(camera_t *cam) {
    // static bool is_active = true;
    // if (input_down(KEY_HOME)) is_active = !is_active;
    // if (!is_active) return;
    bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
    window_cursor( !active );
    int mult_speed = input(KEY_LSHIFT) || input(KEY_LALT);

    static float speed_buildup = 1.0f;
    if( active ) cam->speed = clampf(cam->speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
    vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
    vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam->speed);
    if ( len3sq(wasdecq) ) speed_buildup += (cam->speed * cam->accel * (2.0f * mult_speed + 1.0f) * window_delta());
    // if (!active) speed_buildup = 1.0f;
    else speed_buildup = 1.0f;
    camera_moveby(cam, scale3(wasdecq, window_delta() * 60 * speed_buildup * (2.0f * mult_speed + 1.0f)));
    camera_fps(cam, mouse.x,mouse.y);
}

frustum camera_frustum_build(camera_t *cam) {
    float aspect = cam->aspect ? cam->aspect : window_width() / ((float)window_height()+!window_height());
    float fov = cam->fov * cam->frustum_fov_multiplier;
    mat44 proj;
    if( cam->orthographic ) {
        ortho44(proj, -fov * aspect, fov * aspect, -fov, fov, cam->near_clip, cam->far_clip);
    } else {
        perspective44(proj, fov, aspect, cam->near_clip, cam->far_clip);
    }
    mat44 projview; multiply44x2(projview, proj, cam->view);
    return frustum_build(projview);
}

void camera_fov(camera_t *cam, float fov) {
    last_camera = cam;

    float aspect = cam->aspect ? cam->aspect : window_width() / ((float)window_height()+!window_height());

    cam->fov = fov;

    if( cam->orthographic ) {
        ortho44(cam->proj, -cam->fov * aspect, cam->fov * aspect, -cam->fov, cam->fov, cam->near_clip, cam->far_clip);
        // [ref] https://commons.wikimedia.org/wiki/File:Isometric_dimetric_camera_views.png
        // float pitch = cam->dimetric ? 30.000f : 35.264f; // dimetric or isometric
        // cam->pitch = -pitch; // quickly reorient towards origin
    } else {
        perspective44(cam->proj, cam->fov, aspect, cam->near_clip, cam->far_clip);
    }
}

void camera_fps2(camera_t *cam, float yaw, float pitch, float roll) {
    last_camera = cam;

    // camera damping
    if( cam->damping ) {
        float fr = cam->look_friction; fr *= fr; fr *= fr; fr *= fr;
        float sm = clampf(cam->look_damping, 0, 0.999f); sm *= sm; sm *= sm;

        cam->last_look = scale3(cam->last_look, 1 - fr);
        yaw = cam->last_look.y = yaw * (1 - sm) + cam->last_look.y * sm;
        pitch = cam->last_look.x = pitch * (1 - sm) + cam->last_look.x * sm;
        roll = cam->last_look.z = roll * (1 - sm) + cam->last_look.z * sm;
    }

    cam->yaw += yaw;
    cam->yaw = fmod(cam->yaw, 360);
    cam->pitch += pitch;
    cam->pitch = cam->pitch > 89 ? 89 : cam->pitch < -89 ? -89 : cam->pitch;
    cam->roll += roll;
    cam->roll += fmod(cam->roll, 360);

    const float deg2rad = 0.0174532f, y = cam->yaw * deg2rad, p = cam->pitch * deg2rad, r = cam->roll * deg2rad;
    cam->lookdir = norm3(vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
    vec3 up = vec3(0,1,0);
    // calculate updir
    {
        float cosfa = cosf(r);
        float sinfa = sinf(r);
        vec3 right = cross3(cam->lookdir, up);
        cam->rightdir = right;
        float th = dot3(cam->lookdir, up);

        cam->updir.x = up.x * cosfa + right.x * sinfa + cam->lookdir.x * th * (1.0f - cosfa);
        cam->updir.y = up.y * cosfa + right.y * sinfa + cam->lookdir.y * th * (1.0f - cosfa);
        cam->updir.z = up.z * cosfa + right.z * sinfa + cam->lookdir.z * th * (1.0f - cosfa);
    }

    lookat44(cam->view, cam->position, add3(cam->position, cam->lookdir), cam->updir); // eye,center,up

    camera_fov(cam, cam->fov);
}

void camera_fps(camera_t *cam, float yaw, float pitch) {
    camera_fps2(cam, yaw, pitch, 0.0f);
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
    obj.cast_shadows = true;
    obj.batchable = true;
    return obj;
}

bool object_compare(object_t *obj1, object_t *obj2) {
    // if (obj1->renderbucket != obj2->renderbucket) return false;
    // if (memcmp(obj1->transform, obj2->transform, sizeof(mat44))) return false;
    // if (memcmp(&obj1->rot, &obj2->rot, sizeof(quat))) return false;
    // if (memcmp(&obj1->sca, &obj2->sca, sizeof(vec3))) return false;
    // if (memcmp(&obj1->pos, &obj2->pos, sizeof(vec3))) return false;
    // if (memcmp(&obj1->euler, &obj2->euler, sizeof(vec3))) return false;
    // if (memcmp(&obj1->pivot, &obj2->pivot, sizeof(vec3))) return false;
    // if (memcmp(&obj1->bounds, &obj2->bounds, sizeof(aabb))) return false;
    if (memcmp(&obj1->anim, &obj2->anim, sizeof(anim_t))) return false;
    // if (obj1->anim_speed != obj2->anim_speed) return false;
    if (obj1->billboard != obj2->billboard) return false;
    if (obj1->fullbright != obj2->fullbright) return false;
    if (obj1->checksum != obj2->checksum) return false;
    return true;
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

void object_batchable(object_t *obj, bool batchable) {
    obj->batchable = batchable;
}

vec3 object_position(object_t *obj) {
    return vec3(obj->transform[12], obj->transform[13], obj->transform[14]);
}

void object_model(object_t *obj, model_t model) {
    obj->model = model;
}

void object_model_shadow(object_t *obj, model_t model) {
    obj->model_shadow = model;
}

void object_anim(object_t *obj, anim_t anim, float speed) {
    obj->anim = anim;
    obj->anim_speed = speed;
}

void object_push_diffuse(object_t *obj, texture_t tex) {
    array_push(obj->textures, tex);
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
            bool opt_fullbright = json_int("/[%d]/fullbright",i);
            bool enable_pbr = json_int("/[%d]/pbr",i);
            PRINTF("Scene %d/%d Loading: %s\n", i, e, mesh_file);
            PRINTF("Scene %d/%d Texture: %s\n", i, e, texture_file);
            PRINTF("Scene %d/%d Animation: %s\n", i, e, animation_file);
            PRINTF("Scene %d/%d Position: (%f,%f,%f)\n", i, e, position.x, position.y, position.z);
            PRINTF("Scene %d/%d Rotation: (%f,%f,%f)\n", i, e, rotation.x, rotation.y, rotation.z);
            PRINTF("Scene %d/%d Scale: (%f,%f,%f)\n", i, e, scale.x, scale.y, scale.z);
            PRINTF("Scene %d/%d Swap_ZY: %d\n", i, e, opt_swap_zy );
            PRINTF("Scene %d/%d Flip_UV: %d\n", i, e, opt_flip_uv );
            model_t m = model_from_mem(vfs_read(mesh_file), vfs_size(mesh_file), enable_pbr ? 0 : MODEL_NO_PBR);
            //char *a = archive_read(animation_file);
            object_t *o = scene_spawn();
            object_model(o, m);
            if( texture_file[0] ) object_diffuse(o, texture_from_mem(vfs_read(texture_file), vfs_size(texture_file), (opt_flip_uv ? IMAGE_FLIP : 0)) );
            object_scale(o, scale);
            object_teleport(o, position);
            object_pivot(o, rotation); // object_rotate(o, rotation);
            o->fullbright = opt_fullbright;
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

void scene_merge_lights(const char *source) {
    light_t *lights = lightlist(source);
    for (unsigned i = 0; i < array_count(lights); ++i) {
        light_t *l = &lights[i];
        array_push(last_scene->lights, *l);
    }
    array_free(lights);
}

unsigned scene_count_light() {
    return array_count(last_scene->lights);
}

light_t* scene_index_light(unsigned light_index) {
    unsigned light_count = scene_count_light();
    ASSERT(light_index < light_count, "Light index %d exceeds number (%d) of spawned lights", light_index, light_count);
    return &last_scene->lights[light_index];
}

void scene_skybox(skybox_t sky) {
    skybox_destroy(&last_scene->skybox);
    last_scene->skybox = sky;
}

static
int scene_obj_distance_compare(const void *a, const void *b) {
    const object_t *da = a, *db = b;
    return da->distance < db->distance ? 1 : da->distance > db->distance ? -1 : 0;
}

void scene_render(int flags) {
    camera_t *cam = camera_get_active();

    shadowmap_t *sm = &last_scene->shadowmap;

    if (flags & SCENE_POSTFX) {
        fx_begin();
    }

    if (flags & SCENE_SHADOWS) {
        if (sm->vsm_texture_width == 0) {
            *sm = shadowmap(DEFAULT_SHADOW_VSM_RESOLUTION, DEFAULT_SHADOW_CSM_RESOLUTION);
        }
    } else {
        sm = NULL;
    }

    if(flags & SCENE_BACKGROUND) {
        if(last_scene->skybox.program) {
            skybox_push_state(&last_scene->skybox, cam->proj, cam->view);
            mesh_render(&last_scene->skybox.geometry);
            skybox_pop_state();
        }

        ddraw_flush();
    }

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
            obj->was_batched = false;
            // array_resize(obj->pair_instance, obj_count);

            model->billboard = obj->billboard;
            for (int p = 0; p < RENDER_PASS_OVERRIDES_BEGIN; ++p) {
                model->rs[p].cull_face_enabled = flags&SCENE_CULLFACE ? 1 : 0;
                model->rs[p].polygon_mode_draw = flags&SCENE_WIREFRAME ? GL_LINE : GL_FILL;
            }
            obj->checksum = obj->model.iqm ? model_checksum(&obj->model) : 0;
        }

        for(unsigned j = 0, obj_count = scene_count(); j < obj_count; ++j ) {
            object_t *obj = scene_index(j);
            model_t *model = &obj->model;
            anim_t *anim = &obj->anim;
            mat44 *views = (mat44*)(&cam->view);

            if (obj->skip_draw) continue;
            if (obj->was_batched) continue;

            if (anim) {
                float delta = window_delta() * obj->anim_speed;
                model->curframe = model_animate_clip(*model, model->curframe + delta, anim->from, anim->to, anim->flags & ANIM_LOOP );
            }

            if (!obj->fullbright) {
                model_skybox(model, last_scene->skybox);
            } else {
                skybox_t sb = {0};
                model_skybox(model, sb);
            }
            
            array_resize(obj->instances, 64);
            copy44(obj->instances[0], obj->transform);

            int num_instances = 1;
            for (unsigned k = j+1; k < obj_count; ++k) {
                object_t *obj2 = scene_index(k);
                if (!obj2->batchable || obj2->skip_draw || !object_compare(obj, obj2)) {
                    continue;
                }
                if (num_instances >= array_count(obj->instances)) {
                    array_resize(obj->instances, array_count(obj->instances) + 64);
                }
                copy44(obj->instances[num_instances++], obj2->transform);
                obj2->was_batched = true;
            }
            obj->num_instances = num_instances;

            int do_retexturing = model->iqm && array_count(obj->textures) > 0;
            if( do_retexturing ) {
                for(int i = 0; i < model->iqm->nummeshes; ++i) {
                    material_t *material = &model->materials[model->iqm->mesh_materials[i]];
                    if (!material->layer[MATERIAL_CHANNEL_ALBEDO].map.texture) {
                        material->layer[MATERIAL_CHANNEL_ALBEDO].map.texture = CALLOC(1, sizeof(texture_t));
                        *material->layer[MATERIAL_CHANNEL_ALBEDO].map.texture = texture_checker();
                    }
                    array_push(obj->old_textures, *material->layer[MATERIAL_CHANNEL_ALBEDO].map.texture);
                    *material->layer[MATERIAL_CHANNEL_ALBEDO].map.texture = (*array_back(obj->textures));
                }
            }

            if ( do_relighting || !obj->light_cached ) {
                obj->light_cached = 1;
                // shader_bind(model->iqm->program);
                // light_update(array_count(last_scene->lights), last_scene->lights);
                // @todo: rework light caching
            }
        }

        /* Build shadowmaps */
        if (flags & SCENE_SHADOWS) { 
            shadowmap_begin(sm);
            for (unsigned j = 0; j < array_count(last_scene->lights); ++j) {
                light_t *l = &last_scene->lights[j];
                while (shadowmap_step(sm)) {
                    shadowmap_light(sm, l, cam->proj, cam->view);
                    if (!l->processed_shadows) continue;
                    for(unsigned j = 0, obj_count = scene_count(); j < obj_count; ++j ) {
                        object_t *obj = scene_index(j);
                        model_t *model = obj->model_shadow.iqm ? &obj->model_shadow : &obj->model;
                        if (obj->model.iqm && obj->cast_shadows && !obj->was_batched) {
                            model_render_instanced(*model, cam->proj, cam->view, obj->instances, obj->num_instances);
                        }
                    }
                }
            }
            shadowmap_end(sm);
        }

        if (flags & SCENE_DRAWMAT) {
            drawmat_clear(&last_scene->drawmat);

            for(unsigned j = 0, obj_count = scene_count(); j < obj_count; ++j ) {
                object_t *obj = scene_index(j);
                model_t *model = &obj->model;
                if (!model->iqm) continue;
                if (obj->skip_draw) continue;
                if (obj->was_batched) continue;
                drawmat_render_instanced(&last_scene->drawmat, *model, cam->proj, cam->view, obj->instances, obj->num_instances);
            }
        }

        /* Collect all transparency enabled models and sort them by distance */
        static array(object_t*) transparent_objects = 0;
        for(unsigned j = 0, obj_count = scene_count(); j < obj_count; ++j ) {
            object_t *obj = scene_index(j);
            model_t *model = &obj->model;

            if (!model->iqm) continue;
            if (obj->skip_draw) continue;
            if (obj->was_batched) continue;

            if (model_has_transparency(*model)) {
                obj->distance = len3sq(sub3(cam->position, transform344(model->pivot, obj->pos)));
                array_push(transparent_objects, obj);
            }
        }

        array_sort(transparent_objects, scene_obj_distance_compare);

        /* Opaque pass */
        for(unsigned j = 0, obj_count = scene_count(); j < obj_count; ++j ) {
            object_t *obj = scene_index(j);
            model_t *model = &obj->model;
            if (!model->iqm) continue;
            if (obj->skip_draw) continue;
            if (obj->was_batched) continue;

            model_shadow(model, sm);
            model_light(model, array_count(last_scene->lights), last_scene->lights);
            model_render_instanced_pass(*model, cam->proj, cam->view, obj->instances, obj->num_instances, RENDER_PASS_OPAQUE);
        }

        /* Transparency pass */
        for (unsigned j = 0; j < array_count(transparent_objects); ++j) {
            object_t *obj = transparent_objects[j];
            model_t *model = &obj->model;
            if (!model->iqm) continue;
            if (obj->skip_draw) continue;
            if (obj->was_batched) continue;

            model_shadow(model, sm);
            model_light(model, array_count(last_scene->lights), last_scene->lights);
            model_render_instanced_pass(*model, cam->proj, cam->view, obj->instances, obj->num_instances, RENDER_PASS_TRANSPARENT);
        }

        array_resize(transparent_objects, 0);

        for(unsigned j = 0, obj_count = scene_count(); j < obj_count; ++j ) {
            object_t *obj = scene_index(j);
            model_t *model = &obj->model;
            if (obj->skip_draw) continue;
            if (obj->was_batched) continue;

            int do_retexturing = model->iqm && model->shading != SHADING_PBR && array_count(obj->textures) > 0;
            if( do_retexturing ) {
                for(int i = 0; i < model->iqm->nummeshes; ++i) {
                    material_t *material = &model->materials[model->iqm->mesh_materials[i]];
                    if (i < array_count(obj->old_textures)) {
                        if (material->layer[MATERIAL_CHANNEL_ALBEDO].map.texture)
                            *material->layer[MATERIAL_CHANNEL_ALBEDO].map.texture = obj->old_textures[i];
                    }
                }
                array_resize(obj->old_texture_ids, 0);
                array_resize(obj->old_textures, 0);
            }
        }
        glBindVertexArray(0);
    }

    if (flags & SCENE_POSTFX) {
        fx_end(0,0);
    }
}
