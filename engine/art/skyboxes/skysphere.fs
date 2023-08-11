#version 330 core

in vec3 out_worldpos;

uniform float texture_lod;
uniform sampler2D tex_skysphere;
uniform sampler2D tex_skyenv;
uniform bool has_tex_skyenv;
uniform float skysphere_rotation;
uniform float skysphere_blur;
uniform float skysphere_opacity;
uniform float skysphere_mip_count;
uniform float exposure;
uniform uint frame_count;
uniform vec4 background_color;

out vec4 frag_color;

const float PI = 3.1415926536;

// MurMurHash 3 finalizer. Implementation is in public domain.
uint hash( uint h ) {
    h ^= h >> 16;
    h *= 0x85ebca6bU;
    h ^= h >> 13;
    h *= 0xc2b2ae35U;
    h ^= h >> 16;
    return h;
}

// Random function using the idea of StackOverflow user "Spatial" https://stackoverflow.com/a/17479300
// Creates random 23 bits and puts them into the fraction bits of an 32-bit float.
float random( uvec3 h ) {
    uint m = hash(h.x ^ hash( h.y ) ^ hash( h.z ));
    return uintBitsToFloat( ( m & 0x007FFFFFu ) | 0x3f800000u ) - 1.;
}

float random( vec3 v ) {
    return random(floatBitsToUint( v ));
}

vec2 sphere_to_polar( vec3 normal ) {
    normal = normalize( normal );
    return vec2( ( atan( normal.z, normal.x ) + skysphere_rotation ) / PI / 2.0 + 0.5, acos( normal.y ) / PI );
}

void main(void) {
    vec3 sky_env = textureLod( tex_skyenv, sphere_to_polar( normalize( out_worldpos ) ), 0.0f ).rgb;
    vec3 sky_color = textureLod( tex_skysphere, sphere_to_polar( normalize( out_worldpos ) ), skysphere_blur * skysphere_mip_count ).rgb;

    vec3 color = mix( background_color.rgb, mix( sky_color, sky_env, skysphere_blur ) , skysphere_opacity );
    color *= exposure;
    color = color / (vec3(1.) + color);
    color = pow( color, vec3( 1. / 2.2 ));
    float dither = random( uvec3( floatBitsToUint( gl_FragCoord.xy ), frame_count ) );
    color += vec3( (-1.5/256.) + (3./256.) * dither );

    frag_color = vec4( color, 1.0f );
}
