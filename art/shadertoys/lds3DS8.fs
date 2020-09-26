// Created by Reinder Nijhoff 2013
// @reindernijhoff

// empirical measured values
#define EYEDISTANCE  5.15 
#define LENSDISTANCE -0.136 
#define FOV 0.62  

#define SHADOW
//#define REFLECTION

#define RAYCASTSTEPS 30

#define EXPOSURE 0.9
#define EPSILON 0.0001
#define MAXDISTANCE 400.
#define GRIDSIZE 10.
#define GRIDSIZESMALL 8.
#define MAXHEIGHT 10.
#define SPEED 2.5

float time;

//
// math functions
//

const mat2 mr = mat2 (0.84147,  0.54030,
					  0.54030, -0.84147 );
float hash( float n ) {
	return fract(sin(n)*43758.5453);
}
vec2 hash2( float n ) {
	return fract(sin(vec2(n,n+1.0))*vec2(2.1459123,3.3490423));
}
vec2 hash2( vec2 n ) {
	return fract(sin(vec2( n.x*n.y, n.x+n.y))*vec2(2.1459123,3.3490423));
}
vec3 hash3( float n ) {
	return fract(sin(vec3(n,n+1.0,n+2.0))*vec3(3.5453123,4.1459123,1.3490423));
}
vec3 hash3( vec2 n ) {
	return fract(sin(vec3(n.x, n.y, n+2.0))*vec3(3.5453123,4.1459123,1.3490423));
}
//
// intersection functions
//

bool intersectPlane(vec3 ro, vec3 rd, float height, out float dist) {	
	if (rd.y==0.0) {
		return false;
	}
	
	float d = -(ro.y - height)/rd.y;
	d = min(100000.0, d);
	if( d > 0. ) {
		dist = d;
		return true;
	}
	return false;
}

bool intersectUnitSphere ( in vec3 ro, in vec3 rd, in vec3 sph, out float dist, out vec3 normal ) {
	vec3  ds = ro - sph;
	float bs = dot( rd, ds );
	float cs = dot(  ds, ds ) - 1.0;
	float ts = bs*bs - cs;
	
	if( ts > 0.0 ) {
		ts = -bs - sqrt( ts );
		if( ts>0. ) {
			normal = normalize( (ro+ts*rd)-sph );
			dist = ts;
			return true;
		}
	}
	
	return false;
}

//
// Scene
//

void getSphereOffset( vec2 grid, inout vec2 center ) {
	center = (hash2( grid+vec2(43.12,1.23) ) - vec2(0.5) )*(GRIDSIZESMALL);
}
void getMovingSpherePosition( vec2 grid, vec2 sphereOffset, inout vec3 center ) {
	// falling?
	float s = 0.1+hash( grid.x*1.23114+5.342+754.324231*grid.y );
	float t = 14.*s + time/s;
	
	float y =  s * MAXHEIGHT * abs( cos( t ) );
	vec2 offset = grid + sphereOffset;
	
	center = vec3( offset.x, y, offset.y ) + 0.5*vec3( GRIDSIZE, 2., GRIDSIZE );
}
void getSpherePosition( vec2 grid, vec2 sphereOffset, inout vec3 center ) {
	vec2 offset = grid + sphereOffset;
	center = vec3( offset.x, 0., offset.y ) + 0.5*vec3( GRIDSIZE, 2., GRIDSIZE );
}
vec3 getSphereColor( vec2 grid ) {
	return normalize( hash3( grid+vec2(43.12*grid.y,12.23*grid.x) ) );
}

