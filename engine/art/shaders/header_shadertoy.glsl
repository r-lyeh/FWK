#version 130
#define texture2D texture
uniform float      iGlobalTime;           // shader playback time (in seconds)
uniform float      iGlobalDelta;          // ??
uniform float      iGlobalFrame;          // ??
uniform float      iSampleRate;           // ??
uniform float      iTime;                 // ??
uniform int        iFrame;                // ??
uniform float      iChannelTime[4];       // channel playback time (in seconds)
uniform vec2       iResolution;           // viewport resolution (in pixels)
uniform vec3       iChannelResolution[4]; // channel resolution (in pixels)
uniform vec3       iOffset;               // ?? (0,0,0)
uniform vec4       iMouse;                // mouse pixel coords. xy: hover, zw: LMB click)
uniform vec4       iDate;                 // (year, month, day, time in seconds)
uniform sampler2D  iChannel0;             // input channel 0 /*sampler%s*/
uniform sampler2D  iChannel1;             // input channel 1
uniform sampler2D  iChannel2;             // input channel 2
uniform sampler2D  iChannel3;             // input channel 3
in  vec2 texCoord;
out vec4 fragColor;
void mainImage( out vec4 fragColor, in vec2 fragCoord );
void main() {
   mainImage(fragColor, texCoord.xy);
}
