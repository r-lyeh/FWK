// Ben Weston - 15/08/2013

/*
Eye ball effects:
• Ray-marched shape
• Ray-traced iris refraction
• Fake photon mapping on iris
• Subsurface scattering on sclera
• HDR reflections with fresnel
• Eyelid reflection occlusion
• Eyelid ambient occlusion
• Procedural textures
• Procedural animation
*/

// KEY CONTROLS - (click on eye to give keyboard focus)
const int Key_M = 77; // mouse controls camera / eye direction

const int Key_E = 69; // refraction on/off
const int Key_P = 80; // photon mapping on/off
const int Key_L = 76; // change photon mapping technique (both fake, but one is imitating reality and the other is prettier)

const int Key_S = 83; // subsurface scattering on/off
const int Key_A = 65; // ambient occlusion on/off

const int Key_R = 82; // reflection on/off
const int Key_O = 79; // reflection eyelid occlusion on/off

const int Key_C = 67; // iris colour
const int Key_N = 78; // iris normal


// Lights
#if (1)
	// High-contrast light edge-on
	const vec3 lightDir = vec3(-2,2,.5);
	const vec3 lightColour = vec3(1.0);
	const vec3 fillLightDir = vec3(0,1,0);
	const vec3 fillLightColour = vec3(.65,.7,.8)*.7;//vec3(.15,.2,.25);
#else
	// more neutral "good" lighting (doesn't show off the effects)
	const vec3 lightDir = vec3(-2,2,-1);
	const vec3 lightColour = vec3(.83,.8,.78);
	const vec3 fillLightDir = vec3(0,1,0);
	const vec3 fillLightColour = vec3(.65,.7,.8);
#endif



// Constants
const float tau = 6.28318530717958647692;

// Forward declarations
float Noise( in vec3 x );
vec2 Noise2( in vec3 x );



// Gamma correction
#define GAMMA (2.2)

vec3 ToLinear( in vec3 col )
{
	// simulate a monitor, converting colour values into light values
	return pow( col, vec3(GAMMA) );
}

vec3 ToGamma( in vec3 col )
{
	// convert back into colour values, so the correct light will come out of the monitor
	return pow( col, vec3(1.0/GAMMA) );
}


// key is javascript keycode: http://www.webonweboff.com/tips/js/event_key_codes.aspx
bool ReadKey( int key, bool toggle )
{
	float keyVal = texture( iChannel3, vec2( (float(key)+.5)/256.0, toggle?.75:.25 ) ).x;
	return (keyVal>.5)?true:false;
}


// ------- EDIT THESE THINGS! -------

// Camera (also rotated by mouse)
const vec3 CamPos = vec3(0,0.0,-250.0);
const vec3 CamLook = vec3(0,0,0);
const float CamZoom = 10.0;
const float NearPlane = 0.0; // actually not needed
const float drawDistance = 1000.0;

const vec3 SkyColour = vec3(.4,.25,.2);//fillLightColour*.5;//vec3(.1,.3,.5);

vec3 SkyDome( vec3 rd )
{
	//the cube maps have lines in, and aren't HDR, so make our own shapes
	
	// random variation
	vec3 result = ToLinear(SkyColour)*2.0*Noise(rd);
	
	// square sky-light
	result = mix( result, vec3(8), smoothstep(.8,1.0,rd.y/max((rd.x+1.0),abs(rd.z))) );

	return result;
}

// Eye params
const float IrisAng = tau/12.0;
const float PupilAng = (1.6*IrisAng/5.0);
const float EyeRadius = 10.0;
const float BulgeRadius = 6.0; // used for photon trace, must be bigger than EyeRadius*sin(IrisAng)


