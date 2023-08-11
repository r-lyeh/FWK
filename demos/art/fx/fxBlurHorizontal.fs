// [ref] https://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

uniform float intensity; /// set:4

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;

    vec4 base = texture(iChannel0, uv), color = vec4(0.0);
    vec2 offset = vec2(intensity,0) / iResolution;
    color += base * 0.30;
    color += texture(iChannel0, uv + offset) * 0.35;
    color += texture(iChannel0, uv - offset) * 0.35;

    fragColor = vec4( color.rgb, base.a );
}
