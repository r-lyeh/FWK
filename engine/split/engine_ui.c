// ----------------------------------------------------------------------------------------
// ui extensions first

static float
nk_text_width(struct nk_context *ctx, const char *str, unsigned len) {
    const struct nk_style *style = &ctx->style;
    const struct nk_user_font *f = style->font;
    float pixels_width = f->width(f->userdata, f->height, str, len ? (int)len : (int)strlen(str));
    return pixels_width + 10; // 10 -> internal widget padding
}

static nk_bool
nk_hovered_text(struct nk_context *ctx, const char *str, int len,
    nk_flags align, nk_bool value)
{
    struct nk_window *win;
    struct nk_panel *layout;
    const struct nk_input *in;
    const struct nk_style *style;

    enum nk_widget_layout_states state;
    struct nk_rect bounds;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return 0;

    win = ctx->current;
    layout = win->layout;
    style = &ctx->style;

    state = nk_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NK_WIDGET_ROM || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;

    #if 1 //< @r-lyeh: sim button logic
    struct nk_rect touch;
    touch.x = bounds.x - style->selectable.touch_padding.x;
    touch.y = bounds.y - style->selectable.touch_padding.y;
    touch.w = bounds.w + style->selectable.touch_padding.x * 2;
    touch.h = bounds.h + style->selectable.touch_padding.y * 2;
    int clicked = !!nk_button_behavior(&ctx->last_widget_state, touch, in, NK_BUTTON_DEFAULT);
    in = 0; //< @r-lyeh: do not pass any input
    #endif

    nk_do_selectable(&ctx->last_widget_state, &win->buffer, bounds,
                str, len, align, &value, &style->selectable, in, style->font);

    return clicked; //< @r-lyeh: return sim button logic instead of prev function call
}

#define ui_push_hspace(px) \
    (int xx = px; xx; xx = 0) \
    for(struct nk_context *ctx = (struct nk_context*)ui_handle(); ctx; ctx = 0 ) \
        for(struct nk_panel *layout = ui_ctx->current->layout; layout; ) \
            for( xx = (layout->at_x += px, layout->bounds.w -= px, 0); layout; layout->at_x -= px, layout->bounds.w += px, layout = 0 )

// helper macros to instance an overlayed toolbar within the regions of an existing widget
#define UI_TOOLBAR_OVERLAY_DECLARE(...) \
            __VA_ARGS__; \
            struct nk_rect toolbar_bounds; nk_layout_peek(&toolbar_bounds, ui_ctx); \
            struct nk_vec2 item_padding = ui_ctx->style.text.padding; \
            struct nk_text text; \
            text.padding.x = item_padding.x; \
            text.padding.y = item_padding.y; \
            text.background = ui_ctx->style.window.background;
#define UI_TOOLBAR_OVERLAY(CHOICE,TEXT,COLOR,ALIGNMENT) \
            do { \
            text.text = COLOR; \
            nk_widget_text(&ui_ctx->current->buffer, toolbar_bounds, TEXT, strlen(TEXT), &text, ALIGNMENT, ui_ctx->style.font); \
            int clicked_x = input_down(MOUSE_L) && nk_input_is_mouse_hovering_rect(&ui_ctx->input, toolbar_bounds); \
            if( clicked_x ) clicked_x = (int)((ui_ctx->input.mouse.pos.x - toolbar_bounds.x) - (ALIGNMENT == NK_TEXT_RIGHT ? bounds.w : 0) ); \
            CHOICE = 1 + (ALIGNMENT == NK_TEXT_RIGHT ? -1 : +1) * clicked_x / (UI_ICON_FONTSIZE + UI_ICON_SPACING_X); /* divided by px per ICON_MD_ glyph approximately */ \
            int glyphs = strlen(TEXT) / 4 /*3:MD,4:MDI*/; CHOICE *= !!clicked_x * (CHOICE <= glyphs); } while(0)

// menu macros that work not only standalone but also contained within a panel or window
static int ui_using_v2_menubar = 0;
#define UI_MENU(N, ...) do { \
    enum { MENUROW_HEIGHT = 25 }; \
    int embedded = !!ui_ctx->current; \
    struct nk_rect total_space = {0,0,window_width(),window_height()}; \
    if( embedded ) total_space = nk_window_get_bounds(ui_ctx), total_space.w -= 10; \
    int created = !embedded && nk_begin(ui_ctx, "MENU_" STRINGIZE(__COUNTER__), nk_rect(0, 0, window_width(), UI_MENUROW_HEIGHT), NK_WINDOW_NO_SCROLLBAR); \
    if ( embedded || created ) { \
        ui_using_v2_menubar = 1; \
        int align = NK_TEXT_LEFT, Nth = (N), ITEM_WIDTH = 30, span = 0; \
        nk_menubar_begin(ui_ctx); \
        nk_layout_row_begin(ui_ctx, NK_STATIC, MENUROW_HEIGHT, Nth); \
            __VA_ARGS__; \
        nk_menubar_end(ui_ctx); \
        if( created ) nk_end(ui_ctx); \
    } } while(0)
#define UI_MENU_POPUP(title, px, ...) { \
    int hspace = maxi(ITEM_WIDTH, nk_text_width(ui_ctx,(title),0)); \
    nk_layout_row_push(ui_ctx, hspace); span += hspace; \
    if (nk_menu_begin_label(ui_ctx, (title), align, nk_vec2(px.x>1?px.x:px.x*total_space.w,px.y>1?px.y:px.y*total_space.h))) { \
        __VA_ARGS__; \
        nk_menu_end(ui_ctx); \
    }}
#define UI_MENU_ITEM(title, ...) { \
    int hspace = maxi(ITEM_WIDTH, nk_text_width(ui_ctx,(title),0)); \
    nk_layout_row_push(ui_ctx, hspace); span += hspace; \
    if (nk_menu_begin_label(ui_ctx, (title), align, nk_vec2(1,1))) { \
        __VA_ARGS__; \
        nk_menu_close(ui_ctx); \
        nk_menu_end(ui_ctx); \
    }}
#define UI_MENU_ALIGN_RIGHT(px, ...) { \
    int hspace = total_space.w - span - (px) - 1.5 * ITEM_WIDTH; \
    nk_layout_row_push(ui_ctx, hspace); span += hspace; \
    if (nk_menu_begin_label(ui_ctx, (title), align = NK_TEXT_RIGHT, nk_vec2(1,1))) { \
        __VA_ARGS__; \
        nk_menu_close(ui_ctx); \
        nk_menu_end(ui_ctx); \
    }}

// ----------------------------------------------------------------------------------------
// ui

#ifndef UI_ICONS_SMALL
//#define UI_ICONS_SMALL 1
#endif

#define UI_FONT_ENUM(carlito,b612) b612 // carlito

#define UI_FONT_REGULAR    UI_FONT_ENUM("Carlito",     "B612")     "-Regular.ttf"
#define UI_FONT_HEADING    UI_FONT_ENUM("Carlito",     "B612")     "-BoldItalic.ttf"
#define UI_FONT_TERMINAL   UI_FONT_ENUM("Inconsolata", "B612Mono") "-Regular.ttf"

#if UI_LESSER_SPACING
    enum { UI_SEPARATOR_HEIGHT = 5, UI_MENUBAR_ICON_HEIGHT = 20, UI_ROW_HEIGHT = 22, UI_MENUROW_HEIGHT = 32 };
#else
    enum { UI_SEPARATOR_HEIGHT = 10, UI_MENUBAR_ICON_HEIGHT = 25, UI_ROW_HEIGHT = 32, UI_MENUROW_HEIGHT = 32 };
#endif

#if UI_FONT_LARGE
    #define UI_FONT_REGULAR_SIZE    UI_FONT_ENUM(18,17)
    #define UI_FONT_HEADING_SIZE    UI_FONT_ENUM(20,19)
    #define UI_FONT_TERMINAL_SIZE   UI_FONT_ENUM(14,14)
#elif UI_FONT_SMALL
    #define UI_FONT_REGULAR_SIZE    UI_FONT_ENUM(13,14)
    #define UI_FONT_HEADING_SIZE    UI_FONT_ENUM(14.5,15)
    #define UI_FONT_TERMINAL_SIZE   UI_FONT_ENUM(14,14)
#else
    #define UI_FONT_REGULAR_SIZE    UI_FONT_ENUM(14.5,16)
    #define UI_FONT_HEADING_SIZE    UI_FONT_ENUM(16,17.5)
    #define UI_FONT_TERMINAL_SIZE   UI_FONT_ENUM(14,14)
#endif

    #define UI_FONT_REGULAR_SAMPLING  UI_FONT_ENUM(vec3(1,1,1),vec3(1,1,1))
    #define UI_FONT_HEADING_SAMPLING  UI_FONT_ENUM(vec3(1,1,1),vec3(1,1,1))
    #define UI_FONT_TERMINAL_SAMPLING UI_FONT_ENUM(vec3(1,1,1),vec3(1,1,1))

#if UI_ICONS_SMALL
    #define UI_ICON_FONTSIZE        UI_FONT_ENUM(16.5f,16.5f)
    #define UI_ICON_SPACING_X       UI_FONT_ENUM(-2,-2)
    #define UI_ICON_SPACING_Y       UI_FONT_ENUM(4.5f,3.5f)
#else
    #define UI_ICON_FONTSIZE        UI_FONT_ENUM(20,20)
    #define UI_ICON_SPACING_X       UI_FONT_ENUM(0,0)
    #define UI_ICON_SPACING_Y       UI_FONT_ENUM(6.5f,5.0f)
#endif

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

static struct nk_context *ui_ctx;
static struct nk_glfw nk_glfw = {0};

void* ui_handle() {
    return ui_ctx;
}

static void nk_config_custom_fonts() {
    #define UI_ICON_MIN ICON_MD_MIN
    #define UI_ICON_MED ICON_MD_MAX_16
    #define UI_ICON_MAX ICON_MD_MAX

    #define ICON_BARS        ICON_MD_MENU
    #define ICON_FILE        ICON_MD_INSERT_DRIVE_FILE
    #define ICON_TRASH       ICON_MD_DELETE

    struct nk_font *font = NULL;
    struct nk_font_atlas *atlas = NULL;
    nk_glfw3_font_stash_begin(&nk_glfw, &atlas); // nk_sdl_font_stash_begin(&atlas);

        // Default font(#1)...
        int datalen = 0;
        for( char *data = vfs_load(UI_FONT_REGULAR, &datalen); data; data = 0 ) {
            float font_size = UI_FONT_REGULAR_SIZE;
                struct nk_font_config cfg = nk_font_config(font_size);
                cfg.oversample_h = UI_FONT_REGULAR_SAMPLING.x;
                cfg.oversample_v = UI_FONT_REGULAR_SAMPLING.y;
                cfg.pixel_snap   = UI_FONT_REGULAR_SAMPLING.z;
                #if UI_LESSER_SPACING
                cfg.spacing.x -= 1.0;
                #endif
            // win32: struct nk_font *arial = nk_font_atlas_add_from_file(atlas, va("%s/fonts/arial.ttf",getenv("windir")), font_size, &cfg); font = arial ? arial : font;
            // struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/DroidSans.ttf", font_size, &cfg); font = droid ? droid : font;
            struct nk_font *regular = nk_font_atlas_add_from_memory(atlas, data, datalen, font_size, &cfg); font = regular ? regular : font;
        }

        // ...with icons embedded on it.
        static struct icon_font {
            const char *file; int yspacing; vec3 sampling; nk_rune range[3];
        } icons[] = {
            {"MaterialIconsSharp-Regular.otf", UI_ICON_SPACING_Y, {1,1,1}, {UI_ICON_MIN, UI_ICON_MED /*MAX*/, 0}}, // "MaterialIconsOutlined-Regular.otf" "MaterialIcons-Regular.ttf"
            {"materialdesignicons-webfont.ttf", 2, {1,1,1}, {0xF68C /*ICON_MDI_MIN*/, 0xF1CC7/*ICON_MDI_MAX*/, 0}},
        };
        for( int f = 0; f < countof(icons); ++f )
        for( char *data = vfs_load(icons[f].file, &datalen); data; data = 0 ) {
            struct nk_font_config cfg = nk_font_config(UI_ICON_FONTSIZE);
            cfg.range = icons[f].range; // nk_font_default_glyph_ranges();
            cfg.merge_mode = 1;

            cfg.spacing.x += UI_ICON_SPACING_X;
            cfg.spacing.y += icons[f].yspacing;
         // cfg.font->ascent += ICON_ASCENT;
         // cfg.font->height += ICON_HEIGHT;

            cfg.oversample_h = icons[f].sampling.x;
            cfg.oversample_v = icons[f].sampling.y;
            cfg.pixel_snap   = icons[f].sampling.z;

            #if UI_LESSER_SPACING
            cfg.spacing.x -= 1.0;
            #endif

            struct nk_font *icons = nk_font_atlas_add_from_memory(atlas, data, datalen, UI_ICON_FONTSIZE, &cfg);
        }

        // Monospaced font. Used in terminals or consoles.

        for( char *data = vfs_load(UI_FONT_TERMINAL, &datalen); data; data = 0 ) {
            const float font_size = UI_FONT_TERMINAL_SIZE;
            static const nk_rune icon_range[] = {32, 127, 0};

            struct nk_font_config cfg = nk_font_config(font_size);
            cfg.range = icon_range;

            cfg.oversample_h = UI_FONT_TERMINAL_SAMPLING.x;
            cfg.oversample_v = UI_FONT_TERMINAL_SAMPLING.y;
            cfg.pixel_snap   = UI_FONT_TERMINAL_SAMPLING.z;

            #if UI_LESSER_SPACING
            cfg.spacing.x -= 1.0;
            #endif

            // struct nk_font *proggy = nk_font_atlas_add_default(atlas, font_size, &cfg);
            struct nk_font *bold = nk_font_atlas_add_from_memory(atlas, data, datalen, font_size, &cfg);
        }

        // Extra optional fonts from here...

        for( char *data = vfs_load(UI_FONT_HEADING, &datalen); data; data = 0 ) {
            struct nk_font_config cfg = nk_font_config(UI_FONT_HEADING_SIZE);
            cfg.oversample_h = UI_FONT_HEADING_SAMPLING.x;
            cfg.oversample_v = UI_FONT_HEADING_SAMPLING.y;
            cfg.pixel_snap   = UI_FONT_HEADING_SAMPLING.z;

            #if UI_LESSER_SPACING
            cfg.spacing.x -= 1.0;
            #endif

            struct nk_font *bold = nk_font_atlas_add_from_memory(atlas, data, datalen, UI_FONT_HEADING_SIZE, &cfg);
            // font = bold ? bold : font;
        }

    nk_glfw3_font_stash_end(&nk_glfw); // nk_sdl_font_stash_end();
//  ASSERT(font);
    if(font) nk_style_set_font(ui_ctx, &font->handle);

    // Load Cursor: if you uncomment cursor loading please hide the cursor
    // nk_style_load_all_cursors(ctx, atlas->cursors); glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

static void nk_config_custom_theme() {
    #ifdef UI_HUE
    float default_hue = UI_HUE;
    #else
    // 0.09 orange, 0.14 yellow, 0.40 green, 0.45 turquoise, 0.50 cyan, 0.52 default, 0.55 blue, 0.80 purple, 0.92 cherry, 0.96 red
    float hues[] = { 0.40,0.45,0.50,0.52,0.55,0.80,0.92 };
    float default_hue = hues[ (int)((((date() / 10000) % 100) / 24.f) * countof(hues)) ]; // YYYYMMDDhhmmss -> hh as 0..1
default_hue = 0.52;
    #endif
    float hue = clampf( optionf("--ui-hue", default_hue /*= 0.52*/), 0, 1 );
    struct nk_color main_hue   = nk_hsv_f(hue+0.025, 0.80, 0.400); // washed
    struct nk_color hover_hue  = nk_hsv_f(hue+0.025, 1.00, 0.600); // vivid
    struct nk_color active_hue = nk_hsv_f(hue-0.010, 1.00, 0.600); // bright; same /S/V than vivid, but H/ slighty biased towards a different luma on spectrum
    struct nk_color main       = nk_hsv_f(    0.600, 0.00, 0.125); // washed b/w
    struct nk_color hover      = nk_hsv_f(    0.900, 0.00, 0.000); // vivid  b/w
    struct nk_color active     = nk_hsv_f(    0.600, 0.00, 0.150); // bright b/w
    struct nk_color table[NK_COLOR_COUNT] = {0};
    table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(42, 42, 42, 245);
    table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 245);
    table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
    table[NK_COLOR_BUTTON] = main;
    table[NK_COLOR_BUTTON_HOVER] = hover;
    table[NK_COLOR_BUTTON_ACTIVE] = active;
    // ok
    table[NK_COLOR_TOGGLE] = nk_rgba(45*1.2, 53*1.2, 56*1.2, 255); // table[NK_COLOR_WINDOW]; // nk_rgba(45/1.2, 53/1.2, 56/1.2, 255);
    table[NK_COLOR_TOGGLE_HOVER] = active;
    table[NK_COLOR_TOGGLE_CURSOR] = main; // vivid_blue;
    table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = main_hue;
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = hover_hue;
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = active_hue;
    table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SLIDER_CURSOR] = main_hue;
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = hover_hue;
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = active_hue;
    table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);

    // table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);

    // table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
