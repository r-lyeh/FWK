// noise3.jpg, stars.jpg
// Ben Quantock 2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// parameters
const float SSradius = 200.0; // Okay, it's actually a lot smaller than the Stanford Torus, but it's a cool name.
const float SSthickness = 16.0;

const float angularSpeed = .221; // g = r*w^2, e.g. w = sqrt(9.81/200) = .221

vec3 SunDir = vec3(1,1,-1.5);

const float tau = 6.28318530717958647692;

vec2 Noise( in vec3 x )
{
    vec3 p = floor(x), f = fract(x);
	f = f*f*(3.0-2.0*f);
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec4 rg = textureLod( iChannel0, (uv+0.5)/256.0, 0.0 );
	return mix( rg.yw, rg.xz, f.z );
}


#define MAX 1000.0

float Foliage( vec3 pos )
{
	return
		max(
			abs(pos.z)-SSthickness,
			abs(length(pos.xy)-SSradius-2.0 + Noise(pos/16.0).x*2.0 + 2.0*abs(pos.z)/SSthickness - 1.0)-1.0
		);
	// todo: leafiness
}

float Water( vec3 pos )
{
	return max( abs(pos.z)-SSthickness, abs(length(pos.xy)-SSradius-1.0)-1.0 );
}


float Building( vec3 pos, float seed )
{
	if ( seed < .1 )
		return length(vec3(pos.x,max(abs(pos.y)-2.0,0.0),pos.z))-2.0;
	
	vec3 p = pos;
	p.y += (seed-.5)*p.z;
	p = abs(p)-vec3(2,3,2);
	vec3 p2 = abs(pos+vec3(0,1.67,0))-vec3(1,.67,1);
	return max(max(max(p.x,p.y),p.z),-max(p2.z,p2.y));
}


float Solid( vec3 pos )
{
	float r = length(pos.xy);
	
	vec2 polar = vec2(atan(pos.x,-pos.y),length(pos.xy));

	vec3 spokep = pos;
	const float spokeangle = tau/5.0;
	float spokea = (fract(polar.x/spokeangle)-.5)*spokeangle;
	spokep.xy = polar.y*vec2(sin(spokea),-cos(spokea));
	
	return
		min(min(
			// ring
			max(max(
				// outer torus
				abs( length(vec2(pos.z, r-SSradius))-SSthickness-.5 )-.5,
				// window-gap
				5.0-abs(r-(SSradius-6.0))),
				// top gap
				min(
					6.0-abs(pos.z), //todo: close bottom
					SSradius-r
				)
			),
			// buildings
			//length( vec3((fract(polar.x*40.0)-.5)*polar.y/40.0,polar.y+1.0-SSradius,pos.z+30.0*(fract(pow(floor(polar.x*40.0)*.777,2.0))-.5) ) ) - .5
			//length( vec3((fract(polar.x*20.0)-.5)*polar.y/20.0,polar.y-SSradius,pos.z+30.0*(fract(pow(floor(polar.x*20.0)*.444,2.0))-.5) ) ) - 2.0
			Building( vec3((fract(polar.x*20.0)-.5)*polar.y/20.0,polar.y-SSradius,pos.z+30.0*(fract(pow((floor(polar.x*20.0)+5.0)*.444,2.0))-.5) ), fract(pow(floor(polar.x*20.0)*.777,2.0)) )
			// could do roads connecting them, by drawing 2 for every position
			),
			max(
				min( min(
					// spokes
					max(
						-SSradius-spokep.y,
						length(spokep.xz)-2.0
					),
					// hub
					max(
						r-20.0,
						abs(pos.z)-7.0
					)),
					max(
						r-17.0,
						abs(pos.z)-9.0
					)
				),
				// dock
				min(
					8.0-abs(pos.x),
					3.0-abs(pos.y)
				)
			)
		);
}


// smooth min, to prevent hard edges when I use union of lights
bool domin;
float smin ( float a, float b )
{
	/*const float s = 1.0;
	return -log2(exp2(-a/s)+exp2(-b/s))*s;*/
	if ( domin )
	{
		return min( a, b );
	}
	else
	{
		return 1.0/(1.0/a+1.0/b);
	}
}


