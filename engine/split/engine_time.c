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
    static __thread uint64_t epoch = 0;
    static __thread uint64_t freq = 0;
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

// ----------------------------------------------------------------------------
// ease

float ease_zero(float t) { return 0; }
float ease_one(float t) { return 1; }
float ease_linear(float t) { return t; }

float ease_out_sine(float t) { return sinf(t*(C_PI*0.5f)); }
float ease_out_quad(float t) { return -(t*(t-2)); }
float ease_out_cubic(float t) { float f=t-1; return f*f*f+1; }
float ease_out_quart(float t) { float f=t-1; return f*f*f*(1-t)+1; }
float ease_out_quint(float t) { float f=(t-1); return f*f*f*f*f+1; }
float ease_out_expo(float t) { return (t >= 1) ? t : 1-powf(2,-10*t); }
float ease_out_circ(float t) { return sqrtf((2-t)*t); }
float ease_out_back(float t) { float f=1-t; return 1-(f*f*f-f*sinf(f*C_PI)); }
float ease_out_elastic(float t) { return sinf(-13*(C_PI*0.5f)*(t+1))*powf(2,-10*t)+1; }
float ease_out_bounce(float t) { return (t < 4.f/11) ? (121.f*t*t)/16 : (t < 8.f/11) ? (363.f/40*t*t)-(99.f/10*t)+17.f/5 : (t < 9.f/10) ? (4356.f/361*t*t)-(35442.f/1805*t)+16061.f/1805 : (54.f/5*t*t)-(513.f/25*t)+268.f/25; }

float ease_in_sine(float t) { return 1+sinf((t-1)*(C_PI*0.5f)); }
float ease_in_quad(float t) { return t*t; }
float ease_in_cubic(float t) { return t*t*t; }
float ease_in_quart(float t) { return t*t*t*t; }
float ease_in_quint(float t) { return t*t*t*t*t; }
float ease_in_expo(float t) { return (t <= 0) ? t : powf(2,10*(t-1)); }
float ease_in_circ(float t) { return 1-sqrtf(1-(t*t)); }
float ease_in_back(float t) { return t*t*t-t*sinf(t*C_PI); }
float ease_in_elastic(float t) { return sinf(13*(C_PI*0.5f)*t)*powf(2,10*(t-1)); }
float ease_in_bounce(float t) { return 1-ease_out_bounce(1-t); }

float ease_inout_sine(float t) { return 0.5f*(1-cosf(t*C_PI)); }
float ease_inout_quad(float t) { return (t < 0.5f) ? 2*t*t : (-2*t*t)+(4*t)-1; }
float ease_inout_cubic(float t) { float f; return (t < 0.5f) ? 4*t*t*t : (f=(2*t)-2,0.5f*f*f*f+1); }
float ease_inout_quart(float t) { float f; return (t < 0.5f) ? 8*t*t*t*t : (f=(t-1),-8*f*f*f*f+1); }
float ease_inout_quint(float t) { float f; return (t < 0.5f) ? 16*t*t*t*t*t : (f=((2*t)-2),0.5f*f*f*f*f*f+1); }
float ease_inout_expo(float t) { return (t <= 0 || t >= 1) ? t : t < 0.5f ? 0.5f*powf(2,(20*t)-10) : -0.5f*powf(2,(-20*t)+10)+1; }
float ease_inout_circ(float t) { return t < 0.5f ? 0.5f*(1-sqrtf(1-4*(t*t))) : 0.5f*(sqrtf(-((2*t)-3)*((2*t)-1))+1); }
float ease_inout_back(float t) { float f; return t < 0.5f ? (f=2*t,0.5f*(f*f*f-f*sinf(f*C_PI))) : (f=(1-(2*t-1)),0.5f*(1-(f*f*f-f*sinf(f*C_PI)))+0.5f); }
float ease_inout_elastic(float t) { return t < 0.5f ? 0.5f*sinf(13*(C_PI*0.5f)*(2*t))*powf(2,10*((2*t)-1)) : 0.5f*(sinf(-13*(C_PI*0.5f)*((2*t-1)+1))*powf(2,-10*(2*t-1))+2); }
float ease_inout_bounce(float t) { return t < 0.5f ? 0.5f*ease_in_bounce(t*2) : 0.5f*ease_out_bounce(t*2-1)+0.5f; }

