#version 400
uniform mat4 model, view;
uniform sampler2D u_texture2d;
uniform vec3 u_coefficients_sh[9];
uniform bool u_textured = true;
uniform bool u_lit = false;
uniform bool u_matcaps = false;
uniform vec4 u_diffuse = vec4(1.0,1.0,1.0,1.0);

// lightmapping
uniform sampler2D u_lightmap;
uniform bool u_texlit;
uniform bool u_texmod = true;
uniform float u_litboost = 1.0;

in vec3 v_position;
in vec3 v_position_ws;
#ifdef RIM
uniform mat4 M; // RIM
uniform vec3 u_rimcolor = vec3(0.05,0.05,0.05);
uniform vec3 u_rimrange = vec3(0.11,0.98,0.5);
uniform vec3 u_rimpivot = vec3(0,0,0);
uniform bool u_rimambient = true;
#endif
in vec3 v_normal, v_normal_ws;
in vec2 v_texcoord, v_texcoord2;
in vec4 v_color;
in vec3 v_tangent;
in vec3 v_binormal;
in vec3 v_to_camera;
out vec4 fragcolor;


{{include-shadowmap}}
in vec4 vpeye;
in vec4 vneye;
in vec4 sc;
vec4 shadowing() {
    return shadowmap(vpeye, vneye, v_texcoord, sc);
}

uniform vec3 u_cam_pos;
uniform vec3 u_cam_dir;

uniform int u_num_lights;

struct light_t {
    int type;
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 pos;
    vec3 dir;
    float power;
    float innerCone;
    float outerCone;

    // falloff
    float constant;
    float linear;
    float quadratic;
};

#define MAX_LIGHTS 16
const int LIGHT_DIRECTIONAL = 0;
const int LIGHT_POINT = 1;
const int LIGHT_SPOT = 2;

uniform light_t u_lights[MAX_LIGHTS];

#ifdef SHADING_PHONG
vec3 shading_phong(light_t l) {
    vec3 lightDir;
    float attenuation = 1.0;

    if (l.type == LIGHT_DIRECTIONAL) {
        lightDir = normalize(-l.dir);
    } else if (l.type == LIGHT_POINT || l.type == LIGHT_SPOT) {
        vec3 toLight = l.pos - v_position_ws;
        lightDir = normalize(toLight);
        float distance = length(toLight);
        attenuation = 1.0 / (l.constant + l.linear * distance + l.quadratic * (distance * distance));

        if (l.type == LIGHT_SPOT) {
            float angle = dot(l.dir, -lightDir);
            if (angle > l.outerCone) {
                float intensity = (angle-l.outerCone)/(l.innerCone-l.outerCone);
                attenuation *= clamp(intensity, 0.0, 1.0);
            } else {
                attenuation = 0.0;
            }
        }
    }

    // fast-rejection for faraway vertices
    if (attenuation <= 0.01) {
        return vec3(0,0,0);
    }

    vec3 n = normalize(v_normal_ws);

    float diffuse = max(dot(n, lightDir), 0.0);

    vec3 halfVec = normalize(lightDir + u_cam_dir);
    float specular = pow(max(dot(n, halfVec), 0.0), l.power);

    return (attenuation*l.ambient + diffuse*attenuation*l.diffuse + specular*attenuation*l.specular);
}
#endif

#ifdef SHADING_PBR
uniform vec2 resolution = vec2(640.0,480.0);    // debug options below use this (USE_MAP_DEBUGGING, USE_AMBIENT_DEBUGGING)

#define USE_BRUTEFORCE_IRRADIANCE false         // Samples irradiance from tex_skysphere when enabled.
#define USE_WRAPAROUND_SPECULAR true            // Makes silhouettes more reflective to avoid black pixels.
#define USE_SPECULAR_AO_ATTENUATION true        // Dampens IBL specular ambient with AO if enabled.
#define USE_NORMAL_VARIATION_TO_ROUGHNESS true  // Increases roughness if normal map has variation and was minified.
#define USE_MAP_DEBUGGING false                 // Shows all ColorMaps as horizontal bars
#define USE_AMBIENT_DEBUGGING false             // Splits the screen in two and shows image-based specular (left), full shading (middle), diffuse shading (right).
#define BOOST_LIGHTING  2.00f                   // Multiplies analytic light's color with this constant because otherwise they look really pathetic.
#define BOOST_SPECULAR  1.50f
#define BOOST_NOISE     2.50f

