uniform float hardness = 0.1;
uniform float flickering = 0.01;

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;

    vec4 src = texture( iChannel0, vec2(uv.x,uv.y) );
    vec3 color = src.rgb;

    color *= (1.0 - hardness)+hardness*sin(10.0*iTime+uv.y*1000.0);
    color *= (1.0 - flickering)+flickering*sin(100.0*iTime);

    fragColor = vec4(color, src.a);
}
