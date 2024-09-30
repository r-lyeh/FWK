#include "engine.h"

void game(unsigned frame) {
    static camera_t cam2d,cam3d;
    static sprite_t *s1 = 0;
    static sprite_t *s2 = 0;
    static sprite_t *s3 = 0;
    static sprite_t *s4 = 0;
    if( !frame ) {
        cam3d = camera();
        cam3d.damping = true;

        // camera center(x,y) zoom(z)
        cam2d = camera();
        cam2d.position = vec3(window_width()/2,window_height()/2,8);
        camera_enable(&cam2d);

        sprite_del(s1);
        sprite_del(s2);
        sprite_del(s3);
        sprite_del(s4);

        s1 = sprite_new("Captain Clown Nose.ase", (int[6]){KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_A,KEY_S});
        s2 = sprite_new("Crew-Crabby.ase", (int[6]){KEY_I,KEY_K,KEY_J,KEY_L} );
        s3 = sprite_new("Props-Shooter Traps.ase", (int[6]){0} );
        s4 = sprite_new("Crew-Fierce Tooth.ase", (int[6]){0,0,KEY_N,KEY_M} );

        obj_setname(s1, "Cap");
        obj_setname(s2, "Crab");
        obj_setname(s3, "Cannon");
        obj_setname(s4, "Shark");

        // 2d pos and z-order
        // s1->pos = vec3(window_width()/2, window_height()/2, 2);
        // s2->pos = vec3(window_width()/2, window_height()/2, 1);
        // s3->pos = vec3(window_width()/2, window_height()/2, 1);
        // s4->pos = vec3(window_width()/2, window_height()/2, 1);

        // 3d position+scale
        s1->pos = s2->pos = s3->pos = s4->pos = vec4(0,1.5,0,0);
        s1->sca = s2->sca = s3->sca = s4->sca = vec2(0.125,0.125);
        // 3d zindex
        s1->pos.w = 0; s2->pos.w = 0.1; s3->pos.w = 0.2; s4->pos.w = 0.3;

        s4->flipped ^= 1;
    }

    // draw world
    camera_enable(&cam3d);
    ddraw_grid(0);
    ddraw_flush();

    // camera logic
    enum { is_editing = 0 };
    if( is_editing ) {
        if( !ui_hover() && !ui_active() ) {
            // camera panning (x,y) & zooming (z)
            if( input(MOUSE_L) ) cam2d.position.x -= input_diff(MOUSE_X);
            if( input(MOUSE_L) ) cam2d.position.y -= input_diff(MOUSE_Y);
            cam2d.position.z += input_diff(MOUSE_W) * 0.1; // cam2d.p.z += 0.001f; for tests
        }
        camera_enable(&cam2d);
    } else {

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) {
        cam3d.speed = clampf(cam3d.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam3d.speed);
        camera_moveby(&cam3d, wasdecq);
        camera_fps(&cam3d, mouse.x,mouse.y);
        } else {
        camera_teleport(&cam3d, add3(s1->pos.xyz, vec3(0,10,20)));
        camera_lookat(&cam3d, s1->pos.xyz);
        }

        window_cursor( !active );
    }

    obj_tick(s1);
    obj_draw(s1);

    obj_tick(s2);
    obj_draw(s2);

    obj_tick(s3);
    obj_draw(s3);

    obj_tick(s4);
    obj_draw(s4);

    if( ui_panel("Help", 0) ) {
        ui_label2("Freecam", ICON_MD_MOUSE " RMB");
        ui_label2("Captain", ICON_MD_KEYBOARD " Cursor keys");
        ui_label2("Crab", ICON_MD_KEYBOARD " I/K/J/L keys");
        ui_label2("Shark", ICON_MD_KEYBOARD " N/M");
        ui_panel_end();
    }

    if( ui_panel("Sprites", PANEL_OPEN)) {
        obj_edit(s1);
        obj_edit(s2);
        obj_edit(s3);
        obj_edit(s4);
        ui_panel_end();
    }
}

int main() {
    unsigned frame = 0;

    for( window_create(0.75, 0); window_swap(); ) {
        if( input_down(KEY_Z) && input(KEY_ALT) ) window_record(file_counter(va("%s.mp4",app_name())));
        if( input_down(KEY_F5) ) frame = 0;
        game( frame++ );
    }
}