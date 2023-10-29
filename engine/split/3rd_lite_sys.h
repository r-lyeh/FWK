// lite editor, platform details
// - rlyeh, public domain

#define LT_DATAPATH          "/lite"

#define lt_assert(x)         ASSERT(x)

#define lt_realpath(p, q)    file_pathabs(p)
#define lt_realpath_free(p)

#define lt_malloc(n)         MALLOC(n)
#define lt_calloc(n,m)       CALLOC(n,m)
#define lt_free(p)           FREE(p)
#define lt_memcpy(d,s,c)     memcpy(d,s,c)
#define lt_memset(p,ch,c)    memset(p,ch,c)

#define lt_time_ms()         time_ms()
#define lt_sleep_ms(ms)      sleep_ms(ms)

#define lt_getclipboard(w)   window_clipboard()
#define lt_setclipboard(w,s) window_setclipboard(s)

#define lt_window()          window_handle()
#define lt_setwindowmode(m)  window_fullscreen(m == 2), (m < 2 && (window_maximize(m),1)) // 0:normal,1:maximized,2:fullscreen
#define lt_setwindowtitle(t) //window_title(t)
#define lt_haswindowfocus()  window_has_focus()
#define lt_setcursor(shape)  window_cursor_shape(lt_events & (1<<31) ? CURSOR_SW_AUTO : shape+1) // 0:arrow,1:ibeam,2:sizeh,3:sizev,4:hand

#define lt_prompt(msg,title) ifndef(win32, 0, (MessageBoxA(0, msg, title, MB_YESNO | MB_ICONWARNING) == IDYES))

unsigned lt_events = ~0u;
int lt_mx = 0, lt_my = 0, lt_wx = 0, lt_wy = 0, lt_ww = 0, lt_wh = 0;

typedef struct lt_surface {
    unsigned w, h;
    void *pixels;
    texture_t t;
} lt_surface;

typedef struct lt_rect {
    int x, y, width, height;
} lt_rect;

lt_surface *lt_getsurface(void *window) {
    static lt_surface s = {0};
    return &s;
}
void lt_updatesurfacerects(lt_surface *s, lt_rect* rects, unsigned count) {
    if(0)
    for( int i = 0; i < count; ++i ) {
        memset( (unsigned*)s->pixels + (rects[i].x + rects[i].y * s->w), 0xFF, rects[i].width*4 );
        memset( (unsigned*)s->pixels + (rects[i].x + (rects[i].y + (rects[i].height-1)) * s->w), 0xFF, rects[i].width*4 );
        for( int y = 1; y < (rects[i].height-1); ++y ) {
            ((unsigned*)s->pixels)[ rects[i].x + y * s->w ] =
            ((unsigned*)s->pixels)[ rects[i].x + (rects[i].width-1) + y * s->w ] = 0xFFFFFFFF;
        }
    }

    // update contents
    texture_update(&s->t, s->w, s->h, 4, s->pixels, TEXTURE_LINEAR|TEXTURE_BGRA);
}

void ren_set_clip_rect(struct lt_rect rect);
void rencache_invalidate(void);
int lt_resizesurface(lt_surface *s, int ww, int wh) {
    s->w = ww, s->h = wh;
    if( s->t.id == 0 || s->w != s->t.w || s->h != s->t.h ) {
        // invalidate tiles
        ren_set_clip_rect( (lt_rect) { 0, 0, s->w, s->h } );
        rencache_invalidate();

        // texture clear
        if( !s->t.id ) s->t = texture_create(1, 1, 4, "    ", TEXTURE_LINEAR|TEXTURE_RGBA|TEXTURE_BYTE );
        s->pixels = REALLOC(s->pixels, s->w * s->h * 4);
        memset(s->pixels, 0, s->w * s->h * 4);

        // texture update
        lt_updatesurfacerects(s,0,0);
        return 1; // resized
    }
    return 0; // unchanged
}

void *lt_load_file(const char *filename, int *size) {
    int datalen; char *data = file_load(filename, &datalen);
    if( !data || !datalen ) {
        filename = (const char *)file_normalize(filename);
        if( strbegi(filename, app_path()) ) filename += strlen(app_path());
        data = vfs_load(filename, &datalen);
    }
    if (size) *size = 0;
    if (!data) { return NULL; }
    if (size) *size = datalen;
    // return permanent buffers here, as file_load() and vfs_load() do return temporaries
    data = memcpy(MALLOC(datalen+1), data, datalen);
    data[datalen] = 0;
    return data;
}

