#define BROWSER_ICON  ICON_MD_FOLDER_SPECIAL
#define BROWSER_TITLE "Browser " BROWSER_ICON

EDITOR_BIND(browser, "held(CTRL)&down(2)", { ui_show(BROWSER_TITLE, ui_visible(BROWSER_TITLE) ^ true); });

int editor_browser(int window_mode) {
    window_mode = EDITOR_WINDOW; // force window
    if( editor_begin(BROWSER_TITLE, window_mode) ) {
        const char *file = 0;
        if( ui_browse(&file, NULL) ) {
            const char *sep = ifdef(win32, "\"", "'");
            app_exec(va("%s %s%s%s", ifdef(win32, "start \"\"", ifdef(osx, "open", "xdg-open")), sep, file, sep));
        }
        editor_end(window_mode);
    }
    return 0;
}

AUTORUN {
    array_push(editor.subeditors, editor_browser);
}
