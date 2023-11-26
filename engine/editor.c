#include "fwk.h"
#include "split/3rd_icon_mdi.h"

#define SWAP(T,a,b) do { T c = (a); (a) = (b); (b) = c; } while(0)

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

TODO("COOK_ON_DEMAND 1: this directive should be on client, not within fwk.dll");
TODO("ENABLE_AUTOTESTS 1: this directive should be on client, not within fwk.dll");

// ----------------------------------------------------------------------------

TODO("editor_load_on_boot()");
TODO("editor_save_on_quit()");

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
TODO("edit:   overlay1 world editor: gizmo, grid, snap, splats (@todo: fixed screen size gizmos)");
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
TODO("pack: simplify msgpack API, make it growth similar to va()");

TODO("serialize array(types)");
TODO("serialize map(char*,types)");
TODO("serialize map(int,types)");
TODO("sprite: solid platforms, one-way platforms");
TODO("sprite: shake left-right, up-down");
TODO("sprite: coyote time");
TODO("sprite: jump buffering before grounded");
TODO("sprite: double jump, wall sliding, wall climbing");
TODO("sprite: hitbox for enemies -> wall detection");

TODO("new: integrate with Art/ browser");
TODO("bug: lite key bindings are being sent to editor");
TODO("bug: not sending quit signal to lite neither at window close nor editor close (see: temporary files)");
TODO("bug: missing search results window");
TODO("bug: missing code completions popup");

// TODO("eval:  https://github.com/drmargarido/linters")
// TODO("eval:  https://github.com/monolifed/theme16")
// TODO("eval:  https://github.com/vincens2005/lite-formatters")
// https://github.com/takase1121/lite-xl-img
// https://github.com/takase1121/lite-xl-finder
// https://github.com/rxi/lite/commit/236a585756cb9fa70130eee6c9a604780aced424 > suru.png
// https://github.com/rxi/lite/commit/f90b00748e1fe1cd2340aaa06d2526a1b2ea54ec

#if 0 // good UI/UX blender ideas
ui density is perfect: font face, font size and icon sizes are perfect even in a low res 1366x768 scenario
viewport: not cluttered at all. vertical floating toolbar on the left, floating icons on the right
left icons are tools related to current edition mode, right icons are camera only.
left icons can be long pressed for a hovered submenu, right icons can be clicked or dragged.
non-matching text filtering works by disabling widgets, not by hiding them
window contents can be promoted to any display any other window content by clicking their [icon V]
there are no windows or minimize/maximize buttons
windows are grouped with a vertical toolbar inside the right bar docked section
you can only display 1 window at time.
panels can be dragged within a window and make much more sense than our approach
scene tree: can make collections/groups
widget tree drawing is much simpler than us, just a vertical parent line per collection and indented arrows per child
context menus are always like: optional icon (white), text (white), either optional input binding (gray) or submenu arrow
input boxes are super reactive
< > are hidden unless hovered
< > can be clicked to de/increment (release)
< > can be horizontally dragged to de/increment larger quantities (hold)
< > can be vertically dragged to include other Y,Z properties in the selection (hold)
0.53m can be clicked (release) or dragged (<-> hovered icon) (hold).
0.53m whole text is automatically selected when clicked.
you can enter units when entering values 350m, 350km, 12mi
you can enter math values: 30+20+sin(1)
numeric: units always specified: 0 m, 90º, 1.25x
numeric: epsilons (non-zero) displayed as -0.00000 even if digits cannot fit widget rect

operation context is split into different sections in menu bar: modeling, sculpting, uv editing, shading, anim, etc
cam 3d orientation gizmo top right: can be dragged (orbit cam) or clicked (reposition)
rotation gizmo: anchor can be positioned within world
gizmo: will display always initial position while dragging, and tape distance so far
right floating icons can be dragged: cam orientation, cam panning, cam zooming, cam iso/dimetric
ctrl-z undo, ctrl-shift-z redo
#endif

