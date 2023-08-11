// based on code by arkano22. See: http://www.gamedev.net/forums/topic/550699-ssao-no-halo-artifacts/
// - rlyeh, public domain

uniform vec2 camerarange = vec2(1.0, 1024.0);

// uniform sampler2D som; // Depth texture (iChannel1)

#if 0

uniform sampler2D rand; // Random texture (iChannel2)

vec2 getRandom(vec2 uv) {
    vec3 random = texture2D(rand, uv.st);
    return random*2.0-vec3(1.0);
}

#else

#define MOD3 vec3(.1031,.11369,.13787)

float hash12(vec2 p) {
    vec3 p3  = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

vec2 hash22(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yzx+19.19);
    return fract(vec2((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y));
}

vec3 getPosition(vec2 uv) {
    float fl = textureLod(iChannel0, vec2(0.), 0.).x;
    float d = textureLod(iChannel0, uv, 0.).w;

    vec2 p = uv*2.-1.;
    mat3 ca = mat3(1.,0.,0.,0.,1.,0.,0.,0.,-1./1.5);
    vec3 rd = normalize( ca * vec3(p,fl) );

    vec3 pos = rd * d;
    return pos;
}

vec2 getRandom(vec2 uv) {
    return normalize(hash22(uv*126.1231) * 2. - 1.);
}

#endif


float pw = 1.0/iResolution.x*0.5;
float ph = 1.0/iResolution.y*0.5;

float readDepth(in vec2 coord) {
    if (coord.x<0||coord.y<0) return 1.0;
    float nearZ = camerarange.x;
    float farZ =camerarange.y;
    float posZ = texture(iChannel1, coord).x;
    return (2.0 * nearZ) / (nearZ + farZ - posZ * (farZ - nearZ));
}

float compareDepths(in float depth1, in float depth2,inout int far) {
    float diff = (depth1 - depth2)*100; //depth difference (0-100)
    float gdisplace = 0.2; //gauss bell center
    float garea = 2.0; //gauss bell width 2
    //reduce left bell width to avoid self-shadowing
    if (diff<gdisplace){
         garea = 0.1;
    }else{
         far = 1;
    }
    float gauss = pow(2.7182,-2*(diff-gdisplace)*(diff-gdisplace)/(garea*garea));
    return gauss;
}

float calcAO(float depth, vec2 uv, float dw, float dh) {
    float temp = 0;
    float temp2 = 0;
    float coordw = uv.x + dw/depth;
    float coordh = uv.y + dh/depth;
    float coordw2 = uv.x - dw/depth;
    float coordh2 = uv.y - dh/depth;
    if (coordw  < 1.0 && coordw  > 0.0 && coordh < 1.0 && coordh  > 0.0){
        vec2 coord = vec2(coordw , coordh);
        vec2 coord2 = vec2(coordw2, coordh2);
        int far = 0;
        temp = compareDepths(depth, readDepth(coord),far);
        //DEPTH EXTRAPOLATION:
        if (far > 0){
            temp2 = compareDepths(readDepth(coord2),depth,far);
            temp += (1.0-temp)*temp2;
        }
    }
    return temp;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    //randomization texture:
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 random = getRandom( uv + vec2(iTime) );

    //initialize stuff:
    float depth = readDepth(uv);
    float ao = 0.0;
    for(int i=0; i<4;++i) {
        //calculate color bleeding and ao:
        ao+=calcAO(depth, uv,  pw, ph);
        ao+=calcAO(depth, uv,  pw, -ph);
        ao+=calcAO(depth, uv,  -pw, ph);
        ao+=calcAO(depth, uv,  -pw, -ph);
        ao+=calcAO(depth, uv,  pw*1.2, 0);
        ao+=calcAO(depth, uv,  -pw*1.2, 0);
        ao+=calcAO(depth, uv,  0, ph*1.2);
        ao+=calcAO(depth, uv,  0, -ph*1.2);

        //sample jittering:
        pw += random.x*0.0007;
        ph += random.y*0.0007;

        //increase sampling area:
        pw *= 1.7;
        ph *= 1.7;
    }

    //final values, some adjusting:
    vec4 texel = texture(iChannel0, uv);
    float finalAO = 1.0-(ao/32.0); finalAO = 0.5+finalAO*0.5; 
    fragColor = vec4(texel.rgb * vec3(finalAO), texel.a);
    //fragColor = vec4(vec3(finalAO), texel.a); // << debug
}
