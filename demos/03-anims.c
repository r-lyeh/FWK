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

array(mat44)  M;     // instanced transforms

int main() {
    bool do_showaabb = 0;
    bool do_showbones = 0;
    bool do_showmodel = 1;
    bool do_showgizmo = 1;

    // 75% sized, MSAAx2
    window_create(75, WINDOW_MSAA2);
    window_title(__FILE__);

    camera_t cam = camera();
    skybox_t sky = skybox("cubemaps/stardust", 0);
    model_t  mdl = model("George.fbx", MODEL_RIMLIGHT); // kgirls01.fbx
    anims_t    a = animations("George.fbx", 0); // kgirl/animlist.txt

    // 32*32 max instances
    int NUM_INSTANCES = 1;
    array_resize(M, 32*32);
    for(int z = 0, i = 0; z < 32; ++z) {
        for(int x = 0; x < 32; ++x, ++i) {
            vec3 pos = vec3(-x*3,0,-z*3);
            vec3 rot = vec3(0,-90,0); // kgirl: 0,0,0
            vec3 sca = vec3(1,1,1); // kgirl: 2,2,2
            compose44(M[i], pos, eulerq(rot), sca);
        }
    }

    // shader_bind(mdl.program);
    // shader_vec3("u_rimcolor", vec3(0.12,0.23,0.34));
    // shader_vec3("u_rimrange", vec3(0.06,0.74,0.5));

    // load all postfx files in all subdirs
    fx_load("fx**.fs");

    while( window_swap() && !input(KEY_ESC) ) {
        // fps camera
        if( input(GAMEPAD_CONNECTED) ) {
            vec2 filtered_lpad = input_filter_deadzone(input2(GAMEPAD_LPAD), 0.15f/*do_gamepad_deadzone*/ + 1e-3 );
            vec2 filtered_rpad = input_filter_deadzone(input2(GAMEPAD_RPAD), 0.15f/*do_gamepad_deadzone*/ + 1e-3 );
            vec2 mouse = scale2(vec2(filtered_rpad.x, filtered_rpad.y), 1.0f);
            vec3 wasdec = scale3(vec3(filtered_lpad.x, input(GAMEPAD_LT) - input(GAMEPAD_RT), filtered_lpad.y), 1.0f);
            camera_moveby(&cam, wasdec);
            camera_fps(&cam, mouse.x,mouse.y);
            window_cursor( true );
        } else {
            bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
            if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
            vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
            vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
            camera_moveby(&cam, wasdecq);
            camera_fps(&cam, mouse.x,mouse.y);
            window_cursor( !active );
        }

        // skeletal update
        static bool is_dragging_slider = 0;
        vec2i anim = vec2i( a.anims[ a.inuse ].from, a.anims[ a.inuse ].to );
        profile("Skeletal update") {
            float delta = window_delta() * 30 * a.speed * !is_dragging_slider; // 30fps anim timer
            if(!window_has_pause()) mdl.curframe = model_animate_clip(mdl, mdl.curframe + delta, anim.min, anim.max, a.anims[a.inuse].flags & ANIM_LOOP );
        }

        // render
        fx_begin();

            // Skybox
            profile("Skybox") {
                skybox_render(&sky, cam.proj, cam.view);
            }

            // ground
            ddraw_ground(0);
            ddraw_flush();

            // characters
            profile("Skeletal render") {
                if( do_showmodel ) model_render_instanced(mdl, cam.proj, cam.view, M /*mdl.pivot*/, 0, NUM_INSTANCES);

                if( do_showbones ) model_render_skeleton(mdl, M[0] /*mdl.pivot*/);

                if( do_showaabb ) {
                    aabb box = model_aabb(mdl, M[0] /*mdl.pivot*/);
                    ddraw_aabb(box.min, box.max);
                }

                if( do_showgizmo ) {
                    static vec3 p = {0,0,0}, r = {0,-90,0}, s = {1,1,1};
                    gizmo(&p, &r, &s);
                    compose44(mdl.pivot, p, eulerq(r), s);
                }
            }

        fx_end();

        if ( ui_panel("Rim lighting", 0) ) {
            static vec3 rimcolor = {0.2,0.2,0.2};
            static vec3 rimrange = {0.11,0.98,0.5};
            ui_color3f("Color", &rimcolor.x);
            ui_clampf("Low", &rimrange.x, 0, 1);
            ui_clampf("High", &rimrange.y, 0, 1);
            ui_clampf("Mix", &rimrange.z, 0, 1);
            // ui_vec
            shader_bind(mdl.program);
            shader_vec3("u_rimcolor", rimcolor);
            shader_vec3("u_rimrange", rimrange);
            ui_panel_end();
        }
        if( ui_panel("Animation", PANEL_OPEN) ) {
            if( ui_bool("Show aabb", &do_showaabb) );
            if( ui_bool("Show bones", &do_showbones) );
            if( ui_bool("Show models", &do_showmodel) );
            if( ui_bool("Show gizmo", &do_showgizmo) );

            ui_separator();
            if( ui_int("Instances", &NUM_INSTANCES)) NUM_INSTANCES = clampi(NUM_INSTANCES, 1, array_count(M));
            ui_separator();

            ui_label(va("Anim %s [%d.. %.2f ..%d]", a.anims[ a.inuse ].name, anim.min, mdl.curframe, anim.max ));

            // normalize curframe into [0..1] range
            is_dragging_slider = 0;
            float range = (mdl.curframe - anim.min) / ((anim.max - anim.min) + !(anim.max - anim.min));
            if( ui_slider2("Frame", &range, va("%.2f/%d %02d%%", mdl.curframe - anim.min, anim.max - anim.min, (int)(range * 100.f))) ) {
                mdl.curframe = range * (anim.max - anim.min) + anim.min;
                is_dragging_slider = 1;
            }

            ui_slider2("Speed", &a.speed, va("x%.2f", a.speed));
            ui_separator();

            for( int i = 0; i < array_count(a.anims); ++i ) {
                bool selected = a.inuse == i;
                float progress = selected ? (mdl.curframe - anim.min) * 100.f / (anim.max - anim.min) : 0.f;
                const char *caption = va("%s%s%s %.2f%%", selected ? "*":"", a.anims[i].name, a.anims[i].flags & ANIM_LOOP ? " (Loop)":"", progress);
                int choice = ui_label2_toolbar(caption, va("%s %s %s", ICON_MD_REPLAY_CIRCLE_FILLED, a.inuse == i && a.speed <= 0 ? ICON_MD_NOT_STARTED : ICON_MD_PAUSE_CIRCLE, ICON_MD_PLAY_CIRCLE) );
                if( choice == 1 ) { // play/pause
                    if( mdl.curframe >= anim.max ) mdl.curframe = anim.min; // rewind animation
                    a.speed = 1.0f;
                    a.inuse = i;
                }
                if( choice == 2 ) {  // pause/advance-frame
                    if(a.speed <= 0) mdl.curframe = (int)mdl.curframe + 1;
                    a.speed = 0.0f;
                    a.inuse = i;
                }
                if( choice == 3 ) { // loop on/off
                    if( a.anims[i].flags & ANIM_LOOP )
                    a.anims[ i ].flags &= ~ANIM_LOOP;
                    else
                    a.anims[ i ].flags |= ANIM_LOOP;
                    a.inuse = i;
                    mdl.curframe = anim.min; // restart anim
                }
            }
            ui_panel_end();
        }
    }
}

// @todo: controller demo: root motion on/off
// @todo: controller demo: anim controller.lua

// this demo supersedes following old sources:
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-anims.c
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-instanced.c
