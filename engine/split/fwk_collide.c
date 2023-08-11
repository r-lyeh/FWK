/* poly */
poly poly_alloc(int cnt) {
    poly p = {0};
    p.cnt = cnt;
    p.verts = REALLOC(p.verts, sizeof(p.verts[0]) * cnt); // array_resize(p.verts, cnt);
    return p;
}

void poly_free(poly *p) {
    REALLOC(p->verts, 0); // array_free(p->verts);
    poly z = {0};
    *p = z;
}

/* plane */
vec4 plane4(vec3 p, vec3 n) {
    return vec34(n, -dot3(n,p));
}

/* pyramid */
poly pyramid(vec3 from, vec3 to, float size) {
    /* calculate axis */
    vec3 up, right, forward = norm3( sub3(to, from) );
    ortho3(&right, &up, forward);

    /* calculate extend */
    vec3 xext = scale3(right, size);
    vec3 yext = scale3(up, size);
    vec3 nxext = scale3(right, -size);
    vec3 nyext = scale3(up, -size);

    /* calculate base vertices */
    poly p = {0};
    p.verts = REALLOC(p.verts, sizeof(p.verts[0]) * (5+1)); p.cnt = 5; /*+1 for diamond case*/ // array_resize(p.verts, 5+1); p.cnt = 5;
    p.verts[0] = add3(add3(from, xext), yext); /*a*/
    p.verts[1] = add3(add3(from, xext), nyext); /*b*/
    p.verts[2] = add3(add3(from, nxext), nyext); /*c*/
    p.verts[3] = add3(add3(from, nxext), yext); /*d*/
    p.verts[4] = to; /*r*/
    return p;
}

/* pyramid */
poly diamond(vec3 from, vec3 to, float size) {
    vec3 mid = add3(from, scale3(sub3(to, from), 0.5f));
    poly p = pyramid(mid, to, size);
    p.verts[5] = from; p.cnt = 6;
    return p;
}

// ---

static void transform_(vec3 *r, vec3 v, const float *r33, vec3 t3) {
    for (int i = 0; i < 3; ++i) {
        i[&r->x]  = i[&v.x] * r33[i*3+0];
        i[&r->x] += i[&v.x] * r33[i*3+1];
        i[&r->x] += i[&v.x] * r33[i*3+2];
        i[&r->x] += i[&t3.x];
    }
}
static void transformS(vec3 *v, const float *r33, vec3 t3) {
    vec3 tmp = *v;
    transform_(v, tmp, r33, t3);
}
static void transformT(vec3 *r, vec3 v, const float *r33, vec3 t3) {
    for (int i = 0; i < 3; ++i) {
        float p = i[&v.x] - i[&t3.x];
        i[&r->x]  = p * r33[0*3+i];
        i[&r->x] += p * r33[1*3+i];
        i[&r->x] += p * r33[2*3+i];
    }
}
static void transformST(vec3 *v, const float *r33, vec3 t3) {
    vec3 tmp = *v;
    transformT(v, tmp, r33, t3);
}

/* ============================================================================
 *
 *                                  COLLISION
 *
 * =========================================================================== */

static __thread hit hits[16] = {0};
static __thread int hit_index = -1;
#define hit_next() &hits[ (++hit_index) & 15 ]

static float line_closest_line_(float *t1, float *t2, vec3 *c1, vec3 *c2, line l, line m) {
    vec3 r, d1, d2;
    d1 = sub3(l.b, l.a); /* direction vector segment s1 */
    d2 = sub3(m.b, m.a); /* direction vector segment s2 */
    r = sub3(l.a, m.a);

    float i = dot3(d1, d1);
    float e = dot3(d2, d2);
    float f = dot3(d2, r);

    if (i <= C_EPSILON && e <= C_EPSILON) {
        /* both segments degenerate into points */
        vec3 d12;
        *t1 = *t2 = 0.0f;
        *c1 = l.a;
        *c2 = m.a;
        d12 = sub3(*c1, *c2);
        return dot3(d12,d12);
    }
    if (i > C_EPSILON) {
        float c = dot3(d1,r);
        if (e > C_EPSILON) {
            /* non-degenerate case */
            float b = dot3(d1,d2);
            float denom = i*e - b*b;

            /* compute closest point on L1/L2 if not parallel else pick any t2 */
            if (denom != 0.0f)
                *t1 = clampf(0.0f, (b*f - c*e) / denom, 1.0f);
            else *t1 = 0.0f;

            /* cmpute point on L2 closest to S1(s) */
            *t2 = (b*(*t1) + f) / e;
            if (*t2 < 0.0f) {
                *t2 = 0.0f;
                *t1 = clampf(0.0f, -c/i, 1.0f);
            } else if (*t2 > 1.0f) {
                *t2 = 1.0f;
                *t1 = clampf(0.0f, (b-c)/i, 1.0f);
            }
        } else {
            /* second segment degenerates into a point */
            *t1 = clampf(0.0f, -c/i, 1.0f);
            *t2 = 0.0f;
        }
    } else {
        /* first segment degenerates into a point */
        *t2 = clampf(0.0f, f/e, 1.0f);
        *t1 = 0.0f;
    }
    /* calculate closest points */
    vec3 n, d12;
    n = scale3(d1, *t1);
    *c1 = add3(l.a, n);
    n = scale3(d2, *t2);
    *c2 = add3(m.a, n);

    /* calculate squared distance */
    d12 = sub3(*c1, *c2);
    return dot3(d12,d12);
}

