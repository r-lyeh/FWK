// playground tests for FWK
// - rlyeh, public domain
//
// # quickstart
// - win/vc       : cl hello.c
// - win/clang-cl : clang-cl  hello.c
// - win/tcc      : tools\tcc hello.c -m64
// - win/mingw    : gcc   hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
// - win/clang    : clang hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
// - linux        : cc  hello.c -lm -ldl -lpthread -lX11
// - linux/tcc    : tcc hello.c -lm -ldl -lpthread -lX11 -D__STDC_NO_VLA__
// - osx          : cc -ObjC hello.c -framework cocoa -framework iokit -framework audiotoolbox

#define FWK_IMPLEMENTATION      // unrolls single-header implementation
#include "engine/joint/fwk.h"   // single-header file

int main() {
    // options
    bool do_debugdraw = 0;

    // window (80% sized, MSAA x4 flag)
    window_create(80, WINDOW_MSAA4);
    window_title(__FILE__);

    // load skybox: launch with --mie for rayleigh/mie scattering (no flags)
    skybox_t sky = skybox(flag("--mie") ? 0 : "cubemaps/stardust", 0);

    // animated models loading (no flags)
    model_t girl = model("kgirl/kgirls01.fbx", 0);
    compose44( girl.pivot, vec3(0,0,0), eulerq(vec3(-90,0,0)), vec3(2,2,2)); // position, rotation, scale

    // camera
    camera_t cam = camera();

    // fx: load all post fx files in all subdirs.
    fx_load("fx**.fs");

    // audio (both clips & streams)
    audio_t SFX1 = audio_clip( "coin.wav" ), SFX2 = audio_clip( "pew.sfxr" );
    audio_t BGM1 = audio_stream( "waterworld-map.fur" ), BGM2 = audio_stream( "larry.mid" ), BGM = BGM1;
    audio_play(SFX1, 0);
    audio_play(BGM, 0);

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;
        if( input_down(KEY_F5) ) window_reload();
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );
        if( input_down(KEY_X) && input_held(KEY_LALT) ) window_screenshot(__FILE__ ".png");
        if( input_down(KEY_Z) && input_held(KEY_LALT) ) window_record(__FILE__ ".mp4");

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, wasdecq);
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

        // apply post-fxs from here
        fx_begin();

            // debug draw
            ddraw_ground(0);
            if(do_debugdraw) ddraw_demo(); // showcase many debugdraw shapes
            ddraw_flush();

            // draw skybox
            skybox_render(&sky, cam.proj, cam.view);

            // animate girl
            float delta = window_delta() * 30; // 30fps anim
            girl.curframe = model_animate(girl, girl.curframe + delta);

            // draw girl
            model_render(girl, cam.proj, cam.view, girl.pivot, 0);

        // post-fxs end here
        fx_end();

        // font demo
        font_print(va(FONT_BOTTOM FONT_RIGHT FONT_H6 "%5.2f FPS", window_fps()));

        // draw ui demo (true=showcase windows)
        ui_demo(true);

        // draw ui
        if( ui_panel("App ", PANEL_OPEN))
        {
            ui_section("DebugDraw");
            if( ui_bool("Show debugdraw demo", &do_debugdraw) ) {}

            ui_section("Script");
            if( ui_button("Test Lua") ) script_run("ui_notify(nil, \"Hello from Lua! Version: \" .. _VERSION)");

            ui_section("Camera");
            if( ui_float("Speed", &cam.speed) ) {}
            if( ui_float3("Position", cam.position.v3) ) {}

            ui_section("Audio");
            static float bgm = 1, sfx = 1, master = 1;
            if( ui_slider2("BGM", &bgm, va("%.2f", bgm))) audio_volume_stream(bgm);
            if( ui_slider2("SFX", &sfx, va("%.2f", sfx))) audio_volume_clip(sfx);
            if( ui_slider2("Master", &master, va("%.2f", master))) audio_volume_master(master);
            if( ui_label2_toolbar("BGM: Waterworld Map", ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = BGM1, 0);
            if( ui_label2_toolbar("BGM: Leisure Suit Larry", ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = BGM2, 0);
            if( ui_label2_toolbar("SFX: Coin", ICON_MD_VOLUME_UP)) audio_play(SFX1, 0);
            if( ui_label2_toolbar("SFX: Pew", ICON_MD_VOLUME_UP)) audio_play(SFX2, 0);

            ui_panel_end();
        }
    }

    data_tests();
    script_tests();
    network_tests();
}
