typedef lua_State lua;

// the Lua interpreter
static lua *L;

#if is(linux)
void luaopen_libfwk(lua_State *L) {}
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

#define XMACRO(X) /* @fixme: add all remaining FWK functions */ \
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
        // fwk_init();

        // initialize Lua
        L = lua_newstate(script__realloc, 0); // L = luaL_newstate();

        // load various Lua libraries
        luaL_openlibs(L);
        luaopen_base(L);
        luaopen_table(L);
        luaopen_io(L);
        luaopen_string(L);
        luaopen_math(L);

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
