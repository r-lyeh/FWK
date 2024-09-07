#include "model_fs.glsl"
#include "surface.glsl"
#include "fog.glsl"

void main() {
    surface_t surf = surface();

    fragcolor = surf.fragcolor;

    fragcolor.rgb = do_fog(fragcolor.rgb);
}