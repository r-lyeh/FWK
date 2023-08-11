uniform float amount; /// min:0 set:0.10 max:0.5

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;

    // letterbox
    if( uv.y < amount || (1.0-uv.y) < amount ) {
        fragColor = vec4( 0., 0., 0., 1. );
        return;
    }

    fragColor = texture(iChannel0, uv);
}
