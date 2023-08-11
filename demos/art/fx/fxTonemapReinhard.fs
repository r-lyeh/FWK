// https://www.shadertoy.com/view/lslGzl

vec3 tonemapReinhard(const vec3 color) {
//  color *= toneMappingExposure;
    return color / (color + vec3(1.0));
}

vec3 simpleReinhardToneMapping(vec3 color)
{
    float exposure = 1.5;
    color *= exposure/(1. + color / exposure);
    return color;
}

vec3 lumaBasedReinhardToneMapping(vec3 color)
{
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float toneMappedLuma = luma / (1. + luma);
    color *= toneMappedLuma / luma;
    return color;
}

vec3 whitePreservingLumaBasedReinhardToneMapping(vec3 color)
{
    float white = 2.;
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float toneMappedLuma = luma * (1. + luma / (white*white)) / (1. + luma);
    color *= toneMappedLuma / luma;
    return color;
}

out vec4 color;

void main(void) {
    vec2 uv = TEXCOORD.st;
    vec4 src = texture2D(iChannel0, uv);
    color = vec4( whitePreservingLumaBasedReinhardToneMapping(src.xyz), src.a);
}
