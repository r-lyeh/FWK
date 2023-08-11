// GJK distance algorithm. original code by @vurtun and @randygaul, public domain.
// [src] https://gist.github.com/vurtun/29727217c269a2fbf4c0ed9a1d11cb40
// - rlyeh, public domain.

/*
    Gilbert–Johnson–Keerthi (GJK) 3D distance algorithm
    The Gilbert–Johnson–Keerthi (GJK) distance algorithm is a method of determining
    the minimum distance between two convex sets. The algorithm's stability, speed
    which operates in near-constant time, and small storage footprint make it
    popular for realtime collision detection.

    Unlike many other distance algorithms, it has no requirments on geometry data
    to be stored in any specific format, but instead relies solely on a support
    function to iteratively generate closer simplices to the correct answer using
    the Minkowski sum (CSO) of two convex shapes.

    GJK algorithms are used incrementally. In this mode, the final simplex from a
    previous solution is used as the initial guess in the next iteration. If the
    positions in the new frame are close to those in the old frame, the algorithm
    will converge in one or two iterations.
*/

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

#ifdef GJK_C
//#pragma once
#include <math.h>
#include <assert.h>

#define GJK_FLT_MAX FLT_MAX // 3.40282347E+38F
#define GJK_EPSILON FLT_EPSILON // 1.19209290E-07F

float gjk_inv_sqrt(float n) {
    union {unsigned u; float f;} conv; conv.f = n;
    conv.u = 0x5f375A84 - (conv.u >> 1);
    conv.f = conv.f * (1.5f - (n * 0.5f * conv.f * conv.f));
    return conv.f;
}

