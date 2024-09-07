#ifndef FOG_GLSL
#define FOG_GLSL

#include "utils.glsl"

// Fog uniforms
uniform vec3 u_fog_color;
uniform float u_fog_density;
uniform float u_fog_start;
uniform float u_fog_end;
uniform int u_fog_type;
uniform bool u_fog_dither;

vec3 do_fog(vec3 fragment_color) {
    if (u_fog_type == 0) {
        return fragment_color;
    }
    float distance = length(v_position_ws - u_cam_pos);
    float fog_factor = 0.0;

    if (u_fog_type == 1) {
        // Linear fog
        fog_factor = clamp((u_fog_end - distance) / (u_fog_end - u_fog_start), 0.0, 1.0);
    } else if (u_fog_type == 2) {
        // Exponential fog
        fog_factor = exp(-u_fog_density * distance);
    } else if (u_fog_type == 3) {
        // Exponential squared fog
        fog_factor = exp(-pow(u_fog_density * distance, 2.0));
    } else if (u_fog_type == 4) {
        // Distance fog
        fog_factor = clamp((u_fog_start - v_depth) / (u_fog_end - u_fog_start), 0.0, 1.0);
    }

    // Dithering for PS1-style graininess
    if (u_fog_dither) {
        vec2 ditheredCoord = gl_FragCoord.xy / 12.0;
        float dither = rand(ditheredCoord + frame_time * 0.1) * 0.1 - 0.05;
        fog_factor += dither;
    }

    return mix(u_fog_color, fragment_color, fog_factor);
}


#endif