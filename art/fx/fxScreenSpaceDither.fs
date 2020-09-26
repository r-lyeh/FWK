// https://www.shadertoy.com/view/4dcSRX
// https://www.shadertoy.com/view/MslGR8
// https://www.shadertoy.com/view/Md3XRf *

// note: valve edition from http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
// note: input in pixels (ie not normalized uv)

uniform float intensity = 250.0; // [2..255]

vec3 ScreenSpaceDither2(vec2 vScreenPos, float colorDepth) {
    // lestyn's RGB dither (7 asm instructions) from Portal 2 X360, slightly modified for VR
    vec3 vDither = vec3(dot(vec2(131.0, 312.0), vScreenPos.xy + iTime));
    vDither.rgb = fract(vDither.rgb / vec3(103.0, 71.0, 97.0)) - vec3(0.5, 0.5, 0.5);
    return (vDither.rgb / colorDepth) * 0.375;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;

    vec4 color = texture(iChannel0, uv);
    fragColor = color + vec4(ScreenSpaceDither2(gl_FragCoord.xy,255.0 - intensity), 0);
}
