uniform mat4 model;
uniform mat4 cameraToShadowView;
uniform mat4 cameraToShadowProjector;

in vec3 position;
out vec4 v_position;

void main() {
    gl_Position = cameraToShadowProjector * model * vec4(position, 1.0);
    #if VSMCUBE
    v_position  = cameraToShadowView * model * vec4(position, 1.0);
    #else
    v_position  = gl_Position;
    #endif
}