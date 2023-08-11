uniform float separation; /// min:-0.10 max:0.10 set:0.003
uniform float angle; /// min:0 max:6.28 set:0

void main() {
    vec2 uv = TEXCOORD.st;
    vec2 offset = separation * vec2( cos(angle), sin(angle) );
    vec4 color = texture( iChannel0, uv);
    color.r = texture( iChannel0, uv + offset ).x;
    color.b = texture( iChannel0, uv - offset ).z;

    FRAGCOLOR = color;
}
