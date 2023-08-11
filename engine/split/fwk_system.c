#if (is(tcc) && is(linux)) || (is(gcc) && !is(mingw)) // || is(clang)
int __argc; char **__argv;
#if !is(ems)
__attribute__((constructor)) void init_argcv(int argc, char **argv) { __argc = argc; __argv = argv; }
#endif
#endif

const char *app_path() { // @fixme: should return absolute path always. see tcc -g -run
    static char buffer[1024] = {0};
    if( buffer[0] ) return buffer;
#if is(win32)
    unsigned length = GetModuleFileNameA(NULL, buffer, sizeof(buffer)); // @todo: use GetModuleFileNameW+wchar_t && convert to utf8 instead
    char *a = strrchr(buffer, '/');  if(!a) a = buffer + strlen(buffer);
    char *b = strrchr(buffer, '\\'); if(!b) b = buffer + strlen(buffer);
    char slash = (a < b ? *a : b < a ? *b : '/');
    snprintf(buffer, 1024, "%.*s%c", length - (int)(a < b ? b - a : a - b), buffer, slash);
    if( strendi(buffer, "tools\\tcc\\") ) { // fix tcc -g -run case. @fixme: use TOOLS instead
        strcat(buffer, "..\\..\\");
    }
#else // #elif is(linux)
    char path[21] = {0};
    sprintf(path, "/proc/%d/exe", getpid());
    readlink(path, buffer, sizeof(buffer));
#endif
    return buffer;
}

const char *app_temp() {
    static char buffer[256] = {0};
    if( !buffer[0] ) {
        snprintf(buffer, 256, "%s", ifdef(win32, getenv("TEMP"), P_tmpdir));
        for( int i = 0; buffer[i]; ++i ) if( buffer[i] == '\\' ) buffer[i] = '/';
        if(buffer[strlen(buffer)-1] != '/') strcat(buffer, "/");
    }
    return buffer;
}

/*
    bool exporting_dll = !strcmp(STRINGIZE(API), STRINGIZE(EXPORT));
    bool importing_dll = !strcmp(STRINGIZE(API), STRINGIZE(IMPORT));
    else static_build
*/

#ifndef APP_NAME
#define APP_NAME ifdef(ems, "", (__argv ? __argv[0] : ""))
#endif

const char *app_name() {
    static char buffer[256] = {0};
    if( !buffer[0] ) {
        char s[256];
        strncpy(s, APP_NAME, 256);
        char *a = strrchr(s, '/');
        char *b = strrchr(s, '\\');
        strncpy(buffer, a > b ? a+1 : b > a ? b+1 : s, 256);
        if(strendi(buffer, ".exe")) buffer[strlen(buffer) - 4] = 0;
    }
    return buffer;
}

const char *app_cmdline() {
    static char *cmdline = 0;
    if( !cmdline ) {
        if( argc() <= 1 ) strcatf(&cmdline, "%s", " ");
        for( int i = 1; i < argc(); ++i ) strcatf(&cmdline, " %s", argv(i));
    }
    return cmdline+1;
}

const char *app_cache() {
    static char buffer[256] = {0};
    if( !buffer[0] ) {

        #if is(osx)
        snprintf(buffer, 256, "~/Library/Caches/%s/", app_name()); // getenv("user.home")
        #elif is(win32) // APPDATA for roaming?
        snprintf(buffer, 256, "%s\\%s\\", getenv("LOCALAPPDATA"), app_name()); // getenv("LOCALAPPDATA")
        #else // linux
        snprintf(buffer, 256, "~/.cache/%s/", app_name()); // getenv("user.home")
        #endif

        mkdir(buffer, 0777);

        for( int i = 0; buffer[i]; ++i ) if( buffer[i] == '\\' ) buffer[i] = '/';
    }

    return buffer;
}

const char * app_exec( const char *cmd ) {
    static __thread char output[4096+16] = {0};

    if( !cmd[0] ) return "0               ";
    cmd = file_normalize(cmd);

    int rc = -1;
    char *buf = output + 16; buf[0] = 0; // memset(buf, 0, 4096);
    for( FILE *fp = popen( cmd, "r" ); fp; rc = pclose(fp), fp = 0) {
        while( fgets(buf, 4096 - 1, fp) ) {
        }
    }
    if( rc != 0 ) {
        char *r = strrchr(buf, '\r'); if(r) *r = 0;
        char *n = strrchr(buf, '\n'); if(n) *n = 0;
    }
    return snprintf(output, 16, "%-15d", rc), buf[-1] = ' ', output;
}

