// https://www.shadertoy.com/view/XsBSDm

float g_cw = 15.; // char width in normalized units
float g_ch = 30.; // char height in normalized units

float g_cwb = .6; // character width buffer as a percentage of char width
float g_chb = .5; // line buffer as a percentage of char height

// vertical segment with the bottom of the segment being s
// and having length d
float vd( vec2 s, float d, vec2 uv )
{    
    float t = (d * (uv.y - s.y)) / (d*d);
    t = clamp(t, 0., 1.);
    return .1 * length((s + t * vec2(0., d)) - uv);
}

// horizontal segment with the left of the segment being s
// and having length d
float hd( vec2 s, float d, vec2 uv )
{    
    float t = (d * (uv.x - s.x)) / (d*d);
    t = clamp(t, 0., 1.);
    return .1 * length((s + t * vec2(d, 0.)) - uv);
}

// divide the experience into cells.
vec2 mod_uv(vec2 uv)
{
    return vec2(mod(uv.x, g_cw * (1. + g_cwb)), 
                mod(uv.y, g_ch * (1. + g_chb)));
}

// ---------------------------------------------
// ALPHABET
float a(vec2 uv)
{    
    float r = vd(vec2(0.), g_ch * .9, uv);
    r = min(r, hd(vec2(g_cw * .1, g_ch), g_cw * .8, uv));
    r = min(r, vd(vec2(g_cw, 0.), g_ch * .9, uv));
    r = min(r, hd(vec2(0., g_ch * .6), g_cw, uv));
    return r;
}

float b(vec2 uv)
{    
    float r = vd(vec2(0.), g_ch, uv);
    r = min(r, hd(vec2(.0, g_ch), g_cw, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .7), g_ch * .3, uv));
    r = min(r, hd(vec2(0., g_ch * .6), g_cw * .8, uv));
    r = min(r, vd(vec2(g_cw, 0.), g_ch * .5, uv));
    r = min(r, hd(vec2(0.), g_cw, uv));
    return r;
}

float c(vec2 uv)
{    
    float r = vd(vec2(0., g_ch * .1), g_ch * .8, uv);
    r = min(r, hd(vec2(g_cw * .1, 0.), g_cw * .9, uv));
    r = min(r, hd(vec2(g_cw * .1, g_ch), g_cw * .9, uv));
    return r;
}

float d(vec2 uv)
{    
    float r = vd(vec2(0.), g_ch, uv);
    r = min(r, vd(vec2(g_cw, g_ch * .1), g_ch * .8, uv));
    r = min(r, hd(vec2(0.), g_cw * .9, uv));
    r = min(r, hd(vec2(.0, g_ch), g_cw * .9, uv));
    return r;
}

float e(vec2 uv)
{    
    float r = hd(vec2(.0, g_ch), g_cw, uv);
    r = min(r, vd(vec2(0.), g_ch, uv));
    r = min(r, hd(vec2(0.), g_cw, uv));
    r = min(r, hd(vec2(0., g_ch * .6), g_cw * .8, uv));
    return r;
}

float f(vec2 uv)
{
    float r = hd(vec2(0., g_ch), g_cw, uv);
    r = min(r, vd(vec2(0.), g_ch, uv));
    r = min(r, hd(vec2(0., g_ch * .6), g_cw * .8, uv));
    return r;
}

float g(vec2 uv)
{    
    float r = hd(vec2(g_cw * .1, g_ch), g_cw * .8, uv);
    r = min(r, vd(vec2(0., g_ch * .1), g_ch * .8, uv));
    r = min(r, hd(vec2(g_cw * .1, 0.), g_cw * .8, uv));
    r = min(r, vd(vec2(g_cw, .1 * g_ch), g_ch * .4, uv));
    r = min(r, hd(vec2(g_cw * .5, g_ch * .6), g_cw * .4, uv));
    return r;
}

float h(vec2 uv)
{    
    float r = vd(vec2(0.), g_ch, uv);
    r = min(r, vd(vec2(g_cw, 0.), g_ch, uv));
    r = min(r, hd(vec2(.0, g_ch * .6), g_cw, uv));
    return r;
}
float i(vec2 uv)
{    
    float r = hd(vec2(0.), g_cw, uv);
    r = min(r, vd(vec2(g_cw * .5, 0.), g_ch, uv));
    r = min(r, hd(vec2(0., g_ch), g_cw, uv));
    return r;
}

