//-----------------------------------------------------------------------------
// fps locking

static volatile float framerate = 0;
static volatile unsigned fps_active, timer_counter, loop_counter;
static
int fps__timing_thread(void *arg) {
    int64_t ns_excess = 0;
    while( fps_active ) {
        if( framerate <= 0 ) {
            loop_counter = timer_counter = 0;
            sleep_ms(250);
        } else {
            timer_counter++;
            int64_t tt = (int64_t)(1e9/(float)framerate) - ns_excess;
            uint64_t took = -time_ns();
        #if is(win32)
            timeBeginPeriod(1); Sleep( tt > 0 ? tt/1e6 : 0 );
        #else
            sleep_ns( (float)tt );
        #endif
            took += time_ns();
            ns_excess = took - tt;
            if( ns_excess < 0 ) ns_excess = 0;
            //puts( strf("%lld", ns_excess) );
        }
    }
    fps_active = 1;

    (void)arg;
    return thread_exit(0), 0;
}
static
void fps_locker( int on ) {
    if( on ) {
        // private threaded timer
        fps_active = 1, timer_counter = loop_counter = 0;
        thread_init( fps__timing_thread, 0, "fps__timing_thread()", 0 );
    } else {
        fps_active = 0;
    }
}
// function that locks render to desired `framerate` framerate (in FPS).
// - assumes fps_locker() was enabled beforehand.
// - returns true if must render, else 0.
static
int fps_wait() {
    if( framerate <= 0 ) return 1;
    if( !fps_active ) return 1;

    // if we throttled too much, cpu idle wait
    while( fps_active && (loop_counter > timer_counter) ) {
        //thread_yield();
        sleep_ns(100);
    }

    // max auto frameskip is 10: ie, even if speed is low paint at least one frame every 10
    enum { maxframeskip = 10 };
    if( timer_counter > loop_counter + maxframeskip ) {
        loop_counter = timer_counter;
    }
    loop_counter++;

    // only draw if we are fast enough, otherwise skip the frame
    return loop_counter >= timer_counter;
}
static
void window_vsync(float hz) {
    if( hz <= 0 ) return;
    do_once fps_locker(1);
    framerate = hz;
    fps_wait();
}

//-----------------------------------------------------------------------------

#if 0 // deprecated
static void (*hooks[64])() = {0};
static void *userdatas[64] = {0};

bool window_hook(void (*func)(), void* user) {
    window_unhook( func );
    for( int i = 0; i < 64; ++i ) {
        if( !hooks[i] ) {
            hooks[i] = func;
            userdatas[i] = user;
            return true;
        }
    }
    return false;
}
void window_unhook(void (*func)()) {
    for( int i = 0; i < 64; ++i ) {
        if(hooks[i] == func) {
            hooks[i] = 0;
            userdatas[i] = 0;
        }
    }
}
#endif

static GLFWwindow *window;
static int w, h, xpos, ypos, paused;
static int fullscreen, xprev, yprev, wprev, hprev;
static uint64_t frame_count;
static double t, dt, fps, hz = 0.00;
static char title[128] = {0};
static char screenshot_file[DIR_MAX];
static int locked_aspect_ratio = 0;

// -----------------------------------------------------------------------------
// glfw

struct app {
    GLFWwindow *window;
    int width, height, keep_running;

    struct nk_context *ctx;
    struct nk_glfw *nk_glfw;
} appHandle = {0}, *g;

static void glfw_error_callback(int error, const char *description) {
    if( is(osx) && error == 65544 ) return; // whitelisted
    PANIC("%s (error %x)", description, error);
}

void glfw_quit(void) {
    do_once {
        glfwTerminate();
    }
}

void glfw_init() {
    do_once {
        g = &appHandle;

        glfwSetErrorCallback(glfw_error_callback);
        int ok = !!glfwInit();
        assert(ok); // if(!ok) PANIC("cannot initialize glfw");

        atexit(glfw_quit); //glfwTerminate);
    }
}

void window_drop_callback(GLFWwindow* window, int count, const char** paths) {
    // @fixme: win: convert from utf8 to window16 before processing

    char pathdir[DIR_MAX]; snprintf(pathdir, DIR_MAX, "%s/import/%llu_%s/", ART, (unsigned long long)date(), ifdef(linux, getlogin(), getenv("USERNAME")));
    mkdir( pathdir, 0777 );

    int errors = 0;
    for( int i = 0; i < count; ++i ) {
        const char *src = paths[i];
        const char *dst = va("%s%s", pathdir, file_name(src));
        errors += file_copy(src, dst) ? 0 : 1;
    }

    if( errors ) PANIC("%d errors found during file dropping", errors);
    else  window_reload();

    (void)window;
}

