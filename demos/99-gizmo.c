#include "engine.h"

void gizmo2(float *pos, float *rot, float *sca, float *pivot) {
    vec3 p = ptr3(pos);
    vec3 r = ptr3(rot);
    vec3 s = ptr3(sca);
    gizmo(&p, &r, &s);
    if(pivot) compose44(pivot, p, eulerq(r), s);
    memcpy(pos, &p, sizeof(p));
    memcpy(rot, &r, sizeof(r));
    memcpy(sca, &s, sizeof(s));
}

int main() {
    argvadd("--cook-on-demand");
    window_create(0.75, 0);

    // setup cam
    camera_t cam = camera();
    cam.fov = 60;
    cam.position = vec3(10,10,10);
    camera_enable(&cam);

    while( window_swap() ) {

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, scale3(wasdecq, window_delta() * 60));
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

        static vec3 min = {0,0,0}, max = {1,1,1};
        ddraw_aabb( min, max );
        ddraw_ground(0);
        ddraw_flush();

        static model_t mdl;
        do_once mdl = model("meshes/gizmo.fbx", 0);
        vec3 color = {0,0,0};
        vec3 coeff[9] = { color,color,color,color,color,color,color,color,color,  };
        shader_vec3v("u_coefficients_sh", 9, coeff);
        shader_int("u_textured", false);
        model_render(mdl, cam.proj, cam.view, mdl.pivot);

        static mat44 pivot = {0};
        static int do_showgizmo = 1;
        if( do_showgizmo ) {
            static vec3 p = {0,0,0}, r = {180,180,0}, s = {1,1,1};
            gizmo2(&p.x, &r.x, &s.x, mdl.pivot);
        }

        // debug
        font_print(va(FONT_RIGHT "%d(%d)", gizmo_active(), gizmo_hover()));
    }
}
