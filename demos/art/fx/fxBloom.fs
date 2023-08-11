uniform float intensity = 2.0;

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec4 src = texture(iChannel0, uv);
    fragColor = vec4( clamp(src * intensity, 0.0, 1.0).rgb, src.a );
}
