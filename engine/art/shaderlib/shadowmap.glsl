#ifndef SHADOWMAP_GLSL
#define SHADOWMAP_GLSL

#ifndef NO_SHADOWS

// #define CSM_DEBUG

#include "utils.glsl"

uniform bool u_shadow_receiver;
uniform float u_cascade_distances[NUM_SHADOW_CASCADES];
uniform samplerCube shadowMap[MAX_SHADOW_LIGHTS];
uniform sampler2D shadowMap2D[NUM_SHADOW_CASCADES];
uniform sampler3D shadow_offsets;
uniform int shadow_filter_size;
uniform int shadow_window_size;
uniform mat4 light_shadow_matrix_csm[NUM_SHADOW_CASCADES];
uniform float u_blend_region; /// set:15.0

// const float bias_modifier[NUM_SHADOW_CASCADES] = float[NUM_SHADOW_CASCADES](0.95, 0.35, 0.20, 0.1, 0.1, 0.1);
const float bias_modifier[NUM_SHADOW_CASCADES] = float[NUM_SHADOW_CASCADES](1.0, 2.0, 6.0, 9.0);
// const float bias_modifier[NUM_SHADOW_CASCADES] = float[NUM_SHADOW_CASCADES](0.95, 0.35, 0.20, 0.15);

const vec3 cascade_debug_colors[NUM_SHADOW_CASCADES] = vec3[NUM_SHADOW_CASCADES](
    vec3(1,0,0),
    vec3(0,1,0),
    vec3(0,0,1),
    vec3(1,1,0)
);

vec2 shadow_vsm_variance(vec3 dir, int light_index, float distance, float min_variance, float variance_transition, out float alpha) {
    // Calculate the variance
    vec3 sampledValue = texture(shadowMap[light_index], dir).rgb;
    vec2 moments = sampledValue.rg;
    alpha = 1.0;//sampledValue.b;
    float variance = max(moments.y - (moments.x * moments.x), min_variance);
    float d = distance - moments.x;
    return (vec2(linstep(variance_transition, 1.0, variance / (variance + d * d)), moments.x));
}

vec3 shadow_vsm(float distance, vec3 dir, int light_index, float min_variance, float variance_transition, float shadow_softness_raw, float penumbra_size, bool hard_shadows) {
    float clamped_distance = clamp(distance, 0.0, 200.0);
    float shadow_softness = shadow_softness_raw * 10.0;
    shadow_softness = mix(shadow_softness, distance * 10.0, penumbra_size);
    distance = distance / 200;
    
    if (hard_shadows) {
        // Sample the shadowmap directly
        float alpha;
        vec2 variance = shadow_vsm_variance(dir, light_index, distance, min_variance, variance_transition, alpha);
        return vec3(min(max(step(distance * alpha, variance.y), variance.x), 1.0));
    }

    // Get the offset coordinates
    ivec3 ofs_coord = ivec3(0);
    vec2 ofs = mod(gl_FragCoord.xy, vec2(shadow_window_size));
    ofs_coord.yz = ivec2(ofs);
    float ofs_sum = 0.0;
    int samples_div2 = int(shadow_filter_size * shadow_filter_size / 2.0);
    vec4 sc = vec4(dir, 1.0);
    sc.z = dir.z;

    vec2 tex_size = textureSize(shadowMap[light_index], 0);
    vec3 texelSize = 1.0 / vec3(tex_size.xyy);
    vec3 light_plane_normal = normalize(dir);
    vec3 up_axis = normalize(view[1].xyz);
    vec3 tangent = normalize(cross(light_plane_normal, up_axis));
    vec3 bitangent = cross(light_plane_normal, tangent);

    for (int i = 0; i < 4; i++) {
        ofs_coord.x = i;
        vec4 offsets = texelFetch(shadow_offsets, ofs_coord, 0) * shadow_softness;

        vec3 offset_dir = tangent * offsets.r + bitangent * offsets.g;
        sc.xyz = dir.xyz + offset_dir * texelSize;
        float alpha;
        vec2 variance = shadow_vsm_variance(sc.xyz, light_index, distance, min_variance, variance_transition, alpha);
        ofs_sum += min(max(step(distance*alpha, variance.y), variance.x), 1.0);

        offset_dir = tangent * offsets.b + bitangent * offsets.a;
        sc.xyz = dir.xyz + offset_dir * texelSize;
        variance = shadow_vsm_variance(sc.xyz, light_index, distance, min_variance, variance_transition, alpha);
        ofs_sum += min(max(step(distance*alpha, variance.y), variance.x), 1.0);
    }

    float shadow_sum = ofs_sum / 8.0;

    if (shadow_sum != 0.0 && shadow_sum != 1.0) {
        for (int i = 4; i < samples_div2; i++) {
            ofs_coord.x = i;
            vec4 offsets = texelFetch(shadow_offsets, ofs_coord, 0) * shadow_softness;

            vec3 offset_dir = tangent * offsets.r + bitangent * offsets.g;
            sc.xyz = dir.xyz + offset_dir * texelSize;
            float alpha;
            vec2 variance = shadow_vsm_variance(sc.xyz, light_index, distance, min_variance, variance_transition, alpha);
            ofs_sum += min(max(step(distance*alpha, variance.y), variance.x), 1.0);

            offset_dir = tangent * offsets.b + bitangent * offsets.a;
            sc.xyz = dir.xyz + offset_dir * texelSize;
            variance = shadow_vsm_variance(sc.xyz, light_index, distance, min_variance, variance_transition, alpha);
            ofs_sum += min(max(step(distance*alpha, variance.y), variance.x), 1.0);
        }

        shadow_sum = ofs_sum / (samples_div2 * 2.0);
    }

    return vec3(shadow_sum);
}