float j(vec2 uv)
{    
    float r = vd(vec2(g_cw, g_ch * .1), g_ch * .9, uv);
    r = min(r, hd(vec2(g_cw * .1, 0.), g_cw * .8, uv));
    r = min(r, vd(vec2(0., g_ch * .1), g_ch * .2, uv));
    return r;
}


float k(vec2 uv)
{    
    float r = vd(vec2(0.), g_ch, uv);
    r = min(r, vd(vec2(g_cw, g_ch*.7), g_ch * .3, uv));
    r = min(r, vd(vec2(g_cw, 0.), g_ch * .5, uv));
    r = min(r, hd(vec2(0., g_ch * .6), g_cw * .8, uv));
    return r;
}

float l(vec2 uv)
{    
    float r = vd(vec2(0.), g_ch, uv);
    r = min(r, hd(vec2(0.), g_cw, uv));
    return r;
}

float m(vec2 uv)
{    
    float r = vd(vec2(0.), g_ch, uv);
    r = min(r, vd(vec2(g_cw, 0.), g_ch, uv));
    r = min(r, hd(vec2(0., g_ch), g_cw * .3, uv));
    r = min(r, vd(vec2(g_cw * .5, g_ch * .6), .3 * g_ch, uv));
    r = min(r, hd(vec2(g_cw * .7, g_ch), g_cw * .3, uv));
    return r;
}

float n(vec2 uv)
{    
    float r = vd(vec2(0.), g_ch, uv);
    r = min(r, vd(vec2(g_cw, 0.), g_ch, uv));    
    r = min(r, vd(vec2(g_cw * .1, g_ch * .9), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .3, g_ch * .7), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .5, g_ch * .5), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .7, g_ch * .3), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .9, 0.), g_ch * .2, uv));
    return r;
}

float o(vec2 uv)
{    
    float r = vd(vec2(0., g_ch * .1), g_ch * .8, uv);
    r = min(r, hd(vec2(g_cw * .1, g_ch), g_cw * .8, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .1), g_ch * .8, uv));
    r = min(r, hd(vec2(g_cw * .1, 0.), g_cw * .8, uv));
    return r;
}

float p(vec2 uv)
{    
    float r = vd(vec2(0.), g_ch, uv);
    r = min(r, hd(vec2(0., g_ch), g_cw, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .7), g_ch * .3, uv));
    r = min(r, hd(vec2(0., g_ch * .6), g_cw * .8, uv));
    return r;
}
float q(vec2 uv)
{    
    float r = vd(vec2(0., g_ch * .1), g_ch * .8, uv);
    r = min(r, hd(vec2(g_cw * .1, g_ch), g_cw * .8, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .1), g_ch * .8, uv));
    r = min(r, hd(vec2(g_cw * .1, 0.), g_cw * .8, uv));    
    r = min(r, vd(vec2(g_cw * .7, g_ch * -.05), g_cw * .4, uv));
    return r;
}

float r(vec2 uv)
{    
    float r = vd(vec2(0.), g_ch, uv);
    r = min(r, hd(vec2(.0, g_ch), g_cw, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .7), g_ch * .3, uv));
    r = min(r, hd(vec2(0., g_ch * .6), g_cw * .8, uv));
    r = min(r, vd(vec2(g_cw, 0.), g_ch * .5, uv));
    return r;
}
float s(vec2 uv)
{    
    float r = hd(vec2(0.), g_cw * .9, uv);
    r = min(r, vd(vec2(g_cw, g_ch * .1), g_ch * .4, uv));
    r = min(r, hd(vec2(g_cw * .2, g_ch * .6), g_cw * .7, uv));
    r = min(r, vd(vec2(0., g_ch * .7), g_ch * .2, uv));
    r = min(r, hd(vec2(g_cw * .2, g_ch), g_cw * .8, uv));
    return r;
}

float t(vec2 uv)
{    
    float r = hd(vec2(0., g_ch), g_cw, uv);
    r = min(r, vd(vec2(g_cw * .5, 0.), g_ch, uv));
    return r;
}


float u(vec2 uv)
{    
    float r = vd(vec2(0., g_ch * .1), g_ch * .9, uv);
    r = min(r, hd(vec2(g_cw * .1, 0.), g_cw * .8, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .1), g_ch * .9, uv));
    return r;
}

float v(vec2 uv)
{    
    float r = vd(vec2(0., g_ch * .5), g_ch * .5, uv);
    r = min(r, vd(vec2(g_cw, g_ch * .5), g_ch * .5, uv));
    r = min(r, vd(vec2(g_cw * .2, g_ch * .2), g_ch * .2, uv));
    r = min(r, vd(vec2(g_cw * .8, g_ch * .2), g_ch * .2, uv));
    r = min(r, vd(vec2(g_cw * .5, 0.), g_ch * .1, uv));
    return r;
}

