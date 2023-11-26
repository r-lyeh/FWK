// ----------------------------------------------------------------------------
// sprites

typedef struct sprite_static_t {
    float px, py, pz;         // origin x, y, depth
    float ox, oy, cos, sin;   // offset x, offset y, cos/sin of rotation degree
    float sx, sy;             // scale x,y
    float cellw, cellh;       // dimensions of any cell in spritesheet

    union {
    struct {
        int frame, ncx, ncy;      // frame in a (num cellx, num celly) spritesheet
    };
    struct {
        float x, y, w, h;         // normalized[0..1] within texture bounds
    };
    };

    uint32_t rgba, flags;     // vertex color and flags
} sprite_static_t;

// sprite batching
typedef struct batch_t { array(sprite_static_t) sprites; mesh_t mesh; int dirty; } batch_t;
typedef map(int, batch_t) batch_group_t; // mapkey is anything that forces a flush. texture_id for now, might be texture_id+program_id soon

// sprite stream
typedef struct sprite_vertex { vec3 pos; vec2 uv; uint32_t rgba; } sprite_vertex;
typedef struct sprite_index  { GLuint triangle[3]; } sprite_index;

#define sprite_vertex(...) C_CAST(sprite_vertex, __VA_ARGS__)
#define sprite_index(...)  C_CAST(sprite_index, __VA_ARGS__)

// sprite impl
static int sprite_count = 0;
static int sprite_program = -1;
static array(sprite_index)  sprite_indices = 0;
static array(sprite_vertex) sprite_vertices = 0;

// center_wh << 2 | additive << 1 | projected << 0
static batch_group_t sprite_group[8] = {0};

// rect(x,y,w,h) is [0..1] normalized, pos(xyz,z-index), scale_offset(sx,sy,offx,offy), rotation (degrees), color (rgba)
void sprite_rect( texture_t t, vec4 rect, vec4 pos, vec4 scale_offset, float tilt_deg, unsigned tint_rgba, unsigned flags) {
    float zindex = pos.w;
    float scalex = scale_offset.x;
    float scaley = scale_offset.y;
    float offsetx = scale_offset.z;
    float offsety = scale_offset.w;

    // do not queue if either scales or alpha are zero
    if( 0 == (scalex * scaley * ((tint_rgba>>24) & 255)) ) return;

    ASSERT( (flags & SPRITE_CENTERED) == 0 );
    if( flags & SPRITE_PROJECTED ) {
        tilt_deg += 180, scalex = -scalex; // flip texture Y on mvp3d (same than turn 180º then flip X)
    }

    sprite_static_t s = {0};

    s.px = pos.x, s.py = pos.y, s.pz = pos.z - zindex;
    s.sx = scalex, s.sy = scaley;

    s.x = rect.x, s.y = rect.y, s.w = rect.z, s.h = rect.w;
    s.cellw = s.w * s.sx * t.w, s.cellh = s.h * s.sy * t.h;

    s.rgba = tint_rgba;
    s.flags = flags;

#if 0
    s.ox = 0/*ox*/ * s.sx;
    s.oy = 0/*oy*/ * s.sy;
#else
    s.ox += offsetx * scalex;
    s.oy += offsety * scaley;
#endif

    if( tilt_deg ) {
        tilt_deg = (tilt_deg + 0) * ((float)C_PI / 180);
        s.cos = cosf(tilt_deg);
        s.sin = sinf(tilt_deg);
    } else {
        s.cos = 1;
        s.sin = 0;
    }

    batch_group_t *batches = &sprite_group[ flags & 7 ];
    batch_t *found = map_find_or_add(*batches, t.id, (batch_t){0});

    array_push(found->sprites, s);
}

void sprite_sheet( texture_t texture, float spritesheet[3], float position[3], float rotation, float offset[2], float scale[2], unsigned rgba, unsigned flags) {
    flags |= SPRITE_CENTERED;
    ASSERT( flags & SPRITE_CENTERED );

    const float px = position[0], py = position[1], pz = position[2];
    const float ox = offset[0], oy = offset[1], sx = scale[0], sy = scale[1];
    const float frame = spritesheet[0], xcells = spritesheet[1], ycells = spritesheet[2];

    if (frame < 0) return;
    if (frame > 0 && frame >= (xcells * ycells)) return;

    // no need to queue if alpha or scale are zero
    if( sx && sy && alpha(rgba) ) {
        vec3 bak = camera_get_active()->position;
        if( flags & SPRITE_RESOLUTION_INDEPENDANT ) { // @todo: optimize me
        sprite_flush();
        camera_get_active()->position = vec3(window_width()/2,window_height()/2,1);
        }

        sprite_static_t s;
        s.px = px;
        s.py = py;
        s.pz = pz;
        s.frame = frame;
        s.ncx = xcells ? xcells : 1;
        s.ncy = ycells ? ycells : 1;
        s.sx = sx;
        s.sy = sy;
        s.ox = ox * sx;
        s.oy = oy * sy;
        s.cellw = (texture.x * sx / s.ncx);
        s.cellh = (texture.y * sy / s.ncy);
        s.rgba = rgba;
        s.flags = flags;
        s.cos = 1;
        s.sin = 0;
        if(rotation) {
            rotation = (rotation + 0) * ((float)C_PI / 180);
            s.cos = cosf(rotation);
            s.sin = sinf(rotation);
        }

        batch_group_t *batches = &sprite_group[ flags & 7 ];
#if 0
        batch_t *found = map_find(*batches, texture.id);
        if( !found ) found = map_insert(*batches, texture.id, (batch_t){0});
#else
        batch_t *found = map_find_or_add(*batches, texture.id, (batch_t){0});
#endif

        array_push(found->sprites, s);

        if( flags & SPRITE_RESOLUTION_INDEPENDANT ) { // @todo: optimize me
        sprite_flush();
        camera_get_active()->position = bak;
        }
    }
}

void sprite( texture_t texture, float position[3], float rotation, unsigned color, unsigned flags) {
    float offset[2] = {0,0}, scale[2] = {1,1}, spritesheet[3] = {0,0,0};
    sprite_sheet( texture, spritesheet, position, rotation, offset, scale, color, flags );
}

