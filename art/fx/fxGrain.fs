uniform float intensity = 16.0;

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec4 color = texture(iChannel0, uv), outcolor;

    float x = (uv.x + 4.0 ) * (uv.y + 4.0 ) * (iTime * 10.0);
    vec4 grain = vec4(mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01)-0.005) * intensity;

    outcolor = color + grain; // method 1
    // outcolor = color * (1.0 - grain); // method 2

    FRAGCOLOR = vec4(outcolor.rgb, color.a);
}
