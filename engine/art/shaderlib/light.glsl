#ifndef LIGHT_GLSL
#define LIGHT_GLSL

#ifdef FS_PASS
#include "brdf.glsl"
#include "parallax.glsl"
#endif

struct light_t {
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
    vec4 pos;
    vec4 dir;
    float power;
    float radius;
    float innerCone;
    float outerCone;
    float constant;
    float linear;
    float quadratic;
    float shadow_bias;
    float normal_bias;
    float min_variance;
    float variance_transition;
    float shadow_softness;
    float penumbra_size;
    int type;
    bool processed_shadows;
    bool hard_shadows;
};

layout(std140) uniform LightBuffer {
    light_t lights[MAX_LIGHTS];
} lightBuffer;

const int LIGHT_DIRECTIONAL = 0;
const int LIGHT_POINT = 1;
const int LIGHT_SPOT = 2;

uniform int u_num_lights;

#ifdef FS_PASS
#include "shadowmap.glsl"
#endif

struct material_t {
    vec3 albedo;
    vec3 normal;
    vec3 F0;
    float roughness;
    float metallic;
    float alpha;
    vec3 tangent_view_dir;
    vec2 texcoords;
};

vec3 shading_light(light_t l, material_t m) {
    vec3 lightDir;
    float attenuation = 1.0;

    if (l.type == LIGHT_DIRECTIONAL) {
        lightDir = normalize(-l.dir.xyz);
    } else if (l.type == LIGHT_POINT || l.type == LIGHT_SPOT) {
        vec3 toLight = l.pos.xyz - v_position_ws;
        lightDir = normalize(toLight);
        float distance = length(toLight);
        
        /* fast-reject based on radius */
        if (l.radius != 0.0 && distance > l.radius) {
            return vec3(0,0,0);
        }
        attenuation = 1.0 / (l.constant + l.linear * distance + l.quadratic * (distance * distance));

        if (l.type == LIGHT_SPOT) {
            float angle = dot(l.dir.xyz, -lightDir);
            if (angle > l.innerCone) {
                float intensity = (angle-l.innerCone)/(l.outerCone-l.innerCone);
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

#ifdef SHADING_PBR
#ifdef FS_PASS
    vec3 radiance = l.diffuse.rgb * BOOST_LIGHTING;
    vec3 V = normalize( v_to_camera );
    vec3 N = m.normal;
    vec3 L = normalize( lightDir );
    vec3 H = normalize( V + L );

    vec3 F = fresnel_schlick( H, V, m.F0 );
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - m.metallic;

    // Premultiplied alpha applied to the diffuse component only
    kD *= m.alpha;

    float D = distribution_ggx( N, H, m.roughness );
    float G = geometry_smith( N, V, L, m.roughness );

    vec3 num = D * F * G;
    float denom = 4. * max( 0., dot( N, V ) ) * max( 0., dot( N, L ) ) + 0.0001;

    vec3 specular = kS * (num / max( 0.001, denom ));

    float NdotL = max( 0., dot( N, L ) );

    float self_shadow = 0.0;
    if (map_parallax.has_tex) {
        vec3 ldir = normalize(v_tbn * lightDir);
        self_shadow = parallax_shadowing(N, ldir, m.texcoords);
    }

    return ( pow((1.0 - self_shadow), parallax_shadow_power) * kD * ( m.albedo / PI ) + specular ) * radiance * NdotL * attenuation;
#else
    vec3 n = normalize(v_normal_ws);

    float diffuse = max(dot(n, lightDir), 0.0);

    vec3 halfVec = normalize(lightDir + u_cam_dir);
    float specular = pow(max(dot(n, halfVec), 0.0), l.power);

    return (attenuation*l.ambient.rgb + (diffuse*attenuation*l.diffuse.rgb + specular*attenuation*l.specular.rgb));
#endif
#else // TODO: ugly, figure out how to do this without repetition
    vec3 n = normalize(v_normal_ws);

    float diffuse = max(dot(n, lightDir), 0.0);

    vec3 halfVec = normalize(lightDir + u_cam_dir);
    float specular = pow(max(dot(n, halfVec), 0.0), l.power);

    return (attenuation*l.ambient.rgb + (diffuse*attenuation*l.diffuse.rgb + specular*attenuation*l.specular.rgb));
#endif
}

vec3 lighting(material_t m) {
    vec3 lit = vec3(0,0,0);
#ifndef SHADING_NONE
    for (int i=0; i<u_num_lights; i++) {
        vec3 lit_contrib = shading_light(lightBuffer.lights[i], m);

        vec3 result = lit_contrib;
#ifdef FS_PASS
        result *= shadowing(i).xyz;
#endif
        lit += result;
    }
#endif
    return lit;
}

#endif