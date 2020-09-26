//bio.jpg, hill.jpg, check.jpg
//Raymarch settings

#define MIN_DIST 0.001
#define MAX_DIST 32.0
#define MAX_STEPS 96
#define STEP_MULT 0.9
#define NORMAL_OFFS 0.01
#define FOCAL_LENGTH 0.8

//Scene settings

//#define SHOW_RAY_COST

//Colors
#define SKY_COLOR_1 vec3(0.60,0.00,0.00)
#define SKY_COLOR_2 vec3(1.00,0.50,0.00)

#define SUN_COLOR_1 vec3(1.00, 0.00, 0.00)
#define SUN_COLOR_2 vec3(1.00, 1.00, 0.00)

#define GRID_COLOR_1 vec3(0.00, 0.05, 0.20)
#define GRID_COLOR_2 vec3(1.00, 0.20, 0.60)

#define WATER_COLOR vec3(0.50, 1.00, 2.90)

//Parameters
#define GRID_SIZE 0.20
#define GRID_LINE_SIZE 1.25

#define WATER_LEVEL 0.20
#define WATER_FOG_SIZE 0.05

#define SUN_DIRECTION vec3( 0.10,-1.00,-0.03)

#define CLOUD_SCROLL vec2(0.002, 0.001)
#define CLOUD_BLUR 2.0
#define CLOUD_SCALE vec2(0.04, 0.10)

#define MOUNTAIN_SCALE 6.0
#define MOUNTAIN_SHIFT 5.3

//Color modes
//vec3(#,#,#) Number of bits per channel

//24 bit color
#define RGB888 vec3(8,8,8)
//16 bit color
#define RGB565 vec3(5,6,5)
#define RGB664 vec3(6,6,4)
//8 bit color
#define RGB332 vec3(3,3,2)
#define RGB242 vec3(2,4,2)
#define RGB222 vec3(2,2,2) //+2 unused

//#define DITHER_ENABLE
#define COLOR_MODE RGB242

//Object IDs
#define SKYDOME 0.
#define FLOOR 1.
#define RIVER 2.

float pi = atan(1.0) * 4.0;
float tau = atan(1.0) * 8.0;

vec3 dither(vec3 color, vec3 bits, vec2 pixel)
{
    vec3 cmax = exp2(bits)-1.0;
    
    vec3 dithfactor = mod(color, 1.0 / cmax) * cmax;
    float dithlevel = texture(iChannel2,pixel / iChannelResolution[2].xy).r;
    
    vec3 cl = floor(color * cmax)/cmax;
    vec3 ch = ceil(color * cmax)/cmax;
    
    return mix(cl, ch, step(dithlevel, dithfactor));
}

struct MarchResult
{
    vec3 position;
    vec3 normal;
    float dist;
    float steps;
    float id;
};

//Returns a rotation matrix for the given angles around the X,Y,Z axes.
mat3 Rotate(vec3 angles)
{
    vec3 c = cos(angles);
    vec3 s = sin(angles);
    
    mat3 rotX = mat3( 1.0, 0.0, 0.0, 0.0,c.x,s.x, 0.0,-s.x, c.x);
    mat3 rotY = mat3( c.y, 0.0,-s.y, 0.0,1.0,0.0, s.y, 0.0, c.y);
    mat3 rotZ = mat3( c.z, s.z, 0.0,-s.z,c.z,0.0, 0.0, 0.0, 1.0);

    return rotX * rotY * rotZ;
}

//==== Distance field operators/functions by iq. ====
vec2 opU(vec2 d1, vec2 d2)
{
    return (d1.x < d2.x) ? d1 : d2;
}

vec2 opS(vec2 d1, vec2 d2)
{
    return (-d1.x > d2.x) ? d1*vec2(-1,1) : d2;
}

vec2 sdSphere(vec3 p, float s, float id)
{
  return vec2(length(p) - s, id);
}

vec2 sdPlane(vec3 p, vec4 n, float id)
{
  // n must be normalized
  return vec2(dot(p,n.xyz) + n.w, id);
}

vec2 sdColumn(vec3 p, float r, float id)
{
    return vec2(((abs(p.x)+abs(p.y))-r)/sqrt(2.0), id);
}

vec2 dfRiver(vec3 p, float id)
{
    float offs = sin(p.y)*0.15 + sin(p.y * 0.2);
    
    return sdColumn(p.xzy + vec3(offs,0,0), 0.4, id);
}

//Distance to the scene
vec2 Scene(vec3 p)
{
    vec2 d = vec2(MAX_DIST, SKYDOME);
    
    d = opU(sdPlane(p, vec4(0, 0,-1, 0), FLOOR), d);
    
    d = opS(dfRiver(p, RIVER), d);
    
	return d;
}

//Surface normal at the current position
vec3 Normal(vec3 p)
{
    vec3 off = vec3(NORMAL_OFFS, 0, 0);
    return normalize
    ( 
        vec3
        (
            Scene(p + off.xyz).x - Scene(p - off.xyz).x,
            Scene(p + off.zxy).x - Scene(p - off.zxy).x,
            Scene(p + off.yzx).x - Scene(p - off.yzx).x
        )
    );
}

