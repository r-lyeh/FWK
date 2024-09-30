// dll ------------------------------------------------------------------------

/* deprecated
#if is(win32)
#   include <winsock2.h>
#   define dlopen(name,flags)   (void*)( (name) ? LoadLibraryA(name) : GetModuleHandleA(NULL))
#   define dlsym(handle,symbol) GetProcAddress((HMODULE)(handle), symbol )
#   define dlclose(handle)      0
#else
#   include <dlfcn.h>
#   define dlopen(name,flags)   (void*)( (name) ? dlopen(name, flags) : NULL )
#   define dlsym(handle,symbol) dlsym( (handle) ? (handle) : ifdef(osx,RTLD_SELF,NULL), symbol )
#endif
*/

void* dll(const char *fname, const char *symbol) {
    if( fname && !file_exist(fname) ) {
        char *buf, *path = file_path(fname), *base = file_base(fname);
        if( file_exist(buf = va("%s%s.dll", path, base)) ||
            file_exist(buf = va("%s%s.so", path, base)) ||
            file_exist(buf = va("%slib%s.so", path, base)) ||
            file_exist(buf = va("%s%s.dylib", path, base)) ) {
            fname = (const char *)buf;
        } else {
            return NULL;
        }
    }
#if is(win32)
    return (void*)GetProcAddress(fname ? LoadLibraryA(fname) : GetModuleHandleA(NULL), symbol);
#else
    return dlsym(fname ? dlopen(fname, RTLD_NOW|RTLD_LOCAL) : ifdef(osx, RTLD_SELF, NULL), symbol);
#endif
}

#if 0 // demo: cl demo.c /LD && REM dll
EXPORT int add2(int a, int b) { return a + b; }
int main() { int (*adder)() = dll("demo.dll", "add2"); printf("%d\n", adder(2,3)); }
#endif

// script ---------------------------------------------------------------------

typedef lua_State lua;

// the Lua interpreter(s)
static array(lua*) Ls;

// the **current** Lua interpreter
static lua *L;

#if is(linux)
void luaopen_libengine(lua_State *L) {}
#endif

static void* script__realloc(void *userdef, void *ptr, size_t osize, size_t nsize) {
    (void)userdef;
    return ptr = REALLOC( ptr, /* (osize+1) * */ nsize );
}
static int script__traceback(lua_State *L) {
    if (!lua_isstring(L, 1)) { // try metamethod if non-string error object
        if (lua_isnoneornil(L, 1) ||
            !luaL_callmeta(L, 1, "__tostring") ||
            !lua_isstring(L, -1))
            return 1;  // return non-string error object
        lua_remove(L, 1);  // replace object with result of __tostring metamethod
    }
    luaL_traceback(L, L, lua_tostring(L, 1), 1);
    return 1;
}
static void script__error(lua_State *L, int status) {
    if (status != 0) {
        const char *errormsg = lua_tostring(L, -1);
        PRINTF( "!-- %s\n", errormsg);
        lua_pop(L, 1); // remove error message
    }
}
static int script__call(lua_State *L, int narg, int clear) {
#if ENABLE_FASTCALL_LUA
    lua_call(L, 0, 0);
    return 0;
#else
    int base = lua_gettop(L) - narg;  // function index
    lua_pushcfunction(L, script__traceback);  // push traceback function
    lua_insert(L, base);  // put it under chunk and args
    int status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
    script__error(L, status);
    lua_remove(L, base);  // remove traceback function
    if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0); // force gc in case of errors
    return status;
#endif
}

void script_bind_function(const char *c_name, void *c_function) {
    lua_pushcfunction( L, c_function );
    lua_setglobal( L, c_name );
}

void script_call(const char *lua_function) {
    lua_getglobal( L, lua_function );
    script__call( L, 0, 1 );
}

void script_bind_class(const char *classname, int num, const char **methods, void **functions) {
    lua_newtable( L );

    for( int i = 0; i < num; ++i) {
        lua_pushcfunction( L, functions[i] );
        lua_setfield( L, 1, methods[i] );
    }

    lua_setglobal( L, classname );
}

void script_run(const char *script) {
    int ret = luaL_dostring(L, script);
    if( ret != LUA_OK ) {
      PRINTF("!Script failed to run: %s\n", lua_tostring(L, -1));
      lua_pop(L, 1); // pop error message
    }
}

void script_runfile(const char *pathfile) {
    PRINTF( "Loading script '%s'\n", pathfile );
    int loadResult = luaL_loadfile( L, pathfile );

    /**/ if( loadResult == LUA_OK ) {
        script__call( L, 0, 1 );
    }
    else if( loadResult == LUA_ERRSYNTAX ) {
        PRINTF("!Script failed to load (LUA_ERRSYNTAX, '%s'): %s\n", lua_tostring( L, 1 ), pathfile );
//      lua_pop(L, 1); // pop error message
    }
    else if( loadResult == LUA_ERRMEM ) {
        PRINTF("!Script failed to load (LUA_ERRMEM): %s\n", pathfile);
    }
    else {
        PRINTF("!Script failed to load: %s\n", pathfile );
    }
}

// syntax sugars
/* usage:
int window_create_lua(lua *L) {
    window_create(arg_float(1), arg_int(2));
    return_void(0);
}
int window_swap_lua(lua *L) {
    int r = window_swap();
    return_int(r);
}
*/

