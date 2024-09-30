// playground tests
// - rlyeh, public domain
//
// # quickstart
// - win/vc       : cl hello.c
// - win/clang-cl : clang-cl  hello.c
// - win/tcc      : tools\tcc hello.c -m64
// - win/mingw    : gcc   hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
// - win/clang    : clang hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
// - linux        : cc  hello.c -lm -ldl -lpthread -lX11
// - linux/tcc    : tcc hello.c -lm -ldl -lpthread -lX11 -D__STDC_NO_VLA__
// - osx          : cc -ObjC hello.c -framework cocoa -framework iokit -framework audiotoolbox

#include "engine.h"

#pragma warning(disable : 4716)
#define main concat(app,__COUNTER__)

#include "demos/00-loop.c"
#include "demos/00-script.c"
#include "demos/01-demo2d.c"
#include "demos/01-easing.c"
#include "demos/01-font.c"
#include "demos/01-ui.c"
#include "demos/02-ddraw.c"
#include "demos/02-frustum.c"
#include "demos/03-anims.c"
#include "demos/03-batching.c"
#include "demos/03-mesh.c"
#include "demos/04-actor.c"
#include "demos/06-material.c"
#include "demos/06-parallax.c"
#include "demos/06-scene-sorting.c"
#include "demos/06-scene.c"
#include "demos/06-sorting.c"
#include "demos/07-netsync.c"
#include "demos/07-network.c"
#include "demos/08-audio.c"
#include "demos/08-video.c"
#include "demos/09-cubemap.c"
#include "demos/09-envmap.c"
#include "demos/09-lights.c"
#include "demos/09-shadertoy.c"
#include "demos/09-shadows-scene.c"
#include "demos/09-shadows.c"
#include "demos/99-bt.c"
#include "demos/99-compute.c"
#include "demos/99-controller.c"
#include "demos/99-geom.c"
#include "demos/99-gizmo.c"
#include "demos/99-gui.c"
#include "demos/99-lmap.c"
#include "demos/99-lod.c"
#include "demos/99-pathfind.c"
#include "demos/99-spine.c"
#include "demos/99-splines.c"
#include "demos/99-sponza.c"
#include "demos/99-sprite.c"
#include "demos/99-sprite3d.c"
#include "demos/99-steam.c"

#undef main

int main() {
    int (*demos[])() = {
        app2,app3,app4,app5,app6,app7,app8,app9,
        app10,app11,app12,app13,app14,app15,app16,app17,app18,app19,
        app20,app21,app22,app23,app24,app25,app26,app27,app28,app29,
        app30,app31,app32,app33,app34,app35,app36,app37,app38,app39,
        app40,app41,app42, };

    int demo = argc() > 1 && argv(1)[0] >= '0' && argv(1)[0] <= '9' ? atoi(argv(1)) : -1;
    if( demo >= 0 && demo < countof(demos) ) return demos[demo]();

    window_debug(0);
    window_create(100, WINDOW_TRANSPARENT);
    while( window_swap() && !input(KEY_ESC) ) {
        static int open = 1;
        if( ui_window("Demo Launcher", &open) ) {
            for( int i = 0; i < countof(demos); ++i)
            if( ui_button(va("#%02d",i+1)) ) system(va("%s %d", argv(0), i));
            ui_separator();
            ui_window_end();
        }
    }
    return 0;
}
