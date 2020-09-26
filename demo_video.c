// video player
// - rlyeh, public domain

#include "fwk.h"

int main( int argc, const char** argv ) {
    // 75% window, msaa x8
    window_create( 75, WINDOW_MSAA8 );
    window_title("FWK");

    // load all fx files
    for(const char **list = vfs_list("fx**.fs"); *list; list++) {
        fx_load(*list);
    }

    // load video, no flags
    const char *filename = argc < 2 ? "bjork-all-is-full-of-love.mp4" : argv[1];
    video_t *v = video( filename, 0 );

#if WITH_VIDEO_YCBCR
    void (*fullscreen_quad)(texture_t *, float) = fullscreen_ycbcr_quad;
#else
    void (*fullscreen_quad)(texture_t *, float) = fullscreen_rgb_quad;
#endif

    while( window_swap() ) {
        fx_begin();

        // decode video frame and get associated textures (audio is automatically sent to audiomixer)
        texture_t *textures;
        profile( video decoder ) {
            textures = video_decode( v );
        }

        // send decodes textures to screen as a fullscreen quad
        profile( video quad ) {
            fullscreen_quad( textures, 1.3f );
        }

        fx_end();

        // input controls
        if( input(KEY_ESC) ) break;
        if( input(KEY_LEFT ) ) video_seek(v, video_position(v) - 3);
        if( input(KEY_RIGHT) ) video_seek(v, video_position(v) + 3);

        // post-fx
        if( ui_begin("FX", 0) ) {
            if(ui_button("Clear all")) {
                fx_enable_all(0);
            }
            for( int i = 0; i < 64; ++i ) {
                char *name = fx_name(i); if( !name ) break;
                bool b = fx_enabled(i);
                if( ui_bool(name, &b) ) fx_enable(i, fx_enabled(i) ^ 1);
            }
            ui_end();
        }
        // audio
        if( ui_begin("Audio", 0)) {
            static float master = 1;
            if( ui_slider2("Master", &master, stringf("%.2f", master))) audio_volume_master(master);
            ui_end();
        }
    }
}
