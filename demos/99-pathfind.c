// pathfind demo
// - rlyeh, public domain
//
// @todo: use 2d sprites instead, will shorten things

#include "fwk.h"

bool topdown_cam = 1;

void move_players();

int main() {
    window_create(85, 0); // WINDOW_MSAA4);
    window_title(__FILE__);
    // window_fps_lock(60);

    // game loop
    while( window_swap() && !input(KEY_ESC) ) {
        if( input(KEY_LALT) && input_down(KEY_Z) ) window_record(__FILE__ ".mp4");

        move_players();

        if( input_down(MOUSE_R) ) topdown_cam ^= 1;
    }
}

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
};

// every chunk is a 2D grid [200x200 units], center at (W/2,H/2), bit:1 means traversable
array(unsigned) chunk = 0;    // chunk partition in world
array(aabb)     blockers = 0; // static blockers in world
vec3 from, to; bool recast; bool vis_recast;
enum { W = 200, Z = 200 };

void draw_scene() {
    do_once array_resize(chunk, W*Z);

    enum { NUM_BLOCKS = 200 };
    do_once
    for( int i = 0; i < NUM_BLOCKS; ++i) {
        float ox = randi(-W/2,W/2);
        float oz = randi(-Z/2,Z/2);
        float ww = randi(1,10);
        float dd = randi(1,10);
        float hh = randi(3,10);
        aabb b = { vec3(ox,0,oz), vec3(ox+ww,hh,oz+dd) };

        static aabb spawn_zone = { {-10,-10,-10},{10,10,10} };
        if( aabb_hit_aabb(b, spawn_zone) ) continue;

        array_push(blockers, b);

        for( int y = oz; y < (oz+dd); ++y )
            for( int x = ox; x < (ox+ww); ++x )
                chunk[ (x + W/2) + mini(y + Z/2, Z-1) * W ] |= 1;
    }

    ddraw_color(BLUE);
    for( int i = 0; i < NUM_BLOCKS; ++i) {
        ddraw_aabb(blockers[i].min, blockers[i].max);
    }

    // debug traversal visualization
    if( input(KEY_F1) ) {
        ddraw_color(GREEN);
        for( int z = 0; z < Z; ++z )
            for( int x = 0; x < W; ++x ) 
                if( !chunk[x+z*W] ) ddraw_cube( vec3(x-W/2-0.5,0,z-Z/2-0.5), 1 );

        ddraw_color(RED);
        for( int z = 0; z < Z; ++z )
            for( int x = 0; x < W; ++x ) 
                if( chunk[x+z*W] ) ddraw_cube( vec3(x-W/2-0.5,0,z-Z/2-0.5), 1 );        
    }

    static vec2i path[W*Z] = {0};
    static int path_count = 0;
    static uint64_t taken = 0;
    if( vis_recast || input_down(KEY_SPACE) ) {
        vis_recast = 0;
        taken =- time_ns();
        path_count = pathfind_astar(W, Z, chunk, vec2i(clampf(from.x+W/2,0,W-1),clampf(from.z+Z/2,0,Z-1)), vec2i(clampf(to.x+W/2,0,W-1),clampf(to.z+Z/2,0,Z-1)), path, countof(path));
        taken += time_ns();
    }

    font_print(va(FONT_TOP FONT_RIGHT "%d (%5.2fms)", path_count, taken / 1e6));

    // path visualization
    if( path_count ) {
        ddraw_color(YELLOW); // wizard of oz
        vec2i* p = path;
        for( int i = 0; i < path_count; ++i, ++p ) {
            ddraw_point(vec3(p->x-W/2,0,p->y-Z/2));
        }
    }
}

