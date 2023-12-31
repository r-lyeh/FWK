uniform sampler2D texture0; /*unit0*/

in vec2 uv;
out vec4 fragcolor;

void main() {
    vec4 texel = texture( texture0, uv );
    fragcolor = texel;
}