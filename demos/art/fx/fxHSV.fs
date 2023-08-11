uniform float h = 1.0; // tint shift
uniform float s = 0.5; // saturate: >1, decolorize: <1
uniform float v = 1.0; // white: >1, gray: <1

vec3 hsv2rgb(vec3 c) {
    return mix(vec3(1.),clamp((abs(fract(c.r+vec3(3.,2.,1.)/3.)*6.-3.)-1.),0.,1.),c.g)*c.b;
}

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

void main() {
    vec2 uv = TEXCOORD.st;
    vec4 src = texture2D( iChannel0, uv );

    vec3 c = rgb2hsv(src.rgb);

    FRAGCOLOR = vec4( hsv2rgb(c * vec3(h,s,v)), src.a );
}