static void sprite_rebuild_meshes() {
    sprite_count = 0;

    // w/2,h/2 centered
    for( int l = countof(sprite_group) / 2; l < countof(sprite_group); ++l) {
        for each_map_ptr(sprite_group[l], int,_, batch_t,bt) {

            bt->dirty = array_count(bt->sprites) ? 1 : 0;
            if( !bt->dirty ) continue;

            int index = 0;
            array_clear(sprite_indices);
            array_clear(sprite_vertices);

            array_foreach_ptr(bt->sprites, sprite_static_t,it ) {
                float x0 = it->ox - it->cellw/2, x3 = x0 + it->cellw;
                float y0 = it->oy - it->cellh/2, y3 = y0;
                float x1 = x0,                   x2 = x3;
                float y1 = y0 + it->cellh,       y2 = y1;

                // @todo: move this affine transform into glsl shader
                vec3 v0 = { it->px + ( x0 * it->cos - y0 * it->sin ), it->py + ( x0 * it->sin + y0 * it->cos ), it->pz };
                vec3 v1 = { it->px + ( x1 * it->cos - y1 * it->sin ), it->py + ( x1 * it->sin + y1 * it->cos ), it->pz };
                vec3 v2 = { it->px + ( x2 * it->cos - y2 * it->sin ), it->py + ( x2 * it->sin + y2 * it->cos ), it->pz };
                vec3 v3 = { it->px + ( x3 * it->cos - y3 * it->sin ), it->py + ( x3 * it->sin + y3 * it->cos ), it->pz };

                float cx = (1.0f / it->ncx) - 1e-9f;
                float cy = (1.0f / it->ncy) - 1e-9f;
                int idx = (int)it->frame;
                int px = idx % it->ncx;
                int py = idx / it->ncx;

                float ux = px * cx, uy = py * cy;
                float vx = ux + cx, vy = uy + cy;

                vec2 uv0 = vec2(ux, uy);
                vec2 uv1 = vec2(ux, vy);
                vec2 uv2 = vec2(vx, vy);
                vec2 uv3 = vec2(vx, uy);

                array_push( sprite_vertices, sprite_vertex(v0, uv0, it->rgba) ); // Vertex 0 (A)
                array_push( sprite_vertices, sprite_vertex(v1, uv1, it->rgba) ); // Vertex 1 (B)
                array_push( sprite_vertices, sprite_vertex(v2, uv2, it->rgba) ); // Vertex 2 (C)
                array_push( sprite_vertices, sprite_vertex(v3, uv3, it->rgba) ); // Vertex 3 (D)

                //      A--B                  A               A-B
                // quad |  | becomes triangle |\  and triangle \|
                //      D--C                  D-C               C
                GLuint A = (index+0), B = (index+1), C = (index+2), D = (index+3); index += 4;

                array_push( sprite_indices, sprite_index(C, D, A) ); // Triangle 1
                array_push( sprite_indices, sprite_index(C, A, B) ); // Triangle 2
            }

            mesh_update(&bt->mesh, "p3 t2 c4B", 0,array_count(sprite_vertices),sprite_vertices, 3*array_count(sprite_indices),sprite_indices, MESH_STATIC);

            // clear elements from queue
            sprite_count += array_count(bt->sprites);
            array_clear(bt->sprites);
        }
    }

    // (0,0) centered
    for( int l = 0; l < countof(sprite_group) / 2; ++l) {
        for each_map_ptr(sprite_group[l], int,_, batch_t,bt) {

            bt->dirty = array_count(bt->sprites) ? 1 : 0;
            if( !bt->dirty ) continue;

            int index = 0;
            array_clear(sprite_indices);
            array_clear(sprite_vertices);

            array_foreach_ptr(bt->sprites, sprite_static_t,it ) {
                float x0 = it->ox - it->cellw/2, x3 = x0 + it->cellw;
                float y0 = it->oy - it->cellh/2, y3 = y0;
                float x1 = x0,                   x2 = x3;
                float y1 = y0 + it->cellh,       y2 = y1;

                // @todo: move this affine transform into glsl shader
                vec3 v0 = { it->px + ( x0 * it->cos - y0 * it->sin ), it->py + ( x0 * it->sin + y0 * it->cos ), it->pz };
                vec3 v1 = { it->px + ( x1 * it->cos - y1 * it->sin ), it->py + ( x1 * it->sin + y1 * it->cos ), it->pz };
                vec3 v2 = { it->px + ( x2 * it->cos - y2 * it->sin ), it->py + ( x2 * it->sin + y2 * it->cos ), it->pz };
                vec3 v3 = { it->px + ( x3 * it->cos - y3 * it->sin ), it->py + ( x3 * it->sin + y3 * it->cos ), it->pz };

                float ux = it->x, vx = ux + it->w;
                float uy = it->y, vy = uy + it->h;

                vec2 uv0 = vec2(ux, uy);
                vec2 uv1 = vec2(ux, vy);
                vec2 uv2 = vec2(vx, vy);
                vec2 uv3 = vec2(vx, uy);

                array_push( sprite_vertices, sprite_vertex(v0, uv0, it->rgba) ); // Vertex 0 (A)
                array_push( sprite_vertices, sprite_vertex(v1, uv1, it->rgba) ); // Vertex 1 (B)
                array_push( sprite_vertices, sprite_vertex(v2, uv2, it->rgba) ); // Vertex 2 (C)
                array_push( sprite_vertices, sprite_vertex(v3, uv3, it->rgba) ); // Vertex 3 (D)

                //      A--B                  A               A-B
                // quad |  | becomes triangle |\  and triangle \|
                //      D--C                  D-C               C
                GLuint A = (index+0), B = (index+1), C = (index+2), D = (index+3); index += 4;

                array_push( sprite_indices, sprite_index(C, D, A) ); // Triangle 1
                array_push( sprite_indices, sprite_index(C, A, B) ); // Triangle 2
            }

            mesh_update(&bt->mesh, "p3 t2 c4B", 0,array_count(sprite_vertices),sprite_vertices, 3*array_count(sprite_indices),sprite_indices, MESH_STATIC);

            // clear elements from queue
            sprite_count += array_count(bt->sprites);
            array_clear(bt->sprites);
        }
    }
}

static void sprite_render_meshes_group(batch_group_t* sprites, int alpha_key, int alpha_value, float mvp[16]) {
    if( map_count(*sprites) > 0 ) {
        // setup shader
        if( sprite_program < 0 ) {
            sprite_program = shader( vfs_read("shaders/vs_324_24_sprite.glsl"), vfs_read("shaders/fs_24_4_sprite.glsl"),
                "att_Position,att_TexCoord,att_Color",
                "fragColor", NULL
            );
        }
        shader_bind(sprite_program);
        shader_mat44("u_mvp", mvp);

        // set (unit 0) in the uniform texture sampler, and render batch
        glActiveTexture(GL_TEXTURE0);
        glBlendFunc( alpha_key, alpha_value );

        for each_map_ptr(*sprites, int,texture_id, batch_t,bt) {
            if( bt->dirty ) {
                shader_texture_unit("u_texture", *texture_id, 0);
                mesh_render(&bt->mesh);
            }
        }
//      map_clear(*sprites);
    }
}

static void sprite_init() {
    do_once for(int i = 0; i < countof(sprite_group); ++i) {
    map_init(sprite_group[i], less_int, hash_int);
    }
}

void sprite_flush() {
    profile("Sprite.rebuild_time") {
        sprite_rebuild_meshes();
    }
    profile("Sprite.render_time") {
        // setup rendering state
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glDepthFunc(GL_LEQUAL); // try to help with zfighting

        // 3d
        mat44 mvp3d; multiply44x2(mvp3d, camera_get_active()->proj, camera_get_active()->view);
        // render all additive then translucent groups
        sprite_render_meshes_group(&sprite_group[SPRITE_PROJECTED], GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, mvp3d );
        sprite_render_meshes_group(&sprite_group[SPRITE_PROJECTED|SPRITE_CENTERED], GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, mvp3d );
        sprite_render_meshes_group(&sprite_group[SPRITE_PROJECTED|SPRITE_CENTERED|SPRITE_ADDITIVE], GL_SRC_ALPHA, GL_ONE, mvp3d );
        sprite_render_meshes_group(&sprite_group[SPRITE_PROJECTED|SPRITE_ADDITIVE], GL_SRC_ALPHA, GL_ONE, mvp3d );

        // 2d: (0,0) is center of screen
        mat44 mvp2d;
        vec3 pos = camera_get_active()->position;
        float zoom = absf(pos.z); if(zoom < 0.1f) zoom = 0.1f; zoom = 1.f / (zoom + !zoom);
        float zdepth_max = window_height(); // 1;
        float l = pos.x - window_width()  * zoom / 2;
        float r = pos.x + window_width()  * zoom / 2;
        float b = pos.y + window_height() * zoom / 2;
        float t = pos.y - window_height() * zoom / 2;
        ortho44(mvp2d, l,r,b,t, -zdepth_max, +zdepth_max);
        // render all additive then translucent groups
        sprite_render_meshes_group(&sprite_group[0], GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, mvp2d );
        sprite_render_meshes_group(&sprite_group[SPRITE_CENTERED], GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, mvp2d );
        sprite_render_meshes_group(&sprite_group[SPRITE_CENTERED|SPRITE_ADDITIVE], GL_SRC_ALPHA, GL_ONE, mvp2d );
        sprite_render_meshes_group(&sprite_group[SPRITE_ADDITIVE], GL_SRC_ALPHA, GL_ONE, mvp2d );

        // restore rendering state
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glDepthFunc(GL_LESS);
        glUseProgram(0);
    }
}

// -----------------------------------------------------------------------------
// tilemaps

tilemap_t tilemap(const char *map, int blank_chr, int linefeed_chr) {
    tilemap_t t = {0};
    t.tint = ~0u; // WHITE
    t.blank_chr = blank_chr;
    for( ; *map ; ++map ) {
        if( map[0] == linefeed_chr ) ++t.rows;
        else {
            array_push(t.map, map[0]);
            ++t.cols;
        }
    }
    return t;
}

