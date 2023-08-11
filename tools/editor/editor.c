// in-game editor
// - rlyeh, public domain.

// ## Design
// ### editor (v1)
// The v1 editor is a tool that understands Assets and is able to *edit the details of such Assets*. 
// This understanding is configured via reflection fields in .ini files. Can be reflected from C as well.
// The reflected properties will enable loading, saving and creating generic widget views to edit the Assets.
// Because we load and save the state of Assets, we can also undo and redo changes by simulating load/saves from/into memory. 
// And we can also dump the contents into disk, and create diffs and patches from them.
// - [x] Load Assets
// - [x] Edit Assets
// - [x] Save Assets
// - [x] Undo Assets (automatic, via loading older state)
// - [x] Redo Assets (automatic, via loading newer state)
// - [x] Diff Assets (from two states)
// - [x] Mend Assets (from two states)
//
// Note that the editor is dumb and does not tick/draw your GameObjects. It does not handle the Scene/World either. 
// Those are game-driven systems. Your game should provide the meanings to actually:
// - [?] Spawn Assets
// - [?] Delete Assets
// - [x] Draw Assets
// - [x] Tick Assets
// - [*] Scene Traversals (parent->children*, visibility, collisions and such)
//
// PS: Asset pipeline is external to the editor. Exotic assets could use some fancy plugins to deal with the import/export; eg, Substance 3D.
// PS: Asset versioning is also external to the editor. We could integrate a few VCS plugins within the editor: p4, git, svn, ...
//
// ### editor (v2)
// The v2 editor adds container support and modding features from previous editor.
//
// Your game could use fancy containers everywhere now. However, for simplicity purposes, Editor would be ignorant about them as well.
// Editor can only use containers that can decay to vectors and strides. Examples:
// - [x] Vectors: already supported in the Editor.
// - [?] Arrays: can decay to a fixed/immutable vector.
// - [?] Sparse/Unordered/Ordered Sets: can decay to vector of values.
// - [?] Sparse/Unordered/Ordered Maps: can decay to vector of keys + vector of values.
// - [?] Other fancy containers: can iterate on elements and send each item to editor individually; might be slow.
//
// We also allow here for others to extend or *override the behavior and look of each window and/or widget* by using .lua and .dll plugins:
// - [ ] Draw Windows              --> Custom overrides to alter or enhance the renderer while editing. Via C/C++/dll/lua plugins
// - [ ] Tick Windows              --> Custom overrides to alter or enhance the behavior while editing. Via C/C++/dll/lua plugins
//
// ### editor (v3)
// v3 brings in data driven control; ie, be able to parse & interpret text commands from any input stream. 
// This would allow for remote control (via OSC), extending scripts, offline commands, Telnet sessions or external tools; like GDB does.
// - [ ] Data driven
//
// The v3 editor is also a bootstrapped v2 editor with tons of .luas. The C skeleton is only a window manager at this point.
// The intention here is to *leverage editing workflow purely into data-driven files*, so the engine can grow up exponentially from here.
// Data-driven on steroids. It would be totally a success if the editor could be bootstrapped to include these kind of sub-editors without much work on the C codebase:
// - [ ] Level 2D/Blockout editor
// - [ ] Level 3D/Blockout editor
// - [*] World outliner
// - [ ] Nodegraph editor (ShaderGraph, AnimGraph, AudioGraph, MaterialGraph, DialogGraph, AIGraph, QuestGraph, ...)
// - [ ] Sequencer
// - [ ] Tracker (music tracker)
// - [ ] Etc...
//
// ### editor (v4)
// Bring in remote datas into the editor.
// Go social & marketplace. Allow others to expand, share, publish, subscribe, discuss their sub-editors within a small community.
// I really like the way the way OpenFrameworks.cc does their addons, and I think we should do same: just discover and monitor github repos, and list everything on a website (fwk- prefix?).
// Wishlist for a github-based community flow: discovery, transparent installs, publish on github, star there, watch commits & releases, track issues+discussions, etc
//
// We should have a generic, extensible, script/plugin-driven, working editor at this point (hopefully) that does not require maintenance.

// ## Roadmaps
// ### v1 roadmap (current)
// - [*] menu: open, save, save as, save all, reload
// - [x] basic gizmos (@todo: fixed screen size, snapping)
// - [ ] add/rem entities, add/rem components, add/rem/pause/resume systems
// - [ ] cut/copy/paste (ctrl-c to serialize)
// - [ ] F1, detach from game (long press will send F1 key to game)
// - [ ] TAB, isolated view of selected entity on/off. (long press will send TAB key to game)
// - [ ] standardise binary format. msgpack(<->json)? go .ini instead? .ini+blob? .kvdb?
// - [*] object processing from game: tick,draw*,spawn,delete,un/load from bvh stream,
// - [ ] cut/copy/paste <--> add/del events into game (ctrl-c to serialize)
// - [x] multiple selections/select all
// - [x] tree traversal from game (parent->child)
// - [ ]   operations on trees: load/save -> as filesystem or .zipped level
//
// ### v2 roadmap (mid-term)
// - [ ] add keyboard shortcuts
// - [ ] tree traversal from game
// - [ ]   bvh and collision queries
// - [ ]   visibility and pvs queries
// - [ ] art/ vs prefabs/ discrimination: prefabs/ are archetypes (composed types); ie, data-classes. art/ contains data files.
// - [ ]   can prefabs be done with ecs maybe?
// - [ ]   example: levels are prefabs, composed of other sub-prefabs or art assets.
// - [ ]   example: hitboxes+events. girl=pivot(p,r,s)+model(mesh,tex)+curframe
// - [ ]   extend widgets vec3 as range;customized mesh,texture,audio,any other asset,combo of anything)
//
// ### v3 roadmap (long-term)
// ### v4 roadmap (long-term)
// - [ ] osc server for properties and editor behavior
//

// ## Implementation ideas
// ### editor
// - [x] editor = tree of nodes. world, levels and objects are nodes, and even widgets are also nodes.
// - [ ] you can perform actions on some or all of these nodes, with or without descendants, from any top-bottom or bottom-top directions.
// - [ ] these actions include load/save, reset, undo/redo, play/render, toggle vis:on/off/alpha logic:on/off/other ddraw:on/off log:on/off, etc.
// and that's all.
//
// ### organization: world as a filesystem
// - [ ] anything can be serialized into disk. any object, any entity, any property or any widget can be serialized into disk.
// - [ ] groups of them as well. the whole world state can be serialized into disk as a filesystem snapshot:
// - [ ] - entities are folders. you can attach nodes on nodes (ie, create folders inside folders).
// - [ ] - systems are dlls/scripts. you can modify them on the fly and they should reload.
// - [ ] - components are data files. each component is a file. some components may be pure datas (ie, raw textures) but some others can be human-readable and editable.
//         inside of that, every line is a JSON/INI property that you can tweak, modify or inspect.
//
// ### replication: diffing zips
// - [ ] the whole world/filesystem will be compressed into a zipfile and delivered to the network when sharding/replicating in a network scenario.
// - [ ] clients will be diffing/patching their filesystems on every received packet. there will be 3 operations to support internally that will reflect what the E/C/S world is doing behind the curtains:
// - [ ] - added files/folders [+] : when creating entities/components/systems
// - [ ] - deleted files/folders [-] : when removing entities/components/systems
// - [ ] - modifying files/folders [*] : when altering entities/components/systems
//
// ### communication: osc messages
// - [ ] any living entity in the game, or within the editor, can be inspected, debugged or tweaked from external tools.
// - [ ] in order to achieve that, an opensoundserver is listening on a binding IP and you can send UDP packets to every node in the world.
// - [ ] the UDP port number matches current year (2021, 2022, 2023...)
//
// ### augmentation: widgets escalate from bottom
// - [x] there are only a few basic supplied widgets.
// - [x] and they correlate to C types: bool, u/int 8/16/32/64, float/double, strings and enums.
// - [x] structs are covered by reflecting and editing all members separately.
// - [ ] optionally, you can extend some basic types to have better visualization widgets.
//       ie, you could alias x4 float widgets together into a new shiny vec4 widget that is more compact, fancy and convenient to use.
//       then you can also alias that very same vec4 into a color picker for example; or maybe convert that vec3 into a position gizmo.
//       then maybe alias x2 color pickers and create a gradient widget. and so on...

// ## old notes below
// ==================
// - [ ] editor (json level): load/save jsons, property editor for anything (remote osc server/client)
// -     gizmo: proportional, orbit/arcball XY (+shift for Z/tilt)
// -     scene: scenegraph, obj naming, ~~obj picking, obj bounds,~~ obj collisions, obj/scene streaming
// -     placeholders google
// -     vcs
// - [ ] Level objects: ~~volumes, triggers, platforms, streaming~~.
// -     level: emitters: particles, lights, lightmaps, sound sources, triggers, etc
// -     level: box triggers, start/end, spawn, streaming, checkpoints,
// -     level: jump, shoots, platforms, collisions
// -     level: 60s, 70s, 80s, 90s
// - [ ] Core: wecs+replication
// -     modules: script or dll + ram load/save/diff/patch + play/stop/init/ + attach/detach
// -     logic tree/ << [] |> || >>
// -     - scene |>
// -        - enemies
// -     ecs: sys are modules, ecs: char *messaging, ecs: filesystem (e/dir,c/files,s/dll)
// -     world: streaming, migration

#include "fwk.h"

// #include "labs.vm/ecs.c"

#define EDITOR_VERSION "2022.7"

#if 1
#define EDITOR_PRINTF PRINTF
#else
#define EDITOR_PRINTF(...) do {} while(0)
#endif

// editor controls

//static int      editor_attached = 1;
static int        editor_enabled = 1;
static void*      editor_selected_obj = 0;
static int        editor_key = 0;
static vec2       editor_mouse = {0}; // 2d coord for ray/picking
static bool       editor_gamepad = 1;
static int        editor_hz = 60;
static int        editor_hz_mid = 18;
static int        editor_hz_low = 5;
static bool       editor_power_saving = 0;
static double     editor_t = 0, editor_dt = 0;
static bool       editor_lit = 1;
static bool       editor_ddraw = 1;

