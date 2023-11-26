// atlasc.c
// Copyright 2019 Sepehr Taghdisian (septag@github). All rights reserved.
// License: https://github.com/septag/atlasc#license-bsd-2-clause
//
// sx_math.h
// Copyright 2018 Sepehr Taghdisian (septag@github). All rights reserved.
// License: https://github.com/septag/sx#license-bsd-2-clause

#ifndef ATLASC_HEADER
#define ATLASC_HEADER

#define ATLASC_VERSION "1.2.3"

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#ifndef __cplusplus
#define ATLAS_CAST
#else
#define ATLAS_CAST(T) T
extern "C" {
#endif

typedef union vec2  { struct { float x, y; }; float f[2]; } vec2;
typedef union vec3  { struct { float x, y, z; }; float f[3]; } vec3;
typedef union vec2i { struct { int x, y; }; int n[2]; } vec2i;
typedef union recti { struct { int xmin, ymin, xmax, ymax; }; struct { vec2i vmin, vmax; }; int f[4]; } recti;

typedef struct atlas_flags {
    int         alpha_threshold;
    float       dist_threshold;
    int         max_width;
    int         max_height;
    int         border;
    int         pot;
    int         padding;
    int         mesh;
    int         max_verts_per_mesh;
    float       scale;
} atlas_flags;

typedef struct atlas_image {
    uint8_t* pixels;    // only supports 32bpp RGBA format
    int      width;
    int      height;
    char *name;
} atlas_image;

typedef struct atlas_sprite {
    uint8_t* src_image;   // RGBA image buffer (32bpp)
    vec2i src_size;       // widthxheight
    recti sprite_rect;    // cropped rectangle relative to sprite's source image (pixels)
    recti sheet_rect;     // rectangle in final sheet (pixels)
    char *name;
    unsigned frame;

    // sprite-mesh data (if flag is set. see atlas_flags)
    uint16_t  num_tris;
    int       num_points;
    vec2i* pts;
    vec2i* uvs;
    uint16_t* tris;
} atlas_sprite;

typedef struct atlas_t {
    atlas_sprite*  sprites;
    int            num_sprites;
    int*           frames;
    int            num_frames;
    atlas_image  output;
} atlas_t;

// receives input files and common arguments. returns atlas_t
// you have to free the data after use with `atlas_free`
atlas_t* atlas_loadfiles(array(char*) files, atlas_flags flags);

// receives input image buffers and common arguments. returns atlas_t
// you have to free the data after use with `atlas_free`
atlas_t* atlas_loadimages(array(atlas_image) images, atlas_flags flags);

//
bool atlas_save(const char *outfile, const atlas_t* atlas, atlas_flags flags);

// frees atlas_t memory
void atlas_free(atlas_t* atlas);

// returns the last error string
const char* atlas_last_error();

#ifdef __cplusplus
}
#endif
#endif // ATLASC_HEADER

//
#ifdef ATLASC_IMPLEMENTATION
#include <math.h>
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Types/Primitives

#define vec2(x,y)      (ATLAS_CAST(vec2)  { (float)(x), (float)(y) })
#define vec3(x,y,z)    (ATLAS_CAST(vec3)  { (float)(x), (float)(y), (float)(z) })
#define vec2i(x,y)     (ATLAS_CAST(vec2i) { (int)(x), (int)(y) })
#define recti(x,y,X,Y) (ATLAS_CAST(recti) { (int)(x), (int)(y), (int)(X), (int)(Y) })

#define minf(a,b) ((a) < (b) ? (a) : (b))
#define maxf(a,b) ((a) > (b) ? (a) : (b))
#define clampf(a,b,c) ( (a) < (b) ? (b) : (a) > (c) ? (c) : (a))

static int   nearest_pow2(int n) { return  --n, n |= n >> 1, n |= n >> 2, n |= n >> 4, n |= n >> 8, n |= n >> 16, ++n; } // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static float sx_abs(float _a) { union { float f; unsigned int ui; } u = { _a }; return u.ui &= 0x7FFFFFFF, u.f; }
static bool  equalf(float _a, float _b, float _epsilon) { const float lhs = sx_abs(_a - _b), aa = sx_abs(_a), ab = sx_abs(_b), rhs = _epsilon * maxf(1.0f, maxf(aa, ab)); return lhs <= rhs; } // http://realtimecollisiondetection.net/blog/?t=89

static vec3  cross3(const vec3 _a, const vec3 _b) { return vec3(_a.y * _b.z - _a.z * _b.y, _a.z * _b.x - _a.x * _b.z, _a.x * _b.y - _a.y * _b.x); }

