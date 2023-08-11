// [ref] https://www.inf.ufrgs.br/~oliveira/pubs_files/CVD_Simulation/CVD_Simulation.html

uniform int colorblind_mode = 2; // [0..4]
uniform mat3 colorblind_matrices[5] = mat3[5](
    mat3(1.000,0.000,0.000,  0.000,1.000,0.000,  0.000,0.000,1.000), // 0 no colorblind
    mat3(0.299,0.587,0.114,  0.299,0.587,0.114,  0.299,0.587,0.114), // 1 achromatopsia (luma)
    mat3( // protanomaly (no red cone)
         0.152286, 1.052583,-0.204868,
         0.114503, 0.786281, 0.099216,
        -0.003882,-0.048116, 1.051998
    ),
    mat3( // deuteranomaly (no green cone)
         0.367322, 0.860646, -0.227968,
         0.280085, 0.672501,  0.047413,
        -0.011820, 0.042940,  0.968881
    ),
    mat3( // tritanomaly (no blue cone)
         1.255528,-0.076749,-0.178779,
        -0.078411, 0.930809, 0.147602,
         0.004733, 0.691367, 0.303900
    )
);

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;

    vec4 src = texture(iChannel0, uv);
    fragColor = vec4( src.rgb * colorblind_matrices[colorblind_mode], src.a );
}
