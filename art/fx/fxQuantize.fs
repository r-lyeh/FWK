uniform float factor = 3.0; // [1(max)..255(min)]

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec4 src = texture(iChannel0, uv);
    fragColor = vec4(floor(src.rgb * factor + 0.5) / factor, src.a);
}