void tilemap_render_ext( tilemap_t m, tileset_t t, float zindex, float xy_zoom[3], float tilt, unsigned tint, bool is_additive ) {
    vec3 old_pos = camera_get_active()->position;
    sprite_flush();
    camera_get_active()->position = vec3(window_width()/2,window_height()/2,1);

    float scale[2] = {xy_zoom[2], xy_zoom[2]};
    xy_zoom[2] = zindex;

    float offset[2] = {0,0};
    float spritesheet[3] = {0,t.cols,t.rows}; // selected tile index and spritesheet dimensions (cols,rows)

    for( unsigned y = 0, c = 0; y < m.rows; ++y ) {
        for( unsigned x = 0; x < m.cols; ++x, ++c ) {
            if( m.map[c] != m.blank_chr ) {
                spritesheet[0] = m.map[c];
                sprite_sheet(t.tex, spritesheet, xy_zoom, tilt, offset, scale, tint, is_additive ? SPRITE_ADDITIVE : 0);
            }
            offset[0] += t.tile_w;
        }
        offset[0] = 0, offset[1] += t.tile_h;
    }

    sprite_flush();
    camera_get_active()->position = old_pos;
}

void tilemap_render( tilemap_t map, tileset_t set ) {
    map.position.x += set.tile_w;
    map.position.y += set.tile_h;
    tilemap_render_ext( map, set, map.zindex, &map.position.x, map.tilt, map.tint, map.is_additive );
}

tileset_t tileset(texture_t tex, unsigned tile_w, unsigned tile_h, unsigned cols, unsigned rows) {
    tileset_t t = {0};
    t.tex = tex;
    t.cols = cols, t.rows = rows;
    t.tile_w = tile_w, t.tile_h = tile_h;
    return t;
}

int ui_tileset( tileset_t t ) {
    ui_subimage(va("Selection #%d (%d,%d)", t.selected, t.selected % t.cols, t.selected / t.cols), t.tex.id, t.tex.w, t.tex.h, (t.selected % t.cols) * t.tile_w, (t.selected / t.cols) * t.tile_h, t.tile_w, t.tile_h);
    int choice;
    if( (choice = ui_image(0, t.tex.id, t.tex.w,t.tex.h)) ) {
        int px = ((choice / 100) / 100.f) * t.tex.w / t.tile_w;
        int py = ((choice % 100) / 100.f) * t.tex.h / t.tile_h;
        t.selected = px + py * t.cols;
    }
    // if( (choice = ui_buttons(3, "load", "save", "clear")) ) {}
    return t.selected;
}

// -----------------------------------------------------------------------------
// tiled

tiled_t tiled(const char *file_tmx) {
    tiled_t zero = {0}, ti = zero;

    // read file and parse json
    if( !xml_push(file_tmx) ) return zero;

    // sanity checks
    bool supported = !strcmp(xml_string("/map/@orientation"), "orthogonal") && !strcmp(xml_string("/map/@renderorder"), "right-down");
    if( !supported ) return xml_pop(), zero;

    // tileset
    const char *file_tsx = xml_string("/map/tileset/@source");
    if( !xml_push(vfs_read(file_tsx)) ) return zero;
        const char *set_src = xml_string("/tileset/image/@source");
        int set_w = xml_int("/tileset/@tilewidth");
        int set_h = xml_int("/tileset/@tileheight");
        int set_c = xml_int("/tileset/@columns");
        int set_r = xml_int("/tileset/@tilecount") / set_c;
        tileset_t set = tileset(texture(set_src,0), set_w, set_h, set_c, set_r );
    xml_pop();

    // actual parsing
    ti.w = xml_int("/map/@width");
    ti.h = xml_int("/map/@height");
    ti.tilew = xml_int("/map/@tilewidth");
    ti.tileh = xml_int("/map/@tileheight");
    ti.first_gid = xml_int("/map/tileset/@firstgid");
    ti.map_name = STRDUP( xml_string("/map/tileset/@source") ); // @leak

    for(int l = 0, layers = xml_count("/map/layer"); l < layers; ++l ) {
        if( strcmp(xml_string("/map/layer[%d]/data/@encoding",l), "base64") || strcmp(xml_string("/map/layer[%d]/data/@compression",l), "zlib") ) {
            PRINTF("Warning: layer encoding not supported: '%s' -> layer '%s'\n", file_tmx, *array_back(ti.names));
            continue;
        }

        int cols = xml_int("/map/layer[%d]/@width",l);
        int rows = xml_int("/map/layer[%d]/@height",l);

        tilemap_t tm = tilemap("", ' ', '\n');
        tm.blank_chr = ~0u; //ti.first_gid - 1;
        tm.cols = cols;
        tm.rows = rows;
        array_resize(tm.map, tm.cols * tm.rows);
        memset(tm.map, 0xFF, tm.cols * tm.rows * sizeof(int));

        for( int c = 0, chunks = xml_count("/map/layer[%d]/data/chunk", l); c <= chunks; ++c ) {
            int cw, ch;
            int cx, cy;
            array(char) b64 = 0;

            if( !chunks ) { // non-infinite mode
                b64 = xml_blob("/map/layer[%d]/data/$",l);
                cw = tm.cols, ch = tm.rows;
                cx = 0, cy = 0;
            } else { // infinite mode
                b64 = xml_blob("/map/layer[%d]/data/chunk[%d]/$",l,c);
                cw = xml_int("/map/layer[%d]/data/chunk[%d]/@width",l,c), ch = xml_int("/map/layer[%d]/data/chunk[%d]/@height",l,c); // 20x20
                cx = xml_int("/map/layer[%d]/data/chunk[%d]/@x",l,c), cy = xml_int("/map/layer[%d]/data/chunk[%d]/@y",l,c); // (-16,-32)
                cx = abs(cx), cy = abs(cy);
            }

            int outlen = cw * ch * 4;
            static __thread int *out = 0; out = (int *)REALLOC( 0, outlen + zexcess(COMPRESS_ZLIB) ); // @leak
            if( zdecode( out, outlen, b64, array_count(b64), COMPRESS_ZLIB ) > 0 ) {
                for( int y = 0, p = 0; y < ch; ++y ) {
                    for( int x = 0; x < cw; ++x, ++p ) {
                        if( out[p] >= ti.first_gid ) {
                            int offset = (x + cx) + (y + cy) * tm.cols;
                            if( offset >= 0 && offset < (cw * ch) )
                            tm.map[ offset ] = out[ p ] - ti.first_gid;
                        }
                    }
                }
            }
            else {
                PRINTF("Warning: bad zlib stream: '%s' -> layer #%d -> chunk #%d\n", file_tmx, l, c);
            }

            array_free(b64);
        }

        array_push(ti.layers, tm);
        array_push(ti.names, STRDUP(xml_string("/map/layer[%d]/@name",l)));
        array_push(ti.visible, true);
        array_push(ti.sets, set);
    }

    xml_pop();
    return ti;
}

void tiled_render(tiled_t tmx, vec3 pos) {
    for( unsigned i = 0, end = array_count(tmx.layers); i < end; ++i ) {
        tmx.layers[i].position = pos; // add3(camera_get_active()->position, pos);
        if( tmx.parallax ) tmx.layers[i].position.x /= (3+i), tmx.layers[i].position.y /= (5+i);
        if( tmx.visible[i] ) tilemap_render(tmx.layers[i], tmx.sets[i]);
    }
}

void ui_tiled(tiled_t *t) {
    ui_label2("Loaded map", t->map_name ? t->map_name : "(none)");
    ui_label2("Map dimensions", va("%dx%d", t->w, t->h));
    ui_label2("Tile dimensions", va("%dx%d", t->tilew, t->tileh));
    ui_separator();
    ui_bool("Parallax", &t->parallax);
    ui_separator();
    ui_label2("Layers", va("%d", array_count(t->layers)));
    for( int i = 0; i < array_count(t->layers); ++i ) {
        if( ui_label2_toolbar(va("- %s (%dx%d)", t->names[i], t->layers[i].cols, t->layers[i].rows ), t->visible[i] ? "\xee\xa3\xb4" : "\xee\xa3\xb5") > 0 ) { // ICON_MD_VISIBILITY / ICON_MD_VISIBILITY_OFF
            t->visible[i] ^= true;
        }
    }
    ui_separator();
    if( ui_collapse(va("Sets: %d", array_count(t->layers)), va("%p",t))) {
        for( int i = 0; i < array_count(t->layers); ++i ) {
            if( ui_collapse(va("%d", i+1), va("%p%d",t,i)) ) {
                t->sets[i].selected = ui_tileset( t->sets[i] );
                ui_collapse_end();
            }
        }
        ui_collapse_end();
    }
}