table[NK_COLOR_CHART_COLOR] = main_hue;
table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = hover_hue; // nk_rgba(255, 0, 0, 255);
    // table[NK_COLOR_TAB_HEADER] = main;
    // table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
    // table[NK_COLOR_SELECT_ACTIVE] = main;

// table[NK_COLOR_SELECT] = nk_rgba(255,255,255,255);
table[NK_COLOR_SELECT_ACTIVE] = main_hue;

    // @transparent
    #if !is(ems)
    if( glfwGetWindowAttrib(window_handle(), GLFW_TRANSPARENT_FRAMEBUFFER) == GLFW_TRUE ) {
        table[NK_COLOR_WINDOW].a =
        table[NK_COLOR_HEADER].a = 255;
    }
    #endif
    // @transparent

    nk_style_default(ui_ctx);
    nk_style_from_table(ui_ctx, table);


    if(1)
    {
    struct nk_style_selectable *select;
    select = &ui_ctx->style.selectable;
//    nk_zero_struct(*select);
//    select->hover.data.color     = hover_hue;
//    select->normal_active   = nk_style_item_color(table[NK_COLOR_SELECT_ACTIVE]);
    select->text_hover      = nk_rgba(0,192,255,255);
    select->text_hover_active = select->text_hover;
    select->text_normal_active = select->text_hover; // nk_style_item_color(table[NK_COLOR_SELECT_ACTIVE]).data.color;
    select->rounding        = 2.0f;
    }


    struct nk_style *s = &ui_ctx->style;
    s->window.spacing = nk_vec2(4,0);
    s->window.combo_border = 0.f;
    s->window.scrollbar_size = nk_vec2(5,5);
    s->property.rounding = 0;
    s->combo.border = 0;
    s->combo.button_padding.x = -18;
    s->button.border = 1;
    s->edit.border = 0;

    if( UI_ROW_HEIGHT < 32 ) { // UI_LESSER_SPACING
    s->window.header.label_padding.y /= 2; // 2
    s->window.header.padding.y /= 2; // /= 4 -> 1
    }
}

static float ui_alpha = 1;
static array(float) ui_alphas;
static void ui_alpha_push(float alpha) {
    array_push(ui_alphas, ui_alpha);
    ui_alpha = alpha;

    struct nk_color c;
    struct nk_style *s = &ui_ctx->style;
    c = s->window.background;                  c.a = alpha * 255; nk_style_push_color(ui_ctx, &s->window.background, c);
    c = s->text.color;                         c.a = alpha * 255; nk_style_push_color(ui_ctx, &s->text.color, c);
    c = s->window.fixed_background.data.color; c.a = alpha * 255; nk_style_push_style_item(ui_ctx, &s->window.fixed_background, nk_style_item_color(c));
}
static void ui_alpha_pop() {
    if( array_count(ui_alphas) ) {
        nk_style_pop_style_item(ui_ctx);
        nk_style_pop_color(ui_ctx);
        nk_style_pop_color(ui_ctx);

        ui_alpha = *array_back(ui_alphas);
        array_pop(ui_alphas);
    }
}

// -----------------------------------------------------------------------------
// ui menu

typedef struct ui_item_t {
    char *buf;
    int bufcap;
    int type; // 0xED17 'edit' for a writeable inputbox buffer, else read-only label
} ui_item_t;

static array(ui_item_t) ui_items; // queued menu names. to be evaluated during next frame
static vec2 ui_results = {0}; // clicked menu items from last frame

int ui_item() {
    return ui_items ? (ui_results.x == array_count(ui_items) ? ui_results.y : 0) : 0;
}

int ui_menu(const char *items) { // semicolon- or comma-separated items
    array_push(ui_items, ((ui_item_t){STRDUP(items),0,0}));
    return ui_item();
}
int ui_menu_editbox(char *buf, int bufcap) {
    array_push(ui_items, ((ui_item_t){buf,bufcap,0xED17}));
    return ui_item();
}

int ui_has_menubar() {
    return ui_using_v2_menubar || !!ui_items; // ? UI_MENUROW_HEIGHT + 8 : 0; // array_count(ui_items) > 0;
}

static
void ui_separator_line() {
    struct nk_rect space; nk_layout_peek(&space, ui_ctx); // bounds.w *= 0.95f;
    struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
    nk_stroke_line(canvas, space.x+0,space.y+0,space.x+space.w,space.y+0, 3.0, nk_rgb(128,128,128));
}

