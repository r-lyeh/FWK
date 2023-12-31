#define lerp(a,b,c) mix(a,b,c)
#define saturate(c) clamp(c, 0.0, 1.0)

// This function approximates the black and white film stock
// "Ilford FP4" pushed to 400 ISO. It is typical for noir style. This
// tonal transformation does not approximate gamma 2.2, so an explicit
// sRGB transform should be performed before display.
//
// vec3 hdr (IN): Color pixel in linear space
// vec2 uv (IN): Screen space UV for vignette
//
// returns Filmic remapped pixel in gamma 1.0 space
// src: pic.twitter.com/7ZSMM5RRMz
//

vec3 PostFilmic_IlfordFp4Push(vec3 c, vec2 uv) {
    // film curve coefficients
    const vec3 cb = vec3( 0.0307479,  0.00030400, -0.04458630);
    const vec3 de = vec3(-0.0095000, -0.00162400, -0.01736670);
    const vec3 df = vec3( 0.1493590,  0.21412400,  1.85780000);
    const vec3 ef = vec3(-0.0636051, -0.00758438, -0.00934798);

    c = c * c * 1.88; // quick approximation of skip bleach

    // remap color channels
    vec3 ax = vec3(2.36691,5.14272,0.49020)*c;
    vec3 pn = (c*(ax+cb)+de);
    vec3 pd = (c*(ax+vec3(0.022,0.004,-0.10543))+df);

    // collapse color channels
    float pr = dot(saturate(pn/pd),vec3(0.45,0.45,0.45));

    // vignette
    float pv = saturate(pow(1.0 - dot(uv-.5, uv-.5), -.758) + -.23);
    float x = lerp(pr,pr*pr*pr,pv); // done
    return vec3(x,x,x);
}

out vec4 color;

void main(void) {
    vec2 uv = TEXCOORD.st;
    vec4 src = texture2D(iChannel0, uv);
    color = vec4( PostFilmic_IlfordFp4Push(src.xyz, uv), src.a);
}