float w(vec2 uv)
{    
    float r = vd(vec2(0.), g_ch, uv);
    r = min(r, vd(vec2(g_cw, 0.), g_ch, uv));
    r = min(r, vd(vec2(g_cw * .5, g_ch * .1), g_ch * .3, uv));
    r = min(r, hd(vec2(0.), g_cw * .3, uv));
    r = min(r, hd(vec2(g_cw * .7, 0.), g_cw * .3, uv));
    return r;
}

float x(vec2 uv)
{    
    float r = vd(vec2(0., g_ch * .9), g_ch * .1, uv);
    r = min(r, vd(vec2(g_cw * .2, g_ch * .7), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .5, g_ch * .5), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .8, g_ch * .3), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw, 0.), g_ch * .2, uv));    
    r = min(r, vd(vec2(g_cw, g_ch * .9), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .8, g_ch * .7), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .2, g_ch * .3), g_ch * .1, uv));    
    r = min(r, vd(vec2(0.), g_ch * .2, uv));
    
    return r;
}

float y(vec2 uv)
{    
    float r = vd(vec2(0., g_ch * .8), g_ch * .2, uv);
    r = min(r, vd(vec2(g_cw * .2, g_ch * .6), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .8, g_ch * .6), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .8), g_ch * .2, uv));
    r = min(r, vd(vec2(g_cw * .5, 0.), g_ch * .5, uv));
    
    return r;
}
float z(vec2 uv)
{    
    float r = hd(vec2(0., g_ch), g_cw, uv);
    r = min(r, vd(vec2(g_cw * .9, g_ch * .9), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .7, g_ch * .7), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .5, g_ch * .5), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .3, g_ch * .3), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .1, g_ch * .1), g_ch * .1, uv));
    r = min(r, hd(vec2(0.), g_cw, uv));
    return r;
}

// ---------------------------------------------
// NUMBERS

float n1(vec2 uv)
{    
    float r = hd(vec2(g_cw * .3, g_ch), g_cw * .2, uv);
    r = min(r, vd(vec2(g_cw * .5, 0.) , g_ch, uv));
    return r;
}


float n2(vec2 uv)
{    
    float r = hd(vec2(0., g_ch), .9 * g_cw, uv);
    r = min(r, vd(vec2(g_cw, g_ch*.7), g_ch * .2, uv));
    r = min(r, hd(vec2(g_cw * .2, g_ch * .6), g_cw * .7, uv));
    r = min(r, vd(vec2(0.), g_ch * .5, uv));
    r = min(r, hd(vec2(0.), g_cw * .9, uv));
    return r;
}
float n3(vec2 uv)
{    
    float r = hd(vec2(0., g_ch), .9 * g_cw, uv);
    r = min(r, vd(vec2(g_cw, g_ch * .1), g_ch * .4, uv));
    r = min(r, hd(vec2(g_cw * .2, g_ch * .6), g_cw * .7, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .7), g_ch * .2, uv));
    r = min(r, hd(vec2(0.), g_cw * .9, uv));
    return r;
}
float n4(vec2 uv)
{    
    float r = vd(vec2(0., g_ch * .6), g_ch * .4, uv);
    r = min(r, hd(vec2(.0, g_ch * .6), g_cw, uv));
    r = min(r, vd(vec2(g_cw, 0.), g_ch, uv));
    return r;
}

float n5(vec2 uv)
{    
    float r = hd(vec2(0., g_ch), g_cw, uv);
    r = min(r, vd(vec2(0., g_ch*.6), g_ch * .4, uv));
    r = min(r, hd(vec2(g_cw * .1, g_ch * .6), g_cw * .8, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .1), g_ch * .4, uv));
    r = min(r, hd(vec2(0.), g_cw * .9, uv));
    return r;
}
float n6(vec2 uv)
{    
    float r = hd(vec2(g_cw * .1, g_ch), g_cw * .9, uv);
    r = min(r, vd(vec2(0., g_ch * .1), g_ch * .8, uv));
    r = min(r, hd(vec2(g_cw * .1, 0.), g_cw * .8, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .1), g_ch * .4, uv));
    r = min(r, hd(vec2(g_cw * .2, g_ch * .6), g_cw * .7, uv));
    return r;
}
float n7(vec2 uv)
{
    
    float r = hd(vec2(0., g_ch), g_cw, uv);
    r = min(r, vd(vec2(g_cw * .9, g_ch * .9), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .7, g_ch * .7), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .5, g_ch * .5), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .3, g_ch * .3), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .1, 0.), g_ch * .2, uv));
    return r;
}

