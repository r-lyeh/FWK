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

#include "engine.h"
#define spine spine2
#define spine_render spine_render2
#define spine_animate spine_animate2
#define spine_skin spine_skin2
#define ui_spine ui_spine2

enum { _64 = 64 }; // max bones

typedef struct spine_bone_t {
    char *name, *parent;
    struct spine_bone_t *parent_bone;

    float z; // draw order usually matches bone-id. ie, zindex == bone_id .. root(0) < chest (mid) < finger(top)

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
            array(spine_animkey_t) attach_keys[_64];
            array(spine_animkey_t) rotate_keys[_64];
            array(spine_animkey_t) translate_keys[_64];
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
} spine_t;

// ---

void spine_convert_animkeys_to_animpose(spine_anim_t *input) {
    spine_anim_t copy = *input; // @todo
    // @leak: attach/rot/tra keys
}

int find_bone_id(spine_t *s, const char *bone_name) {
    for( unsigned i = 0, end = array_count(s->bones); i < end; ++i )
        if( !strcmp(s->bones[i].name, bone_name)) return i;
    return -1;
}
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

spine_t spine(const char *file_json, const char *file_atlas, unsigned flags) {
    spine_t z = {0}, t = z;

    char *atlas = vfs_read(file_atlas);
    if(!atlas || !atlas[0]) return z;

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
            array_push(t.atlas, ((spine_atlas_t){0}) );
            sa = &t.atlas[array_count(t.atlas) - 1];
            sa->name = STRDUP(it);
        }
    }
    for( int i = 0; i < array_count(t.atlas); ++i ) {
        sa = &t.atlas[i];
        sa->x /= texture_width, sa->y /= texture_height;
        sa->w /= texture_width, sa->h /= texture_height;
    }

    if(!texture_name) return z;

    t.texture = texture(texture_name, 0); // @todo: add texture flags here

    json_push(vfs_read(file_json)); // @fixme: json_push_from_file() ?

    array_resize(t.bones, json_count("/bones"));
    array_reserve(t.slots, json_count("/slots"));
    array_resize(t.skins, json_count("/skins"));
    array_resize(t.anims, json_count("/animations"));

    for( int i = 0, end = json_count("/bones"); i < end; ++i ) {
        spine_bone_t v = {0};
        v.name = STRDUP(json_string("/bones[%d]/name", i));
        v.parent = STRDUP(json_string("/bones[%d]/parent", i));
        v.x = json_float("/bones[%d]/x", i);
        v.y = json_float("/bones[%d]/y", i);
        v.z = i;
        v.deg = json_float("/bones[%d]/rotation", i);
        t.bones[i] = v;

        for( int j = i-1; j > 0; --j ) {
            if( strcmp(t.bones[j].name,v.parent) ) continue;
            t.bones[i].parent_bone = &t.bones[j];
            break;
        }
    }

    for( int i = 0, end = json_count("/slots"); i < end; ++i ) {
        spine_slot_t v = {0};
        v.name = STRDUP(json_string("/slots[%d]/name", i));
        v.bone = STRDUP(json_string("/slots[%d]/bone", i));
        v.attach = STRDUP(json_string("/slots[%d]/attachment", i));

        array_push(t.slots, v);

        // slots define draw-order. so, update draw-order/zindex in bone
        spine_bone_t *b = find_bone(&t, v.name);
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

        t.skins[i] = v;
    }

#if 1
    // simplify:
    // merge /skins/default into existing /skins/*, then delete /skins/default
    if( array_count(t.skins) > 1 ) {
        for( int i = 1; i < array_count(t.skins); ++i ) {
            for( int j = 0; j < array_count(t.skins[0].rects); ++j ) {
                array_push(t.skins[i].rects, t.skins[0].rects[j]);
            }
        }
        // @leak @fixme: free(t.skins[0])
        for( int i = 0; i < array_count(t.skins)-1; ++i ) {
            t.skins[i] = t.skins[i+1];
        }
        array_pop(t.skins);
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
            int bone_id = find_bone_id(&t, json_key("/animations[%d]/bones[%d]",i,j));
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
                // for(id = 0; t.bones[id].name && strcmp(t.bones[id].name,key.name); ++id)
                // printf("%s vs %s\n", key.name, t.bones[id].name);

                array_push(v.attach_keys[bone_id], key);
            }            
        }

        // bones

        for( int j = 0, jend = json_count("/animations[%d]/bones",i); j < jend; ++j ) // slots or bones
        for( int k = 0, kend = json_count("/animations[%d]/bones[%d]",i,j); k < kend; ++k ) { // bone ids
            int bone_id = find_bone_id(&t, json_key("/animations[%d]/bones[%d]",i,j));
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

        t.anims[i] = v;
    }

    json_pop();

    spine_skin(&t, 0);

    return t;
}

