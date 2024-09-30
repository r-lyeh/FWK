/// tip:"Ensure colorbuffer is alpha clear before calling this one."

uniform int thickness; /// set:2
uniform vec4 border_color; /// set:1,1,0,1

void main() {
    vec4 texel = texture(iChannel0, uv);
    float outline = 0.0;
    if( texel.a == 0.0 ) {
        for( int x = -thickness; x <= thickness; x++ ) {
            for( int y = -thickness;y <= thickness; y++ ) {
                float sampleVal = texture(iChannel0, uv+vec2(float(x)/iWidth, float(y)/iHeight)).a;
                if( sampleVal > 0.0 ) {
                    outline = 1.0;
                }
            }
        }
    }

    FRAGCOLOR = mix(texel, border_color, outline * border_color.a);
}