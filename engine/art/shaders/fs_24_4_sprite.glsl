uniform sampler2D u_texture;

in vec2 vTexCoord;
in vec4 vColor;
out vec4 fragColor;


// [src] https://www.shadertoy.com/view/MllBWf CC1.0
vec4 texture_AA(sampler2D tx, vec2 uv) {
    vec2 res = vec2(textureSize(tx, 0));
    uv = uv*res + 0.5;
    // tweak fractionnal value of the texture coordinate
    vec2 fl = floor(uv);
    vec2 fr = fract(uv);
    vec2 aa = fwidth(uv)*0.75;
    fr = smoothstep( vec2(0.5)-aa, vec2(0.5)+aa, fr);
    // return value
    uv = (fl+fr-0.5) / res;
    return texture(tx, uv);
}


// [src] https://www.shadertoy.com/view/MllBWf CC1.0
vec4 texture_AA2( sampler2D tex, vec2 uv) {
    vec2 res = vec2(textureSize(tex,0));
    uv = uv*res;
    vec2 seam = floor(uv+0.5);
    uv = seam + clamp( (uv-seam)/fwidth(uv), -0.5, 0.5);
    return texture(tex, uv/res);
}


// [src] https://www.shadertoy.com/view/ltBfRD
vec4 texture_AA3(sampler2D tex, vec2 uv) {
    vec2 res = vec2(textureSize(tex,0));
    float width = 2.0;
    uv = uv * res;
    // ---
    vec2 uv_floor = floor(uv + 0.5);
    vec2 uv_fract = fract(uv + 0.5);
    vec2 uv_aa = fwidth(uv) * width * 0.5;
    uv_fract = smoothstep(
    vec2(0.5) - uv_aa,
    vec2(0.5) + uv_aa,
    uv_fract
    );
    uv = (uv_floor + uv_fract - 0.5) / res;
    return texture(tex, uv);
}


void main() {
    vec4 texColor = texture_AA2(u_texture, vTexCoord);
    if(texColor.a < 0.9) discard;
    fragColor = vColor * texColor;
}