vec3 trace(vec3 ro, vec3 rd, out vec3 intersection, out vec3 normal, out float dist, out int material) {
	material = 0; // sky
	dist = MAXDISTANCE;
	float distcheck;
	
	vec3 sphereCenter, col, normalcheck;
	
	if( intersectPlane( ro,  rd, 0., distcheck) && distcheck < MAXDISTANCE ) {
		dist = distcheck;
		material = 1;
		normal = vec3( 0., 1., 0. );
		col = vec3( 1. );
	} else {
		col = vec3( 0. );
	}
	
	// trace grid
	vec2 map = floor( ro.xz / GRIDSIZE ) * GRIDSIZE;
	float deltaDistX = GRIDSIZE*sqrt(1. + (rd.z * rd.z) / (rd.x * rd.x));
	float deltaDistY = GRIDSIZE*sqrt(1. + (rd.x * rd.x) / (rd.z * rd.z));
	float stepX, stepY, sideDistX, sideDistY;
	
	//calculate step and initial sideDist
	if (rd.x < 0.) {
		stepX = -GRIDSIZE;
		sideDistX = (ro.x - map.x) * deltaDistX / GRIDSIZE;
	} else {
		stepX = GRIDSIZE;
		sideDistX = (map.x + GRIDSIZE - ro.x) * deltaDistX / GRIDSIZE;
	}
	if (rd.z < 0.) {
		stepY = -GRIDSIZE;
		sideDistY = (ro.z - map.y) * deltaDistY / GRIDSIZE;
	} else {
		stepY = GRIDSIZE;
		sideDistY = (map.y + GRIDSIZE - ro.z) * deltaDistY / GRIDSIZE;
	}
	
	bool hit = false; 
	
	for( int i=0; i<RAYCASTSTEPS; i++ ) {
		if( hit || distance( ro.xz, map ) > dist+GRIDSIZE ) continue;

		vec2 offset;
		getSphereOffset( map, offset );
		
		getMovingSpherePosition( map, -offset, sphereCenter );
		
		if( intersectUnitSphere( ro, rd, sphereCenter, distcheck, normalcheck ) && distcheck < dist ) {
			dist = distcheck;
			normal = normalcheck;
			material = 2;
			hit = true;
		}
		
		getSpherePosition( map, offset, sphereCenter );
		if( intersectUnitSphere( ro, rd, sphereCenter, distcheck, normalcheck ) && distcheck < dist ) {
			dist = distcheck;
			normal = normalcheck;
			col = vec3( 2. );
			material = 3;
			hit = true;
		}
						
		if (sideDistX < sideDistY) {
			sideDistX += deltaDistX;
			map.x += stepX;
		} else {
			sideDistY += deltaDistY;
			map.y += stepY;
		}		
	}
	
	vec3 color = vec3( 0. );
	if( (hit || material == 1) ) {
		intersection = ro + rd*dist;
		vec2 map = intersection.xz - mod( intersection.xz, vec2(GRIDSIZE,GRIDSIZE) );
		
		if( material == 1 || material == 3 ) {
			// lightning
			vec3 c = vec3( -GRIDSIZE,0., GRIDSIZE );
			for( int x=0; x<3; x++ ) {
				for( int y=0; y<3; y++ ) {
					vec2 mapoffset = map+vec2( c[x], c[y] );		
					vec2 offset;
					getSphereOffset( mapoffset, offset );
					vec3 lcolor = getSphereColor( mapoffset );
					vec3 lpos;
					getMovingSpherePosition( mapoffset, -offset, lpos );
					
					float shadow = 1.;
#ifdef SHADOW
					if( material == 1 ) {
						for( int sx=0; sx<3; sx++ ) {
							for( int sy=0; sy<3; sy++ ) {
								if( shadow < 1. ) continue;
								
								vec2 smapoffset = map+vec2( c[sx], c[sy] );		
								vec2 soffset;
								getSphereOffset( smapoffset, soffset );
								vec3 slpos, sn;
								getSpherePosition( smapoffset, soffset, slpos );
								float sd;
								if( intersectUnitSphere( intersection, normalize( lpos - intersection ), slpos, sd, sn )  ) {
									shadow = 0.;
								}							
							}
						}
					}
#endif
					color += col * lcolor * ( shadow * max( dot( normalize(lpos-intersection), normal ), 0.) *
											 (1. - clamp( distance( lpos, intersection )/GRIDSIZE, 0., 1.) ) );
				}
			}
		} else {
			// emitter
			color = (1.5+dot(normal, vec3( 0.5, 0.5, -0.5) )) *getSphereColor( map );
		}
	}
	return color;
}

// left
float w = 1.0;
float h = 1.0;
float scaleFactor = 1.0;

vec2 leftLensCenter = vec2( LENSDISTANCE, 0. );
vec2 rightLensCenter = vec2( -LENSDISTANCE, 0. );

vec2 Scale;
vec2 ScaleIn = vec2( 1., 1.);
vec4 HmdWarpParam = vec4(1., 0.22, 0.24, 0);

vec2 HmdWarp(vec2 in01, vec2 lensCenter) {
   vec2 theta = (in01-lensCenter) * ScaleIn; // Scales to [-1, 1]
   float rSq = dot(theta, theta);
   vec2 rvector = theta * 
	 (HmdWarpParam.x + HmdWarpParam.y * rSq +
      HmdWarpParam.z * rSq * rSq +
      HmdWarpParam.w * rSq * rSq * rSq);
   return lensCenter + Scale * rvector;
}





void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    Scale = 0.65*vec2( 1., iResolution.x/iResolution.y );
    time = iGlobalTime;
	vec2 q = fragCoord.xy/iResolution.xy;
	vec2 p = 2.0*q;
	
	p.x *= 2.;
		
	bool lefteye = true;
	if( p.x > 2. ) {
		p.x -= 2.;
		lefteye = false;
	}
	p -= vec2(1.);
		
	p = HmdWarp( p, lefteye?leftLensCenter:rightLensCenter );


	p.x *= iResolution.x/iResolution.y;
	
	// camera	
	vec3 ce = vec3( cos( 0.232*time) * 10., 7.+3.*cos(0.3*time), GRIDSIZE*(time/SPEED) );
	vec3 ro = ce;
	vec3 ta = ro + vec3( -sin( 0.232*time) * 10., -2.0+cos(0.23*time), 10.0 );
	
	float roll = -0.15*sin(0.5*time);
	
	// camera tx
	vec3 cw = normalize( ta-ro );
	vec3 cp = vec3( sin(roll), cos(roll),0.0 );
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );

	vec3 go = vec3( 0.0 );
	go.x = (lefteye?-0.5*EYEDISTANCE:0.5*EYEDISTANCE);
	ro += go.x*cu + go.y*cv;

	// create offset voor left or right eye
	vec3 er = normalize( vec3( p.xy, FOV ) );
	vec3 rd = er.x*cu + er.y*cv + er.z*cw;
	
	
	
	// raytrace
	int material;
	vec3 normal, intersection;
	float dist;
	
	vec3 col = trace(ro, rd, intersection, normal, dist, material);

#ifdef REFLECTION
	if( material > 0 ) {
		vec3 ro = intersection + EPSILON*normal;
		rd = reflect( rd, normal );
		col += 0.05 * trace(ro, rd, intersection, normal, dist, material);
	}
#endif
	
	col = pow( col, vec3(EXPOSURE, EXPOSURE, EXPOSURE) );	
	col = clamp(col, 0.0, 1.0);
	


	fragColor = vec4( col,1.0);
}