int gjk(gjk_simplex *s, const gjk_support *sup, vec3 *dv) {
    assert(s);
    assert(dv);
    assert(sup);
    if (!s || !sup || !dv) return 0;
    if (s->max_iter > 0 && s->iter >= s->max_iter)
        return 0;

    /* I.) Initialize */
    if (s->cnt == 0) {
        s->D = GJK_FLT_MAX;
        s->max_iter = !s->max_iter ? GJK_MAX_ITERATIONS: s->max_iter;
    }
    /* II.) Check for duplications */
    for (int i = 0; i < s->cnt; ++i) {
        if (sup->aid != s->v[i].aid) continue;
        if (sup->bid != s->v[i].bid) continue;
        return 0;
    }
    /* III.) Add vertex into simplex */
    gjk_vertex *vert = &s->v[s->cnt];
    vert->a = sup->a;
    vert->b = sup->b;
    vert->p = *dv;
    vert->aid = sup->aid;
    vert->bid = sup->bid;
    s->bc[s->cnt++] = 1.0f;

    /* IV.) Find closest simplex point */
    switch (s->cnt) {
    case 1: break;
    case 2: {
        /* -------------------- Line ----------------------- */
        vec3 a = s->v[0].p;
        vec3 b = s->v[1].p;

        /* compute barycentric coordinates */
        vec3 ab = sub3(a, b);
        vec3 ba = sub3(b, a);

        float u = dot3(b, ba);
        float v = dot3(a, ab);
        if (v <= 0.0f) {
            /* region A */
            s->bc[0] = 1.0f;
            s->cnt = 1;
            break;
        }
        if (u <= 0.0f) {
            /* region B */
            s->v[0] = s->v[1];
            s->bc[0] = 1.0f;
            s->cnt = 1;
            break;
        }
        /* region AB */
        s->bc[0] = u;
        s->bc[1] = v;
        s->cnt = 2;
    } break;
    case 3: {
        /* -------------------- Triangle ----------------------- */
        vec3 a = s->v[0].p;
        vec3 b = s->v[1].p;
        vec3 c = s->v[2].p;

        vec3 ab = sub3(a, b);
        vec3 ba = sub3(b, a);
        vec3 bc = sub3(b, c);
        vec3 cb = sub3(c, b);
        vec3 ca = sub3(c, a);
        vec3 ac = sub3(a, c);

        /* compute barycentric coordinates */
        float u_ab = dot3(b, ba);
        float v_ab = dot3(a, ab);

        float u_bc = dot3(c, cb);
        float v_bc = dot3(b, bc);

        float u_ca = dot3(a, ac);
        float v_ca = dot3(c, ca);

        if (v_ab <= 0.0f && u_ca <= 0.0f) {
            /* region A */
            s->bc[0] = 1.0f;
            s->cnt = 1;
            break;
        }
        if (u_ab <= 0.0f && v_bc <= 0.0f) {
            /* region B */
            s->v[0] = s->v[1];
            s->bc[0] = 1.0f;
            s->cnt = 1;
            break;
        }
        if (u_bc <= 0.0f && v_ca <= 0.0f) {
            /* region C */
            s->v[0] = s->v[2];
            s->bc[0] = 1.0f;
            s->cnt = 1;
            break;
        }
        /* calculate fractional area */
        vec3 n; n = cross3(ba, ca);
        vec3 n1; n1 = cross3(b, c);
        vec3 n2; n2 = cross3(c, a);
        vec3 n3; n3 = cross3(a, b);

        float u_abc = dot3(n1, n);
        float v_abc = dot3(n2, n);
        float w_abc = dot3(n3, n);

        if (u_ab > 0.0f && v_ab > 0.0f && w_abc <= 0.0f) {
            /* region AB */
            s->bc[0] = u_ab;
            s->bc[1] = v_ab;
            s->cnt = 2;
            break;
        }
        if (u_bc > 0.0f && v_bc > 0.0f && u_abc <= 0.0f) {
            /* region BC */
            s->v[0] = s->v[1];
            s->v[1] = s->v[2];
            s->bc[0] = u_bc;
            s->bc[1] = v_bc;
            s->cnt = 2;
            break;
        }
        if (u_ca > 0.0f && v_ca > 0.0f && v_abc <= 0.0f) {
            /* region CA */
            s->v[1] = s->v[0];
            s->v[0] = s->v[2];
            s->bc[0] = u_ca;
            s->bc[1] = v_ca;
            s->cnt = 2;
            break;
        }
        /* region ABC */
        assert(u_abc > 0.0f && v_abc > 0.0f && w_abc > 0.0f);
        s->bc[0] = u_abc;
        s->bc[1] = v_abc;
        s->bc[2] = w_abc;
        s->cnt = 3;
    } break;
    case 4: {
        /* -------------------- Tetrahedron ----------------------- */
        vec3 a = s->v[0].p;
        vec3 b = s->v[1].p;
        vec3 c = s->v[2].p;
        vec3 d = s->v[3].p;

        vec3 ab = sub3(a, b);
        vec3 ba = sub3(b, a);
        vec3 bc = sub3(b, c);
        vec3 cb = sub3(c, b);
        vec3 ca = sub3(c, a);
        vec3 ac = sub3(a, c);

        vec3 db = sub3(d, b);
        vec3 bd = sub3(b, d);
        vec3 dc = sub3(d, c);
        vec3 cd = sub3(c, d);
        vec3 da = sub3(d, a);
        vec3 ad = sub3(a, d);

        /* compute barycentric coordinates */
        float u_ab = dot3(b, ba);
        float v_ab = dot3(a, ab);

        float u_bc = dot3(c, cb);
        float v_bc = dot3(b, bc);

        float u_ca = dot3(a, ac);
        float v_ca = dot3(c, ca);

        float u_bd = dot3(d, db);
        float v_bd = dot3(b, bd);

        float u_dc = dot3(c, cd);
        float v_dc = dot3(d, dc);

        float u_ad = dot3(d, da);
        float v_ad = dot3(a, ad);

        /* check verticies for closest point */
        if (v_ab <= 0.0f && u_ca <= 0.0f && v_ad <= 0.0f) {
            /* region A */
            s->bc[0] = 1.0f;
            s->cnt = 1;
            break;
        }
        if (u_ab <= 0.0f && v_bc <= 0.0f && v_bd <= 0.0f) {
            /* region B */
            s->v[0] = s->v[1];
            s->bc[0] = 1.0f;
            s->cnt = 1;
            break;
        }
        if (u_bc <= 0.0f && v_ca <= 0.0f && u_dc <= 0.0f) {
            /* region C */
            s->v[0] = s->v[2];
            s->bc[0] = 1.0f;
            s->cnt = 1;
            break;
        }
        if (u_bd <= 0.0f && v_dc <= 0.0f && u_ad <= 0.0f) {
            /* region D */
            s->v[0] = s->v[3];
            s->bc[0] = 1.0f;
            s->cnt = 1;
            break;
        }
        /* calculate fractional area */
        vec3 n; n = cross3(da, ba);
        vec3 n1; n1 = cross3(d, b);
        vec3 n2; n2 = cross3(b, a);
        vec3 n3; n3 = cross3(a, d);

        float u_adb = dot3(n1, n);
        float v_adb = dot3(n2, n);
        float w_adb = dot3(n3, n);

        n = cross3(ca, da);
        n1 = cross3(c, d);
        n2 = cross3(d, a);
        n3 = cross3(a, c);

        float u_acd = dot3(n1, n);
        float v_acd = dot3(n2, n);
        float w_acd = dot3(n3, n);

        n = cross3(bc, dc);
        n1 = cross3(b, d);
        n2 = cross3(d, c);
        n3 = cross3(c, b);

        float u_cbd = dot3(n1, n);
        float v_cbd = dot3(n2, n);
        float w_cbd = dot3(n3, n);

        n = cross3(ba, ca);
        n1 = cross3(b, c);
        n2 = cross3(c, a);
        n3 = cross3(a, b);

        float u_abc = dot3(n1, n);
        float v_abc = dot3(n2, n);
        float w_abc = dot3(n3, n);

        /* check edges for closest point */
        if (w_abc <= 0.0f && v_adb <= 0.0f && u_ab > 0.0f && v_ab > 0.0f) {
            /* region AB */
            s->bc[0] = u_ab;
            s->bc[1] = v_ab;
            s->cnt = 2;
            break;
        }
        if (u_abc <= 0.0f && w_cbd <= 0.0f && u_bc > 0.0f && v_bc > 0.0f) {
            /* region BC */
            s->v[0] = s->v[1];
            s->v[1] = s->v[2];
            s->bc[0] = u_bc;
            s->bc[1] = v_bc;
            s->cnt = 2;
            break;
        }
        if (v_abc <= 0.0f && w_acd <= 0.0f && u_ca > 0.0f && v_ca > 0.0f) {
            /* region CA */
            s->v[1] = s->v[0];
            s->v[0] = s->v[2];
            s->bc[0] = u_ca;
            s->bc[1] = v_ca;
            s->cnt = 2;
            break;
        }
        if (v_cbd <= 0.0f && u_acd <= 0.0f && u_dc > 0.0f && v_dc > 0.0f) {
            /* region DC */
            s->v[0] = s->v[3];
            s->v[1] = s->v[2];
            s->bc[0] = u_dc;
            s->bc[1] = v_dc;
            s->cnt = 2;
            break;
        }
        if (v_acd <= 0.0f && w_adb <= 0.0f && u_ad > 0.0f && v_ad > 0.0f) {
            /* region AD */
            s->v[1] = s->v[3];
            s->bc[0] = u_ad;
            s->bc[1] = v_ad;
            s->cnt = 2;
            break;
        }
        if (u_cbd <= 0.0f && u_adb <= 0.0f && u_bd > 0.0f && v_bd > 0.0f) {
            /* region BD */
            s->v[0] = s->v[1];
            s->v[1] = s->v[3];
            s->bc[0] = u_bd;
            s->bc[1] = v_bd;
            s->cnt = 2;
            break;
        }
        /* calculate fractional volume (volume can be negative!) */
        float denom = dot3(cross3(cb, ab), db);        // box3(cb, ab, db)
        float volume = (denom == 0) ? 1.0f: 1.0f/denom;
        float u_abcd = dot3(cross3(c, d), b) * volume; // box3(c, d, b)
        float v_abcd = dot3(cross3(c, a), d) * volume; // box3(c, a, d)
        float w_abcd = dot3(cross3(d, a), b) * volume; // box3(d, a, b)
        float x_abcd = dot3(cross3(b, a), c) * volume; // box3(b, a, c)

        /* check faces for closest point */
        if (x_abcd <= 0.0f && u_abc > 0.0f && v_abc > 0.0f && w_abc > 0.0f) {
            /* region ABC */
            s->bc[0] = u_abc;
            s->bc[1] = v_abc;
            s->bc[2] = w_abc;
            s->cnt = 3;
            break;
        }
        if (u_abcd <= 0.0f && u_cbd > 0.0f && v_cbd > 0.0f && w_cbd > 0.0f) {
            /* region CBD */
            s->v[0] = s->v[2];
            s->v[2] = s->v[3];
            s->bc[0] = u_cbd;
            s->bc[1] = v_cbd;
            s->bc[2] = w_cbd;
            s->cnt = 3;
            break;
        }
        if (v_abcd <= 0.0f && u_acd > 0.0f && v_acd > 0.0f && w_acd > 0.0f) {
            /* region ACD */
            s->v[1] = s->v[2];
            s->v[2] = s->v[3];
            s->bc[0] = u_acd;
            s->bc[1] = v_acd;
            s->bc[2] = w_acd;
            s->cnt = 3;
            break;
        }
        if (w_abcd <= 0.0f && u_adb > 0.0f && v_adb > 0.0f && w_adb > 0.0f) {
            /* region ADB */
            s->v[2] = s->v[1];
            s->v[1] = s->v[3];
            s->bc[0] = u_adb;
            s->bc[1] = v_adb;
            s->bc[2] = w_adb;
            s->cnt = 3;
            break;
        }
        /* region ABCD */
        // assert(u_abcd > 0.0f && v_abcd > 0.0f && w_abcd > 0.0f && x_abcd > 0.0f); // tcc+linux asserts in here: both u_abcd and v_abcd are negative
        s->bc[0] = u_abcd;
        s->bc[1] = v_abcd;
        s->bc[2] = w_abcd;
        s->bc[3] = x_abcd;
        s->cnt = 4;
    } break;}

    /* V.) Check if origin is enclosed by tetrahedron */
    if (s->cnt == 4) {
        s->hit = 1;
        return 0;
    }
    /* VI.) Ensure closing in on origin to prevent multi-step cycling */
    vec3 pnt; float denom = 0;
    for (int i = 0; i < s->cnt; ++i)
        denom += s->bc[i];
    denom = 1.0f / denom;

    switch (s->cnt) {
    case 1: pnt = s->v[0].p; break;
    case 2: {
        /* --------- Line -------- */
        vec3 a = scale3(s->v[0].p, denom * s->bc[0]);
        vec3 b = scale3(s->v[1].p, denom * s->bc[1]);
        pnt = add3(a, b);
    } break;
    case 3: {
        /* ------- Triangle ------ */
        vec3 a = scale3(s->v[0].p, denom * s->bc[0]);
        vec3 b = scale3(s->v[1].p, denom * s->bc[1]);
        vec3 c = scale3(s->v[2].p, denom * s->bc[2]);

        pnt = add3(a, b);
        pnt = add3(pnt, c);
    } break;
    case 4: {
        /* ----- Tetrahedron ----- */
        vec3 a = scale3(s->v[0].p, denom * s->bc[0]);
        vec3 b = scale3(s->v[1].p, denom * s->bc[1]);
        vec3 c = scale3(s->v[2].p, denom * s->bc[2]);
        vec3 d = scale3(s->v[3].p, denom * s->bc[3]);

        pnt = add3(a, b);
        pnt = add3(pnt, c);
        pnt = add3(pnt, d);
    } break;}

    float d2 = dot3(pnt, pnt);
    if (d2 >= s->D) return 0;
    s->D = d2;

    /* VII.) New search direction */
    switch (s->cnt) {
    default: assert(0); break;
    case 1: {
        /* --------- Point -------- */
        *dv = scale3(s->v[0].p, -1);
    } break;
    case 2: {
        /* ------ Line segment ---- */
        vec3 ba = sub3(s->v[1].p, s->v[0].p);
        vec3 b0 = scale3(s->v[1].p, -1);
        vec3 t;  t = cross3(ba, b0);
        *dv = cross3(t, ba);
    } break;
    case 3: {
        /* ------- Triangle ------- */
        vec3 ab = sub3(s->v[1].p, s->v[0].p);
        vec3 ac = sub3(s->v[2].p, s->v[0].p);
        vec3 n;  n = cross3(ab, ac);
        if (dot3(n, s->v[0].p) <= 0.0f)
            *dv = n;
        else *dv = scale3(n, -1);
    }}
    if (dot3(*dv,*dv) < GJK_EPSILON * GJK_EPSILON)
        return 0;
    return 1;
}
gjk_result gjk_analyze(const gjk_simplex *s) {
    gjk_result r = {0}, *res = &r;

    res->iterations = s->iter;
    res->hit = s->hit;

    /* calculate normalization denominator */
    float denom = 0;
    for (int i = 0; i < s->cnt; ++i)
        denom += s->bc[i];
    denom = 1.0f / denom;

    /* compute closest points */
    switch (s->cnt) {
    default: assert(0); break;
    case 1: {
        /* Point */
        res->p0 = s->v[0].a;
        res->p1 = s->v[0].b;
    } break;
    case 2: {
        /* Line */
        float as = denom * s->bc[0];
        float bs = denom * s->bc[1];

        vec3 a = scale3(s->v[0].a, as);
        vec3 b = scale3(s->v[1].a, bs);
        vec3 c = scale3(s->v[0].b, as);
        vec3 d = scale3(s->v[1].b, bs);

        res->p0 = add3(a, b);
        res->p1 = add3(c, d);
    } break;
    case 3: {
        /* Triangle */
        float as = denom * s->bc[0];
        float bs = denom * s->bc[1];
        float cs = denom * s->bc[2];

        vec3 a = scale3(s->v[0].a, as);
        vec3 b = scale3(s->v[1].a, bs);
        vec3 c = scale3(s->v[2].a, cs);

        vec3 d = scale3(s->v[0].b, as);
        vec3 e = scale3(s->v[1].b, bs);
        vec3 f = scale3(s->v[2].b, cs);

        res->p0 = add3(a, b);
        res->p0 = add3(res->p0, c);

        res->p1 = add3(d, e);
        res->p1 = add3(res->p1, f);
    } break;
    case 4: {
        /* Tetrahedron */
        vec3 a = scale3(s->v[0].a, denom * s->bc[0]);
        vec3 b = scale3(s->v[1].a, denom * s->bc[1]);
        vec3 c = scale3(s->v[2].a, denom * s->bc[2]);
        vec3 d = scale3(s->v[3].a, denom * s->bc[3]);

        res->p0 = add3(a, b);
        res->p0 = add3(res->p0, c);
        res->p0 = add3(res->p0, d);
        res->p1 = res->p0;
    } break;}

    if (!res->hit) {
        /* compute distance */
        vec3 d= sub3(res->p1, res->p0);
        res->distance_squared = dot3(d, d);
    } else res->distance_squared = 0;
    return r;
}
gjk_result gjk_quad(float a_radius, float b_radius) {
    gjk_result r = {0}, *res = &r;
    float radius = a_radius + b_radius;
    float radius_squared = radius * radius;
    if (res->distance_squared > GJK_EPSILON &&
        res->distance_squared > radius_squared) {
        res->distance_squared -= radius_squared;

        /* calculate normal */
        vec3 n = sub3(res->p1, res->p0);
        float l2 = dot3(n, n);
        if (l2 != 0.0f) {
            float il = gjk_inv_sqrt(l2);
            n = scale3(n,il);
        }
        vec3 da = scale3(n, a_radius);
        vec3 db = scale3(n, b_radius);

        /* calculate new collision points */
        res->p0 = add3(res->p0, da);
        res->p1 = sub3(res->p1, db);
    } else {
        vec3 p = add3(res->p0, res->p1);
        res->p0 = scale3(p, 0.5f);
        res->p1 = res->p0;
        res->distance_squared = 0;
        res->hit = 1;
    }
    return r;
}

#endif