vec3 shadow_csm(float distance, vec3 lightDir, int light_index, float shadow_bias, float normal_bias, float shadow_softness, bool hard_shadows) {
    // Determine which cascade to use
    int cascade_index = -1;
    for (int i = 0; i < NUM_SHADOW_CASCADES; i++) {
        if (distance < u_cascade_distances[i]) {
            cascade_index = i;
            break;
        }
    }
    if (cascade_index == -1) {
        cascade_index = NUM_SHADOW_CASCADES - 1;
    }
    
    // Blend cascades using a blend region value
    float blend_factor = 0.0;
    int matrix_offset = 0;
    {
        float cascade_distance = u_cascade_distances[cascade_index];
        if (distance > cascade_distance - u_blend_region) {
            blend_factor = (distance - (cascade_distance - u_blend_region)) / u_blend_region;
            blend_factor = clamp(blend_factor, 0.0, 1.0);
            matrix_offset = 1;
        }
    }

#ifdef CSM_DEBUG
    return mix(cascade_debug_colors[cascade_index], cascade_debug_colors[cascade_index + matrix_offset], blend_factor);
#endif

    light_t light = lightBuffer.lights[light_index];

    vec4 fragPosLightSpace = light_shadow_matrix_csm[cascade_index] * vec4(v_position_ws, 1.0);
    vec4 fragPosLightSpace2 = light_shadow_matrix_csm[cascade_index+matrix_offset] * vec4(v_position_ws, 1.0);

    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec3 projCoords2 = fragPosLightSpace2.xyz / fragPosLightSpace2.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    projCoords2 = projCoords2 * 0.5 + 0.5;
    vec4 sc = vec4(projCoords, 1.0);
    vec4 sc2 = vec4(projCoords2, 1.0);

    float currentDepth = projCoords.z;
    float currentDepth2 = projCoords2.z;

    if (currentDepth > 1.0) {
        return vec3(1.0);
    }

    // Calculate bias
    vec3 normal = normalize(v_normal_ws);
    float bias = max(normal_bias * (dot(normal, -lightDir)), shadow_bias);
 
    // CSM
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap2D[cascade_index], 0);
    vec2 texelSize2 = vec2(1,1);
    if (cascade_index < NUM_SHADOW_CASCADES - 1) {
        texelSize2 = 1.0 / textureSize(shadowMap2D[cascade_index + matrix_offset], 0);
    }

    if (hard_shadows) {
        float csmDepth = texture(shadowMap2D[cascade_index], projCoords.xy).r;
        float csmDepth2 = texture(shadowMap2D[cascade_index + matrix_offset], projCoords2.xy).r;
        if (cascade_index == NUM_SHADOW_CASCADES - 1) {
            csmDepth2 = 1;
        }
        return vec3(mix(currentDepth - bias > csmDepth ? 0.0 : 1.0, currentDepth2 - bias > csmDepth2 ? 0.0 : 1.0, blend_factor));
    }

    // Get the offset coordinates
    ivec3 ofs_coord = ivec3(0);
    vec2 ofs = mod(gl_FragCoord.xy, vec2(shadow_window_size));
    ofs_coord.yz = ivec2(ofs);
    float ofs_sum = 0.0;
    int samples_div2 = int(shadow_filter_size * shadow_filter_size / 2.0);

    for (int i = 0; i < 4; i++) {
        ofs_coord.x = i;
        vec4 offsets = texelFetch(shadow_offsets, ofs_coord, 0) * shadow_softness;
        sc.xy = projCoords.xy + offsets.rg * texelSize;
        sc2.xy = projCoords2.xy + offsets.rg * texelSize2;
        float alpha=1.0;
        float csmDepth = texture(shadowMap2D[cascade_index], sc.xy).r;
        float csmDepth2 = texture(shadowMap2D[cascade_index + matrix_offset], sc2.xy).r;
        if (cascade_index == NUM_SHADOW_CASCADES - 1) {
            csmDepth2 = 1;
        }
        ofs_sum += mix(currentDepth - bias > csmDepth ? alpha : 0.0, currentDepth2 - bias > csmDepth2 ? alpha : 0.0, blend_factor);

        sc.xy = projCoords.xy + offsets.ba * texelSize;
        sc2.xy = projCoords2.xy + offsets.ba * texelSize2;
        csmDepth = texture(shadowMap2D[cascade_index], sc.xy).r;
        csmDepth2 = texture(shadowMap2D[cascade_index + matrix_offset], sc2.xy).r;
        if (cascade_index == NUM_SHADOW_CASCADES - 1) {
            csmDepth2 = 1;
        }
        ofs_sum += mix(currentDepth - bias > csmDepth ? alpha : 0.0, currentDepth2 - bias > csmDepth2 ? alpha : 0.0, blend_factor);
    }

    float shadow_sum = ofs_sum / 8.0;

    if (shadow_sum != 0.0 && shadow_sum != 1.0) {
        for (int i = 4; i < samples_div2; i++) {
            ofs_coord.x = i;
            vec4 offsets = texelFetch(shadow_offsets, ofs_coord, 0) * shadow_softness;
            sc.xy = projCoords.xy + offsets.rg * texelSize;
            sc2.xy = projCoords2.xy + offsets.rg * texelSize2;
            float alpha=1.0;
            float csmDepth = texture(shadowMap2D[cascade_index], sc.xy).r;
            float csmDepth2 = texture(shadowMap2D[cascade_index + matrix_offset], sc2.xy).r;
            if (cascade_index == NUM_SHADOW_CASCADES - 1) {
                csmDepth2 = 1;
            }
            ofs_sum += mix(currentDepth - bias > csmDepth ? alpha : 0.0, currentDepth2 - bias > csmDepth2 ? alpha : 0.0, blend_factor);

            sc.xy = projCoords.xy + offsets.ba * texelSize;
            sc2.xy = projCoords2.xy + offsets.ba * texelSize2;
            csmDepth = texture(shadowMap2D[cascade_index], sc.xy).r;
            csmDepth2 = texture(shadowMap2D[cascade_index + matrix_offset], sc2.xy).r;
            if (cascade_index == NUM_SHADOW_CASCADES - 1) {
                csmDepth2 = 1;
            }
            ofs_sum += mix(currentDepth - bias > csmDepth ? alpha : 0.0, currentDepth2 - bias > csmDepth2 ? alpha : 0.0, blend_factor);
        }

        shadow_sum = ofs_sum / (samples_div2 * 2.0);
    }
    return vec3(1.0 - shadow_sum);
}

