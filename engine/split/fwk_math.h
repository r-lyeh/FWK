// -----------------------------------------------------------------------------
// math framework: rand, ease, vec2, vec3, vec4, quat, mat2, mat33, mat34, mat4
// - rlyeh, public domain
//
// Credits: @ands+@krig+@vurtun (PD), @datenwolf (WTFPL2), @evanw+@barerose (CC0), @sgorsten (Unlicense).

#define C_EPSILON  (1e-6)
#define C_PI       (3.141592654f) // (3.14159265358979323846f)
#define TO_RAD     (C_PI/180.f)
#define TO_DEG     (180.f/C_PI)

// ----------------------------------------------------------------------------

//#define ptr(type)         0[&(type).x]
#define vec2i(x, y     )  C_CAST(vec2i,(int)(x),   (int)(y)                          )
#define vec3i(x, y, z  )  C_CAST(vec3i,(int)(x),   (int)(y),   (int)(z)              )
#define vec2(x, y      )  C_CAST(vec2, (float)(x), (float)(y)                        )
#define vec3(x, y, z   )  C_CAST(vec3, (float)(x), (float)(y), (float)(z),           )
#define vec4(x, y, z, w)  C_CAST(vec4, (float)(x), (float)(y), (float)(z), (float)(w))
#define quat(x, y, z, w)  C_CAST(quat, (float)(x), (float)(y), (float)(z), (float)(w))
#define axis(x, y, z)     C_CAST(axis, (float)(x), (float)(y), (float)(z))
#define mat33(...)        C_CAST(mat33, __VA_ARGS__ )
#define mat34(...)        C_CAST(mat34, __VA_ARGS__ )
#define mat44(...)        C_CAST(mat44, __VA_ARGS__ )

typedef union vec2i{ struct { int X,Y; };       struct { int x,y; }; struct { int r,g; }; struct { int w,h; }; struct { int min,max; }; struct { int from,to; }; struct { int src,dst; }; int v2[2]; int array[1]; } vec2i;
typedef union vec3i{ struct { int X,Y,Z; };     struct { int x,y,z; }; struct { int r,g,b; }; struct { int w,h,d; }; struct { int min,max; }; struct { int from,to,step; }; struct { int src,dst; }; int v3[3]; int array[1]; } vec3i;
typedef union vec2 { struct { float X,Y; };     struct { float x,y; }; struct { float r,g; }; struct { float w,h; }; struct { float min,max; }; struct { float from,to; }; struct { float src,dst; }; float v2[2]; float array[1]; } vec2;
typedef union vec3 { struct { float X,Y,Z; };   struct { float x,y,z; }; struct { float r,g,b; }; struct { float min,max; }; struct { float from,to; }; vec2 xy; vec2 rg; vec2 wh; float v3[3]; float array[1]; } vec3;
typedef union vec4 { struct { float X,Y,Z,W; }; struct { float x,y,z,w; }; struct { float r,g,b,a; }; struct { float min,max; }; struct { float from,to; }; vec2 xy; vec3 xyz; vec2 rg; vec3 rgb; vec2 wh; vec3 whd; float v4[4]; float array[1]; } vec4;
typedef union quat { struct { float X,Y,Z,W; }; struct { float x,y,z,w; }; vec3 xyz; vec4 xyzw; float v4[4]; float array[1]; } quat;
typedef float mat33[9];
typedef float mat34[12];
typedef float mat44[16];

// ----------------------------------------------------------------------------

API void     randset(uint64_t state);
API uint64_t rand64(void);
API double   randf(void); // [0, 1) interval
API int      randi(int mini, int maxi); // [mini, maxi) interval
//API double rng(void); // [0..1) Lehmer RNG "minimal standard"

// ----------------------------------------------------------------------------

API float simplex1( float x );
API float simplex2( vec2 xy );
API float simplex3( vec3 xyz );
API float simplex4( vec4 xyzw );

// ----------------------------------------------------------------------------

