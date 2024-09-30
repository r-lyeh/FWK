// @readme: https://gamedev.stackexchange.com/questions/14985/determine-arc-length-of-a-catmull-rom-spline-to-move-at-a-constant-speed/14995#14995

#include "engine.h"

int main() {
    window_create(0.85, WINDOW_MSAA4);

    camera_t cam = camera();

    curve_t cv = curve();
    for(float t = 0; t <= 360; t += 36) {
        curve_add(&cv, vec3(cos(t*TO_RAD)*5,0,sin(t*TO_RAD)*5));
    }
    int num_points = 11; // beware with these: 8,11,17,20,61,100,200
    curve_end(&cv, num_points);

    while(window_swap() && !input_down(KEY_ESC)) {
        // fps camera
        bool active = ui_hover() || ui_active() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, scale3(wasdecq, window_delta() * 60));
        camera_fps(&cam, mouse.x,mouse.y);

        // 3d
        ddraw_grid(0);

        for( int i = 0, end = array_count(cv.samples) - 1; i < end; ++i) {
            int vis = (int)fmod(window_time(), array_count(cv.samples) - 1);
            ddraw_color(i == vis ? BLUE : YELLOW);
            ddraw_point(cv.samples[i]);
            ddraw_color(YELLOW);
            if(i==vis)
            ddraw_line(cv.samples[i], cv.samples[i+1]);
        }

        static int delay = 5;
        float dt = fmod(window_time(), delay) / delay;
        vec3 pos = curve_eval(&cv, dt, NULL);
        ddraw_color(PURPLE);
        ddraw_sphere(pos, 0.5);

        if( ui_panel("Path",PANEL_OPEN)) {
            if(ui_int("Points", &num_points)) { if(num_points < 6) num_points = 6; curve_end(&cv, num_points); }
            if(ui_int("Delay", &delay)) { if(delay <= 0) delay = 1; }
            ui_panel_end();
        }
    }
}
