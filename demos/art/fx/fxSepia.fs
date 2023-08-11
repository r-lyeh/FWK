uniform float intensity = 1.0;

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec4 src = texture(iChannel0, uv);

    vec3 color = vec3(
        dot(src.rgb, vec3(0.393 * intensity, 0.769 * intensity, 0.189 * intensity)),
        dot(src.rgb, vec3(0.349 * intensity, 0.686 * intensity, 0.168 * intensity)),
        dot(src.rgb, vec3(0.272 * intensity, 0.534 * intensity, 0.131 * intensity))
    );

    fragColor = vec4(color, src.a);
}
