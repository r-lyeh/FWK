#version 330 core

in vec3 in_pos;

uniform mat4 mat_mvp;

out vec3 out_worldpos;

void main() {
    out_worldpos = in_pos;

    vec4 position = mat_mvp * vec4(in_pos, 0.0);
    gl_Position = position.xyww;
}
