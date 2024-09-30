// render map

#include "engine.h"

int main() {
    window_create(80, WINDOW_MSAA8);
    window_title(__FILE__);
    window_fps_unlock();

    // load skybox
    scene_skybox(skybox("bridge3", 0)); 

    // load static scene
    model_t map, prop;
    map = model(option("--model","sorting_test.obj"), MODEL_NO_PBR); // MODEL_NO_TEXTURES);
    prop = model(option("--model","sorting_test2.obj"), MODEL_NO_PBR); // MODEL_NO_TEXTURES);

    // define scene
    object_t *prop_obj = scene_spawn();
    object_model(prop_obj, prop);

    object_t *map_obj = scene_spawn();
    object_model(map_obj, map);

    // camera
    camera_t cam = camera();

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;
        if( input_down(KEY_F5) ) window_reload();
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );
        if( input_down(KEY_X) ) window_screenshot(__FILE__ ".png");
        if( input_down(KEY_Z) ) window_record(__FILE__ ".mp4");

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, scale3(wasdecq, window_delta() * 60));
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

        scene_render(SCENE_BACKGROUND|SCENE_FOREGROUND);
    }
}