vec3 line_closest_point(line l, vec3 p) {
    vec3 ab = sub3(l.b,l.a), pa = sub3(p,l.a);
    float t = dot3(pa,ab) / dot3(ab,ab);
    return add3(l.a, scale3(ab, t < 0 ? 0 : t > 1 ? 1 : t));
}
float line_distance2_point(line l, vec3 p) {
    vec3 ab = sub3(l.a,l.b), ap = sub3(l.a,p), bp = sub3(l.b,p);
    /* handle cases p proj outside ab */
    float e = dot3(ap,ab); if (e <= 0) return dot3(ap,ap);
    float f = dot3(ab,ab); if (e >= f) return dot3(bp,bp);
    return dot3(ap,ap) - (e*e)/f;
}
float ray_test_plane(ray r, vec4 plane) {
    /* Ray: P = origin + rd * t
     * Plane: plane_normal * P + d = 0
     *
     * Substitute:
     *      normal * (origin + rd*t) + d = 0
     *
     * Solve for t:
     *      plane_normal * origin + plane_normal * rd*t + d = 0
     *      -(plane_normal*rd*t) = plane_normal * origin + d
     *
     *                  plane_normal * origin + d
     *      t = -1 * -------------------------
     *                  plane_normal * rd
     *
     * Result:
     *      Behind: t < 0
     *      Infront: t >= 0
     *      Parallel: t = 0
     *      Intersection point: ro + rd * t
     */
    vec3 p = ptr3(&plane.x);
    float n = -(dot3(p,r.p) + plane.w);
    if (fabs(n) < 0.0001f) return 0.0f;
    return n/(dot3(p,r.d));
}
float ray_test_triangle(ray r, triangle tr) {
    float t = 0;
    vec3 di0, di1, di2;
    vec3 d21, d02, in;
    vec3 n, d10, d20;
    vec3 in0, in1, in2;

    /* calculate triangle normal */
    d10 = sub3(tr.p1,tr.p0);
    d20 = sub3(tr.p2,tr.p0);
    d21 = sub3(tr.p2,tr.p1);
    d02 = sub3(tr.p0,tr.p2);
    n = cross3(d10,d20);

    /* check for plane intersection */
    vec4 p = plane4(tr.p0, n);
    t = ray_test_plane(r, p);
    if (t <= 0.0f) return t;

    /* intersection point */
    in = scale3(r.d,t);
    in = add3(in,r.p);

    /* check if point inside triangle in plane */
    di0 = sub3(in, tr.p0);
    di1 = sub3(in, tr.p1);
    di2 = sub3(in, tr.p2);

    in0 = cross3(d10, di0);
    in1 = cross3(d21, di1);
    in2 = cross3(d02, di2);

    if (dot3(in0,n) < 0.0f)
        return -1;
    if (dot3(in1,n) < 0.0f)
        return -1;
    if (dot3(in2,n) < 0.0f)
        return -1;
    return t;
}
int ray_test_sphere(float *t0, float *t1, ray r, sphere s) {
    vec3 a;
    float tc,td,d2,r2;
    a = sub3(s.c,r.p);
    tc = dot3(r.d,a);
    if (tc < 0) return 0;

    r2 = s.r*s.r;
    d2 = dot3(a,a) - tc*tc;
    if (d2 > r2) return 0;
    td = sqrtf(r2 - d2);

    *t0 = tc - td;
    *t1 = tc + td;
    return 1;
}
int ray_test_aabb(float *t0, float *t1, ray r, aabb a) {
    float t0x = (a.min.x - r.p.x) / r.d.x;
    float t0y = (a.min.y - r.p.y) / r.d.y;
    float t0z = (a.min.z - r.p.z) / r.d.z;
    float t1x = (a.max.x - r.p.x) / r.d.x;
    float t1y = (a.max.y - r.p.y) / r.d.y;
    float t1z = (a.max.z - r.p.z) / r.d.z;

    float tminx = minf(t0x, t1x);
    float tminy = minf(t0y, t1y);
    float tminz = minf(t0z, t1z);
    float tmaxx = maxf(t0x, t1x);
    float tmaxy = maxf(t0y, t1y);
    float tmaxz = maxf(t0z, t1z);
    if (tminx > tmaxy || tminy > tmaxx)
        return 0;

    *t0 = maxf(tminx, tminy);
    *t1 = minf(tmaxy, tmaxx);
    if (*t0 > tmaxz || tminz> *t1)
        return 0;

    *t0 = maxf(*t0, tminz);
    *t1 = minf(*t1, tmaxz);
    return 1;
}
vec3 sphere_closest_point(sphere s, vec3 p) {
    vec3 d = norm3(sub3(p, s.c));
    return add3(s.c, scale3(d,s.r));
}
int sphere_test_sphere(sphere a, sphere b) {
    vec3 d = sub3(b.c, a.c);
    float r = a.r + b.r;
    if (dot3(d,d) > r*r)
        return 0;
    return 1;
}
hit *sphere_hit_sphere(sphere a, sphere b) {
    vec3 d = sub3(b.c, a.c);
    float r = a.r + b.r;
    float d2 = dot3(d,d);
    if (d2 > r*r) return 0;

    hit *m = hit_next();
    float l = sqrtf(d2);
    float linv = 1.0f / ((l != 0) ? l: 1.0f);
    m->normal = scale3(d, linv);
    m->depth = r - l;
    d = scale3(m->normal, b.r);
    m->contact_point = sub3(b.c, d);
    return m;
}
int sphere_test_aabb(sphere s, aabb a) {
    return aabb_test_sphere(a, s);
}
hit *sphere_hit_aabb(sphere s, aabb a) {
    /* find closest aabb point to sphere center point */
    vec3 ap = aabb_closest_point(a, s.c);
    vec3 d = sub3(s.c, ap);
    float d2 = dot3(d, d);
    if (d2 > s.r*s.r) return 0;

    hit *m = hit_next();
    /* calculate distance vector between sphere and aabb center points */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));
    d = sub3(ac, s.c);

    /* normalize distance vector */
    float l2 = dot3(d,d);
    float l = l2 != 0.0f ? sqrtf(l2): 1.0f;
    float linv = 1.0f/l;
    d = scale3(d, linv);

    m->normal = d;
    m->contact_point = scale3(m->normal, s.r);
    m->contact_point = add3(s.c, m->contact_point);

    /* calculate penetration depth */
    vec3 sp = sphere_closest_point(s, ap);
    d = sub3(sp, ap);
    m->depth = sqrtf(dot3(d,d)) - l;
    return m;
}
int sphere_test_capsule(sphere s, capsule c) {
    return capsule_test_sphere(c, s);
}
hit *sphere_hit_capsule(sphere s, capsule c) {
#if 0
        // original code
        /* find closest capsule point to sphere center point */
        hit *m = hit_next();
        vec3 cp = capsule_closest_point(c, s.c);
        m->normal = sub3(cp, s.c);
        float d2 = dot3(m->normal, m->normal);
        if (d2 > s.r*s.r) return 0;

        /* normalize hit normal vector */
        m->normal = norm3(m->normal);

        /* calculate penetration depth */
        m->depth = d2 - s.r*s.r;
        m->depth = m->depth != 0.0f ? sqrtf(m->depth): 0.0f;
        m->contact_point = add3(s.c, scale3(m->normal, s.r));
        return m;
#else
        // aproximation of I would expect this function to return instead
        vec3 l = sub3(c.a,c.b); float len = len3(l);
        vec3 d = norm3(l);
        ray r = ray(add3(c.a,scale3(d,-2*len)), d);
        s.r += c.r;
        hit *h = ray_hit_sphere(r, s);
        if(!h) return 0;
        s.r -= c.r;
        h->contact_point = add3(s.c,scale3(norm3(sub3(h->contact_point,s.c)),s.r));
        return h;
#endif
}
int sphere_test_poly(sphere s, poly p) {
    return poly_test_sphere(p, s);
}
void aabb_rebalance_transform(aabb *b, aabb a, mat33 m, vec3 t) {
    for (int i = 0; i < 3; ++i) {
        i[&b->min.x] = i[&b->max.x] = i[&t.x];
        for (int j = 0; j < 3; ++j) {
            float e = m[i*3+j] * j[&a.min.x];
            float f = m[i*3+j] * j[&a.max.x];
            if (e < f) {
                i[&b->min.x] += e;
                i[&b->max.x] += f;
            } else {
                i[&b->min.x] += f;
                i[&b->max.x] += e;
            }
        }
    }
}
vec3 aabb_closest_point(aabb a, vec3 p) {
    vec3 res;
    for (int i = 0; i < 3; ++i) {
        float v = i[&p.x];
        if (v < i[&a.min.x]) v = i[&a.min.x];
        if (v > i[&a.max.x]) v = i[&a.max.x];
        i[&res.x] = v;
    }
    return res;
}
float aabb_distance2_point(aabb a, vec3 p) {
    float r = 0;
    for (int i = 0; i < 3; ++i) {
        float v = i[&p.x];
        if (v < i[&a.min.x]) r += (i[&a.min.x]-v) * (i[&a.min.x]-v);
        if (v > i[&a.max.x]) r += (v-i[&a.max.x]) * (v-i[&a.max.x]);
    } return r;
}
int aabb_contains_point(aabb a, vec3 p) {
    if (p.x < a.min.x || p.x > a.max.x) return 0;
    if (p.y < a.min.y || p.y > a.max.y) return 0;
    if (p.z < a.min.z || p.z > a.max.z) return 0;
    return 1;
}
int aabb_test_aabb(aabb a, aabb b) {
    if (a.max.x < b.min.x || a.min.x > b.max.x) return 0;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return 0;
    if (a.max.z < b.min.z || a.min.z > b.max.z) return 0;
    return 1;
}
hit *aabb_hit_aabb(aabb a, aabb b) {
    if (!aabb_test_aabb(a, b))
        return 0;

    hit *m = hit_next();
    /* calculate distance vector between both aabb center points */
    vec3 ac, bc, d;
    ac = sub3(a.max, a.min);
    bc = sub3(b.max, b.min);

    ac = scale3(ac, 0.5f);
    bc = scale3(bc, 0.5f);

    ac = add3(a.min, ac);
    bc = add3(b.min, bc);
    d = sub3(bc, ac);

    /* normalize distance vector */
    float l2 = dot3(d,d);
    float l = l2 != 0.0f ? sqrtf(l2): 1.0f;
    float linv = 1.0f/l;
    d = scale3(d, linv);

    /* calculate contact point */
    m->normal = d;
    m->contact_point = aabb_closest_point(a, bc);
    d = sub3(m->contact_point, ac);

    /* calculate penetration depth */
    float r2 = dot3(d,d);
    float r = sqrtf(r2);
    m->depth = r - l;
    return m;
}
int aabb_test_sphere(aabb a, sphere s) {
    /* compute squared distance between sphere center and aabb */
    float d2 = aabb_distance2_point(a, s.c);
    /* intersection if distance is smaller/equal sphere radius*/
    return d2 <= s.r*s.r;
}
hit *aabb_hit_sphere(aabb a, sphere s) {
    /* find closest aabb point to sphere center point */
    hit *m = hit_next();
    m->contact_point = aabb_closest_point(a, s.c);
    vec3 d = sub3(s.c, m->contact_point);
    float d2 = dot3(d, d);
    if (d2 > s.r*s.r) return 0;

    /* calculate distance vector between aabb and sphere center points */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));
    d = sub3(s.c, ac);

    /* normalize distance vector */
    float l2 = dot3(d,d);
    float l = l2 != 0.0f ? sqrtf(l2): 1.0f;
    float linv = 1.0f/l;
    d = scale3(d, linv);

    /* calculate penetration depth */
    m->normal = d;
    d = sub3(m->contact_point, ac);
    m->depth = sqrtf(dot3(d,d));
    return m;
}
int aabb_test_capsule(aabb a, capsule c) {
    return capsule_test_aabb(c, a);
}
hit *aabb_hit_capsule(aabb a, capsule c) {
    /* calculate aabb center point */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));

    /* calculate closest point from aabb to point on capsule and check if inside aabb */
    vec3 cp = capsule_closest_point(c, ac);
    if (!aabb_contains_point(a, cp))
        return 0;

    hit *m = hit_next();
    /* vector and distance between both capsule closests point and aabb center*/
    vec3 d; float d2;
    d = sub3(cp, ac);
    d2 = dot3(d,d);

    /* calculate penetration depth from closest aabb point to capsule */
    vec3 ap = aabb_closest_point(a, cp);
    vec3 dt = sub3(ap, cp);
    m->depth = sqrtf(dot3(dt,dt));

    /* calculate normal */
    float l = sqrtf(d2);
    float linv = 1.0f / ((l != 0.0f) ? l: 1.0f);
    m->normal = scale3(d, linv);
    m->contact_point = ap;
    return m;
}
int aabb_test_poly(aabb a, poly p) {
    return poly_test_aabb(p, a);
}
float capsule_distance2_point(capsule c, vec3 p) {
    float d2 = line_distance2_point(line(c.a,c.b), p);
    return d2 - (c.r*c.r);
}
vec3 capsule_closest_point(capsule c, vec3 p) {
    /* calculate closest point to internal capsule segment */
    vec3 pp = line_closest_point(line(c.a,c.b), p);

    /* extend point out by radius in normal direction */
    vec3 d = norm3(sub3(p,pp));
    return add3(pp, scale3(d, c.r));
}
int capsule_test_capsule(capsule a, capsule b) {
    float t1, t2;
    vec3 c1, c2;
    float d2 = line_closest_line_(&t1, &t2, &c1, &c2, line(a.a,a.b), line(b.a,b.b));
    float r = a.r + b.r;
    return d2 <= r*r;
}
hit *capsule_hit_capsule(capsule a, capsule b) {
    float t1, t2;
    vec3 c1, c2;
    float d2 = line_closest_line_(&t1, &t2, &c1, &c2, line(a.a,a.b), line(b.a,b.b));
    float r = a.r + b.r;
    if (d2 > r*r) return 0;

    hit *m = hit_next();
    /* calculate normal from both closest points for each segement */
    vec3 cp, d;
    m->normal = sub3(c2, c1);
    m->normal = norm3(m->normal);

    /* calculate contact point from closest point and depth */
    m->contact_point = capsule_closest_point(a, c2);
    cp = capsule_closest_point(b, c1);
    d = sub3(c1, cp);
    m->depth = sqrtf(dot3(d,d));
    return m;
}
int capsule_test_sphere(capsule c, sphere s) {
    /* squared distance bwetween sphere center and capsule line segment */
    float d2 = line_distance2_point(line(c.a,c.b), s.c);
    float r = s.r + c.r;
    return d2 <= r * r;
}
hit *capsule_hit_sphere(capsule c, sphere s) {
    /* find closest capsule point to sphere center point */
    hit *m = hit_next();
    m->contact_point = capsule_closest_point(c, s.c);
    m->normal = sub3(s.c, m->contact_point);
    float d2 = dot3(m->normal, m->normal);
    if (d2 > s.r*s.r) return 0;

    /* normalize hit normal vector */
    float l = d2 != 0.0f ? sqrtf(d2): 1;
    float linv = 1.0f/l;
    m->normal = scale3(m->normal, linv);

    /* calculate penetration depth */
    m->depth = d2 - s.r*s.r;
    m->depth = m->depth != 0.0f ? sqrtf(m->depth): 0.0f;
    return m;
}
int capsule_test_aabb(capsule c, aabb a) {
    /* calculate aabb center point */
    vec3 ac = scale3(sub3(a.max, a.min), 0.5f);

    /* calculate closest point from aabb to point on capsule and check if inside aabb */
    vec3 p = capsule_closest_point(c, ac);
    return aabb_contains_point(a, p);
}
hit *capsule_hit_aabb(capsule c, aabb a) {
    /* calculate aabb center point */
    vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));

    /* calculate closest point from aabb to point on capsule and check if inside aabb */
    vec3 cp = capsule_closest_point(c, ac);
    if (!aabb_contains_point(a, cp))
        return 0;

    hit *m = hit_next();
    /* vector and distance between both capsule closests point and aabb center*/
    vec3 d; float d2;
    d = sub3(ac, cp);
    d2 = dot3(d,d);

    /* calculate penetration depth from closest aabb point to capsule */
    vec3 ap = aabb_closest_point(a, cp);
    vec3 dt = sub3(ap, cp);
    m->depth = sqrtf(dot3(dt,dt));

    /* calculate normal */
    float l = sqrtf(d2);
    float linv = 1.0f / ((l != 0.0f) ? l: 1.0f);
    m->normal = scale3(d, linv);
    m->contact_point = cp;
    return m;
}
int capsule_test_poly(capsule c, poly p) {
    return poly_test_capsule(p, c);
}
int line_support(vec3 *support, vec3 d, vec3 a, vec3 b) {
    int i = 0;
    float adot = dot3(a, d);
    float bdot = dot3(b, d);
    if (adot < bdot) {
        *support = b;
        i = 1;
    } else *support = a;
    return i;
}
int poly_support(vec3 *support, vec3 d, poly p) {
    int imax = 0;
    float dmax = dot3(*p.verts, d);
    for (int i = 1; i < p.cnt; ++i) {
        /* find vertex with max dot product in direction d */
        float dot = dot3(p.verts[i], d);
        if (dot < dmax) continue;
        imax = i, dmax = dot;
    } *support = p.verts[imax];
    return imax;
}
int poly_hit_sphere(struct gjk_result *res,
    poly p,
    sphere s) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *p.verts;
    gs.b = s.c;
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        gs.aid = poly_support(&gs.a, n, p);
        d = sub3(gs.b, gs.a);
    }
    /* check distance between closest points */
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= s.r*s.r;
}
int poly_hit_sphere_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, sphere s) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *p.verts;
    gs.b = s.c;
    transformS(&gs.a, rot33, pos3);
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        vec3 da; transformT(&da, n, rot33, pos3);

        gs.aid = poly_support(&gs.a, da, p);
        transformS(&gs.a, rot33, pos3);
        d = sub3(gs.b, gs.a);
    }
    /* check distance between closest points */
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= s.r*s.r;
}
int poly_test_sphere(poly p, sphere s) {
    struct gjk_result res;
    return poly_hit_sphere(&res, p, s);
}
int poly_test_sphere_transform(poly p, vec3 pos3, mat33 rot33, sphere s) {
    struct gjk_result res;
    return poly_hit_sphere_transform(&res, p, pos3, rot33, s);
}
int poly_hit_capsule(struct gjk_result *res, poly p, capsule c) {
    /* initial guess */
    vec3 d = {0};
    gjk_support s = {0};
    s.a = *p.verts;
    s.b = c.a;
    d = sub3(s.b, s.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &s, &d)) {
        vec3 n = scale3(d, -1);
        s.aid = poly_support(&s.a, n, p);
        s.bid = line_support(&s.b, d, c.a, c.b);
        d = sub3(s.b, s.a);
    }
    /* check distance between closest points */
    assert(gsx.iter < gsx.max_iter);
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= c.r*c.r;
}
int poly_test_capsule(poly p, capsule c) {
    struct gjk_result res;
    return poly_hit_capsule(&res, p, c);
}
int poly_hit_capsule_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, capsule c) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *p.verts;
    gs.b = c.a;
    transformS(&gs.a, rot33, pos3);
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        vec3 da; transformT(&da, n, rot33, pos3);

        gs.aid = poly_support(&gs.a, da, p);
        gs.bid = line_support(&gs.b, d, c.a, c.b);
        transformS(&gs.a, rot33, pos3);
        d = sub3(gs.b, gs.a);
    }
    /* check distance between closest points */
    *res = gjk_analyze(&gsx);
    return res->distance_squared <= c.r*c.r;
}
int poly_test_capsule_transform(poly p, vec3 pos3, mat33 rot33, capsule c) {
    struct gjk_result res;
    return poly_hit_capsule_transform(&res, p, pos3, rot33, c);
}
int poly_hit_poly_transform(struct gjk_result *res,
    poly a, vec3 at3, mat33 ar33,
    poly b, vec3 bt3, mat33 br33) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *a.verts;
    gs.b = *b.verts;
    transformS(&gs.a, ar33, at3);
    transformS(&gs.b, br33, bt3);
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        /* transform direction */
        vec3 n = scale3(d, -1);
        vec3 da; transformT(&da, n, ar33, at3);
        vec3 db; transformT(&db, d, br33, bt3);
        /* run support function on tranformed directions  */
        gs.aid = poly_support(&gs.a, da, a);
        gs.bid = poly_support(&gs.b, db, b);
        /* calculate distance vector on transformed points */
        transformS(&gs.a, ar33, at3);
        transformS(&gs.b, br33, bt3);
        d = sub3(gs.b, gs.a);
    }
    *res = gjk_analyze(&gsx);
    return gsx.hit;
}
int poly_hit_poly(struct gjk_result *res,
    poly a,
    poly b) {
    /* initial guess */
    vec3 d = {0};
    gjk_support gs = {0};
    gs.a = *a.verts;
    gs.b = *b.verts;
    d = sub3(gs.b, gs.a);

    /* run gjk algorithm */
    gjk_simplex gsx = {0};
    while (gjk(&gsx, &gs, &d)) {
        vec3 n = scale3(d, -1);
        gs.aid = poly_support(&gs.a, n, a);
        gs.bid = poly_support(&gs.b, d, b);
        d = sub3(gs.b, gs.a);
    }
    *res = gjk_analyze(&gsx);
    return gsx.hit;
}
int poly_test_poly(poly a, poly b) {
    struct gjk_result res;
    return poly_hit_poly(&res, a, b);
}
int poly_test_poly_transform(poly a, vec3 apos3, mat33 arot33,
    poly b, vec3 bpos3, mat33 brot33) {
    struct gjk_result res;
    return poly_hit_poly_transform(&res, a, apos3, arot33, b, bpos3, brot33);
}
int poly_hit_aabb(struct gjk_result *res, poly p, aabb a) {
    vec3 box[8];
    box[0] = vec3(a.min.x, a.min.y, a.min.z),
    box[1] = vec3(a.min.x, a.min.y, a.max.z);
    box[2] = vec3(a.min.x, a.max.y, a.min.z);
    box[3] = vec3(a.min.x, a.max.y, a.max.z);
    box[4] = vec3(a.max.x, a.min.y, a.min.z);
    box[5] = vec3(a.max.x, a.min.y, a.max.z);
    box[6] = vec3(a.max.x, a.max.y, a.min.z);
    box[7] = vec3(a.max.x, a.max.y, a.max.z);
    return poly_hit_poly(res, p, poly(&box[0], 8));
}
int poly_hit_aabb_transform(struct gjk_result *res, poly p, vec3 pos3, mat33 rot33, aabb a) {
    vec3 zero = {0};
    vec3 id[3] = {{1,0,0},{0,1,0},{0,0,1}};
    vec3 box[8];
    box[0] = vec3(a.min.x, a.min.y, a.min.z),
    box[1] = vec3(a.min.x, a.min.y, a.max.z);
    box[2] = vec3(a.min.x, a.max.y, a.min.z);
    box[3] = vec3(a.min.x, a.max.y, a.max.z);
    box[4] = vec3(a.max.x, a.min.y, a.min.z);
    box[5] = vec3(a.max.x, a.min.y, a.max.z);
    box[6] = vec3(a.max.x, a.max.y, a.min.z);
    box[7] = vec3(a.max.x, a.max.y, a.max.z);
    return poly_hit_poly_transform(res, p, pos3, rot33, poly(&box[0], 8), zero, id[0].v3);
}
int poly_test_aabb(poly p, aabb a) {
    struct gjk_result res;
    return poly_hit_aabb(&res, p, a);
}
int poly_test_aabb_transform(poly p, vec3 apos3, mat33 arot33, aabb a) {
    struct gjk_result res;
    return poly_hit_aabb_transform(&res, p, apos3, arot33, a);
}