static float dot2(const vec2 _a, const vec2 _b) { return _a.x * _b.x + _a.y * _b.y; }
static float len2(const vec2 _a) { return sqrt(dot2(_a, _a)); }
static vec2  norm2(const vec2 _a) { const float len = len2(_a); /*assert(len > 0 && "Divide by zero");*/ return vec2(_a.x / (len + !len), _a.y / (len + !len)); }
static vec2  add2(const vec2 _a, const vec2 _b) { return vec2(_a.x + _b.x, _a.y + _b.y); }
static vec2  sub2(const vec2 _a, const vec2 _b) { return vec2(_a.x - _b.x, _a.y - _b.y); }
static vec2  scale2(const vec2 _a, float _b) { return vec2(_a.x * _b, _a.y * _b); }

static vec2i add2i(const vec2i _a, const vec2i _b) { return vec2i(_a.x + _b.x, _a.y + _b.y); }
static vec2i sub2i(const vec2i _a, const vec2i _b) { return vec2i(_a.x - _b.x, _a.y - _b.y); }
static vec2i min2i(const vec2i _a, const vec2i _b) { return vec2i(minf(_a.x, _b.x), minf(_a.y, _b.y)); }
static vec2i max2i(const vec2i _a, const vec2i _b) { return vec2i(maxf(_a.x, _b.x), maxf(_a.y, _b.y)); }

static recti rectiwh(int _x, int _y, int _w, int _h) { return recti(_x, _y, _x + _w, _y + _h); }
static recti recti_expand(const recti rc, const vec2i expand) { return recti(rc.xmin - expand.x, rc.ymin - expand.y, rc.xmax + expand.x, rc.ymax + expand.y); }
static void  recti_add_point(recti* rc, const vec2i pt) { rc->vmin = min2i(rc->vmin, pt); rc->vmax = max2i(rc->vmax, pt); }

// ----------------------------------------------------------------------------

#ifndef ATLAS_REALLOC
#define ATLAS_REALLOC REALLOC
#endif
#ifndef ATLAS_MSIZE
#define ATLAS_MSIZE MSIZE
#endif
#ifndef ATLAS_CALLOC
#define ATLAS_CALLOC(n,m) memset(ATLAS_REALLOC(0, (n)*(m)), 0, (n)*(m))
#endif
#ifndef ATLAS_FREE
#define ATLAS_FREE(ptr)   ((ptr) = ATLAS_REALLOC((ptr), 0))
#endif

#define align_mask(_value, _mask) (((_value) + (_mask)) & ((~0) & (~(_mask))))

static void panic_if(int fail) { if(fail) exit(-fprintf(stderr, "out of memory!\n")); }

static void path_unixpath(char *buf, unsigned buflen, const char *inpath) {
    snprintf(buf, buflen, "%s", inpath);
    while( strchr(buf, '\\') ) *strchr(buf, '\\') = '/';
}
static void path_basename(char *buf, unsigned buflen, const char *inpath) {
    const char *a = strrchr(inpath, '\\');
    const char *b = strrchr(inpath,  '/');
    snprintf(buf, buflen, "%s", a > b ? a+1 : b > a ? b+1 : inpath );
}
static bool path_isfile(const char* filepath) {
    FILE *f = fopen(filepath, "rb");
    return f ? fclose(f), 1 : 0;
}

static char g_error_str[512];
const char* atlas_last_error()
{
    return g_error_str;
}

static void atlas__free_sprites(atlas_sprite* sprites, int num_sprites)
{
    for (int i = 0; i < num_sprites; i++) {
        if (sprites[i].src_image) {
            stbi_image_free(sprites[i].src_image);
        }

        if (sprites[i].tris) {
            ATLAS_FREE(sprites[i].tris);
        }

        if (sprites[i].pts) {
            ATLAS_FREE(sprites[i].pts);
        }

        if (sprites[i].uvs) {
            ATLAS_FREE(sprites[i].uvs);
        }

        if (sprites[i].name) {
            ATLAS_FREE(sprites[i].name);
        }
    }
    ATLAS_FREE(sprites);
}

static void atlas__blit(uint8_t* dst, int dst_x, int dst_y, int dst_pitch, const uint8_t* src,
                         int src_x, int src_y, int src_w, int src_h, int src_pitch, int bpp)
{
    assert(dst);
    assert(src);

    const int pixel_sz = bpp / 8;
    const uint8_t* src_ptr = src + src_y * src_pitch + src_x * pixel_sz;
    uint8_t* dst_ptr = dst + dst_y * dst_pitch + dst_x * pixel_sz;
    for (int y = src_y; y < (src_y + src_h); y++) {
        memcpy(dst_ptr, src_ptr, src_w * pixel_sz);
        src_ptr += src_pitch;
        dst_ptr += dst_pitch;
    }
}

static vec2 atlas__itof2(const s2o_point p)
{
    return vec2((float)p.x, (float)p.y);
}

