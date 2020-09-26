// stone.jpg
// Abandoned base. Created by Reinder Nijhoff 2014
// @reindernijhoff
//
// https://www.shadertoy.com/view/XsSSRW
//

// #define GOLD
#define BUMPMAP
#define MARCHSTEPS 128
#define MARCHSTEPSREFLECTION 64
#define SPHERE

//----------------------------------------------------------------------
const vec2 dropPosition = vec2(1.05,2.1);
const vec3 backgroundColor = vec3(0.9,0.5,0.2);
#define time iGlobalTime
#define stime2 sin(time*2.)
#define ctime2 cos(time*2.)

//----------------------------------------------------------------------
// noises

float hash( float n ) {
    return fract(sin(n)*43758.5453123);
}

float noise( in vec2 x ) {
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*157.0;
    return mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
               mix( hash(n+157.0), hash(n+158.0),f.x),f.y);
}

const mat2 m2 = mat2( 0.80, -0.60, 0.60, 0.80 );

float fbm( vec2 p ) {
    float f = 0.0;
    f += 0.5000*noise( p ); p = m2*p*2.02;
    f += 0.2500*noise( p ); p = m2*p*2.03;
    f += 0.1250*noise( p ); p = m2*p*2.01;
    f += 0.0625*noise( p );
    
    return f/0.9375;
}

mat2 rot(const in float a) {
	return mat2(cos(a),sin(a),-sin(a),cos(a));	
}

//----------------------------------------------------------------------
// distance primitives

