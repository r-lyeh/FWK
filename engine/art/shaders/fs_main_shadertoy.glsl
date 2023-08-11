void mainImage( out vec4 fragColor, in vec2 fragCoord );
void main() {
    mainImage(fragColor, texcoord.xy * iResolution);
}