static unsigned atlas__rgba( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
    return (unsigned)a << 24 | b << 16 | g << 8 | r;
}

// modified version of:
// https://github.com/anael-seghezzi/Maratis-Tiny-C-library/blob/master/include/m_raster.h
static bool atlas__test_line(const uint8_t* buffer, int w, int h, s2o_point p0, s2o_point p1)
{
    const uint8_t* data = buffer;

    int x0 = p0.x;
    int y0 = p0.y;
    int x1 = p1.x;
    int y1 = p1.y;
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        if (x0 > -1 && y0 > -1 && x0 < w && y0 < h) {
            const uint8_t* pixel = data + (y0 * w + x0);
            if (*pixel)
                return true;    // line intersects with image data
        }

        if (x0 == x1 && y0 == y1)
            break;

        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }

    return false;
}

// returns true if 'pts' buffer is changed
static bool atlas__offset_pt(s2o_point* pts, int num_pts, int pt_idx, float amount, int w, int h)
{
    s2o_point ipt = pts[pt_idx];
    s2o_point _ipt = ipt;
    vec2 pt = atlas__itof2(ipt);
    vec2 prev_pt = (pt_idx > 0) ? atlas__itof2(pts[pt_idx - 1]) : atlas__itof2(pts[num_pts - 1]);
    vec2 next_pt = (pt_idx + 1) < num_pts ? atlas__itof2(pts[pt_idx + 1]) : atlas__itof2(pts[0]);
    vec2 edge1 = norm2(sub2(prev_pt, pt));
    vec2 edge2 = norm2(sub2(next_pt, pt));

    // calculate normal vector to move the point away from the polygon
    vec2 n;
    vec3 c = cross3(vec3(edge1.x, edge1.y, 0), vec3(edge2.x, edge2.y, 0));
    if (equalf(c.z, 0.0f, 0.00001f)) {
        n = scale2(vec2(-edge1.y, edge1.x), amount);
    } else {
        // c.z < 0 -> point intersecting convex edges
        // c.z > 0 -> point intersecting concave edges
        float k = c.z < 0.0f ? -1.0f : 1.0f;
        n = scale2(norm2(add2(edge1, edge2)), k * amount);
    }

    pt = add2(pt, n);
    ipt.x = (int)pt.x;
    ipt.y = (int)pt.y;
    ipt.x = clampf(ipt.x, 0, w);
    ipt.y = clampf(ipt.y, 0, h);
    pts[pt_idx] = ipt;
    return (_ipt.x != ipt.x) || (_ipt.y != ipt.y);
}

static void atlas__fix_outline_pts(const uint8_t* thresholded, int tw, int th, s2o_point* pts,
                                    int num_pts)
{
    // NOTE: winding is assumed to be CW
    const float offset_amount = 2.0f;

    for (int i = 0; i < num_pts; i++) {
        s2o_point pt = pts[i];
        int next_i = (i + 1) < num_pts ? (i + 1) : 0;

//      assert(!thresholded[pt.y * tw + pt.x]);    // point shouldn't be inside threshold

        s2o_point next_pt = pts[next_i];
        while (atlas__test_line(thresholded, tw, th, pt, next_pt)) {
            if (!atlas__offset_pt(pts, num_pts, i, offset_amount, tw, th))
                break;
            atlas__offset_pt(pts, num_pts, next_i, offset_amount, tw, th);
            // refresh points for the new line intersection test
            pt = pts[i];
            next_pt = pts[next_i];
        }
    }
}

