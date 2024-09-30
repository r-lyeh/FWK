uniform float filterRadius; /// set:0.005

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    float aspectRatio = iResolution.x / iResolution.y;
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = filterRadius;
    float y = filterRadius * aspectRatio;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(iChannel0, vec2(uv.x - x, uv.y + y)).rgb;
    vec3 b = texture(iChannel0, vec2(uv.x,     uv.y + y)).rgb;
    vec3 c = texture(iChannel0, vec2(uv.x + x, uv.y + y)).rgb;

    vec3 d = texture(iChannel0, vec2(uv.x - x, uv.y)).rgb;
    vec3 e = texture(iChannel0, vec2(uv.x,     uv.y)).rgb;
    vec3 f = texture(iChannel0, vec2(uv.x + x, uv.y)).rgb;

    vec3 g = texture(iChannel0, vec2(uv.x - x, uv.y - y)).rgb;
    vec3 h = texture(iChannel0, vec2(uv.x,     uv.y - y)).rgb;
    vec3 i = texture(iChannel0, vec2(uv.x + x, uv.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    fragColor.rgb = e*4.0;
    fragColor.rgb += (b+d+f+h)*2.0;
    fragColor.rgb += (a+c+g+i);
    fragColor.rgb *= 1.0 / 16.0;
    fragColor.a = 1.0;
}
