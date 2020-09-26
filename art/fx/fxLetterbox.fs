void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // letterbox
    if( abs(2.*fragCoord.y-iResolution.y) > iResolution.x * 0.42 ) {
        fragColor = vec4( 0., 0., 0., 1. );
        return;
    }

    vec2 uv = fragCoord.xy / iResolution.xy;
    fragColor = texture(iChannel0, uv);
}