/* ============================================================================
 *
 *                              COLLISION VOLUME
 *
 * =========================================================================== */

hit *ray_hit_plane(ray r, plane p) {
    vec4 pf = plane4(p.p, p.n);
    float t = ray_test_plane(r, pf);
    if (t <= 0.0f) return 0;
    hit *o = hit_next();
    o->p = add3(r.p, scale3(r.d, t));
    o->t0 = o->t1 = t;
    o->n = scale3(p.n, -1.0f);
    return o;
}
hit *ray_hit_triangle(ray r, triangle tr) {
    float t = ray_test_triangle(r, tr);
    if (t <= 0) return 0;

    hit *o = hit_next();
    o->t0 = o->t1 = t;
    o->p = add3(r.p, scale3(r.d, t));
    o->n = norm3(cross3(sub3(tr.p1,tr.p0),sub3(tr.p2,tr.p0)));
    return o;
}
hit *ray_hit_sphere(ray r, sphere s) {
    hit *o = hit_next();
    if (!ray_test_sphere(&o->t0, &o->t1, r, s))
        return 0;
    o->p = add3(r.p, scale3(r.d, minf(o->t0,o->t1)));
    o->n = norm3(sub3(o->p, s.c));
    return o;
}
hit *ray_hit_aabb(ray r, aabb a) {
    hit *o = hit_next();

    vec3 pnt, ext, c;
    float d, min;
    if (!ray_test_aabb(&o->t0, &o->t1, r, a))
        return 0;

    o->p = add3(r.p, scale3(r.d, minf(o->t0,o->t1)));
    ext = sub3(a.max, a.min);
    c = add3(a.min, scale3(ext,0.5f));
    pnt = sub3(o->p, c);

    min = fabs(ext.x - fabs(pnt.x));
    o->n = scale3(vec3(1,0,0), signf(pnt.x));
    d = fabs(ext.y - fabs(pnt.y));
    if (d < min) {
        min = d;
        o->n = scale3(vec3(0,1,0), signf(pnt.y));
    }
    d = fabs(ext.z - fabs(pnt.z));
    if (d < min)
        o->n = scale3(vec3(0,0,1), signf(pnt.z));
    return o;
}

