// instanced models demo
// - rlyeh, public domain.

#include "fwk.h"

int main() {
    window_create(75, WINDOW_MSAA2);
    window_title(__FILE__);

    camera_t cam = camera();
    skybox_t sky = skybox("cubemaps/stardust", 0);
    model_t girl = model("kgirls01.fbx", 0);

    while( window_swap() ) {
        if(input(KEY_F5)) window_reload();
        if(input(KEY_ESC)) break;

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdec = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-input(KEY_C),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdec.x,wasdec.y,wasdec.z);
        camera_fps(&cam, mouse.x,mouse.y);

        // ground rendering
        ddraw_ground(0);
        ddraw_flush();

        // skeletal
        profile("Skeletal update") {
            float delta = window_has_pause() ? 0 : window_delta() * 30; // 30fps anim
            girl.curframe = model_animate(girl, girl.curframe + delta);
        }

        profile("Skeletal render") {
            enum { ROW = 32, MAX_INSTANCES = ROW * ROW };
            static mat44 M[MAX_INSTANCES];

            do_once {
                int i = 0;
                for(int z = 0; z < ROW; ++z) {
                    for(int x = 0; x < ROW; ++x, ++i) {
                        vec3 p = vec3(-x*3,0,-z*3);
                        vec3 r = vec3(0,0,0);
                        vec3 s = vec3(2,2,2);
                        rotationq44(M[i], eulerq(r)); scale44(M[i], s.x,s.y,s.z); relocate44(M[i], p.x,p.y,p.z);
                    }
                }
            }

            model_render_instanced(girl, cam.proj, cam.view, M, 0, MAX_INSTANCES);
        }

        // skybox
        profile("Skybox") {
            skybox_render(&sky, cam.proj, cam.view);
        }
    }
}
