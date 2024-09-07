uniform samplerCube u_cubemap;

in vec3 v_direction;
out vec4 fragcolor;

void main() {
    vec3 direction = v_direction;
    direction.x = -direction.x;
    fragcolor = vec4(texture(u_cubemap, direction).rgb, 1.0);
}