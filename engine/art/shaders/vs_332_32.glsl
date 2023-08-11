uniform mat4 u_mvp;


in vec3 att_position;
in vec3 att_normal;
in vec2 att_texcoord;
in vec4 att_color;
out vec4 v_color;
out vec3 v_position;
out vec3 v_position_ws;
out vec3 v_normal;
out vec3 v_normal_ws;
out vec2 v_texcoord;

// shadow
uniform mat4 model, view, proj;
uniform mat4 cameraToShadowProjector; // !VSMCUBE
out vec4 vneye;
out vec4 vpeye;
out vec4 sc; // !VSMCUBE
void do_shadow() {
    vneye = view * model * vec4(att_normal,   0.0f);
    vpeye = view * model * vec4(att_position, 1.0);
    sc = cameraToShadowProjector * model * vec4(att_position, 1.0f);
}


void main() {
    gl_Position = u_mvp * vec4(att_position, 1.0);
    v_position = att_position;
    v_position_ws = vec3(model * vec4(att_position, 1.));
    v_normal = normalize(att_normal);
    v_normal_ws = normalize(vec3(model * vec4(att_normal, 0.))); // normal world/model space
    v_texcoord = att_texcoord;
    v_color = att_color;
    do_shadow();
}