float ease_inout_perlin(float t) { float t3=t*t*t,t4=t3*t,t5=t4*t; return 6*t5-15*t4+10*t3; }

float ease(float t01, unsigned mode) {
    typedef float (*easing)(float);
    easing modes[] = {
        ease_out_sine,
        ease_out_quad,
        ease_out_cubic,
        ease_out_quart,
        ease_out_quint,
        ease_out_expo,
        ease_out_circ,
        ease_out_back,
        ease_out_elastic,
        ease_out_bounce,

        ease_in_sine,
        ease_in_quad,
        ease_in_cubic,
        ease_in_quart,
        ease_in_quint,
        ease_in_expo,
        ease_in_circ,
        ease_in_back,
        ease_in_elastic,
        ease_in_bounce,

        ease_inout_sine,
        ease_inout_quad,
        ease_inout_cubic,
        ease_inout_quart,
        ease_inout_quint,
        ease_inout_expo,
        ease_inout_circ,
        ease_inout_back,
        ease_inout_elastic,
        ease_inout_bounce,

        ease_zero,
        ease_one,
        ease_linear,
        ease_inout_perlin,
    };
    return modes[clampi(mode, 0, countof(modes))](clampf(t01,0,1));
}

float ease_pong(float t, unsigned fn) { return 1 - ease(t, fn); }
float ease_ping_pong(float t, unsigned fn1, unsigned fn2) { return t < 0.5 ? ease(t*2,fn1) : ease(1-(t-0.5)*2,fn2); }
float ease_pong_ping(float t, unsigned fn1, unsigned fn2) { return 1 - ease_ping_pong(t,fn1,fn2); }


const char **ease_enums() {
    static const char *list[] = {
        "ease_out_sine",
        "ease_out_quad",
        "ease_out_cubic",
        "ease_out_quart",
        "ease_out_quint",
        "ease_out_expo",
        "ease_out_circ",
        "ease_out_back",
        "ease_out_elastic",
        "ease_out_bounce",

        "ease_in_sine",
        "ease_in_quad",
        "ease_in_cubic",
        "ease_in_quart",
        "ease_in_quint",
        "ease_in_expo",
        "ease_in_circ",
        "ease_in_back",
        "ease_in_elastic",
        "ease_in_bounce",

        "ease_inout_sine",
        "ease_inout_quad",
        "ease_inout_cubic",
        "ease_inout_quart",
        "ease_inout_quint",
        "ease_inout_expo",
        "ease_inout_circ",
        "ease_inout_back",
        "ease_inout_elastic",
        "ease_inout_bounce",

        "ease_zero",
        "ease_one",
        "ease_linear",
        "ease_inout_perlin",

        0
    };
    return list;
}

const char *ease_enum(unsigned mode) {
    return mode[ ease_enums() ];
}

/*AUTORUN {
    ENUM(EASE_LINEAR|EASE_OUT);
    ENUM(EASE_SINE|EASE_OUT);
    ENUM(EASE_QUAD|EASE_OUT);
    ENUM(EASE_CUBIC|EASE_OUT);
    ENUM(EASE_QUART|EASE_OUT);
    ENUM(EASE_QUINT|EASE_OUT);
    ENUM(EASE_EXPO|EASE_OUT);
    ENUM(EASE_CIRC|EASE_OUT);
    ENUM(EASE_BACK|EASE_OUT);
    ENUM(EASE_ELASTIC|EASE_OUT);
    ENUM(EASE_BOUNCE|EASE_OUT);

    ENUM(EASE_SINE|EASE_IN);
    ENUM(EASE_QUAD|EASE_IN);
    ENUM(EASE_CUBIC|EASE_IN);
    ENUM(EASE_QUART|EASE_IN);
    ENUM(EASE_QUINT|EASE_IN);
    ENUM(EASE_EXPO|EASE_IN);
    ENUM(EASE_CIRC|EASE_IN);
    ENUM(EASE_BACK|EASE_IN);
    ENUM(EASE_ELASTIC|EASE_IN);
    ENUM(EASE_BOUNCE|EASE_IN);

    ENUM(EASE_SINE|EASE_INOUT);
    ENUM(EASE_QUAD|EASE_INOUT);
    ENUM(EASE_CUBIC|EASE_INOUT);
    ENUM(EASE_QUART|EASE_INOUT);
    ENUM(EASE_QUINT|EASE_INOUT);
    ENUM(EASE_EXPO|EASE_INOUT);
    ENUM(EASE_CIRC|EASE_INOUT);
    ENUM(EASE_BACK|EASE_INOUT);
    ENUM(EASE_ELASTIC|EASE_INOUT);
    ENUM(EASE_BOUNCE|EASE_INOUT);

    ENUM(EASE_ZERO);
    ENUM(EASE_ONE);
    ENUM(EASE_LINEAR);
    ENUM(EASE_INOUT_PERLIN);
};*/

