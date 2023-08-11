#include "fwk.h"

int main() {
    window_create(0.75, 0);

    camera_t cam = camera();
    camera_t cam2 = camera();

    int spin = 1;

    while( window_swap() ) {
        // controls
        if(input_down(KEY_SPACE)) spin^=1;

        // setup scene
        cam.fov = 30;
        cam.position = vec3(180,180,180);
        camera_enable(&cam);

        // spin 2nd camera
        double t = window_time(), c = cos(t), s = sin(t);
        if(spin)
        camera_teleport(&cam2, vec3(c * 100, 100, s * 100));
        camera_lookat(&cam2, vec3(0,0,0));

        // render world (ground and cubes only in frustum of cam2)
        mat44 projview; multiply44x2(projview, cam2.proj, cam2.view);
        frustum f = frustum_build(projview);

        ddraw_ground(0);
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

        // debug
        ddraw_camera(&cam2);
        font_print(va(FONT_RIGHT "%d/%d cubes drawn", drawn, total));
    }
}
