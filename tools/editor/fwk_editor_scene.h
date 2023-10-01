#define SCENE_ICON ICON_MDI_FILE_TREE
#define SCENE_TITLE "Scene " SCENE_ICON

EDITOR_PROPERTY(bookmarked,   int,    0);

EDITOR_BIND(node_new, "down(INS)",                      { editor_spawn1(); } );
EDITOR_BIND(node_del, "down(DEL)",                      { editor_destroy_selected(); } );
EDITOR_BIND(node_save, "held(CTRL)&down(S)",            { puts("@todo"); } );
EDITOR_BIND(tree_save, "held(CTRL)&down(S)&held(SHIFT)",{ puts("@todo"); } );
EDITOR_BIND(select_all, "held(CTRL) & down(A)",         { editor_select("**"); } );
EDITOR_BIND(select_none, "held(CTRL) & down(D)",        { editor_select("!**"); } );
EDITOR_BIND(select_invert, "held(CTRL) & down(I)",      { editor_select("~**"); } );
EDITOR_BIND(bookmark, "held(CTRL) & down(B)",           { editor_selected_map_t *map = editor_selected_map(); \
    int on = 0; \
    for each_map_ptr(*map,void*,o,int,selected) if(*selected) on |= !editor_bookmarked(*o); \
    for each_map_ptr(*map,void*,o,int,selected) if(*selected) editor_setbookmarked(*o, on); \
} );

enum {
    TREE_RECURSE = 1,
    TREE_SELECTION = 2,
    TREE_CHECKBOX = 4,
    TREE_INDENT = 8,
    TREE_ALL = ~0u
};

static
void editor_scene_(obj *o, unsigned flags) {
    static unsigned tabs = ~0u;
    ++tabs;

    if( o ) {
        unsigned do_tags = 1;
        unsigned do_indent    = !!(flags & TREE_INDENT);
        unsigned do_checkbox  = !!(flags & TREE_CHECKBOX);
        unsigned do_recurse = !!(flags & TREE_RECURSE);
        unsigned do_selection = !!(flags & TREE_SELECTION);

        nk_layout_row_dynamic(ui_ctx, 25, 1);

        const char *objicon = editor_iconinstance(o);
        if(!objicon) objicon = editor_iconclass(obj_type(o));
        if(!objicon) objicon = ICON_MDI_CUBE_OUTLINE;

        const char *objname = va("%s (%s)", obj_type(o), obj_name(o));

        const char *objchevron =
            !do_recurse || array_count(*obj_children(o)) <= 1 ? ICON_MDI_CIRCLE_SMALL :
            editor_open(o) ? ICON_MDI_CHEVRON_DOWN : ICON_MDI_CHEVRON_RIGHT;

        char *label = va("%*s%s%s %s", do_indent*(4+2*tabs), "", objchevron, objicon, objname);

        const char *iconsL =
            //editor_selected(o) ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK;
            editor_selected(o) ? ICON_MDI_CHECKBOX_MARKED : ICON_MDI_CHECKBOX_BLANK_OUTLINE;

        const char *iconsR = va("%s%s%s",
            editor_script(o) ? ICON_MDI_SCRIPT : ICON_MDI_CIRCLE_SMALL,
            editor_event(o) ? ICON_MDI_CALENDAR : ICON_MDI_CIRCLE_SMALL,
            editor_visible(o) ? ICON_MDI_EYE_OUTLINE : ICON_MDI_EYE_CLOSED );

        UI_TOOLBAR_OVERLAY_DECLARE(int choiceL, choiceR);

        struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
        struct nk_rect bounds; nk_layout_peek(&bounds, ui_ctx);

        int clicked = nk_hovered_text(ui_ctx, label, strlen(label), NK_TEXT_LEFT, editor_selected(o));
        if( clicked && nk_input_is_mouse_hovering_rect(&ui_ctx->input, ((struct nk_rect) { bounds.x,bounds.y,bounds.w*0.66,bounds.h })) )
            editor_altselected( o );

        vec2i treeoffset = {0};

        if( do_indent ) {
            float thickness = 2.f;
            struct nk_color color = {255,255,255,64};

            int offsx = 30;
            int spacx = 10;
            int lenx = (tabs+1)*spacx;
            int halfy = bounds.h / 2;
            int offsy = halfy + 2;

            treeoffset = vec2i(bounds.x+offsx+lenx-spacx,bounds.y+offsy);

            editor_settreeoffsety(o, treeoffset.y);

            for( obj *p = obj_parent(o); p ; p = 0 )
            nk_stroke_line(canvas, treeoffset.x-6,treeoffset.y, treeoffset.x-spacx,treeoffset.y, thickness, color),
            nk_stroke_line(canvas, treeoffset.x-spacx,treeoffset.y,treeoffset.x-spacx,editor_treeoffsety(p)+4, thickness, color);
        }

        if( ui_contextual() ) {
            API int editor_send(const char *);

            int choice = ui_label(ICON_MD_BOOKMARK_ADDED "Toggle bookmarks (CTRL+B)");
            if( choice & 1 ) editor_send("bookmark");

            ui_contextual_end(!!choice);
        }

        UI_TOOLBAR_OVERLAY(choiceL,iconsL,nk_rgba_f(1,1,1,do_checkbox*ui_alpha*0.65),NK_TEXT_LEFT);

        if( do_tags )
        UI_TOOLBAR_OVERLAY(choiceR,iconsR,nk_rgba_f(1,1,1,ui_alpha*0.65),NK_TEXT_RIGHT);

        if( choiceR == 3 ) editor_altscript( o );
        if( choiceR == 2 ) editor_altevent( o);
        if( choiceR == 1 ) editor_altvisible( o );

        if( do_recurse && editor_open(o) ) {
            for each_objchild(o,obj*,oo) {
                editor_scene_(oo,flags);
            }
        }

        if( clicked && !choiceL && !choiceR ) {
            int is_picking = input(KEY_CTRL);
            if( !is_picking ) {
                if( input(KEY_SHIFT) ) {
                    editor_selectgroup( editor_first_selected(), editor_last_selected() );
                } else {
                    editor_unselect();
                    editor_setselected(o, 1);
                }
            }
            for( obj *p = obj_parent(o); p; p = obj_parent(p) ) {
                editor_setopen(p, 1);
            }
            if( nk_input_is_mouse_hovering_rect(&ui_ctx->input, ((struct nk_rect) { bounds.x,bounds.y,treeoffset.x-bounds.x+UI_ICON_FONTSIZE/2,bounds.h })) ) {
                editor_altopen( o );
            }
        }
    }

    --tabs;
}

