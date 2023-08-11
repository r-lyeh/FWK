// exposure tone mapping
// https://learnopengl.com/Advanced-Lighting/HDR

uniform float exposure = 1.0; // [0.1 .. 5]

out vec4 color;

void main(void) {
    vec2 uv = TEXCOORD.st;
    vec4 HDR_color = texture2D(iChannel0, uv); // HDR_color, SRGB texture
    vec3 mapped = vec3(1.0) - exp(-HDR_color.xyz * exposure);
    color = vec4( mapped.xyz, HDR_color.a );
}
