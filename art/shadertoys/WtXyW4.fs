// https://www.shadertoy.com/view/WtXyW4

// License CC0: Alien Waterworld
//  Been working on space themed shaders
//  Code "borrowed" from the usual suspects

#define PI  3.141592654
#define TAU (2.0*PI)

#define TOLERANCE       0.00001
#define MAX_ITER        55
#define MAX_DISTANCE    31.0
#define PERIOD          45.0
#define TIME            mod(iTime, PERIOD)

const vec3  skyCol1       = vec3(0.35, 0.45, 0.6);
const vec3  skyCol2       = skyCol1*skyCol1*skyCol1*3.0;
const vec3  sunCol1       = vec3(1.0,0.9,0.8);
const vec3  sunCol2       = vec3(1.0,0.9,0.8);
const vec3  smallSunCol1  = vec3(1.0,0.5,0.25)*0.5;
const vec3  smallSunCol2  = vec3(1.0,0.5,0.25)*0.5;
const vec3  ringColor     = sqrt(vec3(0.95, 0.65, 0.45));
const vec4  planet        = vec4(80.0, -20.0, 100.0, 50.0)*1000.0;
const vec3  planetCol     = sqrt(vec3(0.9, 0.8, 0.7));
const vec3  ringsNormal   = normalize(vec3(1.0, 1.25, 0.0));
const vec4  rings         = vec4(ringsNormal, -dot(ringsNormal, planet.xyz));

void rot(inout vec2 p, float a) {
  float c = cos(a);
  float s = sin(a);
  p = vec2(p.x*c + p.y*s, -p.x*s + p.y*c);
}

float psin(float f) {
  return 0.5 + 0.5*sin(f);
}

vec2 toRect(vec2 p) {
  return p.x*vec2(cos(p.y), sin(p.y));
}

vec2 toPolar(vec2 p) {
  return vec2(length(p), atan(p.y, p.x));
}

float mod1(inout float p, float size) {
  float halfsize = size*0.5;
  float c = floor((p + halfsize)/size);
  p = mod(p + halfsize, size) - halfsize;
  return c;
}

float circle(vec2 p, float r) {
  return length(p) - r;
}

float rayPlane(vec3 ro, vec3 rd, vec4 p) {
  return -(dot(ro,p.xyz)+p.w)/dot(rd,p.xyz);
}

vec2 raySphere(vec3 ro, vec3 rd, vec4 sphere)
{
  vec3 ce = sphere.xyz;
  float ra = sphere.w;
  vec3 oc = ro - ce;
  float b = dot( oc, rd );
  float c = dot( oc, oc ) - ra*ra;
  float h = b*b - c;
  if( h<0.0 ) return vec2(-1.0); // no intersection
  h = sqrt( h );
  return vec2( -b-h, -b+h );
}

float hash(in vec2 co) {
  return fract(sin(dot(co.xy ,vec2(12.9898,58.233))) * 13758.5453);
}


float noise2(vec2 x) {
  vec2 i = floor(x);
  vec2 w = fract(x);

#if 1
  // quintic interpolation
  vec2 u = w*w*w*(w*(w*6.0-15.0)+10.0);
#else
  // cubic interpolation
  vec2 u = w*w*(3.0-2.0*w);
#endif

  float a = hash(i+vec2(0.0,0.0));
  float b = hash(i+vec2(1.0,0.0));
  float c = hash(i+vec2(0.0,1.0));
  float d = hash(i+vec2(1.0,1.0));

  float k0 =   a;
  float k1 =   b - a;
  float k2 =   c - a;
  float k3 =   d - c + a - b;

  return k0 + k1*u.x + k2*u.y + k3*u.x*u.y;
}

float smoother(float d, float s) {
  return s*tanh(d/s);
}

float heightMod(vec2 p) {
  vec2 pp = toPolar(p);
  pp.y += -pp.x*0.2;
  p = toRect(pp);
  return pow((psin(1.0*p.x)*psin(1.0*p.y)), max(0.25, pp.x*0.20))*0.8;
}

