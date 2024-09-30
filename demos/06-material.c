// material demo
// - rlyeh, public domain
//
// @todo: object_print(obj, "");

#include "engine.h"


const char *skyboxes[][3] = { // reflection, rad, env
    {"hdr/Tokyo_BigSight_1k.hdr","hdr/Tokyo_BigSight_1k.hdr","hdr/Tokyo_BigSight_Env.hdr"},
#if 0
    {"hdr/GCanyon_C_YumaPoint_1k.hdr","hdr/GCanyon_C_YumaPoint_1k.hdr","hdr/GCanyon_C_YumaPoint_Env.hdr"},
    {"hdr/Factory_Catwalk_1k.hdr","hdr/Factory_Catwalk_Rad.hdr","hdr/Factory_Catwalk_Env.hdr"},
    {"hdr/graffiti_shelter_4k.hdr","hdr/graffiti_shelter_Rad.hdr","hdr/graffiti_shelter_Env.hdr"},
    {"hdr/music_hall_01_4k.hdr","hdr/music_hall_01_4k.hdr","hdr/music_hall_01_Env.hdr"},
    {"hdr/the_sky_is_on_fire_2k.hdr","hdr/the_sky_is_on_fire_2k.hdr","hdr/the_sky_is_on_fire_Env.hdr"},
    {"hdr/MonValley_G_DirtRoad_1k.hdr","hdr/MonValley_G_DirtRoad_1k.hdr","hdr/MonValley_G_DirtRoad_Env.hdr"},
    {"hdr/Shiodome_Stairs_1k.hdr","hdr/Shiodome_Stairs_1k.hdr","hdr/Shiodome_Stairs_Env.hdr"},
    {"hdr/mesto.hdr","hdr/mesto.hdr","hdr/mesto_Env.hdr"},
#endif
};

