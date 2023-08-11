// networked gui demo
// - rlyeh, public domain

#include "fwk.h"

#define OSCPACK_C
#define OSCRECV_C
#define OSCSEND_C
#include "oscpack.h"
#include "oscrecv.h"
#include "oscsend.h"



int main() {
    // window (80% sized, MSAA x2 flag)
    window_create(80, WINDOW_MSAA2);
    window_title(__FILE__);

    // scene loading
    #define SCENE(...) #__VA_ARGS__
    const char *my_scene = SCENE([
        {
            skybox: 'cubemaps/stardust/',
        },
        {
            position:[-5.0,-2.0,2.0],
            rotation: [90.0,0.0,180.0],
            scale:0.20,
            mesh:'models/witch/witch.obj',
            texture:'models/witch/witch_diffuse.tga.png',
            flipuv:false,
        },
        {
            position:[-5.0,-2.0,2.0],
            rotation: [90.0,0.0,180.0],
            scale:0.20,
            mesh:'models/witch/witch_object.obj',
            texture:'models/witch/witch_object_diffuse.tga.png',
            flipuv:false,
        },
    ]);
    int num_spawned = scene_merge(my_scene);
    object_t *obj1 = scene_index(0);
    object_t *obj2 = scene_index(1);

    // camera
    camera_t cam = camera();
    cam.speed = 0.2f;

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdecq.x,wasdecq.y,wasdecq.z);
        camera_fps(&cam, mouse.x,mouse.y);

        // queue model scale bounces
        float t = fmod(window_time(), 0.3) / 0.3;
        float s = 0.01f * ease_ping_pong(t, ease_in_cubic,ease_out_cubic);
        object_scale(obj1, vec3(0.20f - s,0.20f + s,0.20f - s));
        object_scale(obj2, vec3(0.20f - s,0.20f + s,0.20f - s));

        // flush render scene (background objects: skybox)
        profile("Scene background") {
            scene_render(SCENE_BACKGROUND);
        }

        // queue debug drawcalls
        profile("Debugdraw") {
            ddraw_grid(0);
            ddraw_color(YELLOW);
            ddraw_text(vec3(+1,+1,-1), 0.04f, va("(%f,%f,%f)", cam.position.x,cam.position.y,cam.position.z));
            ddraw_color(YELLOW);
            ddraw_flush();
        }

        // render scene (foreground objects) with post-effects
        profile("Scene foreground") {
            int scene_flags = 0;
            scene_render(SCENE_FOREGROUND | scene_flags);
        }

#if 0
        do_once ui_netconfig(1234);
        ui_netupdate();
        static int r = 0, g = 0, b = 0; 
        viewport_color3(vec3(r/255.0,g/255.0,b/255.0));
        if( ui_panel("net test1", 0) ) {
            ui_netint("Color R", &r); r = clampi(r, 0, 255);
            ui_netint("Color G", &g); g = clampi(g, 0, 255);
            ui_netint("Color B", &b); b = clampi(b, 0, 255);
            ui_panel_end();
        }
#endif
    }
}
