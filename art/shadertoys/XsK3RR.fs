#define NUM_LIGHTS 6
#define WALL_COLOR vec3(1.0, 0.5, 0.1)
#define FLOOR_COLOR vec3(0.4, 0.4, 0.4)
#define AMBIENT_LIGHT vec3(0.1, 0.1, 0.1)
//#define SHOW_SHADOWMAP

#define INF 1e8
#define EPS 1e-3
float tau = atan(1.0)*8.0;

//Globals
vec2 res = vec2(0); //Y-Normalized resolution
vec2 mouse = vec2(0); //Mouse coords (form center)
float psz = 0.0; //Pixel size
vec2 hpo = vec2(0); //Half pixel offset

// Shapes
float sdCircle(float r, vec2 uv)
{
	return length(uv) - r;    
}

float sdRing(float ir, float or, vec2 uv)
{
	return abs(length(uv) - (ir+or)/2.0) - (or - ir);   
}

float sdBox(float s, vec2 uv)
{
	return max(abs(uv.x), abs(uv.y)) - s;   
}

float sdRect(vec2 s, vec2 uv)
{
    uv = abs(uv) - s;
	return max(uv.x, uv.y);
}

float sdPlane(vec2 dir, vec2 uv)
{
	return dot(normalize(dir), uv);   
}

// Operations
float opU(float a, float b)
{
	return min(a, b);   
}

float opI(float a, float b)
{
	return max(a, b);   
}

float opS(float a, float b)
{
	return max(-a, b);   
}

//Domain modifiers
mat2 Rotate(float a)
{
	return mat2(cos(a), sin(a),-sin(a), cos(a));   
}

vec2 Rep1(vec2 uv, float r)
{
	uv.x = mod(uv.x, r) - r/2.0;
    return uv;
}

vec2 Rep2(vec2 uv, vec2 r)
{
	return mod(uv, r) - r/2.0;  
}

// Scene function (must be changed in both tabs)
float Scene(vec2 uv)
{
	float d = -sdRect(res/2.0 - 0.05, uv);
    
    vec2 rp = Rep2(uv, vec2(0.2));
    
    d = opU(sdCircle(0.02, rp), d);
    
    rp = Rep1(uv, 0.2);
    
    d = opU(sdRect(vec2(0.005,0.1), rp), d);
    
    d = opS(sdBox(0.2, uv), d);
    
    d = opU(sdRing(0.08, 0.09, uv), d);
    
    d = opS(sdRect(vec2(0.11,0.03), uv * Rotate(iGlobalTime)), d);
    
    return d;
}

vec2 LightOrigin(int id)
{
	return texture(iChannel0,vec2(0, float(id))/iResolution.xy + hpo).yz;   
}

vec3 LightColor(int id)
{
	return texture(iChannel0,vec2(1, float(id))/iResolution.xy + hpo).yzw;   
}

float SampleShadow(int id, vec2 uv)
{
    float a = atan(uv.y, uv.x)/tau + 0.5;
    float r = length(uv);
    
    float idn = float(id)/iResolution.y;
    
    float s = texture(iChannel0, vec2(a, idn) + hpo).x;
    
    return 1.0-smoothstep(s, s+0.02, length(uv));    
}

//Reads lights from light buffer and combines them.
vec3 MixLights(vec2 uv)
{
    vec3 b = AMBIENT_LIGHT;
    
	for(int i = 0;i < NUM_LIGHTS;i++)
    {
        vec2 o = LightOrigin(i);
        vec3 c = LightColor(i);
        
        float l = 0.01 / pow(length(vec3(uv - o, 0.1)), 2.0);
        l *= SampleShadow(i, uv-o);
        b += c * l;
    }
    
    return b;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    hpo = 0.5 / iResolution.xy;
    res = iResolution.xy / iResolution.y;
    psz = 1.0 / iResolution.y;
	vec2 uv = fragCoord.xy / iResolution.y - res/2.0;
    
    mouse = iMouse.xy / iResolution.y - res/2.0;
    
    vec3 col = vec3(0);
    
    float d = Scene(uv);
    
    col = mix(FLOOR_COLOR, WALL_COLOR, smoothstep(psz, 0.0, d));
    
    col *= MixLights(uv);
    
    #ifdef SHOW_SHADOWMAP
    col = texture(iChannel0, fragCoord/iResolution.xy).rgb;
    #endif
    
	fragColor = vec4(col, 1.0);
} 