frustum frustum_build(mat44 pv) {
    frustum f;
    f.l = vec4(pv[ 3]+pv[ 0], pv[ 7]+pv[ 4], pv[11]+pv[ 8], pv[15]+pv[12]);
    f.r = vec4(pv[ 3]-pv[ 0], pv[ 7]-pv[ 4], pv[11]-pv[ 8], pv[15]-pv[12]);
    f.t = vec4(pv[ 3]-pv[ 1], pv[ 7]-pv[ 5], pv[11]-pv[ 9], pv[15]-pv[13]);
    f.b = vec4(pv[ 3]+pv[ 1], pv[ 7]+pv[ 5], pv[11]+pv[ 9], pv[15]+pv[13]);
    f.n = vec4(pv[ 3]+pv[ 2], pv[ 7]+pv[ 6], pv[11]+pv[10], pv[15]+pv[14]);
    f.f = vec4(pv[ 3]-pv[ 2], pv[ 7]-pv[ 6], pv[11]-pv[10], pv[15]-pv[14]);
    for (int i = 0; i < 6; i++) f.pl[i] = scale4(f.pl[i], 1 / len3(f.pl[i].xyz));
    return f;
}
int frustum_test_sphere(frustum f, sphere s) {
    for(int i = 0; i < 6; i++) {
        if((dot3(f.pl[i].xyz, s.c) + f.pl[i].w + s.r) < 0) return 0;
    }
    return 1;
}
int frustum_test_aabb(frustum f, aabb a) {
    for(int i = 0; i < 6; i++) {
        vec3 v = vec3(f.pl[i].x > 0 ? a.max.x : a.min.x, f.pl[i].y > 0 ? a.max.y : a.min.y, f.pl[i].z > 0 ? a.max.z : a.min.z);
        if((dot3(f.pl[i].xyz, v) + f.pl[i].w) < 0) return 0;
    }
    return 1;
}

