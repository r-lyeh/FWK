// Src: http://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting
// Based on Filmic Tonemapping Operators http://filmicgames.com/archives/75

vec3 tonemapFilmic(vec3 linearColor) {
    vec3 x = max(vec3(0.0), linearColor - 0.004);
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

// Remember final color = CustomFilmic(Linear color) / CustomFilmic(Linear white point value) : No gamma baked in

vec3 customFilmic(vec3 linearColor, float shoulderStr, float linearStr, float linearAngle, float toeStr, float toeNumer, float toeDenom) {
    return ((linearColor * (shoulderStr * linearColor + linearAngle * linearStr) + toeStr * toeNumer) / (linearColor * (shoulderStr * linearColor + linearStr) + toeStr * toeDenom)) - toeNumer / toeDenom;
}

// This function applies a "film-like" tonemap to supplied
// HDR pixel. Does not apply 2.2 Gamma correction.
//
// hdr: linear colour in HDR
// whitePoint: scene white point / exposure. must be > 0.0
//
// src: https://twitter.com/jimhejl/status/841149752389558272

vec3 tonemapFilmic_hejl2015(vec3 hdr, float whitePoint) {
    vec4 vh = vec4(hdr, whitePoint);
    vec4 va = (1.425 * vh) + 0.05f; // evaluate filmic curve
    vec4 vf = ((vh * va + 0.004f) / ((vh * (va + 0.55f) + 0.0491f))) - 0.0821f;
    return vf.rgb / vf.www;
}

out vec4 color;

void main(void) {
    vec2 uv = TEXCOORD.st;
    vec4 src = texture2D(iChannel0, uv);
    color = vec4( tonemapFilmic_hejl2015(src.xyz, 0.5), src.a);
    // ccolor = vec4( tonemapFilmic(src.xyz), src.a);
}
