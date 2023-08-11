// dll utils
// - rlyeh, public domain

/// !!! `filename` must contain extension
/// load dynamic library `file` and search for `symbol`
/// return: NULL if not found, found symbol otherwise.
/// filename: path to dynamic library file. must contain extension.
/// symbol: symbol name. must not be NULL.
/// see: dlopen^, dlclose^
/// > bool (*plugin_init)(void) = dll("plugin.dll", "init");
/// > assert(plugin_init());
API void* dll(const char *filename, const char *symbol);