void window_hints(unsigned flags) {
    #ifdef __APPLE__
    //glfwInitHint( GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE );
    //glfwWindowHint( GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE );
    //glfwWindowHint( GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_FALSE );
    //glfwWindowHint( GLFW_COCOA_MENUBAR, GLFW_FALSE );
    #endif

    /* We need to explicitly ask for a 3.2 context on OS X */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // osx
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); // osx, 2:#version150,3:330
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //osx
    #endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //osx+ems
    glfwWindowHint(GLFW_STENCIL_BITS, 8); //osx
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    //glfwWindowHint( GLFW_RED_BITS, 8 );
    //glfwWindowHint( GLFW_GREEN_BITS, 8 );
    //glfwWindowHint( GLFW_BLUE_BITS, 8 );
    //glfwWindowHint( GLFW_ALPHA_BITS, 8 );
    //glfwWindowHint( GLFW_DEPTH_BITS, 24 );

    //glfwWindowHint(GLFW_AUX_BUFFERS, Nth);
    //glfwWindowHint(GLFW_STEREO, GL_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

    // Prevent fullscreen window minimize on focus loss
    glfwWindowHint( GLFW_AUTO_ICONIFY, GL_FALSE );

    // Fix SRGB on intels
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // makes it non-resizable
    if(flags & WINDOW_MSAA2) glfwWindowHint(GLFW_SAMPLES, 2); // x2 AA
    if(flags & WINDOW_MSAA4) glfwWindowHint(GLFW_SAMPLES, 4); // x4 AA
    if(flags & WINDOW_MSAA8) glfwWindowHint(GLFW_SAMPLES, 8); // x8 AA
}

struct nk_glfw *window_handle_nkglfw() {
    return g->nk_glfw;
}

void glNewFrame() {

#if 0 // is(ems)
    int canvasWidth, canvasHeight;
    emscripten_get_canvas_element_size("#canvas", &canvasWidth, &canvasHeight);
    w = canvasWidth;
    h = canvasHeight;
    //printf("%dx%d\n", w, h);
#else
    //glfwGetWindowSize(window, &w, &h);
    glfwGetFramebufferSize(window, &w, &h);
    //printf("%dx%d\n", w, h);
#endif

    g->width = w;
    g->height = h;

    // blending defaults
    glEnable(GL_BLEND);

    // culling defaults
//  glEnable(GL_CULL_FACE);
//  glCullFace(GL_BACK);
//  glFrontFace(GL_CCW);

    // depth-testing defaults
    glEnable(GL_DEPTH_TEST);
//  glDepthFunc(GL_LESS);

    // depth-writing defaults
//  glDepthMask(GL_TRUE);

    // seamless cubemaps
//  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glViewport(0, 0, window_width(), window_height());

    //glClearColor(0.15,0.15,0.15,1);
    //glClearColor( clearColor.r, clearColor.g, clearColor.b, clearColor.a );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
}

