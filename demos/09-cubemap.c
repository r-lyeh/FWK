#include "fwk.h"

int SKY_DIR = 0;
const char *SKY_DIRS[] = {
    "cubemaps/bridge3/",
    "cubemaps/colors/",
    "cubemaps/colors2/",
    "hdr/Tokyo_BigSight_1k.hdr",
};

int OBJ_MDL = 0;
const char *OBJ_MDLS[] = {
    "meshes/sphere.obj",
    "meshes/suzanne.obj",
    "meshes/gazebo.obj",
};

int main(int argc, char** argv) {
    window_create(85, WINDOW_MSAA8);

    camera_t cam = camera();
    skybox_t sky = {0};
    model_t mdl = {0};

    bool initialized = 0;
    bool must_reload = 0;

    while( window_swap()) {
        if (input_down(KEY_ESC)) break;
        // reloading
        if( must_reload ) {
            must_reload = 0;
            skybox_destroy(&sky);
            model_destroy(mdl);
            initialized = 0;
        }
        if( !initialized ) {
            initialized = 1;
            sky = skybox(flag("--mie") ? 0 : SKY_DIRS[SKY_DIR], 0);
            mdl = model(OBJ_MDLS[OBJ_MDL], MODEL_NO_PBR);
            // sky.rayleigh_immediate = 1;
            rotation44(mdl.pivot, 0, 1,0,0); // @fixme: -90,1,0,0 -> should we rotate SHMs as well? compensate rotation in shader?
        }

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        camera_freefly(&cam);

        // render
        mat44 mvp; multiply44x2(mvp, cam.proj, cam.view);
        {
            do_once {
                if (flag("--mie")) {
                    skybox_mie_calc_sh(&sky, 4.0f);
                }
            }

            skybox_render(&sky, cam.proj, cam.view);
            model_skybox(&mdl, sky);

            model_render(mdl, cam.proj, cam.view, mdl.pivot);

        }

        if( ui_panel("Scene", 0)) {
            if( ui_list("Skybox", SKY_DIRS, countof(SKY_DIRS), &SKY_DIR) ) {
                must_reload = 1;
            }
            if( ui_list("Model", OBJ_MDLS, countof(OBJ_MDLS), &OBJ_MDL) ) {
                must_reload = 1;
            }
            ui_separator();
            for (int i = 0; i < 9; i++) {
                ui_color3f(va("SH Coefficient [%d]", i), &sky.cubemap.sh[i].x);
            }
            ui_panel_end();
        }
    }
}
