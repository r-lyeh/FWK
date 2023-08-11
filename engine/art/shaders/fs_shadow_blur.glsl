uniform sampler2D textureSource;
uniform vec2 ScaleU;

in vec2 Texcoord;
out vec4 outColor;

void main() {
    vec4 color = vec4(0.0);
    color += texture( textureSource, Texcoord.st + vec2( -3.0*ScaleU.x, -3.0*ScaleU.y ) ) * 0.015625;
    color += texture( textureSource, Texcoord.st + vec2( -2.0*ScaleU.x, -2.0*ScaleU.y ) )*0.09375;
    color += texture( textureSource, Texcoord.st + vec2( -1.0*ScaleU.x, -1.0*ScaleU.y ) )*0.234375;
    color += texture( textureSource, Texcoord.st + vec2( 0.0 , 0.0) )*0.3125;
    color += texture( textureSource, Texcoord.st + vec2( 1.0*ScaleU.x,  1.0*ScaleU.y ) )*0.234375;
    color += texture( textureSource, Texcoord.st + vec2( 2.0*ScaleU.x,  2.0*ScaleU.y ) )*0.09375;
    color += texture( textureSource, Texcoord.st + vec2( 3.0*ScaleU.x, -3.0*ScaleU.y ) ) * 0.015625;
    outColor = vec4(color.xyz, 1.0);
}