float loheight(vec2 p, float d) {
  const float aa   = 0.5;
  const float ff   = 2.03;
  const float tt   = 1.3;
  const float oo   = 0.93;

  float hm = heightMod(p);

  vec2 s = vec2(0.0);
  float a = 1.0;
  float o = 0.2;

  for (int i = 0; i < 3; ++i) {
    float nn = a*(noise2(2.0*p));
    s.x += nn;
    s.y += abs(a);
    p += o;
    a *= aa;
    p *= ff;
    o *= oo;
    rot(p, tt);
  }

  s.x /= s.y;
  s.x -= 1.0;
  s.x += 0.7*hm;
  s.x = smoother(s.x, 0.125);

  return max(s.x+0.125, 0.0)*0.5;
}

float height(vec2 p, float d) {
  const float aa   = 0.5;
  const float ff   = 2.03;
  const float tt   = 1.3;
  const float oo   = 0.93;

  float hm = heightMod(p);

  vec2 s = vec2(0.0);
  float a = 1.0;
  float o = 0.2;

  for (int i = 0; i < 5; ++i) {
    float nn = a*(noise2(2.0*p));
    s.x += nn;
    s.y += abs(a);
    p += o;
    a *= aa;
    p *= ff;
    o *= oo;
    rot(p, tt);
  }

  s.x /= s.y;
  s.x -= 1.0;
  s.x += 0.7*hm;
  s.x = smoother(s.x, 0.125);

  return max(s.x+0.125, 0.0)*0.5;
}

float hiheight(vec2 p, float d) {
  const float aa   = 0.5;
  const float ff   = 2.03;
  const float tt   = 1.3;
  const float oo   = 0.93;

  float hm = heightMod(p);

  vec2 s = vec2(0.0);
  float a = 1.0;
  float o = 0.2;

  for (int i = 0; i < 6; ++i) {
    float nn = a*(noise2(2.0*p));
    s.x += nn;
    s.y += abs(a);
    p += o;
    a *= aa;
    p *= ff;
    o *= oo;
    rot(p, tt);
  }

  s.x /= s.y;
  s.x -= 1.0;
  s.x += 0.7*hm;
  s.x = smoother(s.x, 0.125);

  return max(s.x+0.125, 0.0)*0.5;
}

vec3 normal(vec2 p, float d) {
  vec2 eps = vec2(0.000125, 0.0);

  vec3 n;

  n.x = (hiheight(p - eps.xy, d) - hiheight(p + eps.xy, d));
  n.y = 2.0*eps.x;
  n.z = (hiheight(p - eps.yx, d) - hiheight(p + eps.yx, d));

  return normalize(n);
}

float march(vec3 ro, vec3 rd, float id, out int max_iter) {
  float dt = 0.1;
  float d = id;
  int currentStep = 0;
  float lastd = d;
  for (int i = 0; i < MAX_ITER; ++i) {
    vec3 p = ro + d*rd;
    float h = height(p.xz, d);

    if (d > MAX_DISTANCE) {
      max_iter = i;
      return MAX_DISTANCE;
    }

    float hd = p.y - h;

    if (hd < TOLERANCE) {
      return d;
    }

    float sl = 0.9;

    dt = max(hd*sl, TOLERANCE+0.0005*d);
    lastd = d;
    d += dt;
  }

  max_iter = MAX_ITER;
  return MAX_DISTANCE;
}

vec3 sunDirection() {
  return normalize(vec3(-0.5, 0.2, 1.0));
}

vec3 smallSunDirection() {
  return normalize(vec3(-0.2, -0.05, 1.0));
}

