// dll ------------------------------------------------------------------------

/// !!! `filename` must contain extension
/// load dynamic library `file` and search for `symbol`
/// return: NULL if not found, found symbol otherwise.
/// filename: path to dynamic library file. must contain extension.
/// symbol: symbol name. must not be NULL.
/// see: dlopen^, dlclose^
/// > bool (*plugin_init)(void) = dll("plugin.dll", "init");
/// > assert(plugin_init());
API void* dll(const char *filename, const char *symbol);

// -----------------------------------------------------------------------------
// script framework

enum {
    SCRIPT_LUA = 1,
    SCRIPT_DEBUGGER = 2,
};

API void  script_init(); // @deprecate
API void *script_init_env(unsigned flags);
API bool  script_push(void *env);

API void    script_run(const char *script);
API void    script_runfile(const char *pathfile);

API void    script_bind_class(const char *objname, int num_methods, const char **c_names, void **c_functions);
API void    script_bind_function(const char *c_name, void *c_function);
API void    script_call(const char *lua_function);

API bool    script_tests();

API bool  script_pop();
