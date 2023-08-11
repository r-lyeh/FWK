// PBR model viewer. Based on Foxotron by @gargaj + cce/Peisik (UNLICENSE).
// - rlyeh, public domain.
//
// @todo: Middle mouse button to pan camera @todo

#include "fwk.h"

#if is(tcc) && !is(win32) // @todo: remove this & test on linux
int log2_64 (uint64_t value) {
    const int tab64[64] = {
        63,  0, 58,  1, 59, 47, 53,  2,
        60, 39, 48, 27, 54, 33, 42,  3,
        61, 51, 37, 40, 49, 18, 28, 20,
        55, 30, 34, 11, 43, 14, 22,  4,
        62, 57, 46, 52, 38, 26, 32, 41,
        50, 36, 17, 19, 29, 10, 13, 21,
        56, 45, 25, 31, 35, 16,  9, 12,
        44, 24, 15,  8, 23,  7,  6,  5};
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return tab64[((uint64_t)((value - (value >> 1))*0x07EDD5E59A4E28C2)) >> 58];
}
#define log2 log2_64
#endif

// -----------------------------------------------------------------------------
// textures

texture_t *LoadTextureRGBA8( const char *pathfile, unsigned flags ) {
    int flags_hdr = strendi(pathfile, ".hdr") ? TEXTURE_FLOAT | TEXTURE_RGBA : 0;
    texture_t t = texture(pathfile, flags | TEXTURE_LINEAR | TEXTURE_MIPMAPS | TEXTURE_REPEAT | flags_hdr);
    if( t.id == texture_checker().id ) {
        return NULL;
    }
    texture_t *tex = CALLOC(1, sizeof(texture_t));
    *tex = t;
    return tex;
}

// -----------------------------------------------------------------------------
// models

typedef struct Mesh {
    GLuint vao, vbo, ibo;

    int vert_stride;
    void *vert_stream;
    int num_verts, num_tris;

    int material_idx;
    bool transparent;

    vec3 aabb_min, aabb_max;
} Mesh;

typedef struct Model {
    array(Mesh) meshes;
    array(pbr_material_t) materials;
    unsigned shader;
} Model;

bool ModelLoad( Model *G, const char *_path );
void ModelDestroy( Model *G );
void ModelRebind( Model *G, unsigned shader );
void ModelRender( Model *G, const mat44 _worldRootMatrix );

void ModelDestroy( Model *G) {
    for( int i = 0, end = array_count(G->materials); i < end; ++i ) {
        pbr_material_destroy(&G->materials[i]);
    }
    array_free(G->materials);

    for( int i = 0, end = array_count(G->meshes); i < end; ++i ) {
        Mesh *it = &G->meshes[i];
        glDeleteBuffers( 1, &it->ibo );
        glDeleteBuffers( 1, &it->vbo );
        glDeleteVertexArrays( 1, &it->vao );
    }
    array_free(G->meshes);
}