float sdBox( in vec3 p, in vec3 b ) {
    vec3 d = abs(p) - b;
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float sdSphere( in vec3 p, in float s ) {
    return length(p)-s;
}

float sdCylinder( in vec3 p, in vec2 h ) {
    vec2 d = abs(vec2(length(p.xz),p.y)) - h;
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float sdPipe( in vec3 p, in vec3 h ) {
    return length(h.xy-p.xy) - h.z;
}

float sdPPipe( in vec3 p, in vec3 h ) {
    return length(h.xy-p.xy) - h.z - 0.02*(max(sin(p.z*20.)-0.8,0.));
}

//----------------------------------------------------------------------
// distance operators

float opU( float d2, float d1 ) { return min( d1,d2); }
float opS( float d2, float d1 ) { return max(-d1,d2); }
vec2  opU( vec2  d2, vec2  d1 ) { return mix(d1,d2,step(d2.x,d1.x)); }//( d2.x<d1.x)? d2:d1; }
vec2  opS( vec2  d2, vec2  d1 ) { return mix(-d1,d2,step(-d1.x,d2.x)); }//(-d1.x>d2.x)?-d1:d2; }

//----------------------------------------------------------------------
// Map functions

#ifdef SPHERE
vec3 sP;
#endif

float xoffset( float z) { 
    return 2.1*sin(z*0.1);
}

vec2 getSectorId( float z ) {
    float id = floor( (z+6.)/12.);
    return vec2( id, hash(id) );
}

float map( vec3 p ) {
    float zorig = p.z;
    p = vec3( p.x+xoffset(p.z), p.y-2., mod( p.z + 6., 12. ) - 6.);
    
    float x = p.x*2., y = p.y-0.8, z = p.z;
    float d =  -sdBox( vec3((x+y)*0.7071, (y-x)*0.7071, z), vec3(3.8,3.8,20.) );

	d = opS( d, sdBox( p, vec3( 2.5, 2., 5.75 ) )  ); 
    d = opU( d, sdPPipe( vec3(abs(p.x),p.y,p.z), vec3( 2.2, -1.7, 0.25 ) ) );
    d = opU( d, sdPipe( vec3(p.x,abs(p.y-0.2),p.z), vec3( 2.4, 1.5, 0.1 ) ) ); 
    d = opU( d, sdPipe( vec3(p.x,p.y+0.6*cos(p.z*3.1415926/12.),p.z), vec3( -2.4, 1.8, 0.12 ) ) );
    
    d = opU( d, 2.2-p.y );
    float l = distance( p.xz, dropPosition );
	p.y += 0.003*sin(40.*l-6.*time)*exp(-4.*l);
    
    d = opU( d, p.y+2.03 );  
    d = opU( d,  sdSphere( vec3( p.x, p.y-2.3, p.z*0.3), 0.2) );
    
    if( getSectorId(zorig).y > 0.75 ) {
        d = opS( d,  sdCylinder( vec3(p.x, p.y-9., p.z), vec2(1.5,10.) ) );

        vec3 pr = vec3( stime2*p.x+ctime2*p.z, p.y-2.4, stime2*p.z-ctime2*p.x);
        d = opU( d, sdBox( pr, vec3(3.0,0.1,0.1) ) );
        d = opU( d, sdBox( pr, vec3(0.1,0.1,3.0) ) );
    } 
#ifdef SPHERE
    d = opU( d,  sdSphere( vec3( p.x, p.y, zorig)-sP, 0.2) );
#endif    
	return d;
}

float mapMaterial( vec3 p ) {
    float zorig = p.z;
    p = vec3( p.x+xoffset(p.z), p.y-2., mod( p.z + 6., 12. ) - 6.);
    
    float x = p.x*2., y = p.y-0.8, z = p.z;
    vec2 d = vec2( -sdBox( vec3((x+y)*0.7071, (y-x)*0.7071, z), vec3(3.8,3.8,20.) ), 5.);

	d = opS( d, vec2( sdBox( p, vec3( 2.5, 2., 5.75 ) ), 3. ) );
    d = opU( d, vec2( sdPPipe( vec3(abs(p.x),p.y,p.z), vec3( 2.2, -1.7, 0.25 ) ), 1. ) );    
    d = opU( d, vec2( sdPipe( vec3(p.x,abs(p.y-0.2),p.z), vec3( 2.4, 1.5, 0.1 ) ), 4. ) );
    d = opU( d, vec2( sdPipe( vec3(p.x,p.y+0.6*cos(p.z*3.1415926/12.),p.z), vec3( -2.4, 1.8, 0.12 ) ), 4. ) );
    

    d = opU( d, vec2( 2.2-p.y, 5. ) );
    d = opU( d, vec2( p.y+2.03, 2. ) );  
    d = opU( d, vec2( sdSphere( vec3( p.x, p.y-2.3, p.z*0.3), 0.2), 6.) );
    
    if( getSectorId(zorig).y > 0.75 ) {
        d = opS( d, vec2( sdCylinder( vec3(p.x, p.y-4., p.z), vec2(1.5,2.) ), 5.) );

        vec3 pr = vec3( stime2*p.x+ctime2*p.z, p.y-2.4, stime2*p.z-ctime2*p.x);
        d = opU( d, vec2( sdBox( pr, vec3(3.0,0.1,0.1) ), 4.) );
        d = opU( d, vec2( sdBox( pr, vec3(0.1,0.1,3.0) ), 4.) );
    } 
    
#ifdef SPHERE
    d = opU( d,  vec2( sdSphere( vec3( p.x, p.y, zorig)-sP, 0.2), 7.) );
#endif
    
	return abs(d.y);
}

//----------------------------------------------------------------------

vec3 calcNormal( in vec3 pos ) {
    const vec2 e = vec2(1.0,-1.0)*0.005;

    vec3 n = normalize( e.xyy*map( pos + e.xyy ) + 
					    e.yyx*map( pos + e.yyx )   + 
					    e.yxy*map( pos + e.yxy )   + 
					    e.xxx*map( pos + e.xxx )   );  
    
#ifdef BUMPMAP
    vec3 p = pos * 20.;
    if( abs(pos.x+xoffset(pos.z))>2.48 )n = normalize( n + 0.1*vec3(0.,fbm(p.yz)-0.5,fbm(p.zy)-0.5));
#endif
    
    return n;
}

vec3 int1, int2;

float intersect( in vec3 ro, in vec3 rd ) {
	const float maxd = 60.0;
	const float precis = 0.001;
    float h = precis*2.0;
    float t = 0.;
    int1.z = -1.;
    
	for( int i=0; i < MARCHSTEPS; i++ ) {
        if( h < precis ) {
            int1 = ro+rd*t;
            break;
        } 
        h = map( ro+rd*t );
        t += h;
    }
    if( int1.z < 0. ) return -1.;

    ro = ro + rd*t;
    vec3 nor = calcNormal(ro);           
    rd = reflect( rd, nor );
    ro += 0.1 * nor;
    t = 0.0;
    h = precis*2.0;
    
    for( int i=0; i < MARCHSTEPSREFLECTION; i++ ) {
        if( h < precis ) {
            int2 = ro+rd*t;
            return 1.;
        }   
        h = map( ro+rd*t );
        t += h;
    }

    return 0.;
}

float softshadow( in vec3 ro, in vec3 rd, in float mint, in float maxt, in float k ) {
	float res = 1.0;
    float t = mint;
    for( int i=0; i<16; i++ ) {
		if( t>maxt ) break;
        float h = map( ro + rd*t );
        res = min( res, k*h/t );
        t += 0.03 + h;
    }
    return clamp( res, 0.0, 1.0 );

}

float calcOcc( in vec3 pos, in vec3 nor ) {
	float totao = 0.0;
    float sca = 1.0;
    for( int i=0; i<5; i++ )
    {
        float h = 0.02 + float(i)/4.0;
        float d = map( pos + h*nor );
        totao += clamp(h-d,0.0,1.0)*sca;
        sca *= 0.4;
    }
    return clamp( 1.0 - 2.0*totao, 0.0, 1.0 );
}


//----------------------------------------------------------------------
// shade

vec4 texcube( sampler2D sam, in vec3 p, in vec3 n )
{
	vec4 x = texture( sam, p.yz );
	vec4 y = texture( sam, p.zx );
	vec4 z = texture( sam, p.xy );
	return x*abs(n.x) + y*abs(n.y) + z*abs(n.z);
}

float calcLightning( in vec3 pos, in vec3 light, in vec3 nor, in float maxDist, in bool shadow ) {
    float sh = 1.;
    vec3 lig = ( light-pos );
    float im = length( lig );
	lig /= im;
   	if(shadow) sh = softshadow( pos, lig, 1.02, im, 32.0  );
    sh /= im;
    sh *= clamp(dot(nor,lig),0.,1.);
    return sh * (1.-smoothstep(0.,maxDist,im));
}

vec3 shade( in vec3 ro, in vec3 pos, in bool shadow, in float m, in float r ) {
    vec3 light, col = vec3(0.);
    vec3 nor = calcNormal(pos);
    
	vec2 id = getSectorId(pos.z);

    float occ = calcOcc( pos, nor );
    float dist, sh = 1., xo = xoffset(pos.z);

    float rc = hash(id.x+43.);
    float gc = hash(id.x+153.);
    vec3 lc = normalize(vec3( max(rc,gc), min(rc,gc), 0.1 ) );
    
    if( id.y > 0.75 ) {
    	light = vec3( -xo, 6.5, id*12. );
        light.xz += vec2( hash(id.x+56423.), hash(id.x+124.) ) - 0.5;
        sh =  8.;
        dist = 8.;
    } else {
	    light = vec3( -xo, 3.9, id*12. );
        sh = 3.;
        dist = 5.3;
        if( hash(id.x+234.) < 0.15 ) lc *= 1.-clamp( 10.*(fbm( vec2(time*10., id.x) )-2.5*id.y), 0., 1.);
        if( pos.y > 4. ) sh*=0.5;
    }
    
    sh *=  calcLightning( pos, light, nor, dist, shadow );
    
       
    if( m < 6.5 ) col = texcube( iChannel0, pos*0.5, nor ).xyz;
    if( m == 1. ) col *= 0.05;
    if( m == 4. && pos.y > 2. ) col *= vec3(0.1,0.,0.);
    if( m == 4. && pos.y < 2. ) col *= vec3(0.1,0.4,1.2);
    
    if( m == 5. ) col *= (1.+0.5*fbm(pos.yz*2.))*vec3(0.2,0.1,0.05);
    if( m == 2. ) col *= vec3(0.8,0.6,0.4);
    
    col *= lc * occ * sh;  
    
    if( m == 6. ) col = mix( 0.1*col, col*fbm(pos.xz*10.) + 0.8*lc, 
                           (1.-smoothstep( 4.15, 4.2,pos.y)) *
                           smoothstep( 0.01, 0.04,abs(mod(pos.z+0.15,0.3)-0.15)) *
                           smoothstep( 0.01, 0.02,abs(pos.x+xo)));

    col *= clamp(1.-2.*r, 0.65, 1.);
    
	col = mix(  0.05*backgroundColor, col, exp( -0.04*distance(pos, ro) ) );
  

    return col;
}

//----------------------------------------------------------------------
// main

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {    
    vec2 q = fragCoord.xy / iResolution.xy;
	vec2 p = -1.0 + 2.0*q;
	p.x *= iResolution.x / iResolution.y;
       
    if (q.y < .12 || q.y >= .88) {
		fragColor=vec4(0.,0.,0.,1.);
		return;
	}
    
    // camera
    float o = 0.2*noise(vec2(time,0.));
    float z = 2.*time+o;
    float x = -0.95*xoffset(z);
	vec3 ro = vec3(x,1.7+0.02*sin(time*1.13*2.*3.1415926+o), z-1.);
    vec3 ta = vec3(x,1.7, z+5.);
	
#ifdef SPHERE
	sP = vec3(sin(time), 1.6*cos(time*0.4), ro.z+9.+6.*sin(time*0.15) );
//    ta = mix(ta,sP+vec3(0.,2.,0.),0.2);
#endif
    
    if (iMouse.z>=1.) {
		ta.yz *= rot( -(0.5-iMouse.y/iResolution.y)*0.15 );
		ta.xz *= rot( -(0.5-iMouse.x/iResolution.x)*0.5 );
	}
    
    vec3 ww = normalize( ta - ro );
    vec3 uu = normalize( cross(ww,vec3(0.0,1.0,0.0) ) );
    vec3 vv = normalize( cross(uu,ww));
	vec3 rd = normalize( -p.x*uu + p.y*vv + 2.2*ww );
    
    vec3 col = 0.035*backgroundColor;

    // raymarch
    float ints = intersect(ro ,rd );
    if(  ints > -0.5 ) {
        float m = mapMaterial(int1);        
   
#ifdef GOLD
		float r = .8;
#else        
        float xo = xoffset(int1.z);
    	vec3 p = vec3( int1.x+xo, int1.y-2., mod( int1.z + 6., 12. ) - 6.);
        float l = m == 2.?exp(-2.*distance( p.xz, dropPosition )):0.;
        
        float r = 0.6 * clamp(2.*(fbm( int1.xz )-0.6+l), 0.0125, 1.)*clamp(2.-int1.y, 0., 1.);
        if(m == 1.) r = 0.05+0.3 * fbm( int1.xz * 20. );
         if(m == 7.) r = .8;       
        
        if(abs(int1.x+xo) > 2.49) {
            r = fbm(int1.yz*0.5)*0.4*
            clamp(2.*(fbm( int1.yz*vec2(3.2,24.)+vec2(0.5*time,0.) )-0.5+l), 0.0, 1.)
            *clamp(0.5*int1.y, 0., 1.);
        }
        if(m == 4.) { r = 0.1; }
#endif
        col = shade( ro, int1.xyz, true, m, r );
        
        if( ints > 0.5 ) {
            col += r * shade( int1.xyz, int2.xyz, false, mapMaterial(int2), 0. );
        }
	}
    
    // gamma
	col = pow( clamp(col*2.,0.0,1.0), vec3(0.4545) );
	col *= 1.2*vec3(1.,0.99,0.95);   
	col = clamp(1.06*col-0.03, 0., 1.);   

    fragColor = vec4( col, 1.0 );
}