NK_API nk_bool
nk_menu_begin_text_styled(struct nk_context *ctx, const char *title, int len,
    nk_flags align, struct nk_vec2 size, struct nk_style_button *style_button) //< @r-lyeh: added style_button param
{
    struct nk_window *win;
    const struct nk_input *in;
    struct nk_rect header;
    int is_clicked = nk_false;
    nk_flags state;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return 0;

    win = ctx->current;
    state = nk_widget(&header, ctx);
    if (!state) return 0;
    in = (state == NK_WIDGET_ROM || win->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    if (nk_do_button_text(&ctx->last_widget_state, &win->buffer, header,
        title, len, align, NK_BUTTON_DEFAULT, style_button, in, ctx->style.font))
        is_clicked = nk_true;
    return nk_menu_begin(ctx, win, title, is_clicked, header, size);
}

static
vec2 ui_toolbar_(array(ui_item_t) ui_items, vec2 ui_results) {
    // adjust size for all the upcoming UI elements
    // old method: nk_layout_row_dynamic(ui_ctx, UI_MENUBAR_ICON_HEIGHT/*h*/, array_count(ui_items));
    {
        const struct nk_style *style = &ui_ctx->style;

        nk_layout_row_template_begin(ui_ctx, UI_MENUBAR_ICON_HEIGHT/*h*/);
        for(int i = 0; i < array_count(ui_items); ++i) {
            char first_token[512];
            sscanf(ui_items[i].buf, "%[^,;|]", first_token); // @fixme: vsnscanf

            char *tooltip = strchr(first_token, '@');
            int len = tooltip ? (int)(tooltip - first_token /*- 1*/) : strlen(first_token);

            float pixels_width = nk_text_width(ui_ctx, first_token, len);
            pixels_width += style->window.header.label_padding.x * 2 + style->window.header.padding.x * 2;
            if( pixels_width < 5 ) pixels_width = 5;
            nk_layout_row_template_push_static(ui_ctx, pixels_width);
        }
        nk_layout_row_template_end(ui_ctx);
    }

    // display the UI elements
    bool has_popups = ui_popups();
    for( int i = 0, end = array_count(ui_items); i < end; ++i ) {
        array(char*) ids = strsplit(ui_items[i].buf, ",;|");

        // transparent style
        static struct nk_style_button transparent_style;
        do_once transparent_style = ui_ctx->style.button;
        do_once transparent_style.normal.data.color = nk_rgba(0,0,0,0);
        do_once transparent_style.border_color = nk_rgba(0,0,0,0);
        do_once transparent_style.active = transparent_style.normal;
        do_once transparent_style.hover = transparent_style.normal;
        do_once transparent_style.hover.data.color = nk_rgba(0,0,0,127);
        transparent_style.text_alignment = NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE; // array_count(ids) > 1 ? NK_TEXT_ALIGN_LEFT : NK_TEXT_ALIGN_CENTERED;

        char *tooltip = strchr(ids[0], '@');
        int len = tooltip ? (int)(tooltip -  ids[0]) : strlen(ids[0]);

        // single button
        if( array_count(ids) == 1 ) {
            // tooltip
            if( tooltip && !has_popups ) {
                struct nk_rect bounds = nk_widget_bounds(ui_ctx);
                if (nk_input_is_mouse_hovering_rect(&ui_ctx->input, bounds) && nk_window_has_focus(ui_ctx)) {
                    nk_tooltip(ui_ctx, tooltip+1);
                }
            }
            // input...
            if( ui_items[i].type == 0xED17 ) {
                int active = nk_edit_string_zero_terminated(ui_ctx, NK_EDIT_AUTO_SELECT|NK_EDIT_CLIPBOARD|NK_EDIT_FIELD/*NK_EDIT_BOX*/|NK_EDIT_SIG_ENTER, ui_items[i].buf, ui_items[i].bufcap, nk_filter_default);
                if( !!(active & NK_EDIT_COMMITED) ) ui_results = vec2(i+1, 0+1), nk_edit_unfocus(ui_ctx);
            }
            else
            // ... else text
            if( nk_button_text_styled(ui_ctx, &transparent_style, ids[0], len) ) {
                ui_results = vec2(i+1, 0+1);
            }
        }
        else {
            struct nk_vec2 dims = {120, array_count(ids) * UI_MENUROW_HEIGHT};
            const struct nk_style *style = &ui_ctx->style;
            const struct nk_user_font *f = style->font;
            static array(float) lens = 0; array_resize(lens, array_count(ids));
            lens[0] = len;
            for( int j = 1; j < array_count(ids); ++j ) {
                lens[j] = strlen(ids[j]);
                float width_px = f->width(f->userdata, f->height, ids[j], lens[j]);
                dims.x = maxf(dims.x, width_px);
            }
            dims.x += 2 * style->window.header.label_padding.x;

            // dropdown menu
            if( nk_menu_begin_text_styled(ui_ctx, ids[0], lens[0], NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE, dims, &transparent_style) ) {
                nk_layout_row_dynamic(ui_ctx, 0, 1);

                for( int j = 1; j < array_count(ids); ++j ) {
                    char *item = ids[j];
                    if( *item == '-' ) {
                        while(*item == '-') ++item, --lens[j];
                        //nk_menu_item_label(ui_ctx, "---", NK_TEXT_LEFT);
                        ui_separator_line();
                    }

                    if( nk_menu_item_text(ui_ctx, item, lens[j], NK_TEXT_LEFT) ) {
                        ui_results = vec2(i+1, j+1-1);
                    }
                }

                nk_menu_end(ui_ctx);
            }
        }
    }

    return ui_results;
}

int ui_toolbar(const char *icons) { // usage: int clicked_icon = ui_toolbar( ICON_1 ";" ICON_2 ";" ICON_3 ";" ICON_4 );
    vec2 results = {0};
    array(char*) items = strsplit(icons, ",;|");
        static array(ui_item_t) temp = 0;
        array_resize(temp, array_count(items));
        for( int i = 0; i < array_count(items); ++i ) temp[i].buf = items[i], temp[i].bufcap = 0, temp[i].type = 0;
    return ui_toolbar_(temp, results).x;
}


// UI Windows handlers. These are not OS Windows but UI Windows instead. For OS Windows check window_*() API.

#ifndef WINDOWS_INI
#define WINDOWS_INI editor_path("windows.ini")
#endif

static map(char*,unsigned) ui_windows = 0;

static void ui_init() {
    do_once {
        nk_config_custom_fonts();
        nk_config_custom_theme();

        map_init(ui_windows, less_str, hash_str);
    }
}

static int ui_window_register(const char *panel_or_window_title) {
    unsigned *state = map_find_or_add_allocated_key(ui_windows, STRDUP(panel_or_window_title), 0);

    // check for visibility flag on first call
    int visible = 0;
    if( *state == 0 ) {
        static ini_t i = 0;
        do_once i = ini(WINDOWS_INI); // @leak
        char **found = i ? map_find(i, va("%s.visible", panel_or_window_title)) : NULL;
        if( found ) visible = (*found)[0] == '1';
    }

    *state |= 2;
    return visible;
}
int ui_visible(const char *panel_or_window_title) {
    return *map_find_or_add_allocated_key(ui_windows, STRDUP(panel_or_window_title), 0) & 1;
}
int ui_show(const char *panel_or_window_title, int enabled) {
    unsigned *found = map_find_or_add_allocated_key(ui_windows, STRDUP(panel_or_window_title), 0);
    if( enabled ) {
        *found |= 1;
        nk_window_collapse(ui_ctx, panel_or_window_title, NK_MAXIMIZED); // in case windows was previously collapsed
    } else {
        *found &= ~1;
    }
    return !!enabled;
}
int ui_dims(const char *panel_or_window_title, float width, float height) {
    nk_window_set_size(ui_ctx, panel_or_window_title, (struct nk_vec2){width, height});
    return 0;
}
vec2 ui_get_dims() {
    return (vec2){nk_window_get_width(ui_ctx), nk_window_get_height(ui_ctx)};
}
static char *ui_build_window_list() {
    char *build_windows_menu = 0;
    strcatf(&build_windows_menu, "%s;", ICON_MD_VIEW_QUILT); // "Windows");
    for each_map_ptr_sorted(ui_windows, char*, k, unsigned, v) {
        strcatf(&build_windows_menu, "%s %s;", ui_visible(*k) ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK, *k); // ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF, *k); // ICON_MD_TOGGLE_ON : ICON_MD_TOGGLE_OFF, *k);
    }
    strcatf(&build_windows_menu, "-%s;%s", ICON_MD_RECYCLING " Reset layout", ICON_MD_SAVE_AS " Save layout");
    return build_windows_menu; // @leak if discarded
}
static int ui_layout_all_reset(const char *mask);
static int ui_layout_all_save_disk(const char *mask);
static int ui_layout_all_load_disk(const char *mask);


static
void ui_menu_render() {
    // clean up from past frame
    ui_results = vec2(0,0);
    if( !ui_items ) return;
    if( !array_count(ui_items) ) return;

// artificially inject Windows menu on the first icon
bool show_window_menu = !!array_count(ui_items);
if( show_window_menu ) {
    array_push_front(ui_items, ((ui_item_t){ui_build_window_list(), 0, 0}));
}

    // process menus
    if( nk_begin(ui_ctx, "Menu", nk_rect(0, 0, window_width(), UI_MENUROW_HEIGHT), NK_WINDOW_NO_SCROLLBAR/*|NK_WINDOW_BACKGROUND*/)) {
        if( ui_ctx->current ) {
            nk_menubar_begin(ui_ctx);

            ui_results = ui_toolbar_(ui_items, ui_results);

            //nk_layout_row_end(ui_ctx);
            nk_menubar_end(ui_ctx);
        }
    }
    nk_end(ui_ctx);

if( show_window_menu ) {
    // if clicked on first menu (Windows)
    if( ui_results.x == 1 ) {
        array(char*) split = strsplit(ui_items[0].buf,";"); // *array_back(ui_items), ";");
        const char *title = split[(int)ui_results.y]; title += title[0] == '-'; title += 3 * (title[0] == '\xee'); title += title[0] == ' '; /*skip separator+emoji+space*/
        // toggle window unless clicked on lasts items {"reset layout", "save layout"}
        bool clicked_reset_layout = ui_results.y == array_count(split) - 2;
        bool clicked_save_layout = ui_results.y == array_count(split) - 1;
        /**/ if( clicked_reset_layout ) ui_layout_all_reset("*");
        else if( clicked_save_layout ) file_delete(WINDOWS_INI), ui_layout_all_save_disk("*");
        else ui_show(title, ui_visible(title) ^ true);
        // reset value so developers don't catch this click
        ui_results = vec2(0,0);
    }
    // restore state prior to previously injected Windows menu
    else
    ui_results.x = ui_results.x > 0 ? ui_results.x - 1 : 0;
}

    // clean up for next frame
    for( int i = 0; i < array_count(ui_items); ++i ) {
        if(ui_items[i].type != 0xED17)
            FREE(ui_items[i].buf);
    }
    array_resize(ui_items, 0);
}

// -----------------------------------------------------------------------------

static int ui_dirty = 1;
static int ui_has_active_popups = 0;
static float ui_hue = 0; // hue
static int ui_is_hover = 0;
static int ui_is_active = 0;
static uint64_t ui_active_mask = 0;

int ui_popups() {
    return ui_has_active_popups;
}
int ui_hover() {
    return ui_is_hover;
}
int ui_active() {
    return ui_is_active || postfx_debug_tool_enabled; //window_has_cursor() && nk_window_is_any_hovered(ui_ctx) && nk_item_is_any_active(ui_ctx);
}

static
int ui_set_enable_(int enabled) {
    static struct nk_style off, on;
    do_once {
        off = on = ui_ctx->style;
        float alpha = 0.5f;

        off.text.color.a *= alpha;

#if 0
        off.button.normal.data.color.a *= alpha;
        off.button.hover.data.color.a *= alpha;
        off.button.active.data.color.a *= alpha;
        off.button.border_color.a *= alpha;
        off.button.text_background.a *= alpha;
        off.button.text_normal.a *= alpha;
        off.button.text_hover.a *= alpha;
        off.button.text_active.a *= alpha;

        off.contextual_button.normal.data.color.a *= alpha;
        off.contextual_button.hover.data.color.a *= alpha;
        off.contextual_button.active.data.color.a *= alpha;
        off.contextual_button.border_color.a *= alpha;
        off.contextual_button.text_background.a *= alpha;
        off.contextual_button.text_normal.a *= alpha;
        off.contextual_button.text_hover.a *= alpha;
        off.contextual_button.text_active.a *= alpha;
#endif
        off.menu_button.normal.data.color.a *= alpha;
        off.menu_button.hover.data.color.a *= alpha;
        off.menu_button.active.data.color.a *= alpha;
        off.menu_button.border_color.a *= alpha;
        off.menu_button.text_background.a *= alpha;
        off.menu_button.text_normal.a *= alpha;
        off.menu_button.text_hover.a *= alpha;
        off.menu_button.text_active.a *= alpha;
#if 0
        off.option.normal.data.color.a *= alpha;
        off.option.hover.data.color.a *= alpha;
        off.option.active.data.color.a *= alpha;
        off.option.border_color.a *= alpha;
        off.option.cursor_normal.data.color.a *= alpha;
        off.option.cursor_hover.data.color.a *= alpha;
        off.option.text_normal.a *= alpha;
        off.option.text_hover.a *= alpha;
        off.option.text_active.a *= alpha;
        off.option.text_background.a *= alpha;

        off.checkbox.normal.data.color.a *= alpha;
        off.checkbox.hover.data.color.a *= alpha;
        off.checkbox.active.data.color.a *= alpha;
        off.checkbox.border_color.a *= alpha;
        off.checkbox.cursor_normal.data.color.a *= alpha;
        off.checkbox.cursor_hover.data.color.a *= alpha;
        off.checkbox.text_normal.a *= alpha;
        off.checkbox.text_hover.a *= alpha;
        off.checkbox.text_active.a *= alpha;
        off.checkbox.text_background.a *= alpha;

        off.selectable.normal.data.color.a *= alpha;
        off.selectable.hover.data.color.a *= alpha;
        off.selectable.pressed.data.color.a *= alpha;
        off.selectable.normal_active.data.color.a *= alpha;
        off.selectable.hover_active.data.color.a *= alpha;
        off.selectable.pressed_active.data.color.a *= alpha;
        off.selectable.text_normal.a *= alpha;
        off.selectable.text_hover.a *= alpha;
        off.selectable.text_pressed.a *= alpha;
        off.selectable.text_normal_active.a *= alpha;
        off.selectable.text_hover_active.a *= alpha;
        off.selectable.text_pressed_active.a *= alpha;
        off.selectable.text_background.a *= alpha;

        off.slider.normal.data.color.a *= alpha;
        off.slider.hover.data.color.a *= alpha;
        off.slider.active.data.color.a *= alpha;
        off.slider.border_color.a *= alpha;
        off.slider.bar_normal.a *= alpha;
        off.slider.bar_hover.a *= alpha;
        off.slider.bar_active.a *= alpha;
        off.slider.bar_filled.a *= alpha;
        off.slider.cursor_normal.data.color.a *= alpha;
        off.slider.cursor_hover.data.color.a *= alpha;
        off.slider.cursor_active.data.color.a *= alpha;
        off.slider.dec_button.normal.data.color.a *= alpha;
        off.slider.dec_button.hover.data.color.a *= alpha;
        off.slider.dec_button.active.data.color.a *= alpha;
        off.slider.dec_button.border_color.a *= alpha;
        off.slider.dec_button.text_background.a *= alpha;
        off.slider.dec_button.text_normal.a *= alpha;
        off.slider.dec_button.text_hover.a *= alpha;
        off.slider.dec_button.text_active.a *= alpha;
        off.slider.inc_button.normal.data.color.a *= alpha;
        off.slider.inc_button.hover.data.color.a *= alpha;
        off.slider.inc_button.active.data.color.a *= alpha;
        off.slider.inc_button.border_color.a *= alpha;
        off.slider.inc_button.text_background.a *= alpha;
        off.slider.inc_button.text_normal.a *= alpha;
        off.slider.inc_button.text_hover.a *= alpha;
        off.slider.inc_button.text_active.a *= alpha;

        off.progress.normal.data.color.a *= alpha;
        off.progress.hover.data.color.a *= alpha;
        off.progress.active.data.color.a *= alpha;
        off.progress.border_color.a *= alpha;
        off.progress.cursor_normal.data.color.a *= alpha;
        off.progress.cursor_hover.data.color.a *= alpha;
        off.progress.cursor_active.data.color.a *= alpha;
        off.progress.cursor_border_color.a *= alpha;
#endif
        off.property.normal.data.color.a *= alpha;
        off.property.hover.data.color.a *= alpha;
        off.property.active.data.color.a *= alpha;
        off.property.border_color.a *= alpha;
        off.property.label_normal.a *= alpha;
        off.property.label_hover.a *= alpha;
        off.property.label_active.a *= alpha;
        off.property.edit.normal.data.color.a *= alpha;
        off.property.edit.hover.data.color.a *= alpha;
        off.property.edit.active.data.color.a *= alpha;
        off.property.edit.border_color.a *= alpha;
        off.property.edit.cursor_normal.a *= alpha;
        off.property.edit.cursor_hover.a *= alpha;
        off.property.edit.cursor_text_normal.a *= alpha;
        off.property.edit.cursor_text_hover.a *= alpha;
        off.property.edit.text_normal.a *= alpha;
        off.property.edit.text_hover.a *= alpha;
        off.property.edit.text_active.a *= alpha;
        off.property.edit.selected_normal.a *= alpha;
        off.property.edit.selected_hover.a *= alpha;
        off.property.edit.selected_text_normal.a *= alpha;
        off.property.edit.selected_text_hover.a *= alpha;
        off.property.dec_button.normal.data.color.a *= alpha;
        off.property.dec_button.hover.data.color.a *= alpha;
        off.property.dec_button.active.data.color.a *= alpha;
        off.property.dec_button.border_color.a *= alpha;
        off.property.dec_button.text_background.a *= alpha;
        off.property.dec_button.text_normal.a *= alpha;
        off.property.dec_button.text_hover.a *= alpha;
        off.property.dec_button.text_active.a *= alpha;
        off.property.inc_button.normal.data.color.a *= alpha;
        off.property.inc_button.hover.data.color.a *= alpha;
        off.property.inc_button.active.data.color.a *= alpha;
        off.property.inc_button.border_color.a *= alpha;
        off.property.inc_button.text_background.a *= alpha;
        off.property.inc_button.text_normal.a *= alpha;
        off.property.inc_button.text_hover.a *= alpha;
        off.property.inc_button.text_active.a *= alpha;

        off.edit.normal.data.color.a *= alpha;
        off.edit.hover.data.color.a *= alpha;
        off.edit.active.data.color.a *= alpha;
        off.edit.border_color.a *= alpha;
        off.edit.cursor_normal.a *= alpha;
        off.edit.cursor_hover.a *= alpha;
        off.edit.cursor_text_normal.a *= alpha;
        off.edit.cursor_text_hover.a *= alpha;
        off.edit.text_normal.a *= alpha;
        off.edit.text_hover.a *= alpha;
        off.edit.text_active.a *= alpha;
        off.edit.selected_normal.a *= alpha;
        off.edit.selected_hover.a *= alpha;
        off.edit.selected_text_normal.a *= alpha;
        off.edit.selected_text_hover.a *= alpha;
#if 0
        off.chart.background.data.color.a *= alpha;
        off.chart.border_color.a *= alpha;
        off.chart.selected_color.a *= alpha;
        off.chart.color.a *= alpha;

        off.scrollh.normal.data.color.a *= alpha;
        off.scrollh.hover.data.color.a *= alpha;
        off.scrollh.active.data.color.a *= alpha;
        off.scrollh.border_color.a *= alpha;
        off.scrollh.cursor_normal.data.color.a *= alpha;
        off.scrollh.cursor_hover.data.color.a *= alpha;
        off.scrollh.cursor_active.data.color.a *= alpha;
        off.scrollh.cursor_border_color.a *= alpha;

        off.scrollv.normal.data.color.a *= alpha;
        off.scrollv.hover.data.color.a *= alpha;
        off.scrollv.active.data.color.a *= alpha;
        off.scrollv.border_color.a *= alpha;
        off.scrollv.cursor_normal.data.color.a *= alpha;
        off.scrollv.cursor_hover.data.color.a *= alpha;
        off.scrollv.cursor_active.data.color.a *= alpha;
        off.scrollv.cursor_border_color.a *= alpha;

        off.tab.background.data.color.a *= alpha;
        off.tab.border_color.a *= alpha;
        off.tab.text.a *= alpha;
#endif
        off.combo.normal.data.color.a *= alpha;
        off.combo.hover.data.color.a *= alpha;
        off.combo.active.data.color.a *= alpha;
        off.combo.border_color.a *= alpha;
        off.combo.label_normal.a *= alpha;
        off.combo.label_hover.a *= alpha;
        off.combo.label_active.a *= alpha;
        off.combo.symbol_normal.a *= alpha;
        off.combo.symbol_hover.a *= alpha;
        off.combo.symbol_active.a *= alpha;
        off.combo.button.normal.data.color.a *= alpha;
        off.combo.button.hover.data.color.a *= alpha;
        off.combo.button.active.data.color.a *= alpha;
        off.combo.button.border_color.a *= alpha;
        off.combo.button.text_background.a *= alpha;
        off.combo.button.text_normal.a *= alpha;
        off.combo.button.text_hover.a *= alpha;
        off.combo.button.text_active.a *= alpha;
#if 0
        off.window.fixed_background.data.color.a *= alpha;
        off.window.background.a *= alpha;
        off.window.border_color.a *= alpha;
        off.window.popup_border_color.a *= alpha;
        off.window.combo_border_color.a *= alpha;
        off.window.contextual_border_color.a *= alpha;
        off.window.menu_border_color.a *= alpha;
        off.window.group_border_color.a *= alpha;
        off.window.tooltip_border_color.a *= alpha;
        off.window.scaler.data.color.a *= alpha;
        off.window.header.normal.data.color.a *= alpha;
        off.window.header.hover.data.color.a *= alpha;
        off.window.header.active.data.color.a *= alpha;
#endif
    }
    static struct nk_input input;
    if (!enabled) {
        ui_alpha_push(0.5);
        ui_ctx->style = off; // .button = off.button;
        input = ui_ctx->input;
        memset(&ui_ctx->input, 0, sizeof(ui_ctx->input));
    } else {
        ui_alpha_pop();
        ui_ctx->style = on; // .button = on.button;
        ui_ctx->input = input;
    }
    return enabled;
}

static int ui_is_enabled = 1;
int ui_enable() {
    return ui_is_enabled == 1 ? 0 : ui_set_enable_(ui_is_enabled = 1);
}
int ui_disable() {
    return ui_is_enabled == 0 ? 0 : ui_set_enable_(ui_is_enabled = 0);
}
int ui_enabled() {
    return ui_is_enabled;
}

static
void ui_destroy(void) {
    if(ui_ctx) {
        nk_glfw3_shutdown(&nk_glfw); // nk_sdl_shutdown();
        ui_ctx = 0;
    }
}
static
void ui_create() {
    do_once atexit(ui_destroy);

    if( ui_dirty ) {
        nk_glfw3_new_frame(&nk_glfw); //g->nk_glfw);
        ui_dirty = 0;

        ui_enable();
    }
}

enum {
    UI_NOTIFICATION_1 = 32, // sets panel as 1-story notification. used by ui_notify()
    UI_NOTIFICATION_2 = 64, // sets panel as 2-story notification. used by ui_notify()
};

struct ui_notify {
    char *title;
    char *body; // char *icon;
    float timeout;
    float alpha;
    int   used;
};

static array(struct ui_notify) ui_notifications; // format=("%d*%s\n%s", timeout, title, body)

static
void ui_notify_render() {
    // draw queued notifications
    if( array_count(ui_notifications) ) {
        struct ui_notify *n = array_back(ui_notifications);

        static double timeout = 0;
        timeout += window_delta(); // @fixme: use editor_time() instead

        ui_alpha_push( timeout >= n->timeout ? 1 - clampf(timeout - n->timeout,0,1) : 1 );

            if( timeout < (n->timeout + 1) ) { // N secs display + 1s fadeout
                if(n->used++ < 3) nk_window_set_focus(ui_ctx, "!notify");

                if( ui_panel( "!notify", n->title && n->body ? UI_NOTIFICATION_2 : UI_NOTIFICATION_1 ) ) {
                    if(n->title) ui_label(n->title);
                    if(n->body)  ui_label(n->body);

                    ui_panel_end();
                }
            }

            if( timeout >= (n->timeout + 2) ) { // 1s fadeout + 1s idle
                timeout = 0;

                if(n->title) FREE(n->title);
                if(n->body)  FREE(n->body);
                array_pop(ui_notifications);
            }

        ui_alpha_pop();
    }
}

static
void ui_hue_cycle( unsigned num_cycles ) {
    // cycle color (phi ratio)
    for( unsigned i = 0; i < num_cycles; ++i ) {
        //ui_hue = (ui_hue+0.61803f)*1.61803f; while(ui_hue > 1) ui_hue -= 1;
        ui_hue *= 1.61803f / 1.85f; while(ui_hue > 1) ui_hue -= 1;
    }
}

static bool win_debug_visible = true;
static bool ui_hidden;

int ui_hide(bool hidden) {
    return ui_hidden = hidden;
}

static
void ui_render() {
    // draw queued menus
    ui_notify_render();
    ui_menu_render();

    /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state after
     * rendering the UI. */
    //nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

    if (!ui_hidden) {
        GLfloat bkColor[4]; glGetFloatv(GL_COLOR_CLEAR_VALUE, bkColor); // @transparent
        glClearColor(0,0,0,1); // @transparent
        glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,/*bkColor[3] ? GL_FALSE :*/ GL_TRUE);  // @transparent
        nk_glfw3_render(&nk_glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
        glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);  // @transparent
    } else {
        nk_clear(&nk_glfw.ctx);
    }

#if is(ems)
    glFinish();
#endif

    ui_dirty = 1;
    ui_hue = 0;

    ui_is_hover = nk_window_is_any_hovered(ui_ctx) && window_has_cursor();

    if(input_down(MOUSE_L))
        ui_is_active = (ui_is_hover && nk_item_is_any_active(ui_ctx));
    if(input_up(MOUSE_L))
        ui_is_active = 0;
}


