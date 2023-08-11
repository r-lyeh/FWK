uniform sampler2D fsDiffTex;
uniform sampler2D fsNormalTex;
uniform sampler2D fsPositionTex;
uniform mat4 MVP;


in vec3 v_normal;
in vec2 v_texcoord;
out vec4 fragColor;


void main() {
    vec4 diff = texture(fsDiffTex, v_texcoord).rgba;
    vec3 n = normalize(mat3(MVP) * v_normal); // transform normal to eye space
    fragColor = diff;// * vec4(v_normal.xyz, 1);
}