vec3 skyColor(vec3 ro, vec3 rd) {
  vec3 sunDir = sunDirection();
  vec3 smallSunDir = smallSunDirection();

  float sunDot = max(dot(rd, sunDir), 0.0);
  float smallSunDot = max(dot(rd, smallSunDir), 0.0);

  float angle = atan(rd.y, length(rd.xz))*2.0/PI;

  vec3 sunCol = 0.5*sunCol1*pow(sunDot, 20.0) + 8.0*sunCol2*pow(sunDot, 2000.0);
  vec3 smallSunCol = 0.5*smallSunCol1*pow(smallSunDot, 200.0) + 8.0*smallSunCol2*pow(smallSunDot, 20000.0);


  vec2 si = raySphere(ro, rd, planet);
  float pi = rayPlane(ro, rd, rings);

  float dustTransparency = smoothstep(-0.15, 0.075, rd.y);

  vec3 skyCol = mix(skyCol1, skyCol2, sqrt(dustTransparency));
  skyCol *= (1.0-dustTransparency);


  vec3 planetSurface = ro + si.x*rd;
  vec3 planetNormal = normalize(planetSurface - planet.xyz);
  float planetDiff = max(dot(planetNormal, sunDir), 0.0);
  float planetBorder = max(dot(planetNormal, -rd), 0.0);
  float planetLat = (planetSurface.x+planetSurface.y)*0.0005;
  vec3 planetCol = mix(1.3*planetCol, 0.3*planetCol, pow(psin(planetLat+1.0)*psin(sqrt(2.0)*planetLat+2.0)*psin(sqrt(3.5)*planetLat+3.0), 0.5));

  vec3 ringsSurface = ro + pi*rd;

  float borderTransparency = smoothstep(0.0, 0.1, planetBorder);

  float ringsDist = length(ringsSurface - planet.xyz)*1.0;
  float ringsPeriod = ringsDist*0.001;
  const float ringsMax = 150000.0*0.655;
  const float ringsMin = 100000.0*0.666;
  float ringsMul = pow(psin(ringsPeriod+1.0)*psin(sqrt(0.5)*ringsPeriod+2.0)*psin(sqrt(0.45)*ringsPeriod+4.0)*psin(sqrt(0.35)*ringsPeriod+5.0), 0.25);
  float ringsMix = psin(ringsPeriod*10.0)*psin(ringsPeriod*10.0*sqrt(2.0))*(1.0 - smoothstep(50000.0, 200000.0, pi));

  vec3 ringsCol = mix(vec3(0.125), 0.75*ringColor, ringsMix)*step(-pi, 0.0)*step(ringsDist, ringsMax)*step(-ringsDist, -ringsMin)*ringsMul;

  vec3 final = vec3(0.0);

  final += ringsCol*(step(pi, si.x) + step(si.x, 0.0));

  final += step(0.0, si.x)*pow(planetDiff, 0.75)*mix(planetCol, ringsCol, 0.0)*dustTransparency*borderTransparency + ringsCol*(1.0 - borderTransparency);

  final += skyCol + sunCol + smallSunCol;

  return final;
}

vec3 shipColor(vec2 p) {
  vec2 pp = toPolar(p);
  pp.y += pp.x*0.05;
  p = toRect(pp);

  float n = mod1(p.x, 0.15);
  p.y += 3.0-TIME*0.5+0.05*abs(n*n);

  float td = abs(p.x) - (0.005-p.y*0.002);
  td = abs(td) - (0.02*pow(-p.y, 0.25));
  float sd = circle(p, 0.05);

  vec3 trailCol = vec3(0.5)*smoothstep(-5.0, 0.0, p.y)*step(p.y, 0.0)*smoothstep(0.0, 0.025, -td);
  vec3 shipCol = vec3(0.5+smoothstep(-1.0, 1.0, sin(TIME*15.0*TAU+n)))*smoothstep(0.0, 0.075, -sd);

  vec3 col = trailCol;
  col += shipCol;

  float sm = step(abs(n), 2.0);

  return col*sm;
}

