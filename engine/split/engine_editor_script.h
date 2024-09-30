
int ui_texture_fit(texture_t t, struct nk_rect bounds) {
    // allocate complete window space
    struct nk_rect total_space = nk_window_get_content_region(ui_ctx);
    nk_layout_space_begin(ui_ctx, NK_DYNAMIC, total_space.h - 4, 1); // -4 to hide scrollbar Y
    nk_layout_space_push(ui_ctx, nk_rect(0,0,1,1));

    struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
    struct nk_image image = nk_image_id((int)t.id);
    nk_draw_image(canvas, bounds, &image, nk_white);

    nk_layout_space_end(ui_ctx);
    return 0;
}

#define LITE_ICON  ICON_MDI_SCRIPT_TEXT
#define LITE_TITLE "Script " LITE_ICON

EDITOR_BIND(script, "held(CTRL)&down(6)", { ui_show(LITE_TITLE, ui_visible(LITE_TITLE) ^ true); });

int editor_scripted(int window_mode) {
    window_mode = EDITOR_WINDOW; // force mode

    static lua_State *L = 0;
    do_once {
        L = script_init_env(SCRIPT_LUA|SCRIPT_DEBUGGER);

        const char *platform = "" // "Android" "FreeBSD" "OpenBSD" "NetBSD"
            ifdef(ems, "Emscripten")
            ifdef(linux, "Linux")
            ifdef(osx, "macOS")
            ifdef(win32, "Windows")
        ;
        const char *pathexe = vac("%s%s%s", app_path(), app_name(), ifdef(win32, ".exe", ""));

        gleqInit();
        gleqTrackWindow(window_handle());
        lt_init(L, window_handle(), LT_DATAPATH, __argc, __argv, window_scale(), platform, pathexe);
    }

    unsigned lt_none = 0u;
    unsigned lt_all = ~0u & ~(GLEQ_WINDOW_MOVED/*|GLEQ_WINDOW_RESIZED|GLEQ_WINDOW_REFRESH*/);
    lt_events = lt_none;

    int mouse_in_rect = 0;
    if( editor_begin(LITE_TITLE, window_mode) ) {

        lt_events = lt_all;
        if( !nk_window_has_focus(ui_ctx) ) lt_events = lt_none;

        struct nk_rect bounds = nk_window_get_content_region(ui_ctx);

        lt_mx = input(MOUSE_X) - bounds.x;
        lt_my = input(MOUSE_Y) - bounds.y;
        lt_wx = bounds.x;
        lt_wy = bounds.y;
        lt_ww = bounds.w;
        lt_wh = bounds.h;

        if( lt_resizesurface(lt_getsurface(0), lt_ww, lt_wh) ) {
            gleq_window_refresh_callback(window_handle());
        }
        // fullscreen_quad_rgb( lt_getsurface(0)->t, 1.2f );
        ui_texture_fit(lt_getsurface(0)->t, bounds);

        if( !!nk_input_is_mouse_hovering_rect(&ui_ctx->input, ((struct nk_rect){lt_wx+5,lt_wy+5,lt_ww-10,lt_wh-10})) ) {
            lt_events &= ~(1<<31); // dont cursor shape
        }

        editor_end(window_mode);
    }

    lt_tick(L);
    return 0;
}

AUTORUN {
    array_push(editor.subeditors, editor_scripted);
}
