#ifndef SURFACE_GLSL
#define SURFACE_GLSL

#include "sh_lighting.glsl"
#include "rimlight.glsl"
#include "light.glsl"
#include "parallax.glsl"

struct surface_t {
    vec3 normal;
    vec4 albedo;
    vec4 fragcolor;
    vec3 light_direct;
    vec3 light_indirect;
    vec3 emissive;
    float roughness;
    float metallic;
    float ao;
    float alpha;
};

vec4 get_diffuse_map() {
    vec4 result;
    result = sample_colormap(map_albedo, v_texcoord);
    result.a *= u_global_alpha*u_global_opacity;
    return result;
}

surface_t surface() {
    surface_t s;

    s.normal = normalize(v_normal_ws);
    s.light_direct = vec3(0.0, 0.0, 0.0);
    s.light_indirect = vec3(1.0, 1.0, 1.0);
    s.albedo = vec4(0.5, 0.5, 0.5, 1.0);
    s.emissive = vec3(0.0, 0.0, 0.0);
    s.roughness = 1.0;
    s.metallic = 0.0;
    s.ao = 1.0;
    s.alpha = 1.0;

    vec2 uvs = v_texcoord;
    
    // SH lighting
    vec3 result = sh_lighting(s.normal);
    if((result.x*result.x+result.y*result.y+result.z*result.z) > 0.0 ) s.light_indirect = result;

#ifdef SHADING_PHONG
    material_t dummy_mat;
    s.light_direct = lighting(dummy_mat);
#endif

#ifdef SHADING_VERTEXLIT
    s.light_direct = v_vertcolor;
#endif

#ifdef SHADING_PBR
    vec3 tangent_view_dir = normalize(v_tangent_view - v_tangent_world);
    if (map_parallax.has_tex) {
        uvs = parallax(uvs, tangent_view_dir);
        // if(uvs.x > 1.0 || uvs.y > 1.0 || uvs.x < 0.0 || uvs.y < 0.0)
        //     discard;
    }

    vec4 baseColor_alpha;
    baseColor_alpha = sample_colormap( map_albedo, uvs );
    s.albedo = baseColor_alpha;

    if (!map_albedo.has_tex) {
        s.albedo.rgb = pow(s.albedo.rgb, vec3(2.2));
    }

    if( map_metallic.has_tex && map_roughness.has_tex ) {
        s.metallic = sample_colormap( map_metallic, uvs ).x;
        s.roughness = sample_colormap( map_roughness, uvs ).x;
    }
    else if( map_roughness.has_tex ) {
        s.metallic = sample_colormap( map_roughness, uvs ).b;
        s.roughness = sample_colormap( map_roughness, uvs ).g;
    } else {
        // rely on color specifically
        s.metallic = sample_colormap( map_metallic, uvs ).x;
        s.roughness = sample_colormap( map_roughness, uvs ).x;
    }

    // if ( map_ao.has_tex )
    s.ao = sample_colormap( map_ao, uvs ).x;
    // if ( map_ambient.has_tex )
    //     s.ao = sample_colormap( map_ambient, uvs ).x;

    s.emissive = sample_colormap( map_emissive, uvs ).rgb;
    if (!map_emissive.has_tex) {
        s.emissive.rgb = pow(s.emissive.rgb, vec3(2.2));
    }
    s.emissive *= u_emissive_value;

    vec3 normalmap = texture( map_normals_tex, uvs ).xyz * vec3(2.0) - vec3(1.0);
    float normalmap_mip = textureQueryLod( map_normals_tex, uvs ).x;
    float normalmap_length = length(normalmap);
    normalmap /= normalmap_length;

    s.normal = v_normal_ws;

    if ( map_normals.has_tex )
    {
        // Mikkelsen's tangent space normal map decoding. See http://mikktspace.com/ for rationale.
        vec3 bi = cross( v_normal_ws, v_tangent );
        vec3 nmap = normalmap.xyz;
        s.normal = nmap.x * v_tangent + nmap.y * bi + nmap.z * v_normal_ws;
    }

    s.normal = normalize( s.normal );

    if (USE_NORMAL_VARIATION_TO_ROUGHNESS)
    {
        // Try to reduce specular aliasing by increasing roughness when minified normal maps have high variation.
        float variation = 1. - pow( normalmap_length, 8. );
        float minification = clamp( normalmap_mip - 2., 0., 1. );
        s.roughness = mix( s.roughness, 1.0, variation * minification );
    }

    vec3 N = s.normal;
    vec3 V = normalize( v_to_camera );

    vec3 Lo = vec3(0.);
    vec3 F0 = vec3(0.04);
    F0 = mix( F0, s.albedo.rgb, s.metallic );

    bool use_ibl = has_tex_skysphere || has_tex_skycube;

    material_t pbr_mat;
    pbr_mat.albedo = s.albedo.rgb;
    pbr_mat.normal = N;
    pbr_mat.F0 = F0;
    pbr_mat.roughness = s.roughness;
    pbr_mat.metallic = s.metallic;
    pbr_mat.alpha = s.alpha;
    pbr_mat.tangent_view_dir = tangent_view_dir;
    pbr_mat.texcoords = uvs;
    Lo += lighting(pbr_mat);
    s.light_direct = Lo;

    if ( map_ambient.has_tex )
        s.light_indirect = sample_colormap( map_ambient, uvs ).xyz;

    vec3 diffuse_ambient;
    vec3 specular_ambient;

    if ( use_ibl )
    {
        // Image based lighting.
        // Based on https://learnopengl.com/PBR/IBL/Diffuse-irradiance

        vec3 irradiance = sample_irradiance_fast( N, v_tangent );

        // Compute the Fresnel term for a perfect mirror reflection with L = R.
        // In this case the halfway vector H = N.
        //
        // We use a modified Fresnel function that dampens specular reflections of very
        // rough surfaces to avoid too bright pixels at grazing angles.
        vec3 F = fresnel_schlick_roughness( N, V, F0, s.roughness );
        vec3 kS = F;

        // Subtract the amount of reflected light (specular) to get the energy left for
        // absorbed (diffuse) light.
        vec3 kD = vec3(1.) - kS;

        // Metallic surfaces have only a specular reflection.
        kD *= 1.0 - s.metallic;

        // Premultiplied alpha applied to the diffuse component only
        kD *= s.alpha;

        // Modulate the incoming lighting with the diffuse color: some wavelengths get absorbed.
        diffuse_ambient = irradiance * s.albedo.rgb;

        // Ambient light also has a specular part.
        specular_ambient = specular_ibl( V, N, s.roughness, F, s.metallic );

        // s.light_indirect *= 0.5;
        // s.light_indirect *= 1.0 - s.metallic;

        // Ambient occlusion tells us the fraction of sky light that reaches this point.
        if (USE_SPECULAR_AO_ATTENUATION)
        {
            s.light_indirect = s.ao * (kD * diffuse_ambient + specular_ambient);
        }
        else
        {
            // We don't attenuate specular_ambient ambient here with AO which might cause flickering in dark cavities.
            s.light_indirect = s.ao * (kD * diffuse_ambient) + specular_ambient;
        }
    }
#else
    if(u_matcaps) {
        vec2 muv = vec2(view * vec4(v_normal_ws, 0))*0.5+vec2(0.5,0.5); // normal (model space) to view space
        s.albedo = sample_colormap(map_albedo, vec2(muv.x, 1.0-muv.y));
    } else {
        s.albedo = sample_colormap(map_albedo, uvs);
    }
#endif

    s.albedo *= v_color;

    s.fragcolor = s.albedo;
    s.fragcolor.rgb *= s.light_direct + s.light_indirect;
    s.fragcolor.rgb += s.emissive;
    s.fragcolor.rgb += get_rimlight();
    s.fragcolor.a *= u_global_alpha;
    s.fragcolor *= vec4(u_global_opacity);

#ifdef SHADING_PBR
    {
        // aces film (CC0, src: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/)
        // vec3 x = s.fragcolor.rgb;
        // float a = 2.51f;
        // float b = 0.03f;
        // float c = 2.43f;
        // float d = 0.59f;
        // float e = 0.14f;
        // vec3 adjusted_color = clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
        // gamma correction

        vec3 color = s.fragcolor.rgb;
        // color = color / ( vec3(1.) + color );
        // color = pow( color, vec3(1. / 2.2) );
        color = pow( color, vec3(1. / 2.2) );
        // float dither = random( uvec3( floatBitsToUint( gl_FragCoord.xy ), frame_count ) );
        // color += vec3( (-1.0/256.) + (2./256.) * dither );
        s.fragcolor.rgb = color;
        // s.fragcolor.rgb = adjusted_color;
        // s.fragcolor.rgb = pow(s.fragcolor.rgb, vec3(1.0/2.2) );
    }
#endif
    
    return s;
}

#endif