// -----------------------------------------------------------------------------
// spine json loader (wip)
// - rlyeh, public domain
//
// [ref] http://es.esotericsoftware.com/spine-json-format
//
// notable misses:
// - mesh deforms
// - cubic beziers
// - shears
// - bounding boxes

enum { SPINE_MAX_BONES = 64 }; // max bones

typedef struct spine_bone_t {
    char *name, *parent;
    struct spine_bone_t *parent_bone;

    float z; // draw order usually matches bone-id. ie, zindex == bone_id .. root(0) < chest (mid) < finger(top)

    float len;
    float x, y, deg;        // base
    float x2, y2, deg2;     // accum / temporaries during bone transform time
    float x3, y3, deg3;     // values from timeline

    unsigned rect_id;
    unsigned atlas_id;
} spine_bone_t;

typedef struct spine_slot_t {
    char *name, *bone, *attach;
} spine_slot_t;

typedef struct spine_rect_t {
    char *name;
    float x,y,w,h,sx,sy,deg;
} spine_rect_t;

typedef struct spine_skin_t {
    char *name;
    array(spine_rect_t) rects;
} spine_skin_t;

typedef struct spine_animkey_t { // offline; only during loading
    float time, curve[4];        // time is mandatory, curve is optional
    union {
        char *name;              // type: attachment (mode-1)
        struct { float deg; };   // type: rotate (mode-2)
        struct { float x,y; };   // type: translate (mode-3)
    };
} spine_animkey_t;

#if 0
typedef struct spine_pose_t { // runtime; only during playing
    unsigned frame;
    array(vec4) xform; // entry per bone. translation(x,y),rotation(z),attachment-id(w)
} spine_pose_t;
#endif

typedef struct spine_anim_t {
    char *name;
    union {
#if 0
        struct {
            unsigned frames;
            array(spine_pose_t) poses;
        };
#endif
        struct {
            array(spine_animkey_t) attach_keys[SPINE_MAX_BONES];
            array(spine_animkey_t) rotate_keys[SPINE_MAX_BONES];
            array(spine_animkey_t) translate_keys[SPINE_MAX_BONES];
        };
    };
} spine_anim_t;

typedef struct spine_atlas_t {
    char *name;
    float x,y,w,h,deg;
} spine_atlas_t;

typedef struct spine_t {
    char *name;
    texture_t texture;
    unsigned skin;
    array(spine_bone_t) bones;
    array(spine_slot_t) slots;
    array(spine_skin_t) skins;
    array(spine_anim_t) anims;
    array(spine_atlas_t) atlas;
    // anim controller
    unsigned inuse;
    float time, maxtime;
    unsigned debug_atlas_id;
} spine_t;

// ---

static
void spine_convert_animkeys_to_animpose(spine_anim_t *input) {
    spine_anim_t copy = *input; // @todo
    // @leak: attach/rot/tra keys
}

static
int find_bone_id(spine_t *s, const char *bone_name) {
    for( unsigned i = 0, end = array_count(s->bones); i < end; ++i )
        if( !strcmp(s->bones[i].name, bone_name)) return i;
    return -1;
}
static
spine_bone_t *find_bone(spine_t *s, const char *bone_name) {
    int bone_id = find_bone_id(s, bone_name);
    return bone_id >= 0 ? &s->bones[bone_id] : NULL;
}

void spine_skin(spine_t *p, unsigned skin) {
    if( !p->texture.id ) return;
    if( skin >= array_count(p->skins) ) return;

    p->skin = skin;

    char *skin_name = va("%s/", p->skins[skin].name);
    int header = strlen(skin_name);

    for( int i = 0; i < array_count(p->atlas); ++i) {
        if(!strbeg(p->atlas[i].name, skin_name)) continue;

        int bone_id = find_bone_id(p, p->atlas[i].name+header );
        if( bone_id < 0 ) continue;

        p->bones[bone_id].atlas_id = i;
    }

    for( int i = 0; i < array_count(p->skins[p->skin].rects); ++i) {
        int bone_id = find_bone_id(p, p->skins[p->skin].rects[i].name );
        if( bone_id < 0 ) continue;

        p->bones[bone_id].rect_id = i;
    }
}

