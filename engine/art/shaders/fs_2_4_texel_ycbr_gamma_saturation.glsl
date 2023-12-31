uniform sampler2D u_texture_y;  /*unit0*/
uniform sampler2D u_texture_cb; /*unit1*/
uniform sampler2D u_texture_cr; /*unit2*/
// uniform float u_gamma; /// set:2.2


in vec2 uv;
out vec4 fragcolor;


void main() {
    float y = texture(u_texture_y, uv).r;
    float cb = texture(u_texture_cb, uv).r;
    float cr = texture(u_texture_cr, uv).r;
    
    const mat4 to_rgb = mat4(
    1.0000, 1.0000, 1.0000, 0.0000,
    0.0000, -0.3441, 1.7720, 0.0000,
    1.4020, -0.7141, 0.0000, 0.0000,
    -0.7010, 0.5291, -0.8860, 1.0000
    );
    vec4 texel = to_rgb * vec4(y, cb, cr, 1.0);
    /* same as:
    vec3 yCbCr = vec3(y,cb-0.5,cr-0.5);
    vec4 texel = vec4(  dot( vec3( 1.0,  0.0,      1.402 ),    yCbCr ),
    dot( vec3( 1.0 , -0.34414 , -0.71414 ), yCbCr ),
    dot( vec3( 1.0,  1.772,    0.0 ),      yCbCr ), 1.0);
    */
    // gamma correction
    // texel.rgb = pow(texel.rgb, vec3(u_gamma));
    
    // saturation (algorithm from Chapter 16 of OpenGL Shading Language)
    if(false) { float saturation = 2.0; const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec3 intensity = vec3(dot(texel.rgb, W));
    texel.rgb = mix(intensity, texel.rgb, saturation); }
    fragcolor = vec4(texel.rgb, 1.0);
}