vec4 shadowmap(int idx, in vec4 peye, in vec4 neye) {
    vec3 fragment = vec3(peye);
    vec3 shadowFactor = vec3(1.0);
    light_t light = lightBuffer.lights[idx];

    if (light.processed_shadows) {
        if (light.type == LIGHT_DIRECTIONAL) {
            shadowFactor = shadow_csm(-peye.z, light.dir.xyz, idx, light.shadow_bias, light.normal_bias, light.shadow_softness, light.hard_shadows);
        } else if (light.type == LIGHT_POINT || light.type == LIGHT_SPOT) {
            vec3 light_pos = (view * vec4(light.pos.xyz, 1.0)).xyz;
            vec3 dir = light_pos - fragment;
            vec4 sc = inv_view * vec4(dir, 0.0);
            shadowFactor = shadow_vsm(length(dir), -sc.xyz, idx, light.min_variance, light.variance_transition, light.shadow_softness, light.penumbra_size, light.hard_shadows);
        }
    }

    return vec4(shadowFactor, 1.0);
}

vec4 shadowing(int idx) {
    if (u_shadow_receiver) {
        return shadowmap(idx, vpeye, vneye);
    } else {
        return vec4(1.0);
    }
}

#else

vec4 shadowing(int idx) {
    return vec4(1.0);
}

#endif

#endif