const char* lt_button_name(int button) {
    if(button == GLFW_MOUSE_BUTTON_LEFT) return "left";
    if(button == GLFW_MOUSE_BUTTON_RIGHT) return "right";
    if(button == GLFW_MOUSE_BUTTON_MIDDLE) return "middle";
    return "?";
}

char* lt_key_name(char *dst, int key, int vk, int mods) {
    // @todo: "altgr" -> left ctrl + right alt

    if( key == GLFW_KEY_UP ) return "up";
    if( key == GLFW_KEY_DOWN ) return "down";
    if( key == GLFW_KEY_LEFT ) return "left";
    if( key == GLFW_KEY_RIGHT ) return "right";
    if( key == GLFW_KEY_LEFT_ALT ) return "left alt";
    if( key == GLFW_KEY_RIGHT_ALT ) return "right alt";
    if( key == GLFW_KEY_LEFT_SHIFT ) return "left shift";
    if( key == GLFW_KEY_RIGHT_SHIFT ) return "right shift";
    if( key == GLFW_KEY_LEFT_CONTROL ) return "left ctrl";
    if( key == GLFW_KEY_RIGHT_CONTROL ) return "right ctrl";
    if( key == GLFW_KEY_LEFT_SUPER ) return "left windows";
    if( key == GLFW_KEY_RIGHT_SUPER ) return "left windows";
    if( key == GLFW_KEY_MENU ) return "menu";

    if( key == GLFW_KEY_ESCAPE ) return "escape";
    if( key == GLFW_KEY_BACKSPACE ) return "backspace";
    if( key == GLFW_KEY_ENTER ) return "return";
    if( key == GLFW_KEY_KP_ENTER ) return "keypad enter";
    if( key == GLFW_KEY_TAB ) return "tab";
    if( key == GLFW_KEY_CAPS_LOCK ) return "capslock";

    if( key == GLFW_KEY_HOME ) return "home";
    if( key == GLFW_KEY_END ) return "end";
    if( key == GLFW_KEY_INSERT ) return "insert";
    if( key == GLFW_KEY_DELETE ) return "delete";
    if( key == GLFW_KEY_PAGE_UP ) return "pageup";
    if( key == GLFW_KEY_PAGE_DOWN ) return "pagedown";

    if( key == GLFW_KEY_F1 ) return "f1";
    if( key == GLFW_KEY_F2 ) return "f2";
    if( key == GLFW_KEY_F3 ) return "f3";
    if( key == GLFW_KEY_F4 ) return "f4";
    if( key == GLFW_KEY_F5 ) return "f5";
    if( key == GLFW_KEY_F6 ) return "f6";
    if( key == GLFW_KEY_F7 ) return "f7";
    if( key == GLFW_KEY_F8 ) return "f8";
    if( key == GLFW_KEY_F9 ) return "f9";
    if( key == GLFW_KEY_F10 ) return "f10";
    if( key == GLFW_KEY_F11 ) return "f11";
    if( key == GLFW_KEY_F12 ) return "f12";

    const char *name = glfwGetKeyName(key, vk);
    strcpy(dst, name ? name : "");
    char *p = dst;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    return dst;
}

void lt_globpath(struct lua_State*L, const char *path) {
    unsigned j = 0;

    if(!strend(path, "/")) path = (const char *)va("%s/", path);
    for( dir *d = dir_open(path, ""); d; dir_close(d), d = 0 ) {
        for( unsigned i = 0, end = dir_count(d); i < end; ++i ) {
            char *name = dir_name(d,i);
            char *last = name + strlen(name) - 1;
            if( *last == '/' ) *last = '\0';
            name = file_name(name);
            lua_pushstring(L, name);
            lua_rawseti(L, -2, ++j);
        }
    }

    for( const char *section = strstri(path, LT_DATAPATH); section && section[sizeof(LT_DATAPATH)-1] == '/'; section = 0) {
        array(char*) list = vfs_list("**");
        for( unsigned i = 0, end = array_count(list); i < end; ++i ) {
            char *name = list[i];
            if( !strstri(name, section+1) ) continue;
            lua_pushstring(L, file_name(name));
            lua_rawseti(L, -2, ++j);
        }
        if( array_count(list) ) return;
    }
}