static
void editor_init_variables() {
}

bool editor_active() {
    return ui_hover() || ui_active() || gizmo_active() ? editor_enabled : 0;
}
double editor_ss() {
    return 1000 + editor_t;
}
double editor_delta() {
    return editor_dt;
}


enum editor_keys {
    key_none,
    key_pause,
    key_reload,
    key_browser,
    key_recording,
    key_fullscreen,
    key_screenshot, // @todo: add meta-info in exif or invisibile pixels (cam details, player details, map level, map location, level state, etc)
    key_quit,
    key_mute,
    key_battery,
    key_profiler,
    key_stop,
    key_outliner,
    key_undo,
    key_redo,
    key_save_mem,
    key_save_disk,
    key_load_disk,
    key_reset,
    key_debugger,
    key_gamepad,
    key_lit,
    key_ddraw,
};

// editor core

typedef void*        obj_t;
typedef array(obj_t) objs_t;

typedef struct property { // meta: "vec3 namespace.position = {1,2,3}; // minv=(0,0,0) key1=value1 key2=value2 [...] @this is a tooltip @@this is a comment"
    char *mark; // namespace
    char *name; // display name
    char *type; // pointed type
    char *hint; // name@tooltip
    char *minv; // min value
    char *maxv; // max value
#if 0 // @todo: implement me
    char *incv; // inc value
    char *defv; // default value
    char *isro; // is read-only/enabled
    char *issv; // is save pending
#endif
    void *value;
    unsigned typebits;
    unsigned flags;
} property;


// low-level operations
int save1(bool apply, array(char) *buffer, array(property) arrp) {
    // iterate and save
    unsigned total = 0;
    for each_array_ptr(arrp, property, p) {
        unsigned bytes = 0;
        /**/ if( p->type[0] == 'f') bytes = sizeof(float);
        else if( p->type[0] == 'v') bytes = sizeof(vec3);
        else if( p->type[0] == 'i') bytes = sizeof(int);
        else if( p->type[0] == 'b') bytes = sizeof(bool);

        if( !apply ) continue;

        if( bytes ) {
            array_resize(*buffer, array_count(*buffer) + bytes);
            memcpy( &(*buffer)[array_count(*buffer) - bytes], p->value, bytes);
            total += bytes;
        }
    }

    EDITOR_PRINTF("%d bytes written\n", total);
    return total;
}
int load1(bool apply, const char *buffer, unsigned buflen, array(property) arrp, unsigned skip_bytes) {
    // iterate and load properties
    unsigned cursor = 0, loaded = 0, limit = buflen;

    while( cursor <= skip_bytes )
    for each_array_ptr(arrp, property, p) {
        unsigned bytes = 0;
        /**/ if( p->type[0] == 'f') bytes = sizeof(float);
        else if( p->type[0] == 'v') bytes = sizeof(vec3);
        else if( p->type[0] == 'i') bytes = sizeof(int);
        else if( p->type[0] == 'b') bytes = sizeof(bool);

        if( (cursor + bytes) > limit ) {
            return -1;
        }

        if( apply && cursor >= skip_bytes ) {
        memcpy( p->value, &buffer[cursor], bytes);
        loaded += bytes;
        }

        cursor += bytes;
    }

    EDITOR_PRINTF("%d bytes read, %d bytes loaded\n", cursor, loaded);
    return cursor;
}

int diff1( array(char) src, array(char) dst, array(char) *patch ) { // @testme
    int slen = array_count(src);
    int dlen = array_count(dst);
    if( dlen != slen ) return -1;

    for( int i = 0; i < slen; ++i ) {
        int diff = dst[i] - src[i];
        array_push(*patch, (char)diff);
    }

    EDITOR_PRINTF("%d bytes diff\n", slen);
    return slen;
}

int patch1( array(char) src, array(char) dst, array(char) patch ) { // @testme
    int slen = array_count(src);
    int dlen = array_count(dst);
    if( dlen != slen ) return -1;

    int plen = array_count(patch);
    if( plen != dlen ) return -1;

    for( int i = 0; i < plen; ++i ) {
        dst[i] += patch[i];
    }

    EDITOR_PRINTF("%d bytes patched\n", plen);
    return plen;
}

// syntax sugars for collections/containers
// #define bulk_load(obj_min,obj_max,objs,...)  for( unsigned i = 0; i < array_count(objs); ++i ) { bool apply = obj_min >= i && i < obj_max; load1(apply, objs[i], __VA_ARGS__); }
// #define bulk_save(obj_min,obj_max,objs,...)  for( unsigned i = 0; i < array_count(objs); ++i ) { bool apply = obj_min >= i && i < obj_max; save1(apply, objs[i], __VA_ARGS__); }


// state - retained mode

typedef struct editor_state_t {
    array(property) properties;
    array(char)     buffer;
    array(vec2i)    history;
    unsigned        cursor;
} editor_state_t;

typedef map(char*, char*) editor_dict_t;

typedef struct editor_call_t {
    void* (*call)();
    unsigned bound;
    void *vargs[4];
} editor_call_t;

typedef struct editor_module_t {
    enum {
        fn_init,
         fn_load,
          fn_tick,
          fn_draw,
          fn_aabb,  // hitboxes
          fn_debug, // call for debug ui (like loggers and sliders)
         fn_save,
        fn_quit,

        fn_num_,
    } dummy;

    editor_call_t methods[fn_num_];

} editor_module_t;

static map(void*, editor_state_t)  editor_state; // world
static map(void*, array(void*))    editor_children; // groups for stacking, bvh and visibility
static map(void*, array(void*))    editor_children_tick; // groups for ticking
static map(void*, array(void*))    editor_children_draw; // groups for drawing
static map(void*, editor_module_t) editor_module;
static map(void*, editor_dict_t)   editor_dicts;
static set(void*)                  editor_world;
static set(void*)                  editor_selection; // objects selected in scene

void editor_init_states() {
    do_once map_init_ptr(editor_state);
    do_once map_init_ptr(editor_module);
    do_once map_init_ptr(editor_children);
    do_once map_init_ptr(editor_children_tick);
    do_once map_init_ptr(editor_children_draw);
    do_once map_init_ptr(editor_dicts);
    do_once set_init_ptr(editor_world);
    do_once set_init_ptr(editor_selection);
}

// handle selection

void editor_select(void *obj, bool multi) {
    do_once editor_init_states();

    editor_selected_obj = NULL;

    if(!multi) {
        set_clear(editor_selection);
        if( obj ) set_find_or_add(editor_selection, editor_selected_obj = obj);
    } else {
        if( !obj ) return;
        bool on = !!set_find(editor_selection, obj);
        if(on) set_erase(editor_selection, obj);
        else   set_find_or_add(editor_selection, editor_selected_obj = obj);
    }
}
bool editor_is_selected(void *obj) {
    do_once editor_init_states();

    return !!set_find(editor_selection, obj);
}
void editor_select_none(void) {
    editor_select(NULL, false);
}
void editor_select_all(void) {
    editor_select_none();
    for each_set_ptr(editor_world, void*, o) {
        void *obj = *o;
        editor_select(obj, true);
    }
}

// obj/scene: load/save, undo/redo,

bool editor_clear_redo(void *obj) {
    editor_state_t *ed = map_find_or_add(editor_state, obj, (editor_state_t){0});
    
    if( ed->cursor >= array_count(ed->history) ) 
        return false;

    array_resize(ed->buffer, ed->history[ed->cursor].to);
    array_resize(ed->history, ed->cursor + 1);
    return true;
}

bool editor_save_disk(const void *obj, const char *outfile) {
    editor_state_t *ed = map_find_or_add(editor_state, (void*)obj, (editor_state_t){0});

    static __thread array(char) buffer = 0;
    array_resize(buffer, 0); // <-- reused as an optimization

    bool ok = 0;
    if( save1(true, &buffer, ed->properties) > 0 ) {
        ok = file_write(outfile, buffer, array_count(buffer));
    }

    ui_notify("Save", ok ? "OK" : ICON_MD_WARNING " Failed!");
    return ok;
}

bool editor_load_disk(void *obj, const char *infile) {
    editor_state_t *ed = map_find_or_add(editor_state, obj, (editor_state_t){0});

    int buflen;
    char *buffer = file_load(infile, &buflen);
    if( buffer && buflen ) {
        if( load1(true, buffer, buflen, ed->properties, 0) > 0 ) {
            return true;
        }
    }

    return false;
}

bool editor_save_mem(void *obj) {
    editor_state_t *ed = map_find_or_add(editor_state, obj, (editor_state_t){0});

    static array(char) buffer = 0;
    array_resize(buffer, 0);

    // save
    int bytes = save1(true, &buffer, ed->properties);
    if( bytes <= 0 ) return false;

    // discard save if same size + same content (ie, no changes between this save and previous one)
    if( array_count(ed->history) > 1 ) {
        vec2i before = *array_back(ed->history);
        if( bytes == (before.to - before.from) ) {
            if( !memcmp(buffer, ed->buffer + array_count(ed->buffer) - bytes, bytes ) ) {
                return false; // puts("discarding save...");
            }
        }
    } 

#if 0
    // discard redo
    if( ed->cursor < array_count(ed->history) ) {
        array_resize(ed->buffer, ed->history[ed->cursor].to);
        array_resize(ed->history, ed->cursor + 1);
    }
#else
    editor_clear_redo(obj);
#endif

    // append
    int checkpoint = array_count(ed->buffer);
    array_resize(ed->buffer, array_count(ed->buffer) + bytes);
    memcpy(ed->buffer + checkpoint, buffer, bytes);

    // proceed
    array_push(ed->history, vec2i(checkpoint, array_count(ed->buffer)));

    // move cursor to latest
    ed->cursor = array_count(ed->history) - 1;

    return true;
}