float Lights( vec3 pos )
{
	return
		smin(smin(//smin(
			// ring light
			sqrt( pow(length(vec2(pos.z, length(pos.xy)-188.0)),2.0) + pow(max(.0,dot(pos.xy,vec2(sin(iGlobalTime*.3),cos(iGlobalTime*.3)))),2.0) )-.5,
			
			// landing lights
			//length( vec3( abs(pos.x)-8.0, max(vec2(0),abs(pos.yz)-vec2(3,8.5)) ) )-.1),
				  
			// moving lights to test fake lighting/shadows
			length(pos-vec3(-sin(iGlobalTime*.2)*20.0+8.0,-197,12.0*sin(iGlobalTime)))-.3),
			length(pos-vec3(-sin(iGlobalTime*.22)*30.0,-197.0+4.0*sin(iGlobalTime*.7),12.0*sin(iGlobalTime*.87)))-2.0
		);
}

vec4 SampleLights( vec3 pos )
{
	domin = false;
	
	// like then Normal calc
	float r = .1;
	vec2 d = vec2(-1,1) * r;
	vec3 p0 = pos+d.xxx; // tetrahedral offsets
	vec3 p1 = pos+d.xyy;
	vec3 p2 = pos+d.yxy;
	vec3 p3 = pos+d.yyx;
	
	float f0 = Lights(p0);
	float f1 = Lights(p1);
	float f2 = Lights(p2);
	float f3 = Lights(p3);
	
	// this direction is completely  innaccurrate for hard-edged intersections!
	// it gets one or other surface normal, rather than a smoothed result
	// so, build smooth light shapes
	
	return vec4(
				-normalize( p0*f0+p1*f1+p2*f2+p3*f3 - pos*(f0+f1+f2+f3) ),// /r, should be possible to use non-normalized value, for more realistic effect
				(f0+f1+f2+f3)/4.0
			);
}


float DistanceField( vec3 pos )
{
	domin = true;
	
	return min( min( min(
			Foliage( pos ),
			Water( pos ) ),
			Solid( pos ) ),
			Lights( pos )
		);
}


float DistanceFieldNoLights( vec3 pos )
{
	return min( min(
			Foliage( pos ),
			Water( pos ) ),
			Solid( pos )
		);
}


struct ShadeData { vec3 pos, ray, normal; float shadow; float t; };
ShadeData SetShadeData( vec3 pos, vec3 ray, vec3 normal, float shadow, float t )
{
	ShadeData s;
	s.pos = pos;
	s.ray = ray;
	s.normal = normal;
	s.shadow = shadow;
	s.t = t;
	return s;
}


vec3 DiffuseLight( ShadeData s )
{
	// sunlight
	vec3 sun = vec3(1)*2.0*max(.0,dot(s.normal,SunDir))*s.shadow;
	
	// local soft light sources
	// Actually sample the lights!!
	vec4 sl = SampleLights( s.pos );
//	float fade = 1.0/sl.w; // point lights should be 1/(w*w), line lights are 1/w, infinite area lights are 1
	//vec3 local = vec3(.4,.7,1)*5.0*(dot(sl.xyz,s.normal)*.5+.5)*fade;
	vec3 local = vec3(.4,.7,1)*2.0*(dot(sl.xyz,s.normal)+1.0)/sl.w;

	// like ambient occlusion, but towards the light
	vec3 l = normalize(sl.xyz);
	float d = 2.0;//s.t/20.0;
//shadow strength should depend on strength of SH vector
	local *= .3+.7*min( 1.0, max( 0.0, (DistanceFieldNoLights(s.pos+l*d)/d)) );// /max(.001,dot(s.normal,l)) );
	
	// ambient
	vec3 ambient = vec3(0) + local;
	// this ambient occlusion trick works ridiculously well
	// sample the distance field at a point in front of the surface
	// if there's a nearby concave surface the value will be less than the distance to the sample point
	float aoRange = s.t/20.0;
	float occlusion = max( 0.0, 1.0 - DistanceFieldNoLights( s.pos + s.normal*aoRange )/aoRange ); // can be > 1.0
	ambient *= exp2( -2.0*pow(occlusion,2.0) ); // tweak the curve
	
	return ambient;//+sun;
}


vec3 ShadeFoliage( ShadeData s )
{
	vec3 albedo = mix( vec3(.05,.02,.01), vec3(.1,.5,.0), Noise(s.pos*20.0).x*.3+.7 );
	
	return albedo*DiffuseLight(s);
}

