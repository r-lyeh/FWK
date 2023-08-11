// full controller demo: anims, input, collide; @todo: gamepad, input opts, easing on hits, notify on gamepad connect
// - rlyeh, public domain.
//
// Compile with:
//    `make     demos\99-controller.c` (windows)
// `sh MAKE.bat demos/99-controller.c` (linux, osx)

#include "fwk.h"

int main() {
    // 75% window, MSAAx2 flag
    window_create(75, WINDOW_MSAA2);

    // fx: load all post fx files in all subdirs
    fx_load("fx**.fs");

    // create a camera
    camera_t cam = camera();
    camera_enable(&cam);

    // config 3d model #1
    model_t witch = model("witch/witch.obj", 0);
    model_set_texture(witch, texture("witch/witch_diffuse.tga.png", 0));
    mat44 witch_pivot; vec3 witch_p = {-5,0,-5}, witch_r={-180,180,0}, witch_s={0.1,-0.1,0.1};

    // config 3d model #2
    model_t girl = model("kgirl/kgirls01.fbx", 0);
    mat44 girl_pivot; vec3 girl_p = {0,0,0}, girl_r = {270,0,0}, girl_s = {2,2,2};

    // skybox
    skybox_t sky = skybox("cubemaps/stardust", 0);

    // BGM
    audio_play( audio_stream("waterworld-map.fur"), 0 );

    // editor loop
    while( window_swap() ) {

        // game camera
        profile("Game.Camera") {
            camera_t *cam = camera_get_active();

            static vec3 source;
            do_once source = cam->position;

            vec3 target = add3(girl_p, vec3(0,10,0));
                 target = add3(target, scale3(norm3(sub3(source, target)), 10.0));
            source = mix3(source, target, 1-0.99f);

            camera_teleport(cam, source);
            camera_lookat(cam, vec3(girl_p.x,0,girl_p.z));
        }

        // render begin (postfx)
        fx_begin();

            // skybox
            skybox_render(&sky, cam.proj, cam.view);

            // world
            ddraw_grid(0);
            ddraw_flush();

            // models
            compose44(girl.pivot, girl_p, eulerq(girl_r), girl_s);
            model_render(girl, cam.proj, cam.view, girl.pivot, 0);

            compose44(witch.pivot, witch_p, eulerq(witch_r), witch_s);
            model_render(witch, cam.proj, cam.view, witch.pivot, 0);

        // render end (postfx)
        fx_end();

        // input controllers

        double GAME_JUMP_DOWN = input_down(KEY_Z);
        double GAME_FIRE_DOWN = input_down(KEY_X);
        double GAME_JUMP  = input(KEY_Z);
        double GAME_FIRE  = input(KEY_X);
        double GAME_LEFT  = input(KEY_J);
        double GAME_RIGHT = input(KEY_L);
        double GAME_UP    = input(KEY_I);
        double GAME_DOWN  = input(KEY_K);
        double GAME_AXISX = input(KEY_L) - input(KEY_J);
        double GAME_AXISY = input(KEY_I) - input(KEY_K);

        if( !input_anykey() ) {
            if( input(GAMEPAD_CONNECTED) ) {
                vec2 filtered_lpad = input_filter_deadzone(input2(GAMEPAD_LPAD), 0.15f /*15% deadzone*/);
                GAME_JUMP_DOWN = input_down(GAMEPAD_A);
                GAME_FIRE_DOWN = input_down(GAMEPAD_B) || input_down(GAMEPAD_X) || input_down(GAMEPAD_Y);
                GAME_JUMP  = input(GAMEPAD_A);
                GAME_FIRE  = input(GAMEPAD_B) || input(GAMEPAD_X) || input(GAMEPAD_Y);
                GAME_AXISX = filtered_lpad.x;
                GAME_AXISY = filtered_lpad.y;
            }
        }

        // animation controllers

        profile("Game.Animate scene") if( !window_has_pause() ) {
            float delta = window_delta() * 30; // 30fps anim

            // animate girl
            girl.curframe = model_animate(girl, girl.curframe + delta);

            // jump controller: jump duration=1.5s, jump height=6 units, anims (expo->circ)
            float jump_delta = 1.0;
            static double jump_timer = 0, jump_ss = 1.5, jump_h = 6;
            if( GAME_JUMP_DOWN ) if( jump_timer == 0 ) jump_timer = time_ss();
            jump_delta = clampf(time_ss() - jump_timer, 0, jump_ss) * (1.0/jump_ss);
            if( jump_delta >= 1 ) { jump_timer = 0; }
            float y = ease_ping_pong( jump_delta, ease_out_expo, ease_out_circ);
            girl_p.y = y * jump_h;

            // punch controller
            float punch_delta = 1;
            if( jump_delta >= 1 ) {
            static vec3 origin;
            static double punch_timer = 0, punch_ss = 0.5;
            if( GAME_FIRE_DOWN ) if( punch_timer == 0 ) punch_timer = time_ss(), origin = girl_p;
            punch_delta = clampf(time_ss() - punch_timer, 0, punch_ss) * (1.0/punch_ss);
            if( punch_delta >= 1 ) { punch_timer = 0; }
            else {
                float x = ease_out_expo( punch_delta );
                vec3 fwd = rotate3q(vec3(0,0,1), eulerq(vec3(girl_r.x - 170,girl_r.y,girl_r.z)));
                vec3 mix = mix3(girl_p, add3(origin,scale3(fwd,x*2)), x);
                girl_p.x = mix.x, girl_p.z = mix.z;
            }
            }

            int modern_controller = 1;
            int running = 0;

            // girl controller

                // locomotion vars
                float speed = 0.2f * delta;
                float yaw_boost = GAME_AXISY > 0 ? 1.0 : 1.75;
                if(punch_delta < 1) yaw_boost = 0.0; // if firing...
                else if(punch_delta <= 0.1) yaw_boost = 4.0; // unless initial punch chaining, extra yaw

                // old fashioned locomotion controller (boat controller)
                if(!modern_controller) {
                    running = GAME_AXISY > 0;

                    girl_r.x -= 170;
                        quat q = eulerq(girl_r); // += custom.pivot
                        vec3 rgt = rotate3q(vec3(1,0,0), q);
                        vec3 up  = rotate3q(vec3(0,1,0), q);
                        vec3 fwd = rotate3q(vec3(0,0,1), q);
                        vec3 dir = scale3(fwd, speed * GAME_AXISY * (GAME_AXISY > 0 ? 2.0 : 0.5));
                        girl_r.x += speed * 20.0 * yaw_boost * GAME_AXISX; // yaw
                        girl_p = add3(girl_p, dir);
                    girl_r.x += 170;
                }

                // modern locomotion controller (mario 3d)
                if(modern_controller) {
                    running = GAME_AXISX != 0 || GAME_AXISY != 0;

                    camera_t *cam = camera_get_active();
                    vec3 fwd = sub3(girl_p, cam->position); fwd.y = 0; fwd = norm3(fwd);
                    vec3 rgt = norm3(cross3(fwd, vec3(0,1,0)));

                    // target
                    vec3 dir = add3(
                        scale3(fwd, GAME_AXISY),
                        scale3(rgt, GAME_AXISX)
                    ); dir.y = 0; dir = norm3(dir);

                    // smoothing
                    static vec3 olddir; do_once olddir = dir;
                    dir = mix3(dir, olddir, 1 - (yaw_boost / 4.0) * 0.85);
                    olddir = dir;

                    // vis
                    // ddraw_arrow(girl_p, add3(girl_p,scale3(dir,10)));

                    // apply direction
                    girl_p = add3(girl_p, scale3(dir, speed * 2));

                    // apply rotation
                    {
                        girl_r.x -= 170;
                        quat q = eulerq(girl_r);
                        vec3 fwdg = rotate3q(vec3(0,0,1), q);
                        girl_r.x += 170;

                        //float cosAngle = dot3(dir,fwdg);
                        //float angle = acos(cosAngle) * TO_DEG;
                        float angle = TO_DEG * ( atan2(fwdg.z, fwdg.x) - atan2(dir.z, dir.x));

                        if( !isnan(angle) ) {
                            girl_r.x -= angle;
                            while(girl_r.x> 180) girl_r.x-=360;
                            while(girl_r.x<-180) girl_r.x+=360;
                        }
                    }
                }

            // anim loops
            if( jump_delta < 1 ) { // jump/kick anim
#if 0
                girl.curframe = clampf(girl.curframe, 184, 202);
                if( girl.curframe > 202-4 && GAME_FIRE_DOWN ) girl.curframe = 184+4;
#else
                #define loopf(frame, min, max) (frame < min ? min : frame > max ? min + frame - max : frame)
                if(girl.curframe >= 203)
                girl.curframe = loopf(girl.curframe, 203, 220);
                else
                girl.curframe = clampf(girl.curframe, 184, 202);
                if( girl.curframe > 202-4 && girl.curframe < 208 && GAME_FIRE_DOWN ) girl.curframe = 203;
#endif
            }
            else if( punch_delta < 1 ) { // punch anim
                girl.curframe = clampf(girl.curframe, 90, 101);
                if( girl.curframe > 101-6 && GAME_FIRE_DOWN ) girl.curframe = 101-6;
            }
            else if( running ) {
                // loop running anim
                if( girl.curframe < 65 ) girl.curframe = 65;
                if( girl.curframe > 85 ) girl.curframe = 65;
            }
            else { // loop idle anim
                if( girl.curframe > 59 ) girl.curframe = 0;
            }
        }

        // Game collisions

        profile("Game.collisions") {
            bool punching = girl.curframe >= 90 && girl.curframe < 101;
            bool air_kicking = girl.curframe >= 184 && girl.curframe < 202;
            bool jump_kicking = girl.curframe >= 203 && girl.curframe < 220;
            bool attacking = punching || air_kicking || jump_kicking;

            if( attacking ) {
                aabb boxg = model_aabb(girl, girl_pivot);
                aabb boxw = model_aabb(witch, witch_pivot);
#if 0 // halve aabb. ok
                {
                    vec3 diag = sub3(boxg.max, boxg.min);
                    vec3 halve = scale3(diag, 0.25);
                    vec3 center = scale3(add3(boxg.min, boxg.max), 0.5);
                    boxg.min = sub3(center, halve);
                    boxg.max = add3(center, halve);
                }
#endif
                hit* h = aabb_hit_aabb(boxg, boxw);
                if( h && GAME_FIRE ) {
                    vec3 dir = norm3(sub3(witch_p, girl_p));
                    witch_p = add3(witch_p, mul3(dir,vec3(1,0,1)));
                }
            }
        }

        // ui
        if( ui_panel("Input", 0) ) { // @todo: showcase input binding
            ui_section("Controllers");
            ui_label("Gamepad #1");
            ui_label("Keys I/J/K/L + Z/X");
            ui_panel_end();
        }
        if( ui_panel("FX", 0) ) {
            for( int i = 0; i < 64; ++i ) {
                char *name = fx_name(i); if( !name ) break;
                bool b = fx_enabled(i);
                if( ui_bool(name, &b) ) fx_enable(i, b);
            }
            ui_panel_end();
        }
    }
}

//    vec2  do_gamepad_polarity = vec2(+1,+1);
//    vec2  do_gamepad_sensitivity = vec2(0.1f,0.1f);
//    vec2  do_mouse_polarity = vec2(+1,-1);
//    vec2  do_mouse_sensitivity = vec2(0.2f,0.2f);
//    float do_gamepad_deadzone = 0.15f;//
//
//    if(ui_separator()) {}
//    if(ui_slider("Gamepad deadzone", &do_gamepad_deadzone)) {}
//    if(ui_float2("Gamepad polarity", do_gamepad_polarity.v2)) {}
//    if(ui_float2("Gamepad sensitivity", do_gamepad_sensitivity.v2)) {}
//    if(ui_separator()) {}
//    if(ui_float2("Mouse polarity", do_mouse_polarity.v2)) {}
//    if(ui_float2("Mouse sensitivity", do_mouse_sensitivity.v2)) {}//