// -----------------------------------------------------------------------------
// save/restore all window layouts on every framebuffer resize

#define UI_SNAP_PX      1 /*treshold of pixels when snapping panels/windows to the application borders [1..N]*/
#define UI_ANIM_ALPHA 0.9 /*animation alpha used when restoring panels/windows state from application resizing events: [0..1]*/
//#define UI_MENUBAR_Y     32 // menubar and row

typedef struct ui_layout {
    const char *title;

    bool is_panel;

    vec2   desktop;
    vec2     p0,p1;
    float    l0,l1;

    float alpha;
    float anim_timer;

} ui_layout;

static array(ui_layout) ui_layouts[2] = {0};

static
int ui_layout_find(const char *title, bool is_panel) {
    int i = 0;
    for each_array_ptr(ui_layouts[is_panel], ui_layout, s) {
        if( !strcmp(title, s->title) ) return i;
        ++i;
    }
    ui_layout s = {0};
    s.is_panel = is_panel;
    s.title = STRDUP(title);
    array_push(ui_layouts[is_panel], s);
    return array_count(ui_layouts[is_panel]) - 1;
}

static
void ui_layout_save_mem(int idx, vec2 desktop, float workarea_h, struct nk_rect *xywh_, bool is_panel) {
    struct nk_rect xywh = *xywh_; //< workaround for a (tcc-0.9.27+lubuntu16) bug, where xywh_ is never populated (ie, empty always) when passed by-copy

    ui_layout *s = &ui_layouts[is_panel][idx];

    struct nk_window *win = nk_window_find(ui_ctx, s->title);
    // if(win && win->flags & NK_WINDOW_FULLSCREEN) return;    // skip if maximized

    s->desktop = desktop;

float excess = 0;
if( win && (win->flags & NK_WINDOW_MINIMIZED)) {
    excess = xywh.h - UI_MENUROW_HEIGHT;
    xywh.h = UI_MENUROW_HEIGHT;
}

    // sanity checks
    if(xywh.x<0)                               xywh.x = 0;
    if(xywh.w>desktop.w-UI_SNAP_PX)            xywh.w = desktop.w-UI_SNAP_PX-1;

    if(xywh.y<workarea_h)                      xywh.y = workarea_h;
    if(xywh.h>desktop.h-workarea_h-UI_SNAP_PX) xywh.h = desktop.h-workarea_h-UI_SNAP_PX-1;

    if((xywh.x+xywh.w)>desktop.w)              xywh.x-= xywh.x+xywh.w-desktop.w;
    if((xywh.y+xywh.h)>desktop.h)              xywh.y-= xywh.y+xywh.h-desktop.h;

if( win && (win->flags & NK_WINDOW_MINIMIZED)) {
    xywh.h += excess;
}

    // build reconstruction vectors from bottom-right corner
    s->p0 = vec2(xywh.x/s->desktop.x,xywh.y/s->desktop.y);
    s->p1 = vec2(xywh.w/s->desktop.x,xywh.h/s->desktop.y);
    s->p0 = sub2(s->p0, vec2(1,1)); s->l0 = len2(s->p0);
    s->p1 = sub2(s->p1, vec2(1,1)); s->l1 = len2(s->p1);
}

static
struct nk_rect ui_layout_load_mem(int idx, vec2 desktop, bool is_panel) {
    ui_layout *s = &ui_layouts[is_panel][idx];

    // extract reconstruction coords from bottom-right corner
    vec2 p0 = mul2(add2(vec2(1,1), scale2(norm2(s->p0), s->l0)), desktop);
    vec2 p1 = mul2(add2(vec2(1,1), scale2(norm2(s->p1), s->l1)), desktop);

    return nk_rect( p0.x, p0.y, p1.x, p1.y );
}

static
int ui_layout_all_reset(const char *mask) {
    ui_layout z = {0};

    vec2 desktop = vec2(window_width(), window_height());
    float workarea_h = ui_has_menubar()*UI_MENUROW_HEIGHT; // @fixme workarea -> reserved_area

    for( int is_panel = 0; is_panel < 2; ++is_panel ) {
        for( int j = 0; j < array_count(ui_layouts[is_panel]); ++j ) {
            if( ui_layouts[is_panel][j].title ) {

                if( nk_window_is_hidden(ui_ctx, ui_layouts[is_panel][j].title) ) continue;

                struct nk_rect xywh = { 0, workarea_h + j * UI_MENUROW_HEIGHT, desktop.w / 3.333, UI_MENUROW_HEIGHT };
                if( is_panel ) {
                    xywh.x = 0;
                    xywh.y = workarea_h + j * UI_MENUROW_HEIGHT;
                    xywh.w = desktop.w / 4;
                    xywh.h = desktop.h / 3;
                } else {
                    xywh.x = desktop.w / 3.00 + j * UI_MENUROW_HEIGHT;
                    xywh.y = workarea_h + j * UI_MENUROW_HEIGHT;
                    xywh.w = desktop.w / 4;
                    xywh.h = desktop.h / 3;
                }
                nk_window_set_focus(ui_ctx, ui_layouts[is_panel][j].title);
                nk_window_collapse(ui_ctx, ui_layouts[is_panel][j].title, is_panel ? 0 : 1);
                struct nk_window* win = is_panel ? 0 : nk_window_find(ui_ctx, ui_layouts[is_panel][j].title );
                if(win) win->flags &= ~NK_WINDOW_FULLSCREEN;
                if(win) win->flags &= ~NK_WINDOW_MINIMIZED;
                ui_layout_save_mem(j, desktop, workarea_h, &xywh, is_panel);
                ui_layouts[is_panel][j].anim_timer = 1.0;
            }
        }
    }

    return 1;
}

static
int ui_layout_all_save_disk(const char *mask) {
    float w = window_width(), h = window_height();
    for each_map_ptr_sorted(ui_windows, char*, k, unsigned, v) {
        struct nk_window *win = nk_window_find(ui_ctx, *k);
        if( win && strmatchi(*k, mask) ) {
            ini_write(WINDOWS_INI, *k, "x", va("%f", win->bounds.x / w ));
            ini_write(WINDOWS_INI, *k, "y", va("%f", win->bounds.y / h ));
            ini_write(WINDOWS_INI, *k, "w", va("%f", win->bounds.w / w ));
            ini_write(WINDOWS_INI, *k, "h", va("%f", win->bounds.h / h ));
            ini_write(WINDOWS_INI, *k, "visible", ui_visible(*k) ? "1":"0");
        }
    }
    return 1;
}

static
const char *ui_layout_load_disk(const char *title, const char *mask, ini_t i, struct nk_rect *r) {
    if(!i) return 0;

    const char *dot = strrchr(title, '.');
    if( dot ) title = va("%.*s", (int)(dot - title), title);
    if( !strmatchi(title, mask) ) return 0;

    char **x = map_find(i, va("%s.x", title));
    char **y = map_find(i, va("%s.y", title));
    char **w = map_find(i, va("%s.w", title));
    char **h = map_find(i, va("%s.h", title));
    if( x && y && w && h ) {
        float ww = window_width(), wh = window_height();
        r->x = atof(*x) * ww;
        r->y = atof(*y) * wh;
        r->w = atof(*w) * ww;
        r->h = atof(*h) * wh;

        char **on = map_find(i, va("%s.visible", title));

        return title;
    }
    return 0;
}

static
int ui_layout_all_load_disk(const char *mask) {
    ini_t i = ini(WINDOWS_INI); // @leak
    if( !i ) return 0;
    for each_map(i, char*, k, char*, v) {
        struct nk_rect out = {0};
        const char *title = ui_layout_load_disk(k, mask, i, &out);
        if( title ) {
            struct nk_window *win = nk_window_find(ui_ctx, title);
            if( win ) {
                win->bounds.x = out.x;
                win->bounds.y = out.y;
                win->bounds.w = out.w;
                win->bounds.h = out.h;
            }
        }
    }
    return 1;
}


// -----------------------------------------------------------------------------
// shared code for both panels and windows. really messy.

static
int ui_begin_panel_or_window_(const char *title, int flags, bool is_window) {

struct nk_window *win = nk_window_find(ui_ctx, title);

int is_panel = !is_window;
bool starts_minimized = is_panel ? !(flags & PANEL_OPEN) : 0;
bool is_closable = is_window;
bool is_scalable = true;
bool is_movable = true;
bool is_auto_minimizes = starts_minimized; // false;
bool is_pinned = win && (win->flags & NK_WINDOW_PINNED);

if( is_pinned ) {
//    is_closable = false;
    is_auto_minimizes = false;
    is_scalable = false;
//    is_movable = false;
}

    ui_create();

    uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
    for(int i = 0; title[i]; ++i) hash = (hash ^ title[i]) * mult;
    ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

    int idx = ui_layout_find(title, is_panel);
    ui_layout *s = &ui_layouts[is_panel][idx];

vec2 desktop = vec2(window_width(), window_height());
float workarea_h = ui_has_menubar()*UI_MENUROW_HEIGHT;

    int row = idx + !!ui_has_menubar(); // add 1 to skip menu
    vec2 offset = vec2(0, UI_ROW_HEIGHT*row);
    float w = desktop.w / 3.33, h = (flags & UI_NOTIFICATION_2 ? UI_MENUROW_HEIGHT*2 : (flags & UI_NOTIFICATION_1 ? UI_MENUROW_HEIGHT : desktop.h - offset.y * 2 - 1)); // h = desktop.h * 0.66; //
    struct nk_rect start_coords = {offset.x, offset.y, offset.x+w, offset.y+h};

if(is_window) {
    w = desktop.w / 1.5;
    h = desktop.h / 1.5;
    start_coords.x = (desktop.w-w)/2;
    start_coords.y = (desktop.h-h)/2 + workarea_h/2;
    start_coords.w = w;
    start_coords.h = h;
}

    static vec2 edge = {0}; static int edge_type = 0; // [off],L,R,U,D
    do_once edge = vec2(desktop.w * 0.33, desktop.h * 0.66);

// do not snap windows and/or save windows when using may be interacting with UI
int is_active = 0;
int mouse_pressed = !!input(MOUSE_L) && ui_ctx->active == win;
if( win ) {
    // update global window activity bitmask
    is_active = ui_ctx->active == win;
    ui_active_mask = is_active ? ui_active_mask | (1ull << idx) : ui_active_mask & ~(1ull << idx);
}

//  struct nk_style *s = &ui_ctx->style;
//  nk_style_push_color(ui_ctx, &s->window.header.normal.data.color, nk_hsv_f(ui_hue,0.6,0.8));

// adjust inactive edges automatically
if( win ) {
    bool group1_any             = !is_active; // && !input(MOUSE_L);
    bool group2_not_resizing    =  is_active && !win->is_window_resizing;
    bool group2_interacting     =  is_active && input(MOUSE_L);

#if 0
    if( group1_any ) {
        // cancel self-adjust if this window is not overlapping the active one that is being resized at the moment
        struct nk_window *parent = ui_ctx->active;

        struct nk_rect a = win->bounds, b = parent->bounds;
        bool overlap = a.x <= (b.x+b.w) && b.x <= (a.x+a.w) && a.y <= (b.y+b.h) && b.y <= (a.y+a.h);

        group1_any = overlap;
    }
#else
    if( group1_any )
        group1_any = !(win->flags & NK_WINDOW_PINNED);
#endif

    if( group1_any ) {
        float mouse_x = clampf(input(MOUSE_X), 0, desktop.w);
        float mouse_y = clampf(input(MOUSE_Y), 0, desktop.h);
        float distance_x = absf(mouse_x - win->bounds.x) / desktop.w;
        float distance_y = absf(mouse_y - win->bounds.y) / desktop.h;
        float alpha_x = sqrt(sqrt(distance_x)); // amplify signals a little bit: 0.1->0.56,0.5->0.84,0.98->0.99,etc
        float alpha_y = sqrt(sqrt(distance_y));

        /**/ if( (edge_type & 1) && win->bounds.x <= UI_SNAP_PX ) {
            win->bounds.w = win->bounds.w * alpha_y + edge.w * (1-alpha_y);
        }
        else if( (edge_type & 2) && (win->bounds.x + win->bounds.w) >= (desktop.w-UI_SNAP_PX) ) {
            win->bounds.w = win->bounds.w * alpha_y + edge.w * (1-alpha_y);
            win->bounds.x = desktop.w - win->bounds.w;
        }
        if( (edge_type & 8) && (win->bounds.y + (win->flags & NK_WINDOW_MINIMIZED ? UI_ROW_HEIGHT : win->bounds.h)) >= (desktop.h-UI_SNAP_PX) ) {
            win->bounds.h = win->bounds.h * alpha_x + edge.h * (1-alpha_x);
            win->bounds.y = desktop.h - (win->flags & NK_WINDOW_MINIMIZED ? UI_ROW_HEIGHT : win->bounds.h);
        }
    }

    // skip any saving if window is animating (moving) and/or maximized
    bool anim_in_progress = s->anim_timer > 1e-3;
    s->anim_timer *= anim_in_progress * UI_ANIM_ALPHA;

    if( group1_any || !group2_interacting || anim_in_progress ) {
        struct nk_rect target = ui_layout_load_mem(idx, desktop, is_panel);
        float alpha = len2sq(sub2(s->desktop, desktop)) ? 0 : UI_ANIM_ALPHA; // smooth unless we're restoring a desktop change
#if 1
        if( is_window && win->flags & NK_WINDOW_FULLSCREEN ) {
            target.x = 1;
            target.w = desktop.w - 1;
            target.y = workarea_h + 1;
            target.h = desktop.h - workarea_h - 2;
        }
        if( is_window && win->is_window_restoring > 1e-2) {
            win->is_window_restoring = win->is_window_restoring * alpha + 0 * (1 - alpha);
            target.w = desktop.w / 2;
            target.h = (desktop.h - workarea_h) / 2;
            target.x = (desktop.w - target.w) / 2;
            target.y = ((desktop.h - workarea_h) - target.h) / 2;
        }
#endif
        win->bounds = nk_rect(
            win->bounds.x * alpha + target.x * (1 - alpha),
            win->bounds.y * alpha + target.y * (1 - alpha),
            win->bounds.w * alpha + target.w * (1 - alpha),
            win->bounds.h * alpha + target.h * (1 - alpha)
        );
    }
    if(!anim_in_progress)
    ui_layout_save_mem(idx, desktop, workarea_h, &win->bounds, is_panel);
} else { // if(!win)
    ui_layout_save_mem(idx, desktop, workarea_h, &start_coords, is_panel);
}


    int window_flags = NK_WINDOW_PINNABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_NO_SCROLLBAR_X | (is_window ? NK_WINDOW_MAXIMIZABLE : 0);
    if( starts_minimized ) window_flags |= (win ? 0 : NK_WINDOW_MINIMIZED);
    if( is_auto_minimizes ) window_flags |= is_active ? 0 : !!starts_minimized * NK_WINDOW_MINIMIZED;
    if( is_movable )  window_flags |= NK_WINDOW_MOVABLE;
    if( is_closable ) window_flags |= NK_WINDOW_CLOSABLE;
    if( is_scalable ) {
        window_flags |= NK_WINDOW_SCALABLE;
        if(win) window_flags |= input(MOUSE_X) < (win->bounds.x + win->bounds.w/2) ? NK_WINDOW_SCALE_LEFT : 0;
        if(win) window_flags |= input(MOUSE_Y) < (win->bounds.y + win->bounds.h/2) ? NK_WINDOW_SCALE_TOP : 0;
    }

//    if( is_pinned )
        window_flags |= NK_WINDOW_BORDER;

if( is_panel && win && !is_active ) {
    if( !is_pinned && is_auto_minimizes ) {
        window_flags |= NK_WINDOW_MINIMIZED;
    }
}

// if( is_modal ) window_flags &= ~(NK_WINDOW_MINIMIZED | NK_WINDOW_MINIMIZABLE);
if( is_panel && win ) {
//    if( win->bounds.x > 0 && (win->bounds.x+win->bounds.w) < desktop.w-1 ) window_flags &= ~NK_WINDOW_MINIMIZED;
}

if(!win) { // if newly created window (!win)
    // first time, try to restore from WINDOWS_INI file
    static ini_t i; do_once i = ini(WINDOWS_INI); // @leak
    ui_layout_load_disk(title, "*", i, &start_coords);
    ui_layout_save_mem(idx, desktop, workarea_h, &start_coords, is_panel);
}

bool is_notify = flags & (UI_NOTIFICATION_1 | UI_NOTIFICATION_2);
if( is_notify ) {
    window_flags = NK_WINDOW_MOVABLE | NK_WINDOW_NOT_INTERACTIVE | NK_WINDOW_NO_SCROLLBAR;
    start_coords = nk_rect(desktop.w / 2 - w / 2, -h, w, h);
}

    if( nk_begin(ui_ctx, title, start_coords, window_flags) ) {

// set width for all inactive panels
struct nk_rect bounds = nk_window_get_bounds(ui_ctx);
if( mouse_pressed && win && win->is_window_resizing ) {
    edge = vec2(bounds.w, bounds.h);

    // push direction
    int top  = !!(win->is_window_resizing & NK_WINDOW_SCALE_TOP);
    int left = !!(win->is_window_resizing & NK_WINDOW_SCALE_LEFT), right = !left;

    edge_type = 0;
    /**/ if( right && (win->bounds.x <= UI_SNAP_PX) ) edge_type |= 1;
    else if(  left && (win->bounds.x + win->bounds.w) >= (desktop.w-UI_SNAP_PX) ) edge_type |= 2;
    /**/ if(   top && (win->bounds.y + win->bounds.h) >= (desktop.h-UI_SNAP_PX) ) edge_type |= 8;

    // @fixme
    // - if window is in a corner (sharing 2 edges), do not allow for multi edges. either vert or horiz depending on the clicked scaler
    // - or maybe, only propagate edge changes to the other windows that overlapping our window.
}

        return 1;
    } else {

if(is_panel) {
   ui_panel_end();
} else ui_window_end();

        return 0;
    }
}