int editor_scene(int window_mode) {
    window_mode = 0; // force panel
    if( editor_begin(SCENE_TITLE, window_mode)) {
        // #define HELP ICON_MDI_INFORMATION_OUTLINE "@-A\n-B\n-C\n" ";"
        int choice = ui_toolbar(ICON_MDI_PLUS "@New node (CTRL+N);" ICON_MDI_DOWNLOAD "@Save node (CTRL+S);" ICON_MDI_DOWNLOAD "@Save scene (SHIFT+CTRL+S);" ICON_MD_BOOKMARK_ADDED "@Toggle Bookmark (CTRL+B);");
        if( choice == 1 ) editor_send("node_new");
        if( choice == 2 ) editor_send("node_save");
        if( choice == 3 ) editor_send("tree_save");
        if( choice == 4 ) editor_send("bookmark");

        array(obj*) bookmarks = 0;
        for each_map_ptr(*editor_bookmarked_map(), void*,o,int,bookmarked) {
            if( *bookmarked ) {
                array_push(bookmarks, *o);
            }
        }
        if( ui_collapse("!" ICON_MD_BOOKMARK "Bookmarks", "DEBUG:BOOKMARK")) {
            for each_array( bookmarks, obj*, o )
                editor_scene_( o, TREE_ALL & ~(TREE_RECURSE|TREE_INDENT|TREE_CHECKBOX) );
            ui_collapse_end();
        }
        array_free(bookmarks);

        editor_scene_( editor.root, TREE_ALL );

        for each_array( editor.objs, obj*, o )
            editor_scene_( o, TREE_ALL );

        ui_separator();

        // edit selection
        for each_map(*editor_selected_map(), void*,o, int, k) {
            if( k ) editor_inspect(o);
        }

        editor_end(window_mode);
    }

    return 0;
}

AUTORUN {
    array_push(editor.subeditors, editor_scene);
}