float n8(vec2 uv)
{    
    float r = hd(vec2(g_cw * .1, 0.), g_cw * .8, uv);
    r = min(r, vd(vec2(g_cw, g_ch * .1), g_ch * .4, uv));
    r = min(r, hd(vec2(g_cw * .2, g_ch * .6), g_cw * .7, uv));
    r = min(r, vd(vec2(0., g_ch * .1), g_ch * .4, uv));
    r = min(r, hd(vec2(g_cw * .1, g_ch), .8 * g_cw, uv));
    r = min(r, vd(vec2(0., g_ch * .7), g_ch * .2, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .7), g_ch * .2, uv));
    return r;
}
float n9(vec2 uv)
{    
    float r = hd(vec2(g_cw * .1, 0.), g_cw * .8, uv);
    r = min(r, vd(vec2(g_cw, g_ch * .1), g_ch * .8, uv));
    r = min(r, hd(vec2(g_cw * .2, g_ch * .6), g_cw * .6, uv));
    r = min(r, hd(vec2(g_cw * .1, g_ch), g_cw * .8, uv));
    r = min(r, vd(vec2(0., g_ch * .7), g_ch * .2, uv));
    return r;
}
float n0(vec2 uv)
{    
    float r = vd(vec2(0., g_ch * .1), g_ch * .8, uv);
    r = min(r, hd(vec2(g_cw * .1, g_ch), g_cw * .8, uv));
    r = min(r, vd(vec2(g_cw, g_ch * .1), g_ch * .8, uv));
    r = min(r, hd(vec2(g_cw * .1, 0.), g_cw * .8, uv));

    r = min(r, vd(vec2(g_cw * .9, g_ch * .9), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .7, g_ch * .7), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .5, g_ch * .5), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .3, g_ch * .3), g_ch * .1, uv));
    r = min(r, vd(vec2(g_cw * .1, g_ch * .1), g_ch * .1, uv));
    
    return r;
}

// ---------------------------------------------
// SPECIAL CHARACTERS

float pe(vec2 uv) // period
{    
    float r = vd(vec2(0.), g_ch * .02, uv);
    
    return r;
}

// ---------------------------------------------
// MACROS

#define A if (cs == idx) { tx = min(tx, a(uv)); } idx++;
#define B if (cs == idx) { tx = min(tx, b(uv)); } idx++;
#define C if (cs == idx) { tx = min(tx, c(uv)); } idx++;
#define D if (cs == idx) { tx = min(tx, d(uv)); } idx++;
#define E if (cs == idx) { tx = min(tx, e(uv)); } idx++;
#define F if (cs == idx) { tx = min(tx, f(uv)); } idx++;
#define G if (cs == idx) { tx = min(tx, g(uv)); } idx++;
#define H if (cs == idx) { tx = min(tx, h(uv)); } idx++;
#define I if (cs == idx) { tx = min(tx, i(uv)); } idx++;
#define J if (cs == idx) { tx = min(tx, j(uv)); } idx++;
#define K if (cs == idx) { tx = min(tx, k(uv)); } idx++;
#define L if (cs == idx) { tx = min(tx, l(uv)); } idx++;
#define M if (cs == idx) { tx = min(tx, m(uv)); } idx++;
#define N if (cs == idx) { tx = min(tx, n(uv)); } idx++;
#define O if (cs == idx) { tx = min(tx, o(uv)); } idx++;
#define P if (cs == idx) { tx = min(tx, p(uv)); } idx++;
#define Q if (cs == idx) { tx = min(tx, q(uv)); } idx++;
#define R if (cs == idx) { tx = min(tx, r(uv)); } idx++;
#define S if (cs == idx) { tx = min(tx, s(uv)); } idx++;
#define T if (cs == idx) { tx = min(tx, t(uv)); } idx++;
#define U if (cs == idx) { tx = min(tx, u(uv)); } idx++;
#define V if (cs == idx) { tx = min(tx, v(uv)); } idx++;
#define W if (cs == idx) { tx = min(tx, w(uv)); } idx++;
#define X if (cs == idx) { tx = min(tx, x(uv)); } idx++;
#define Y if (cs == idx) { tx = min(tx, y(uv)); } idx++;
#define Z if (cs == idx) { tx = min(tx, z(uv)); } idx++;
#define N1 if (cs == idx) { tx = min(tx, n1(uv)); } idx++;
#define N2 if (cs == idx) { tx = min(tx, n2(uv)); } idx++;
#define N3 if (cs == idx) { tx = min(tx, n3(uv)); } idx++;
#define N4 if (cs == idx) { tx = min(tx, n4(uv)); } idx++;
#define N5 if (cs == idx) { tx = min(tx, n5(uv)); } idx++;
#define N6 if (cs == idx) { tx = min(tx, n6(uv)); } idx++;
#define N7 if (cs == idx) { tx = min(tx, n7(uv)); } idx++;
#define N8 if (cs == idx) { tx = min(tx, n8(uv)); } idx++;
#define N9 if (cs == idx) { tx = min(tx, n9(uv)); } idx++;
#define N0 if (cs == idx) { tx = min(tx, n0(uv)); } idx++;
#define PE if (cs == idx) { tx = min(tx, pe(uv)); } idx++;
#define SP idx++;
#define NL idx+=int(cc - mod(float(idx), cc));