static const char *ui_last_title = 0;
static int *ui_last_enabled = 0;
static int ui_has_window = 0;
static int ui_window_has_menubar = 0;
int ui_window(const char *title, int *enabled) {
    if( window_width() <= 0 ) return 0;
    if( window_height() <= 0 ) return 0;
    if( !ui_ctx || !ui_ctx->style.font ) return 0;

    bool forced_creation = enabled && *enabled; // ( enabled ? *enabled : !ui_has_menubar() );
    forced_creation |= ui_window_register(title);
    if(!ui_visible(title)) {
        if( !forced_creation ) return 0;
        ui_show(title, forced_creation);
    }

    ui_last_enabled = enabled;
    ui_last_title = title;
    ui_has_window = 1;
    return ui_begin_panel_or_window_(title, /*flags*/0, true);
}
int ui_window_end() {
    if(ui_window_has_menubar) nk_menubar_end(ui_ctx), ui_window_has_menubar = 0;
    nk_end(ui_ctx), ui_has_window = 0;

    int closed = 0;
    if( nk_window_is_hidden(ui_ctx, ui_last_title) ) {
        nk_window_close(ui_ctx, ui_last_title);
        ui_show(ui_last_title, false);
        if( ui_last_enabled ) *ui_last_enabled = 0; // clear developers' flag
        closed = 1;
    }

    // @transparent
    #if !is(ems)
    static bool has_transparent_attrib = 0; do_once has_transparent_attrib = glfwGetWindowAttrib(window_handle(), GLFW_TRANSPARENT_FRAMEBUFFER) == GLFW_TRUE;
    if( closed && has_transparent_attrib && !ui_has_menubar() ) {
        bool any_open = 0;
        for each_map_ptr(ui_windows, char*, k, unsigned, v) any_open |= *v & 1;
        if( !any_open ) glfwSetWindowShouldClose(window_handle(), GLFW_TRUE);
    }
    #endif
    // @transparent

    return 0;
}

int ui_panel(const char *title, int flags) {
    if( window_width() <= 0 ) return 0;
    if( window_height() <= 0 ) return 0;
    if( !ui_ctx || !ui_ctx->style.font ) return 0;

    if( ui_has_window ) {
        // transparent style
        static struct nk_style_button transparent_style;
        do_once transparent_style = ui_ctx->style.button;
        do_once transparent_style.normal.data.color = nk_rgba(0,0,0,0);
        do_once transparent_style.border_color = nk_rgba(0,0,0,0);
        do_once transparent_style.active = transparent_style.normal;
        do_once transparent_style.hover = transparent_style.normal;
        do_once transparent_style.hover.data.color = nk_rgba(0,0,0,127);
        transparent_style.text_alignment = NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE;

        if(!ui_window_has_menubar) nk_menubar_begin(ui_ctx);
        if(!ui_window_has_menubar) nk_layout_row_begin(ui_ctx, NK_STATIC, UI_MENUBAR_ICON_HEIGHT, 4);
        if(!ui_window_has_menubar) nk_layout_row_push(ui_ctx, 70);
        ui_window_has_menubar = 1;

        return nk_menu_begin_text_styled(ui_ctx, title, strlen(title), NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE, nk_vec2(220, 200), &transparent_style);
    }

    return ui_begin_panel_or_window_(title, flags, false);
}
int ui_panel_end() {
    if( ui_has_window ) {
        nk_menu_end(ui_ctx);
        return 0;
    }
    nk_end(ui_ctx);
//  nk_style_pop_color(ui_ctx);
    return 0;
}

static unsigned ui_collapse_state = 0;
int ui_collapse(const char *label, const char *id) { // mask: 0(closed),1(open),2(created)
    int start_open = label[0] == '!'; label += start_open;

    uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
    for(int i = 0; id[i]; ++i) hash = (hash ^ id[i]) * mult;
    ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

    int forced = ui_filter && ui_filter[0];
    enum nk_collapse_states forced_open = NK_MAXIMIZED;

    ui_collapse_state = nk_tree_base_(ui_ctx, NK_TREE_NODE, 0, label, start_open ? NK_MAXIMIZED : NK_MINIMIZED, forced ? &forced_open : NULL, id, strlen(id), 0);

    return ui_collapse_state & 1; // |1 open, |2 clicked, |4 toggled
}
int ui_collapse_clicked() {
    return ui_collapse_state >> 1; // |1 clicked, |2 toggled
}
int ui_collapse_end() {
    return nk_tree_pop(ui_ctx), 1;
}


int ui_contextual() {
#if 0
    struct nk_rect bounds = nk_widget_bounds(ui_ctx); // = nk_window_get_bounds(ui_ctx);
    bounds.y -= 25;
    return ui_popups() ? 0 : nk_contextual_begin(ui_ctx, 0, nk_vec2(150, 300), bounds);
#else
    return ui_popups() ? 0 : nk_contextual_begin(ui_ctx, 0, nk_vec2(300, 220), nk_window_get_bounds(ui_ctx));
#endif
}
int ui_contextual_end(int close) {
    if(close) nk_contextual_close(ui_ctx);
    nk_contextual_end(ui_ctx);
    return 1;
}
int ui_submenu(const char *options) {
    int choice = 0;
    if( ui_contextual() ) {
        array(char*) tokens = strsplit(options, ";");
        for( int i = 0; i < array_count(tokens) ; ++i ) {
            if( ui_button_transparent(tokens[i]) ) choice = i + 1;
        }
        ui_contextual_end(0);
    }
    return choice;
}

// -----------------------------------------------------------------------------
// code for all the widgets

static
int nk_button_transparent(struct nk_context *ctx, const char *text) {
    static struct nk_style_button transparent_style;
    do_once transparent_style = ctx->style.button;
    do_once transparent_style.text_alignment = NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE;
    do_once transparent_style.normal.data.color = nk_rgba(0,0,0,0);
    do_once transparent_style.border_color = nk_rgba(0,0,0,0);
    do_once transparent_style.active = transparent_style.normal;
    do_once transparent_style.hover = transparent_style.normal;
    do_once transparent_style.hover.data.color = nk_rgba(0,0,0,127);
    transparent_style.text_background.a = 255 * ui_alpha;
    transparent_style.text_normal.a = 255 * ui_alpha;
    transparent_style.text_hover.a = 255 * ui_alpha;
    transparent_style.text_active.a = 255 * ui_alpha;
    return nk_button_label_styled(ctx, &transparent_style, text);
}

// internal vars for our editor. @todo: maybe expose these to the end-user as well?
bool ui_label_icon_highlight;
vec2 ui_label_icon_clicked_L; // left
vec2 ui_label_icon_clicked_R; // right

static
int ui_label_(const char *label, int alignment) {
    // beware: assuming label can start with any ICON_MD_ glyph, which I consider them to be a 3-bytes utf8 sequence.
    // done for optimization reasons because this codepath is called a lot!
    const char *icon = label ? label : ""; while( icon[0] == '!' || icon[0] == '*' ) ++icon;
    int has_icon = (unsigned)icon[0] > 127, icon_len = 3, icon_width_px = 1*24;

    struct nk_rect bounds = nk_widget_bounds(ui_ctx);
    const struct nk_input *input = &ui_ctx->input;
    int is_hovering = nk_input_is_mouse_hovering_rect(input, bounds) && !ui_has_active_popups;
    if( is_hovering ) {
        struct nk_rect winbounds = nk_window_get_bounds(ui_ctx);
        is_hovering &= nk_input_is_mouse_hovering_rect(input, winbounds);

        struct nk_window *win = ui_ctx->current;
        bool has_contextual = !win->name; // contextual windows are annonymous

        is_hovering &= has_contextual || nk_window_has_focus(ui_ctx);
    }

    int skip_color_tab = label && label[0] == '!';
    if( skip_color_tab) label++;

    int spacing = 8; // between left colorbar and content
    struct nk_window *win = ui_ctx->current;
    struct nk_panel *layout = win->layout;
    layout->at_x += spacing;
    layout->bounds.w -= spacing;
    if( !skip_color_tab ) {
        float w = is_hovering ? 4 : 2; // spacing*3/4 : spacing/2-1;
        bounds.w = w;
        bounds.h -= 1;
        struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
        nk_fill_rect(canvas, bounds, 0, nk_hsva_f(ui_hue, 0.75f, 0.8f, ui_alpha) );
    }

    if(!label || !label[0]) {
        nk_label(ui_ctx, "", alignment);
        layout->at_x -= spacing;
        layout->bounds.w += spacing;
        return 0;
    }

        const char *split = strchr(label, '@');
            char buffer[128]; if( split ) label = (snprintf(buffer, 128, "%.*s", (int)(split-label), label), buffer);

struct nk_style *style = &ui_ctx->style;
bool bold = label[0] == '*'; label += bold;
struct nk_font *font = bold && nk_glfw.atlas.fonts->next ? nk_glfw.atlas.fonts->next->next /*3rd font*/ : NULL; // list

if( !has_icon ) {
    // set bold style and color if needed
    if( font && nk_style_push_font(ui_ctx, &font->handle) ) {} else font = 0;
    if( font )  nk_style_push_color(ui_ctx, &style->text.color, nk_rgba(255, 255, 255, 255 * ui_alpha));
    nk_label(ui_ctx, label, alignment);
} else {
    char *icon_glyph = va("%.*s", icon_len, icon);

// @todo: implement nk_push_layout()
//  nk_rect bounds = {..}; nk_panel_alloc_space(bounds, ctx);
    struct nk_window *win = ui_ctx->current;
    struct nk_panel *layout = win->layout, copy = *layout;
    struct nk_rect before; nk_layout_peek(&before, ui_ctx);
    nk_label_colored(ui_ctx, icon_glyph, alignment, nk_rgba(255, 255, 255, (64 + 192 * ui_label_icon_highlight) * ui_alpha) );
    struct nk_rect after; nk_layout_peek(&after, ui_ctx);
    *layout = copy;
    layout->at_x += icon_width_px; layout->bounds.w -= icon_width_px; // nk_layout_space_push(ui_ctx, nk_rect(0,0,icon_width_px,0));

    // set bold style and color if needed
    if( font && nk_style_push_font(ui_ctx, &font->handle) ) {} else font = 0;
    if( font )  nk_style_push_color(ui_ctx, &style->text.color, nk_rgba(255, 255, 255, 255 * ui_alpha));
    nk_label(ui_ctx, icon+icon_len, alignment);

    layout->at_x -= icon_width_px; layout->bounds.w += icon_width_px;
}

if( font )  nk_style_pop_color(ui_ctx);
if( font )  nk_style_pop_font(ui_ctx);

            if (split && is_hovering && !ui_has_active_popups && nk_window_has_focus(ui_ctx)) {
                nk_tooltip(ui_ctx, split + 1); // @fixme: not working under ui_disable() state
            }

    layout->at_x -= spacing;
    layout->bounds.w += spacing;

    // old way
    // ui_labeicon_l_icked_L.x = is_hovering ? nk_input_has_mouse_click_down_in_rect(input, NK_BUTTON_LEFT, layout->bounds, nk_true) : 0;
    // new way
    // this is an ugly hack to detect which icon (within a label) we're clicking on.
    // @todo: figure out a better way to detect this... would it be better to have a ui_label_toolbar(lbl,bar) helper function instead?
    ui_label_icon_clicked_L.x = is_hovering ? ( (int)((input->mouse.pos.x - bounds.x) - (alignment == NK_TEXT_RIGHT ? bounds.w : 0) ) * nk_input_is_mouse_released(input, NK_BUTTON_LEFT)) : 0;

    return ui_label_icon_clicked_L.x;
}

int ui_label(const char *label) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    int align = label[0] == '>' ? (label++, NK_TEXT_RIGHT) : label[0] == '=' ? (label++, NK_TEXT_CENTERED) : label[0] == '<' ? (label++, NK_TEXT_LEFT) : NK_TEXT_LEFT;
    nk_layout_row_dynamic(ui_ctx, 0, 1);
    return ui_label_(label, align);
}