vec4 ComputeEyeRotation()
{
	vec2 rot;
	if ( !ReadKey( Key_M, true ) && iMouse.w > .00001 )
		rot = .25*vec2(1.0,1.0)*tau*(iMouse.xy-iResolution.xy*.5)/iResolution.x;
	else
	{
		float time = iGlobalTime/2.0;
		time += Noise(vec3(0,time,0)); // add noise to time (this adds SO MUCH character!)
		float flick = floor(time)+smoothstep(0.0,0.05,fract(time));
		rot = vec2(.2,.1)*tau*(texture( iChannel0, vec2((flick+.5)/256.0,.5), -100.0 ).rb-.5);
	}
	
	return vec4(cos(rot.x),sin(rot.x),cos(rot.y),sin(rot.y));
}


vec3 ApplyEyeRotation( vec3 pos, vec4 rotation )
{
	pos.yz = rotation.z*pos.yz + rotation.w*pos.zy*vec2(1,-1);
	pos.xz = rotation.x*pos.xz + rotation.y*pos.zx*vec2(1,-1);
	
	return pos;
}
	


// Shape
// This should return continuous positive values when outside and negative values inside,
// which roughly indicate the distance of the nearest surface.
float Isosurface( vec3 pos, vec4 eyeRotation )
{
	pos = ApplyEyeRotation(pos,eyeRotation);
	
/*	float f = length(pos)-EyeRadius;
	
//	f += Noise(pos*3.0)*.008;

	// cornea bulge
	float o = EyeRadius*cos(IrisAng)-sqrt(BulgeRadius*BulgeRadius-EyeRadius*EyeRadius*pow(sin(IrisAng),2.0));
	float g = length(pos-vec3(0,0,-o))-BulgeRadius;

//g += Noise(pos/2.0)*.5;

	return min(f,g);
	//return -log(exp(-g*2.0)+exp(-f*2.0))/2.0;*/
	
	vec2 slice = vec2(length(pos.xy),pos.z);
	
	float aa = atan(slice.x,-slice.y);
	float bulge = cos(tau*.2*aa/IrisAng);
	bulge = bulge*.8-.8;
	bulge *= smoothstep(tau*.25,0.0,aa);
	
	// sharp-edged bulge
//	if ( aa < IrisAng )
//		bulge += cos(tau*.25*aa/IrisAng)*.5;
	bulge += cos(tau*.25*aa/IrisAng)*.5 * smoothstep(-.02,.1,IrisAng-aa); // slightly softer
	
	return length(slice) - EyeRadius - bulge;
}



float GetEyelidMask( vec3 pos, vec4 eyeRotation )
{
	vec3 eyelidPos = pos;
	float eyelidTilt = -.05;
	eyelidPos.xy = cos(eyelidTilt)*pos.xy + sin(eyelidTilt)*pos.yx*vec2(1,-1);
	
	float highLid = tan(max(tau*.05,asin(eyeRotation.w)+IrisAng+.05));
	float lowLid = tan(tau*.1);
	
	float blink = smoothstep(.0,.02,abs(Noise(vec3(iGlobalTime*.2,0,0))-.5 ));
	highLid *= blink;
	lowLid *= blink;
	
	return min(
				(-eyelidPos.z-2.0) - (-eyelidPos.y/lowLid),
				(-eyelidPos.z-2.0) - (eyelidPos.y/highLid)
			);
}
	
float GetIrisPattern( vec2 uv )
{
	return Noise( vec3( 10.0*uv/pow(length(uv),.7), 0 ) );
}