struct ColorMap
{
    bool has_tex;
    vec4 color;
};

uniform ColorMap map_albedo;    uniform sampler2D map_albedo_tex;
uniform ColorMap map_diffuse;   uniform sampler2D map_diffuse_tex;
uniform ColorMap map_specular;  uniform sampler2D map_specular_tex; // not used
uniform ColorMap map_normals;   uniform sampler2D map_normals_tex;
uniform ColorMap map_roughness; uniform sampler2D map_roughness_tex;
uniform ColorMap map_metallic;  uniform sampler2D map_metallic_tex;
uniform ColorMap map_ao;        uniform sampler2D map_ao_tex;
uniform ColorMap map_ambient;   uniform sampler2D map_ambient_tex;
uniform ColorMap map_emissive;  uniform sampler2D map_emissive_tex;

#define sample_colormap(ColorMap_, uv_) \
    (ColorMap_.has_tex ? texture( ColorMap_##_tex, uv_ ) : ColorMap_.color)

uniform float skysphere_rotation=0;
uniform float skysphere_mip_count;
uniform float exposure=1;
uniform uint frame_count;
uniform float specular_shininess;

uniform sampler2D tex_skysphere;
uniform sampler2D tex_skyenv;
uniform sampler2D tex_brdf_lut;

uniform bool has_tex_skysphere;
uniform bool has_tex_skyenv;

const float PI = 3.1415926536;

// MurMurHash 3 finalizer. Implementation is in public domain.
uint hash( uint h )
{
    h ^= h >> 16;
    h *= 0x85ebca6bU;
    h ^= h >> 13;
    h *= 0xc2b2ae35U;
    h ^= h >> 16;
    return h;
}

// Random function using the idea of StackOverflow user "Spatial" https://stackoverflow.com/a/17479300
// Creates random 23 bits and puts them into the fraction bits of an 32-bit float.
float random( uvec3 h )
{
    uint m = hash(h.x ^ hash( h.y ) ^ hash( h.z ));
    return uintBitsToFloat( ( m & 0x007FFFFFu ) | 0x3f800000u ) - 1.;
}

float random( vec3 v )
{
    return random(floatBitsToUint( v ));
}

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
    else
    {
        vec2 polar = sphere_to_polar( normal );
        return textureLod( tex_skysphere, polar, 0.80 * skysphere_mip_count ).rgb * exposure;
    }
}


vec3 specular_ibl( vec3 V, vec3 N, float roughness, vec3 fresnel )
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

    vec3 R = 2. * dot( V, N ) * N - V;

    vec2 polar = sphere_to_polar( R );

    // Map roughness from range [0, 1] into a mip LOD [0, skysphere_mip_count].
    // The magic numbers were chosen empirically.

    float mip = 0.9 * skysphere_mip_count * pow(roughness, 0.25 * BOOST_SPECULAR);

    vec3 prefiltered = textureLod( tex_skysphere, polar, mip ).rgb * exposure;

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
    vec2 envBRDF = texture( tex_brdf_lut, vec2(NdotV, 1.0-roughness) ).xy; // (NdotV,1-roughtness) for green top-left (NdotV,roughness) for green bottom-left
    vec3 specular = prefiltered * (fresnel * envBRDF.x + vec3(envBRDF.y));

    return specular;
}
#endif

vec3 lighting() {
    vec3 lit = vec3(0,0,0);
#ifndef SHADING_NONE
    for (int i=0; i<u_num_lights; i++) {
    #ifdef SHADING_PHONG
        lit += shading_phong(u_lights[i]);
    #endif
    #ifdef SHADING_PBR
    #endif
    }
#endif
    return lit;
}