API float ease_linear(float t);

API float ease_out_sine(float t);
API float ease_out_quad(float t);
API float ease_out_cubic(float t);
API float ease_out_quart(float t);
API float ease_out_quint(float t);
API float ease_out_expo(float t);
API float ease_out_circ(float t);
API float ease_out_back(float t);
API float ease_out_elastic(float t);
API float ease_out_bounce(float t);

API float ease_in_sine(float t);
API float ease_in_quad(float t);
API float ease_in_cubic(float t);
API float ease_in_quart(float t);
API float ease_in_quint(float t);
API float ease_in_expo(float t);
API float ease_in_circ(float t);
API float ease_in_back(float t);
API float ease_in_elastic(float t);
API float ease_in_bounce(float t);

API float ease_inout_sine(float t);
API float ease_inout_quad(float t);
API float ease_inout_cubic(float t);
API float ease_inout_quart(float t);
API float ease_inout_quint(float t);
API float ease_inout_expo(float t);
API float ease_inout_circ(float t);
API float ease_inout_back(float t);
API float ease_inout_elastic(float t);
API float ease_inout_bounce(float t);

API float ease_inout_perlin(float t);

enum EASE_FLAGS {
    EASE_LINEAR,
    EASE_SINE,
    EASE_QUAD,
    EASE_CUBIC,
    EASE_QUART,
    EASE_QUINT,
    EASE_EXPO,
    EASE_CIRC,
    EASE_BACK,
    EASE_ELASTIC,
    EASE_BOUNCE,

    EASE_IN,
    EASE_INOUT = EASE_IN * 2,
    EASE_OUT = 0,
};

API float ease(float t01, unsigned mode); // 0=linear,1=out_sine...31=inout_perlin

API float ease_ping_pong(float t, float(*fn1)(float), float(*fn2)(float));
API float ease_pong_ping(float t, float(*fn1)(float), float(*fn2)(float));

// ----------------------------------------------------------------------------

API float deg      (float radians);
API float rad      (float degrees);

API int   mini     (int    a, int    b);
API int   maxi     (int    a, int    b);
API int   absi     (int    a          );
API int   clampi   (int v,int a,int b);

API float minf     (float  a, float  b);
API float maxf     (float  a, float  b);
API float absf     (float  a          );
API float pmodf    (float  a, float  b);
API float signf    (float  a)           ;
API float clampf   (float v,float a,float b);
API float mixf     (float a,float b,float t);
API float fractf   (float a);

// ----------------------------------------------------------------------------

API vec2  ptr2     (const float *a    );
//
API vec2  neg2     (vec2   a          );
API vec2  add2     (vec2   a, vec2   b);
API vec2  sub2     (vec2   a, vec2   b);
API vec2  mul2     (vec2   a, vec2   b);
API vec2  div2     (vec2   a, vec2   b);
API vec2  inc2     (vec2   a, float  b);
API vec2  dec2     (vec2   a, float  b);
API vec2  scale2   (vec2   a, float  b);
API vec2  pmod2    (vec2   a, float  b);
API vec2  min2     (vec2   a, vec2   b);
API vec2  max2     (vec2   a, vec2   b);
API vec2  abs2     (vec2   a          );
API vec2  floor2   (vec2   a          );
API vec2  fract2   (vec2   a          );
API vec2  ceil2    (vec2   a          );
API float dot2     (vec2   a, vec2   b);
API vec2  refl2    (vec2   a, vec2   b);
API float cross2   (vec2   a, vec2   b);
API float len2sq   (vec2   a          );
API float len2     (vec2   a          );
API vec2  norm2    (vec2   a          );
API int   finite2  (vec2   a          );
API vec2  mix2  (vec2 a,vec2 b,float t);
API vec2  clamp2(vec2 v,float a,float b);
// ----------------------------------------------------------------------------