void move_players() {

    static struct player_t player[3];
    do_once
        player[0] = (struct player_t)
            { "PLAYER-1", model("kgirls01.fbx", 0), loop(0,60,0.25,0), loop(66,85,0.25,0), // idle anim [0..60], run anim [66..85]
                {KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT}, 2, {0.90,0.80}, {-100}, {3, 0.30, 30}, {0}, {1} },
        player[1] = (struct player_t)
            { "PLAYER-2", model("george.fbx", 0), loop(0,100,0.25,0), loop(372,396,0.25,0), // idle anim [0..100], run anim [372..396]
                {KEY_I,KEY_K,KEY_J,KEY_L}, 1, {0.95,0.90}, {-90,-90}, {1.75, 0.25, 24}, {-5}, {1} },
        player[2] = (struct player_t)
            { "PLAYER-3", model("alien.fbx", 0), loop(110,208,0.25,0), loop(360,380,0.25,0), // idle anim [110..208], run anim [360..380]
                {KEY_W,KEY_S,KEY_A,KEY_D}, 0.85, {0.85,0.75}, {-90,-90}, {3.5, 0.35, 60}, {5}, {1} };

    static camera_t cam; do_once cam = camera();
    static skybox_t sky; do_once sky = skybox("cubemaps/stardust", 0);
    skybox_render(&sky, cam.proj, cam.view);
    for( int i = 0; i < countof(player); ++i )
        ddraw_position_dir(player[i].pos, player[i].dir, 1.0f);
    ddraw_ground(0);
    draw_scene();
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
            if(h) {
                if (array_count(points) == 0) recast = 1, vis_recast = 1;
                array_push(points, h->p), from = player[0].pos, to = *array_back(points); 
            }
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
            static vec2i path[W*Z] = {0};
            static int path_count = 0;
            static int path_index = 0;
            struct player_t *p = &player[i];
            vec3 pt = points[0];
            if( recast ) {
                recast = 0;
                path_count = pathfind_astar(W, Z, chunk, vec2i(clampf(p->pos.x+W/2,0,W-1),clampf(p->pos.z+Z/2,0,Z-1)), vec2i(clampf(pt.x+W/2,0,W-1),clampf(pt.z+Z/2,0,Z-1)), path, countof(path));
                path_index = path_count-1;
            }
            if (path_count == 0) {
                array_pop_front(points);
                continue;
            }

            vec2i ph = path[path_index];
            vec3 dst = vec3(ph.x-W/2,0,ph.y-Z/2);
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
                path_index--;
                if (path_index <= 0) {
                    // goal
                    array_pop_front(points);
                    recast = 1, vis_recast = 1;
                }
            }
            else {
                if( dist < 10 && abs(angle) > 10 ) {
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
        p->pad.x = p->pad.x * (p->inertia.y) + yaw * (1-p->inertia.y);
        p->pad.y = p->pad.y * (p->inertia.x) + fwd * (1-p->inertia.x);

        // rotate yaw dir, then apply into position
        p->dir = rotatey3(p->dir, p->speed.x * p->pad.x);
        p->pos = add3(p->pos, scale3(p->dir, p->speed.y * p->pad.y));

        // animate clips and blend
        anim_t *primary = fwd ? &p->run : &p->idle;
        anim_t *secondary = fwd ? &p->idle : &p->run;
        model_animate_blends(p->mdl, primary, secondary, window_delta() * p->speed.z);

        // render model. transforms on top of initial pivot and scale
        mat44 M; compose44(M, p->pos, eulerq(add3(p->pivot,vec3(atan2(p->dir.z,p->dir.x)*180/C_PI,0,0))),vec3(p->scale,p->scale,p->scale));
        model_render(p->mdl, cam.proj, cam.view, M, 0);

        // ui
        if( yaw||fwd ) if( !p->notified ) p->notified = 1, ui_notify(0, va(ICON_MD_GAMEPAD " %s joined the game.", p->name));
        ddraw_text(p->pos, 0.01, va("%s: %6.3f", fwd?"run":"idle", (fwd ? p->run : p->idle).timer ));
    }

    static int reset_topdown_cam = 1;
    if( topdown_cam ) {
        if( reset_topdown_cam ) reset_topdown_cam = 0, camera_teleport(&cam, vec3(0,80,0)), camera_lookat(&cam, vec3(cam.position.x,0,cam.position.z+1));
        // gta1 cam style: top-down view, look-at player, smoothed values. optional rotation
        if(0) { static vec3 smooth1; camera_lookat(&cam, smooth1 = mix3(add3(player[0].pos,player[0].dir),smooth1,0.95)); } 
        if(1) { static vec3 smooth2; camera_teleport(&cam, smooth2 = mix3(add3(player[0].pos,vec3(0.1,80,0.1)),smooth2,0.95)); }
        camera_fov(&cam, cam.fov * 0.99 + 60 * 0.01);
    } else {
        reset_topdown_cam = 1;
        // look at the players that are moving; center of their triangle otherwise.
        float A = len3(player[0].pad); if(A<0.01) A=0;
        float B = len3(player[1].pad); if(B<0.01) B=0;
        float C = len3(player[2].pad); if(C<0.01) C=0;
        float weight = A + B + C;
        if( weight ) A /= weight, B /= weight, C /= weight; else A = B = C = 0.33333;
        // smooth target before sending to camera
        vec3 target = add3(add3(scale3(player[0].pos,A), scale3(player[1].pos,B)), scale3(player[2].pos,C));
        static vec3 smooth1; camera_lookat(&cam, smooth1 = mix3(target,smooth1,!weight ? 0.98 : 0.95));
        target = vec3(10,10,10);
        static vec3 smooth2; camera_teleport(&cam, smooth2 = mix3(target,smooth2,!weight ? 0.98 : 0.95));
        camera_fov(&cam, cam.fov * 0.99 + 45 * 0.01);
    }

    // ui
    if( ui_panel("Controls", 0) ) {
        ui_label2("[F1]",  ICON_MD_KEYBOARD     " Debug traversal visualization");
        ui_label2("[Left mouse]",   ICON_MD_MOUSE   " Set Waypoint");
        ui_label2("[Right mouse]",   ICON_MD_MOUSE   " Toggle camera");
        ui_label2("Girl",  ICON_MD_GAMEPAD " CURSOR keys");
        ui_label2("Alien", ICON_MD_GAMEPAD " W,A,S,D keys");
        ui_label2("Robot", ICON_MD_GAMEPAD " I,J,K,L keys");
        ui_panel_end();
    }
}