static
bool spine_(spine_t *t, const char *file_json, const char *file_atlas, unsigned flags) {
    char *atlas = vfs_read(file_atlas);
    if(!atlas || !atlas[0]) return false;

    memset(t, 0, sizeof(spine_t));

    // goblins.png
    //   size: 1024, 128
    //   filter: Linear, Linear
    //   pma: true
    // dagger
    //   bounds: 2, 18, 26, 108
    // goblin/eyes-closed
    //   bounds: 2, 4, 34, 12
    spine_atlas_t *sa = 0;
    const char *last_id = 0;
    const char *texture_name = 0;
    const char *texture_filter = 0;
    const char *texture_format = 0;
    const char *texture_repeat = 0;
    float texture_width = 0, texture_height = 0, temp;
    for each_substring(atlas, "\r\n", it) {
        it += strspn(it, " \t\f\v");
        /**/ if( strbeg(it, "pma:" ) || strbeg(it, "index:") ) {} // ignored
        else if( strbeg(it, "size:" ) ) sscanf(it+5, "%f,%f", &texture_width, &texture_height);
        else if( strbeg(it, "rotate:" ) ) { float tmp; tmp=sa->w,sa->w=sa->h,sa->h=tmp; sa->deg = 90; } // assert(val==90)
        else if( strbeg(it, "repeat:" ) ) texture_repeat = it+7; // temp string
        else if( strbeg(it, "filter:" ) ) texture_filter = it+7; // temp string
        else if( strbeg(it, "format:" ) ) texture_format = it+7; // temp string
        else if( strbeg(it, "bounds:" ) ) {
            sscanf(it+7, "%f,%f,%f,%f", &sa->x, &sa->y, &sa->w, &sa->h);
        }
        else if( !texture_name ) texture_name = va("%s", it);
        else {
            array_push(t->atlas, ((spine_atlas_t){0}) );
            sa = &t->atlas[array_count(t->atlas) - 1];
            sa->name = STRDUP(it);
        }
    }
    for( int i = 0; i < array_count(t->atlas); ++i ) {
        sa = &t->atlas[i];
        sa->x /= texture_width, sa->y /= texture_height;
        sa->w /= texture_width, sa->h /= texture_height;
    }

    if(!texture_name) return false;

    t->texture = texture(texture_name, TEXTURE_LINEAR);

    json_push(vfs_read(file_json)); // @fixme: json_push_from_file() ?

    array_resize(t->bones, json_count("/bones"));
    array_reserve(t->slots, json_count("/slots"));
    array_resize(t->skins, json_count("/skins"));
    array_resize(t->anims, json_count("/animations"));

    for( int i = 0, end = json_count("/bones"); i < end; ++i ) {
        spine_bone_t v = {0};
        v.name = STRDUP(json_string("/bones[%d]/name", i));
        v.parent = STRDUP(json_string("/bones[%d]/parent", i));
        v.x = json_float("/bones[%d]/x", i);
        v.y = json_float("/bones[%d]/y", i);
        v.z = i;
        v.len = json_float("/bones[%d]/length", i);
        v.deg = json_float("/bones[%d]/rotation", i);
        t->bones[i] = v;

        for( int j = i-1; j > 0; --j ) {
            if( strcmp(t->bones[j].name,v.parent) ) continue;
            t->bones[i].parent_bone = &t->bones[j];
            break;
        }
    }

    for( int i = 0, end = json_count("/slots"); i < end; ++i ) {
        spine_slot_t v = {0};
        v.name = STRDUP(json_string("/slots[%d]/name", i));
        v.bone = STRDUP(json_string("/slots[%d]/bone", i));
        v.attach = STRDUP(json_string("/slots[%d]/attachment", i));

        array_push(t->slots, v);

        // slots define draw-order. so, update draw-order/zindex in bone
        spine_bone_t *b = find_bone(t, v.name);
        if( b ) b->z = i;
    }

    for( int i = 0, end = json_count("/skins"); i < end; ++i ) {
        spine_skin_t v = {0};
        v.name = STRDUP(json_string("/skins[%d]/name", i));

        for( int j = 0, jend = json_count("/skins[%d]/attachments",i); j < jend; ++j ) // /skins/default/
        for( int k = 0, kend = json_count("/skins[%d]/attachments[%d]",i,j); k < kend; ++k ) { // /skins/default/left hand item/
            spine_rect_t r = {0};
            r.name = STRDUP(json_key("/skins[%d]/attachments[%d][%d]",i,j,k)); // stringf("%s-%s-%s", json_key("/skins[%d]",i), json_key("/skins[%d][%d]",i,j), json_key("/skins[%d][%d][%d]",i,j,k));
            r.x = json_float("/skins[%d]/attachments[%d][%d]/x",i,j,k);
            r.y = json_float("/skins[%d]/attachments[%d][%d]/y",i,j,k);
            r.sx= json_float("/skins[%d]/attachments[%d][%d]/scaleX",i,j,k); r.sx += !r.sx;
            r.sy= json_float("/skins[%d]/attachments[%d][%d]/scaleY",i,j,k); r.sy += !r.sy;
            r.w = json_float("/skins[%d]/attachments[%d][%d]/width",i,j,k);
            r.h = json_float("/skins[%d]/attachments[%d][%d]/height",i,j,k);
            r.deg = json_float("/skins[%d]/attachments[%d][%d]/rotation",i,j,k);
            array_push(v.rects, r);
        }

        t->skins[i] = v;
    }

#if 1
    // simplify:
    // merge /skins/default into existing /skins/*, then delete /skins/default
    if( array_count(t->skins) > 1 ) {
        for( int i = 1; i < array_count(t->skins); ++i ) {
            for( int j = 0; j < array_count(t->skins[0].rects); ++j ) {
                array_push(t->skins[i].rects, t->skins[0].rects[j]);
            }
        }
        // @leak @fixme: FREE(t->skins[0])
        for( int i = 0; i < array_count(t->skins)-1; ++i ) {
            t->skins[i] = t->skins[i+1];
        }
        array_pop(t->skins);
    }
#endif

    for( int i = 0, end = json_count("/animations"); i < end; ++i ) {
        int id;
        const char *name;

        spine_anim_t v = {0};
        v.name = STRDUP(json_key("/animations[%d]", i));

        // slots / attachments

        for( int j = 0, jend = json_count("/animations[%d]/slots",i); j < jend; ++j )
        for( int k = 0, kend = json_count("/animations[%d]/slots[%d]",i,j); k < kend; ++k ) // ids
        {
            int bone_id = find_bone_id(t, json_key("/animations[%d]/bones[%d]",i,j));
            if( bone_id < 0 ) continue;

            for( int l = 0, lend = json_count("/animations[%d]/slots[%d][%d]",i,j,k); l < lend; ++l ) { // channels (rot,tra,attach)
                spine_animkey_t key = {0};

                key.name = STRDUP(json_string("/animations[%d]/slots[%d][%d][%d]/name",i,j,k,l));
                key.time = json_float("/animations[%d]/slots[%d][%d][%d]/time",i,j,k,l);
                if( json_count("/animations[%d]/slots[%d][%d][%d]/curve",i,j,k,l) == 4 ) {
                key.curve[0] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[0]",i,j,k,l);
                key.curve[1] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[1]",i,j,k,l);
                key.curve[2] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[2]",i,j,k,l);
                key.curve[3] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[3]",i,j,k,l);
                }

                // @todo: convert name to id
                // for(id = 0; t->bones[id].name && strcmp(t->bones[id].name,key.name); ++id)
                // printf("%s vs %s\n", key.name, t->bones[id].name);

                array_push(v.attach_keys[bone_id], key);
            }
        }

        // bones

        for( int j = 0, jend = json_count("/animations[%d]/bones",i); j < jend; ++j ) // slots or bones
        for( int k = 0, kend = json_count("/animations[%d]/bones[%d]",i,j); k < kend; ++k ) { // bone ids
            int bone_id = find_bone_id(t, json_key("/animations[%d]/bones[%d]",i,j));
            if( bone_id < 0 ) continue;

            // parse bones
            for( int l = 0, lend = json_count("/animations[%d]/bones[%d][%d]",i,j,k); l < lend; ++l ) { // channels (rot,tra,attach)
                const char *channel = json_key("/animations[%d]/bones[%d][%d]",i,j,k);
                int track = !strcmp(channel, "rotate") ? 1 : !strcmp(channel, "translate") ? 2 : 0;
                if( !track ) continue;

                spine_animkey_t key = {0};

                key.time = json_float("/animations[%d]/bones[%d][%d][%d]/time",i,j,k,l);
                if( json_count("/animations[%d]/bones[%d][%d][%d]/curve",i,j,k,l) == 4 ) {
                key.curve[0] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[0]",i,j,k,l);
                key.curve[1] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[1]",i,j,k,l);
                key.curve[2] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[2]",i,j,k,l);
                key.curve[3] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[3]",i,j,k,l);
                }

                if( track == 1 )
                key.deg = json_float("/animations[%d]/bones[%d][%d][%d]/value",i,j,k,l), // "/angle"
                array_push(v.rotate_keys[bone_id], key);
                else
                key.x = json_float("/animations[%d]/bones[%d][%d][%d]/x",i,j,k,l),
                key.y = json_float("/animations[%d]/bones[%d][%d][%d]/y",i,j,k,l),
                array_push(v.translate_keys[bone_id], key);
            }
        }

        t->anims[i] = v;
    }

    json_pop();

    spine_skin(t, 0);

    return true;
}

spine_t* spine(const char *file_json, const char *file_atlas, unsigned flags) {
    spine_t *t = MALLOC(sizeof(spine_t));
    if( !spine_(t, file_json, file_atlas, flags) ) return FREE(t), NULL;
    return t;
}

void spine_render(spine_t *p, vec3 offset, unsigned flags) {
    if( !p->texture.id ) return;
    if( !flags ) return;

    ddraw_push_2d();
        // if( flags & 2 ) ddraw_line(vec3(0,0,0), vec3(window_width(),window_height(),0));
        // if( flags & 2 ) ddraw_line(vec3(window_width(),0,0), vec3(0,window_height(),0));

        // int already_computed[SPINE_MAX_BONES] = {0}; // @fixme: optimize: update longest chains first, then remnant branches

        for( int i = 1; i < array_count(p->bones); ++i ) {
            spine_bone_t *self = &p->bones[i];
            if( !self->rect_id ) continue;

            int num_bones = 0;
            static array(spine_bone_t*) chain = 0; array_resize(chain, 0);
            for( spine_bone_t *next = self; next ; next = next->parent_bone, ++num_bones ) {
                array_push(chain, next);
            }

            vec3 target = {0}, prev = {0};
            for( int j = 0, end = array_count(chain); j < end; ++j ) { // traverse from root(skipped) -> `i` bone direction
                int j_opposite = end - 1 - j;

                spine_bone_t *b = chain[j_opposite]; // bone
                spine_bone_t *pb = b->parent_bone; // parent bone

                float pb_x2 = 0, pb_y2 = 0, pb_deg2 = 0;
                if( pb ) pb_x2 = pb->x2, pb_y2 = pb->y2, pb_deg2 = pb->deg2;

                const float deg2rad = C_PI / 180;
                b->x2 =      b->x3 + pb_x2   + b->x * cos( -pb_deg2 * deg2rad ) - b->y * sin( -pb_deg2 * deg2rad );
                b->y2 =     -b->y3 + pb_y2   - b->y * cos(  pb_deg2 * deg2rad ) + b->x * sin(  pb_deg2 * deg2rad );
                b->deg2 = -b->deg3 + pb_deg2 - b->deg;

                prev = target;
                target = vec3(b->x2,b->y2,b->deg2);
            }

            target.z = 0;
            target = add3(target, offset);
            prev.z = 0;
            prev = add3(prev, offset);

            if( flags & 2 ) {
                ddraw_point( target );
                ddraw_text( target, -0.25f, self->name );
                ddraw_bone( prev, target ); // from parent to bone
            }
            if( flags & 1 ) {
                spine_atlas_t *a = &p->atlas[self->atlas_id];
                spine_rect_t *r = &p->skins[p->skin].rects[self->rect_id];

                vec4 rect = ptr4(&a->x);
                float zindex = self->z;
                float offsx = 0;
                float offsy = 0;
                float tilt = self->deg2 + (a->deg - r->deg);
                unsigned tint = self->atlas_id == p->debug_atlas_id ? 0xFF<<24 | 0xFF : ~0u;

                if( 1 ) {
                    vec3 dir = vec3(r->x,r->y,0);
                    dir = rotatez3(dir, self->deg2);
                    offsx = dir.x * r->sx;
                    offsy = dir.y * r->sy;
                }

                sprite_rect(p->texture, rect, vec4(target.x,target.y,0,zindex), vec4(1,1,offsx,offsy), tilt, tint, 0);
            }
         }

    ddraw_pop_2d();
    ddraw_flush();
}