vec3 getColor(vec3 ro, vec3 rd) {
  int max_iter = 0;
  vec3 skyCol = skyColor(ro, rd);
  vec3 col = vec3(0);

  const float shipHeight = 1.0;
  const float seaHeight = 0.0;
  const float cloudHeight = 0.2;
  const float upperCloudHeight = 0.5;

  float id = (cloudHeight - ro.y)/rd.y;

  if (id > 0.0) {
    float d = march(ro, rd, id, max_iter);
    vec3 sunDir = sunDirection();
    vec3 osunDir = sunDir*vec3(-1.0, 1.0, -1.0);
    vec3 p = ro + d*rd;

    float loh = loheight(p.xz, d);
    float loh2 = loheight(p.xz+sunDir.xz*0.05, d);
    float hih = hiheight(p.xz, d);
    vec3 normal = normal(p.xz, d);

    float ud = (upperCloudHeight - 4.0*loh - ro.y)/rd.y;

    float sd = (seaHeight - ro.y)/rd.y;
    vec3 sp = ro + sd*rd;
    float scd = (cloudHeight - sp.y)/sunDir.y;
    vec3 scp = sp + sunDir*scd;

    float sloh = loheight(scp.xz, d);
    float cshd = exp(-15.0*sloh);

    float amb = 0.3;

    vec3 seaNormal = normalize(vec3(0.0, 1.0, 0.0));
    vec3 seaRef = reflect(rd, seaNormal);
    vec3 seaCol = .25*skyColor(p, seaRef);
    seaCol += pow(max(dot(seaNormal, sunDir), 0.0), 2.0);
    seaCol *= cshd;
    seaCol += 0.075*pow(vec3(0.1, 1.3, 4.0), vec3(max(dot(seaNormal, seaRef), 0.0)));

    float spe = pow(max(dot(sunDir, reflect(rd, normal)), 0.0), 3.0);
    float fre = pow(1.0-dot(normal, -rd), 2.0);

    col = seaCol;


    const float level = 0.00;
    const float level2 = 0.3;
    // REALLY fake shadows and lighting
    vec3 scol = sunCol1*(smoothstep(level, level2, hih) - smoothstep(level, level2, loh2));
    col = mix(vec3(1.0), col, exp(-17.0*(hih-0.25*loh)));
    col = mix(vec3(.75), col, exp(-10.0*loh*(max(d-ud, 0.0))));
    col += scol;

    col += vec3(0.5)*spe*fre;

    float ssd = (shipHeight - ro.y)/rd.y;

    col += shipColor((ro + rd*ssd).xz);

    col = mix(col, skyCol, smoothstep(0.5*MAX_DISTANCE, 1.*MAX_DISTANCE, d));

  } else {
    col = skyCol;
  }

//  col += vec3(1.1, 0.0, 0.0)* smoothstep(0.25, 1.0,(float(max_iter)/float(MAX_ITER)));
  return col;
}

vec3 postProcess(in vec3 col, in vec2 q)
{
  //col = saturate(col);
  col=pow(clamp(col,0.0,1.0),vec3(0.75));
  col=col*0.6+0.4*col*col*(3.0-2.0*col);  // contrast
  col=mix(col, vec3(dot(col, vec3(0.33))), -0.4);  // satuation
  col*=0.5+0.5*pow(19.0*q.x*q.y*(1.0-q.x)*(1.0-q.y),0.7);  // vigneting
  return col;
}

vec3 getSample1(vec2 p, float time) {
  vec3 ro  = vec3(0.5, 5.5, -2.0);
  vec3 la  = ro + vec3(0.0, -1.+0.9*TIME/PERIOD,  1.0);

  vec3 ww = normalize(la - ro);
  vec3 uu = normalize(cross(vec3(0.0,1.0,0.0), ww));
  vec3 vv = normalize(cross(ww, uu));
  vec3 rd = normalize(p.x*uu + p.y*vv + 2.0*ww);

  vec3 col = getColor(ro, rd)  ;

  return col;

}

vec3 getSample2(vec2 p, float time) {
  p.y-=time*0.25;
  float h = height(p, 0.0);
  vec3 n = normal(p, 0.0);

  vec3 lp = vec3(10.0, -1.2, 0.0);

  vec3 ld = normalize(vec3(p.x, h, p.y)- lp);

  float d = max(dot(ld, n), 0.0);

  vec3 col = vec3(0.0);

  col = vec3(1.0)*(h+0.1);
  col += vec3(1.5)*pow(d, 0.75);

  return col;

}

void mainImage(out vec4 fragColor, vec2 fragCoord) {
  vec2 q = fragCoord.xy/iResolution.xy;
  vec2 p = -1.0 + 2.0*q;
  p.x *= iResolution.x/iResolution.y;

  vec3 col = getSample1(p, TIME);

  col = postProcess(col, q);

  col *= smoothstep(0.0, 2.0, TIME);
  col *= 1.0-smoothstep(PERIOD-2.0, PERIOD, TIME);

  fragColor = vec4(col, 1.0);
}
