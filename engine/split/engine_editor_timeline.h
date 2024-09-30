#define TIMELINE_ICON ICON_MDI_CHART_TIMELINE
#define TIMELINE_TITLE "Timeline " TIMELINE_ICON

EDITOR_BIND(timeline, "held(CTRL)&down(3)", { ui_show(TIMELINE_TITLE, ui_visible(TIMELINE_TITLE) ^ true); });

int ui_tween(const char *label, tween_t *t) {
    if( ui_filter && ui_filter[0] ) if( !strstr(label, ui_filter) ) return 0;

    int expand_keys = label[0] == '!'; label += expand_keys;
    const char *id = label;
    if( strchr(id, '@') ) *strchr((char*)(id = (const char*)va("%s", label)), '@') = '\0';

    enum { LABEL_SPACING = 250 };
    enum { ROUNDING = 0 };
    enum { THICKNESS = 1 };
    enum { PIXELS_PER_SECOND = 60 };
    enum { KEY_WIDTH = 5, KEY_HEIGHT = 5 };
    enum { TIMELINE_HEIGHT = 25 };
    enum { MARKER1_HEIGHT = 5, MARKER10_HEIGHT = 20, MARKER5_HEIGHT = (MARKER1_HEIGHT + MARKER10_HEIGHT) / 2 };
    unsigned base_color = WHITE;
    unsigned time_color = YELLOW;
    unsigned duration_color = ORANGE;
    unsigned key_color = GREEN;

    int changed = 0;

#if 0
    // two rows with height:30 composed of three widgets
    nk_layout_row_template_begin(ui_ctx, 30);
    nk_layout_row_template_push_variable(ui_ctx, t->duration * PIXELS_PER_SECOND); // min 80px. can grow
    nk_layout_row_template_end(ui_ctx);
#endif

        char *sid = va("%s.%d", id, 0);
        uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
        for(int i = 0; sid[i]; ++i) hash = (hash ^ sid[i]) * mult;
        ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

    ui_label(label);

    struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
    struct nk_rect bounds; nk_layout_peek(&bounds, ui_ctx);
    bounds.y -= 30;

    struct nk_rect baseline = bounds; baseline.y += 30/2;
    baseline.x += LABEL_SPACING;
    baseline.w -= LABEL_SPACING;

    // tween duration
    {
        struct nk_rect pos = baseline;
        pos.w  = pos.x + t->duration * PIXELS_PER_SECOND;
        pos.y -= TIMELINE_HEIGHT/2;
        pos.h  = TIMELINE_HEIGHT;
        nk_stroke_rect(canvas, pos, ROUNDING, THICKNESS*2, AS_NKCOLOR(duration_color));
    }

    // tween ranges
    for(int i = 0, end = array_count(t->keyframes) - 1; i < end; ++i) {
        tween_keyframe_t *k = t->keyframes + i;
        tween_keyframe_t *next = k + 1;

        struct nk_rect pos = baseline;
        pos.x += k->t * PIXELS_PER_SECOND;
        pos.w  = (next->t - k->t) * PIXELS_PER_SECOND;
        pos.y -= TIMELINE_HEIGHT/2;
        pos.h  = TIMELINE_HEIGHT;

        char *sid = va("%s.%d", id, i);
        uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
        for(int i = 0; sid[i]; ++i) hash = (hash ^ sid[i]) * mult;
        ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

        struct nk_color c = nk_hsva_f(ui_hue, 0.75f, 0.8f, ui_alpha);
        nk_fill_rect(canvas, pos, ROUNDING, k->ease == EASE_ZERO ? AS_NKCOLOR(0) : c); // AS_NKCOLOR(track_color));
    }

    // horizontal line
    nk_stroke_line(canvas, baseline.x, baseline.y, baseline.x+baseline.w,baseline.y, THICKNESS, AS_NKCOLOR(base_color));

    // unit, 5-unit and 10-unit markers
    for( float i = 0, j = 0; i < baseline.w; i += PIXELS_PER_SECOND/10, ++j ) {
        int len = !((int)j%10) ? MARKER10_HEIGHT : !((int)j%5) ? MARKER5_HEIGHT : MARKER1_HEIGHT;
        nk_stroke_line(canvas, baseline.x+i, baseline.y-len, baseline.x+i, baseline.y+len, THICKNESS, AS_NKCOLOR(base_color));
    }

    // time marker
    float px = t->time * PIXELS_PER_SECOND;
    nk_stroke_line(canvas, baseline.x+px, bounds.y, baseline.x+px, bounds.y+bounds.h, THICKNESS*2, AS_NKCOLOR(time_color));
    nk_draw_symbol(canvas, NK_SYMBOL_TRIANGLE_DOWN, ((struct nk_rect){ baseline.x+px-4,bounds.y-4-8,8,8}), /*bg*/AS_NKCOLOR(0), /*fg*/AS_NKCOLOR(time_color), 0.f/*border_width*/, ui_ctx->style.font);

    // key markers
    for each_array_ptr(t->keyframes, tween_keyframe_t, k) {
        struct nk_rect pos = baseline;
        pos.x += k->t * PIXELS_PER_SECOND;

        vec2 romboid[] = {
            {pos.x-KEY_WIDTH,pos.y}, {pos.x,pos.y-KEY_HEIGHT},
            {pos.x+KEY_WIDTH,pos.y}, {pos.x,pos.y+KEY_HEIGHT}
        };

        nk_fill_polygon(canvas, (float*)romboid, countof(romboid), AS_NKCOLOR(key_color));
    }

    // keys ui
    if( expand_keys )
    for(int i = 0, end = array_count(t->keyframes); i < end; ++i) {
        tween_keyframe_t *k = t->keyframes + i;
        if( ui_collapse(va("Key %d", i), va("%s.%d", id, i))) {
            changed |= ui_float("Time", &k->t);
            changed |= ui_float3("Value", &k->v.x);
            changed |= ui_list("Ease", ease_enums(), EASE_NUM, &k->ease );
            ui_collapse_end();
        }
    }

    return changed;
}

