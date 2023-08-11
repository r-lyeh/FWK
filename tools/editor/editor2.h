#define EDITOR_VERSION "2022.7"

#if 1
#define EDITOR_PRINTF PRINTF
#else
#define EDITOR_PRINTF(...) do {} while(0)
#endif

#define ICON_PLAY   ICON_MD_PLAY_ARROW
#define ICON_PAUSE  ICON_MD_PAUSE
#define ICON_STOP   ICON_MD_STOP
#define ICON_CANCEL ICON_MD_CLOSE

#define ICON_WARNING      ICON_MD_WARNING
#define ICON_BROWSER      ICON_MD_FOLDER_SPECIAL
#define ICON_OUTLINER     ICON_MD_VIEW_IN_AR
#define ICON_BUILD        ICON_MD_BUILD
#define ICON_SCREENSHOT   ICON_MD_PHOTO_CAMERA
#define ICON_CAMERA_ON    ICON_MD_VIDEOCAM
#define ICON_CAMERA_OFF   ICON_MD_VIDEOCAM_OFF
#define ICON_GAMEPAD_ON   ICON_MD_VIDEOGAME_ASSET
#define ICON_GAMEPAD_OFF  ICON_MD_VIDEOGAME_ASSET_OFF
#define ICON_AUDIO_ON     ICON_MD_VOLUME_UP
#define ICON_AUDIO_OFF    ICON_MD_VOLUME_OFF
#define ICON_WINDOWED     ICON_MD_FULLSCREEN_EXIT
#define ICON_FULLSCREEN   ICON_MD_FULLSCREEN
#define ICON_LIGHTS_ON    ICON_MD_LIGHTBULB
#define ICON_LIGHTS_OFF   ICON_MD_LIGHTBULB_OUTLINE
#define ICON_RENDER_BASIC ICON_MD_IMAGE_SEARCH
#define ICON_RENDER_FULL  ICON_MD_INSERT_PHOTO

#define ICON_SIGNAL     ICON_MD_SIGNAL_CELLULAR_ALT
#define ICON_DISK       ICON_MD_STORAGE
#define ICON_RATE       ICON_MD_SPEED

#define ICON_CLOCK      ICON_MD_TODAY
#define ICON_CHRONO     ICON_MD_TIMELAPSE

#define ICON_SETTINGS   ICON_MD_SETTINGS
#define ICON_LANGUAGE   ICON_MD_G_TRANSLATE
#define ICON_PERSONA    ICON_MD_FACE
#define ICON_SOCIAL     ICON_MD_MESSAGE
#define ICON_GAME       ICON_MD_ROCKET_LAUNCH
#define ICON_KEYBOARD   ICON_MD_KEYBOARD
#define ICON_MOUSE      ICON_MD_MOUSE
#define ICON_GAMEPAD    ICON_MD_GAMEPAD
#define ICON_MONITOR    ICON_MD_MONITOR
#define ICON_WIFI       ICON_MD_WIFI
#define ICON_BUDGET     ICON_MD_SAVINGS
#define ICON_NEW_FOLDER ICON_MD_CREATE_NEW_FOLDER
#define ICON_PLUGIN     ICON_MD_EXTENSION
#define ICON_RESTART    ICON_MD_REPLAY
#define ICON_QUIT       ICON_MD_CLOSE

#define ICON_POWER            ICON_MD_BOLT // ICON_MD_POWER
#define ICON_BATTERY_CHARGING ICON_MD_BATTERY_CHARGING_FULL
#define ICON_BATTERY_LEVELS \
        ICON_MD_BATTERY_ALERT, \
        ICON_MD_BATTERY_0_BAR,ICON_MD_BATTERY_1_BAR, \
        ICON_MD_BATTERY_2_BAR,ICON_MD_BATTERY_3_BAR, \
        ICON_MD_BATTERY_4_BAR,ICON_MD_BATTERY_5_BAR, \
        ICON_MD_BATTERY_6_BAR,ICON_MD_BATTERY_FULL

// state - retained mode

typedef int property; // @fixme

typedef struct editor_state_t {
    array(property) properties;
    array(char)     buffer;
    array(vec2i)    history;
    unsigned        cursor;
} editor_state_t;

typedef map(char*, char*) editor_dict_t;