bool window_create_from_handle(void *handle, float scale, unsigned flags) {
    glfw_init();
    fwk_init();
    if(!t) t = glfwGetTime();

    #if is(ems)
    scale = 100.f;
    #endif

    scale = (scale < 1 ? scale * 100 : scale);
    bool FLAGS_FULLSCREEN = scale > 100;
    bool FLAGS_FULLSCREEN_DESKTOP = scale == 100;
    bool FLAGS_WINDOWED = scale < 100;
    scale = (scale > 100 ? 100 : scale) / 100.f;
    int winWidth = window_canvas().w * scale;
    int winHeight = window_canvas().h * scale;

    window_hints(flags);

    GLFWmonitor* monitor = NULL;
    #ifndef __EMSCRIPTEN__
    if( FLAGS_FULLSCREEN || FLAGS_FULLSCREEN_DESKTOP ) {
        monitor = glfwGetPrimaryMonitor();
    }
    if( FLAGS_FULLSCREEN_DESKTOP ) {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        winWidth = mode->width;
        winHeight = mode->height;
    }
    if( FLAGS_WINDOWED ) {
        // windowed
        float ratio = (float)winWidth / (winHeight + !winHeight);
        if( flags & WINDOW_SQUARE )    winWidth = winHeight = winWidth > winHeight ? winHeight : winWidth;
        //if( flags & WINDOW_LANDSCAPE ) if( winWidth < winHeight ) winHeight = winWidth * ratio;
        if( flags & WINDOW_PORTRAIT )  if( winWidth > winHeight ) winWidth = winHeight * (1.f / ratio);
    }
    #endif

    window = handle ? handle : glfwCreateWindow(winWidth, winHeight, "", monitor, NULL);
    if( !window ) return PANIC("GLFW Window creation failed"), false;

    glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    if( flags & WINDOW_FIXED ) { // disable resizing
        glfwSetWindowSizeLimits(window, w, h, w, h);
    }
    if( flags & (WINDOW_SQUARE | WINDOW_PORTRAIT | WINDOW_LANDSCAPE | WINDOW_ASPECT) ) { // keep aspect ratio
        window_aspect_lock(w, h);
    }

    #ifndef __EMSCRIPTEN__
    if( FLAGS_WINDOWED ) {
        // center window
        monitor = monitor ? monitor : glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        int area_width = mode->width, area_height = mode->height;
        glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &area_width, &area_height);
        glfwSetWindowPos(window, xpos = xpos + (area_width - winWidth) / 2, ypos = ypos + (area_height - winHeight) / 2);
        //printf("%dx%d @(%d,%d) [res:%dx%d]\n", winWidth, winHeight, xpos,ypos, area_width, area_height );

        wprev = w, hprev = h;
        xprev = xpos, yprev = ypos;
    }
    #endif

    glfwMakeContextCurrent(window);

    #if is(ems)
    if( FLAGS_FULLSCREEN ) window_fullscreen(1);
    #else
    gladLoadGL(glfwGetProcAddress);
    #endif

    glDebugEnable();
    
    // setup nuklear ui
    ui_ctx = nk_glfw3_init(&nk_glfw, window, NK_GLFW3_INSTALL_CALLBACKS);
   
    //glEnable(GL_TEXTURE_2D);

    // 0:disable vsync, 1:enable vsync, <0:adaptive (allow vsync when framerate is higher than syncrate and disable vsync when framerate drops below syncrate)
    flags |= optioni("--vsync", 1) || flag("--vsync") ? WINDOW_VSYNC : WINDOW_VSYNC_DISABLED;
    flags |= optioni("--vsync-adaptive", 0) || flag("--vsync-adaptive") ? WINDOW_VSYNC_ADAPTIVE : 0;
    int has_adaptive_vsync = glfwExtensionSupported("WGL_EXT_swap_control_tear") || glfwExtensionSupported("GLX_EXT_swap_control_tear") || glfwExtensionSupported("EXT_swap_control_tear");
    int wants_adaptive_vsync = (flags & WINDOW_VSYNC_ADAPTIVE);
    int interval = has_adaptive_vsync && wants_adaptive_vsync ? -1 : (flags & WINDOW_VSYNC_DISABLED ? 0 : 1);
    glfwSwapInterval(interval);

    const GLFWvidmode *mode = glfwGetVideoMode(monitor ? monitor : glfwGetPrimaryMonitor());
    PRINTF("Monitor: %s (%dHz, vsync=%d)\n", glfwGetMonitorName(monitor ? monitor : glfwGetPrimaryMonitor()), mode->refreshRate, interval);
    PRINTF("GPU device: %s\n", glGetString(GL_RENDERER));
    PRINTF("GPU driver: %s\n", glGetString(GL_VERSION));

    g->ctx = ui_ctx;
    g->nk_glfw = &nk_glfw;
    g->window = window;
    g->width = window_width();
    g->height = window_height();

    // window_cursor(flags & WINDOW_NO_MOUSE ? false : true);
    glfwSetDropCallback(window, window_drop_callback);

    // camera inits for fwk_pre_init() -> ddraw_flush() -> get_active_camera()
    // static camera_t cam = {0}; id44(cam.view); id44(cam.proj); extern camera_t *last_camera; last_camera = &cam;
    fwk_pre_init();

        // display a progress bar meanwhile cook is working in the background
        // Sleep(500);
        if( !COOK_ON_DEMAND )
        if( file_exist(COOK_INI) && cook_jobs() )
        while( cook_progress() < 100 ) {
            for( int frames = 0; frames < 2/*10*/ && window_swap(); frames += cook_progress() >= 100 ) {
                window_title(va("%s %.2d%%", cook_cancelling ? "Aborting" : "Cooking assets", cook_progress()));
                if( input(KEY_ESC) ) cook_cancel();

                glNewFrame();

                static float previous[JOBS_MAX] = {0};

                #define ddraw_progress_bar(JOB_ID, JOB_MAX, PERCENT) do { \
                   /* NDC coordinates (2d): bottom-left(-1,-1), center(0,0), top-right(+1,+1) */ \
                   float progress = (PERCENT+1) / 100.f; if(progress > 1) progress = 1; \
                   float speed = progress < 1 ? 0.2f : 0.5f; \
                   float smooth = previous[JOB_ID] = progress * speed + previous[JOB_ID] * (1-speed); \
                   \
                   float pixel = 2.f / window_height(), dist = smooth*2-1, y = pixel*3*JOB_ID; \
                   if(JOB_ID==0)ddraw_line(vec3(-1,y-pixel*2,0), vec3(1,   y-pixel*2,0)); /* full line */ \
                   ddraw_line(vec3(-1,y-pixel  ,0), vec3(dist,y-pixel  ,0)); /* progress line */ \
                   ddraw_line(vec3(-1,y+0      ,0), vec3(dist,y+0      ,0)); /* progress line */ \
                   ddraw_line(vec3(-1,y+pixel  ,0), vec3(dist,y+pixel  ,0)); /* progress line */ \
                   if(JOB_ID==JOB_MAX-1)ddraw_line(vec3(-1,y+pixel*2,0), vec3(1,   y+pixel*2,0)); /* full line */ \
                } while(0)

                for(int i = 0; i < cook_jobs(); ++i) ddraw_progress_bar(i, cook_jobs(), jobs[i].progress);
                // ddraw_progress_bar(0, 1, cook_progress());

                ddraw_flush();

                do_once window_visible(1);

                // render progress bar at 30Hz + give the cook threads more time to actually cook the assets.
                // no big deal since progress bar is usually quiet when cooking assets most of the time.
                // also, make the delay even larger when window is minimized or hidden.
                // shaved cook times: 88s -> 57s (tcc), 50s -> 43s (vc)
                sleep_ms( window_has_visible() && window_has_focus() ? 8 : 16 );
            }
            // set black screen
            glNewFrame();
            window_swap();
            window_title("");
        }

    if(cook_cancelling) cook_stop(), exit(-1);

    fwk_post_init(mode->refreshRate);
    return true;
}

