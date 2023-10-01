// ----------------------------------------------------------------------------
// time

#if 0
uint64_t time_gpu() {
    GLint64 t = 123456789;
    glGetInteger64v(GL_TIMESTAMP, &t);
    return (uint64_t)t;
}
#endif
uint64_t date() {
    time_t epoch = time(0);
    struct tm *ti = localtime(&epoch);
    return atoi64(va("%04d%02d%02d%02d%02d%02d",ti->tm_year+1900,ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec));
}
char *date_string() {
    time_t epoch = time(0);
    struct tm *ti = localtime(&epoch);
    return va("%04d-%02d-%02d %02d:%02d:%02d",ti->tm_year+1900,ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec);
}
uint64_t date_epoch() {
    time_t epoch = time(0);
    return epoch;
}
#if 0
double time_ss() {
    return glfwGetTime();
}
double time_ms() {
    return glfwGetTime() * 1000.0;
}
uint64_t time_us() {
    return (uint64_t)(glfwGetTime() * 1000000.0); // @fixme: use a high resolution timer instead, or time_gpu below
}
uint64_t sleep_us(uint64_t us) { // @fixme: use a high resolution sleeper instead
    return sleep_ms( us / 1000.0 );
}
double sleep_ms(double ms) {
    double now = time_ms();
    if( ms <= 0 ) {
#if is(win32)
        Sleep(0); // yield
#else
        usleep(0);
#endif
    } else {
#if is(win32)
        Sleep(ms);
#else
        usleep(ms * 1000);
#endif
    }
    return time_ms() - now;
}
double sleep_ss(double ss) {
    return sleep_ms( ss * 1000 ) / 1000.0;
}
#endif

// high-perf functions

#define TIMER_E3 1000ULL
#define TIMER_E6 1000000ULL
#define TIMER_E9 1000000000ULL

#ifdef CLOCK_MONOTONIC_RAW
#define TIME_MONOTONIC CLOCK_MONOTONIC_RAW
#elif defined CLOCK_MONOTONIC
#define TIME_MONOTONIC CLOCK_MONOTONIC
#else
// #define TIME_MONOTONIC CLOCK_REALTIME // untested
#endif

static uint64_t nanotimer(uint64_t *out_freq) {
    if( out_freq ) {
#if is(win32)
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        *out_freq = li.QuadPart;
//#elif is(ANDROID)
//      *out_freq = CLOCKS_PER_SEC;
#elif defined TIME_MONOTONIC
        *out_freq = TIMER_E9;
#else
        *out_freq = TIMER_E6;
#endif
    }
#if is(win32)
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (uint64_t)li.QuadPart;
//#elif is(ANDROID)
//    return (uint64_t)clock();
#elif defined TIME_MONOTONIC
    struct timespec ts;
    clock_gettime(TIME_MONOTONIC, &ts);
    return (TIMER_E9 * (uint64_t)ts.tv_sec) + ts.tv_nsec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (TIMER_E6 * (uint64_t)tv.tv_sec) + tv.tv_usec;
#endif
}

uint64_t time_ns() {
    static uint64_t epoch = 0;
    static uint64_t freq = 0;
    if( !freq ) {
        epoch = nanotimer(&freq);
    }

    uint64_t a = nanotimer(NULL) - epoch;
    uint64_t b = TIMER_E9;
    uint64_t c = freq;

    // Computes (a*b)/c without overflow, as long as both (a*b) and the overall result fit into 64-bits.
    // [ref] https://github.com/rust-lang/rust/blob/3809bbf47c8557bd149b3e52ceb47434ca8378d5/src/libstd/sys_common/mod.rs#L124
    uint64_t q = a / c;
    uint64_t r = a % c;
    return q * b + r * b / c;
}
uint64_t time_us() {
    return time_ns() / TIMER_E3;
}
uint64_t time_ms() {
    return time_ns() / TIMER_E6;
}
double time_ss() {
    return time_ns() / 1e9; // TIMER_E9;
}
double time_mm() {
    return time_ss() / 60;
}
double time_hh() {
    return time_mm() / 60;
}

void sleep_ns( double ns ) {
#if is(win32)
    if( ns >= 100 ) {
        LARGE_INTEGER li;      // Windows sleep in 100ns units
        HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
        li.QuadPart = (LONGLONG)(__int64)(-ns/100); // Negative for relative time
        SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
#else
    if( ns > 0 ) {
        struct timespec wait = {0};
        wait.tv_sec = ns / 1e9;
        wait.tv_nsec = ns - wait.tv_sec * 1e9;
        nanosleep(&wait, NULL);
#endif
    } else {
#if is(win32)
        Sleep(0); // yield, Sleep(0), SwitchToThread
#else
        usleep(0);
#endif
    }
}
void sleep_us( double us ) {
    sleep_ns(us * 1e3);
}
void sleep_ms( double ms ) {
    sleep_ns(ms * 1e6);
}
void sleep_ss( double ss ) {
    sleep_ns(ss * 1e9);
}

// ----------------------------------------------------------------------------
// timer

struct timer_internal_t {
    unsigned ms;
    unsigned (*callback)(unsigned interval, void *arg);
    void *arg;
    thread_ptr_t thd;
};

static int timer_func(void *arg) {
    struct timer_internal_t *p = (struct timer_internal_t*)arg;

    sleep_ms( p->ms );

    for( ;; ) {
        unsigned then = time_ms();

            p->ms = p->callback(p->ms, p->arg);
            if( !p->ms ) break;

        unsigned now = time_ms();
        unsigned lapse = now - then;
        int diff = p->ms - lapse;
        sleep_ms( diff <= 0 ? 0 : diff );
    }

    thread_exit(0);
    return 0;
}

static __thread array(struct timer_internal_t *) timers;

unsigned timer(unsigned ms, unsigned (*callback)(unsigned ms, void *arg), void *arg) {
    struct timer_internal_t *p = MALLOC( sizeof(struct timer_internal_t) );
    p->ms = ms;
    p->callback = callback;
    p->arg = arg;
    p->thd = thread_init( timer_func, p, "", 0 );

    array_push(timers, p);
    return array_count(timers);
}
void timer_destroy(unsigned i) {
    if( i-- ) {
        thread_join(timers[i]->thd);
        thread_term(timers[i]->thd);
        FREE(timers[i]);
        timers[i] = 0;
    }
}

// ----------------------------------------------------------------------------
// guid

//typedef vec3i guid;

guid guid_create() {
    static __thread unsigned counter = 0;
    static uint64_t appid = 0; do_once appid = hash_str(app_name());

    union conv {
        struct {
            unsigned timestamp : 32;
            unsigned threadid  : 16; // inverted order in LE
            unsigned appid     : 16; //
            unsigned counter   : 32;
        };
        vec3i v3;
    } c;
    c.timestamp = date_epoch() - 0x65000000;
    c.appid = (unsigned)appid;
    c.threadid = (unsigned)(uintptr_t)thread_current_thread_id();
    c.counter = ++counter;

    return c.v3;
}