bool is_hovering(vec4 rect, vec2 mouse) { // rect(x,y,x2,y2)
    if( mouse.x < rect.x ) return 0;
    if( mouse.x > rect.z ) return 0;
    if( mouse.y < rect.y ) return 0;
    if( mouse.y > rect.w ) return 0;
    return 1;
}
vec4 editor_toolbar_drag; // xy mouse, z time
vec4 editor_toolbar_rect;
int editor_toolbar_hovered() {
    return is_hovering(editor_toolbar_rect, vec2(input(MOUSE_X),input(MOUSE_Y)));
}
vec2 editor_toolbar_dragged() {
    return vec2( editor_toolbar_drag.x, editor_toolbar_drag.y );
}
int editor_toolbar(int x, int y, int incw, int inch, const char *sym) {
    array(uint32_t) codepoints = string32(sym);
    if( incw < 0 ) return editor_toolbar(x + incw * (array_count(codepoints)-1), y, -incw, inch, sym);
    if( inch < 0 ) return editor_toolbar(x, y + inch * (array_count(codepoints)-1), incw, -inch, sym);

    int mx = input(MOUSE_X);
    int my = input(MOUSE_Y);
    int inc = maxi(incw, inch);

    static int ox = 0, oy = 0; // drag origin
    static uint64_t dragging = 0;

    editor_toolbar_rect = vec4(x,y,x + (incw ? incw * array_count(codepoints) : inch), y + (inch ? inch * array_count(codepoints) : incw) );
    int oo = is_hovering(editor_toolbar_rect, vec2(ox,oy));

    int ix = x, iy = y;
    for each_array(codepoints, uint32_t, g) {
        int selected = oo ? is_hovering(vec4(ix,iy,ix+inc,iy+inc),vec2(ox,oy)) : 0;
        int hovering = dragging ? 0 : is_hovering(vec4(ix,iy,ix+inc,iy+inc), vec2(mx,my));
        const char *str8 = codepoint_to_utf8(g);
        editor_glyphs(ix + inc/8, iy + inc/3 + 2, selected || hovering ? FONT_COLOR1 FONT_H1 : FONT_COLOR2 FONT_H1, str8);
        ix += incw;
        iy += inch;
    }
    if( 0 && editor_toolbar_hovered() ) { // debug:
        ddraw_push_2d();
        ddraw_aabb(vec3(editor_toolbar_rect.x,editor_toolbar_rect.y,0),vec3(editor_toolbar_rect.z,editor_toolbar_rect.w,0));
        ddraw_pop_2d();
    }

    if( 1 ) // is_hovering(editor_toolbar_rect, vec2(mx,my)) )
    {
        uint64_t id = hash_bin(&editor_toolbar_rect, sizeof(vec4));

        if( input_down(MOUSE_L) && editor_toolbar_hovered() && !dragging ) {
            window_cursor_shape(0);
            editor_toolbar_drag = vec4(0,0, mx,my);
            ox = mx, oy = my, dragging = id;
            int mcx = ((ox - x) / inc) + 1, mcy = ((oy - y) / inc) + 1; // mouse cells
            return incw ? -mcx : -mcy;
        }
        if( input(MOUSE_L) && dragging == id ) {
            int mcx = ((ox - x) / inc) + 1, mcy = ((oy - y) / inc) + 1; // mouse cells
            editor_toolbar_drag.x  = mx - editor_toolbar_drag.z;
            editor_toolbar_drag.y  = my - editor_toolbar_drag.w;
            API void editor_setmouse(int x, int y);
            editor_setmouse(ox, oy);
            editor_toolbar_drag.z  = ox;
            editor_toolbar_drag.w  = oy;
            return incw ? -mcx : -mcy;
        }
        if( input_up(MOUSE_L) && dragging == id ) {
            int mcx = ((ox - x) / inc) + 1, mcy = ((oy - y) / inc) + 1; // mouse cells
            window_cursor_shape(CURSOR_SW_AUTO);
            ox = oy = 0, dragging = 0;
            return incw ? mcx : mcy;
        }
    }
    return 0;
}


// ----------------------------------------------------------------------------
// demo

typedef struct lit { OBJ
    vec3 pos;
    vec3 dir;
    int type;
} lit;

