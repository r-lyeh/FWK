// ## Editor long-term plan
// - editor = tree of nodes. levels and objects are nodes, and their widgets are also nodes
// - you can perform actions on nodes, with or without descendants, top-bottom or bottom-top
// - these operations include load/save, undo/redo, reset, play/render, ddraw, etc
// - nodes are saved to disk as a filesystem layout: parents are folders, and leafs are files
// - network replication can be done by external tools by comparing the filesystems and by sending the resulting diff zipped
//
// ## Editor roadmap
// - Gizmos✱, scene tree, property editor✱, load/save✱, undo/redo✱, copy/paste, on/off (vis,tick,ddraw,log), vcs.
// - Scenenode pass: node singleton display, node console, node labels, node outlines✱.<!-- node == gameobj ? -->
// - Render pass: billboards✱, materials, un/lit, cast shadows, wireframe, skybox✱/mie✱, fog/atmosphere
// - Level pass: volumes, triggers, platforms, level streaming, collide✱, physics
// - Edit pass: Procedural content, brushes, noise and CSG.
// - GUI pass: timeline and data tracks, node graphs. <!-- worthy: will be reused into materials, animgraphs and blueprints -->

// ## Alt plan
// editor is a database + window/tile manager + ui toolkit; all network driven.
// to be precise, editor is a dumb app and ...
// - does not know a thing about what it stores.
// - does not know how to render the game graphics.
// - does not know how to run the game logic.
//
// the editor will create a canvas for your game to render.
// your game will be responsible to tick the logic and render the window inside the editor.
//
// that being said, editor...
// - can store datas hierarchically.
// - can perform diffs and merges, and version the datas into repositories.
// - can be instructed to render UI on top of game and window views.
// - can download new .natvis and plugins quickly.
// - can dump whole project in a filesystem form (zip).

// - editor reflects database contents up-to-date.
// - database can be queried and modified via OSC(UDP) commands.

// editor database uses one table, and stores two kind of payload types:
// - classes: defines typename and dna. class names are prefixed by '@'
// - instances: defines typename and datas. instance names are as-is, not prefixed.
//
// every save contains 5Ws: what, who, when, where, how,
// every save can be diffed/merged.

// ----------------------------------------------------------------------------

#define EDITOR_VERSION "2023.10"

// ----------------------------------------------------------------------------

typedef struct editor_bind_t {
    const char *command;
    const char *bindings;
    void (*fn)();
} editor_bind_t;

array(editor_bind_t) editor_binds;

#define EDITOR_BIND(CMD,KEYS,...) void macro(editor_bind_##CMD##_fn_)() { __VA_ARGS__ }; AUTORUN { array_push(editor_binds, ((editor_bind_t){#CMD,KEYS,macro(editor_bind_##CMD##_fn_)}) ); }

// ----------------------------------------------------------------------------

typedef void (*editor_no_property)(void *);
array(void*) editor_persist_kv;
array(editor_no_property) editor_no_properties;

