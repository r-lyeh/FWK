// -----------------------------------------------------------------------------
// immediate ui framework
// - rlyeh, public domain
//
// @todo: logger/console
// @todo: surround-adaptive window resizing. ie, surrounding windows adapting theirselves to fit dragged active window

enum PANEL_FLAGS {
    PANEL_OPEN = 1,
};

API int ui_notify(const char *title, const char *body);
API int ui_window(const char *title, int *enabled);
API int  ui_panel(const char *title, int flags); // may be embedded inside a window, or standalone
API int   ui_collapse(const char *label, const char *id);
API int   ui_contextual();
API int    ui_section(const char *title);
API int    ui_int(const char *label, int *value);
API int    ui_bool(const char *label, bool *value);
API int    ui_short(const char *label, short *value);
API int    ui_float(const char *label, float *value);
API int    ui_float2(const char *label, float value[2]);
API int    ui_float3(const char *label, float value[3]);
API int    ui_float4(const char *label, float value[4]);
API int    ui_double(const char *label, double *value);
API int    ui_buffer(const char *label, char *buffer, int buflen);
API int    ui_string(const char *label, char **string);
API int    ui_color3(const char *label, float *color3); //[0..255]
API int    ui_color3f(const char *label, float *color3); //[0..1]
API int    ui_color4(const char *label, float *color4); //[0..255]
API int    ui_color4f(const char *label, float *color4); //[0..1]
API int    ui_unsigned(const char *label, unsigned *value);
API int    ui_button(const char *label);
API int    ui_button_transparent(const char *label);
API int    ui_buttons(int buttons, /*labels*/...);
API int    ui_toolbar(const char *options); // int choice = ui_toolbar("A;B;C;D");
API int    ui_submenu(const char *options); // int choice = ui_submenu("A;B;C;D");
API int    ui_browse(const char **outfile, bool *inlined); // may be embedded inside a window or inside a panel
API int    ui_toggle(const char *label, bool *value);
API int    ui_dialog(const char *title, const char *text, int choices, bool *show); // @fixme: return
API int    ui_list(const char *label, const char **items, int num_items, int *selector);
API int    ui_radio(const char *label, const char **items, int num_items, int *selector);
API int    ui_texture(const char *label, texture_t t);
API int    ui_subtexture(const char *label, texture_t t, unsigned x, unsigned y, unsigned w, unsigned h);
API int    ui_image(const char *label, handle id, unsigned w, unsigned h); //(w,h) can be 0
API int    ui_subimage(const char *label, handle id, unsigned iw, unsigned ih, unsigned sx, unsigned sy, unsigned sw, unsigned sh);
API int    ui_colormap(const char *label, colormap_t *cm); // returns num member changed: 1 for color, 2 for texture map
API int    ui_separator();
API int    ui_bits8(const char *label, uint8_t *bits);
API int    ui_bits16(const char *label, uint16_t *bits);
API int    ui_console();
API int    ui_clampf(const char *label, float *value, float minf, float maxf);
API int    ui_label(const char *label);
API int    ui_label2(const char *label, const char *caption);
API int    ui_label2_toolbar(const char *label, const char *icons);
API int    ui_slider(const char *label, float *value);
API int    ui_slider2(const char *label, float *value, const char *caption);
API int    ui_const_bool(const char *label, const double value);
API int    ui_const_float(const char *label, const double value);
API int    ui_const_string(const char *label, const char *value);
API int   ui_contextual_end();
API int   ui_collapse_clicked();
API int   ui_collapse_end();
API int  ui_panel_end();
API int ui_window_end();

API int ui_show(const char *panel_or_window_title, int enabled);
API int ui_visible(const char *panel_or_window_title); // @todo: include ui_collapse() items that are open as well?
API int ui_enable();
API int ui_disable();

API int ui_has_menubar();
API int ui_menu(const char *items); // semicolon-separated or comma-separated items
API int ui_menu_editbox(char *buf, int bufcap);
API int ui_item();

API int ui_popups(); // ui_any_popup()? ui_has_popups()?
API int ui_hover(); // ui_is_hover()?
API int ui_active(); // ui_is_active()?

API int ui_demo(int do_windows);