API vec3  rnd3     (void); // @todo: rnd2,rnd4,rndq
API vec3  ptr3     (const float *a    );
API vec3  vec23    (vec2   a, float z );
//
API vec3  neg3     (vec3   a          );
API vec3  add3     (vec3   a, vec3   b);
API vec3  sub3     (vec3   a, vec3   b);
API vec3  mul3     (vec3   a, vec3   b);
API vec3  div3     (vec3   a, vec3   b);
API vec3  inc3     (vec3   a, float  b);
API vec3  dec3     (vec3   a, float  b);
API vec3  scale3   (vec3   a, float  b);
API vec3  pmod3    (vec3   a, float  b);
API vec3  min3     (vec3   a, vec3   b);
API vec3  max3     (vec3   a, vec3   b);
API vec3  abs3     (vec3   a          );
API vec3  floor3   (vec3   a          );
API vec3  fract3   (vec3   a          );
API vec3  ceil3    (vec3   a          );
API vec3  cross3   (vec3   a, vec3   b);
API float dot3     (vec3   a, vec3   b);
API vec3  refl3    (vec3   a, vec3   b);
API float len3sq   (vec3   a          );
API float len3     (vec3   a          );
API vec3  norm3    (vec3   a          );
API vec3  norm3sq  (vec3   a          );
API int   finite3  (vec3   a          );
API vec3  mix3  (vec3 a,vec3 b,float t);
API vec3  clamp3(vec3 v,float a,float b);
//vec3 tricross3 (vec3 a, vec3 b, vec3 c);
API void  ortho3   (vec3 *left, vec3 *up, vec3 v);

API vec3  rotatex3 (vec3 dir, float degrees);
API vec3  rotatey3 (vec3 dir, float degrees);
API vec3  rotatez3 (vec3 dir, float degrees);

// ----------------------------------------------------------------------------

API vec4  ptr4     (const float *a    );
API vec4  vec34    (vec3   a, float w );
//
API vec4  neg4     (vec4   a          );
API vec4  add4     (vec4   a, vec4   b);
API vec4  sub4     (vec4   a, vec4   b);
API vec4  mul4     (vec4   a, vec4   b);
API vec4  div4     (vec4   a, vec4   b);
API vec4  inc4     (vec4   a, float  b);
API vec4  dec4     (vec4   a, float  b);
API vec4  scale4   (vec4   a, float  b);
API vec4  pmod4    (vec4   a, float  b);
API vec4  min4     (vec4   a, vec4   b);
API vec4  max4     (vec4   a, vec4   b);
API vec4  abs4     (vec4   a          );
API vec4  floor4   (vec4   a          );
API vec4  fract4   (vec4   a          );
API vec4  ceil4    (vec4   a          );
API float dot4     (vec4   a, vec4   b);
API vec4  refl4    (vec4   a, vec4   b);
API float len4sq   (vec4   a          );
API float len4     (vec4   a          );
API vec4  norm4    (vec4   a          );
API vec4  norm4sq  (vec4   a          );
API int   finite4  (vec4   a          );
API vec4  mix4  (vec4 a,vec4 b,float t);
API vec4  clamp4(vec4 v,float a,float b);
// vec4 cross4(vec4 v0, vec4 v1);

// ----------------------------------------------------------------------------

API quat  idq      (                  );
API quat  ptrq     (const float *a    );
API quat  vec3q    (vec3   a, float w );
API quat  vec4q    (vec4   a          );
//
API quat  negq     (quat   a          );
API quat  conjq    (quat   a          );
API quat  addq     (quat   a, quat   b);
API quat  subq     (quat   a, quat   b);
API quat  mulq     (quat   p, quat   q);
API quat  scaleq   (quat   a, float  s);
API quat  normq    (quat   a          );
API float dotq     (quat   a, quat   b);
API quat  mixq(quat a, quat b, float t);
/* quat lerpq(quat a, quat b, float s);
    return norm(quat((1-s)*a.x + s*b.x, (1-s)*a.y + s*b.y, (1-s)*a.z + s*b.z, (1-s)*a.w + s*b.w));
}*/
API quat slerpq(quat a, quat b, float s);