void spine_render(spine_t *p, vec3 offset, unsigned flags) {
    if( !p->texture.id ) return;
    if( !flags ) return;

    ddraw_push_2d();
        if( flags & 2 ) ddraw_line(vec3(0,0,0), vec3(window_width(),window_height(),0));
        if( flags & 2 ) ddraw_line(vec3(window_width(),0,0), vec3(0,window_height(),0));

        for( int i = 1; i < array_count(p->bones); ++i ) {
            spine_bone_t *self = &p->bones[i];

            static array(spine_bone_t*) chain = 0; array_resize(chain, 0);
            for( spine_bone_t *next = self; next ; next = next->parent_bone ) {
                array_push(chain, next);
            }

            vec3 target = {0}, prev = {0};
            for( int j = 1, end = array_count(chain); j < end; ++j ) { // traverse from root(skipped) -> `i` bone direction
                int j_opposite = (end - 1) - j;

                spine_bone_t *b = chain[j_opposite]; // bone
                spine_bone_t *pb = chain[j_opposite+1]; // parent bone

                prev = target;

                const float deg2rad = C_PI / 180;
                b->x2 =      b->x3 + pb->x2   + b->x * cos( -pb->deg2 * deg2rad ) - b->y * sin( -pb->deg2 * deg2rad );
                b->y2 =     -b->y3 + pb->y2   - b->y * cos(  pb->deg2 * deg2rad ) + b->x * sin(  pb->deg2 * deg2rad );
                b->deg2 = -b->deg3 + pb->deg2 - b->deg;

                target = vec3(b->x2,b->y2,b->deg2);
            }

            float deg = target.z, deg_prev = prev.z;
            target.z = 0; prev.z = 0;

            target = add3(target, offset);
            prev = add3(prev, offset);

            if( flags & 2 ) {
                ddraw_point( target );
                ddraw_text( target, -0.25f, self->name );
                ddraw_line( target, prev ); // from bone to parent
            }
            if( flags & 1 ) {
                vec4 rect = ptr4(&p->atlas[self->atlas_id].x);
                float zindex = self->z;
                float offsx = 0; // -(rect.w * p->texture.w); // -p->atlas[self->atlas_id].w - (self->rect_id ? p->skins[p->skin].rects[self->rect_id].w/2 : 0);
                float offsy = 0; // /*-(rect.z * p->texture.h)*2*/ -p->atlas[self->atlas_id].h - (self->rect_id ? p->skins[p->skin].rects[self->rect_id].h/2 : 0);
                float deg_rect = self->rect_id ? p->skins[p->skin].rects[self->rect_id].deg : 0;
                float tilt = p->atlas[self->atlas_id].deg + self->deg2 - deg_rect; // + self->deg2 + deg_rect + p->atlas[self->atlas_id].deg
                unsigned tint = ~0u, flags = 0;
                sprite_rect(p->texture, rect, vec4(target.x,target.y,0,zindex), vec4(1,1,offsx,offsy), tilt, tint, flags);
            }
         }

    ddraw_pop_2d();
    ddraw_flush();
}

void spine_animate(spine_t *p, float *time, float *maxtime, float delta) {
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

void ui_spine(spine_t *p) {

    if( ui_collapse(va("Anims: %d", array_count(p->anims)), va("%p-a", p))) {
        for each_array_ptr(p->anims, spine_anim_t, q) {
            if(ui_slider2("", &p->time, va("%.2f/%.0f %.2f%%", p->time, p->maxtime, p->time * 100.f))) {
                spine_animate(p, &p->time, &p->maxtime, 0);
            }

            int choice = ui_label2_toolbar(q->name, ICON_MD_PAUSE_CIRCLE " " ICON_MD_PLAY_CIRCLE);
            if( choice == 1 ) window_pause( 0 ); // play
            if( choice == 2 ) window_pause( 1 ); // pause

            for( int i = 0; i < _64; ++i ) {
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



int main() {
    window_create(0.75, 0);

    camera_t cam = camera();
    cam.position = vec3(0,0,1);
    camera_enable(&cam);

    spine_t s = spine("goblins.json", "goblins.atlas", 0);

    while( window_swap() ) {
        camera_get_active()->position.x = window_width()/2;
        camera_get_active()->position.y = window_height()/2;

        static bool do_skin = 1, do_skel = 1;
        spine_animate(&s, &s.time, &s.maxtime, !window_has_pause() * window_delta());

        spine_render(&s, vec3(window_width()/2, window_height()/2, 0), do_skin );
        sprite_flush();
        glClear(GL_DEPTH_BUFFER_BIT);
        spine_render(&s, vec3(window_width()/2, window_height()/2, 0), (do_skel*2));

        if( ui_panel("Spine", 0) ) {
            if(ui_button("Load")) {
                s = spine("goblins.json", "goblins.atlas", 0);
            }
            ui_spine(&s);
            if(ui_bool("Draw Skin", &do_skin));
            if(ui_bool("Draw Skeleton", &do_skel));
            ui_panel_end();
        }
    }
}
