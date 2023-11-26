// render sponza

#include "fwk.h"

int main() {
    window_create(80, WINDOW_MSAA8);
    window_title(__FILE__);
    window_fps_unlock();

    // load all fx files
    fx_load("fx**.fs");

    // load skybox
    skybox_t sky = skybox(flag("--mie") ? 0 : "hdr/Tokyo_BigSight_1k.hdr", 0); // --mie for rayleigh/mie scattering

    // load static scene
    model_t sponza;
    sponza = model(option("--model","sponza.obj"), 0); // MODEL_NO_TEXTURES);
    translation44(sponza.pivot, 0,-1,0);
    rotate44(sponza.pivot, -90,1,0,0);
    scale44(sponza.pivot, 10,10,10);

    // camera
    camera_t cam = camera();

    double sky_update_until = 0.0;

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;
        if( input_down(KEY_F5) ) window_reload();
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );
        if( input_down(KEY_X) ) window_screenshot(__FILE__ ".png");
        if( input_down(KEY_Z) ) window_record(__FILE__ ".mp4");

        // initialize SH coefficients from rayleigh skybox
        if (flag("--mie") && sky_update_until <= window_time()) {
            shader_bind(sky.program);
            shader_vec3("uSunPos", vec3(0, (cosf((float)window_time()*0.25)*0.3)+0.2, -1));
            skybox_mie_calc_sh(&sky, 2.0f);
            sky_update_until = window_time() + 0.02;
        }

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, wasdecq);
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

        // draw skybox
        profile("Skybox") {
            skybox_render(&sky, cam.proj, cam.view);
        }

        // apply post-fxs from here
        fx_begin();

        float scale = 1.00;
        mat44 M; copy44(M, sponza.pivot); translate44(M, 0,0,0); scale44(M, scale,scale,scale);

        shader_bind(sponza.program);
        shader_vec3v("u_coefficients_sh", 9, sky.cubemap.sh);

        model_render(sponza, cam.proj, cam.view, M, 0);

        // post-fxs end here
        fx_end(0);
    }
}
