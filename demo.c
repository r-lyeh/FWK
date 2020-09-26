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

    // load all fx files
    for(const char **list = vfs_list("fx**.fs"); *list; list++) {
        fx_load(*list);
    }

    // load skybox
    skybox_t sky = skybox("cubemaps/stardust", 0); // NULL, 0); for rayleigh/mei scattering

    // animated models loading
    model_t custom = model("kgirl/kgirls01.fbx", 0);
    model_t george = model("robots/george.fbx", 0);
    model_t leela = model("robots/leela.fbx", 0);
    model_t mike = model("robots/mike.fbx", 0);
    model_t stan = model("robots/stan.fbx", 0);
    model_t robots[4] = { george, leela, mike, stan };
    for( int i = 0; i < countof(robots); ++i ) {
        rotation44(robots[i].pivot, -90,1,0,0);
    }

    // camera
    camera_t cam = camera();
    cam.speed = 0.2f;

    // audio (both clips & streams)
    audio_t voice = audio_clip("coin.wav");
    audio_t stream = audio_stream("wrath_of_the_djinn.xm"); // "larry.mid"
    audio_play(voice, 0);
    audio_play(stream, 0);

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;
        if( input_down(KEY_F5) ) app_reload();
        if( input_down(KEY_W) && input_held(KEY_LCTRL) ) break;
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );
        if( input_down(KEY_Z) ) window_screenshot(__FILE__ ".png");
        if( input_down(KEY_V) ) window_videorec(window_has_videorec() ? 0 : __FILE__ ".mpg");

        // vec2 filtered_lpad = input_filter_deadzone(input2(GAMEPAD_LPAD), do_gamepad_deadzone + 1e-3);
        // vec2 filtered_rpad = input_filter_deadzone(input2(GAMEPAD_RPAD), do_gamepad_deadzone + 1e-3);

        // fps camera
        bool active = ui_active() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdecq.x,wasdecq.y,wasdecq.z);
        camera_fps(&cam, mouse.x,mouse.y);

        // queue debug drawcalls
        profile(Debugdraw) {
            ddraw_grid(0);
            ddraw_color(YELLOW);
            ddraw_text(vec3(+1,+1,-1), 0.04f, "(%f,%f,%f)", cam.position.x,cam.position.y,cam.position.z);
            if(do_debugdraw) ddraw_demo(); // showcase many debugdraw shapes
            ddraw_color(YELLOW);
            ddraw_flush();
        }

        // draw skybox
        profile(Skybox) {
            skybox_push_state(&sky, cam.proj, cam.view);
            glEnable(GL_DEPTH_TEST);
            mesh_render(&sky.geometry);
            skybox_pop_state(&sky);
        }

        profile(Editor) {
            editor();
        }

        // apply post-fxs from here
        fx_begin();

        profile(Skeletal update) if(!window_has_pause()) {
            float delta = window_delta() * 30; // 30fps anim

            // animate girl
            custom.curframe = model_animate(custom, custom.curframe + delta);

            // animate robots
            for(int i = 0; i < countof(robots); ++i) {
                robots[i].curframe = model_animate(robots[i], robots[i].curframe + delta);
            }
        }

        profile(Skeletal render) {
            static vec3 p = {-10,0,-10}, r = {0,0,0}, s = {2,2,2};
            gizmo(&p, &r, &s);
            mat44 M; rotationq44(M, eulerq(r)); scale44(M, s.x,s.y,s.z); relocate44(M, p.x,p.y,p.z);

            model_render(custom, cam.proj, cam.view, M); // custom.pivot);

            aabb box = model_aabb(custom, M); // custom.pivot);
            ddraw_color(YELLOW);
            ddraw_aabb(box.min, box.max);
        }

        profile(Skeletal render) for(int i = 0; i < countof(robots); ++i) {
            float scale = 0.50; // 0.025;
            mat44 M; copy44(M, robots[i].pivot); translate44(M, i*3,0,0); scale44(M, scale,scale,scale);
            model_render(robots[i], cam.proj, cam.view, M);
        }

        // post-fxs end here
        fx_end();

        // queue ui
        if( ui_begin("App", 0)) {
            if(ui_bool("Show debugdraw demo", &do_debugdraw)) {}
            if(ui_separator()) {}
            if(ui_slider("Gamepad deadzone", &do_gamepad_deadzone)) {}
            if(ui_float2("Gamepad polarity", &do_gamepad_polarity.x)) {}
            if(ui_float2("Gamepad sensitivity", &do_gamepad_sensitivity.x)) {}
            if(ui_separator()) {}
            if(ui_float2("Mouse polarity", &do_mouse_polarity.x)) {}
            if(ui_float2("Mouse sensitivity", &do_mouse_sensitivity.x)) {}
            if(ui_separator()) {}
            if(ui_button("About...")) { do_about = 1; audio_play(voice, 0); }
            if(ui_dialog("About", __FILE__ "\n" __DATE__ "\n" "Public Domain.", 0, &do_about)) {}
            ui_end();
        }
        if( ui_begin("Camera", 0)) {
            if( ui_float("Speed", &cam.speed) ) {}
            if( ui_float3("Position", &cam.position.x) ) {}
            ui_end();
        }
        if( ui_begin("FX", 0) ) {
            for( int i = 0; i < 64; ++i ) {
                char *name = fx_name(i); if( !name ) break;
                bool b = fx_enabled(i);
                if( ui_bool(name, &b) ) fx_enable(i, fx_enabled(i) ^ 1);
            }
            ui_end();
        }
        if( ui_begin("Audio", 0)) {
            static float fx = 1, bgm = 1, master = 1;
            if( ui_slider2("FX", &fx, stringf("%.2f", fx))) audio_volume_clip(fx);
            if( ui_slider2("BGM", &bgm, stringf("%.2f", bgm))) audio_volume_stream(bgm);
            if( ui_slider2("Master", &master, stringf("%.2f", master))) audio_volume_master(master);
            if( ui_button("Coin FX") ) audio_play(voice, 0);
            ui_end();
        }

        input_demo(); // show some keyboard/mouse/gamepad UI tabs
        ui_demo(); // show all UI widgets in a tab
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
    if( data_push(json5) ) {
        assert( data_float("/abc") == 42.67 );
        assert( data_int("/def") == 1 );
        assert( data_int("/integer") == 0x100 );
        assert( data_float("/huge") > 2.22e5 );
        assert( strlen(data_string("/hello")) == 35 );
        assert( data_int("/children/a") == 1 );
        assert( data_int("/children.b") == 2 );
        assert( data_int("/children[c]") == 3 );
        assert( data_int("/array[%d]", 2) == -3 );
        assert( data_count("/invalids") == 8 );
        assert( isnan(data_float("/invalids[0]")) );
        assert( ~puts("json5 tests: ok") );
        data_pop();
    }

    // network test (https)
    FILE *webfile = tmpfile();
    download(webfile, "https://www.google.com/");
    printf("Network test: %d bytes downloaded from google.com\n", (int)ftell(webfile));

    // script test (lua)
    script_run( "-- Bye.lua\nio.write(\"script test: Bye world!, from \", _VERSION, \"\\n\")" );
}
