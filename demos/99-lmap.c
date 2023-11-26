#include "fwk.h"

model_t litm;

void bakedrawmodel(lightmap_t *lm, model_t *m, float *view, float *proj, void *userdata) {
    shader_bind(lm->shader);
    model_render(*m, proj, view, m->pivot, lm->shader);
    shader_float("u_litboost", 4.0);
    model_render(litm, proj, view, litm.pivot, lm->shader);
}

void progressupdate(float progress) {
    static double lastUpdateTime = 0.0;
    double time = time_ss();
    if (time - lastUpdateTime > 1.0) {
        lastUpdateTime = time;
        PRINTF("progress: %.02f%%", progress*100);
    }
    // window_swap();
}

int main()
{
    window_create(0.5, 0);
    window_title(__FILE__);
    camera_t cam = camera();
    skybox_t sky = skybox(0, 0); skybox_mie_calc_sh(&sky, 2.0f);
    model_t mdl = model(option("--model","gazebo.obj"), 0);
    litm = model("cube.obj", MODEL_MATCAPS);
    {
        mat44 lp; scaling44(lp, 0.3, 0.3, 0.3); translate44(lp, 8,4,0);
        copy44(litm.pivot, lp);
    }
    rotate44(mdl.pivot, -90, 1, 0, 0);
    scale44(mdl.pivot, 4,4,4);
    shader_bind(mdl.program);
    shader_vec3v("u_coefficients_sh", 9, sky.cubemap.sh);
    // shader_bool("u_texmod", 0);

    unsigned char emissive[] = { 255, 180, 0, 255 };
    texture_t emission = texture_create(1,1,4,emissive,TEXTURE_LINEAR);
    model_set_texture(litm, emission);

    lightmap_t baker = lightmap(64, 0.01, 100, vec3(0,0,0), 2, 0.01, 0.0);
    lightmap_setup(&baker, 512, 512);
    array_push(baker.models, &mdl);

    bool do_bake=0;
    int b=1;

    while (window_swap() && !input(KEY_ESC)) {
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, wasdecq);
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

        skybox_render(&sky, cam.proj, cam.view);
        model_render(mdl, cam.proj, cam.view, mdl.pivot, 0);
        model_render(litm, cam.proj, cam.view, litm.pivot, 0);

        if( ui_panel("Lightmapper", PANEL_OPEN) ) {
            ui_label2("Freecam", "Mouse + W/A/S/D/E/Q keys");
            ui_label("Warning " ICON_MD_WARNING "@This will take a few seconds and bake a lightmap illuminated by: The mesh itself (initially black) + A white sky (1.0f, 1.0f, 1.0f)");
            ui_int("Bounces", &b);
            if( ui_button(va("Bake %d light bounce(s)", b)) ) {
                do_bake=1;
            }
            ui_panel_end();
        }

        if (do_bake) {
            do_bake=0;
            lightmap_bake(&baker, b, bakedrawmodel, progressupdate, 0);
        }
    }
}