void collide_demo() { // debug draw collisions // @fixme: fix leaks: poly_free()

    // animation
    static float dx = 0, dy = 0;
    float delta = (0.25f / 60.f);
    dx = dx + delta * 2.0f;
    dy = dy + delta * 0.8f;

#if 0
    // 3D
    glEnable(GL_DEPTH_TEST);

    // grid
    ddraw_grid(0);
#endif

    {
        // Triangle-Ray Intersection*/
        vec3 ro, rd;
        int suc;

        triangle tri = { vec3(-9,1,28), vec3(-10,0,28), vec3(-11,1,28) };

        // ray
        ro = vec3(-10,-1,20);
        rd = vec3(-10+0.4f*sin(dx), 2.0f*cos(dy), 29.81023f);
        rd = sub3(rd, ro);
        rd = norm3(rd);

        ray r = ray(ro, rd);
        hit *hit = ray_hit_triangle(r, tri);
        if (hit) {
            // point of intersection
            ddraw_color(RED);
            ddraw_box(hit->p, vec3(0.10f, 0.10f, 0.10f));

            // intersection normal
            ddraw_color(BLUE);
            vec3 v = add3(hit->p, hit->n);
            ddraw_arrow(hit->p, v);
        }

        // line
        ddraw_color(RED);
        rd = scale3(rd,10);
        rd = add3(ro,rd);
        ddraw_line(ro, rd);

        // triangle
        if (hit) ddraw_color(RED);
        else ddraw_color(WHITE);
        ddraw_triangle(tri.p0,tri.p1,tri.p2);
    }
    {
        // Plane-Ray Intersection*/
        vec3 ro, rd;
        mat33 rot;

        // ray
        static float d = 0;
        d += delta * 2.0f;
        ro = vec3(0,-1,20);
        rd = vec3(0.1f, 0.5f, 9.81023f);
        rd = sub3(rd, ro);
        rd = norm3(rd);

        // rotation
        rotation33(rot, deg(d), 0,1,0);
        rd = mulv33(rot, rd);

        // intersection
        ray r = ray(ro, rd);
        plane pl = plane(vec3(0,0,28), vec3(0,0,1));
        hit *hit = ray_hit_plane(r, pl);
        if (hit) {
            // point of intersection
            ddraw_color(RED);
            ddraw_box(hit->p, vec3(0.10f, 0.10f, 0.10f));

            // intersection normal
            ddraw_color(BLUE);
            vec3 v = add3(hit->p, hit->n);
            ddraw_arrow(hit->p, v);
            ddraw_color(RED);
        }
        // line
        ddraw_color(RED);
        rd = scale3(rd,9);
        rd = add3(ro,rd);
        ddraw_line(ro, rd);

        // plane
        if (hit) ddraw_color(RED);
        else ddraw_color(WHITE);
        ddraw_plane(vec3(0,0,28), vec3(0,0,1), 3.0f);
    }
    {
        // Sphere-Ray Intersection*/
        vec3 ro, rd;
        sphere s;

        // ray
        ro = vec3(0,-1,0);
        rd = vec3(0.4f*sin(dx), 2.0f*cos(dy), 9.81023f);
        rd = sub3(rd, ro);
        rd = norm3(rd);

        ray r = ray(ro, rd);
        s = sphere(vec3(0,0,8), 1);
        hit *hit = ray_hit_sphere(r, s);
        if(hit) {
            // points of intersection
            vec3 in = add3(ro,scale3(rd,hit->t0));

            ddraw_color(GREEN);
            ddraw_box(in, vec3(0.05f, 0.05f, 0.05f));

            in = add3(ro,scale3(rd,hit->t1));

            ddraw_color(YELLOW);
            ddraw_box(in, vec3(0.05f, 0.05f, 0.05f));

            // intersection normal
            ddraw_color(BLUE);
            vec3 v = add3(hit->p, hit->n);
            ddraw_arrow(hit->p, v);
            ddraw_color(RED);
        }
        // line
        ddraw_color(RED);
        rd = scale3(rd,10);
        rd = add3(ro,rd);
        ddraw_line(ro, rd);

        // sphere
        if (hit) ddraw_color(RED);
        else ddraw_color(WHITE);
        ddraw_sphere(vec3(0,0,8), 1);
    }
    {   // ray-aabb
        aabb bounds = aabb(vec3(10-0.5f,-0.5f,7.5f), vec3(10.5f,0.5f,8.5f));

        vec3 ro = vec3(10,-1,0);
        vec3 rd = vec3(10+0.4f*sin(dx), 2.0f*cos(dy), 9.81023f);
        rd = norm3(sub3(rd, ro));
        ray r = ray(ro, rd);

        hit *hit = ray_hit_aabb(r, bounds);
        if(hit) {
            // points of intersection
            vec3 in;
            in = scale3(rd,hit->t0);
            in = add3(ro,in);

            ddraw_color(RED);
            ddraw_box(in, vec3(0.05f, 0.05f, 0.05f));

            in = scale3(rd,hit->t1);
            in = add3(ro,in);

            ddraw_color(RED);
            ddraw_box(in, vec3(0.05f, 0.05f, 0.05f));

            // intersection normal
            ddraw_color(BLUE);
            vec3 v = add3(hit->p, hit->n);
            ddraw_arrow(hit->p, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);
        ddraw_box(vec3(10,0,8), vec3(1,1,1));

        // line
        ddraw_color(RED);
        rd = scale3(rd,10);
        rd = add3(ro,rd);
        ddraw_line(ro, rd);
    }
    {
        // Sphere-Sphere intersection*/
        sphere a = sphere(vec3(-10,0,8), 1);
        sphere b = sphere(vec3(-10+0.6f*sin(dx), 3.0f*cos(dy),8), 1);
        hit *m = sphere_hit_sphere(a, b);
        if (m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);

        ddraw_sphere(a.c, 1);
        ddraw_sphere(b.c, 1);
    }
    {
        // AABB-AABB intersection*/
        const float x = 10+0.6f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = 20.0f;

        aabb a = aabb(vec3(10-0.5f,-0.5f,20-0.5f), vec3(10+0.5f,0.5f,20.5f));
        aabb b = aabb(vec3(x-0.5f,y-0.5f,z-0.5f), vec3(x+0.5f,y+0.5f,z+0.5f));
        hit *m = aabb_hit_aabb(a, b);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);

        ddraw_box(vec3(10,0,20), vec3(1,1,1));
        ddraw_box(vec3(x,y,z), vec3(1,1,1));
    }
    {
        // Capsule-Capsule intersection*/
        const float x = 20+0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = 28.5f;

        capsule a = capsule(vec3(20.0f,-1.0f,28.0f), vec3(20.0f,1.0f,28.0f), 0.2f);
        capsule b = capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        hit *m = capsule_hit_capsule(a, b);
        if( m ) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);
        ddraw_capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        ddraw_capsule(vec3(20.0f,-1.0f,28.0f), vec3(20.0f,1.0f,28.0f), 0.2f);
    }
    {
        // AABB-Sphere intersection*/
        aabb a = aabb(vec3(20-0.5f,-0.5f,7.5f), vec3(20.5f,0.5f,8.5f));
        sphere s = sphere(vec3(20+0.6f*sin(dx), 3.0f*cos(dy),8), 1);
        hit *m  = aabb_hit_sphere(a, s);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);

        ddraw_box(vec3(20,0,8), vec3(1,1,1));
        ddraw_sphere(s.c, 1);
    }
    {
        // Sphere-AABB intersection*/
        const float x = 10+0.6f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -8.0f;

        sphere s = sphere(vec3(10,0,-8), 1);
        aabb a = aabb(vec3(x-0.5f,y-0.5f,z-0.5f), vec3(x+0.5f,y+0.5f,z+0.5f));
        hit *m = sphere_hit_aabb(s, a);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);

        ddraw_box(vec3(x,y,z), vec3(1,1,1));
        ddraw_sphere(s.c, 1);
    }
    {
        // Capsule-Sphere intersection*/
        capsule c = capsule(vec3(-20.5f,-1.0f,7.5f), vec3(-20+0.5f,1.0f,8.5f), 0.2f);
        sphere b = sphere(vec3(-20+0.6f*sin(dx), 3.0f*cos(dy),8), 1);
        hit *m = capsule_hit_sphere(c, b);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);
        ddraw_sphere(b.c, 1);
        ddraw_capsule(vec3(-20.5f,-1.0f,7.5f), vec3(-20+0.5f,1.0f,8.5f), 0.2f);
    }
    {
        // Sphere-Capsule intersection*/
        const float x = 20+0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -8;

        sphere s = sphere(vec3(20,0,-8), 1);
        capsule c = capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        hit *m = sphere_hit_capsule(s, c);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);

        ddraw_capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        ddraw_sphere(s.c, 1);
    }
    {
        // Capsule-AABB intersection*/
        const float x = -20+0.6f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = 28.0f;

        capsule c = capsule(vec3(-20.5f,-1.0f,27.5f), vec3(-20+0.5f,1.0f,28.5f), 0.2f);
        aabb b = aabb(vec3(x-0.5f,y-0.5f,z-0.5f), vec3(x+0.5f,y+0.5f,z+0.5f));
        hit *m = capsule_hit_aabb(c, b);
        if(m) {
            vec3 v;
            ddraw_color(BLUE);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            v = add3(m->contact_point, m->normal);
            ddraw_arrow(m->contact_point, v);
            ddraw_color(RED);
        } else ddraw_color(WHITE);
        ddraw_box(vec3(x,y,z), vec3(1,1,1));
        ddraw_capsule(vec3(-20.5f,-1.0f,27.5f), vec3(-20+0.5f,1.0f,28.5f), 0.2f);
    }
    {
        // AABB-Capsule intersection*/
        const float x = 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -8;

        aabb a = aabb(vec3(-0.5f,-0.5f,-8.5f), vec3(0.5f,0.5f,-7.5f));
        capsule c = capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        hit *m = aabb_hit_capsule(a, c);
        if(m) {
            ddraw_color(RED);
            ddraw_box(m->contact_point, vec3(0.05f, 0.05f, 0.05f));
            ddraw_arrow(m->contact_point, add3(m->contact_point, m->normal));
        } else ddraw_color(WHITE);

        ddraw_capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z-1.0f), 0.2f);
        ddraw_box(vec3(0,0,-8.0f), vec3(1,1,1));
    }
    {
        // poly(Pyramid)-Sphere (GJK) intersection*/
        sphere s = sphere(vec3(-10+0.6f*sin(dx), 3.0f*cos(dy),-8), 1);
        poly pyr = pyramid(vec3(-10.5f,-0.5f,-7.5f), vec3(-10.5f,1.0f,-7.5f), 1.0f);

        gjk_result gjk;
        if (poly_hit_sphere(&gjk, pyr, s))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_sphere(s.c, 1);
        ddraw_pyramid(vec3(-10.5f,-0.5f,-7.5f), 0.5f/*vec3(-10.5f,1.0f,-7.5f)*/, 1.0f);

        poly_free(&pyr);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }
    {
        // poly(Diamond)-Sphere (GJK) intersection*/

        sphere s = sphere(vec3(-20+0.6f*sin(dx), 3.0f*cos(dy),-8), 1);
        poly dmd = diamond(vec3(-20.5f,-0.5f,-7.5f), vec3(-20.5f,1.0f,-7.5f), 0.5f);

        gjk_result gjk;
        if (poly_hit_sphere(&gjk, dmd, s))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_sphere(s.c, 1);
        ddraw_diamond(vec3(-20.5f,-0.5f,-7.5f), vec3(-20.5f,1.0f,-7.5f), 0.5f);

        poly_free(&dmd);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }
    {
        // poly(Pyramid)-Capsule (GJK) intersection*/

        const float x = 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -15;

        capsule c = capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z), 0.2f);
        poly pyr = pyramid(vec3(-0.5f,-0.5f,-15.5f), vec3(-0.5f,1.0f,-15.5f), 1.0f);

        gjk_result gjk;
        if (poly_hit_capsule(&gjk, pyr, c))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_capsule(c.a, c.b, c.r);
        ddraw_pyramid(vec3(-0.5f,-0.5f,-15.5f), 0.5f/*vec3(-0.5f,1.0f,-15.5f)*/, 1.0f);

        poly_free(&pyr);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }

    {
        // poly(Diamond)-Capsule (GJK) intersection*/

        const float x = -10 + 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -15;

        capsule c = capsule(vec3(x,y-1.0f,z), vec3(x,y+1.0f,z), 0.2f);
        poly dmd = diamond(vec3(-10.5f,-0.5f,-15.5f), vec3(-10.5f,1.0f,-15.5f), 0.5f);

        gjk_result gjk;
        if (poly_hit_capsule(&gjk, dmd, c))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_capsule(c.a, c.b, c.r);
        ddraw_diamond(vec3(-10.5f,-0.5f,-15.5f), vec3(-10.5f,1.0f,-15.5f), 0.5f);

        poly_free(&dmd);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }

    {
        // poly(Diamond)-poly(Pyramid) (GJK) intersection*/

        const float x = -20 + 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -15;

        poly pyr = pyramid(vec3(x,y-0.5f,z), vec3(x,y+1,z), 0.8f);
        poly dmd = diamond(vec3(-20.5f,-0.5f,-15.5f), vec3(-20.5f,1.0f,-15.5f), 0.5f);

        gjk_result gjk;
        if (poly_hit_poly(&gjk, dmd, pyr))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_pyramid(vec3(x,y-0.5f,z), 1/*vec3(x,y+1,z)*/, 1/*0.8f*/);
        ddraw_diamond(vec3(-20.5f,-0.5f,-15.5f), vec3(-20.5f,1.0f,-15.5f), 0.5f);

        poly_free(&dmd);
        poly_free(&pyr);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }
    {
        // poly(Pyramid)-poly(Diamond) (GJK) intersection*/

        const float x = 10 + 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -15;

        poly dmd = diamond(vec3(x,y-0.5f,z), vec3(x,y+1,z), 0.5f);
        poly pyr = pyramid(vec3(10.5f,-0.5f,-15.5f), vec3(10.5f,1.0f,-15.5f), 1.0f);

        gjk_result gjk;
        if (poly_hit_poly(&gjk, dmd, pyr))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        ddraw_diamond(vec3(x,y-0.5f,z), vec3(x,y+1,z), 0.5f);
        ddraw_pyramid(vec3(10.5f,-0.5f,-15.5f), 0.5f/*vec3(10.5f,1.0f,-15.5f)*/, 1.0f);

        poly_free(&dmd);
        poly_free(&pyr);

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }
    {
        // poly(Diamond)-AABB (GJK) intersection*/

        const float x = 20 + 0.4f*sin(dx);
        const float y = 3.0f*cos(dy);
        const float z = -15;

        poly dmd = diamond(vec3(x,y-0.5f,z), vec3(x,y+1,z), 0.5f);
        aabb a = aabb(vec3(19.5f,-0.5f,-14.5f), vec3(20.5f,0.5f,-15.5f));

        gjk_result gjk;
        if (poly_hit_aabb(&gjk, dmd, a))
            ddraw_color(RED);
        else ddraw_color(WHITE);

        poly_free(&dmd);

        ddraw_diamond(vec3(x,y-0.5f,z), vec3(x,y+1,z), 0.5f);
        ddraw_box(vec3(20,0,-15), vec3(1,1,1));

        ddraw_box(gjk.p0, vec3(0.05f, 0.05f, 0.05f));
        ddraw_box(gjk.p1, vec3(0.05f, 0.05f, 0.05f));
        ddraw_line(gjk.p0, gjk.p1);
    }
}
