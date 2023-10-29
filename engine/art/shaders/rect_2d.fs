uniform sampler2D texture0; /*unit0*/
uniform float u_inv_gamma;
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
    fragcolor.rgb = pow( fragcolor.rgb, vec3( u_inv_gamma ) ); // defaults: 1.0/2.2 gamma
}