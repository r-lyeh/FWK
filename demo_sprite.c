// sprite routines
// - rlyeh,
//
// credits: original lovely demo by rxi (MIT License).
// see https://github.com/rxi/autobatch/tree/master/demo/cats

#include "fwk.h"

texture_t kids, catImage, shadowImage;
int NUMSPRITES = 100, NUMSPRITES_CHANGED = 1;

typedef struct Cat {
    int cat, flip;
    double x, y;
    double vx, vy;
    double animSpeed;
    double moveTimer;
    double elapsed;
} Cat;

void demo_cats() {
    static array(Cat) cats = 0;

    // init
    if( NUMSPRITES_CHANGED ) {
        NUMSPRITES_CHANGED = 0;

        array_resize(cats, NUMSPRITES); int i = 0;
        for each_array_ptr(cats, Cat, c) {
            randset(i++);
            c->x = randf() * window_width();
            c->y = randf() * window_height();
            c->vx = c->vy = 0;
            c->cat = randi(0, 4);
            c->flip = randf() < 0.5;
            c->animSpeed = 0.8 + randf() * 0.3;
            c->moveTimer = 0;
            c->elapsed = 0;
        }
    }

    // move
    const float dt = 1/120.f;
    const int appw = window_width(), apph = window_height();

    enum { yscale = 1 };
    for( int i = 0; i < NUMSPRITES; ++i ) {
        Cat *c = &cats[i];
        // Add velocity to position //and wrap to screen
        c->x += yscale * c->vx * dt; // % ;
        c->y += yscale * c->vy * dt; // % (int)window_height();
        if( c->x < 0 ) c->x += appw; else if( c->x > appw ) c->x -= appw;
        if( c->y < 0 ) c->y += apph; else if( c->y > apph ) c->y -= apph;
        // Faster animation if walking
        int boost = c->vx == 0 && c->vy == 0 ? 1 : 3;
        // Update elapsed time
        c->elapsed += dt * boost;
        // Update move timer -- if we hit zero then change or zero velocity
        c->moveTimer -= dt * boost;
        if (c->moveTimer < 0) {
            if (randf() < .2) {
                c->vx = (randf() * 2 - 1) * 30 * 2;
                c->vy = (randf() * 2 - 1) * 15 * 2;
                c->flip = c->vx < 0;
            } else {
                c->vx = c->vy = 0;
            }
            c->moveTimer = 1 + randf() * 5;
        }
    }

    // render
    uint32_t white = rgba(255,255,255,255);
    uint32_t alpha = rgba(255,255,255,255*0.6);
    for( int i = 0; i < NUMSPRITES; ++i ) {
        Cat *c = &cats[i];
        // Get current animation frame (8x4 tilesheet)
        double e = c->elapsed * c->animSpeed;
        double frame_num = c->cat * 8 + floor( ((int)(e * 8)) % 4 );
        frame_num = c->vx != 0 || c->vy != 0 ? frame_num + 4 : frame_num;
        // Get x scale based on flip flag
        int xscale = yscale * (c->flip ? -1 : 1);
        // Draw
        sprite_ex(catImage,
            c->x,c->y,c->y, 0,          // position(x,y,depth: sort by Y), angle
            0,0, xscale,yscale,         // offset(x,y), scale(x,y)
            0,white,                    // is_additive, tint color
            frame_num, 8,4              // frame_number in a 8x4 spritesheet
        );
        sprite_ex(shadowImage,
            c->x,c->y,-c->y, 0,         // position(x,y,depth: sort by Y), angle
            -1,5, xscale,yscale,        // offset(x,y), scale(x,y)
            0,alpha,                    // is_additive, tint color
            0, 0,0                      // no frame_number (0x0 spritesheet)
        );
    }
}

void demo_kids() {
    static int angle; //++angle;
    static int *x, *y, *v;

    // init
    if( NUMSPRITES_CHANGED ) {
        NUMSPRITES_CHANGED = 0;

        y = (int*)REALLOC(y, 0 );
        x = (int*)REALLOC(x, NUMSPRITES * sizeof(int) );
        y = (int*)REALLOC(y, NUMSPRITES * sizeof(int) );
        v = (int*)REALLOC(v, NUMSPRITES * sizeof(int) );
        for( int i = 0; i < NUMSPRITES; ++i ) {
            randset(i);
            x[i] = randi(0, window_width());
            y[i] = randi(0, window_height());
            v[i] = randi(1, 3);
        }
    }

    // config
    const int appw = window_width(), apph = window_height();

    // move & render
    for( int i = 0; i < NUMSPRITES; ++i ) {
        y[i] = (y[i] + v[i]) % (apph + 128);
        int col = ((x[i] / 10) % 4); // 4x4 tilesheet
        int row = ((y[i] / 10) % 4);
        int num_frame = col * 4 + row;
        sprite_ex(kids,
            x[i],y[i],y[i], angle,    // position(x,y,depth: sort by Y), angle
            0,0, 1,1,                 // offset(x,y), scale(x,y)
            0, ~0u,                   // is_additive, tint color
            num_frame, 4,4            // num_frame in a 4x4 spritesheet
        );
    }
}

int main(int argc, char **argv) {
    window_create(75.f, 0);
    window_title("FWK - Sprite");

    kids = texture( "spriteSheetExample.png", TEXTURE_NEAREST );
    catImage = texture( "cat.png", TEXTURE_NEAREST );
    shadowImage = texture( "cat-shadow.png", TEXTURE_NEAREST );

    int option_cats = 1;
    NUMSPRITES = argc > 1 ? atoi(argv[1]) : NUMSPRITES; // argvi("--numsprites,-N", "100");
    if(option_cats) NUMSPRITES/=2; // cat-sprite+cat-shadow == 2 sprites

    while(window_swap()) {
        if( input(KEY_F5)) app_reload();
        if( input(KEY_F11)) window_fullscreen( window_has_fullscreen() ^ 1);
        if( input(KEY_ESC) ) break;

        viewport_color(vec3(0.4,0.4,0.4));

        profile(sprite batching) {
            if(option_cats) demo_cats(); else demo_kids();
        }

        if( ui_begin("Sprite", 0) ) {
            const char *labels[] = {"Kids","Cats"};
            if( ui_list("Sprite type", labels, countof(labels), &option_cats) ) NUMSPRITES_CHANGED = 1;
            if( ui_int("Number of Sprites", &NUMSPRITES) ) NUMSPRITES_CHANGED = 1;
            ui_end();
        }
    }
}
