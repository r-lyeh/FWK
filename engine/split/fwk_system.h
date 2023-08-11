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

API int         flag(const char *commalist); // --arg // app_flag?
API const char* option(const char *commalist, const char *defaults); // --arg=value or --arg value
API int         optioni(const char *commalist, int defaults); // argvi() ?
API float       optionf(const char *commalist, float defaults); // app_option?

API void        tty_color(unsigned color);
API void        tty_reset();
API void        tty_attach();
API void        tty_detach();

API const char* app_exec(const char *command); // returns ("%15d %s", retcode, output_last_line)
API int         app_cores();
API int         app_battery(); /// return battery level [1..100]. also positive if charging (+), negative if discharging (-), and 0 if no battery is present.

API const char* app_name();
API const char* app_path();
API const char* app_cache();
API const char* app_temp();
API const char* app_cmdline();

API uint64_t    date();        // YYYYMMDDhhmmss
API uint64_t    date_epoch();  // linux epoch
API char*       date_string(); // "YYYY-MM-DD hh:mm:ss"
API double      time_hh();
API double      time_mm();
API double      time_ss();
API uint64_t    time_ms();
API uint64_t    time_us();
API uint64_t    time_ns();
API void        sleep_ss(double ss);
API void        sleep_ms(double ms);
API void        sleep_us(double us);
API void        sleep_ns(double us);

API unsigned    timer(unsigned ms, unsigned (*callback)(unsigned ms, void *arg), void *arg);
API void        timer_destroy(unsigned timer_handle);

API char*       callstack( int traces ); // write callstack into a temporary string. <0 traces to invert order. do not free().
API int         callstackf( FILE *fp, int traces ); // write callstack to file. <0 traces to invert order.

API void        die(const char *message);
API void        alert(const char *message);
API void        hexdump( const void *ptr, unsigned len );
API void        hexdumpf( FILE *fp, const void *ptr, unsigned len, int width );
API void        breakpoint(const char *optional_reason);
API bool        has_debugger();

API void        signal_hooks(void);
API void        signal_handler_ignore(int signal);
API void        signal_handler_quit(int signal);
API void        signal_handler_abort(int signal);
API void        signal_handler_debug(int signal);
API const char *signal_name(int signal); // helper util

API uint16_t    lil16(uint16_t n); // swap16 as lil
API uint32_t    lil32(uint32_t n); // swap32 as lil
API float       lil32f(float n);   // swap32 as lil
API uint64_t    lil64(uint64_t n); // swap64 as lil
API double      lil64f(double n);  // swap64 as lil
API uint16_t    big16(uint16_t n); // swap16 as big
API uint32_t    big32(uint32_t n); // swap32 as big
API float       big32f(float n);   // swap32 as big
API uint64_t    big64(uint64_t n); // swap64 as big
API double      big64f(double n);  // swap64 as big

API uint16_t*   lil16p(void *n, int sz);
API uint32_t*   lil32p(void *n, int sz);
API float*      lil32pf(void *n, int sz);
API uint64_t*   lil64p(void *n, int sz);
API double*     lil64pf(void *n, int sz);
API uint16_t*   big16p(void *n, int sz);
API uint32_t*   big32p(void *n, int sz);
API float*      big32pf(void *n, int sz);
API uint64_t*   big64p(void *n, int sz);
API double*     big64pf(void *n, int sz);

#define PANIC(...)   PANIC(va(__VA_ARGS__), __FILE__, __LINE__) // die() ?
API int (PANIC)(const char *error, const char *file, int line);

#define PRINTF(...)  PRINTF(va(__VA_ARGS__), 1[#__VA_ARGS__] == '!' ? callstack(+48) : "", __FILE__, __LINE__, __FUNCTION__)
API int (PRINTF)(const char *text, const char *stack, const char *file, int line, const char *function);
