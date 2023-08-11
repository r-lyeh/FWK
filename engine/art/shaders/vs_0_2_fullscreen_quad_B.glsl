out vec2 uv;


void main() {
    float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u);
    float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u);
    gl_Position = vec4(-1.0 + x*2.0, 0.0+(-1.0+y*2.0), 0.0, 1.0); // normal(0+),flipped(0-)
    uv = vec2(x, y); // normal(y),flipped(1.0-y)
}