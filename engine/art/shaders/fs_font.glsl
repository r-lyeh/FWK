in vec2 uv;
in float color_index;

uniform sampler2D sampler_font;
uniform sampler2D sampler_colors;
uniform float num_colors;

out vec4 outColor;

void main() {
    vec4 col = texture(sampler_colors, vec2((color_index+0.5)/num_colors, 0.));
    float s = texture(sampler_font, uv).r;
    outColor = vec4(col.rgb, s*col.a);
}
