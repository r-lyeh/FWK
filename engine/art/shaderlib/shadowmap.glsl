#ifndef SHADOWMAP_GLSL
#define SHADOWMAP_GLSL

#ifndef NO_SHADOWS

#include "utils.glsl"

uniform bool u_shadow_receiver;
uniform float u_cascade_distances[NUM_SHADOW_CASCADES];
uniform samplerCube shadowMap[MAX_SHADOW_LIGHTS];
uniform sampler2D shadowMap2D[NUM_SHADOW_CASCADES];
uniform sampler3D shadow_offsets;
uniform int shadow_filter_size;
uniform int shadow_window_size;
uniform mat4 light_shadow_matrix_csm[NUM_SHADOW_CASCADES];

// const float bias_modifier[NUM_SHADOW_CASCADES] = float[NUM_SHADOW_CASCADES](0.95, 0.35, 0.20, 0.1, 0.1, 0.1);
const float bias_modifier[NUM_SHADOW_CASCADES] = float[NUM_SHADOW_CASCADES](1.0, 2.0, 6.0, 9.0);
// const float bias_modifier[NUM_SHADOW_CASCADES] = float[NUM_SHADOW_CASCADES](0.95, 0.35, 0.20, 0.15);

vec2 shadow_vsm_variance(vec3 dir, int light_index, float distance, float min_variance, float variance_transition, out float alpha) {
    // Calculate the variance
    vec3 sampledValue = texture(shadowMap[light_index], dir).rgb;
    vec2 moments = sampledValue.rg;
    alpha = 1.0;//sampledValue.b;
    float variance = max(moments.y - (moments.x * moments.x), min_variance);
    float d = distance - moments.x;
    return (vec2(linstep(variance_transition, 1.0, variance / (variance + d * d)), moments.x));
}

float shadow_vsm(float distance, vec3 dir, int light_index, float min_variance, float variance_transition, float shadow_softness_raw, float penumbra_size) {
    float clamped_distance = clamp(distance, 0.0, 200.0);
    float shadow_softness = shadow_softness_raw * 10.0;
    shadow_softness = mix(shadow_softness, distance * 10.0, penumbra_size);
    distance = distance / 200;
    

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

    return shadow_sum;
}

float shadowmap_cascade_sample(vec2 sc, int cascade_index, float blend_factor, out float alpha) {
    float s1 = texture(shadowMap2D[cascade_index], sc).r;
    // float s2 = texture(shadowMap2D[cascade_index + 1], sc).r;
    alpha = 1.0;
    return s1;//mix(s1, s2, blend_factor);
}

float shadow_csm(float distance, vec3 lightDir, int light_index, float shadow_bias, float normal_bias, float shadow_softness) {
    // Determine which cascade to use
    int cascade_index = -1;
    int min_cascades_range = 0;
    int max_cascades_range = NUM_SHADOW_CASCADES;
    for (int i = min_cascades_range; i < max_cascades_range; i++) {
        if (distance < u_cascade_distances[i]) {
            cascade_index = i;
            break;
        }
    }
    if (cascade_index == -1) {
        cascade_index = max_cascades_range - 1;
    }
    
    int matrix_index = cascade_index - min_cascades_range;

    // Blend cascades using a blend region value
    float blend_region = 200.0;
    float blend_factor = 0.0;
    if (matrix_index < NUM_SHADOW_CASCADES - 1) {
        blend_factor = 0.5;
    }

    light_t light = lightBuffer.lights[light_index];

    vec4 fragPosLightSpace = light_shadow_matrix_csm[matrix_index] * vec4(v_position_ws, 1.0);

    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    vec4 sc = vec4(projCoords, 1.0);

    float currentDepth = projCoords.z;

    if (currentDepth > 1.0) {
        return 1.0;
    }

    // Calculate bias
    vec3 normal = normalize(v_normal_ws);
    // float bias = mix(0.001, 0.0, dot(normal, -lightDir));
    float bias = max(normal_bias * (dot(normal, -lightDir)), shadow_bias);// * bias_modifier[cascade_index];
    // bias *= 1 / (u_cascade_distances[cascade_index]);
 
    // CSM
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap2D[cascade_index], 0);

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
        float alpha;
        float csmDepth = shadowmap_cascade_sample(sc.xy, cascade_index, blend_factor, alpha);
        ofs_sum += currentDepth - bias > csmDepth ? alpha : 0.0;

        sc.xy = projCoords.xy + offsets.ba * texelSize;
        csmDepth = shadowmap_cascade_sample(sc.xy, cascade_index, blend_factor, alpha);
        ofs_sum += currentDepth - bias > csmDepth ? alpha : 0.0;
    }

    float shadow_sum = ofs_sum / 8.0;

    if (shadow_sum != 0.0 && shadow_sum != 1.0) {
        for (int i = 4; i < samples_div2; i++) {
            ofs_coord.x = i;
            vec4 offsets = texelFetch(shadow_offsets, ofs_coord, 0) * shadow_softness;
            sc.xy = projCoords.xy + offsets.rg * texelSize;
            float alpha;
            float csmDepth = shadowmap_cascade_sample(sc.xy, cascade_index, blend_factor, alpha);
            ofs_sum += currentDepth - bias > csmDepth ? alpha : 0.0;

            sc.xy = projCoords.xy + offsets.ba * texelSize;
            csmDepth = shadowmap_cascade_sample(sc.xy, cascade_index, blend_factor, alpha);
            ofs_sum += currentDepth - bias > csmDepth ? alpha : 0.0;
        }

        shadow_sum = ofs_sum / (samples_div2 * 2.0);
    }
    return 1.0 - shadow_sum;
}

vec4 shadowmap(int idx, in vec4 peye, in vec4 neye) {
    vec3 fragment = vec3(peye);
    float shadowFactor = 1.0;
    light_t light = lightBuffer.lights[idx];

    if (light.processed_shadows) {
        if (light.type == LIGHT_DIRECTIONAL) {
            shadowFactor = shadow_csm(-peye.z, light.dir.xyz, idx, light.shadow_bias, light.normal_bias, light.shadow_softness);
        } else if (light.type == LIGHT_POINT || light.type == LIGHT_SPOT) {
            vec3 light_pos = (view * vec4(light.pos.xyz, 1.0)).xyz;
            vec3 dir = light_pos - fragment;
            vec4 sc = inv_view * vec4(dir, 0.0);
            shadowFactor = shadow_vsm(length(dir), -sc.xyz, idx, light.min_variance, light.variance_transition, light.shadow_softness, light.penumbra_size);
        }
    }

    return vec4(vec3(shadowFactor), 1.0);
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

