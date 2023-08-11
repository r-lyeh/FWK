// editing:
// nope > functions: add/rem property

char *editor_path(const char *path) {
    return va("%s/%s", EDITOR, path);
}

vec3 editor_pick(float mouse_x, float mouse_y) {
    // unproject 2d coord as 3d coord
    camera_t *camera = camera_get_active();
    vec3 out, xyd = vec3(mouse_x,window_height()-mouse_y,1); // usually x:mouse_x,y:window_height()-mouse_y,d:0=znear/1=zfar
    mat44 mvp, model; identity44(model); multiply44x3(mvp, camera->proj, camera->view, model);
    bool ok = unproject44(&out, xyd, vec4(0,0,window_width(),window_height()), mvp);
    return out;
}

int editor_ui_bits8(const char *label, uint8_t *enabled) { // @to deprecate
    int clicked = 0;
    uint8_t copy = *enabled;

    // @fixme: better way to retrieve widget width? nk_layout_row_dynamic() seems excessive
    nk_layout_row_dynamic(ui_ctx, 1, 1);
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);

    // actual widget: label + 8 checkboxes
    enum { HEIGHT = 18, BITS = 8, SPAN = 118 }; // bits widget below needs at least 118px wide
    nk_layout_row_begin(ui_ctx, NK_STATIC, HEIGHT, 1+BITS);

        int offset = bounds.w > SPAN ? bounds.w - SPAN : 0;
        nk_layout_row_push(ui_ctx, offset);
        if( ui_label_(label, NK_TEXT_LEFT) ) clicked = 1<<31;

        for( int i = 0; i < BITS; ++i ) {
            nk_layout_row_push(ui_ctx, 10);
            // bit
            int val = (*enabled >> i) & 1;
            int chg = nk_checkbox_label(ui_ctx, "", &val);
            *enabled = (*enabled & ~(1 << i)) | ((!!val) << i);
            // tooltip
            struct nk_rect bb = { offset + 10 + i * 14, bounds.y, 14, HEIGHT }; // 10:padding,14:width
            if (nk_input_is_mouse_hovering_rect(&ui_ctx->input, bb) && !ui_popups()) {
                const char *tips[BITS] = {"Init","Tick","Draw","Quit","","","",""};
                if(tips[i][0]) nk_tooltipf(ui_ctx, "%s", tips[i]);
            }
        }

    nk_layout_row_end(ui_ctx);
    return clicked | (copy ^ *enabled);
}