// ---------------------------------------------


// Periodic saw tooth function that repeats with a period of 
// 4 and ranges from [-1, 1].  
// The function starts out at 0 for x=0,
//  raises to 1 for x=1,
//  drops to 0 for x=2,
//  continues to -1 for x=3,
//  and then rises back to 0 for x=4
// to complete the period

float sawtooth( float x )
{
    float xmod = mod(x+3.0, 4.);
    return abs(xmod-2.0) - 1.;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
        
    float display_width = 1010.;
    float cc = floor(display_width / (g_cw * (1. + g_cwb))); // character count per line
    
    vec2 uv = (fragCoord.xy) / iResolution.xx;
    uv.y = iResolution.y/iResolution.x - uv.y;  // type from top to bottom, left to right   
    uv *= display_width;

    int cs = int(floor(uv.x / (g_cw * (1. + g_cwb))) + cc * floor(uv.y/(g_ch * (1. + g_chb))));

    uv = mod_uv(uv);
    uv.y = g_ch * (1. + g_chb) - uv.y; // paint the character from the bottom left corner
    vec3 ccol = .35 * vec3(.1, .3, .2) * max(smoothstep(3., 0., uv.x), smoothstep(5., 0., uv.y));   
    uv -= vec2(g_cw * g_cwb * .5, g_ch * g_chb * .5);
    
    float tx = 10000.;
    int idx = 0;
    
    //A B C D E F G H I J K L M N O P Q R S T U V W X Y Z N1 N2 N3 N4 N5 N6 N7 N8 N9 N0 PE NL
    
    NL 
	NL 
    SP I N SP T H E SP N O T SP T O O SP D I S T A N T SP F U T U R E NL 
    SP W A Y SP D O W N SP I N SP D E E P SP N1 N3 NL
    SP D R PE SP F O R R E S T E R SP A N D SP T V S SP F R A N K NL
    SP W E R E SP H A T C H I N G SP A N SP E V I L SP S C H E M E PE NL
    SP T H E Y SP H I R E D SP A SP T E M P SP B Y SP T H E SP N A M E SP O F SP M I K E NL
    SP J U S T SP A SP R E G U L A R SP J O E SP T H E Y SP D I D N T SP L I K E PE NL
    SP T H E I R SP E X P E R I M E N T SP N E E D E D SP A SP G O O D SP T E S T SP C A S E NL
    SP S O SP T H E Y SP C O N K E D SP H I M SP O N SP T H E SP N O G G I N NL
    SP A N D SP T H E Y SP S H O T SP H I M SP I N T O SP S P A C E SP PE PE PE 

    float throb = .4 + .2 * (.5 * sin(4. * iGlobalTime - .05 * float(cs)) + .5);
    vec3 tcol = vec3(.7, 1., .8) * smoothstep(.2, .0, tx);
    vec3 tglow = vec3(.7, 1., .8) * (.3 * throb + .7 * mix(.6, 1., throb) * smoothstep(throb, .1, tx));
    
    vec3 terminal_color = ccol + tcol + tglow;
    vec3 simple_color = vec3(max(0., 1. - tx));
    float trans = smoothstep(-.1, .1, sawtooth(.3 * iGlobalTime - 1.));
        
    fragColor = vec4(mix(terminal_color, simple_color, trans), 1.0);
    
}