// Colour
vec3 Shading( vec3 worldPos, vec3 norm, float shadow, vec3 rd, vec4 eyeRotation )
{
	vec3 view = normalize(-rd);

	// eyelids - just match BG colour
	float eyelidMask = GetEyelidMask(worldPos, eyeRotation);
	
	if ( eyelidMask < 0.0 || (-worldPos.z-3.0) < (worldPos.x/tan(tau*.23)) )
	{
		return ToLinear(SkyColour);
	}
	
	vec3 pos = ApplyEyeRotation(worldPos,eyeRotation);
	
	float lenposxy = length(pos.xy);
	float ang = atan(lenposxy/(-pos.z));
	if ( ang < 0.0 )
		ang += tau/2.0;
	

	// refract ray
	vec3 irisRay = ApplyEyeRotation(-view,eyeRotation);
	vec3 localNorm = ApplyEyeRotation(norm,eyeRotation);
	float a = dot(irisRay,localNorm);
	float b = cos(acos(a)*1.33);
	if ( !ReadKey( Key_E, true ) )
		irisRay += localNorm*(b-a);
	irisRay = normalize(irisRay);
	
	// intersect with plane
	float planeDist = -cos(IrisAng)*EyeRadius;
	float t = (planeDist-pos.z)/irisRay.z;

	vec3 ppos = t*irisRay+pos;


	// polar coord map
	float rad = length(ppos.xy);
	float pupilr = EyeRadius*sin(PupilAng);
	float irisr = EyeRadius*sin(IrisAng);
	
	float irisPattern = GetIrisPattern(ppos.xy); // reduce contrast of this now we have actual lighting!

/*	vec3 iris = mix( mix( vec3(.3,.1,.1)*.5+.5*vec3(.6,.4,.1), vec3(.6,.4,.1), irisPattern ), // hazel
					mix( vec3(.2,.2,.2)*.5+.5*vec3(.5,.45,.2), vec3(.5,.45,.2), irisPattern ),*/

/*	vec3 iris = mix( mix( vec3(.1,.1,.4), vec3(.7,.9,1), irisPattern ), // blue
					mix( vec3(.1,.1,.4), vec3(.3,.4,.7), irisPattern ),*/

//					smoothstep(pupilr*2.0,irisr,rad));

	vec3 iris = ToLinear( mix( pow( vec3(.65,.82,.85), 2.0*vec3(1.2-sqrt(irisPattern)) ),
					vec3(1,.5,.2), .7*pow( mix( smoothstep(pupilr,irisr,rad), Noise(ppos), .7), 2.0) ));

	if ( ReadKey( Key_C, true ) )
		iris = vec3(1);

	// darken outer
	iris *= pow( smoothstep( irisr+1.0, irisr-1.5, rad ), GAMMA );


	vec3 irisNorm;
	irisNorm.x = GetIrisPattern(ppos.xy+vec2(-.001,0)) - GetIrisPattern(ppos.xy+vec2(.001,0));
	irisNorm.y = GetIrisPattern(ppos.xy+vec2(0,-.001)) - GetIrisPattern(ppos.xy+vec2(0,.001));

	// add a radial lump
	irisNorm.xy += -.01*normalize(ppos.xy)*sin(1.*tau*rad/irisr);

	irisNorm.z = -.15; // adjust severity of bumps
	irisNorm = normalize(irisNorm);

	if ( ReadKey( Key_N, true ) )
		irisNorm = vec3(0,0,-1);
		

	// lighting
	// fake photon mapping by crudely sampling the photon density

	// apply lighting with this modified normal
	vec3 lightDirN = normalize(lightDir);
	vec3 localLightDir = ApplyEyeRotation(lightDirN,eyeRotation);

	vec3 fillLightDirN = normalize(fillLightDir);
	vec3 localFillLightDir = ApplyEyeRotation(fillLightDirN,eyeRotation);

	// Bend the light, imitating results of offline photon-mapping
	// Jimenez's paper makes this seem very complex, because their mapping used a non-flat receiver
	// but the self-shadowing was negligible, so the main effect was just like premultiplying by a normal
	// where we'd get better results by using the actual normal.

	float photonsL, photonsFL;

	if ( !ReadKey( Key_P, true ) )
	{
		if ( !ReadKey( Key_L, true ) )
		{
			// Nice retro-reflective effect, but not correct
			vec3 nn = normalize(vec3( ppos.xy, -sqrt(max(0.0,BulgeRadius*BulgeRadius-rad*rad)) ));
			
			vec3 irisLDir = localLightDir;
			vec3 irisFLDir = localFillLightDir;
		//	irisLDir.z = -cos(acos(-irisLDir.z)/1.33); // experiments showed it cuts out at 120 degrees, i.e. 1.33*the usual 90 degree cutoff
		//	irisFLDir.z = -cos(acos(-irisFLDir.z)/1.33); // experiments showed it cuts out at 120 degrees, i.e. 1.33*the usual 90 degree cutoff
			float d = dot(nn,irisLDir);
			irisLDir += nn*(cos(acos(d)/1.33) - d);
			d = dot(nn,irisFLDir);
			irisFLDir += nn*(cos(acos(d)/1.33) - d);
			irisLDir = normalize(irisLDir);
			irisFLDir = normalize(irisFLDir);
			photonsL = smoothstep(0.0,1.0,dot(irisNorm,irisLDir)); //soften terminator
			photonsFL = (dot(irisNorm,irisFLDir)*.5+.5);
			//Seriously, this^ looks really nice, but not like reality. Bah!
		
		/* reverse it, to make it look a lot like the accurate version - meh
			vec3 nn = normalize(vec3( -ppos.xy, -sqrt(max(0.0,BulgeRadius*BulgeRadius-rad*rad)) ));
			
			vec3 irisLDir = localLightDir;
			vec3 irisFLDir = localFillLightDir;
			float d = dot(nn,irisLDir);
			irisLDir += nn*(cos(acos(d)/1.33) - d);
			d = dot(nn,irisFLDir);
			irisFLDir += nn*(cos(acos(d)/1.33) - d);
			irisLDir = normalize(irisLDir);
			irisFLDir = normalize(irisFLDir);
			
			float photonsL = smoothstep(0.0,1.0,dot(irisNorm,irisLDir)); // soften the terminator
			float photonsFL = (dot(irisNorm,irisFLDir)*.5+.5);
		*/
	
		}
		else
		{
			//this is a reasonable match to the dark crescent effect seen in photos and offline photon mapping, but it looks wrong to me.
			vec3 irisLDir = localLightDir;
			vec3 irisFLDir = localFillLightDir;
			irisLDir.z = -cos(acos(-irisLDir.z)/1.5); // experiments showed it cuts out at 120 degrees, i.e. 1.33*the usual 90 degree cutoff
			irisFLDir.z = -cos(acos(-irisFLDir.z)/1.5); // experiments showed it cuts out at 120 degrees, i.e. 1.33*the usual 90 degree cutoff
			irisLDir = normalize(irisLDir);
			irisFLDir = normalize(irisFLDir);
		
			photonsL = smoothstep(0.0,1.0,dot(irisNorm,irisLDir)); // soften the terminator
			photonsFL = (dot(irisNorm,irisFLDir)*.5+.5);
		
			// dark caustic ring
			photonsL *= .3+.7*smoothstep( 1.2, .9, length(ppos.xy/irisr+.2*irisLDir.xy/(irisLDir.z-.05)) );
		//	photonsFL *= ...;
		}
	}
	else
	{
		// no photons
		photonsL = max( 0.0, dot(irisNorm,localLightDir) ); 
		photonsFL = .5+.5*dot(irisNorm,localLightDir); 
	}
		
	vec3 l = ToLinear(lightColour)*photonsL;
	vec3 fl = ToLinear(fillLightColour)*photonsFL;

	vec3 ambientOcclusion = vec3(1);
	vec3 eyelidShadow = vec3(1);
	if ( !ReadKey( Key_A, true ) )
	{
		// ambient occlusion on fill light
		ambientOcclusion = mix( vec3(1), ToLinear(vec3(.8,.7,.68)), pow(smoothstep( 5.0, 0.0, eyelidMask ),1.0) );
		
		// shadow on actual light
		eyelidShadow = mix( vec3(1), ToLinear(vec3(.8,.7,.68)), smoothstep( 2.0, -2.0, GetEyelidMask( worldPos+lightDir*1.0, eyeRotation ) ) );
	}
	fl *= ambientOcclusion;
	l *= eyelidShadow;
	
	iris *= l+fl;

	// darken pupil
	iris *= smoothstep( pupilr-.01, pupilr+.5, rad );


	// veins
	float theta = atan(pos.x,pos.y);
	theta += Noise(pos*1.0)*tau*.03;
	float veins = (sin(theta*60.0)*.5+.5);
	veins *= veins;
	veins *= (sin(theta*13.0)*.5+.5);
	veins *= smoothstep( IrisAng, tau*.2, ang );
	veins *= veins;
	veins *= .5;
	
	vec3 sclera = mix( ToLinear(vec3(1,.98,.96)), ToLinear(vec3(.9,.1,0)), veins );

	float ndotl = dot(norm,lightDirN);
	
	// subsurface scattering
//	float subsurface = max(0.0,-2.0*ndotl*EyeRadius);
//	l = pow(ToLinear(vec3(.5,.3,.25)),vec3(subsurface*.2)); // more intense the further light had to travel

	// fake, because that^ approximation gives a hard terminator
	l = pow(ToLinear(vec3(.5,.3,.25)), vec3(mix( 3.0, 0.0, smoothstep(-1.0,.2,ndotl) )) );
	
	if ( ReadKey( Key_S, true ) )
//		l = mix( l, vec3(max(0.0,ndotl)), 0.5 );
//	else
		l = vec3(max(0.0,ndotl));

	l *= ToLinear(lightColour);
	
	fl = ToLinear(fillLightColour)*(dot(norm,fillLightDirN)*.5+.5);

	fl *= ambientOcclusion;
	l *= eyelidShadow;
	
	
	sclera *= l+fl;

	// blend between them
	float blend = smoothstep(-.1,.1,ang-IrisAng);
	vec3 result = mix(iris,sclera,blend);

	
	// eyelid ambient occlusion/radiosity
//	if ( !ReadKey( Key_A, true ) )
		//result *= mix( vec3(1), ToLinear(vec3(.65,.55,.55)), exp2(-eyelidMask*2.0) );
//		result *= mix( vec3(1), ToLinear(vec3(.8,.7,.68)), pow(smoothstep( 5.0, 0.0, eyelidMask ),1.0) );
	
	
	// bumps - in specular only to help sub-surface scattering look smooth
	vec3 bumps;
	bumps.xy = .7*Noise2( pos*3.0 );
	bumps.z = sqrt(1.0-dot(bumps.xy,bumps.xy));

	bumps = mix( vec3(0,0,1), bumps, blend );
	
	norm.xy += bumps.xy*.1;
	norm = normalize(norm);
	
	float glossiness = mix(.7,1.0,bumps.z);
	
	// reflection map
	float ndoti = dot( view, norm );
	vec3 rr = -view+2.0*ndoti*norm;
	vec3 reflection = SkyDome( rr );
	
	// specular
	vec3 h = normalize(view+lightDir);
	float specular = pow(max(0.0,dot(h,norm)),2000.0);

	// should fresnel affect specular? or should it just be added?
	reflection += specular*32.0*glossiness*ToLinear(lightColour);

	// reflection of eyelids
	//float eyelidReflection = smoothstep( 1.8, 2.0, eyelidMask );
	// apply some parallax (subtle improvement when looking up/down at eye)
	float eyelidReflection = smoothstep( .8, 1.0, GetEyelidMask( normalize(worldPos + rd*2.0)*EyeRadius, eyeRotation ) );
	if ( !ReadKey( Key_O, true ) )
		reflection *= eyelidReflection;

	// fresnel
	float fresnel = mix(.04*glossiness,1.0,pow(1.0-ndoti,5.0));

	if ( !ReadKey( Key_R, true ) )
		result = mix ( result, reflection, fresnel );


	//anti-alias the edge
	float mask2 = min( eyelidMask, (-worldPos.z-3.0) - (worldPos.x/tan(tau*.23)) );
	result = mix( ToLinear(SkyColour), result, smoothstep(.0,.3,mask2) );
	
	return result;
}


