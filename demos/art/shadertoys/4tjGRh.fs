// Planet Shadertoy. Created by Reinder Nijhoff 2015
// @reindernijhoff
//
// https://www.shadertoy.com/view/4tjGRh
//

//#define HIGH_QUALITY
//#define MED_QUALITY
//#define LOW_QUALITY
#define VERY_LOW_QUALITY

const float PI = 3.14159265359;
const float DEG_TO_RAD = (PI / 180.0);
const float MAX = 10000.0;

const float EARTH_RADIUS = 1000.;
const float EARTH_ATMOSPHERE = 5.;
const float EARTH_CLOUDS = 1.;

const float RING_INNER_RADIUS = 1500.;
const float RING_OUTER_RADIUS = 2300.;
const float RING_HEIGHT = 2.;

#ifdef HIGH_QUALITY
    const int   SEA_NUM_STEPS = 7;
    const int	TERRAIN_NUM_STEPS = 140;
    const int   ASTEROID_NUM_STEPS = 11;
	const int	ASTEROID_NUM_BOOL_SUB = 7;
    const int   RING_VOXEL_STEPS = 25;
    const float ASTEROID_MAX_DISTANCE = 1.1; 
	const int   FBM_STEPS = 4;
    const int   ATMOSPHERE_NUM_OUT_SCATTER = 5;
    const int   ATMOSPHERE_NUM_IN_SCATTER = 7;

    #define DISPLAY_LLAMEL
    #define DISPLAY_CLOUDS
    #define DISPLAY_CLOUDS_DETAIL
    #define DISPLAY_TERRAIN_DETAIL
#endif

#ifdef MED_QUALITY
    const int   SEA_NUM_STEPS = 6;
    const int	TERRAIN_NUM_STEPS = 100;
    const int   ASTEROID_NUM_STEPS = 10;
	const int	ASTEROID_NUM_BOOL_SUB = 6;
    const int   RING_VOXEL_STEPS = 24;
    const float ASTEROID_MAX_DISTANCE = 1.; 
	const int   FBM_STEPS = 4;
    const int   ATMOSPHERE_NUM_OUT_SCATTER = 4;
    const int   ATMOSPHERE_NUM_IN_SCATTER = 6;
    #define DISPLAY_CLOUDS
    #define DISPLAY_TERRAIN_DETAIL
    #define DISPLAY_CLOUDS_DETAIL
#endif

#ifdef LOW_QUALITY
    const int   SEA_NUM_STEPS = 5;
    const int	TERRAIN_NUM_STEPS = 75;
    const int   ASTEROID_NUM_STEPS = 9;
	const int	ASTEROID_NUM_BOOL_SUB = 5;
    const int   RING_VOXEL_STEPS = 20;
    const float ASTEROID_MAX_DISTANCE = .85; 
	const int   FBM_STEPS = 3;
    const int   ATMOSPHERE_NUM_OUT_SCATTER = 3;
    const int   ATMOSPHERE_NUM_IN_SCATTER = 5;
#endif

#ifdef VERY_LOW_QUALITY
    const int   SEA_NUM_STEPS = 4;
    const int	TERRAIN_NUM_STEPS = 60;
    const int   ASTEROID_NUM_STEPS = 7;
	const int	ASTEROID_NUM_BOOL_SUB = 4;
    const int   RING_VOXEL_STEPS = 16;
    const float ASTEROID_MAX_DISTANCE = .67; 
	const int   FBM_STEPS = 3;
    const int   ATMOSPHERE_NUM_OUT_SCATTER = 2;
    const int   ATMOSPHERE_NUM_IN_SCATTER = 4;
	#define HIDE_TERRAIN
#endif

const vec3  SUN_DIRECTION = vec3( .940721,  .28221626, .18814417 );
const vec3  SUN_COLOR = vec3(.3, .21, .165);

float time;

//-----------------------------------------------------
// Noise functions
//-----------------------------------------------------

float hash( const in float n ) {
    return fract(sin(n)*43758.5453123);
}
float hash( const in vec2 p ) {
	float h = dot(p,vec2(127.1,311.7));	
    return fract(sin(h)*43758.5453123);
}
float hash( const in vec3 p ) {
	float h = dot(p,vec3(127.1,311.7,758.5453123));	
    return fract(sin(h)*43758.5453123);
}
vec3 hash31( const in float p) {
	vec3 h = vec3(1275.231,4461.7,7182.423) * p;	
    return fract(sin(h)*43758.543123);
}
vec3 hash33( const in vec3 p) {
    return vec3( hash(p), hash(p.zyx), hash(p.yxz) );
}

float noise( const in  float p ) {    
    float i = floor( p );
    float f = fract( p );	
	float u = f*f*(3.0-2.0*f);
    return -1.0+2.0* mix( hash( i + 0. ), hash( i + 1. ), u);
}

float noise( const in  vec2 p ) {    
    vec2 i = floor( p );
    vec2 f = fract( p );	
	vec2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}
float noise( const in  vec3 x ) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+157.0), hash(n+158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
}

float tri( const in vec2 p ) {
    return 0.5*(cos(6.2831*p.x) + cos(6.2831*p.y));
   
}

const mat2 m2 = mat2( 0.80, -0.60, 0.60, 0.80 );

float fbm( in vec2 p ) {
    float f = 0.0;
    f += 0.5000*noise( p ); p = m2*p*2.02;
    f += 0.2500*noise( p ); p = m2*p*2.03;
    f += 0.1250*noise( p ); 
    
#ifndef LOW_QUALITY
#ifndef VERY_LOW_QUALITY
    p = m2*p*2.01;
    f += 0.0625*noise( p );
#endif
#endif
    return f/0.9375;
}

float fbm( const in vec3 p, const in float a, const in float f) {
    float ret = 0.0;    
    float amp = 1.0;
    float frq = 1.0;
    for(int i = 0; i < FBM_STEPS; i++) {
        float n = pow(noise(p * frq),2.0);
        ret += n * amp;
        frq *= f;
        amp *= a * (pow(n,0.2));
    }
    return ret;
}

//-----------------------------------------------------
// Lightning functions
//-----------------------------------------------------

float diffuse( const in vec3 n, const in vec3 l) { 
    return clamp(dot(n,l),0.,1.);
}

