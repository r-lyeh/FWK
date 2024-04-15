// -----------------------------------------------------------------------------
// original code by @vurtun (PD) and @barerose (CC0).
// [src] https://gist.github.com/vurtun/95f088e4889da2474ad1ce82d7911fee
// - rlyeh, public domain.

typedef struct line     { vec3 a, b;                                                  } line;
typedef struct sphere   { vec3 c; float r;                                            } sphere;
typedef struct aabb     { vec3 min, max;                                              } aabb;
typedef struct plane    { vec3 p, n;                                                  } plane;
typedef struct capsule  { vec3 a, b; float r;                                         } capsule;
typedef struct ray      { vec3 p, d;                                                  } ray;
typedef struct triangle { vec3 p0,p1,p2;                                              } triangle;
typedef union  frustum  { struct { vec4 l, r, t, b, n, f; }; vec4 pl[6]; float v[24]; } frustum;

#define line(...)       C_CAST(line, __VA_ARGS__)
#define sphere(...)     C_CAST(sphere, __VA_ARGS__)
#define aabb(...)       C_CAST(aabb, __VA_ARGS__)
#define plane(...)      C_CAST(plane, __VA_ARGS__)
#define capsule(...)    C_CAST(capsule, __VA_ARGS__)
#define ray(p,normdir)  C_CAST(ray, p, normdir)
#define triangle(...)   C_CAST(triangle, __VA_ARGS__)
#define frustum(...)    C_CAST(frustum, __VA_ARGS__)

// ----------------------------------------------------------------------------

typedef struct hit {
    union {
        // general case
        float depth;
        // rays only: penetration (t0) and extraction (t1) points along ray line
        struct { float t0, t1; };
        // gjk only
        struct { int hits; vec3 p0, p1; float distance2; int iterations; };
    };
    union { vec3 p; vec3 contact_point; };
    union { vec3 n; vec3 normal; };
} hit;

#define hit(...)        C_CAST(hit, __VA_ARGS__)

// ----------------------------------------------------------------------------

/* line/segment */
API float   line_distance2_point(line l, vec3 p);
API vec3    line_closest_point(line l, vec3 p);
/* ray */
API float   ray_test_plane(ray r, vec4 p4);
API float   ray_test_triangle(ray r, triangle t);
API int     ray_test_sphere(float *t0, float *t1, ray r, sphere s);
API int     ray_test_aabb(float *t0, float *t1, ray r, aabb a);
API hit*    ray_hit_plane(ray r, plane p);
API hit*    ray_hit_triangle(ray r, triangle t);
API hit*    ray_hit_sphere(ray r, sphere s);
API hit*    ray_hit_aabb(ray r, aabb a);
/* sphere */
API vec3    sphere_closest_point(sphere s, vec3 p);
API hit*    sphere_hit_aabb(sphere s, aabb a);
API hit*    sphere_hit_capsule(sphere s, capsule c);
API hit*    sphere_hit_sphere(sphere a, sphere b);
API int     sphere_test_aabb(sphere s, aabb a);
API int     sphere_test_capsule(sphere s, capsule c);
API int     sphere_test_sphere(sphere a, sphere b);
/* aabb */
API vec3    aabb_closest_point(aabb a, vec3 p);
API float   aabb_distance2_point(aabb a, vec3 p);
API int     aabb_contains_point(aabb a, vec3 p);
API hit*    aabb_hit_aabb(aabb a, aabb b);
API hit*    aabb_hit_capsule(aabb a, capsule c);
API hit*    aabb_hit_sphere(aabb a, sphere s);
API int     aabb_test_aabb(aabb a, aabb b);
API int     aabb_test_capsule(aabb a, capsule c);
API int     aabb_test_sphere(aabb a, sphere s);
/* capsule */
API float   capsule_distance2_point(capsule c, vec3 p);
API vec3    capsule_closest_point(capsule c, vec3 p);
API hit*    capsule_hit_aabb(capsule c, aabb a);
API hit*    capsule_hit_capsule(capsule a, capsule b);
API hit*    capsule_hit_sphere(capsule c, sphere s);
API int     capsule_test_aabb(capsule c, aabb a);
API int     capsule_test_capsule(capsule a, capsule b);
API int     capsule_test_sphere(capsule c, sphere s);

API vec4    plane4(vec3 p, vec3 n);

API frustum frustum_build(mat44 projview);
API int     frustum_test_sphere(frustum f, sphere s);
API int     frustum_test_aabb(frustum f, aabb a);

API void    collide_demo(); // debug draw collisions