int lt_emit_event(lua_State *L, const char *event_name, const char *event_fmt, ...) {
    int count = 0;
    lua_pushstring(L, event_name);
    if( event_fmt ) {
        va_list va;
        va_start(va, event_fmt);
        for( ; event_fmt[count]; ++count ) {
            /**/ if( event_fmt[count] == 'd' ) { int d = va_arg(va, int); lua_pushnumber(L, d); }
            else if( event_fmt[count] == 'f' ) { double f = va_arg(va, double); lua_pushnumber(L, f); }
            else if( event_fmt[count] == 's' ) { const char *s = va_arg(va, const char *); lua_pushstring(L, s); }
        }
        va_end(va);
    }
    return 1+count;
}

int printi(int i) {
    // printf("clicks: %d\n", i);
    return i;
}

static const char* codepoint_to_utf8_(unsigned c);
int lt_poll_event(lua_State *L) { // init.lua > core.step() wakes on mousemoved || inputtext
    int rc = 0;
    char buf[16];
    static int prevx = 0, prevy = 0;

    static unsigned clicks_time = 0, clicks = 0;
    if( (lt_time_ms() - clicks_time) > 400 ) clicks = 0;

    //

    for( GLEQevent e; gleqNextEvent(&e); gleqFreeEvent(&e) )
    if( lt_events & e.type )
    switch (e.type) {
    default:
        break; case GLEQ_WINDOW_CLOSED: // it used to be ok. depends on window_swap() flow
        rc += lt_emit_event(L, "quit", NULL);
        return rc;

        break; case GLEQ_WINDOW_MOVED:
        lt_wx = e.pos.x;
        lt_wy = e.pos.y;

        break; case GLEQ_WINDOW_RESIZED:
        rc += lt_emit_event(L, "resized", "dd", lt_ww = e.size.width, lt_wh = e.size.height);
        lt_resizesurface(lt_getsurface(lt_window()), lt_ww, lt_wh);

        break; case GLEQ_WINDOW_REFRESH:
        rc += lt_emit_event(L, "exposed", NULL);
        rencache_invalidate();

        break; case GLEQ_FILE_DROPPED:
        rc += lt_emit_event(L, "filedropped", "sdd", e.file.paths[0], lt_mx, lt_my);

        break; case GLEQ_KEY_PRESSED:
        case GLEQ_KEY_REPEATED:
        rc += lt_emit_event(L, "keypressed", "s", lt_key_name(buf, e.keyboard.key, e.keyboard.scancode, e.keyboard.mods));
        goto bottom;

        break; case GLEQ_KEY_RELEASED:
        rc += lt_emit_event(L, "keyreleased", "s", lt_key_name(buf, e.keyboard.key, e.keyboard.scancode, e.keyboard.mods));
        goto bottom;

        break; case GLEQ_CODEPOINT_INPUT:
        rc += lt_emit_event(L, "textinput", "s", codepoint_to_utf8_(e.codepoint));

        break; case GLEQ_BUTTON_PRESSED:
        rc += lt_emit_event(L, "mousepressed", "sddd", lt_button_name(e.mouse.button), lt_mx, lt_my, printi(1 + clicks));

        break; case GLEQ_BUTTON_RELEASED:
        clicks += e.mouse.button == GLFW_MOUSE_BUTTON_1;
        clicks_time = lt_time_ms();
        rc += lt_emit_event(L, "mousereleased", "sdd", lt_button_name(e.mouse.button), lt_mx, lt_my);

        break; case GLEQ_CURSOR_MOVED:
        lt_mx = e.pos.x - lt_wx, lt_my = e.pos.y - lt_wy;
        rc += lt_emit_event(L, "mousemoved", "dddd", lt_mx, lt_my, lt_mx - prevx, lt_my - prevy);
        prevx = lt_mx, prevy = lt_my;

        break; case GLEQ_SCROLLED:
        rc += lt_emit_event(L, "mousewheel", "f", e.scroll.y);
    }

bottom:;

    return rc;
}