int lit_aabb(lit *obj, aabb *box) {
    *box = aabb( vec3(obj->pos.x-16,obj->pos.y-16,0), vec3(obj->pos.x+16,obj->pos.y+16,1) );
    return 1;
}
const char *lit_icon(lit *obj) {
    const char *icon =
        obj->type == 0 ? ICON_MD_WB_IRIDESCENT :
        obj->type == 1 ? ICON_MD_WB_INCANDESCENT :
        obj->type == 2 ? ICON_MD_FLARE :
        obj->type == 3 ? ICON_MD_WB_SUNNY : "";
    return icon;
}
int lit_edit(lit *obj) {
    const char *all_icons =
        ICON_MD_WB_IRIDESCENT
        ICON_MD_WB_INCANDESCENT
        ICON_MD_FLARE
        ICON_MD_WB_SUNNY

        ICON_MD_LIGHT_MODE
        ICON_MD_LIGHT

        ICON_MD_FLASHLIGHT_OFF
        ICON_MD_FLASHLIGHT_ON
        ICON_MD_HIGHLIGHT
        ICON_MD_HIGHLIGHT_ALT
        ICON_MD_LIGHTBULB
        ICON_MD_LIGHTBULB_OUTLINE
        ICON_MD_NIGHTLIGHT
        ICON_MD_NIGHTLIGHT_ROUND

        // MDI
        ICON_MDI_LIGHTBULB_ON_OUTLINE        // generic
        ICON_MDI_WALL_SCONCE_ROUND           //
        ICON_MDI_WALL_SCONCE_FLAT            // emissive
        ICON_MDI_CEILING_LIGHT               // spotlight
        ICON_MDI_TRACK_LIGHT                 // spotlight
        ICON_MDI_WEATHER_SUNNY               // directional
        ICON_MDI_LIGHTBULB_FLUORESCENT_TUBE_OUTLINE
    ;
    // editor_glyphs(obj->pos.x+16,obj->pos.y-32,all_icons);
    if( editor_selected(obj) ) {
    obj->pos.x += input(KEY_RIGHT) - input(KEY_LEFT);
    obj->pos.y += input(KEY_DOWN) - input(KEY_UP);
    obj->type = (obj->type + !!input_down(KEY_SPACE)) % 4;
    }
    editor_glyphs(obj->pos.x,obj->pos.y,0,lit_icon(obj));



    return 1;
}

OBJTYPEDEF(lit,200);

AUTORUN {
    STRUCT(lit, vec3, pos);
    STRUCT(lit, vec3, dir);
    STRUCT(lit, int, type);
    EXTEND(lit, edit,icon,aabb);
}

typedef struct kid { OBJ
    int kid;
    vec2 pos;
    vec2 vel;
    float angle;
    unsigned color;
    int controllerid;

    // --- private
    char *filename;
    texture_t texture_;
} kid;

void kid_ctor(kid *obj) {
    obj->kid = randi(0,3);
    obj->pos = vec2(randi(0, window_width()), randi(0, window_height()));
    obj->vel.x = obj->vel.y = 100 + 200 * randf();
    obj->controllerid = randi(0,3);

    obj->texture_ = texture(obj->filename, TEXTURE_RGBA|TEXTURE_LINEAR);
}
void kid_tick(kid *obj, float dt) {
    // add velocity to position
    vec2 off = vec2( input(KEY_RIGHT)-input(KEY_LEFT), input(KEY_DOWN)-input(KEY_UP) );
    obj->pos = add2(obj->pos, scale2(mul2(obj->vel, off), dt * (obj->controllerid == 0)));

    // wrap at screen boundaries
    const int appw = window_width(), apph = window_height();
    if( obj->pos.x < 0 ) obj->pos.x += appw; else if( obj->pos.x > appw ) obj->pos.x -= appw;
    if( obj->pos.y < 0 ) obj->pos.y += apph; else if( obj->pos.y > apph ) obj->pos.y -= apph;
}
void kid_draw(kid *obj) {
    // 4x4 tilesheet
    int col = (((int)obj->kid) % 4);
    int row = (((int)obj->pos.x / 10 ^ (int)obj->pos.y / 10) % 4);
    float position[3] = {obj->pos.x,obj->pos.y,obj->pos.y}; // position(x,y,depth: sort by Y)
    float offset[2]={0,0}, scale[2]={1,1};
    float coords[3]={col * 4 + row,4,4}; // num_frame(x) in a 4x4(y,z) spritesheet
    unsigned flags = 0;
    sprite_sheet(obj->texture_, coords, position, obj->angle*TO_DEG, offset, scale, obj->color, flags);
}
int kid_aabb(kid *obj, aabb *box) {
    *box = aabb( vec3(obj->pos.x-16,obj->pos.y-16,0), vec3(obj->pos.x+16,obj->pos.y+16,1) );
    return 1;
}
int kid_edit(kid *obj) {
    aabb box;
    if( kid_aabb(obj, &box) ) {
        ddraw_color_push(YELLOW);
        ddraw_push_2d();
        ddraw_aabb(box.min, box.max);
        ddraw_pop_2d();
        ddraw_color_pop();
    }
    if( editor_selected(obj) ) {
        obj->pos.x += input(KEY_RIGHT) - input(KEY_LEFT);
        obj->pos.y += input(KEY_DOWN) - input(KEY_UP);

        editor_glyphs(obj->pos.x+16,obj->pos.y-16,0,ICON_MD_VIDEOGAME_ASSET);
    }
    return 1;
}
void kid_menu(kid *obj, const char *argv) {
    ui_label("Private section");
    ui_color4("Color_", &obj->color);
    ui_texture("Texture_", obj->texture_);
    ui_separator();
}

