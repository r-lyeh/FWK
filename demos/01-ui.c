// hello ui: config, window, system, ui, video
// - rlyeh, public domain.
//
// Compile with:
//    `make     demos\01-ui.c` (windows)
// `sh MAKE.bat demos/01-ui.c` (linux, osx)

#include "engine.h"

int main() {
    float    app_volume = 1.00f;
    unsigned app_size = flag("--fullscreen") ? 100 : 75;
    unsigned app_flags = flag("--msaa") ? WINDOW_MSAA4 : 0;
    unsigned app_target_fps = optioni("--fps", 60); // --fps=30, --fps=45, etc. defaults to 60.

    // window api (fullscreen or 75% sized, optional MSAA flags)
    window_create(app_size, app_flags);
    window_title(__FILE__);
    window_fps_lock(app_target_fps);

    // load video
    video_t *v = video( "pexels-pachon-in-motion-17486489.mp4", VIDEO_RGB | VIDEO_LOOP );

    // app loop
    while( window_swap() ) {
        // input controls
        if( input(KEY_ESC) ) break;

        // profile api
        texture_t *textures;
        profile( "Video decoder" ) {
            // video api: decode frame and get associated textures (audio is sent to audiomixer automatically)
            textures = video_decode( v );
            // fullscreen video
            // if(video_is_rgb(v)) fullscreen_quad_rgb( textures[0] );
            // else fullscreen_quad_ycbcr( textures, 1.3f );
        }

        // create menubar on top
        int choice1 = ui_menu("File;Shell;Exit");
        int choice2 = ui_menu("Help;About");
        if( choice1 == 1 ) system(ifdef(win32, "start \"\" cmd", ifdef(osx, "open sh", "xdg-open sh")));
        if( choice1 == 2 ) break;

        // showcase a few ui widgets
        ui_demo(0);

        // create ui panel
        if( ui_panel("myPanel", PANEL_OPEN) ) {
            // Print some numbers
            ui_section("Stats");
            ui_label2("FPS", va("%5.2f", window_fps()));
            ui_separator();

            // add some buttons
            ui_section("Buttons");
            if( ui_button("Screenshot") ) window_screenshot(__FILE__ ".png"), ui_notify(0,ICON_MD_WARNING "Screenshot");
            if( ui_button("Record Video") ) window_record(__FILE__ ".mp4"), ui_notify(0,ICON_MD_WARNING "Recoding video");
            if( ui_button("Toggle fullscreen") ) window_fullscreen( !window_has_fullscreen() );
            ui_separator();

            // some more video controls
            ui_section("Video");
            if( ui_button("Rewind") )  video_seek(v, video_position(v) - 3);
            if( ui_button("Pause") )   video_pause(v, video_is_paused(v) ^ 1);
            if( ui_button("Forward") ) video_seek(v, video_position(v) + 3);
            if( ui_slider2("Volume", &app_volume, va("%.2f", app_volume))) audio_volume_master(app_volume);

            // end of panel. must be enclosed within same if() branch.
            ui_panel_end();
        }

        // create window
        static int open = 1;
        if( ui_window("myWindow", &open) ) {
            // present decoded texture in a widget, without any label (NULL)
            ui_texture( NULL, textures[0] );
            // end of window. must be enclosed within same if() branch.
            ui_window_end();
        }
    }
}

// this demo supersedes following old sources:
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-hello.c
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-video.c
