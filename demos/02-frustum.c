#include "fwk.h"

void ddraw_camera(camera_t *c) {
    vec3 center = c->position;
//    ddraw_prism(add3(center,vec3(2,0,0)), 0.5, 1, vec3(-1,0,0), 4); // center,radius,height,normal,segments
//    ddraw_box(center, vec3(2,2,1)); // center,extents

    mat33 r; rotationq33(r, eulerq(vec3(-c->yaw,-c->pitch,0)));
    ddraw_cube33(center, vec3(2,2,2), r);

    ddraw_circle(add3(center,vec3(+1,1,0)), vec3(0,0,1), 0.8); // pos,normal,radius
    ddraw_circle(add3(center,vec3(-1,1,0)), vec3(0,0,1), 0.8); // pos,normal,radius

    mat44 projview; multiply44x2(projview, c->proj, c->view);
    ddraw_frustum(projview);
}

int main() {
    window_create(0.75, 0);

    camera_t cam = camera();
    camera_t cam2 = camera();

    int spin = 1;

    while( window_swap() ) {
        if(input_down(KEY_SPACE)) spin^=1;
        // spin 2nd camera
        double t = window_time(), c = cos(t), s = sin(t);
        if(spin)
        camera_teleport(&cam2, vec3(c * 100, 100, s * 100));
        camera_lookat(&cam2, vec3(0,0,0));

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdecq.x,wasdecq.y,wasdecq.z);
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

        mat44 projview; multiply44x2(projview, cam2.proj, cam2.view);
        frustum f = frustum_build(projview);

        ddraw_ground(0);
        ddraw_camera(&cam2);

        int drawn = 0, total = 0;
        for(int z = -300; z < 300; z += 5) {
            for(int x =  -300; x < 300; x += 5) {
                vec3 min = vec3(x, 0, z);
                vec3 max = add3(min, vec3(2.5,2.5,2.5));

                if( frustum_test_aabb(f, aabb(min, max)) ) {
                    ddraw_aabb( min, max );
                    ++drawn;
                }
                ++total;
            }
        }

        font_print(va(FONT_RIGHT "%d/%d cubes drawn", drawn, total));
    }
}