tween_t* rand_tween() {
    tween_t demo = tween();
    int num_keys = randi(2,8);
    double t = 0;
    for( int i = 0; i < num_keys; ++i) {
        tween_setkey(&demo, t, scale3(vec3(randf(),randf(),randf()),randi(-5,5)), randi(0,EASE_NUM) );
        t += randi(1,5) / ((float)(1 << randi(0,2)));
    }
    tween_t *p = CALLOC(1, sizeof(tween_t));
    memcpy(p, &demo, sizeof(tween_t));
    return p;
}

int editor_timeline(int window_mode) {
    static array(tween_t*) tweens = 0;

    do_once {
        array_push(tweens, rand_tween());
    }

    if( editor.t == 0 )
    for each_array(tweens, tween_t*,t) {
        tween_reset(t);
    }
    else
    for each_array(tweens, tween_t*,t) {
        tween_update(t, editor.dt);
    }

    static void *selected = NULL;
    if( editor_begin(TIMELINE_TITLE, window_mode) ) {

        int choice = ui_toolbar(ICON_MDI_PLUS ";" ICON_MDI_MINUS );
        if( choice == 1 ) array_push(tweens, rand_tween());
        if( choice == 2 && selected ) {
            int target = -1;
            for( int i = 0, end = array_count(tweens); i < end; ++i ) if( tweens[i] == selected ) { target = i; break; }
            if( target >= 0 ) { array_erase_slow(tweens, target); selected = NULL; }
        }

        for each_array(tweens, tween_t*,t) {
            ui_tween(va("%s%p@%05.2fs Value: %s", t == selected ? "!":"", t, t->time, ftoa3(t->result)), t);
            if(ui_label_icon_clicked_L.x) selected = (t != selected) ? t : NULL;
        }

        editor_end(window_mode);
    }
    return 0;
}

AUTORUN {
    array_push(editor.subeditors, editor_timeline);
}