int app_spawn( const char *cmd ) {
    if( !cmd[0] ) return -1;
    cmd = file_normalize(cmd);

    return system(cmd);
}

#if is(osx)
#include <execinfo.h> // backtrace, backtrace_symbols
#include <dlfcn.h>    // dladdr, Dl_info
#elif is(gcc) && !is(ems) && !is(mingw) // maybe is(linux) is enough?
#include <execinfo.h>  // backtrace, backtrace_symbols
#elif is(win32) // && !defined __TINYC__
#include <winsock2.h>  // windows.h alternative
#include <dbghelp.h>
#pragma comment(lib, "DbgHelp")
#pragma comment(lib, "Kernel32")
static int backtrace( void **addr, int maxtraces ) {
    static bool init = 0;
    do_once SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES);
    do_once init = SymInitialize(GetCurrentProcess(), NULL, TRUE);
    if(!init) return 0; // error: cannot initialize DbgHelp.lib

    //typedef USHORT (WINAPI *pFN)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG); // _MSC_VER
    typedef USHORT (WINAPI *pFN)(); // TINYC
    static pFN rtlCaptureStackBackTrace = 0;
    if( !rtlCaptureStackBackTrace ) {
        rtlCaptureStackBackTrace = (pFN)GetProcAddress(LoadLibraryA("kernel32.dll"), "RtlCaptureStackBackTrace");
    }
    if( !rtlCaptureStackBackTrace ) {
        return 0;
    }
    return rtlCaptureStackBackTrace(1, maxtraces, (PVOID *)addr, (DWORD *) 0);
}
static char **backtrace_symbols(void *const *list,int size) {
    HANDLE process = GetCurrentProcess();

    struct symbol_t {
        SYMBOL_INFO info;
        TCHAR symbolname[256], terminator;
    } si = { {0} };
    si.info.SizeOfStruct = sizeof(SYMBOL_INFO);
    si.info.MaxNameLen = sizeof(si.symbolname) / sizeof(TCHAR); // number of chars, not bytes

    IMAGEHLP_LINE l64 = { 0 };
    l64.SizeOfStruct = sizeof(IMAGEHLP_LINE);

    static __thread char **symbols = 0; //[32][64] = {0};
    if( !symbols ) {
        symbols = SYS_REALLOC(0, 128 * sizeof(char*));
        for( int i = 0; i < 128; ++i) symbols[i] = SYS_REALLOC(0, 128 * sizeof(char));
    }

    if(size > 128) size = 128;
    for( int i = 0; i < size; ++i ) {

        char *ptr = symbols[i];
        *ptr = '\0';

        if (SymFromAddr(process, (DWORD64)(uintptr_t)list[i], 0, &si.info)) {
            //char undecorated[1024];
            //UnDecorateSymbolName(si.info.Name, undecorated, sizeof(undecorated)-1, UNDNAME_COMPLETE);
            char* undecorated = (char*)si.info.Name;
            ptr += snprintf(ptr, 128, "%s", undecorated);
        } else {
            ptr += snprintf(ptr, 128, "%s", "(?""?)");
        }

        DWORD dw = 0;
        if (SymGetLineFromAddr(process, (DWORD64)(uintptr_t)list[i], &dw, &l64)) {
            ptr += snprintf(ptr, 128 - (ptr - symbols[i]), " (%s:%u)", l64.FileName, (unsigned)l64.LineNumber);
        }
    }

    return symbols;
}
#else
static int backtrace(void **heap, int num) { return 0; }
static char **backtrace_symbols(void *const *sym,int num) { return 0; }
#endif