static int gizmo__mode;
static int gizmo__active;
static int gizmo__hover;
bool gizmo_active() {
    return gizmo__active;
}
bool gizmo_hover() {
    return gizmo__hover;
}
int gizmo(vec3 *pos, vec3 *rot, vec3 *sca) {
#if 0
    ddraw_flush();
    mat44 copy; copy44(copy, camera_get_active()->view);
    if( 1 ) {
        float *mv = camera_get_active()->view;
        float d = sqrt(mv[4*0+0] * mv[4*0+0] + mv[4*1+1] * mv[4*1+1] + mv[4*2+2] * mv[4*2+2]);
        if(4) mv[4*0+0] = d, mv[4*0+1] = 0, mv[4*0+2] = 0;
        if(2) mv[4*1+0] = 0, mv[4*1+1] = d, mv[4*1+2] = 0;
        if(1) mv[4*2+0] = 0, mv[4*2+1] = 0, mv[4*2+2] = d;
    }
#endif

    ddraw_color_push(dd_color);
    ddraw_ontop_push(1);

    int enabled = !ui_active() && !ui_hover();
    vec3 mouse = enabled ? vec3(input(MOUSE_X),input(MOUSE_Y),input_down(MOUSE_L)) : vec3(0,0,0); // x,y,l
    vec3 from = camera_get_active()->position;
    vec3 to = editor_pick(mouse.x, mouse.y);
    ray r = ray(from, to);

    static vec3 src3, hit3, off3; static vec2 src2;
    #define on_gizmo_dragged(X,Y,Z,COLOR,DRAWCMD, ...) do { \
        vec3 dir = vec3(X,Y,Z); \
        line axis = {add3(*pos, scale3(dir,100)), add3(*pos, scale3(dir,-100))}; \
        plane ground = { vec3(0,0,0), vec3(Y?1:0,Y?0:1,0) }; \
        vec3 unit = vec3(X+(1.0-X)*0.3,Y+(1.0-Y)*0.3,Z+(1.0-Z)*0.3); \
        aabb arrow = { sub3(*pos,unit), add3(*pos,unit) }; \
        hit *hit_arrow = ray_hit_aabb(r, arrow), *hit_ground = ray_hit_plane(r, ground); \
        ddraw_color( hit_arrow || gizmo__active == (X*4+Y*2+Z) ? gizmo__hover = 1, YELLOW : COLOR ); \
        DRAWCMD; \
        if( !gizmo__active && hit_arrow && mouse.z ) src2 = vec2(mouse.x,mouse.y), src3 = *pos, hit3 = hit_ground->p, off3 = mul3(sub3(src3,hit3),vec3(X,Y,Z)), gizmo__active = X*4+Y*2+Z; \
        if( (gizmo__active && gizmo__active==(X*4+Y*2+Z)) || (!gizmo__active && hit_arrow) ) { ddraw_color( COLOR ); ( 1 ? ddraw_line : ddraw_line_dashed)(axis.a, axis.b); } \
        if( gizmo__active == (X*4+Y*2+Z) && hit_ground ) {{ __VA_ARGS__ }; modified = 1; gizmo__active *= !!input(MOUSE_L); } \
    } while(0)
    #define gizmo_translate(X,Y,Z,COLOR) \
        on_gizmo_dragged(X,Y,Z,COLOR, ddraw_arrow(*pos,add3(*pos,vec3(X,Y,Z))), { \
            *pos = add3(line_closest_point(axis, hit_ground->p), off3); \
        } )
    #define gizmo_scale(X,Y,Z,COLOR) \
        on_gizmo_dragged(X,Y,Z,COLOR, (ddraw_line(*pos,add3(*pos,vec3(X,Y,Z))),ddraw_sphere(add3(*pos,vec3(X-0.1*X,Y-0.1*Y,Z-0.1*Z)),0.1)), { /*ddraw_aabb(arrow.min,arrow.max)*/ \
            int component = (X*1+Y*2+Z*3)-1; \
            float mag = len2(sub2(vec2(mouse.x, mouse.y), src2)); \
            float magx = (mouse.x - src2.x) * (mouse.x - src2.x); \
            float magy = (mouse.y - src2.y) * (mouse.y - src2.y); \
            float sgn = (magx > magy ? mouse.x > src2.x : mouse.y > src2.y) ? 1 : -1; \
            sca->v3[component] -= sgn * mag * 0.01; \
            src2 = vec2(mouse.x, mouse.y); \
        } )
    #define gizmo_rotate(X,Y,Z,COLOR) do { \
            vec3 dir = vec3(X,Y,Z); \
            line axis = {add3(*pos, scale3(dir,100)), add3(*pos, scale3(dir,-100))}; \
            plane ground = { vec3(0,0,0), vec3(0,1,0) }; \
                vec3 unit = vec3(X+(1.0-X)*0.3,Y+(1.0-Y)*0.3,Z+(1.0-Z)*0.3); \
                aabb arrow = { sub3(*pos,unit), add3(*pos,unit) }; \
                hit *hit_arrow = ray_hit_aabb(r, arrow), *hit_ground = ray_hit_plane(r, ground); \
                int hover = (hit_arrow ? (X*4+Y*2+Z) : 0); \
            if( gizmo__active == (X*4+Y*2+Z) ) { ddraw_color(gizmo__active ? gizmo__hover = 1, YELLOW : WHITE); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            else if( !gizmo__active && hover == (X*4+Y*2+Z) ) { gizmo__hover = 1; ddraw_color(COLOR); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            else if( !gizmo__active ) { ddraw_color(WHITE); ddraw_circle(*pos, vec3(X,Y,Z), 1); } \
            if( !gizmo__active && hit_arrow && mouse.z ) src2 = vec2(mouse.x,mouse.y), gizmo__active = hover; \
            if( (!gizmo__active && hover == (X*4+Y*2+Z)) || gizmo__active == (X*4+Y*2+Z) ) { gizmo__hover = 1; ddraw_color( COLOR ); ( 1 ? ddraw_line_thin : ddraw_line_dashed)(axis.a, axis.b); } \
            if( gizmo__active && gizmo__active == (X*4+Y*2+Z) && hit_ground && enabled ) { \
                int component = (Y*1+X*2+Z*3)-1; /*pitch,yaw,roll*/ \
                float mag = len2(sub2(vec2(mouse.x, mouse.y), src2)); \
                float magx = (mouse.x - src2.x) * (mouse.x - src2.x); \
                float magy = (mouse.y - src2.y) * (mouse.y - src2.y); \
                float sgn = (magx > magy ? mouse.x > src2.x : mouse.y > src2.y) ? 1 : -1; \
                rot->v3[component] += sgn * mag; \
                /*rot->v3[component] = clampf(rot->v3[component], -360, +360);*/ \
                src2 = vec2(mouse.x, mouse.y); \
                \
            } \
            gizmo__active *= enabled && !!input(MOUSE_L); \
        } while(0)

    gizmo__hover = 0;

    int modified = 0;
    if(enabled && input_down(KEY_SPACE)) gizmo__active = 0, gizmo__mode = (gizmo__mode + 1) % 3;
    if(gizmo__mode == 0) gizmo_translate(1,0,0, RED);
    if(gizmo__mode == 0) gizmo_translate(0,1,0, GREEN);
    if(gizmo__mode == 0) gizmo_translate(0,0,1, BLUE);
    if(gizmo__mode == 1) gizmo_scale(1,0,0, RED);
    if(gizmo__mode == 1) gizmo_scale(0,1,0, GREEN);
    if(gizmo__mode == 1) gizmo_scale(0,0,1, BLUE);
    if(gizmo__mode == 2) gizmo_rotate(1,0,0, RED);
    if(gizmo__mode == 2) gizmo_rotate(0,1,0, GREEN);
    if(gizmo__mode == 2) gizmo_rotate(0,0,1, BLUE);

#if 0
    ddraw_flush();
    copy44(camera_get_active()->view, copy);
#endif

    ddraw_ontop_pop();
    ddraw_color_pop();

    return modified;
}