float specular( const in vec3 n, const in vec3 l, const in vec3 e, const in float s) {    
    float nrm = (s + 8.0) / (3.1415 * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

float fresnel( const in vec3 n, const in vec3 e, float s ) {
    return pow(clamp(1.-dot(n,e), 0., 1.),s);
}

//-----------------------------------------------------
// Math functions
//-----------------------------------------------------

vec2 rotate(float angle, vec2 v) {
    return vec2(cos(angle) * v.x + sin(angle) * v.y, cos(angle) * v.y - sin(angle) * v.x);
}

float boolSub(float a,float b) { 
    return max(a,-b); 
}
float sphere(vec3 p,float r) {
	return length(p)-r;
}

//-----------------------------------------------------
// Intersection functions (by iq)
//-----------------------------------------------------

vec3 nSphere( in vec3 pos, in vec4 sph ) {
    return (pos-sph.xyz)/sph.w;
}

float iSphere( in vec3 ro, in vec3 rd, in vec4 sph ) {
	vec3 oc = ro - sph.xyz;
	float b = dot( oc, rd );
	float c = dot( oc, oc ) - sph.w*sph.w;
	float h = b*b - c;
	if( h<0.0 ) return -1.0;
	return -b - sqrt( h );
}

float iCSphereF( vec3 p, vec3 dir, float r ) {
	float b = dot( p, dir );
	float c = dot( p, p ) - r * r;
	float d = b * b - c;
	if ( d < 0.0 ) return -MAX;
	return -b + sqrt( d );
}

vec2 iCSphere2( vec3 p, vec3 dir, float r ) {
	float b = dot( p, dir );
	float c = dot( p, p ) - r * r;
	float d = b * b - c;
	if ( d < 0.0 ) return vec2( MAX, -MAX );
	d = sqrt( d );
	return vec2( -b - d, -b + d );
}

vec3 nPlane( in vec3 ro, in vec4 obj ) {
    return obj.xyz;
}

float iPlane( in vec3 ro, in vec3 rd, in vec4 pla ) {
    return (-pla.w - dot(pla.xyz,ro)) / dot( pla.xyz, rd );
}

//-----------------------------------------------------
// Wet stone by TDM
// 
// https://www.shadertoy.com/view/ldSSzV
//-----------------------------------------------------

const float ASTEROID_TRESHOLD 	= 0.001;
const float ASTEROID_EPSILON 	= 1e-6;
const float ASTEROID_DISPLACEMENT = 0.1;
const float ASTEROID_RADIUS = 0.13;

const vec3  RING_COLOR_1 = vec3(0.42,0.3,0.2);
const vec3  RING_COLOR_2 = vec3(0.51,0.41,0.32) * 0.2;

float asteroidRock( const in vec3 p, const in vec3 id ) {  
    float d = sphere(p,ASTEROID_RADIUS);    
    for(int i = 0; i < ASTEROID_NUM_BOOL_SUB; i++) {
        float ii = float(i)+id.x;
        float r = (ASTEROID_RADIUS*2.5) + ASTEROID_RADIUS*hash(ii);
        vec3 v = normalize(hash31(ii) * 2.0 - 1.0);
    	d = boolSub(d,sphere(p+v*r,r * 0.8));       
    }
    return d;
}

float asteroidMap( const in vec3 p, const in vec3 id) {
    float d = asteroidRock(p, id) + noise(p*4.0) * ASTEROID_DISPLACEMENT;
    return d;
}

float asteroidMapDetailed( const in vec3 p, const in vec3 id) {
    float d = asteroidRock(p, id) + fbm(p*4.0,0.4,2.96) * ASTEROID_DISPLACEMENT;
    return d;
}

void asteroidTransForm(inout vec3 ro, const in vec3 id ) {
    float xyangle = (id.x-.5)*time*2.;
    ro.xy = rotate( xyangle, ro.xy );
    
    float yzangle = (id.y-.5)*time*2.;
    ro.yz = rotate( yzangle, ro.yz );
}

void asteroidUnTransForm(inout vec3 ro, const in vec3 id ) {
    float yzangle = (id.y-.5)*time*2.;
    ro.yz = rotate( -yzangle, ro.yz );

    float xyangle = (id.x-.5)*time*2.;
    ro.xy = rotate( -xyangle, ro.xy );  
}

vec3 asteroidGetNormal(vec3 p, vec3 id) {
    asteroidTransForm( p, id );
    
    vec3 n;
    n.x = asteroidMapDetailed(vec3(p.x+ASTEROID_EPSILON,p.y,p.z), id);
    n.y = asteroidMapDetailed(vec3(p.x,p.y+ASTEROID_EPSILON,p.z), id);
    n.z = asteroidMapDetailed(vec3(p.x,p.y,p.z+ASTEROID_EPSILON), id);
    n = normalize(n-asteroidMapDetailed(p, id));
    
    asteroidUnTransForm( n, id );
    return n;
}

vec2 asteroidSpheretracing(vec3 ori, vec3 dir, vec3 id) {
    asteroidTransForm( ori, id );
    asteroidTransForm( dir, id );
    
    vec2 td = vec2(0.0);
    for(int i = 0; i < ASTEROID_NUM_STEPS; i++) {
        vec3 p = ori + dir * td.x;
        td.y = asteroidMap(p, id);
        if(td.y < ASTEROID_TRESHOLD) break;
        td.x += (td.y-ASTEROID_TRESHOLD) * 0.9;
    }
    return td;
}

vec3 asteroidGetStoneColor(vec3 p, float c, vec3 l, vec3 n, vec3 e) {
	return mix( diffuse(n,l)*RING_COLOR_1*SUN_COLOR, SUN_COLOR*specular(n,l,e,3.0), .5*fresnel(n,e,5.));    
}

//-----------------------------------------------------
// Ring (by me ;))
//-----------------------------------------------------

const float RING_DETAIL_DISTANCE = 40.;
const float RING_VOXEL_STEP_SIZE = .03;

vec3 ringShadowColor( const in vec3 ro ) {
    if( iSphere( ro, SUN_DIRECTION, vec4( 0., 0., 0., EARTH_RADIUS ) ) > 0. ) {
        return vec3(0.);
    }
    return vec3(1.);
}

bool ringMap( const in vec3 ro ) {
    return ro.z < RING_HEIGHT/RING_VOXEL_STEP_SIZE && hash(ro)<.5;
}

vec4 renderRingNear( const in vec3 ro, const in vec3 rd ) { 
// find startpoint 
    float d1 = iPlane( ro, rd, vec4( 0., 0., 1., RING_HEIGHT ) );
    float d2 = iPlane( ro, rd, vec4( 0., 0., 1., -RING_HEIGHT ) );
   
    if( d1 < 0. && d2 < 0. ) return vec4( 0. );
    
    float d = min( max(d1,0.), max(d2,0.) );
    
    if( d > ASTEROID_MAX_DISTANCE ) return vec4( 0. );
    
    vec3 ros = ro + rd*d;
    
    // avoid precision problems..
    vec2 mroxy = mod(ros.xy, vec2(10.));
    vec2 roxy = ros.xy - mroxy;
    ros.xy -= roxy;
    ros /= RING_VOXEL_STEP_SIZE;
    ros.xy -= vec2(.013,.112)*time*.5;
    
	vec3 pos = floor(ros);
	vec3 ri = 1.0/rd;
	vec3 rs = sign(rd);
	vec3 dis = (pos-ros + 0.5 + rs*0.5) * ri;
	
    float alpha = 0., dint;
	vec3 offset = vec3(0.), id, asteroidro;
    vec2 asteroid;
    
	for( int i=0; i<RING_VOXEL_STEPS; i++ ) {
		if( ringMap(pos) ) {
            id = hash33(pos);
            offset = id*(1.-2.*ASTEROID_RADIUS)+ASTEROID_RADIUS;
            dint = iSphere( ros, rd, vec4(pos+offset, ASTEROID_RADIUS) );
            
            if( dint > 0. ) {
                asteroidro = ros+rd*dint-(pos+offset);
    	        asteroid = asteroidSpheretracing( asteroidro, rd, id );
				
                if( asteroid.y < .1 ) {
	                alpha = 1.;
        	    	break;	    
                }
            }

        }
		vec3 mm = step(dis.xyz, dis.yxy) * step(dis.xyz, dis.zzx);
		dis += mm * rs * ri;
        pos += mm * rs;
	}
    
    if( alpha > 0. ) {       
        vec3 intersection = ros + rd*(asteroid.x+dint);
        vec3 n = asteroidGetNormal( asteroidro + rd*asteroid.x, id );

        vec3 col = asteroidGetStoneColor(intersection, .1, SUN_DIRECTION, n, rd);

        intersection *= RING_VOXEL_STEP_SIZE;
        intersection.xy += roxy;
        col *= ringShadowColor( intersection );
         
	    return vec4( col, 1.-smoothstep(0.4*ASTEROID_MAX_DISTANCE, 0.5* ASTEROID_MAX_DISTANCE, distance( intersection, ro ) ) );
    }
    
	return vec4(0.);
}

//-----------------------------------------------------
// Ring (by me ;))
//-----------------------------------------------------

float renderRingFarShadow( const in vec3 ro, const in vec3 rd ) {
    // intersect plane
    float d = iPlane( ro, rd, vec4( 0., 0., 1., 0.) );
    
    if( d > 0. ) {
	    vec3 intersection = ro + rd*d;
        float l = length(intersection.xy);
        
        if( l > RING_INNER_RADIUS && l < RING_OUTER_RADIUS ) {
            return .5 + .5 * (.2+.8*noise( l*.07 )) * (.5+.5*noise(intersection.xy));
        }
    }
    return 0.;
}

vec4 renderRingFar( const in vec3 ro, const in vec3 rd, inout float maxd ) {
    // intersect plane
    float d = iPlane( ro, rd, vec4( 0., 0., 1., 0.) );
    
    if( d > 0. && d < maxd ) {
        maxd = d;
	    vec3 intersection = ro + rd*d;
        float l = length(intersection.xy);
        
        if( l > RING_INNER_RADIUS && l < RING_OUTER_RADIUS ) {
            float dens = .5 + .5 * (.2+.8*noise( l*.07 )) * (.5+.5*noise(intersection.xy));
            vec3 col = mix( RING_COLOR_1, RING_COLOR_2, abs( noise(l*0.2) ) ) * abs(dens) * 1.5;
            
            col *= ringShadowColor( intersection );
    		col *= .8+.3*diffuse( vec3(0,0,1), SUN_DIRECTION );
			col *= SUN_COLOR;
            return vec4( col, dens );
        }
    }
    return vec4(0.);
}

vec4 renderRing( const in vec3 ro, const in vec3 rd, inout float maxd ) {
    vec4 far = renderRingFar( ro, rd, maxd );
    float l = length( ro.xy );

    if( abs(ro.z) < RING_HEIGHT+RING_DETAIL_DISTANCE 
        && l < RING_OUTER_RADIUS+RING_DETAIL_DISTANCE 
        && l > RING_INNER_RADIUS-RING_DETAIL_DISTANCE ) {
     	
	    float d = iPlane( ro, rd, vec4( 0., 0., 1., 0.) );
        float detail = mix( .5 * noise( fract(ro.xy+rd.xy*d) * 92.1)+.25, 1., smoothstep( 0.,RING_DETAIL_DISTANCE, d) );
        far.xyz *= detail;    
    }
    
	// are asteroids neaded ?
    if( abs(ro.z) < RING_HEIGHT+ASTEROID_MAX_DISTANCE 
        && l < RING_OUTER_RADIUS+ASTEROID_MAX_DISTANCE 
        && l > RING_INNER_RADIUS-ASTEROID_MAX_DISTANCE ) {
        
        vec4 near = renderRingNear( ro, rd );
        far = mix( far, near, near.w );
        maxd=0.;
    }
            
    return far;
}

//-----------------------------------------------------
// Stars (by me ;))
//-----------------------------------------------------

vec4 renderStars( const in vec3 rd ) {
	vec3 rds = rd;
	vec3 col = vec3(0);
    float v = 1.0/( 2. * ( 1. + rds.z ) );
    
    vec2 xy = vec2(rds.y * v, rds.x * v);
    float s = noise(rds*134.);
    
    s += noise(rds*470.);
    s = pow(s,19.0) * 0.00001;
    if (s > 0.5) {
        vec3 backStars = vec3(s)*.5 * vec3(0.95,0.8,0.9); 
        col += backStars;
    }
	return   vec4( col, 1 ); 
} 

//-----------------------------------------------------
// Atmospheric Scattering by GLtracy
// 
// https://www.shadertoy.com/view/lslXDr
//-----------------------------------------------------

const float ATMOSPHERE_K_R = 0.166;
const float ATMOSPHERE_K_M = 0.0025;
const float ATMOSPHERE_E = 12.3;
const vec3  ATMOSPHERE_C_R = vec3( 0.3, 0.7, 1.0 );
const float ATMOSPHERE_G_M = -0.85;

const float ATMOSPHERE_SCALE_H = 4.0 / ( EARTH_ATMOSPHERE );
const float ATMOSPHERE_SCALE_L = 1.0 / ( EARTH_ATMOSPHERE );

const float ATMOSPHERE_FNUM_OUT_SCATTER = float(ATMOSPHERE_NUM_OUT_SCATTER);
const float ATMOSPHERE_FNUM_IN_SCATTER = float(ATMOSPHERE_NUM_IN_SCATTER);

const int   ATMOSPHERE_NUM_OUT_SCATTER_LOW = 2;
const int   ATMOSPHERE_NUM_IN_SCATTER_LOW = 4;
const float ATMOSPHERE_FNUM_OUT_SCATTER_LOW = float(ATMOSPHERE_NUM_OUT_SCATTER_LOW);
const float ATMOSPHERE_FNUM_IN_SCATTER_LOW = float(ATMOSPHERE_NUM_IN_SCATTER_LOW);

float atmosphericPhaseMie( float g, float c, float cc ) {
	float gg = g * g;
	float a = ( 1.0 - gg ) * ( 1.0 + cc );
	float b = 1.0 + gg - 2.0 * g * c;
    
	b *= sqrt( b );
	b *= 2.0 + gg;	
	
	return 1.5 * a / b;
}

float atmosphericPhaseReyleigh( float cc ) {
	return 0.75 * ( 1.0 + cc );
}

float atmosphericDensity( vec3 p ){
	return exp( -( length( p ) - EARTH_RADIUS ) * ATMOSPHERE_SCALE_H );
}

float atmosphericOptic( vec3 p, vec3 q ) {
	vec3 step = ( q - p ) / ATMOSPHERE_FNUM_OUT_SCATTER;
	vec3 v = p + step * 0.5;
	
	float sum = 0.0;
	for ( int i = 0; i < ATMOSPHERE_NUM_OUT_SCATTER; i++ ) {
		sum += atmosphericDensity( v );
		v += step;
	}
	sum *= length( step ) * ATMOSPHERE_SCALE_L;
	
	return sum;
}

vec4 atmosphericInScatter( vec3 o, vec3 dir, vec2 e, vec3 l ) {
	float len = ( e.y - e.x ) / ATMOSPHERE_FNUM_IN_SCATTER;
	vec3 step = dir * len;
	vec3 p = o + dir * e.x;
	vec3 v = p + dir * ( len * 0.5 );

    float sumdensity = 0.;
	vec3 sum = vec3( 0.0 );

    for ( int i = 0; i < ATMOSPHERE_NUM_IN_SCATTER; i++ ) {
        vec3 u = v + l * iCSphereF( v, l, EARTH_RADIUS + EARTH_ATMOSPHERE );
		float n = ( atmosphericOptic( p, v ) + atmosphericOptic( v, u ) ) * ( PI * 4.0 );
		float dens = atmosphericDensity( v );
  
	    float m = MAX;
		sum += dens * exp( -n * ( ATMOSPHERE_K_R * ATMOSPHERE_C_R + ATMOSPHERE_K_M ) ) 
    		* (1. - renderRingFarShadow( u, SUN_DIRECTION ) );
 		sumdensity += dens;
        
		v += step;
	}
	sum *= len * ATMOSPHERE_SCALE_L;
	
	float c  = dot( dir, -l );
	float cc = c * c;
	
	return vec4( sum * ( ATMOSPHERE_K_R * ATMOSPHERE_C_R * atmosphericPhaseReyleigh( cc ) + 
                         ATMOSPHERE_K_M * atmosphericPhaseMie( ATMOSPHERE_G_M, c, cc ) ) * ATMOSPHERE_E, 
                	     clamp(sumdensity * len * ATMOSPHERE_SCALE_L,0.,1.));
}

float atmosphericOpticLow( vec3 p, vec3 q ) {
	vec3 step = ( q - p ) / ATMOSPHERE_FNUM_OUT_SCATTER_LOW;
	vec3 v = p + step * 0.5;
	
	float sum = 0.0;
	for ( int i = 0; i < ATMOSPHERE_NUM_OUT_SCATTER_LOW; i++ ) {
		sum += atmosphericDensity( v );
		v += step;
	}
	sum *= length( step ) * ATMOSPHERE_SCALE_L;
	
	return sum;
}

vec3 atmosphericInScatterLow( vec3 o, vec3 dir, vec2 e, vec3 l ) {
	float len = ( e.y - e.x ) / ATMOSPHERE_FNUM_IN_SCATTER_LOW;
	vec3 step = dir * len;
	vec3 p = o + dir * e.x;
	vec3 v = p + dir * ( len * 0.5 );

	vec3 sum = vec3( 0.0 );

    for ( int i = 0; i < ATMOSPHERE_NUM_IN_SCATTER_LOW; i++ ) {
		vec3 u = v + l * iCSphereF( v, l, EARTH_RADIUS + EARTH_ATMOSPHERE );
		float n = ( atmosphericOpticLow( p, v ) + atmosphericOpticLow( v, u ) ) * ( PI * 4.0 );
	    float m = MAX;
		sum += atmosphericDensity( v ) * exp( -n * ( ATMOSPHERE_K_R * ATMOSPHERE_C_R + ATMOSPHERE_K_M ) );
		v += step;
	}
	sum *= len * ATMOSPHERE_SCALE_L;
	
	float c  = dot( dir, -l );
	float cc = c * c;
	
	return sum * ( ATMOSPHERE_K_R * ATMOSPHERE_C_R * atmosphericPhaseReyleigh( cc ) + 
                   ATMOSPHERE_K_M * atmosphericPhaseMie( ATMOSPHERE_G_M, c, cc ) ) * ATMOSPHERE_E;
}

vec4 renderAtmospheric( const in vec3 ro, const in vec3 rd, inout float d ) {    
    // inside or outside atmosphere?
    vec2 e = iCSphere2( ro, rd, EARTH_RADIUS + EARTH_ATMOSPHERE );
	vec2 f = iCSphere2( ro, rd, EARTH_RADIUS );
        
    if( length(ro) <= EARTH_RADIUS + EARTH_ATMOSPHERE ) {
        if( d < e.y ) {
            e.y = d;
        }
		d = e.y;
	    e.x = 0.;
        
	    if ( iSphere( ro, rd, vec4(0,0,0,EARTH_RADIUS)) > 0. ) {
	        d = iSphere( ro, rd, vec4(0,0,0,EARTH_RADIUS));
		}
    } else {
    	if(  iSphere( ro, rd, vec4(0,0,0,EARTH_RADIUS + EARTH_ATMOSPHERE )) < 0. ) return vec4(0.);
        
        if ( e.x > e.y ) {
        	d = MAX;
			return vec4(0.);
		}
		d = e.y = min( e.y, f.x );
    }
	return atmosphericInScatter( ro, rd, e, SUN_DIRECTION );
}

vec3 renderAtmosphericLow( const in vec3 ro, const in vec3 rd ) {    
    vec2 e = iCSphere2( ro, rd, EARTH_RADIUS + EARTH_ATMOSPHERE );
    e.x = 0.;
    return atmosphericInScatterLow( ro, rd, e, SUN_DIRECTION );
}

//-----------------------------------------------------
// Seascape by TDM
// 
// https://www.shadertoy.com/view/Ms2SD1
//-----------------------------------------------------

const int   SEA_ITER_GEOMETRY = 3;
const int   SEA_ITER_FRAGMENT = 5;

const float SEA_EPSILON	= 1e-3;
#define       SEA_EPSILON_NRM	(0.1 / iResolution.x)
const float SEA_HEIGHT = 0.6;
const float SEA_CHOPPY = 4.0;
const float SEA_SPEED = 0.8;
const float SEA_FREQ = 0.16;
const vec3  SEA_BASE = vec3(0.1,0.19,0.22);
const vec3  SEA_WATER_COLOR = vec3(0.8,0.9,0.6);
float       SEA_TIME;
const mat2  sea_octave_m = mat2(1.6,1.2,-1.2,1.6);

float seaOctave( in vec2 uv, const in float choppy) {
    uv += noise(uv);        
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

float seaMap(const in vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < SEA_ITER_GEOMETRY; i++) {        
    	d = seaOctave((uv+SEA_TIME)*freq,choppy);
    	d += seaOctave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
    	uv *= sea_octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

float seaMapHigh(const in vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < SEA_ITER_FRAGMENT; i++) {        
    	d = seaOctave((uv+SEA_TIME)*freq,choppy);
    	d += seaOctave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
    	uv *= sea_octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

vec3 seaGetColor( const in vec3 n, vec3 eye, const in vec3 l, const in float att, 
                  const in vec3 sunc, const in vec3 upc, const in vec3 reflected) {  
    vec3 refracted = SEA_BASE * upc + diffuse(n,l) * SEA_WATER_COLOR * 0.12 * sunc; 
    vec3 color = mix(refracted,reflected,fresnel(n, -eye, 3.)*.65 );
    
    color += upc*SEA_WATER_COLOR * (att * 0.18);
    color += sunc * vec3(specular(n,l,eye,60.0));
    
    return color;
}

vec3 seaGetNormal(const in vec3 p, const in float eps) {
    vec3 n;
    n.y = seaMapHigh(p);    
    n.x = seaMapHigh(vec3(p.x+eps,p.y,p.z)) - n.y;
    n.z = seaMapHigh(vec3(p.x,p.y,p.z+eps)) - n.y;
    n.y = eps;
    return normalize(n);
}

float seaHeightMapTracing(const in vec3 ori, const in vec3 dir, out vec3 p) {  
    float tm = 0.0;
    float tx = 1000.0;    
    float hx = seaMap(ori + dir * tx);
    if(hx > 0.0) return tx;   
    float hm = seaMap(ori + dir * tm);    
    float tmid = 0.0;
    for(int i = 0; i < SEA_NUM_STEPS; i++) {
        tmid = mix(tm,tx, hm/(hm-hx));                   
        p = ori + dir * tmid;                   
    	float hmid = seaMap(p);
		if(hmid < 0.0) {
        	tx = tmid;
            hx = hmid;
        } else {
            tm = tmid;
            hm = hmid;
        }
    }
    return tmid;
}

vec3 seaTransform( in vec3 x ) {
    x.yz = rotate( 0.8, x.yz );
    return x;
}

vec3 seaUntransform( in vec3 x ) {
    x.yz = rotate( -0.8, x.yz );
    return x;
}

void renderSea( const in vec3 ro, const in vec3 rd, inout vec3 n, inout float att ) {    
    vec3 p,
    rom = seaTransform(ro),
    rdm = seaTransform(rd);
    
    rom.y -= EARTH_RADIUS;
    rom *= 1000.;
    rom.xz += vec2(3.1,.2)*time;

    SEA_TIME = time * SEA_SPEED;
    
    seaHeightMapTracing(rom,rdm,p);
    float squareddist = dot(p - rom, p-rom );
    n = seaGetNormal(p, squareddist * SEA_EPSILON_NRM );
    
    n = seaUntransform(n);
    
    att = clamp(SEA_HEIGHT+p.y, 0.,1.);
}

//-----------------------------------------------------
// Terrain based on Elevated and Terrain Tubes by IQ
//
// https://www.shadertoy.com/view/MdX3Rr
// https://www.shadertoy.com/view/4sjXzG
//-----------------------------------------------------

#ifndef HIDE_TERRAIN

const mat2 terrainM2 = mat2(1.6,-1.2,1.2,1.6);

float terrainLow( vec2 p ) {
    p *= 0.0013;

    float s = 1.0;
	float t = 0.0;
	for( int i=0; i<2; i++ ) {
        t += s*tri( p );
		s *= 0.5 + 0.1*t;
        p = 0.97*terrainM2*p + (t-0.5)*0.12;
	}
	return t*33.0;
}

float terrainMed( vec2 p ) {
    p *= 0.0013;

    float s = 1.0;
	float t = 0.0;
	for( int i=0; i<6; i++ ) {
        t += s*tri( p );
		s *= 0.5 + 0.1*t;
        p = 0.97*terrainM2*p + (t-0.5)*0.12;
	}
            
    return t*33.0;
}

float terrainHigh( vec2 p ) {
    vec2 q = p;
    p *= 0.0013;

    float s = 1.0;
	float t = 0.0;
	for( int i=0; i<7; i++ ) {
        t += s*tri( p );
		s *= 0.5 + 0.1*t;
        p = 0.97*terrainM2*p + (t-0.5)*0.12;
	}
    
    t += t*0.015*fbm( q );
	return t*33.0;
}

float terrainMap( const in vec3 pos ) {
	return pos.y - terrainMed(pos.xz);  
}

float terrainMapH( const in vec3 pos ) {
    float y = terrainHigh(pos.xz);
    float h = pos.y - y;
    return h;
}

float terrainIntersect( in vec3 ro, in vec3 rd, in float tmin, in float tmax ) {
    float t = tmin;
	for( int i=0; i<TERRAIN_NUM_STEPS; i++ ) {
        vec3 pos = ro + t*rd;
        float res = terrainMap( pos );
        if( res<(0.001*t) || t>tmax  ) break;
        t += res*.9;
	}

	return t;
}

float terrainCalcShadow(in vec3 ro, in vec3 rd ) {
	vec2  eps = vec2(150.0,0.0);
    float h1 = terrainMed( ro.xz );
    float h2 = terrainLow( ro.xz );
    
    float d1 = 10.0;
    float d2 = 80.0;
    float d3 = 200.0;
    float s1 = clamp( 1.0*(h1 + rd.y*d1 - terrainMed(ro.xz + d1*rd.xz)), 0.0, 1.0 );
    float s2 = clamp( 0.5*(h1 + rd.y*d2 - terrainMed(ro.xz + d2*rd.xz)), 0.0, 1.0 );
    float s3 = clamp( 0.2*(h2 + rd.y*d3 - terrainLow(ro.xz + d3*rd.xz)), 0.0, 1.0 );

    return min(min(s1,s2),s3);
}
vec3 terrainCalcNormalHigh( in vec3 pos, float t ) {
    vec2 e = vec2(1.0,-1.0)*0.001*t;

    return normalize( e.xyy*terrainMapH( pos + e.xyy ) + 
					  e.yyx*terrainMapH( pos + e.yyx ) + 
					  e.yxy*terrainMapH( pos + e.yxy ) + 
					  e.xxx*terrainMapH( pos + e.xxx ) );
}

vec3 terrainCalcNormalMed( in vec3 pos, float t ) {
	float e = 0.005*t;
    vec2  eps = vec2(e,0.0);
    float h = terrainMed( pos.xz );
    return normalize(vec3( terrainMed(pos.xz-eps.xy)-h, e, terrainMed(pos.xz-eps.yx)-h ));
}

vec3 terrainTransform( in vec3 x ) {
    x.zy = rotate( -.83, x.zy );
    return x;
}

vec3 terrainUntransform( in vec3 x ) {
    x.zy = rotate( .83, x.zy );
    return x;
}


float llamelTime;
const float llamelScale = 5.;

vec3 llamelPosition() {
    llamelTime = time*2.5;
    vec2 pos = vec2( -400., 135.-llamelTime*0.075* llamelScale);
    return vec3( pos.x, terrainMed( pos ), pos.y );
}

vec3 terrainShade( const in vec3 col, const in vec3 pos, const in vec3 rd, const in vec3 n, const in float spec, 
                   const in vec3 sunc, const in vec3 upc, const in vec3 reflc ) {
	vec3 sunDirection =  terrainTransform(SUN_DIRECTION);
    float dif = diffuse( n, sunDirection );
    float bac = diffuse( n, vec3(-sunDirection.x, sunDirection.y, -sunDirection.z) );
    float sha = terrainCalcShadow( pos, sunDirection );
    float amb = clamp( n.y,0.0,1.0);
        
    vec3 lin  = vec3(0.0);
    lin += 2.*dif*sunc*vec3( sha, sha*sha*0.1+0.9*sha, sha*sha*0.2+0.8*sha );
    lin += 0.2*amb*upc;
    lin += 0.08*bac*clamp(vec3(1.)-sunc, vec3(0.), vec3(1.));
    return mix( col*lin*3., reflc, spec*fresnel(n,-terrainTransform(rd),5.0) );
}

vec3 terrainGetColor( const in vec3 pos, const in vec3 rd, const in float t, const in vec3 sunc, const in vec3 upc, const in vec3 reflc ) {
    vec3 nor = terrainCalcNormalHigh( pos, t );
    vec3 sor = terrainCalcNormalMed( pos, t );
        
    float spec = 0.005;

#ifdef DISPLAY_TERRAIN_DETAIL
    float no = noise(5.*fbm(1.11*pos.xz));
#else
    const float no = 0.;
#endif
    float r = .5+.5*fbm(.95*pos.xz);
	vec3 col = (r*0.25+0.75)*0.9*mix( vec3(0.08,0.07,0.07), vec3(0.10,0.09,0.08), noise(0.4267*vec2(pos.x*2.,pos.y*9.8))+.01*no );
    col = mix( col, 0.20*vec3(0.45,.30,0.15)*(0.50+0.50*r),smoothstep(0.825,0.925,nor.y+.025*no) );
	col = mix( col, 0.15*vec3(0.30,.30,0.10)*(0.25+0.75*r),smoothstep(0.95,1.0,nor.y+.025*no) );
    col *= .88+.12*no;
        
    float s = nor.y + 0.03*pos.y + 0.35*fbm(0.05*pos.xz) - .35;
    float sf = fwidth(s) * 1.5;
    s = smoothstep(0.84-sf, 0.84+sf, s );
    col = mix( col, 0.29*vec3(0.62,0.65,0.7), s);
    nor = mix( nor, sor, 0.7*smoothstep(0.9, 0.95, s ) );
    spec = mix( spec, 0.45, smoothstep(0.9, 0.95, s ) );

   	col = terrainShade( col, pos, rd, nor, spec, sunc, upc, reflc );

#ifdef DISPLAY_LLAMEL
    col *= clamp( distance(pos.xz, llamelPosition().xz )*0.4, 0.4, 1.);
#endif
    
    return col;
}

vec3 terrainTransformRo( const in vec3 ro ) {
    vec3 rom = terrainTransform(ro);
    rom.y -= EARTH_RADIUS - 100.;
    rom.xz *= 5.;
    rom.xz += vec2(-170.,50.)+vec2(-4.,.4)*time;    
    rom.y += (terrainLow( rom.xz ) - 86.)*clamp( 1.-1.*(length(ro)-EARTH_RADIUS), 0., 1.);
    return rom;
}

vec4 renderTerrain( const in vec3 ro, const in vec3 rd, inout vec3 intersection, inout vec3 n ) {    
    vec3 p,
    rom = terrainTransformRo(ro),
    rdm = terrainTransform(rd);
        
    float tmin = 10.0;
    float tmax = 3200.0;
    
    float res = terrainIntersect( rom, rdm, tmin, tmax );
    
    if( res > tmax ) {
        res = -1.;
    } else {
        vec3 pos =  rom+rdm*res;
        n = terrainCalcNormalMed( pos, res );
        n = terrainUntransform( n );
        
        intersection = ro+rd*res/100.;
    }
    return vec4(res, rom+rdm*res);
}

#endif

//-----------------------------------------------------
// LLamels by Eiffie
//
// https://www.shadertoy.com/view/ltsGz4
//-----------------------------------------------------
#ifdef DISPLAY_LLAMEL
float llamelMapSMin(const in float a,const in float b,const in float k){
    float h=clamp(0.5+0.5*(b-a)/k,0.0,1.0);return b+h*(a-b-k+k*h);
}

float llamelMapLeg(vec3 p, vec3 j0, vec3 j3, vec3 l, vec4 r, vec3 rt){//z joint with tapered legs
	float lx2z=l.x/(l.x+l.z),h=l.y*lx2z;
	vec3 u=(j3-j0)*lx2z,q=u*(0.5+0.5*(l.x*l.x-h*h)/dot(u,u));
	q+=sqrt(max(0.0,l.x*l.x-dot(q,q)))*normalize(cross(u,rt));
	vec3 j1=j0+q,j2=j3-q*(1.0-lx2z)/lx2z;
	u=p-j0;q=j1-j0;
	h=clamp(dot(u,q)/dot(q,q),0.0,1.0);
	float d=length(u-q*h)-r.x-(r.y-r.x)*h;
	u=p-j1;q=j2-j1;
	h=clamp(dot(u,q)/dot(q,q),0.0,1.0);
	d=min(d,length(u-q*h)-r.y-(r.z-r.y)*h);
	u=p-j2;q=j3-j2;
	h=clamp(dot(u,q)/dot(q,q),0.0,1.0);
	return min(d,length(u-q*h)-r.z-(r.w-r.z)*h);
}

float llamelMap(in vec3 p) {
	const vec3 rt=vec3(0.0,0.0,1.0);	
	p.y += 0.25*llamelScale;
    p.xz -= 0.5*llamelScale;
    p.xz = vec2(-p.z, p.x);
    vec3 pori = p;
        
    p /= llamelScale;
    
	vec2 c=floor(p.xz);
	p.xz=fract(p.xz)-vec2(0.5);
    p.y -= p.x*.04*llamelScale;
	float sa=sin(c.x*2.0+c.y*4.5+llamelTime*0.05)*0.15;

    float b=0.83-abs(p.z);
	float a=c.x+117.0*c.y+sign(p.x)*1.57+sign(p.z)*1.57+llamelTime,ca=cos(a);
	vec3 j0=vec3(sign(p.x)*0.125,ca*0.01,sign(p.z)*0.05),j3=vec3(j0.x+sin(a)*0.1,max(-0.25+ca*0.1,-0.25),j0.z);
	float dL=llamelMapLeg(p,j0,j3,vec3(0.08,0.075,0.12),vec4(0.03,0.02,0.015,0.01),rt*sign(p.x));
	p.y-=0.03;
	float dB=(length(p.xyz*vec3(1.0,1.75,1.75))-0.14)*0.75;
	a=c.x+117.0*c.y+llamelTime;ca=cos(a);sa*=0.4;
	j0=vec3(0.125,0.03+abs(ca)*0.03,ca*0.01),j3=vec3(0.3,0.07+ca*sa,sa);
	float dH=llamelMapLeg(p,j0,j3,vec3(0.075,0.075,0.06),vec4(0.03,0.035,0.03,0.01),rt);
	dB=llamelMapSMin(min(dL,dH),dB,clamp(0.04+p.y,0.0,1.0));
	a=max(abs(p.z),p.y)+0.05;
	return max(min(dB,min(a,b)),length(pori.xz-vec2(0.5)*llamelScale)-.5*llamelScale);
}

vec3 llamelGetNormal( in vec3 ro ) {
    vec2 e = vec2(1.0,-1.0)*0.001;

    return normalize( e.xyy*llamelMap( ro + e.xyy ) + 
					  e.yyx*llamelMap( ro + e.yyx ) + 
					  e.yxy*llamelMap( ro + e.yxy ) + 
					  e.xxx*llamelMap( ro + e.xxx ) );
}

vec4 renderLlamel( in vec3 ro, const in vec3 rd, const in vec3 sunc, const in vec3 upc, const in vec3 reflc ) {
    ro -= llamelPosition();
	float t=.1*hash(rd.xy),d,dm=10.0,tm;
	for(int i=0;i<36;i++){
		t+=d=llamelMap(ro+rd*t);
		if(d<dm){dm=d;tm=t;}
		if(t>1000.0 || d<0.00001)break;
	}
	dm=max(0.0,dm);
    if( dm < .02 ) {
        vec3 col = vec3(0.45,.30,0.15)*.2;
        vec3 pos = ro + rd*tm;
        vec3 nor = llamelGetNormal( pos );
        col = terrainShade( col, pos, rd, nor, .01, sunc, upc, reflc );        
        return vec4(col, clamp( 1.-(dm-0.01)/0.01,0., 1.) );
    }
    
    return vec4(0.);
}
#endif

//-----------------------------------------------------
// Clouds (by me ;))
//-----------------------------------------------------

vec4 renderClouds( const in vec3 ro, const in vec3 rd, const in float d, const in vec3 n, const in float land, 
                   const in vec3 sunColor, const in vec3 upColor, inout float shadow ) {
	vec3 intersection = ro+rd*d;
    vec3 cint = intersection*0.009;
    float rot = -.2*length(cint.xy) + .6*fbm( cint*.4,0.5,2.96 ) + .05*land;

    cint.xy = rotate( rot, cint.xy );

    vec3 cdetail = mod(intersection*3.23,vec3(50.));
    cdetail.xy = rotate( .25*rot, cdetail.xy );

    float clouds = 1.3*(fbm( cint*(1.+.02*noise(intersection)),0.5,2.96)+.4*land-.3);

#ifdef DISPLAY_CLOUDS_DETAIL
    if( d < 200. ) {
        clouds += .3*(fbm(cdetail,0.5,2.96)-.5)*(1.-smoothstep(0.,200.,d));
    }
#endif

    shadow = clamp(1.-clouds, 0., 1.);

    clouds = clamp(clouds, 0., 1.);
    clouds *= clouds;
    clouds *= smoothstep(0.,0.4,d);

    vec3 clbasecolor = vec3(1.);
    vec3 clcol = .1*clbasecolor*sunColor * vec3(specular(n,SUN_DIRECTION,rd,36.0));
    clcol += .3*clbasecolor*sunColor;
    clcol += clbasecolor*(diffuse(n,SUN_DIRECTION)*sunColor+upColor);  
    
    return vec4( clcol, clouds );
}

//-----------------------------------------------------
// Planet (by me ;))
//-----------------------------------------------------

vec4 renderPlanet( const in vec3 ro, const in vec3 rd, const in vec3 up, inout float maxd ) {
    float d = iSphere( ro, rd, vec4( 0., 0., 0., EARTH_RADIUS ) );

    vec3 intersection = ro + rd*d;
    vec3 n = nSphere( intersection, vec4( 0., 0., 0., EARTH_RADIUS ) );
    vec4 res;

#ifndef HIDE_TERRAIN
    bool renderTerrainDetail = length(ro) < EARTH_RADIUS+EARTH_ATMOSPHERE && 
        					   dot( terrainUntransform( vec3(0.,1.,0.) ), normalize(ro) ) > .9996;
#endif
    bool renderSeaDetail     = d < 1. && dot( seaUntransform( vec3(0.,1.,0.) ), normalize(ro) ) > .9999; 
    float mixDetailColor = 0.;
        
	if( d < 0. || d > maxd) {
#ifndef HIDE_TERRAIN
        if( renderTerrainDetail ) {
       		intersection = ro;
            n = normalize( ro );
        } else { 	       
	        return vec4(0);
        }
#else 
      	return vec4(0.);
#endif
	}
    if( d > 0. ) {
	    maxd = d;
    }
    float att = 0.;
    
    if( dot(n,SUN_DIRECTION) < -0.1 ) return vec4( 0., 0., 0., 1. );
    
    float dm = MAX, e = 0.;
    vec3 col, detailCol, nDetail;
    
    // normal and intersection 
#ifndef HIDE_TERRAIN
    if( renderTerrainDetail ) {   
        res = renderTerrain( ro, rd, intersection, nDetail );
        if( res.x < 0. && d < 0. ) {
	        return vec4(0);
        }
        if( res.x >= 0. ) {
            maxd = pow(res.x/4000.,4.)*50.;
            e = -10.;
        }
        mixDetailColor = 1.-smoothstep(.75, 1., (length(ro)-EARTH_RADIUS) / EARTH_ATMOSPHERE);
        n = normalize( mix( n, nDetail, mixDetailColor ) );
    } else 
#endif        
    if( renderSeaDetail ) {    
        float attsea, mf = smoothstep(.5,1.,d);

        renderSea( ro, rd, nDetail, attsea );

        n = normalize(mix( nDetail, n, mf ));
        att = mix( attsea, att, mf );
    } else {
        e = fbm( .003*intersection+vec3(1.),0.4,2.96) + smoothstep(.85,.95, abs(intersection.z/EARTH_RADIUS));
#ifndef HIDE_TERRAIN
        if( d < 1500. ) {
            e += (-.03+.06* fbm( intersection*0.1,0.4,2.96))*(1.-d/1500.);
        }
#endif  
    }
    
    vec3 sunColor = .25*renderAtmosphericLow( intersection, SUN_DIRECTION).xyz;  
    vec3 upColor = 2.*renderAtmosphericLow( intersection, n).xyz;  
    vec3 reflColor = renderAtmosphericLow( intersection, reflect(rd,n)).xyz; 
                 
    // color  
#ifndef HIDE_TERRAIN
    if(renderTerrainDetail ) {
        detailCol = col =  terrainGetColor(res.yzw, rd, res.x, sunColor, upColor, reflColor);
		d = 0.;
    }   
#endif
     
    if( mixDetailColor < 1. ) {
        if( e < .45 ) {
            // sea
            col = seaGetColor(n,rd,SUN_DIRECTION, att, sunColor, upColor, reflColor);    
        } else {
            // planet (land) far
            float land1 = max(0.1, fbm( intersection*0.0013,0.4,2.96) );
            float land2 = max(0.1, fbm( intersection*0.0063,0.4,2.96) );
            float iceFactor = abs(pow(intersection.z/EARTH_RADIUS,13.0))*e;

            vec3 landColor1 = vec3(0.43,0.65,0.1) * land1;
            vec3 landColor2 = RING_COLOR_1 * land2;
            vec3 mixedLand = (landColor1 + landColor2)* 0.5;
            vec3 finalLand = mix(mixedLand, vec3(7.0, 7.0, 7.0) * land1 * 1.5, max(iceFactor+.02*land2-.02, 0.));

            col = (diffuse(n,SUN_DIRECTION)*sunColor+upColor)*finalLand*.75;
#ifdef HIGH_QUALITY
            col *= (.5+.5*fbm( intersection*0.23,0.4,2.96) );
#endif
        }
    }
    
    if( mixDetailColor > 0. ) {
        col = mix( col, detailCol, mixDetailColor );
    }
        
#ifdef DISPLAY_LLAMEL
    if(renderTerrainDetail ) {
        vec3 rom = terrainTransformRo(ro),
        rdm = terrainTransform(rd);
        d = iSphere( rom, rdm, vec4( llamelPosition(), llamelScale*3. ) );
        if( d > 0. ) {
            vec4 llamel = renderLlamel( rom+rdm*d, rdm, sunColor, upColor, reflColor );
            col = mix(col, llamel.rgb, llamel.a);
        }
    }
#endif
    
    d = iSphere( ro, rd, vec4( 0., 0., 0., EARTH_RADIUS+EARTH_CLOUDS ) );
    if( d > 0. ) { 
        float shadow;
		vec4 clouds = renderClouds( ro, rd, d, n, e, sunColor, upColor, shadow);
        col *= shadow; 
        col = mix( col, clouds.rgb, clouds.w );
    }
    
    float m = MAX;
    col *= (1. - renderRingFarShadow( ro+rd*d, SUN_DIRECTION ) );

 	return vec4( col, 1. ); 
}

//-----------------------------------------------------
// Lens flare by musk
//
// https://www.shadertoy.com/view/4sX3Rs
//-----------------------------------------------------

vec3 lensFlare( const in vec2 uv, const in vec2 pos) {
	vec2 main = uv-pos;
	vec2 uvd = uv*(length(uv));
	
	float f0 = 1.5/(length(uv-pos)*16.0+1.0);
	
	float f1 = max(0.01-pow(length(uv+1.2*pos),1.9),.0)*7.0;

	float f2 = max(1.0/(1.0+32.0*pow(length(uvd+0.8*pos),2.0)),.0)*00.25;
	float f22 = max(1.0/(1.0+32.0*pow(length(uvd+0.85*pos),2.0)),.0)*00.23;
	float f23 = max(1.0/(1.0+32.0*pow(length(uvd+0.9*pos),2.0)),.0)*00.21;
	
	vec2 uvx = mix(uv,uvd,-0.5);
	
	float f4 = max(0.01-pow(length(uvx+0.4*pos),2.4),.0)*6.0;
	float f42 = max(0.01-pow(length(uvx+0.45*pos),2.4),.0)*5.0;
	float f43 = max(0.01-pow(length(uvx+0.5*pos),2.4),.0)*3.0;
	
	vec3 c = vec3(.0);
	
	c.r+=f2+f4; c.g+=f22+f42; c.b+=f23+f43;
	c = c*.5 - vec3(length(uvd)*.05);
	c+=vec3(f0);
	
	return c;
}

//-----------------------------------------------------
// cameraPath
//-----------------------------------------------------

vec3 pro, pta, pup;
float dro, dta, dup;

void camint( inout vec3 ret, const in float t, const in float duration, const in vec3 dest, inout vec3 prev, inout float prevt ) {
    if( t >= prevt && t <= prevt+duration ) {
    	ret = mix( prev, dest, smoothstep(prevt, prevt+duration, t) );
    }
    prev = dest;
    prevt += duration;
}

void cameraPath( in float t, out vec3 ro, out vec3 ta, out vec3 up ) {
#ifndef HIDE_TERRAIN
    time = t = mod( t, 92. );
#else
    time = t = mod( t, 66. );
#endif
    dro = dta = dup = 0.;

    pro = ro = vec3(900. ,7000. ,1500. );
    pta = ta = vec3(    0. ,    0. ,   0. );
    pup = up = vec3(    0. ,    0.4,   1. ); 
   
    camint( ro, t, 5., vec3(-4300. ,-1000. , 500. ), pro, dro );
    camint( ta, t, 5., vec3(    0. ,    0. ,   0. ), pta, dta );
    camint( up, t, 7., vec3(    0. ,    0.1,   1. ), pup, dup ); 
 
    camint( ro, t, 3., vec3(-1355. , 1795. , 1.2 ), pro, dro );
    camint( ta, t, 1., vec3(    0. , 300. ,-600. ), pta, dta );
    camint( up, t, 6., vec3(    0. ,  0.1,    1. ), pup, dup );

    camint( ro, t, 10., vec3(-1355. , 1795. , 1.2 ), pro, dro );
    camint( ta, t, 14., vec3(    0. , 100. ,   600. ), pta, dta );
    camint( up, t, 13., vec3(    0. ,  0.3,    1. ), pup, dup );
    
    vec3 roe = seaUntransform( vec3( 0., EARTH_RADIUS+0.004, 0. ) );
    vec3 upe = seaUntransform( vec3( 0., 1., 0. ) );
    
    camint( ro, t, 7.,roe, pro, dro );
    camint( ta, t, 7., vec3( EARTH_RADIUS + 0., EARTH_RADIUS - 500., 500. ), pta, dta );
    camint( up, t, 6., upe, pup, dup );
        
    camint( ro, t, 17.,roe, pro, dro );
    camint( ta, t, 17., vec3( EARTH_RADIUS + 500., EARTH_RADIUS + 1300., -100. ), pta, dta );
    camint( up, t, 18., vec3(.0,1.,1.), pup, dup );
    
    camint( ro, t, 11., vec3(  3102. ,  0. , 1450. ), pro, dro );
    camint( ta, t, 4., vec3(    0. ,   -100. ,   0. ), pta, dta );
    camint( up, t, 8., vec3(    0. ,    0.15,   1. ), pup, dup ); 
#ifndef HIDE_TERRAIN    
    roe = terrainUntransform( vec3( 0., EARTH_RADIUS+0.004, 0. ) );
    upe = terrainUntransform( vec3( 0., 1., 0. ) );
    
    camint( ro, t, 7., roe, pro, dro );
    camint( ta, t, 12., vec3( -EARTH_RADIUS, EARTH_RADIUS+200., 100.), pta, dta );
    camint( up, t, 2., upe, pup, dup );
        
    roe = terrainUntransform( vec3( 0., EARTH_RADIUS+0.001, 0. ) );
    camint( ro, t, 17.,roe, pro, dro );
    camint( ta, t, 18., roe + vec3( 5000., EARTH_RADIUS-100., -2000.), pta, dta );
    camint( up, t, 18., vec3(.0,1.,1.), pup, dup );
        
    roe = terrainUntransform( vec3( 0., EARTH_RADIUS+1.8, 0. ) );
    camint( ro, t, 4.,roe, pro, dro );
    camint( ta, t, 4.5, roe + vec3( EARTH_RADIUS, EARTH_RADIUS+2000., -30.), pta, dta );
    camint( up, t, 4., vec3(.0,1.,1.), pup, dup );
#endif    
    camint( ro, t, 10., vec3(900. ,7000. , 1500. ), pro, dro );
    camint( ta, t, 2., vec3(    0. ,    0. ,   0. ), pta, dta );
    camint( up, t, 10., vec3(    0. ,    0.4,   1. ), pup, dup ); 
    
    up = normalize( up );
}

//-----------------------------------------------------
// mainImage
//-----------------------------------------------------

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
	vec2 uv = fragCoord.xy / iResolution.xy;
    
    vec2 p = -1.0 + 2.0 * (fragCoord.xy) / iResolution.xy;
    p.x *= iResolution.x/iResolution.y;
    
    vec3 col;
    
// black bands
    vec2 bandy = vec2(.1,.9);
    if( uv.y < bandy.x || uv.y > bandy.y ) {
        col = vec3(0.);
    } else {
        // camera
        vec3 ro, ta, up;
        cameraPath( iGlobalTime*.7, ro, ta, up );

        vec3 ww = normalize( ta - ro );
        vec3 uu = normalize( cross(ww,up) );
        vec3 vv = normalize( cross(uu,ww));
        vec3 rd = normalize( -p.x*uu + p.y*vv + 2.2*ww );

        float maxd = MAX;  
        col = renderStars( rd ).xyz;

        vec4 planet = renderPlanet( ro, rd, up, maxd );       
        if( planet.w > 0. ) col.xyz = planet.xyz;

        float atmosphered = maxd;
        vec4 atmosphere = .85*renderAtmospheric( ro, rd, atmosphered );
        col = col * (1.-atmosphere.w ) + atmosphere.xyz; 

        vec4 ring = renderRing( ro, rd, maxd );
        if( ring.w > 0. && atmosphered < maxd ) {
           ring.xyz = ring.xyz * (1.-atmosphere.w ) + atmosphere.xyz; 
        }
        col = col * (1.-ring.w ) + ring.xyz;

#ifdef DISPLAY_CLOUDS
        float lro = length(ro);
        if( lro < EARTH_RADIUS+EARTH_CLOUDS*1.25 ) {
            vec3 sunColor = 2.*renderAtmosphericLow( ro, SUN_DIRECTION);  
            vec3 upColor = 4.*renderAtmosphericLow( ro, vec3(-SUN_DIRECTION.x, SUN_DIRECTION.y, -SUN_DIRECTION.z));  

            if( lro < EARTH_RADIUS+EARTH_CLOUDS ) {
                // clouds
                float d = iCSphereF( ro, rd, EARTH_RADIUS + EARTH_CLOUDS );
                if( d < maxd ) {
                    float shadow;
                    vec4 clouds = renderClouds( ro, rd, d, normalize(ro), 0., sunColor, upColor, shadow );
                    clouds.w *= 1.-smoothstep(0.8*EARTH_CLOUDS,EARTH_CLOUDS,lro-EARTH_RADIUS);
                    col = mix(col, clouds.rgb, clouds.w * (1.-smoothstep( 10., 30., d)) );
                }
            }
            float offset = lro-EARTH_RADIUS-EARTH_CLOUDS;
            col = mix( col, .5*sunColor, .15*abs(noise(offset*100.))*clamp(1.-4.*abs(offset)/EARTH_CLOUDS, 0., 1.) );
        }
#endif 

        // post processing
        col = pow( clamp(col,0.0,1.0), vec3(0.4545) );
        col *= vec3(1.,0.99,0.95);   
        col = clamp(1.06*col-0.03, 0., 1.);      

        vec2 sunuv =  2.7*vec2( dot( SUN_DIRECTION, -uu ), dot( SUN_DIRECTION, vv ) );
        float flare = dot( SUN_DIRECTION, normalize(ta-ro) );
        col += vec3(1.4,1.2,1.0)*lensFlare(p, sunuv)*clamp( flare+.3, 0., 1.);

        uv.y = (uv.y-bandy.x)*(1./(bandy.y-bandy.x));
        col *= 0.5 + 0.5*pow( 16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.1 ); 
    }
    fragColor = vec4( col ,1.0);
}