bool window_create(float scale, unsigned flags) {
    return window_create_from_handle(NULL, scale, flags);
}

static double boot_time = 0;

char* window_stats() {
    static double num_frames = 0, begin = FLT_MAX, prev_frame = 0;

    double now = time_ss();
    if( boot_time < 0 ) boot_time = now;

    if( begin > now ) {
        begin = now;
        num_frames = 0;
    }
    if( (now - begin) >= 0.25f ) {
        fps = num_frames * (1.f / (now - begin));
    }
    if( (now - begin) > 1 ) {
        begin = now + ((now - begin) - 1);
        num_frames = 0;
    }

    const char *cmdline = app_cmdline();

    // @todo: print %used/%avail kib mem, %used/%avail objs as well
    static char buf[256];
    snprintf(buf, 256, "%s | boot %.2fs | %5.2ffps (%.2fms)%s%s", title, !boot_time ? now : boot_time, fps, (now - prev_frame) * 1000.f, cmdline[0] ? " | ":"", cmdline[0] ? cmdline:"");

    prev_frame = now;
    ++num_frames;

    return buf + 3 * (buf[0] == ' ');
}

int window_frame_begin() {
    glfwPollEvents();

    // we cannot simply terminate threads on some OSes. also, aborted cook jobs could leave temporary files on disc.
    // so let's try to be polite: we will be disabling any window closing briefly until all cook is either done or canceled.
    static bool has_cook; do_once has_cook = !COOK_ON_DEMAND && file_exist(COOK_INI) && cook_jobs();
    if( has_cook ) {
        has_cook = cook_progress() < 100;
        if( glfwWindowShouldClose(g->window) ) cook_cancel();
        glfwSetWindowShouldClose(g->window, GLFW_FALSE);
    }

    if( glfwWindowShouldClose(g->window) ) {
        return 0;
    }

    glNewFrame();

    ui_create();

    profile_render();
 
#if 0 // deprecated
    // run user-defined hooks
    for(int i = 0; i < 64; ++i) {
        if( hooks[i] ) hooks[i]( userdatas[i] );
    }
#endif

    double now = paused ? t : glfwGetTime();
    dt = now - t;
    t = now;

    char *st = window_stats();
    static double timer = 0;
    timer += window_delta();
    if( timer >= 0.25 ) {
        glfwSetWindowTitle(window, st);
        timer = 0;
    }

    void input_update();
    input_update();

    return 1;
}