char *callstack( int traces ) {
    static __thread char *output = 0;
    if(!output ) output = SYS_REALLOC( 0, 128 * (64+2) );
    if( output ) output[0] = '\0';
    char *ptr = output;

    enum { skip = 1 }; /* exclude 1 trace from stack (this function) */
    enum { maxtraces = 128 };

    int inc = 1;
    if( traces < 0 ) traces = -traces, inc = -1;
    if( traces == 0 ) return "";
    if( traces > maxtraces ) traces = maxtraces;

    void* stacks[maxtraces/* + 1*/]; // = { 0 };
    traces = backtrace( stacks, traces );
    char **symbols = backtrace_symbols( stacks, traces ); // @todo: optimization: map(void*,char*) cache; and retrieve only symbols not in cache

    char demangled[1024] = "??";
    int L = 0, B = inc>0 ? skip - 1 : traces, E = inc>0 ? traces : skip - 1;
    for( int i = B; ( i += inc ) != E; ) {
#if is(linux)
        #if ENABLE_LINUX_CALLSTACKS
        // @fixme: following snippet works if compiled with '-g', albeit terribly slow
        // should concat addresses into a multi-address line

        char *binary = symbols[i];
        char *address = strchr( symbols[i], '(' ) + 1; 
        *strrchr( address, ')') = '\0'; *(address - 1) = '\0';

        for( FILE *fp = popen(va("addr2line -e %s %s", binary, address), "r" ); fp ; pclose(fp), fp = 0 ) { //addr2line -e binary -f -C address
            fgets(demangled, sizeof(demangled), fp);
            int len = strlen(demangled); while( len > 0 && demangled[len-1] < 32 ) demangled[--len] = 0;
        }
        symbols[i] = demangled;
        #else
        // make it shorter. ie, `0x00558997ccc87e ./a.out(+0x20187e) [0x00558997ccc87e]`
        strchr(symbols[i], ')')[1] = '\0';
        #endif
#elif is(osx)
        /*struct*/ Dl_info info;
        if( dladdr(stacks[i], &info) && info.dli_sname ) {
            const char *dmgbuf = info.dli_sname[0] != '_' ? NULL :
                 ifdef(cpp, __cxa_demangle(info.dli_sname, NULL, 0, NULL), info.dli_sname);
            strcpy( demangled, dmgbuf ? dmgbuf : info.dli_sname );
            symbols[i] = demangled;
            if( dmgbuf ) free( (void*)dmgbuf );
        }
#endif
        ptr += sprintf(ptr, "%03d: %#016llx %s\n", ++L, (unsigned long long)(uintptr_t)stacks[i], symbols[i]); // format gymnastics because %p is not standard when printing pointers
    }

#if is(linux) || is(osx)
     if(symbols) free(symbols);
#endif

     return output ? output : "";
}

int callstackf( FILE *fp, int traces ) {
    char *buf = callstack(traces);
    fputs(buf, fp);
    return 0;
}

// signals --------------------------------------------------------------------

const char *signal_name(int signal) {
    if(signal == SIGABRT) return "SIGABRT - \"abort\", abnormal termination";
    if(signal == SIGFPE) return "SIGFPE - floating point exception";
    if(signal == SIGILL) return "SIGILL - \"illegal\", invalid instruction";
    if(signal == SIGSEGV) return "SIGSEGV - \"segmentation violation\", invalid memory access";
    if(signal == SIGINT) return "SIGINT - \"interrupt\", interactive attention request sent to the program";
    if(signal == SIGTERM) return "SIGTERM - \"terminate\", termination request sent to the program";
    ifndef(win32, if(signal == SIGBUS) return "SIGBUS");
    ifdef(linux, if(signal == SIGSTKFLT) return "SIGSTKFLT");
    ifndef(win32, if(signal == SIGQUIT) return "SIGQUIT");
    return "??";
}
void signal_handler_ignore(int signal_) {
    signal(signal_, signal_handler_ignore);
}
void signal_handler_quit(int signal) {
    // fprintf(stderr, "Ok: caught signal %s (%d)\n", signal_name(signal), signal);
    exit(0);
}
void signal_handler_abort(int signal) {
    fprintf(stderr, "Error: unexpected signal %s (%d)\n%s\n", signal_name(signal), signal, callstack(16));
    exit(-1);
}
void signal_handler_debug(int signal) {
    breakpoint("Error: unexpected signal");
    fprintf(stderr, "Error: unexpected signal %s (%d)\n%s\n", signal_name(signal), signal, callstack(16));
    exit(-1);
}
void signal_hooks(void) {
    // expected signals
    signal(SIGINT, signal_handler_quit);
    signal(SIGTERM, signal_handler_quit);
    ifndef(win32, signal(SIGQUIT, signal_handler_quit));
    // unexpected signals
    signal(SIGABRT, signal_handler_abort);
    signal(SIGFPE, signal_handler_abort);
    signal(SIGILL, signal_handler_abort);
    signal(SIGSEGV, signal_handler_abort);
    ifndef(win32, signal(SIGBUS, signal_handler_abort));
    ifdef(linux, signal(SIGSTKFLT, signal_handler_abort));
}

