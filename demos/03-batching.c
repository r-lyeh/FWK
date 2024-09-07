// anims demo: input, mesh, anim, render, fx, ui, instancing
// - rlyeh, public domain.
//
// Compile with:
//    `make     demos\03-anims.c` (windows)
// `sh MAKE.bat demos/03-anims.c` (linux, osx)
//
// @todo: instanced poses, instanced render_bones
// @todo: ik, modify bone transform (bone space)

#include "fwk.h"

typedef struct anims_t {
    int   inuse; // animation number in use
    float speed; // x1.00
    array(anim_t) anims; // [begin,end,flags] frames of every animation in set
} anims_t;

anims_t animations(const char *pathfile, int flags) {
    anims_t a = {0};
    a.anims = animlist(pathfile);
    if(a.anims) a.speed = 1.0;
    return a;
}

int main() {
    bool do_showaabb = 0;
    bool do_showbones = 0;
    bool do_showmodel = 1;
    bool do_showgizmo = 1;
    bool do_anims = 1;

    // 75% sized, MSAAx2
    window_create(75, 0);
    window_title(__FILE__);
    window_fps_unlock();

    camera_t cam = camera(); {
        cam.position = vec3(35.65, 11.62, -9.44);
        cam.yaw = 180;
        cam.pitch = 0;
        camera_fps(&cam, 0, 0);
    }
    skybox_t sky = skybox("cubemaps/stardust", 0);
    model_t  mdl = model("cube.obj", 0);
    model_t  plane = model("plane.obj", 0);
    scene_skybox(sky);

    light_t *light = scene_spawn_light();
    light->type = LIGHT_DIRECTIONAL; 

    int NUM_INSTANCES = 1024;
    for(int z = 0, i = 0; z < 128; ++z) {
        for(int x = 0; x < 128; ++x, ++i) {
            if (i >= NUM_INSTANCES) break;
            object_t *obj = scene_spawn();
            vec3 pos = vec3(-x*3,1.05,-z*3);
            vec3 rot = vec3(0,180,0); 
            vec3 sca = vec3(1,1,1);
            object_teleport(obj, pos);
            object_rotate(obj, rot);
            object_scale(obj, sca);
            object_model(obj, mdl);
        }
    }

    object_t *obj_plane = scene_spawn();
    object_model(obj_plane, plane);
    object_scale(obj_plane, vec3(100,100,100));

    while( window_swap() && !input(KEY_ESC) ) {
        // fps camera
        if( input(GAMEPAD_CONNECTED) ) {
            vec2 filtered_lpad = input_filter_deadzone(input2(GAMEPAD_LPAD), 0.15f/*do_gamepad_deadzone*/ + 1e-3 );
            vec2 filtered_rpad = input_filter_deadzone(input2(GAMEPAD_RPAD), 0.15f/*do_gamepad_deadzone*/ + 1e-3 );
            vec2 mouse = scale2(vec2(filtered_rpad.x, filtered_rpad.y), 1.0f);
            vec3 wasdec = scale3(vec3(filtered_lpad.x, input(GAMEPAD_LT) - input(GAMEPAD_RT), filtered_lpad.y), 1.0f);
            camera_moveby(&cam, scale3(wasdec, window_delta() * 60));
            camera_fps(&cam, mouse.x,mouse.y);
            window_cursor( true );
        } else {
            camera_freefly(&cam);
        }

        // ground
        ddraw_ground(0);
        ddraw_flush();
        scene_render(SCENE_FOREGROUND|SCENE_BACKGROUND|SCENE_CAST_SHADOWS);

        if (ui_panel("Scene", 0)) {
            ui_separator();
            ui_light(light);
            ui_panel_end();
        }
    }
}

// @todo: controller demo: root motion on/off
// @todo: controller demo: anim controller.lua

// this demo supersedes following old sources:
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-anims.c
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-instanced.c
