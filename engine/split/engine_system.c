#if (is(tcc) && is(linux)) || (is(gcc) && !is(mingw)) // || is(clang)
int __argc; char **__argv;
#if !is(ems)
__attribute__((constructor)) void init_argcv(int argc, char **argv) { __argc = argc; __argv = argv; }
#endif
#endif

void argvadd(const char *arg) {
    char **argv = MALLOC( sizeof(char*) * (__argc+1) );
    for( int i = 0; i < __argc; ++i ) {
        argv[i] = __argv[i];
    }
    argv[__argc] = STRDUP(arg);
    __argv = argv;
    ++__argc;
}

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
    char path[32] = {0};
    sprintf(path, "/proc/%d/exe", getpid());
    readlink(path, buffer, sizeof(buffer));
    if(strrchr(buffer,'/')) 1[strrchr(buffer,'/')] = '\0';
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
    char *buf = output + 16; buf[0] = 0; // memset(buf, 0, 4096);

    if( !cmd[0] ) return "0               ";
    cmd = file_normalize(cmd);

    int rc = -1;

    // pick the fastest code path per platform
#if is(osx)
    rc = system(cmd);
#elif is(win32)
    STARTUPINFOA si = {0}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {0};

    snprintf(output+16, 4096, "cmd /c \"%s\"", cmd);

    int prio = //strstr(cmd, "ffmpeg") || strstr(cmd, "furnace") || strstr(cmd, "ass2iqe") ?
    REALTIME_PRIORITY_CLASS; //: 0;

//prio |= DETACHED_PROCESS;
//si.dwFlags = STARTF_USESTDHANDLES;

    if( CreateProcessA(
        NULL, output+16, // cmdline
        NULL,
        NULL,
        FALSE, // FALSE: dont inherit handles
        prio /*CREATE_DEFAULT_ERROR_MODE|CREATE_NO_WINDOW*/, // 0|HIGH_PRIORITY_CLASS
        NULL, // "", // NULL would inherit env
        NULL, // current dir
        &si, &pi) )
    {
        // Wait for process
        DWORD dwExitCode2 = WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD dwExitCode; GetExitCodeProcess(pi.hProcess, &dwExitCode);
        rc = dwExitCode;
    }
    else
    {
        // CreateProcess() failed
        rc = GetLastError();
    }
#else
    rc = system(cmd);
#endif

    return snprintf(output, 16, "%-15d", rc), buf[-1] = ' ', output;
}

int app_spawn( const char *cmd ) {
    if( !cmd[0] ) return false;
    cmd = file_normalize(cmd);

#if _WIN32
    bool ok = WinExec(va("cmd /c \"%s\"", cmd), SW_HIDE) > 31;
#else
    bool ok = system(va("%s &", cmd)) == 0;
#endif

    return ok;
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
        symbols = SYS_MEM_REALLOC(0, 128 * sizeof(char*));
        for( int i = 0; i < 128; ++i) symbols[i] = SYS_MEM_REALLOC(0, 128 * sizeof(char));
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
#if is(tcc) && is(linux)
    return "";
#endif
#if is(ems) // there is a stack overflow failure somewhere in the impl below
    static char empty[1]; return empty[0] = '\0', empty;
#endif

    enum { skip = 1 }; /* exclude 1 trace from stack (this function) */
    enum { maxtraces = 96 };

    static __thread char *output = 0;
    if(!output ) output = SYS_MEM_REALLOC( 0, maxtraces * (128+2) );
    if( output ) output[0] = '\0';
    char *ptr = output;

    int inc = 1;
    if( traces < 0 ) traces = -traces, inc = -1;
    if( traces == 0 ) return "";
    if( traces > maxtraces ) traces = maxtraces;

    void* stacks[maxtraces + 1]; stacks[maxtraces] = NULL; // = { 0 };
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
            ifdef(cpp, dmgbuf && free( (void*)dmgbuf ) );
        }
#endif
        if( symbols[i] )
        ptr += sprintf(ptr, "%03d: %p %s\n", ++L, (void*)(uintptr_t)stacks[i], symbols[i]); // format gymnastics because %p is not standard when printing pointers
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

// trap signals ---------------------------------------------------------------