OBJTYPEDEF(kid,201);

AUTORUN {
    // reflect
    STRUCT(kid, int, kid);
    STRUCT(kid, vec2, pos);
    STRUCT(kid, vec2, vel);
    STRUCT(kid, float, angle, "Tilt degrees");
    STRUCT(kid, rgba,  color, "Tint color");
    STRUCT(kid, char*, filename, "Filename" );
    EXTEND(kid, ctor,tick,draw,aabb,edit,menu);
}

void game(unsigned frame, float dt, double t) {
    static kid *root;
    static kid *o1;
    static kid *o2;
    static camera_t cam;
    if( !frame ) {
        // init camera (x,y) (z = zoom)
        cam = camera();
        cam.position = vec3(window_width()/2,window_height()/2,1);
        camera_enable(&cam);

        root = obj_make(
            "[kid]\n"
            "filename=spriteSheetExample.png\n"
            "pos=5,2\n"
            "angle=pi/12\n"
            "color=#ffcf\n"
        );
        o1 = obj_make(
            "[kid]\n"
            "filename=spriteSheetExample.png\n"
            "pos=1,100\n"
            "angle=pi/12\n"
            "color=#fcc\n"
        );
        o2 = obj_make(
            "[kid]\n"
            "filename=spriteSheetExample.png\n"
            "pos=50,200\n"
            "angle=pi/12\n"
            "color=#ccccffff\n"
        );

        //obj_setname(root, "root");
        obj_setname(o1, "o1");
        obj_setname(o2, "o2");

        obj*o3 = obj_make(
            "[lit]\n"
            "pos=300,300,0\n"
            "type=1"
        );
        obj*o4 = obj_new_ext(obj, "o4");
        obj*o5 = obj_new_ext(obj, "o5");

        obj_attach(root, o1);
            obj_attach(o1, o2);
                obj_attach(o2, o3);
            obj_attach(o1, o4);
        obj_attach(root, o5);

        editor_watch(root);
    }

    // camera panning (x,y) & zooming (z)
    if(0)
    if( !ui_hover() && !ui_active() ) {
        if( input(MOUSE_L) ) cam.position.x -= input_diff(MOUSE_X);
        if( input(MOUSE_L) ) cam.position.y -= input_diff(MOUSE_Y);
        cam.position.z += input_diff(MOUSE_W) * 0.1; // cam.p.z += 0.001f; for tests
    }

    // tick game
    if( dt ) {
        kid_tick(root, dt);
        kid_tick(o1, dt);
        kid_tick(o2, dt);

        root->angle = 5 * sin(t+dt);
    }

    // fps camera
    bool active = 0;
    if( input_down(MOUSE_M) || input_down(MOUSE_R) ) {
        active = ui_hover() || ui_active() || gizmo_active() || editor_first_selected() ? false : true;
    } else {
        active = !window_has_cursor() && (input(MOUSE_M) || input(MOUSE_R));
    }
    window_cursor( !active );
    if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
    vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
    vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed * active);
    camera_moveby(&cam, wasdecq);
    camera_fps(&cam, mouse.x,mouse.y);

    // draw world
    ddraw_ontop_push(0);
    ddraw_grid(0); // 1+10+100
  ddraw_grid(1000);
    ddraw_ontop_pop();
    ddraw_flush();

    // draw game
    kid_draw(root);
    kid_draw(o1);
    kid_draw(o2);
}

