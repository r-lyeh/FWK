#include "fwk.h"

int main() {
    window_create(80, WINDOW_MSAA4);
    window_title(__FILE__);

    // load skybox: launch with --mie for rayleigh/mie scattering
    skybox_t sky = skybox(flag("--mie") ? 0 : "cubemaps/stardust", 0);
    light_t *l = scene_spawn_light();

    // terrain model
    model_t mdl = model("rabbit.obj", 0); {
        model_light(&mdl, 1, l);
    }

    // camera
    camera_t cam = camera();
    cam.position = vec3(5,3,-4);
    camera_lookat(&cam, vec3(2,0,0));

    // initialise LOD
    float lo_detail=0.25f;
    float hi_detail=1.00f;
    float morph=0.75f;
    model_lod(&mdl, lo_detail, hi_detail, morph);

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;
        if( input_down(KEY_F5) ) window_reload();
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, scale3(wasdecq, window_delta() * 60));
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

        // draw scene
        skybox_render(&sky, cam.proj, cam.view);

        model_render(mdl, cam.proj, cam.view, mdl.pivot);

        if( ui_panel("LODs", PANEL_OPEN) ) {
            float old_lo = lo_detail;
            float old_hi = hi_detail;
            float old_morph = morph;
            if(ui_slider("Lo detail", &lo_detail)) { lo_detail = clampf(lo_detail, 0.01f, 1.0f); if(lo_detail > hi_detail) hi_detail = lo_detail; }
            if(ui_slider("Hi detail", &hi_detail)) { hi_detail = clampf(hi_detail, 0.01f, 1.0f); if(hi_detail < lo_detail) lo_detail = hi_detail; }
            ui_slider("Morph", &morph);

            if (old_lo != lo_detail || old_hi != hi_detail || old_morph != morph) {
                model_lod(&mdl, lo_detail, hi_detail, morph);
            }
            ui_panel_end();
        }
    }
}