// Precision controls
const float epsilon = .003;
const float normalPrecision = .1;
const float shadowOffset = .1;
const int traceDepth = 100; // takes time



// ------- BACK-END CODE -------

vec2 Noise2( in vec3 x )
{
    vec3 p = floor(x.xzy);
    vec3 f = fract(x.xzy);
	f = f*f*(3.0-2.0*f);
//	vec3 f2 = f*f; f = f*f2*(10.0-15.0*f+6.0*f2);

	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec4 rg = textureLod( iChannel0, (uv+0.5)/256.0, 0.0 );
	return mix( rg.yw, rg.xz, f.z );
}
		
float Noise( in vec3 x )
{
	return Noise2(x).x;
}

float Trace( vec3 ro, vec3 rd, vec4 eyeRotation )
{
	float t = 0.0;
	float dist = 1.0;
	for ( int i=0; i < traceDepth; i++ )
	{
		if ( abs(dist) < epsilon || t > drawDistance || t < 0.0 )
			continue;
		dist = Isosurface( ro+rd*t, eyeRotation );
		t = t+dist;
	}
	
	return t;//vec4(ro+rd*t,dist);
}

// get normal
vec3 GetNormal( vec3 pos, vec4 eyeRotation )
{
	const vec2 delta = vec2(normalPrecision, 0);
	
	vec3 n;

// it's important this is centred on the pos, it fixes a lot of errors
	n.x = Isosurface( pos + delta.xyy, eyeRotation ) - Isosurface( pos - delta.xyy, eyeRotation );
	n.y = Isosurface( pos + delta.yxy, eyeRotation ) - Isosurface( pos - delta.yxy, eyeRotation );
	n.z = Isosurface( pos + delta.yyx, eyeRotation ) - Isosurface( pos - delta.yyx, eyeRotation );
	return normalize(n);
}				

