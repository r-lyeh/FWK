// editing:
// nope > functions: add/rem property

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

char *editor_path(const char *path) {
    return va("%s/%s", EDITOR, path);
}

vec3 editor_pick(float mouse_x, float mouse_y) {
#if 0
    // unproject 2d coord as 3d coord
    camera_t *camera = camera_get_active();
    vec3 out, xyd = vec3(mouse_x,window_height()-mouse_y,1); // usually x:mouse_x,y:window_height()-mouse_y,d:0=znear/1=zfar
    mat44 mvp, model; identity44(model); multiply44x3(mvp, camera->proj, camera->view, model);
    bool ok = unproject44(&out, xyd, vec4(0,0,window_width(),window_height()), mvp);
    return out;
#else
    // unproject 2d coord as 3d coord
    vec2 dpi = ifdef(osx, window_dpi(), vec2(1,1));
    camera_t *camera = camera_get_active();
    float x = (2.0f * mouse_x) / (dpi.x * window_width()) - 1.0f;
    float y = 1.0f - (2.0f * mouse_y) / (dpi.y * window_height());
    float z = 1.0f;
    vec3 ray_nds = vec3(x, y, z);
    vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    mat44 inv_proj; invert44(inv_proj, camera->proj);
    mat44 inv_view; invert44(inv_view, camera->view);
    vec4 p = transform444(inv_proj, ray_clip);
    vec4 eye = vec4(p.x, p.y, -1.0, 0.0);
    vec4 wld = norm4(transform444(inv_view, eye));
    return vec3(wld.x, wld.y, wld.z);
#endif
}


typedef union engine_var {
    int i;
    float f;
    char *s;
} engine_var;
static map(char*,engine_var) engine_vars;
float *engine_getf(const char *key) {
    if(!engine_vars) map_init_str(engine_vars);
    engine_var *found = map_find_or_add(engine_vars, (char*)key, ((engine_var){0}) );
    return &found->f;
}
int *engine_geti(const char *key) {
    if(!engine_vars) map_init_str(engine_vars);
    engine_var *found = map_find_or_add(engine_vars, (char*)key, ((engine_var){0}) );
    return &found->i;
}
char **engine_gets(const char *key) {
    if(!engine_vars) map_init_str(engine_vars);
    engine_var *found = map_find_or_add(engine_vars, (char*)key, ((engine_var){0}) );
    if(!found->s) found->s = stringf("%s","");
    return &found->s;
}

int engine_send(const char *cmd, const char *optional_value) {
    unsigned *gamepads = engine_geti("gamepads"); // 0 off, mask gamepad1(1), gamepad2(2), gamepad3(4), gamepad4(8)...
    unsigned *renders = engine_geti("renders"); // 0 off, mask: 1=lit, 2=ddraw, 3=whiteboxes
    float *speed = engine_getf("speed"); // <0 num of frames to advance, 0 paused, [0..1] slomo, 1 play regular speed, >1 fast-forward (x2/x4/x8)
    unsigned *powersave = engine_geti("powersave");

    char *name;
    /**/ if( !strcmp(cmd, "key_quit" ))       record_stop(), exit(0);
    else if( !strcmp(cmd, "key_stop" ))       window_pause(1);
    else if( !strcmp(cmd, "key_mute" ))       audio_volume_master( 1 ^ !!audio_volume_master(-1) );
    else if( !strcmp(cmd, "key_pause" ))      window_pause( window_has_pause() ^ 1 );
    else if( !strcmp(cmd, "key_reload" ))     window_reload();
    else if( !strcmp(cmd, "key_battery" ))    *powersave = optional_value ? !!atoi(optional_value) : *powersave ^ 1;
    else if( !strcmp(cmd, "key_browser" ))    ui_show("File Browser", ui_visible("File Browser") ^ true);
    else if( !strcmp(cmd, "key_outliner" ))   ui_show("Outliner", ui_visible("Outliner") ^ true);
    else if( !strcmp(cmd, "key_record" ))     if(record_active()) record_stop(), ui_notify(va("Video recorded"), date_string()); else
                                              app_beep(), name = file_counter(va("%s.mp4",app_name())), window_record(name);
    else if( !strcmp(cmd, "key_screenshot" )) name = file_counter(va("%s.png",app_name())), window_screenshot(name), ui_notify(va("Screenshot: %s", name), date_string());
    else if( !strcmp(cmd, "key_profiler" ))   ui_show("Profiler", profiler_enable(ui_visible("Profiler") ^ true));
    else if( !strcmp(cmd, "key_fullscreen" )) record_stop(), window_fullscreen( window_has_fullscreen() ^ 1 ); // framebuffer resizing corrupts video stream, so stop any recording beforehand
    else if( !strcmp(cmd, "key_gamepad" ))    *gamepads = (*gamepads & ~1u) | ((*gamepads & 1) ^ 1);
    else if( !strcmp(cmd, "key_lit" ))        *renders = (*renders & ~1u) | ((*renders & 1) ^ 1);
    else if( !strcmp(cmd, "key_ddraw" ))      *renders = (*renders & ~2u) | ((*renders & 2) ^ 2);
    else alert(va("editor could not handle `%s` command.", cmd));

    return 0;
}