//Raymarch the scene with the given ray
MarchResult MarchRay(vec3 orig,vec3 dir)
{
    float steps = 0.0;
    float dist = 0.0;
    float id = 0.0;
    
    for(int i = 0;i < MAX_STEPS;i++)
    {
        vec2 object = Scene(orig + dir * dist);
        
        //Add the sky dome and have it follow the camera.
        object = opU(object, -sdSphere(dir * dist, MAX_DIST, SKYDOME));
        
        dist += object.x * STEP_MULT;
        
        id = object.y;
        
        steps++;
        
        if(abs(object.x) < MIN_DIST * dist)
        {
            break;
        }
    }
    
    MarchResult result;
    
    result.position = orig + dir * dist;
    result.normal = Normal(result.position);
    result.dist = dist;
    result.steps = steps;
    result.id = id;
    
    return result;
}

//Scene texturing/shading
vec3 Shade(MarchResult hit, vec3 direction, vec3 camera)
{
    vec3 color = vec3(0.0);

    if(hit.id == FLOOR)
    {
        vec2 uv = abs(mod(hit.position.xy + GRID_SIZE/2.0, GRID_SIZE) - GRID_SIZE/2.0); 
        
        uv /= fwidth(hit.position.xy);
        
        float riverEdge = dfRiver(hit.position, 0.0).x / fwidth(hit.position.xy).x;
                                                       
        float gln = min(min(uv.x, uv.y), riverEdge) / GRID_SIZE;
        
    	color = mix(GRID_COLOR_1, GRID_COLOR_2, 1.0 - smoothstep(0.0, GRID_LINE_SIZE / GRID_SIZE, gln));
    } 
    
    if(hit.id == RIVER)
    {
        vec2 uv = vec2(hit.position.z, abs(mod(hit.position.y + GRID_SIZE/2.0, GRID_SIZE) - GRID_SIZE/2.0)); 
        uv /= fwidth(hit.position.xy);
        
        float gln = min(uv.x, uv.y) / GRID_SIZE;
        
    	color = mix(GRID_COLOR_1, GRID_COLOR_2, 1.0 - smoothstep(0.0, GRID_LINE_SIZE / GRID_SIZE, gln));
    }
    
    //Distance fog
    color *= 1.0 - smoothstep(0.0, MAX_DIST*0.9, hit.dist);
    
    //Water
    float waterMix = smoothstep(WATER_LEVEL - WATER_FOG_SIZE, WATER_LEVEL + WATER_FOG_SIZE, hit.position.z);
    
    color = mix(color, WATER_COLOR, waterMix);  
    
    if(hit.id == SKYDOME)
    {
        //Sky gradient
        //Causes weird position-colored artefacts around the horizon (AMD R9 270)
        //color = mix(SKY_COLOR_1, SKY_COLOR_2, -hit.position.z/16.0);
    	color += mix(SKY_COLOR_1, SKY_COLOR_2, -hit.position.z/16.0);
        
        //Sun
        vec3 sunDir = normalize(SUN_DIRECTION);
        
        float sun = smoothstep(0.950, 0.952, dot(direction, sunDir));
        
        vec3 sunCol = mix(SUN_COLOR_1, SUN_COLOR_2, -hit.position.z/16.0);

        color = mix(color, sunCol, sun);
        
        //Clouds
        vec2 cloudUV = CLOUD_SCALE * direction.xy / dot(direction, vec3(0, 0,-1));
        cloudUV += CLOUD_SCROLL * iGlobalTime;
        
        color *= smoothstep(0.5, 0.3, texture(iChannel1, cloudUV, CLOUD_BLUR).r) * 0.5 + 0.5;
        
        //Mountains
        float a = atan(hit.position.y, hit.position.x)/tau + 0.5;
        a -= 3.28;
        
        float mountains = MOUNTAIN_SCALE * texture(iChannel0, vec2(a, 0.1),-99.0).r - hit.position.z - MOUNTAIN_SHIFT;
        
        color = mix(color, vec3(0.0), 1.0 - smoothstep(0.6, 0.7, mountains));  
    }
    
    return color;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 res = iResolution.xy / iResolution.y;
	vec2 uv = fragCoord.xy / iResolution.y;
    
    //Camera stuff   
    vec3 angles = vec3(0);
    
    //Auto mode
    if(iMouse.xy == vec2(0,0))
    {
        angles.y = tau * (1.8 / 8.0);
        angles.x = tau * (3.9 / 8.0) + sin(iGlobalTime * 0.1) * 0.3;
    }
    else
    {    
    	angles = vec3((iMouse.xy / iResolution.xy) * pi, 0);
        angles.xy *= vec2(2.0, 1.0);
    }
    
    angles.y = clamp(angles.y, 0.0, 15.5 * tau / 64.0);
    
    mat3 rotate = Rotate(angles.yzx);
    
    vec3 orig = vec3(0, 0,-2) * rotate;
    
    vec3 dir = normalize(vec3(uv - res / 2.0, FOCAL_LENGTH)) * rotate;
    
    //Ray marching
    MarchResult hit = MarchRay(orig, dir);
    
    //Shading
    vec3 color = Shade(hit, dir, orig);
    
    #ifdef SHOW_RAY_COST
    color = mix(vec3(0,1,0), vec3(1,0,0), hit.steps / float(MAX_STEPS));
    #endif
    
    #ifdef DITHER_ENABLE
    color = dither(color, COLOR_MODE, fragCoord);
    #endif
    
	fragColor = vec4(color, 1.0);
}