static map(void*, editor_state_t)  editor_state; // world
static map(void*, editor_dict_t)   editor_dicts;
static set(void*)                  editor_world;
static set(void*)                  editor_selection; // objects selected in scene

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
void editor_init() {
    do_once {
        map_init_ptr(editor_state);
        map_init_ptr(editor_dicts);
        set_init_ptr(editor_world);
        set_init_ptr(editor_selection);
        profile_enable( false );
        window_pause( true );
    }
}

void editor_tick() {
    // timing
    editor_dt = window_delta() * !window_has_pause(); if(editor_dt > 1/60.f) editor_dt = 1/60.f;
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

void editor_select_all() {}
void editor_select_none() {}


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

void editor_menubar() {
    do_once editor_init();

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

    if( !editor_key && editor_selected_obj ) {
        if( input_up(MOUSE_L)    ) editor_key = key_save_mem;
        if( input_down(MOUSE_R)  ) ui_show("Properties", true);
    }

    // @fixme: send all editor keys to game?
    // if( input_repeat(KEY_ESC, 300)) {}
    // if( input_repeat(KEY_F1, 300)) {}
    // etc...

    // menubar

    if( ui_menu( ICON_SETTINGS "@Preferences;" 
                ICON_LANGUAGE " Language;" 
                ICON_PERSONA " Profile;" // editor account, but also fake profile and 1st party credentials
                ICON_SOCIAL " Social;"
                ICON_GAME " Game;" // 
                ICON_KEYBOARD " Keyboard;" 
                ICON_MOUSE " Mouse;" 
                ICON_GAMEPAD " Gamepad;" 
                ICON_MONITOR " Display;" // @todo: RENDER settings, AUDIO settings
                ICON_WIFI " Network;"
                ICON_BUDGET " Budget;" // mem,gfx,net,hdd,... also logging
                ICON_NEW_FOLDER " Folders;" // including project folders
                ICON_PLUGIN " Plugins;" // including VCS
                ICON_RESTART " Restart;"
                ICON_QUIT " Quit;"
                "-" ICON_MD_RECYCLING " Reset all preferences;" ICON_MD_SAVE_AS " Save all preferences"
    ) ) {
        if( ui_item() == 3 ) {} // key mappings
        if( ui_item() == 4 ) {} // sensitivity, invert xylrw
        if( ui_item() == 5 ) {} // sensitivity, invert xy,ab, deadzones
        if( ui_item() == 7 ) {} // name,email,icon,link,github
        if( ui_item() == 13) editor_key = key_reload;
        if( ui_item() == 14) editor_key = key_quit;
    }

    static char game_args[16] = "--game-args"; // @fixme @todo remove '_' special char to signal that ui_menu() is writeable (inputbox)
    if( ui_menu_editbox( game_args, 16 ) ) {}

    if( ui_menu( ICON_CANCEL "@Cancel" ) ) {}

    if( ui_menu( window_has_pause() ? ICON_PLAY "@Tap to Play Game" : ICON_PAUSE "@Tap to Pause Game" )) editor_key = key_pause;
    if( ui_menu( ICON_MD_SKIP_NEXT "@Next frame") ) {}
    if( ui_menu( ICON_MD_FAST_FORWARD "@Fast forward") ) {}
    //if( ui_menu( ICON_STOP "@Stop game" )) editor_key = key_stop;

    if( ui_menu( va(ICON_BUILD "@Build game"))) ui_notify("Build", ICON_WARNING " Not implemented.");
    if( ui_menu( va(ICON_MD_ROCKET_LAUNCH "@Launch game"))) ui_notify("Launch", ICON_WARNING " Not implemented.");

    // ICON_MD_TROUBLESHOOT -> PROFILER
    // ICON_MD_SCHEMA -> GRAPHNODES
    // ICON_MD_ACCOUNT_TREE -> GRAPHNODES
    // ICON_MD_TIPS_AND_UPDATES -> BULB
    // if( ui_menu( ICON_MD_MENU )) {}

//    if( ui_menu( ICON_BROWSER "@Content browser" )) editor_key = key_browser;
//    if( ui_menu( va(ICON_OUTLINER " %d/%d@World outliner", set_count(editor_selection), map_count(editor_state) ))) editor_key = key_outliner;

    if( ui_menu( ICON_SCREENSHOT "@Take Screenshot" )) editor_key = key_screenshot; // MD_SCREENSHOT_MONITOR
    if( ui_menu( record_active() ? ICON_CAMERA_OFF "@Stop video recording" : ICON_CAMERA_ON "@Start video recording" )) { if(record_active()) record_stop(); else editor_key = key_recording; }
    if( ui_menu( editor_gamepad ? ICON_GAMEPAD_ON "@Gamepad is enabled. Tap to disable." : ICON_GAMEPAD_OFF "@Gamepad is disabled. Tap to enable." )) editor_key = key_gamepad;
    if( ui_menu( audio_volume_master(-1) > 0 ? ICON_AUDIO_ON "@Audio is enabled. Tap to mute." :  ICON_AUDIO_OFF "@Audio is muted. Tap to enable." )) editor_key = key_mute;
    if( ui_menu( window_has_fullscreen() ? ICON_WINDOWED "@Fullscreen. Tap to go Windowed." : ICON_FULLSCREEN "@Windowed. Tap to go Fullscreen." )) editor_key = key_fullscreen;

    if( ui_menu( editor_ddraw ? ICON_RENDER_BASIC "@Debug renderer. Tap to go Retail Renderer." : ICON_RENDER_FULL "@Retail renderer. Tap to go Debug Renderer." )) editor_key = key_ddraw; // ICON_MD_ADD_PHOTO_ALTERNATE
    if( ui_menu( editor_lit ? ICON_LIGHTS_ON "@Lit. Tap to disable lighting." : ICON_LIGHTS_OFF "@Unlit. Tap to enable lighting." )) editor_key = key_lit;

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

    // bug report, signal status, disk status, framerate status
    if( ui_menu(va(ICON_SIGNAL " 0/0KiB" ))) {} // SIGNAL_CELLULAR_1_BAR SIGNAL_CELLULAR_2_BAR
    if( ui_menu(va(ICON_DISK " %s", xstats() ))) {} // 012/136MB
    if( ui_menu(va(ICON_RATE " %5.2f/%d", window_fps(), (int)window_fps_target()))) editor_key = key_profiler; // 012/136MB

    // bug report, profile, warnings, time/chrono (@todo: alarm/snooze? calendar?)
    if( ui_menu( ICON_MD_BUG_REPORT /*"^"*/ "0" ) ) {}
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

    for each_map_ptr(editor_state, void *, o, editor_state_t, ed) {
        profile_incstat("Editor.num_objects", +1);

        void *obj = *o;

#if 1
#elif 0
        // auto-load from disk during init. @fixme kvdb database
        if( array_count(ed->history) == 0 )
            if( editor_load_disk(obj, editor_obj_string(obj, ".path")) )
                {}

        // auto-save in-mem during first edit
        if( array_count(ed->history) == 0 )
            editor_save_mem(obj);
#endif

        // @todo: continue if obj not found in selection set
        if( obj != editor_selected_obj )
            continue;

        if( editor_key == key_debugger  ) { breakpoint("User requested breakpoint on this object"); }
#if 1
#elif 0
        if( editor_key == key_reset     ) { const char *ok = editor_reset(obj) ? "ok" : "err"; EDITOR_PRINTF("reset: %s\n", ok); }
        if( editor_key == key_save_mem  ) { const char *ok = editor_save_mem(obj) ? "ok" : "err"; EDITOR_PRINTF("mem saved: %s\n", ok); }
        if( editor_key == key_undo      ) { const char *ok = editor_undo(obj) ? "ok" : "err"; EDITOR_PRINTF("undo: %s\n",  ok); }
        if( editor_key == key_redo      ) { const char *ok = editor_redo(obj) ? "ok" : "err"; EDITOR_PRINTF("redo: %s\n",  ok); }
        if( editor_key == key_save_disk ) { const char *ok = editor_save_disk(obj, editor_obj_string(obj, ".path")) ? "ok" : "err"; EDITOR_PRINTF("save: %s\n",  ok); }
        if( editor_key == key_load_disk ) { const char *ok = editor_load_disk(obj, editor_obj_string(obj, ".path")) ? "ok" : "err"; EDITOR_PRINTF("load: %s\n",  ok); }
#endif
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

    editor_key = 0;
}
