uniform float contrast = 1.5; // > 1 to saturate, < 1 to bleach-to-gray
uniform float brightness = 0;

void main() {
    vec4 pixelColor = texture(iChannel0, TEXCOORD.st);
    pixelColor.rgb /= pixelColor.a;

    pixelColor.rgb = ((pixelColor.rgb - 0.5f) * max(contrast, 0)) + 0.5f;

    pixelColor.rgb += brightness;

    pixelColor.rgb *= pixelColor.a;

    FRAGCOLOR = pixelColor;
}
