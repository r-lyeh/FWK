uniform mat4 u_mvp;

void main() {
    gl_Position = u_mvp * vec4(0.0, 0.0, 0.0, 1.0);
}
