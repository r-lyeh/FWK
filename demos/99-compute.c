#include "fwk.h"

int main() {
    // 75% sized, MSAAx2
    window_create(50, WINDOW_SQUARE);
    window_title(__FILE__);

    unsigned TEX_WIDTH = 1000;

    unsigned comp = compute(vfs_read("shaders/compute-test.glsl"));
    texture_t tex = texture_create(TEX_WIDTH, TEX_WIDTH, 4, 0, TEXTURE_LINEAR|TEXTURE_FLOAT);
    shader_bind(comp);
    shader_image(tex, 0, 0, 0, BUFFER_READ);

    struct {
        float f;
    } data;

    unsigned buf = ssbo_create(&data, sizeof(data), STREAM_DRAW);

    while ( window_swap() && !input_down(KEY_ESC) ){
        if (input(KEY_F5)) window_reload();
        
        shader_bind(comp);
        data.f = (float)window_time();
        ssbo_bind(buf, 1);
        ssbo_update(0, sizeof(data), &data);
        
        compute_dispatch(TEX_WIDTH/10, TEX_WIDTH/10, 1);
        write_barrier_image();
        
        fullscreen_quad_rgb(tex);
    }

    return 0;
}