#define EDITOR_PROPERTY(property_name,T,defaults) \
typedef map(void*,T) editor_##property_name##_map_t; \
editor_##property_name##_map_t *editor_##property_name##_map() { \
    static editor_##property_name##_map_t map = 0; do_once map_init_ptr(map); \
    return &map; \
} \
T editor_##property_name(const void *obj) { \
    return *map_find_or_add(*editor_##property_name##_map(), (void*)obj, ((T) defaults)); \
} \
void editor_set##property_name(const void *obj, T value) { \
    *map_find_or_add(*editor_##property_name##_map(), (void*)obj, ((T) value)) = ((T) value); \
} \
void editor_alt##property_name(const void *obj) { \
    T* found = map_find_or_add(*editor_##property_name##_map(), (void*)obj, ((T) defaults)); \
    *found = (T)(uintptr_t)!(*found); \
} \
void editor_no##property_name(void *obj) { \
    T* found = map_find_or_add(*editor_##property_name##_map(), (void*)obj, ((T) defaults)); \
    map_erase(*editor_##property_name##_map(), (void*)obj); \
} \
AUTORUN { array_push(editor_persist_kv, #T); array_push(editor_persist_kv, editor_##property_name##_map()); array_push(editor_no_properties, editor_no##property_name); }

EDITOR_PROPERTY(open,         int,    0);
EDITOR_PROPERTY(selected,     int,    0);
EDITOR_PROPERTY(changed,      int,    0);
EDITOR_PROPERTY(visible,      int,    0);
EDITOR_PROPERTY(script,       int,    0);
EDITOR_PROPERTY(event,        int,    0);
EDITOR_PROPERTY(iconinstance, char*,  0);
EDITOR_PROPERTY(iconclass,    char*,  0);
EDITOR_PROPERTY(treeoffsety,  int,    0);
// new prop: breakpoint: request to break on any given node

void editor_destroy_properties(void *o) {
    for each_array(editor_no_properties,editor_no_property,fn) {
        fn(o);
    }
}

void editor_load_on_boot(void) {
    puts("@todo: load editor");
}
void editor_save_on_quit(void) {
    puts("@todo: save editor");
}
AUTORUN {
    editor_load_on_boot();
    (atexit)(editor_save_on_quit);
}

// ----------------------------------------------------------------------------

typedef int(*subeditor)(int mode);

struct editor_t {
    // time
    unsigned   frame;
    double     t, dt, slomo;
    // controls
    int        attached;
    int        active; // focus? does_grabinput instead?
    int        key;
    vec2       mouse; // 2d coord for ray/picking
    bool       gamepad; // mask instead? |1|2|4|8
    int        hz;
    int        hz_mid;
    int        hz_low;
    int        filter;
    bool       powersave;
    bool       lit;
    bool       ddraw;
    // event root nodes
    obj* root;
    obj*  on_init;
    obj*   on_tick;
    obj*   on_draw;
    obj*   on_edit;
    obj*  on_quit;
    // all of them
    array(obj*) objs; // @todo:set() world?
    array(char*) cmds;
    // subeditors
    array(subeditor) subeditors;
} editor = {
    .active = 1,
    .gamepad = 1,
    .hz = 60,
    .hz_mid = 18,
    .hz_low = 5,
    .lit = 1,
    .ddraw = 1,
};

int editor_begin(const char *title, int mode) {
    if( mode == 0 ) return ui_panel(title, PANEL_OPEN);
    if( mode == 1 ) return ui_window(title, 0);

    int ww = window_width();  int w = ww * 0.66;
    int hh = window_height(); int h = hh * 0.66;

    nk_flags flags = NK_WINDOW_TITLE | NK_WINDOW_BORDER |
    NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
    NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE |
    // NK_WINDOW_MAXIMIZABLE | NK_WINDOW_PINNABLE |
    0; // NK_WINDOW_SCROLL_AUTO_HIDE;
    if (nk_begin(ui_ctx, title, nk_rect( (ww-w)/2,(hh-h)/2, w,h), flags))
        return 1;

    nk_end(ui_ctx);
    return 0;
}
int editor_end(int mode) {
    if( mode == 0 ) return ui_panel_end();
    if( mode == 1 ) return ui_window_end();
    nk_end(ui_ctx);
    return 0;
}

bool editor_active() {
    return ui_hover() || ui_active() || gizmo_active() ? editor.active : 0;
}

int editor_filter() {
    if( editor.filter ) {
        if (nk_begin(ui_ctx, "Filter", nk_rect(window_width()-window_width()*0.33,32, window_width()*0.33, 40),
            NK_WINDOW_NO_SCROLLBAR)) {

            char *bak = ui_filter; ui_filter = 0;
            ui_string(ICON_MD_CLOSE " Filter " ICON_MD_SEARCH, &bak);
            ui_filter = bak;

            if( input(KEY_ESC) || ( ui_label_icon_clicked_L.x > 0 && ui_label_icon_clicked_L.x <= 24 )) {
                if( ui_filter ) ui_filter[0] = '\0';
                editor.filter = 0;
            }
        }
        nk_end(ui_ctx);
    }

    return editor.filter;
}

static
int editor_select_(void *o, const char *mask) {
    int matches = 0;
    int off = mask[0] == '!', inv = mask[0] == '~';
    int match = strmatchi(obj_type(o), mask+off+inv) || strmatchi(obj_name(o), mask+off+inv);
    if( match ) {
        editor_setselected(o, inv ? editor_selected(o) ^ 1 : !off);
        ++matches;
    }
    for each_objchild(o, obj*, oo) {
        matches += editor_select_(oo, mask);
    }
    return matches;
}
void editor_select(const char *mask) {
    for each_array( editor.objs, obj*, o )
        editor_select_(o, mask);
}
void editor_unselect() { // same than editor_select("!**");
    for each_map_ptr(*editor_selected_map(), void*,o, int, k) {
        if( *k ) *k = 0;
    }
}


static obj* active_ = 0;
static void editor_selectgroup_(obj *o, obj *first, obj *last) {
    // printf("%s (looking for %s in [%s..%s])\n", obj_name(o), active_ ? obj_name(active_) : "", obj_name(first), obj_name(last));
    if( !active_ ) if( o == first || o == last ) active_ = o == first ? last : first;
    if( active_ ) editor_setselected(o, 1);
    if( o == active_ ) active_ = 0;
    for each_objchild(o, obj*, oo) {
        editor_selectgroup_(oo, first, last);
    }
}
void editor_selectgroup(obj *first, obj *last) {
    if( last ) {
        if( !first ) first = array_count(editor.objs) ? editor.objs[0] : NULL;
        if( !first ) editor_setselected(last, 1);
        else {
            active_ = 0;
            for each_array(editor.objs,obj*,o) {
                editor_selectgroup_(o, first, last);
            }
        }
    }
}

static obj *find_any_selected_(obj *o) {
    if( editor_selected(o) ) return o;
    for each_objchild(o,obj*,oo) {
        obj *ooo = find_any_selected_(oo);
        if( ooo )
            return ooo;
    }
    return 0;
}
void* editor_first_selected() {
    for each_array(editor.objs,obj*,o) {
        obj *oo = find_any_selected_(o);
        // if( oo ) printf("1st found: %s\n", obj_name(oo));
        if( oo ) return oo;
    }
    return 0;
}

static obj *find_last_selected_(obj *o) {
    void *last = 0;
    if( editor_selected(o) ) last = o;
    for each_objchild(o,obj*,oo) {
        obj *ooo = find_last_selected_(oo);
        if( ooo )
            last = ooo;
    }
    return last;
}
void* editor_last_selected() {
    void *last = 0;
    for each_array(editor.objs,obj*,o) {
        obj *oo = find_last_selected_(o);
        // if( oo ) printf("last found: %s\n", obj_name(oo));
        if( oo ) last = oo;
    }
    return last;
}

// ----------------------------------------------------------------------------------------

void editor_watch(const void *o) {
    array_push(editor.objs, (obj*)o);
    obj_push(o); // save state
}
void* editor_spawn(const char *ini) { // deprecate?
    obj *o = obj_make(ini);
    editor_watch(o);
    return o;
}
void editor_spawn1() {
    obj *selected = editor_first_selected();
    obj *o = selected ? obj_make(obj_saveini(selected)) : obj_new(obj);
    if( selected ) obj_attach(selected, o), editor_setopen(selected, 1);
    else
    editor_watch(o);

    editor_unselect();
    editor_setselected(o, 1);
}

typedef set(obj*) set_objp_t;
static
void editor_glob_recurse(set_objp_t*list, obj *o) {
    set_find_or_add(*list, o);
    for each_objchild(o,obj*,oo) {
        editor_glob_recurse(list, oo);
    }
}
void editor_destroy_selected() {
    set_objp_t list = 0;
    set_init_ptr(list);
    for each_map_ptr(*editor_selected_map(), obj*,o, int,selected) {
        if( *selected ) { editor_glob_recurse(&list, *o); }
    }
    for each_set(list, obj*, o) {
        obj_detach(o);
    }
    for each_set(list, obj*, o) {
        // printf("deleting %p %s\n", o, obj_name(o));
        // remove from watched items
        for (int i = 0, end = array_count(editor.objs); i < end; ++i) {
            if (editor.objs[i] == o) {
                editor.objs[i] = 0;
                array_erase_slow(editor.objs, i);
                --end;
                --i;
            }
        }
        // delete properties + obj
        editor_destroy_properties(o);
        obj_free(o);
    }
    set_free(list);
}
void editor_inspect(obj *o) {
    ui_section(va("%s (%s)", obj_type(o), obj_name(o)));

    if( obj_edit[obj_typeid(o)] ) {
        obj_edit(o);
    }

    for each_objmember(o,TYPE,NAME,PTR) {
        if( !editor_changed(PTR) ) {
            obj_push(o);
        }
        ui_label_icon_highlight = editor_changed(PTR); // @hack: remove ui_label_icon_highlight hack
        char *label = va(ICON_MD_UNDO "%s", NAME);
        int changed = 0;
        /**/ if( !strcmp(TYPE,"float") ) changed = ui_float(label, PTR);
        else if( !strcmp(TYPE,"vec2") )  changed = ui_float2(label, PTR);
        else if( !strcmp(TYPE,"vec3") )  changed = ui_float3(label, PTR);
        else if( !strcmp(TYPE,"vec4") )  changed = ui_float4(label, PTR);
        else if( !strcmp(TYPE,"color") ) changed = ui_color4(label, PTR);
        else if( !strcmp(TYPE,"char*") ) changed = ui_string(label, PTR);
        else ui_label2(label, va("(%s)", TYPE)); // INFO instead of (TYPE)?
        if( changed ) {
            editor_setchanged(PTR, 1);
        }
        if( ui_label_icon_highlight )
        if( ui_label_icon_clicked_L.x >= 6 && ui_label_icon_clicked_L.x <= 26 ) { // @hack: if clicked on UNDO icon (1st icon)
            editor_setchanged(PTR, 0);
        }
        if( !editor_changed(PTR) ) {
            obj_pop(o);
        }
    }
}

// ----------------------------------------------------------------------------------------
// tty

static thread_mutex_t *console_lock;
static array(char*) editor_jobs;
int editor_send(const char *cmd) { // return job-id
    int skip = strspn(cmd, " \t\r\n");
    char *buf = STRDUP(cmd + skip);
    strswap(buf, "\r\n", "");
    int jobid;
    do_threadlock(console_lock) {
        array_push(editor_jobs, buf);
        jobid = array_count(editor_jobs) - 1;
    }
    return jobid;
}
const char* editor_recv(int jobid, double timeout_ss) {
    char *answer = 0;

    while(!answer && timeout_ss >= 0 ) {
        do_threadlock(console_lock) {
            if( editor_jobs[jobid][0] == '\0' )
                answer = editor_jobs[jobid];
        }
        timeout_ss -= 0.1;
        if( timeout_ss > 0 ) sleep_ms(100); // thread_yield()
    }

    return answer + 1;
}

EDITOR_BIND(play, "held(CTRL) & down(SPC)",                 { window_pause(0); /* if(!editor.slomo) editor.active = 0; */ editor.slomo = 1; } );
EDITOR_BIND(slomo, "",                                      { window_pause(0); editor.slomo = maxf(fmod(editor.slomo * 2, 16), 0.125); } );
EDITOR_BIND(stop, "(held(ALT)|held(SHIFT))&down(ESC)",      { window_pause(1), editor.frame = 0, editor.t = 0, editor.dt = 0, editor.slomo = 0, editor.active = 1; editor_select("**"); editor_destroy_selected(); } );
EDITOR_BIND(pause, "down(ESC)",                             { window_pause( window_has_pause() ^ 1 ); } );
EDITOR_BIND(frame, "held(CTRL) & held(RIGHT)",              { window_pause(1); editor.frame++, editor.t += (editor.dt = 1/60.f); } );
EDITOR_BIND(eject, "held(SHIFT) & down(F1)",                { editor.active ^= 1; } );
EDITOR_BIND(reload, "down(F5)",                             { window_reload(); } );
EDITOR_BIND(quit, "held(ALT) & down(F4)",                   { record_stop(), exit(0); } );
EDITOR_BIND(battery, "held(ALT) & down(B)",                 { editor.powersave ^= 1; } );
EDITOR_BIND(outliner, "held(ALT) & down(O)",                { ui_show("Outliner", ui_visible("Outliner") ^ true); } );
EDITOR_BIND(profiler, "held(ALT) & down(P)",                { ui_show("Profiler", profiler_enable(ui_visible("Profiler") ^ true)); } );
EDITOR_BIND(fullscreen, "down(F11)|(held(ALT)&down(ENTER))",{ record_stop(), window_fullscreen( window_has_fullscreen() ^ 1 ); } ); // close any recording before framebuffer resizing, which would corrupt video stream
EDITOR_BIND(mute, "held(ALT) & down(M)",                    { audio_volume_master( 1 ^ !!audio_volume_master(-1) ); } );
EDITOR_BIND(filter, "held(CTRL) & down(F)",                 { editor.filter ^= 1; } );
EDITOR_BIND(gamepad, "held(ALT) & down(G)",                 { editor.gamepad ^= 1; } );
EDITOR_BIND(lit, "held(ALT) & down(L)",                     { editor.lit ^= 1; } );
EDITOR_BIND(ddraw, "held(ALT) & down(D)",                   { editor.ddraw ^= 1; } );
EDITOR_BIND(screenshot, "held(ALT) & down(X)",              { char *name = file_counter(va("%s.png",app_name())); window_screenshot(name), ui_notify(va("Screenshot: %s", name), date_string()); } );
EDITOR_BIND(record, "held(ALT) & down(Z)",                  { if(record_active()) record_stop(), ui_notify(va("Video recorded"), date_string()); else { char *name = file_counter(va("%s.mp4",app_name())); app_beep(), window_record(name); } } );

void editor_pump() {
    for each_array(editor_binds,editor_bind_t,b) {
        if( input_eval(b.bindings) ) {
            editor_send(b.command);
        }
    }

    do_threadlock(console_lock) {
        for each_array_ptr(editor_jobs,char*,cmd) {
            if( (*cmd)[0] ) {
                int found = 0;
                for each_array(editor_binds,editor_bind_t,b) {
                    if( !strcmpi(b.command, *cmd)) {
                        b.fn();
                        found = 1;
                        break;
                    }
                }

                if( !found ) {
                    // alert(va("Editor: could not handle `%s` command.", *cmd));
                    (*cmd)[0] = '\0'; strcatf(&(*cmd), "\1%s\n", "Err\n"); (*cmd)[0] = '\0';
                }

                if( (*cmd)[0] ) {
                    (*cmd)[0] = '\0'; strcatf(&(*cmd), "\1%s\n", "Ok\n"); (*cmd)[0] = '\0';
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------------------

void editor_frame( void (*game)(unsigned, float, double) ) {
    do_once {
        //set_init_ptr(editor.world);
        //set_init_ptr(editor.selection);
        profiler_enable( false );

        window_pause( true );
        window_cursor_shape(CURSOR_SW_AUTO);

        fx_load("editorOutline.fs");
        fx_enable(0, 1);

        obj_setname(editor.root = obj_new(obj), "Signals");
        obj_setname(editor.on_init = obj_new(obj), "onInit");
        obj_setname(editor.on_tick = obj_new(obj),  "onTick");
        obj_setname(editor.on_draw = obj_new(obj),  "onDraw");
        obj_setname(editor.on_edit = obj_new(obj),  "onEdit");
        obj_setname(editor.on_quit = obj_new(obj), "onQuit");

        obj_attach(editor.root, editor.on_init);
        obj_attach(editor.root, editor.on_tick);
        obj_attach(editor.root, editor.on_draw);
        obj_attach(editor.root, editor.on_edit);
        obj_attach(editor.root, editor.on_quit);

        editor_seticoninstance(editor.root, ICON_MDI_SIGNAL_VARIANT);
        editor_seticoninstance(editor.on_init, ICON_MDI_SIGNAL_VARIANT);
        editor_seticoninstance(editor.on_tick, ICON_MDI_SIGNAL_VARIANT);
        editor_seticoninstance(editor.on_draw, ICON_MDI_SIGNAL_VARIANT);
        editor_seticoninstance(editor.on_edit, ICON_MDI_SIGNAL_VARIANT);
        editor_seticoninstance(editor.on_quit, ICON_MDI_SIGNAL_VARIANT);

        editor_seticonclass(obj_type(editor.root), ICON_MDI_CUBE_OUTLINE);
    }

    // game tick
    game(editor.frame, editor.dt, editor.t);

    // timing
    editor.dt = clampf(window_delta(), 0, 1/60.f) * !window_has_pause() * editor.slomo;
    editor.t += editor.dt;
    editor.frame += !window_has_pause();
    editor.frame += !editor.frame;

    // process inputs & messages
    editor_pump();

    // draw menubar
    static int stats_mode = 1;
    static double last_fps = 0; if(!window_has_pause()) last_fps = window_fps();
    const char *STATS = va("x%4.3f %03d.%03dss %02dF %s",
        editor.slomo,
        (int)editor.t, (int)(1000 * (editor.t - (int)editor.t)),
        (editor.frame-1) % ((int)window_fps_target() + !(int)window_fps_target()),
        stats_mode == 1 ? va("%5.2f/%dfps", last_fps, (int)window_fps_target()) : stats_mode == 0 ? "0/0 KiB" : xstats());
    const char *ICON_PL4Y = window_has_pause() ? ICON_MDI_PLAY : ICON_MDI_PAUSE;
    const char *ICON_SKIP = window_has_pause() ? ICON_MDI_STEP_FORWARD/*ICON_MDI_SKIP_NEXT*/ : ICON_MDI_FAST_FORWARD;

    UI_MENU(12, \
        UI_MENU_POPUP(ICON_MD_SETTINGS, vec2(0.33,1.00), ui_debug()) \
        UI_MENU_ITEM(ICON_PL4Y, editor_send(window_has_pause() ? "play" : "pause")) \
        UI_MENU_ITEM(ICON_SKIP, editor_send(window_has_pause() ? "frame" : "slomo")) \
        UI_MENU_ITEM(ICON_MDI_STOP, editor_send("stop")) \
        UI_MENU_ITEM(ICON_MDI_EJECT, editor_send("eject")) \
        UI_MENU_ITEM(STATS, stats_mode = (++stats_mode) % 3) \
        UI_MENU_ALIGN_RIGHT(32+32+32+32+34) \
        UI_MENU_ITEM(ICON_MDI_CHART_TIMELINE, editor_send("timeline")) \
        UI_MENU_ITEM(ICON_MDI_CONSOLE, editor_send("console")) \
        UI_MENU_ITEM(ICON_MD_FOLDER_SPECIAL, editor_send("browser")) \
        UI_MENU_ITEM(ICON_MD_SEARCH, editor_send("filter")) \
        UI_MENU_ITEM(ICON_MD_CLOSE, editor_send("quit")) \
    );

    if(! editor.active ) return;

    // draw silhouettes
    sprite_flush();
    for each_map_ptr(*editor_selected_map(),void*,o,int,selected) {
        if(*selected && obj_draw[obj_typeid(*o)]) {
            fx_begin();
            obj_draw(*o);
            sprite_flush();
            fx_end();
        }
    }

    // draw subeditors
    static int window_mode = 1; // panel(0), window(1), nk_window(2)
    static int transparent_mode = 0;
    static struct nk_color bak, *on = 0; do_once bak = ui_ctx->style.window.fixed_background.data.color; // ui_ctx->style.window.fixed_background.data.color = !!(on = (on ? NULL : &bak)) ? AS_NKCOLOR(0) : bak;  };
    if( transparent_mode ^= !!input_down(KEY_CAPS) ) ui_ctx->style.window.fixed_background.data.color = AS_NKCOLOR(0);
    for each_array(editor.subeditors, subeditor, fn) {
        fn(window_mode);
    }
    ui_ctx->style.window.fixed_background.data.color = bak;

    // draw ui filter (note: render at end-of-frame, so it's hopefully on-top)
    editor_filter();
}

#include "fwk_editor_timeline.h"
#include "fwk_editor_scene.h"
#include "fwk_editor_browser.h"
#include "fwk_editor_console.h"