#ifdef SIGNAL_DEMO
void crash() {
    char *ptr = 0;
    *ptr = 1;
}
void hang() {
    for(;;);
}
int main(int argc, char **argv) {
    signal_hooks();
    crash(); // hang();
}
#define main main__
#endif

// endian ----------------------------------------------------------------------

#if is(cl)
#include <stdlib.h>
#define swap16 _byteswap_ushort
#define swap32 _byteswap_ulong
#define swap64 _byteswap_uint64
#elif is(gcc)
#define swap16 __builtin_bswap16
#define swap32 __builtin_bswap32
#define swap64 __builtin_bswap64
#else
uint16_t swap16( uint16_t x ) { return (x << 8) | (x >> 8); }
uint32_t swap32( uint32_t x ) { x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); return (x << 16) | (x >> 16); }
uint64_t swap64( uint64_t x ) { x = ((x <<  8) & 0xff00ff00ff00ff00ULL) | ((x >>  8) & 0x00ff00ff00ff00ffULL); x = ((x << 16) & 0xffff0000ffff0000ULL) | ((x >> 16) & 0x0000ffff0000ffffULL); return (x << 32) | (x >> 32); }
#endif

float    swap32f(float n)  { union { float  t; uint32_t i; } conv; conv.t = n; conv.i = swap32(conv.i); return conv.t; }
double   swap64f(double n) { union { double t; uint64_t i; } conv; conv.t = n; conv.i = swap64(conv.i); return conv.t; }

#define is_big()    ((*(uint16_t *)"\0\1") == 1)
#define is_little() ((*(uint16_t *)"\0\1") != 1)

uint16_t  lil16(uint16_t n) { return is_big()    ? swap16(n) : n; }
uint32_t  lil32(uint32_t n) { return is_big()    ? swap32(n) : n; }
uint64_t  lil64(uint64_t n) { return is_big()    ? swap64(n) : n; }
uint16_t  big16(uint16_t n) { return is_little() ? swap16(n) : n; }
uint32_t  big32(uint32_t n) { return is_little() ? swap32(n) : n; }
uint64_t  big64(uint64_t n) { return is_little() ? swap64(n) : n; }

float     lil32f(float n)  { return is_big()    ? swap32f(n) : n; }
double    lil64f(double n) { return is_big()    ? swap64f(n) : n; }
float     big32f(float n)  { return is_little() ? swap32f(n) : n; }
double    big64f(double n) { return is_little() ? swap64f(n) : n; }

