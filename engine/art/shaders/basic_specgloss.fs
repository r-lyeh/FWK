#version 330 core

struct Light
{
    vec3 direction;
    vec3 color;
};

struct ColorMap
{
    bool has_tex;
    vec4 color;
};

uniform ColorMap map_albedo;    uniform sampler2D map_albedo_tex;
uniform ColorMap map_diffuse;   uniform sampler2D map_diffuse_tex;
uniform ColorMap map_specular;  uniform sampler2D map_specular_tex;
uniform ColorMap map_normals;   uniform sampler2D map_normals_tex;
uniform ColorMap map_roughness; uniform sampler2D map_roughness_tex;
uniform ColorMap map_metallic;  uniform sampler2D map_metallic_tex;
uniform ColorMap map_ao;        uniform sampler2D map_ao_tex;
uniform ColorMap map_ambient;   uniform sampler2D map_ambient_tex;
uniform ColorMap map_emissive;  uniform sampler2D map_emissive_tex;

#define sample_colormap(ColorMap_, uv_) \
        (ColorMap_.has_tex ? texture( ColorMap_##_tex, uv_ ) : ColorMap_.color)


in vec3 out_normal;
in vec3 out_tangent;
in vec3 out_binormal;
in vec2 out_texcoord;
in vec3 out_worldpos;
in vec3 out_to_camera;


uniform float specular_shininess;

uniform float skysphere_rotation;
uniform float skysphere_mip_count;
uniform float exposure;

uniform vec3 camera_position;
uniform Light lights[3];

uniform vec4 global_ambient;

uniform bool has_tex_skysphere;
uniform bool has_tex_skyenv;

uniform sampler2D tex_skysphere;
uniform sampler2D tex_skyenv;

out vec4 frag_color;

const float PI = 3.1415926536;

vec2 sphere_to_polar( vec3 normal )
{
    normal = normalize( normal );
    return vec2( ( atan( normal.z, normal.x ) + skysphere_rotation ) / PI / 2.0 + 0.5, acos( normal.y ) / PI );
}

vec3 sample_irradiance_fast( vec3 normal )
{
    // Sample the irradiance map if it exists, otherwise fall back to blurred reflection map.
    if ( has_tex_skyenv )
    {
        vec2 polar = sphere_to_polar( normal );
        // HACK: Sample a smaller mip here to avoid high frequency color variations on detailed normal
        //       mapped areas.
        float miplevel = 5.5; // tweaked for a 360x180 irradiance texture
        return textureLod( tex_skyenv, polar, miplevel ).rgb * exposure;
    }
    else
    {
        vec2 polar = sphere_to_polar( normal );
        return textureLod( tex_skysphere, polar, 0.80 * skysphere_mip_count ).rgb * exposure;
    }
}

float calculate_specular( vec3 normal, vec3 light_direction )
{
    vec3 V = normalize( out_to_camera );
    vec3 L = -normalize( light_direction );
    vec3 H = normalize( V + L );
    float rdotv = clamp( dot( normal, H ), 0.0, 1.0 );
    float total_specular = pow( rdotv, specular_shininess );

    return total_specular;
}

void main(void)
{
    vec4 specular;
    if( map_specular.has_tex ) {
        specular = sample_colormap( map_specular, out_texcoord );
    } else {
        float roughness = 1.0;
        float metallic = 0.0;

        if( map_metallic.has_tex && map_roughness.has_tex ) {
            metallic = sample_colormap( map_metallic, out_texcoord ).x;
            roughness = sample_colormap( map_roughness, out_texcoord ).x;
        }
        else if( map_roughness.has_tex ) {
            //< @r-lyeh, metalness B, roughness G, (@todo: self-shadowing occlusion R; for now, any of R/B are metallic)
            metallic = sample_colormap( map_roughness, out_texcoord ).b + sample_colormap( map_roughness, out_texcoord ).r;
            roughness = sample_colormap( map_roughness, out_texcoord ).g;
        }

        float gloss = metallic * (1.0 - roughness);
        specular = vec4(gloss);
    }

    vec4 baseColor_alpha;
    if ( map_albedo.has_tex )
        baseColor_alpha = sample_colormap( map_albedo, out_texcoord );
    else
        baseColor_alpha = sample_colormap( map_diffuse, out_texcoord );

    vec3 diffuse = baseColor_alpha.xyz;
    float alpha = baseColor_alpha.w;
    vec3 ambient = sample_colormap( map_ambient, out_texcoord ).xyz;
    vec3 normals = normalize(texture( map_normals_tex, out_texcoord ).xyz * vec3(2.0) - vec3(1.0));

    vec3 normal = out_normal;
    if ( map_normals.has_tex )
    {
        // Mikkelsen's tangent space normal map decoding. See http://mikktspace.com/ for rationale.
        vec3 bi = cross( out_normal, out_tangent );
        vec3 nmap = normals.xyz;
        normal = nmap.x * out_tangent + nmap.y * bi + nmap.z * out_normal;
    }

    normal = normalize( normal );

    vec3 irradiance = sample_irradiance_fast( normal );
    ambient *= irradiance;

    vec3 color = ambient * global_ambient.rgb;
    for ( int i = 0; i < lights.length(); i++ )
    {
        float ndotl = clamp( dot( normal, -normalize( lights[ i ].direction ) ), 0.0, 1.0 );

        vec3 specular = specular.rgb * calculate_specular( normal, lights[ i ].direction ) * specular.a;
        color += (diffuse + specular) * ndotl * lights[ i ].color;
    }

    color += sample_colormap( map_emissive, out_texcoord ).rgb;

    frag_color = vec4( pow( color * exposure, vec3(1. / 2.2) ), alpha );
}
