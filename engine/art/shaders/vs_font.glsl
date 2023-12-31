in vec2 vertexPosition;
in vec4 instanceGlyph;

uniform sampler2D sampler_font;
uniform sampler2D sampler_meta;

uniform float offset_firstline; // ascent
uniform float scale_factor;     // scaling factor proportional to font size
uniform vec2 string_offset;     // offset of upper-left corner

uniform vec2 res_meta;   // 96x2 
uniform vec2 res_bitmap; // 512x256
uniform vec2 resolution; // screen resolution

out vec2 uv;
out float color_index; // for syntax highlighting

void main() {
    // (xoff, yoff, xoff2, yoff2), from second row of texture
    vec4 q2 = texture(sampler_meta, vec2((instanceGlyph.z + 0.5)/res_meta.x, 0.75))*vec4(res_bitmap, res_bitmap);

    vec2 p = vertexPosition*(q2.zw - q2.xy) + q2.xy; // offset and scale it properly relative to baseline
    p *= vec2(1.0, -1.0);                            // flip y, since texture is upside-down
    p.y -= offset_firstline;                         // make sure the upper-left corner of the string is in the upper-left corner of the screen
    p *= scale_factor;                               // scale relative to font size
    p += instanceGlyph.xy + string_offset;           // move glyph into the right position
    p *= 2.0/resolution;                             // to NDC
    p += vec2(-1.0, 1.0);                            // move to upper-left corner instead of center

    gl_Position = vec4(p, 0.0, 1.0);

    // (x0, y0, x1-x0, y1-y0), from first row of texture
    vec4 q = texture(sampler_meta, vec2((instanceGlyph.z + 0.5)/res_meta.x, 0.25));

    // send the correct uv's in the font atlas to the fragment shader
    uv = q.xy + vertexPosition*q.zw;
    color_index = instanceGlyph.w;
}