// camera function by TekF
// compute ray from camera parameters
vec3 GetRay( vec3 dir, float zoom, vec2 uv )
{
	uv = uv - .5;
	uv.x *= iResolution.x/iResolution.y;
	
	dir = zoom*normalize(dir);
	vec3 right = normalize(cross(vec3(0,1,0),dir));
	vec3 up = normalize(cross(dir,right));
	
	return dir + right*uv.x + up*uv.y;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = fragCoord.xy / iResolution.xy;

	vec3 camPos = CamPos;
	vec3 camLook = CamLook;

	vec2 camRot = .5*tau*(iMouse.xy-iResolution.xy*.5)/iResolution.x;
	if ( !ReadKey( Key_M, true ) )
		camRot = vec2(0,0);
	camPos.yz = cos(camRot.y)*camPos.yz + sin(camRot.y)*camPos.zy*vec2(1,-1);
	camPos.xz = cos(camRot.x)*camPos.xz + sin(camRot.x)*camPos.zx*vec2(1,-1);
	
	vec4 eyeRotation = ComputeEyeRotation();
	
	if ( Isosurface(camPos, eyeRotation) <= 0.0 )
	{
		// camera inside ground
		fragColor = vec4(0,0,0,0);
		return;
	}

	vec3 ro = camPos;
	vec3 rd;
	rd = GetRay( camLook-camPos, CamZoom, uv );
	
	ro += rd*(NearPlane/CamZoom);
	
	rd = normalize(rd);
	
	float t = Trace(ro,rd,eyeRotation);

	vec3 result = ToLinear(SkyColour);
	if ( t > 0.0 && t < drawDistance )
	{
		vec3 pos = ro+t*rd;
			
		vec3 norm = GetNormal(pos,eyeRotation);
		
		// shadow test
		float shadow = 1.0;
		if ( Trace( pos+lightDir*shadowOffset, lightDir, eyeRotation ) < drawDistance )
			shadow = 0.0;
		
		result = Shading( pos, norm, shadow, rd, eyeRotation );
		
		// fog
//		result = mix ( SkyColour, result, exp(-t*t*.0002) );
	}

	fragColor = vec4( ToGamma( result ), 1.0 );
}