const char *trap_name(int signal) {
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
void trap_on_ignore(int sgn) {
    signal(sgn, trap_on_ignore);
}
void trap_on_quit(int sgn) {
    signal(sgn, trap_on_quit);
    exit(0);
}
void trap_on_abort(int sgn) {
    char *cs = va("Error: unexpected signal %s (%d)\n%s", trap_name(sgn), sgn, callstack(+16));
    fprintf(stderr, "%s\n", cs), alert(cs), breakpoint();
    signal(sgn, trap_on_abort);
    exit(-sgn);
}
void trap_on_debug(int sgn) { // @todo: rename to trap_on_choice() and ask the developer what to do next? abort, continue, debug
    char *cs = va("Error: unexpected signal %s (%d)\n%s", trap_name(sgn), sgn, callstack(+16));
    fprintf(stderr, "%s\n", cs), alert(cs), breakpoint();
    signal(sgn, trap_on_debug);
}
#if is(win32)
LONG WINAPI trap_on_SEH(PEXCEPTION_POINTERS pExceptionPtrs) {
    char *cs = va("Error: unexpected SEH exception\n%s", callstack(+16));
    fprintf(stderr, "%s\n", cs), alert(cs), breakpoint();
    return EXCEPTION_EXECUTE_HANDLER; // Execute default exception handler next
}
#endif
void trap_install(void) {
    // expected signals
    signal(SIGINT, trap_on_quit);
    signal(SIGTERM, trap_on_quit);
    ifndef(win32, signal(SIGQUIT, trap_on_quit));
    // unexpected signals
    signal(SIGABRT, trap_on_abort);
    signal(SIGFPE, trap_on_abort);
    signal(SIGILL, trap_on_abort);
    signal(SIGSEGV, trap_on_abort);
    ifndef(win32, signal(SIGBUS, trap_on_abort));
    ifdef(linux, signal(SIGSTKFLT, trap_on_abort));
    // others
    ifdef(win32,SetUnhandledExceptionFilter(trap_on_SEH));
}

#ifdef TRAP_DEMO
AUTORUN {
    trap_install();
    app_crash(); // app_hang();
}
#endif

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
        // if( color == RED ) alert("break on error message (RED)"), breakpoint(); // debug
        unsigned r = (color >>  0) & 255;
        unsigned g = (color >>  8) & 255;
        unsigned b = (color >> 16) & 255;
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
        if( !AttachConsole(ATTACH_PARENT_PROCESS) && GetLastError() != ERROR_ACCESS_DENIED ) { bool ok = !!AllocConsole(); ASSERT( ok ); }
        printf("\n"); // print >= 1 byte to distinguish empty stdout from a redirected stdout (fgetpos() position <= 0)
        fpos_t pos = 0;
        if( fgetpos(stdout, &pos) != 0 || pos <= 0 ) {
            bool ok1 = !!freopen("CONIN$" , "r", stdin ); ASSERT( ok1 );
            bool ok2 = !!freopen("CONOUT$", "w", stderr); ASSERT( ok2 );
            bool ok3 = !!freopen("CONOUT$", "w", stdout); ASSERT( ok3 );
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

void alert_caption(const char *caption, const char *message) { // @todo: move to app_, besides die()
    window_visible(false);
    message = message[0] == '!' ? (const char*)va("%s\n%s", message+1, callstack(+48)) : message;

#if is(win32)
    MessageBoxA(0, message, caption,0);
#elif is(ems)
    emscripten_run_script(va("alert('%s')", message));
#elif is(linux)
    for(FILE *fp = fopen("/tmp/.warning","wb");fp;fp=0)
    fputs(message,fp), fclose(fp), system("xmessage -center -file /tmp/.warning");
#elif is(osx)
    system(va("osascript -e 'display alert \"Alert\" message \"%s\"'", message));
#endif

    window_visible(true);
}

void alert(const char *message) { // @todo: move to app_, besides die()
    alert_caption(NULL, message);
}

void breakpoint() {
    debugbreak();
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

//static int __thread _thread_id;
//#define PRINTF(...)      (printf("%03d %07.3fs|%-16s|", (((unsigned)(uintptr_t)&_thread_id)>>8) % 1000, time_ss(), __FUNCTION__), printf(__VA_ARGS__), printf("%s", 1[#__VA_ARGS__] == '!' ? callstack(+48) : "")) // verbose logger

int (PRINTF)(const char *text, const char *stack, const char *file, int line, const char *function) {
    double secs = time_ss();
    uint32_t color = 0;
    /**/ if( strstri(text, "fail") || strstri(text, "error") ) color = RED;
    else if( strstri(text, "warn") || strstri(text, "not found") ) color = YELLOW;
    #if is(cl)
    char *slash = strrchr(file, '\\'); if(slash) file = slash + 1;
    #endif
    char *location = va("|%s|%s:%d", /*errno?strerror(errno):*/function, file, line);
    int cols = tty_cols() + 1 - (int)strlen(location);

    flockfile(stdout);

    tty_color(color);
    printf("\r%*.s%s", cols, "", location);
    printf("\r%07.3fs|%s%s", secs, text, stack);
    tty_color(0);

    funlockfile(stdout);

    return 1;
}

// ----------------------------------------------------------------------------
// panic

static void *panic_oom_reserve; // for out-of-memory recovery
int (PANIC)(const char *error, const char *file, int line) {
    panic_oom_reserve = SYS_MEM_REALLOC(panic_oom_reserve, 0);

    tty_color(RED);

    error += error[0] == '!';
    fprintf(stderr, "Error: %s (%s:%d) (errno:%s)\n", error, file, line, strerror(errno));
    fprintf(stderr, "%s", callstack(+16)); // no \n
    fprintf(stderr, "\n(This error has been copied to your clipboard)");
    fflush(0); // fflush(stderr);

    tty_color(0);

    glfwSetClipboardString(window_handle(), error);

    alert_caption("PANIC! (This error has been copied to your clipboard)", error);
    breakpoint();

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

void app_hang() {
    for(;;);
}
void app_crash() {
    volatile int *p = 0;
    *p = 42;
}
void app_beep() {
    ifdef(win32, app_spawn("rundll32 user32.dll,MessageBeep"); return; );
    ifdef(linux, app_spawn("paplay /usr/share/sounds/freedesktop/stereo/message.oga"); return; );
    ifdef(osx,   app_spawn("tput bel"); return; );

    //fallback:
    fputc('\x7', stdout);

    // win32:
    // rundll32 user32.dll,MessageBeep ; ok
    // rundll32 cmdext.dll,MessageBeepStub ; ok

    // osx:
    // tput bel
    // say "beep"
    // osascript -e 'beep'
    // osascript -e "beep 1"
    // afplay /System/Library/Sounds/Ping.aiff
    // /usr/bin/printf "\a"

    // linux:
    // paplay /usr/share/sounds/freedesktop/stereo/message.oga ; ok
    // paplay /usr/share/sounds/freedesktop/stereo/complete.oga ; ok
    // paplay /usr/share/sounds/freedesktop/stereo/bell.oga ; ok
    // beep ; apt-get
    // echo -e '\007' ; mute
    // echo -e "\007" >/dev/tty10 ; sudo
    // tput bel ; mute
}

void app_singleton(const char *guid) {
    #ifdef _WIN32
    do_once {
        char buffer[128];
        snprintf(buffer, 128, "Global\\{%s}", guid);
        static HANDLE app_mutex = 0;
        app_mutex = CreateMutexA(NULL, FALSE, buffer);
        if( ERROR_ALREADY_EXISTS == GetLastError() ) {
            exit(-1);
        }
    }
    #endif
}

#ifdef APP_SINGLETON_GUID
AUTORUN { app_singleton(APP_SINGLETON_GUID); }
#endif

static
bool app_open_folder(const char *file) {
    char buf[1024];
#ifdef _WIN32
    snprintf(buf, sizeof(buf), "start \"\" \"%s\"", file);
#elif __APPLE__
    snprintf(buf, sizeof(buf), "%s \"%s\"", file_directory(file) ? "open" : "open --reveal", file);
#else
    snprintf(buf, sizeof(buf), "xdg-open \"%s\"", file);
#endif
    return app_spawn(buf);
}

static
bool app_open_file(const char *file) {
    char buf[1024];
#ifdef _WIN32
    snprintf(buf, sizeof(buf), "start \"\" \"%s\"", file);
#elif __APPLE__
    snprintf(buf, sizeof(buf), "open \"%s\"", file);
#else
    snprintf(buf, sizeof(buf), "xdg-open \"%s\"", file);
#endif
    return app_spawn(buf);
}

static
bool app_open_url(const char *url) {
    return app_open_file(url);
}

bool app_open(const char *link) {
    if( file_directory(link) ) return app_open_folder(link);
    if( file_exist(link) ) return app_open_file(link);
    return app_open_url(link);
}

const char* app_loadfile() {
    const char *windowTitle = NULL;
    const char *defaultPathFile = NULL;
    const char *filterHints = NULL; // "image files"
    const char *filters[] = { "*.*" };
    int allowMultipleSelections = 0;

    tinyfd_assumeGraphicDisplay = 1;
    return tinyfd_openFileDialog( windowTitle, defaultPathFile, countof(filters), filters, filterHints, allowMultipleSelections );
}
const char* app_savefile() {
    const char *windowTitle = NULL;
    const char *defaultPathFile = NULL;
    const char *filterHints = NULL; // "image files"
    const char *filters[] = { "*.*" };

    tinyfd_assumeGraphicDisplay = 1;
    return tinyfd_saveFileDialog( windowTitle, defaultPathFile, countof(filters), filters, filterHints );
}

// ----------------------------------------------------------------------------
// tests

static __thread int test_oks, test_errors, test_once;
static void test_exit(void) { fprintf(stderr, "%d/%d tests passed\n", test_oks, test_oks+test_errors); }
int (test)(const char *file, int line, const char *expr, bool result) {
    static int breakon = -1; if(breakon<0) breakon = optioni("--test-break", 0);
    if( breakon == (test_oks+test_errors+1) ) alert("user requested to break on this test"), breakpoint();
    test_once = test_once || !(atexit)(test_exit);
    test_oks += result, test_errors += !result;
    return (result || (tty_color(RED), fprintf(stderr, "(Test `%s` failed %s:%d)\n", expr, file, line), tty_color(0), 0) );
}
