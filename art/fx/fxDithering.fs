// https://en.wikipedia.org/wiki/Ordered_dithering

#define BAYER 4

#if BAYER == 2

const float threshold[4] = float[4](
    1/4.,2/4.,
    3/4.,1/4.
);

#elif BAYER == 4

const float threshold[16] = float[16](
     1/16.,  9/16.,  3/16., 11/16.,
    13/16.,  5/16., 15/16.,  7/16.,
     4/16., 12/16.,  2/16., 10/16.,
    16/16.,  8/16., 14/16.,  6/16.
);

#else // 8

const float threshold[64] = float[64](
     1/64.,33/64., 9/64.,41/64., 3/64.,35/64.,11/64.,43/64.,
    49/64.,17/64.,57/64.,25/64.,51/64.,19/64.,59/64.,27/64.,
    13/64.,45/64., 5/64.,37/64.,15/64.,47/64., 7/64.,39/64.,
    61/64.,29/64.,53/64.,21/64.,63/64.,31/64.,55/64.,23/64.,
     4/64.,36/64.,12/64.,42/64., 2/64.,34/64.,10/64.,42/64.,
    52/64.,20/64.,60/64.,28/64.,50/64.,18/64.,58/64.,26/64.,
    16/64.,48/64., 8/64.,40/64.,14/64.,46/64., 6/64.,38/64.,
    64/64.,32/64.,56/64.,24/64.,62/64.,30/64.,54/64.,22/64.
);

#endif

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord/iResolution.xy;
    vec4 src = texture(iChannel0, uv);

    int x = int(fragCoord.x) % BAYER;
    int y = int(fragCoord.y) % BAYER;

    float luma = dot(vec3(0.2126, 0.7152, 0.0722), src.rgb);
    luma = step(threshold[BAYER*x+y], luma); // find closest

    fragColor = vec4(vec3(luma), src.a);
}
