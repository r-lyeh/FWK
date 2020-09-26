uniform float intensity = 1.0;

void mainImage( out vec4 fragColor, in vec2 fragCoord ){
    vec2 uv = fragCoord / iResolution.xy;
    vec4 src = texture(iChannel0, uv);
    vec3 kernel = src.rgb * 9. +
        -1. * texture(iChannel0, uv + vec2(-1,-1) / iResolution.xy).rgb
        -1. * texture(iChannel0, uv + vec2( 0,-1) / iResolution.xy).rgb
        -1. * texture(iChannel0, uv + vec2( 1,-1) / iResolution.xy).rgb
        -1. * texture(iChannel0, uv + vec2(-1, 0) / iResolution.xy).rgb
        -1. * texture(iChannel0, uv + vec2( 1, 0) / iResolution.xy).rgb
        -1. * texture(iChannel0, uv + vec2(-1, 1) / iResolution.xy).rgb
        -1. * texture(iChannel0, uv + vec2( 0, 1) / iResolution.xy).rgb
        -1. * texture(iChannel0, uv + vec2( 1, 1) / iResolution.xy).rgb;
    vec3 outcolor = mix(src.rgb, kernel, intensity);
    fragColor = vec4(outcolor, src.a);
}
