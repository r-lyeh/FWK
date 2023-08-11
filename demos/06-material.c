// material demo
// - rlyeh, public domain
//
// @todo: object_print(obj, "");

#include "fwk.h"

int main() {
    // create the window
    window_create( 0.75f, WINDOW_MSAA8 );
    window_color( GRAY );

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

    // create point light
    light_t* l = scene_spawn_light();
    light_type(l, LIGHT_POINT);

    while(window_swap() && !input(KEY_ESC)) {
        // draw environment
        ddraw_grid(0);

        // update video
        video_decode( v );

        // update light position
        light_teleport(l, cam.position);

        // draw scene
        scene_render(SCENE_FOREGROUND|SCENE_BACKGROUND|SCENE_UPDATE_SH_COEF);

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouselook = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdec = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-input(KEY_C),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, wasdec);
        camera_fps(&cam, mouselook.x,mouselook.y);
        window_cursor( !active );
    }
}