char* dialog_load() {
    const char *windowTitle = NULL;
    const char *defaultPathFile = NULL;
    const char *filterHints = NULL; // "image files"
    const char *filters[] = { "*.*" };
    int allowMultipleSelections = 0;

    tinyfd_assumeGraphicDisplay = 1;
    return tinyfd_openFileDialog( windowTitle, defaultPathFile, countof(filters), filters, filterHints, allowMultipleSelections );
}
char* dialog_save() {
    const char *windowTitle = NULL;
    const char *defaultPathFile = NULL;
    const char *filterHints = NULL; // "image files"
    const char *filters[] = { "*.*" };

    tinyfd_assumeGraphicDisplay = 1;
    return tinyfd_saveFileDialog( windowTitle, defaultPathFile, countof(filters), filters, filterHints );
}

// -- localization kit

static const char *kit_lang = "enUS", *kit_langs = 
    "enUS,enGB,"
    "frFR,"
    "esES,esAR,esMX,"
    "deDE,deCH,deAT,"
    "itIT,itCH,"
    "ptBR,ptPT,"
    "zhCN,zhSG,zhTW,zhHK,zhMO,"
    "ruRU,elGR,trTR,daDK,noNB,svSE,nlNL,plPL,fiFI,jaJP,"
    "koKR,csCZ,huHU,roRO,thTH,bgBG,heIL"
;

static map(char*,char*) kit_ids;
static map(char*,char*) kit_vars;

#ifndef KIT_FMT_ID2 
#define KIT_FMT_ID2 "%s.%s"
#endif

void kit_init() {
    do_once map_init(kit_ids, less_str, hash_str);
    do_once map_init(kit_vars, less_str, hash_str);
}