static void atlas__make_mesh(atlas_sprite* spr, const s2o_point* pts, int pt_count, int max_verts,
                              const uint8_t* thresholded, int width, int height)
{
    s2o_point* temp_pts = ATLAS_CALLOC(pt_count,sizeof(s2o_point));
    panic_if(!temp_pts);

    memcpy(temp_pts, pts, sizeof(s2o_point)*pt_count);
    int num_verts = pt_count;

    if (width > 1 && height > 1) {
        const float delta = 0.5f;
        const float threshold_start = 0.5f;
        float threshold = threshold_start;

        for(;;) {
            s2o_distance_based_path_simplification(temp_pts, &num_verts, threshold);

            if(num_verts <= max_verts) break;

            memcpy(temp_pts, pts, sizeof(s2o_point)*pt_count);
            num_verts = pt_count;

            threshold += delta;
        }

        // fix any collisions with the actual image // @r-lyeh: method below is buggy. will return dupe points
        atlas__fix_outline_pts(thresholded, width, height, temp_pts, num_verts);
    }

    //< @r-lyeh: remove dupes
    for (int i = 0; i < num_verts - 1; i++) {
        for (int j = i + 1; j < num_verts; j++) {
            if( temp_pts[i].x == temp_pts[j].x && temp_pts[i].y == temp_pts[j].y ) {
                temp_pts[j].x = temp_pts[num_verts - 1].x;
                temp_pts[j].y = temp_pts[num_verts - 1].y;
                --num_verts;
                --j;
            }
        }
    }
    //<

    // triangulate
    del_point2d_t* dpts = ATLAS_CALLOC(num_verts, sizeof(del_point2d_t));
    panic_if(!dpts);

    for (int i = 0; i < num_verts; i++) {
        dpts[i].x = (double)temp_pts[i].x;
        dpts[i].y = (double)temp_pts[i].y;
        //printf("%d) %f,%f\n", i, dpts[i].x, dpts[i].y); //< @r-lyeh: debug dupe points
    }

    delaunay2d_t* polys = delaunay2d_from(dpts, num_verts);
    assert(polys);
    tri_delaunay2d_t* tris = tri_delaunay2d_from(polys);
    assert(tris);
    ATLAS_FREE(dpts);
    delaunay2d_release(polys);

    assert(tris->num_triangles < UINT16_MAX);
    spr->tris = ATLAS_CALLOC(tris->num_triangles * 3,sizeof(uint16_t));
    spr->pts = ATLAS_CALLOC(tris->num_points, sizeof(vec2i));
    assert(spr->tris);
    assert(spr->pts);

    for (unsigned int i = 0; i < tris->num_triangles; i++) {
        unsigned int index = i * 3;
        spr->tris[index] = (uint16_t)tris->tris[index];
        spr->tris[index + 1] = (uint16_t)tris->tris[index + 1];
        spr->tris[index + 2] = (uint16_t)tris->tris[index + 2];
    }
    for (unsigned int i = 0; i < tris->num_points; i++) {
        spr->pts[i] = vec2i((int)tris->points[i].x, (int)tris->points[i].y);
    }
    spr->num_tris = (uint16_t)tris->num_triangles;
    spr->num_points = (int)tris->num_points;

    tri_delaunay2d_release(tris);
    ATLAS_FREE(temp_pts);
}

