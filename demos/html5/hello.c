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

#include "fwk.h"

// options
unsigned no_flags = 0;
bool do_debugdraw = 0;

// load skybox: launch with --mie for rayleigh/mie scattering
skybox_t sky;

// animated models loading
model_t girl;

// camera
camera_t cam;

void gameloop(void *userdata)
{
    // input
    if( input_down(KEY_ESC) && !is(ems) ) exit(0);
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
    //fx_begin();

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
        model_render(girl, cam.proj, cam.view, girl.pivot, no_flags);

    // post-fxs end here
    //fx_end();

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

        ui_panel_end();
    }
}

int main() {
    // window (80% sized, no MSAA)
    window_create(80, 0);
    window_title(__FILE__);

    // load skybox: launch with --mie for rayleigh/mie scattering
    sky = skybox(flag("--mie") ? 0 : "cubemaps/stardust", no_flags);

    // animated models loading
    girl = model("kgirl/kgirls01.fbx", no_flags);
    compose44( girl.pivot, vec3(0,0,0), eulerq(vec3(-90,0,0)), vec3(2,2,2)); // position, rotation, scale

    // camera
    cam = camera();

    // demo loop
    window_loop(gameloop, NULL);
}
