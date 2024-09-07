#include "fwk.h"

int main() {
    window_create(0.75, 0);

    camera_t cam = camera();
    model_t mdl = model("meshes/parallaxTest.obj", 0);
    skybox_t sky = skybox(0, 0);
    light_t sun = light();
    sun.type = LIGHT_POINT;
    sun.pos = vec3(0.5f, 1.0f, 0.5f);

    model_light(&mdl, 1, &sun);
    model_cubemap(&mdl, &sky.cubemap);
    
    while(window_swap() && !input(KEY_ESC)) {
        camera_freefly(&cam);
        skybox_render(&sky, cam.proj, cam.view);
        model_render(mdl, cam.proj, cam.view, mdl.pivot);

        if (ui_panel("Light", 0)) {
            ui_light(&sun); 
            ui_panel_end();
        }
        if (ui_panel("Materials", 0)) {
            ui_materials(&mdl);
            ui_panel_end();
        }
    }
}
