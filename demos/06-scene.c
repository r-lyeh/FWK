// scene demo
// - rlyeh, public domain

#include "engine.h"

int main() {
    // options
    bool  do_twosided = 1;
    bool  do_wireframe = 0;
    bool  do_billboard_x = 0, do_billboard_y = 0, do_billboard_z = 0;

    // window (80% sized, MSAA x4 flag)
    window_create(80, WINDOW_MSAA4);
    window_title(__FILE__);

    // load all postfx files in all subdirs
    fx_load("fx**.fs");

    // scene loading
    #define SCENE(...) #__VA_ARGS__
    const char *my_scene = SCENE([
        {
            skybox: 'cubemaps/stardust/',
        },
        {
            position:[-5.0,-2.0,2.0],
            rotation: [90.0,-90.0,0.0],
            scale:0.20,
            mesh:'models/witch/witch.obj',
            texture:'models/witch/witch_diffuse.tga.png',
            flipuv:false,
            fullbright:true,
            pbr:false,
        },
        {
            position:[-5.0,-2.0,2.0],
            rotation: [90.0,-90.0,0.0],
            scale:2.20,
            mesh:'models/witch/witch_object.obj',
            texture:'models/witch/witch_object_diffuse.tga.png',
            flipuv:false,
            fullbright:true,
            pbr:false,
        },
    ]);
    int num_spawned = scene_merge(my_scene);
    object_t *obj1 = scene_index(0);
    object_t *obj2 = scene_index(1);

    // manual spawn & loading
    model_t m1 = model("kgirl/kgirls01.fbx", MODEL_NO_PBR); //MODEL_NO_ANIMS);
    texture_t t1 = texture("kgirl/g01_texture.png", 0);
    object_t* obj3 = scene_spawn();
    obj3->fullbright = true;
    object_model(obj3, m1);
    object_diffuse(obj3, t1);
    object_scale(obj3, vec3(3,3,3));
    object_move(obj3, vec3(-10,0,-10));
    object_pivot(obj3, vec3(0,90,-90));

    // camera
    camera_t cam = camera();

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, scale3(wasdecq, window_delta() * 60));
        camera_fps(&cam, mouse.x,mouse.y);

        // queue model scale bounces
        float t = fmod(window_time(), 0.3) / 0.3;
        float s = 0.01f * ease_ping_pong(t, EASE_IN|EASE_CUBIC,EASE_OUT|EASE_CUBIC);
        object_scale(obj1, vec3(0.20f - s,0.20f + s,0.20f - s));
        object_scale(obj2, vec3(0.20f - s,0.20f + s,0.20f - s));

        // queue model billboard
        object_billboard(obj1, (do_billboard_x << 2)|(do_billboard_y << 1)|(do_billboard_z << 0));
        object_billboard(obj2, (do_billboard_x << 2)|(do_billboard_y << 1)|(do_billboard_z << 0));

        // queue model rotation
        //object_rotate(obj3, vec3(0,1*window_time() * 20,0));

        // flush render scene (background objects: skybox)
        profile("Scene background") {
            scene_render(SCENE_BACKGROUND);
        }

        // queue debug drawcalls
        profile("Debugdraw") {
            ddraw_ground(0);
            ddraw_color(YELLOW);
            ddraw_text(vec3(+1,+1,-1), 0.04f, va("(%f,%f,%f)", cam.position.x,cam.position.y,cam.position.z));
            ddraw_color(YELLOW);
            ddraw_flush();
        }

        // apply post-fxs from here
        fx_begin();

        // render scene (foreground objects) with post-effects
        profile("Scene foreground") {
            int scene_flags = 0;
            scene_flags |= do_wireframe ? SCENE_WIREFRAME : 0;
            scene_flags |= do_twosided ? 0 : SCENE_CULLFACE;
            scene_render(SCENE_FOREGROUND | scene_flags);
        }

        profile("Skeletal update") if(!window_has_pause()) {
            float delta = window_delta() * 30 ; // 30fps anim
            obj3->model.curframe = model_animate(obj3->model, obj3->model.curframe + delta);

            ddraw_text(vec3(-10,5,-10), 0.05, va("Frame: %.1f", obj3->model.curframe));
        }

        // post-fxs end here
        fx_end(0,0);

        // queue ui
        if( ui_panel("Scene", 0)) {
            if(ui_toggle("Billboard X", &do_billboard_x)) {}
            if(ui_toggle("Billboard Y", &do_billboard_y)) {}
            if(ui_toggle("Billboard Z", &do_billboard_z)) {}
            if(ui_separator()) {}
            if(ui_bool("Wireframe", &do_wireframe)) {}
            if(ui_bool("Two sided", &do_twosided)) {}
            ui_panel_end();
        }
    }
}

#if 0

// ----------------------------------------------------------------------------
// material demo
// - rlyeh, public domain
//
// @todo: object_print(obj, "");

// create camera
camera_t cam = camera();
// load video, RGB texture, no audio
video_t *v = video( "pexels-pachon-in-motion-17486489.mp4", VIDEO_RGB | VIDEO_NO_AUDIO | VIDEO_LOOP ); video_seek(v, 30);
// load texture
texture_t t1 = texture("kgirl/g01_texture.png", TEXTURE_RGB);
texture_t t2 = texture("matcaps/material3", 0);
// load model
model_t m1 = model("suzanne.obj", MODEL_NO_ANIMATIONS);
model_t m2 = model("suzanne.obj", MODEL_NO_ANIMATIONS|MODEL_MATCAPS);

// spawn object1 (diffuse)
object_t* obj1 = scene_spawn();
object_model(obj1, m1);
object_diffuse(obj1, t1);
object_scale(obj1, vec3(3,3,3));
object_move(obj1, vec3(-10+5*0,0,-10));
object_pivot(obj1, vec3(0,90,0));

// spawn object2 (matcap)
object_t* obj2 = scene_spawn();
object_model(obj2, m2);
object_diffuse(obj2, t2);
object_scale(obj2, vec3(3,3,3));
object_move(obj2, vec3(-10+5*2,0,-10));
object_pivot(obj2, vec3(0,90,0));

// spawn object2 (video)
object_t* obj3 = scene_spawn();
object_model(obj3, m1);
object_diffuse(obj3, video_textures(v)[0]);
object_scale(obj3, vec3(3,3,3));
object_move(obj3, vec3(-10+5*1,0,-10));
object_pivot(obj3, vec3(0,90,0));

// @todo: add shadertoy material
        static model_t cube; do_once cube = model("cube.obj", 0);
        static shadertoy_t s; do_once s = shadertoy("shadertoys/4ttGWM.fs", 256);
        model_set_texture(&cube, shadertoy_render(&s, window_delta())->tx);
        model_render(cube, cam.proj, cam.view, cube.pivot,  0);

while(window_swap() && !input(KEY_ESC)) {
    // draw environment
    viewport_color( RGB3(22,22,32) );
    ddraw_grid(0);
    ddraw_flush();

    // update video
    video_decode( v );

    // draw scene
    scene_render(SCENE_FOREGROUND);
}

// load static scene
// model_t sponza = model("sponza.obj", MODEL_MATCAPS);
// model_set_texture(&sponza, texture("matcaps/normals", 0));
// translation44(sponza.pivot, 0,-1,0);
// rotate44(sponza.pivot, -90,1,0,0);
// scale44(sponza.pivot, 10,10,10);
    // model_render(sponza, cam.proj, cam.view, sponza.pivot, 0);

// this demo supersedes following old sources:
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-material.c
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-shadertoy.c

#endif