int main() {
    // create the window
    window_create( 0.75f, 0 );
    window_color( GRAY );

    // create camera
    camera_t cam = camera();

    // fx: load all post fx files in all subdirs.
    fx_load("fx/**.fs");
    fx_enable_ordered(fx_find("fxTonemapACES.fs"));
    fx_enable_ordered(fx_find("fxFXAA3.fs"));
 
    // load video, RGB texture, no audio
    video_t *v = video( "pexels-pachon-in-motion-17486489.mp4", VIDEO_RGB | VIDEO_NO_AUDIO | VIDEO_LOOP ); video_seek(v, 30);
    // load texture
    texture_t t1 = texture("kgirl/g01_texture.png", 0); 
    texture_t t2 = texture("matcaps/material3", 0); 
    // load model 
    model_t m1 = model("suzanne.obj", MODEL_NO_ANIMATIONS|MODEL_NO_PBR);
    model_t m5 = model("suzanne.obj", MODEL_NO_ANIMATIONS|MODEL_NO_PBR);
    model_t m2 = model("suzanne.obj", MODEL_NO_ANIMATIONS|MODEL_NO_PBR|MODEL_MATCAPS);
    model_t m3 = model("damagedhelmet.gltf", MODEL_NO_ANIMATIONS);
    // model_t m3 = model("Scutum_low.fbx", MODEL_NO_ANIMATIONS|MODEL_PBR); 
    model_t m4 = model("cube.obj", MODEL_NO_ANIMATIONS|MODEL_NO_PBR); 
    model_t m6 = model("cube.obj", MODEL_NO_ANIMATIONS); 
    // model_t m4 = model("avp/scene.gltf", MODEL_NO_ANIMATIONS|MODEL_PBR);
    // model_t m3 = model("Cerberus_LP.FBX", MODEL_NO_ANIMATIONS|MODEL_PBR); 

    array(char*) list = 0;
    for each_array( vfs_list("demos/art/shadertoys/**.fs"), char*, dir ) {
        array_push(list, STRDUP(file_name(dir)));
    }

    shadertoy_t sh = shadertoy(*list, SHADERTOY_IGNORE_MOUSE|SHADERTOY_FLIP_Y); // 0:no flags
    sh.dims.x = 1024;
    sh.dims.y = 1024;
    shadertoy_render(&sh, 0);

    // spawn object1 (diffuse)
    object_t* obj1 = scene_spawn();
    object_model(obj1, m1);
    object_diffuse(obj1, t1);
    object_scale(obj1, vec3(3,3,3));
    object_move(obj1, vec3(-10+5*0,0,-10));

    // spawn object2 (matcap)
    object_t* obj2 = scene_spawn();
    object_model(obj2, m2);
    object_diffuse(obj2, t2);
    object_scale(obj2, vec3(3,3,3));
    object_move(obj2, vec3(-10+5*2,0,-10));

    // spawn object3 (video)
    object_t* obj3 = scene_spawn();
    object_model(obj3, m5);
    object_diffuse(obj3, video_textures(v)[0]);
    object_scale(obj3, vec3(3,3,3));
    object_move(obj3, vec3(-10+5*1,0,-10));

    // spawn object4 (pbr)
    object_t* obj4 = scene_spawn();
    object_model(obj4, m3);
    object_scale(obj4, vec3(3,3,3));
    object_move(obj4, vec3(-10+6*3,0,-30));
    object_pivot(obj4, vec3(0,180,180));

    obj4->model.materials[0].layer[MATERIAL_CHANNEL_EMISSIVE].value = 30.0f;

    // spawn object5 (shadertoy)
    object_t* obj5 = scene_spawn();
    object_model(obj5, m4); 
    object_diffuse(obj5, sh.tx); 
    object_scale(obj5, vec3(3,3,3));
    object_move(obj5, vec3(-10+8*3,0,-10));

    // spawn object6 (solid)
    object_t* obj6 = scene_spawn();
    object_model(obj6, m6); 
    // object_diffuse(obj6, (texture)); 
    object_scale(obj6, vec3(3,3,3));
    object_move(obj6, vec3(-10+12*3,0,-10));

    obj6->model.materials[0].layer[MATERIAL_CHANNEL_ALBEDO].map.color = vec4(1,1,1,1);
    obj6->model.materials[0].layer[MATERIAL_CHANNEL_AMBIENT].map.color = vec4(1,1,1,1);
    obj6->model.materials[0].layer[MATERIAL_CHANNEL_EMISSIVE].map.color = vec4(1,0,0,1);
    obj6->model.materials[0].layer[MATERIAL_CHANNEL_EMISSIVE].value = 10.0f;
    obj6->model.materials[0].layer[MATERIAL_CHANNEL_AO].map.color = vec4(1,1,1,1);
 
    // scene_spawn_light(); // sun
    light_t* l = scene_spawn_light(); 
    light_type(l, LIGHT_POINT);
    l->diffuse = vec3(1,1,1);
 
    // load skybox
    scene_skybox(skybox_pbr(skyboxes[0][0], skyboxes[0][1], skyboxes[0][2]));
 
    fbo_t main_fb = fbo(window_width(), window_height(), 0, TEXTURE_FLOAT);

    scene_t *scene = scene_get_active();

    bloom_params_t bloom_params = {
        .mips_count = 12,
        .filter_radius = 0.005f,
        .strength = 0.04f,
    };

    while(window_swap() && !input(KEY_ESC)) { 
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );

        // draw environment 
        fbo_resize(&main_fb, window_width(), window_height());
 
        // update video
        video_decode( v );

        // update light position
        light_pos(l, cam.position);

        // update shadertoy
        shadertoy_render(&sh, window_delta());

        // fps camera
        camera_freefly(&cam);

        fbo_resize(&main_fb, window_width(), window_height());
        fbo_bind(main_fb.id);
            viewport_clear(false, true);
            viewport_clip(vec2(0,0), vec2(window_width(), window_height()));
            scene_render(SCENE_BACKGROUND|SCENE_FOREGROUND|SCENE_SHADOWS|SCENE_DRAWMAT);
            fx_drawpass(fx_find("fx/fxSSAO.fs"), main_fb.texture_color, main_fb.texture_depth);
        fbo_unbind();

        {
            texture_t bloom_fb = fxt_bloom(main_fb.texture_color, bloom_params);
            fbo_blit(main_fb.id, bloom_fb, 1);
            // fullscreen_quad_rgb_flipped(reflect_fb);
        }

        fx_apply(main_fb.texture_color, main_fb.texture_depth);

        if (ui_panel("FXs", 0)) {
            ui_fxs();
            ui_panel_end();
        }

        if (ui_panel("Materials", 0)) {
            ui_materials(&obj4->model);
            ui_panel_end();
        }

        if( ui_panel( "Viewer", 0 ) ) {
            for( int i = 0; i < countof(skyboxes); i++ ) {
                const char *filename = skyboxes[i][0];
                // bool selected = !strcmp(g_skybox.reflection->filename, file_name(filename));
                bool selected = false; 
                if( ui_bool( filename, &selected ) ) {
                    scene_skybox(skybox_pbr(skyboxes[i][0], skyboxes[i][1], skyboxes[i][2]));
                }
            }
            ui_section("bloom");
            ui_int("Mips Count", &bloom_params.mips_count);
            ui_float("Filter Radius", &bloom_params.filter_radius);
            ui_float("Strength", &bloom_params.strength);
            ui_float("Threshold", &bloom_params.threshold);
            ui_float("Soft Threshold", &bloom_params.soft_threshold);
            ui_bool("Suppress Fireflies", &bloom_params.suppress_fireflies);
            ui_float("Cube emission",  &obj6->model.materials[0].layer[MATERIAL_CHANNEL_EMISSIVE].value);
            ui_panel_end();
        }

        static int selected = 0;
        if( ui_panel("Shadertoy", 0)) {
            for( int i = 0; i < array_count(list); ++i ) {
                bool in_use = i == selected;
                if( ui_bool(list[i], &in_use) ) {
                    sh = shadertoy( list[selected = i], SHADERTOY_IGNORE_MOUSE|SHADERTOY_FLIP_Y );
                    sh.dims.x = 1024;
                    sh.dims.y = 1024;
                    shadertoy_render(&sh, 0);
                    object_diffuse(obj5, sh.tx);
                }
            }
            ui_panel_end();
        }
    }
}
