// framework demo
// - rlyeh, public domain

#include "fwk.h"

int main() {
    // options
    bool  do_about = 0;
    float do_scale = 0.10f;
    bool  do_debugdraw = 0;
    float do_gamepad_deadzone = 0.15f;
    vec2  do_gamepad_polarity = vec2(+1,+1);
    vec2  do_gamepad_sensitivity = vec2(0.1f,0.1f);
    vec2  do_mouse_polarity = vec2(+1,-1);
    vec2  do_mouse_sensitivity = vec2(0.2f,0.2f);
    bool  do_billboard_x = 0, do_billboard_y = 0, do_billboard_z = 0;

    // window (80% sized, MSAA x4 flag)
    window_create(80, WINDOW_MSAA4);
    window_title(__FILE__);

    // load all fx files, including subdirs
    fx_load("fx**.fs");

    // load skybox
    skybox_t sky = skybox(flag("--mie") ? 0 : "cubemaps/stardust", 0); // --mie for rayleigh/mie scattering

    // load static scene
    model_t sponza;
    int do_sponza = flag("--sponza");
    if( do_sponza ) {
        sponza = model("sponza.obj", 0); // MODEL_NO_TEXTURES);
        translation44(sponza.pivot, 0,-1,0);
        rotate44(sponza.pivot, -90,1,0,0);
        scale44(sponza.pivot, 10,10,10);
    }

    model_t shaderball;
    int do_shaderball = flag("--shaderball");
    if( do_shaderball ) {
        shaderball = model("shaderball.glb", 0);
        translation44(shaderball.pivot, 0,0,-10);
        rotate44(shaderball.pivot, -90,1,0,0);
        scale44(shaderball.pivot, 0.02,0.02,0.02);
    }

    // animated models loading
    int model_flags = flag("--matcaps") ? MODEL_MATCAPS : 0;
    model_t girl = model("kgirl/kgirls01.fbx", model_flags);
    model_t alien = model("alien/alien_helmet.fbx", model_flags); rotation44(alien.pivot, -90,1,0,0);
    model_t george = model("robots/george.fbx", model_flags);
    model_t leela = model("robots/leela.fbx", model_flags);
    model_t mike = model("robots/mike.fbx", model_flags);
    model_t stan = model("robots/stan.fbx", model_flags);
    model_t robots[4] = { george, leela, mike, stan };
    for( int i = 0; i < countof(robots); ++i ) {
        rotation44(robots[i].pivot, -90,1,0,0);
    }

    if( flag("--matcaps") ) {
        // patch models to use matcaps
        model_set_texture(george, texture("matcaps/3B6E10_E3F2C3_88AC2E_99CE51-256px", 0)); // green
        model_set_texture(leela,  texture("matcaps/39433A_65866E_86BF8B_BFF8D8-256px", 0));
        model_set_texture(mike,   texture("matcaps/394641_B1A67E_75BEBE_7D7256-256px.png", 0));
        model_set_texture(stan,   texture("matcaps/test_steel", 0));
        model_set_texture(girl,   texture("matcaps/material3", 0));
        model_set_texture(alien,  texture("matcaps/material3", 0));

        if( flag("--shaderball") )
        model_set_texture(shaderball, texture("matcaps/normals", 0));
    }

    // camera
    camera_t cam = camera();

    // audio (both clips & streams)
    audio_t SFX1 = audio_clip( "coin.wav" );
    audio_t SFX2 = audio_clip( "pew.sfxr" );
    audio_t BGM1 = audio_stream( "waterworld-map.fur"); // wrath_of_the_djinn.xm" );
    audio_t BGM2 = audio_stream( "larry.mid" );
    audio_t BGM3 = audio_stream( "monkey1.mid" ), BGM = BGM1;
    audio_play(SFX1, 0);
    audio_play(BGM, 0);

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;
        if( input_down(KEY_F5) ) window_reload();
        if( input_down(KEY_W) && input_held(KEY_LCTRL) ) break;
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );
        if( input_down(KEY_X) ) window_screenshot(__FILE__ ".png");
        if( input_down(KEY_Z) ) window_record(__FILE__ ".mp4");

        // vec2 filtered_lpad = input_filter_deadzone(input2(GAMEPAD_LPAD), do_gamepad_deadzone + 1e-3);
        // vec2 filtered_rpad = input_filter_deadzone(input2(GAMEPAD_RPAD), do_gamepad_deadzone + 1e-3);

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

        // queue debug drawcalls
        profile("Debugdraw") {
            ddraw_grid(0);
            ddraw_color(YELLOW);
            ddraw_text(vec3(+1,+1,-1), 0.04f, va("(%f,%f,%f)", cam.position.x,cam.position.y,cam.position.z));
            if(do_debugdraw) ddraw_demo(); // showcase many debugdraw shapes
            ddraw_color(YELLOW);
            ddraw_flush();
        }

        // draw skybox
        profile("Skybox") {
            skybox_render(&sky, cam.proj, cam.view);
        }

        profile("Skeletal update") if(!window_has_pause()) {
            float delta = window_delta() * 30; // 30fps anim

            // animate girl & alien
            girl.curframe = model_animate(girl, girl.curframe + delta);
            alien.curframe = model_animate(alien, alien.curframe + delta);

            // animate robots
            for(int i = 0; i < countof(robots); ++i) {
                robots[i].curframe = model_animate(robots[i], robots[i].curframe + delta);
            }
        }

        profile("Skeletal render") {
            static vec3 p = {-10,0,-10}, r = {0,0,0}, s = {2,2,2};
            gizmo(&p, &r, &s);
            mat44 M; rotationq44(M, eulerq(r)); scale44(M, s.x,s.y,s.z); relocate44(M, p.x,p.y,p.z);

            model_render(girl, cam.proj, cam.view, M, 0);

            aabb box = model_aabb(girl, M);
            ddraw_color(YELLOW);
            ddraw_aabb(box.min, box.max);
        }

        profile("Skeletal render") {
            static vec3 p = {+10,0,-10}, r = {0,-90,0}, s = {1,1,1};
            //gizmo(&p, &r, &s);
            mat44 M; rotationq44(M, eulerq(r)); scale44(M, s.x,s.y,s.z); relocate44(M, p.x,p.y,p.z);

            model_render(alien, cam.proj, cam.view, M, 0);

            aabb box = model_aabb(alien, M); // @fixme: neg Y
            ddraw_color(YELLOW);
            //ddraw_aabb(box.min, box.max);
        }

        profile("Skeletal render") for(int i = 0; i < countof(robots); ++i) {
            float scale = 0.50;
            mat44 M; copy44(M, robots[i].pivot); translate44(M, i*3,0,0); scale44(M, scale,scale,scale);
            model_render(robots[i], cam.proj, cam.view, M, 0);
        }

        if(do_sponza) profile("Sponza") {
            float scale = 1.00;
            mat44 M; copy44(M, sponza.pivot); translate44(M, 0,0,0); scale44(M, scale,scale,scale);
            model_render(sponza, cam.proj, cam.view, M, 0);
        }

        if(do_shaderball) profile("Shaderball") {
            float scale = 1.00;
            mat44 M; copy44(M, shaderball.pivot); translate44(M, 0,0,0); scale44(M, scale,scale,scale);
            model_render(shaderball, cam.proj, cam.view, M, 0);
        }

        // post-fxs end here
        fx_end();

        // font demo
        do_once font_scales(FONT_FACE1, 48, 24, 18, 12, 9, 6);
        font_print(va(FONT_RIGHT FONT_BOTTOM FONT_H4 "%5.2f FPS", window_fps()));

        // queue ui
        if( ui_panel("App", 0)) {
            if(ui_bool("Show debugdraw demo", &do_debugdraw)) {}
            if(ui_separator()) {}
            if(ui_slider("Gamepad deadzone", &do_gamepad_deadzone)) {}
            if(ui_float2("Gamepad polarity", do_gamepad_polarity.v2)) {}
            if(ui_float2("Gamepad sensitivity", do_gamepad_sensitivity.v2)) {}
            if(ui_separator()) {}
            if(ui_float2("Mouse polarity", do_mouse_polarity.v2)) {}
            if(ui_float2("Mouse sensitivity", do_mouse_sensitivity.v2)) {}
            if(ui_separator()) {}
            if(ui_button("About...")) { do_about = 1; audio_play(SFX1, 0); }
            if(ui_dialog("About", __FILE__ "\n" __DATE__ "\n" "Public Domain.", 0, &do_about)) {}
            ui_panel_end();
        }
        if( ui_panel("Audio", 0)) {
            static float bgm = 1, sfx = 1, master = 1;
            if( ui_slider2("BGM", &bgm, va("%.2f", bgm))) audio_volume_stream(bgm);
            if( ui_slider2("SFX", &sfx, va("%.2f", sfx))) audio_volume_clip(sfx);
            if( ui_slider2("Master", &master, va("%.2f", master))) audio_volume_master(master);
            if( ui_label2_toolbar("BGM: Waterworld Map" /*Wrath of the Djinn"*/, ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = BGM1, AUDIO_SINGLE_INSTANCE);
            if( ui_label2_toolbar("BGM: Leisure Suit Larry", ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = BGM2, AUDIO_SINGLE_INSTANCE);
            if( ui_label2_toolbar("BGM: Monkey Island", ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = BGM3, AUDIO_SINGLE_INSTANCE);
            if( ui_label2_toolbar("SFX: Coin", ICON_MD_VOLUME_UP)) audio_play(SFX1, 0);
            if( ui_label2_toolbar("SFX: Pew", ICON_MD_VOLUME_UP)) audio_play(SFX2, 0);
            ui_panel_end();
        }

        ui_demo(1); // show all UI widgets in a tab
    }

    // data tests (json5)
    const char json5[] =
    "  /* json5 */ // comment\n"
    "  abc: 42.67, def: true, integer:0x100 \n"
    "  huge: 2.2239333e5, \n"
    "  hello: 'world /*comment in string*/ //again', \n"
    "  children : { a: 1, b: 2, c: 3 },\n"
    "  array: [+1,2,-3,4,5],    \n"
    "  invalids : [ nan, NaN, -nan, -NaN, inf, Infinity, -inf, -Infinity ],";
    if( json_push(json5) ) {
        assert( json_float("/abc") == 42.67 );
        assert( json_int("/def") == 1 );
        assert( json_int("/integer") == 0x100 );
        assert( json_float("/huge") > 2.22e5 );
        assert( strlen(json_string("/hello")) == 35 );
        assert( json_int("/children/a") == 1 );
        assert( json_int("/children.b") == 2 );
        assert( json_int("/children[c]") == 3 );
        assert( json_int("/array[%d]", 2) == -3 );
        assert( json_count("/invalids") == 8 );
        assert( isnan(json_float("/invalids[0]")) );
        assert( !json_find("/non_existing") );
        assert( PRINTF("json5 tests OK\n") );
        json_pop();
    }

    // data tests (xml)
    const char *xml = vfs_read("test1.xml");
    if( xml_push(xml) ) {
        puts( xml );
        puts( xml_string("/person/firstName/$") );
        puts( xml_string("/person/lastName/$") );
        puts( xml_string("/person/address/@type") );
        xml_pop();
    }

    // network test (https)
    array(char) webfile = download("https://www.google.com/");
    printf("Network test: %d bytes downloaded from google.com\n", array_count(webfile));

    // script test (lua)
    script_run( "-- Bye.lua\nio.write(\"script test: Bye world!, from \", _VERSION, \"\\n\")" );
}