uint16_t* lil16p(void *p, int sz)  { if(is_big()   ) { uint16_t *n = (uint16_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap16(n[i]); } return p; }
uint16_t* big16p(void *p, int sz)  { if(is_little()) { uint16_t *n = (uint16_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap16(n[i]); } return p; }
uint32_t* lil32p(void *p, int sz)  { if(is_big()   ) { uint32_t *n = (uint32_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap32(n[i]); } return p; }
uint32_t* big32p(void *p, int sz)  { if(is_little()) { uint32_t *n = (uint32_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap32(n[i]); } return p; }
uint64_t* lil64p(void *p, int sz)  { if(is_big()   ) { uint64_t *n = (uint64_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap64(n[i]); } return p; }
uint64_t* big64p(void *p, int sz)  { if(is_little()) { uint64_t *n = (uint64_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap64(n[i]); } return p; }

float   * lil32pf(void *p, int sz) { if(is_big()   ) { float    *n = (float    *)p; for(int i = 0; i < sz; ++i) n[i] = swap32f(n[i]); } return p; }
float   * big32pf(void *p, int sz) { if(is_little()) { float    *n = (float    *)p; for(int i = 0; i < sz; ++i) n[i] = swap32f(n[i]); } return p; }
double  * lil64pf(void *p, int sz) { if(is_big()   ) { double   *n = (double   *)p; for(int i = 0; i < sz; ++i) n[i] = swap64f(n[i]); } return p; }
double  * big64pf(void *p, int sz) { if(is_little()) { double   *n = (double   *)p; for(int i = 0; i < sz; ++i) n[i] = swap64f(n[i]); } return p; }

// cpu -------------------------------------------------------------------------

#if is(linux)
#include <sched.h>
#endif

int app_cores() {
#if is(win32)
    DWORD_PTR pm, sm;
    if( GetProcessAffinityMask(GetCurrentProcess(), &pm, &sm) ) if( pm ) {
        int count = 0;
        while( pm ) {
            ++count;
            pm &= pm - 1;
        }
        return count;
    }
    { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
#else // unix
    int count = sysconf(_SC_NPROCESSORS_ONLN);
    return count > 0 ? count : 1;
#endif
#if 0
#elif is(linux)
    cpu_set_t prevmask, testmask;
    CPU_ZERO(&prevmask);
    CPU_ZERO(&testmask);
    sched_getaffinity(0, sizeof(prevmask), &prevmask);     //Get current mask
    sched_setaffinity(0, sizeof(testmask), &testmask);     //Set zero mask
    sched_getaffinity(0, sizeof(testmask), &testmask);     //Get mask for all CPUs
    sched_setaffinity(0, sizeof(prevmask), &prevmask);     //Reset current mask
    int num = CPU_COUNT(&testmask);
    return (num > 1 ? num : 1);
#elif is(cpp)
    return (int)std::thread::hardware_concurrency();
#elif defined(_OPENMP)
    // omp
    int cores = 0;
    #pragma omp parallel
    {
        #pragma omp atomic
        ++cores;
    }
    return cores;
#endif
}

// -----------------------------------------------------------------------------
// Battery API. Based on code by Rabia Alhaffar (UNLICENSE)
// - rlyeh, public domain.

#if is(win32)
#include <winsock2.h>

int app_battery() {
    SYSTEM_POWER_STATUS ibstatus;

    if (GetSystemPowerStatus(&ibstatus) == FALSE) {
        return 0;
    }

    int level = (ibstatus.BatteryLifePercent != 255) ? ibstatus.BatteryLifePercent : 0;
    int charging = (ibstatus.BatteryFlag & 8) > 0;
    return charging ? +level : -level;
}

#elif defined __linux__ // is(linux)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int app_battery() {
    static int battery_status_handle;
    static int battery_capacity_handle;

    do_once {
        battery_status_handle = open("/sys/class/power_supply/BAT0/status", O_RDONLY);
        battery_capacity_handle = open("/sys/class/power_supply/BAT0/capacity", O_RDONLY);
    }

    if (battery_status_handle == -1 || battery_capacity_handle == -1) {
        return 0;
    }

    char buffer[512];

    // level
    lseek(battery_capacity_handle, 0, SEEK_SET);
    int readlen = read(battery_capacity_handle, buffer, 511); buffer[readlen < 0 ? 0 : readlen] = '\0';
    int level = atoi(buffer);

    // charging
    lseek(battery_status_handle, 0, SEEK_SET);
    readlen = read(battery_status_handle, buffer, 511); buffer[readlen < 0 ? 0 : readlen] = '\0';
    int charging = strstr(buffer, "Discharging") ? 0 : 1;
    return charging ? +level : -level;
}

#elif is(osx)
#import <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#import <IOKit/ps/IOPowerSources.h>
#import <IOKit/ps/IOPSKeys.h>

int app_battery() {
    static CFDictionaryRef psrc;

    do_once {
        CFTypeRef blob = IOPSCopyPowerSourcesInfo();
        CFArrayRef sources = IOPSCopyPowerSourcesList(blob);
        int sourcesCount = CFArrayGetCount(sources);

        if (sourcesCount > 0) {
            psrc = IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, 0));
        }
    }

    if(psrc == NULL) return 0;

    int cur_cap = 0;
    CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(psrc, CFSTR(kIOPSCurrentCapacityKey)), kCFNumberSInt32Type, &cur_cap);

    int max_cap = 0;
    CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(psrc, CFSTR(kIOPSMaxCapacityKey)), kCFNumberSInt32Type, &max_cap);

    int level = (int)(cur_cap * 100.f / max_cap);
    int charging = CFDictionaryGetValue(psrc, CFSTR(kIOPSIsChargingKey)) == kCFBooleanTrue;
    return charging ? +level : -level;
}

#else

int app_battery() {
    return 0;
}

#endif

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
// argc/v

static void argc_init() {
#if is(tcc) && is(linux)
    do_once {
        char buffer[128], arg0[128] = {0};
        for( FILE *fp = fopen("/proc/self/status", "rb"); fp; fclose(fp), fp = 0) {
            while( fgets(buffer, 128, fp) ) {
                if( strbeg(buffer, "Name:") ) {
                    sscanf(buffer + 5, "%s", arg0 );
                    break;
                }
            }
        }
        extern char **environ;
        __argv = environ - 2; // last argv, as stack is [argc][argv0][argv1][...][NULL][envp]
        while( !strend(*__argv,arg0) ) --__argv;
        __argc = *(int*)(__argv-1);
    }
#endif
}

int argc() {
    do_once argc_init();
    return __argc;
}
char* argv(int arg) {
    do_once argc_init();
    static __thread char empty[1];
    return (unsigned)arg < __argc ? __argv[arg] : (empty[0] = '\0', empty);
}

// ----------------------------------------------------------------------------
// options

int flag(const char *commalist) {
    while( commalist[0] ) {
        const char *begin = commalist;
        while(*commalist != ',' && *commalist != '\0') ++commalist;
        const char *end = commalist;

        char token[128];
        snprintf(token,   128, "%.*s",  (int)(end - begin), begin);

        for( int i = 1; i < argc(); ++i ) {
            char *arg = argv(i);

            if( !strcmpi( arg, token ) ) {  // --arg
                return 1;
            }
        }

        commalist = end + !!end[0];
    }
    return 0;
}

const char *option(const char *commalist, const char *defaults) {
    while( commalist[0] ) {
        const char *begin = commalist;
        while(*commalist != ',' && *commalist != '\0') ++commalist;
        const char *end = commalist;

        char token[128], tokeneq[128];
        snprintf(token,   128, "%.*s",  (int)(end - begin), begin);
        snprintf(tokeneq, 128, "%.*s=", (int)(end - begin), begin);

        for( int i = 1; i < argc(); ++i ) {
            char *arg = argv(i);

            if( strbegi( arg, tokeneq ) ) { // --arg=value
                return argv(i) + strlen(tokeneq);
            }
            if( !strcmpi( arg, token ) ) {  // --arg value
                if( (i+1) < argc() ) {
                    return argv(i+1);
                }
            }
        }

        commalist = end + !!end[0];
    }
    return defaults;
}

int optioni(const char *commalist, int defaults) {
    const char *rc = option(commalist, 0);
    return rc ? atoi(rc) : defaults;
}
float optionf(const char *commalist, float defaults) {
    const char *rc = option(commalist, 0);
    return rc ? atof(rc) : defaults;
}

// ----------------------------------------------------------------------------
// tty

void tty_color(unsigned color) {
    #if is(win32)
    do_once {
        DWORD mode = 0; SetConsoleMode(GetStdHandle(-11), (GetConsoleMode(GetStdHandle(-11), &mode), mode|4));
    }
    #endif
    if( color ) {
        // if( color == RED ) breakpoint("break on RED"); // debug
        unsigned r = (color >> 16) & 255;
        unsigned g = (color >>  8) & 255;
        unsigned b = (color >>  0) & 255;
        // 24-bit console ESC[ … 38;2;<r>;<g>;<b> … m Select RGB foreground color
        // 256-color console ESC[38;5;<fgcode>m
        // 0x00-0x07:  standard colors (as in ESC [ 30..37 m)
        // 0x08-0x0F:  high intensity colors (as in ESC [ 90..97 m)
        // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
        // 0xE8-0xFF:  grayscale from black to white in 24 steps
        r /= 51, g /= 51, b /= 51; // [0..5]
        printf("\033[38;5;%dm", r*36+g*6+b+16); // "\033[0;3%sm", color_code);
    } else {
        printf("%s", "\x1B[39;49m"); // reset
    }
}
void tty_puts(unsigned color, const char *text) {
    tty_color(color); puts(text);
}
void tty_init() {
    tty_color(0);
}
int tty_cols() {
#if is(win32)
    CONSOLE_SCREEN_BUFFER_INFO c;
    if( GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &c) ) {
        int w = c.srWindow.Right-c.srWindow.Left-c.dwCursorPosition.X;
        return w > 2 ? w - 1 : w; // w-1 to allow window resizing to a larger dimension (already printed text would break otherwise)
    }
#endif
#ifdef TIOCGWINSZ
    struct winsize ws;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    return ws.ws_col - 1;
#endif
#ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    return ts.ts_cols - 1;
#endif
    return 80;
}
void tty_detach() {
    ifdef(win32, FreeConsole());
}
void tty_attach() {
#if is(win32)
    // in order to have a Windows gui application with console:
    // - use WinMain() then AllocConsole(), but that may require supporintg different entry points for different platforms.
    // - /link /SUBSYSTEM:CONSOLE and then call FreeConsole() if no console is needed, but feels naive to flash the terminal for a second.
    // - /link /SUBSYSTEM:WINDOWS /entry:mainCRTStartup, then AllocConsole() as follows. Quoting @pmttavara: 
    //   "following calls are the closest i'm aware you can get to /SUBSYSTEM:CONSOLE in a gui program 
    //   while cleanly handling existing consoles (cmd.exe), pipes (ninja) and no console (VS/RemedyBG; double-clicking the game)"
    do_once {
        if( !AttachConsole(ATTACH_PARENT_PROCESS) && GetLastError() != ERROR_ACCESS_DENIED ) ASSERT( AllocConsole() );
        printf("\n"); // print >= 1 byte to distinguish empty stdout from a redirected stdout (fgetpos() position <= 0)
        fpos_t pos = 0;
        if( fgetpos(stdout, &pos) != 0 || pos <= 0 ) {
            ASSERT(freopen("CONIN$" , "r", stdin ));
            ASSERT(freopen("CONOUT$", "w", stderr));
            ASSERT(freopen("CONOUT$", "w", stdout));
        }
    }
#endif
}

// -----------------------------------------------------------------------------
// debugger

#include <stdio.h>
void hexdumpf( FILE *fp, const void *ptr, unsigned len, int width ) {
    unsigned char *data = (unsigned char*)ptr;
    for( unsigned jt = 0; jt <= len; jt += width ) {
        fprintf( fp, "; %05d%s", jt, jt == len ? "\n" : " " );
        for( unsigned it = jt, next = it + width; it < len && it < next; ++it ) {
            fprintf( fp, "%02x %s", (unsigned char)data[it], &" \n\0...\n"[ (1+it) < len ? 2 * !!((1+it) % width) : 3 ] );
        }
        fprintf( fp, "; %05d%s", jt, jt == len ? "\n" : " " );
        for( unsigned it = jt, next = it + width; it < len && it < next; ++it ) {
            fprintf( fp, " %c %s", (signed char)data[it] >= 32 ? (signed char)data[it] : (signed char)'.', &" \n\0..."[ (1+it) < len ? 2 * !!((1+it) % width) : 3 ] );
        }
    }
    fprintf(fp, " %d bytes\n", len);
}
void hexdump( const void *ptr, unsigned len ) {
    hexdumpf( stdout, ptr, len, 16 );
}

#if 0 // is(cl) only
static void debugbreak(void) {
    do { \
        __try { DebugBreak(); } \
        __except (GetExceptionCode() == EXCEPTION_BREAKPOINT ? \
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {} \
    } while(0);
}
#endif

#if is(win32)
static void debugbreak(void) { if(IsDebuggerPresent()) DebugBreak(); }
#else // is(unix)
static int is_debugger_present = -1;
static void _sigtrap_handler(int signum) {
    is_debugger_present = 0;
    signal(SIGTRAP, SIG_DFL);
}
static void debugbreak(void) { // break if debugger present
    // __builtin_trap(); //
    //raise(SIGABRT); // SIGTRAP);
    //__asm__ volatile("int $0x03");
    if( is_debugger_present < 0 ) {
        is_debugger_present = 1;
        signal(SIGTRAP, _sigtrap_handler);
        raise(SIGTRAP);
    }
}
#endif

void alert(const char *message) { // @todo: move to app_, besides die()
#if is(win32)
    MessageBoxA(0, message, 0,0);
#elif is(ems)
    emscripten_run_script(va("alert('%s')", message));
#elif is(linux)
    for(FILE *fp = fopen("/tmp/fwk.warning","wb");fp;fp=0)
    fputs(message,fp), fclose(fp), system("xmessage -center -file /tmp/fwk.warning");
#elif is(osx)
    system(va("osascript -e 'display alert \"Alert\" message \"%s\"'", message));
#endif
}

void breakpoint(const char *reason) {
    window_visible(false);
    if( reason ) {
        const char *fulltext = reason[0] == '!' ? va("%s\n%s", reason+1, callstack(+48)) : reason;
        PRINTF("%s", fulltext);

        (alert)(fulltext);
    }
    debugbreak();
    window_visible(true);
}

bool has_debugger() {
#if is(win32)
    return IsDebuggerPresent(); // SetLastError(123); OutputDebugStringA("\1"); enabled = GetLastError() != 123;
#else
    return false;
#endif
}

void die(const char *message) {
   fprintf(stderr, "%s\n", message);
   fflush(stderr);
   alert(message);
   exit(-1);
}

// ----------------------------------------------------------------------------
// logger

unsigned determine_color_from_text(const char *text) {
    /**/ if( strstri(text, "fail") || strstri(text, "error") ) return RED;
    else if( strstri(text, "warn") || strstri(text, "not found") ) return YELLOW;
    return 0;
}

//static int __thread _thread_id;
//#define PRINTF(...)      (printf("%03d %07.3fs|%-16s|", (((unsigned)(uintptr_t)&_thread_id)>>8) % 1000, time_ss(), __FUNCTION__), printf(__VA_ARGS__), printf("%s", 1[#__VA_ARGS__] == '!' ? callstack(+48) : "")) // verbose logger

int (PRINTF)(const char *text, const char *stack, const char *file, int line, const char *function) {
    double secs = time_ss();
    uint32_t color = /*errno ? RED :*/ determine_color_from_text(text); // errno = 0;
    #if is(cl)
    char *slash = strrchr(file, '\\'); if(slash) file = slash + 1;
    #endif
    char *location = va("|%s|%s:%d", /*errno?strerror(errno):*/function, file, line);
    int cols = tty_cols() + 1 - (int)strlen(location);

static thread_mutex_t lock, *init = 0; if(!init) thread_mutex_init(init = &lock);
thread_mutex_lock( &lock );

    tty_color(color);
    printf("\r%*.s%s", cols, "", location);
    printf("\r%07.3fs|%s%s", secs, text, stack);
    tty_color(0);

thread_mutex_unlock( &lock );
    return 1;
}

// ----------------------------------------------------------------------------
// panic

static void *panic_oom_reserve; // for out-of-memory recovery
int (PANIC)(const char *error, const char *file, int line) {
    panic_oom_reserve = SYS_REALLOC(panic_oom_reserve, 0);

    tty_color(RED);

    error += error[0] == '!';
    fprintf(stderr, "Error: %s (%s:%d) (errno:%s)\n", error, file, line, strerror(errno));
    fprintf(stderr, "%s", callstack(+16)); // no \n
    fflush(0); // fflush(stderr);

    tty_color(0);

    breakpoint(error);
    exit(-line);
    return 1;
}

// ----------------------------------------------------------------------------
// threads

struct thread_wrapper {
    int (*func)(void *user_data);
    void *user_data;
};

static
int thread_proc( void* user_data ) {
    struct thread_wrapper *w = (struct thread_wrapper*)user_data;
    int return_code = w->func( w->user_data );
    thread_exit( return_code );
    FREE(w);
    return 0;
}

void* thread( int (*thread_func)(void* user_data), void* user_data ) {
    struct thread_wrapper *w = MALLOC(sizeof(struct thread_wrapper));
    w->func = thread_func;
    w->user_data = user_data;

    int thread_stack_size = 0;
    const char *thread_name = "";
    thread_ptr_t thd = thread_init( thread_proc, w, thread_name, thread_stack_size );
    return thd;
}
void thread_destroy( void *thd ) {
    int rc = thread_join(thd);
    thread_term(thd);
}

