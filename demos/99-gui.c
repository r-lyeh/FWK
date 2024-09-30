#include "engine.h"

int main() {
    window_create(75.0, 0 );

    gui_pushskin(gui_skinned("golden.ase", 3.0f)); // x3 scale
    skinned_t *skinned = (skinned_t*)gui_userdata();

    vec4 pos = vec4(400,400,100, 30);

    float testval=7.5f;
    float testval2=7.5f;

    while( window_swap() && !input(KEY_ESC) ) { // game loop

        if (ui_panel("Atlas", 0)) {
            ui_atlas(&skinned->atlas);
            ui_panel_end();
        }

        if (ui_panel("GUI", 0)) {
            ui_float("Scale", &skinned->scale);

            ui_panel_end();
        }

        if (input_down(MOUSE_R)) {
            pos.x = input(MOUSE_X);
            pos.y = input(MOUSE_Y);
        }
        if (input(MOUSE_R)) {
            pos.z = input(MOUSE_X)-pos.x;
            pos.w = input(MOUSE_Y)-pos.y;
        }

        // font
        do_once font_face(FONT_FACE2, "lilita_one_regular.ttf", 48.0f, FONT_EU | FONT_2048);
        do_once font_color(FONT_COLOR6, WHITE);
        font_print(va(FONT_MIDDLE FONT_CENTER FONT_FACE2 FONT_COLOR6 "%s", "Hello"));

        // background
        vec4 panel_pos = vec4(0, 0, window_width(), window_height());
        gui_panel(panel_pos, "panel");

            // button
            if (gui_button_label(FONT_FACE2 "PRESS", pos, 0)) {
                printf("%s\n", "Button pressed!");
            }

            // vials
            gui_rect(vec4(40,140, 320, 20*skinned->scale), "vial");
            gui_rect(vec4(40,140, 200, 14*skinned->scale), "hp");
            gui_rect(vec4(40,240, 240, 20*skinned->scale), "vial");
            gui_rect(vec4(40,240, 160, 14*skinned->scale), "mp");

            // buttons
            vec2 badge_size = gui_getskinsize("badge", NULL);
            badge_size.x += 2; // padding
            gui_rect(vec4(60+badge_size.x*0,320, 1, 1), "badge");
            gui_rect(vec4(60+badge_size.x*1,320, 1, 1), "badge");
            gui_rect(vec4(60+badge_size.x*2,320, 1, 1), "badge_empty");

            // sliders
            vec2 slider_size = gui_getskinsize("slider", NULL);
            gui_slider(vec4(60, 420, 80*skinned->scale, 1), 0, 0.0f, 15.0f, 1.0f, &testval);
            gui_slider_label(va(FONT_FACE2 "%.02f", testval2), vec4(60, 420+slider_size.y+10, 120*skinned->scale, 1), 0, -5.0f, 20.0f, 0.0f, &testval2);
        gui_panel_end();
    }

    gui_popskin();
}
