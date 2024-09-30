 #include "engine.h"


const char *skyboxes09b[][3] = { // reflection, rad, env
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


int OBJ09s_MDL = 0;
const char *OBJ09s_MDLS[] = {
    "meshes/ShadowsTest.obj",
    "meshes/sphere.obj",
    "meshes/suzanne.obj",
    "meshes/gazebo.obj",
};

int main(int argc, char** argv) {
    window_create(85, 0);
    window_title(__FILE__);
    // window_fps_unlock();

    fx_load("fx/**.fs");
    fx_enable(fx_find("fxTonemapUchimura.fs"), 1);

    camera_t cam = camera(); {
        cam.position = vec3(0, 7.5, 15);
        cam.pitch = -15;
        cam.yaw = -90;
        camera_fps(&cam, 0, 0);
    }
    skybox_t sky = {0};
    model_t mdl = {0};
    shadowmap_t sm = {0};

    light_t lit = light(); {
        lit.type = LIGHT_POINT;
        lit.cast_shadows = true;
        // lit.shadow_distance = 1.0f;
        // lit.falloff.linear = 0.5f;
        lit.falloff.quadratic = 0.1f;
    }
    light_t lit2 = light(); {
        lit2.type = LIGHT_POINT;
        lit2.cast_shadows = true;
        lit2.diffuse = scale3(vec3(1, 0.7, 0.8), 1.0f);
        // lit2.shadow_distance = 1.0f;
        // lit2.falloff.linear = 0.5f;
        lit2.falloff.quadratic = 0.1f;
    }
    light_t lit3 = light(); {
        lit3.type = LIGHT_SPOT;
        lit3.cast_shadows = true;
        // lit3.shadow_distance = 5.0f;
        lit3.diffuse = scale3(vec3(1, 0.7, 0.8), 1.0f);
    }
    light_t lit4 = light(); {
        lit4.type = LIGHT_DIRECTIONAL;
        lit4.cast_shadows = true;
        // lit4.shadow_distance = 2000.0f;
        lit4.diffuse = scale3(vec3(1, 0.7, 0.8), 1.0f);
    }
    light_t lit5 = light(); {
        lit5.type = LIGHT_POINT;
        lit5.cast_shadows = true;
        // lit4.shadow_distance = 2000.0f;
        lit5.diffuse = scale3(vec3(1, 0, 0), 1.0f);
    }
    light_t lit6 = light(); {
        lit6.type = LIGHT_POINT;
        lit6.cast_shadows = true;
        // lit4.shadow_distance = 2000.0f;
        lit6.diffuse = scale3(vec3(0, 1, 0), 1.0f);
    }
    light_t lit7 = light(); {
        lit7.type = LIGHT_POINT;
        lit7.cast_shadows = true;
        lit7.radius = 0.1f;
    }

    array(light_t) point_lights = 0;
    array_push(point_lights, lit);
    array_push(point_lights, lit2);
    
    array(light_t) spot_lights = 0;
    array_push(spot_lights, lit3);

    array(light_t) directional_lights = 0;
    array_push(directional_lights, lit4);

    array(light_t) all_lights = 0;
    array_push(all_lights, lit);
    array_push(all_lights, lit2);
    array_push(all_lights, lit3);
    array_push(all_lights, lit4);
    array_push(all_lights, lit5);
    array_push(all_lights, lit6);

    bool initialized = 0;
    bool must_reload = 0;

    while( window_swap()) {
        if (input_down(KEY_ESC)) break;
        // reloading
        if( must_reload ) {
            must_reload = 0;
            skybox_destroy(&sky);
            model_destroy(mdl);
            initialized = 0;
        }
        if( !initialized ) {
            initialized = 1;
            sky = skybox_pbr(skyboxes09b[0][0], skyboxes09b[0][1], skyboxes09b[0][2]);
            sm = shadowmap(512, 4096);
            mdl = model(OBJ09s_MDLS[OBJ09s_MDL], 0);
            model_skybox(&mdl, sky);
        }

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        camera_freefly(&cam);

        enum {
            POINT, SPOT, DIR, ALL
        };
        static unsigned mode = ALL;

        if (!ui_active()) {
            if (input_down(KEY_1)) mode = POINT;
            if (input_down(KEY_2)) mode = SPOT;
            if (input_down(KEY_3)) mode = DIR;
            if (input_down(KEY_4)) mode = ALL;
        }
        light_t *lights = 0;
        switch (mode) {
            case POINT: lights = point_lights; break;
            case SPOT: lights = spot_lights; break;
            case DIR: lights = directional_lights; break;
            case ALL: lights = all_lights; break;
        }
 
        // Animate light
        if (mode == POINT) {
            lights[0].pos = vec3(0, 5.5, 1);
            lights[0].pos.x += sinf(window_time()*2)*4.5f;
            lights[0].pos.y += cosf(window_time()*2)*1.0;
            lights[0].pos.z += cosf(window_time()*2)*6.0;

            lights[1].pos = vec3(0, 7.5, 1);
            lights[1].pos.x += sinf(window_time()*4)*4.5f;
            lights[1].pos.y += cosf(window_time()*4)*1.0;
            lights[1].pos.z += cosf(window_time()*4)*6.0;
        }
 
        static bool camera_spot = true;
        if (input_down(KEY_SPACE)) camera_spot = !camera_spot;
        if (mode == SPOT && camera_spot) {
            lights[0].pos = cam.position;
            lights[0].pos = add3(lights[0].pos, scale3(cam.lookdir, 1.5f));
            vec3 rhs = cross3(cam.lookdir, vec3(0,1,0));
            lights[0].pos = add3(lights[0].pos, scale3(rhs, 0.5));
            lights[0].dir = cam.lookdir;
        }

        if (mode == DIR) {
            lights[0].dir = vec3(1,-1,-1);
        }

        if (mode == ALL) {
            lights[0].pos = vec3(0, 5.5, 1);
            lights[0].pos.x += sinf(window_time()*2)*4.5f;
            lights[0].pos.y += cosf(window_time()*2)*1.0;
            lights[0].pos.z += cosf(window_time()*2)*6.0;

            lights[1].pos = vec3(0, 7.5, 1);
            lights[1].pos.x += sinf(window_time()*4)*4.5f;
            lights[1].pos.y += cosf(window_time()*4)*1.0;
            lights[1].pos.z += cosf(window_time()*4)*6.0;

            lights[2].pos = cam.position;
            lights[2].pos = add3(lights[2].pos, scale3(cam.lookdir, 1.5f));
            vec3 rhs = cross3(cam.lookdir, vec3(0,1,0));
            lights[2].pos = add3(lights[2].pos, scale3(rhs, 0.5));
            lights[2].dir = cam.lookdir;

            lights[3].dir = vec3(1,-1,-1);

            lights[4].pos = vec3(-3, 3, 0);
            lights[5].pos = vec3(3, 3, 0);
        }

        // Render shadowmap
        shadowmap_begin(&sm);
        {
            for (int i = 0; i < array_count(lights); i++) {
                while (shadowmap_step(&sm)) {
                    shadowmap_light(&sm, &lights[i], cam.proj, cam.view);
                    model_render(mdl, cam.proj, cam.view, mdl.pivot);
                }
            }
        }
        shadowmap_end(&sm);

        // render
        mat44 mvp; multiply44x2(mvp, cam.proj, cam.view);
        fx_begin();
        {
            skybox_render(&sky, cam.proj, cam.view);

            model_shadow(&mdl, &sm);
            model_light(&mdl, array_count(lights), lights);
            model_render(mdl, cam.proj, cam.view, mdl.pivot);
        }
        fx_end(0,0);

        // {
            // quad_render_id(GL_TEXTURE_2D, sm.maps[0].texture_2d[0], vec2(sm.csm_texture_width, sm.csm_texture_width), vec2(0,0), vec2(4096,4096), 0xFFFFFFFF, vec2(0,0), vec2(512,512));
        // }

        if( ui_panel("Scene", 0)) {
            // if( ui_list("Skybox", SKY_DIRS, countof(SKY_DIRS), &SKY_DIR) ) {
            //     must_reload = 1;
            // }
            if( ui_list("Model", OBJ09s_MDLS, countof(OBJ09s_MDLS), &OBJ09s_MDL) ) {
                must_reload = 1;
            }
            ui_separator();
            for( int i = 0; i < countof(skyboxes09b); i++ ) {
                const char *filename = skyboxes09b[i][0];
                bool selected = false;
                if( ui_bool( filename, &selected ) ) {
                    sky = skybox_pbr(skyboxes09b[i][0], skyboxes09b[i][1], skyboxes09b[i][2]);
                    model_skybox(&mdl, sky);
                }
            }
            ui_separator();
            ui_materials(&mdl);
            ui_separator();
            ui_lights(array_count(lights), lights);
            ui_separator();
            ui_shadowmap(&sm);
            ui_panel_end();
        }
    }
}