// ----------------------------------------------------------------------------
// tween

tween_t tween() {
    tween_t tw = {0};
    return tw;
}

float tween_update(tween_t *tw, float dt) {
    if( !array_count(tw->keyframes) ) return 0.0f;

    for( int i = 0, end = array_count(tw->keyframes) - 1; i < end; ++i ) {
        tween_keyframe_t *kf1 = &tw->keyframes[i];
        tween_keyframe_t *kf2 = &tw->keyframes[i + 1];
        if (tw->time >= kf1->t && tw->time <= kf2->t) {
            float localT = (tw->time - kf1->t) / (kf2->t - kf1->t);
            float easedT = ease(localT, kf1->ease);
            tw->result = mix3(kf1->v, kf2->v, easedT);
            break;
        }
    }

    float done = (tw->time / tw->duration);
    tw->time += dt;
    return clampf(done, 0.0f, 1.0f);
}

void tween_reset(tween_t *tw) {
    tw->time = 0.0f;
}

void tween_destroy(tween_t *tw) {
    tween_t tw_ = {0};
    array_free(tw->keyframes);
    *tw = tw_;
}

static INLINE
int tween_comp_keyframes(const void *a, const void *b) {
    float t1 = ((const tween_keyframe_t*)a)->t;
    float t2 = ((const tween_keyframe_t*)b)->t;
    return (t1 > t2) - (t1 < t2);
}

void tween_setkey(tween_t *tw, float t, vec3 v, unsigned mode) {
    tween_keyframe_t keyframe = { t, v, mode };
    array_push(tw->keyframes, keyframe);
    array_sort(tw->keyframes, tween_comp_keyframes);
    tw->duration = array_back(tw->keyframes)->t;
}

void tween_delkey(tween_t *tw, float t) { // @todo: untested
    for( int i = 0, end = array_count(tw->keyframes); i < end; i++ ) {
        if( tw->keyframes[i].t == t ) {
            array_erase_slow(tw->keyframes, i);
            tw->duration = array_back(tw->keyframes)->t;
            return;
        }
    }
}

// ----------------------------------------------------------------------------
// curve

curve_t curve() {
    curve_t c = {0};
    return c;
}

