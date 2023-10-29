#define CONSOLE_ICON  ICON_MDI_CONSOLE
#define CONSOLE_TITLE "Console " CONSOLE_ICON

EDITOR_BIND(console, "held(CTRL)&down(4)", { ui_show(CONSOLE_TITLE, ui_visible(CONSOLE_TITLE) ^ true); });

int editor_console(int window_mode) {
    if( editor_begin(CONSOLE_TITLE, window_mode) ) {

        // peek complete window space
        struct nk_rect bounds = nk_window_get_content_region(ui_ctx);

        enum { CONSOLE_LINE_HEIGHT = 20 };
        static array(char*) lines = 0;
        do_once {
            array_push(lines, stringf("> Editor v%s. Type `%s` for help.", EDITOR_VERSION, ""));
        }
        int max_lines = (bounds.h - UI_ROW_HEIGHT) / (CONSOLE_LINE_HEIGHT * 2);
        if( max_lines >= 1 ) {
            nk_layout_row_static(ui_ctx, bounds.h - UI_ROW_HEIGHT, bounds.w, 1);
            if(nk_group_begin(ui_ctx, "console.group", NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER)) {
                nk_layout_row_static(ui_ctx, CONSOLE_LINE_HEIGHT, bounds.w, 1);
                for( int i = array_count(lines); i < max_lines; ++i )
                    array_push_front(lines, 0);
                for( int i = array_count(lines) - max_lines; i < array_count(lines); ++i ) {
                    if( !lines[i] ) {
                        #if 0 // debug
                        nk_label_wrap(ui_ctx, va("%d.A/%d",i+1,max_lines));
                        nk_label_wrap(ui_ctx, va("%d.B/%d",i+1,max_lines));
                        #else
                        nk_label_wrap(ui_ctx, "");
                        nk_label_wrap(ui_ctx, "");
                        #endif
                    } else {
                        nk_label_wrap(ui_ctx, lines[i]);
                        const char *answer = isdigit(*lines[i]) ? editor_recv( atoi(lines[i]), 0 ) : NULL;
                        nk_label_wrap(ui_ctx, answer ? answer : "");
                    }
                }
                nk_group_end(ui_ctx);
            }
        }
        static char *cmd = 0;
        if( ui_string(NULL, &cmd) ) {
            int jobid = editor_send(cmd);
            array_push(lines, stringf("%d> %s", jobid, cmd));
            cmd[0] = 0;
        }

        editor_end(window_mode);
    }
    return 0;
}

AUTORUN {
    array_push(editor.subeditors, editor_console);
}
