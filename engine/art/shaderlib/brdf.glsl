#ifndef BRDF_GLSL
#define BRDF_GLSL

struct ColorMap
{
    bool has_tex;
    vec4 color;
};

uniform ColorMap map_diffuse;   uniform sampler2D map_diffuse_tex;
uniform ColorMap map_albedo;    uniform sampler2D map_albedo_tex;
uniform ColorMap map_specular;  uniform sampler2D map_specular_tex; // not used
uniform ColorMap map_normals;   uniform sampler2D map_normals_tex;
uniform ColorMap map_roughness; uniform sampler2D map_roughness_tex;
uniform ColorMap map_metallic;  uniform sampler2D map_metallic_tex;
uniform ColorMap map_ao;        uniform sampler2D map_ao_tex;
uniform ColorMap map_ambient;   uniform sampler2D map_ambient_tex;
uniform ColorMap map_emissive;  uniform sampler2D map_emissive_tex;
uniform ColorMap map_parallax;  uniform sampler2D map_parallax_tex;

#define sample_colormap(ColorMap_, uv_) \
    (ColorMap_.has_tex ? texture( ColorMap_##_tex, uv_ ) : ColorMap_.color)


#ifdef SHADING_PBR

#include "utils.glsl"

#define USE_BRUTEFORCE_IRRADIANCE false         // Samples irradiance from tex_skysphere when enabled.
#define USE_WRAPAROUND_SPECULAR true            // Makes silhouettes more reflective to avoid black pixels.
#define USE_SPECULAR_AO_ATTENUATION true        // Dampens IBL specular ambient with AO if enabled.
#define USE_NORMAL_VARIATION_TO_ROUGHNESS true  // Increases roughness if normal map has variation and was minified.
#define BOOST_SPECULAR  1.00f
#define BOOST_LIGHTING  8.00f

uniform float skysphere_rotation; /// set:0
uniform float skysphere_mip_count;
uniform float exposure; /// set:1.0
uniform float specular_shininess;

uniform samplerCube tex_skycube;
uniform sampler2D tex_skysphere;
uniform sampler2D tex_skyenv;
uniform sampler2D tex_brdf_lut;

uniform bool has_tex_skycube;
uniform bool has_tex_skysphere;
uniform bool has_tex_skyenv;

vec3 fresnel_schlick( vec3 H, vec3 V, vec3 F0 )
{
    float cosTheta = clamp( dot( H, V ), 0., 1. );
    return F0 + ( vec3( 1.0 ) - F0 ) * pow( 1. - cosTheta, 5.0 );
}

// A Fresnel term that dampens rough specular reflections.
// https://seblagarde.wordpress.com/2011/08/17/hello-world/
vec3 fresnel_schlick_roughness( vec3 H, vec3 V, vec3 F0, float roughness )
{
    float cosTheta = clamp( dot( H, V ), 0., 1. );
    return F0 + ( max( vec3( 1.0 - roughness ), F0 ) - F0 ) * pow( 1. - cosTheta, 5.0 );
}

float distribution_ggx( vec3 N, vec3 H, float roughness )
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max( 0., dot( N, H ) );
    float factor = NdotH * NdotH * ( a2 - 1. ) + 1.;

    return a2 / ( PI * factor * factor );
}

float geometry_schlick_ggx( vec3 N, vec3 V, float k )
{
    float NdotV = max( 0., dot( N, V ) );
    return NdotV / (NdotV * ( 1. - k ) + k );
}

float geometry_smith( vec3 N, vec3 V, vec3 L, float roughness )
{
#if 1 // original
    float r = roughness + 1.;
    float k = (r * r) / 8.;
#elif 0 // vries
    float a = roughness;
    float k = (a * a) / 2.0;
#elif 0 // vries improved?
    float a = roughness * roughness;
    float k = a / 2.0;
#endif
    return geometry_schlick_ggx( N, V, k ) * geometry_schlick_ggx( N, L, k );
}

vec2 sphere_to_polar( vec3 normal ) {
    normal = normalize( normal );
    return vec2( 1-atan( normal.z, normal.x ) / PI + 0.5 , acos( normal.y ) / PI  );
}

// Our vertically GL_CLAMPed textures seem to blend towards black when sampling the half-pixel edge.
// Not sure if it has a border, or this if is a driver bug, but can repro on multiple nvidia cards.
// Knowing the texture height we can limit sampling to the centers of the top and bottom pixel rows.
vec2 sphere_to_polar_clamp_y( vec3 normal, float texture_height )
{
    normal = normalize( normal );
    return vec2( ( atan( normal.z, normal.x ) + skysphere_rotation ) / PI / 2.0 + 0.5, clamp(acos( normal.y ) / PI, 0.5 / texture_height, 1.0 - 0.5 / texture_height) );
}

vec3 sample_sky( vec3 normal )
{
    vec2 polar = sphere_to_polar( normal );
    return texture( tex_skysphere, polar ).rgb * exposure;
}

// Takes samples around the hemisphere, converts them to radiances via weighting and
// returns a normalized sum.
vec3 sample_irradiance_slow( vec3 normal, vec3 vertex_tangent )
{
    float delta = 0.10;

    vec3 up = abs( normal.y ) < 0.999 ? vec3( 0., 1., 0. ) : vec3( 0., 0., 1. );
    vec3 tangent_x = normalize( cross( up, normal ) );
    vec3 tangent_y = cross( normal, tangent_x );

    int numIrradianceSamples = 0;

    vec3 irradiance = vec3(0.);

    for ( float phi = 0.; phi < 2. * PI ; phi += delta )
    {
        for ( float theta = 0.; theta < 0.5 * PI; theta += delta )
        {
            vec3 tangent_space = vec3(
                sin( theta ) * cos( phi ),
                sin( theta ) * sin( phi ),
                cos( theta ) );

            vec3 world_space = tangent_space.x * tangent_x + tangent_space.y + tangent_y + tangent_space.z * normal;

            vec3 color = sample_sky( world_space );
            irradiance += color * cos( theta ) * sin( theta );
            numIrradianceSamples++;
        }
    }

    irradiance = PI * irradiance / float( numIrradianceSamples );
    return irradiance;
}

vec3 sample_irradiance_fast( vec3 normal, vec3 vertex_tangent )
{
    // Sample the irradiance map if it exists, otherwise fall back to blurred reflection map.
    if ( has_tex_skyenv )
    {
        vec2 polar = sphere_to_polar( normal );
        return textureLod( tex_skyenv, polar, 0.0 ).rgb * exposure;
    }
    else if (has_tex_skysphere)
    {
        vec2 polar = sphere_to_polar( normal );
        return textureLod( tex_skysphere, polar, 0.80 * skysphere_mip_count ).rgb * exposure;
    }
    else
    {
        return textureLod(tex_skycube, normal, 0.0).rgb * exposure;
    }
}

vec3 specular_ibl( vec3 V, vec3 N, float roughness, vec3 fresnel, float metallic )
{
    // What we'd like to do here is take a LOT of skybox samples around the reflection
    // vector R according to the BRDF lobe.
    //
    // Unfortunately it's not possible in real time so we use the following UE4 style approximations:
    // 1. Integrate incoming light and BRDF separately ("split sum approximation")
    // 2. Assume V = R = N so that we can just blur the skybox and sample that.
    // 3. Bake the BRDF integral into a lookup texture so that it can be computed in constant time.
    //
    // Here we also simplify approximation #2 by using bilinear mipmaps with a magic formula instead
    // of properly convolving it with a GGX lobe.
    //
    // For details, see Brian Karis, "Real Shading in Unreal Engine 4", 2013.

    // vec3 R = 2. * dot( V, N ) * N - V;
    vec3 R = reflect(-V, N);

    R.x = -R.x;

    vec2 polar = sphere_to_polar( R );

    // Map roughness from range [0, 1] into a mip LOD [0, skysphere_mip_count].
    // The magic numbers were chosen empirically.

    float mip = 0.9 * skysphere_mip_count * pow(roughness, 0.25 * BOOST_SPECULAR);
    // float mip = skysphere_mip_count * roughness;

    vec3 prefiltered = vec3(0.0);
    if (has_tex_skysphere)
    {
        prefiltered = textureLod( tex_skysphere, polar, mip ).rgb * exposure;
    }
    else
    {
        prefiltered = textureLod( tex_skycube, R, mip ).rgb * exposure;
    }

    // prefiltered = pow(prefiltered, vec3(1.2));

    float NdotV = dot( N, V );

    // dot( N, V ) seems to produce negative values so we can try to stretch it a bit behind the silhouette
    // to avoid black pixels.
    if (USE_WRAPAROUND_SPECULAR)
    {
        NdotV = NdotV * 0.9 + 0.1;
    }

    NdotV = min(0.99, max(0.01, NdotV));

    // A precomputed lookup table contains a scale and a bias term for specular intensity (called "fresnel" here).
    // See equation (8) in Karis' course notes mentioned above.
    vec2 envBRDF = texture( tex_brdf_lut, vec2(NdotV, roughness) ).xy; // (NdotV,1-roughtness) for green top-left (NdotV,roughness) for green bottom-left
    envBRDF = pow(envBRDF, vec2(2.2));
    vec3 specular = prefiltered * (fresnel * envBRDF.x + vec3(envBRDF.y));

    return specular;
}
#endif
#endif