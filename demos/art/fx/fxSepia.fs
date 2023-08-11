uniform float brightness; /// set:1

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec4 src = texture(iChannel0, uv);

    vec3 color = vec3(
        dot(src.rgb, vec3(0.393 * brightness, 0.769 * brightness, 0.189 * brightness)),
        dot(src.rgb, vec3(0.349 * brightness, 0.686 * brightness, 0.168 * brightness)),
        dot(src.rgb, vec3(0.272 * brightness, 0.534 * brightness, 0.131 * brightness))
    );

    fragColor = vec4(color, src.a);
}
