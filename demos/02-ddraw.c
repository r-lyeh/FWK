// ddraw demo: fps camera, renderdd, collide, math, ui, fx, boids
// - rlyeh, public domain.
//
// Compile with:
//    `make     demos\02-ddraw.c` (windows)
// `sh MAKE.bat demos/02-ddraw.c` (linux, osx)

#include "fwk.h"

int main() {
    bool do_boids_demo = 1;
    bool do_colliders_demo = 1;
    bool do_debugdraw_demo = 1;

    // 75% size, MSAAx2
    window_create(75.0, WINDOW_MSAA2);
    window_title(__FILE__);

    // camera that points to origin
    camera_t cam = camera();
    // load skybox folder (no flags)
    skybox_t sky = skybox("cubemaps/stardust", 0);
    // load all postfx files in all subdirs
    fx_load("fx**.fs");

    // main loop
    while( window_swap() ) {

        // input handler
        if (input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen()^1 );
        if (input_down(KEY_ESC) ) break;

        // fps camera
        profile("FPS camera") {
            if( input(GAMEPAD_CONNECTED) ) {
                vec2 filtered_lpad = input_filter_deadzone(input2(GAMEPAD_LPAD), 0.15f/*do_gamepad_deadzone*/ + 1e-3 );
                vec2 filtered_rpad = input_filter_deadzone(input2(GAMEPAD_RPAD), 0.15f/*do_gamepad_deadzone*/ + 1e-3 );
                vec2 mouse = scale2(vec2(filtered_rpad.x, filtered_rpad.y), 1.0f);
                vec3 wasdec = scale3(vec3(filtered_lpad.x, input(GAMEPAD_LT) - input(GAMEPAD_RT), filtered_lpad.y), 1.0f);
                camera_moveby(&cam, scale3(wasdec, window_delta() * 60));
                camera_fps(&cam, mouse.x,mouse.y);
                window_cursor( true );
            } else {
                bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
                if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
                vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
                vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
                camera_moveby(&cam, scale3(wasdecq, window_delta() * 60));
                camera_fps(&cam, mouse.x,mouse.y);
                window_cursor( !active );
            }
        }

        // draw skybox
        skybox_render(&sky, cam.proj, cam.view);

        // world
        ddraw_grid(0);

        // boids
        static swarm_t sw;
        if( do_boids_demo ) profile("boids") {
            do_once sw = swarm();
            do_once array_push(sw.steering_targets, vec3(0,0,0));
            do_once for(int i = 0; i < 100; ++i)
                array_push(sw.boids, boid(scale3(rnd3(),10), scale3(rnd3(),.10))); // pos,vel

            // move
            sw.steering_targets[0] = cam.position;
            swarm_update(&sw, window_delta()/60);

            // draw
            for (int j = 0, end = array_count(sw.boids); j < end; ++j) {
                vec3 dir = norm3(sub3(sw.boids[j].position, sw.boids[j].prev_position));
                ddraw_boid(sw.boids[j].position, dir);
            }
        }

        // showcase many debugdraw shapes
        if( do_debugdraw_demo ) {
            ddraw_demo();
        }

        // showcase many colliding tests
        if( do_colliders_demo ) {
            collide_demo();
        }

        // ui
        if( ui_panel("App", 0) ) {
            ui_bool("Boids demo", &do_boids_demo);
            ui_bool("Collide demo", &do_colliders_demo);
            ui_bool("DebugDraw demo", &do_debugdraw_demo);
            ui_panel_end();
        }
        if( ui_panel("Swarm", 0) ) {
            ui_swarm(&sw);
            ui_panel_end();
        }
    }
}

// this demo supersedes following old sources:
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-collide.c
