uniform float contrast; /// set:1.5 tip:"bleach-to-gray < 1 > saturate"
uniform float brightness; /// set:0 max:2

void main() {
    vec4 pixelColor = texture(iChannel0, TEXCOORD.st);
    pixelColor.rgb /= pixelColor.a;

    pixelColor.rgb = ((pixelColor.rgb - 0.5f) * max(contrast, 0)) + 0.5f;

    pixelColor.rgb += brightness;

    pixelColor.rgb *= pixelColor.a;

    FRAGCOLOR = pixelColor;
}