bool editor_load_mem(void *obj) {
    editor_state_t *ed = map_find_or_add(editor_state, obj, (editor_state_t){0});

    // load latest & update history
    int slots = array_count(ed->history);
    if( slots )
        if( load1(true, ed->buffer, array_count(ed->buffer), ed->properties, ed->history[slots - 1].from) > 0 ) 
            return ed->cursor = slots - 1, true;

    return false;
}

bool editor_reset(void *obj) { // load first checkpoint
    editor_state_t *ed = map_find_or_add(editor_state, obj, (editor_state_t){0});
    
    // load first slot
    if( load1(true, ed->buffer, array_count(ed->buffer), ed->properties, ed->history[0].from) > 0 ) {
        // discard redo
        array_resize(ed->buffer, ed->history[0].to);
        // update history
        array_resize(ed->history, 1);
        // move cursor to latest
        ed->cursor = array_count(ed->history) - 1;
        return true;
    }

    return false;
}

bool editor_undo(void *obj) {
    editor_state_t *ed = map_find_or_add(editor_state, obj, (editor_state_t){0});

    // load previous & rewind history by -1
    if( ed->cursor > 0 ) 
        if( load1(true, ed->buffer, array_count(ed->buffer), ed->properties, ed->history[ed->cursor - 1].from) >= 0 )
            return ed->cursor -= 1, true;

    return false;
}

bool editor_redo(void *obj) {
    editor_state_t *ed = map_find_or_add(editor_state, obj, (editor_state_t){0});

    // load next & forward history by +1
    if( ed->cursor < (array_count(ed->history)-1) )
        if( load1(true, ed->buffer, array_count(ed->buffer), ed->properties, ed->history[ed->cursor + 1].from) >= 0 )
            return ed->cursor += 1, true;

    return false;
}

bool editor_diff(const void *obj1, const void *obj2, array(char) patch) { // @todo
    // @todo check: if both valid && both same type
    return false;
}
bool editor_patch(void *obj1, array(char) patch) { // @todo
    return false;
}

// obj/module: persist

char *editor_obj_intern(void *obj, const char *quark, const char *value) {
    editor_init_states();

    editor_dict_t *dict = map_find_or_add(editor_dicts, obj, 0);
    if( *dict == 0 ) map_init_str(*dict);

    char **key = map_find_or_add_allocated_key(*dict, STRDUP(quark), 0);
    if(*key) FREE(*key);
    *key = STRDUP(value);

    return *key;
}

char *editor_obj_string(const void *obj, const char *quark) {
    editor_dict_t *dict = map_find_or_add(editor_dicts, (void*)obj, 0);
    if( *dict == 0 ) map_init_str(*dict);

    char **key = map_find_or_add_allocated_key(*dict, STRDUP(quark), 0);
    return *key ? *key : "";
}

// obj/module: hierarchy

void editor_obj_childof_tick(void *obj, void *parent) {
    array(void*) *found = map_find(editor_children_tick, parent);
    if(!found) found = map_insert(editor_children_tick, parent, 0);
    if( obj && obj != parent ) { // dont recurse
        for( int i = 0; i < array_count(*found); ++i ) {
            if( (*found)[i] == obj ) return; // child was already added
        }
        array_push(*found, obj);
    }
}

void editor_obj_childof_draw(void *obj, void *parent) {
    array(void*) *found = map_find(editor_children_draw, parent);
    if(!found) found = map_insert(editor_children_draw, parent, 0);
    if( obj && obj != parent ) { // dont recurse
        for( int i = 0; i < array_count(*found); ++i ) {
            if( (*found)[i] == obj ) return; // child was already added
        }
        array_push(*found, obj);
    }
}

void editor_obj_childof(void *obj, void *parent) {
    array(void*) *found = map_find(editor_children, parent);
    if(!found) found = map_insert(editor_children, parent, 0);
    if( obj && obj != parent ) { // dont recurse
        for( int i = 0; i < array_count(*found); ++i ) {
            if( (*found)[i] == obj ) return; // child was already added
        }
        array_push(*found, obj);
    }
}

// obj/module: methods

typedef void* (*generic_method)();

void editor_obj_bind0(const void *obj, unsigned method, generic_method func ) {
    do_once editor_init_states();
    editor_call_t *m = &(map_find_or_add(editor_module, (void*)obj, (editor_module_t){0})->methods[method]);
    m->call = func;
    m->bound = 0;

    set_find_or_add(editor_world, (void*)obj);
}
void editor_obj_bind1(const void *obj, unsigned method, generic_method func, void *arg1 ) {
    do_once editor_init_states();
    editor_call_t *m = &(map_find_or_add(editor_module, (void*)obj, (editor_module_t){0})->methods[method]);
    m->call = func;
    m->vargs[0] = arg1;
    m->bound = 1;

    set_find_or_add(editor_world, (void*)obj);
}
void editor_obj_bind2(const void *obj, unsigned method, generic_method func, void *arg1, void *arg2 ) {
    do_once editor_init_states();
    editor_call_t *m = &(map_find_or_add(editor_module, (void*)obj, (editor_module_t){0})->methods[method]);
    m->call = func;
    m->vargs[0] = arg1;
    m->vargs[1] = arg2;
    m->bound = 2;

    set_find_or_add(editor_world, (void*)obj);
}
void editor_obj_bind3(const void *obj, unsigned method, generic_method func, void *arg1, void *arg2, void *arg3 ) {
    do_once editor_init_states();
    editor_call_t *m = &(map_find_or_add(editor_module, (void*)obj, (editor_module_t){0})->methods[method]);
    m->call = func;
    m->vargs[0] = arg1;
    m->vargs[1] = arg2;
    m->vargs[2] = arg3;
    m->bound = 3;

    set_find_or_add(editor_world, (void*)obj);
}
void editor_obj_bind4(const void *obj, unsigned method, generic_method func, void *arg1, void *arg2, void *arg3, void *arg4 ) {
    do_once editor_init_states();
    editor_call_t *m = &(map_find_or_add(editor_module, (void*)obj, (editor_module_t){0})->methods[method]);
    m->call = func;
    m->vargs[0] = arg1;
    m->vargs[1] = arg2;
    m->vargs[2] = arg3;
    m->vargs[3] = arg4;
    m->bound = 4;

    set_find_or_add(editor_world, (void*)obj);
}
void *editor_obj_call0(const void *obj, unsigned method ) {
    editor_call_t *m = &(map_find_or_add(editor_module, (void*)obj, (editor_module_t){0})->methods[method]);
    if( !m->call ) return 0;
    if( m->bound == 1 ) return (m->call)(obj, m->vargs[0]);
    if( m->bound == 2 ) return (m->call)(obj, m->vargs[0], m->vargs[1]);
    if( m->bound == 3 ) return (m->call)(obj, m->vargs[0], m->vargs[1], m->vargs[2]);
    if( m->bound == 4 ) return (m->call)(obj, m->vargs[0], m->vargs[1], m->vargs[2], m->vargs[3]);
    return (m->call)(obj);
}
void *editor_obj_call1(const void *obj, unsigned method, void *arg1 ) {
    editor_call_t *m = &(map_find_or_add(editor_module, (void*)obj, (editor_module_t){0})->methods[method]);
    return m->call ? m->call(obj, arg1) : 0;
}
void *editor_obj_call2(const void *obj, unsigned method, void *arg1, void *arg2 ) {
    editor_call_t *m = &(map_find_or_add(editor_module, (void*)obj, (editor_module_t){0})->methods[method]);
    return m->call ? m->call(obj, arg1, arg2) : 0;
}
void *editor_obj_call3(const void *obj, unsigned method, void *arg1, void *arg2, void *arg3 ) {
    editor_call_t *m = &(map_find_or_add(editor_module, (void*)obj, (editor_module_t){0})->methods[method]);
    return m->call ? m->call(obj, arg1, arg2, arg3) : 0;
}
void *editor_obj_call4(const void *obj, unsigned method, void *arg1, void *arg2, void *arg3, void *arg4 ) {
    editor_call_t *m = &(map_find_or_add(editor_module, (void*)obj, (editor_module_t){0})->methods[method]);
    return m->call ? m->call(obj, arg1, arg2, arg3, arg4) : 0;
}

// obj/module: ui/property

void editor_obj_property(void *obj, void *value, const char *metas) {
    do_once editor_init_states();

    ASSERT( obj );
    ASSERT( value );
    ASSERT( metas );
    char *meta = va("%s", metas);

    struct property p = {0};

    // parse tooltip, if present
    for( char *tooltip = strstr(meta, " @"); tooltip; *tooltip = 0, tooltip = 0) {
        p.hint = STRDUP(tooltip + 2);
    }

    // parse metas, if present
    for( char *metas = strstr(meta, "//"); metas; *metas = 0, metas = 0) {
        for each_substring(metas + 2, " ", token) {
        /**/ if(strbegi(token, "hint=")) token = token + 5 + strspn(token + 5, " "), p.hint = STRDUP(token);
        else if(strbegi(token, "minv=")) token = token + 5 + strspn(token + 5, " "), p.minv = STRDUP(token);
        else if(strbegi(token, "maxv=")) token = token + 5 + strspn(token + 5, " "), p.maxv = STRDUP(token);
        }
    }

    // parse declaration
    unsigned field = ~0u;
    for each_substring(meta, "={,}(); ", token) {
        // next field
        ++field;
        // parse fields
        /**/ if(field == 0) p.type = STRDUP(token);
        else if(field == 1) { // either name or namespace.name
            p.name = strchr(token, '.');
            if( !p.name ) p.name = STRDUP(token);
            else p.name = STRDUP(p.name + 1), p.mark = STRDUP(token), *strchr(p.mark, '.') = '\0';
        }
        else {} // any initialization values here
    }

    // required fields
    ASSERT(p.name);
    ASSERT(p.type);

    // combine name+hint together
    if( p.hint ) {
        char *combined = va("%s@%s", p.name, p.hint);
        FREE(p.hint), p.hint = 0;
        strcatf(&p.hint, "%s", combined);
    } else {
        p.hint = p.name;
    }

    // defaults
    p.value = value;

    editor_state_t *ed = map_find_or_add(editor_state, obj, (editor_state_t){0});
    array_push( ed->properties, p );
}

