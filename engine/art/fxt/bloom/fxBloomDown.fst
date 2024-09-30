vec3 pow_vec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float invGamma = 2.2;
vec3 srgb_to_linear(vec3 v)   { return pow_vec3(v, invGamma); }

float srgb_to_luma(vec3 col)
{
    //return dot(col, vec3(0.2126f, 0.7152f, 0.0722f));
	return dot(col, vec3(0.299f, 0.587f, 0.114f));
}

float karis_average(vec3 col)
{
	// Formula is 1 / (1 + luma)
	float luma = srgb_to_luma(srgb_to_linear(col)) * 0.25f;
	return 1.0f / (1.0f + luma);
}

uniform int miplevel; /// set:1
uniform int karis_disabled; /// set:0
uniform float threshold; /// set:1.0
uniform float soft_threshold; /// set:0.5
vec3 prefilter(vec3 color0) {
    // vec3 color = srgb_to_linear(color0);
    vec3 color = color0;
    float brightness = max(color.r, max(color.g, color.b));
    float knee = threshold * soft_threshold;
    float softness = brightness - threshold + knee;
    softness = clamp(softness, 0, 2.0 * knee);
    softness = softness * softness / (4.0 * knee + 0.00001);
    float contribution = max(softness, brightness - threshold);
    contribution /= max(brightness, 0.00001);
    // return pow_vec3(color * contribution, 1.0 / invGamma);
    return color0 * contribution;
}


void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 texel_size = 1.0 / textureSize(iChannel0, 0);
    float x = texel_size.x;
    float y = texel_size.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(iChannel0, vec2(uv.x - 2*x, uv.y + 2*y)).rgb;
    vec3 b = texture(iChannel0, vec2(uv.x,       uv.y + 2*y)).rgb;
    vec3 c = texture(iChannel0, vec2(uv.x + 2*x, uv.y + 2*y)).rgb;

    vec3 d = texture(iChannel0, vec2(uv.x - 2*x, uv.y)).rgb;
    vec3 e = texture(iChannel0, vec2(uv.x,       uv.y)).rgb;
    vec3 f = texture(iChannel0, vec2(uv.x + 2*x, uv.y)).rgb;

    vec3 g = texture(iChannel0, vec2(uv.x - 2*x, uv.y - 2*y)).rgb;
    vec3 h = texture(iChannel0, vec2(uv.x,       uv.y - 2*y)).rgb;
    vec3 i = texture(iChannel0, vec2(uv.x + 2*x, uv.y - 2*y)).rgb;

    vec3 j = texture(iChannel0, vec2(uv.x - x, uv.y + y)).rgb;
    vec3 k = texture(iChannel0, vec2(uv.x + x, uv.y + y)).rgb;
    vec3 l = texture(iChannel0, vec2(uv.x - x, uv.y - y)).rgb;
    vec3 m = texture(iChannel0, vec2(uv.x + x, uv.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1

    int miplevel1 = miplevel;

    if (karis_disabled == 1) {
        miplevel1 = -1;
    }

    if (miplevel1 == 0) {
        vec3 groups[5];
        // We are writing to mip 0, so we need to apply Karis average to each block
        // of 4 samples to prevent fireflies (very bright subpixels, leads to pulsating
        // artifacts).
        groups[0] = (a+b+d+e) * (0.125*0.25);
        groups[1] = (b+c+e+f) * (0.125*0.25);
        groups[2] = (d+e+g+h) * (0.125*0.25);
        groups[3] = (e+f+h+i) * (0.125*0.25);
        groups[4] = (j+k+l+m) * (0.5*0.25);
        groups[0] *= karis_average(groups[0]);
        groups[1] *= karis_average(groups[1]);
        groups[2] *= karis_average(groups[2]);
        groups[3] *= karis_average(groups[3]);
        groups[4] *= karis_average(groups[4]);
        fragColor.rgb = groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
    } else {
        fragColor.rgb = e*0.125;
        fragColor.rgb += (a+c+g+i)*0.03125;
        fragColor.rgb += (b+d+f+h)*0.0625;
        fragColor.rgb += (j+k+l+m)*0.125;
    }

    fragColor.rgb = max(fragColor.rgb, 0.0001f);
    fragColor.rgb = prefilter(fragColor.rgb);
    fragColor.a = 1.0;
}