API quat  rotationq(float deg,float x,float y,float z);
API quat  mat44q   (mat44 M);

API vec3 rotate3q_2(vec3 v, quat q);
API vec3 rotate3q(vec3 v, quat r);

// euler <-> quat
API vec3  euler    (quat q);
API quat  eulerq   (vec3 pyr_degrees);

// ----------------------------------------------------------------------------

API void scaling33(mat33 m, float x, float y, float z);
API void scale33(mat33 m, float x, float y, float z);
API void id33(mat33 m);
API void extract33(mat33 m, const mat44 m4);
API void copy33(mat33 m, const mat33 a);//
API vec3 mulv33(mat33 m, vec3 v);
API void multiply33x2(mat33 m, const mat33 a, const mat33 b);

API void rotation33(mat33 m, float degrees, float x,float y,float z);
API void rotationq33(mat33 m, quat q);
API void rotate33(mat33 r, float degrees, float x,float y,float z);
API void compose33(mat33 m, quat r, vec3 s);

// ----------------------------------------------------------------------------

API void id34(mat34 m);
API void copy34(mat34 m, const mat34 a);
API void scale34(mat34 m, float s);
API void add34(mat34 m, mat34 n);
API void muladd34(mat34 m, mat34 n, float s);
API void add34x2(mat34 m, mat34 n, mat34 o);
API void lerp34(mat34 m, mat34 n, mat34 o, float alpha); // mix34?
API void multiply34x2(mat34 m, const mat34 m0, const mat34 m1);
API void multiply34(mat34 m, const mat34 a);
API void multiply34x3(mat34 m, const mat34 a, const mat34 b, const mat34 c);
API void compose34(mat34 m, vec3 t, quat q, vec3 s);
API void invert34(mat34 m, const mat34 o);

// ----------------------------------------------------------------------------

API void id44(mat44 m);
API void identity44(mat44 m);
API void copy44(mat44 m, const mat44 a);
API void multiply44x2(mat44 m, const mat44 a, const mat44 b);
API void multiply44x3(mat44 m, const mat44 a, const mat44 b, const mat44 c);
API void multiply44(mat44 m, const mat44 a);
// ---
API void ortho44(mat44 m, float l, float r, float b, float t, float n, float f);
API void frustum44(mat44 m, float l, float r, float b, float t, float n, float f);
API void perspective44(mat44 m, float fovy_degrees, float aspect, float nearp, float farp);
API void lookat44(mat44 m, vec3 eye, vec3 center, vec3 up);
// ---
API void translation44(mat44 m, float x, float y, float z);
API void translate44(mat44 m, float x, float y, float z);
API void relocate44(mat44 m, float x, float y, float z);
API void rotationq44(mat44 m, quat q);
API void rotation44(mat44 m, float degrees, float x, float y, float z);
API void rotate44(mat44 m, float degrees, float x, float y, float z);
API void scaling44(mat44 m, float x, float y, float z);
API void scale44(mat44 m, float x, float y, float z);
// ---
API void transpose44(mat44 m, const mat44 a);
API float det44(const mat44 M);
API bool invert44(mat44 T, const mat44 M);

API void compose44(mat44 m, vec3 t, quat q, vec3 s);

// ----------------------------------------------------------------------------

API vec3 transformq(const quat q, const vec3 v);
API vec3 transform33(const mat33 m, vec3 p);
API vec3 transform344(const mat44 m, const vec3 p);
API vec4 transform444(const mat44 m, const vec4 p);
API bool unproject44(vec3 *out, vec3 xyd, vec4 viewport, mat44 mvp);

// ----------------------------------------------------------------------------
// !!! for debugging

API void print2( vec2 v );
API void print3( vec3 v );
API void print4( vec4 v );
API void printq( quat q );
API void print33( float *m );
API void print34( float *m );
API void print44( float *m );
