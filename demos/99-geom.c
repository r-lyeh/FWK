#include "engine.h"

int main() {
    // 75% sized, MSAAx2
    window_create(25, WINDOW_SQUARE);
    window_title(__FILE__);

    unsigned program = shader_geom(vfs_read("shaders/geom-test/line.glsl"), vfs_read("shaders/geom-test/line-vs.glsl"), vfs_read("shaders/geom-test/line-fs.glsl"), "aPos", "FragColor", NULL);

    float points[] = {
        -0.5f,  0.5f, // top-left
         0.5f,  0.5f, // top-right
         0.5f, -0.5f, // bottom-right
        -0.5f, -0.5f  // bottom-left
    };

    mesh_t m = mesh();
    mesh_update(&m, "p2", 0, 4, points, 0, 0, 0);

    while ( window_swap() && !input_down(KEY_ESC) ){
        if (input(KEY_F5)) window_reload();

        shader_bind(program);
        shader_float("t", (float)window_time());
        mesh_render_prim(&m, GL_POINTS);
    }

    return 0;
}