atlas_t* atlas_loadimages(array(atlas_image) images, atlas_flags flags)
{
    assert(images);

    array(int) frames = 0;
    array(atlas_sprite) sprites = 0;

    for (int i = 0; i < array_count(images); i++) {

        // find is_cached
        {
            int found = 0, k = 0;
            static array(uint64_t) cache = 0;
            static array(uint64_t) index = 0;
            uint64_t hash = hash_init;
            hash = hash_bin(&images[i].width, sizeof(images[i].width), hash);
            hash = hash_bin(&images[i].height, sizeof(images[i].height), hash);
            hash = hash_bin((char*)images[i].pixels, images[i].width * images[i].height * 4, hash);
            for (; k < array_count(cache); ++k)
                if (cache[k] == hash) { found = 1; break; }
            if (found) {
                array_push(frames, index[k]);
                continue;
            } else {
                array_push(cache, hash);
                array_push(index, k);
                array_push(frames, k);
            }
            //printf("%d) %llx\n", array_count(cache), hash);
        }

        atlas_sprite zero = {0};
        atlas_sprite* spr = &zero;
        if(images[i].name) spr->name = STRDUP(images[i].name);
        spr->frame = i;

        spr->src_size.x = images[i].width;
        spr->src_size.y = images[i].height;
        assert(images[i].width > 0 && images[i].height > 0);
        assert(images[i].pixels);
        uint8_t* pixels = images[i].pixels;

        // rescale
        if (!equalf(flags.scale, 1.0f, 0.0001f)) {
            int target_w = (int)((float)spr->src_size.x * flags.scale);
            int target_h = (int)((float)spr->src_size.y * flags.scale);
            uint8_t* resized_pixels = ATLAS_CALLOC(1, 4 * target_w * target_h);
            panic_if(!resized_pixels);

            if (!stbir_resize_uint8(pixels, spr->src_size.x, spr->src_size.y, 4 * spr->src_size.x,
                                    resized_pixels, target_w, target_h, 4 * target_w, 4)) {
                snprintf(g_error_str, sizeof(g_error_str), "could not resize image: #%d", i + 1);
                atlas__free_sprites(sprites, array_count(sprites));
                return NULL;
            }

            stbi_image_free(pixels);

            spr->src_size.x = target_w;
            spr->src_size.y = target_h;
            pixels = resized_pixels;
        }

        spr->src_image = pixels;

        recti sprite_rect = {0};
        int pt_count = 0;
        s2o_point* pts = 0;
        uint8_t* alpha = s2o_rgba_to_alpha(spr->src_image, spr->src_size.x, spr->src_size.y);
        uint8_t* thresholded = s2o_alpha_to_thresholded(alpha, spr->src_size.x, spr->src_size.y, flags.alpha_threshold);
        free(alpha);

        if (flags.mesh && spr->src_size.x > 1 && spr->src_size.y > 1) {
            uint8_t* dilate_thres = s2o_dilate_thresholded(thresholded, spr->src_size.x, spr->src_size.y);

            uint8_t* outlined = s2o_thresholded_to_outlined(dilate_thres, spr->src_size.x, spr->src_size.y);
            free(dilate_thres);

            pts = s2o_extract_outline_path(outlined, spr->src_size.x, spr->src_size.y, &pt_count, NULL);
            free(outlined);

            //< @r-lyeh @fixme: many sprites will return extremely low num of points (like 8) even if the sprite is complex enough.
            //< this will lead to produce here a nearly zero sprite_rect, then sheet_rect, then eventually an empty frame at end of pipeline.

            // calculate cropped rectangle
            sprite_rect = recti(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
            for (int k = 0; k < pt_count; k++) {
                recti_add_point(&sprite_rect, vec2i(pts[k].x, pts[k].y));
            }
            sprite_rect.xmax++;
            sprite_rect.ymax++;
        } else {
            sprite_rect = recti(0, 0, spr->src_size.x, spr->src_size.y);
            pt_count = 4;
            pts = ATLAS_CALLOC(pt_count, sizeof(s2o_point));
            pts[0] = (s2o_point) {0, 0};
            pts[1] = (s2o_point) {spr->src_size.x, 0};
            pts[2] = (s2o_point) {spr->src_size.x, spr->src_size.y};
            pts[3] = (s2o_point) {0, spr->src_size.y};
        }

        // generate mesh if set in arguments
        if (flags.mesh) {
            atlas__make_mesh(spr, pts, pt_count, flags.max_verts_per_mesh, thresholded,
                              spr->src_size.x, spr->src_size.y);
        }

        ATLAS_FREE(pts);
        free(thresholded);
        spr->sprite_rect = sprite_rect;

        array_push(sprites, *spr);
    }

    int num_sprites = array_count(sprites);

    // pack sprites into a sheet
    stbrp_context rp_ctx = {0};
    int max_width = flags.max_width;
    int max_height = flags.max_height;
    int num_rp_nodes = max_width + max_height;
    stbrp_rect* rp_rects = ATLAS_CALLOC(num_sprites, sizeof(stbrp_rect));
    stbrp_node* rp_nodes = ATLAS_CALLOC(num_rp_nodes, sizeof(stbrp_node));
    panic_if(!rp_rects || !rp_nodes);

    for (int i = 0; i < num_sprites; i++) {
        recti rc = sprites[i].sprite_rect;
        int rc_resize = (flags.border + flags.padding) * 2;
        rp_rects[i].w = (rc.xmax - rc.xmin) + rc_resize;
        rp_rects[i].h = (rc.ymax - rc.ymin) + rc_resize;
    }
    stbrp_init_target(&rp_ctx, max_width, max_height, rp_nodes, num_rp_nodes);
    recti final_rect = recti(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
    if (stbrp_pack_rects(&rp_ctx, rp_rects, num_sprites)) {
        for (int i = 0; i < num_sprites; i++) {
            atlas_sprite* spr = &sprites[i];
            recti sheet_rect = rectiwh(rp_rects[i].x, rp_rects[i].y, rp_rects[i].w, rp_rects[i].h);

            // calculate the total size of output image
            recti_add_point(&final_rect, sheet_rect.vmin);
            recti_add_point(&final_rect, sheet_rect.vmax);

            // shrink back rect and set the real sheet_rect for the sprite
            spr->sheet_rect =
                recti_expand(sheet_rect, vec2i(-flags.border, -flags.border));
        }
    }

    int dst_w = final_rect.xmax - final_rect.xmin;
    int dst_h = final_rect.ymax - final_rect.ymin;
    // make output size divide by 4 by default
    dst_w = align_mask(dst_w, 3);
    dst_h = align_mask(dst_h, 3);

    if (flags.pot) {
        dst_w = nearest_pow2(dst_w);
        dst_h = nearest_pow2(dst_h);
    }

    uint8_t* dst = ATLAS_CALLOC(1, dst_w * dst_h * 4);
    panic_if(!dst);

    // calculate UVs for sprite meshes
    if (flags.mesh) {
        for (int i = 0; i < num_sprites; i++) {
            atlas_sprite* spr = &sprites[i];
            // if sprite has mesh, calculate UVs for it
            if (spr->pts && spr->num_points) {
                const int padding = flags.padding;
                vec2i offset = spr->sprite_rect.vmin;
                vec2i sheet_pos =
                    vec2i(spr->sheet_rect.xmin + padding, spr->sheet_rect.ymin + padding);
                vec2i* uvs = ATLAS_CALLOC(spr->num_points, sizeof(vec2i));
                assert(uvs);
                for (int pi = 0; pi < spr->num_points; pi++) {
                    vec2i pt = spr->pts[pi];
                    uvs[pi] = add2i(sub2i(pt, offset), sheet_pos);
                }

                spr->uvs = uvs;
            }    // generate uvs
        }
    }

    for (int i = 0; i < num_sprites; i++) {
        const atlas_sprite* spr = &sprites[i];

        // calculate UVs for sprite-meshes

        // remove padding and blit from src_image to dst
        recti dstrc = recti_expand(spr->sheet_rect, vec2i(-flags.padding, -flags.padding));
        recti srcrc = spr->sprite_rect;
        atlas__blit(dst, dstrc.xmin, dstrc.ymin, dst_w * 4, spr->src_image, srcrc.xmin, srcrc.ymin,
                     srcrc.xmax - srcrc.xmin, srcrc.ymax - srcrc.ymin, spr->src_size.x * 4, 32);
    }

    atlas_t* atlas = ATLAS_CALLOC(1, sizeof(atlas_t));
    panic_if(!atlas);

    atlas->output.pixels = dst;
    atlas->output.width = dst_w;
    atlas->output.height = dst_h;
    atlas->sprites = sprites;
    atlas->num_sprites = num_sprites;
    atlas->frames = frames;
    atlas->num_frames = array_count(frames);

    ATLAS_FREE(rp_nodes);
    ATLAS_FREE(rp_rects);

    return atlas;
}

static char *atlas_anims = 0;
static char *atlas_slices = 0;
static char *atlas_current_anim = 0;

atlas_t* atlas_loadfiles(array(char*) files, atlas_flags flags)
{
    assert(files);

    array(atlas_image) images = 0;

    for (int i = 0; i < array_count(files); ++i) {
        if (!path_isfile(files[i])) {
            snprintf(g_error_str, sizeof(g_error_str), "input image not found: %s", files[i]);
            goto err_cleanup;
        }

        int comp;
        atlas_image img = {0};
        img.pixels = stbi_load(files[i], &img.width, &img.height, &comp, 4);

#ifdef CUTE_ASEPRITE_H
        if (!img.pixels) {
            bool loaded = 0;

            for( ase_t* ase = cute_aseprite_load_from_file(files[i], NULL); ase; cute_aseprite_free(ase), ase = 0, loaded = 1) {
                ase_tag_t *parent = ase->tags + 0;

                //< abc/def/ghi.aseprite -> ghi
                if( atlas_current_anim ) *atlas_current_anim = '\0';
                strcatf(&atlas_current_anim, files[i]);
                path_basename(atlas_current_anim, strlen(atlas_current_anim), files[i]);
                if( strrchr(atlas_current_anim, '.')) *strrchr(atlas_current_anim, '.') = '\0';
                trimspace(atlas_current_anim);
                //<

                for( int f = 0; f < ase->frame_count; ++f) {
                    ase_frame_t *frame = ase->frames + f;

                    // find rect
                    int x = INT_MAX, y = INT_MAX, x2 = INT_MIN, y2 = INT_MIN;
                    for( int c = 0; c < frame->cel_count; ++c ) {
                        ase_cel_t *cel = frame->cels + c;
                        if( cel->layer->flags & ASE_LAYER_FLAGS_VISIBLE ) {
                        if( cel->x < x ) x = cel->x;
                        if( cel->h < y ) y = cel->y;
                        if( (cel->x + cel->w) > x2 ) x2 = cel->x + cel->w;
                        if( (cel->y + cel->h) > y2 ) y2 = cel->y + cel->h;
                        }
                    }
                    if (x2 <= 0 || y2 <= 0) { // submit empty frame
                        img.width = 1;
                        img.height = 1;
                        img.pixels = calloc(1, 1*1*4);
                        array_push(images, img);
                        continue;
                    }
                    int cx = x;
                    int cy = y;
                    int cw = x2-x;
                    int ch = y2-y;
                    int tn = 4;
                    int tw = ase->w;

                    // find clip
                    img.width = cw;
                    img.height = ch;
                    img.pixels = calloc(1, cw*ch*4); // @fixme: because of a stbi_image_free() within rescale section, this should be allocated with stbi allocator
                    for( unsigned y = 0; y < ch; ++y )
                    memcpy((char *)img.pixels + (0+(0+y)*cw)*tn, (char*)frame->pixels + (cx+(cy+y)*tw)*tn, cw*tn);
                    array_push(images, img);
                }

                static int slice_idx = -1;
                static int slice_frame_idx = 0;
                static const char *slice_name = 0;
                if(!atlas_slices) strcatf(&atlas_slices, "[slices]\n");

                for( int t = 0; t < ase->slice_count; ++t) {
                    ase_slice_t *slice = ase->slices + t;
                    if (!slice_name || strcmp(slice_name, slice->name)) {
                        ++slice_idx;
                        strcatf(&atlas_slices, "[%d].sl_name=%s\n", slice_idx, slice->name);
                        strcatf(&atlas_slices, "[%d].sl_frames=", slice_idx);
                        for( int u = 0; u < ase->slice_count; ++u) {
                            if (!strcmp(slice->name, ase->slices[u].name)) {
                                strcatf(&atlas_slices, "%d,", u);
                            }
                        }
                        strcatf(&atlas_slices, "\n");
                    }
                    strcatf(&atlas_slices, "[%d].sl_bounds=%d,%d,%d,%d\n", slice_idx, slice->origin_x, slice->origin_y, slice->w, slice->h);
                    strcatf(&atlas_slices, "[%d].sl_9slice=%d\n", slice_idx, slice->has_center_as_9_slice);
                    if (slice->has_center_as_9_slice)
                        strcatf(&atlas_slices, "[%d].sl_core=%d,%d,%d,%d\n", slice_idx, slice->center_x, slice->center_y, slice->center_w, slice->center_h);
                    if (slice->has_pivot)
                        strcatf(&atlas_slices, "[%d].sl_pivot=%d,%d\n", slice_idx, slice->pivot_x, slice->pivot_y);
                    if (slice->udata.has_color)
                        strcatf(&atlas_slices, "[%d].sl_color=%u\n", slice_idx, atlas__rgba(slice->udata.color.r, slice->udata.color.g, slice->udata.color.b, slice->udata.color.a));
                    if (slice->udata.has_text)
                        strcatf(&atlas_slices, "[%d].sl_text=%s\n", slice_idx, slice->udata.text);
                    
                    slice_name = slice->name;
                    ++slice_frame_idx;
                }

                static int anim_idx = 0;
                if(!atlas_anims) strcatf(&atlas_anims, "[anims]\n");

                for( int t = 0; t < ase->tag_count; ++t) {
                    ase_tag_t *tag = ase->tags + t;

                    // find full name
                    int range[2] = {tag->from_frame, tag->to_frame};
                    char name[256] = {0};
                    for( int tt = 0; tt < ase->tag_count; ++tt ) {
                        ase_tag_t *ttag = ase->tags + tt;
                        if( range[0] >= ttag->from_frame && range[1] <= ttag->to_frame )
                            strcat(name, "."), strcat(name, ttag->name);
                    }
                    trimspace(name);

                    char *sep = "";
                    strcatf(&atlas_anims, "[%d].name=%s.%s\n", anim_idx, atlas_current_anim, name+1);
                    strcatf(&atlas_anims, "[%d].frames=", anim_idx);
                    if( tag->loop_animation_direction != ASE_ANIMATION_DIRECTION_BACKWARDS)
                    for( int from = tag->from_frame; from <= tag->to_frame; ++from ) {
                        strcatf(&atlas_anims, "%s%d,%d", sep, from, ase->frames[from].duration_milliseconds), sep = ",";
                    }
                    sep = "";
                    if( tag->loop_animation_direction != ASE_ANIMATION_DIRECTION_FORWARDS)
                    for( int from = tag->from_frame; from <= tag->to_frame; ++from ) {
                        strcatf(&atlas_anims, "%s%d,%d", sep, from, ase->frames[from].duration_milliseconds), sep = ",";
                    }
                    strcatf(&atlas_anims,"\n");

                    ++anim_idx;
                }
            }

            if( loaded ) continue;
        }
#endif

        if (!img.pixels) {
            continue; //< @r-lyeh: keep going

            snprintf(g_error_str, sizeof(g_error_str), "invalid image format: %s", files[i]);
            goto err_cleanup;
        }

        if( !img.name ) img.name = STRDUP(files[i]);

        array_push(images, img);
    }

    atlas_t* atlas = atlas_loadimages(images, flags);
    return atlas;

err_cleanup:
    for (int i = 0; i < array_count(images); i++) {
        if (images[i].pixels) {
            stbi_image_free(images[i].pixels);
        }
        if (images[i].name) {
            ATLAS_FREE(images[i].name);
        }
    }
    array_free(images);
    return NULL;
}

void atlas_free(atlas_t* atlas)
{
    assert(atlas);

    if (atlas->sprites)
        atlas__free_sprites(atlas->sprites, atlas->num_sprites);
    if (atlas->frames)
        ATLAS_FREE(atlas->frames);
    if (atlas->output.pixels)
        ATLAS_FREE(atlas->output.pixels);
    ATLAS_FREE(atlas);
}



// custom write function
typedef struct {
    int offset;
    void *buffer;
} stbi_mem_context;
static void stbi_write_mem(void *context, void *data, int size) {
   stbi_mem_context *ctx = (stbi_mem_context*)context;
   memcpy( ctx->buffer, data, size );
   ctx->offset += size;
}

bool atlas_save(const char *outfile, const atlas_t *atlas, atlas_flags flags)
{
    assert(outfile);

    const bool is_file = strcmp(outfile, "stdout");
    const atlas_sprite* sprites = atlas->sprites;
    const int* frames = atlas->frames;
    const int num_frames = atlas->num_frames;
    const int num_sprites = atlas->num_sprites;
    const uint8_t* dst = atlas->output.pixels;
    const int dst_w = atlas->output.width;
    const int dst_h = atlas->output.height;

    char image_filepath[256];
    char image_filename[256];
    snprintf(image_filepath, sizeof(image_filepath), "%s.png", outfile);
    path_basename(image_filename, sizeof(image_filename), image_filepath);

    stbi_write_png_compression_level = 5; // 8

    // write texture, if needed
    if( is_file ) {
        if (!stbi_write_png(image_filepath, dst_w, dst_h, 4, dst, dst_w * 4)) {
            fprintf(stderr, "could not write image file `%s`\n", image_filepath);
            return false;
        }
    }

    // write atlas description into .ini file
    FILE *writer = is_file ? fopen(outfile, "wt") : stdout;
    if (!writer) {
        fprintf(stderr, "could not write ini file `%s`\n", outfile);
        return false;
    }

    fprintf(writer, "[atlas]\n");

    if (is_file) {
        fprintf(writer, "file=%s\n", image_filepath);
    } else {
        stbi_mem_context ctx = {0, ATLAS_CALLOC(1, dst_w*dst_h*4+256) };
        int result = stbi_write_png_to_func(stbi_write_mem, &ctx, dst_w, dst_h, 4, dst, dst_w*4);
        char *b64 = base64_encode(ctx.buffer, ctx.offset);
        fprintf(writer, "bitmap=%s\n", b64); // %d:%s\n", ctx.offset, b64);
        ATLAS_FREE(ctx.buffer);
        FREE(b64);
    }

    fprintf(writer, "size=%d,%d\n", dst_w, dst_h);
    fprintf(writer, "border=%d,%d\n", flags.border, flags.border);
    fprintf(writer, "padding=%d,%d\n", flags.padding, flags.padding);

    for( int i = 0; i < num_frames; i++ ) {
        const atlas_sprite* spr = sprites + frames[i];

        char name[256];
        path_unixpath(name, sizeof(name), spr->name ? spr->name : "");

        if(name[0])
        fprintf(writer, "[%d].name=%s\n", i, name);
        fprintf(writer, "[%d].frame=%u\n", i, spr->frame);
        //fprintf(writer, "[%d].size=%d,%d\n", i, spr->src_size.n[0], spr->src_size.n[1]);
        //fprintf(writer, "[%d].rect=%u,%u,%u,%u\n", i, spr->sprite_rect.f[0], spr->sprite_rect.f[1], spr->sprite_rect.f[2], spr->sprite_rect.f[3]);
        fprintf(writer, "[%d].sheet=%u,%u,%u,%u\n", i, spr->sheet_rect.f[0], spr->sheet_rect.f[1], spr->sheet_rect.f[2], spr->sheet_rect.f[3]);
        if( spr->num_tris ) {
            fprintf(writer, "[%d].indices=", i); // %d:", i, (int)spr->num_tris * 3);
            for( int j = 0, jend = (int)spr->num_tris * 3; j < jend; ++j )
            fprintf(writer, "%u%s", spr->tris[j], j < (jend-1) ? "," : "\n");

            fprintf(writer, "[%d].coords=", i); // %d:", i, spr->num_points*2);
            for( int j = 0, jend = spr->num_points; j < jend; j++ )
            fprintf(writer, "%.f,%.f%s", (double)spr->pts[j].x, (double)spr->pts[j].y, j < (jend-1) ? ",":"\n" );

            fprintf(writer, "[%d].uvs=", i); // %d:", i, spr->num_points*2);
            for( int j = 0, jend = spr->num_points; j < jend; j++ )
            fprintf(writer, "%.f,%.f%s", (double)spr->uvs[j].x, (double)spr->uvs[j].y, j < (jend-1) ? ",":"\n" );
        }
    }

    if( atlas_anims ) fprintf(writer, "%s\n", atlas_anims);
    if( atlas_slices ) fprintf(writer, "%s\n", atlas_slices);

    if(writer != stdout) fclose(writer);
    return true;
}

#endif // ATLASC_IMPLEMENTATION
