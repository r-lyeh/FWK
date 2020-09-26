// model + skybox + gizmo demo
// - rlyeh, public domain.

#include "fwk.h"

// @todo: debugdraw skeleton
// @todo: root motion on/off
// @todo: anim controller.lua
// @todo: get bone location (world space), modify bone transform (bone space)

typedef struct animations_t {
    int anim_selector;
    array(vec2) anims;
    array(char *) anim_names;
    map(char *, vec2) anim_names2ids;
    float anim_speed;
} animations_t;

animations_t animations(const char *pathfile, int flags) {
    animations_t a = {0};
    char *anim_file = vfs_read(pathfile);
    map_init(a.anim_names2ids, less_str, hash_str);
    for each_substring(anim_file, "\r\n", anim) {
        int from, to;
        char anim_name[128] = {0};
        /**/ if( sscanf(anim, "%*s %d-%d %127[^\r\n]", &from, &to, anim_name) == 3) {/*ok*/}
        else if( sscanf(anim, "%*s %d %127[^\r\n]", &to, anim_name) == 2) { from = array_count(a.anims) ? array_back(a.anims)->max + 0.1 : 0; }
        else continue;
        array_push(a.anims, vec2(from, to - 0.1));
        array_push(a.anim_names, STRDUP(anim_name));
        map_insert(a.anim_names2ids, *array_back(a.anim_names), *array_back(a.anims) );
    }
    a.anim_speed = 1.0;
    return a;
}

int main() {
    window_create(75, WINDOW_MSAA8);
    window_title(__FILE__);

    for(const char **list = vfs_list("fx**.fs"); *list; list++) {
        fx_load(*list);
    }

    camera_t cam = camera();
    skybox_t sky = skybox("cubemaps/stardust", 0);

    model_t custom = model("kgirls01.fbx", 0);
    animations_t a = animations("kgirl/animlist.txt", 0);

    while( window_swap() ) {
        if(input(KEY_F5)) app_reload();
        if(input(KEY_ESC)) break;

        // fps camera
        bool active = ui_active() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdec = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-input(KEY_C),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdec.x,wasdec.y,wasdec.z);
        camera_fps(&cam, mouse.x,mouse.y);

        // rendering
        ddraw_ground(0);
        ddraw_flush();

        fx_begin();


            profile(Skeletal update) if(!window_has_pause()) {
                float delta = window_delta() * 30; // 30fps anim

                // anims array contains pairs of frame numbers [from, to]
                vec2 anim = a.anims[ a.anim_selector ];
                custom.curframe += delta * a.anim_speed; // 30 hz anim timer
                custom.curframe = model_animate_clip(custom, custom.curframe, anim.min, anim.max, strstri(a.anim_names[a.anim_selector], "loop"));

                ddraw_text(vec3(-10,5,-10), 0.05, "Frame: %.1f", custom.curframe);
                ddraw_color(WHITE);
                ddraw_text(vec3(-10,8,-10), 0.05, "%d/%d) %s\nx%.2f [%.0f..%.1f..%.0f]\n",
                    a.anim_selector, array_count(a.anims)-1, a.anim_names[ a.anim_selector ],
                    a.anim_speed, anim.min, custom.curframe, anim.max);
                // @todo: get bone location (world space)
                ddraw_color(YELLOW);
            }

            profile(Skeletal render) {
                static vec3 p = {0,0,0}, r = {0,0,0}, s = {2,2,2};
                gizmo(&p, &r, &s);
                mat44 M; rotationq44(M, eulerq(r)); scale44(M, s.x,s.y,s.z); relocate44(M, p.x,p.y,p.z);

                    // pivot
                    r.x -= 90;
                        // control
                        float speed = 0.1f;
                        quat q = eulerq(r); // += custom.pivot
                        vec3 rgt = rotate3q(vec3(1,0,0), q);
                        vec3 up  = rotate3q(vec3(0,1,0), q);
                        vec3 fwd = rotate3q(vec3(0,0,1), q);
                        vec3 dir = scale3(fwd, speed * (input(KEY_UP) - input(KEY_DOWN)));
                        r.x += input(KEY_RIGHT) - input(KEY_LEFT); // yaw
                        p = add3(p, dir);
                    r.x += 90;

                model_render(custom, cam.proj, cam.view, M); // custom.pivot);

                aabb box = model_aabb(custom, M); // custom.pivot);
                ddraw_aabb(box.min, box.max);

                ddraw_position_dir(p, fwd, 1.0f);
            }

            profile(skybox) {
                skybox_push_state(&sky, cam.proj, cam.view);
                glEnable(GL_DEPTH_TEST);
                mesh_render(&sky.geometry);
                skybox_pop_state(&sky);
            }

        fx_end();

        // UI
        if( ui_begin("Animation", 0)) {
            if( input_down(KEY_UP) ) a.anim_speed = 1, ++a.anim_selector; if(a.anim_selector>=array_count(a.anims)) a.anim_selector = 0; // next anim
            if( input_down(KEY_DOWN) ) a.anim_speed = 1, --a.anim_selector; if(a.anim_selector<0) a.anim_selector = array_count(a.anims) - 1; // previous anim
            if( input_down(KEY_LEFT) || input_repeat(KEY_LEFT, 300) ) a.anim_speed = 0, custom.curframe-=0.5; // previous half frame
            if( input_down(KEY_RIGHT) || input_repeat(KEY_RIGHT, 300) ) a.anim_speed = 0, custom.curframe+=0.5; // next half frame
            if( input_down(KEY_SPACE)) a.anim_speed = !a.anim_speed; // toggle pause
            if( input_down(KEY_ENTER) ) custom.curframe = 0; // restart anim
            if( input(KEY_N) ) a.anim_speed = a.anim_speed - 0.01; // decrease anim speed
            if( input(KEY_M) ) a.anim_speed = a.anim_speed + 0.01; // increase anim speed
            vec2 anim = a.anims[ a.anim_selector ];
            custom.curframe = model_animate_clip(custom, custom.curframe, anim.min, anim.max, strstri(a.anim_names[a.anim_selector], "loop"));
            ui_label(stringf("Anim %d/%d) %s@UP/DOWN keys", a.anim_selector, array_count(a.anims)-1, a.anim_names[ a.anim_selector ] ));
            ui_label(stringf("Frame: [%.0f..%.1f..%.0f]@LEFT/RIGHT keys", anim.min, custom.curframe, anim.max));
            ui_label(stringf("Speed: x%.2f@N,M keys", a.anim_speed));
            if( ui_button("Restart anim@ENTER key") ) custom.curframe = 0;
            if( ui_button("Pause anim@SPACE key") ) a.anim_speed = !a.anim_speed;
            ui_end();
        }
        if( ui_begin("FX", 0) ) {
            for( int i = 0; i < 64; ++i ) {
                char *name = fx_name(i); if( !name ) break;
                bool b = fx_enabled(i);
                if( ui_bool(name, &b) ) fx_enable(i, fx_enabled(i) ^ 1);
            }
            ui_end();
        }
    }
}