int main(){
    argvadd("--cook-on-demand");

    // borderless, see:
    // https://github.com/glfw/glfw/pull/1420
    //   https://github.com/glfw/glfw/pull/987
    //   https://github.com/glfw/glfw/pull/991
    //   https://github.com/glfw/glfw/pull/990

    window_title("Editor " EDITOR_VERSION);
    window_create( flag("--transparent") ? 101 : 80, WINDOW_MSAA4 | (flag("--windowed") ? 0 : WINDOW_BORDERLESS));
    window_icon("scale-ruler-icon.png");

    while( window_swap() ) {
        editor_frame(game);
        editor_gizmos(2);

        camera_t *cam = camera_get_active();

        int font_ascent_diff = 4; // ascent(mdi) - ascent(md) // @todo: move this into font api



        int choiceV = editor_toolbar(window_width()-32, ui_has_menubar() ? 34 + font_ascent_diff : 0, 0, 32,
            va(
            ICON_MD_VISIBILITY
            ICON_MD_360 // ICON_MDI_ORBIT
            ICON_MDI_ARROW_ALL
            ICON_MD_LOUPE // ZOOM_OUT_MAP // ICON_MD_ZOOM_IN
            "%s", camera_get_active()->orthographic ? ICON_MDI_AXIS_ARROW_INFO : ICON_MDI_AXIS_ARROW // ICON_MDI_GRID_OFF : ICON_MDI_GRID // ICON_MD_GRID_ON
            )
        );
        static int rot_snapping = 0;
        static int pos_snapping = 1;
        int choiceH = editor_toolbar(window_width()-32*2, ui_has_menubar() ? 34 : 0, -32, 0,
            va(
            ICON_MDI_ANGLE_ACUTE
            "15º"
            //ICON_MDI_ARROW_COLLAPSE
            //ICON_MDI_ARTBOARD
            "%s", pos_snapping ? ICON_MDI_DOTS_SQUARE : ICON_MDI_DOTS_CIRCLE
            )
        );

        if( choiceV ) { // clicked[>0] dragged[<0]
            camera_t *cam = camera_get_active();
            vec2 mouse_sensitivity = vec2(0.1, -0.1); // sensitivity + polarity
            vec2 drag = mul2( editor_toolbar_dragged(), mouse_sensitivity );
            if( choiceV == -1 ) camera_fps(cam, drag.x, drag.y );
            if( choiceV == -2 ) camera_orbit(cam, drag.x, drag.y, 0); //len3(cam->position) );
            if( choiceV == -3 ) camera_moveby(cam, scale3(vec3(drag.x, drag.y, 0), 10)) ;
            if( choiceV == -4 ) camera_fov(cam, cam->fov += drag.y - drag.x);
            if( choiceV ==  5 ) cam->orthographic ^= 1, camera_fps(cam, 0, 0);
        }
        if( choiceH ) {
            if( choiceH ==  2 ) pos_snapping ^= 1;
        }

        //
        char *cam_info = NULL, *cam_info_fmt = FONT_BOTTOM FONT_RIGHT FONT_H5;
        if( choiceV == -1 ) cam_info = va("%s(CAM POS: %5.2f,%5.2f,%5.2f)", cam_info_fmt, cam->position.x, cam->position.y, cam->position.z, cam->fov);
        if( choiceV == -2 ) cam_info = va("%s(CAM YAW: %5.2f PITCH: %5.2f)", cam_info_fmt, cam->yaw, cam->pitch);
        if( choiceV == -4 ) cam_info = va("%s(CAM FOV: %5.2f)", cam_info_fmt, cam->fov);
        if( choiceV == -5 ) cam_info = va("%s(CAM ORTHOGRAPHIC: %d)", cam_info_fmt, cam->orthographic);
        if( choiceH == -2 ) cam_info = va("%s(OBJ SNAPPING: %d)", cam_info_fmt, pos_snapping);
        if( cam_info ) font_print(cam_info);
    }
}
