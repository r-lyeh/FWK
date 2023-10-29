// ----------------------------------------------------------------------------
// game ui (utils)

API void gui_drawrect( texture_t spritesheet, vec2 tex_start, vec2 tex_end, int rgba, vec2 start, vec2 end );

#define v42v2(rect) vec2(rect.x,rect.y), vec2(rect.z,rect.w)

void gui_drawrect( texture_t texture, vec2 tex_start, vec2 tex_end, int rgba, vec2 start, vec2 end ) {
    float gamma = 1;
    static int program = -1, vbo = -1, vao = -1, u_inv_gamma = -1, u_tint = -1, u_has_tex = -1, u_window_width = -1, u_window_height = -1;
    vec2 dpi = ifdef(osx, window_dpi(), vec2(1,1));
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/rect_2d.vs");
        const char* fs = vfs_read("shaders/rect_2d.fs");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        ASSERT(program > 0);
        u_inv_gamma = glGetUniformLocation(program, "u_inv_gamma");
        u_tint = glGetUniformLocation(program, "u_tint");
        u_has_tex = glGetUniformLocation(program, "u_has_tex");
        u_window_width = glGetUniformLocation(program, "u_window_width");
        u_window_height = glGetUniformLocation(program, "u_window_height");
        glGenVertexArrays( 1, (GLuint*)&vao );
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    start = mul2(start, dpi);
    end = mul2(end, dpi);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLenum texture_type = texture.flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    glUseProgram( program );
    glUniform1f( u_inv_gamma, 1.0f / (gamma + !gamma) );

    glBindVertexArray( vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( texture_type, texture.id );

    glUniform1i(u_has_tex, (texture.id != 0));
    glUniform1f(u_window_width, (float)window_width());
    glUniform1f(u_window_height, (float)window_height());

    vec4 rgbaf = {((rgba>>24)&255)/255.f, ((rgba>>16)&255)/255.f,((rgba>>8)&255)/255.f,((rgba>>0)&255)/255.f};
    glUniform4fv(u_tint, GL_TRUE, &rgbaf.x);

    // normalize texture regions
    if (texture.id != 0) {
        tex_start.x /= texture.w;
        tex_start.y /= texture.h;
        tex_end.x /= texture.w;
        tex_end.y /= texture.h;
    }

    GLfloat vertices[] = {
        // Positions      // UVs
        start.x, start.y, tex_start.x, tex_start.y,
        end.x, start.y,   tex_end.x, tex_start.y,
        end.x, end.y,     tex_end.x, tex_end.y,
        start.x, start.y, tex_start.x, tex_start.y,
        end.x, end.y,     tex_end.x, tex_end.y,
        start.x, end.y,   tex_start.x, tex_end.y
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( texture_type, 0 );
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindVertexArray( 0 );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram( 0 );
}

// ----------------------------------------------------------------------------
// game ui

typedef struct gui_state_t {
    union {
        struct {
            bool held;
            bool hover;
        };
    };
} gui_state_t;

static __thread array(guiskin_t) skins=0;
static __thread guiskin_t *last_skin=0;
static __thread map(int, gui_state_t) ctl_states=0; //@leak

void gui_pushskin(guiskin_t skin) {
    array_push(skins, skin);
    last_skin = array_back(skins);
}

void gui_popskin() {
    if (!last_skin) return;
    if (last_skin->free) last_skin->free(last_skin->userdata);
    array_pop(skins);
    last_skin = array_count(skins) ? array_back(skins) : NULL;
}

void *gui_userdata() {
    return last_skin->userdata;
}

vec2 gui_getskinsize(const char *skin) {
    vec2 size={0};
    if (last_skin->getskinsize) last_skin->getskinsize(last_skin->userdata, skin, &size);
    return size;
}

static
gui_state_t *gui_getstate(int id) {
    if (!ctl_states) map_init(ctl_states, less_int, hash_int);
    return map_find_or_add(ctl_states, id, (gui_state_t){0});
}

bool (gui_button)(int id, vec4 r, const char *skin) {
    gui_state_t *entry = gui_getstate(id);
    bool was_clicked=0;
    entry->hover = false;

    if (input(MOUSE_X) > r.x && input(MOUSE_X) < (r.x+r.z) && input(MOUSE_Y) > r.y && input(MOUSE_Y) < (r.y+r.w)) {
        if (input_up(MOUSE_L) && entry->held) {
            was_clicked=1;
        }

        entry->held = input_held(MOUSE_L);
        entry->hover = true;
    }
    else if (input_up(MOUSE_L) && entry->held) {
        entry->held = false;
    }

    char *btn = va("%s%s", skin?skin:"button", entry->held?"_press":entry->hover?"_hover":"");
    if (last_skin->drawrect) last_skin->drawrect(last_skin->userdata, btn, r);

    return was_clicked;
}

void (gui_panel)(int id, vec4 r, const char *skin) {
    if (last_skin->drawrect) last_skin->drawrect(last_skin->userdata, skin?skin:"panel", r);
}

/* skinned */

static
void skinned_free(void* userdata) {
    skinned_t *a = C_CAST(skinned_t*, userdata);
    atlas_destroy(&a->atlas);
    FREE(a);
}

static
atlas_slice_frame_t *skinned_getsliceframe(atlas_t *a, const char *name) {
    for (int i = 0; i < array_count(a->slices); i++) 
        if (!strcmp(quark_string(&a->db, a->slices[i].name), name))
            return &a->slice_frames[a->slices[i].frames[0]];
    return NULL;
}

static
void skinned_draw_missing_rect(vec4 r) {
    vec4 size = vec4(0, 0, texture_checker().w, texture_checker().h);
    gui_drawrect(texture_checker(), v42v2(size), 0x800080FF, v42v2(r));
}

static
void skinned_draw_sprite(float scale, atlas_t *a, atlas_slice_frame_t *f, vec4 r) {
    vec4 outer = f->bounds;
    r.x -= f->pivot.x*scale;
    r.y -= f->pivot.y*scale;
    r.z += r.x;
    r.w += r.y;

    // Ensure dest rectangle is large enough to render the whole element
    if ((r.z-r.x) < (outer.z-outer.x) * scale) {
        r.z = r.x + (outer.z-outer.x) * scale;
    }
    if ((r.w-r.y) < (outer.w-outer.y) * scale) {
        r.w = r.y + (outer.w-outer.y) * scale;
    }

    if (!f->has_9slice) {
        gui_drawrect(a->tex, v42v2(f->bounds), 0xFFFFFFFF, v42v2(r));
        return;
    }

    vec4 core  = f->core;
    core.x += outer.x;
    core.y += outer.y;
    core.z += outer.x;
    core.w += outer.y;

    // Define the 9 slices
    vec4 top_left_slice = {outer.x, outer.y, core.x, core.y};
    vec4 top_middle_slice = {core.x, outer.y, core.z, core.y};
    vec4 top_right_slice = {core.z, outer.y, outer.z, core.y};

    vec4 middle_left_slice = {outer.x, core.y, core.x, core.w};
    vec4 center_slice = core;
    vec4 middle_right_slice = {core.z, core.y, outer.z, core.w};

    vec4 bottom_left_slice = {outer.x, core.w, core.x, outer.w};
    vec4 bottom_middle_slice = {core.x, core.w, core.z, outer.w};
    vec4 bottom_right_slice = {core.z, core.w, outer.z, outer.w};

    vec4 top_left = {r.x, r.y, r.x + (core.x - outer.x) * scale, r.y + (core.y - outer.y) * scale};
    vec4 top_right = {r.z - (outer.z - core.z) * scale, r.y, r.z, r.y + (core.y - outer.y) * scale};
    vec4 bottom_left = {r.x, r.w - (outer.w - core.w) * scale, r.x + (core.x - outer.x) * scale, r.w};
    vec4 bottom_right = {r.z - (outer.z - core.z) * scale, r.w - (outer.w - core.w) * scale, r.z, r.w};

    vec4 top = {top_left.z, r.y, top_right.x, top_left.w};
    vec4 bottom = {bottom_left.z, bottom_left.y, bottom_right.x, r.w};
    vec4 left = {r.x, top_left.w, top_left.z, bottom_left.y};
    vec4 right = {top_right.x, top_right.w, r.z, bottom_right.y};

    vec4 center = {top_left.z, top_left.w, top_right.x, bottom_right.y};

    gui_drawrect(a->tex, v42v2(center_slice), 0xFFFFFFFF, v42v2(center));
    gui_drawrect(a->tex, v42v2(top_left_slice), 0xFFFFFFFF, v42v2(top_left));
    gui_drawrect(a->tex, v42v2(top_right_slice), 0xFFFFFFFF, v42v2(top_right));
    gui_drawrect(a->tex, v42v2(bottom_left_slice), 0xFFFFFFFF, v42v2(bottom_left));
    gui_drawrect(a->tex, v42v2(bottom_right_slice), 0xFFFFFFFF, v42v2(bottom_right));
    gui_drawrect(a->tex, v42v2(top_middle_slice), 0xFFFFFFFF, v42v2(top));
    gui_drawrect(a->tex, v42v2(bottom_middle_slice), 0xFFFFFFFF, v42v2(bottom));
    gui_drawrect(a->tex, v42v2(middle_left_slice), 0xFFFFFFFF, v42v2(left));
    gui_drawrect(a->tex, v42v2(middle_right_slice), 0xFFFFFFFF, v42v2(right));
}

static
void skinned_draw_rect(void* userdata, const char *skin, vec4 r) {
    skinned_t *a = C_CAST(skinned_t*, userdata);

    atlas_slice_frame_t *f = skinned_getsliceframe(&a->atlas, skin);
    if (!f) skinned_draw_missing_rect(r);
    else skinned_draw_sprite(a->scale, &a->atlas, f, r);
}

void skinned_getskinsize(void *userdata, const char *skin, vec2 *size) {
    skinned_t *a = C_CAST(skinned_t*, userdata);

    atlas_slice_frame_t *f = skinned_getsliceframe(&a->atlas, skin);
    if (f) {
        size->x = (f->bounds.z-f->bounds.x)*a->scale;
        size->y = (f->bounds.w-f->bounds.y)*a->scale;
    }
}

guiskin_t gui_skinned(const char *inifile, float scale) {
    skinned_t *a = REALLOC(0, sizeof(skinned_t));
    a->atlas = atlas_create(inifile, 0);
    a->scale = scale?scale:1.0f;
    guiskin_t skin={0};
    skin.userdata = a;
    skin.drawrect = skinned_draw_rect;
    skin.getskinsize = skinned_getskinsize;
    skin.free = skinned_free;
    return skin;
}
