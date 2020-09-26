#include "fwk.h"

int SKY_DIR = 0;
const char *SKY_DIRS[] = {
    "cubemaps/bridge3/",
    "cubemaps/coittower2/",
    "cubemaps/colors/",
    "cubemaps/colors2/",
    "cubemaps/mountain/",
    "cubemaps/room/",
    "cubemaps/stardust/",
    "cubemaps/tantolunden2/",
    "cubemaps/theforge/",
    "hdr/monument-valley.hdr",
    "hdr/factory-catwalk.hdr",
    "hdr/shiodome-stairs.hdr",
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
        // reloading
        if( must_reload ) {
            must_reload = 0;
            skybox_destroy(&sky);
            model_destroy(mdl);
            initialized = 0;
        }
        if( !initialized ) {
            initialized = 1;
            sky = skybox(SKY_DIRS[SKY_DIR], 0);
            mdl = model(OBJ_MDLS[OBJ_MDL], 0);
            rotation44(mdl.pivot, 0, 1,0,0);
        }

        // fps camera
        bool active = ui_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdec = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-input(KEY_C),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdec.x,wasdec.y,wasdec.z);
        camera_fps(&cam, mouse.x,mouse.y);

        // render
        mat44 mvp; multiply44x2(mvp, cam.proj, cam.view);
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            //glDisable(GL_CULL_FACE);

            // mesh
            glDepthMask(GL_TRUE);
            glUseProgram(mdl.program);
            glUniform3fv(glGetUniformLocation(mdl.program, "u_coefficients_sh"), 9, &sky.cubemap.sh[0].x);
            glUniform1i(glGetUniformLocation(mdl.program, "u_textured"), false);
            model_render(mdl, cam.proj, cam.view, mdl.pivot);

            // sky
            skybox_push_state(&sky, cam.proj, cam.view);
            glEnable(GL_DEPTH_TEST);
            mesh_render(&sky.geometry);
            skybox_pop_state(&sky);
        }

        if( ui_begin("Scene", 0)) {
            if( ui_list("Skybox", SKY_DIRS, countof(SKY_DIRS), &SKY_DIR) ) {
                must_reload = 1;
            }
            if( ui_list("Model", OBJ_MDLS, countof(OBJ_MDLS), &OBJ_MDL) ) {
                must_reload = 1;
            }
            ui_separator();
            for (int i = 0; i < 9; i++) {
                vec3 remap = scale3(add3(sky.cubemap.sh[i], vec3(1,1,1)), 127.5f); // -1..+1 -> 0..255
                ui_color3(stringf("SH Coefficient [%d]", i), &remap.x);
                sky.cubemap.sh[i] = sub3(scale3(remap, 1/127.5f), vec3(1,1,1));
            }
            ui_end();
        }
    }
}
