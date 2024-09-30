#define texture2D texture
#define texture2DLod textureLod
#define FRAGCOLOR fragColor
#define texcoord uv
#define TEXCOORD uv
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform float iWidth, iHeight, iTime, iFrame, iMousex, iMousey, iMousez, iMousew;
uniform float iChannelRes0x, iChannelRes0y;
uniform float iChannelRes1x, iChannelRes1y;
vec2 iResolution = vec2(iWidth, iHeight);
vec4 iMouse = vec4(iMousex, iMousey, iMousez, iMousew);
vec2 iChannelResolution[2] = vec2[2]( vec2(iChannelRes0x, iChannelRes0y),vec2(iChannelRes1x, iChannelRes1y) );
float iGlobalTime = iTime;
in vec2 texcoord;
out vec4 fragColor;