void* editor_obj_get_property_by_index(const void *obj, unsigned property_no) {
    editor_state_t *ed = map_find_or_add(editor_state, (void*)obj, (editor_state_t){0});
    return property_no < array_count(ed->properties) ? ed->properties[property_no].value : 0;
}

void* editor_obj_get_property_by_name(const void *obj, const char *property_name) {
    editor_state_t *ed = map_find_or_add(editor_state, (void*)obj, (editor_state_t){0});
    for each_array(ed->properties, struct property, p) {
        if( !strmatchi(p.name, property_name) ) continue;
        return p.value;
    }
    return 0;
}

bool editor_obj_render_min_properties(const void *obj, const char *mask) {
    editor_state_t *ed = map_find_or_add(editor_state, (void*)obj, (editor_state_t){0});

    if(!mask) return false;

    const char *section = 0;
    for each_array(ed->properties, struct property, p) {
        if( p.mark ) {
            if( section == 0 || strcmp(section, p.mark) ) {
                if( section != 0 ) ui_separator();
                section = p.mark;
                ui_label(va("*%s", section)); // '*' adds bold style in labels
            }
        }

        if( !strmatchi(p.name, mask) ) continue;

        /**/ if( p.type[0] == 'l') ui_label(p.hint);
        else if( p.type[0] == 'f') ui_float(p.hint, p.value);
        else if( p.type[0] == 'v') ui_float3(p.hint, p.value);
        else if( p.type[0] == 'i') ui_int(p.hint, p.value);
        else if( p.type[0] == 'b') ui_bool(p.hint, p.value);
    }
    return true;
}

void editor_obj_render_max_properties(void *obj, const char *mask) { // headless, needs layout (window/panel)
    const char *toolbar_text = va(
        ICON_MD_LOOP ";" ICON_MD_SD_CARD ";" ICON_MD_UNDO ";" ICON_MD_REDO ";Save;Load;%s",
            va(ICON_MD_BUG_REPORT "@Raise breakpoint (%sebugger detected).", has_debugger() ? "D":"No d")); // ICON_MD_FRONT_HAND

    int button = ui_toolbar(toolbar_text);
    if( button ) {
        if( button == 1 ) editor_key = key_load_disk; // key_reset;
        if( button == 2 ) editor_key = key_save_disk;
        if( button == 3 ) editor_key = key_undo;
        if( button == 4 ) editor_key = key_redo;
        if( button == 5 ) editor_key = key_save_disk;
        if( button == 6 ) editor_key = key_load_disk;
        if( button == 7 ) editor_key = key_debugger;
    }

    ui_separator();

    editor_obj_render_min_properties(obj, mask);

    ui_separator();
    ui_label("*Debug");

    editor_state_t *ed = map_find_or_add(editor_state, obj, (editor_state_t){0});

    static char *s = 0; if(s) *s = 0;
    for( int i = 0; i < array_count(ed->history); ++i ) strcatf(&s, ",%s%d..%d", ed->cursor == i ? "->":"", (int)ed->history[i].from, (int)ed->history[i].to);
    if(s) ui_label(va("Object Savepoints: %s", s+1));

    ui_buffer("Object console", va("%s","(empty)"), 7+1);
}

// main editor interface

void editor_render_menubar() {
    int alts = input(KEY_LALT) || input(KEY_RALT); // @todo: move to fwk.c
    int ctrls = input(KEY_LCTRL) || input(KEY_RCTRL); // @todo: move to fwk.c
    int shifts = input(KEY_LSHIFT) || input(KEY_RSHIFT); // @todo: move to fwk.c
    int mods = alts || ctrls || shifts; // @todo: move to fwk.c
    if( input_down(KEY_F5) )  editor_key = key_reload;
    if( input_down(KEY_F11) ) editor_key = key_fullscreen;
    if( input_down(KEY_PAUSE) ) editor_key = key_pause;
    if( input_down(KEY_PRINT) ) editor_key = (mods ? key_recording : key_screenshot);
    // if( input_down(KEY_W) && input_held(KEY_LCTRL) ) editor_key = key_quit;

    if( ctrls ) {
        /**/ if( input_down(KEY_Z) ) editor_key = key_undo;
        else if( input_down(KEY_Y) ) editor_key = key_redo;
        else if( input_down(KEY_S) ) editor_key = key_save_disk;
        else if( input_down(KEY_A) ) editor_select_all();
        else if( input_down(KEY_D) ) editor_select_none();
    }

    if( !editor_key /*&& !input_anykey()*/ && editor_selected_obj ) {
        if( input_up(MOUSE_L)    ) editor_key = key_save_mem;
        if( input_down(MOUSE_R)  ) ui_show("Properties", true);
        #if 0
        {
            vec2 dims = { 200, 400 };
            if( nk_tooltip_begin(ui_ctx, dims.w)) {
                nk_layout_row_dynamic(ui_ctx, dims.h, 1);
                editor_obj_render_min_properties(editor_selected_obj);
                nk_tooltip_end(ui_ctx);
            }
        }
        #endif
    }

    // @fixme: send all editor keys to game?
    // if( input_repeat(KEY_ESC, 300)) {}
    // if( input_repeat(KEY_F1, 300)) {}
    // etc...

    // menubar

    if( ui_menu( ICON_MD_SETTINGS "@Preferences;" 
                ICON_MD_G_TRANSLATE " Language;" 
                ICON_MD_FACE " Profile;" // editor account, but also fake profile and 1st party credentials
                ICON_MD_MESSAGE " Social;"
                ICON_MD_ROCKET_LAUNCH " Game;" // 
                ICON_MD_KEYBOARD " Keyboard;" 
                ICON_MD_MOUSE " Mouse;" 
                ICON_MD_GAMEPAD " Gamepad;" 
                ICON_MD_MONITOR " Display;" // @todo: RENDER settings, AUDIO settings
                ICON_MD_WIFI " Network;"
                ICON_MD_SAVINGS " Budget;" // mem,gfx,net,hdd,... also logging
                ICON_MD_CREATE_NEW_FOLDER " Folders;" // including project folders
                ICON_MD_EXTENSION " Plugins;" // including VCS
                ICON_MD_REPLAY " Restart;"
                ICON_MD_CLOSE " Quit;"
                "-" ICON_MD_RECYCLING " Reset all preferences;" ICON_MD_SAVE_AS " Save all preferences"
    ) ) {
        if( ui_item() == 3 ) {} // key mappings
        if( ui_item() == 4 ) {} // sensitivity, invert xylrw
        if( ui_item() == 5 ) {} // sensitivity, invert xy,ab, deadzones
        if( ui_item() == 7 ) {} // name,email,icon,link,github
        if( ui_item() == 13) editor_key = key_reload;
        if( ui_item() == 14) editor_key = key_quit;
    }

    if( ui_menu( window_has_pause() ? ICON_MD_PLAY_ARROW "@Tap to Play Game" : ICON_MD_PAUSE "@Tap to Pause Game" )) editor_key = key_pause;
    if( ui_menu( ICON_MD_STOP "@Stop game" )) editor_key = key_stop;
    if( ui_menu( ICON_MD_CLOSE "@Close game" ) ) {}
    static char game_args[16] = "--game-args"; // @fixme @todo remove '_' special char to signal that ui_menu() is writeable (inputbox)
    if( ui_menu_editbox( game_args, 16 ) ) {}

    // ICON_MD_TROUBLESHOOT -> PROFILER
    // ICON_MD_SCHEMA -> GRAPHNODES
    // ICON_MD_ACCOUNT_TREE -> GRAPHNODES
    // ICON_MD_TIPS_AND_UPDATES -> BULB
    // if( ui_menu( ICON_MD_MENU )) {}

    if( ui_menu( ICON_MD_FOLDER_SPECIAL "@Content browser" )) editor_key = key_browser;
    if( ui_menu( va(ICON_MD_VIEW_IN_AR " %d/%d@World outliner", set_count(editor_selection), map_count(editor_state) ))) editor_key = key_outliner;

    if( ui_menu( va(ICON_MD_BUILD "@Build game"))) ui_notify("Build", ICON_MD_WARNING " Not implemented.");

    if( ui_menu( ICON_MD_PHOTO_CAMERA "@Take Screenshot" )) editor_key = key_screenshot; // MD_SCREENSHOT_MONITOR
    if( ui_menu( record_active() ? ICON_MD_VIDEOCAM_OFF "@Stop video recording" : ICON_MD_VIDEOCAM "@Start video recording" )) { if(record_active()) record_stop(); else editor_key = key_recording; }
    if( ui_menu( editor_gamepad ? ICON_MD_VIDEOGAME_ASSET "@Gamepad is enabled. Tap to disable." : ICON_MD_VIDEOGAME_ASSET_OFF "@Gamepad is disabled. Tap to enable." )) editor_key = key_gamepad;
    if( ui_menu( audio_volume_master(-1) > 0 ? ICON_MD_VOLUME_UP "@Audio is enabled. Tap to mute." :  ICON_MD_VOLUME_OFF "@Audio is muted. Tap to enable." )) editor_key = key_mute;
    if( ui_menu( window_has_fullscreen() ? ICON_MD_FULLSCREEN_EXIT "@Fullscreen. Tap to go Windowed." : ICON_MD_FULLSCREEN "@Windowed. Tap to go Fullscreen." )) editor_key = key_fullscreen;

    if( ui_menu( editor_ddraw ? ICON_MD_IMAGE_SEARCH "@Debug renderer. Tap to go Retail Renderer." : ICON_MD_INSERT_PHOTO "@Retail renderer. Tap to go Debug Renderer." )) editor_key = key_ddraw; // ICON_MD_ADD_PHOTO_ALTERNATE
    if( ui_menu( editor_lit ? ICON_MD_LIGHTBULB "@Lit. Tap to disable lighting." : ICON_MD_LIGHTBULB_OUTLINE "@Unlit. Tap to enable lighting." )) editor_key = key_lit;

    // logic: either plug icon (power saving off) or one of the following ones (power saving on):
    //        if 0% batt (no batt): battery alert
    //        if discharging:       battery levels [alert,0..6,full]
    //        if charging:          battery charging
    int battery_read = app_battery();
    int battery_level = abs(battery_read);
    int battery_discharging = battery_read < 0 && battery_level < 100;
    const char *battery_levels[9] = { // @todo: remap [7%..100%] -> [0..1] ?
        ICON_MD_BATTERY_ALERT,
        ICON_MD_BATTERY_0_BAR,ICON_MD_BATTERY_1_BAR,
        ICON_MD_BATTERY_2_BAR,ICON_MD_BATTERY_3_BAR,
        ICON_MD_BATTERY_4_BAR,ICON_MD_BATTERY_5_BAR,
        ICON_MD_BATTERY_6_BAR,ICON_MD_BATTERY_FULL,
    };
    if( ui_menu( !editor_power_saving ? ICON_MD_POWER"@Full power. Tap to save power." :
        va("%s@Power saving. Tap to go full power. %3d%% battery.",
            battery_read == 0 ? ICON_MD_BATTERY_ALERT :
            battery_discharging ? battery_levels[(int)((9-1)*clampf(battery_level/100.f,0,1))] :
              ICON_MD_BATTERY_CHARGING_FULL, battery_level) ))
        editor_key = key_battery;

    // @todo: combine-in-1? cycle mem -> cpu/profiler -> network mon -> debugger
    if( ui_menu(va(ICON_MD_SIGNAL_CELLULAR_ALT " 0/0KiB" ))) {} // SIGNAL_CELLULAR_1_BAR SIGNAL_CELLULAR_2_BAR
    if( ui_menu(va(ICON_MD_STORAGE " %s", xstats() ))) {} // 012/136MB
    if( ui_menu(va(ICON_MD_SPEED " %5.2f/%d", window_fps(), (int)window_fps_target()))) editor_key = key_profiler; // 012/136MB

    // @todo: alarm/snooze, chrono, time (calendar?)
    {
        static double base = 0, tap = 0, delta = 0, enabled = 0;
        double timer = base + delta;
        if( ui_menu( !enabled ?
            va(ICON_MD_TODAY "%02d:%02d@Tap to start chrono.", (int)((date() / 10000) % 100), (int)((date() / 100) % 100))
            :
            va(ICON_MD_TIMELAPSE "%02dh:%02dm:%02ds:%02df@Tap to reset chrono.",((int)(timer/3600))%24,((int)(timer/60))%60,((int)timer)%60,(int)((timer - (int)timer)*window_fps_target())))) {
            base = 0, tap = time_ss(), delta = 0;
            enabled = 1;
        }
        if( editor_key == key_stop ) enabled = 0;
        if( enabled ) {
            if( !window_has_pause() ) delta = time_ss() - tap;
            else base += delta, tap = time_ss(), delta = 0;
        }
    }

    for each_map_ptr(editor_state, void *, o, editor_state_t, ed) {
        profile_incstat("Editor.num_objects", +1);

        void *obj = *o;

        // auto-load from disk during init. @fixme kvdb database
        if( array_count(ed->history) == 0 )
            if( editor_load_disk(obj, editor_obj_string(obj, ".path")) )
                {}

        // auto-save in-mem during first edit
        if( array_count(ed->history) == 0 )
            editor_save_mem(obj);

        // @todo: continue if obj not found in selection set
        if( obj != editor_selected_obj )
            continue;

        if( editor_key == key_debugger  ) { breakpoint("User requested breakpoint on this object"); }
        if( editor_key == key_reset     ) { const char *ok = editor_reset(obj) ? "ok" : "err"; EDITOR_PRINTF("reset: %s\n", ok); }
        if( editor_key == key_save_mem  ) { const char *ok = editor_save_mem(obj) ? "ok" : "err"; EDITOR_PRINTF("mem saved: %s\n", ok); }
        if( editor_key == key_undo      ) { const char *ok = editor_undo(obj) ? "ok" : "err"; EDITOR_PRINTF("undo: %s\n",  ok); }
        if( editor_key == key_redo      ) { const char *ok = editor_redo(obj) ? "ok" : "err"; EDITOR_PRINTF("redo: %s\n",  ok); }
        if( editor_key == key_save_disk ) { const char *ok = editor_save_disk(obj, editor_obj_string(obj, ".path")) ? "ok" : "err"; EDITOR_PRINTF("save: %s\n",  ok); }
        if( editor_key == key_load_disk ) { const char *ok = editor_load_disk(obj, editor_obj_string(obj, ".path")) ? "ok" : "err"; EDITOR_PRINTF("load: %s\n",  ok); }
    }

    char *name;
    switch( editor_key ) {
        default:
        break; case key_quit:       record_stop(), exit(0);
        break; case key_stop:       window_pause(1);
        break; case key_mute:       audio_volume_master( 1 ^ !!audio_volume_master(-1) );
        break; case key_pause:      window_pause( window_has_pause() ^ 1 );
        break; case key_reload:     window_reload();
        break; case key_battery:    editor_power_saving ^= 1;
        break; case key_browser:    ui_show("File Browser", ui_visible("File Browser") ^ true);
        break; case key_outliner:   ui_show("Outliner", ui_visible("Outliner") ^ true);
        break; case key_recording:  name = file_counter(va("%s.mp4",app_name())), window_record(name),     ui_notify(va("Video capturing: %s", name), date_string());
        break; case key_screenshot: name = file_counter(va("%s.png",app_name())), window_screenshot(name), ui_notify(va("Screenshot: %s", name), date_string());
        break; case key_profiler:   ui_show("Profiler", profile_enable(ui_visible("Profiler") ^ true));
        break; case key_fullscreen: record_stop(), window_fullscreen( window_has_fullscreen() ^ 1 ); // framebuffer resizing corrupts video stream, so stop any recording beforehand
        break; case key_gamepad:    editor_gamepad ^= 1;
        break; case key_lit:        editor_lit ^= 1;
        break; case key_ddraw:      editor_ddraw ^= 1;
    }
}