vec3 ShadeWater( ShadeData s )
{
	const float albedoScale = 4.0;
	float rad = (SSradius+1.0)/albedoScale;
	rad = floor( rad*tau+.5 )/tau; // round it so we get a whole number of texture repeats
	vec2 uv = vec2(s.pos.z/albedoScale,atan(s.pos.x,s.pos.y)*rad);
	
	vec3 base = vec3(.05,.1,.2)*DiffuseLight(s);
	
	// normal map
	vec2 noise = (Noise( s.pos*2.0+iGlobalTime*vec3(-4,4,0) )*2.0-1.0)*.1;
	vec3 tangent = normalize(vec3(s.pos.y,-s.pos.x,0));
	vec3 binormal = vec3(0,0,1);
	vec3 normal = s.normal + tangent*noise.x + binormal*noise.y;
	
	float fresnel = dot(normal,s.ray);
	vec3 reflection = s.ray-2.0*fresnel*normal;
	fresnel = pow( 1.0-abs(fresnel), 5.0 );
	float up = dot( reflection, normal );
	float across = dot( reflection, binormal );
	vec3 refcol = vec3(.4,.7,1)*1.0*pow(1.0-abs(across),40.0);
	
	return mix( base, refcol, fresnel );
}

vec3 ShadeSolid( ShadeData s )
{
	return vec3(.8) * DiffuseLight(s);
}

vec3 ShadeLights( ShadeData s )
{
	return vec3(.4,.7,1)*8.0*abs(dot(s.ray,s.normal)); // draw the lightbulb
}

vec3 Shade( ShadeData s )
{
	float foliage = Foliage(s.pos);
	float water = Water(s.pos);
	float solid = Solid(s.pos);
	float lights = Lights(s.pos);
	float dist = min(min(min(foliage, water),solid),lights);
	
	if ( lights == dist )
		return ShadeLights(s);
	
#if (1) // test lighting
	return DiffuseLight(s)*.8;
#else
	else if ( foliage == dist )
		return ShadeFoliage(s);
	else if ( water == dist )
		return ShadeWater(s);
	else
		return ShadeSolid(s);
#endif
}


//Compute the surface normal
vec3 Normal( vec3 pos )
{
	// in theory we should be able to get a good gradient using just 4 points
	vec2 d = vec2(-1,1) * .01;
	vec3 p0 = pos+d.xxx; // tetrahedral offsets
	vec3 p1 = pos+d.xyy;
	vec3 p2 = pos+d.yxy;
	vec3 p3 = pos+d.yyx;
	
	float f0 = DistanceField(p0);
	float f1 = DistanceField(p1);
	float f2 = DistanceField(p2);
	float f3 = DistanceField(p3);
	
	return normalize( p0*f0+p1*f1+p2*f2+p3*f3 - pos*(f0+f1+f2+f3) );
}


struct Camera { vec3 pos, target, up; float zoom; };

Camera SetCam( vec3 pos, vec3 target, vec3 up, float zoom )
{
	Camera cam;
	cam.pos = pos;
	cam.target = target;
	cam.up = up;
	cam.zoom = zoom;
	return cam;
}

Camera MixCam( Camera a, Camera b, float c )
{
	Camera cam;
	cam.pos = mix( a.pos, b.pos, c );
	cam.target = mix( a.target, b.target, c );
	cam.up = mix( a.up, b.up, c );
	cam.zoom = mix( a.zoom, b.zoom, c );
	return cam;
}

vec3 BGRot( in vec3 v, in vec3 cs )
{
	return vec3(v.xy*cs.x + v.yx*cs.yz, v.z);
}