static int nk_label_(struct nk_context *ui_ctx, const char *text_, int align2 ) {
    const struct nk_input *input = &ui_ctx->input;
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);
    int is_hovering = nk_input_is_mouse_hovering_rect(input, bounds) && !ui_has_active_popups;
    if( is_hovering ) {
        struct nk_rect winbounds = nk_window_get_bounds(ui_ctx);
        is_hovering &= nk_input_is_mouse_hovering_rect(input, winbounds);
        is_hovering &= nk_window_has_focus(ui_ctx);
    }

        nk_label(ui_ctx, text_, align2);

    // this is an ugly hack to detect which icon (within a label) we're clicking on.
    // @todo: figure out a better way to detect this... would it be better to have a ui_label_toolbar(lbl,bar) helper function instead?
    ui_label_icon_clicked_R.x = is_hovering ? ( (int)((input->mouse.pos.x - bounds.x) - (align2 == NK_TEXT_RIGHT ? bounds.w : 0) ) * nk_input_is_mouse_released(input, NK_BUTTON_LEFT)) : 0;

    return ui_label_icon_clicked_R.x;
}


int ui_label2(const char *label, const char *text_) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);

    int align1 = NK_TEXT_LEFT;
    int align2 = NK_TEXT_LEFT;
    if( label ) align1 = label[0] == '>' ? (label++, NK_TEXT_RIGHT) : label[0] == '=' ? (label++, NK_TEXT_CENTERED) : label[0] == '<' ? (label++, NK_TEXT_LEFT) : NK_TEXT_LEFT;
    if( text_ ) align2 = text_[0] == '>' ? (text_++, NK_TEXT_RIGHT) : text_[0] == '=' ? (text_++, NK_TEXT_CENTERED) : text_[0] == '<' ? (text_++, NK_TEXT_LEFT) : NK_TEXT_LEFT;
    ui_label_(label, align1);

    return nk_label_(ui_ctx, text_, align2);
}
int ui_label2_bool(const char *text, bool value) {
    bool b = !!value;
    return ui_bool(text, &b), 0;
}
int ui_label2_float(const char *text, float value) {
    float f = (float)value;
    return ui_float(text, &f), 0;
}
int ui_label2_wrap(const char *label, const char *str) { // @fixme: does not work (remove dynamic layout?)
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);
    nk_text_wrap(ui_ctx, str, strlen(str));
    return 0;
}
int ui_label2_toolbar(const char *label, const char *icons) {
    int mouse_click = ui_label2(label, va(">%s", icons));
    int choice = !mouse_click ? 0 : 1 + -mouse_click / (UI_ICON_FONTSIZE + UI_ICON_SPACING_X); // divided by px per ICON_MD_ glyph approximately
    int glyphs = strlen(icons) / 3;
    return choice > glyphs ? 0 : choice;
}

int ui_notify(const char *title, const char *body) {
    app_beep();

    struct ui_notify n = {0};
    n.title = title && title[0] ? stringf("*%s", title) : 0;
    n.body = body && body[0] ? STRDUP(body) : 0;
    n.timeout = 2; // 4s = 2s timeout (+ 1s fade + 1s idle)
    n.alpha = 1;
    array_push_front(ui_notifications, n);
    return 1;
}

int ui_button_transparent(const char *text) {
    nk_layout_row_dynamic(ui_ctx, 0, 1);
    int align = text[0] == '>' ? (text++, NK_TEXT_RIGHT) : text[0] == '=' ? (text++, NK_TEXT_CENTERED) : text[0] == '<' ? (text++, NK_TEXT_LEFT) : NK_TEXT_CENTERED;
    return !!nk_contextual_item_label(ui_ctx, text, align);
}

#ifndef UI_BUTTON_MONOCHROME
#define UI_BUTTON_MONOCHROME 0
#endif

static
int ui_button_(const char *label) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    if( 1 ) {
#if UI_BUTTON_MONOCHROME
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_normal, nk_rgba(0,0,0,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_hover,  nk_rgba(0,0,0,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_active, nk_rgba(0,0,0,ui_alpha));

        nk_style_push_color(ui_ctx, &ui_ctx->style.button.normal.data.color, nk_hsva_f(ui_hue,0.0,0.8*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.hover.data.color,  nk_hsva_f(ui_hue,0.0,1.0*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.active.data.color, nk_hsva_f(ui_hue,0.0,0.4*ui_alpha));
#elif 0 // old
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_normal, nk_rgba(0,0,0,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_hover,  nk_rgba(0,0,0,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_active, nk_rgba(0,0,0,ui_alpha));

        nk_style_push_color(ui_ctx, &ui_ctx->style.button.normal.data.color, nk_hsva_f(ui_hue,0.75,0.8*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.hover.data.color,  nk_hsva_f(ui_hue,1.00,1.0*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.active.data.color, nk_hsva_f(ui_hue,0.60,0.4*ui_alpha));
#else // new
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_normal, nk_rgba_f(0.00,0.00,0.00,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_hover,  nk_rgba_f(0.11,0.11,0.11,ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.text_active, nk_rgba_f(0.00,0.00,0.00,ui_alpha));

        nk_style_push_color(ui_ctx, &ui_ctx->style.button.normal.data.color, nk_hsva_f(ui_hue,0.80,0.6,0.90*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.hover.data.color,  nk_hsva_f(ui_hue,0.85,0.9,0.90*ui_alpha));
        nk_style_push_color(ui_ctx, &ui_ctx->style.button.active.data.color, nk_hsva_f(ui_hue,0.80,0.6,0.90*ui_alpha));
#endif
    }

    struct nk_rect bounds = nk_widget_bounds(ui_ctx);

    const char *split = strchr(label, '@'), *tooltip = split + 1;
    int ret = nk_button_text(ui_ctx, label, split ? (int)(split - label) : strlen(label) );

    const struct nk_input *in = &ui_ctx->input;
    if (split && nk_input_is_mouse_hovering_rect(in, bounds) && !ui_has_active_popups && nk_window_has_focus(ui_ctx)) {
        nk_tooltip(ui_ctx, tooltip);
    }

    if( 1 ) {
        nk_style_pop_color(ui_ctx);
        nk_style_pop_color(ui_ctx);
        nk_style_pop_color(ui_ctx);

        nk_style_pop_color(ui_ctx);
        nk_style_pop_color(ui_ctx);
        nk_style_pop_color(ui_ctx);
    }

    return ret;
}

int ui_buttons(int buttons, ...) {
    static array(char*) args = 0;
    array_resize(args, 0);

    int num_skips = 0;
        va_list list;
        va_start(list, buttons);
        for( int i = 0; i < buttons; ++i ) {
            const char *label = va_arg(list, const char*);
            int skip = ui_filter && ui_filter[0] && !strstri(label, ui_filter);
            array_push(args, skip ? NULL : (char*)label);
            num_skips += skip;
        }
        va_end(list);

    if( num_skips == array_count(args) ) return 0;
    buttons = array_count(args) - num_skips;

    nk_layout_row_dynamic(ui_ctx, 0, buttons);

    float ui_hue_old = ui_hue;

        int indent = 8;
        struct nk_window *win = ui_ctx->current;
        struct nk_panel *layout = win->layout;
        struct nk_panel copy = *layout;
        ui_label_("", NK_TEXT_LEFT);
        *layout = copy;
        layout->at_x += indent;
        layout->bounds.w -= indent;

            int rc = 0;
            for( int i = 0, end = array_count(args); i < end; ++i ) {
                if( args[i] && ui_button_( args[i] ) ) rc = i+1;
                ui_hue_cycle( 3 );
            }
            va_end(list);

        layout->at_x -= indent;
        layout->bounds.w += indent;

    ui_hue = ui_hue_old;
    return rc;
}

int ui_button(const char *s) {
    return ui_buttons(1, s);
}

int ui_toggle(const char *label, bool *value) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    // nk_label(ui_ctx, label, alignment);
    int rc = nk_button_transparent(ui_ctx, *value ? ICON_MD_TOGGLE_ON : ICON_MD_TOGGLE_OFF);
    return rc ? (*value ^= 1), rc : rc;
}

static enum color_mode {COL_RGB, COL_HSV} ui_color_mode = COL_RGB;

int ui_color4f(const char *label, float *color) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_colorf after = { color[0]*ui_alpha, color[1]*ui_alpha, color[2]*ui_alpha, color[3]*ui_alpha }, before = after;
    struct nk_colorf clamped = { clampf(after.r,0,1), clampf(after.g,0,1), clampf(after.b,0,1), clampf(after.a,0,1) };
    if (nk_combo_begin_color(ui_ctx, nk_rgb_cf(clamped), nk_vec2(200,400))) {
        nk_layout_row_dynamic(ui_ctx, 120, 1);
        after = nk_color_picker(ui_ctx, after, NK_RGB);

        nk_layout_row_dynamic(ui_ctx, 0, 2);
        ui_color_mode = nk_option_label(ui_ctx, "RGB", ui_color_mode == COL_RGB) ? COL_RGB : ui_color_mode;
        ui_color_mode = nk_option_label(ui_ctx, "HSV", ui_color_mode == COL_HSV) ? COL_HSV : ui_color_mode;

        nk_layout_row_dynamic(ui_ctx, 0, 1);
        if (ui_color_mode == COL_RGB) {
            after.r = nk_propertyf(ui_ctx, "#R:", -FLT_MAX, after.r, FLT_MAX, 0.01f,0.005f);
            after.g = nk_propertyf(ui_ctx, "#G:", -FLT_MAX, after.g, FLT_MAX, 0.01f,0.005f);
            after.b = nk_propertyf(ui_ctx, "#B:", -FLT_MAX, after.b, FLT_MAX, 0.01f,0.005f);
        } else {
            float hsva[4];
            nk_colorf_hsva_fv(hsva, after);
            hsva[0] = nk_propertyf(ui_ctx, "#H:", -FLT_MAX, hsva[0], FLT_MAX, 0.01f,0.005f);
            hsva[1] = nk_propertyf(ui_ctx, "#S:", -FLT_MAX, hsva[1], FLT_MAX, 0.01f,0.005f);
            hsva[2] = nk_propertyf(ui_ctx, "#V:", -FLT_MAX, hsva[2], FLT_MAX, 0.01f,0.005f);
            after = nk_hsva_colorfv(hsva);
        }
        nk_label(ui_ctx, va("#%02X%02X%02X", (unsigned)clampf(after.r*255,0,255), (unsigned)clampf(after.g*255,0,255), (unsigned)clampf(after.b*255,0,255)), NK_TEXT_CENTERED);

        color[0] = after.r;
        color[1] = after.g;
        color[2] = after.b;
        color[3] = after.a;

        nk_combo_end(ui_ctx);
    }
    return !!memcmp(&before.r, &after.r, sizeof(struct nk_colorf));
}
int ui_color4(const char *label, unsigned *color) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    unsigned a = *color >> 24;
    unsigned b =(*color >> 16)&255;
    unsigned g =(*color >> 8)&255;
    unsigned r = *color & 255;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_colorf after = { r*ui_alpha/255, g*ui_alpha/255, b*ui_alpha/255, a*ui_alpha/255 }, before = after;
    if (nk_combo_begin_color(ui_ctx, nk_rgb_cf(after), nk_vec2(200,400))) {
        nk_layout_row_dynamic(ui_ctx, 120, 1);
        after = nk_color_picker(ui_ctx, after, NK_RGBA);

        nk_layout_row_dynamic(ui_ctx, 0, 2);
        ui_color_mode = nk_option_label(ui_ctx, "RGB", ui_color_mode == COL_RGB) ? COL_RGB : ui_color_mode;
        ui_color_mode = nk_option_label(ui_ctx, "HSV", ui_color_mode == COL_HSV) ? COL_HSV : ui_color_mode;

        nk_layout_row_dynamic(ui_ctx, 0, 1);
        if (ui_color_mode == COL_RGB) {
            after.r = nk_propertyi(ui_ctx, "#R:", 0, after.r * 255, 255, 1,1) / 255.f;
            after.g = nk_propertyi(ui_ctx, "#G:", 0, after.g * 255, 255, 1,1) / 255.f;
            after.b = nk_propertyi(ui_ctx, "#B:", 0, after.b * 255, 255, 1,1) / 255.f;
            after.a = nk_propertyi(ui_ctx, "#A:", 0, after.a * 255, 255, 1,1) / 255.f;
        } else {
            float hsva[4];
            nk_colorf_hsva_fv(hsva, after);
            hsva[0] = nk_propertyi(ui_ctx, "#H:", 0, hsva[0] * 255, 255, 1,1) / 255.f;
            hsva[1] = nk_propertyi(ui_ctx, "#S:", 0, hsva[1] * 255, 255, 1,1) / 255.f;
            hsva[2] = nk_propertyi(ui_ctx, "#V:", 0, hsva[2] * 255, 255, 1,1) / 255.f;
            hsva[3] = nk_propertyi(ui_ctx, "#A:", 0, hsva[3] * 255, 255, 1,1) / 255.f;
            after = nk_hsva_colorfv(hsva);
        }
        r = after.r * 255;
        g = after.g * 255;
        b = after.b * 255;
        a = after.a * 255;
        *color = rgba(r,g,b,a);

        nk_label(ui_ctx, va("#%02X%02X%02X%02X", r, g, b, a), NK_TEXT_CENTERED);

        nk_combo_end(ui_ctx);
    }
    return !!memcmp(&before.r, &after.r, sizeof(struct nk_colorf));
}

int ui_color3f(const char *label, float *color) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_colorf after = { color[0]*ui_alpha, color[1]*ui_alpha, color[2]*ui_alpha, 1.0f }, before = after;
    struct nk_colorf clamped = { clampf(after.r,0,1), clampf(after.g,0,1), clampf(after.b,0,1), ui_alpha };
    if (nk_combo_begin_color(ui_ctx, nk_rgb_cf(clamped), nk_vec2(200,400))) {
        nk_layout_row_dynamic(ui_ctx, 120, 1);
        after = nk_color_picker(ui_ctx, after, NK_RGB);

        nk_layout_row_dynamic(ui_ctx, 0, 2);
        ui_color_mode = nk_option_label(ui_ctx, "RGB", ui_color_mode == COL_RGB) ? COL_RGB : ui_color_mode;
        ui_color_mode = nk_option_label(ui_ctx, "HSV", ui_color_mode == COL_HSV) ? COL_HSV : ui_color_mode;

        nk_layout_row_dynamic(ui_ctx, 0, 1);
        if (ui_color_mode == COL_RGB) {
            after.r = nk_propertyf(ui_ctx, "#R:", -FLT_MAX, after.r, FLT_MAX, 0.01f,0.005f);
            after.g = nk_propertyf(ui_ctx, "#G:", -FLT_MAX, after.g, FLT_MAX, 0.01f,0.005f);
            after.b = nk_propertyf(ui_ctx, "#B:", -FLT_MAX, after.b, FLT_MAX, 0.01f,0.005f);
        } else {
            float hsva[4];
            nk_colorf_hsva_fv(hsva, after);
            hsva[0] = nk_propertyf(ui_ctx, "#H:", -FLT_MAX, hsva[0], FLT_MAX, 0.01f,0.005f);
            hsva[1] = nk_propertyf(ui_ctx, "#S:", -FLT_MAX, hsva[1], FLT_MAX, 0.01f,0.005f);
            hsva[2] = nk_propertyf(ui_ctx, "#V:", -FLT_MAX, hsva[2], FLT_MAX, 0.01f,0.005f);
            after = nk_hsva_colorfv(hsva);
        }
        nk_label(ui_ctx, va("#%02X%02X%02X", (unsigned)clampf(after.r*255,0,255), (unsigned)clampf(after.g*255,0,255), (unsigned)clampf(after.b*255,0,255)), NK_TEXT_CENTERED);

        color[0] = after.r;
        color[1] = after.g;
        color[2] = after.b;

        nk_combo_end(ui_ctx);
    }
    return !!memcmp(&before.r, &after.r, sizeof(struct nk_colorf));
}
int ui_color3(const char *label, unsigned *color) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    unsigned a = *color >> 24;
    unsigned b =(*color >> 16)&255;
    unsigned g =(*color >> 8)&255;
    unsigned r = *color & 255;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_colorf after = { r*ui_alpha/255, g*ui_alpha/255, b*ui_alpha/255, ui_alpha }, before = after;
    if (nk_combo_begin_color(ui_ctx, nk_rgb_cf(after), nk_vec2(200,400))) {
        nk_layout_row_dynamic(ui_ctx, 120, 1);
        after = nk_color_picker(ui_ctx, after, NK_RGB);

        nk_layout_row_dynamic(ui_ctx, 0, 2);
        ui_color_mode = nk_option_label(ui_ctx, "RGB", ui_color_mode == COL_RGB) ? COL_RGB : ui_color_mode;
        ui_color_mode = nk_option_label(ui_ctx, "HSV", ui_color_mode == COL_HSV) ? COL_HSV : ui_color_mode;

        nk_layout_row_dynamic(ui_ctx, 0, 1);
        if (ui_color_mode == COL_RGB) {
            after.r = nk_propertyi(ui_ctx, "#R:", 0, after.r * 255, 255, 1,1) / 255.f;
            after.g = nk_propertyi(ui_ctx, "#G:", 0, after.g * 255, 255, 1,1) / 255.f;
            after.b = nk_propertyi(ui_ctx, "#B:", 0, after.b * 255, 255, 1,1) / 255.f;
        } else {
            float hsva[4];
            nk_colorf_hsva_fv(hsva, after);
            hsva[0] = nk_propertyi(ui_ctx, "#H:", 0, hsva[0] * 255, 255, 1,1) / 255.f;
            hsva[1] = nk_propertyi(ui_ctx, "#S:", 0, hsva[1] * 255, 255, 1,1) / 255.f;
            hsva[2] = nk_propertyi(ui_ctx, "#V:", 0, hsva[2] * 255, 255, 1,1) / 255.f;
            after = nk_hsva_colorfv(hsva);
        }
        r = after.r * 255;
        g = after.g * 255;
        b = after.b * 255;
        *color = rgba(r,g,b,a);

        nk_label(ui_ctx, va("#%02X%02X%02X", r, g, b), NK_TEXT_CENTERED);

        nk_combo_end(ui_ctx);
    }
    return !!memcmp(&before.r, &after.r, sizeof(struct nk_colorf));
}

int ui_list(const char *label, const char **items, int num_items, int *selector) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int val = nk_combo(ui_ctx, items, num_items, *selector, UI_ROW_HEIGHT, nk_vec2(200,200));
    int chg = val != *selector;
    *selector = val;
    return chg;
}

int ui_slider(const char *label, float *slider) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    // return ui_slider2(label, slider, va("%.2f ", *slider));
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    nk_size val = *slider * 1000;
    int chg = nk_progress(ui_ctx, &val, 1000, NK_MODIFIABLE);
    *slider = val / 1000.f;
    return chg;
}
int ui_slider2(const char *label, float *slider, const char *caption) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_window *win = ui_ctx->current;
    const struct nk_style *style = &ui_ctx->style;
    struct nk_rect bounds; nk_layout_peek(&bounds, ui_ctx); bounds.w -= 10; // bounds.w *= 0.95f;
    struct nk_vec2 item_padding = style->text.padding;
    struct nk_text text;
    text.padding.x = item_padding.x;
    text.padding.y = item_padding.y;
    text.background = style->window.background;
    text.text = nk_rgba_f(1,1,1,ui_alpha);

        nk_size val = *slider * 1000;
        int chg = nk_progress(ui_ctx, &val, 1000, NK_MODIFIABLE);
        *slider = val / 1000.f;

    chg |= input(MOUSE_L) && nk_input_is_mouse_hovering_rect(&ui_ctx->input, bounds); // , true);

    nk_widget_text(&win->buffer, bounds, caption, strlen(caption), &text, NK_TEXT_RIGHT, style->font);
    return chg;
}