void window_frame_end() {
    // flush batching systems that need to be rendered before frame swapping. order matters.
    {
        font_goto(0,0);        
        touch_flush();
        sprite_flush();

        // flush all debugdraw calls before swap
        dd_ontop = 0;
        ddraw_flush();
        glClear(GL_DEPTH_BUFFER_BIT);
        dd_ontop = 1;
        ddraw_flush();

        ui_render();
    }

#if !is(ems)
    // save screenshot if queued
    if( screenshot_file[0] ) {
        int n = 3;
        void *rgb = screenshot(n);
        stbi_flip_vertically_on_write(true);
        if(!stbi_write_png(screenshot_file, w, h, n, rgb, n * w) ) {
            PANIC("!could not write screenshot file `%s`\n", screenshot_file);
        }
        screenshot_file[0] = 0;
    }
    if( record_active() ) {
        void record_frame();
        record_frame();
    }
#endif
}

void window_frame_swap() {
    // glFinish();
#if !is(ems)
    window_vsync(hz);
#endif
    glfwSwapBuffers(window);
    // emscripten_webgl_commit_frame();
}

static
void window_shutdown() {
    do_once {
        #if ENABLE_SELFIES

        snprintf(screenshot_file, DIR_MAX, "%s.png", app_name());

        int n = 3;
        void *rgb = screenshot(n);
        stbi_flip_vertically_on_write(true);
        if(!stbi_write_png(screenshot_file, w, h, n, rgb, n * w) ) {
            PANIC("!could not write screenshot file `%s`\n", screenshot_file);
        }
        screenshot_file[0] = 0;

        #endif

        window_loop_exit(); // finish emscripten loop automatically        
    }
}

int window_swap() {

    // end frame
    if( frame_count > 0 ) {
        window_frame_end();
        window_frame_swap();
    }

    ++frame_count;

    // begin frame
    int ready = window_frame_begin();
    if( !ready ) {
        window_shutdown();
        return 0;
    }
    return 1;
}

static
void (*window_render_callback)(void* loopArg);

static
void window_loop_wrapper( void *loopArg ) {
    if( window_frame_begin() ) {
        window_render_callback(loopArg);
        window_frame_end();
        window_frame_swap();
    } else {
        do_once window_shutdown();
    }
}

void window_loop(void (*user_function)(void* loopArg), void* loopArg ) {
#if is(ems)
    window_render_callback = user_function;
    emscripten_set_main_loop_arg(window_loop_wrapper, loopArg, 0, 1);
#else
    g->keep_running = true;
    while (g->keep_running)
        user_function(loopArg);
#endif /* __EMSCRIPTEN__ */
}

void window_loop_exit() {
#if is(ems)
    emscripten_cancel_main_loop();
#else
    g->keep_running = false;
#endif /* __EMSCRIPTEN__ */
}

vec2 window_canvas() {
#if is(ems)
    int width = EM_ASM_INT_V(return window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth);
    int height = EM_ASM_INT_V(return window.innerHeight|| document.documentElement.clientHeight|| document.body.clientHeight);
    return vec2(width, height);
#else
    glfw_init();
    const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
    assert( mode );
    return vec2(mode->width, mode->height);
#endif /* __EMSCRIPTEN__ */
}

int window_width() {
    return w;
}
int window_height() {
    return h;
}
double window_time() {
    return t;
}
double window_delta() {
    return dt;
}

double window_fps() {
    return fps;
}
void window_fps_lock(float fps) {
    hz = fps;
}
void window_fps_unlock() {
    hz = 0;
}
double window_fps_target() {
    return hz;
}