bool ModelLoad( Model *G, const char *_path ) {
    ModelDestroy(G);

    // -------------------------------------------------------------------------
    Model g = {0};
    *G = g;

    model_t m = model(_path, 0);

    int scn_num_meshes = m.num_meshes;
    int scn_num_materials = array_count(m.materials);

    // -------------------------------------------------------------------------

    for( int i = 0; i < scn_num_materials; i++ ) {
        const char *name = m.materials[i].name;

        PRINTF("Loading material %d/%d: '%s'\n", i + 1, scn_num_materials, name);

        pbr_material_t mt;
        pbr_material(&mt, name);

        array_push(G->materials, mt);
    }

    for( int i = 0; i < scn_num_meshes; i++ ) {
        PRINTF("Loading mesh %d/%d\n", i + 1, scn_num_meshes);

        int verts = m.num_verts;
        int faces = m.num_triangles;
        unsigned material_index = 0; // &m.iqm->meshes[i].material; // aiGetMeshMaterialIndex(scn_mesh[i]);

        bool has_data = verts && faces;
        if( !has_data ) {
            continue;
        }

        PRINTF("Loading mesh v%d/f%d\n", verts, faces);

        Mesh mesh = { 0 };

        mesh.vao = m.vao;
        mesh.vbo = m.vbo;
        mesh.ibo = m.ibo;

        mat44 id; id44(id);
        mesh.aabb_min = model_aabb(m, id).min;
        mesh.aabb_max = model_aabb(m, id).max;

        // p3 n3 t3 b3 u2

        mesh.vert_stride = m.stride;
        mesh.vert_stream = m.verts;

        mesh.num_verts = verts;
        mesh.num_tris = faces;

        mesh.material_idx = material_index;

        // By importing materials before meshes we can investigate whether a mesh is transparent and flag it as such.
        const pbr_material_t* mtl = G->materials ? &G->materials[mesh.material_idx] : NULL;
        mesh.transparent = false;
        if( mtl ) {
            mesh.transparent |= mtl->albedo .texture ? mtl->albedo .texture->transparent : mtl->albedo .color.a < 1.0f;
            mesh.transparent |= mtl->diffuse.texture ? mtl->diffuse.texture->transparent : mtl->diffuse.color.a < 1.0f;
        }

        array_push(G->meshes, mesh);
    }

    #if 0
    G->mGlobalAmbient = vec4( 0.3,0.3,0.3,0.3 );
    int scn_num_lights = 0;
    for( int i = 0; i < scn_num_lights; i++ ) {
        PRINTF("Loading light %d/%d\n", i + 1, scn_num_lights);

        vec4 *color = aiGetLightColor(scn_light[i]);
        char *type = aiGetLightType(scn_light[i]);
        if( 0 == strcmp(type, "AMBIENT") ) {
            memcpy( &G->mGlobalAmbient, &color->r, sizeof( float ) * 4 );
        } else {
            // @todo
        }
    }
    #endif

    return true;
}

void ModelRender( Model *G, const mat44 _worldRootMatrix ) {
    unsigned _shader = G->shader;
    shader_bind( _shader );

    shader_vec4("global_ambient", vec4(1,1,1,1)); // unused

    // loop thrice: first opaque, then transparent backface, then transparent frontface
    for(int j = 0; j < 3; ++j) {
        bool bTransparentPass = j > 0;
        if(bTransparentPass) {
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glCullFace( j == 1 ? GL_FRONT : GL_BACK ); // glDepthMask( GL_FALSE);
        }

        mat44 mat_world; copy44(mat_world, _worldRootMatrix); // @fixme mMatrices[ node.mID ] * _worldRootMatrix
        shader_mat44( "mat_world", mat_world );

        for( int i = 0, end = array_count(G->meshes); i < end; i++ ) {
            const Mesh *mesh = &G->meshes[ i ];
            // Postpone rendering transparent meshes
            if(mesh->transparent != bTransparentPass)
                continue;

            const pbr_material_t *material = &G->materials[ mesh->material_idx ];
            shader_colormap( "map_diffuse", material->diffuse );
            shader_colormap( "map_normals", material->normals );
            shader_colormap( "map_specular", material->specular );
            shader_colormap( "map_albedo", material->albedo );
            shader_colormap( "map_roughness", material->roughness );
            shader_colormap( "map_metallic", material->metallic );
            shader_colormap( "map_ao", material->ao );
            shader_colormap( "map_ambient", material->ambient );
            shader_colormap( "map_emissive", material->emissive );
            shader_float( "specular_shininess", material->specular_shininess ); // unused, basic_specgloss.fs only

            shader_vec2( "resolution", vec2(window_width(),window_height()));

            glActiveTexture(GL_TEXTURE0); // be nice to Mesa before rendering
            glBindVertexArray( mesh->vao );
            glDrawElements( GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_INT, NULL );
        }

        if(bTransparentPass) {
            glDisable( GL_BLEND );
            // glDepthMask( GL_TRUE );
        }
    }

    //glBindVertexArray( 0 );
    //glUseProgram( 0 );
}

