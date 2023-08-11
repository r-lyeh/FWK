uniform sampler2D texture0; /*unit0*/
uniform float u_inv_gamma;


in vec2 uv;
out vec4 fragcolor;


void main() {
    vec4 texel = texture( texture0, uv );
    fragcolor = texel;
    fragcolor.rgb = pow( fragcolor.rgb, vec3( u_inv_gamma ) ); // defaults: 1.0/2.2 gamma
}