uint64_t window_frame() {
    return frame_count;
}
void window_title(const char *title_) {
    snprintf(title, 128, "%s", title_);
    if( !title[0] ) glfwSetWindowTitle(window, title);
}
void window_color(unsigned color) {
    unsigned b = (color >>  0) & 255;
    unsigned g = (color >>  8) & 255;
    unsigned r = (color >> 16) & 255;
    unsigned a = (color >> 24) & 255;
    glClearColor(r / 255.0, g / 255.0, b / 255.0, 1.0);
}
void window_icon(const char *file_icon) {
    unsigned len = file_size(file_icon); // len = len ? len : vfs_size(file_icon); // @fixme: reenable this to allow icons to be put in cooked .zipfiles
    if( len ) {
        void *data = file_read(file_icon); // data = data ? data : vfs_read(file_icon); // @fixme: reenable this to allow icons to be put in cooked .zipfiles
        if( data ) {
            image_t img = image_from_mem(data, len, IMAGE_RGBA);
            if( img.w && img.h && img.pixels ) {
                GLFWimage images[1];
                images[0].width = img.w;
                images[0].height = img.h;
                images[0].pixels = img.pixels;
                glfwSetWindowIcon(window, 1, images);
                return;
            }
        }
    }
#if is(win32)
    HANDLE hIcon = LoadImageA(0, file_icon, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    if( hIcon ) {
        HWND hWnd = glfwGetWin32Window(window);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
        SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
        return;
    }
#endif
}
void* window_handle() {
    return window;
}

void window_reload() {
    // @todo: save_on_exit();
    fflush(0);
    chdir(app_path());
    execv(__argv[0], __argv);
    exit(0);
}

int window_record(const char *outfile_mp4) {
    record_start(outfile_mp4);
    // @todo: if( flags & RECORD_MOUSE )
    if( record_active() ) window_cursor_shape(CURSOR_SW_AUTO); else window_cursor_shape(CURSOR_HW_ARROW);
    return record_active();
}


// -----------------------------------------------------------------------------
// fullscreen

static
GLFWmonitor *window_find_monitor(int wx, int wy) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();

    // find best monitor given current window coordinates. @todo: select by ocuppied window area inside each monitor instead.
    int num_monitors = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&num_monitors);
#if is(ems)
    return *monitors;
#else
    for( int i = 0; i < num_monitors; ++i) {
        int mx = 0, my = 0, mw = 0, mh = 0;
        glfwGetMonitorWorkarea(monitors[i], &mx, &my, &mw, &mh);
        monitor = wx >= mx && wx <= (mx+mw) && wy >= my && wy <= (my+mh) ? monitors[i] : monitor;
    }
    return monitor;
#endif
}

#if 0 // to deprecate

void window_fullscreen(int enabled) {
    fullscreen = !!enabled;
#ifndef __EMSCRIPTEN__
    if( fullscreen ) {
        int wx = 0, wy = 0; glfwGetWindowPos(window, &wx, &wy);
        GLFWmonitor *monitor = window_find_monitor(wx, wy);

        wprev = w, hprev = h, xprev = wx, yprev = wy; // save window context for further restoring

        int width, height;
        glfwGetMonitorWorkarea(monitor, NULL, NULL, &width, &height);
        glfwSetWindowMonitor(window, monitor, 0, 0, width, height, GLFW_DONT_CARE);
    } else {
        glfwSetWindowMonitor(window, NULL, xpos, ypos, wprev, hprev, GLFW_DONT_CARE);
        glfwSetWindowPos(window, xprev, yprev);
    }
#endif
}
int window_has_fullscreen() {
    return fullscreen;
}

#else

int window_has_fullscreen() {
#if is(ems)
    EmscriptenFullscreenChangeEvent fsce;
    emscripten_get_fullscreen_status(&fsce);
    return !!fsce.isFullscreen;
#else
    return !!glfwGetWindowMonitor(g->window);
#endif /* __EMSCRIPTEN__ */
}

