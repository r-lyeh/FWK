// -----------------------------------------------------------------------------
// system framework utils
// - rlyeh, public domain.
//
// Note: Windows users add `/Zi` compilation flags, else add `-g` and/or `-ldl` flags
// Note: If you are linking your binary using GNU ld you need to add --export-dynamic

API void*       thread( int (*thread_func)(void* user_data), void* user_data );
API void        thread_destroy( void *thd );

API int         argc();
API char*       argv(int);
API void        argvadd(const char *arg);

API int         flag(const char *commalist); // --arg // app_flag?
API const char* option(const char *commalist, const char *defaults); // --arg=string or --arg string
API int         optioni(const char *commalist, int defaults); // --arg=integer or --arg integer  // argvi() ?
API float       optionf(const char *commalist, float defaults); // --arg=float or --arg float    // flagf() ?

API void        tty_attach();
API void        tty_detach();
API void        tty_color(unsigned color);
API void        tty_reset();

API const char* app_exec(const char *command); // returns ("%15d %s", retcode, output_last_line)
API int         app_spawn(const char *command);
API int         app_cores();
API int         app_battery(); /// returns battery level [1..100]. also positive if charging (+), negative if discharging (-), and 0 if no battery is present.

API const char* app_name();
API const char* app_path();
API const char* app_cache();
API const char* app_temp();
API const char* app_cmdline();

API void        app_beep();
API void        app_hang();
API void        app_crash();
API void        app_singleton(const char *guid);
API bool        app_open(const char *folder_file_or_url);

API const char* app_loadfile();
API const char* app_savefile();


API char*       callstack( int traces ); // write callstack into a temporary string. <0 traces to invert order. do not free().
API int         callstackf( FILE *fp, int traces ); // write callstack to file. <0 traces to invert order.

API void        die(const char *message);
API void        alert(const char *message);
API void        hexdump( const void *ptr, unsigned len );
API void        hexdumpf( FILE *fp, const void *ptr, unsigned len, int width );
API void        breakpoint();
API bool        has_debugger();

API void        trap_install(void);
API const char *trap_name(int signal);      // helper util
API void        trap_on_ignore(int signal); // helper util
API void        trap_on_quit(int signal);   // helper util
API void        trap_on_abort(int signal);  // helper util
API void        trap_on_debug(int signal);  // helper util

#define PANIC(...)   PANIC(va(""__VA_ARGS__), __FILE__, __LINE__) // die() ?
API int (PANIC)(const char *error, const char *file, int line);

#define PRINTF(...)  PRINTF(va(""__VA_ARGS__), 1[""#__VA_ARGS__] == '!' ? callstack(+48) : "", __FILE__, __LINE__, __FUNCTION__)
API int (PRINTF)(const char *text, const char *stack, const char *file, int line, const char *function);

#define test(expr) test(__FILE__,__LINE__,#expr,!!(expr))
API int (test)(const char *file, int line, const char *expr, bool result);

#if ENABLE_AUTOTESTS
#define AUTOTEST AUTORUN
#else
#define AUTOTEST static void concat(concat(concat(disabled_test_, __LINE__), _), __COUNTER__)()
#endif

// AUTOTEST { test(1<2); }

#if ENABLE_RETAIL
#undef  PRINTF
#define PRINTF(...) 0
#undef  test
#define test(expr)  0
#endif