static
void G_SetupVertexArray( unsigned _shader, const char *name, int stride, int num_floats, int *offset, int opt_location ) {
    int location = opt_location >= 0 ? opt_location : glGetAttribLocation( _shader, name );
    if( location >= 0 ) {
        glVertexAttribPointer( location, num_floats, GL_FLOAT, GL_FALSE, stride, (GLvoid *)(uintptr_t)(*offset) );
        glEnableVertexAttribArray( location );
    }
    *offset += num_floats * sizeof( GLfloat );
}

void ModelRebind( Model *G, unsigned _shader ) {
    shader_bind(_shader);
    if(_shader == G->shader) return;
    G->shader = _shader;

    for( int i = 0, end = array_count(G->meshes); i < end; i++ ) {
        const Mesh *mesh = &G->meshes[ i ];

        glBindVertexArray( mesh->vao );
        glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->ibo );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );
        glDisableVertexAttribArray( 2 );
        glDisableVertexAttribArray( 3 );
        glDisableVertexAttribArray( 4 );
        glDisableVertexAttribArray( 5 );
        glDisableVertexAttribArray( 6 );
        glDisableVertexAttribArray( 7 );

        int offset = 0, stride = mesh->vert_stride;
        G_SetupVertexArray( _shader, "in_pos", stride, 3, &offset, -1/*0*/ );
        G_SetupVertexArray( _shader, "in_texcoord", stride, 2, &offset, -1/*1*/ );
        G_SetupVertexArray( _shader, "in_normal", stride, 3, &offset, -1/*2*/ );
        G_SetupVertexArray( _shader, "in_tangent", stride, 4, &offset, -1/*3*/ );

        //glBindVertexArray( 0 );
    }
}

// -----------------------------------------------------------------------------
// skyboxes

// Extracts single key value from an HDRLabs IBL file. Returns an empty string on error.
static const char *ibl_readkey( const char* pathfile, const char* key ) {
    char *data = vfs_read(pathfile);
    if( data ) {
        const char *found = strstr(data, va("%s=", key));
        if( found ) return found + strlen(key) + 1;
        found = strstr(data, va("%s =", key));
        if( found ) return found + strlen(key) + 2;
    }
    return "";
}

typedef struct Skybox {
    vec3 sunColor; float sunYaw, sunPitch;   // ibl settings
    texture_t *reflection;                   // reflection map (hdr)
    texture_t *env;                          // irradiance map (env)
} Skybox;

Skybox g_skybox = { {1,1,1} };

void SkyboxDestroy( Skybox *s ) {
    if( s->reflection ) texture_destroy( s->reflection );
    if( s->env ) texture_destroy( s->env );
    *s = (Skybox){0};
}

