// 2d demo: window, audio, camera, font, tiled, render, fx, spritesheet, input, ui. @todo: spine
// - rlyeh, public domain.
//
// Compile with:
//    `make     demos\01-demo2d.c` (windows)
// `sh MAKE.bat demos/01-demo2d.c` (linux, osx)

#include "fwk.h"

void demo_kids(vec3 offs) {
    // init
    static texture_t kids; do_once kids = texture( "spriteSheetExample.png", TEXTURE_LINEAR );
    static vec3 pos[2] = {{490,362},{442,362}}, vel[2] = {0};
    static int row[2] = {0,3}, frame[2] = {0};
    static int inputs[2][4] = {{KEY_W,KEY_A,KEY_S,KEY_D},{KEY_UP,KEY_LEFT,KEY_DOWN,KEY_RIGHT}};

    // move
    for( int i = 0; i < countof(pos); ++i ) {
        vel[i].x = input(inputs[i][3]) - input(inputs[i][1]);
        vel[i].y = input(inputs[i][2]) - input(inputs[i][0]);
        pos[i].x = fmod(pos[i].x + vel[i].x, window_width() + 128);
        pos[i].y = fmod(pos[i].y + vel[i].y, window_height() + 128);
        frame[i] += vel[i].x || vel[i].y;
    }

    // render
    for( int i = 0; i < countof(pos); ++i ) {
        int col = frame[i] / 10, num_frame = row[i] * 4 + col % 4; // 4x4 tilesheet
        float position[3] = {pos[i].x,pos[i].y,pos[i].y}, offset[2]={0,0}, scale[2]={0.5,0.5};
        float spritesheet[3]={num_frame,4,4};
        sprite_sheet(kids,
            spritesheet,        // num_frame in a 4x4 spritesheet
            position, 0,        // position(x,y,depth:sort-by-Y), angle
            offset, scale,      // offset(x,y), scale(x,y)
            false, WHITE, false // is_additive, tint color, resolution-independent
        );
    }
}

void demo_hud() {
    // draw pixel-art hud, 16x16 ui element, scaled and positioned in resolution-independant way
    static texture_t inputs; do_once inputs = texture( "prompts_tilemap_34x24_16x16x1.png", TEXTURE_LINEAR );
    float spritesheet[3] = {17,34,24}, offset[2] = {0, - 2*absf(sin(window_time()*5))}; // sprite cell and animation
    float scale[2] = {3, 3}, tile_w = 16 * scale[0], tile_h = 16 * scale[1]; // scaling
    float position[3] = {window_width() - tile_w, window_height() - tile_h, window_height() }; // position in screen-coordinates (x,y,z-index)
    sprite_sheet(inputs, spritesheet, position, 0/*deg*/, offset, scale, false, WHITE, 1);
}

int main() {
    // window (80% sized, MSAA x4 flag)
    window_create(80.0, WINDOW_MSAA4);
    window_title(__FILE__);

    // tiled map
    tiled_t tmx = tiled(vfs_read("castle.tmx"));
    // tmx.parallax = true;

    // spine model
    //spine_t *spn = spine("goblins.json", "goblins.atlas", 0);

    // camera 2d
    camera_t cam = camera();
    cam.position = vec3(window_width()/2, window_height()/2, 3); // at(CX,CY) zoom(x3)
    camera_enable(&cam);

    // audio (both clips & streams)
    audio_t clip1 = audio_clip( "coin.wav" );
    audio_t clip2 = audio_clip( "pew.sfxr" );
    audio_t stream1 = audio_stream( "larry.mid" );
    audio_t stream2 = audio_stream( "waterworld-map.fur" );
    audio_t BGM = stream1;
    audio_play(BGM, 0);

    // font config: faces (6 max) and colors (10 max)
    #define FONT_CJK       FONT_FACE3
    #define FONT_YELLOW    FONT_COLOR2
    #define FONT_LIME      FONT_COLOR3
    font_face(FONT_CJK,      "mplus-1p-medium.ttf", 48.f, FONT_JP|FONT_2048); // CJK|FONT_2048|FONT_OVERSAMPLE_Y);
    font_color(FONT_YELLOW,  RGB4(255,255,0,255));
    font_color(FONT_LIME,    RGB4(128,255,0,255));

    // fx: load all post fx files in all subdirs. enable a few filters by default
    fx_load("fx**.fs");
    fx_enable(fx_find("fxCRT2.fs"), 1);
    fx_enable(fx_find("fxGrain.fs"), 1);
    fx_enable(fx_find("fxContrast.fs"), 1);
    fx_enable(fx_find("fxVignette.fs"), 1);

    // demo loop
    while (window_swap() && !input_down(KEY_ESC)) {

        // handle input
        if( input_down(KEY_F5) ) window_reload();
        if( input_down(KEY_F11) ) window_fullscreen( !window_has_fullscreen() );

        // camera panning (x,y) & zooming (z)
        if( !ui_hover() && !ui_active() ) {
            if( input(MOUSE_L) ) cam.position.x += input_diff(MOUSE_X);
            if( input(MOUSE_L) ) cam.position.y += input_diff(MOUSE_Y);
            cam.position.z += input_diff(MOUSE_W) * 0.1;
        }

        // apply post-fxs from here
        fx_begin();

            profile("Rendering") {
                vec3 center = add3(cam.position,vec3(-window_width()/1,-window_height()/2,0));
                // render tiled map
                tiled_render(tmx, center);
                //
                demo_kids(vec3(0,0,1));
                demo_hud();
                // render spine model
                // spine_animate(spn, !window_has_pause() * window_delta());
                // spine_render(spn, vec3(cam.position.x, cam.position.y, 1), true );
                // sprite_flush();
            }

            // subtitle sample
            font_print(
                FONT_BOTTOM FONT_CENTER
                FONT_CJK FONT_H1
                FONT_YELLOW "私はガラスを食べられます。" FONT_LIME "それは私を傷つけません。\n"
            );

        // post-fxs end here
        fx_end();

        // ui
        if( ui_panel("Audio", 0)) {
            if( ui_label2_toolbar("BGM: Track #1", ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = stream1, AUDIO_SINGLE_INSTANCE);
            if( ui_label2_toolbar("BGM: Track #2", ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = stream2, AUDIO_SINGLE_INSTANCE);
            if( ui_label2_toolbar("SFX: Coin", ICON_MD_VOLUME_UP)) audio_play(clip1, 0);
            if( ui_label2_toolbar("SFX: Pew", ICON_MD_VOLUME_UP)) audio_play(clip2, 0);
            ui_panel_end();
        }
        if( ui_panel("Tiled", 0)) {
            ui_float("Zoom in", &cam.position.z);
            tiled_ui(&tmx);
            ui_panel_end();
        }
        /*if( ui_panel("Spine", 0)) {
            spine_ui(spn);
            ui_panel_end();
        }*/
    }
}

// this demo supersedes following old sources:
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-audio.c
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-font.c
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-spine.c
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-sprite.c
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-tiled.c
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-tilemap.c