static
void spine_animate_(spine_t *p, float *time, float *maxtime, float delta) {
    if( !p->texture.id ) return;

    if( delta > 1/120.f ) delta = 1/120.f;
    if( *time >= *maxtime ) *time = 0; else *time += delta;

    // reset root // needed?
    p->bones[0].x2 = 0;
    p->bones[0].y2 = 0;
    p->bones[0].deg2 = 0;
    p->bones[0].x3 = 0;
    p->bones[0].y3 = 0;
    p->bones[0].deg3 = 0;

    for( int i = 0, end = array_count(p->bones); i < end; ++i) {
        // @todo: attach channel
        // @todo: per channel: if curve == linear || curve == stepped || array_count(curve) == 4 {...}
        for each_array_ptr(p->anims[p->inuse].rotate_keys[i], spine_animkey_t, r) {
            double r0 = r->time;
            *maxtime = maxf( *maxtime, r0 );
            if( absf(*time - r0) < delta ) {
                p->bones[i].deg3 = r->deg;
            }
        }
        for each_array_ptr(p->anims[p->inuse].translate_keys[i], spine_animkey_t, r) {
            double r0 = r->time;
            *maxtime = maxf( *maxtime, r0 );
            if( absf(*time - r0) < delta ) {
                p->bones[i].x3 = r->x;
                p->bones[i].y3 = r->y;
            }
        }
    }
}

void spine_animate(spine_t *p, float delta) {
    spine_animate_(p, &p->time, &p->maxtime, delta);
}

void ui_spine(spine_t *p) {
    if( ui_collapse(va("Anims: %d", array_count(p->anims)), va("%p-a", p))) {
        for each_array_ptr(p->anims, spine_anim_t, q) {
            if(ui_slider2("", &p->time, va("%.2f/%.0f %.2f%%", p->time, p->maxtime, p->time * 100.f))) {
                spine_animate(p, 0);
            }

            int choice = ui_label2_toolbar(q->name, ICON_MD_PAUSE_CIRCLE " " ICON_MD_PLAY_CIRCLE);
            if( choice == 1 ) window_pause( 0 ); // play
            if( choice == 2 ) window_pause( 1 ); // pause

            for( int i = 0; i < SPINE_MAX_BONES; ++i ) {
                ui_separator();
                ui_label(va("Bone %d: Attachment keys", i));
                for each_array_ptr(q->attach_keys[i], spine_animkey_t, r) {
                    ui_label(va("%.2f [%.2f %.2f %.2f %.2f] %s", r->time, r->curve[0], r->curve[1], r->curve[2], r->curve[3], r->name));
                }
                ui_label(va("Bone %d: Rotate keys", i));
                for each_array_ptr(q->rotate_keys[i], spine_animkey_t, r) {
                    ui_label(va("%.2f [%.2f %.2f %.2f %.2f] %.2f deg", r->time, r->curve[0], r->curve[1], r->curve[2], r->curve[3], r->deg));
                }
                ui_label(va("Bone %d: Translate keys", i));
                for each_array_ptr(q->translate_keys[i], spine_animkey_t, r) {
                    ui_label(va("%.2f [%.2f %.2f %.2f %.2f] (%.2f,%.2f)", r->time, r->curve[0], r->curve[1], r->curve[2], r->curve[3], r->x, r->y));
                }
            }
        }
        ui_collapse_end();
    }
    if( ui_collapse(va("Bones: %d", array_count(p->bones)), va("%p-b", p))) {
        for each_array_ptr(p->bones, spine_bone_t, q)
        if( ui_collapse(q->name, va("%p-b2", q)) ) {
            ui_label2("Parent:", q->parent);
            ui_label2("X:", va("%.2f", q->x));
            ui_label2("Y:", va("%.2f", q->y));
            ui_label2("Length:", va("%.2f", q->len));
            ui_label2("Rotation:", va("%.2f", q->deg));
            ui_collapse_end();
        }
        ui_collapse_end();
    }
    if( ui_collapse(va("Slots: %d", array_count(p->slots)), va("%p-s", p))) {
        for each_array_ptr(p->slots, spine_slot_t, q)
        if( ui_collapse(q->name, va("%p-s2", q)) ) {
            ui_label2("Bone:", q->bone);
            ui_label2("Attachment:", q->attach);
            ui_collapse_end();
        }
        ui_collapse_end();
    }
    if( ui_collapse(va("Skins: %d", array_count(p->skins)), va("%p-k", p))) {
        for each_array_ptr(p->skins, spine_skin_t, q)
        if( ui_collapse(q->name, va("%p-k2", q)) ) {
            for each_array_ptr(q->rects, spine_rect_t, r)
            if( ui_collapse(r->name, va("%p-k3", r)) ) {
                ui_label2("X:", va("%.2f", r->x));
                ui_label2("Y:", va("%.2f", r->y));
                ui_label2("Scale X:", va("%.2f", r->sx));
                ui_label2("Scale Y:", va("%.2f", r->sy));
                ui_label2("Width:", va("%.2f", r->w));
                ui_label2("Height:", va("%.2f", r->h));
                ui_label2("Rotation:", va("%.2f", r->deg));
                ui_collapse_end();

                spine_bone_t *b = find_bone(p, r->name);
                if( b ) {
                    p->debug_atlas_id = b->atlas_id;

                    static float tilt = 0;
                    if( input(KEY_LCTRL) ) tilt += 60*1/60.f; else tilt = 0;
                    spine_atlas_t *r = p->atlas + b->atlas_id;
                    sprite_flush();
                    camera_get_active()->position = vec3(0,0,2);
                        vec4 rect = ptr4(&r->x); float zindex = 0; vec4 scale_offset = vec4(1,1,0,0);
                        sprite_rect(p->texture, ptr4(&r->x), vec4(0,0,0,zindex), scale_offset, r->deg + tilt, ~0u, 0);
                        sprite_flush();
                    camera_get_active()->position = vec3(+window_width()/3,window_height()/2.25,2);
                }
            }
            ui_collapse_end();
        }
        ui_collapse_end();
    }

    if( ui_int("Use skin", &p->skin) ) {
    p->skin = clampf(p->skin, 0, array_count(p->skins) - 1);
    spine_skin(p, p->skin);
    }

    if( p->texture.id ) ui_texture(0, p->texture);
}

// ----------------------------------------------------------------------------

// texture_t texture_createclip(unsigned cx,unsigned cy,unsigned cw,unsigned ch, unsigned tw,unsigned th,unsigned tn,void *pixels, unsigned flags) {
//     return texture_create(tw,th,tn,pixels,flags);
//     static array(unsigned) clip = 0;
//     array_resize(clip, cw*ch*4);
//     for( unsigned y = 0; y < ch; ++y )
//     memcpy((char *)clip + (0+(0+y)*cw)*tn, (char*)pixels + (cx+(cy+y)*tw)*tn, cw*tn);
//     return texture_create(cw,ch,tn,clip,flags);
// }

