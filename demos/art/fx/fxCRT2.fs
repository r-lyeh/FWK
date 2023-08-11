// [src] https://github.com/Hammster/windows-terminal-shaders (CC0)

// Settings
#define GRAIN_INTENSITY 0.02
#define TINT_COLOR vec4(1, 0.7f, 0, 0)
#define ENABLE_SCANLINES 1
#define ENABLE_REFRESHLINE 1
#define ENABLE_NOISE 1
#define ENABLE_CURVE 1
#define ENABLE_TINT 0
#define ENABLE_GRAIN 0
#define DEBUG 0

// Grain Lookup Table
#define a0  0.151015505647689
#define a1 -0.5303572634357367
#define a2  1.365020122861334
#define b0  0.132089632343748
#define b1 -0.7607324991323768

const vec4 tint = TINT_COLOR;
const vec4 scanlineTint = vec4(0.6f, 0.6f, 0.6f, 0.0f);

float permute(float x) {
    x *= (34 * x + 1);
    return 289 * fract(x * 1 / 289.0f);
}

float rand(float state) {
    return fract(permute(state) / 41.0f);
}

float fmod(float x, float y) {
    return x - y * trunc(x/y);
}

vec4 CRT( vec2 uv ) {
    vec2 xy = uv;

    #if ENABLE_CURVE
    // TODO: add control variable for transform intensity
    xy -= 0.5f;             // offcenter screen
    float r = xy.x * xy.x + xy.y * xy.y;    // get ratio
    xy *= 4.2f + r;             // apply ratio
    xy *= 0.25f;                // zoom
    xy += 0.5f;             // move back to center

    // TODO: add monitor visuals and make colors static consts
    // Outter Box
    if(xy.x < -0.025f || xy.y < -0.025f) return vec4(0, 0, 0, 0);
    if(xy.x > 1.025f  || xy.y > 1.025f)  return vec4(0, 0, 0, 0);
    // Bazel
    if(xy.x < -0.015f || xy.y < -0.015f) return vec4(0.03f, 0.03f, 0.03f, 0.0f);
    if(xy.x > 1.015f  || xy.y > 1.015f)  return vec4(0.03f, 0.03f, 0.03f, 0.0f);
    // Screen Border
    if(xy.x < 0.001f  || xy.y < 0.001f)  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
    if(xy.x > 0.999f  || xy.y > 0.999f)  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
    #endif

    vec4 color = texture(iChannel0, xy);

    #if DEBUG
    if(xy.x < 0.5f) return color;
    #endif

    #if ENABLE_REFRESHLINE
    float timeOver = fmod(iTime / 5, 1);
    float refreshLineColorTint = timeOver - xy.y;
    if(xy.y > timeOver && xy.y - 0.03f < timeOver ) color.rgb += (refreshLineColorTint * 2.0f);
    #endif

    #if ENABLE_SCANLINES
    // scanlines are always every 1px
    if(fmod(floor(uv.y * iResolution.y), 2) != 0) color *= scanlineTint;
    #endif

    #if ENABLE_TINT
    float grayscale = (color.r + color.g + color.b) / 3.f;
    color = vec4(grayscale, grayscale, grayscale, 0);
    color *= tint;
    #endif

    #if ENABLE_GRAIN
    vec3 m = vec3(tex, fmod(iTime, 5) / 5) + 1.;
    float state = permute(permute(m.x) + m.y) + m.z;

    float p = 0.95 * rand(state) + 0.025;
    float q = p - 0.5;
    float r2 = q * q;

    float grain = q * (a2 + (a1 * r2 + a0) / (r2 * r2 + b1 * r2 + b0));
    color.rgb += GRAIN_INTENSITY * grain;
    #endif

    return color;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec4 src = texture(iChannel0, uv);

    fragColor = vec4(CRT(uv).rgb, src.a);
}