bool SkyboxLoad( Skybox *s, const char **slots ) { // hdr,env,ibl
    SkyboxDestroy( s );

    const char* reflectionPath = slots[0];
    const char* envPath = slots[1];
    const char* iblPath = slots[2];

//  unsigned invalid = texture_checker().id;

    // Reflection map
    if( reflectionPath ) {
        if( (s->reflection = LoadTextureRGBA8( reflectionPath, TEXTURE_SRGB )) != NULL ) {
            glBindTexture( GL_TEXTURE_2D, s->reflection->id );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }

    // Irradiance map
    if( envPath ) {
        if( (s->env = LoadTextureRGBA8( envPath, TEXTURE_SRGB )) != NULL ) {
            glBindTexture( GL_TEXTURE_2D, s->env->id );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }

    // Sun color & direction from .ibl file
    s->sunColor = vec3(1,1,1);
    s->sunYaw = 0, s->sunPitch = 0;
    if( iblPath ) {
        vec3 sc; if( 3 == sscanf(ibl_readkey(iblPath, "SUNcolor"), "%f,%f,%f", &sc.x, &sc.y, &sc.z) ) {
            s->sunColor = scale3(sc, 1/255.f);
        }
        vec2 uv = vec2(atof(ibl_readkey(iblPath, "SUNu")), atof(ibl_readkey(iblPath, "SUNv")));
        if( len2(uv) > 0 ) {
            s->sunYaw = C_PI * (-2. * uv.x + 0.5f);
            s->sunPitch = (0.5f - uv.y) * C_PI;
        }
    }

    return s->reflection && s->env;
}

// -----------------------------------------------------------------------------
// main

const char *shader_names[] = {"Physically Based", "Basic SpecGloss" };
const char *shaders[][2] = { // name, vs, fs
    {  "Shaders/pbr.vs", "Shaders/pbr.fs" },
    { "Shaders/basic_specgloss.vs", "Shaders/basic_specgloss.fs" }
};

const char *skyboxes[][3] = { // reflection, env, metadata
    {"hdr/Tokyo_BigSight_1k.hdr","hdr/Tokyo_BigSight_Env.hdr","hdr/Tokyo_BigSight.ibl"},
    {"hdr/GCanyon_C_YumaPoint_1k.hdr","hdr/GCanyon_C_YumaPoint_Env.hdr","hdr/GCanyon_C_YumaPoint.ibl"},
    {"hdr/Factory_Catwalk_1k.hdr","hdr/Factory_Catwalk_Env.hdr","hdr/Factory_Catwalk.ibl"},
    {"hdr/MonValley_G_DirtRoad_1k.hdr","hdr/MonValley_G_DirtRoad_Env.hdr","hdr/MonValley_G_DirtRoad.ibl"},
    {"hdr/Shiodome_Stairs_1k.hdr","hdr/Shiodome_Stairs_Env.hdr","hdr/Shiodome_Stairs.ibl"},
};

Model gModel;
unsigned gShader = ~0u;
unsigned gShaderConfig = ~0u;

bool LoadShaderConfig( int slot ) { // name,vs,fs
    unsigned newShader = shader( vfs_read(shaders[slot][0]), vfs_read(shaders[slot][1]), NULL, NULL );
    if( newShader == ~0u ) return false;

    shader_destroy( gShader );
    gShaderConfig = slot;
    gShader = newShader;

    return true;
}

void camera_fit(camera_t *cam) {
    vec3 target = scale3( add3( gModel.meshes[0].aabb_min, gModel.meshes[0].aabb_max ), 0.5f);
    float distance = len3( sub3( gModel.meshes[0].aabb_max, gModel.meshes[0].aabb_min ) ) * 0.85f;
    cam->position = add3(target, scale3(norm3(sub3(cam->position,target)), distance));
    camera_lookat(cam, vec3(0,0,0));
}

int main( int argc, const char *argv[] ) {
    window_create( 75, WINDOW_MSAA2 );
    window_title(__FILE__);

    // load all fx files in all subdirs
    fx_load("fx**.fs");

    if( !LoadShaderConfig( 0 ) ) {
        return -4;
    }

    brdf_lut();

    // -------------------------------------------------------------------------
    // Mainloop
    float model_yaw = 0, model_pitch = 0;
    float lightYaw = 0.0f;
    float lightPitch = 0.0f;
    vec4  skyBackgroundColor = vec4(0.01,0.01,0.02,1); // vec4(1,0,0,1);
    float skyExposure = 1.0; // plain 'exposure' instead? this is camera related
    float skyBlur = 0.00; // 0.00
    float skyOpacity = 0.99; // 0.99
    bool do_wireframe = false;
    bool do_xzySpace = true; // xzySpace or xyzSpace
    bool do_flipY = false;
    const mat44 xzyMatrix = {
        1, 0, 0, 0,
        0, 0, 1, 0,
        0,+1, 0, 0,
        0, 0, 0, 1 };

    camera_t cam = camera(); cam.speed = 0.1;

    int firstskyboxes = 0; // 0: tokyo_bigsight
    SkyboxLoad( &g_skybox, &skyboxes[firstskyboxes][0] );
    lightYaw = g_skybox.sunYaw;
    lightPitch = g_skybox.sunPitch;

    unsigned skysphereShader = shader( vfs_read("Skyboxes/skysphere.vs"), vfs_read("Skyboxes/skysphere.fs"), NULL, NULL );
    Model skysphere = { 0 }; ModelLoad(&skysphere, "Skyboxes/skysphere.fbx"); ModelRebind(&skysphere, skysphereShader);

    if( ModelLoad( &gModel, argc > 1 && argv[1][0] != '-' ? argv[ 1 ] : "damagedhelmet.gltf" ) ) {
        ModelRebind( &gModel, gShader );
    }

    cam.position = vec3(+1,0,+1);
    camera_fit(&cam);

    static mat44 worldRootXYZ; do_once id44(worldRootXYZ); // mat44( 1.0f );

    while( window_swap() && !input(KEY_ESC) ) {

        if( input(KEY_F5) ) window_reload();
        if( input_down( KEY_F ) ) camera_fit(&cam);

        // ---------------------------------------------------------------------

        static int fps_mode;
        if(input_down(KEY_TAB)) { fps_mode ^= 1; camera_fit(&cam); }
        if(fps_mode) {
            // fps camera
            bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
            window_cursor( !active );
            if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
            vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
            vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
            camera_move(&cam, wasdecq.x,wasdecq.y,wasdecq.z);
            camera_fps(&cam, mouse.x,mouse.y);
        } else {
            // orbit camera
            window_cursor( true );
            bool active = !ui_active() && !ui_hover() && !gizmo_active();
            vec2 inc_mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active * input(MOUSE_L));
            float inc_distance = -0.2f * active * input_diff(MOUSE_W);
            camera_orbit(&cam, inc_mouse.x, inc_mouse.y, inc_distance);
            // rotate model
            model_yaw -= input_diff(MOUSE_X) * 0.2f * active * input(MOUSE_R);
            model_pitch += input_diff(MOUSE_Y) * 0.2f * active * input(MOUSE_R);
        }

        // ---------------------------------------------------------------------

        glClearColor( skyBackgroundColor.r, skyBackgroundColor.g, skyBackgroundColor.b, skyBackgroundColor.a );
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);

        // ---------------------------------------------------------------------
        // Mesh state

        fx_begin();

        profile("PBR Model (bindings)") {
            ModelRebind( &gModel, gShader );

            shader_mat44( "mat_projection", cam.proj );

            //cameraPosition = scale3(cameraPosition, gCameraDistance);
            shader_vec3( "camera_position", cam.position );

            vec3 lightDirection = vec3( 0, 0, 1 );
            lightDirection = rotatex3( lightDirection, deg(lightPitch) );
            lightDirection = rotatey3( lightDirection, deg(lightYaw) );

            vec3 fillLightDirection = vec3( 0, 0, 1 );
            fillLightDirection = rotatex3( fillLightDirection, deg(lightPitch - 0.4f) );
            fillLightDirection = rotatey3( fillLightDirection, deg(lightYaw + 0.8f) );

            shader_vec3( "lights[0].direction", lightDirection );
            shader_vec3( "lights[0].color", g_skybox.sunColor );
            shader_vec3( "lights[1].direction", fillLightDirection );
            shader_vec3( "lights[1].color", vec3( 0.5f, 0.5f, 0.5f ) );
            shader_vec3( "lights[2].direction", neg3(fillLightDirection) );
            shader_vec3( "lights[2].color", vec3( 0.25f, 0.25f, 0.25f ) );

            shader_float( "skysphere_rotation", lightYaw - g_skybox.sunYaw );

            mat44 viewMatrix, inv_viewMatrix;
            copy44(viewMatrix, cam.view);
            invert44( inv_viewMatrix, viewMatrix);
            shader_mat44( "mat_view", viewMatrix );
            shader_mat44( "mat_view_inverse", inv_viewMatrix );

            shader_bool( "has_tex_skysphere", g_skybox.reflection != NULL );
            shader_bool( "has_tex_skyenv", g_skybox.env != NULL );
            if( g_skybox.reflection ) {
                float mipCount = floor( log2( g_skybox.reflection->h ) );
                shader_texture( "tex_skysphere", *g_skybox.reflection );
                shader_float( "skysphere_mip_count", mipCount );
            }
            if( g_skybox.env ) {
                shader_texture( "tex_skyenv", *g_skybox.env );
            }
            shader_texture( "tex_brdf_lut", brdf_lut() );
            shader_float( "exposure", skyExposure );
            shader_uint( "frame_count", (unsigned)window_frame() );
        }

        // ---------------------------------------------------------------------
        // Mesh render

        mat44 M;
        copy44( M, do_xzySpace ? xzyMatrix : worldRootXYZ );
        if( do_flipY ) scale44( M, 1,-1,1 );
        rotate44( M, model_yaw, 0,0,1 );
        rotate44( M, model_pitch, 1,0,0 );

        profile("PBR Model (render)") {
            ModelRender( &gModel, M );
        }

        profile("PBR Model (wireframe)") {
            if( do_wireframe ) {
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                glDepthFunc( GL_LEQUAL );

                shader_float("exposure", 100.0f );
                ModelRender( &gModel, M );

                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                glDepthFunc( GL_LESS );
            }
        }

        // ---------------------------------------------------------------------
        // Skysphere render

        profile("PBR Skybox") {
            ModelRebind(&skysphere, skysphereShader );

            mat44 projview; multiply44x2(projview, cam.proj, cam.view);
            shader_mat44( "mat_mvp", projview );

            shader_bool( "has_tex_skysphere", g_skybox.reflection != NULL );
            shader_bool( "has_tex_skyenv", g_skybox.env != NULL );

            if( g_skybox.reflection ) {
                const float mipCount = floor( log2( g_skybox.reflection->h ) );
                shader_texture( "tex_skysphere", *g_skybox.reflection );
                shader_float( "skysphere_mip_count", mipCount );
            }

            if( g_skybox.env ) {
                shader_texture( "tex_skyenv", *g_skybox.env );
            }

            shader_vec4( "background_color", skyBackgroundColor );
            shader_float( "skysphere_blur", skyBlur );
            shader_float( "skysphere_opacity", skyOpacity );
            shader_float( "skysphere_rotation", lightYaw - g_skybox.sunYaw  );
            shader_float( "exposure", skyExposure );
            shader_uint( "frame_count", (unsigned)window_frame() );

            glDepthFunc( GL_LEQUAL );
            ModelRender(&skysphere, worldRootXYZ );
            glDepthFunc( GL_LESS );
        }

        fx_end();

        // ---------------------------------------------------------------------
        // UI

        if( ui_panel( "Viewer", 0 ) ) {
            ui_bool( "Wireframe", &do_wireframe );
            ui_separator();

            if( ui_radio("Shader config:", shader_names, countof(shader_names), &gShaderConfig) ) {
                LoadShaderConfig( gShaderConfig );
                ModelRebind(&gModel, gShader );
            }

            ui_separator();
            for( int i = 0; i < countof(skyboxes); i++ ) {
                const char *filename = skyboxes[i][0];
                bool selected = !strcmp(g_skybox.reflection->filename, file_name(filename));
                if( ui_bool( filename, &selected ) ) {
                    SkyboxLoad(  &g_skybox, &skyboxes[i][0] );
                    lightYaw = g_skybox.sunYaw;
                    lightPitch = g_skybox.sunPitch;
                }
            }

            ui_separator();
            ui_float( "Sky exposure", &skyExposure); skyExposure = clampf(skyExposure, 0.1f, 4.0f );
            ui_float( "Sky blur", &skyBlur); skyBlur = clampf(skyBlur, 0.0f, 1.0f );
            ui_float( "Sky opacity", &skyOpacity); skyOpacity = clampf(skyOpacity, 0.0f, 1.0f );
            ui_color4f( "Sky background", (float *) &skyBackgroundColor.x );

            ui_separator();
            ui_float( "SunLight Yaw", &lightYaw );
            ui_float( "SunLight Pitch", &lightPitch );

            ui_panel_end();
        }

        if( ui_panel( "Model", 0 ) ) {
            ui_label(va("Material count: %d", array_count(gModel.materials)));
            ui_label(va("Mesh count: %d", array_count(gModel.meshes)));
            int triCount = 0; for( int i = 0, end = array_count(gModel.meshes); i < end; ++i ) triCount += gModel.meshes[i].num_tris;
            ui_label(va("Triangle count: %d", triCount));
            ui_separator();

            bool xyzSpace = !do_xzySpace;
            if( ui_bool( "XYZ space", &xyzSpace ) ) {
                do_xzySpace = !do_xzySpace;
            }
            ui_bool( "XZY space", &do_xzySpace );
            ui_bool( "invert Y", &do_flipY );

            ui_separator();
            for( int i = 0, end = array_count(gModel.materials); i < end; ++i ) {
                pbr_material_t *it = &gModel.materials[i];
                ui_label(va("Name: %s", it->name));
                ui_float( "Specular shininess", &it->specular_shininess );
                ui_separator(); if(ui_colormap( "Albedo",    &it->albedo    )) colormap(&it->albedo   , dialog_load(), 1);
                ui_separator(); if(ui_colormap( "Ambient",   &it->ambient   )) colormap(&it->ambient  , dialog_load(), 0);
                ui_separator(); if(ui_colormap( "AO",        &it->ao        )) colormap(&it->ao       , dialog_load(), 0);
                ui_separator(); if(ui_colormap( "Diffuse",   &it->diffuse   )) colormap(&it->diffuse  , dialog_load(), 1);
                ui_separator(); if(ui_colormap( "Emissive",  &it->emissive  )) colormap(&it->emissive , dialog_load(), 1);
                ui_separator(); if(ui_colormap( "Metallic",  &it->metallic  )) colormap(&it->metallic , dialog_load(), 0);
                ui_separator(); if(ui_colormap( "Normal",    &it->normals   )) colormap(&it->normals  , dialog_load(), 0);
                ui_separator(); if(ui_colormap( "Roughness", &it->roughness )) colormap(&it->roughness, dialog_load(), 0);
                ui_separator(); if(ui_colormap( "Specular",  &it->specular  )) colormap(&it->specular , dialog_load(), 0);
            }

            ui_panel_end();
        }

        if( ui_panel("FX", 0) ) {
            for( int i = 0; i < 64; ++i ) {
                char *name = fx_name(i); if( !name ) break;
                bool b = fx_enabled(i);
                if( ui_bool(name, &b) ) fx_enable(i, fx_enabled(i) ^ 1);
            }
            ui_panel_end();
        }

        if( ui_panel("Help", 0)) {
            if( fps_mode ) {
            ui_label("TAB: switch to Orbit camera mode");
            ui_label("WASD,QEC: move camera");
            ui_label("Drag + Mouse Button: camera freelook");
            } else {
            ui_label("TAB: switch to FPS camera mode");
            ui_label("Drag + Left Mouse Button: orbit camera");
            ui_label("Drag + Right Mouse Button: rotate model");
            ui_label("Mouse wheel: camera distance");
            }
            ui_label("F: center view");
            ui_panel_end();
        }
    }
}