#define arg_int(nth) lua_tointeger(L, nth)
#define arg_bool(nth) lua_toboolean(L, nth)
#define arg__Bool(nth) lua_toboolean(L, nth)
#define arg_float(nth) (float)lua_tonumber(L, nth)
#define arg_double(nth) lua_tonumber(L, nth)
#define arg_string(nth) lua_tolstring(L, nth, 0)
#define return_void(x) return ((x), 0)
#define return_bool(x) return (lua_pushboolean(L, x), 1)
#define return__Bool(x) return (lua_pushboolean(L, x), 1)
#define return_int(x) return (lua_pushinteger(L, x), 1)
#define return_float(x) return (lua_pushnumber(L, x), 1)
#define return_double(x) return (lua_pushnumber(L, x), 1)
#define return_string(x) return (lua_pushstring(L, x), 1)

#define WRAP_ALL(...)                EXPAND(WRAP_ALL, __VA_ARGS__)
#define WRAP_ALL2(rc, func)          int func##_lua(lua*L) { return_##rc(func()); }
#define WRAP_ALL3(rc, func, a1)      int func##_lua(lua*L) { return_##rc(func(arg_##a1(1))); }
#define WRAP_ALL4(rc, func, a1,a2)   int func##_lua(lua*L) { return_##rc(func(arg_##a1(1),arg_##a2(2))); }

#define BIND_ALL(...)                EXPAND(BIND_ALL, __VA_ARGS__);
#define BIND_ALL2(rc,func)           script_bind_function(#func, func##_lua)
#define BIND_ALL3(rc,func,a1)        script_bind_function(#func, func##_lua)
#define BIND_ALL4(rc,func,a1,a2)     script_bind_function(#func, func##_lua)

#define XMACRO(X) /* @fixme: add all remaining ENGINE functions */ \
    X(bool, window_create, float, int ) \
    X(bool, window_swap ) \
    X(void, ddraw_grid, float ) \
    X(bool, ui_panel, string, int ) \
    X(bool, ui_notify, string, string ) \
    X(void, ui_panel_end )

XMACRO(WRAP_ALL)

void script_quit(void) {
    if( L ) {
        lua_close(L);
        L = 0;
    }
}
void script_init() {
    if( !L ) {
        // framework_init();

        // initialize Lua
        L = lua_newstate(script__realloc, 0); // L = luaL_newstate();

        // load various Lua libraries
        luaL_openlibs(L);
        luaopen_base(L);
        luaopen_table(L);
        luaopen_io(L);
        luaopen_string(L);
        luaopen_math(L);

#if !is(ems)
        // enable ffi (via luaffi)
        luaopen_ffi(L);
#endif

        // @fixme: workaround that prevents script binding on lua 5.4.3 on top of luajit 2.1.0-beta3 on linux. lua_setglobal() crashing when accessing null L->l_G
        if(L->l_G) {
        XMACRO(BIND_ALL);
        }

        atexit(script_quit);
    }
}

bool script_tests() {
    // script test (lua)
    script_run( "-- Bye.lua\nio.write(\"script test: Bye world!, from \", _VERSION, \"\\n\")" );
    return true;
}

#undef XMACRO

// script v2 ------------------------------------------------------------------

#define luaL_dostringsafe(L, str) \
    luaL_dostring(L, \
        "xpcall(function()\n" \
            str \
        "end, function(err)\n" \
        "  print('Error: ' .. tostring(err))\n" \
        "  print(debug.traceback(nil, 2))\n" \
        "  if core and core.on_error then\n" \
        "    pcall(core.on_error, err)\n" \
        "  end\n" \
        "  os.exit(1)\n" \
        "end)" \
    );

static int f_vfs_read(lua_State *L) {
    char *file = file_normalize(luaL_checkstring(L, 1));
    if( strbegi(file, app_path()) ) file += strlen(app_path());
    strswap(file+1, ".", "/");
    strswap(file+1, "/lua", ".lua");
    int len; char *data = vfs_load(file, &len);
    if( len ) {
        data = memcpy(MALLOC(len+1), data, len), data[len] = 0;
        //tty_color(data ? GREEN : RED);
        //printf("%s (%s)\n%s", file, data ? "ok" : "failed", data);
        //tty_color(0);
    }
    return lua_pushstring(L, data), 1; // "\n\tcannot find `%s` within mounted zipfiles", file), 1;
}

// add our zip loader at the end of package.loaders
void lua_add_ziploader(lua_State* L) {
    lua_pushcfunction( L, f_vfs_read );
    lua_setglobal( L, "vfs_read" );

    luaL_dostringsafe(L,
//    "package.path = [[;<?>;<<?.lua>>;]]\n" // .. package.path\n"
    "package.searchers[#package.searchers + 1] = function(libraryname)\n"
    "    for pattern in string.gmatch( package.path, '[^;]+' ) do\n"
    "        local proper_path = string.gsub(pattern, '?', libraryname)\n"
    "        local f = vfs_read(proper_path)\n"
    "        if f ~= nil then\n"
    "           return load(f, proper_path)\n"
    "        end\n"
    "    end\n"
    "    return nil\n"
    "end\n"
    );
}

void *script_init_env(unsigned flags) {
    if( flags & SCRIPT_LUA ) {
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);

        if( flags & SCRIPT_DEBUGGER ) {
            // Register debuggers/inspectors
            // luaL_dostringsafe(L, "I = require('inspect').inspect\n");
            dbg_setup_default(L);
        }

        lua_add_ziploader(L);
        return L;
    }

    return 0;
}

bool script_push(void *env) {
    array_push(Ls, L = env);
    return true;
}

bool script_pop() {
    L = array_count(Ls) && (array_pop(Ls), array_count(Ls)) ? *array_back(Ls) : NULL;
    return !!array_count(Ls);
}
