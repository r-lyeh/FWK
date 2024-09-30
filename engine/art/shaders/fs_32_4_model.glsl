#include "model_fs.glsl"
#include "surface.glsl"
#include "fog.glsl"

void main() {
    vec4 albedo = get_diffuse_map();
    if (albedo.a < u_cutout_alpha) discard;
    surface_t surf = surface();

    fragcolor = surf.fragcolor;

    fragcolor.rgb = do_fog(fragcolor.rgb);
}