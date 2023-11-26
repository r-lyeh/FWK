#if 0
    key_screenshot, // @todo: add meta-info in exif or invisibile pixels (cam details, player details, map level, map location, level state, etc)

    if( !editor_key && editor_selected_obj ) {
        if( input_up(MOUSE_L)    ) editor_key = key_save_mem;
        if( input_down(MOUSE_R)  ) ui_show("Properties", true);
    }

    // @fixme: send all editor keys to game?
    // if( input_repeat(KEY_ESC, 300)) {}
    // if( input_repeat(KEY_F1, 300)) {}
    // etc...

    // menubar

    if( ui_menu(
                ICON_LANGUAGE " Language;" 
                ICON_PERSONA " Profile;" // editor account, but also fake profile and 1st party credentials
                ICON_SOCIAL " Social;" // name,email,icon,link,github
                ICON_GAME " Game;" // 
                ICON_WIFI " Network;"
                ICON_BUDGET " Budget;" // mem,gfx,net,hdd,... also logging
                ICON_NEW_FOLDER " Folders;" // including project folders
                ICON_RESTART " Restart;"
    ) ) {
        if( ui_item() == 3 ) {} // keyboard: key mappings
        if( ui_item() == 4 ) {} // mouse: sensitivity, invert xylrw
        if( ui_item() == 5 ) {} // gamepad: sensitivity, invert xy,ab, deadzones
    }

    static char game_args[16] = "--game-args"; // @fixme @todo remove '_' special char to signal that ui_menu() is writeable (inputbox)
    if( ui_menu_editbox( game_args, 16 ) ) {}

    // ICON_MD_TROUBLESHOOT -> PROFILER
    // ICON_MD_TIPS_AND_UPDATES -> BULB
    // if( ui_menu( ICON_MD_MENU )) {}

    // logic: either plug icon (power saving off) or one of the following ones (power saving on):
    //        if 0% batt (no batt): battery alert
    //        if discharging:       battery levels [alert,0..6,full]
    //        if charging:          battery charging
    int battery_read = app_battery();
    int battery_level = abs(battery_read);
    int battery_discharging = battery_read < 0 && battery_level < 100;
    const char *battery_levels[] = { // @todo: remap [7%..100%] -> [0..1] ?
        ICON_BATTERY_LEVELS
    };
    if( ui_menu( !editor_power_saving ? ICON_POWER"@Full power. Tap to save power." :
        va("%s@Power saving. Tap to go full power. %3d%% battery.",
            battery_read == 0 ? battery_levels[0] :
            battery_discharging ? battery_levels[(int)((countof(battery_levels)-1)*clampf(battery_level/100.f,0,1))] :
              ICON_BATTERY_CHARGING, battery_level) ))
        editor_key = key_battery;

    // @todo: combine-in-1? cycle mem -> cpu/profiler -> network mon -> debugger

    // bug report, profile, warnings, time/chrono (@todo: alarm/snooze? calendar?)
    if( ui_menu( ICON_MD_FACE /*"^"*/ "3" ) ) {} // @todo: do both messaging/warnings + profile settings here
    {
        static double base = 0, tap = 0;

        if( tap == 0 ) tap = time_ss();
        double delta = time_ss() - tap;
        tap = time_ss();
        base += delta * !window_has_pause();

        if( ui_menu( base == 0 ?
            va(ICON_CLOCK "%02d:%02d", (int)((date() / 10000) % 100), (int)((date() / 100) % 100))
            :
            va(ICON_CHRONO "%03dm:%02ds:%02df@Tap to reset chrono.",((int)(base/60))%60,((int)base)%60,(int)((base - (int)base)*window_fps_target())))
            || editor_key == key_stop
        ) {
            base = 0, tap = 0;
        }
    }

#endif

#if 1
#elif 0
// ## Roadmaps
// ### v2 roadmap (mid-term)
// - [ ] art/ vs prefabs/ discrimination: prefabs/ are archetypes (composed types); ie, data-classes. art/ contains data files.
// - [ ]   can prefabs be done with ecs maybe?
// - [ ]   example: levels are prefabs, composed of other sub-prefabs or art assets.
// - [ ]   example: hitboxes+events. girl=pivot(p,r,s)+model(mesh,tex)+curframe
// - [ ]   extend widgets vec3 as range;customized mesh,texture,audio,any other asset,combo of anything)
//
// ### organization: world as a filesystem
// - [ ] anything can be serialized into disk. any object, any entity, any property or any widget can be serialized into disk.
// - [ ] groups of them as well. the whole world state can be serialized into disk as a filesystem snapshot:
// - [ ] - objects are folders. you can attach nodes on nodes (ie, create folders inside folders).
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

// plugins
// vcs
// google/midjourney placeholders

// prefabs
// - [ ] Level objects: ~~volumes, triggers, platforms, streaming~~.
// -     level: emitters: particles, lights, lightmaps, sound sources, triggers, etc
// -     level: box triggers, start/end, spawn, streaming, checkpoints,
// -     level: jump, shoots, platforms, collisions
// -     level: 60s, 70s, 80s, 90s

// ## old notes below
// ==================
// -     gizmo: proportional, orbit/arcball XY (+shift for Z/tilt)
// -     vcs
// - [ ] Core: wecs+replication
// -     modules: script or dll + ram load/save/diff/patch + play/stop/init/ + attach/detach
// -     logic tree/ << [] |> || >>
// -     - scene |>
// -        - enemies
// -     ecs: sys are modules, ecs: char *messaging, ecs: filesystem (e/dir,c/files,s/dll)
// -     world: streaming, migration

//    key_screenshot, // @todo: add meta-info in exif or invisibile pixels (cam details, player details, map level, map location, level state, etc)

// fn_init,
//  fn_load,
//   fn_tick,
//   fn_draw,
//   fn_aabb,  // hitboxes
//   fn_edit, // call for debug ui (like loggers and sliders)
//  fn_save,
// fn_quit,

if( !editor_key /*&& !input_anykey()*/ && editor_selected_obj ) {
    if( input_up(MOUSE_L)    ) editor_key = key_save_mem;
    if( input_down(MOUSE_R)  ) ui_contextual("Properties", true);
}

void editor_render_windows() {
    // Scene/nodes
    if( ui_window("Outliner", 0) ) {

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

        //
        for( int c = ui_collapse(va(ICON_MD_FACTORY " Prefabs/ (%d)", map_count(editor_state)), "PRF"); c; ui_collapse_end(), c = 0)

        // others
        for( int c = ui_collapse(ICON_MD_PRECISION_MANUFACTURING " Editors/", "EDT"); c; ui_collapse_end(), c = 0) {

        // dynamic/static bounds: depth + bounds + visibility
        do_context_cmd = 0;
        do_context_obj = 0;
        for( int c = ui_collapse(va(ICON_MD_ACCOUNT_TREE " Levels/ (%d)", map_count(editor_children)), "LVL"); c; ui_collapse_end(), c = 0)
        for each_map_ptr(editor_children, void*, o, array(void*), objs) {
            void *k = *o;
            editor_obj_render_properties_recursively(k, mask);
        }
        if( do_context_cmd == cc4(l,i,s,t) && do_context_obj ) {
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
    camera_moveby(&cam, move);
    camera_fps(&cam, view.x,view.y);
}

// my game

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


camera_t *cam = camera_get_active();

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

#endif