vec3 sh_lighting(vec3 n) {
    vec3 SHLightResult[9];
    SHLightResult[0] =  0.282095f * u_coefficients_sh[0];
    SHLightResult[1] = -0.488603f * u_coefficients_sh[1] * n.y;
    SHLightResult[2] =  0.488603f * u_coefficients_sh[2] * n.z;
    SHLightResult[3] = -0.488603f * u_coefficients_sh[3] * n.x;
    SHLightResult[4] =  1.092548f * u_coefficients_sh[4] * n.x * n.y;
    SHLightResult[5] = -1.092548f * u_coefficients_sh[5] * n.y * n.z;
    SHLightResult[6] =  0.315392f * u_coefficients_sh[6] * (3.0f * n.z * n.z - 1.0f);
    SHLightResult[7] = -1.092548f * u_coefficients_sh[7] * n.x * n.z;
    SHLightResult[8] =  0.546274f * u_coefficients_sh[8] * (n.x * n.x - n.y * n.y);
    vec3 result = vec3(0.0);
    for (int i = 0; i < 9; ++i)
        result += SHLightResult[i];
    return result;
}

#ifdef LIGHTMAP_BAKING
void main() {
    vec3 n = normalize(v_normal_ws);
    vec4 diffuse;

    if(u_textured) {
        diffuse = texture(u_texture2d, v_texcoord);
    } else {
        diffuse = u_diffuse; // * v_color;
    }

    if (u_texlit) {
        vec4 litsample = texture(u_lightmap, v_texcoord);
        diffuse *= litsample;
    }

    fragcolor = vec4(diffuse.rgb*u_litboost, 1.0);
}
#endif

