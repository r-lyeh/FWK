//  uniform mat4 view = mat4(1.0);
uniform vec3 lightPos = vec3(1.0);
uniform float doTexture = 0.;
uniform sampler2D shadowMap;


vec4 shadowmap(in vec4 vpeye, in vec4 vneye, in vec2 Texcoord, in vec4 sc) {
    return vec4(1.);
    };