int do_context_cmd = 0;
void *do_context_obj = 0;

void editor_obj_render_properties_recursively(void *obj, const char *mask) {
    array(void*) *found = map_find(editor_children, obj);
    int num_subobjects = found ? array_count(*found) : 0;

    char *name = editor_obj_string(obj,".name"); name = name[0] ? name : va("%p", obj);
    char *title = va("%s%s/ (%d)",
        editor_is_selected(obj) ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK, name, num_subobjects);
//  if( !strmatchi(title, mask) ) return;
    char *id = va("LVL%p",obj);

    int clicked_or_toggled, open; // 1|clicked, 2|toggled
    for( int p = (open = ui_collapse(title, id)), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {

        // contextual menu (open)
        if( ui_contextual() ) {
            if( ui_button_transparent("<Load" ) ) do_context_obj = obj, do_context_cmd = cc4(load);
            if( ui_button_transparent("<Save" ) ) do_context_obj = obj, do_context_cmd = cc4(save);
            if( ui_button_transparent("<Merge") ) do_context_obj = obj, do_context_cmd = cc4(merge);
            if( ui_button_transparent("<Cut"  ) ) do_context_obj = obj, do_context_cmd = cc4(cut);
            if( ui_button_transparent("<Copy" ) ) do_context_obj = obj, do_context_cmd = cc4(copy);
            if( ui_button_transparent("<Paste") ) do_context_obj = obj, do_context_cmd = cc4(paste);
            ui_contextual_end();
        }

        for( int i = 0; i < num_subobjects; ++i ) {
            editor_obj_render_properties_recursively((*found)[i], mask);
        }

        if( num_subobjects == 0 ) {
            // if( (mask[0] == '*' && !mask[1]) || strmatchi(title, mask) )
            editor_obj_render_min_properties(obj, mask);
        }
    }

    // contextual menu (close)
    if( !open && ui_contextual() ) {
        if( ui_button_transparent("<Load" ) ) do_context_obj = obj, do_context_cmd = cc4(load);
        if( ui_button_transparent("<Save" ) ) do_context_obj = obj, do_context_cmd = cc4(save);
        if( ui_button_transparent("<Merge") ) do_context_obj = obj, do_context_cmd = cc4(merge);
        if( ui_button_transparent("<Cut"  ) ) do_context_obj = obj, do_context_cmd = cc4(cut);
        if( ui_button_transparent("<Copy" ) ) do_context_obj = obj, do_context_cmd = cc4(copy);
        if( ui_button_transparent("<Paste") ) do_context_obj = obj, do_context_cmd = cc4(paste);
        ui_contextual_end();
    }

    if( clicked_or_toggled & 1 ) {
        if( input_down(MOUSE_L) && (input(KEY_LCTRL) || input(KEY_RCTRL)) )
            editor_select(obj, true);
    }
}


enum {
    TICK_ENABLED = 1,
    TICK_DO_UI = 4,

    DRAW_ENABLED = 1,
    DRAW_DO_UI = 4,
};

void editor_tick_objs_recursively(void *obj, unsigned flags) {
    array(void*) *found = map_find(editor_children_tick, obj);
    int num_subobjects = found ? array_count(*found) : 0;

    if( flags & TICK_ENABLED ) editor_obj_call0(obj, fn_tick);

    if( flags & TICK_DO_UI ) {
        char *name = editor_obj_string(obj,".name"); name = name[0] ? name : va("%p", obj);
        char *title = va("%s%s/ (%d)", editor_is_selected(obj) ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK, name, num_subobjects);
    //  if( !strmatchi(title, mask) ) return;
        char *id = va("CPU%p",obj);

        int clicked;
        for( int p = ui_collapse(title, id), dummy = (clicked = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {

            int choice = ui_submenu("<A;B;>C");
            if( choice ) printf("%d\n", choice);

            for( int i = 0; i < num_subobjects; ++i ) {
                editor_tick_objs_recursively((*found)[i], flags);
            }
        }
    } else {
        for( int i = 0; i < num_subobjects; ++i ) {
            editor_tick_objs_recursively((*found)[i], flags);
        }
    }
}

void editor_draw_objs_recursively(void *obj, unsigned flags) {
    array(void*) *found = map_find(editor_children_draw, obj);
    int num_subobjects = found ? array_count(*found) : 0;

    if( flags & DRAW_ENABLED ) editor_obj_call0(obj, fn_draw);

    if( flags & DRAW_DO_UI ) {
        char *name = editor_obj_string(obj,".name"); name = name[0] ? name : va("%p", obj);
        char *title = va("%s%s/ (%d)", editor_is_selected(obj) ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK, name, num_subobjects);
    //  if( !strmatchi(title, mask) ) return;
        char *id = va("GPU%p",obj);

        int clicked;
        for( int p = ui_collapse(title, id), dummy = (clicked = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {

            int choice = ui_submenu("<A;<B;=C;>D");
            if( choice ) printf("%d\n", choice);

            for( int i = 0; i < num_subobjects; ++i ) {
                editor_draw_objs_recursively((*found)[i], flags);
            }
        }

    } else {
        for( int i = 0; i < num_subobjects; ++i ) {
            editor_draw_objs_recursively((*found)[i], flags);
        }
    }
}

void editor_render_windows() {
    // content browser
    if( ui_window("File Browser", 0) ) {
        const char *file = 0;
        if( ui_browse(&file, NULL) ) {
            const char *sep = ifdef(win32, "\"", "'");
            app_exec(va("%s %s%s%s", ifdef(win32, "start \"\"", ifdef(osx, "open", "xdg-open")), sep, file, sep));
        }
        ui_window_end();
    }

    // console/terminal window
    if( 0 && ui_window("Console", 0) ) {
        ui_console();
        ui_window_end();
    }

    // Scene/nodes
    if( ui_window("Outliner", 0) ) {
        // @todo: keys up,down,left,right -> tree nav
        // @todo: tab -> cycle next node of matching highlighted type

        static int do_filter = 0;

        int choice = ui_toolbar(ICON_MD_SEARCH"@Filter;"ICON_MD_UPLOAD"@Load;"ICON_MD_DOWNLOAD"@Save;"ICON_MD_REFRESH"@Reset;"ICON_MD_UNDO"@Undo;"ICON_MD_REDO"@Redo;");
        if( choice == 1 ) do_filter ^= 1;

        static char filter[128] = {0};
        if( do_filter ) {
            ui_buffer(ICON_MD_SEARCH " Filter", filter, 128);
        } else {
            filter[0] = '\0';
        }
        char *mask = filter[0] ? va("*%s*", filter) : "*";

#if 0
        static unsigned tabs = 0xFF;
        int choice = ui_toolbar( 
             "LV@Level tree: hierarchical logic datas used when ticking game.;"
             "RN@Rendering tree: hierarchical rendering datas used when drawing game.;"
             "VS@Visibility tree: hierarchical visibility datas used when ticking game and editor. Also collisions.;"
             "ST@Streaming tree: hierarchical streaming datas used when streaming content off disk.;"
             "PS@Persist tree: hierarchical storage datas within different game sessions.;"
             "PR@Prefabs tree: hierarchical datas of prefabs definitions.;"
             "ED@Editor tree: hierarchical datas used when ticking editor.;"
        );
#endif

        for( int c = ui_collapse(ICON_MD_FOLDER_SPECIAL " Art/", "ART"); c; ui_collapse_end(), c = 0) {
            static const char *file;
            static bool show_browser = 1;
            if( ui_browse(&file, &show_browser) ) {
                app_exec(va("%s %s", ifdef(win32, "start", ifdef(osx, "open", "xdg-open")), file));
                //puts(file);
            }
        }

        //
        for( int c = ui_collapse(va(ICON_MD_FACTORY " Prefabs/ (%d)", map_count(editor_state)), "PRF"); c; ui_collapse_end(), c = 0)
        for each_map_ptr(editor_state, void*, o, editor_state_t, ed) {
            void *k = *o;
            for( int p = ui_collapse(va("%s",strrchr(editor_obj_string(k,".path"),'/')+1), va("PF#%p",k)); p; ui_collapse_end(), p = 0) {
                editor_obj_render_min_properties(k, mask);
            }
        }

        // dynamic/static bounds: depth + bounds + visibility 
        do_context_cmd = 0;
        do_context_obj = 0;
        for( int c = ui_collapse(va(ICON_MD_ACCOUNT_TREE " Levels/ (%d)", map_count(editor_children)), "LVL"); c; ui_collapse_end(), c = 0)
        for each_map_ptr(editor_children, void*, o, array(void*), objs) {
            void *k = *o;
            editor_obj_render_properties_recursively(k, mask);
        }
        if( do_context_cmd == cc4(list) && do_context_obj ) {
            printf("list [%p]\n", do_context_obj);
        }
        // draw: depth + state (alpha0=off)
        // @fixme: make it a tree
        for( int c = ui_collapse(va(ICON_MD_PALETTE " Rendering/ (%d)", map_count(editor_children_draw)), "GPU"); c; ui_collapse_end(), c = 0)
        for each_map_ptr(editor_children_draw, void*, o, array(void*), objs) {
            void *k = *o;
            editor_draw_objs_recursively(k, DRAW_DO_UI);
        }
        // tick: depth + rate (00=off) --> logic
        // @todo: physics tick group? anim tick group? any other tick group?
        // @fixme: make it a tree
        for( int c = ui_collapse(va(ICON_MD_FLAG " Ticking/ (%d)", map_count(editor_children_tick)), "CPU"); c; ui_collapse_end(), c = 0)
        for each_map_ptr(editor_children_tick, void*, o, array(void*), objs) {
            void *k = *o;
            editor_tick_objs_recursively(k, TICK_DO_UI);
        }
        // init/quit: depth + prio
        // @fixme: make it a tree
        for( int c = ui_collapse(ICON_MD_CLOUD " Streaming/", "BVH"); c; ui_collapse_end(), c = 0) {}
        // save/load: depth + savetomem?yn + savetodisk?yn + quant + lossy/lossless
        // @fixme: make it a tree
        for( int c = ui_collapse(va(ICON_MD_SD_CARD " Storage/ (%d)", map_count(editor_dicts)), "DSK"); c; ui_collapse_end(), c = 0)
        for each_map_ptr(editor_dicts, void*, o, editor_dict_t, d) {
            void *k = *o;
            for( int p = ui_collapse(editor_obj_string(k,".name"), va("DSK%p",k)); p; ui_collapse_end(), p = 0) {
                for each_map(*d, char*, s, char *, v) {
                    ui_label(va("%s: %s", s, v));
                }
            }
        }

        // others
        for( int c = ui_collapse(ICON_MD_PRECISION_MANUFACTURING " Editors/", "EDT"); c; ui_collapse_end(), c = 0) {
            // @todo: add settings here as well?
        }

        for( int c = ui_collapse(ICON_MD_INFO " Help", "NFO"); c; ui_collapse_end(), c = 0) {
            ui_label("=*General");
            ui_label2("*ESC", ">Editor on/off");
            ui_label2("*F11", ">Fullscreen on/off");
            ui_label2("*F5", ">Refresh");
            ui_separator();
            ui_label("=*Edit");
            ui_label2("*^Z, ^Y", ">Undo, Redo");
            ui_label2("*^X, ^C, ^V", ">Cut, Copy, Paste");
            ui_label2("*^S, ^L, ^R", ">Save, Load*, Restart*");
            ui_separator();
            ui_label("=*Select");
            ui_label2("*LMB, ^A, ^D", ">Select, All, None");
            ui_label2("*RMB", ">Contextual menu*");
            ui_label2("*SPACE@Cycle transform gizmo: position, rotation, scale.", ">Cycle transform gizmo");
            ui_separator();
            ui_label("=*Camera");
            ui_label2("*Q,E,C", ">Camera elevation");
            ui_label2("*W,A,S,D", ">Camera move");
            ui_label2("*LMB/RMB+drag", ">Camera view");
            ui_label2("*WMB", ">Camera speed");
        }

        ui_window_end();
    }

    // UI properties
    if( ui_window("Properties", NULL) ) {
        if( editor_selected_obj )
            editor_obj_render_max_properties(editor_selected_obj, "*");
        ui_window_end();
    }

    // Icon browser
    if( ui_window("Icon Palette", 0 )) {
        static const char *icons[] = {
            #define ICON(x) ICON_MD_##x
            #include "editor"
        };
        static const char *titles[] = {
            #define ICON(x) #x
            #include "editor"
        };

        for( int i = 0, cols = 8; i < countof(icons); i += cols ) {
            char buf[256], *p = buf;
            for( int j = i, jmax = mini(i+cols, countof(icons)); j < jmax; ++j ) p += sprintf(p, "%s%03d@%s;", icons[j], j, titles[j]);
            ui_toolbar(buf);
        }

        ui_window_end();
    }
}

ray *editor_pickup() {
//  if(!window_has_cursor()) return NULL;

    // pick entity
    bool any_active = ui_active() || ui_hover() || gizmo_active() || gizmo_hover() || input_touch_active();
    if( editor_enabled && !any_active && input_down(MOUSE_L) ) {
        editor_mouse = vec2(input(MOUSE_X), input(MOUSE_Y));
        vec3 out = editor_pick(editor_mouse.x, editor_mouse.y); // unprj 2d as 3d coord
        vec3 from = camera_get_active()->position, to = out;
        static ray last_ray;
        last_ray = ray(from, to);
        return &last_ray;
    }

    return 0;
}

static
void editor_init() {
    // enable outlines
    do_once fx_load("fxEditorOutline.fs");
    do_once fx_enable(0, 1);

    // init editor
    do_once editor_init_states();
    do_once editor_init_variables();
}

bool editor() {
    do_once editor_init();

    // timing
    editor_dt = window_delta() * !window_has_pause(); if(editor_dt > 1/60.f) editor_dt = 1/60.f;
    editor_t += editor_dt;

    // enabled?
    if( input_up(KEY_ESC) ) editor_enabled ^= 1; // editor on/off
    if( !editor_enabled ) return false;

    // rendering + logic
    editor_key = 0;
    editor_render_windows();
    editor_render_menubar(); // must be last

    // adaptive framerate
    int app_on_background = !window_has_focus();
    int hz = app_on_background ? editor_hz_low : editor_power_saving ? editor_hz_mid : editor_hz; 
    window_fps_lock( hz < 5 ? 5 : hz );

    return true;
}

void editor_camera_fps(void) {
    static camera_t cam = {0};
    cam = *camera_get_active();

    vec3 move = {0};
    vec2 view = {0};

    // show/hide cursor
    bool dragging = input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
    bool any_active = ui_active() || ui_hover() || gizmo_active() || input_touch_active(); 
    if( any_active ) dragging = false;
    window_cursor( !dragging );

    // keyboard/mouse
    if( dragging ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
    vec3 wasdec = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_Q)||input(KEY_C)),input(KEY_W)-input(KEY_S)), cam.speed * !any_active);
    vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * dragging * !any_active);
    if( !input(KEY_LCTRL) && !input(KEY_RCTRL) ) // invalidate keys if pressing ctrl (ie, when saving CTRL-S)
    move = add3(move, wasdec); 
    view = add2(view, mouse);

    // gamepad
    if(0) {
    vec2 filtered_lpad = input_filter_deadzone(input2(GAMEPAD_LPAD), 0.15f /*15% deadzone*/);
    vec2 filtered_rpad = input_filter_deadzone(input2(GAMEPAD_RPAD), 0.15f /*15% deadzone*/);
    vec3 gamepad_move = scale3(vec3(filtered_lpad.x, input(GAMEPAD_LT) - input(GAMEPAD_RT), filtered_lpad.y), 1.0f);
    vec2 gamepad_view = scale2(filtered_rpad, 1.0f);
    move = add3(move, gamepad_move);
    view = add2(view, gamepad_view);
    }

    // multi-touch
    vec2 touch_move = input_touch_delta_from_origin(0, 0.0125f /*sensitivityFwd*/); // button #0 (left border)
    vec2 touch_view = input_touch_delta(1, 0.125f /*sensitivityRot*/);              // button #1 (right border) 
    move = add3(move, vec3(touch_move.x, 0, -touch_move.y));
    view = add2(view, vec2(touch_view.x, -touch_view.y));

    // apply inputs
    camera_move(&cam, move.x,move.y,move.z);
    camera_fps(&cam, view.x,view.y);    
}

// sugars
static void *editor_with = 0;
#define editor_obj(x)                (editor_with = (x))
#define editor_obj_childof(...)      editor_obj_childof(editor_with, __VA_ARGS__)
#define editor_obj_childof_tick(...) editor_obj_childof_tick(editor_with, __VA_ARGS__)
#define editor_obj_childof_draw(...) editor_obj_childof_draw(editor_with, __VA_ARGS__)
#define editor_obj_intern(...)       editor_obj_intern(editor_with, __VA_ARGS__)
#define editor_obj_property(...)     editor_obj_property(editor_with, __VA_ARGS__)
#define editor_obj_bind1(...)        editor_obj_bind1(editor_with, __VA_ARGS__)
#define editor_obj_bind2(...)        editor_obj_bind2(editor_with, __VA_ARGS__)
#define editor_obj_bind3(...)        editor_obj_bind3(editor_with, __VA_ARGS__)
#define editor_obj_bind4(...)        editor_obj_bind4(editor_with, __VA_ARGS__)


// my game

void* mygrid_draw(void *singleton, float *ground_size) {
    ddraw_ontop_push(0);
    ddraw_grid(*ground_size);
    ddraw_ontop_pop();
    ddraw_flush();
    return 0;
}
void* mymodel_draw(model_t *m, float pivot[16]) {
    camera_t *cam = camera_get_active();
    model_render(*m, cam->proj, cam->view, pivot, 0);
    return 0;
}
void* mymodel_tick(model_t *m, float pivot[16], vec3 *p, vec3 *r, vec3 *s) {
    rotationq44(pivot, eulerq(*r));
    scale44(pivot, s->x,s->y,s->z);
    relocate44(pivot,p->x,p->y,p->z);
    return 0;
}
void* mymodel_aabb(model_t *m, float pivot[16]) {
    static __thread struct aabb aabb[64];
    static __thread int counter = 0; counter = (counter + 1) % 64;
    aabb[counter] = model_aabb(*m, pivot);
    return &aabb[counter];
}


int main() {
    // 80% window, MSAAx2 flag
    window_create(80, WINDOW_MSAA2);
    window_title("Editor " EDITOR_VERSION " (wip)");
    window_icon("logo.png");

    // @fixme
    camera_t x = camera();

    // config ground floor
    float ground_size = 0;
    editor_obj(&ground_size);
    editor_obj_intern(".name", "ground");
    editor_obj_intern(".path", editor_path("ground.ini"));
    editor_obj_property(&ground_size, "float Size");
    editor_obj_bind1(fn_draw, mygrid_draw, &ground_size);

    // config 3d model #1
    mat44 witch_pivot; 
    vec3 witch_p = {-5,0,-5}, witch_r={-180,180,0}, witch_s={0.1,-0.1,0.1};
    model_t witch = model("witch/witch.obj", 0);
    model_set_texture(witch, texture("witch/witch_diffuse.tga.png", 0));

    editor_obj(&witch);
    editor_obj_childof(&ground_size);
    editor_obj_childof_tick(&ground_size);
    editor_obj_childof_draw(&ground_size);
    editor_obj_intern(".name", "witch");
    editor_obj_intern(".path", editor_path("witch.ini"));
    editor_obj_property(&witch_p, "vec3 Position"); // property #0
    editor_obj_property(&witch_r, "vec3 Rotation"); // property #1
    editor_obj_property(&witch_s, "vec3 Scale");    // property #2
    editor_obj_bind1(fn_draw, mymodel_draw, witch_pivot);
    editor_obj_bind1(fn_aabb, mymodel_aabb, witch_pivot);
    editor_obj_bind4(fn_tick, mymodel_tick, witch_pivot, &witch_p, &witch_r, &witch_s);

    // config 3d model #2
    mat44 girl_pivot; id44(girl_pivot);
    model_t girl = model("kgirl/kgirls01.fbx", 0);
    vec3 girl_p = {0,0,0}, girl_r = {270,0,0}, girl_s = {2,2,2};

    editor_obj(&girl);
    editor_obj_childof(&ground_size);
    editor_obj_childof_tick(&ground_size);
    editor_obj_childof_draw(&ground_size);
    editor_obj_intern(".name", "girl");
    editor_obj_intern(".path", editor_path("girl.ini"));
    editor_obj_property(&girl_p,  "vec3  Transform.Position; // @Position in world units");
    editor_obj_property(&girl_r,  "vec3  Transform.Rotation; // @Rotation in degrees");
    editor_obj_property(&girl_s,  "vec3  Transform.Scale;    // @Scale factor (decimal)");
    editor_obj_bind1(fn_draw, mymodel_draw, girl_pivot);
    editor_obj_bind1(fn_aabb, mymodel_aabb, girl_pivot);
    editor_obj_bind4(fn_tick, mymodel_tick, girl_pivot, &girl_p, &girl_r, &girl_s);

    // meta(&girl_frame, "float Animation.Frame; // @Animation frame");

    editor_select(&girl, false);

    ui_notify("Hint", "Keys I/J/K/L + Z/X to control the girl");

    // editor loop
    while( window_swap() ) {

        // editor tick
        profile("Editor") {
            editor();
            //ui_demo();
        }

        // fps camera
        if( /*editor_attached ||*/ editor_enabled ) {
            profile("Editor.Camera") {
                editor_camera_fps();
            }
        } else {
            profile("Game.Camera") {
                camera_t *cam = camera_get_active();

                static vec3 source;
                do_once source = cam->position;

                vec3 target = add3(girl_p, vec3(0,10,0));
                     target = add3(target, scale3(norm3(sub3(source, target)), 10.0));
                source = mix3(source, target, 1-0.99f);

                camera_teleport(cam, source);
                camera_lookat(cam, vec3(girl_p.x,0,girl_p.z));

                // @todo: orbit cam w/ right pad
            }
        }

        double GAME_JUMP_DOWN = input_down(KEY_Z);
        double GAME_FIRE_DOWN = input_down(KEY_X);
        double GAME_JUMP  = input(KEY_Z);
        double GAME_FIRE  = input(KEY_X);
        double GAME_LEFT  = input(KEY_J);
        double GAME_RIGHT = input(KEY_L);
        double GAME_UP    = input(KEY_I);
        double GAME_DOWN  = input(KEY_K);
        double GAME_AXISX = input(KEY_L) - input(KEY_J);
        double GAME_AXISY = input(KEY_I) - input(KEY_K);

        if( editor_gamepad && !input_anykey() ) {
            if( input(GAMEPAD_CONNECTED) ) {
                vec2 filtered_lpad = input_filter_deadzone(input2(GAMEPAD_LPAD), 0.15f /*15% deadzone*/);
                GAME_JUMP_DOWN = input_down(GAMEPAD_A);
                GAME_FIRE_DOWN = input_down(GAMEPAD_B) || input_down(GAMEPAD_X) || input_down(GAMEPAD_Y);
                GAME_JUMP  = input(GAMEPAD_A);
                GAME_FIRE  = input(GAMEPAD_B) || input(GAMEPAD_X) || input(GAMEPAD_Y);
                GAME_AXISX = filtered_lpad.x;
                GAME_AXISY = filtered_lpad.y;
            }
        }

        profile("Game.Animate scene") if( editor_delta() ) {
            float delta = editor_delta() * 30; // 30fps anim

            // animate girl
            girl.curframe = model_animate(girl, girl.curframe + delta);

            // jump controller: jump duration=1.5s, jump height=6 units, anims (expo->circ)
            float jump_delta = 1.0;
            static double jump_timer = 0, jump_ss = 1.5, jump_h = 6;
            if( GAME_JUMP_DOWN ) if( jump_timer == 0 ) jump_timer = editor_ss();
            jump_delta = clampf(editor_ss() - jump_timer, 0, jump_ss) * (1.0/jump_ss);
            if( jump_delta >= 1 ) { jump_timer = 0; }
            float y = ease_ping_pong( jump_delta, ease_out_expo, ease_out_circ);
            girl_p.y = y * jump_h;

            // punch controller
            float punch_delta = 1;
            if( jump_delta >= 1 ) {
            static vec3 origin;
            static double punch_timer = 0, punch_ss = 0.5;
            if( GAME_FIRE_DOWN ) if( punch_timer == 0 ) punch_timer = editor_ss(), origin = girl_p;
            punch_delta = clampf(editor_ss() - punch_timer, 0, punch_ss) * (1.0/punch_ss);
            if( punch_delta >= 1 ) { punch_timer = 0; }
            else {
                float x = ease_out_expo( punch_delta );
                vec3 fwd = rotate3q(vec3(0,0,1), eulerq(vec3(girl_r.x - 170,girl_r.y,girl_r.z)));
                vec3 mix = mix3(girl_p, add3(origin,scale3(fwd,x*2)), x);
                girl_p.x = mix.x, girl_p.z = mix.z;
            }
            }

            int modern_controller = 1;
            int running = 0;

            // girl controller

                // locomotion vars
                float speed = 0.2f * delta;
                float yaw_boost = GAME_AXISY > 0 ? 1.0 : 1.75;
                if(punch_delta < 1) yaw_boost = 0.0; // if firing...
                else if(punch_delta <= 0.1) yaw_boost = 4.0; // unless initial punch chaining, extra yaw

                // old fashioned locomotion controller (boat controller)
                if(!modern_controller) {
                    running = GAME_AXISY > 0;

                    girl_r.x -= 170;
                        quat q = eulerq(girl_r); // += custom.pivot
                        vec3 rgt = rotate3q(vec3(1,0,0), q);
                        vec3 up  = rotate3q(vec3(0,1,0), q);
                        vec3 fwd = rotate3q(vec3(0,0,1), q);
                        vec3 dir = scale3(fwd, speed * GAME_AXISY * (GAME_AXISY > 0 ? 2.0 : 0.5));
                        girl_r.x += speed * 20.0 * yaw_boost * GAME_AXISX; // yaw
                        girl_p = add3(girl_p, dir);
                    girl_r.x += 170;
                }

                // modern locomotion controller (mario 3d)
                if(modern_controller) {
                    running = GAME_AXISX != 0 || GAME_AXISY != 0;

                    camera_t *cam = camera_get_active();
                    vec3 fwd = sub3(girl_p, cam->position); fwd.y = 0; fwd = norm3(fwd);
                    vec3 rgt = norm3(cross3(fwd, vec3(0,1,0)));

                    // target
                    vec3 dir = add3(
                        scale3(fwd, GAME_AXISY),
                        scale3(rgt, GAME_AXISX)
                    ); dir.y = 0; dir = norm3(dir);

                    // smoothing
                    static vec3 olddir; do_once olddir = dir;
                    dir = mix3(dir, olddir, 1 - (yaw_boost / 4.0) * 0.85);
                    olddir = dir;

                    // vis
                    // ddraw_arrow(girl_p, add3(girl_p,scale3(dir,10)));

                    // apply direction
                    girl_p = add3(girl_p, scale3(dir, speed * 2));

                    // apply rotation
                    {
                        girl_r.x -= 170;
                        quat q = eulerq(girl_r);
                        vec3 fwdg = rotate3q(vec3(0,0,1), q);
                        girl_r.x += 170;

                        //float cosAngle = dot3(dir,fwdg);
                        //float angle = acos(cosAngle) * TO_DEG;
                        float angle = TO_DEG * ( atan2(fwdg.z, fwdg.x) - atan2(dir.z, dir.x));

                        if( !isnan(angle) ) {
                            girl_r.x -= angle;
                            while(girl_r.x> 180) girl_r.x-=360;
                            while(girl_r.x<-180) girl_r.x+=360;
                        }
                    }
                }

            // anim loops
            if( jump_delta < 1 ) { // jump/kick anim
#if 0
                girl.curframe = clampf(girl.curframe, 184, 202);
                if( girl.curframe > 202-4 && GAME_FIRE_DOWN ) girl.curframe = 184+4;
#else
                #define loopf(frame, min, max) (frame < min ? min : frame > max ? min + frame - max : frame)
                if(girl.curframe >= 203)
                girl.curframe = loopf(girl.curframe, 203, 220);
                else
                girl.curframe = clampf(girl.curframe, 184, 202);
                if( girl.curframe > 202-4 && girl.curframe < 208 && GAME_FIRE_DOWN ) girl.curframe = 203;
#endif
            }
            else if( punch_delta < 1 ) { // punch anim
                girl.curframe = clampf(girl.curframe, 90, 101);
                if( girl.curframe > 101-6 && GAME_FIRE_DOWN ) girl.curframe = 101-6;
            }
            else if( running ) {
                // loop running anim
                if( girl.curframe < 65 ) girl.curframe = 65;
                if( girl.curframe > 85 ) girl.curframe = 65;
            }
            else { // loop idle anim
                if( girl.curframe > 59 ) girl.curframe = 0;
            }
        }

        profile("Game.collisions") {
            bool punching = girl.curframe >= 90 && girl.curframe < 101;
            bool air_kicking = girl.curframe >= 184 && girl.curframe < 202;
            bool jump_kicking = girl.curframe >= 203 && girl.curframe < 220;
            bool attacking = punching || air_kicking || jump_kicking;

            if( attacking ) {
                aabb boxg = model_aabb(girl, girl_pivot);
                aabb boxw = model_aabb(witch, witch_pivot);
#if 0 // halve aabb. ok
                {
                    vec3 diag = sub3(boxg.max, boxg.min);
                    vec3 halve = scale3(diag, 0.25);
                    vec3 center = scale3(add3(boxg.min, boxg.max), 0.5);
                    boxg.min = sub3(center, halve);
                    boxg.max = add3(center, halve);
                }
#endif
                hit* h = aabb_hit_aabb(boxg, boxw);
                if( h && GAME_FIRE ) {
                    vec3 dir = norm3(sub3(witch_p, girl_p));
                    witch_p = add3(witch_p, mul3(dir,vec3(1,0,1)));
                }

                if( editor_enabled && editor_ddraw ) {
                    ddraw_color_push(h ? RED : GREEN);
                    ddraw_aabb(boxw.min, boxw.max);
                    ddraw_aabb(boxg.min, boxg.max);
                    ddraw_color_pop();
                }
            }
        }

        camera_t *cam = camera_get_active();

        profile("Game.Draw scene") {
            // draw grid/axis
            editor_obj_call0(&ground_size, fn_draw);

            // tick+draw girl
            editor_obj_call0(&girl, fn_tick);
            editor_obj_call0(&girl, fn_draw);

            // tick+draw witch
            editor_obj_call0(&witch, fn_tick);
            editor_obj_call0(&witch, fn_draw);
        }

        if(!editor_enabled) continue;

        profile("Editor.Draw outline") {

            // handle (multi-)selection
            ray *r = editor_pickup();
            if( r ) {
                bool found = false;
                bool multi_selection = input(KEY_LCTRL) || input(KEY_RCTRL);
                for each_map_ptr(editor_state, void*, o, editor_state_t, ed) {
                    void *obj = *o;
                    if( obj == &ground_size ) continue; // @fixme: add ray+plane. also, bvh

                    aabb *box = editor_obj_call0(obj, fn_aabb);
                    if( ray_hit_aabb(*r, *box)) {
                        editor_select(obj, multi_selection);
                        found = true;
                    }
                }
                if( !found )
                if( ray_hit_plane(*r, plane(vec3(0,0,0), vec3(0,1,0)) )) {
                    editor_select(&ground_size, multi_selection);
                }
            }

            if(!set_count(editor_selection)) continue;

            // draw silhouettes
            fx_begin();
            for each_set_ptr(editor_selection, void*, o) {
                void *obj = *o;

                editor_obj_call0(obj, fn_draw);
            }
            fx_end();

            // draw gizmos, aabbs, markers, etc
            for each_set_ptr(editor_selection, void*, o) {
                void *obj = *o;

                // get transform
                vec3 *p =     editor_obj_get_property_by_name(obj, "position");
                vec3 *r = p ? editor_obj_get_property_by_name(obj, "rotation") : NULL;
                vec3 *s = r ? editor_obj_get_property_by_name(obj, "scale") : NULL;

                // debugdraw
                ddraw_ontop_push(0);

                    // bounding box
                    aabb *box = editor_obj_call0(obj, fn_aabb);
                    if( box ) {
                        ddraw_color_push(YELLOW);
                        ddraw_aabb(box->min, box->max);
                        ddraw_color_pop();
                    }

                    // skeleton anim
                    // model_render_skeleton(model, pivot);

                    // position marker
                    if( p ) {
                        static map(void*, vec3) prev_dir = 0;
                        do_once map_init_ptr(prev_dir);
                        vec3* dir = map_find_or_add(prev_dir, obj, vec3(1,0,0));

                        static map(void*, vec3) prev_pos = 0;
                        do_once map_init_ptr(prev_pos);
                        vec3* found = map_find_or_add(prev_pos, obj, *p), fwd = sub3(*p, *found);
                        if( (fwd.y = 0, len3sq(fwd)) ) {
                            *found = *p;
                            *dir = norm3(fwd);
                        }

                        // float diameter = len2( sub2(vec2(box->max.x,box->max.z), vec2(box->min.x,box->min.z) ));
                        // float radius = diameter * 0.5;
                        ddraw_position_dir(*p, *dir, 1);
                    }

                ddraw_ontop(1);

                    // transform gizmo
                    if( p && r && s ) {
                        gizmo(p,r,s);
                    }

                ddraw_ontop_pop();
            }
        }
    }
}

// @todo
// editor_add_tick_before()
// editor_add_tick_after()
// editor_add_draw_before()
// editor_add_draw_after()
