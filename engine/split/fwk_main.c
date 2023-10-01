// ----------------------------------------------------------------------------

static void fwk_pre_init() {
    window_icon(va("%s%s.png", app_path(), app_name()));

    glfwPollEvents();

    int i;
    #pragma omp parallel for
    for( i = 0; i <= 3; ++i) {
        /**/ if( i == 0 ) ddraw_init();// init this on thread#0 since it will be compiling shaders, and shaders need to be compiled from the very same thread than glfwMakeContextCurrent() was set up
        else if( i == 1 ) sprite_init();
        else if( i == 2 ) profiler_init();
        else if( i == 3 ) storage_mount("save/"), storage_read(), touch_init(); // for ems
    }

    // window_swap();
}
static void fwk_post_init(float refresh_rate) {
    // cook cleanup
    cook_stop();

    vfs_reload();

    // init subsystems that depend on cooked assets now

    int i;
    #pragma omp parallel for
    for( i = 0; i <= 3; ++i ) {
        if(i == 0) scene_init(); // init these on thread #0, since both will be compiling shaders, and shaders need to be compiled from the very same thread than glfwMakeContextCurrent() was set up
        if(i == 0) ui_init(); // init these on thread #0, since both will be compiling shaders, and shaders need to be compiled from the very same thread than glfwMakeContextCurrent() was set up
        if(i == 0) window_icon(va("%s.png", app_name())); // init on thread #0, because of glfw
        if(i == 0) input_init(); // init on thread #0, because of glfw
        if(i == 1) audio_init(0);
        if(i == 2) script_init(), kit_init(), midi_init();
        if(i == 3) network_init();
    }

    // display window
    glfwShowWindow(window);
    glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    randset(time_ns());
    boot_time = -time_ss(); // measure boot time, this is continued in window_stats()

    // clean any errno setup by cooking stage
    errno = 0;

    hz = refresh_rate;
    // t = glfwGetTime();
}

// ----------------------------------------------------------------------------

static
void fwk_quit(void) {
    storage_flush();
    midi_quit();
}

void fwk_init() {
    do_once {
        // install signal handlers
        ifdef(debug, trap_install());

        // init panic handler
        panic_oom_reserve = SYS_REALLOC(panic_oom_reserve, 1<<20); // 1MiB

        // init glfw
        glfw_init();

        // enable ansi console
        tty_init();

        // chdir to root (if invoked as tcc -g -run)
        // chdir(app_path());

        // skip tcc argvs (if invoked as tcc file.c fwk.c -g -run) (win)
        if( __argc > 1 ) if( strstr(__argv[0], "/tcc") || strstr(__argv[0], "\\tcc") ) {
            __argc = 0;
        }

        // create or update cook.zip file
        if( /* !COOK_ON_DEMAND && */ have_tools() && cook_jobs() ) {
            cook_start(COOK_INI, "**", 0|COOK_ASYNC|COOK_CANCELABLE );
        }

        atexit(fwk_quit);
    }
}
