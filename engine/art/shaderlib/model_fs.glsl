#ifndef MODEL_FS_GLSL
#define MODEL_FS_GLSL
#define FS_PASS

struct ColorMap
{
    bool has_tex;
    vec4 color;
};

#ifdef FS_PASS
uniform ColorMap map_albedo;    uniform sampler2D map_albedo_tex;
uniform ColorMap map_normals;   uniform sampler2D map_normals_tex;
uniform ColorMap map_roughness; uniform sampler2D map_roughness_tex;
uniform ColorMap map_metallic;  uniform sampler2D map_metallic_tex;
uniform ColorMap map_ao;        uniform sampler2D map_ao_tex;
uniform ColorMap map_ambient;   uniform sampler2D map_ambient_tex;
uniform ColorMap map_emissive;  uniform sampler2D map_emissive_tex;
uniform ColorMap map_parallax;  uniform sampler2D map_parallax_tex;
#endif

#define sample_colormap(ColorMap_, uv_) \
    (ColorMap_.has_tex ? texture( ColorMap_##_tex, uv_ ) : ColorMap_.color)

uniform mat4 model, view, inv_view;
uniform vec3 u_coefficients_sh[9];
uniform bool u_lit; /// set:0
uniform bool u_matcaps; /// set:0
uniform float u_cutout_alpha; /// set:0.75
uniform float u_emissive_value; /// set:1.0
// lightmapping
uniform sampler2D u_lightmap;
uniform bool u_texlit;
uniform bool u_texmod; /// set:1
uniform float u_litboost; /// set:1

in vec3 v_position;
in vec3 v_position_ws;
in vec4 vpeye;
in vec4 vneye;

#ifdef RIM
uniform mat4 M; // RIM
uniform vec3 u_rimcolor; /// set:0.05,0.05,0.05
uniform vec3 u_rimrange; /// set:0.11,0.98,0.5
uniform vec3 u_rimpivot; /// set:0,0,0
uniform bool u_rimambient; /// set:1
#endif
in vec3 v_normal, v_normal_ws;
in vec2 v_texcoord, v_texcoord2;
in vec4 v_color;
in vec3 v_tangent, v_tangent_ws;
in vec3 v_tangent_view, v_tangent_world;
in vec3 v_binormal;
in vec3 v_to_camera;
in vec3 v_vertcolor;
in float v_depth;
in mat3 v_tbn;
out vec4 fragcolor;

uniform float u_global_alpha; /// set:1.0
uniform float u_global_opacity; /// set:1.0
uniform vec3 u_cam_pos;
uniform vec3 u_cam_dir;
uniform float frame_time;
uniform uint frame_count;

#endif