int engine_tick() {
    enum { engine_hz = 60 };
    enum { engine_hz_mid = 18 };
    enum { engine_hz_low = 5 };
    if( *engine_geti("powersave") ) {
        // adaptive framerate
        int app_on_background = !window_has_focus();
        int hz = app_on_background ? engine_hz_low : engine_hz_mid;
        window_fps_lock( hz < 5 ? 5 : hz );
    } else {
        // window_fps_lock( editor_hz );
    }

    return 0;
}

int ui_engine() {
    static int time_factor = 0;
    static int playing = 0;
    static int paused = 0;
    int advance_frame = 0;

#if 0
    static int do_filter = 0;
    static int do_profile = 0;
    static int do_extra = 0;

    char *EDITOR_TOOLBAR_ICONS = va("%s;%s;%s;%s;%s;%s;%s;%s",
        do_filter ? ICON_MD_CLOSE : ICON_MD_SEARCH,
        ICON_MD_PLAY_ARROW,
        paused ? ICON_MD_SKIP_NEXT : ICON_MD_PAUSE,
        ICON_MD_FAST_FORWARD,
        ICON_MD_STOP,
        ICON_MD_REPLAY,
        ICON_MD_FACE,
        ICON_MD_MENU
    );

    if( input_down(KEY_F) ) if( input(KEY_LCTRL) || input(KEY_RCTRL) ) do_filter ^= 1;
    int choice = ui_toolbar(EDITOR_TOOLBAR_ICONS);
    if( choice == 1 ) do_filter ^= 1, do_profile = 0, do_extra = 0;
    if( choice == 2 ) playing = 1, paused = 0;
    if( choice == 3 ) advance_frame = !!paused, paused = 1;
    if( choice == 4 ) paused = 0, time_factor = (++time_factor) % 4;
    if( choice == 5 ) playing = 0, paused = 0, advance_frame = 0, time_factor = 0;
    if( choice == 6 ) window_reload();
    if( choice == 7 ) do_filter = 0, do_profile ^= 1, do_extra = 0;
    if( choice == 8 ) do_filter = 0, do_profile = 0, do_extra ^= 1;

    if( do_filter ) {
        char *bak = ui_filter; ui_filter = 0;
        ui_string(ICON_MD_CLOSE " Filter " ICON_MD_SEARCH, &bak);
        ui_filter = bak;
        if( ui_label_icon_clicked_L.x > 0 && ui_label_icon_clicked_L.x <= 24 ) { // if clicked on CANCEL icon (1st icon)
            do_filter = 0;
        }
    } else {
        if( ui_filter ) ui_filter[0] = '\0';
    }
    char *filter_mask = ui_filter && ui_filter[0] ? va("*%s*", ui_filter) : "*";

    static char *username = 0;
    static char *userpass = 0;
    if( do_profile ) {
        ui_string(ICON_MD_FACE " Username", &username);
        ui_string(ICON_MD_FACE " Password", &userpass);
    }

    if( do_extra ) {
        int choice2 = ui_label2_toolbar(NULL,
            ICON_MD_VIEW_IN_AR
            ICON_MD_MESSAGE
            ICON_MD_TIPS_AND_UPDATES ICON_MD_LIGHTBULB ICON_MD_LIGHTBULB_OUTLINE
            ICON_MD_IMAGE_SEARCH ICON_MD_INSERT_PHOTO
            ICON_MD_VIDEOGAME_ASSET ICON_MD_VIDEOGAME_ASSET_OFF

            ICON_MD_VOLUME_UP ICON_MD_VOLUME_OFF // audio_volume_master(-1) > 0

            ICON_MD_TROUBLESHOOT ICON_MD_SCHEMA ICON_MD_MENU
        );
    }
#endif

    int open = 0, clicked_or_toggled = 0;


    #define EDITOR_UI_COLLAPSE(f,...) \
    for( int macro(p) = (open = ui_collapse(f,__VA_ARGS__)), macro(dummy) = (clicked_or_toggled = ui_collapse_clicked()); macro(p); ui_collapse_end(), macro(p) = 0)

    EDITOR_UI_COLLAPSE(ICON_MD_VIEW_QUILT " Windows", "Debug.Windows") {
        int choice = ui_toolbar(ICON_MD_RECYCLING "@Reset layout;" ICON_MD_SAVE_AS "@Save layout");
        if( choice == 1 ) ui_layout_all_reset("*");
        if( choice == 2 ) file_delete(WINDOWS_INI), ui_layout_all_save_disk("*");

        for each_map_ptr_sorted(ui_windows, char*, k, unsigned, v) {
            bool visible = ui_visible(*k);
            if( ui_bool( *k, &visible ) ) {
                ui_show( *k, ui_visible(*k) ^ true );
            }
        }
    }

    EDITOR_UI_COLLAPSE(ICON_MD_BUG_REPORT " Bugs 0", "Debug.Bugs") {
        // @todo. parse /bugs.ini, includes saved screenshots & videos.
        // @todo. screenshot include parseable level, position screen markers (same info as /bugs.ini)
    }


    // Art and bookmarks
    EDITOR_UI_COLLAPSE(ICON_MD_FOLDER_SPECIAL " Art", "Debug.Art") {
        bool inlined = true;
        const char *file = 0;
        if( ui_browse(&file, &inlined) ) {
            const char *sep = ifdef(win32, "\"", "'");
            app_exec(va("%s %s%s%s", ifdef(win32, "start \"\"", ifdef(osx, "open", "xdg-open")), sep, file, sep));
        }
    }
    EDITOR_UI_COLLAPSE(ICON_MD_BOOKMARK " Bookmarks", "Debug.Bookmarks") { /* @todo */ }


    // E,C,S,W
    EDITOR_UI_COLLAPSE(ICON_MD_ACCOUNT_TREE " Scene", "Debug.Scene") {
        EDITOR_UI_COLLAPSE(ICON_MD_BUBBLE_CHART/*ICON_MD_SCATTER_PLOT*/ " Entities", "Debug.Entities") { /* @todo */ }
        EDITOR_UI_COLLAPSE(ICON_MD_TUNE " Components", "Debug.Components") { /* @todo */ }
        EDITOR_UI_COLLAPSE(ICON_MD_PRECISION_MANUFACTURING " Systems", "Debug.Systems") { /* @todo */ }
        EDITOR_UI_COLLAPSE(ICON_MD_PUBLIC " Levels", "Debug.Levels") {
            //node_edit(editor.edit.down,&editor.edit);
        }

        //EDITOR_UI_COLLAPSE(ICON_MD_ACCOUNT_TREE " Init", "Debug.HierarchyInit") { /* @todo */ }
        //EDITOR_UI_COLLAPSE(ICON_MD_ACCOUNT_TREE " Draw", "Debug.HierarchyDraw") { /* @todo */ }
        //EDITOR_UI_COLLAPSE(ICON_MD_ACCOUNT_TREE " Tick", "Debug.HierarchyTick") { /* @todo */ }
        //EDITOR_UI_COLLAPSE(ICON_MD_ACCOUNT_TREE " Edit", "Debug.HierarchyEdit") { /* @todo */ }
        //EDITOR_UI_COLLAPSE(ICON_MD_ACCOUNT_TREE " Quit", "Debug.HierarchyQuit") { /* @todo */ }

        // node_edit(&editor.init,&editor.init);
        // node_edit(&editor.draw,&editor.draw);
        // node_edit(&editor.tick,&editor.tick);
        // node_edit(&editor.edit,&editor.edit);
        // node_edit(&editor.quit,&editor.quit);
    }

    EDITOR_UI_COLLAPSE(ICON_MD_ROCKET_LAUNCH " AI", "Debug.AI") {
        // @todo
    }
    EDITOR_UI_COLLAPSE(ICON_MD_VOLUME_UP " Audio", "Debug.Audio") {
        ui_audio();
    }
    EDITOR_UI_COLLAPSE(ICON_MD_VIDEOCAM " Camera", "Debug.Camera") {
        ui_camera( camera_get_active() );
    }
    EDITOR_UI_COLLAPSE(ICON_MD_MONITOR " Display", "Debug.Display") {
        // @todo: fps lock, fps target, aspect ratio, fullscreen
        char *text = va("%s;%s;%s",
            window_has_fullscreen() ? ICON_MD_FULLSCREEN_EXIT : ICON_MD_FULLSCREEN,
            ICON_MD_PHOTO_CAMERA,
            record_active() ? ICON_MD_VIDEOCAM_OFF : ICON_MD_VIDEOCAM
        );

        int choice = ui_toolbar(text);
        if( choice == 1 ) engine_send("key_fullscreen",0);
        if( choice == 2 ) engine_send("key_screenshot",0);
        if( choice == 3 ) engine_send("key_record",0);
    }
    EDITOR_UI_COLLAPSE(ICON_MD_KEYBOARD " Keyboard", "Debug.Keyboard") {
        ui_keyboard();
    }
    EDITOR_UI_COLLAPSE(ICON_MD_MOUSE " Mouse", "Debug.Mouse") {
        ui_mouse();
    }
    EDITOR_UI_COLLAPSE(ICON_MD_GAMEPAD " Gamepads", "Debug.Gamepads") {
        for( int q = 0; q < 4; ++q ) {
            for( int r = (open = ui_collapse(va("Gamepad #%d",q+1), va("Debug.Gamepads%d",q))), dummy = (clicked_or_toggled = ui_collapse_clicked()); r; ui_collapse_end(), r = 0) {
                ui_gamepad(q);
            }
        }
    }

    EDITOR_UI_COLLAPSE(ICON_MD_TEXT_FIELDS " Fonts", "Debug.Fonts") {
        ui_font();
    }


    EDITOR_UI_COLLAPSE(ICON_MD_CONTENT_PASTE " Scripts", "Debug.Scripts") {
        // @todo
    }
    EDITOR_UI_COLLAPSE(ICON_MD_STAR_HALF " Shaders", "Debug.Shaders") {
        ui_shaders();
    }
    EDITOR_UI_COLLAPSE(ICON_MD_MOVIE " FXs", "Debug.FXs") {
        ui_fxs();
    }


    EDITOR_UI_COLLAPSE(ICON_MD_SAVINGS " Budgets", "Debug.Budgets") {
        // @todo. // mem,fps,gfx,net,hdd,... also logging
    }
    EDITOR_UI_COLLAPSE(ICON_MD_WIFI/*ICON_MD_SIGNAL_CELLULAR_ALT*/ " Network 0/0 KiB", "Debug.Network") {
        // @todo
        // SIGNAL_CELLULAR_1_BAR SIGNAL_CELLULAR_2_BAR
    }
    EDITOR_UI_COLLAPSE(va(ICON_MD_SPEED " Profiler %5.2f/%dfps", window_fps(), (int)window_fps_target()), "Debug.Profiler") {
        ui_profiler();
    }
    EDITOR_UI_COLLAPSE(va(ICON_MD_STORAGE " Storage %s", xstats()), "Debug.Storage") {
        // @todo
    }



    // logic: either plug icon (power saving off) or one of the following ones (power saving on):
    //        if 0% batt (no batt): battery alert
    //        if discharging:       battery levels [alert,0..6,full]
    //        if charging:          battery charging
    int battery_read = app_battery();
    int battery_level = abs(battery_read);
    int battery_discharging = battery_read < 0 && battery_level < 100;
    const char *power_icon_label = ICON_MD_POWER " Power";
    if( battery_level ) {
        const char *battery_levels[9] = { // @todo: remap [7%..100%] -> [0..1] ?
            ICON_MD_BATTERY_ALERT,ICON_MD_BATTERY_0_BAR,ICON_MD_BATTERY_1_BAR,
            ICON_MD_BATTERY_2_BAR,ICON_MD_BATTERY_3_BAR,ICON_MD_BATTERY_4_BAR,
            ICON_MD_BATTERY_5_BAR,ICON_MD_BATTERY_6_BAR,ICON_MD_BATTERY_FULL,
        };
        power_icon_label = (const char*)va("%s Power %d%%",
            battery_discharging ? battery_levels[(int)((9-1)*clampf(battery_level/100.f,0,1))] : ICON_MD_BATTERY_CHARGING_FULL,
            battery_level);
    }

    EDITOR_UI_COLLAPSE(power_icon_label, "Debug.Power") {
        int choice = ui_toolbar( ICON_MD_POWER ";" ICON_MD_BOLT );
        if( choice == 1 ) engine_send("key_battery","0");
        if( choice == 2 ) engine_send("key_battery","1");
    }

    EDITOR_UI_COLLAPSE(ICON_MD_WATER " Reflection", "Debug.Reflect") {
        ui_reflect("*");
    }

    EDITOR_UI_COLLAPSE(ICON_MD_EXTENSION " Plugins", "Debug.Plugins") {
        // @todo. include VCS
        EDITOR_UI_COLLAPSE(ICON_MD_BUILD " Cook", "Debug.Cook") {
            // @todo
        }
    }

    return 0;
}