typedef unsigned quark_t;

#define array_reserve_(arr,x) (array_count(arr) > (x) ? (arr) : array_resize(arr, 1+(x)))

#define ui_array(label,type,ptr) do { \
    int changed = 0; \
    if( ui_collapse(label, va(#type "%p",ptr)) ) { \
        char label_ex[8]; \
        for( int idx = 0, iend = array_count(*(ptr)); idx < iend; ++idx ) { \
            type* it = *(ptr) + idx; \
            snprintf(label_ex, sizeof(label_ex), "[%d]", idx); \
            changed |= ui_##type(label_ex, it); \
        } \
        ui_collapse_end(); \
    } \
} while(0)

int ui_vec2i(const char *label, vec2i *v) { return ui_unsigned2(label, (unsigned*)v); }
int ui_vec3i(const char *label, vec3i *v) { return ui_unsigned3(label, (unsigned*)v); }
int ui_vec2(const char *label, vec2 *v) { return ui_float2(label, (float*)v); }
int ui_vec3(const char *label, vec3 *v) { return ui_float3(label, (float*)v); }
int ui_vec4(const char *label, vec4 *v) { return ui_float4(label, (float*)v); }

char *trimspace(char *str) {
    for( char *s = str; *s; ++s )
        if(*s <= 32) memmove(s, s+1, strlen(s));
    return str;
}

char *file_parent(const char *f) {   // folder/folder/abc
    char *p = file_path(f);          // folder/folder/
    char *last = strrchr(p, '/');    //              ^
    if( !last ) return p;            // return parent if no sep
    *last = '\0';                    // folder/folder
    last = strrchr(p, '/');          //       ^
    return last ? last + 1 : p;      // return parent if no sep
}

int ui_obj(const char *fmt, obj *o) {
    int changed = 0, item = 1;
    for each_objmember(o, TYPE,NAME,PTR) {
        char *label = va(fmt, NAME);
        /**/ if(!strcmp(TYPE,"float"))    { if(ui_float(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"int"))      { if(ui_int(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"unsigned")) { if(ui_unsigned(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"vec2"))     { if(ui_float2(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"vec3"))     { if(ui_float3(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"vec4"))     { if(ui_float4(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"rgb"))      { if(ui_color3(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"rgba"))     { if(ui_color4(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"color"))    { if(ui_color4f(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"color3f"))  { if(ui_color3f(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"color4f"))  { if(ui_color4f(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"char*"))    { if(ui_string(label, PTR)) changed = item; }
        else ui_label2(label, va("(%s)", TYPE)); // INFO instead of (TYPE)?
        ++item;
    }
    return changed;
}

#define OBJTYPEDEF2(...) OBJTYPEDEF(__VA_ARGS__); AUTORUN

// ----------------------------------------------------------------------------
// atlas

int ui_atlas_frame(atlas_frame_t *f) {
    ui_unsigned("delay", &f->delay);
    ui_vec4("sheet", &f->sheet);
    ui_array("indices", vec3i, &f->indices);
    ui_array("coords", vec2, &f->coords);
    ui_array("uvs", vec2, &f->uvs);
    return 0;
}

int ui_atlas_slice_frame(atlas_slice_frame_t *f) {
    ui_vec4("bounds", &f->bounds);
    ui_bool("9-slice", &f->has_9slice);
    ui_vec4("core", &f->core);
    return 0;
}

int ui_atlas(atlas_t *a) {
    int changed = 0;
    ui_texture(NULL, a->tex);
    for( int i = 0; i < array_count(a->anims); ++i ) {
        if( ui_collapse(quark_string(&a->db, a->anims[i].name), va("%p%d", a, a->anims[i].name) ) ) {
            changed = i+1;
            for( int j = 0; j < array_count(a->anims[i].frames); ++j ) {
                if( ui_collapse(va("[%d]",j), va("%p%d.%d", a, a->anims[i].name,j) ) ) {
                    ui_unsigned("Frame", &a->anims[i].frames[j]);
                    ui_atlas_frame(a->frames + a->anims[i].frames[j]);
                    ui_collapse_end();
                }
            }
            ui_collapse_end();
        }
    }
    for( int i = 0; i < array_count(a->slices); ++i ) {
        if( ui_collapse(quark_string(&a->db, a->slices[i].name), va("%p%d", a, a->slices[i].name) ) ) {
            changed = i+1;
            for( int j = 0; j < array_count(a->slices[i].frames); ++j ) {
                if( ui_collapse(va("[%d]",j), va("%p%d.%d", a, a->slices[i].name,j) ) ) {
                    // ui_unsigned("Frame", &a->slices[i].frames[j]);
                    ui_atlas_slice_frame(a->slice_frames + a->slices[i].frames[j]);
                    ui_collapse_end();
                }
            }
            ui_collapse_end();
        }
    }
    return changed;
}

void atlas_destroy(atlas_t *a) {
    if( a ) {
        texture_destroy(&a->tex);
        memset(a, 0, sizeof(atlas_t));
    }
}
atlas_t atlas_create(const char *inifile, unsigned flags) {
    atlas_t a = {0};
    int padding = 0, border = 0;

    ini_t kv = ini(inifile);
    for each_map(kv, char*,k, char*,v ) {
        unsigned index = atoi(k);
        // printf("entry %s=%s\n", k, v);
        /**/ if( strend(k, ".name") ) {
            array_reserve_(a.anims, index);

            a.anims[index].name = quark_intern(&a.db, v);
        }
        else if ( strend(k, ".sl_name") ) {
            array_reserve_(a.slices, index);

            a.slices[index].name = quark_intern(&a.db, v);
        }
        else if ( strend(k, ".sl_frames") ) {
            array_reserve_(a.slices, index);

            const char *text = v;
            array(char*) frames = strsplit(text, ",");
            for( int i = 0; i < array_count(frames); i++ ) {
                unsigned frame = atoi(frames[i]);
                array_push(a.slices[index].frames, frame);
            }
        }
        else if ( strend(k, ".sl_bounds") ) {
            array_reserve_(a.slice_frames, index);

            float x,y,z,w;
            sscanf(v, "%f,%f,%f,%f", &x, &y, &z, &w);

            a.slice_frames[index].bounds = vec4(x,y,x+z,y+w);
        }
        else if ( strend(k, ".sl_9slice") ) {
            array_reserve_(a.slice_frames, index);

            a.slice_frames[index].has_9slice = atoi(v);
        }
        else if ( strend(k, ".sl_core") ) {
            array_reserve_(a.slice_frames, index);

            float x,y,z,w;
            sscanf(v, "%f,%f,%f,%f", &x, &y, &z, &w);

            a.slice_frames[index].core = vec4(x,y,x+z,y+w);
        }
        else if ( strend(k, ".sl_pivot") ) {
            array_reserve_(a.slice_frames, index);

            float x,y;
            sscanf(v, "%f,%f", &x, &y);

            a.slice_frames[index].pivot = vec2(x,y);
        }
        else if ( strend(k, ".sl_color") ) {
            array_reserve_(a.slice_frames, index);

            unsigned color;
            sscanf(v, "%u", &color);

            a.slice_frames[index].color = color;
        }
        else if ( strend(k, ".sl_text") ) {
            array_reserve_(a.slice_frames, index);
            a.slice_frames[index].text = STRDUP(v);
        }
        else if( strend(k, ".frames") ) {
            array_reserve_(a.anims, index);

            array(char*) pairs = strsplit(v, ",");
            for( int i = 0, end = array_count(pairs); i < end; i += 2 ) {
                unsigned frame = atoi(pairs[i]);
                unsigned delay = atoi(pairs[i+1]);

                array_reserve_(a.frames, frame);
                a.frames[frame].delay = delay;

                array_push(a.anims[index].frames, frame);
            }
        }
        else if( strend(k, ".sheet") ) {
            array_reserve_(a.frames, index);

            vec4 sheet = atof4(v); //x,y,x2+2,y2+2 -> x,y,w,h (for 2,2 padding)
            a.frames[index].sheet = vec4(sheet.x,sheet.y,sheet.z-sheet.x,sheet.w-sheet.y);
        }
        else if( strend(k, ".indices") ) {
            array_reserve_(a.frames, index);

            const char *text = v;
            array(char*) tuples = strsplit(text, ",");
            for( int i = 0, end = array_count(tuples); i < end; i += 3 ) {
                unsigned p1 = atoi(tuples[i]);
                unsigned p2 = atoi(tuples[i+1]);
                unsigned p3 = atoi(tuples[i+2]);
                array_push(a.frames[index].indices, vec3i(p1,p2,p3));
            }
        }
        else if( strend(k, ".coords") ) {
            array_reserve_(a.frames, index);

            const char *text = v;
            array(char*) pairs = strsplit(text, ",");
            for( int i = 0, end = array_count(pairs); i < end; i += 2 ) {
                unsigned x = atoi(pairs[i]);
                unsigned y = atoi(pairs[i+1]);
                array_push(a.frames[index].coords, vec2(x,y));
            }
        }
        else if( strend(k, ".uvs") ) {
            array_reserve_(a.frames, index);

            const char *text = v;
            array(char*) pairs = strsplit(text, ",");
            for( int i = 0, end = array_count(pairs); i < end; i += 2 ) {
                unsigned u = atoi(pairs[i]);
                unsigned v = atoi(pairs[i+1]);
                array_push(a.frames[index].uvs, vec2(u,v));
            }
        }
        else if( strend(k, "padding") ) {
            padding = atoi(v);
        }
        else if( strend(k, "border") ) {
            border = atoi(v);
        }
        else if( strend(k, "file") ) {
            a.tex = texture(v, 0);
        }
        else if( strend(k, "bitmap") ) {
            const char *text = v;
            array(char) bin = base64_decode(text, strlen(text));
            a.tex = texture_from_mem(bin, array_count(bin), 0);
            array_free(bin);
        }
#if 0
        else if( strend(k, ".frame") ) {
            array_reserve_(a.frames, index);
            puts(k), puts(v);
        }
#endif
    }

    // post-process: normalize uvs and coords into [0..1] ranges
    for each_array_ptr(a.frames, atlas_frame_t, f) {
        for each_array_ptr(f->uvs, vec2, uv) {
            uv->x /= a.tex.w;
            uv->y /= a.tex.h;
        }
        for each_array_ptr(f->coords, vec2, xy) {
            xy->x /= a.tex.w;
            xy->y /= a.tex.h;
        }
        // @todo: adjust padding/border
    }
    for each_array_ptr(a.slice_frames, atlas_slice_frame_t, f) {
        f->bounds.x += padding+border;
        f->bounds.y += padding+border;
        f->bounds.z += padding+border;
        f->bounds.w += padding+border;
    }
#if 0
    // post-process: specify an anchor for each anim based on 1st frame dims
    for each_array_ptr(a.anims, atlas_anim_t, anim) {
        atlas_frame_t *first = a.frames + *anim->frames;
        for( int i = 0; i < array_count(anim->frames); i += 2) {
            atlas_frame_t *ff = a.frames + anim->frames[ i ];
            ff->anchor.x = (ff->sheet.z - first->sheet.z) / 2;
            ff->anchor.y = (ff->sheet.w - first->sheet.w) / 2;
        }
    }
#endif

    return a;
}

// ----------------------------------------------------------------------------
// sprite v2

void sprite_ctor(sprite_t *s) {
    s->tint = WHITE;
    s->timer_ms = 100;
    s->flipped = 1;
    s->sca.x += !s->sca.x;
    s->sca.y += !s->sca.y;
}
void sprite_dtor(sprite_t *s) {
    memset(s, 0, sizeof(*s));
}
void sprite_tick(sprite_t *s) {
    int right = input(s->gamepad.array[3]) - input(s->gamepad.array[2]); // RIGHT - LEFT
    int forward = input(s->gamepad.array[1]) - input(s->gamepad.array[0]); // DOWN - UP
    int move = right || forward;
    int dt = 16; // window_delta() * 1000;

    unsigned over = (s->timer - dt) > s->timer;
    if(!s->paused) s->timer -= dt;
    if( over ) {
        int len = array_count(s->a->anims[s->play].frames);
        unsigned next = (s->frame + 1) % (len + !len);
        unsigned eoa = next < s->frame;
        s->frame = next;

        atlas_frame_t *f = &s->a->frames[ s->a->anims[s->play].frames[s->frame] ];
        s->timer_ms = f->delay;
        s->timer += s->timer_ms;
    }

    if( s->play == 0 && move ) sprite_setanim(s, 1);
    if( s->play == 1 ) { //<
        if(right) s->flip_ = right < 0, sprite_setanim(s, 1);
        if(!right && !forward) sprite_setanim(s, 0);

        float speed = s->sca.x*2;
        s->pos = add4(s->pos, scale4(norm4(vec4(right,0,forward,0)),speed));
    }
}
void sprite_draw(sprite_t *s) {
    atlas_frame_t *f = &s->a->frames[ s->a->anims[s->play].frames[s->frame] ];

#if 1
    // @todo {
        unsigned sample = s->a->anims[s->play].frames[s->frame];
        sample = 0;
        f->anchor.x = (-s->a->frames[sample].sheet.z + f->sheet.z) / 2;
        f->anchor.y = (+s->a->frames[sample].sheet.w - f->sheet.w) / 2;
    // }
#endif

    // rect(x,y,w,h) is [0..1] normalized, z-index, pos(x,y,scale), rotation (degrees), color (rgba)
    vec4 rect = { f->sheet.x / s->a->tex.w, f->sheet.y / s->a->tex.h, f->sheet.z / s->a->tex.w, f->sheet.w / s->a->tex.h };
    sprite_rect(s->a->tex, rect, s->pos, vec4(s->flip_ ^ s->flipped?s->sca.x:-s->sca.x,s->sca.y,f->anchor.x,f->anchor.y), s->tilt, s->tint, 0|SPRITE_PROJECTED);
}
void sprite_edit(sprite_t *s) {
    const char *name = obj_name(s);
    const char *id = vac("%p", s);
    if( s && ui_collapse(name ? name : id, id) ) {
        ui_obj("%s", (obj*)s);

        ui_bool("paused", &s->paused);
        ui_label(va("frame anim [%d]", s->a->anims[s->play].frames[s->frame]));

        int k = s->play;
        if( ui_int("anim", &k) ) {
            sprite_setanim(s, k);
        }

        int selected = ui_atlas(s->a);
        if( selected ) sprite_setanim(s, selected - 1);

        ui_collapse_end();
    }
}

sprite_t* sprite_new(const char *ase, int bindings[6]) {
    sprite_t *s = obj_new(sprite_t, {bindings[0],bindings[1],bindings[2],bindings[3]}, {bindings[4],bindings[5]});
    atlas_t own = atlas_create(ase, 0);
    memcpy(s->a = MALLOC(sizeof(atlas_t)), &own, sizeof(atlas_t)); // s->a = &s->own;
    return s;
}
void sprite_del(sprite_t *s) {
    if( s ) {
        if( s->a ) atlas_destroy(s->a), FREE(s->a); // if( s->a == &s->own )
        obj_free(s);
        memset(s, 0, sizeof(sprite_t));
    }
}
void sprite_setanim(sprite_t *s, unsigned name) {
    if( s->play != name ) {
        s->play = name;
        s->frame = 0;

        atlas_frame_t *f = &s->a->frames[ s->a->anims[s->play].frames[s->frame] ];

        s->timer_ms = f->delay;
        s->timer = s->timer_ms;
    }
}

AUTORUN {
    STRUCT(sprite_t, vec4, pos);
    STRUCT(sprite_t, vec2, sca);
    STRUCT(sprite_t, float, tilt);
    STRUCT(sprite_t, vec4, gamepad);
    STRUCT(sprite_t, vec2, fire);
    STRUCT(sprite_t, rgba,  tint);
    STRUCT(sprite_t, unsigned, frame);
    STRUCT(sprite_t, unsigned, timer);
    STRUCT(sprite_t, unsigned, timer_ms);
    STRUCT(sprite_t, unsigned, flipped);
    STRUCT(sprite_t, unsigned, play);
    EXTEND_T(sprite, ctor,edit,draw,tick);
}
