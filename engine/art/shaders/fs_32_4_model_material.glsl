#include "model_fs.glsl"
#include "surface.glsl"

out vec3 out_matprops;
out vec3 out_normals;
out vec4 out_albedo;

void main() {
    vec4 albedo = get_diffuse_map();
    if (albedo.a < u_cutout_alpha) discard;

    albedo.rgb = pow( albedo.rgb, vec3(1. / 2.2) );

    vec2 uvs = v_texcoord;
    float metallic = 0.0;
    float roughness = 1.0;
    float ao = sample_colormap( map_ao, uvs ).x;
    vec3 N = v_normal_ws;

#ifdef SHADING_PBR
    if( map_metallic.has_tex && map_roughness.has_tex ) {
        metallic = sample_colormap( map_metallic, uvs ).x;
        roughness = sample_colormap( map_roughness, uvs ).x;
    }
    else if( map_roughness.has_tex ) {
        metallic = sample_colormap( map_roughness, uvs ).b;
        roughness = sample_colormap( map_roughness, uvs ).g;
    } else {
        // rely on color specifically
        metallic = sample_colormap( map_metallic, uvs ).x;
        roughness = sample_colormap( map_roughness, uvs ).x;
    }

    vec3 normalmap = texture( map_normals_tex, uvs ).xyz * vec3(2.0) - vec3(1.0);
    float normalmap_mip = textureQueryLod( map_normals_tex, uvs ).x;
    float normalmap_length = length(normalmap);
    normalmap /= normalmap_length;


    if ( map_normals.has_tex )
    {
        // Mikkelsen's tangent space normal map decoding. See http://mikktspace.com/ for rationale.
        vec3 bi = cross( v_normal_ws, v_tangent );
        N = normalmap.x * v_tangent + normalmap.y * bi + normalmap.z * v_normal_ws;
    }

    N = normalize( N );

    if (USE_NORMAL_VARIATION_TO_ROUGHNESS)
    {
        // Try to reduce specular aliasing by increasing roughness when minified normal maps have high variation.
        float variation = 1. - pow( normalmap_length, 8. );
        float minification = clamp( normalmap_mip - 2., 0., 1. );
        roughness = mix( roughness, 1.0, variation * minification );
    }
#endif

    out_matprops = vec3(metallic, roughness, ao);
    out_normals = N;
    out_albedo = albedo*u_global_alpha*u_global_opacity;
}