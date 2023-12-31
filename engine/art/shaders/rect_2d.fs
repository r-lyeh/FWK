uniform sampler2D texture0; /*unit0*/
uniform vec4 u_tint;
uniform int u_has_tex;

in vec2 uv;
out vec4 fragcolor;

void main() {
    vec4 texel = texture( texture0, uv );
    if (u_has_tex > 0) {
        fragcolor = vec4(texel.xyz*u_tint.xyz, texel.a*u_tint.a);
    } else {
        fragcolor = u_tint;
    }
}