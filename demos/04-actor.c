// actor controller demo: anims, anim blending, input, math
// - rlyeh, public domain.
//
// Compile with:
//    `make     demos\04-actor.c` (windows)
// `sh MAKE.bat demos/04-actor.c` (linux, osx)

#include "fwk.h"

int main() {
    // create window (75% sized, MSAAx4)
    window_create(75, WINDOW_MSAA4);
    window_title(__FILE__);

    // set up our players
    struct player_t {
        const char *name;
        model_t mdl;
        anim_t idle, run;     // anim clips
        float keys[4], scale; // up,down,left,right
        vec2 inertia;         // [forward,yaw]
        vec3 pivot, speed;    // [pitch,yaw,roll] [turn speed,forward speed,anim speed fps]
        vec3 pos, dir, pad;   // [position] [facing dir] [gamepad accumulator]
        bool notified;
        float brain[4];       // AI
    } player[3] = {
        { "PLAYER-1", model("kgirls01.fbx", MODEL_NO_PBR), loop(0,60,0.25,0), loop(66,85,0.25,0), // idle anim [0..60], run anim [66..85]
            {KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT}, 2, {0.90,0.80}, {0,90,-90}, {3, 0.30, 30}, {0}, {1} },
        { "PLAYER-2", model("george.fbx", MODEL_NO_PBR), loop(0,100,0.25,0), loop(372,396,0.25,0), // idle anim [0..100], run anim [372..396]
            {KEY_I,KEY_K,KEY_J,KEY_L}, 1, {0.95,0.90}, {0,90}, {1.75, 0.25, 24}, {-5}, {1} },
        { "PLAYER-3", model("alien.fbx", MODEL_NO_PBR), loop(110,208,0.25,0), loop(360,380,0.25,0), // idle anim [110..208], run anim [360..380]
            {KEY_W,KEY_S,KEY_A,KEY_D}, 0.85, {0.85,0.75}, {0,90}, {3.5, 0.35, 60}, {5}, {1} }
    };

    // camera that points to origin, skybox, and a background tune
    camera_t cam = camera();
    skybox_t sky = skybox("cubemaps/stardust", 0);
    audio_play( audio_stream("waterworld-map.fur"), 0 );

    // game loop
    while( window_swap() ) {
        // world: skybox, position markers and ground grid
        skybox_render(&sky, cam.proj, cam.view);
        for( int i = 0; i < countof(player); ++i )
            ddraw_position_dir(player[i].pos, player[i].dir, 1.0f);
        ddraw_grid(0);
        ddraw_flush();

        // move and render players
        for( int i = 0; i < countof(player); ++i ) {
            struct player_t *p = &player[i];

            // capture inputs
            p->brain[0] = input(p->keys[0]);
            p->brain[1] = input(p->keys[1]);
            p->brain[2] = input(p->keys[2]);
            p->brain[3] = input(p->keys[3]);

            // setup waypoints for PLAYER-1
            static array(vec3) points;
            if( input_down(MOUSE_L) && !ui_hover() ) {
                vec3 pt = editor_pick(input(MOUSE_X), input(MOUSE_Y));
                hit *h = ray_hit_plane(ray(cam.position, pt), plane(vec3(0,0,0),vec3(0,1,0)));
                if(h) array_push(points, h->p);
            }
            // ddraw waypoints
            ddraw_color(YELLOW);
            for( int i = 1; i < array_count(points); ++i) ddraw_line(points[i-1],points[i]);
            for( int i = 0; i < array_count(points); ++i) ddraw_circle(points[i], vec3(0,1,0), 1); // prism(points[i], 1, 0, vec3(0,1,0), 4);
            ddraw_color(RED);
            for( int i = 0; i < array_count(points); ++i) ddraw_point(points[i]);
            ddraw_color(WHITE);
            // move thru waypoints (PLAYER-1 only)
            if( i == 0 && array_count(points) ) {
                struct player_t *p = &player[i];
                vec3 dst = points[0];
                vec3 vector1 = norm3(vec3(p->dir.x,0,p->dir.z));
                vec3 vector2 = norm3(sub3(dst,p->pos));

                float angle = atan2(vector2.z, vector2.x) - atan2(vector1.z, vector1.x);
                angle *= 180 / C_PI;
                // range [0, 360)
                // if (angle < 0) { angle += 2 * 180; }
                // range (-180, 180]
                if (angle > 180)        { angle -= 2 * 180; }
                else if (angle <= -180) { angle += 2 * 180; }

                float dist = len3(sub3(p->pos, dst));
                if(dist < 1) {
                    // goal
                    array_pop_front(points);
                }
                else {
                    if( dist < 10 && fabsf(angle) > 10 ) {
                        // spin only
                        p->brain[ angle < 0 ? 2 : 3 ] = 1;
                    }
                    else {
                        // spin
                        p->brain[ angle < 0 ? 2 : 3 ] = 1;
                        // move forward
                        p->brain[ 0 ] = 1;
                    }
                }
            }

            // accumulate movement
            float yaw = p->brain[2] - p->brain[3];
            float fwd = p->brain[0] - p->brain[1]; if(fwd<0) fwd = 0;
            p->pad.x = p->pad.x * p->inertia.y + yaw * (1-p->inertia.y);
            p->pad.y = p->pad.y * p->inertia.x + fwd * (1-p->inertia.x);

            // rotate yaw dir, then apply into position
            p->dir = rotatey3(p->dir, p->speed.x * p->pad.x);
            p->pos = add3(p->pos, scale3(p->dir, p->speed.y * p->pad.y));

            // animate clips and blend
            anim_t *primary = fwd ? &p->run : &p->idle;
            anim_t *secondary = fwd ? &p->idle : &p->run;
            model_animate_blends(p->mdl, primary, secondary, window_delta() * p->speed.z);

            // render model. transforms on top of initial pivot and scale
            mat44 M; compose44(M, p->pos, eulerq(add3(p->pivot,vec3(0,-atan2(p->dir.z,p->dir.x)*180/C_PI,0))),vec3(p->scale,p->scale,p->scale));
            model_render(p->mdl, cam.proj, cam.view, M);

            // ui
            if( yaw||fwd ) if( !p->notified ) p->notified = 1, ui_notify(0, va(ICON_MD_GAMEPAD " %s joined the game.", p->name));
            ddraw_text(p->pos, 0.01, va("%s: %6.3f", fwd?"run":"idle", (fwd ? p->run : p->idle).timer ));
        }

        // look at the players that are moving; center of their triangle otherwise.
        float A = len3(player[0].pad); if(A<0.01) A=0;
        float B = len3(player[1].pad); if(B<0.01) B=0;
        float C = len3(player[2].pad); if(C<0.01) C=0;
        float weight = A + B + C;
        if( weight ) A /= weight, B /= weight, C /= weight; else A = B = C = 0.33333;
        vec3 target = add3(add3(scale3(player[0].pos,A), scale3(player[1].pos,B)), scale3(player[2].pos,C));
        // smooth target before sending to camera
        static vec3 smooth; camera_lookat(&cam, smooth = mix3(target,smooth,!weight ? 0.98 : 0.95));

        // ui
        if( ui_panel("Controls", 0) ) {
            ui_label2("Girl",  ICON_MD_MOUSE   " Set Waypoint");
            ui_label2("Girl",  ICON_MD_GAMEPAD " CURSOR keys");
            ui_label2("Alien", ICON_MD_GAMEPAD " W,A,S,D keys");
            ui_label2("Robot", ICON_MD_GAMEPAD " I,J,K,L keys");
            ui_panel_end();
        }
    }
}