int ui_bool(const char *label, bool *enabled ) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int val = *enabled;
#if 0
    int chg = !!nk_checkbox_label(ui_ctx, "", &val);
#else
    int chg = !!nk_button_transparent(ui_ctx, val ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK );
#endif
    *enabled ^= chg;
    return chg;
}

static int ui_num_signs = 0;

int ui_int(const char *label, int *v) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int prev = *v;
    *v = nk_propertyi(ui_ctx, "#", INT_MIN, *v, INT_MAX, 1,1);
    return prev != *v;
}

int ui_unsigned(const char *label, unsigned *v) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    unsigned prev = *v;
    *v = (unsigned)nk_propertyd(ui_ctx, "#", 0, *v, UINT_MAX, 1,1);
    return prev != *v;
}
int ui_unsigned2(const char *label, unsigned *v) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = ui_num_signs ?
        --ui_num_signs, va("+%2u +%2u", v[0], v[1]) :
        va("%2u, %2u", v[0], v[1]);

    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 0, 1);
        unsigned prev0 = v[0]; nk_property_int(ui_ctx, "#X:", 0, &v[0], INT_MAX, 1,0.5f);
        unsigned prev1 = v[1]; nk_property_int(ui_ctx, "#Y:", 0, &v[1], INT_MAX, 1,0.5f);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1];
    }
    return 0;
}
int ui_unsigned3(const char *label, unsigned *v) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = ui_num_signs ?
        --ui_num_signs, va("+%2u +%2u +%2u", v[0], v[1], v[2]) :
        va("%2u, %2u, %2u", v[0], v[1], v[2]);

    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 0, 1);
        unsigned prev0 = v[0]; nk_property_int(ui_ctx, "#X:", 0, &v[0], INT_MAX, 1,0.5f);
        unsigned prev1 = v[1]; nk_property_int(ui_ctx, "#Y:", 0, &v[1], INT_MAX, 1,0.5f);
        unsigned prev2 = v[2]; nk_property_int(ui_ctx, "#Z:", 0, &v[2], INT_MAX, 1,0.5f);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1] || prev2 != v[2];
    }
    return 0;
}

int ui_short(const char *label, short *v) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    int i = *v, ret = ui_int( label, &i );
    return *v = (short)i, ret;
}

static inline
float ui_float_adjust_step(float step) {
    if ((input(KEY_LSHIFT) || input(KEY_RSHIFT))&&(input(KEY_LCTRL) || input(KEY_RCTRL))) step *= 100.0f;
    else if (input(KEY_LSHIFT) || input(KEY_RSHIFT)) step *= 10.0f;
    else if (input(KEY_LCTRL) || input(KEY_RCTRL)) step *= 0.1f;
    else if (input(KEY_LALT) || input(KEY_RALT)) step *= 0.01f;
    return step;
}

int ui_float_(const char *label, float *v, float step) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    float inc_per_pixel = ui_float_adjust_step(0.005f);
    step = ui_float_adjust_step(step);

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    float prev = v[0]; v[0] = nk_propertyf(ui_ctx, "#", -FLT_MAX, v[0], FLT_MAX, step, inc_per_pixel);
    return prev != v[0];
}

int ui_float(const char *label, float *v) {
    return ui_float_(label, v, 0.01f);
}

int ui_double_(const char *label, double *v, double step) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    float inc_per_pixel = ui_float_adjust_step(0.005f);
    step = ui_float_adjust_step(step);

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    double prev = v[0]; v[0] = nk_propertyd(ui_ctx, "#", -DBL_MAX, v[0], DBL_MAX, step, inc_per_pixel);
    return prev != v[0];
}

int ui_double(const char *label, double *v) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    double prev = v[0]; v[0] = nk_propertyd(ui_ctx, "#", -DBL_MAX, v[0], DBL_MAX, 0.01f,0.005f);
    return prev != v[0];
}

int ui_clampf_(const char *label, float *v, float minf, float maxf, float step) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    float inc_per_pixel = ui_float_adjust_step(0.005f);
    step = ui_float_adjust_step(step);

    if( minf > maxf ) return ui_clampf(label, v, maxf, minf);

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    float prev = v[0]; v[0] = nk_propertyf(ui_ctx, "#", minf, v[0], maxf, step, inc_per_pixel);
    return prev != v[0];
}

int ui_clampf(const char *label, float *v, float minf, float maxf) {
    return ui_clampf_(label, v, minf, maxf, 0.1f);
}

int ui_float2_(const char *label, float *v, float step) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    float inc_per_pixel = ui_float_adjust_step(0.5f);
    step = ui_float_adjust_step(step);
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = ui_num_signs ?
        --ui_num_signs, va("%+.3f %+.3f", v[0], v[1]) :
        va("%.3f, %.3f", v[0], v[1]);

    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 0, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, step, inc_per_pixel);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, step, inc_per_pixel);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1];
    }
    return 0;
}

int ui_float2(const char *label, float *v) {
    return ui_float2_(label, v, 1.0f);
}

int ui_float3_(const char *label, float *v, float step) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    float inc_per_pixel = ui_float_adjust_step(0.5f);
    step = ui_float_adjust_step(step);
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = ui_num_signs ?
        --ui_num_signs, va("%+.2f %+.2f %+.2f", v[0], v[1], v[2]) :
        va("%.2f, %.2f, %.2f", v[0], v[1], v[2]);

    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 0, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, step, inc_per_pixel);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, step, inc_per_pixel);
        float prev2 = v[2]; nk_property_float(ui_ctx, "#Z:", -FLT_MAX, &v[2], FLT_MAX, step, inc_per_pixel);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1] || prev2 != v[2];
    }
    return 0;
}


int ui_float3(const char *label, float *v) {
    return ui_float3_(label, v, 1.0f);
}

int ui_float4_(const char *label, float *v, float step) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    float inc_per_pixel = ui_float_adjust_step(0.5f);
    step = ui_float_adjust_step(step);
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = ui_num_signs ?
        --ui_num_signs, va("%+.2f %+.2f %+.2f %+.2f", v[0], v[1], v[2], v[3]) :
        va("%.2f,%.2f,%.2f,%.2f", v[0], v[1], v[2], v[3]);

    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 0, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, step, inc_per_pixel);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, step, inc_per_pixel);
        float prev2 = v[2]; nk_property_float(ui_ctx, "#Z:", -FLT_MAX, &v[2], FLT_MAX, step, inc_per_pixel);
        float prev3 = v[3]; nk_property_float(ui_ctx, "#W:", -FLT_MAX, &v[3], FLT_MAX, step, inc_per_pixel);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1] || prev2 != v[2] || prev3 != v[3];
    }

    return 0;
}

int ui_float4(const char *label, float *v) {
    return ui_float4_(label, v, 1.0f);
}

int ui_mat33(const char *label, float M[9]) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    ui_num_signs = 3;
    int changed = 0;
    changed |= ui_label(label);
    changed |= ui_float3(NULL, M);
    changed |= ui_float3(NULL, M+3);
    changed |= ui_float3(NULL, M+6);
    return changed;
}
int ui_mat34(const char *label, float M[12]) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    ui_num_signs = 3;
    int changed = 0;
    changed |= ui_label(label);
    changed |= ui_float4(NULL, M);
    changed |= ui_float4(NULL, M+4);
    changed |= ui_float4(NULL, M+8);
    return changed;
}
int ui_mat44(const char *label, float M[16]) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    ui_num_signs = 4;
    int changed = 0;
    changed |= ui_label(label);
    changed |= ui_float4(NULL, M);
    changed |= ui_float4(NULL, M+4);
    changed |= ui_float4(NULL, M+8);
    changed |= ui_float4(NULL, M+12);
    return changed;
}

int ui_buffer(const char *label, char *buffer, int buflen) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, 0, 1 + (label && label[0]));
    if(label && label[0]) ui_label_(label, NK_TEXT_LEFT);

    int active = nk_edit_string_zero_terminated(ui_ctx, NK_EDIT_AUTO_SELECT|NK_EDIT_CLIPBOARD|NK_EDIT_FIELD/*NK_EDIT_BOX*/|NK_EDIT_SIG_ENTER, buffer, buflen, nk_filter_default);
    return !!(active & NK_EDIT_COMMITED) ? nk_edit_unfocus(ui_ctx), 1 : 0;
}

int ui_string(const char *label, char **str) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    char *bak = va("%s%c", *str ? *str : "", '\0');
    int rc = ui_buffer(label, bak, strlen(bak)+2);
    if( *str ) 0[*str] = '\0';
    strcatf(str, "%s", bak);
    return rc;
}

int ui_separator() {
    if( /*label &&*/ ui_filter && ui_filter[0] ) return 0; // if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, UI_SEPARATOR_HEIGHT, 1);

    ui_hue_cycle( 1 );

    struct nk_command_buffer *canvas;
    struct nk_input *input = &ui_ctx->input;
    canvas = nk_window_get_canvas(ui_ctx);
    struct nk_rect space;
    enum nk_widget_layout_states state;
    state = nk_widget(&space, ui_ctx);
    if (state) nk_fill_rect(canvas, space, 0, ui_ctx->style.window.header.normal.data.color );

    return 1;
}

int ui_subimage(const char *label, handle id, unsigned iw, unsigned ih, unsigned sx, unsigned sy, unsigned sw, unsigned sh) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    nk_layout_row_dynamic(ui_ctx, sh < 30 || id == texture_checker().id ? 0 : sh, 1 + (label && label[0]));
    if( label && label[0] ) ui_label_(label, NK_TEXT_LEFT);

    struct nk_rect bounds; nk_layout_peek(&bounds, ui_ctx); bounds.w -= 10; // bounds.w *= 0.95f;

    int ret = nk_button_image_styled(ui_ctx, &ui_ctx->style.button, nk_subimage_id((int)id, iw, ih, nk_rect(sx,sy,sw,sh)));
    if( !ret ) {
        ret |= input(MOUSE_L) && nk_input_is_mouse_hovering_rect(&ui_ctx->input, bounds); // , true);
    }
    if( ret ) {
        int px = 100 * (ui_ctx->input.mouse.pos.x - bounds.x ) / (float)bounds.w;
        int py = 100 * (ui_ctx->input.mouse.pos.y - bounds.y ) / (float)bounds.h;
        return px * 100 + py; // printf("%d %d xy:%d\n", px, py, (px * 100) + py);
    }
    return ret; // !!ret;
}

int ui_image(const char *label, handle id, unsigned w, unsigned h) {
    return ui_subimage(label, id, w,h, 0,0,w,h);
}

int ui_texture(const char *label, texture_t t) {
    return ui_subimage(label, t.id, t.w,t.h, 0,0,t.w,t.h);
}

int ui_subtexture(const char *label, texture_t t, unsigned x, unsigned y, unsigned w, unsigned h) {
    return ui_subimage(label, t.id, t.w,t.h, x,y,w,h);
}

int ui_colormap( const char *label, colormap_t *cm ) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    int ret = 0;
    if( !cm->texture ) {
        const char *title = va("%s (no image)", label);
        if( ui_image( title, texture_checker().id, 0,0 ) ) {
            ret = 2;
        }
    } else {
        unsigned w = cm->texture->w, h = cm->texture->h;
        ui_label(va("%s (%s)", label, cm->texture->filename) ); // @fixme: labelf would crash?

        const char *fmt[] = { "", "R", "RG", "RGB", "RGBA" };
        const char *title = va("%s %dx%d %s", label, w, h, fmt[cm->texture->n]);
        if( ui_image( title, cm->texture->id, 128, 128 ) ) {
            ret = 2;
        }
    }

    if( ui_color4f( va("%s Color", label), (float *) &cm->color ) ) {
        ret = 1;
    }
    return ret;
}

int ui_radio(const char *label, const char **items, int num_items, int *selector) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    int ret = 0;
    if( label && label[0] ) ui_label(label);
    for( int i = 0; i < num_items; i++ ) {
        bool enabled = *selector == i;
        if( ui_bool( va("%s%s", label && label[0] ? "  ":"", items[i]), &enabled ) ) {
            *selector = i;
            ret |= 1;
        }
    }
    return ret;
}

