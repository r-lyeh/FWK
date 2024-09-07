const int SHADOW_VSM = 0;
const int SHADOW_CSM = 1;

uniform int shadow_technique;

#include "model_fs.glsl"
#include "surface.glsl"

void main() {
    vec4 diffuse = get_diffuse_map();
    diffuse.a *= u_global_alpha*u_global_opacity;

    if (diffuse.a < 0.1)
        discard;

    if (shadow_technique == SHADOW_VSM) {
        float depth = length(v_position) / 200;

        float moment1 = depth;
        float moment2 = depth * depth;
        
        float dx = dFdx(depth);
        float dy = dFdy(depth);
        moment2 += 0.25*(dx*dx+dy*dy);
        fragcolor = vec4( moment1, moment2, /* diffuse.a */ 0.0, 1.0);
    }
    else if (shadow_technique == SHADOW_CSM) {
        fragcolor = vec4(gl_FragCoord.z, /* diffuse.a */ 0.0, 0.0, 1.0);
    }
}