void kit_insert( const char *id, const char *translation) {
    char *id2 = va(KIT_FMT_ID2, kit_lang, id);

    char **found = map_find_or_add_allocated_key(kit_ids, STRDUP(id2), NULL);
    if(*found) FREE(*found);
    *found = STRDUP(translation);
}

bool kit_merge( const char *filename ) {
    // @todo: xlsx2ini
    return false;
}

void kit_clear() {
    map_clear(kit_ids);
}

bool kit_load( const char *filename ) {
    return kit_clear(), kit_merge( filename );
}

void kit_set( const char *key, const char *value ) {
    value = value && value[0] ? value : "";

    char **found = map_find_or_add_allocated_key(kit_vars, STRDUP(key), NULL );
    if(*found) FREE(*found);
    *found = STRDUP(value);
}

void kit_reset() {
    map_clear(kit_vars);
}

char *kit_translate2( const char *id, const char *lang ) {
    char *id2 = va(KIT_FMT_ID2, lang, id);

    char **found = map_find(kit_ids, id2);

    // return original [[ID]] if no translation is found
    if( !found ) return va("[[%s]]", id);

    // return translation if no {{moustaches}} are found
    if( !strstr(*found, "{{") ) return *found;

    // else replace all found {{moustaches}} with context vars
    {
        // make room
        static __thread char *results[16] = {0};
        static __thread unsigned counter = 0; counter = (counter+1) % 16;

        char *buffer = results[ counter ];
        if( buffer ) FREE(buffer), buffer = 0;

        // iterate moustaches
        const char *begin, *end, *text = *found;
        while( NULL != (begin = strstr(text, "{{")) ) {
            end = strstr(begin+2, "}}");
            if( end ) {
                char *var = va("%.*s", (int)(end - (begin+2)), begin+2);
                char **found_var = map_find(kit_vars, var);

                if( found_var && 0[*found_var] ) {
                    strcatf(&buffer, "%.*s%s", (int)(begin - text), text, *found_var);
                } else {
                    strcatf(&buffer, "%.*s{{%s}}", (int)(begin - text), text, var);
                }

                text = end+2;
            } else {
                strcatf(&buffer, "%.*s", (int)(begin - text), text);

                text = begin+2;
            }
        }

        strcatf(&buffer, "%s", text);
        return buffer;
    }
}

char *kit_translate( const char *id ) {
    return kit_translate2( id, kit_lang );
}

void kit_locale( const char *lang ) {
    kit_lang = STRDUP(lang); // @leak
}

void kit_dump_state( FILE *fp ) {
    for each_map(kit_ids, char *, k, char *, v) {
        fprintf(fp, "[ID ] %s=%s\n", k, v);
    }
    for each_map(kit_vars, char *, k, char *, v) {
        fprintf(fp, "[VAR] %s=%s\n", k, v);
    }
}

/*
int main() {
    kit_init();

    kit_locale("enUS");
    kit_insert("HELLO_PLAYERS", "Hi {{PLAYER1}} and {{PLAYER2}}!");
    kit_insert("GREET_PLAYERS", "Nice to meet you.");

    kit_locale("esES");
    kit_insert("HELLO_PLAYERS", "Hola {{PLAYER1}} y {{PLAYER2}}!");
    kit_insert("GREET_PLAYERS", "Un placer conoceros.");

    kit_locale("enUS");
    printf("%s %s\n", kit_translate("HELLO_PLAYERS"), kit_translate("GREET_PLAYERS")); // Hi {{PLAYER1}} and {{PLAYER2}}! Nice to meet you.

    kit_locale("esES");
    kit_set("PLAYER1", "John");
    kit_set("PLAYER2", "Karl");
    printf("%s %s\n", kit_translate("HELLO_PLAYERS"), kit_translate("GREET_PLAYERS")); // Hola John y Karl! Un placer conoceros.

    assert( 0 == strcmp(kit_translate("NON_EXISTING"), "[[NON_EXISTING]]")); // [[NON_EXISTING]]
    assert(~puts("Ok"));
}
*/
