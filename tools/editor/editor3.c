#define COOK_ON_DEMAND 1 // @fixme: these directives should be on client, not within fwk.dll
#define ENABLE_AUTOTESTS 1
#define FWK_IMPLEMENTATION
#include "fwk.h"
#include "objtests.h"

#include "3rd_icon_mdi.h"
#include "fwk_editor.h"

// ----------------------------------------------------------------------------

TODO("file_id: glow.hdr vs glow.png");
TODO("reflect: iterate components+metas on REFLECT too, so they're properly saved/loaded");

TODO("edit: tree nav");
TODO("edit:   keys up,down,left,right -> move selection");
TODO("edit:   reordering/dragging items on a list. ctrl+cursors");
TODO("edit:   tab -> cycle next node of matching highlighted type");
TODO("edit: ^C^V^X thru clipboard. ^C to serialize to clipboard.");
TODO("edit: ^Z^Y cursor too. also fix undo ordering");
TODO("edit: ^S^L^N. load/save as filesystems");
TODO("edit: ^B(toolbar)");
TODO("edit: ^E prop single-view for multiple selections: should inspect common fields only");
TODO("edit:    two-column way (or Nth) for multiple selections");
TODO("edit: tab/caps view, world + entity only, obj printf");
TODO("edit: obj bounds, obj picking, obj collisions");
TODO("edit:   LMB object picking, RMB object serialization + log, floating ICONS bulb light");
TODO("edit:   worldtraveller component");
TODO("edit:   levelstreamer component");
TODO("edit: OSC server/client port 2023");
TODO("edit: add/rem entities, add/rem components, add/rem/pause/resume systems");
TODO("edit: event loop: tick,draw*,spawn,delete,un/load from bvh stream,");

TODO("edit: overlay scene editor");
TODO("edit:   overlay0 artwork");
TODO("edit:   overlay1 gizmo, grid, snap, splats (@todo: fixed screen size gizmos)");
TODO("edit:   overlay2 script editor");
TODO("edit:   overlay3 track, spline, keys editor");
TODO("edit:   overlay4 node editor (shader/anim/bt/hfsm/material/audio/blueprints)");
TODO("edit:   overlay5 csv editor");
TODO("edit:   overlay6 bug/task editor");

TODO("gfx: tree traversal from game");
TODO("gfx:   bvh and collision queries");
TODO("gfx:   visibility and pvs queries");

TODO("obj: finish SYSTEMS and join queries");
TODO("obj: make it work with /GL flag (VC)");
TODO("obj: impl obj_mutate() ... deprecate?");
TODO("obj: make() from mpack(m) + native(n)");
TODO("obj: make() should save schema `[{mn`+version. and (m)pack and (n)ative should start with objtype");
TODO("obj: super()");
TODO("obj: lock()/trylock()/unlock()/barrier(N)");
TODO("obj: diff()/patch()");
TODO("obj: free obj_children()/payload");
TODO("obj: free obj_components()/payload2");

TODO("pack: mp2json, json2mp");
TODO("pack: simplify msgpack API, make it growth similar to va()")
#if 0 // fwk_pack proposal
static __thread char*    mpin;
static __thread unsigned mpinlen;
static __thread char     mpinbuf[256];
static __thread char*    mpout;
static __thread unsigned mpoutlen;
static __thread char     mpoutbuf[256];
#define PACKMSG(...) (msgpack_new(mpin = mpinbuf, mpinlen = sizeof(mpinbuf)), mpinlen = msgpack(__VA_ARGS__), cobs_encode(mpin, mpinlen, mpout = mpoutbuf, mpoutlen = cobs_bounds(mpinlen)), mpout)
#define UNPACKMSG(ptr,fmt,...) (mpin = (char*)ptr, mpinlen = strlen(ptr), mpout = mpoutbuf, mpoutlen = sizeof(mpoutbuf), mpoutlen = cobs_decode(mpin, mpinlen, mpout, mpoutlen), msgunpack_new(mpout, mpoutlen) && msgunpack(fmt, __VA_ARGS__))
#endif

// ----------------------------------------------------------------------------
// demo

typedef struct my_sprite { OBJ
    char *filename;
    vec3 position;
    float tilt;
    vec4 tint;
    // --- private
    unsigned rgba_;
    texture_t texture_;
} my_sprite;

OBJTYPEDEF(my_sprite,201);

void my_sprite_ctor(my_sprite *obj) {
    obj->texture_ = texture(obj->filename, TEXTURE_RGBA);
    obj->rgba_ = rgbaf( obj->tint.x/255.0, obj->tint.y/255.0, obj->tint.z/255.0, obj->tint.w/255.0 );
}
void my_sprite_draw(my_sprite *obj) {
    obj->rgba_ = rgbaf( obj->tint.x/255.0, obj->tint.y/255.0, obj->tint.z/255.0, obj->tint.w/255.0 ); // @fixme: del me
    sprite( obj->texture_, &(obj->position.x), obj->tilt, obj->rgba_ );
}
void my_sprite_edit(my_sprite *obj) {
    ui_label("Private section");
    ui_color4("Tint_", &obj->tint.x);
    ui_texture("Texture_", obj->texture_);
    ui_separator();
}

AUTORUN {
    // reflect
    STRUCT( my_sprite, char*, filename, "Filename" );
    STRUCT( my_sprite, vec3, position, "Position" );
    STRUCT( my_sprite, float, tilt, "Tilt degrees" );
    STRUCT( my_sprite, vec4, tint, "Tint color" );

    // extend
    EXTEND(my_sprite,ctor);
    EXTEND(my_sprite,draw);
    EXTEND(my_sprite,edit);
}

void game(unsigned frame, float dt, double t) {
    static my_sprite *root;
    static my_sprite *o1;
    static my_sprite *o2;
    static camera_t cam;
    if( !frame ) {
        cam = camera();
        camera_enable(&cam);

        root = obj_make(
            "[my_sprite]\n"
            "filename=cat.png\n"
            "position=5,2,100\n"
            "tilt=46 + 45 -90\n"
            "tint=255, 255, 0, 255\n"
        );
        o1 = obj_make(
            "[my_sprite]\n"
            "filename=cat.png\n"
            "position=1,2,100\n"
            "tilt=45 + 45 -90\n"
            "tint=255, 0, 0, 255\n"
        );
        o2 = obj_make(
            "[my_sprite]\n"
            "filename=cat.png\n"
            "position=1,2,100\n"
            "tilt=45\n"
            "tint=0, 0, 255, 255\n"
        );

        //obj_setname(root, "root");
        obj_setname(o1, "o1");
        obj_setname(o2, "o2");

        obj*o3 = obj_new_ext(obj, "o3");
        obj*o4 = obj_new_ext(obj, "o4");
        obj*o5 = obj_new_ext(obj, "o5");

        obj_attach(root, o1);
            obj_attach(o1, o2);
                obj_attach(o2, o3);
            obj_attach(o1, o4);
        obj_attach(root, o5);

        editor_watch(root);
    }

    // draw game
    my_sprite_draw(root);
    my_sprite_draw(o1);
    my_sprite_draw(o2);

    // tick game
    root->tilt = 5 * sin(t+dt);
}

int main(){
    for( window_create(flag("--transparent") ? 101 : 80,0); window_swap(); ) {
        editor_frame(game);
    }
}