int ui_section(const char *label) {
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    //ui_separator();
    return ui_label(va("*%s", label));
}

int ui_dialog(const char *label, const char *text, int choices, bool *show) { // @fixme: return
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0;

    ui_has_active_popups = 0;
    if(*show) {
        static struct nk_rect s = {0, 0, 300, 190};
        if (nk_popup_begin(ui_ctx, NK_POPUP_STATIC, label, NK_WINDOW_BORDER|NK_WINDOW_CLOSABLE, s)) {
            nk_layout_row_dynamic(ui_ctx, 20, 1);
            for( char t[1024]; *text && sscanf(text, "%[^\r\n]", t); ) {
                nk_label(ui_ctx, t, NK_TEXT_LEFT);
                text += strlen(t); while(*text && *text < 32) text++;
            }

            if( choices ) {
                if( ui_buttons(choices > 1 ? 2 : 1, "OK", "Cancel") ) {
                    *show = 0;
                    nk_popup_close(ui_ctx);
                }
            }

            nk_popup_end(ui_ctx);
        } else {
            *show = nk_false;
        }
        ui_has_active_popups = *show;
    }
    return *show;
}

#define ui_bitmask_template(X) \
int ui_bitmask##X(const char *label, uint##X##_t *enabled) { \
    if( label && ui_filter && ui_filter[0] ) if( !strstri(label, ui_filter) ) return 0; \
\
    /* @fixme: better way to retrieve widget width? nk_layout_row_dynamic() seems excessive */ \
    nk_layout_row_dynamic(ui_ctx, 1, 1); \
    struct nk_rect bounds = nk_widget_bounds(ui_ctx); \
\
    /* actual widget: label + X checkboxes */ \
    nk_layout_row_begin(ui_ctx, NK_STATIC, 0, 1+X); \
\
        int offset = bounds.w > (15*X) ? bounds.w - (15*X) : 0; /* bits widget below needs at least 118px wide */ \
        nk_layout_row_push(ui_ctx, offset); \
        ui_label_(label, NK_TEXT_LEFT); \
\
        uint8_t copy = *enabled; \
        for( int i = 0; i < X; ++i ) { \
            int b = (X-1-i); \
            nk_layout_row_push(ui_ctx, 10); \
            /* bit */ \
            int val = (*enabled >> b) & 1; \
            int chg = nk_checkbox_label(ui_ctx, "", &val); \
            *enabled = (*enabled & ~(1 << b)) | ((!!val) << b); \
            /* tooltip */ \
            struct nk_rect bb = { offset + 10 + i * 14, bounds.y, 14, 30 }; /* 10:padding,14:width,30:height */ \
            if (nk_input_is_mouse_hovering_rect(&ui_ctx->input, bb) && !ui_has_active_popups && nk_window_has_focus(ui_ctx)) { \
                nk_tooltipf(ui_ctx, "Bit %d", b); \
            } \
        } \
\
    nk_layout_row_end(ui_ctx); \
    return copy ^ *enabled; \
}

ui_bitmask_template(8);
ui_bitmask_template(16);
//ui_bitmask_template(32);

int ui_console() { // @fixme: buggy
    static char *cmd = 0;
    static int enter = 0;

    struct nk_font *font = nk_glfw.atlas.fonts->next /*2nd font*/;
    if( font && nk_style_push_font(ui_ctx, &font->handle) ) {} else font = 0;

        struct nk_rect bounds = {0,0,400,300}; // @fixme: how to retrieve inlined region below? (inlined)
        if( 1 ) /*if( windowed || (!windowed && *inlined) )*/ bounds = nk_window_get_content_region(ui_ctx);
        else { struct nk_rect b; nk_layout_peek(&b, ui_ctx); bounds.w = b.w; }

    nk_layout_row_static(ui_ctx, bounds.h - UI_ROW_HEIGHT, bounds.w, 1);

    static array(char*) lines = 0;
    for( int i = 0; i < array_count(lines); ++i )
    if(lines[i]) nk_label_wrap(ui_ctx, lines[i]); // "This is a very long line to hopefully get this text to be wrapped into multiple lines to show line wrapping");

    if( enter ) {
        array_push(lines, 0);

        for( FILE *fp = popen(cmd, "r"); fp; pclose(fp), fp = 0) {
            int ch;
            do {
                ch = fgetc(fp);
                if( strchr("\r\n\t\b", ch) ) {
                    array_push(lines,0);
                    continue;
                }
                if( ch >= ' ' ) strcatf(array_back(lines), "%c", ch);
            } while(ch > 0);
        }

        cmd[0] = 0;
    }

    enter = ui_string("", &cmd);

    if( font )  nk_style_pop_font(ui_ctx);

    return enter;
}

int ui_browse(const char **output, bool *inlined) {
    static struct browser_media media = {0};
    static struct browser browsers[2] = {0}; // 2 instances max: 0=inlined, 1=windowed
    static char *results[2] = {0}; // 2 instances max: 0=inlined, 1=windowed
    do_once {
        const int W = 96, H = 96; // 2048x481 px, 21x5 cells
        texture_t i = texture("icons/suru.png", TEXTURE_RGBA|TEXTURE_MIPMAPS);
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H, 16, 3), BROWSER_FOLDER); // default group
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H,  2, 4), BROWSER_HOME);
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H, 17, 3), BROWSER_COMPUTER);
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H,  1, 4), BROWSER_PROJECT);
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H,  0, 4), BROWSER_DESKTOP);

        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  8, 0), "");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 10, 2), ".txt.md.doc.license" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  8, 1), ".ini.cfg" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  8, 3), ".xlsx" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  9, 0), ".c" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  4, 1), ".h.hpp.hh.hxx" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  4, 2), ".fs.vs.gs.fx.glsl.shader" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 12, 0), ".cpp.cc.cxx" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 15, 0), ".json" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 15, 2), ".bat.sh" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  6, 1), ".htm.html" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 20, 1), ".xml" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 12, 1), ".js" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  0, 3), ".ts" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  6, 2), ".py" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 16, 1), ".lua" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 10, 0), ".css.doc" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  6, 0), ".wav.flac.ogg.mp1.mp3.mod.xm.s3m.it.sfxr.mid.fur" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  1, 3), ".ttf.ttc.otf" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  7, 1), ".jpg.jpeg.png.bmp.psd.pic.pnm.ico.ktx.pvr.dds.astc.basis.hdr.tga.gif" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  4, 3), ".mp4.mpg.ogv.mkv.wmv.avi" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  2, 1), ".iqm.iqe.gltf.gltf2.glb.fbx.obj.dae.blend.md3.md5.ms3d.smd.x.3ds.bvh.dxf.lwo" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  0, 1), ".exe" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  7, 0), ".bin.dSYM.pdb.o.lib.dll" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 15, 3), ".zip.rar.7z.pak" ".");

        for( int j = 0; j < countof(browsers); ++j ) browser_init(&browsers[j]);
        browsers[0].listing = 1; // inlined version got listing by default, as there is not much space for layouting
    }
    // at_exit: browser_free(&browser);

    int clicked = 0;
    bool windowed = !inlined;
    if( windowed || (!windowed && *inlined) ) {

        struct browser *browser = browsers + windowed; // select instance
        char **result = results + windowed; // select instance

        struct nk_rect bounds = {0}; // // {0,0,400,300};
        bounds = nk_window_get_content_region(ui_ctx);
        if( !windowed && *inlined ) bounds.h *= 0.80;

        clicked = browser_run(ui_ctx, browser, windowed, bounds);
        if( clicked ) {
            strcatf(result, "%d", 0);
            (*result)[0] = '\0';
            strcatf(result, "%s", browser->file);
            if( inlined ) *inlined = 0;

            const char *target = ifdef(win32, "/", "\\"), *replace = ifdef(win32, "\\", "/");
            strswap(*result, target, replace);

            if( output ) *output = *result;
        }
    }

    return clicked;
}

/*
//  demo:
    static const char *file;
    if( ui_panel("inlined", 0)) {
        static bool show_browser = 0;
        if( ui_button("my button") ) { show_browser = true; }
        if( ui_browse(&file, &show_browser) ) puts(file);
        ui_panel_end();
    }
    if( ui_window("windowed", 0) ) {
        if( ui_browse(&file, NULL) ) puts(file);
        ui_window_end();
    }
*/

// ----------------------------------------------------------------------------

int ui_demo(int do_windows) {
    static int integer = 42;
    static bool toggle = true;
    static bool boolean = true;
    static float floating = 3.14159;
    static float float2[2] = {1,2};
    static float float3[3] = {1,2,3};
    static float float4[4] = {1,2,3,4};
    static float rgbf[3] = {0.84,0.67,0.17};
    static float rgbaf[4] = {0.67,0.90,0.12,1};
    static unsigned rgb = CYAN;
    static unsigned rgba = PINK;
    static float slider = 0.5f;
    static float slider2 = 0.5f;
    static char string[64] = "hello world 123";
    static int item = 0; const char *list[] = {"one","two","three"};
    static bool show_dialog = false;
    static bool show_browser = false;
    static const char* browsed_file = "";
    static uint8_t bitmask = 0x55;
    static int hits = 0;
    static int window1 = 0, window2 = 0, window3 = 0;
    static int disable_all = 0;

    if( ui_panel("UI", 0) ) {
        int choice = ui_toolbar("Browser;Toast@Notify;Toggle on/off");
            if(choice == 1) show_browser = true;
            if(choice == 2) ui_notify(va("My random toast (%d)", rand()), va("This is notification #%d", ++hits));
            if(choice == 3) disable_all ^= 1;

        if( disable_all ) ui_disable();

        if( ui_browse(&browsed_file, &show_browser) ) puts(browsed_file);

        if( ui_section("Labels")) {}
        if( ui_label("my label")) {}
        if( ui_label("my label with tooltip@built on " __DATE__ " " __TIME__)) {}
        if( ui_label2_toolbar("my toolbar", ICON_MD_STAR ICON_MD_STAR_OUTLINE ICON_MD_BOOKMARK ICON_MD_BOOKMARK_BORDER) ) {}
        //if( ui_label2_wrap("my long label", "and some long long long long text wrapped")) {}

        if( ui_section("Types")) {}
        if( ui_bool("my bool", &boolean) ) puts("bool changed");
        if( ui_int("my int", &integer) ) puts("int changed");
        if( ui_float("my float", &floating) ) puts("float changed");
        if( ui_buffer("my string", string, 64) ) puts("string changed");

        if( ui_section("Vectors") ) {}
        if( ui_float2("my float2", float2) ) puts("float2 changed");
        if( ui_float3("my float3", float3) ) puts("float3 changed");
        if( ui_float4("my float4", float4) ) puts("float4 changed");

        if( ui_section("Lists")) {}
        if( ui_list("my list", list, 3, &item ) ) puts("list changed");

        if( ui_section("Colors")) {}
        if( ui_color3("my color3", &rgb) ) puts("color3 changed");
        if( ui_color4("my color4@this is a tooltip", &rgba) ) puts("color4 changed");
        if( ui_color3f("my color3f", rgbf) ) puts("color3f changed");
        if( ui_color4f("my color4f@this is a tooltip", rgbaf) ) puts("color4f changed");

        if( ui_section("Sliders")) {}
        if( ui_slider("my slider", &slider)) puts("slider changed");
        if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");

        if( do_windows ) {
        if( ui_section("Windows")) {}
        int show = ui_buttons(3, "Container", "SubPanel", "SubRender");
        if( show == 1 ) window1 = 1;
        if( show == 2 ) window2 = 1;
        if( show == 3 ) window3 = 1;
        }

        if( ui_section("Others")) {}
        if( ui_bitmask8("my bitmask", &bitmask) ) printf("bitmask changed %x\n", bitmask);
        if( ui_toggle("my toggle", &toggle) ) printf("toggle %s\n", toggle ? "on":"off");
        if( ui_image("my image", texture_checker().id, 0, 0) ) { puts("image clicked"); }

        if( ui_separator() ) {}
        if( ui_button("my button") ) { puts("button clicked"); show_dialog = true; }
        if( ui_buttons(2, "yes", "no") ) { puts("button clicked"); }
        if( ui_buttons(3, "yes", "no", "maybe") ) { puts("button clicked"); }
        if( ui_dialog("my dialog", __FILE__ "\n" __DATE__ "\n" "Public Domain.", 2/*two buttons*/, &show_dialog) ) {}

        if( disable_all ) ui_enable(); // restore enabled state

        ui_panel_end();
    }

    if( !do_windows ) return 0;

    // window api showcasing
    if( ui_window("Container demo", &window1) ) {
        ui_label("label #1");
        if( ui_bool("my bool", &boolean) ) puts("bool changed");
        if( ui_int("my int", &integer) ) puts("int changed");
        if( ui_float("my float", &floating) ) puts("float changed");
        if( ui_buffer("my string", string, 64) ) puts("string changed");
        ui_window_end();
    }

    if( ui_window("SubPanel demo", &window2) ) {
        if( ui_panel("panel #2", 0) ) {
            ui_label("label #2");
            if( ui_bool("my bool", &boolean) ) puts("bool changed");
            if( ui_int("my int", &integer) ) puts("int changed");
            if( ui_float("my float", &floating) ) puts("float changed");
            if( ui_buffer("my string", string, 64) ) puts("string changed");
            ui_panel_end();
        }
        ui_window_end();
    }

    if( ui_window("SubRender demo", &window3) ) {
        if( ui_panel("panel #3A", 0) ) {
            if( ui_bool("my bool", &boolean) ) puts("bool changed");
            if( ui_int("my int", &integer) ) puts("int changed");
            if( ui_float("my float", &floating) ) puts("float changed");
            if( ui_buffer("my string", string, 64) ) puts("string changed");
            if( ui_separator() ) {}
            if( ui_slider("my slider", &slider)) puts("slider changed");
            if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");
            ui_panel_end();
        }
        if( ui_panel("panel #3B", 0) ) {
            if( ui_bool("my bool", &boolean) ) puts("bool changed");
            if( ui_int("my int", &integer) ) puts("int changed");
            if( ui_float("my float", &floating) ) puts("float changed");
            if( ui_buffer("my string", string, 64) ) puts("string changed");
            if( ui_separator() ) {}
            if( ui_slider("my slider", &slider)) puts("slider changed");
            if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");
            ui_panel_end();
        }

        const char *title = "SubRender demo";
        struct nk_window *win = nk_window_find(ui_ctx, title);
        if( win ) {
            enum { menubar_height = 65 }; // title bar (~32) + menu bounds (~25)
            struct nk_rect bounds = win->bounds; bounds.y += menubar_height; bounds.h -= menubar_height;
#if 1
            ddraw_flush();

            // @fixme: this is breaking rendering when post-fxs are in use. edit: cannot reproduce
            static texture_t tx = {0};
            if( texture_rec_begin(&tx, bounds.w, bounds.h )) {
                glClearColor(0.15,0.15,0.15,1);
                glClear(GL_COLOR_BUFFER_BIT);
                ddraw_grid(10);
                ddraw_flush();
                texture_rec_end(&tx);
            }
            struct nk_image image = nk_image_id((int)tx.id);
            nk_draw_image_flipped(nk_window_get_canvas(ui_ctx), bounds, &image, nk_white);
#else
            static video_t *v = NULL;
            do_once v = video( "bjork-all-is-full-of-love.mp4", VIDEO_RGB );

            texture_t *textures = video_decode( v );

            struct nk_image image = nk_image_id((int)textures[0].id);
            nk_draw_image(nk_window_get_canvas(ui_ctx), bounds, &image, nk_white);
#endif
        }

        ui_window_end();
    }
    return 0;
}