vec2 Trace( vec3 pos, vec3 ray, vec2 interval )
{
	float t = interval.x;
	float h;
	for( int i=0; i<200; i++ )
	{
		h = DistanceField( pos + t*ray );
		if ( h < .01 || t > interval.y )
			break;
		t += h;
	}
	return vec2(t,h);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	// rotate background
	float a = iGlobalTime*angularSpeed; // radians per second
	vec3 cs = vec3( cos(a), sin(a)*vec2(1,-1) );
	SunDir = normalize( BGRot( SunDir, cs ) );
	
	// todo: animate camera between pairs of keyframes
	Camera camKeys[8];
	camKeys[0] = SetCam( vec3(-400,-300,-300), vec3(0,0,0), vec3(0,.5,-1), .8 );
	camKeys[1] = SetCam( vec3(-20,3.0-SSradius,-16), vec3(SSradius,-130,0), vec3(0,1,1), .7 );
	camKeys[2] = SetCam( vec3(-10,2.0-SSradius,16), vec3(0,-SSradius,0), vec3(0,1,0), .8 );
	camKeys[3] = SetCam( vec3(30,2.5-SSradius,5), vec3(-SSradius,40.0-SSradius,-50), vec3(0,1,0), .8 );
	camKeys[4] = SetCam( vec3(30,2.5-SSradius,5), vec3(-SSradius,40.0-SSradius,-50), vec3(0,1,0), .8 );
	camKeys[5] = SetCam( vec3(-10,2.5-SSradius,2), vec3(0,SSradius,0), vec3(.5,3,1), 1.2 );
	camKeys[6] = SetCam( vec3(200,-150,400), vec3(0,0,0), vec3(0,.5,1), .8 );
	camKeys[7] = SetCam( vec3(0,0,50), vec3(0,0,0), vec3(cs.y,-cs.x,0), .8 );

	// pick a pair of cameras using time
	// todo: could manually pick cam using all combinations of "iop", shown on screen when auto-cycling (cam: io)
	// mouse moves cam along path, and target up/down (by up*length(target-pos))
	float T = fract(iGlobalTime/52.0)*4.0;
	Camera cam1, cam2;
	bool rotCam = false;
	if ( T < 1.0 ) { cam1 = camKeys[0]; cam2 = camKeys[1]; rotCam = true; }
	else if ( T < 2.0 ) { cam1 = camKeys[2]; cam2 = camKeys[3]; }
	else if ( T < 3.0 ) { cam1 = camKeys[4]; cam2 = camKeys[5]; }
	else { cam1 = camKeys[6]; cam2 = camKeys[7]; rotCam = true; }

	// mix between them
	//T = smoothstep( .15, .85, fract(T) );
	T = fract(T);
	float T2 = T*T;
	T = (6.0*T2 - 15.0*T + 10.0)*T2*T;
	Camera cam = MixCam( cam1, cam2, T );
	
//rotCam = true; cam = camKeys[1];
rotCam = false; cam = camKeys[3];
	
	//Camera cam = SetCam( vec3(500.0*(iMouse.xy/iResolution.xy-.5),0)+vec3(0,-300,-300), vec3(0,-100,0), vec3(0,1,0), .7 );
	
	// fire a ray from the camera
	vec3 pos = cam.pos;
	vec3 forward = normalize(cam.target-cam.pos);
	vec3 right = normalize(cross(cam.up,forward));
	vec3 up = normalize(cross(forward,right));
	vec3 ray = normalize(vec3( fragCoord.xy-iResolution.xy*.5, iResolution.x*cam.zoom ));
	ray = right*ray.x + up*ray.y + forward*ray.z;
	
	if ( rotCam )
	{
		pos = BGRot( pos, cs );
		ray = BGRot( ray, cs );
	}
	
	// intersect that ray with isosurface bounding volume
	vec2 interval = vec2(0,1000);
	
	// march isosurface
	vec2 th = Trace( pos, ray, interval );
	
	// shading
	vec3 col;
	
	if ( th.y < 1.0 ) // shade some near misses to reduce artefacts
	{
		vec3 p = pos + th.x*ray;
		vec3 n = Normal(p);
		
		float shadowBias = mix ( 1.0, .1, abs(dot( n, SunDir )) );

		float shadow = Trace( p, SunDir, vec2(shadowBias, 1000.0) ).y;
		
		shadow = smoothstep( .01, 4.0, shadow );
		
		col = Shade( SetShadeData(p,ray,n,shadow,th.x) );
	}
	else
	{
		// draw background
		ray = BGRot( ray, cs.xzy );
		float s = 1.3;
		vec3 X = texture( iChannel1, ray.yz*s ).rgb;
		vec3 Y = texture( iChannel1, ray.xz*s ).rgb;
		vec3 Z = texture( iChannel1, ray.xy*s ).rgb;
		col = mix( X, Y, smoothstep(-.3,.3,abs(ray.y)-abs(ray.x)) );
		col = mix( col, Z, smoothstep(-.3,.3,abs(ray.z)-max(abs(ray.x),abs(ray.y))) );
		col = pow(col,vec3(7,5,3))*.1;
		
		col = vec3(0);
	}

	fragColor = vec4(pow(col,vec3(1.0/2.2)),1.0);
}