void window_fullscreen(int enabled) {
    if( window_has_fullscreen() == !!enabled ) return;

#if is(ems)

#if 0 // deprecated: crash
    if( enabled ) {
        emscripten_exit_soft_fullscreen();

        /* Workaround https://github.com/kripken/emscripten/issues/5124#issuecomment-292849872 */
        EM_ASM(JSEvents.inEventHandler = true);
        EM_ASM(JSEvents.currentEventHandler = {allowsDeferredCalls:true});

        EmscriptenFullscreenStrategy strategy = {0};
        strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH; // _ASPECT
        strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF; // _NONE _HIDEF
        strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT; // _NEAREST

        emscripten_request_fullscreen_strategy(NULL, EM_FALSE/*EM_TRUE*/, &strategy);
        //emscripten_enter_soft_fullscreen(NULL, &strategy);
    } else {
        emscripten_exit_fullscreen();
    }
#else
    if( enabled )
    EM_ASM(Module.requestFullscreen(1, 1)); 
    else
    EM_ASM(Module.exitFullscreen()); 
#endif

#else

#if 0
    if( enabled ) {
        /*glfwGetWindowPos(g->window, &g->window_xpos, &g->window_ypos);*/
        glfwGetWindowSize(g->window, &g->width, &g->height);
        glfwSetWindowMonitor(g->window, glfwGetPrimaryMonitor(), 0, 0, g->width, g->height, GLFW_DONT_CARE);
    } else {            
        glfwSetWindowMonitor(g->window, NULL, 0, 0, g->width, g->height, GLFW_DONT_CARE);
    }
#else
    if( enabled ) {
        int wx = 0, wy = 0; glfwGetWindowPos(window, &wx, &wy);
        GLFWmonitor *monitor = window_find_monitor(wx, wy);

        wprev = w, hprev = h, xprev = wx, yprev = wy; // save window context for further restoring

        int width, height;
        glfwGetMonitorWorkarea(monitor, NULL, NULL, &width, &height);
        glfwSetWindowMonitor(window, monitor, 0, 0, width, height, GLFW_DONT_CARE);
    } else {
        glfwSetWindowMonitor(window, NULL, xpos, ypos, wprev, hprev, GLFW_DONT_CARE);
        glfwSetWindowPos(window, xprev, yprev);
    }
#endif

#endif
}

#endif

void window_pause(int enabled) {
    paused = enabled;
}
int window_has_pause() {
    return paused;
}
void window_focus() {
    glfwFocusWindow(window);
}
int window_has_focus() {
    return !!glfwGetWindowAttrib(window, GLFW_FOCUSED);
}
void window_cursor(int visible) {
    glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}
int window_has_cursor() {
    return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}
void window_cursor_shape(unsigned mode) {
    static GLFWcursor* cursors[7] = { 0 };
    static unsigned enums[7] = {
        0,
        GLFW_ARROW_CURSOR,
        GLFW_IBEAM_CURSOR,
        GLFW_CROSSHAIR_CURSOR,
        GLFW_HAND_CURSOR,
        GLFW_HRESIZE_CURSOR,
        GLFW_VRESIZE_CURSOR,
    };
    do_once {
        static unsigned pixels[16 * 16] = { 0x01000000 }; // ABGR(le) glfw3 note: A(0x00) means 0xFF for some reason
        static GLFWimage image = { 16, 16, (void*)pixels };
        static GLFWcursor* empty;
        for( int x = 0; x < 16 * 16; ++x ) pixels[x] = pixels[0];
        empty = glfwCreateCursor(&image, 0, 0);

        for(int i = 0; i < countof(enums); ++i) cursors[i] = i ? glfwCreateStandardCursor( enums[i] ) : empty;
    }
    if( mode == CURSOR_SW_AUTO ) { // UI (nuklear) driven cursor
        nk_style_show_cursor(ui_handle());
        glfwSetCursor(window, cursors[0] );
    } else {
        nk_style_hide_cursor(ui_handle());
        glfwSetCursor(window, mode < countof(enums) ? cursors[mode] : NULL);
    }
}
void window_visible(int visible) {
    if(!window) return;
    //if(window) (visible ? glfwRestoreWindow : glfwIconifyWindow)(window);
    (visible ? glfwShowWindow : glfwHideWindow)(window);
    // call glfwpollevents in linux to flush visiblity changes that would happen in next frame otherwise
    #if is(linux) || is(osx)
    glfwPollEvents();
    #endif
}
int window_has_visible() {
    return glfwGetWindowAttrib(window, GLFW_VISIBLE);
}

void window_screenshot(const char* outfile_png) {
    snprintf(screenshot_file, DIR_MAX, "%s", outfile_png ? outfile_png : "");
}

double window_aspect() {
    return (double)w / (h + !h);
}
void window_aspect_lock(unsigned numer, unsigned denom) {
    if(!window) return;
    if( numer * denom ) {
        glfwSetWindowAspectRatio(window, numer, denom);
    } else {
        glfwSetWindowAspectRatio(window, GLFW_DONT_CARE, GLFW_DONT_CARE);
    }
}
void window_aspect_unlock() {
    if(!window) return;
    window_aspect_lock(0, 0);
}
