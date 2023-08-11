uniform mat4 u_mvp;


in vec3 att_Position;
in vec2 att_TexCoord;
in vec4 att_Color;
out vec2 vTexCoord;
out vec4 vColor;


void main() {
    vColor = att_Color;
    vTexCoord = att_TexCoord;
    gl_Position = u_mvp * vec4(att_Position, 1.0);
}