#ifdef SHADING_PHONG
void main() {
    vec3 n = normalize(v_normal_ws);

    vec4 lit = vec4(1.0, 1.0, 1.0, 1.0);
    // SH lighting
    if (!u_texlit) {
        vec3 result = sh_lighting(n);
        if( (result.x*result.x+result.y*result.y+result.z*result.z) > 0.0 ) lit = vec4(result, 1.0);
    }

    // analytical lights
    lit += vec4(lighting(), 0.0);

    // base
    vec4 diffuse;

    if(u_matcaps) {
        vec2 muv = vec2(view * vec4(v_normal_ws, 0))*0.5+vec2(0.5,0.5); // normal (model space) to view space
        diffuse = texture(u_texture2d, vec2(muv.x, 1.0-muv.y));
    } else if(u_textured) {
        diffuse = texture(u_texture2d, v_texcoord);
    } else {
        diffuse = u_diffuse; // * v_color;
    }

    if (u_texlit) {
        vec4 litsample = texture(u_lightmap, v_texcoord);

        if (u_texmod) {
            diffuse *= litsample;
        } else {
            diffuse += litsample;
        }

        diffuse.rgb += sh_lighting(n);
    }
    
    // lighting mix
    fragcolor = diffuse * lit * shadowing();
    
    // rimlight
    #ifdef RIM
    {
        vec3 n = normalize(mat3(M) * v_normal);  // convert normal to view space
        vec3 p = (M * vec4(v_position,1.0)).xyz; // convert position to view space
        vec3 v = vec3(0,-1,0);
        if (!u_rimambient) {
            v = normalize(u_rimpivot-p);
        }
        float rim = 1.0 - max(dot(v,n), 0.0);
        vec3 col = u_rimcolor*(pow(smoothstep(1.0-u_rimrange.x,u_rimrange.y,rim), u_rimrange.z));
        fragcolor += vec4(col, 0.0);
    }
    #endif
}
#endif
#ifdef SHADING_PBR
void main(void)
{
    vec3 baseColor = vec3( 0.5, 0.5, 0.5 );
    float roughness = 1.0;
    float metallic = 0.0;
    float ao = 1.0;
    float alpha = 1.0;

    vec4 baseColor_alpha;
    if ( map_albedo.has_tex )
        baseColor_alpha = sample_colormap( map_albedo, v_texcoord );
    else
        baseColor_alpha = sample_colormap( map_diffuse, v_texcoord );
    baseColor = baseColor_alpha.xyz;
    alpha = baseColor_alpha.w;

    if( map_metallic.has_tex && map_roughness.has_tex ) {
        metallic = sample_colormap( map_metallic, v_texcoord ).x;
        roughness = sample_colormap( map_roughness, v_texcoord ).x;
    }
    else if( map_roughness.has_tex ) {
        //< @r-lyeh, metalness B, roughness G, (@todo: self-shadowing occlusion R; for now, any of R/B are metallic)
        metallic = sample_colormap( map_roughness, v_texcoord ).b;// + sample_colormap( map_roughness, v_texcoord ).r;
        roughness = sample_colormap( map_roughness, v_texcoord ).g;
    }

    if ( map_ao.has_tex )
        ao = sample_colormap( map_ao, v_texcoord ).x;
    else if ( map_ambient.has_tex )
        ao = sample_colormap( map_ambient, v_texcoord ).x;

    vec3 emissive = sample_colormap( map_emissive, v_texcoord ).rgb;

    vec3 normalmap = texture( map_normals_tex, v_texcoord ).xyz * vec3(2.0) - vec3(1.0);
    float normalmap_mip = textureQueryLod( map_normals_tex, v_texcoord ).x;
    float normalmap_length = length(normalmap);
    normalmap /= normalmap_length;

    vec3 normal = v_normal_ws;

    if ( map_normals.has_tex )
    {
        // Mikkelsen's tangent space normal map decoding. See http://mikktspace.com/ for rationale.
        vec3 bi = cross( v_normal_ws, v_tangent );
        vec3 nmap = normalmap.xyz;
        normal = nmap.x * v_tangent + nmap.y * bi + nmap.z * v_normal_ws;
    }

    normal = normalize( normal );

    if( USE_MAP_DEBUGGING && !USE_AMBIENT_DEBUGGING )
    {
        vec3 c = vec3(1., 0., 0.);
        float x = gl_FragCoord.x / resolution.x;
        float y = gl_FragCoord.y / resolution.y;
        if ( y < (7.0/7.0) ) c = vec3(.5) + .5*v_normal_ws;
        if ( y < (6.0/7.0) ) c = vec3(.5) + .5*normalmap;
        if ( y < (5.0/7.0) ) c = vec3(ao);
        if ( y < (4.0/7.0) ) c = vec3(emissive);
        if ( y < (3.0/7.0) ) c = vec3(metallic);
        if ( y < (2.0/7.0) ) c = vec3(roughness);
        if ( y < (1.0/7.0) ) c = baseColor;
        fragcolor = vec4(c, 1.);
        return;
    }

    if (USE_NORMAL_VARIATION_TO_ROUGHNESS)
    {
        // Try to reduce specular aliasing by increasing roughness when minified normal maps have high variation.
        float variation = 1. - pow( normalmap_length, 8. );
        float minification = clamp( normalmap_mip - 2., 0., 1. );
        roughness = mix( roughness, 1.0, variation * minification );
    }

    fragcolor = baseColor_alpha;


    vec3 N = normal;
    vec3 V = normalize( v_to_camera );

    vec3 Lo = vec3(0.);
    vec3 F0 = vec3(0.04);
    F0 = mix( F0, baseColor, metallic );

    bool use_ibl = has_tex_skysphere;
    // use_ibl = false;

    // Add contributions from analytic lights.

    {
        for ( int i = 0; i < u_num_lights; i++ )
        {
            light_t l = u_lights[i];
            vec3 lightDir;
            float attenuation = 1.0;

            if (l.type == LIGHT_DIRECTIONAL) {
                lightDir = normalize(-l.dir);
            } else if (l.type == LIGHT_POINT || l.type == LIGHT_SPOT) {
                vec3 toLight = l.pos - v_position_ws;
                lightDir = normalize(toLight);
                float distance = length(toLight);
                attenuation = 1.0 / (l.constant + l.linear * distance + l.quadratic * (distance * distance));

                if (l.type == LIGHT_SPOT) {
                    float angle = dot(l.dir, -lightDir);
                    if (angle > l.outerCone) {
                        float intensity = (angle-l.outerCone)/(l.innerCone-l.outerCone);
                        attenuation *= clamp(intensity, 0.0, 1.0);
                    } else {
                        attenuation = 0.0;
                    }
                }
            }

            // fast-rejection for faraway vertices
            if (attenuation <= 0.01) {
                continue;
            }

            vec3 radiance = l.diffuse * BOOST_LIGHTING;

            vec3 L = normalize( lightDir );
            vec3 H = normalize( V + L );

            vec3 F = fresnel_schlick( H, V, F0 );
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;

            // Premultiplied alpha applied to the diffuse component only
            kD *= alpha;

            float D = distribution_ggx( N, H, roughness );
            float G = geometry_smith( N, V, L, roughness );

            vec3 num = D * F * G;
            float denom = 4. * max( 0., dot( N, V ) ) * max( 0., dot( N, L ) );

            vec3 specular = kS * (num / max( 0.001, denom ));

            float NdotL = max( 0., dot( N, L ) );

            Lo += ( kD * ( baseColor / PI ) + specular ) * radiance * NdotL * attenuation;
        }
    }

    vec3 ambient = sample_colormap( map_ambient, v_texcoord ).xyz;
    vec3 diffuse_ambient;
    vec3 specular_ambient;

    if ( use_ibl )
    {
        // Image based lighting.
        // Based on https://learnopengl.com/PBR/IBL/Diffuse-irradiance

        vec3 irradiance = vec3(0.);

        if ( USE_BRUTEFORCE_IRRADIANCE )
        {
            irradiance = sample_irradiance_slow( normal, v_tangent );
        }
        else
        {
            irradiance = sample_irradiance_fast( normal, v_tangent );
        }

        // Compute the Fresnel term for a perfect mirror reflection with L = R.
        // In this case the halfway vector H = N.
        //
        // We use a modified Fresnel function that dampens specular reflections of very
        // rough surfaces to avoid too bright pixels at grazing angles.
        vec3 F = fresnel_schlick_roughness( N, V, F0, roughness );
        vec3 kS = F;

        // Subtract the amount of reflected light (specular) to get the energy left for
        // absorbed (diffuse) light.
        vec3 kD = vec3(1.) - kS;

        // Metallic surfaces have only a specular reflection.
        kD *= 1.0 - metallic;

        // Premultiplied alpha applied to the diffuse component only
        kD *= alpha;

        // Modulate the incoming lighting with the diffuse color: some wavelengths get absorbed.
        diffuse_ambient = irradiance * baseColor;

        // Ambient light also has a specular part.
        specular_ambient = specular_ibl( V, normal, roughness, F );

        // Ambient occlusion tells us the fraction of sky light that reaches this point.
        if (USE_SPECULAR_AO_ATTENUATION)
        {
            ambient = ao * (kD * diffuse_ambient + specular_ambient);
        }
        else
        {
            // We don't attenuate specular_ambient ambient here with AO which might cause flickering in dark cavities.
            ambient = ao * (kD * diffuse_ambient) + specular_ambient;
        }
    }

    vec3 color = (ambient + Lo) + emissive;

    if ( USE_AMBIENT_DEBUGGING )
    {
        float y = gl_FragCoord.y / resolution.y;
        if( USE_MAP_DEBUGGING && y > 0.5 )
        {
            if ( (y-0.5) < (7.0/7.0/2.0) ) color = vec3(.5) + .5*v_normal_ws;
            if ( (y-0.5) < (6.0/7.0/2.0) ) color = vec3(.5) + .5*normalmap;
            if ( (y-0.5) < (5.0/7.0/2.0) ) color = vec3(ao);
            if ( (y-0.5) < (4.0/7.0/2.0) ) color = vec3(emissive);
            if ( (y-0.5) < (3.0/7.0/2.0) ) color = vec3(metallic);
            if ( (y-0.5) < (2.0/7.0/2.0) ) color = vec3(roughness);
            if ( (y-0.5) < (1.0/7.0/2.0) ) color = baseColor;
        } else {
            float x = gl_FragCoord.x / resolution.x;
            if ( x < 0.33 )
                    color = specular_ambient;
            else if( x > 0.66 )
                    color = diffuse_ambient;
        }
    }

    // dither with noise.
    // float dither = random( uvec3( floatBitsToUint( gl_FragCoord.xy ), frame_count ) );
    // color += BOOST_NOISE * vec3( (-1.0/256.) + (2./256.) * dither );

#if 0 // original
    // basic tonemap and gamma correction
    color = color / ( vec3(1.) + color );
    color = pow( color, vec3(1. / 2.2) );
#elif 0
    // filmic tonemapper
    vec3 linearColor = color;
    vec3 x = max(vec3(0.0), linearColor - 0.004);
    color = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
    // gamma correction
    // color = pow( color, vec3(1. / 2.2) );
#elif 0
    // aces film (CC0, src: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/)
    vec3 x = color;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    color = clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
    // gamma correction
#endif
    // color = pow( color, vec3(1. / 2.2) );

    // Technically this alpha may be too transparent, if there is a lot of reflected light we wouldn't
    // see the background, maybe we can approximate it well enough by adding a fresnel term
    fragcolor = vec4( color * shadowing().xyz, alpha );
}

#endif
