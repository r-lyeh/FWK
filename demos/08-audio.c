// audio demo
// - rlyeh, public domain

#include "fwk.h"

int main() {
    // window (80% sized, MSAA x4 flag)
    window_create(80, WINDOW_MSAA4 | WINDOW_SQUARE);

    // audio (both streams & clips)
    audio_t stream1 = audio_stream( "larry.mid" );
    audio_t stream2 = audio_stream( "waterworld-map.fur" );
    audio_t BGM = stream1;
    audio_play(BGM, 0);

    audio_t SFX1 = audio_clip( "coin.wav" );
    audio_t SFX2 = audio_clip( "pew.sfxr" );
    audio_play(SFX1, 0);

    // demo loop
    while (window_swap() && !input_down(KEY_ESC)) {
        static int open = 1;
        if( ui_window("Audio", &open)) {
            static float bgm = 1, sfx = 1, master = 1;
            if( ui_slider2("BGM", &bgm, va("%.2f", bgm))) audio_volume_stream(bgm);
            if( ui_slider2("SFX", &sfx, va("%.2f", sfx))) audio_volume_clip(sfx);
            if( ui_slider2("Master", &master, va("%.2f", master))) audio_volume_master(master);
            if( ui_label2_toolbar("BGM: Leisure Suit Larry", ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = stream1, AUDIO_SINGLE_INSTANCE);
            if( ui_label2_toolbar("BGM: Waterworld Map", ICON_MD_VOLUME_UP)) audio_stop(BGM), audio_play(BGM = stream2, AUDIO_SINGLE_INSTANCE);
            if( ui_label2_toolbar("SFX: Coin", ICON_MD_VOLUME_UP)) audio_play(SFX1, 0);
            if( ui_label2_toolbar("SFX: Pew", ICON_MD_VOLUME_UP)) audio_play(SFX2, 0);
            ui_window_end();
        }
    }
}
