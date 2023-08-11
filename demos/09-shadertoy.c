// shadertoy viewer
// - rlyeh, public domain

#include "fwk.h"

int main() {
    window_create(75, 0); // WINDOW_MSAA8);

    array(char*) list = 0;
    for( const char **dir = file_list("demos/art/shadertoys/", "**.fs"); *dir; dir++ ) {
        array_push(list, STRDUP(file_name(*dir)));
    }

    shadertoy_t sh = shadertoy(*list, 0); // 0:no flags

    while(window_swap()) {
        // selector
        static int selected = 0;
        int prev = input_down(KEY_UP) || input_down(KEY_LEFT);
        int next = input_down(KEY_DOWN) || input_down(KEY_RIGHT);
        if( prev ) if( selected > 0 ) sh = shadertoy( list[--selected], 0 );
        if( next ) if( selected < array_count(list) - 1 ) sh = shadertoy( list[++selected], 0 );

        // draw
        shadertoy_render(&sh, window_delta());

        // UI
        if( ui_panel("Shadertoy", 1)) {
            for( int i = 0; i < array_count(list); ++i ) {
                bool in_use = i == selected;
                if( ui_bool(list[i], &in_use) ) {
                    sh = shadertoy( list[selected = i], 0 );
                }
            }
            ui_panel_end();
        }
    }
}
