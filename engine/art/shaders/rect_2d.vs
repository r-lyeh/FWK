//version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
uniform float u_window_width;
uniform float u_window_height;
out vec2 uv;

void main() {
    // float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u);
    // float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u);
    float px = (aPos.x / u_window_width) * 2.0 - 1.0;
    float py = 1.0 - (aPos.y / u_window_height) * 2.0;
    gl_Position = vec4(px, py, 0.0, 1.0);
    // gl_Position = vec4(-1.0 + x*2.0, 0.0-(-1.0+y*2.0), 0.0, 1.0); // normal(0+),flipped(0-)
    // uv = vec2(x, y); // normal(y),flipped(1.0-y)
    uv = aTexCoord; // normal(y),flipped(1.0-y)
}