static INLINE
vec3 catmull( vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t ) {
    float t2 = t*t;
    float t3 = t*t*t;

    vec3 c;
    c.x = 0.5 * ((2 * p1.x) + (-p0.x + p2.x) * t + (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 + (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3);
    c.y = 0.5 * ((2 * p1.y) + (-p0.y + p2.y) * t + (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 + (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3);
    c.z = 0.5 * ((2 * p1.z) + (-p0.z + p2.z) * t + (2 * p0.z - 5 * p1.z + 4 * p2.z - p3.z) * t2 + (-p0.z + 3 * p1.z - 3 * p2.z + p3.z) * t3);
    return c;
}

void curve_add(curve_t *c, vec3 p) {
    array_push(c->points, p);
}

void curve_end( curve_t *c, int k ) {
    ASSERT( k > 0 );

    array_free(c->lengths);
    array_free(c->samples);
    array_free(c->indices);
    array_free(c->colors);

    // refit points[N] to samples[K]
    int N = array_count(c->points);
    if( k < N ) {
        // truncate: expected k-points lesser or equal than existing N points
        for( int i = 0; i <= k; ++i ) {
            float s = (float)i / k;
            int t = s * (N-1);
            array_push(c->samples, c->points[t]);

            float p = fmod(i, N-1) / (N-1); // [0..1)
            int is_control_point = p <= 0 || p >= 1;
            array_push(c->colors, is_control_point ? ORANGE: BLUE);
        }

    } else {
        // interpolate: expected k-points greater than existing N-points
        --N;
        int upper = N - (k%N);
        int lower = (k%N);
        if(upper < lower)
        k += upper;
        else
        k -= lower;

        int points_per_segment = (k / N);
        ++N;

        int looped = len3sq(sub3(c->points[0], *array_back(c->points))) < 0.1;

        for( int i = 0; i <= k; ++i ) {
            int point = i % points_per_segment;
            float p = point / (float)points_per_segment; // [0..1)
            int t = i / points_per_segment;

            // linear
            vec3 l = mix3(c->points[t], c->points[t+(i!=k)], p);

            // printf("%d) %d>%d %f\n", i, t, t+(i!=k), p);
            ASSERT(p <= 1);

            // catmull
            int p0 = t - 1;
            int p1 = t + 0;
            int p2 = t + 1;
            int p3 = t + 2;
            if( looped )
            {
                int M = N-1;
                if(p0<0) p0+=M; else if(p0>=M) p0-=M;
                if(p1<0) p1+=M; else if(p1>=M) p1-=M;
                if(p2<0) p2+=M; else if(p2>=M) p2-=M;
                if(p3<0) p3+=M; else if(p3>=M) p3-=M;
            }
            else
            {
                int M = N-1;
                if(p0<0) p0=0; else if(p0>=M) p0=M;
                if(p1<0) p1=0; else if(p1>=M) p1=M;
                if(p2<0) p2=0; else if(p2>=M) p2=M;
                if(p3<0) p3=0; else if(p3>=M) p3=M;
            }
            vec3 m = catmull(c->points[p0],c->points[p1],c->points[p2],c->points[p3],p);
            l = m;

            array_push(c->samples, l);

            int is_control_point = p <= 0 || p >= 1;
            array_push(c->colors, is_control_point ? ORANGE: BLUE);
        }
    }

    array_push(c->lengths, 0 );
    for( int i = 1; i <= k; ++i ) {
        // approximate curve length at every sample point
        array_push(c->lengths, len3(sub3(c->samples[i], c->samples[i-1])) + c->lengths[i-1] );
    }
    // normalize lengths to be between 0 and 1
    float maxv = c->lengths[k];
    for( int i = 1; i <= k; ++i ) c->lengths[i] /= maxv;

    array_push(c->indices, 0 );
    for( int i = 0/*1*/; i </*=*/ k; ++i ) {
        float s = (float)i / (k-1); //k;
        int j; // j = so that lengths[j] <= s < lengths[j+1];

        // j = Index of the highest length that is less or equal to s
        // Can be optimized with a binary search instead
        for( j = *array_back(c->indices) + 1; j </*=*/ k; ++j ) {
            if( c->lengths[j] </*=*/ s ) continue;
            break;
        }

        if (c->lengths[j] > 0.01)
        array_push(c->indices, j );
    }
}

vec3 curve_eval(curve_t *c, float dt, unsigned *color) {
    dt = clampf(dt, 0.0f, 1.0f);
    int l = (int)(array_count(c->indices) - 1);
    int p = (int)(dt * l);
    int t = c->indices[p];

    t %= (array_count(c->indices)-1);
    vec3 pos = mix3(c->samples[t], c->samples[t+1], dt * l - p);
    if(color) *color = c->colors[t];

    return pos;
}

void curve_destroy(curve_t *c) {
    array_free(c->lengths);
    array_free(c->colors);
    array_free(c->samples);
    array_free(c->points);
    array_free(c->indices);
}
