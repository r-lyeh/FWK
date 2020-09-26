// scene demo
// - rlyeh, public domain

#include "fwk.h"

int main() {
    // options
    bool  do_twosided = 1;
    bool  do_wireframe = 0;
    bool  do_billboard_x = 0, do_billboard_y = 0, do_billboard_z = 0;

    // window (80% sized, MSAA x4 flag)
    window_create(80, WINDOW_MSAA4);
    window_title(__FILE__);

    // load all fx files
    for(const char **list = vfs_list("fx**.fs"); *list; list++) {
        fx_load(*list);
    }

    // scene loading
    #define SCENE(...) #__VA_ARGS__
    const char *my_scene = SCENE([
        {
            skybox: 'cubemaps/stardust/',
        },
        {
            position:[-5.0,-2.0,2.0],
            rotation: [90.0,0.0,180.0],
            scale:0.20,
            //anchor/pivot:[],
            // vertex:'p3 t2',
            mesh:'models/witch/witch.obj',
            texture:'models/witch/witch_diffuse.tga.png',
//            swapzy:true,
            flipuv:false,
        },
        {
            position:[-5.0,-2.0,2.0],
            rotation: [90.0,0.0,180.0],
            scale:2.20,
            //anchor/pivot:[],
            // vertex:'p3 t2',
            mesh:'models/witch/witch_object.obj',
            texture:'models/witch/witch_object_diffuse.tga.png',
//            swapzy:true,
            flipuv:false,
        },
    ]);
    int num_spawned = scene_merge(my_scene);
    object_t *obj1 = scene_index(0);
    object_t *obj2 = scene_index(1);

    // manual spawn & loading
    model_t m1 = model("kgirl/kgirls01.fbx", 0); //MODEL_NO_ANIMS);
    texture_t t1 = texture("kgirl/g01_texture.png", TEXTURE_RGB);
    object_t* obj3 = scene_spawn();
    object_model(obj3, m1);
    object_diffuse(obj3, t1);
    object_scale(obj3, vec3(3,3,3));
    object_move(obj3, vec3(-10,0,-10));
    object_pivot(obj3, vec3(-90+180,180,0));

    // camera
    camera_t cam = camera();
    cam.speed = 0.2f;

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;

        // fps camera
        bool active = ui_active() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdecq.x,wasdecq.y,wasdecq.z);
        camera_fps(&cam, mouse.x,mouse.y);

        // queue model scale bounces
        float t = fmod(window_time(), 0.3) / 0.3;
        float s = 0.01f * ease_ping_pong(t, ease_in_cubic,ease_out_cubic);
        object_scale(obj1, vec3(0.20f - s,0.20f + s,0.20f - s));
        object_scale(obj2, vec3(0.20f - s,0.20f + s,0.20f - s));

        // queue model rotation
        //object_rotate(obj3, vec3(0,1*window_time() * 20,0));
        object_billboard(obj3, (do_billboard_x << 2)|(do_billboard_y << 1)|(do_billboard_z << 0));

        // flush render scene (background objects: skybox)
        profile(Scene background) {
            scene_render(SCENE_BACKGROUND);
        }

        // queue debug drawcalls
        profile(Debugdraw) {
            ddraw_grid(0);
            ddraw_color(YELLOW);
            ddraw_text(vec3(+1,+1,-1), 0.04f, "(%f,%f,%f)", cam.position.x,cam.position.y,cam.position.z);
            ddraw_color(YELLOW);
            ddraw_flush();
        }

        // edit entities
        profile(Editor) {
            editor();
        }

        // apply post-fxs from here
        fx_begin();

        // render scene (foreground objects) with post-effects
        profile(Scene foreground) {
            int scene_flags = 0;
            scene_flags |= do_wireframe ? SCENE_WIREFRAME : 0;
            scene_flags |= do_twosided ? 0 : SCENE_CULLFACE;
            scene_render(SCENE_FOREGROUND | scene_flags);
        }

        profile(Skeletal update) if(!window_has_pause()) {
            float delta = (window_delta()*30); // 30fps anim
            m1.curframe = model_animate(m1, m1.curframe + delta);

            ddraw_text(vec3(-10,5,-10), 0.05, "Frame: %.1f", m1.curframe);
        }

        // post-fxs end here
        fx_end();

        // queue ui
        if( ui_begin("Camera", 0)) {
            if( ui_float("Speed", &cam.speed) ) {}
            if( ui_float3("Position", &cam.position.x) ) {}
            ui_end();
        }
        if( ui_begin("Scene", 0)) {
            if(ui_toggle("Billboard X", &do_billboard_x)) {}
            if(ui_toggle("Billboard Y", &do_billboard_y)) {}
            if(ui_toggle("Billboard Z", &do_billboard_z)) {}
            if(ui_separator()) {}
            if(ui_bool("Wireframe", &do_wireframe)) {}
            if(ui_bool("Two sided", &do_twosided)) {}
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
