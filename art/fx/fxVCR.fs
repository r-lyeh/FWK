/**
 * (c) 2021 FMS_Cat
 * Original shader: https://www.shadertoy.com/view/MdffD7
 * I dumbass don't know what it says despite it's my own shader
 */
/*
 * Copyright 2021 FMS_Cat
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define half2 vec2
#define half3 vec3
#define half4 vec4
#define saturate(c) clamp(c, 0.0, 1.0)

//uniform float iTime;
uniform float iScale;
//uniform vec2  iResolution;
uniform vec4  iBackground;

const float PI = 3.14159265f;

const int SAMPLES = 6;

const float COLOR_NOISE_AMP = 0.1f;
const vec3 YIQ_OFFSET = vec3( -0.1f, -0.1f, 0.0f );
const vec3 YIQ_AMP = vec3( 1.2f, 1.1f, 1.5f );

bool validuv( vec2 uv )
{
    return 0.0f < uv.x && uv.x < 1.0f && 0.0f < uv.y && uv.y < 1.0f;
}

vec2 yflip( vec2 uv )
{
    return vec2( uv.x, 1.0 - uv.y );
}

float fs( float s )
{
    return fract( sin( s * 114.514f ) * 1919.810f );
}

float fs2( vec2 s )
{
    return fs( s.x + fs( s.y ) );
}

mat2x2 rotate2D( float t )
{
  return mat2x2( cos( t ), sin( t ), -sin( t ), cos( t ) );
}

vec3 rgb2yiq( vec3 rgb )
{
  return mat3x3( 0.299f, 0.596f, 0.211f, 0.587f, -0.274f, -0.523f, 0.114f, -0.322f, 0.312f ) * rgb;
}

vec3 yiq2rgb( vec3 yiq )
{
  return mat3x3( 1.000f, 1.000f, 1.000f, 0.956f, -0.272f, -1.106f, 0.621f, -0.647f, 1.703f ) * yiq;
}

float v2Random( vec2 v )
{
    vec2 vf = fract( v * 256.0f );
    vec2 vi = floor( v * 256.0f ) / 256.0f;
    vec2 d = vec2( 0.0f, 1.0f / 256.0f );

    return mix(
        mix( fs2( vi + d.xx ), fs2( vi + d.yx ), vf.x ),
        mix( fs2( vi + d.xy ), fs2( vi + d.yy ), vf.x ),
        vf.y
    );
}

half3 vhsTex2D( vec2 uv ) {
    if ( validuv( uv ) ) {
        half3 yiq = half3( 0.0f, 0.0f, 0.0f );
        for ( int i = 0; i < SAMPLES; i ++ ) {
            vec2 uvt = uv - vec2( float( i ), 0.0f ) / iResolution;
            if ( validuv( uvt ) ) {
                half4 tex = texture(iChannel0, uvt );
                yiq += (
                    rgb2yiq( mix( iBackground.rgb, tex.rgb, tex.a ) ) *
                    vec2( float( i ), float( SAMPLES - 1 - i ) ).yxx / float( SAMPLES - 1 )
                ) / float( SAMPLES ) * 2.0f;
            }
        }
        return yiq2rgb( yiq );
    }
    return half3( 0.1f, 0.1f, 0.1f );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;

    vec2 uvt = yflip( uv );
    vec3 col = vec3( 0.0f, 0.0f, 0.0f );

    // tape wave
    uvt.x += ( v2Random( vec2( uvt.y / 10.0f, iTime / 10.0f ) / 1.0f ) - 0.5f ) / iResolution.x * 1.0f;
    uvt.x += ( v2Random( vec2( uvt.y, iTime * 10.0f ) ) - 0.5f ) / iResolution.x * 1.0f;

    // tape crease
    float tcPhase = smoothstep( 0.9f, 0.96f, sin( uvt.y * 8.0f - ( iTime + 0.14f * v2Random( iTime * vec2( 0.67f, 0.59f ) ) ) * PI * 1.2f ) );
    float tcNoise = smoothstep( 0.3f, 1.0f, v2Random( vec2( uvt.y * 4.77f, iTime ) ) );
    float tc = tcPhase * tcNoise;
    uvt.x = uvt.x - tc / iResolution.x * 8.0f;

    // switching noise
    float snPhase = smoothstep( 6.0f / iResolution.y, 0.0f, uvt.y );
    uvt.y += snPhase * 0.3f;
    uvt.x += snPhase * ( ( v2Random( vec2( uv.y * 100.0f, iTime * 10.0f ) ) - 0.5f ) / iResolution.x * 24.0f );

    // fetch
    half4 tex = texture(iChannel0, uv);
    half3 color = vhsTex2D( yflip( uvt ) );
    color = pow( color, vec3(0.4545f) );

    // crease noise
    float cn = tcNoise * ( 0.3f + 0.7f * tcPhase );
    if ( 0.29f < cn ) {
        vec2 uvtt = ( uvt + vec2( 1.0f, 0.0f ) * v2Random( vec2( uvt.y, iTime ) ) ) * vec2( 0.1f, 1.0f );
        float n0 = v2Random( uvtt );
        float n1 = v2Random( uvtt + vec2( 1.0f, 0.0f ) / iResolution.x );
        if ( n1 < n0 ) {
            color = mix( color, vec3( 2.0f, 2.0f, 2.0f ), pow( n0, 10.0f ) );
        }
    }

    // ac beat
    color *= 1.0f + 0.1f * smoothstep( 0.4f, 0.6f, v2Random( vec2( 0.0f, 0.1f * ( uv.y + iTime * 0.2f ) ) / 10.0f ) );

    // color noise
    half2 noiseuv = uvt + vec2( fs( iTime ), fs( iTime / 0.7f ) );
    half3 noise = half3(
        v2Random( noiseuv ),
        v2Random( noiseuv + 0.7f ),
        v2Random( noiseuv + 1.4f )
    );
    color = saturate( color );

    // yiq
    color = rgb2yiq( color );
    color += COLOR_NOISE_AMP * ( noise - 0.5f );
    color = YIQ_OFFSET + YIQ_AMP * color;
    color = yiq2rgb( color );
    color = pow( color, vec3(2.2f) );

    fragColor = half4( color, tex.a );
}
