uniform float vignette = 0.75;

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;

    vec4 src = texture( iChannel0, uv );
    vec3 color = src.rgb;

    color *= (1.0 - vignette) + vignette * 16.0 * uv.x * uv.y * (1.0-uv.x) * (1.0-uv.y);

    fragColor = vec4(color, src.a);
}
