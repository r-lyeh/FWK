// -----------------------------------------------------------------------------
// script framework
// - rlyeh, public domain

API void script_init();
API void script_run(const char *script);
API void script_runfile(const char *pathfile);

API void script_bind_class(const char *objname, int num_methods, const char **c_names, void **c_functions);
API void script_bind_function(const char *c_name, void *c_function);
API void script_call(const char *lua_function);

API bool script_tests();
