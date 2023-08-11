// -----------------------------------------------------------------------------
// original code by @vurtun (PD) and @barerose (CC0).
// [src] https://gist.github.com/vurtun/95f088e4889da2474ad1ce82d7911fee
// - rlyeh, public domain.

#ifndef GJK_H
#define GJK_H

#define GJK_MAX_ITERATIONS 20

typedef struct gjk_support {
    int aid, bid;
    vec3 a;
    vec3 b;
} gjk_support;
typedef struct gjk_vertex {
    vec3 a;
    vec3 b;
    vec3 p;
    int aid, bid;
} gjk_vertex;
typedef struct gjk_simplex {
    int max_iter, iter;
    int hit, cnt;
    gjk_vertex v[4];
    float bc[4], D;
} gjk_simplex;
typedef struct gjk_result {
    int hit;
    vec3 p0;
    vec3 p1;
    float distance_squared;
    int iterations;
} gjk_result;

int gjk(gjk_simplex *s, const gjk_support *sup, vec3 *dv);
gjk_result gjk_analyze(const gjk_simplex *s);
gjk_result gjk_quad(float a_radius, float b_radius);

#endif

//typedef struct gjk_result gjk_result;

typedef struct line     { vec3 a, b;                                                  } line;
typedef struct sphere   { vec3 c; float r;                                            } sphere;
typedef struct aabb     { vec3 min, max;                                              } aabb;
typedef struct plane    { vec3 p, n;                                                  } plane;
typedef struct capsule  { vec3 a, b; float r;                                         } capsule;
typedef struct ray      { vec3 p, d;                                                  } ray;
typedef struct triangle { vec3 p0,p1,p2;                                              } triangle;
typedef struct poly     { vec3* verts; int cnt;                                       } poly;
typedef union  frustum  { struct { vec4 l, r, t, b, n, f; }; vec4 pl[6]; float v[24]; } frustum;

#define line(...)       C_CAST(line, __VA_ARGS__)
#define sphere(...)     C_CAST(sphere, __VA_ARGS__)
#define aabb(...)       C_CAST(aabb, __VA_ARGS__)
#define plane(...)      C_CAST(plane, __VA_ARGS__)
#define capsule(...)    C_CAST(capsule, __VA_ARGS__)
#define ray(p,normdir)  C_CAST(ray, p, normdir)
#define triangle(...)   C_CAST(triangle, __VA_ARGS__)
#define poly(...)       C_CAST(poly, __VA_ARGS__)
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
API int     sphere_test_poly(sphere s, poly p);
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
API int     aabb_test_poly(aabb a, poly p);
API int     aabb_test_sphere(aabb a, sphere s);
/* capsule */
API float   capsule_distance2_point(capsule c, vec3 p);
API vec3    capsule_closest_point(capsule c, vec3 p);
API hit*    capsule_hit_aabb(capsule c, aabb a);
API hit*    capsule_hit_capsule(capsule a, capsule b);
API hit*    capsule_hit_sphere(capsule c, sphere s);
API int     capsule_test_aabb(capsule c, aabb a);
API int     capsule_test_capsule(capsule a, capsule b);
API int     capsule_test_poly(capsule c, poly p);
API int     capsule_test_sphere(capsule c, sphere s);
/* poly: query */
API int     poly_test_sphere(poly p, sphere s);
API int     poly_test_aabb(poly p, aabb a);
API int     poly_test_capsule(poly p, capsule c);
API int     poly_test_poly(poly a, poly b);
/* poly: query transformed */
API int     poly_test_sphere_transform(poly p, vec3 pos3, mat33 rot33, sphere s);
API int     poly_test_aabb_transform(poly p, vec3 apos3, mat33 arot33, aabb a);
API int     poly_test_capsule_transform(poly p, vec3 pos3, mat33 rot33, capsule c);
API int     poly_test_poly_transform(poly a, vec3 apos3, mat33 arot33, poly b, vec3 bpos3, mat33 brot33);
/* poly: gjk result */
API int     poly_hit_sphere(struct gjk_result *res, poly p, sphere s);
API int     poly_hit_aabb(struct gjk_result *res, poly p, aabb a);
API int     poly_hit_capsule(struct gjk_result *res, poly p, capsule c);
API int     poly_hit_poly(struct gjk_result *res, poly a, poly b);
/* poly: gjk result transformed */
API int     poly_hit_sphere_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, sphere s);
API int     poly_hit_aabb_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, aabb a);
API int     poly_hit_capsule_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, capsule c);
API int     poly_hit_poly_transform(struct gjk_result *res, poly a, vec3 at3, mat33 ar33, poly b, vec3 bt3, mat33 br33);

API vec4    plane4(vec3 p, vec3 n);

API frustum frustum_build(mat44 projview);
API int     frustum_test_sphere(frustum f, sphere s);
API int     frustum_test_aabb(frustum f, aabb a);

API poly    poly_alloc(int cnt);
API void    poly_free(poly *p);

API poly    pyramid(vec3 from, vec3 to, float size); // poly_free() required
API poly    diamond(vec3 from, vec3 to, float size); // poly_free() required

API void    collide_demo(); // debug draw collisions
