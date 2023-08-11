uniform float intensity = 0.003f;
uniform float angle = 0.0f;

void main() {
    vec2 uv = TEXCOORD.st;
    vec2 offset = intensity * vec2( cos(angle), sin(angle) );
    vec4 color = texture( iChannel0, uv);
    color.r = texture( iChannel0, uv + offset ).x;
    color.b = texture( iChannel0, uv - offset ).z;

    FRAGCOLOR = color;
}
