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
    bool do_instancing = 1;

    // 75% sized, MSAAx2
    window_create(75, 0);
    window_title(__FILE__);
    // window_fps_unlock();

    camera_t cam = camera();
    skybox_t sky = skybox("cubemaps/stardust", 0);
    model_t  mdl = model("George.fbx", 0);
    anims_t    a = animations("George.fbx", 0);

    // 32*32 max instances
    int NUM_INSTANCES = 1;
    array_resize(M, 128*128);
    for(int z = 0, i = 0; z < 128; ++z) {
        for(int x = 0; x < 128; ++x, ++i) {
            vec3 pos = vec3(-x*3,0,-z*3);
            vec3 rot = vec3(0,0,0); // kgirl: 0,0,0
            vec3 sca = vec3(1,1,1); // kgirl: 2,2,2
            compose44(M[i], pos, eulerq(rot), sca);
        }
    }

    // load all postfx files in all subdirs
    fx_load("fx**.fs");

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

        static vec3 rim_color = {0.2,0.2,0.2};
        static vec3 rim_range = {0.11,0.98,0.5};
        static vec3 rim_pivot = {0,0,0};
        static bool rim_ambient = true;

        // skeletal update
        static bool is_dragging_slider = 0;
        vec2i anim = vec2i( a.anims[ a.inuse ].from, a.anims[ a.inuse ].to );
        if (do_anims) {
            profile("Skeletal update") {
                float delta = window_delta() * 30 * a.speed * !is_dragging_slider; // 30fps anim timer
                if(!window_has_pause()) mdl.curframe = model_animate_clip(mdl, mdl.curframe + delta, anim.min, anim.max, a.anims[a.inuse].flags & ANIM_LOOP );
            }
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
                if( do_showmodel ) {
                    model_rimlight(&mdl, rim_range, rim_color, rim_pivot, rim_ambient);
                    if (do_instancing) {
                        model_render_instanced(mdl, cam.proj, cam.view, M /*mdl.pivot*/, NUM_INSTANCES);
                    } else {
                        for (int i = 0; i < NUM_INSTANCES; i++) {
                            model_render(mdl, cam.proj, cam.view, M[i] /*mdl.pivot*/);
                        }
                    }
                }

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
            ui_color3f("Color", &rim_color.x);
            ui_clampf("Low", &rim_range.x, 0, 1);
            ui_clampf("High", &rim_range.y, 0, 1);
            ui_clampf("Mix", &rim_range.z, 0, 1);
            ui_panel_end();
        }
        if( ui_panel("Animation", PANEL_OPEN) ) {
            if( ui_bool("Show aabb", &do_showaabb) );
            if( ui_bool("Show bones", &do_showbones) );
            if( ui_bool("Show models", &do_showmodel) );
            if( ui_bool("Show gizmo", &do_showgizmo) );
            if( ui_bool("Anims", &do_anims) );
            if( ui_bool("Instancing", &do_instancing) );
            
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
